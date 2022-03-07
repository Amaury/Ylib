/**
 * @header	yinit.h
 * @abstract	All definitions about INI file parser.
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

/** @typedef INI parser main structure. */
typedef struct yini_s yini_t;

/**
 * Create a new INI parser.
 * @return	A pointer to the allocated INI parser.
 */
yini_t *yini_new(void);
/**
 * Delete a previously allocated INI parser.
 * @param	ini	Pointer to a pointer to the INI parser.
 * @param	cleanup	True if all allocated strings must be freed.
 * @return	Always NULL.
 */
void *yini_free(yini_t *ini, bool cleanup);
/**
 * Parse an INI file.
 * @param	ini		Pointer to the INI parser.
 * @param	filename	Path to the file to parse.
 * @return	YENOERR if OK.
 */
ystatus_t yini_parse_file(yini_t *ini, const char *filename);
/**
 * Parse an INI stream.
 * @param	ini	Pointer to the INI parser.
 * @param	stream	Pointer to the stream to parse.
 * @return	YENOERR if OK.
 */
ystatus_t yini_parse_stream(yini_t *ini, FILE *stream);
/**
 * Parse an INI string.
 * @param	ini	Pointer to the INI parser.
 * @param	str	String to parse.
 * @return	YENOERR if OK.
 */
ystatus_t yini_parse_string(yini_t *ini, const char *str);
/**
 * Tell if a section exists.
 * @param	ini		Pointer to the INI parser.
 * @param	section_name	Name of the searched section.
 * @return	True if the section exists.
 */
bool yini_section_exists(yini_t *ini, const char *section_name);
/**
 * Search for an element.
 * @param	ini		Pointer to the INI parser.
 * @param	section_name	Name of the section, or NULL for global search.
 * @param	key		Key to search for.
 * @return	The value, or NULL if not found. This ystring must not be freed.
 */
ystr_t yini_search(yini_t *ini, const char *section_name, const char *key);
/**
 * Search for an element. The data is copied.
 * @param	ini		Pointer to the INI parser.
 * @param	section_name	Name of the section, or NULL for global search.
 * @param	key		Key to search for.
 * @return	A copy of the value, or NULL if not found.
 */
char *yini_extract(yini_t *ini, const char *section_name, const char *key);
/**
 * Search for a list of elements with the same name (with a "[]" suffix).
 * @param	ini		Pointer to the INI parser.
 * @param	section_name	Name of the section, or NULL for global search.
 * @param	key		Key to search for (without the "[]" suffix).
 * @return	A list of values, or NULL if not found. The list must be freed,
 *		but not its values.
 */
yarray_t yini_search_list(yini_t *ini, const char *section_name, const char *key);
/**
 * Search for a list of elements with the same name (with a "[]" suffix).
 * The data are copied.
 * @param	ini		Pointer to the INI parser.
 * @param	section_name	Name of the section, or NULL for global search.
 * @param	key		Key to search for (without the "[]" suffix).
 * @return	A list of values, or NULL if not found. The list and its content
 *		could be freed.
 */
yarray_t yini_extract_list(yini_t *ini, const char *section_name, const char *key);
/**
 * Print the content of a parsed INI to stdout.
 * @param	ini	Pointer to the INI parser.
 */
void yini_print(yini_t *ini);
/**
 * Print the content of a parsed INI to the given stream.
 * @param	ini	Pointer to the INI parser.
 * @param	stream	Pointer to the stream.
 */
void yini_fprint(yini_t *ini, FILE *stream);
/**
 * Generate a string from a parsed INI.
 * @param	ini	Pointer to the INI parser.
 * @param	str	Pointer to a ystring.
 */
void yini_sprint(yini_t *ini, ystr_t *str);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

