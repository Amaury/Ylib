/**
 * @header	ycgi.h
 * @abstract	All definitions for CGI requests handling.
 * @discussion	These functions try to help CGI programming in C, providing
 *		simple means to get and decode request data.
 * @version	1.0 Aug 12 2003
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "y.h"

/** @define REQUEST_METHOD String definition. */
#define	REQUEST_METHOD	"REQUEST_METHOD"
/** @define METHOD_GET String definition. */
#define METHOD_GET	"GET"
/** @define METHOD_POST String definition. */
#define	METHOD_POST	"POST"
/** @define QUERY_STRING String definition. */
#define	QUERY_STRING	"QUERY_STRING"
/** @define CONTENT_LENGTH String definition. */
#define	CONTENT_LENGTH	"CONTENT_LENGTH"
/** @define HTTP_COOKIE String definition. */
#define HTTP_COOKIE	"HTTP_COOKIE"

/** @define ITEMS_SEPARATOR Character definition. */
#define ITEMS_SEPARATOR		AMP
/** @define VALUE_SEPARATOR Character definition. */
#define	VALUE_SEPARATOR		EQ
/** @define ENCODED_PREFIX Character definition. */
#define ENCODED_PREFIX		PERCENT
/** @define SPACE_SYMBOL Character definition. */
#define SPACE_SYMBOL		PLUS
/** @define COOKIE_SEPARATOR Character definition. */
#define COOKIE_SEPARATOR	SEMICOLON

/**
 * @typedef	ycgi_item_t
 *		Structure for one CGI request's item.
 * @field	name	Item's name.
 * @field	value	Item's value.
 */
typedef struct {
	ystr_t name;
	ystr_t value;
} ycgi_item_t;

/**
 * @function	ycgi_header_nochange
 *		Send a "204 No Change" HTTP header.
 */
void ycgi_header_nochange(void);

/**
 * @function	ycgi_header_redirect
 *		Send a redirection HTTP header.
 * @param	url	Redirection's URL.
 */
void ycgi_header_redirect(const char *url);

/**
 * @function	ycgi_header
 *		Send a "Content-Type" HTTP header.
 * @param	content_type	Header's content-type. Optional. Set to
 *				"text/html" if NULL.
 */
void ycgi_header(const char *content_type);

/**
 * @function	ycgi_header_cookie_begin
 *		Send the content-type header before sending cookies.
 * @param	content_type	Header's content-type. Optional. Set to
 *				"text/html" if NULL.
 */
void ycgi_header_cookie_begin(const char *content_type);

/**
 * @function	ycgi_header_cookie
 *		Send a cookie.
 * @param	name		Cookie's name. Mandatory.
 * @param	value		Cookie's value. Mandatory.
 * @param	expiration	Cookie's expiration date. Optional. Must be
 *				like "Sunday, 01-Jan-2000 00:00:00 GMT".
 * @param	path		Cookie's path. Optional. Must be like "/" or
 *				"/path/to/cgi".
 * @param	domain		Cookie's domain. Optional. Must be like
 *				"host.domain.tld" or ".domain.tld".
 */
void ycgi_header_cookie(const char *name, const char *value,
			const char *expiration, const char *path,
			const char *domain);

/**
 * @function	ycgi_header_cookie_end
 *		Send the end of HTTP headers.
 */
void ycgi_header_cookie_end(void);

/**
 * @function	ycgi_read_cookies
 *		Read the cookies.
 * @return	An array of cookies.
 */
yarray_t ycgi_read_cookies(void);

/**
 * @function	ycgi_read_request
 *		Read a CGI request (GET or POST).
 * @return	An array of CGI items.
 */
yarray_t ycgi_read_request(void);

/**
 * @function	ycgi_read_get_request
 *		Read a GET CGI request.
 * @return	An array of CGI items.
 */
yarray_t ycgi_read_get_request(void);

/**
 * @function	ycgi_read_post_request
 *		Read a POST CGI request.
 * @return	An array of CGI items.
 */
yarray_t ycgi_read_post_request(void);

/**
 * @function	ycgi_del_items
 *		Free the result of a CGI request or cookies reading.
 * @param	items	A pointer to the resulting array.
 */
void ycgi_del_items(yarray_t *items);

/**
 * @function	ycgi_del_item
 *		Free one CGI request item.
 * @param	A pointer to the CGI item.
 */
void ycgi_del_item(ycgi_item_t *item);

/**
 * @function	ycgi_separate_items
 *		Separate the request's items and decode them.
 * @param	The query string.
 * @return	An array of CGI items.
 */
yarray_t ycgi_separate_items(const char *query);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

