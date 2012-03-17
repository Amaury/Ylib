#include <stdio.h>
#include <string.h>
#include "ydefs.h"
#include "ystr.h"
#include "yurl.h"

static yurl_protocol_def_t _yurl_proto_table[] =
{
  {YURL_PROTOCOL_HTTP, "http", 80},
  {YURL_PROTOCOL_HTTPS, "https", 443},
  {YURL_PROTOCOL_MAILTO, "mailto", 25},
  {YURL_PROTOCOL_SMTP, "smtp", 25},
  {YURL_PROTOCOL_POP, "pop", 110},
  {YURL_PROTOCOL_FTP, "ftp", 21},
  {YURL_PROTOCOL_NNTP, "nntp", 119},
  {YURL_PROTOCOL_TELNET, "telnet", 23},
  {YURL_PROTOCOL_WAIS, "wais", 210},
  {YURL_PROTOCOL_OABP, "oabp", 11137},
  {YURL_PROTOCOL_FILE, "file", 0},
  {YURL_PROTOCOL_UNDEF, "", 0},
  {0, 0, 0}
};

/*
** yurl_create()
** Create a yurl_t. Parameters are copied.
**/
yurl_t *yurl_create(yurl_protocol_t proto, char *login, char *pass, char *auth,
		    char *host, unsigned short port, char *location, char *query)
{
  yurl_t *res;

  if (!(res = malloc0(sizeof(yurl_t))))
    return (NULL);
  res->proto = proto;
  res->login = login ? strdup(login) : NULL;
  res->pass = pass ? strdup(pass) : NULL;
  res->auth = auth ? strdup(auth) : NULL;
  res->host = host ? strdup(host) : NULL;
  res->port = port;
  res->location = location ? strdup(location) : NULL;
  res->query = query ? strdup(query) : NULL;
  return (res);
}

/*
** yurl_parse()
** Parse a given URL.
*/
yurl_t *yurl_parse(const char *url, ybool_t strict)
{
  yurl_t *res;
  const char *pt, *pt2;
  char *arobase, *colon;
  int len;

  if (!(res = malloc0(sizeof(yurl_t))))
    return (NULL);
  res->proto = strict ? YURL_PROTOCOL_UNDEF : YURL_PROTOCOL_HTTP;
  res->port = strict ? 0 : 80;
  /* search for protocol */
  if ((pt = strstr(url, "://")))
    {
      int i;
      for (i = 0; _yurl_proto_table[i].string; ++i)
	if (!strncmp(_yurl_proto_table[i].string, url, pt - url))
	  {
	    res->proto = _yurl_proto_table[i].proto;
	    res->port = _yurl_proto_table[i].port;
	    break ;
	  }
      url = pt + strlen("://");
    }
  /* search for login/password and authentication type*/
  if ((arobase = strchr(url, AT)))
    {
      res->login = malloc0(arobase - url + 1);
      memcpy(res->login, url, arobase - url);
      res->login[arobase - url] = '\0';
      if ((colon = strchr(res->login, COLON)))
	{
	  len = strlen(colon + 1);
	  res->pass = malloc0(len + 1);
	  memcpy(res->pass, colon + 1, len);
	  res->pass[len] = *colon = '\0';
	}
      if ((colon = strstr(res->login, ";auth=")) ||
	  (colon = strstr(res->login, ";AUTH=")))
	{
	  len = strlen(colon + strlen(";auth="));
	  res->auth = malloc0(len + 1);
	  memcpy(res->auth, colon + strlen(";auth="), len);
	  res->auth[len] = *colon = '\0';
	}
      url = arobase + 1;
    }
  /* search for hostname */
  for (pt = url;
       IS_CHAR(*pt) || IS_NUM(*pt) || *pt == '-' || *pt == '.';
       ++pt)
    ;
  res->host = malloc0(pt - url + 1);
  memcpy(res->host, url, pt - url);
  res->host[pt - url] = '\0';
  /* search for port number */
  if (*pt == COLON)
    {
      int i;
      if (!sscanf(pt + 1, "%d", &i))
	{
	  yurl_free(res);
	  return (NULL);
	}
      res->port = (unsigned short)i;
      for (pt += 1; IS_NUM(*pt); ++pt)
	;
    }
  /* search location and query strings */
  if (*pt == SLASH)
    {
      if ((pt2 = strchr(pt, INTERROG)))
	{
	  if ((res->location = malloc0(pt2 - pt + 1)))
	    {
	      memcpy(res->location, pt, pt2 - pt);
	      res->location[pt2 - pt] = '\0';
	    }
	  len = strlen(pt2 + 1);
	  if ((res->query = malloc0(len + 1)))
	    {
	      memcpy(res->query, pt2 + 1, len);
	      res->query[len] = '\0';
	    }
	}
      else
	{
	  len = strlen(pt);
	  if ((res->location = malloc0(len + 1)))
	    {
	      memcpy(res->location, pt, len);
	      res->location[len] = '\0';
	    }
	}
    }
  else if (*pt == INTERROG)
    {
      res->location = strdup("/");
      len = strlen(pt + 1);
      if ((res->query = malloc(len + 1)))
	{
	  memcpy(res->location, pt + 1, len);
	  res->query[len] = '\0';
	}
    }
  else if (*pt)
    {
      yurl_free(res);
      return (NULL);
    }
  if (!strict && !res->location)
    res->location = strdup("/");
  return (res);
}

/*
** yurl_get_proto_string()
** Return the string associated to a protocol.
*/
char *yurl_get_proto_string(yurl_protocol_t proto)
{
  int i;

  for (i = 0; _yurl_proto_table[i].string; ++i)
    if (proto == _yurl_proto_table[i].proto)
      return (_yurl_proto_table[i].string ?
	      strdup(_yurl_proto_table[i].string) :
	      NULL);
  return (NULL);
}

/*
** yurl_get_proto_port()
** Return the port number associated to a protocol.
*/
unsigned short yurl_get_proto_port(yurl_protocol_t proto)
{
  int i;

  for (i = 0; _yurl_proto_table[i].string; ++i)
    if (proto == _yurl_proto_table[i].proto)
      return (_yurl_proto_table[i].port);
  return (0);
}

/*
** yurl_assemble()
** Create a string from a yurl_t.
*/
char *yurl_assemble(yurl_t *url)
{
  ystr_t str1, str2;
  unsigned short port;
  char *proto, *res;

  if (!(proto = yurl_get_proto_string(url->proto)))
    return (NULL);
  port = yurl_get_proto_port(url->proto);
  if (!(str1 = ys_new(proto)))
    {
      free0(proto);
      return (NULL);
    }
  free0(proto);
  if (!ys_cat(&str1, "://"))
    {
      ys_del(&str1);
      return (NULL);
    }
  if (url->login &&
      (!ys_cat(&str1, url->login) ||
       (url->auth &&
	(!ys_cat(&str1, ";auth=") ||
	 !ys_cat(&str1, url->auth))) ||
       (url->pass &&
	(!ys_cat(&str1, ":") ||
	 !ys_cat(&str1, url->pass))) ||
       !ys_cat(&str1, "@")))
    {
      ys_del(&str1);
      return (NULL);
    }
  if (url->host &&
      !ys_cat(&str1, url->host))
    {
      ys_del(&str1);
      return (NULL);
    }
  if (url->port != port)
    {
      if (!(str2 = ys_new("")))
	{
	  ys_del(&str1);
	  return (NULL);
	}
      if (!ys_printf(&str2, "%s%d", ":", url->port) ||
	  !ys_cat(&str1, (char*)str2))
	{
	  ys_del(&str2);
	  ys_del(&str1);
	  return (NULL);
	}
      ys_del(&str2);
    }
  if ((url->location && !ys_cat(&str1, url->location)) ||
      (!url->location && !ys_cat(&str1, "/")))
    {
      ys_del(&str1);
      return (NULL);
    }
  if (url->query &&
      (!ys_cat(&str1, "?") || !ys_cat(&str1, url->query)))
    {
      ys_del(&str1);
      return (NULL);
    }
  res = ys_string(str1);
  ys_del(&str1);
  return (res);
}

/*
** yurl_free()
** Free the memory allocated for a yurl_t.
*/
void *yurl_free(yurl_t *url_struct)
{
  if (url_struct)
    {
      free0(url_struct->login);
      free0(url_struct->pass);
      free0(url_struct->host);
      free0(url_struct->location);
      free0(url_struct);
    }
  return (NULL);
}

/*
** yurl_encode()
** URL-encode a character string.
*/
ystr_t yurl_encode(const char *url)
{
  ystr_t res;
  const char *pt;
  char tmp[4];

  if (!(res = ys_new("")))
    return (NULL);
  for (pt = url; *pt; ++pt)
    {
      if (*pt == SPACE)
	ys_addc(&res, PLUS);
      else if (IS_CHAR(*pt) || IS_NUM(*pt) ||
	       *pt == DOT || *pt == MINUS ||
	       *pt == UNDERSCORE)
	ys_addc(&res, *pt);
      else
	{
	  sprintf(tmp, "%c%2x", PERCENT, *pt);
	  ys_cat(&res, tmp);
	}
    }
  return (res);
}

/*
** yurl_decode()
** Decode an URL-encoded character string.
*/
ystr_t yurl_decode(const char *url)
{
  ystr_t res;
  const char *pt;

  if (!(res = ys_new("")))
    return (NULL);
  for (pt = url; *pt; ++pt)
    {
      if (*pt == PERCENT)
	{
	  unsigned int c;
	  sscanf(pt + 1, "%2x", &c);
	  ys_addc(&res, (char)c);
	  pt += 2;
	}
      else if (*pt == PLUS)
	ys_addc(&res, SPACE);
      else
	ys_addc(&res, *pt);
    }
  return (res);
}
