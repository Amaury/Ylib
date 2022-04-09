#include "y.h"

/* *** declaration of private functions *** */
static char _yini_get_char(yini_t *ini);
static char _yini_get_trimmed_char(yini_t *ini);
static void _yini_unget_char(yini_t *ini, char c);
static ystatus_t _yini_parse(yini_t *ini);
static ystatus_t _yini_parse_data(yini_t *ini);
static ystatus_t _yini_parse_section_name(yini_t *ini);
static void _yini_parse_comment(yini_t *ini);
static ystatus_t _yini_delete_section(size_t index, void *element, void *data);
static ystatus_t _yini_delete_element(size_t index, void *element, void *data);
static void _yini_sfprint(yini_t *ini, ystr_t *str, FILE *stream);
static void _yini_print_elements(yarray_t elements, ystr_t *str, FILE *stream);

/* INI parser main structure. */
struct yini_s {
	enum {
		YINI_FILE = 0,
		YINI_STREAM,
		YINI_STRING
	} input_type;
	union {
		FILE *stream;
		const char *str;
	} input;
	yarray_t sections;
	yarray_t orphan_elements;
	bool parse_end;
	char last_char;
	bool buffered_char;
};

/* Section structure. */
typedef struct {
	ystr_t name;
	yarray_t elements;
} yini_section_t;

/* Element structure. */
typedef struct {
	ystr_t key;
	ystr_t value;
} yini_element_t;

/* Create an INI parser object. */
yini_t *yini_new() {
	yini_t *ini = malloc0(sizeof(yini_t));
	return (ini);
}
/* Delete an INI parser. */
void *yini_free(yini_t *ini, bool cleanup) {
	if (!ini)
		return (NULL);
	if (ini->sections) {
		yarray_delete(&ini->sections, _yini_delete_section, &cleanup);
	}
	if (ini->orphan_elements) {
		yarray_delete(&ini->orphan_elements, _yini_delete_element, &cleanup);
	}
	free0(ini);
	return (NULL);
}
/* Parse a INI file. */
ystatus_t yini_parse_file(yini_t *ini, const char *filename) {
	ini->input_type = YINI_FILE;
	ini->input.stream = fopen(filename, "r");
	if (!ini->input.stream) {
		//ini->parse_end = true;
		return (YENOSTR);
	}
	return (_yini_parse(ini));
}
/* Parse a INI stream. */
ystatus_t yini_parse_stream(yini_t *ini, FILE *stream) {
	ini->input_type = YINI_STREAM;
	ini->input.stream = stream;
	if (!stream) {
		//ini->parse_end = true;
		return (YEBADFD);
	}
	return (_yini_parse(ini));
}
/* Parse a INI string. */
ystatus_t yini_parse_string(yini_t *ini, const char *str) {
	ini->input_type = YINI_STRING;
	ini->input.str = str;
	if (!str)
		return (YEFAULT);
	return (_yini_parse(ini));
}
/* Tell if a section exists. */
bool yini_section_exists(yini_t *ini, const char *section_name) {
	if (!ini->sections)
		return (false);
	size_t i, len;
	for (i= 0, len = yarray_length(ini->sections); i < len; ++i) {
		yini_section_t *section = ini->sections[i];
		if (!strcmp(section->name, section_name)) {
			return (true);
		}
	}
	return (false);
}
/* Search a value. */
ystr_t yini_search(yini_t *ini, const char *section_name, const char *key) {
	yarray_t elements = ini->orphan_elements;
	size_t i, len;

	if (section_name && ini->sections) {
		for (i = 0, len = yarray_length(ini->sections); i < len; ++i) {
			yini_section_t *section = ini->sections[i];
			if (!strcmp(section->name, section_name)) {
				elements = section->elements;
				break;
			}
		}
	}
	if (!elements)
		return (NULL);
	for (i = 0, len = yarray_length(elements); i < len; ++i) {
		yini_element_t *elem = elements[i];
		if (!strcmp(elem->key, key)) {
			return (elem->value);
		}
	}
	return (NULL);
}
/* Extract a value. */
char *yini_extract(yini_t *ini, const char *section_name, const char *key) {
	ystr_t result = yini_search(ini, section_name, key);
	return (ys_string(result));
}
/** Search a list of values. */
yarray_t yini_search_list(yini_t *ini, const char *section_name, const char *key) {
	yarray_t elements = ini->orphan_elements;
	size_t i, len;

	if (section_name && ini->sections) {
		for (i = 0, len = yarray_length(ini->sections); i < len; ++i) {
			yini_section_t *section = ini->sections[i];
			if (!strcmp(section->name, section_name)) {
				elements = section->elements;
				break;
			}
		}
	}
	if (!elements)
		return (NULL);
	yarray_t result = NULL;
	ystr_t longkey = ys_new(key);
	ys_append(&longkey, "[]");
	for (i = 0, len = yarray_length(elements); i < len; ++i) {
		yini_element_t *elem = elements[i];
		if (!strcmp(elem->key, longkey)) {
			// found a matching value
			result = result ? result : yarray_new();
			yarray_push(&result, elem->value);
		}
	}
	ys_free(longkey);
	return (result);
}
/** Extract a list of values. */
yarray_t yini_extract_list(yini_t *ini, const char *section_name, const char *key) {
	yarray_t elements = ini->orphan_elements;
	size_t i, len;

	if (section_name && ini->sections) {
		for (i = 0, len = yarray_length(ini->sections); i < len; ++i) {
			yini_section_t *section = ini->sections[i];
			if (!strcmp(section->name, section_name)) {
				elements = section->elements;
				break;
			}
		}
	}
	if (!elements)
		return (NULL);
	yarray_t result = NULL;
	ystr_t longkey = ys_new(key);
	ys_append(&longkey, "[]");
	for (i = 0, len = yarray_length(elements); i < len; ++i) {
		yini_element_t *elem = elements[i];
		if (!strcmp(elem->key, longkey)) {
			// found a matching value
			char *pt = NULL;
			if (elem->value)
				pt = ys_string(elem->value);
			result = result ? result : yarray_new();
			yarray_push(&result, pt);
		}
	}
	ys_delete(&longkey);
	return (result);
}
/* Print the content of an INI file. */
void yini_print(yini_t *ini) {
	_yini_sfprint(ini, NULL, stdout);
}
/* Print the content of an INI file to a given stream. */
void yini_fprint(yini_t *ini, FILE *stream) {
	_yini_sfprint(ini, NULL, stream);
}
/* Generate a string from a parsed INI. */
void yini_sprint(yini_t *ini, ystr_t *str) {
	_yini_sfprint(ini, str, NULL);
}

/* ********** PRIVATE FUNCTIONS ********** */
/* Delete a section. */
static ystatus_t _yini_delete_section(size_t index, void *element, void *data) {
	yini_section_t *sect = (yini_section_t*)element;
	bool *cleanup = (bool*)data;

	if (*cleanup && sect->name)
		ys_delete(&sect->name);
	if (sect->elements)
		yarray_delete(&sect->elements, _yini_delete_element, cleanup);
	return (YENOERR);
}
/* Delete an element. */
static ystatus_t _yini_delete_element(size_t index, void *element, void *data) {
	yini_element_t *elem = (yini_element_t*)element;
	bool *cleanup = (bool*)data;

	if (!*cleanup)
		return (YENOERR);
	if (elem->key)
		ys_delete(&elem->key);
	if (elem->value)
		ys_delete(&elem->value);
	return (YENOERR);
}
/* Return the next character to parse. */
static char _yini_get_char(yini_t *ini) {
	if (ini->parse_end)
		return ('\0');
	if (ini->buffered_char) {
		ini->buffered_char = false;
		return (ini->last_char);
	}
	char c = '\0';
	if (ini->input_type == YINI_STRING) {
		c = *ini->input.str;
		if (!c)
			ini->parse_end = true;
		else
			++ini->input.str;
		return (c);
	} else {
		// YINI_FILE || YINI_STREAM
		if (feof(ini->input.stream) ||
		    (c = fgetc(ini->input.stream)) == EOF) {
			ini->parse_end = true;
			if (ini->input_type == YINI_FILE) {
				fclose(ini->input.stream);
			}
		}
		return (c);
	}
}
/* Return the next charactar to parse that is not a space. */
static char _yini_get_trimmed_char(yini_t *ini) {
	char c;

	if (ini->parse_end)
		return ('\0');
	for (;;) {
		c = _yini_get_char(ini);
		if (ini->parse_end || !isspace(c))
			return (c);
	}
}
/* Put a character to be parseable again. */
static void _yini_unget_char(yini_t *ini, char c) {
	ini->buffered_char = true;
	ini->last_char = c;
}
/* Parse an INI file. */
static ystatus_t _yini_parse(yini_t *ini) {
	ystatus_t status;

	for (;;) {
		char c = _yini_get_trimmed_char(ini);
		if (ini->parse_end) {
			return (YENOERR);
		}
		if (c == ';') {
			_yini_parse_comment(ini);
		} else if (c == '[') {
			status = _yini_parse_section_name(ini);
		} else {
			_yini_unget_char(ini, c);
			status = _yini_parse_data(ini);
		}
		if (status != YENOERR)
			return (status);
	}
}
/* Parse a key-value data. */
static ystatus_t _yini_parse_data(yini_t *ini) {
	char c;
	ystr_t key, value;
	bool quoted, end_found;

	// parse the key
	{
		quoted = end_found = false;
		// check if the key is quoted
		c = _yini_get_trimmed_char(ini);
		if (ini->parse_end) {
			return (YENOERR);
		}
		if (c == '"') {
			quoted = true;
		} else {
			_yini_unget_char(ini, c);
		}
		// loop on the key
		key = ys_new("");
		for (;;) {
			c = _yini_get_char(ini);
			if (ini->parse_end) {
				ys_free(key);
				return (YESYNTAX);
			}
			if (c == '"') {
				if (!quoted) {
					ys_free(key);
					return (YESYNTAX);
				}
				c = _yini_get_trimmed_char(ini);
				if (c != '=') {
					ys_free(key);
					return (YESYNTAX);
				}
				break;
			} else if (!quoted && c == '=') {
				ys_trim(key);
				break;
			} else {
				ys_addc(&key, c);
			}
		}
	}
	// parse the value
	{
		quoted = false;
		// check if the data is quoted
		c = _yini_get_trimmed_char(ini);
		if (ini->parse_end) {
			ys_free(key);
			return (YESYNTAX);
		}
		if (c == '"')
			quoted = true;
		else
			_yini_unget_char(ini, c);
		// loop on the value
		value = ys_new("");
		for (;;) {
			c = _yini_get_char(ini);
			if (ini->parse_end)
				break;
			if (c == '\\') {
				char c2 = _yini_get_char(ini);
				if (c2 == '\n')
					continue;
				_yini_unget_char(ini, c2);
				ys_addc(&value, c);
			} else if ((quoted && c == '"') ||
			           (!quoted && c == ';') ||
			           (!quoted && c == '\n')) {
				ys_trim(value);
				break;
			} else
				ys_addc(&value, c);
		}
	}
	// add the element
	{
		yini_element_t *elem = malloc0(sizeof(yini_element_t));

		elem->key = key;
		elem->value = value;
		if (!ini->sections) {
			// add the element to the list of orphans
			if (!ini->orphan_elements)
				ini->orphan_elements = yarray_new();
			yarray_push(&ini->orphan_elements, elem);
		} else {
			// add the element to the last section
			yini_section_t *section = ini->sections[yarray_length(ini->sections) - 1];
			if (!section->elements)
				section->elements = yarray_new();
			yarray_push(&section->elements, elem);
		}
	}
	return (YENOERR);
}
/* Parse a section name. */
static ystatus_t _yini_parse_section_name(yini_t *ini) {
	// create a new section
	yini_section_t *section = malloc0(sizeof(yini_section_t));
	// create the section name's string
	section->name = ys_new("");

	// create the list of sections if it doesn't exists
	if (!ini->sections)
		ini->sections = yarray_new();
	// add the new section to the list
	yarray_push(&ini->sections, section);
	// loop
	for (;;) {
		char c = _yini_get_char(ini);
		if (ini->parse_end)
			return (YESYNTAX);
		if (c == ']') {
			break;
		}
		ys_addc(&section->name, c);
	}
	return (YENOERR);
}
/* Parse a comment. */
static void _yini_parse_comment(yini_t *ini) {
	for (;;) {
		char c = _yini_get_char(ini);
		if (ini->parse_end || c == '\n')
			return;
	}
}
/* Generate a string from a parsed INI. */
static void _yini_sfprint(yini_t *ini, ystr_t *str, FILE *stream) {
	if (!str && !stream)
		stream = stdout;
	_yini_print_elements(ini->orphan_elements, str, stream);
	if (ini->sections) {
		void **ptr;

		for (ptr = ini->sections; *ptr;
		     ptr = (void**)((size_t)ptr + sizeof(void*))) {
			yini_section_t *section = (yini_section_t*)*ptr;
			if (str) {
				ystr_t s = ys_new("");
				ys_printf(&s, "[%s]\n", section->name);
				ys_append(str, s);
				ys_delete(&s);
			} else
				fprintf(stream, "[%s]\n", section->name);
			_yini_print_elements(section->elements, str, stream);
		}
	}
}
/* Print a list of elements. */
static void _yini_print_elements(yarray_t elements, ystr_t *str, FILE *stream) {
	void **ptr;
	yini_element_t *elem;

	if (!elements)
		return;
	if (!str && !stream)
		stream = stdout;
	for (ptr = elements; *ptr;
	     ptr = (void**)((size_t)ptr + sizeof(void*))) {
		elem = (yini_element_t*)*ptr;
		if (str) {
			ystr_t s = ys_new("");
			ys_printf(&s, "\"%s\"=\"%s\"\n", elem->key, elem->value);
			ys_append(str, s);
			ys_delete(&s);
		} else
			fprintf(stream, "\"%s\" = \"%s\"\n", elem->key, elem->value);
	}
}

