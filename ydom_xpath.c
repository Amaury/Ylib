#include "ydom.h"
#include "ylog.h"

/* Private prototypes -- DON'T USE THEM */
static ybool_t _ydom_xpath_contains_pipe(const char *str);
static yvect_t _ydom_get_descendant_list(ydom_node_t *node);
static yvect_t _ydom_get_descendant_or_self_list(ydom_node_t *node);
static char *_ydom_get_next_xpath_name(const char **str);
static yvect_t _ydom_get_attributes_of_node(ydom_node_t *node, char *attr_name);
static yvect_t _ydom_get_children_of_node(ydom_node_t *node, char *child_name);
static ybool_t _ydom_is_child_of_node(ydom_node_t *node, char *child_name);
static ybool_t _ydom_is_attr_of_node(ydom_node_t *node, char *attr);
static ybool_t _ydom_is_boolean_expression(const char *pt);
static int _ydom_cmp_equal(char *s1, char *s2);
static int _ydom_cmp_not_equal(char *s1, char *s2);
static yvect_t _ydom_process_boolean(yvect_t list, const char *pt);

/* notes : a implementer
** noeud			-- OK
** noeud/noeud			-- OK
** ../noeud			-- OK
** ./noeud			-- OK
** *				-- OK
** //				-- OK
** /				-- OK
** @attribut			-- OK
** @*				-- OK
** noeud/noeud1 | noeud2	-- OK
** node()
** text()
** comment()
** processing-instruction()
** /toto[1]			1er noeud 'toto'
** /toto[titi]			les noeuds 'toto' ayant un fils 'titi'
** /toto[@titre='toto']		les noeuds 'toto' avec @titre='toto'
** /toto[titi | tutu]		les noeuds 'toto' ayant un fils 'titi' ou 'tutu'
** /toto[@titre='toto' | tutu]
** /toto[@titre='toto' | @chapitre='tutu']
** /toto[last()]		le dernier noeud 'toto'
** /toto[position() &lt; 3]	les deux premiers noeuds 'toto'
** /toto[position() != last()]	tous les noeuds 'toto' sauf le dernier
*/

/*
** ydom_get_nodes_from_xpath()
** main XPath parsing function
** return a set of nodes, or NULL if the path isn't correct
*/
yvect_t ydom_get_nodes_from_xpath(yvect_t nodes, char *xpath)
{
  const char *pt = NULL;
  ybool_t first_char = YTRUE;
  yvect_t set, tmp;
  ydom_node_t *node = NULL;
  char *tmp_char = NULL;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!yv_len(nodes) || xpath == NULL || !strlen(xpath))
    {
      YLOG_MOD("ydom", YLOG_DEBUG, "No input nodes");
      return (NULL);
    }
  if (!(set = yv_new()))
    {
      YLOG_MOD("ydom", YLOG_DEBUG, "Memory error");
      return (NULL);
    }
  for (pt = xpath, first_char = YTRUE; *pt; pt++, first_char = YFALSE)
    {
      if (IS_SPACE(*pt))
	continue ;
      if (*pt == SLASH)
	{
	  if (*(pt + 1) == SLASH)
	    {
	      /* Xpath example : "//foo" */
	      pt++;
	      while ((node = yv_pop(nodes)))
		{
		  tmp = _ydom_get_descendant_or_self_list(node);
		  yv_cat(&set, tmp);
		  yv_del(&tmp, NULL, NULL);
		}
	      tmp = nodes;
	      nodes = set;
	      set = tmp;
	    }
	  else if (first_char)
	    {
	      /* Xpath example : "/foo" */
	      while ((node = yv_pop(nodes)))
		yv_add(&set, _ydom_get_root_node_of_node(node));
	      tmp = nodes;
	      nodes = set;
	      set = tmp;
	    }
	}
      else if (*pt == DOT && *(pt + 1) == DOT)
	{
	  /* Xpath example : ".." */
	  pt++;
	  while ((node = yv_pop(nodes)))
	    if (node->parent && node->parent->node_type == ELEMENT_NODE)
	      yv_add(&set, node->parent);
	  tmp = nodes;
	  nodes = set;
	  set = tmp;
          yv_uniq(nodes);
	}
      else if (*pt == AT)
	{
	  /* Xpath examples : "@attribute" or "@*" */
	  pt++;
	  tmp_char = _ydom_get_next_xpath_name(&pt);
	  pt--;
	  if (tmp_char == NULL)
	    continue ;
	  while ((node = yv_pop(nodes)))
	    {
	      tmp = _ydom_get_attributes_of_node(node, tmp_char);
	      yv_cat(&set, tmp);
	      yv_del(&tmp, NULL, NULL);
	    }
	  tmp = nodes;
	  nodes = set;
	  set = tmp;
	  free0(tmp_char);
	}
      else if (*pt == LBRACKET)
	{
	  yvect_t tmp2;
	  if (!(tmp2 = yv_new()))
            {
              YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
	      return (NULL);
            }
	  /* Xpath examples : "toto[titi | tutu]" */
	  pt++;
	  while (_ydom_xpath_contains_pipe(pt))
	    {
	      tmp_char = _ydom_get_next_xpath_name(&pt);
	      yv_add(&set, tmp_char);
	      for (; *pt == PIPE || IS_SPACE(*pt); ++pt)
		;
	    }
	  if (!(tmp_char = _ydom_get_next_xpath_name(&pt)))
            continue ;
	  yv_add(&set, tmp_char);
	  while ((node = yv_pop(nodes)))
	    {
              int offset;
	      for (offset = 0; offset < yv_len(set); ++offset)
		{
                  tmp_char = set[offset];
		  if (*tmp_char == AT)
		    {
		      if (_ydom_is_attr_of_node(node, tmp_char))
			yv_add(&tmp2, node);
		    }
		  else if (_ydom_is_child_of_node(node, tmp_char))
		    yv_add(&tmp2, node);
		}
	    }
          while ((tmp_char = yv_pop(set)))
            free0(tmp_char);
	  yv_del(&nodes, NULL, NULL);
	  yv_trunc(set, NULL, NULL);
	  nodes = tmp2;
	}
      else if (_ydom_is_boolean_expression(pt))
	{
	  /* Xpath example : "toto != 'titi'" */
          YLOG_MOD("ydom", YLOG_DEBUG, "Exiting after boolean expression");
	  return (_ydom_process_boolean(nodes, pt));
	}
      else
	{
	  while (_ydom_xpath_contains_pipe(pt))
	    {
	      int i;
	      tmp_char = _ydom_get_next_xpath_name(&pt);
	      for (i = 0; i < yv_len(nodes); ++i)
		{
		  node = nodes[i];
		  tmp = _ydom_get_children_of_node(node, tmp_char);
		  yv_cat(&set, tmp);
		  yv_del(&tmp, NULL, NULL);
		}
	      free0(tmp_char);
	      for (; *pt == PIPE || IS_SPACE(*pt); ++pt)
		;
	    }
	  tmp_char = _ydom_get_next_xpath_name(&pt);
	  pt--;
	  if (tmp_char == NULL)
            {
              YLOG_MOD("ydom", YLOG_DEBUG, "Exiting abnormally");
	      return (nodes);
            }
	  while ((node = yv_pop(nodes)))
	    {
	      if ((tmp = _ydom_get_children_of_node(node, tmp_char)))
		{
		  yv_cat(&set, tmp);
		  yv_del(&tmp, NULL, NULL);
		}
	    }
	  free0(tmp_char);
	  tmp = nodes;
	  nodes = set;
	  set = tmp;
	}
    }
  yv_del(&set, NULL, NULL);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
  return (nodes);
}

/*
** _ydom_xpath_contains_pipe() -- INTERNAL FUNCTION
** Return YTRUE if the string contains a '|' charactere, 0 otherwise
*/
static ybool_t _ydom_xpath_contains_pipe(const char *str)
{
  while (IS_SPACE(*str))
    str++;
  while (*str && *str != LT && *str != GT && *str != SLASH &&
	 *str != INTERROG && *str != EXCLAM && /**str != EQ &&*/
	 *str != LBRACKET && *str != RBRACKET &&
	 /**str != AT &&*/ *str != PIPE)
    str++;
  if (*str == PIPE)
    return (YTRUE);
  return (YFALSE);
}

/*
** _ydom_get_descendant_list() -- INTERNAL FUNCTION
** return a vector of nodes which contains copies of each sons, sons of sons, ...
** of the node
*/
static yvect_t _ydom_get_descendant_list(ydom_node_t *node)
{
  ydom_node_t *node_pt = NULL;
  yvect_t tmp;
  yvect_t res;
  
  if (!(res = yv_new()))
    return (NULL);
  for (node_pt = node->first_child; node_pt; node_pt = node_pt->next)
    {
      if (node_pt->node_type == ELEMENT_NODE)
	{
	  yv_add(&res, node_pt);
	  tmp = _ydom_get_descendant_list(node_pt);
	  yv_cat(&res, tmp);
	  yv_del(&tmp, NULL, NULL);
	}
    }
  return (res);
}

/*
** _ydom_get_descendant_or_self_list() -- INTERNAL FUNCTION
** return a vector of nodes which contains a copy of the node, and copies of each
** sons, sons of sons, ... of the node
*/
static yvect_t _ydom_get_descendant_or_self_list(ydom_node_t *node)
{
  yvect_t tmp;
  yvect_t res;

  if (!(res = yv_new()))
    return (NULL);
  yv_add(&res, node);
  tmp = _ydom_get_descendant_list(node);
  yv_cat(&res, tmp);
  yv_del(&tmp, NULL, NULL);
  return (res);
}

/*
** _ydom_get_next_xpath_name() -- INTERNAL FUNCTION
** return a copy of the next name in an Xpath string, and put the string pointer
** at the end of the name
*/
static char *_ydom_get_next_xpath_name(const char **str)
{
  ystr_t res;
  char *result;
  int end, have_quote, have_dblquote;
  
  while (IS_SPACE(**str))
    (*str)++;
  end = have_quote = have_dblquote = 0;
  if (!(res = ys_new("")))
    return (NULL);
  while (**str && **str != LT && **str != GT && **str != SLASH &&
	 **str != INTERROG && **str != EXCLAM && /***str != EQ &&*/
	 **str != LBRACKET && **str != RBRACKET &&
	 /***str != AT &&*/ **str != PIPE && !IS_SPACE(**str))
    {
      if (**str == LPAR || **str == RPAR)
	{
	  ys_del(&res);
	  return (NULL);
	}
      ys_addc(&res, **str);
      (*str)++;
      if (*(*str - 1) == QUOTE)
	{
	  for (; **str != QUOTE; (*str)++)
	    ys_addc(&res, **str);
	  if (**str == QUOTE)
	    {
	      ys_addc(&res, **str);
	      (*str)++;
	    }
	}
      else if (*(*str - 1) == DQUOTE)
	{
	  for (; **str != DQUOTE; (*str)++)
	    ys_addc(&res, **str);
	  if (**str == DQUOTE)
	    {
	      ys_addc(&res, **str);
	      (*str)++;
	    }
	}
    }
  while (IS_SPACE(**str))
    (*str)++;
  result = ys_string(res);
  ys_del(&res);
  return (result);
}

/*
** _ydom_get_attributes_of_node() -- INTERNAL FUNCTION
** return a list of attributes matching a name
** if the attribute name parameter is equal to NULL, all attributes are returned
*/
static yvect_t _ydom_get_attributes_of_node(ydom_node_t *node, char *attr_name)
{
  ydom_node_t *node_pt = NULL;
  yvect_t res;

  if (!(res = yv_new()))
    return (NULL);
  if (*attr_name == STAR)
    attr_name = NULL;
  for (node_pt = node->attributes; node_pt; node_pt = node_pt->next)
    {
      if (attr_name == NULL || !strcmp(attr_name, node_pt->name))
	yv_add(&res, node_pt);
    }
  return (res);
}

/*
** _ydom_get_children_of_node() -- INTERNAL FUNCTION
** return a vector of children nodes matching a name
** if the child name parameter is equal to NULL, all children are returned
** return NULL if the node doesn't have any child
*/
static yvect_t _ydom_get_children_of_node(ydom_node_t *node, char *child_name)
{
  ydom_node_t *node_pt = NULL;
  yvect_t res;

  if (!(res = yv_new()))
    return (NULL);
  if (*child_name == STAR)
    child_name = NULL;
  for (node_pt = node->first_child; node_pt; node_pt = node_pt->next)
    {
      if (node_pt->node_type == ELEMENT_NODE &&
	  (child_name == NULL || !strcmp(child_name, node_pt->name)))
	{
	  yv_add(&res, node_pt);
	}
    }
  return (res);
}

/*
** _ydom_is_child_of_node()
** return YTRUE if a node have a given child, otherwise 0
*/
static ybool_t _ydom_is_child_of_node(ydom_node_t *node, char *child_name)
{
  ydom_node_t *node_pt = NULL;

  for (node_pt = node->first_child; node_pt; node_pt = node_pt->next)
    {
      if (!strcmp(child_name, node_pt->name))
	return (YTRUE);
    }
  return (YFALSE);
}

/*
** _ydom_is_attr_of_node()
** return YTRUE if a node have a given attribute
*/
static ybool_t _ydom_is_attr_of_node(ydom_node_t *node, char *attr)
{
  ystr_t attr_name, attr_value;
  char *pt, *pt2;
  ydom_node_t *attr_pt = NULL;
  ybool_t res = YFALSE;
  char init_delim = '\0';

  if (!(attr_name = ys_new("")))
    return (YFALSE);
  if (!(attr_value = ys_new("")))
    {
      ys_del(&attr_name);
      return (YFALSE);
    }
  attr = (*attr == AT) ? (attr + 1) : attr;
  if ((pt = strchr(attr, EQ)))
    {
      ys_ncat(&attr_name, attr, pt - attr);
      pt++;
      for (; IS_SPACE(*pt); ++pt)
	;
      if (*pt == QUOTE || *pt == DQUOTE)
	{
	  init_delim = *pt;
	  pt++;
	}
      if (init_delim == QUOTE)
	{
	  if (!(pt2 = strchr(pt, QUOTE)))
	    ys_cat(&attr_value, pt);
	  else
	    ys_ncat(&attr_value, pt, pt2 - pt);
	}
      else if (!(pt2 = strchr(pt, DQUOTE)))
	ys_cat(&attr_value, pt);
      else
	  ys_ncat(&attr_value, pt, pt2 - pt);
      /*
	tmp = str2xmlentity(attr_value);
	free0(attr_value);
	attr_value = tmp;
      */
    }
  else
    ys_cat(&attr_name, attr);
  for (attr_pt = node->attributes; attr_pt && !res; attr_pt = attr_pt->next)
    if (!strcmp(attr_name, attr_pt->name) &&
	(!ys_len(attr_value) || !strcmp(attr_value, attr_pt->value)))
      res = YTRUE;
  ys_del(&attr_name);
  ys_del(&attr_value);
  return (res);
}

/*
** _ydom_is_boolean_expression()
** return YTRUE if the next word of an XPath expression is a boolean expression.
*/
static ybool_t _ydom_is_boolean_expression(const char *pt)
{
  if (!strncmp(pt, "=", 1) || !strncmp(pt, "!=", 2) || !strncmp(pt, "&gt;", 4) || 
      !strncmp(pt, "&lt;", 4) || !strncmp(pt, "&lt;=", 5) || !strncmp(pt, "&gt;=", 5))
    return (YTRUE);
  return (YFALSE);
}

static int _ydom_cmp_equal(char *s1, char *s2)
{
  return (strcmp(s1, s2));
}

static int _ydom_cmp_not_equal(char *s1, char *s2)
{
  return (!strcmp(s1, s2));
}

/*
** _ydom_process_boolean()
** process a boolean expression
*/
static yvect_t _ydom_process_boolean(yvect_t list, const char *pt)
{
  ydom_node_t *node = NULL;
  ybool_t tmp_bool = YFALSE;
  yvect_t res;
  int (*f)();
  char *cmp, *s1, *s2;
  char init_delim;
  int i;

  if (!(res = yv_new()))
    return (NULL);
  if (!strncmp(pt, "=", 1))
    {
      f = _ydom_cmp_equal;
      pt++;
    }
  else if (!strncmp(pt, "!=", 2))
    {
      f = _ydom_cmp_not_equal;
      pt += 2;
    }
  else
    {
      yv_del(&res, NULL, NULL);
      return (NULL);
    }
  cmp = _ydom_get_next_xpath_name(&pt);
  if (cmp[0] == QUOTE || cmp[0] == DQUOTE)
    {
      init_delim = cmp[0];
      for (i = 0; cmp[i]; ++i)
	cmp[i] = cmp[i + 1];
      if ((cmp[strlen(cmp) - 1] == QUOTE && init_delim == QUOTE)|| 
	  (cmp[strlen(cmp) - 1] == DQUOTE && init_delim == DQUOTE))
	{
	  cmp[strlen(cmp) - 1] = '\0';
	}
    }
  while ((node = yv_pop(list)))
    {
      s1 = ydom_node_get_value(node);
      s2 = ydom_node_get_name(node);
      if ((ydom_node_is_attr(node) && !f(s1, cmp)) ||
	  (ydom_node_is_element(node) && !f(s2, cmp)))
	{
	  free0(s1);
	  free0(s2);
	  tmp_bool = YTRUE;
	  break ;
	}
      free0(s1);
      free0(s2);
    }
  yv_del(&list, NULL, NULL);
  free0(cmp);
  if (tmp_bool)
    {
      yv_add(&res, node);
    }
  return (res);
}
