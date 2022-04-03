#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "y.h"

/* Minimal size of ystrings. */
#define YSTR_MINIMAL_SIZE	8
/** @define _YARRAY_HEAD Get a pointer to a yarray's header. */
#define _YSTR_HEAD(p)  ((ystr_head_t*)((void*)(p) - sizeof(ystr_head_t)))

/* Create a new ystring.  */
ystr_t ys_new(const char *s) {
	char *res;
	size_t strsz, totalsz;
	ystr_head_t *y;

	strsz = (!s || !s[0]) ? 0 : strlen(s);
	if (strsz < YSTR_MINIMAL_SIZE)
		totalsz = YSTR_MINIMAL_SIZE;
	else {
		totalsz = (((strsz / YSTR_MINIMAL_SIZE) + 1) * YSTR_MINIMAL_SIZE) + 1;
	}
	res = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	if (!res)
		return (NULL);
	y = (ystr_head_t*)res;
	res += sizeof(ystr_head_t);
	y->total = totalsz;
	y->used = strsz;
	if (!strsz)
		*res = '\0';
	else
		memcpy(res, s, strsz + 1);
	return ((ystr_t)res);
}
/* Create a minimal ystring that contains a copy of the given string. */
ystr_t ys_copy(const char *s) {
	char *res;
	size_t strsz, totalsz;
	ystr_head_t *y;

	strsz = (!s) ? 0 : strlen(s);
	totalsz = strsz + 1;
	res = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	y = (ystr_head_t*)res;
	res += sizeof(ystr_head_t);
	y->total = totalsz;
	y->used = strsz;
	if (!strsz)
		*res = '\0';
	else
		memcpy(res, s, strsz + 1);
	return ((ystr_t)res);
}
/* Delete an existing ystring. */
void ys_delete(ystr_t *s) {
	ystr_head_t *y;

	if (!s || !*s)
		return;
	y = _YSTR_HEAD(*s);
	free0(y);
	*s = NULL;
}
/* Delete an existing ystring. */
void ys_free(ystr_t s) {
	ystr_head_t *y;

	if (!s)
		return;
	y = _YSTR_HEAD(s);
	free0(y);
}
/* Truncate an existing ystring. The allocated memory size doesn't change. */
void ys_trunc(ystr_t s) {
	ystr_head_t *y;

	if (!s)
		return;
	y = _YSTR_HEAD(s);
	*s = '\0';
	y->used = 0;
}
/* Set the minimum size of a ystring. */
ystatus_t ys_resize(ystr_t *s, size_t sz) {
	ystr_head_t *y, *ny;
	size_t totalsz;
	char *ns;

	if (!s || !*s)
		return (YENOERR);
	y = _YSTR_HEAD(*s);
	if (sz <= y->total)
		return (YENOERR);
	totalsz = (((sz / YSTR_MINIMAL_SIZE) + 1) * YSTR_MINIMAL_SIZE) + 1;
	ns = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	if (!ns)
		return (YENOMEM);
	ny = (ystr_head_t*)ns;
	ns += sizeof(ystr_head_t);
	ny->total = totalsz;
	ny->used = y->used;
	memcpy(ns, s, y->used + 1);
	free0(y);
	*s = ns;
	return (YENOERR);
}
/* Return true if a ystring is empty or NULL. */
bool ys_empty(const ystr_t s) {
	char *pt = (char*)s;
	if (!s || !*pt)
		return (true);
	return (false);
}
/* Return the length of an ystring in bytes. */
size_t ys_bytesize(const ystr_t s) {
	ystr_head_t *y;

	if (!s)
		return (0);
	y = _YSTR_HEAD(s);
	return (y->used);
}
/* Concatenate a character string at the end of an ystring. */
ystatus_t ys_append(ystr_t *dest, const char *src) {
	size_t srcsz, strsz, totalsz;
	ystr_head_t *y, *ny;
	char *ns;

	if (!src || !(srcsz = strlen(src)))
		return (YENOERR);
	if (!dest)
		return (YEINVAL);
	if (!*dest) {
		if (!(*dest = ys_new(src)))
			return (YENOMEM);
		return (YENOERR);
	}
	y = _YSTR_HEAD(*dest);
	if ((y->used + 1 + srcsz) <= y->total) {
		memcpy(*dest + y->used, src, srcsz + 1);
		y->used += srcsz;
		return (YENOERR);
	}
	strsz = y->used + srcsz;
	totalsz = (y->total > YSTR_MINIMAL_SIZE) ? y->total : YSTR_MINIMAL_SIZE;
	while (totalsz < (strsz + 1))
		totalsz *= 2;
	ns = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	if (!ns)
		return (YENOMEM);
	ny = (ystr_head_t*)ns;
	ns += sizeof(ystr_head_t);
	ny->total = totalsz;
	ny->used = strsz;
	memcpy(ns, *dest, y->used);
	memcpy(ns + y->used, src, srcsz + 1);
	free0(y);
	*dest = ns;
	return (YENOERR);
}
/* Concatenate a character string at the begining of an ystring. */
ystatus_t ys_prepend(ystr_t *dest, const char *src) {
	size_t srcsz, strsz, totalsz;
	ystr_head_t *y, *ny;
	char *ns;

	if (!src || !(srcsz = strlen(src)))
		return (YENOERR);
	if (!dest)
		return (YEINVAL);
	if (!*dest) {
		if (!(*dest = ys_new(src)))
			return (YENOMEM);
		return (YENOERR);
	}
	y = _YSTR_HEAD(*dest);
	if ((y->used + 1 + srcsz) <= y->total) {
		char *pt1, *pt2;
		for (pt1 = *dest + y->used, pt2 = pt1 + srcsz;
		     pt1 >= *dest; --pt1, --pt2)
			*pt2 = *pt1;
		memcpy(*dest, src, srcsz);
		y->used += srcsz;
		return (YENOERR);
	}
	strsz = y->used + srcsz;
	totalsz = (y->total > YSTR_MINIMAL_SIZE) ? y->total : YSTR_MINIMAL_SIZE;
	while (totalsz < (strsz + 1))
		totalsz *= 2;
	ns = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	if (!ns)
		return (YENOMEM);
	ny = (ystr_head_t*)ns;
	ns += sizeof(ystr_head_t);
	ny->total = totalsz;
	ny->used = strsz;
	memcpy(ns, src, srcsz);
	memcpy(ns + srcsz, *dest, y->used + 1);
	free0(y);
	*dest = ns;
	return (YENOERR);
}
/* See strncat(). */
ystatus_t ys_nappend(ystr_t *dest, const char *src, size_t n) {
	size_t strsz, totalsz;
	ystr_head_t *y, *ny;
	char *ns;

	if (!src || !n)
		return (YENOERR);
	if (!dest)
		return (YEINVAL);
	if (!*dest) {
		if (!(*dest = ys_new(src)))
			return (YENOMEM);
		return (YENOERR);
	}
	y = _YSTR_HEAD(*dest);
	if ((y->used + 1 + n) <= y->total) {
		strncpy(*dest + y->used, src, n);
		y->used += n;
		(*dest)[y->used] = '\0';
		return (YENOERR);
	}
	strsz = y->used + n;
	totalsz = (y->total > YSTR_MINIMAL_SIZE) ? y->total : YSTR_MINIMAL_SIZE;
	while (totalsz < (strsz + 1))
		totalsz *= 2;
	ns = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	if (!ns)
		return (YENOMEM);
	ny = (ystr_head_t*)ns;
	ns += sizeof(ystr_head_t);
	ny->total = totalsz;
	ny->used = strsz;
	strcpy(ns, *dest);
	strncpy(ns + y->used, src, n);
	ns[ny->used] = '\0';
	free0(y);
	*dest = ns;
	return (YENOERR);
}
/* Same as ystr_prepend() but at the begining of a ystring. */
ystatus_t ys_nprepend(ystr_t *dest, const char *src, size_t n) {
	size_t strsz, totalsz;
	ystr_head_t *y, *ny;
	char *ns;

	if (!src || !*src || !n)
		return (YENOERR);
	if (!dest)
		return (YEINVAL);
	if (!*dest) {
		if (!(*dest = ys_new(src)))
			return (YENOMEM);
		return (YENOERR);
	}
	n = (strlen(src) < n) ? strlen(src) : n;
	y = _YSTR_HEAD(*dest);
	if ((y->used + 1 + n) <= y->total) {
		char *pt1, *pt2;
		for (pt1 = *dest + y->used, pt2 = pt1 + n;
		     pt1 >= *dest; --pt1, --pt2)
			*pt2 = *pt1;
		memcpy(*dest, src, n);
		y->used += n;
		return (YENOERR);
	}
	strsz = y->used + n;
	totalsz = (y->total > YSTR_MINIMAL_SIZE) ? y->total : YSTR_MINIMAL_SIZE;
	while (totalsz < (strsz + 1))
		totalsz *= 2;
	ns = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	if (!ns)
		return (YENOMEM);
	ny = (ystr_head_t*)ns;
	ns += sizeof(ystr_head_t);
	ny->total = totalsz;
	ny->used = strsz;
	memcpy(ns, src, n);
	memcpy(ns + n, *dest, y->used + 1);
	free0(y);
	*dest = ns;
	return (YENOERR);
}
/* Duplicate an ystring. */
ystr_t ys_dup(const ystr_t s) {
	ystr_head_t *y, *ny;
	char *ns;

	if (!s)
		return (ys_new(""));
	y = _YSTR_HEAD(s);
	ns = (char*)malloc0(y->total + sizeof(ystr_head_t));
	if (!ns)
		return (NULL);
	ny = (ystr_head_t*)ns;
	ns += sizeof(ystr_head_t);
	ny->total = y->total;
	ny->used = y->used;
	memcpy(ns, s, y->used);
	ns[y->used] = '\0';
	return ((ystr_t)ns);
}
/* Create a copy of a ystring. The copy is a simple (char*) string, not bufferized. */
char *ys_string(const ystr_t s) {
	ystr_head_t *y;
	char *res;

	if (!s)
		return (NULL);
	y = _YSTR_HEAD(s);
	res = (char*)malloc0(y->used + 1);
	if (!res)
		return (NULL);
	return (memcpy(res, s, y->used + 1));
}
/* Create a new ystring that is the concatenation of 2 ystrings. */
ystr_t ys_merge(const char *s1, const char *s2) {
	ystr_t ns;

	if (s1 && *s1 && (!s2 || !*s2))
		return (ys_new(s1));
	if ((!s1 || !*s1) && s2 && *s2)
		return (ys_new(s2));
	if ((!s1 || !*s1) && (!s2 || !*s2))
		return (ys_new(""));
	ns = ys_new(s1);
	if (!ns)
		return (NULL);
	if (ys_append(&ns, s2) != YENOERR)
		return (NULL);
	return (ns);
}
/* Remove all spaces at the beginning of an ystring. */
void ys_ltrim(ystr_t s) {
	ystr_head_t *y;
	char *pt;

	if (!s || !*s)
		return;
	y = _YSTR_HEAD(s);
	for (pt = s; isspace(*pt); ++pt, y->used--)
		;
	if (pt == s)
		return;
	for (; *pt; ++pt, ++s)
		*s = *pt;
	*s = '\0';
}
/* Remove all spaces at the end of an ystring. */
void ys_rtrim(ystr_t s) {
	ystr_head_t *y;
	char *pt;
	size_t initsz;

	if (!s || !*s)
		return;
	y = _YSTR_HEAD(s);
	initsz = y->used;
	for (pt = s + y->used - 1; isspace(*pt); --pt) {
		if (pt == s) {
			*pt = '\0';
			y->used = 0;
			return;
		}
		y->used--;
	}
	if (initsz != y->used)
		*(pt + 1) = '\0';
}
/* Remove all spaces at the beginning and the end of an ystring. */
void ys_trim(ystr_t s) {
	ys_ltrim(s);
	ys_rtrim(s);
}
/* Remove the first character of a ystring and return it. */
char ys_lshift(ystr_t s) {
	char c;

	if (!s || !*s)
		return ('\0');
	c = *s;
	*s = ' ';
	ys_ltrim(s);
	return (c);
}
/* Remove the last character of a ystring and return it. */
char ys_rshift(ystr_t s) {
	ystr_head_t *y;
	char c;

	if (!s || !*s)
		return ('\0');
	y = (ystr_head_t*)(s - sizeof(ystr_head_t));
	c = *(s + y->used - 1);
	*(s + y->used - 1) = '\0';
	y->used--;
	return (c);
}
/* Add a character at the beginning of a ystring. */
void ys_putc(ystr_t *s, char c) {
	ystr_head_t *y, *ny;
	char *pt1, *pt2, *ns;
	size_t totalsz;

	if (c == '\0')
		return;
	if (!*s) {
		char tc[2] = {'\0', '\0'};
		tc[0] = c;
		*s = ys_new(tc);
		return;
	}
	y = (ystr_head_t*)(*s - sizeof(ystr_head_t));
	if (y->total >= (y->used + 2)) {
		for (pt1 = *s + y->used, pt2 = pt1 + 1; pt1 >= *s; --pt1, --pt2)
			*pt2 = *pt1;
		**s = c;
		y->used++;
		return;
	}
	totalsz = (y->used * 2) + 1;
	ns = (char*)malloc0(totalsz + sizeof(ystr_head_t));
	ny = (ystr_head_t*)ns;
	ns += sizeof(ystr_head_t);
	ny->total = totalsz;
	ny->used = y->used + 1;
	*ns = c;
	memcpy(ns + 1, *s, y->used + 1);
	free0(y);
	*s = ns;
}
/* Add a character at the end of a ystring. */
void ys_addc(ystr_t *s, char c) {
	char tc[2] = {'\0', '\0'};

	tc[0] = c;
	ys_append(s, tc);
}
/* Convert all characters of a character string to upper case. */
void ys_upcase(char *s) {
	if (!s)
		return ;
	for (; *s; ++s) {
		if (*s >= 'a' && *s <= 'z')
			*s = 'A' + (*s - 'a');
	}
}
/* Convert all characters of a character string to lower case. */
void ys_lowcase(char *s) {
	if (!s)
		return ;
	for (; *s; ++s) {
		if (*s >= 'A' && *s <= 'Z')
			*s = 'a' + (*s - 'A');
	}
}
/*
 * Write inside a ystring using formatted arguments. The
 * ystring must be long enough (use ys_setsz() before),
 * otherwise the resulting string will be truncate.
 */
ystr_t ys_printf(ystr_t *s, char *format, ...) {
	va_list p_list;
	ystr_head_t *y;

	if (s && *s) {
		y = (ystr_head_t*)(*s - sizeof(ystr_head_t));
		free0(y);
	}
	int size;
	char *pt;
	if (format == NULL) {
		size = asprintf(&pt, "%0*d%*c", (int)sizeof(ystr_head_t), 0,
		                YSTR_MINIMAL_SIZE, '\0');
		if (size == -1) {
			ythrow("Not enough memory.", YENOMEM);
			return (NULL);
		}
	} else {
		char *yformat = NULL;
		if (asprintf(&yformat, "%0*d%s", (int)sizeof(ystr_head_t), 0, format) == -1) {
			ythrow("Not enough memory.", YENOMEM);
			return (NULL);
		}
		va_start(p_list, format);
		size = vasprintf(&pt, yformat, p_list);
		free0(yformat);
		va_end(p_list);
		if (size == -1) {
			ythrow("Not enough memory.", YENOMEM);
			return (NULL);
		}
	}
	y = (ystr_head_t*)pt;
	y->used = size - sizeof(ystr_head_t);
	y->total = y->used + 1;
	pt += sizeof(ystr_head_t);
	if (s) {
		*s = (ystr_t)pt;
	}
	return ((ystr_t)pt);
}
/* Same as ys_printf(), but the variable arguments are given trough a va_list. */
ystr_t ys_vprintf(ystr_t *s, char *format, va_list args) {
	ystr_head_t *y;

	if (s && *s) {
		y = (ystr_head_t*)(*s - sizeof(ystr_head_t));
		free0(y);
	}
	int size;
	char *pt;
	if (format == NULL) {
		size = asprintf(&pt, "%0*d%*c", (int)sizeof(ystr_head_t), 0,
		                YSTR_MINIMAL_SIZE, '\0');
		if (size == -1) {
			ythrow("Not enough memory.", YENOMEM);
			return (NULL);
		}
	} else {
		char *yformat = NULL;
		if (asprintf(&yformat, "%0*d%s", (int)sizeof(ystr_head_t), 0, format) == -1) {
			ythrow("Not enough memory.", YENOMEM);
			return (NULL);
		}
		size = vasprintf(&pt, yformat, args);
		free0(yformat);
		if (size == -1) {
			ythrow("Not enough memory.", YENOMEM);
			return (NULL);
		}
	}
	y = (ystr_head_t*)pt;
	y->used = size - sizeof(ystr_head_t);
	y->total = y->used + 1;
	pt += sizeof(ystr_head_t);
	if (s) {
		*s = (ystr_t)pt;
	}
	return ((ystr_t)pt);
}
/* Convert a character string in an hexadecimal ystring. */
ystr_t ys_str2hexa(char *str) {
	char h[3] = {'\0', '\0', '\0'}, *pt;
	ystr_t ys;

	if (!str || !(ys = ys_new("")))
		return (NULL);
	for (pt = str; *pt; ++pt) {
		snprintf(h, 3, "%x", *pt);
		ys_append(&ys, h);
	}
	return (ys);
}
/* Substitute a string by another, inside a character string. */
ystr_t ys_subs(const char *orig, const char *from, const char *to) {
	ystr_t ys;
	const char *pt;
	unsigned int from_len;

	if (!orig || !(ys = ys_new("")))
		return (NULL);
	from_len = (from) ? strlen(from) : 0;
	for (pt = orig; *pt; ++pt) {
		if (from_len && !strncmp(from, pt, from_len)) {
			ys_append(&ys, to);
			pt = pt + from_len - 1;
		} else
			ys_addc(&ys, *pt);
	}
	return (ys);
}
/* Substitute a string by another in a case-insensitive manner. */
ystr_t ys_casesubs(const char *orig, const char *from, const char *to) {
	ystr_t ys;
	const char *pt;
	unsigned int from_len;

	if (!orig || !(ys = ys_new("")))
		return (NULL);
	from_len = (from) ? strlen(from) : 0;
	for (pt = orig; *pt; ++pt) {
		if (from_len && !strncasecmp(from, pt, from_len)) {
			ys_append(&ys, to);
			pt = pt + from_len - 1;
		} else
			ys_addc(&ys, *pt);
	}
	return (ys);
}
/* Read a line of text from a stream and trim it. */
ystatus_t ys_gets(ystr_t *s, FILE *stream) {
	ystr_head_t *y = (ystr_head_t*)(*s - sizeof(ystr_head_t));

	if (!fgets(*s, y->total, stream)) {
		**s = '\0';
		y->used = 0;
		return (YEIO);
	}
	y->used = strlen(*s);
	ys_trim(*s);
	return (YENOERR);
}
/* Tell if a string only contains digits. */
bool ys_is_numeric(const char *s) {
	if (!s)
		return (false);
	for (const char *pt = s; *pt; ++pt) {
		if (!isdigit(*pt))
			return (false);
	}
	return (true);
}
/*
 * Convert a character string in another one where each XML special
 * characters are replaced by their corresponding XML entities.
 */
char *str2xmlentity(char *str) {
	char *pt, *result;
	ystr_t res;

	if (!str || !(res = ys_new("")))
		return (NULL);
	for (pt = str; *pt; ++pt) {
		if (*pt == LT)
			ys_append(&res, "&lt;");
		else if (*pt == GT)
			ys_append(&res, "&gt;");
		else if (*pt == DQUOTE)
			ys_append(&res, "&quot;");
		else if (*pt == QUOTE)
			ys_append(&res, "&apos;");
		else if (*pt == AMP)
			ys_append(&res, "&amp;");
		else
			ys_addc(&res, *pt);
	}
	result = ys_string(res);
	ys_free(res);
	return (result);
}
/*
 * Convert a string in another one where XML entities are replaced
 * by their XML special characters.
 */
char *xmlentity2str(char *str) {
	ystr_t res;
	char *pt, *result, *pt2;
	int i;
  
	if (!str || !(res = ys_new("")))
		return (NULL);
	for (pt = str; *pt; pt += i) {
		if (*pt != AMP) {
			ys_addc(&res, *pt);
			i = 1;
		} else if (!strncmp(pt, "&amp;", (i = strlen("&amp;"))))
			ys_addc(&res, AMP);
		else if (!strncmp(pt, "&lt;", (i = strlen("&lt;"))))
			ys_addc(&res, LT);
		else if (!strncmp(pt, "&gt;", (i = strlen("&gt;"))))
			ys_addc(&res, GT);
		else if (!strncmp(pt, "&quot;", (i = strlen("&quot;"))))
			ys_addc(&res, DQUOTE);
		else if (!strncmp(pt, "&apos;", (i = strlen("&apos;"))))
			ys_addc(&res, QUOTE);
		else if (*(pt + 1) == SHARP && (pt2 = strchr(pt + 2, ';'))) {
			ys_addc(&res, atoi(pt + 2));
			i = (pt2 - pt) + 1;
		} else {
			ys_addc(&res, *pt);
			i = 1;
		}
	}
	result = ys_string(res);
	ys_free(res);
	return (result);
}
/* Call the standard strcmp() function, checking first if parameters are not NULL. */
int strcmp0(const char *s1, const char *s2) {
	if (!s1 && !s2)
		return (0);
	if (!s1)
		return (-s2[0]);
	if (!s2)
		return (s1[0]);
	return (strcmp(s1, s2));
}
/* Call the standard strncmp() function, checking first if parameters are not NULL. */
int strncmp0(const char *s1, const char *s2, size_t n) {
	if (!s1 && !s2)
		return (0);
	if (!s1)
		return (-s2[0]);
	if (!s2)
		return (s1[0]);
	return (strncmp(s1, s2, n));
}

