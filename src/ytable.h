/**
 * @header	ytable.h
 * @abstract	Table used for array/hashmap/hashtables features.
 * @version	1.0.0, March 22 2022
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#include <stdint.h>
#include "ystatus.h"

/** @typedef ytable_function_t	Function pointer. */
typedef ystatus_t (*ytable_function_t)(uint64_t hash, char *key, void *data, void *user_data);
/**
 * @typedef	_ytable_t
 *		ytable structure.
 * @field	length		Number of stored elements.
 * @field	array_size	Allocated size of the array.
 * @field	next_index	Next numeric index.
 * @field	elements	Array of table's elements.
 * @field	buckets		Array of bucket lists.
 * @field	delete_function	Pointer to a function used to delete elements.
 * @field	delete_data	Pointer to data pass to the delete function.
 */
typedef struct ytable_s {
	uint32_t length;
	uint32_t array_size;
	uint64_t next_index;
	struct _ytable_element_s *elements;
	uint32_t **buckets;
	ytable_function_t delete_function;
	void *delete_data;
} ytable_t;

#include "yresult.h"

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
 * @function	ytable_init
 *		Initialize a ytable (for static usage).
 * @param	table		Pointer to the ytable.
 * @param	delete_function	Pointer to the function used to delete elements. Could be NULL.
 * @param	delete_data	Pointer to some data given to the delete function. Could be NULL.
 * @return	A pointer to the given ytable.
 */
ytable_t *ytable_init(ytable_t *table, ytable_function_t delete_function, void *delete_data);
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
/**
 * @function	ytable_clone
 *		Clone a ytable. Pointers are simply copied, as well as destruction data.
 * @param	table	Pointer to the ytable.
 * @return	The cloned table.
 */
ytable_t *ytable_clone(ytable_t *table);

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
 * @function	ytable_madd
 *		Add multiple elements at the end of a ytable (used as an array).
 * @param	table	Pointer to the ytable.
 * @param	count	Number of elements to add.
 * @param	...	Elements to add.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_madd(ytable_t *table, uint32_t count, ...);
/**
 * @function	ytable_push
 *		Add an element at the beginning of a ytable (used as an array).
 * @param	table	Pointer to the ytable.
 * @param	data	Pointer to data.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_push(ytable_t *table, void *data);
/**
 * @function	ytable_mpush
 *		Add multiple elements at the beginning of a ytable (used as an array).
 * @param	table	Pointer to the ytable.
 * @param	count	Number of elements to add.
 * @param	...	Elements to add.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_mpush(ytable_t *table, uint32_t count, ...);
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
 * @function	ytable_index_isset
 *		Tell if a given index exists and is set in a ytable.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the searched element.
 * @return	true if the index exists and the associated value is not NULL.
 */
bool ytable_index_isset(ytable_t *table, uint64_t index);
/**
 * @function	ytable_get_index
 *		Return the value associated to the given index.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 * @return	YENOERR if the element exists, and a pointer to the element's data.
 *		YEINVAL if the table doesn't exist.
 *		YEUNDEF if the index doesn't exist.
 */
yres_pointer_t ytable_get_index(ytable_t *table, uint64_t index);
/**
 * @function	ytable_get_index_data
 *		Return the data value associated to the given index.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 * @return	A pointer to the element's data, or NULL if the element doesn't exist.
 */
void *ytable_get_index_data(ytable_t *table, uint64_t index);
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
 *		Extract an element from its index and return its data.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 * @return	YENOERR if the element exists, and a pointer to the element's data.
 *		YEINVAL if the table doesn't exist.
 *		YEUNDEF if the index doesn't exist.
 */
yres_pointer_t ytable_extract_index(ytable_t *table, uint64_t index);
/**
 * @function	ytable_extract_index_data
 *		Extract an element from its index and return its data.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 * @return	A pointer to the removed element's data.
 */
void *ytable_extract_index_data(ytable_t *table, uint64_t index);
/**
 * @function	ytable_remove_index
 *		Remove an element from its index.
 * @param	table	Pointer to the ytable.
 * @param	index	Index of the element.
 * @return	YENOERR if OK.
 */
ystatus_t ytable_remove_index(ytable_t *table, uint64_t index);

/* ********** KEYED FUNCTIONS ********** */
/**
 * @function	ytable_key_exists
 *		Tell if a given string key exists in a ytable.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the searched element.
 * @return	true if the string key exists.
 */
bool ytable_key_exists(ytable_t *table, const char *key);
/**
 * @function	ytable_key_isset
 *		Tell if a given string key exists and is set in a ytable.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the searched element.
 * @return	true if the string key exists and the associated value is not NULL.
 */
bool ytable_key_isset(ytable_t *table, const char *key);
/**
 * @function	ytable_get_key
 *		Return the value associated to the given string key.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the search element.
 * @return	YENOERR if the element exists, and a pointer to the element's data.
 *		YEINVAL if the table doesn't exist.
 *		YEUNDEF if the index doesn't exist.
 */
yres_pointer_t ytable_get_key(ytable_t *table, const char *key);
/**
 * @function	ytable_get_key_data
 *		Return the data value associated to the given string key.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the search element.
 * @return	A pointer to the element's data, or NULL if the element doesn't exist.
 */
void *ytable_get_key_data(ytable_t *table, const char *key);
/**
 * @function	ytable_set_key
 *		Add an element in a ytable using a string key.
 * @param	table	Pointer to the ytable.
 * @param	key	String key of the element.
 * @param	data	Pointer to the data.
 * @return	YENOERR if OK.
 *		YEINVAL if the table doesn't exist or the key parameter is NULL.
 *		YENOMEM if the functions used to expand the ytable failed.
 *		Other error code if the function used to delete data failed.
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
 * @function	ytable_is_array
 *		Tell if a ytable is used as an array (continuous list of elememnts).
 * @param	table	Pointer to the ytable.
 * @return	True if the table is an array.
 */
bool ytable_is_array(ytable_t *table);
/**
 * @function	ytable_foreach
 *		Apply a function on every elements of a ytable.
 * @param	table		Pointer to the ytable.
 * @param	func		Pointer to the callback function.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if the callback return YENOERR for all elements.
 */
ystatus_t ytable_foreach(ytable_t *table, ytable_function_t func, void *user_data);

