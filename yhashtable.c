#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yhashtable.h"

/* *** definition of private functions *** */
void _yht_add(yhashtable_t *hashtable, yht_hash_value_t hash_value, char *key, void *data);

/*
 * yht_new()
 * Creates a new hash table.
 */
yhashtable_t *yht_new(yht_size_t size, yht_function_t destroy_func, void *destroy_data) {
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
	hash->items = NULL;
	hash->destroy_func = destroy_func;
	hash->destroy_data = destroy_data;
	return (hash);
}

/*
 * yht_delete
 * Destroy a hash table.
 */
void yht_delete(yhashtable_t *hashtable) {
	/* remove elements */
	if (hashtable->used > 0) {
		yht_list_t	*old_item, *item_ptr = hashtable->items;
		yht_element_t	*element;

		while (item_ptr) {
			element = item_ptr->element;
			if (hashtable->destroy_func != NULL)
				hashtable->destroy_func(element->hash_value, element->key, element->data, hashtable->destroy_data);
			old_item = item_ptr;
			item_ptr = item_ptr->next;
#ifdef USE_BOEHM_GC
			GC_FREE(old_item);
			GC_FREE(element);
#else
			free(old_item);
			GC_FREE(element);
#endif /* USE_BOEHM_GC */
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
 * yht_add_from_string
 * Add an element to a hash table, using a string key.
 */
void yht_add_from_string(yhashtable_t *hashtable, char *key, void *data) {
	_yht_add(hashtable, 0, key, data);
}

/*
 * yht_add_from_int
 * Add an element to a hash table, using an integer key.
 */
void yht_add_from_int(yhashtable_t *hashtable, size_t key, void *data) {
	_yht_add(hashtable, key, NULL, data);
}

/*
 * yhm_search()
 * Search an element in a hash map, from its key.
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
 * Remove an element from a hash map.
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
 * Resize a hash table.
 */
void yht_resize(yhashtable_t *hashtable, size_t size) {
	yht_hash_value_t	modulo_value;
	yht_bucket_t		*new_buckets, *bucket;
	yht_list_t		*item;
	yht_element_t		*element;

#ifdef USE_BOEHM_GC
	new_buckets = (yht_bucket_t*)GC_MALLOC(size * sizeof(yht_bucket_t));
#else
	new_buckets = (yht_bucket_t*)calloc(size, sizeof(yht_bucket_t));
#endif /* USE_BOEHM_GC */
	for (item = hashtable->items; item; item = item->next) {
		element = item->element;
		modulo_value = element->hash_value % size;
		bucket = &(new_buckets[modulo_value]);
		/* checking the new bucket */
		if (bucket->nbr_elements == 0) {
			element->previous = element->next = element;
			bucket->elements = element;
		} else {
			element->next = bucket->elements;
			element->previous = bucket->elements->previous;
			bucket->elements->previous->next = element;
			bucket->elements->previous = element;
		}
		bucket->nbr_elements++;
	}
	/* freeing old array of buckets */
#ifdef USE_BOEHM_GC
	GC_FREE(hashtable->buckets);
#else
	free(hashtable->buckets);
#endif /* USE_BOEHM_GC */
	/* swapping buckets */
	hashtable->buckets = new_buckets;
}

/*
 * yht_foreach
 * Apply a function on every elements of a hash table.
 */
void yht_foreach(yhashtable_t *hashtable, yht_function_t func, void *user_data) {
	yht_list_t	*item;
	yht_element_t	*element;

	for (item = hashtable->items; item; item = item->next) {
		element = item->element;
		func(element->hash_value, element->key, element->data, user_data);
	}
}

/* ****** PRIVATE FUNCTIONS ******* */
/*
 * _yht_add
 * Add an element to a hash table, using a string or an integer key.
 */
void _yht_add(yhashtable_t *hashtable, yht_hash_value_t hash_value, char *key, void *data) {
	float			load_factor;
	yht_hash_value_t	modulo_value;
	yht_bucket_t		*bucket;
	yht_element_t		*element;
	yht_list_t		*item;

	/* resize the table if its load factor would excess the limit */
	load_factor = (float)(hashtable->used + 1) / hashtable->size;
	if (load_factor > YHT_MAX_LOAD_FACTOR)
		yht_resize(hashtable, (hashtable->size * 2));
	/* compute the key's hash value */
	if (key != NULL)
		hash_value = yht_hash(key);
	modulo_value = hash_value % hashtable->size;
	/* checking the bucket */
	bucket = &(hashtable->buckets[modulo_value]);
	if (bucket->nbr_elements == 0) {
		/* add the first element */
#ifdef USE_BOEHM_GC
		element = (yht_element_t*)GC_MALLOC(sizeof(yht_element_t));
		item = (yht_list_t*)GC_MALLOC(sizeof(yht_list_t));
#else
		element = (yht_element_t*)calloc(1, sizeof(yht_element_t));
		item = (yht_list_t*)calloc(1, sizeof(yht_list_t));
#endif /* USE_BOEHM_GC */
		element->previous = element->next = element;
		/* add the element to the bucket */
		bucket->elements = element;
	} else {
		/* there is already some elements in the bucket, checking if the element exists and must be updated */
		size_t	offset;
		element = bucket->elements;
		for (offset = 0; offset < bucket->nbr_elements; offset++) {
			if (element->hash_value = hash_value &&
			    ((key == NULL && element->key == NULL) ||
			     (key != NULL && element->key != NULL && !strcmp(key, element->key)))) {
				/* an existing element was found */
				/* removing old data */
				if (hashtable->destroy_func != NULL)
					hashtable->destroy_func(element->hash_value, element->key, element->data, hashtable->destroy_data);
				/* updating the element */
				element->key = key;
				element->data = data;
				return;
			}
		}
		/* no element was already existing with this key */
#ifdef USE_BOEHM_GC
		element = (yht_element_t*)GC_MALLOC(sizeof(yht_element_t));
		item = (yht_list_t*)GC_MALLOC(sizeof(yht_list_t));
#else
		element = (yht_element_t*)calloc(1, sizeof(yht_element_t));
		item = (yht_list_t*)calloc(1, sizeof(yht_list_t));
#endif /* USE_BOEHM_GC */
		/* add the element to the bucket */
		element->next = bucket->elements;
		element->previous = bucket->elements->previous;
		bucket->elements->previous->next = element;
		bucket->elements->previous = element;
	}
	/* filling the element and the item */
	element->hash_value = hash_value;
	element->key = key;
	element->data = data;
	element->item = item;
	item->element = element;
	/* update the bucket */
	bucket->nbr_elements++;
	/* update the hash table */
	hashtable->used++;
	/* add the item to the list */
	if (hashtable->items == NULL)
		hashtable->items = item;
	else {
		item->next = hashtable->items;
		item->previous = hashtable->items->previous;
		hashtable->items->previous->next = item;
		hashtable->items->previous = item;
	}
}

