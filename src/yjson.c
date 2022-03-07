#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "yjson.h"

/* Private functions */
static ystatus_t _yjson_array_print_elem(size_t index, void *data, void *user_data);
static ystatus_t _yjson_object_print_elem(char *key, void *data, void *user_data);
static void _yjson_value_print(yvar_t *value, int depth, bool linefeed);
static ystatus_t _yjson_remove_space(yjson_parser_t *json);
static yvar_t _yjson_parse_chunk(yjson_parser_t *json);
static void _yjson_parse_string(yjson_parser_t *json, yvar_t *value);
static void _yjson_parse_number(yjson_parser_t *json, yvar_t *value);
static void _yjson_parse_array(yjson_parser_t *json, yvar_t *value);
static void _yjson_parse_object(yjson_parser_t *json, yvar_t *value);

/** @define YJSON_INDENT Macro used for indentation purpose. */
#define YJSON_INDENT(depth) do { for (int _i = 0; _i < depth; ++_i) printf("\t"); } while (0)

/* Initialiser of JSON parser. */
yjson_parser_t *yjson_new(char *input) {
	yjson_parser_t *json = malloc0(sizeof(yjson_parser_t));
	if (!json)
		return (NULL);
	*json = (yjson_parser_t){
		.input = input,
		.ptr = input,
		.status = YENOERR
	};
	return (json);
}

/* Destroy a JSON parser. */
void yjson_free(yjson_parser_t *json) {
	free0(json);
}

/* Starts a JSON parser. */
yvar_t yjson_parse(yjson_parser_t *json) {
	yvar_t value = _yjson_parse_chunk(json);
	if (json->status == YENOERR && *json->ptr != '\0') {
		json->status = YESYNTAX;
		yvar_init_undef(&value);
	}
	return (value);
}

/* Prints a JSON value node and its subnodes. */
static ystatus_t _yjson_array_print_elem(size_t index, void *data, void *user_data) {
	yvar_t *val = data;
	int *p = user_data;
	int depth = p[0], len = p[1], linefeed = p[2];
	bool last = ((int)index < (len - 1)) ? false : true;
	if (linefeed)
		YJSON_INDENT(depth + 1);
	_yjson_value_print(val, depth + 1, false);
	if (!last && linefeed)
		printf(",\n");
	else if (!last)
		printf(",");
	else if (last && linefeed)
		printf("\n");
	return (YENOERR);
}
static ystatus_t _yjson_object_print_elem(char *key, void *data, void *user_data) {
	yvar_t *val = data;
	int *p = user_data;
	int depth = p[0], len = p[1], linefeed = p[2];
	if (linefeed)
		YJSON_INDENT(depth + 1);
	printf("\"%s\": ", key);
	_yjson_value_print(val, depth + 1, false);
	if (len > 1 && linefeed)
		printf(",\n");
	else if (len > 1)
		printf(",");
	else if (len <= 1 && linefeed)
		printf("\n");
	((int*)user_data)[1]--;
	return (YENOERR);
}
static void _yjson_value_print(yvar_t *value, int depth, bool linefeed) {
	if (yvar_isnull(value)) {
		printf("null");
	} else if (yvar_isbool(value)) {
		printf("%s", yvar_get_bool(value) ? "true" : "false");  
	} else if (yvar_isint(value)) {
		printf("%" PRId64, yvar_get_int(value));
	} else if (yvar_isfloat(value)) {
		printf("%g", yvar_get_float(value));
	} else if (yvar_isstring(value)) {
		printf("\"%s\"", yvar_get_string(value));
	} else if (yvar_isarray(value)) {
		printf(linefeed ? "[\n" : "[");
		yarray_t array_value = yvar_get_array(value);
		int p[3] = {
			depth,
			yarray_length(array_value),
			(linefeed ? 1 : 0),
		};
		yarray_foreach(array_value, _yjson_array_print_elem, p);
		if (linefeed)
			YJSON_INDENT(depth);
		printf("]");
	} else if (yvar_ishashmap(value)) {
		printf(linefeed ? "{\n" : "{");
		yhashmap_t *hashmap_value = yvar_get_hashmap(value);
		int p[3] = {
			depth,
			yhashmap_length(hashmap_value),
			(linefeed ? 1 : 0),
		};
		yhashmap_foreach(hashmap_value, _yjson_object_print_elem, p);
		if (linefeed)
			YJSON_INDENT(depth);
		printf("}");
	}
	if (linefeed)
		printf("\n");
}
/* Prints a JSON value node and its subnodes, with newlines and tabulations. */
void yjson_print(yvar_t *value) {
	_yjson_value_print(value, 0, true);
}
/* Prints a JSON value node and its subnodes, without newlines nor indentations. */
void yjson_print_inline(yvar_t *value) {
	_yjson_value_print(value, 0, false);
}

/* Parse */
/* Remove spaces from a JSON string. */
static ystatus_t _yjson_remove_space(yjson_parser_t *json) {
	// process spaces
	while (isspace(*json->ptr)) {
		if (*json->ptr == LF)
			++json->line;
		++json->ptr;
	}
	// process mono-line comments
	if (!strncmp(json->ptr, "//", 2)) {
		for (json->ptr += 2; *json->ptr != '\0' && *json->ptr != LF; ++json->ptr)
			;
		// remove remaining spaces
		return (_yjson_remove_space(json));
	}
	// process multi-lines comments
	if (strncmp(json->ptr, "/*", 2))
		return (json->status);
	bool found = false;
	for (json->ptr += 2; *json->ptr != '\0'; ++json->ptr) {
		if (!strncmp(json->ptr, "*/", 2)) {
			found = true;
			json->ptr += 2;
			break;
		}
	}
	if (!found) {
		json->status = YESYNTAX;
		return (json->status);
	}
	// remove remaining spaces
	return (_yjson_remove_space(json));
}
/* Parse a chunk of JSON. */
static yvar_t _yjson_parse_chunk(yjson_parser_t *json) {
	char c;
	yvar_t result;

	yvar_init_undef(&result);
	if (_yjson_remove_space(json) != YENOERR)
		goto end;
	c = *json->ptr;
	if (c == '\0')
		goto end;
	else if (c == '{') {
		// object
		json->ptr++;
		_yjson_parse_object(json, &result);
		goto end;
	} else if (c == '[') {
		// array
		json->ptr++;
		_yjson_parse_array(json, &result);
		goto end;
	} else if (c == '"') {
		// string
		json->ptr++;
		_yjson_parse_string(json, &result);
		goto end;
	} else if (!strncasecmp(json->ptr, "null", 4)) {
		// null
		json->ptr += 4;
		yvar_init_null(&result);
		goto end;
	} else if (!strncasecmp(json->ptr, "false", 5)) {
		// false
		json->ptr += 5;
		yvar_init_bool(&result, false);
		goto end;
	} else if (!strncasecmp(json->ptr, "true", 4)) {
		// true
		json->ptr += 4;
		yvar_init_bool(&result, true);
		goto end;
	} else {
		// number
		_yjson_parse_number(json, &result);
		goto end;
	}
end:
	if (json->status != YENOERR || _yjson_remove_space(json) != YENOERR)
		yvar_init_undef(&result);
	return (result);
}
/* Parse a string. */
static void _yjson_parse_string(yjson_parser_t *json, yvar_t *value) {
	char *ptr = json->ptr;
	while (*json->ptr != '\0' &&
	       (*json->ptr != '"' ||
	        (json->ptr > json->input && *(json->ptr - 1) == '\\'))) {
		if (*json->ptr == '\n')
			++json->line;
		++json->ptr;
	}
	if (*json->ptr != '"') {
		yvar_init_undef(value);
		json->status = YESYNTAX;
		return;
	}
	*json->ptr = '\0';
	++json->ptr;
	ystr_t str = ys_copy(ptr);
	if (!str) {
		yvar_init_undef(value);
		json->status = YENOMEM;
		return;
	}
	yvar_init_string(value, str);
}
/* Parse a number. */
static void _yjson_parse_number(yjson_parser_t *json, yvar_t *value) {
	/*
	 * Number are on the form:
	 * minus character (optional)
	 * one or more digits (0-9)
	 * dot character followed by one or more digits (optional)
	 * exponential part (optional)
	 *	"e|e+|e-|E|E+|E-" followed by one or more digits
	 */
	char *end;
	char *ptr = json->ptr;
	bool dot = false;

	// check syntax
	if (*ptr == '-')
		++ptr;
	if (!isdigit(*ptr)) {
		json->status = YESYNTAX;
		return;
	}
	while (isdigit(*ptr))
		++ptr;
	if (*ptr == '.') {
		dot = true;
		++ptr;
		if (!isdigit(*ptr)) {
			json->status = YESYNTAX;
			return;
		}
	}
	// conversion
	if (dot) {
		double float_value = strtold(json->ptr, &end);
		yvar_init_float(value, float_value);
	} else {
		int64_t int_value = (int64_t)strtoll(json->ptr, &end, 10);
		yvar_init_int(value, int_value);
	}
	json->ptr = end;
}
/* Parse an array. */
static void _yjson_parse_array(yjson_parser_t *json, yvar_t *value) {
	if (_yjson_remove_space(json) != YENOERR)
		return;
	yarray_t array = yarray_new();
	while (*json->ptr != '\0') {
		// search for end of list
		if (*json->ptr == RBRACKET) {
			++json->ptr;
			yvar_init_array(value, array);
			return;
		}
		// get value
		yvar_t val = _yjson_parse_chunk(json);
		if (json->status != YENOERR)
			goto error;
		yvar_t *pval = yvar_clone(&val);
		if (!pval) {
			json->status = YENOMEM;
			goto error;
		}
		// add the value to the array
		yarray_push(&array, pval);
		// skip spaces
		if (_yjson_remove_space(json) != YENOERR)
			goto error;
		// process the rest
		if (*json->ptr == RBRACKET) {
			continue;
		} else if (*json->ptr != COMMA) {
			json->status = YESYNTAX;
			goto error;
		}
		++json->ptr;
		if (_yjson_remove_space(json) != YENOERR)
			goto error;
	}
	// end of string
	json->status = YESYNTAX;
error:
	yarray_free(array);
}
/* Parse an object. */
static void _yjson_parse_object(yjson_parser_t *json, yvar_t *value) {
	ystr_t key = NULL;
	if (_yjson_remove_space(json) != YENOERR)
		return;
	yhashmap_t *map = yhashmap_create(10, NULL, json);
	while (*json->ptr != '\0') {
		// search for end of object
		if (*json->ptr == RBRACE) {
			++json->ptr;
			yvar_init_hashmap(value, map);
			return;
		}
		// get key
		yvar_t val_key = _yjson_parse_chunk(json);
		if (json->status != YENOERR)
			goto error;
		if (!yvar_isstring(&val_key)) {
			json->status = YESYNTAX;
			goto error;
		}
		key = yvar_get_string(&val_key);
		if (_yjson_remove_space(json) != YENOERR)
			goto error;
		// search colon character
		if (*json->ptr != COLON) {
			json->status = YESYNTAX;
			goto error;
		}
		++json->ptr;
		// get value
		yvar_t val = _yjson_parse_chunk(json);
		if (json->status != YENOERR)
			goto error;
		yvar_t *pval = yvar_clone(&val);
		if (!pval) {
			json->status = YENOMEM;
			goto error;
		}
		// add to hashmap
		yhashmap_add(map, key, pval);
		// process the rest
		if (*json->ptr == RBRACE) {
			continue;
		} else if (*json->ptr != COMMA) {
			json->status = YESYNTAX;
			goto error;
		}
		++json->ptr;
		if (_yjson_remove_space(json) != YENOERR)
			goto error;
	}
error:
	ys_free(key);
	yhashmap_delete(map);
}
