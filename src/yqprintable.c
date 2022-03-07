#include <stdio.h>
#include <string.h>
#include "ylog.h"
#include "yqprintable.h"

/*
 * yqprintable_encode()
 * Encode some binary data to quoted-printable string.
 */
ystr_t yqprintable_encode(const ybin_t bin) {
	ystr_t str, para;
	unsigned char *pt, tmp[4];
	size_t i;

	if (!bin.data || !bin.bytesize)
		return (NULL);
	if (!(str = ys_new("")))
		return (NULL);
	if (!(para = ys_new(""))) {
		ys_free(str);
		return (NULL);
	}
	pt = bin.data;
	for (i = 0; i < bin.bytesize; i++) {
		if (pt[i] == CR && pt[i + 1] == LF) {
			i += 2;
			ys_append(&str, para);
			ys_append(&str, (char*)tmp);
			ys_trunc(para);
			continue;
		}
		if (pt[i] < 32 || pt[i] == EQ || pt[i] > 126)
			sprintf((char*)tmp, "%c%2X", EQ, pt[i]);
		else
			sprintf((char*)tmp, "%c", pt[i]);
		if ((ys_bytesize(para) + strlen((char*)tmp)) >= 76) {
			ys_append(&str, para);
			ys_append(&str, "=\r\n");
			ys_trunc(para);
		}
		ys_append(&para, (char*)tmp);
	}
	ys_append(&str, para);
	ys_trim(str);
	return (str);
}

/*
 * yqprintable_decode()
 * Decode a quoted-printable encoded string.
 */
ybin_t yqprintable_decode(const char *pt) {
	ybin_t res = {0};
	unsigned char *str;
	int i, j;

	YLOG_ADD(YLOG_DEBUG, "yqprintable_decode entering");
	if (!(str = malloc0(strlen(pt) + 2))) {
		return (res);
	}
	i = j = 0;
	while (pt[i]) {
		if (pt[i] == EQ) {
			if (pt[i + 1] && pt[i + 2] && isxdigit(pt[i + 1]) && isxdigit(pt[i + 2])) {
				unsigned int c;
				sscanf(pt + i + 1, "%02x", &c);
				str[j++] = (char)c;
				i += 3;
			} else {
				int k = 1;
				while (pt[i + k] && (pt[i + k] == SPACE || pt[i + k] == TAB))
					k++;
				if (!pt[i + k])
					i += k;
				else if (pt[i + k] == CR && pt[i + k + 1] == LF)
					i += k + 2;
				else if (pt[i + k] == CR || pt[i + k] == LF)
					i += k + 1;
			}
		} else
			str[j++] = pt[i++];
	}
	str[j] = '\0';
	ybin_set(&res, (void*)str, j);
	return (res);
}
