#include <stdio.h>
#include "yhashstack.h"

/*
 * yhs_new
 * Creates a new hash stack.
 */
yhashstack_t yhs_new(yhs_size_t size) {
	return ((yhashstack_t)yv_create((yv_size_t)size));
}

/*
 * yhs_delete
 * Destroy a hash stack. The enclosed hash tables are NOT deleted.
 */
void yhs_delete(yhashstack_t *hashstack) {
	yv_del((yvect_t*)hashstack, NULL, NULL);
}

/*
 * yhs_duplicate
 * Duplicate a hash stack.
 */
yhashstack_t yhs_duplicate(yhashstack_t hashstack) {
	return (yv_dup((yvect_t)hashstack));
}

/*
 * yhs_cat
 * Add the content of a hash stack at the end of another hash stack.
 * If a hash table of the source hash stack already exists in the
 * destination hash stack, it is not added.
 */
void yhs_cat(yhashstack_t *destination, yhashstack_t source) {
	size_t	len;

	for (len = yv_len((yvect_t)source); len > 0; --len) {
		yhs_push_hash(destination, (yhashtable_t*)source[len - 1]);
	}
}

/*
 * yhs_tac
 * Add the content of a hash stack at the beginning of another hash stack.
 * If a hash table of the source hash stack already exists in the
 * destination hash stack, it is not added.
 */
void yhs_tac(yhashstack_t *destination, yhashstack_t source) {
	size_t	len;

	for (len = yv_len((yvect_t)source); len > 0; --len) {
		yhs_add_hash(destination, (yhashtable_t*)source[len - 1]);
	}
}

/*
 * yhs_get_last_hash
 * Returns a pointer to the last hash table of a hash stack.
 */
yhashtable_t *yhs_get_last_hash(yhashstack_t hashstack) {
	size_t		len;
	yhashtable_t	*hashtable;

	len = yv_len((yvect_t)hashstack);
	if (len == 0)
		return (NULL);
	hashtable = (yhashtable_t*)hashstack[len - 1];
	return (hashtable);
}

/*
 * yhs_push_hash
 * Add a hash table at the end of a hash stack, only if it doesn't already
 * exists in the stack.
 */
void yhs_push_hash(yhashstack_t *hashstack, yhashtable_t *hashtable) {
	size_t	len;

	for (len = yv_len((yvect_t)*hashstack); len > 0; --len) {
		if ((yhashtable_t*)hashstack[len - 1] == hashtable)
			return;
	}
	yv_add((yvect_t*)hashstack, hashtable);
}

/*
 * yhs_add_hash
 * Add a hash table at the beginning of a hash stack, only if it doesn't already
 * exists in the stack.
 */
void yhs_add_hash(yhashstack_t *hashstack, yhashtable_t *hashtable) {
	size_t	len;

	for (len = yv_len((yvect_t)*hashstack); len > 0; --len) {
		if ((yhashtable_t*)hashstack[len - 1] == hashtable)
			return;
	}
	yv_put((yvect_t*)hashstack, hashtable);
}

/*
 * yhs_pop_hash
 * Remove the last hash table of a hash stack, and return it.
 */
yhashtable_t *yhs_pop_hash(yhashstack_t hashstack) {
	return (yv_get((yvect_t)hashstack));
}

/*
 * yhs_add_from_string
 * Add an element in the last hash table of a hash stack, from its string key.
 * If the hash stack doesn't contain any hash table, an empty hash table is added
 * to it first.
 */
void yhs_add_from_string(yhashstack_t hashstack, char *key, void *data) {
	yhashtable_t	*hashtable;

	hashtable = yhs_get_last_hash(hashstack);
	if (hashtable == NULL) {
		hashtable = yht_new(YHT_SIZE_NANO, NULL, NULL);
		yhs_push_hash(&hashstack, hashtable);
	}
	yht_add_from_string(hashtable, key, data);
}

/*
 * yhs_add_from_int
 * Add an element in the last hash table of a hash stack, from its integer key.
 * If the hash stack doesn't contain any hash table, an empty hash table is added
 * to it first.
 */
void yhs_add_from_int(yhashstack_t hashstack, size_t key, void *data) {
	yhashtable_t	*hashtable;

	hashtable = yhs_get_last_hash(hashstack);
	if (hashtable == NULL) {
		hashtable = yht_new(YHT_SIZE_NANO, NULL, NULL);
		yhs_push_hash(&hashstack, hashtable);
	}
	yht_add_from_int(hashtable, key, data);
}

/*
 * yhs_search_from_string
 * Search an element in a hash stack, from its string key.
 */
void *yhs_search_from_string(yhashstack_t hashstack, const char *key) {
	yht_hash_value_t	hval;
	size_t			len;
	void			*res;
	yhashtable_t		*ht;

	hval = yht_hash(key);
	for (len = yv_len((yvect_t)hashstack); len > 0; --len) {
		ht = (yhashtable_t*)hashstack[len - 1];
		res = yht_search_from_hashed_string(ht, hval, key);
		if (res != NULL)
			return (res);
	}
	return (NULL);
}

/*
 * yhs_search_from_int
 * Search an element in a hash stack, from its integer key.
 */
void *yhs_search_from_int(yhashstack_t hashstack, size_t key) {
	size_t		len;
	void		*res;
	yhashtable_t	*ht;

	for (len = yv_len((yvect_t)hashstack); len > 0; --len) {
		ht = (yhashtable_t*)hashstack[len - 1];
		res = yht_search_from_int(ht, key);
		if (res != NULL)
			return (res);
	}
	return (NULL);
}
