#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yhashmap.h"

/*
 * yhm_new()
 * Creates a new hash map.
 */
yhashmap_t *yhm_new(size_t size, yhm_function_t destroy_func, void *destroy_data) {
	yhashmap_t	*hash;

#ifdef USE_BOEHM_GC
	hash = (yhashmap_t*)GC_MALLOC(sizeof(yhashmap_t));
	hash->buckets = (yhm_bucket_t*)GC_MALLOC(size * sizeof(yhm_bucket_t));
#else
	hash = (yhashmap_t*)calloc(1, sizeof(yhashmap_t));
	hash->buckets = (yhm_bucket_t*)calloc(size, sizeof(yhm_bucket_t));
#endif /* USE_BOEM_GC */
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
			element = bucket->elements;
			for (offset2 = 0; offset2 < bucket->nbr_elements; offset2++) {
				if (hashmap->destroy_func != NULL)
					hashmap->destroy_func(element->key, element->data, hashmap->destroy_data);
				old_element = element;
				element = element->next;
#ifdef USE_BOEHM_GC
				GC_FREE(old_element);
#else
				free(old_element);
#endif
			}
		}
	}
	/* remove buckets and the hash map itself */
#ifdef USE_BOEHM_GC
	GC_FREE(hashmap->buckets);
	GC_FREE(hashmap);
#else
	free(hashmap->buckets);
	free(hashmap);
#endif /* USE_BOEHM_GC */
}

/*
 * yhm_hash()
 * Compute the hash value of a key, using the SDBM algorithm.
 */
yhm_hash_value_t yhm_hash(const char *key) {
	yhm_hash_value_t	hash_value;

	for (hash_value = 0; *key; key++)
		hash_value = *key + (hash_value << 6) + (hash_value << 16) - hash_value;
	return (hash_value);
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

	/* resize the able if its load factor would excess the limit */
	load_factor = (float)(hashmap->used + 1) / hashmap->size;
	if (load_factor > YHM_MAX_LOAD_FACTOR) {
		yhm_resize(hashmap, (hashmap->size * 2));
		printf("done\n");
	}
	/* compute the key's hash value */
	hash_value = yhm_hash(key);
	hash_value %= hashmap->size;
	/* create the new element */
#ifdef USE_BOEHM_GC
	element = (yhm_element_t*)GC_MALLOC(sizeof(yhm_element_t));
#else
	element = (yhm_element_t*)calloc(1, sizeof(yhm_element_t));
#endif /* USE_BOEHM_GC */
	element->key = key;
	element->data = data;
	/* checking the bucket */
	bucket = &(hashmap->buckets[hash_value]);
	if (bucket->nbr_elements == 0) {
		/* add the first element */
		bucket->elements = element;
		element->previous = element->next = element;
	} else {
		/* update first and last elements */
		element->next = bucket->elements;
		element->previous = bucket->elements->previous;
		bucket->elements->previous->next = element;
		bucket->elements->previous = element;
	}
	/* update the bucket */
	bucket->nbr_elements++;
	/* update the hash map */
	hashmap->used++;
}

/*
 * yhm_search()
 * Search an element in an hash map, from its key.
 */
void *yhm_search(yhashmap_t *hashmap, const char *key) {
	yhm_hash_value_t	hash_value;
	yhm_bucket_t		*bucket;
	yhm_element_t		*element;
	size_t			offset;

	/* compute the key's hash value */
	hash_value = yhm_hash(key);
	hash_value %= hashmap->size;
	/* retreiving the bucket */
	bucket = &(hashmap->buckets[hash_value]);
	if (bucket->nbr_elements == 0)
		return (NULL);
	if (bucket->nbr_elements == 1)
		return (bucket->elements->data);
	element = bucket->elements;
	for (offset = 0; offset < bucket->nbr_elements; offset++) {
		if (!strcmp(key, element->key))
			return (element->data);
		element = element->next;
	}
	return (NULL);
}

/*
 * yhm_remove()
 * Remove an element from an hash map.
 */
char yhm_remove(yhashmap_t *hashmap, const char *key) {
	yhm_hash_value_t	hash_value;
	yhm_bucket_t		*bucket;
	yhm_element_t		*element;
	size_t			offset;
	char			found = 0;
	float			load_factor;

	/* compute the key's hash value */
	hash_value = yhm_hash(key);
	hash_value %= hashmap->size;
	/* retreiving the bucket */
	bucket = &(hashmap->buckets[hash_value]);
	if (bucket->nbr_elements == 0)
		return (0);
	if (bucket->nbr_elements == 1) {
		found = 1;
		if (hashmap->destroy_func)
			hashmap->destroy_func(bucket->elements->key, bucket->elements->data, hashmap->destroy_data);
#ifdef USE_BOEHM_GC
		GC_FREE(bucket->elements);
#else
		free(bucket->elements);
#endif /* USE_BOEHM_GC */
		bucket->elements = NULL;
	} else {
		element = bucket->elements;
		for (offset = 0; offset < bucket->nbr_elements; offset++) {
			printf("loop %d\n", offset);
			if (!strcmp(key, element->key)) {
				found = 1;
				if (offset == 0)
					bucket->elements = element->next;
				element->previous->next = element->next;
				element->next->previous = element->previous;
				/* call the destroy function */
				if (hashmap->destroy_func)
					hashmap->destroy_func(element->key, element->data, hashmap->destroy_data);
#ifdef USE_BOEHM_GC
				GC_FREE(element);
#else
				free(element);
#endif /* USE_BOEHM_GC */
				break;
			}
			element = element->next;
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
		element = bucket->elements;
		for (offset2 = 0; offset2 < bucket->nbr_elements; offset2++) {
			yhm_add(new_hashmap, element->key, element->data);
			old_element = element;
			element = element->next;
#ifdef USE_BOEHM_GC
			GC_FREE(old_element);
#else
			free(old_element);
#endif /* USE_BOEHM_GC */
		}
	}
	hashmap->size = size;
	/* moving the list of buckets */
	old_buckets = hashmap->buckets;
	hashmap->buckets = new_hashmap->buckets;
	/* free memory */
#ifdef USE_BOEHM_GC
	GC_FREE(old_buckets);
	GC_FREE(new_hashmap);
#else
	free(old_buckets);
	free(new_hashmap);
#endif /* USE_BOEHM_GC */
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
		element = bucket->elements;
		for (offset2 = 0; offset2 < bucket->nbr_elements; offset2++) {
			func(element->key, element->data, user_data);
			if (++nbr_processed == hashmap->used)
				return;
			element = element->next;
		}
	}
}
