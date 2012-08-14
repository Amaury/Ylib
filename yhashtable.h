/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	yhashtable.h
 * @abstract	All definitions about hash tables.
 * @discussion  Hash tables are data structures that uses hash function to map values.
 * @version     1.0 Aug 14 2012
 * @author      Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YHASHTABLE_H__
#define __YHASHTABLE_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#ifdef USE_BOEHM_GC
# include "gc.h"
#endif /* USE_BOEHM_GC */

/** @define YHT_MAX_LOAD_FACTOR	Maximum load factor of an hash table before increasing it. */
#define YHT_MAX_LOAD_FACTOR	0.7

/** @define YHT_MIN_LOAD_FACTOR Minimum load factor of an hash table before reducing it. */
#define YHT_MIN_LOAD_FACTOR	0.25

/**
 * @typedef	yht_size_t
 *		Enum used to define the size of an hash table.
 * @constant	YHT_SIZE_MINI		Minimal size of hash tables (32).
 * @constant	YHT_SIZE_MEDIUM		Medium size of hash tables (256).
 * @constant	YHT_SIZE_DEFAULT	Default size of hash tables (4K).
 * @constant	YHT_SIZE_BIG		Size of big hash tables (65K).
 * @constant	YHT_SIZE_HUGE		Size of huge hash tables (1M).
 */
typedef enum yht_size_e {
	YHT_SIZE_MINI		= 32,
	YHT_SIZE_MEDIUM		= 256,
	YHT_SIZE_DEFAULT	= 4096,
	YHT_SIZE_BIG		= 65536,
	YHT_SIZE_HUGE		= 1048576
} yht_size_t;

/**
 * typedef	yht_element_t
 *		Structure used to store an hash table's element.
 * @field	key		Element's key.
 * @field	data		Element's data.
 * @field	previous	Pointer to the previous element with the same hash value.
 * @field	next		Pointer to the next element with the same hash value.
 */
typedef struct yht_element_s {
	char			*key;
	void			*data;
	struct yht_element_s	*previous;
	struct yht_element_s	*next;
} yht_element_t;

/**
 * typedef	yht_bucket_t
 *		Structure used to store the values associated with an hash value.
 * @field	nbr_elements	Elements count for this bucket.
 * @field	elements	Pointer to the first element.
 */
typedef struct yht_bucket_s {
	size_t		nbr_elements;
	yht_element_t	*elements;
} yht_bucket_t;

/**
 * typedef	yht_function_t
 *		Function pointer, used to apply a procedure to an element.
 * @param	key		Pointer to the key.
 * @param	data		Pointer to the data.
 * @param	user_data	Pointer to some user data.
 */
typedef void (*yht_function_t)(char *key, void *data, void *user_data);

/**
 * typedef	yht_hash_value_t
 * 		An hash value.
 */
typedef long int yht_hash_value_t;

/**
 * @typedef	yhashtable_t
 * @field	size		Current size of the hash table.
 * @field	used		Current number of elements stored in the hash table.
 * @field	buckets		Array of buckets.
 * @field	elements	Array of pointer to the stored elements.
 * @field	destroy_func	Pointer to the function called when an element is removed.
 * @field	destroy_data	Pointer to some user data given to the destroy function.
 */
typedef struct yhashtable_s {
	size_t		size;
	size_t		used;
	yht_bucket_t	*buckets;
	yht_function_t	destroy_func;
	void		*destroy_data;
} yhashtable_t;

/* ****************** FUNCTIONS **************** */
/**
 * @function	yht_new
 *		Creates a new hash table.
 * @param	size		Initial size of the hash table.
 * @param	destroy_func	Pointer to the function called when an element is removed.
 * @param	destroy_data	Pointer to some suer data given to the destroy function.
 * @return	The created hash table.
 */
yhashtable_t *yht_new(size_t size, yht_function_t destroy_func, void *destroy_data);

/**
 * @function	yht_delete
 *		Destroy an hash table.
 * @param	hash	Pointer to the hash table.
 */
void yht_delete(yhashtable_t *hashtable);

/**
 * @function	yht_hash
 *		Compute the hash value of a key, using the SDBM algorithm.
 * @see		http://www.cse.yorku.ca/~oz/hash.html
 * @see		http://en.literateprograms.org/Hash_function_comparison_%28C,_sh%29#chunk
 * @param	Key	The data to hash.
 * @return	The computed hash value.
 */
yht_hash_value_t yht_hash(const char *key);

/**
 * @function	yht_add_from_string
 *		Add an element to an hash table, using a string key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @param	data		The element's data.
 */
void yht_add_from_string(yhashtable_t *hashtable, char *key, void *data); 

/**
 * @function	yht_add_from_int
 *		Add an element to an hash table, using an integer key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @param	data		The element's data.
 */
void yht_add_from_int(yhashtable_t *hashtable, size_t key, void *data);

/**
 * @function	yht_search_from_string
 *		Search an element in an hash table, from its string key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yht_search_from_string(yhashtable_t *hashtable, const char *key);

/**
 * @function	yht_search_from_int
 *		Search an element in an hash table, from its integer key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yht_search_from_int(yhashtable_t *hashtable, size_t key);

/**
 * @function	yht_remove_from_string
 * 		Remove an element from an hash table, using its string key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	1 if the element was found, 0 otherwise.
 */
char yht_remove_from_string(yhashtable_t *hashtable, const char *key);

/**
 * @function	yht_remove_from_int
 * 		Remove an element from an hash table, using its integer key.
 * @param	hashtable	Pointer to the hash table.
 * @param	key		Key used to index the element.
 * @return	1 if the element was found, 0 otherwise.
 */
char yht_remove_from_int(yhashtable_t *hashtable, size_t key);

/**
 * @function	yht_resize
 *		Resize an hash table.
 * @param	hashtable	Pointer to the hash table.
 * @param	size		The new size.
 */
void yht_resize(yhashtable_t *hashtable, size_t size);

/**
 * @function	yht_foreach
 *		Apply a function on every elements of an hash table.
 * @param	hashtable	Pointer to the hash table.
 * @param	func		Pointer to the executed function.
 * @param	user_data	Pointer to some user data.
 */
void yht_foreach(yhashtable_t *hashtable, yht_function_t func, void *user_data);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YHASHTABLE_H__ */
