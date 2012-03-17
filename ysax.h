/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	ysax.h
 * @abstract	All declarations for XML SAX parser.
 * @discussion	The XML SAX (Simple API for XML) parser is the simpliest way
 *		to read XML files, streams or character strings. Functions are
 *		affected to events (when the parser find an open or a close tag,
 *		a comment, ...).
 *		<p /><b>How to use the SAX API ?</b><br />
 *		First of all, you must create a SAX parser:
 *		<pre>ysax_t *sax = ysax_new(data);</pre>
 *		Where <i>data</i> is a pointer to data that handlers should use to
 *		do there jobs.<p />
 *		<pre>ysax_t *sax = ysax_new(filename, stream, mem, data);</pre>
 *		Where <i>filename</i> is the name of the XML file to parse, <i>stream</i>
 *		is a pointer to a stream used if <i>filename</i> is set to NULL, and <i>mem</i>
 *		is a pointer to a character string (which contains XML data) used if 
 *		<i>filename</i> and <i>stream</i> are set to NULL. <i>data</i> is a pointer
 *		to data that handlers should use to do there jobs.<p />
 *		After, you set the handlers, with ysax_set_tag_hdlr(),
 *		ysax_set_inside_text_hdlr(), ...<p />
 *		<ul>
 *		<li>The handler for open tags must have this prototype:
 *		<pre>void func(ysax_t *sax, char *name, yvect_t attrs);</pre>
 *		<ul><li>A pointer to the parser</li>
 *		<li>A string which contains the tag name</li>
 *		<li>A yvector of ysax_attr_t which contains all name/value attributes of the tag,
 *		with the last element of the array set to zero</li>
 *		</ul><p /></li>
 *		<li>The handler for close tag must have this prototype:
 *		<pre>void func(ysax_t *sax, char *name);</pre>
 *		<ul><li>A pointer to the parser</li>
 *		<li>A string which contains the tag name</li>
 *		</ul><p /></li>
 *		<li>The handler for inside text is call when some text is finded between an open 
 *		and a close tag; it must have this prototype:
 *		<pre>void func(ysax_t *sax, char *text);</pre>
 *		<ul><li>A pointer to the parser</li>
 *		<li>A string finded between an open and a close tag</li>
 *		</ul><p /></li>
 *		<li>The comment handler is call when an XML comment is finded (text between 
 *		"&lt;!--" and "--&gt;"); it must have this prototype:
 *		<pre>void func(ysax_t *sax, char *comment);</pre>
 *		<ul><li>A pointer to the parser</li>
 *		<li>A string which contains the finded comment</li>
 *		</ul><p /></li>
 *		<li>The processing instruction handler is call when a processing instruction is 
 *		finded (text between "&lt;?" and "?&gt;"); it must have this prototype:
 *		<pre>void func(ysax_t *sax, char *target, char *content);</pre>
 *		<ul><li>A pointer to the parser</li>
 *		<li>A string which contains the target of the processing instruction</li>
 *		<li>A string which contains the processing instruction itself</li>
 *		</ul><p /></li>
 *		<li>The CDATA handler is call when a CDATA instruction is finded (text between
 *		"&lt;![CDATA[[" and "]]&gt;"); it must have this prototype:
 *		<pre>void  func(ysax_t *sax, char *content);</pre>
 *		<ul><li>A pointer to the parser</li>
 *		<li>A string which contains the CDATA content</li>
 *		</ul><p /></li>
 *		</ul>
 *		To launch the parser execution, you just have to call:
 *		<pre>ysax_parse(sax);</pre>
 *		This function return YENOERR if all is OK. It read the XML file, and call
 *		the handlers.<p />
 *		If an error is detected in a handler, you could stop the parsing be calling
 *		this function (and then the ysax_parse() function will return an YEL2HLT
 *		error):
 *		<pre>ysax_stop(sax);</pre>
 *		<p />When you have finish, you could free the memory allocated by the parser:
 *		<pre>ysax_del(sax);</pre>
 * @version	1.0.0 Sep 14 2002
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YSAX_H__
#define __YSAX_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include <stdio.h>
#include "ydefs.h"
#include "ystr.h"
#include "yvect.h"
#include "yerror.h"

/*! @define YSAX_DATA Rpovide direct access to SAX parsing data. */
#define YSAX_DATA(x)	(((ysax_t*)x)->parse_data)

/* XML strings definitions */
#define	XML			"xml"
#define	XML_VERSION		"version"
#define	ENCODING		"encoding"
#define	STANDALONE		"standalone"
#define	CDATA			"CDATA"
#define	XMLNS			"xmlns"

/*!
 * @struct	ysax_attr_s
 *		Structure used to describe an XML attribute.
 * @field	name	Attribute's name.
 * @field	value	Attibute's value.
 */
struct ysax_attr_s
{
  char *name;
  char *value;
};

/*! @typedef ysax_attr_t See struct ysax_attr_s. */
typedef struct ysax_attr_s ysax_attr_t;

/*!
 * @struct	ysax_s
 *		Object for SAX XML parser.
 * @field	file			Stream of the XML file
 * @field	file_mode		Set to TRUE if the file must be close.
 * @field	mem			Pointer to character string to parse.
 * @field	getc_hdlr		Function pointer to get data.
 * @field	ungetc_hdlr		Function pointer to unget data.
 * @field	xml_data		Pointer to data used by getc/ungetc handlers.
 * @field	parse_data		Pointer to some data (for handlers using).
 * @field	open_tag_hdlr		Function pointer to call when an open tag is finded.
 * @field	inside_text_hdlr	Function pointer for text inside tags.
 * @field	close_tag_hdlr		Function pointer for close tags.
 * @field	comment_hdlr		Function pointer for XML comments.
 * @field	process_instr_hdlr	Function pointer for processing instructions.
 * @field	cdata_hdlr		Function pointer for CDATA instructions.
 * @field	error			Current status of SAX parsing.
 * @field	line_nbr		Current line of the file parsing.
 */
struct ysax_s
{
  FILE *file;
  ybool_t must_close_stream;
  char *mem;
  char (*getc_hdlr)(void*);
  void (*ungetc_hdlr)(char, void*);
  void *xml_data;
  void *parse_data;
  void (*open_tag_hdlr)(struct ysax_s*, char*, yvect_t);
  void (*inside_text_hdlr)(struct ysax_s*, char*);
  void (*close_tag_hdlr)(struct ysax_s*, char*);
  void (*comment_hdlr)(struct ysax_s*, char*);
  void (*process_instr_hdlr)(struct ysax_s*, char*, char*);
  void (*cdata_hdlr)(struct ysax_s*, char*);
  yerr_t error;
  int line_nbr;
};

/*! @typedef ysax_t See struct ysax_s. */
typedef struct ysax_s ysax_t;

/*!
 * @function	ysax_new
 *		Create a SAX XML parser.
 * @param	parse_data	Pointer to data that can be used by handlers.
 * @return	A pointer to the created parser, or NULL if an error occurs.
 */
ysax_t *ysax_new(void *parse_data);

/*!
 * @function	ysax_read_handler
 *		Launch the parsing of some XML data, using handlers
 *		to read data.
 * @param	sax		A pointer to the SAX object.
 * @param	getc_hdlr	Handler called to get one character.
 * @param	ungetc_hdlr	Handler called to unget one character.
 * @param	xml_data	Pointer to data used by handlers.
 * @return	An error code that show the return status of parse action.
 */
yerr_t ysax_read_handler(ysax_t *sax, char (*getc_hdlr)(void*),
			 void (*ungetc_hdlr)(char,void*), void *xml_data);

/*!
 * @function	ysax_read_file
 *		Launch the parsing of an XML file.
 * @param	sax		A pointer to the SAX object.
 * @param	filename	Path to the file to parse.
 * @param	An error code that shows the return status of parse action.
 */
yerr_t ysax_read_file(ysax_t *sax, const char *filename);

/*!
 * @function	ysax_read_stream
 *		Launch the parsing of an XML stream.
 * @param	sax	A pointer to the SAX object.
 * @param	stream	A pointer to an opened stream.
 * @return	An error code that shows the return status of parse action.
 */
yerr_t ysax_read_stream(ysax_t *sax, FILE *stream);

/*!
 * @function	ysax_read_memory
 *		Launch the parsing of an XML buffer.
 * @param	sax	A pointer to the SAX object.
 * @param	mem	A pointer to a character string that contains the
 *			XML ti parse.
 * @return	An error code that shows the return status of parse action.
 */
yerr_t ysax_read_memory(ysax_t *sax, char *mem);

/*!
 * @function	ysax_del
 *		Delete a previously created Sax XML parser.
 * @param	sax	A pointer to the existing Sax parser.
 */
void ysax_del(ysax_t *sax);

/*!
 * @function	ysax_set_tag_hdlr
 *		Set the open tag and close tag handlers of the Sax XML parser.
 * @param	sax		A pointer to the Sax parser.
 * @param	open_hdlr	Function pointer to the handler call when an
 *				open tag is finded.
 * @param	close_hdlr	Function pointer to the handler call when a
 *				close tag is finded.
 */
void ysax_set_tag_hdlr(ysax_t *sax,
		       void (*open_hdlr)(ysax_t*, char*, yvect_t),
		       void (*close_hdlr)(ysax_t*, char*));

/*!
 * @function	ysax_set_inside_text_hdlr
 *		Set the inside text handler of the Sax XML parser.
 * @param	sax	A pointer to the Sax parser.
 * @param	hdlr	Function pointer to the handler call when a character
 *			is finded between an open and a close tag.
 */
void ysax_set_inside_text_hdlr(ysax_t *sax,
			       void (*hdlr)(ysax_t*, char*));

/*!
 * @function	ysax_set_comment_hdlr
 *		Set the comment handler of a Sax XML parser.
 * @param	sax	A pointer to the Sax parser.
 * @param	hdlr	Function pointer to the handler call when a XML
 *			comment is finded.
 */
void ysax_set_comment_hdlr(ysax_t *sax,
			   void (*hdlr)(ysax_t*, char*));

/*!
 * @function	ysax_set_process_instr_hdlr
 *		Set the processing instruction of a Sax XML parser.
 * @param	sax	A pointer to the Sax parser.
 * @param	hdlr	Function pointer to the handler call when a
 *			processing instruciton is finded.
 */
void ysax_set_process_instr_hdlr(ysax_t *sax,
				 void (*hdlr)(ysax_t*, char*, char*));

/*!
 * @function	ysax_set_cdata_hdlr
 *		Set the CDATA handler of a Sax XML parser.
 * @param	sax	A pointer to the Sax parser.
 * @param	hdlr	Function pointer to the handler call when a CDATA
 *			instruction is finded.
 */
void ysax_set_cdata_hdlr(ysax_t *sax,
			 void (*hdlr)(ysax_t*, char*));

/*!
 * @function	ysax_stop
 *		Stop the Sax parser. Use this function if you detect an
 *		error in a handler. The parser will return an YEL2HLT
 *		error (Level 2 halted).
 * @param	sax	A pointer to the Sax parser.
 */
void ysax_stop(ysax_t *sax);

/*!
 * @function	ysax_free_attr
 *		When a yvector of XML attributes is destroyed, this function
 *		can be passed as parameter to yv_del() to free all memory
 *		allocated for each element.
 * @param	e	Pointer to the vector element (= ysax_attr_t*).
 * @param	data	Non used pointer, here for prototype compatibility.
 */
void ysax_free_attr(void *e, void *data);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YSAX_H__ */
