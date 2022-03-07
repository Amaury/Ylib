/**
 * @header	yjson.h
 * @abstrat	JSON manipulation.
 * @version	1.0.0 Feb 24 2022
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "ystatus.h"
/**
 * @typedef	yjson_parser_t
 * 		JSON parser structure.
 * @field	input	Pointer to the input string.
 * @field	ptr	Pointer to the currently parsed character.
 * @field	line	Number of the currently parsed line.
 * @field	status	Parsing status.
 */
typedef struct {
	char *input;
	char *ptr;
	unsigned int line;
	ystatus_t status;
} yjson_parser_t;

#include <stdbool.h>
#include "y.h"

/**
 * @function	yjson_new
 *		Create a new JSON parser.
 * @param	json	Pointer to JSON parser object.
 * @param	input	Pointer to the string to parse.
 * @return	A pointer to the initialised JSON parser object.
 */
yjson_parser_t *yjson_new(char *input);

/**
 * @function	yjson_free
 *		Destroy a JSON parser.
 * @param	json	Pointer to a JSON parser object.
 */
void yjson_free(yjson_parser_t *json);

/**
 * @function	yjson_parse
 *		Starts a JSON parser.
 * @param	json	Pointer to the JSON parser object.
 * @return	The root node value.
 */
yvar_t yjson_parse(yjson_parser_t *json);

/**
 * @function	yjson_print
 *		Prints a JSON value node and its subnodes, with newlines and tabulations.
 * @param	value	Pointer to the value node.
 */
void yjson_print(yvar_t *value);
/**
 * @function	yjson_print_inline
 *		Prints a JSON value node and its subnodes, without newlines nor indentations.
 * @param	value	Pointer to the value node.
 */
void yjson_print_inline(yvar_t *value);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */
