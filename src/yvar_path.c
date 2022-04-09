#include <y.h>

/* Return a value from a JSON root element and a path (similar to XPath). */
yvar_t *yvar_get_from_path(yvar_t *root, const char *path) {
	const char *pt = NULL;
	yvar_t *result = root;
	ystr_t s = NULL;

	if (!root || !path || !strlen(path))
		return (root);
	// loop
	for (pt = path; *pt; ++pt) {
		if (isspace(*pt))
			continue;
		if (*pt == SLASH) {
			++pt;
			s = ys_new("");
			for (bool loop = true; loop && *pt; ++pt) {
				if (*pt == SLASH || *pt == LBRACKET) {
					loop = false;
					break;
				}
				ys_addc(&s, *pt);
			}
			--pt;
			if (ys_bytesize(s)) {
				if (!yvar_is_table(result) ||
				    ytable_is_array(result->table_value))
					goto error;
				result = ytable_get_key_data(result->table_value, s);
				if (!result)
					goto error;
			}
		} else if (*pt == LBRACKET) {
			if (!yvar_is_array(result))
				goto error;
			// get the content of the expression
			s = ys_new("");
			for (++pt; *pt; ++pt) {
				if (*pt == RBRACKET)
					break;
				else if (*pt == LBRACKET)
					goto error;
				else
					ys_addc(&s, *pt);
			}
			if (!*pt)
				goto error;
			ys_trim(s);
			// empty expression
			if (!ys_bytesize(s))
				continue;
			// numerical expression: get the nth element of a list
			if (ys_is_numeric(s)) {
				size_t index = strtoll(s, NULL, 10);
				result = ytable_get_index_data(result->table_value, index);
				continue;
			}
			goto error;
		} else
			goto error;
		ys_delete(&s);
	}
	return (result);
error:
	ys_free(s);
	return (NULL);
}

