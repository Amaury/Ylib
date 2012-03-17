/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	ydom.h
 * @abstract	Include file for XML DOM parser.
 * @discussion	<b>How to use the XML DOM API ?</b><p />
 *		First of all, you must create an arkml object:
 *		<pre>ydom_t *dom = ydom_new();</pre>
 *		If you want to read the content of an existing XML file, you
 *		could use the <i>ydom_read_file()</i>,
 *		<i>ydom_read_stream()</i> or
 *		<i>ydom_read_memory()</i> functions.<br />
 *		To write the XML tree into a stream (a file, or standard
 *		output), use the <i>ydom_write()</i> function.<br />
 *		When you have finish with an XML object, use the
 *		<i>ydom_del()</i> function to free all allocated memory.<p />
 *		<b>Simple example: use XML as a linked list</b><p />
 *		<ul>
 *		<li><i>Create an XML object</i>
 *		<pre>ydom_t *dom = ydom_new();</pre></li>
 *		<li><i>Create a top-level element</i>
 *		<pre>ydom_node_t *data = ydom_add_elem(dom, "LIST-DATA");</pre>
 *		</li>
 *		<li><i>Add a list element</i>
 *		<pre>ydom_node_t *elem = ydom_node_add_elem(data, "LIST-ELEM");</pre></li>
 *		<li><i>Add an attribute to the element</i>
 *		<pre>ydom_node_add_attr(elem, "prop1", "value1");</pre></li>
 *		<li><i>Create another list element</i>
 *		<pre>elem = ydom_node_add_elem(data, "LIST-ELEM");</pre></li>
 *		<li><i>Add an attribute to it</i>
 *		<pre>ydom_node_add_attr(elem, "prop1", "value2");</pre></li>
 *		<li><i>Add a second attribute to it</i>
 *		<pre>ydom_node_add_attr(elem, "prop2", "value3");</pre></li>
 *		<li><i>Write the XML content to standard output</i>
 *		<pre>ydom_write(dom, stdout);</pre></li>
 *		<li><i>Delete the XML object</i>
 *		<pre>ydom_del(arkml);</pre></li>
 *		</ul>
 *		<i>You will have:</i><p />
 *		&lt;?xml?&gt;<br />
 *		&lt;LIST-DATA&gt;<br />
 *		&nbsp;&nbsp;&nbsp;&nbsp;&lt;LIST-ELEM prop1="value1"/&gt;<br />
 *		&nbsp;&nbsp;&nbsp;&nbsp;&lt;LIST-ELEM prop1="value2"
 *		prop2="value3"/&gt;<br />
 *		&lt;LIST-DATA&gt;<p />
 *		<b>Get data with XPath expressions (see below for
 *		more)</b><p />
 *		<ul>
 *		<li><i>To get all "LIST-ELEM" elements from the previously
 *		created list</i>
 *		<pre>yvect_t array = ydom_xpath(dom, "/LIST-DATA/LIST-ELEM");</pre>
 *		You will have a vector of XML nodes.<p /></li>
 *		<li><i>To get all "prop2" attributes of the "LIST-ELEM"
 *		elements</i>
 *		<pre>array = ydom_xpath(arkml, "/LIST-DATA/LIST-ELEM/@prop2");</pre>
 *		You will have the same type of vector. Remember that attributes
 *		are, in fact, the same kind of node than XML elements.<p /></li>
 *		<li><i>To get all "LIST-ELEM" elements that have a "SUB-ELEM"
 *		child element (no one in this example)</i>
 *		<pre>array = ydom_xpath(arkml, "/LIST-DATA/LIST-ELEM[SUB-ELEM]");</pre></li>
 *		<li><i>To get all "LIST-ELEM" elements that have a "prop2"
 *		attribute</i>
 *		<pre>array = ydom_xpath(arkml, "/LIST-DATA/LIST-ELEM[@prop2]");</pre></li>
 *		<li><i>To get all "LIST-ELEM" elements that have a "prop1"
 *		attribute with a "value1" value</i>
 *		<pre>array = ydom_xpath(arkml, "LIST-DATA/LIST-ELEM[@prop1='value1']");</pre></li>
 *		</ul>
 *		<p />
 *		<b>What is XPath ?</b><p />
 *		XPath is a solution to search nodes or attributes inside an
 *		XML tree. It is very similar to LDAP database search engines.
 *		<p />
 *		<table>
 *		<tr><td>toto</td>
 *		<td>Return all "toto" tags, children of the current node.</td>
 *		</tr>
 *		<tr><td>/toto</td>
 *		<td>Return all "toto" tags at the top-level of the XML
 *		document of the current node.</td></tr>
 *		<tr><td>toto/titi</td>
 *		<td>Return all "titi" tags, children of any "toto" node
 *		child of the current node.</td></tr>
 *		<tr><td>../tata</td><td>Return all "toto" nodes, children of
 *		the father of the current node (so, all "toto" sibling
 *		nodes).</td></tr>
 *		<tr><td>./toto</td><td>Like the first example.</td></tr>
 *		<tr><td>*</td>
 *		<td>Return all the children tags of the current node.</td></tr>
 *		<tr><td>* /titi</td><td>return all "titi" tags which have the
 *		current node as grandfather.</td></tr>
 *		<tr><td>//foo</td><td>Return all "foo" descent tags (sons,
 *		grand-sons, grand-grand-sons, etc.).</td></tr>
 *		<tr><td>toto//foo</td><td>Return all "foo" nodes that descent
 *		of a "toto" tag, which is a child of the current node.</td></tr>
 *		<tr><td>ying | yang</td><td>Return all children of the current
 *		node named "ying" or "yang".</td></tr>
 *		<tr><td>@id</td><td>Return the "id" attribute of the current
 *		node.</td></tr>
 *		<tr><td>@*</td><td>Return all attributes of the current
 *		node.</td></tr>
 *		<tr><td>toto/@name</td><td>Return the "name" attribute of all
 *		"toto" nodes children of the current node.</td></tr>
 *		<tr><td>toto[titi]</td><td>Return all "toto" tags, children of
 *		the current node, which have a "titi" child node.</td></tr>
 *		<tr><td>toto[titi | tutu]</td><td>Return all "toto" tags,
 *		children of the current node, which have a "titi" or a "tutu"
 *		child node.</td></tr>
 *		<tr><td>toto[@name]</td><td>Return all "toto" tags, children
 *		of the current node, which have a "name" attribute.</td></tr>
 *		<tr><td>toto[@name="pouet"]</td><td>Return all "toto" tags,
 *		children of the current node, which have a "name" attribute
 *		with the "pouet" value.</td></tr>
 *		<tr><td>*[@id='3' | tutu]</td><td>Return all elements, child
 *		of the current tag, which have an "id" attribute with a value
 *		equal to "3" and/or a child tag named "tutu".</td></tr>
 *		<tr><td>toto[@name='titi'][@id='25']</td><td>Return all "toto"
 *		tags which have a "name" attribute with a value equal to
 *		"titi", and an "id" attribute equal to "25".</td></tr>
 *		</table>
 * @version	1.0.0 Sep 14 2002
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YDOM_H__
#define __YDOM_H__

#include "ydefs.h"
#include "ysax.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

/*!
 * @enum	ydom_node_type_e
 *		Node type definitions.
 * @constant	VOID_NODE		Node without any type.
 * @constant	ELEMENT_NODE		It is an element (node of the XML tre)e
 * @constant	ATTRIBUTE_NODE		The node is an attribute of a node.
 * @constant	TEXT_NODE		The node contains text only.
 * @constant	CDATA_SECTION_NODE	The node correspond to a CDATA section.
 * @constant	PROCESSING_INSTRUCTION_NODE	The node contains some
 *						processing instruction.
 * @constant	COMMENT_NODE		XML comment node.
 * @constant	DOCUMENT_NODE		Root node that represent the whole
 *					XML document.
 */
enum ydom_node_type_e
{
  VOID_NODE = 0,
  ELEMENT_NODE,
  ATTRIBUTE_NODE,
  TEXT_NODE,
  CDATA_SECTION_NODE,
  PROCESSING_INSTRUCTION_NODE,
  COMMENT_NODE,
  DOCUMENT_NODE
};

/*! @typedef ydom_node_type_t See enum ydom_node_type_e. */
typedef enum ydom_node_type_e ydom_node_type_t;

/*!
 * @struct	ydom_s
 *		Structure for DOM representation.
 * @field	document_element	Pointer to the root document node.
 * @field	current_parsed_node	Pointer used during XML file parsing.
 * @field	error			Error status value.
 * @field	xml_version		String which contains the XML version
 *					of the document.
 * @field	encoding		String which contains the type of
 *					document encoding.
 * @field	standalone		"yes" if the document has a DTD.
 */
struct ydom_s
{
	struct ydom_node_s *document_element;
	struct ydom_node_s *current_parsed_node;
	yerr_t error;
	char *xml_version;
	char *encoding;
	char *standalone;
};

/*! @typedef ydom_t See struct ydom_s. */
typedef struct ydom_s ydom_t;

/*!
 * @struct	ydom_node_s
 *		Structure for XML Node representation.
 * @field	node_type	Type of the node.
 * @field	complete	Set to TRUE when the parse of the node in the
 *				XML file is complete.
 * @field	name		Name of the node.
 * @field	value		Value of the node.
 * @field	position	Node posision relative to its sibblings.
 * @field	parent		Pointer to the parent node.
 * @field	prev		Pointer to the previous sibling of the node.
 * @field	next		Pointer to the next sibling of the node.
 * @field	attributes	Pointer to the first attribute of the node.
 * @field	first_child	Pointer to the first child of the node.
 * @field	last_child	Pointer to the last child of the node.
 * @field	document	Pointer to the document node of this node.
 */
struct ydom_node_s
{
	ydom_node_type_t node_type;
	ybool_t complete;
	char *name;
	char *value;
	int position;
	struct ydom_node_s *parent;
	struct ydom_node_s *prev;
	struct ydom_node_s *next;
	struct ydom_node_s *attributes;
	struct ydom_node_s *first_child;
	struct ydom_node_s *last_child;
	struct ydom_node_s *document;
};

/*! @typedef ydom_node_t See struct ydom_node_s. */
typedef struct ydom_node_s ydom_node_t;

/*!
 * @function	ydom_new
 *		Create a new XML DOM object.
 * @return	A pointer to the created DOM object.
 */
ydom_t *ydom_new(void);

/*!
 * @function	ydom_del
 *		Delete a previously created XML DOM object and all
 *		memory allocated for it.
 * @param	dom	A pointer to the DOM object.
 */
void ydom_del(ydom_t *dom);

/*!
 * @function	ydom_read_file
 *		Parse an existing XML file.
 * @param	dom		A pointer to the DOM object.
 * @param	filename	Path to the file to parse.
 * @return	YENOERR if OK, an error code otherwise.
 */
yerr_t ydom_read_file(ydom_t *dom, const char *filename);

/*!
 * @function	ydom_read_stream
 *		Parse a stream (opened file, pipe, ...) that contains
 *		XML. If you have a file descriptor instead of a stream,
 *		use fdopen() to convert it.
 * @param	dom	A opinter to the DOM object.
 * @param	stream	The stream to read.
 * @return	YENOERR if OK, an error code otherwise.
 */
yerr_t ydom_read_stream(ydom_t *dom, FILE *stream);

/*!
 * @function	ydom_read_memory
 *		Parse a character string that contains XML.
 * @param	dom	A pointer to the DOM object.
 * @param	mem	The string to parse.
 * @return	YENOERR if OK, an error code otherwise.
 */
yerr_t ydom_read_memory(ydom_t *dom, char *mem);

/*!
 * @function	_ydom_get_root_node_of_node
 *		INTERNAL FUNCTION.
 * @param	node	A pointer to the node.
 * @return	The root node of the given node.
 */
ydom_node_t *_ydom_get_root_node_of_node(ydom_node_t *node);

/*!
 * @function	ydom_write
 *		Write the content of an XML tree to a stream.
 * @param	dom	A pointer to the DOM object.
 * @param	stream	The stream to write on.
 */
void ydom_write(ydom_t *dom, FILE *stream);

/*!
 * @function	ydom_dump
 *		Allocate a memory buffer and dump into it the XML content.
 * @param	dom	A pointer to the DOM object.
 * @return	A ystring that contains the XML.
 */
ystr_t ydom_dump(ydom_t *dom);

/*!
 * @function	ydom_set_version
 *		Set the version of XML language used in the document.
 * @param	dom	A pointer to the DOM object.
 * @param	version	Version string (i.e. "1.0" or "1.1").
 */
void ydom_set_version(ydom_t *dom, char *version);

/*!
 * @function	ydom_set_encoding
 *		Set the character encoding (like "iso-8859-1") of the document.
 * @param	dom		A pointer to the dom object.
 * @param	encoding	Encoding string.
 */
void ydom_set_encoding(ydom_t *dom, char *encoding);

/*!
 * @function	ydom_set_standalone
 *		Set the standalone state of the document.
 * @param	dom		A pointer to the DOM object.
 * @param	standalone	Standalone state ("yes" or "no").
 */
void ydom_set_standalone(ydom_t *dom, char *standalone);

/*!
 * @function	ydom_add_elem
 *		Create an XML element (a tag or a node, if you prefer) in the
 *		top-level hierarchy of the XML document.
 * @param	dom	A pointer to the DOM object.
 * @param	tagname	Name of the tag to add.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_add_elem(ydom_t *dom, char *tagname);

/*!
 * @function	ydom_add_text
 *		Create a text node in the top-level hierarchy of the XML tree.
 * @param	dom	A pointer to the DOM object.
 * @param	data	Text string.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_add_text(ydom_t *dom, char *data);

/*!
 * @function	ydom_add_comment
 *		Create an XML comment node in the top-level hierarchy of
 *		the XML document.
 * @param	dom	A pointer to the DOM object.
 * @param	data	Comment string.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_add_comment(ydom_t *dom, char *data);

/*!
 * @function	ydom_add_process_instr
 *		Create an XML processing instruction in the top-level
 *		hierarchy of the XML document.
 * @param	dom	A pointer to the DOM object.
 * @param	target	The processing instruction's target.
 * @param	data	The processing instruction's content.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_add_process_instr(ydom_t *dom, char *target, char *data);

/*!
 * @function	ydom_add_cdata
 *		Create a CDATA section in the top-level hierarchy of
 *		the XML document.
 * @param	dom	A pointer to the DOM object.
 * @param	data	The CDATA content.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_add_cdata(ydom_t *dom, char *data);

/*!
 * @function	ydom_get_document
 *		Return a pointer to the document element node of an XML tree.
 * @param	dom	A pointer to the DOM object.
 * @return	A pointer the the document node.
 */
ydom_node_t *ydom_get_document(ydom_t *dom);

/*!
 * @function	ydom_xpath
 *		Search for nodes in a XML tree from a XPath string.
 * @param	dom	A pointer to the DOM object.
 * @param	xpath	XPath string.
 * @return	A yvector of matching nodes.
 */
yvect_t ydom_xpath(ydom_t *dom, char *xpath);

/*!
 * @function	ydom_sort
 *		Do a quick sort on all nodes of the XML document.
 * @param	dom	A pointer to the DOM object.
 * @param	func	A function pointer ued to compare elements.
 */
void ydom_sort(ydom_t *dom, int (*func)(const void*, const void*));

/*!
 * @function	ydom_node_is_element
 *		Check if a node is an element (an XML tag), or not.
 * @param	node	A pointer to a node.
 * @return	TRUE or FALSE.
 */
ybool_t ydom_node_is_element(ydom_node_t *node);

/*!
 * @function	ydom_node_is_text
 *		Check if a node is a text node or not.
 * @param	node	A pointer to a node.
 * @return	TRUE or FALSE.
 */
ybool_t ydom_node_is_text(ydom_node_t *node);

/*!
 * @function	ydom_node_is_comment
 *		Check if a node is a comment node or not.
 * @param	node	A pointer to a node.
 * @return	TRUE or FALSE.
 */
ybool_t ydom_node_is_comment(ydom_node_t *node);

/*!
 * @function	ydom_node_is_process_instr
 *		Check if a node if a processing instruction node, or not.
 * @param	node	A pointer to a node.
 * @return	TRUE or FALSE.
 */
ybool_t ydom_node_is_process_instr(ydom_node_t *node);

/*!
 * @function	ydom_node_is_cdata
 *		Check if a node is a CDATA section or not.
 * @param	node	A pointer to a node.
 * @return	TRUE or FALSE.
 */
ybool_t ydom_node_is_cdata(ydom_node_t *node);

/*!
 * @function	ydom_node_is_attr
 *		Check if a node is an attribute node or not.
 * @param	node	A pointer to a node.
 * @return	TRUE or FALSE.
 */
ybool_t ydom_node_is_attr(ydom_node_t *node);

/*!
 * @function	ydom_node_is_document
 *		Check if a node is the document node (logical node which
 *		contains all the top-level hierarchy of the XML document).
 * @param	node	A pointer to a node.
 * @return	TRUE or FALSE.
 */
ybool_t ydom_node_is_document(ydom_node_t *node);

/*!
 * @function	ydom_node_add_attr
 *		Add an attribute to an existing XML node.
 * @param	node		A pointer to a node.
 * @param	attr_name	Name of the new attribute.
 * @param	attr_value	Value of the new attribute.
 * @return	A pointer to the created attribute node.
 */
ydom_node_t *ydom_node_add_attr(ydom_node_t *node, char *attr_name,
                                char *attr_value);

/*!
 * @function	ydom_node_set_attr
 *		Add or update an attribute.
 * @param	node		A pointer to a node.
 * @param	attr_name	Name of the new attribute.
 * @param	attr_value	Value of the new attribute.
 * @return	A pointer to the created attribute node.
 */
ydom_node_t *ydom_node_set_attr(ydom_node_t *node, char *attr_name,
				char *attr_value);

/*!
 * @function	ydom_node_add_elem
 *		Create an XML element (a tag or a node, if you prefer)
 *		and insert it as a child of an existing node.
 * @param	node		A pointer to a node.
 * @param	tagname		Name of the new tag.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_node_add_elem(ydom_node_t *node, char *tagname);

/*!
 * @function	ydom_node_add_text
 *		Create a text node and insert it as a child of an existing node.
 * @param	node	A pointer to a node.
 * @param	data	Text string.
 * @return	A pointer to the created text node.
 */
ydom_node_t *ydom_node_add_text(ydom_node_t *node, char *data);

/*!
 * @function	ydom_node_add_comment
 *		Create an XML comment node and insert it as a child of an
 *		existing node.
 * @param	node	A pointer to a node.
 * @param	data	Comment string.
 * @return	A pointer to the created comment node.
 */
ydom_node_t *ydom_node_add_comment(ydom_node_t *node, char *data);

/*!
 * @function	ydom_node_add_process_instr
 *		Create an XML processing instruction and insert it as a child
 *		of an existing node.
 * @param	node	A pointer to a node.
 * @param	target	Processing instruction's target.
 * @param	data	Processing instruction's content.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_node_add_process_instr(ydom_node_t *node, char *target,
					 char *data);

/*!
 * @function	ydom_node_add_cdata
 *		Create a CDATA section and insert it as a child of an
 *		existing node.
 * @param	node	A pointer to a node.
 * @param	data	CDATA content.
 * @return	A pointer to the created node.
 */
ydom_node_t *ydom_node_add_cdata(ydom_node_t *node, char *data);

/*!
 * @function	ydom_node_get_name
 *		Return a copy of the name of a node.
 * @param	node	A pointer to a node.
 * @return	A copy of the node's name.
 */
char *ydom_node_get_name(ydom_node_t *node);

/*!
 * @function	ydom_node_get_value
 *		Return a copy of the value of a node.
 * @param	node	A pointer to a node.
 * @return	A copy of the node's name.
 */
char *ydom_node_get_value(ydom_node_t *node);

/*!
 * @function	ydom_node_get_nbr_children
 *		Return the number of children of a given XML node.
 * @param	node	A pointer to a node.
 * @return	The number of children.
 */
int ydom_node_get_nbr_children(ydom_node_t *node);

/*!
 * @function	ydom_node_get_nbr_attr
 *		Return the number of attributes of a given node.
 * @param	node	A pointer to a node.
 * @return	The number of attributes.
 */
int ydom_node_get_nbr_attr(ydom_node_t *node);

/*!
 * @function	ydom_node_get_nbr_same_attr
 *		Return the number of attributes of a given node, with
 *		the same name.
 * @param	node		A pointer to a node.
 * @param	attr_name	The searched attribute's name.
 * @return	The number of attributes with the same name.
 */
int ydom_node_get_nbr_same_attr(ydom_node_t *node, char *attr_name);

/*!
 * @function	ydom_node_get_parent
 *		Return a pointer to the parent node of an XML node.
 * @param	node	A pointer to a node.
 * @return	A pointer to the parent's node.
 */
ydom_node_t *ydom_node_get_parent(ydom_node_t *node);

/*!
 * @function	ydom_node_get_prev
 *		Return a pointer to the previous sibling node of an XML node.
 * @param	node	A pointer to a node.
 * @return	A pointer to the node's previous sibling node.
 */
ydom_node_t *ydom_node_get_prev(ydom_node_t *node);

/*!
 * @function	ydom_node_get_next
 *		Return a pointer to the next sibling node of an XML node.
 * @param	node	A pointer to a node.
 * @return	A pointer to the node's next sibling node.
 */
ydom_node_t *ydom_node_get_next(ydom_node_t *node);

/*!
 * @function	ydom_node_get_attr_value
 *		Return the value of the first attribute of a node that have
 *		a given name.
 * @param	node		A pointer to a node.
 * @param	attr_name	The searched attribute's name.
 * @return	The attribute's value.
 */
char *ydom_node_get_attr_value(ydom_node_t *node, char *attr_name);

/*!
 * @function	ydom_node_get_first_attr
 *		Return a pointer to the first attribute of a node.
 * @param	node	A pointer to a node.
 * @return	A pointer to the first attribute's node.
 */
ydom_node_t *ydom_node_get_first_attr(ydom_node_t *node);

/*!
 * @function	ydom_node_get_first_child
 *		Return a pointer to the first child of a node.
 * @param	node	A pointer to a node.
 * @return	A pointer to the first child's node.
 */
ydom_node_t *ydom_node_get_first_child(ydom_node_t *node);

/*!
 * @function	ydom_node_get_last_child
 *		Return a pointer to the last child of a node.
 * @param	node	A pointer to a node.
 * @return	A pointer to the last child's node.
 */
ydom_node_t *ydom_node_get_last_child(ydom_node_t *node);

/*!
 * @function	ydom_node_xpath
 *		Search for nodes or attributes from a XPath string, taking
 *		a given node as root.
 * @param	node	A pointer to a node.
 * @param	xpath	The XPath string.
 * @return	A vector of matching nodes.
 */
yvect_t ydom_node_xpath(ydom_node_t *node, char *xpath);

/*!
 * @function	ydom_node_sort
 *		Do a quick sort on the children of a node. Sub-children are
 *		not sorted.
 * @param	node	A pointer to a node.
 * @param	func	A functin pointer used to compare elements.
 */
void ydom_node_sort(ydom_node_t *node, int (*func)(const void *, const void*));

/*!
 * @function	ydom_node_sort_all
 *		Do a quick sort on the children of a node. Sub-children,
 *		sub-sub-children, ... are recursivly sorted.
 * @param	node	A pointer to a node.
 * @param	func	A functin pointer used to compare elements.
 */
void ydom_node_sort_all(ydom_node_t *node,
                        int (*func)(const void*, const void*));

/*!
 * @function	ydom_node_rm
 *		Delete an XML node and all its children and attributes, and
 *		remove all links that point to it in the parent and siblings
 *		nodes.
 * @param	node	A pointer to a node.
 * @return	A pointer to the deleted node's parent.
 */
ydom_node_t *ydom_node_rm(ydom_node_t *node);

/*!
 * @function	ydom_node_rm_children
 *		Delete all children of an XML node, but node the node itself.
 * @param	node	A pointer to a node.
 */
void ydom_node_rm_children(ydom_node_t *node);

/*!
 * @function	ydom_node_rm_attr
 *		Remove the first attribute of a node that have a given name.
 * @param	node		A pointer to a node.
 * @param	attr_name	Name of the attribute to remove.
 */
void ydom_node_rm_attr(ydom_node_t *node, char *attr_name);

/*!
 * @function	ydom_node_rm_attributes
 *		Remove all attributes of an XML node.
 * @param	node	A pointer to a node.
 */
void ydom_node_rm_attributes(ydom_node_t *node);

/*!
 * @function	ydom_get_nodes_from_xpath
 *		Main XPath parsing function. Process an XPath expression
 *		trough a set of XML nodes to search XML nodes.
 * @param	nodes	A yvector of XML nodes from where the XPath
 *			search must be done.
 * @param	xpath	The XPath expression.
 * @return	A yvector of XML nodes that matches the XPath.
 */
yvect_t ydom_get_nodes_from_xpath(yvect_t nodes, char *xpath);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YDOM_H__ */
