/**
 * @header	ytable.h
 * @abstract	Table used for array/hashmap/hashtables features.
 * @version	1.0.0, March 22 2022
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#include "y.h"

/** @typedef ytable_t	Opaque pointer to ytable structure. */
typedef void *ytable_t;
/** @typedef ytable_function_t	Function pointer. */
typedef ystatus_t (*ytable_function_t)(yvar_t key, void *data, void *user_data);

/**
 * @function	ytable_new
 *		Create a new ytable, initialized to the default size.
 * @return	A pointer to the allocated ytable.
 */
ytable_t *ytable_new(void);
/**
 * @function	ytable_create
 *		Create a new ytable, initialized to the given size.
 * @param	size		Table size. If set to zero, the default size will be used.
 * @param	delete_function	Pointer to a function used to delete elements. Could be NULL.
 * @param	delete_data	Pointer to some data given to the delete function. Could be NULL.
 * @return	A pointer to the allocated ytable.
 */
ytable_t *ytable_create(size_t size, ytable_function_t delete_function, void *delete_data);
/**
 * @function	ytable_set_delete_function
 *		Define the delete function of a ytable.
 * @param	table		Pointer to the ytable.
 * @param	delete_function	Pointer to the function used to delete elements. Could be NULL.
 * @param	delete_data	Pointer to some data given to the delete function. Could be NULL.
 * @return	A pointer to the given ytable.
 */
ytable_t *ytable_set_delete_function(ytable_t *table, ytable_function_t delete_function, void *delete_data);
