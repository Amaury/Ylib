#include <stdio.h>
#include "ylog.h"
#include "ydom.h"

/* Private prototypes */
static void _ydom_open_hdlr(ysax_t *sax, char *tag_name, yvect_t attrs);
static void _ydom_close_hdlr(ysax_t *sax, char *tag_name);
static void _ydom_inside_hdlr(ysax_t *sax, char *str);
static void _ydom_comment_hdlr(ysax_t *sax, char *s);
static void _ydom_process_instr_hdlr(ysax_t *sax, char *target, char *content);
static void _ydom_cdata_hdlr(ysax_t *sax, char *content);
static void _ydom_add_child_to_node(ydom_node_t *node, ydom_node_t *child);
static void _ydom_add_next_to_node(ydom_node_t *node, ydom_node_t *next);
static ydom_node_t *_ydom_add_attr_to_node(ydom_node_t *node,
					   char *attr_name, char *attr_value);

static void _ydom_write_node(ydom_node_t *node, int nb_tab, FILE *file);
static void _ydom_write_node_attr(ydom_node_t *node, FILE *file);
static void _ydom_dump_node(ydom_node_t *node, ystr_t *s);
static void _ydom_dump_node_attr(ydom_node_t *node, ystr_t *s);

/*
** ydom_new()
** Create a new XML DOM object.
*/
ydom_t *ydom_new()
{
  ydom_t *dom;
  ydom_node_t *node;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!(dom = malloc0(sizeof(ydom_t))))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (NULL);
    }
  if (!(node = malloc0(sizeof(ydom_node_t))))
    {
      free0(dom);
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (NULL);
    }
  node->node_type = DOCUMENT_NODE;
  node->complete = YTRUE;
  node->name = NULL;
  dom->error = YENOERR;
  dom->document_element = dom->current_parsed_node = node;
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
  return (dom);
}

/*
** ydom_del()
** Delete a previously created XML DOM object and all memory allocated for it.
*/
void ydom_del(ydom_t *dom)
{
  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  ydom_node_rm(dom->document_element);
  if (dom->xml_version)
    free0(dom->xml_version);
  if (dom->encoding)
    free0(dom->encoding);
  if (dom->standalone)
    free0(dom->standalone);
  free0(dom);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** ydom_read_file()
** Parse an existing XML file.
*/
yerr_t ydom_read_file(ydom_t *dom, const char *filename)
{
  ysax_t *sax;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!dom)
    {
      YLOG_ADD(YLOG_WARN, "NULL dom pointer");
      return (YENOERR);
    }
  if ((sax = ysax_new(dom)) == NULL)
    {
      YLOG_ADD(YLOG_ERR, "Bad SAX creation");
      return (YENOENT);
    }
  ysax_set_tag_hdlr(sax, _ydom_open_hdlr, _ydom_close_hdlr);
  ysax_set_inside_text_hdlr(sax, _ydom_inside_hdlr);
  ysax_set_comment_hdlr(sax, _ydom_comment_hdlr);
  ysax_set_process_instr_hdlr(sax, _ydom_process_instr_hdlr);
  ysax_set_cdata_hdlr(sax, _ydom_cdata_hdlr);
  dom->error = ysax_read_file(sax, filename);
  ysax_del(sax);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
  return (dom->error);
}

/*
** ydom_read_stream()
** Parse a stream (opened file, pipe, ...) that contains XML. If you have a file
** descriptor instead of a stream, use fdopen() to convert it.
*/
yerr_t ydom_read_stream(ydom_t *dom, FILE *stream)
{
  ysax_t *sax;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!dom)
    {
      YLOG_ADD(YLOG_WARN, "NULL dom pointer");
      return (YENOERR);
    }
  if ((sax = ysax_new(dom)) == NULL)
    {
      YLOG_ADD(YLOG_ERR, "Bad SAX creation");
      return (YENOENT);
    }
  ysax_set_tag_hdlr(sax, _ydom_open_hdlr, _ydom_close_hdlr);
  ysax_set_inside_text_hdlr(sax, _ydom_inside_hdlr);
  ysax_set_comment_hdlr(sax, _ydom_comment_hdlr);
  ysax_set_process_instr_hdlr(sax, _ydom_process_instr_hdlr);
  ysax_set_cdata_hdlr(sax, _ydom_cdata_hdlr);
  dom->error = ysax_read_stream(sax, stream);
  ysax_del(sax);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
  return (dom->error);
}

/*
** ydom_read_memory()
** Parse a character string that contains XML.
*/
yerr_t ydom_read_memory(ydom_t *dom, char *mem)
{
  ysax_t *sax;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!dom)
    {
      YLOG_ADD(YLOG_WARN, "NULL dom pointer");
      return (YENOERR);
    }
  if ((sax = ysax_new(dom)) == NULL)
    {
      YLOG_ADD(YLOG_ERR, "Bad SAX creation");
      return (YENOENT);
    }
  ysax_set_tag_hdlr(sax, _ydom_open_hdlr, _ydom_close_hdlr);
  ysax_set_inside_text_hdlr(sax, _ydom_inside_hdlr);
  ysax_set_comment_hdlr(sax, _ydom_comment_hdlr);
  ysax_set_process_instr_hdlr(sax, _ydom_process_instr_hdlr);
  ysax_set_cdata_hdlr(sax, _ydom_cdata_hdlr);
  dom->error = ysax_read_memory(sax, mem);
  ysax_del(sax);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
  return (dom->error);
}

/*
** ydom_write()
** Write the content of an XML tree to a stream.
*/
void ydom_write(ydom_t *dom, FILE *stream)
{
  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!dom)
    {
      YLOG_ADD(YLOG_WARN, "NULL dom pointer");
      return ;
    }
  if (dom->error != YENOERR)
    {
      YLOG_ADD(YLOG_ERR, "Previous error");
      return ;
    }
  fprintf(stream, "<?xml");
  if (dom->xml_version)
    fprintf(stream, " version=\"%s\"", dom->xml_version);
  if (dom->encoding)
    fprintf(stream, " encoding=\"%s\"", dom->encoding);
  if (dom->standalone)
    fprintf(stream, " standalone=\"%s\"", dom->standalone);
  fprintf(stream, "?>\n");
  _ydom_write_node(dom->document_element, 0, stream);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** ydom_dump()
** Allocate a memory buffer and dump into it the XML content.
*/
ystr_t ydom_dump(ydom_t *dom)
{
  ystr_t str;

  if (!dom || dom->error != YENOERR || !(str = ys_new("<?xml")))
    {
      return (NULL);
    }
  if (dom->xml_version)
    {
      ys_cat(&str, " version=\"");
      ys_cat(&str, dom->xml_version);
      ys_addc(&str, '\"');
    }
  if (dom->encoding)
    {
      ys_cat(&str, " encoding=\"");
      ys_cat(&str, dom->encoding);
      ys_addc(&str, '\"');
    }
  if (dom->standalone)
    {
      ys_cat(&str, " standalone=\"");
      ys_cat(&str, dom->standalone);
      ys_addc(&str, '\"');
    }
  ys_cat(&str, "?>");
  _ydom_dump_node(dom->document_element, &str);
  return (str);
}

/*
** ydom_set_version()
** Set the version of XML language used in the document.
*/
void ydom_set_version(ydom_t *dom, char *version)
{
  if (!dom)
    return ;
  free0(dom->xml_version);
  if (version)
    dom->xml_version = strdup(version);
}

/*
** ydom_set_encoding()
** Set the character encoding (like "iso-8859-1") of the document.
*/
void ydom_set_encoding(ydom_t *dom, char *encoding)
{
  if (!dom)
    return ;
  free0(dom->encoding);
  if (encoding)
    dom->encoding = strdup(encoding);
}

/*
** ydom_set_standalone()
** Set the standalone state of the document.
*/
void ydom_set_standalone(ydom_t *dom, char *standalone)
{
  if (!dom)
    return ;
  free0(dom->standalone);
  if (standalone)
    dom->standalone = strdup(standalone);
}

/*
** ydom_add_elem()
** Create an XML element (a tag or a node, if you prefer) in the top-level
** hierarchy of the XML document.
*/
ydom_node_t *ydom_add_elem(ydom_t *dom, char *tagname)
{
  return (!dom ? NULL : ydom_node_add_elem(dom->document_element, tagname));
}

/*
** ydom_add_text()
** Create a text node in the top-level hierarchy of the XML document.
*/
ydom_node_t *ydom_add_text(ydom_t *dom, char *data)
{
  return (!dom ? NULL : ydom_node_add_text(dom->document_element, data));
}

/*
** ydom_add_comment()
** Create an XML comment node in the top-level hierarchy of the XML document.
*/
ydom_node_t *ydom_add_comment(ydom_t *dom, char *data)
{
  return (!dom ? NULL : ydom_node_add_comment(dom->document_element, data));
}

/*
** ydom_add_process_instr()
** Create an XML processing instruction in the top-level hierarchy of the
** XML document.
*/
ydom_node_t *ydom_add_process_instr(ydom_t *dom, char *target, char *data)
{
  return (!dom ? NULL : ydom_node_add_process_instr(dom->document_element, target, data));
}

/*
** ydom_add_cdata()
** Create a CDATA section in the top-level hierarchy of the XML document.
*/
ydom_node_t *ydom_add_cdata(ydom_t *dom, char *data)
{
  return (!dom ? NULL : ydom_node_add_cdata(dom->document_element, data));
}

/*
** ydom_get_document()
** Return a pointer to the document element node of an XML tree.
*/
ydom_node_t *ydom_get_document(ydom_t *dom)
{
  return (!dom ? NULL : dom->document_element);
}

/*
** ydom_xpath()
** Search for nodes or attributes in a XML tree from a XPath string.
*/
yvect_t ydom_xpath(ydom_t *dom, char *xpath)
{
  ydom_node_t *node;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!dom || !dom->document_element || !dom->document_element->first_child)
    return (NULL);
  for (node = dom->document_element->first_child;
       node->node_type != ELEMENT_NODE;
       node = node->next)
    ;
  if (!node)
    return (NULL);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
  return (ydom_node_xpath(node, xpath));
}

/*
** ydom_sort()
** Do a quick sort on all nodes of the XML document.
*/
void ydom_sort(ydom_t *dom, int (*func)(const void*, const void*))
{
  if (!dom || !dom->document_element || !dom->document_element->first_child)
    return ;
  ydom_node_sort_all(dom->document_element, func);
}

/*
** ydom_node_is_element()
** Check if a node is an element (an XML tag), or not.
*/
ybool_t ydom_node_is_element(ydom_node_t *node)
{
  if (node && node->node_type == ELEMENT_NODE)
    return (YTRUE);
  return (YFALSE);
}

/*
** ydom_node_is_text()
** Check if a node is a text node or not.
*/
ybool_t ydom_node_is_text(ydom_node_t *node)
{
  if (node && node->node_type == TEXT_NODE)
    return (YTRUE);
  return (YFALSE);
}

/*
** ydom_node_is_comment()
** Check if a node is a comment node or not.
*/
ybool_t ydom_node_is_comment(ydom_node_t *node)
{
  if (node && node->node_type == COMMENT_NODE)
    return (YTRUE);
  return (YFALSE);
}

/*
** ydom_node_is_process_instr()
** Check if a node if a processing instruction node, or not.
*/
ybool_t ydom_node_is_process_instr(ydom_node_t *node)
{
  if (node && node->node_type == PROCESSING_INSTRUCTION_NODE)
    return (YTRUE);
  return (YFALSE);
}

/*
** ydom_node_is_cdata()
** Check if a node is a CDATA section or not.
*/
ybool_t ydom_node_is_cdata(ydom_node_t *node)
{
  if (node && node->node_type == CDATA_SECTION_NODE)
    return (YTRUE);
  return (YFALSE);
}

/*
** ydom_node_is_attr()
** Check if a node is an attribute node or not.
*/
ybool_t ydom_node_is_attr(ydom_node_t *node)
{
  if (node && node->node_type == ATTRIBUTE_NODE)
    return (YTRUE);
  return (YFALSE);
}

/*
** ydom_node_is_document()
** Check if a node is the document node (logical node which contains all the
** top-level hierarchy of the XML document).
*/
ybool_t ydom_node_is_document(ydom_node_t *node)
{
  if (node && node->node_type == DOCUMENT_NODE)
    return (YTRUE);
  return (YFALSE);
}

/*
** ydom_node_add_attr()
** Add an attribute to an existing XML node.
*/
ydom_node_t *ydom_node_add_attr(ydom_node_t *node, char *attr_name, char *attr_value)
{
  char *name;
  char *value;

  if (!node)
    return (NULL);
  value = str2xmlentity(attr_value);
  name = strdup(attr_name);
  return (_ydom_add_attr_to_node(node, name, value));
}

/*
** ydom_node_set_attr()
** Add or update an attribute.
*/
ydom_node_t *ydom_node_set_attr(ydom_node_t *node, char *attr_name, char *attr_value)
{
  char *value, *name;
  ydom_node_t *pt;

  if (!node)
    return (NULL);
  value = str2xmlentity(attr_value);
  for (pt = node->attributes; pt; pt = pt->next)
    {
      if (!strcmp(pt->name, attr_name))
	{
	  free0(pt->value);
	  pt->value = value;
	  return (pt);
	}
    }
  name = strdup(attr_name);
  return (_ydom_add_attr_to_node(node, name, value));
}

/*
** ydom_node_add_elem()
** Create an XML element (a tag or a node, if you prefer) and insert it as
** a child of an existing node.
*/
ydom_node_t *ydom_node_add_elem(ydom_node_t *node, char *tagname)
{
  ydom_node_t *new_node;

  if (!node || !(new_node = malloc0(sizeof(ydom_node_t))))
    return (NULL);
  new_node->node_type = ELEMENT_NODE;
  new_node->name = tagname;
  new_node->complete = YTRUE;
  _ydom_add_child_to_node(node, new_node);
  return (new_node);
}

/*
** ydom_node_add_text()
** Create a text node and insert it as a child of an existing node.
*/
ydom_node_t *ydom_node_add_text(ydom_node_t *node, char *data)
{
  ydom_node_t *text_node = NULL;
  char *tmp;

  if (!node)
    return (NULL);
  if (node->last_child && node->last_child->node_type == TEXT_NODE)
    {
      tmp = malloc0(strlen(node->last_child->value) + strlen(data) + 1);
      strcpy(tmp, node->last_child->value);
      strcat(tmp, data);
      free0(node->last_child->value);
      node->last_child->value = tmp;
    }
  else
    {
      if (!(text_node = malloc0(sizeof(ydom_node_t))))
	return (NULL);
      text_node->node_type = TEXT_NODE;
      text_node->value = str2xmlentity(data);
      text_node->complete = YTRUE;
      _ydom_add_child_to_node(node, text_node);
    }
  return (text_node);
}

/*
** ydom_node_add_comment()
** Create an XML comment node and insert it as a child of an existing node.
*/
ydom_node_t *ydom_node_add_comment(ydom_node_t *node, char *data)
{
  ydom_node_t *new_node;

  if (!node || !(new_node = malloc0(sizeof(ydom_node_t))))
    return (NULL);
  new_node->node_type = COMMENT_NODE;
  new_node->value = data;
  new_node->complete = YTRUE;
  _ydom_add_child_to_node(node, new_node);
  return (new_node);
}

/*
** ydom_node_add_process_instr()
** Create an XML processing instruction and insert it as a child of an
** existing node.
*/
ydom_node_t *ydom_node_add_process_instr(ydom_node_t *node, char *target,
					 char *data)
{
  ydom_node_t *new_node;

  if (!node || !(new_node = malloc0(sizeof(ydom_node_t))))
    return (NULL);
  new_node->node_type = PROCESSING_INSTRUCTION_NODE;
  new_node->name = target;
  new_node->value = data;
  new_node->complete = YTRUE;
  _ydom_add_child_to_node(node, new_node);
  return (new_node);
}

/*
** ydom_node_add_cdata()
** Create a CDATA section and insert it as a child of an existing node.
*/
ydom_node_t *ydom_node_add_cdata(ydom_node_t *node, char *data)
{
  ydom_node_t *new_node;

  if (!node || !(new_node = malloc0(sizeof(ydom_node_t))))
    return (NULL);
  new_node->node_type = CDATA_SECTION_NODE;
  new_node->value = data;
  new_node->complete = YTRUE;
  _ydom_add_child_to_node(node, new_node);
  return (new_node);
}

/*
** ydom_node_get_name()
** Return a copy of the name of a node.
*/
char *ydom_node_get_name(ydom_node_t *node)
{
  if (!node || !node->name)
    return (NULL);
  return (strdup(node->name));
}

/*
** ydom_node_get_value()
** Return a copy of the value of a node.
*/
char *ydom_node_get_value(ydom_node_t *node)
{
  if (!node || !node->value)
    return (NULL);
  return (xmlentity2str(node->value));
}

/*
** ydom_node_get_nbr_children()
** Return the number of children of a given XML node.
*/
int ydom_node_get_nbr_children(ydom_node_t *node)
{
  ydom_node_t *pt;
  int res;

  if (!node)
    return (0);
  for (res = 0, pt = node->first_child; pt; pt = pt->next, ++res)
    ;
  return (res);
}

/*
** ydom_node_get_nbr_attr()
** Return the number of attributes of a given node.
*/
int ydom_node_get_nbr_attr(ydom_node_t *node)
{
  ydom_node_t *pt;
  int res;

  if (!node)
    return (0);
  for (res = 0, pt = node->attributes; pt; pt = pt->next, ++res)
    ;
  return (res);
}

/*
** ydom_node_get_nbr_same_attr()
** Return the number of attributes of a given node, with the same name.
*/
int ydom_node_get_nbr_same_attr(ydom_node_t *node, char *attr_name)
{
  ydom_node_t *pt;
  int res;

  if (!node)
    return (0);
  if (!attr_name)
    return (ydom_node_get_nbr_attr(node));
  for (res = 0, pt = node->attributes; pt; pt = pt->next)
    if (!strcmp(pt->name, attr_name))
      res++;
  return (res);
}

/*
** ydom_node_get_parent()
** Return a pointer to the parent node of an XML node.
*/
ydom_node_t *ydom_node_get_parent(ydom_node_t *node)
{
  return (!node ? NULL : node->parent);
}

/*
** ydom_node_get_prev()
** Return a pointer to the previous sibling node of an XML node.
*/
ydom_node_t *ydom_node_get_prev(ydom_node_t *node)
{
  return (!node ? NULL : node->prev);
}

/*
** ydom_node_get_next()
** Return a pointer to the next sibling node of an XML node.
*/
ydom_node_t *ydom_node_get_next(ydom_node_t *node)
{
  return (!node ? NULL : node->next);
}

/*
** ydom_node_get_attr_value()
** Return the value of the first attribute of a node that have a given name.
*/
char *ydom_node_get_attr_value(ydom_node_t *node, char *attr_name)
{
  ydom_node_t *attr;

  if (!node)
    return (NULL);
  for (attr = node->attributes; attr; attr = attr->next)
    if (!strcmp(attr_name, attr->name))
      return (xmlentity2str(attr->value));
  return (NULL);
}


/*
** ydom_node_get_first_attr()
** Return a pointer to the first attribute of a node.
*/
ydom_node_t *ydom_node_get_first_attr(ydom_node_t *node)
{
  return (!node ? NULL : node->attributes);
}

/*
** ydom_node_get_first_child()
** Return a pointer to the first child of a node.
*/
ydom_node_t *ydom_node_get_first_child(ydom_node_t *node)
{
  return (!node ? NULL : node->first_child);
}

/*
** ydom_node_get_last_child()
** Return a pointer to the last child of a node.
*/
ydom_node_t *ydom_node_get_last_child(ydom_node_t *node)
{
  return (!node ? NULL : node->last_child);
}

/*
** ydom_node_xpath()
** Search for nodes or attributes from a XPath string, taking a given node
** as root.
*/
yvect_t ydom_node_xpath(ydom_node_t *node, char *xpath)
{
  yvect_t res;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  if (!node || !(res = yv_new()))
    return (NULL);
  yv_put(&res, node);
  res = ydom_get_nodes_from_xpath(res, xpath);
  /*
  if (!yv_len(res)) {
    yv_del(&res, NULL, NULL);
    res = NULL;
  }
  */
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting",
           yv_len(res));
  return (res);
}

/*
** ydom_node_sort()
** Do a quick sort on the children of a node. Sub-children are not sorted.
*/
void ydom_node_sort(ydom_node_t *node, int (*func)(const void *, const void*))
{
  ydom_node_t *pt, *prev, *next;
  yvect_t array;
  char str_star[2] = {STAR, '\0'};

  if ((array = ydom_node_xpath(node, str_star)))
    {
      if (array && yv_len(array))
	{
	  yv_sort(array, func);
	  for (prev = NULL; (pt = yv_pop(array)); )
	    {
	      next = array[0];
	      pt->prev = prev;
	      pt->next = next;
	      if (!prev)
		node->first_child = pt;
	      if (!next)
		node->last_child = pt;
	      prev = pt;
	    }
	}
      yv_del(&array, NULL, NULL);
    }
}

/*
** ydom_node_sort_all()
** Do a quick sort on the children of a node. Sub-children, sub-sub-children, ... are
** recursivly sorted.
*/
void ydom_node_sort_all(ydom_node_t *node, int (*func)(const void*, const void*))
{
  ydom_node_t *pt, *prev, *next;
  yvect_t array;
  char str_star[2] = {STAR, '\0'};
  
  if ((array = ydom_node_xpath(node, str_star)))
    {
      if (array && yv_len(array))
	{
	  yv_sort(array, func);
	  for (prev = NULL; (pt = yv_pop(array)); )
	    {
	      next = array[0];
	      pt->prev = prev;
	      pt->next = next;
	      ydom_node_sort_all(pt, func);
	      if (!prev)
		node->first_child = pt;
	      if (!next)
		node->last_child = pt;
	      prev = pt;
	    }
	}
      yv_del(&array, NULL, NULL);
    }
}

/*
** ydom_node_rm()
** Delete an XML node and all its children, and remove all links that point to it
** in the parent and siblings nodes.
*/
ydom_node_t *ydom_node_rm(ydom_node_t *node)
{
  ydom_node_t *res;

  if (!node)
    return (NULL);
  res = node->parent;
  ydom_node_rm_children(node);
  ydom_node_rm_attributes(node);
  if (node->prev)
    node->prev->next = node->next;
  if (node->next)
    node->next->prev = node->prev;
  if (node->parent && node->parent->first_child == node)
    node->parent->first_child = node->next;
  if (node->parent && node->parent->last_child == node)
    node->parent->last_child = node->prev;
  free0(node->name);
  free0(node->value);
  free0(node);
  return (res);
}

/*
** ydom_node_rm_children()
** Delete all children of an XML node, but node the node itself.
*/
void ydom_node_rm_children(ydom_node_t *node)
{
  ydom_node_t *pt;

  if (!node || !node->first_child)
    return ;
  while (node->first_child)
    {
      ydom_node_rm_children(node->first_child);
      ydom_node_rm_attributes(node->first_child);
      free0(node->first_child->name);
      free0(node->first_child->value);
      pt = node->first_child;
      node->first_child = pt->next;
      free0(pt);
    }
  node->last_child = NULL;
}

/*
** ydom_node_rm_attr()
** Remove the first attribute of an XML node that have a given name.
*/
void ydom_node_rm_attr(ydom_node_t *node, char *attr_name)
{
  ydom_node_t *attr;

  if (!node)
    return ;
  for (attr = node->attributes; attr; attr = attr->next)
    {
      if (!strcmp(attr_name, attr->name))
	{
	  if (!attr->prev)
	    node->attributes = attr->next;
	  else
	    attr->prev->next = attr->next;
	  if (attr->next)
	    attr->next->prev = attr->prev;
	  free0(attr->name);
	  free0(attr->value);
	  free0(attr);
	  return ;
	}
    }
}

/*
** ydom_node_rm_attributes()
** Remove all attributes of an XML node.
*/
void ydom_node_rm_attributes(ydom_node_t *node)
{
  ydom_node_t *attr, *to_rm;

  if (!node)
    return ;
  for (attr = node->attributes; attr; )
    {
      to_rm = attr;
      attr = attr->next;
      free0(to_rm->name);
      free0(to_rm->value);
      free0(to_rm);
    }
  node->attributes = NULL;
}


/* ********************************************************************* */

/*
** _ydom_open_hdlr()
** DOM handler for SAX parsing
*/
static void _ydom_open_hdlr(ysax_t *sax, char *tag_name, yvect_t attrs)
{
  ydom_t *dom;
  ydom_node_t *node;
  ysax_attr_t *pt;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  dom = (ydom_t*)YSAX_DATA(sax);
  if (!(node = malloc0(sizeof(ydom_node_t))))
    {
      YLOG_ADD(YLOG_ERR, "Memory alloc error");
      return ;
    }
  node->node_type = ELEMENT_NODE;
  node->name = tag_name;
  while ((pt = yv_pop(attrs)))
    {
      _ydom_add_attr_to_node(node, pt->name, pt->value);
      free0(pt);
    }
  yv_del(&attrs, NULL, NULL);

  if (dom->current_parsed_node->node_type == TEXT_NODE)
    {
      dom->current_parsed_node->complete = YTRUE;
      _ydom_add_next_to_node(dom->current_parsed_node, node);
    }
  else
    _ydom_add_child_to_node(dom->current_parsed_node, node);
  dom->current_parsed_node = node;
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** _ydom_close_hdlr()
** DOM handler for SAX parsing
*/
static void _ydom_close_hdlr(ysax_t *sax, char *tag_name)
{
  ydom_t *dom;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  dom = (ydom_t*)YSAX_DATA(sax);
  if (dom->current_parsed_node->node_type == TEXT_NODE)
    {
      dom->current_parsed_node->complete = YTRUE;
      dom->current_parsed_node = dom->current_parsed_node->parent;
    }
  if (strcmp(dom->current_parsed_node->name, tag_name))
    {
      dom->error = YEINVAL;
      ysax_stop(sax);
    }
  else
    {
      dom->current_parsed_node->complete = YTRUE;
      dom->current_parsed_node = dom->current_parsed_node->parent;
    }
  free0(tag_name);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** _ydom_inside_hdlr()
** DOM handler for SAX parsing
*/
static void _ydom_inside_hdlr(ysax_t *sax, char *str)
{
  ydom_t *dom;
  ydom_node_t *text;
  char *tmp;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  dom = (ydom_t*)YSAX_DATA(sax);
  if (dom->current_parsed_node->node_type == TEXT_NODE)
    {
      tmp = malloc0(strlen(dom->current_parsed_node->value) + strlen(str) + 1);
      strcpy(tmp, dom->current_parsed_node->value);
      strcat(tmp, str);
      free0(str);
      free0(dom->current_parsed_node->value);
      dom->current_parsed_node->value = tmp;
    }
  else
    {
      if (!(text = malloc0(sizeof(ydom_node_t))))
	{
	  YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
	  return ;
	}
      text->node_type = TEXT_NODE;
      text->value = str;
      _ydom_add_child_to_node(dom->current_parsed_node, text);
      dom->current_parsed_node = text;
    }
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** _ydom_comment_hdlr()
** DOM handler for SAX parsing
*/
static void _ydom_comment_hdlr(ysax_t *sax, char *s)
{
  ydom_t *dom;
  ydom_node_t *node;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  dom = (ydom_t*)YSAX_DATA(sax);
  if (!(node = malloc0(sizeof(ydom_node_t))))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return ;
    }
  node->node_type = COMMENT_NODE;
  node->value = s;
  node->complete = YTRUE;
  if (dom->current_parsed_node->node_type == TEXT_NODE)
    {
      dom->current_parsed_node->complete = YTRUE;
      _ydom_add_next_to_node(dom->current_parsed_node, node);
      dom->current_parsed_node = dom->current_parsed_node->parent;
    }
  else
    _ydom_add_child_to_node(dom->current_parsed_node, node);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** _ydom_process_instr_hdlr()
** DOM handler for SAX parsing
*/
static void _ydom_process_instr_hdlr(ysax_t *sax, char *target, char *content)
{
  ydom_t *dom;
  ydom_node_t *node;
  char *pt;
  char *pt2;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  dom = (ydom_t*)YSAX_DATA(sax);
  if (!strcmp(target, XML))
    {
      if (content && (pt = strstr(content, XML_VERSION)))
	{
	  if ((pt = strchr(pt, EQ)) &&
	      ((pt = strchr(pt, DQUOTE)) || (pt = strchr(pt, QUOTE))) &&
	      ((pt2 = strchr(pt + 1, DQUOTE)) || (pt2 = strchr(pt + 1, QUOTE))))
	    {
	      if ((dom->xml_version = malloc0(pt2 - pt)))
		{
		  memcpy(dom->xml_version, pt + 1, pt2 - pt - 1);
		  dom->xml_version[pt2 - pt - 1] = '\0';
		}
	    }
	}
      if (content && (pt = strstr(content, ENCODING)))
	{
	  if ((pt = strchr(pt, EQ)) &&
	      ((pt = strchr(pt, DQUOTE)) || (pt = strchr(pt, QUOTE))) &&
	      ((pt2 = strchr(pt + 1, DQUOTE)) || (pt2 = strchr(pt + 1, QUOTE))))
	    {
	      if ((dom->encoding = malloc0(pt2 - pt)))
		{
		  memcpy(dom->encoding, pt + 1, pt2 - pt - 1);
		  dom->encoding[pt2 - pt - 1] = '\0';
		}
	    }
	}
      if (content && (pt = strstr(content, STANDALONE)))
	{
	  if ((pt = strchr(pt, EQ)) &&
	      ((pt = strchr(pt, DQUOTE)) || (pt = strchr(pt, QUOTE))) &&
	      ((pt2 = strchr(pt + 1, DQUOTE)) || (pt2 = strchr(pt + 1, QUOTE))))
	    {
	      if ((dom->standalone = malloc0(pt2 - pt)))
		{
		  memcpy(dom->standalone, pt + 1, pt2 - pt - 1);
		  dom->standalone[pt2 - pt - 1] = '\0';
		}
	    }
	}
    }
  else
    {
      if (!(node = malloc0(sizeof(ydom_node_t))))
	{
	  YLOG_ADD(YLOG_DEBUG, "Unable to allocate memory");
	  return ;
	}
      node->node_type = PROCESSING_INSTRUCTION_NODE;
      node->name = target;
      node->value = content;
      node->complete = YTRUE;
      if (dom->current_parsed_node->node_type == TEXT_NODE)
	{
	  dom->current_parsed_node->complete = YTRUE;
	  _ydom_add_next_to_node(dom->current_parsed_node, node);
	  dom->current_parsed_node = dom->current_parsed_node->parent;
	}
      else
	_ydom_add_child_to_node(dom->current_parsed_node, node);
    }
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** _ydom_cdata_hdlr()
** DOM handler for SAX parsing
*/
static void _ydom_cdata_hdlr(ysax_t *sax, char *content)
{
  ydom_t *dom;
  ydom_node_t *node;

  YLOG_MOD("ydom", YLOG_DEBUG, "Entering");
  dom = (ydom_t*)YSAX_DATA(sax);
  if (!(node = malloc0(sizeof(ydom_node_t))))
    {
      YLOG_MOD("ydom", YLOG_DEBUG, "Unable to allocate memory");
      return ;
    }
  node->node_type = CDATA_SECTION_NODE;
  node->value = content;
  node->complete = YTRUE;
  if (dom->current_parsed_node->node_type == TEXT_NODE)
    {
      dom->current_parsed_node->complete = YTRUE;
      _ydom_add_next_to_node(dom->current_parsed_node, node);
      dom->current_parsed_node = dom->current_parsed_node->parent;
    }
  else
    _ydom_add_child_to_node(dom->current_parsed_node, node);
  YLOG_MOD("ydom", YLOG_DEBUG, "Exiting");
}

/*
** _ydom_add_child_to_node()
** Add a node child to an existing node
*/
static void _ydom_add_child_to_node(ydom_node_t *node, ydom_node_t *child)
{
  child->parent = node;
  child->document = node->document;
  if (node->first_child == NULL)
    {
      child->position = 1;
      node->first_child = node->last_child = child;
      child->prev = child->next = NULL;
    }
  else
    {
      child->position = node->last_child->position + 1;
      node->last_child->next = child;
      child->prev = node->last_child;
      child->next = NULL;
      node->last_child = child;
    }
}

/*
** _ydom_add_next_to_node()
** Add a next sibling node to an existing node
*/
static void _ydom_add_next_to_node(ydom_node_t *node, ydom_node_t *next)
{
  ydom_node_t *pt;

  for (pt = node; pt->next; pt = pt->next)
    ;
  next->position = pt->position + 1;
  pt->next = next;
  next->prev = pt;
  next->parent = node->parent;
  next->document = node->document;
  if (pt->parent)
    pt->parent->last_child = next;
}

/*
** _ydom_add_attr_to_node()
** Add an attribute to an existing node
*/
static ydom_node_t *_ydom_add_attr_to_node(ydom_node_t *node,
					   char *attr_name, char *attr_value)
{
  ydom_node_t *attribute;
  ydom_node_t *pt;

  if (!(attribute = malloc0(sizeof(ydom_node_t))))
    return (NULL);
  attribute->node_type = ATTRIBUTE_NODE;
  attribute->complete = YTRUE;
  attribute->name = attr_name;
  attribute->value = attr_value;
  if (node->attributes == NULL)
    node->attributes = attribute;
  else
    {
      for (pt = node->attributes; pt->next; pt = pt->next)
	;
      pt->next = attribute;
      attribute->prev = pt;
    }
  return (attribute);
}

/*
** _ydom_get_root_node_of_node()
** Return a pointer to the highest ancestor of a node, or a pointer to the node
** itself if the node doesn't have any ancestor
*/
ydom_node_t *_ydom_get_root_node_of_node(ydom_node_t *node)
{
  ydom_node_t *pt;

  for (pt = node; pt->parent; pt = pt->parent)
    ;
  return (pt);
}

/* ************************************************************************** */

/*
** _ydom_write_node()
** Write the content of a node on a stream.
*/
static void _ydom_write_node(ydom_node_t *node, int nb_tab, FILE *file)
{
  int i;

  if (node == NULL)
    return ;
  if (node->node_type == DOCUMENT_NODE)
    _ydom_write_node(node->first_child, 0, file);
  if (node->node_type == ELEMENT_NODE)
    {
      for (i = 0; i < nb_tab; i++)
	fprintf(file, "\t");
      fprintf(file, "<%s", node->name);
      _ydom_write_node_attr(node, file);
      if (!node->first_child)
	fprintf(file, "/>\n");
      else {
	fprintf(file, ">\n");
	_ydom_write_node(node->first_child, nb_tab + 1, file);
	for (i = 0; i < nb_tab; i++)
	  fprintf(file, "\t");
	fprintf(file, "</%s>\n", node->name);
      }
      _ydom_write_node(node->next, nb_tab, file);
    }
  else if (node->node_type == TEXT_NODE)
    {
      for (i = 0; i < nb_tab; i++)
	fprintf(file, "\t");
      fprintf(file, "%s\n", node->value);
      _ydom_write_node(node->next, nb_tab, file);
    }
  else if (node->node_type == COMMENT_NODE)
    {
      for (i = 0; i < nb_tab; i++)
	fprintf(file, "\t");
      fprintf(file, "<!-- %s -->\n", node->value);
      _ydom_write_node(node->next, nb_tab, file);
    }
  else if (node->node_type == PROCESSING_INSTRUCTION_NODE)
    {
      for (i = 0; i < nb_tab; i++)
	fprintf(file, "\t");
      fprintf(file, "<?%s %s?>\n", node->name, node->value);
      _ydom_write_node(node->next, nb_tab, file);
    }
  else if (node->node_type == CDATA_SECTION_NODE)
    {
      for (i = 0; i < nb_tab; i++)
	fprintf(file, "\t");
      fprintf(file, "<![CDATA[[%s]]>\n", node->value);
      _ydom_write_node(node->next, nb_tab, file);
    }
}

/*
** _ydom_write_node_attr()
** write the attributes of a node on a stream
*/
static void _ydom_write_node_attr(ydom_node_t *node, FILE *file)
{
  ydom_node_t *pt;

  if (node->attributes == NULL)
    return ;
  for (pt = node->attributes; pt; pt = pt->next)
    fprintf(file, " %s=\"%s\"", pt->name, pt->value);
}

/*
** _ydom_dump_node()
** append the content of a node at the end of a string
*/
static void _ydom_dump_node(ydom_node_t *node, ystr_t *s)
{
  if (!node)
    return ;
  if (node->node_type == DOCUMENT_NODE)
    {
      _ydom_dump_node(node->first_child, s);
      return ;
    }
  if (node->node_type == ELEMENT_NODE)
    {
      ys_addc(s, LT);
      ys_cat(s, node->name);
      _ydom_dump_node_attr(node, s);
      if (!node->first_child)
	ys_cat(s, "/>");
      else
	{
	  ys_addc(s, GT);
	  _ydom_dump_node(node->first_child, s);
	  ys_cat(s, "</");
	  ys_cat(s, node->name);
	  ys_addc(s, GT);
	}
    }
  else if (node->node_type == TEXT_NODE)
    ys_cat(s, node->value);
  else if (node->node_type == COMMENT_NODE)
    {
      ys_cat(s, "<!-- ");
      ys_cat(s, node->value);
      ys_cat(s, " -->");
    }
  else if (node->node_type == PROCESSING_INSTRUCTION_NODE)
    {
      ys_cat(s, "<?");
      if (node->name)
	ys_cat(s, node->name);
      ys_addc(s, SPACE);
      ys_cat(s, node->value);
      ys_cat(s, " ?>");
    }
  else if (node->node_type == CDATA_SECTION_NODE)
    {
      ys_cat(s, "<![CDATA[[");
      ys_cat(s, node->value);
      ys_cat(s, "]]>");
    }
  else
    return ;
  _ydom_dump_node(node->next, s);
}

/*
** _ydom_dump_node_attr()
** write the attributes of a node on a stream
*/
static void _ydom_dump_node_attr(ydom_node_t *node, ystr_t *s)
{
  ydom_node_t *pt;

  if (node->attributes == NULL)
    return ;
  for (pt = node->attributes; pt; pt = pt->next)
    {
      ys_addc(s, SPACE);
      ys_cat(s, pt->name);
      ys_cat(s, "=\"");
      ys_cat(s, pt->value);
      ys_addc(s, DQUOTE);
    }
}
