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
typedef ystatus_t (*ytable_function_t)(uint64_t hash, char *key, void *data, void *user_data);

/* ********** CREATION / DELETION ********** */
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
ytable_t *ytable_set_delete_function(ytable_t *table, ytable_function_t delete_function,
                                     void *delete_data);
/**
 * @function	ytable_free
 *		Destroy a ytable.
 * @param	table	Pointer to the ytable.
 */
void ytable_free(ytable_t *table);

/* ********** ARRAY-LIKE ********** */
/**
 * @function	ytable_add
 *		Add an element at the end of a ytable (used as an array).
 * @param	table	Pointer to the ytable.
 * @param	data	Pointer to data.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_add(ytable_t *table, void *data);
/**
 * @function	ytable_push
 *		Add an element at the beginning of a ytable (used as an array).
 * @param	table	Pointer to the ytable.
 * @param	data	Pointer to data.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_push(ytable_t *table, void *data);
/**
 * @function	ytable_pop
 *		Remove the last element of a ytable and return it.
 * @param	table	Pointer to the ytable.
 * @return	A pointer to the removed data.
 */
void *ytable_pop(ytable_t *table);
/**
 * @function	ytable_shift
 *		Remove the first element of a ytable and return it.
 * @param	table	Pointer to the ytable.
 * @return	A pointer to the removed data.
 */
void *ytable_shift(ytable_t *table);

/* ******** INDEXED FUNCTIONS ********** */
/**
 * @function	ytable_index_exists
 *		Tell if a given key exists in a ytable.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the searched element.
 * @return	true if the index exists.
 */
bool ytable_index_exists(ytable_t *table, uint64_t index);
/**
 * @function	ytable_set_index
 *		Add an element in a ytable using an index.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 * @param	data	Pointer to the data.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_set_index(ytable_t *table, uint64_t index, void *data);
/**
 * @function	ytable_extract_index
 *		Extract an element from its index and return it.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 * @return	A pointer to the removed element data.
 */
void *ytable_extract_index(ytable_t *table, uint64_t index);
/**
 * @function	ytable_remove_index
 *		Remove an element from its index.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 */
void ytable_remove_index(ytable_t *table, uint64_t index);

/* ********** KEYED FUNCTIONS ********** */
/**
 * @function	ytable_key_exists
 *		Tell if a given key exists in a ytable.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the searched element.
 * @return	true if the key exists.
 */
bool ytable_key_exists(ytable_t *table, const char *key);
/**
 * @function	ytable_set_key
 *		Add an element in a ytable using a string key.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the element.
 * @param	data	Pointer to the data.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_set_key(ytable_t *table, const char *key, void *data);
/**
 * @function	ytable_extract_key
 *		Extract an element from its string key and return it.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the element.
 * @return	A pointer to the removed element data.
 */
void *ytable_extract_key(ytable_t *table, const char *key);

/* ********** GENERAL FUNCTIONS ********** */
/**
 * @function	ytable_length
 *		Return the used length of a ytable.
 * @return	The length of the table.
 */
uint32_t ytable_length(ytable_t *table);
/**
 * @function	ytable_foreach
 *		Apply a function on every elements of a ytable.
 * @param	table		Pointer to the ytable.
 * @param	func		Pointer to the callback function.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if the callback return YENOERR for all elements.
 */
ystatus_t ytable_foreach(ytable_t *table, ytable_function_t func, void *user_data);

