#include <stdio.h>
#include <string.h>
#include "ylog.h"
#include "yqprintable.h"

/*
** yqprintable_encode()
** Encode some binary data to quoted-printable string.
*/
ystr_t yqprintable_encode(const ybin_t bin)
{
   ystr_t str, para;
   unsigned char *pt, tmp[4];
   int i;

   if (!bin.data || !bin.len)
     return (NULL);
   if (!(str = ys_new("")))
     return (NULL);
   if (!(para = ys_new("")))
     {
	ys_del(&str);
	return (NULL);
     }
   pt = bin.data;
   for (i = 0; i < bin.len; i++)
     {
	if (pt[i] == CR && pt[i + 1] == LF)
	  {
	     i += 2;
	     ys_cat(&str, para);
	     ys_cat(&str, (char*)tmp);
	     ys_trunc(para);
	     continue ;
	  }
	if (pt[i] < 32 || pt[i] == EQ || pt[i] > 126)
	  sprintf((char*)tmp, "%c%2X", EQ, pt[i]);
	else
	  sprintf((char*)tmp, "%c", pt[i]);
	if ((ys_len(para) + strlen((char*)tmp)) >= 76)
	  {
	     ys_cat(&str, para);
	     ys_cat(&str, "=\r\n");
	     ys_trunc(para);
	  }
	ys_cat(&para, (char*)tmp);
     }
   ys_cat(&str, para);
   ys_trim(str);
   return (str);
}

/*
** yqprintable_decode()
** Decode a quoted-printable encoded string.
*/
ybin_t yqprintable_decode(const char *pt)
{
  ybin_t res;
  unsigned char *str;
  int i, j;

  YLOG_ADD(YLOG_DEBUG, "yqprintable_decode entering");
  if (!(str = malloc0(strlen(pt) + 2)))
    {
      res.data = NULL;
      res.len = 0;
      return (res);
    }
  i = j = 0;
  while (pt[i])
    {
      if (pt[i] == EQ)
	{
	   if (pt[i + 1] && pt[i + 2] &&
	       IS_HEXA(pt[i + 1]) && IS_HEXA(pt[i + 2]))
	     {
		unsigned int c;
		sscanf(pt + i + 1, "%02x", &c);
		str[j++] = (char)c;
		i += 3;
	     }
	   else
	     {
		int k = 1;
		while (pt[i + k] &&
		       (pt[i + k] == SPACE || pt[i + k] == TAB))
		  k++;
		if (!pt[i + k])
		  i += k;
		else if (pt[i + k] == CR && pt[i + k + 1] == LF)
		  i += k + 2;
		else if (pt[i + k] == CR || pt[i + k] == LF)
		  i += k + 1;
	     }
	}
      else
	str[j++] = pt[i++];
    }
   str[j] = '\0';
   res.data = (void*)str;
   res.len = j;
   return (res);
}
