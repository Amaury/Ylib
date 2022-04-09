/*!
 * @header	yhashtable.h
 * @abstract	All definitions about hash tables (like an hashmap, but items are ordered
 *		as their creation order).
 * @discussion  Hash tables are data structures that uses hash function to map values.
 * @version     1.0 Aug 14 2012
 * @author      Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "y.h"

#ifdef USE_BOEHM_GC
# include "gc.h"
#endif /* USE_BOEHM_GC */

/** @define YHT_MAX_LOAD_FACTOR	Maximum load factor of a hash table before increasing it. */
#define YHT_MAX_LOAD_FACTOR	0.7

/** @define YHT_MIN_LOAD_FACTOR Minimum load factor of a hash table before reducing it. */
#define YHT_MIN_LOAD_FACTOR	0.25

/**
 * @typedef	yhashtable_size_t
 *		Enum used to define the size of a hash table.
 * @constant	YHT_SIZE_NANO		Size of ultra-light hash tables, for debug purposes (4).
 * @constant	YHT_SIZE_MINI		Minimal size of hash tables (32).
 * @constant	YHT_SIZE_MEDIUM		Medium size of hash tables (256).
 * @constant	YHT_SIZE_DEFAULT	Default size of hash tables (4K).
 * @constant	YHT_SIZE_BIG		Size of big hash tables (65K).
 * @constant	YHT_SIZE_HUGE		Size of huge hash tables (1M).
 */
typedef enum yhashtable_size_e {
	YHT_SIZE_NANO		= 4,
	YHT_SIZE_MINI		= 32,
	YHT_SIZE_MEDIUM		= 256,
	YHT_SIZE_DEFAULT	= 4096,
	YHT_SIZE_BIG		= 65536,
	YHT_SIZE_HUGE		= 1048576
} yhashtable_size_t;

/**
 * typedef	yhashtable_element_t
 *		Structure used to store a hash table's element.
 * @field	hash_value	Element's hash value.
 * @field	key		Element's key.
 * @field	data		Element's data.
 * @field	previous	Pointer to the previous element with the same hash value.
 * @field	next		Pointer to the next element with the same hash value.
 */
typedef struct yhashtable_element_s {
	size_t hash_value;
	char *key;
	void *data;
	struct yhashtable_element_s *previous;
	struct yhashtable_element_s *next;
	struct yhashtable_list_s *item;
} yhashtable_element_t;

/**
 * typedef	yhashtable_bucket_t
 *		Structure used to store the values associated with a hash value.
 * @field	nbr_elements	Elements count for this bucket.
 * @field	elements	Pointer to the first element.
 */
typedef struct yhashtable_bucket_s {
	size_t nbr_elements;
	yhashtable_element_t *elements;
} yhashtable_bucket_t;

/**
 * typedef	yhashtable_list_t
 *		Double linked structure used to list the created elements.
 * @field	element		Pointer to the element.
 * @field	previous	Pointer to the previous list item.
 * @field	next		Pointer to the next list item.
 */
typedef struct yhashtable_list_s {
	yhashtable_element_t *element;
	struct yhashtable_list_s *previous;
	struct yhashtable_list_s *next;
} yhashtable_list_t;

/**
 * typedef	yhashtable_function_t
 *		Function pointer, used to apply a procedure to an element.
 * @param	yh_hash_value_t	Hash value of the element.
 * @param	key		Pointer to the key. Null if the element was inserted using an integer key.
 * @param	data		Pointer to the data.
 * @param	user_data	Pointer to some user data.
 */
typedef void (*yhashtable_function_t)(size_t hash_value, char *key, void *data, void *user_data);

/**
 * @typedef	yhashtable_t
 * @field	size		Current size of the hash table.
 * @field	used		Current number of elements stored in the hash table.
 * @field	buckets		Array of buckets.
 * @field	items		List of pointers to the stored elements.
 * @field	next_offset	Next free hash value.
 * @field	destroy_func	Pointer to the function called when an element is removed.
 * @field	destroy_data	Pointer to some user data given to the destroy function.
 */
typedef struct yhashtable_s {
	size_t size;
	size_t used;
	yhashtable_bucket_t *buckets;
	yhashtable_list_t *items;
	size_t next_offset;
	yhashtable_function_t destroy_func;
	void *destroy_data;
} yhashtable_t;

/* ****************** FUNCTIONS **************** */
/**
 * @function	yhashtable_new
 *		Creates a new hash table.
 * @param	size		Initial size of the hash table.
 * @param	destroy_func	Pointer to the function called when an element is removed.
 * @param	destroy_data	Pointer to some suer data given to the destroy function.
 * @return	The created hash table.
 */
yhashtable_t *yhashtable_new(yhashtable_size_t size, yhashtable_function_t destroy_func, void *destroy_data);

/**
 * @function	yhashtable_delete
 *		Destroy a hash table.
 * @param	hash	Pointer to the hash table.
 */
void yhashtable_delete(yhashtable_t *hashtable);

/**
 * @function	yhashtable_add_from_string
 *		Add an element to a hash table, using a string key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @param	data		The element's data.
 */
void yhashtable_add_from_string(yhashtable_t *hashtable, char *key, void *data); 

/**
 * @function	yhashtable_add_from_int
 *		Add an element to a hash table, using an integer key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @param	data		The element's data.
 */
void yhashtable_add_from_int(yhashtable_t *hashtable, size_t key, void *data);

/**
 * @function	yhashtable_push_data
 *		Add an element at the end of a hash table.
 * @param	hashtable	Pointer to the hash table.
 * @param	data		The element's data.
 */
void yhashtable_push_data(yhashtable_t *hashtable, void *data);

/**
 * @function	yhashtable_search_from_string
 *		Search an element in a hash table, from its string key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yhashtable_search_from_string(yhashtable_t *hashtable, const char *key);

/**
 * @function	yhashtable_search_from_int
 *		Search an element in a hash table, from its integer key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yhashtable_search_from_int(yhashtable_t *hashtable, size_t key);

/**
 * @function	yhashtable_search_from_hashed_string
 *		Search an element in a hash table, from its hashed string key.
 * @param	hashtable	Pointer to the hash table.
 * @param	hash_value	Hash value of the string.
 * @param	key		String key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yhashtable_search_from_hashed_string(yhashtable_t *hashtable, size_t hash_value, const char *key);

/**
 * @function	yhashtable_pop_data
 *		Remove the last element of a hash table and returns it.
 * @param	hashtable	Pointer to the hash table.
 * @return	A pointer to the element's data.
 */
void *yhashtable_pop_data(yhashtable_t *hashtable);

/**
 * @function	yhashtable_remove_from_string
 * 		Remove an element from a hash table, using its string key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	YTRUE if the element was found, YFALSE otherwise.
 */
bool yhashtable_remove_from_string(yhashtable_t *hashtable, const char *key);

/**
 * @function	yhashtable_remove_from_int
 * 		Remove an element from a hash table, using its integer key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	YTRUE if the element was found, YFALSE otherwise.
 */
bool yhashtable_remove_from_int(yhashtable_t *hashtable, size_t key);

/**
 * @function	yhashtable_resize
 *		Resize a hash table.
 * @param	hashtable	Pointer to the hash table.
 * @param	size		The new size.
 */
void yhashtable_resize(yhashtable_t *hashtable, size_t size);

/**
 * @function	yhashtable_foreach
 *		Apply a function on every elements of a hash table.
 * @param	hashtable	Pointer to the hash table.
 * @param	func		Pointer to the executed function.
 * @param	user_data	Pointer to some user data.
 */
void yhashtable_foreach(yhashtable_t *hashtable, yhashtable_function_t func, void *user_data);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

