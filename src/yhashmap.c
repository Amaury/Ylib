#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "y.h"

/* ************ PRIVATE DEFINITIONS AND MACROS ************ */
/** @define YHM_MINIMAL_SIZE Minimal size of an hashmap. */
#define _YHASHMAP_DEFAULT_SIZE	256
/** @define _ARRAY_SIZE Compute the size of a new yhashmap's buffer. */
#define _YHASHMAP_SIZE(s)	COMPUTE_SIZE((s), _YHASHMAP_DEFAULT_SIZE)
/** @define YHM_MAX_LOAD_FACTOR	Maximum load factor before increasing a hash map. */
#define YHM_MAX_LOAD_FACTOR	0.7
/** @define YHM_MIN_LOAD_FACTOR Minimum load factor before reducing a hash map. */
#define YHM_MIN_LOAD_FACTOR	0.25

/* ********** DECLARATION OF PRIVATE FUNCTIONS ********** */
ystatus_t _yhashmap_clone_bucket(size_t index, void *data, void *user_data);

/* ********** FUNCTIONS ********** */
/* Create a new hash map. */
yhashmap_t *yhashmap_new(yhashmap_function_t destroy_func, void *destroy_data) {
	return (yhashmap_create(_YHASHMAP_DEFAULT_SIZE, destroy_func, destroy_data));
}
/* Create a new hash map of a given size. */
yhashmap_t *yhashmap_create(size_t size, yhashmap_function_t destroy_func, void *destroy_data) {
	yhashmap_t *hash;
	size = _YHASHMAP_SIZE(size);

	hash = (yhashmap_t*)malloc0(sizeof(yhashmap_t));
	if (!hash)
		return (NULL);
	hash->buckets = yarray_create(size);
	hash->size = size;
	hash->used = 0;
	hash->destroy_func = destroy_func;
	hash->destroy_data = destroy_data;
	return (hash);
}
/* Duplicate a hashmap. */
yhashmap_t *yhashmap_clone(const yhashmap_t *src) {
	yhashmap_t *dest = (yhashmap_t*)malloc0(sizeof(yhashmap_t));
	if (!dest)
		return (NULL);
	dest->buckets = yarray_create(yarray_size(src->buckets));
	if (!dest->buckets) {
		free0(dest);
		return (NULL);
	}
	ystatus_t st = yarray_foreach(src->buckets, _yhashmap_clone_bucket, dest);
	if (st != YENOERR) {
		yhashmap_delete(dest);
		return (NULL);
	}
	return (dest);
}
ystatus_t _yhashmap_clone_bucket(size_t index, void *data, void *user_data) {
	yarray_t src_bucket = (yarray_t)data;
	yarray_t dest_bucket = yarray_clone(src_bucket);
	if (!dest_bucket)
		return (YENOMEM);
	yhashmap_t *dest = (yhashmap_t*)user_data;
	yarray_push(&dest->buckets, dest_bucket);
	return (YENOERR);
}
/* Destroy an hash map. */
void yhashmap_delete(yhashmap_t *hashmap) {
	// remove elements and buckets
	while (yarray_length(hashmap->buckets)) {
		yarray_t bucket = yarray_pop(hashmap->buckets);
		if (!bucket)
			continue;
		while (yarray_length(bucket)) {
			yhashmap_element_t *elem = yarray_pop(bucket);
			if (elem && hashmap->destroy_func) {
				hashmap->destroy_func(elem->key, elem->data,
				                      hashmap->destroy_data);
			}
		}
		yarray_free(bucket);
	}
	yarray_free(hashmap->buckets);
	// remove the hash map itself
	free0(hashmap);
}
/* Return the used size of an hash map. */
size_t yhashmap_length(yhashmap_t *hashmap) {
	if (!hashmap)
		return (0);
	return (hashmap->used);
}
/* Return the allocated size of an hash map. */
size_t yhashmap_size(yhashmap_t *hashmap) {
	if (!hashmap)
		return (0);
	return (hashmap->size);
}
/* Add an element to an hash map. */
void yhashmap_add(yhashmap_t *hashmap, char *key, void *data) {
	double load_factor;
	yhash_value_t hash_value;
	yarray_t *bucket;
	yhashmap_element_t *element;

	if (!key)
		return;
	// resize the map if its load factor will excess the limit
	load_factor = (double)(hashmap->used + 1) / hashmap->size;
	if (load_factor > YHM_MAX_LOAD_FACTOR) {
		yhashmap_resize(hashmap, (hashmap->size * 2));
	}
	// compute the key's hash value
	hash_value = yhash_compute(key);
	hash_value %= hashmap->size;
	// check the bucket
	bucket = (yarray_t*)&(hashmap->buckets[hash_value]);
	if (!*bucket) {
		// create the bucket
		yarray_t new_bucket = yarray_new();
		yarray_set(hashmap->buckets, new_bucket, hash_value, NULL, NULL);
		bucket = &new_bucket;
	} else {
		// there is already some elements in the bucket, checking if the
		// element exists and must be updated
		for (size_t offset = 0; offset < yarray_length(*bucket); ++offset) {
			element = (*bucket)[offset];
			if (element && element->key && !strcmp(key, element->key)) {
				// an existing element was found with the same key
				// remove old data
				if (hashmap->destroy_func) {
					hashmap->destroy_func(element->key,
					                      element->data,
					                      hashmap->destroy_data);
				}
				// update the element
				element->data = data;
				return;
			}
		}
		// no existing element with this key
	}
	// create the element
	element = (yhashmap_element_t*)malloc0(sizeof(yhashmap_element_t));
	// filling the element
	element->key = key;
	element->data = data;
	// add the element to the bucket
	yarray_push(bucket, element);
	// update the hash map
	hashmap->used++;

	int total = 0;
	for (size_t i = 0; i < yarray_size(hashmap->buckets); ++i) {
		yarray_t bucket = hashmap->buckets[i];
		if (!bucket)
			continue;
		for (size_t j = 0; j < yarray_length(bucket); ++j) {
			if (bucket[j])
				total++;
		}
	}
}
/* Search an element in an hash map, and returns its value. */
void *yhashmap_search(yhashmap_t *hashmap, const char *key) {
	yhashmap_element_t *element;

	element = yhashmap_search_element(hashmap, key);
	if (element != NULL)
		return (element->data);
	return (NULL);
}
/* Search an element in a hash map, and return a pointer to the element item. */
yhashmap_element_t *yhashmap_search_element(yhashmap_t *hashmap, const char *key) {
	yhash_value_t hash_value;
	yarray_t bucket;
	yhashmap_element_t *element;

	if (!key)
		return (NULL);
	/* compute the key's hash value */
	hash_value = yhash_compute(key);
	hash_value %= hashmap->size;
	/* retreiving the bucket */
	bucket = hashmap->buckets[hash_value];
	if (!*bucket)
		return (NULL);
	size_t len = yarray_length(bucket);
	if (!len)
		return (NULL);
	if (len == 1) {
		element = bucket[0];
		if (element && element->key && !strcmp(key, element->key))
			return (element);
		return (NULL);
	}
	for (size_t offset = 0; offset < len; ++offset) {
		element = bucket[offset];
		if (element && element->key && !strcmp(key, element->key))
			return (element);
	}
	return (NULL);
}
/* Extract an element from an hash map. */
void *yhashmap_extract(yhashmap_t *hashmap, const char *key) {
	yhash_value_t hash_value;
	yarray_t bucket;
	yhashmap_element_t *element;
	void *result = NULL;
	bool found = false;
	double load_factor;

	/* compute the key's hash value */
	hash_value = yhash_compute(key);
	hash_value %= hashmap->size;
	/* retreiving the bucket */
	bucket = &(hashmap->buckets[hash_value]);
	if (!bucket)
		return (NULL);
	size_t len = yarray_length(bucket);
	if (!len)
		return (NULL);
	if (len == 1) {
		found = true;
		element = yarray_pop(bucket);
		if (!element)
			return (NULL);
		result = element->data;
		free0(element);
		hashmap->used--;
		return (result);
	}
	size_t max_offset = yarray_length(bucket);
	for (size_t offset = 0; offset < max_offset; ++offset) {
		element = bucket[offset];
		if (!element || !element->key || strcmp(key, element->key))
			continue;
		found = true;
		yarray_extract(bucket, offset);
		result = element->data;
		free0(element);
		break;
	}
	if (found) {
		hashmap->used--;
		// resize the map if its load factor felt under the limit
		load_factor = (double)(hashmap->used + 1) / hashmap->size;
		if (load_factor < YHM_MIN_LOAD_FACTOR)
			yhashmap_resize(hashmap, (hashmap->size / 2));
	}
	return (result);
}
/* Resize an hashmap. */
void yhashmap_resize(yhashmap_t *hashmap, size_t size) {
	yhashmap_t *new_hashmap;

	if (!hashmap || size < hashmap->used)
		return;
	new_hashmap = yhashmap_create(size, hashmap->destroy_func, hashmap->destroy_data);
	for (size_t i = 0; i < yarray_size(hashmap->buckets); ++i) {
		yarray_t bucket = hashmap->buckets[i];
		if (!bucket)
			continue;
		while (yarray_length(bucket)) {
			yhashmap_element_t *elem = yarray_pop(bucket);
			if (!elem)
				continue;
			yhashmap_add(new_hashmap, elem->key, elem->data);
			free0(elem);
		}
		yarray_free(bucket);
	}
	yarray_free(hashmap->buckets);
	// moving data from new hashmap to old hashmap
	hashmap->size = new_hashmap->size;
	hashmap->used = new_hashmap->used;
	hashmap->buckets = new_hashmap->buckets;
	// free memory
	free0(new_hashmap);
}
/* Apply a function on every elements of an hash map. */
ystatus_t yhashmap_foreach(yhashmap_t *hashmap, yhashmap_function_t func, void *user_data) {
	if (!hashmap || !func)
		return (YENOERR);
	for (size_t offset = 0; offset < yarray_size(hashmap->buckets); ++offset) {
		yarray_t bucket = hashmap->buckets[offset];
		if (!bucket)
			continue;
		for (size_t offset2 = 0; offset2 < yarray_length(bucket); ++offset2) {
			yhashmap_element_t *elem = bucket[offset2];
			if (!elem)
				continue;
			ystatus_t st = func(elem->key, elem->data, user_data);
			if (st != YENOERR)
				return (st);
		}
	}
	return (YENOERR);
}

