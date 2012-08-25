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
 * yhs_get_last_hash
 * Returns a pointer to the last hash table of a hash stack.
 */
yhashtable_t *yhs_get_last_hash(yhashstack_t hashstack) {
	size_t		len;
	yhashtable_t	*hashtable;

	len = yv_len((yvect_t)hashstack);
	hashtable = (yhashtable_t*)hashstack[len];
	return (hashtable);
}

/*
 * yhs_push_hash
 * Add a hash table at the end of a hash stack, only if it doesn't already
 * exists in the stack.
 */
void yhs_push_hash(yhashstack_t *hashstack, yhashtable_t *hashtable) {
	yv_add((yvect_t*)hashstack, hashtable);
}

/*
 * yhs_pop_hash
 * Remove the last hash table of a hash stack, and return it.
 */
yhashtable_t *yhs_pop_hash(yhashstack_t hashstack) {
	return (yv_get((yvect_t)hashstack));
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
