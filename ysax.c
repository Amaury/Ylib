#include "ylog.h"
#include "ysax.h"

/* Private prototypes */
static yerr_t _ysax_parse(ysax_t *sax);
static char _ysax_getc(ysax_t *sax);
static void _ysax_ungetc(ysax_t *sax, char c);
static yerr_t _ysax_do_open_tag(ysax_t *sax);
static ysax_attr_t *_ysax_add_attrs(yvect_t *attrs, char *attr_name, char *attr_value);
static yerr_t _ysax_parse_open_tag(ysax_t *sax);
static yerr_t _ysax_parse_close_tag(ysax_t *sax);
static yerr_t _ysax_parse_comment(ysax_t *sax);
static yerr_t _ysax_parse_process_instr(ysax_t *sax);
static yerr_t _ysax_parse_cdata(ysax_t *sax);

/*
** ysax_new()
** Create a SAX XML parser.
*/
ysax_t *ysax_new(void *parse_data)
{
  ysax_t *sax;

  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!(sax = malloc0(sizeof(ysax_t))))
    {
      YLOG_ADD(YLOG_ERR, "Memory allocation error");
      return (NULL);
    }
  sax->must_close_stream = YFALSE;
  sax->parse_data = parse_data;
  sax->error = YENOERR;
  sax->line_nbr = 0;
  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
  return (sax);
}

/*
** ysax_read_handler()
** Launch the parsing of some XML data, using handlers
** to read data.
*/
yerr_t ysax_read_handler(ysax_t *sax, char (*getc_hdlr)(void*),
			 void (*ungetc_hdlr)(char, void*), void *xml_data)
{
  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!getc_hdlr || !ungetc_hdlr)
    {
      YLOG_ADD(YLOG_ERR, "Invalid handler");
      return (YEUNDEF);
    }
  sax->getc_hdlr = getc_hdlr;
  sax->ungetc_hdlr = ungetc_hdlr;
  sax->xml_data = xml_data;
  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
  return (_ysax_parse(sax));
}

/*
** ysax_read_file()
** Launch the parsing of an XML file.
*/
yerr_t ysax_read_file(ysax_t *sax, const char *filename)
{
  if (!filename || !(sax->file = fopen(filename, "r")))
    {
      YLOG_ADD(YLOG_ERR, "Bad file parameter");
      return (YEUNDEF);
    }
  sax->must_close_stream = YTRUE;
  return (_ysax_parse(sax));
}

/*
** ysax_read_stream()
** Launch the parsing of an XML stream.
*/
yerr_t ysax_read_stream(ysax_t *sax, FILE *stream)
{
  if (!stream)
    {
      YLOG_ADD(YLOG_ERR, "Bad stream parameter");
      return (YEUNDEF);
    }
  sax->file = stream;
  sax->must_close_stream = YFALSE;
  return (_ysax_parse(sax));
}

/*
** ysax_read_memory()
** Launch the parsing of an XML buffer.
*/
yerr_t ysax_read_memory(ysax_t *sax, char *mem)
{
  if (!mem)
    {
      YLOG_ADD(YLOG_ERR, "Bad mem parameter");
      return (YEUNDEF);
    }
  sax->mem = mem;
  return (_ysax_parse(sax));
}

/*
** ysax_del()
** Delete a SAX XML parser.
*/
void ysax_del(ysax_t *sax)
{
  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "%s: %s", "ysax_del", "Bad pointer");
      return ;
    }
  if (sax->must_close_stream && fclose(sax->file))
    YLOG_ADD(YLOG_ERR, "Unable to close file");
  free0(sax);
  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
}

/*
** ysax_set_tag_hdlr()
** Function used to set the handlers for open and clse tags.
*/
void ysax_set_tag_hdlr(ysax_t *sax,
		       void (*open_hdlr)(ysax_t*, char*, yvect_t),
		       void (*close_hdlr)(ysax_t*, char*))
{
  if (sax)
    {
      sax->open_tag_hdlr = open_hdlr;
      sax->close_tag_hdlr = close_hdlr;
    }
}

/*
** ysax_set_inside_text_hdlr()
** Function used to set the handler for inside text.
*/
void ysax_set_inside_text_hdlr(ysax_t *sax,
			       void (*hdlr)(ysax_t*, char*))
{
  if (sax)
    sax->inside_text_hdlr = hdlr;
}

/*
** ysax_set_comment_hdlr()
** Function used to set the handler for XML comments.
*/
void ysax_set_comment_hdlr(ysax_t *sax,
			   void (*hdlr)(ysax_t*, char*))
{
  if (sax)
    sax->comment_hdlr = hdlr;
}

/*
** ysax_set_process_instr_hdlr()
** Functio used to set the handler for processing instructions.
*/
void ysax_set_process_instr_hdlr(ysax_t *sax,
				 void (*hdlr)(ysax_t*, char*, char*))
{
  if (sax)
    sax->process_instr_hdlr = hdlr;
}

/*
** ysax_set_cdata_hdlr()
** Function used to set the handler for CDATA sections.
*/
void ysax_set_cdata_hdlr(ysax_t *sax,
			 void (*hdlr)(ysax_t*, char*))
{
  if (sax)
    sax->cdata_hdlr = hdlr;
}

/*
** ysax_stop()
** Stop the Sax parser. Use this function if you detect an
** error in a handler. The parser will return an YEL2HLT
** error (Level 2 halted).
*/
void ysax_stop(ysax_t *sax)
{
  if (sax)
    sax->error = YEL2HLT;
}

/*
** ysax_free_attr()
** Function called when the array of attributes is freed.
*/
void ysax_free_attr(void *e, void *data)
{
  ysax_attr_t *attr = e;

  if (!e)
    return ;
  free0(attr->name);
  free0(attr->value);
  free0(attr);
}

/*
** _ysax_parse() -- PRIVATE FUNCTION
** Launch the parsing of an XML file.
*/
yerr_t _ysax_parse(ysax_t *sax)
{
  char c;
  char last = '\0';
  ybool_t inside_char = YFALSE;
  ystr_t inside_str;

  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "Bad pointer");
      return (YEUNDEF);
    }
  if (!(inside_str = ys_new("")))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return ((sax->error = YENOMEM));
    }
  sax->line_nbr = 1;
  while ((c = _ysax_getc(sax)) != EOF && sax->error == YENOERR)
    {
      if (IS_SPACE(c))
	{
	  if (c == CR)
	    sax->line_nbr++;
	  last = c;
	}
      else if (c == LT)
	{
	  if (ys_len(inside_str))
	    {
	      if (sax->inside_text_hdlr)
		sax->inside_text_hdlr(sax, ys_string(inside_str));
	      ys_trunc(inside_str);
	    }
	  if ((sax->error = _ysax_do_open_tag(sax)) != YENOERR)
	    {
	      ys_del(&inside_str);
	      YLOG_ADD(YLOG_ERR, "Bad end");
	      return (sax->error);
	    }
	  inside_char = YFALSE;
	  last = '\0';
	}
      else if (c == GT)
	{
	  ys_del(&inside_str);
	  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
	  return ((sax->error = YENOERR));
	}
      else
	{
	  if (inside_char && IS_SPACE(last) && sax->inside_text_hdlr)
	    ys_addc(&inside_str, last);
	  inside_char = YTRUE;
	  if (sax->inside_text_hdlr)
	    ys_addc(&inside_str, c);
	  last = c;
	}
    }
  ys_del(&inside_str);
  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
  return (sax->error);
}

/*
** _ysax_getc() -- PRIVATE FUNCTION
** Function called by SAX parser to get the next character of the file/stream/string.
*/
static char _ysax_getc(ysax_t *sax)
{
  char c;

  if (!sax)
    return (EOF);
  if (sax->getc_hdlr)
    c = sax->getc_hdlr(sax->xml_data);
  else if (sax->mem)
    {
      if (!(c = *(sax->mem)))
	return (EOF);
      sax->mem++;
    }
  else
    c = (char)fgetc(sax->file);
  return (c);
}

/*
** _ysax_ungetc() -- PRIVATE FUNCTION
** Function called to reput a character in the stream/string
*/
static void _ysax_ungetc(ysax_t *sax, char c)
{
  if (!sax)
    return ;
  if (sax->ungetc_hdlr)
    sax->ungetc_hdlr(c, sax->xml_data);
  else if (sax->mem)
    sax->mem--;
  else
    ungetc(c, sax->file);
}

/*
** _ysax_do_open_tag() -- PRIVATE FUNCTION
** function called when a tag is found
*/
static yerr_t _ysax_do_open_tag(ysax_t *sax)
{
  char c;

  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "Bad pointer");
      return (YEUNDEF);
    }
  if ((c = _ysax_getc(sax)) == EOF)
    return (YENOERR);
  if (c == INTERROG)
    return (_ysax_parse_process_instr(sax));
  else if (c == EXCLAM)
    {
      if ((c = _ysax_getc(sax)) == EOF)
	return (YENOERR);
      if (c == MINUS)
	return (_ysax_parse_comment(sax));
      else if (c == LBRACKET)
	return (_ysax_parse_cdata(sax));
    }
  else if (c == SLASH)
    return (_ysax_parse_close_tag(sax));
  else
    {
      _ysax_ungetc(sax, c);
      return (_ysax_parse_open_tag(sax));
    }
  YLOG_ADD(YLOG_ERR, "Syntax error");
  return (YESYNTAX);
}

/*
** _ysax_add_attrs() -- PRIVATE FUNCTION
** This function take a vector of attributes, and extends it with a new couple
** of attribute name/value
*/
static ysax_attr_t *_ysax_add_attrs(yvect_t *attrs, char *attr_name, char *attr_value)
{
  ysax_attr_t *attr;

  if (!attrs || !(attr = malloc0(sizeof(ysax_attr_t))))
    return (NULL);
  if (!*attrs && !(*attrs = yv_new()))
    {
      free0(attr);
      return (NULL);
    }
  attr->name = attr_name;
  attr->value = attr_value;
  yv_add(attrs, attr);
  return (attr);
}

/*
** _ysax_parse_open_tag() -- PRIVATE FUNCTION
** This function parse an open tag (between '<' and '>') and call the
** handler
*/
static yerr_t _ysax_parse_open_tag(ysax_t *sax)
{
  ystr_t name_buf, attr_name_buf, attr_value_buf;
  yvect_t attrs;
  char c;
  ybool_t get_name = YTRUE, get_attr_name = YFALSE, get_attr_value = YFALSE;

  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "Bad pointer");
      return (YEUNDEF);
    }
  if (!(name_buf = ys_new("")) || !(attr_name_buf = ys_new("")) ||
      !(attr_value_buf = ys_new("")) || !(attrs = yv_new()))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (YENOMEM);
    }
  while ((c = _ysax_getc(sax)) != EOF)
    {
      if (!IS_SPACE(c))
	{
	  if (c == LT)
	    {
	      ys_del(&name_buf);
	      ys_del(&attr_name_buf);
	      ys_del(&attr_value_buf);
	      yv_del(&attrs, ysax_free_attr, NULL);
	      YLOG_ADD(YLOG_ERR, "Syntax error");
	      return (YESYNTAX);
	    }
	  else if (c == SLASH)
	    {
	      if ((c = _ysax_getc(sax)) == EOF || c != GT)
		{
		  ys_del(&name_buf);
		  ys_del(&attr_name_buf);
		  ys_del(&attr_value_buf);
		  yv_del(&attrs, ysax_free_attr, NULL);
		  YLOG_ADD(YLOG_ERR, "Syntax error");
		  return (YESYNTAX);
		}
	      if (sax->open_tag_hdlr)
		sax->open_tag_hdlr(sax, ys_string(name_buf), attrs);
	      if (sax->close_tag_hdlr)
		sax->close_tag_hdlr(sax, ys_string(name_buf));
	      ys_del(&name_buf);
	      ys_del(&attr_name_buf);
	      ys_del(&attr_value_buf);
	      YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
	      return (YENOERR);
	    }
	  else if (c == GT)
	    {
	      if (!ys_len(name_buf))
		{
		  ys_del(&name_buf);
		  ys_del(&attr_name_buf);
		  ys_del(&attr_value_buf);
		  yv_del(&attrs, ysax_free_attr, NULL);
		  YLOG_ADD(YLOG_ERR, "Syntax error");
		  return (YESYNTAX);
		}
	      if (sax->open_tag_hdlr)
		sax->open_tag_hdlr(sax, ys_string(name_buf), attrs);
	      ys_del(&name_buf);
	      ys_del(&attr_name_buf);
	      ys_del(&attr_value_buf);
	      YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
	      return (YENOERR);
	    }
	  else if (c == EQ)
	    {
	      if (!get_name && get_attr_name && !get_attr_value)
		{
		  get_attr_name = YFALSE;
		  get_attr_value = YTRUE;
		  while ((c = _ysax_getc(sax)) != EOF)
		    if (!IS_SPACE(c))
		      break;
		  if (c == QUOTE)
		    while ((c = _ysax_getc(sax)) != EOF && c != QUOTE)
		      ys_addc(&attr_value_buf, c);
		  else if (c == DQUOTE)
		    while ((c = _ysax_getc(sax)) != EOF && c != DQUOTE)
		      ys_addc(&attr_value_buf, c);
		  else
		    {
		      ys_del(&name_buf);
		      ys_del(&attr_name_buf);
		      ys_del(&attr_value_buf);
		      yv_del(&attrs, ysax_free_attr, NULL);
		      YLOG_ADD(YLOG_ERR, "Syntax error");
		      return (YESYNTAX);
		    }
		  if (ys_len(attr_name_buf))
		    _ysax_add_attrs(&attrs, ys_string(attr_name_buf),
					ys_string(attr_value_buf));
		  ys_trunc(attr_name_buf);
		  ys_trunc(attr_value_buf);
		}
	      else
		{
		  ys_del(&name_buf);
		  ys_del(&attr_name_buf);
		  ys_del(&attr_value_buf);
		  yv_del(&attrs, ysax_free_attr, NULL);
		  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
		  return (YENOERR);
		}
	    }
	  else
	    {
	      if (get_name)
		ys_addc(&name_buf, c);
	      else if (get_attr_name)
		ys_addc(&attr_name_buf, c);
	    }
	}
      else
	{
	  if (c == CR)
	    sax->line_nbr++;
	  if (ys_len(name_buf) && get_name && !get_attr_name)
	    {
	      get_name = YFALSE;
	      get_attr_name = YTRUE;
	      ys_trunc(attr_name_buf);
	      ys_trunc(attr_value_buf);
	    }
	  else if (!get_attr_name && get_attr_value)
	    {
	      get_attr_name = YTRUE;
	      get_attr_value = YFALSE;
	      ys_trunc(attr_name_buf);
	      ys_trunc(attr_value_buf);
	    }
	}
    }
  ys_del(&name_buf);
  ys_del(&attr_name_buf);
  ys_del(&attr_value_buf);
  yv_del(&attrs, ysax_free_attr, NULL);
  YLOG_ADD(YLOG_ERR, "Syntax error");
  return (YESYNTAX);
}

/*
** _ysax_parse_close_tag() -- PRIVATE FUNCTION
** This function parse a close tag (between '</' and '>') and call
** the handler
*/
static yerr_t _ysax_parse_close_tag(ysax_t *sax)
{
  ystr_t name_buff;
  ybool_t find_space = YFALSE, find_char = YFALSE;
  char c;

  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "Bad pointer");
      return (YEUNDEF);
    }
  if (!(name_buff = ys_new("")))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (YENOMEM);
    }
  while ((c = _ysax_getc(sax)) != EOF)
    {
      if (IS_SPACE(c))
	{
	  if (c == CR)
	    sax->line_nbr++;
	  find_space = YTRUE;
	}
      else if (c == LT)
	{
	  ys_del(&name_buff);
	  return (YESYNTAX);
	}
      else if (c == GT)
	{
	  if (sax->close_tag_hdlr)
	    sax->close_tag_hdlr(sax, ys_string(name_buff));
	  ys_del(&name_buff);
	  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
	  return (YENOERR);
	}
      else
	{
	  if (find_char && find_space)
	    {
	      ys_del(&name_buff);
	      YLOG_ADD(YLOG_ERR, "Syntax error");
	      return (YESYNTAX);
	    }
	  find_char = YTRUE;
	  ys_addc(&name_buff, c);
	}
    }
  ys_del(&name_buff);
  YLOG_ADD(YLOG_ERR, "Syntax error");
  return (YESYNTAX);
}

/*
** _ysax_parse_comment() -- PRIVATE FUNCTION
** This function parse a comment (between '<!--' and '-->') and
** call the handler
*/
static yerr_t _ysax_parse_comment(ysax_t *sax)
{
  char c, next = EOF;
  ystr_t s;

  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "Bad pointer");
      return (YEUNDEF);
    }
  if (!(s = ys_new("")))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (YENOMEM);
    }
  if ((c = _ysax_getc(sax)) == EOF || c != MINUS)
    {
      YLOG_ADD(YLOG_ERR, "Syntax error");
      return (YESYNTAX);
    }
  while ((c = _ysax_getc(sax)) != EOF)
    {
      if (c != MINUS)
	ys_addc(&s, c);
      else if (c == MINUS && (next = _ysax_getc(sax)) != EOF && next == MINUS)
	break;
      else
	{
	  ys_addc(&s, c);
	  ys_addc(&s, next);
	}
    }
  if ((c = _ysax_getc(sax)) == EOF || c != GT)
    {
      ys_del(&s);
      YLOG_ADD(YLOG_ERR, "Syntax error");
      return (YESYNTAX);
    }
  if (sax->comment_hdlr)
    sax->comment_hdlr(sax, ys_string(s));
  ys_del(&s);
  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
  return (YENOERR);
}

/*
** _ysax_parse_process_instr() -- PRIVATE FUNCTION
** This function parse a processing instruction (between '<?' and '?>')
** and call the handler
*/
static yerr_t _ysax_parse_process_instr(ysax_t *sax)
{
  char c;
  ystr_t target, content;

  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "Bad pointer");
      return (YEUNDEF);
    }
  if (!(target = ys_new("")))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (YENOMEM);
    }
  while ((c = _ysax_getc(sax)) != EOF && c != INTERROG && !IS_SPACE(c))
    ys_addc(&target, c);
  if (c == INTERROG && (c = _ysax_getc(sax)) != EOF && c == GT)
    {
      if (sax->process_instr_hdlr)
	sax->process_instr_hdlr(sax, ys_string(target), NULL);
      ys_del(&target);
      YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
      return (YENOERR);
    }
  if (!(content = ys_new("")))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (YENOMEM);
    }
  while ((c = _ysax_getc(sax)) != EOF && c != INTERROG)
    ys_addc(&content, c);
  if (c == EOF)
    {
      ys_del(&target);
      ys_del(&content);
      YLOG_ADD(YLOG_ERR, "Syntax error");
      return (YESYNTAX);
    }
  if ((c = _ysax_getc(sax)) == EOF || c != GT)
    {
      ys_del(&target);
      ys_del(&content);
      YLOG_ADD(YLOG_ERR, "Syntax error");
      return (YESYNTAX);
    }
  if (sax->process_instr_hdlr)
    sax->process_instr_hdlr(sax, ys_string(target), ys_string(content));
  ys_del(&target);
  ys_del(&content);
  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
  return (YENOERR);
}

/*
** _ysax_parse_cdata() -- PRIVATE FUNCTION
** This function parse a CDATA tag (between '<[CDATA[' and ']]>')
** and call the handler
*/
static yerr_t _ysax_parse_cdata(ysax_t *sax)
{
  char c;
  char *pt;
  unsigned int i;
  ystr_t content;
	
  YLOG_MOD("ysax", YLOG_DEBUG, "Entering");
  if (!sax)
    {
      YLOG_ADD(YLOG_WARN, "Bad pointer");
      return (YEUNDEF);
    }
  pt = CDATA;
  for (i = 0; i < strlen(CDATA); i++)
    {
      if ((c = _ysax_getc(sax)) == EOF || c != *(pt + i))
	{
	  YLOG_ADD(YLOG_ERR, "Syntax error");
	  return (YESYNTAX);
	}
    }
  if ((c = _ysax_getc(sax)) == EOF || c != LBRACKET)
    {
      YLOG_ADD(YLOG_ERR, "Syntax error");
      return (YESYNTAX);
    }
  if (!(content = ys_new("")))
    {
      YLOG_ADD(YLOG_ERR, "Unable to allocate memory");
      return (YENOMEM);
    }
  while ((c = _ysax_getc(sax)) != EOF && c != RBRACKET)
    ys_addc(&content, c);
  if ((c = _ysax_getc(sax)) == EOF || c != RBRACKET ||
      (c = _ysax_getc(sax)) == EOF || c != GT)
    {
      ys_del(&content);
      YLOG_ADD(YLOG_ERR, "Syntax error");
      return (YESYNTAX);
    }
  if (sax->cdata_hdlr)
    sax->cdata_hdlr(sax, ys_string(content));
  ys_del(&content);
  YLOG_MOD("ysax", YLOG_DEBUG, "Exiting");
  return (YENOERR);
}
