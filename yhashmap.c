#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yhashtable.h"

/*
 * yht_new()
 * Creates a new hash table.
 */
yhashtable_t *yht_new(size_t size, yht_function_t destroy_func, void *destroy_data) {
	yhashtable_t	*hash;

#ifdef USE_BOEHM_GC
	hash = (yhashtable_t*)GC_MALLOC(sizeof(yhashtable_t));
	hash->buckets = (yht_bucket_t*)GC_MALLOC(size * sizeof(yht_bucket_t));
#else
	hash = (yhashtable_t*)calloc(1, sizeof(yhashtable_t));
	hash->buckets = (yht_bucket_t*)calloc(size, sizeof(yht_bucket_t));
#endif /* USE_BOEM_GC */
	hash->size = size;
	hash->used = 0;
	hash->destroy_func = destroy_func;
	hash->destroy_data = destroy_data;
	return (hash);
}

/*
 * yht_delete
 * Destroy an hash table.
 */
void yht_delete(yhashtable_t *hashtable) {
	/* remove elements */
	if (hashtable->used > 0) {
		size_t		offset, offset2;
		yht_bucket_t	*bucket;
		yht_element_t	*element, *old_element;

		for (offset = 0; offset < hashtable->size; offset++) {
			bucket = &(hashtable->buckets[offset]);
			element = bucket->elements;
			for (offset2 = 0; offset2 < bucket->nbr_elements; offset2++) {
				if (hashtable->destroy_func != NULL)
					hashtable->destroy_func(element->key, element->data, hashtable->destroy_data);
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
	/* remove buckets and the hash table itself */
#ifdef USE_BOEHM_GC
	GC_FREE(hashtable->buckets);
	GC_FREE(hashtable);
#else
	free(hashtable->buckets);
	free(hashtable);
#endif /* USE_BOEHM_GC */
}

/*
 * yht_hash()
 * Compute the hash value of a key, using the SDBM algorithm.
 */
yht_hash_value_t yht_hash(const char *key) {
	yht_hash_value_t	hash_value;

	for (hash_value = 0; *key; key++)
		hash_value = *key + (hash_value << 6) + (hash_value << 16) - hash_value;
	return (hash_value);
}

/*
 * yht_add
 * Add an element to an hash table.
 */
void yht_add(yhashtable_t *hashtable, char *key, void *data) {
	float			load_factor;
	yht_hash_value_t	hash_value;
	yht_bucket_t		*bucket;
	yht_element_t		*element;

	/* resize the able if its load factor would excess the limit */
	load_factor = (float)(hashtable->used + 1) / hashtable->size;
	if (load_factor > YHT_MAX_LOAD_FACTOR) {
		yht_resize(hashtable, (hashtable->size * 2));
		printf("done\n");
	}
	/* compute the key's hash value */
	hash_value = yht_hash(key);
	hash_value %= hashtable->size;
	/* create the new element */
#ifdef USE_BOEHM_GC
	element = (yht_element_t*)GC_MALLOC(sizeof(yht_element_t));
#else
	element = (yht_element_t*)calloc(1, sizeof(yht_element_t));
#endif /* USE_BOEHM_GC */
	element->key = key;
	element->data = data;
	/* checking the bucket */
	bucket = &(hashtable->buckets[hash_value]);
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
	/* update the hash table */
	hashtable->used++;
}

/*
 * yht_search()
 * Search an element in an hash table, from its key.
 */
void *yht_search(yhashtable_t *hashtable, const char *key) {
	yht_hash_value_t	hash_value;
	yht_bucket_t		*bucket;
	yht_element_t		*element;
	size_t			offset;

	/* compute the key's hash value */
	hash_value = yht_hash(key);
	hash_value %= hashtable->size;
	/* retreiving the bucket */
	bucket = &(hashtable->buckets[hash_value]);
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
 * yht_remove()
 * Remove an element from an hash table.
 */
char yht_remove(yhashtable_t *hashtable, const char *key) {
	yht_hash_value_t	hash_value;
	yht_bucket_t		*bucket;
	yht_element_t		*element;
	size_t			offset;
	char			found = 0;
	float			load_factor;

	/* compute the key's hash value */
	hash_value = yht_hash(key);
	hash_value %= hashtable->size;
	/* retreiving the bucket */
	bucket = &(hashtable->buckets[hash_value]);
	if (bucket->nbr_elements == 0)
		return (0);
	if (bucket->nbr_elements == 1) {
		found = 1;
		if (hashtable->destroy_func)
			hashtable->destroy_func(bucket->elements->key, bucket->elements->data, hashtable->destroy_data);
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
				if (hashtable->destroy_func)
					hashtable->destroy_func(element->key, element->data, hashtable->destroy_data);
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
		hashtable->used--;
		/* resize the table if its load factor will fall under the limit */
		load_factor = (float)(hashtable->used + 1) / hashtable->size;
		if (load_factor < YHT_MIN_LOAD_FACTOR)
			yht_resize(hashtable, (hashtable->size / 2));
	}
	return (found);
}

/*
 * yht_resize
 * Resize an hashtable.
 */
void yht_resize(yhashtable_t *hashtable, size_t size) {
	yhashtable_t	*new_hashtable;
	size_t		offset, offset2;
	yht_bucket_t	*bucket, *old_buckets;
	yht_element_t	*element, *old_element;

	new_hashtable = yht_new(size, NULL, NULL);
	for (offset = 0; offset < hashtable->size; offset++) {
		bucket = &(hashtable->buckets[offset]);
		if (bucket->nbr_elements == 0)
			continue;
		element = bucket->elements;
		for (offset2 = 0; offset2 < bucket->nbr_elements; offset2++) {
			yht_add(new_hashtable, element->key, element->data);
			old_element = element;
			element = element->next;
#ifdef USE_BOEHM_GC
			GC_FREE(old_element);
#else
			free(old_element);
#endif /* USE_BOEHM_GC */
		}
	}
	hashtable->size = size;
	/* moving the list of buckets */
	old_buckets = hashtable->buckets;
	hashtable->buckets = new_hashtable->buckets;
	/* free memory */
#ifdef USE_BOEHM_GC
	GC_FREE(old_buckets);
	GC_FREE(new_hashtable);
#else
	free(old_buckets);
	free(new_hashtable);
#endif /* USE_BOEHM_GC */
}

/*
 * yht_foreach
 * Apply a function on every elements of an hash table.
 */
void yht_foreach(yhashtable_t *hashtable, yht_function_t func, void *user_data) {
	size_t		offset, nbr_processed, offset2;
	yht_bucket_t	*bucket;
	yht_element_t	*element;

	for (offset = nbr_processed = 0; offset < hashtable->size; offset++) {
		bucket = &(hashtable->buckets[offset]);
		if (bucket->nbr_elements == 0)
			continue;
		element = bucket->elements;
		for (offset2 = 0; offset2 < bucket->nbr_elements; offset2++) {
			func(element->key, element->data, user_data);
			if (++nbr_processed == hashtable->used)
				return;
			element = element->next;
		}
	}
}
