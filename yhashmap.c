#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yhashmap.h"

/* *** definition of private functions *** */
static yhm_hash_value_t _yhm_hash(const char *key);

/*
 * yhm_new()
 * Creates a new hash map.
 */
yhashmap_t *yhm_new(yhm_size_t size, yhm_function_t destroy_func, void *destroy_data) {
	yhashmap_t	*hash;

	hash = (yhashmap_t*)YMALLOC(sizeof(yhashmap_t));
	hash->buckets = (yhm_bucket_t*)YCALLOC(size, sizeof(yhm_bucket_t));
	hash->size = size;
	hash->used = 0;
	hash->destroy_func = destroy_func;
	hash->destroy_data = destroy_data;
	return (hash);
}

/*
 * yhm_delete
 * Destroy an hash map.
 */
void yhm_delete(yhashmap_t *hashmap) {
	/* remove elements */
	if (hashmap->used > 0) {
		size_t		offset, offset2;
		yhm_bucket_t	*bucket;
		yhm_element_t	*element, *old_element;

		for (offset = 0; offset < hashmap->size; offset++) {
			bucket = &(hashmap->buckets[offset]);
			for (offset2 = 0, element = bucket->elements;
			     offset2 < bucket->nbr_elements;
			     offset2++) {
				if (hashmap->destroy_func != NULL)
					hashmap->destroy_func(element->key, element->data, hashmap->destroy_data);
				old_element = element;
				element = element->next;
				YFREE(old_element);
			}
		}
	}
	/* remove buckets and the hash map itself */
	YFREE(hashmap->buckets);
	YFREE(hashmap);
}

/*
 * yhm_add
 * Add an element to an hash map.
 */
void yhm_add(yhashmap_t *hashmap, char *key, void *data) {
	float			load_factor;
	yhm_hash_value_t	hash_value;
	yhm_bucket_t		*bucket;
	yhm_element_t		*element;

	/* resize the map if its load factor would excess the limit */
	load_factor = (float)(hashmap->used + 1) / hashmap->size;
	if (load_factor > YHM_MAX_LOAD_FACTOR)
		yhm_resize(hashmap, (hashmap->size * 2));
	/* compute the key's hash value */
	hash_value = _yhm_hash(key);
	hash_value %= hashmap->size;
	/* checking the bucket */
	bucket = &(hashmap->buckets[hash_value]);
	if (bucket->nbr_elements == 0) {
		/* create the first element */
		element = (yhm_element_t*)YMALLOC(sizeof(yhm_element_t));
		element->previous = element->next = element;
		/* add the element to the bucket */
		bucket->elements = element;
	} else {
		/* there is already some elements in the bucket, checking if the element exists and must be updated */
		size_t	offset;

		for (offset = 0, element = bucket->elements;
		     offset < bucket->nbr_elements;
		     offset++, element = element->next) {
			if (!strcmp(key, element->key)) {
				/* an existing element was found */
				/* removing old data */
				if (hashmap->destroy_func != NULL)
					hashmap->destroy_func(element->key, element->data, hashmap->destroy_data);
				/* updating the element */
				element->key = key;
				element->data = data;
				return;
			}
		}
		/* no element was already existing with this key */
		element = (yhm_element_t*)YMALLOC(sizeof(yhm_element_t));
		/* add the element to the bucket */
		element->next = bucket->elements;
		element->previous = bucket->elements->previous;
		bucket->elements->previous->next = element;
		bucket->elements->previous = element;
	}
	/* filling the element */
	element->key = key;
	element->data = data;
	/* update the bucket */
	bucket->nbr_elements++;
	/* update the hash map */
	hashmap->used++;
}

/*
 * yhm_search()
 * Search an element in an hash map, and returns its value.
 */
void *yhm_search(yhashmap_t *hashmap, const char *key) {
	yhm_element_t	*element;

	element = yhm_search_element(hashmap, key);
	if (element != NULL)
		return (element->data);
	return (NULL);
}

/*
 * yhm_search_element()
 * Search an element in a hash map, and return a pointer to the element item.
 */
yhm_element_t *yhm_search_element(yhashmap_t *hashmap, const char *key) {
	yhm_hash_value_t	hash_value;
	yhm_bucket_t		*bucket;
	yhm_element_t		*element;
	size_t			offset;

	/* compute the key's hash value */
	hash_value = _yhm_hash(key);
	hash_value %= hashmap->size;
	/* retreiving the bucket */
	bucket = &(hashmap->buckets[hash_value]);
	if (bucket->nbr_elements == 0)
		return (NULL);
	if (bucket->nbr_elements == 1)
		return (bucket->elements);
	for (offset = 0, element = bucket->elements;
	     offset < bucket->nbr_elements;
	     offset++, element = element->next) {
		if (!strcmp(key, element->key))
			return (element);
	}
	return (NULL);
}

/*
 * yhm_remove()
 * Remove an element from an hash map.
 */
ybool_t yhm_remove(yhashmap_t *hashmap, const char *key) {
	yhm_hash_value_t	hash_value;
	yhm_bucket_t		*bucket;
	yhm_element_t		*element;
	size_t			offset;
	ybool_t			found = YFALSE;
	float			load_factor;

	/* compute the key's hash value */
	hash_value = _yhm_hash(key);
	hash_value %= hashmap->size;
	/* retreiving the bucket */
	bucket = &(hashmap->buckets[hash_value]);
	if (bucket->nbr_elements == 0)
		return (YFALSE);
	if (bucket->nbr_elements == 1) {
		found = YTRUE;
		if (hashmap->destroy_func)
			hashmap->destroy_func(bucket->elements->key, bucket->elements->data, hashmap->destroy_data);
		YFREE(bucket->elements);
		bucket->elements = NULL;
	} else {
		for (offset = 0, element = bucket->elements;
		     offset < bucket->nbr_elements;
		     offset++, element = element->next) {
			if (!strcmp(key, element->key)) {
				found = YTRUE;
				if (offset == 0)
					bucket->elements = element->next;
				element->previous->next = element->next;
				element->next->previous = element->previous;
				/* call the destroy function */
				if (hashmap->destroy_func)
					hashmap->destroy_func(element->key, element->data, hashmap->destroy_data);
				YFREE(element);
				break;
			}
		}
	}
	if (found) {
		bucket->nbr_elements--;
		hashmap->used--;
		/* resize the map if its load factor will fall under the limit */
		load_factor = (float)(hashmap->used + 1) / hashmap->size;
		if (load_factor < YHM_MIN_LOAD_FACTOR)
			yhm_resize(hashmap, (hashmap->size / 2));
	}
	return (found);
}

/*
 * yhm_resize
 * Resize an hashmap.
 */
void yhm_resize(yhashmap_t *hashmap, size_t size) {
	yhashmap_t	*new_hashmap;
	size_t		offset, offset2;
	yhm_bucket_t	*bucket, *old_buckets;
	yhm_element_t	*element, *old_element;

	new_hashmap = yhm_new(size, NULL, NULL);
	for (offset = 0; offset < hashmap->size; offset++) {
		bucket = &(hashmap->buckets[offset]);
		if (bucket->nbr_elements == 0)
			continue;
		for (offset2 = 0, element = bucket->elements;
		     offset2 < bucket->nbr_elements;
		     offset2++, element = element->next) {
			yhm_add(new_hashmap, element->key, element->data);
			old_element = element;
			YFREE(old_element);
		}
	}
	hashmap->size = size;
	/* moving the list of buckets */
	old_buckets = hashmap->buckets;
	hashmap->buckets = new_hashmap->buckets;
	/* free memory */
	YFREE(old_buckets);
	YFREE(new_hashmap);
}

/*
 * yhm_foreach
 * Apply a function on every elements of an hash map.
 */
void yhm_foreach(yhashmap_t *hashmap, yhm_function_t func, void *user_data) {
	size_t		offset, nbr_processed, offset2;
	yhm_bucket_t	*bucket;
	yhm_element_t	*element;

	for (offset = nbr_processed = 0; offset < hashmap->size; offset++) {
		bucket = &(hashmap->buckets[offset]);
		if (bucket->nbr_elements == 0)
			continue;
		for (offset2 = 0, element = bucket->elements;
		     offset2 < bucket->nbr_elements;
		     offset2++, element = element->next) {
			func(element->key, element->data, user_data);
			if (++nbr_processed == hashmap->used)
				return;
		}
	}
}

/* ************ PRIVATE FUNCTIONS ********* */
/**
 * @function	yhm_hash
 *		Compute the hash value of a key, using the SDBM algorithm.
 * @see		http://www.cse.yorku.ca/~oz/hash.html
 * @see		http://en.literateprograms.org/Hash_function_comparison_%28C,_sh%29
 * @param	Key	The data to hash.
 * @return	The computed hash value.
 */
static yhm_hash_value_t _yhm_hash(const char *key) {
	yhm_hash_value_t	hash_value;

	for (hash_value = 0; *key; key++)
		hash_value = *key + (hash_value << 6) + (hash_value << 16) - hash_value;
	return (hash_value);
}

