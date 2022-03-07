/*!
 * @header	yhashmap.h
 * @abstract	All definitions about hash maps.
 * @discussion  Hash maps are data structures that uses hash function to map values.
 * @version     1.0 Aug 13 2012
 * @author      Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "yarray.h"
#include "ystatus.h"

/**
 * typedef	yhashmap_element_t
 *		Structure used to store a hash map's element.
 * @field	key	Element's key.
 * @field	data	Element's data.
 */
typedef struct {
	char *key;
	void *data;
} yhashmap_element_t;

/**
 * typedef	yhashmap_function_t
 *		Function pointer, used to apply a procedure to an element.
 * @param	key		Pointer to the key.
 * @param	data		Pointer to the data.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if OK.
 */
typedef ystatus_t (*yhashmap_function_t)(char *key, void *data, void *user_data);

/**
 * @typedef	yhashmap_t
 * @field	size		Current size of the hash map.
 * @field	used		Current number of elements stored in the hash map.
 * @field	buckets		Array of buckets.
 * @field	destroy_func	Pointer to the function called when an element is removed.
 * @field	destroy_data	Pointer to some user data given to the destroy function.
 */
typedef struct yhashmap_s {
	size_t size;
	size_t used;
	yarray_t buckets;
	yhashmap_function_t destroy_func;
	void *destroy_data;
} yhashmap_t;

/* ****************** FUNCTIONS **************** */
/**
 * @function	yhashmap_new
 *		Creates a new hash map.
 * @param	destroy_func	Function called when an element is removed.
 * @param	destroy_data	Pointer to user data given to the destroy function.
 * @return	The created hash map.
 */
yhashmap_t *yhashmap_new(yhashmap_function_t destroy_func, void *destroy_data);
/**
 * @function	yhashmap_create
 *		Creates a new hash map of a given size.
 * @param	size		Size of the hash map.
 * @param	destroy_func	Function called when an element is removed.
 * @param	destroy_data	Pointer to user data given to the destroy function.
 * @return	The created hash map.
 */
yhashmap_t *yhashmap_create(size_t size, yhashmap_function_t destroy_func, void *destroy_data);
/**
 * @function	yhashmap_clone
 *		Duplicate a hashmap.
 * @param	src Pointer to the source yhashmap.
 * @return	The created yhashmap.
 */
yhashmap_t *yhashmap_clone(const yhashmap_t *src);
/**
 * @function	yhashmap_delete
 *		Destroy a hash map.
 * @param	hashmap	Pointer to the hash map.
 */
void yhashmap_delete(yhashmap_t *hashmap);
/**
 * @function	yhashmap_length
 *		Return the number of elements stored in an hash map.
 * @param	hashmap	Pointer to the hash map.
 * @return	The number of elements in the hash map.
 */
size_t yhashmap_length(yhashmap_t *hashmap);
/**
 * @function	yhashmap_size
 *		Return the allocated size of an hash map.
 * @param	hashmap	Pointer to the hash map.
 * @return	The allocated size of the hash map.
 */
size_t yhashmap_size(yhashmap_t *hashmap);
/**
 * @function	yhashmap_add
 *		Add an element to a hash map.
 * @param	hashmap	Pointer to the hash map.
 * @param	key	Key used to index the element.
 * @param	data	The element's data.
 */
void yhashmap_add(yhashmap_t *hashmap, char *key, void *data); 
/**
 * @function	yhashmap_search
 *		Search an element in a hash map, and returns its value. Returns
 *		NULL if the elements doesn't exist (and if it exists and contains
 *		NULL as its value).
 * @param	hashmap	Pointer to the hash map.
 * @param	key	Key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yhashmap_search(yhashmap_t *hashmap, const char *key);
/**
 * @function	yhashmap_search_element
 *		Search an element in a hash map, and returns a pointer to the
 *		element item. Returns NULL if the element doesn't exist. This
 *		function is useful to find an element wich contains the NULL value,
 *		or to chek if an element exists.
 * @param	hashmap	Pointer to the hash map.
 * @param	key	Key used to index the element.
 * @return	A pointer to the element's structure.
 */
yhashmap_element_t *yhashmap_search_element(yhashmap_t *hahsmap, const char *key);
/*
 * @function	yhashmap_extract
 * 		Extract an element from a hash map.
 * @param	hashmap	Pointer to the hash map.
 * @param	key	Key used to index the element.
 * @return	A pointer to the extracted element, or NULL if it wasn't found.
 */
void *yhashmap_extract(yhashmap_t *hashmap, const char *key);
/**
 * @function	yhashmap_resize
 *		Resize a hash map.
 * @param	hashmap	Pointer to the hash map.
 * @param	size	The new size.
 */
void yhashmap_resize(yhashmap_t *hashmap, size_t size);
/**
 * @function	yhashmap_foreach
 *		Apply a function on every elements of a hash map.
 * @param	hashmap		Pointer to the hash map.
 * @param	func		Pointer to the executed function.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if OK.
 */
ystatus_t yhashmap_foreach(yhashmap_t *hashmap, yhashmap_function_t func, void *user_data);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

