#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "ylog.h"
#include "ycgi.h"

/*
** ycgi_header_nochange()
** Send a "204 No Change" HTTP header.
*/
void ycgi_header_nochange()
{
  printf("HTTP/1.0 204 No Change\n\n");
  fflush(stdout);
}

/*
** ycgi_header_redirect()
** Send a redirection HTTP header.
*/
void ycgi_header_redirect(const char *url)
{
  if (!url)
    ycgi_header_nochange();
  else
    {
      printf("Location: %s\n\n", url);
      fflush(stdout);
    }
}

/*
** ycgi_header()
** Send a "Content-Type" HTTP header.
*/
void ycgi_header(const char *content_type)
{
  printf("Content-type: %s\n\n", content_type ? content_type : "text/html");
  fflush(stdout);
}

/*
** ycgi_header_cookie_begin()
** Send the content-type header before sending cookies.
*/
void ycgi_header_cookie_begin(const char *content_type)
{
  printf("Content-type: %s\n", content_type ? content_type : "text/html");
  fflush(stdout);
}

/*
** ycgi_header_cookie()
** Send a cookie.
*/
void ycgi_header_cookie(const char *name, const char *value,
			const char *expiration, const char *path,
			const char *domain)
{
  ystr_t enc_name, enc_value;

  enc_name = yurl_encode(name);
  enc_value = yurl_encode(value);
  printf("Set-Cookie: %s=%s", enc_name, enc_value);
  ys_del(&enc_name);
  ys_del(&enc_value);
  if (expiration)
    printf("; expires=%s", expiration);
  if (path)
    printf("; path=%s", path);
  if (domain)
    printf("; domain=%s", domain);
  printf("\n");
  fflush(stdout);
}

/*
** ycgi_header_cookie_end()
** Send the end of HTTP headers.
*/
void ycgi_header_cookie_end()
{
  printf("\n");
  fflush(stdout);
}

/*
** ycgi_read_cookies()
** Read the cookies.
*/
yvect_t ycgi_read_cookies()
{
  char *cookie_str = NULL;
  yvect_t res;
  char *begin, *pt, *sep;
  char *name, *value;
  ycgi_item_t *item;

  if (!(cookie_str = getenv(HTTP_COOKIE)) || !strlen(cookie_str) ||
      !(res = yv_new()))
    return (NULL);
  for (begin = pt = cookie_str; ; ++pt)
    {
      if ((*pt == COOKIE_SEPARATOR || !*pt) &&
	  (sep = strchr(begin, VALUE_SEPARATOR)) &&
	  sep < pt)
	{
	  item = malloc0(sizeof(ycgi_item_t));
	  name = malloc0(sep - begin + 1);
	  strncpy(name, begin, sep - begin);
	  name[sep - begin] = '\0';
	  value = malloc0(pt - sep);
	  strncpy(value, sep + 1, pt - sep - 1);
	  value[pt - sep - 1] = '\0';
	  item->name = yurl_decode(name);
	  item->value = yurl_decode(value);
	  free0(name);
	  free0(value);
	  yv_add(&res, item);
	  begin = pt + 1;
	}
      if (!*pt)
	break ;
    }
  return (res);
}

/*
** ycgi_read_request()
** Read a CGI request (GET or POST).
*/
yvect_t ycgi_read_request()
{
  char *method = NULL;

  if (!(method = getenv(REQUEST_METHOD)) || !strlen(method))
    return (NULL);
  if (!strcasecmp(method, METHOD_GET))
    return (ycgi_read_get_request());
  else if (!strcasecmp(method, METHOD_POST))
    return (ycgi_read_post_request());
  return (NULL);
}

/*
** ycgi_read_get_request()
** Read a GET CGI request.
*/
yvect_t ycgi_read_get_request()
{
  char *query_string = NULL;

  if (!(query_string = getenv(QUERY_STRING)) || !strlen(query_string))
    return (NULL);
  return (ycgi_separate_items(query_string));
}

/*
** ycgi_read_post_request()
** Read a POST CGI request.
*/
yvect_t ycgi_read_post_request()
{
  int content_length = 0, readed = 0;
  char *pt;
  yvect_t res = NULL;

  if ((pt = getenv(CONTENT_LENGTH)) && (content_length = atoi(pt)))
    {
      pt = malloc0(content_length + 1);
      while (readed < content_length)
	readed += read(0, pt + readed, content_length);
      pt[content_length] = '\0';
      res = ycgi_separate_items(pt);
      free0(pt);
    }
  return (res);
}

/*
** ycgi_del_items()
** Free the result of a CGI request reading.
*/
void ycgi_del_items(yvect_t *items)
{
  if (items && *items)
    yv_del(items, _ycgi_free_item, NULL);
}

/*
** ycgi_del_item()
** Free one CGI request item.
*/
void ycgi_del_item(ycgi_item_t *item)
{
  if (!item)
    return ;
  if (item->name)
    ys_del(&item->name);
  if (item->value)
    ys_del(&item->value);
  free0(item);
}

/*
** ycgi_separate_items()
** Separate the request's items and decode them.
*/
yvect_t ycgi_separate_items(const char *query)
{
  yvect_t res;
  const char *begin, *pt, *sep;
  ycgi_item_t *item;
  char *name, *value;

  if (!(res = yv_new()))
    return (NULL);
  for (begin = pt = query; ; ++pt)
    {
      if ((*pt == ITEMS_SEPARATOR || !*pt) &&
	  (sep = strchr(begin, VALUE_SEPARATOR)) &&
	  sep < pt)
	{
	  item = malloc0(sizeof(ycgi_item_t));
	  name = malloc0(sep - begin + 1);
	  strncpy(name, begin, sep - begin);
	  name[sep - begin] = '\0';
	  value = malloc0(pt - sep);
	  strncpy(value, sep + 1, pt - sep - 1);
	  value[pt - sep - 1] = '\0';
	  item->name = yurl_decode(name);
	  item->value = yurl_decode(value);
	  free0(name);
	  free0(value);
	  yv_add(&res, item);
	  begin = pt + 1;
	}
      if (!*pt)
	break ;
    }
  return (res);
}

/*
** _ycgi_free_item()
** Function used to free the result of a request.
*/
void _ycgi_free_item(void *elem, void *data)
{
  ycgi_del_item((ycgi_item_t*)elem);
}
