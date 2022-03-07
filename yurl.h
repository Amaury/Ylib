/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	yurl.h
 *		Handle URLs. An URL as the form:
 *		<pre>protocol://&lt;user&gt;[;auth=&lt;AUTH&gt;][:&lt;password&gt;]@host[:&lt;port&gt;]/path[?query][;type=&lt;TYPECODE&gt;]</pre>
 *		<br />
 *		Where AUTH is used for POP authentication (see RFC2384) and
 *		TYPECODE is used for FTP commands (see RFC1738).
 * @version	1.0.0 Oct 27 2002
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YURL_H__
#define __YURL_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "ydefs.h"
#include "ystr.h"

/*!
 * @enum	yurl_protocol_e
 *		List of known protocols.
 * @constant	YURL_PROTOCOL_HTTP	Protocol for HTTP.
 * @constant	YURL_PROTOCOL_HTTPS	Protocol for HTTPS.
 * @constant	YURL_PROTOCOL_MAILTO	Protocol for mail sending.
 * @constant	YURL_PROTOCOL_SMTP	Protocol for mail sending.
 * @constant	YURL_PROTOCOL_POP	Protocol for POP.
 * @constant	YURL_PROTOCOL_FTP	Protocol for FTP.
 * @constant	YURL_PROTOCOL_NNTP	Protocol for NNTP.
 * @constant	YURL_PROTOCOL_TELNET	Protocol for Telnet.
 * @constant	YURL_PROTOCOL_WAIS	Protocol for WAIS.
 * @constant	YURL_PROTOCOL_OABP	Protocol for OABP.
 * @constant	YURL_PROTOCOL_FILE	Protocol for file access.
 * @constant	YURL_PROTOCOL_UNDEF	Undefined protocol.
 */
enum yurl_protocol_e
{
  YURL_PROTOCOL_HTTP = 0,
  YURL_PROTOCOL_HTTPS,
  YURL_PROTOCOL_MAILTO,
  YURL_PROTOCOL_SMTP,
  YURL_PROTOCOL_POP,
  YURL_PROTOCOL_FTP,
  YURL_PROTOCOL_NNTP,
  YURL_PROTOCOL_TELNET,
  YURL_PROTOCOL_WAIS,
  YURL_PROTOCOL_OABP,
  YURL_PROTOCOL_FILE,
  YURL_PROTOCOL_UNDEF
};

/*! @typedef yurl_protocol_t See enum yrul_protocol_e. */
typedef enum yurl_protocol_e yurl_protocol_t;

/*!
 * @struct	yurl_protocol_def_s
 *		Definition of a known protocol.
 * @field	proto	Protocol's name.
 * @field	string	Protocol's string definition.
 * @field	port	Protocol's default port.
 */
struct yurl_protocol_def_s
{
  yurl_protocol_t proto;
  char *string;
  int port;
};

/*! @typedef yurl_protocol_def_t See struct yurl_protocol_def_t. */
typedef struct yurl_protocol_def_s yurl_protocol_def_t;

/*!
 * @struct	yurl_s
 *		Describe an URL.
 * @field	proto		Protocol used for this URL.
 * @field	login		Login (in case of HTTP authentication).
 * @field	pass		Password (in case of HTTP authentication).
 * @field	auth		Authentication method (see RFC2384).
 * @field	host		Hostname.
 * @field	port		Port number.
 * @field	location	Location string.
 * @field	query		Query string.
 * @field	typecode	FTP type code (see RFC1738).
 */
struct yurl_s
{
  yurl_protocol_t proto;
  char *login;
  char *pass;
  char *auth;
  char *host;
  unsigned short port;
  char *location;
  char *query;
  char *typecode;
};

/*! @typedef yurl_t See struct yurl_s. */
typedef struct yurl_s yurl_t;

/*!
 * @function	yurl_create
 *		Create a yurl_t. Parameters are copied.
 * @param	proto	URL's protocol.
 * @param	login	HTTP authentication's login (if needed).
 * @param	pass	HTTP authentication's password (if needed).
 * @param	auth	Authntification method (if nedded).
 * @param	host	Hostname
 * @param	port	Port number
 * @param	location	Location (beginning with a slash).
 * @param	query	URL's querystring (if needed).
 * @return	NULL if an error occurs, otherwise a pointer to the created yurl_t.
 */
yurl_t *yurl_create(yurl_protocol_t proto, char *login, char *pass, char *auth,
		    char *host, unsigned short port, char *location, char *query);

/*!
 * @function	yurl_parse
 *		Parse a given URL.
 * @param	url	The URL string.
 * @param	strict	If set to TRUE, the parser request strictly valid URLs.
 *			If set to FALSE, the parser is "relax" with the given URL.
 *			Protocol prefix ('http://') are not necessary, default port
 *			is set to 80, default location is set to '/'.
 * @return	NULL if the URL isn't valid, otherwise a pointer to a yurl_t.
 */
yurl_t *yurl_parse(const char *url, ybool_t strict);

/*!
 * @function	yurl_get_proto_string
 *		Return the string associated to a protocol. The string is
 *		allocated (it must be freed).
 * @param	proto	The protocol.
 * @return	The character string or NULL.
 */
char *yurl_get_proto_string(yurl_protocol_t proto);

/*!
 * @function	yurl_get_proto_port
 *		Return the port number associated to a protocol.
 * @param	proto	The protocol.
 * @return	The port number.
 */
unsigned short yurl_get_proto_port(yurl_protocol_t proto);

/*!
 * @function	yurl_assemble
 *		Create a string from a yurl_t.
 * @param	url	A pointer to the URL object.
 * @return	The character string, or NULL if an error occurs.
 */
char *yurl_assemble(yurl_t *url);

/*!
 * @function	yurl_free
 *		Free the memory allocated for a yurl_t.
 * @param	url_struct	A pointer to the yurl_t structure.
 * @return	Always NULL.
 */
void *yurl_free(yurl_t *url_struct);

/*!
 * @function	yurl_encode
 *		URL-encode a character string.
 * @param	url	The URL to encode.
 * @return	A ystring that contains the encoded URL.
 */
ystr_t yurl_encode(const char *url);

/*!
 * @function	yurl_decode
 *		Dncode an URL-encoded character string.
 * @param	url	The string to decode.
 * @return	A ystring that contains the decoded URL.
 */
ystr_t yurl_decode(const char *url);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YURL_H__ */
