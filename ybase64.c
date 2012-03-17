#include <string.h>
#include "ybase64.h"

static char _b64_enc[65] =
{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
};

#define _B64_DEC(c)	((c >= 'A' && c <= 'Z') ? (c - 65) : \
			 (c >= 'a' && c <= 'z') ? (c - 71) : \
			 (c >= '0' && c <= '9') ? (c + 4) : \
			 (c == '+') ? 62 : (c == '/') ? 63 : 0)

/*
** ybase64_encode()
** Encode binary data to base64 form.
*/
char *ybase64_encode(ybin_t bin)
{
  unsigned char *str;
  unsigned int len = bin.len;
  const unsigned char *pt = bin.data;
  int i;

  if (!(str = malloc0(((len + 3 - len % 3) * 4 / 3 + 1))))
    return (NULL);
  for (i = 0; len > 0; len -= 3, pt += 3)
    {
      str[i++] = _b64_enc[pt[0] >> 2];
      str[i++] = _b64_enc[((pt[0] & 0x03) << 4) | pt[1] >> 4];
      str[i++] = (len == 1) ? '=' : _b64_enc[((pt[1] & 0x0F) << 2) | (pt[2] >> 6)];
      str[i++] = (len == 2) ? '=' : _b64_enc[pt[2] & 0x3F];
      if (len < 3)
	break;
    }
  str[i] = '\0';
  return ((char*)str);
}

/*
** ybase64_decode()
** Decode a base64 encoded string.
*/
ybin_t ybase64_decode(const char *pt)
{
   ybin_t res;
   unsigned char *str;
   int i, len;

   if (!(str = malloc0(strlen(pt) + 2)))
     {
	res.data = NULL;
	res.len = 0;
	return (res);
     }
   for (i = 0, len = strlen(pt); len >= 4; pt += 4, len -= 4)
     {
	str[i++] = (_B64_DEC(pt[0]) << 2) | (_B64_DEC(pt[1]) >> 4 & 0x03);
	if (pt[2] == '=')
	  break;
	str[i++] = (_B64_DEC(pt[1]) << 4) | (_B64_DEC(pt[2]) >> 2 & 0x0F);
	if (pt[3] == '=')
	  break;
	str[i++] = (_B64_DEC(pt[2]) << 6) | (_B64_DEC(pt[3]) & 0x3F);
     }
   str[i] = '\0';
   res.data = str;
   res.len = i;
   return (res);
}
