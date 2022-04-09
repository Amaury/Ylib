#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yhashtable.h"

/* *** definition of private functions *** */
static bool _yhashtable_remove(yhashtable_t *hashtable, yhash_value_t hash_value,
                               const char *key, bool try_to_destroy);
static void *_yhashtable_search(yhashtable_t *hashtable, yhash_value_t hash_value,
                                const char *key);
static void _yhashtable_add(yhashtable_t *hashtable, yhash_value_t hash_value, char *key,
                            void *data);

/*
 * yhashtable_new()
 * Creates a new hash table.
 */
yhashtable_t *yhashtable_new(yhashtable_size_t size, yhashtable_function_t destroy_func, void *destroy_data) {
	yhashtable_t *hash;

	hash = malloc0(sizeof(yhashtable_t));
	hash->buckets = calloc0(size, sizeof(yhashtable_bucket_t));
	hash->size = size;
	hash->used = 0;
	hash->items = NULL;
	hash->next_offset = 0;
	hash->destroy_func = destroy_func;
	hash->destroy_data = destroy_data;
	return (hash);
}

/*
 * yhashtable_delete
 * Destroy a hash table.
 */
void yhashtable_delete(yhashtable_t *hashtable) {
	/* remove elements */
	if (hashtable->used > 0) {
		size_t offset;
		yhashtable_list_t *old_item, *item;
		yhashtable_element_t *element;

		item = hashtable->items;
		for (offset = 0, item = hashtable->items;
		     offset < hashtable->used;
		     offset++) {
			element = item->element;
			if (hashtable->destroy_func != NULL)
				hashtable->destroy_func(element->hash_value, element->key, element->data, hashtable->destroy_data);
			old_item = item;
			item = item->next;
			YFREE(old_item);
			YFREE(element);
		}
	}
	/* remove buckets and the hash table itself */
	YFREE(hashtable->buckets);
	YFREE(hashtable);
}

/*
 * yhashtable_add_from_string
 * Add an element to a hash table, using a string key.
 */
void yhashtable_add_from_string(yhashtable_t *hashtable, char *key, void *data) {
	_yhashtable_add(hashtable, 0, key, data);
}

/*
 * yhashtable_add_from_int
 * Add an element to a hash table, using an integer key.
 */
void yhashtable_add_from_int(yhashtable_t *hashtable, size_t key, void *data) {
	if (key >= hashtable->next_offset)
		hashtable->next_offset = key + 1;
	_yhashtable_add(hashtable, key, NULL, data);
}

/*
 * yhashtable_push_data
 * Add an element at the end of a hash table.
 */
void yhashtable_push_data(yhashtable_t *hashtable, void *data) {
	yhashtable_add_from_int(hashtable, hashtable->next_offset, data);
}

/*
 * yhashtable_search_from_string
 * Search an element in a hash table, from its string key.
 */
void *yhashtable_search_from_string(yhashtable_t *hashtable, const char *key) {
	return (_yhashtable_search(hashtable, 0, key));
}

/*
 * yhashtable_search_from_int
 * Search an element in a hash table, from its integer key.
 */
void *yhashtable_search_from_int(yhashtable_t *hashtable, size_t key) {
	return (_yhashtable_search(hashtable, key, NULL));
}

/*
 * yhashtable_search_from_hashed_string
 * Search an element in a hash table, from its hashed string key.
 */
void *yhashtable_search_from_hashed_string(yhashtable_t *hashtable, size_t hash_value, const char *key) {
	return (_yhashtable_search(hashtable, hash_value, key));
}

/*
 * yhashtable_pop_data
 * Remove the last element of a hash table and returns it.
 */
void *yhashtable_pop_data(yhashtable_t *hashtable) {
	yhashtable_element_t *element;
	void *data;

	if (hashtable->items == NULL)
		return (NULL);
	element = hashtable->items->previous->element;
	data = element->data;
	_yhashtable_remove(hashtable, element->hash_value, element->key, false);
	return (data);
}

/*
 * yhashtable_remove_from_string
 * Remove an element from a hash table, using its string key.
 */
bool yhashtable_remove_from_string(yhashtable_t *hashtable, const char *key) {
	return (_yhashtable_remove(hashtable, 0, key, true));
}

/*
 * yhashtable_remove_from_int
 * Remove an element from a hash table, using its integer key.
 */
bool yhashtable_remove_from_int(yhashtable_t *hashtable, size_t key) {
	return (_yhashtable_remove(hashtable, key, NULL, true));
}

/*
 * yhashtable_resize
 * Resize a hash table.
 */
void yhashtable_resize(yhashtable_t *hashtable, size_t size) {
	size_t offset;
	size_t modulo_value;
	yhashtable_bucket_t *new_buckets, *bucket;
	yhashtable_list_t *item;
	yhashtable_element_t *element;

	new_buckets = (yhashtable_bucket_t*)YCALLOC(size, sizeof(yhashtable_bucket_t));
	for (offset = 0, item = hashtable->items;
	     offset < hashtable->used;
	     offset++, item = item->next) {
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
	YFREE(hashtable->buckets);
	/* swapping buckets */
	hashtable->buckets = new_buckets;
	hashtable->size = size;
}

/*
 * yhashtable_foreach
 * Apply a function on every elements of a hash table.
 */
void yhashtable_foreach(yhashtable_t *hashtable, yhashtable_function_t func, void *user_data) {
	size_t offset;
	yhashtable_list_t *item;
	yhashtable_element_t *element;

	for (offset = 0, item = hashtable->items;
	     offset < hashtable->used;
	     offset++, item = item->next) {
		element = item->element;
		func(element->hash_value, element->key, element->data, user_data);
	}
}

/* ****** PRIVATE FUNCTIONS ******* */
/**
 * _yhashtable_remove
 * Remove an element from a hash table, using a string or an integer key.
 */
static bool _yhashtable_remove(yhashtable_t *hashtable, yhash_value_t hash_value,
                               const char *key, bool try_to_destroy) {
	size_t modulo_value;
	yhashtable_bucket_t *bucket;
	yhashtable_element_t *element;
	yhashtable_list_t *item;
	size_t offset;
	bool found = false;
	float load_factor;

	/* compute the key's hash value */
	if (key != NULL)
		hash_value = yhash_compute(key);
	modulo_value = hash_value % hashtable->size;
	/* retreiving the bucket */
	bucket = &(hashtable->buckets[modulo_value]);
	if (bucket->nbr_elements == 0)
		return (false);
	/* searching in the bucket */
	for (offset = 0, element = bucket->elements;
	     offset < bucket->nbr_elements;
	     offset++, element = element->next) {
		if (element->hash_value == hash_value &&
		    ((key == NULL && element->key == NULL) ||
		     (key != NULL && element->key != NULL && !strcmp(key, element->key)))) {
			found = true;
			if (try_to_destroy && hashtable->destroy_func != NULL)
				hashtable->destroy_func(element->hash_value, element->key, element->data, hashtable->destroy_data);
			item = element->item;
			if (hashtable->used == 1)
				YFREE(hashtable->items);
			else {
				item->next->previous = item->previous;
				item->previous->next = item->next;
				YFREE(item);
			}
			if (bucket->nbr_elements == 1)
				YFREE(bucket->elements);
			else {
				element->next->previous = element->previous;
				element->previous->next = element->next;
				YFREE(element);
			}
			break;
		}
	}
	if (found) {
		bucket->nbr_elements--;
		hashtable->used--;
		/* resize the map if its load factor will fall under the limit */
		load_factor = (float)(hashtable->used + 1) / hashtable->size;
		if (load_factor < YHT_MIN_LOAD_FACTOR)
			yhashtable_resize(hashtable, (hashtable->size / 2));
	}
	return (found);
}

/**
 * _yhashtable_search
 * Search an element in a hash table, using a string or an integer key.
 */
static void *_yhashtable_search(yhashtable_t *hashtable, yhash_value_t hash_value,
                                const char *key) {
	size_t modulo_value;
	yhashtable_bucket_t *bucket;
	yhashtable_element_t *element;
	size_t offset;

	/* compute the key's hash value if necessary */
	if (hash_value == 0 && key != NULL)
		hash_value = yhash_compute(key);
	modulo_value = hash_value % hashtable->size;
	/* retreiving the bucket */
	bucket = &(hashtable->buckets[modulo_value]);
	if (bucket->nbr_elements == 0)
		return (NULL);
	/* searching in the bucket's elements */
	element = bucket->elements;
	for (offset = 0, element = bucket->elements;
	     offset < bucket->nbr_elements;
	     offset++, element = element->next) {
		if (element->hash_value == hash_value &&
		    ((key == NULL && element->key == NULL) ||
		     (key != NULL && element->key != NULL && !strcmp(key, element->key))))
			return (element->data);
	}
	return (NULL);
}

/*
 * _yhashtable_add
 * Add an element to a hash table, using a string or an integer key.
 */
static void _yhashtable_add(yhashtable_t *hashtable, yhash_value_t hash_value,
                            char *key, void *data) {
	float load_factor;
	size_t modulo_value;
	yhashtable_bucket_t *bucket;
	yhashtable_element_t *element;
	yhashtable_list_t *item;

	/* resize the table if its load factor would excess the limit */
	load_factor = (float)(hashtable->used + 1) / hashtable->size;
	if (load_factor > YHT_MAX_LOAD_FACTOR)
		yhashtable_resize(hashtable, (hashtable->size * 2));
	/* compute the key's hash value */
	if (key != NULL)
		hash_value = yhash_compute(key);
	modulo_value = hash_value % hashtable->size;
	/* checking the bucket */
	bucket = &(hashtable->buckets[modulo_value]);
	if (bucket->nbr_elements == 0) {
		/* create the first element */
		element = malloc0(sizeof(yhashtable_element_t));
		item = malloc0(sizeof(yhashtable_list_t));
		element->previous = element->next = element;
		/* add the element to the bucket */
		bucket->elements = element;
	} else {
		/* there is already some elements in the bucket, checking if the element exists and must be updated */
		size_t	offset;

		for (offset = 0, element = bucket->elements;
		     offset < bucket->nbr_elements;
		     offset++, element = element->next) {
			if (element->hash_value == hash_value &&
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
		element = malloc0(sizeof(yhashtable_element_t));
		item = malloc0(sizeof(yhashtable_list_t));
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
	if (hashtable->items == NULL) {
		hashtable->items = item;
		item->next = item->previous = item;
	} else {
		item->next = hashtable->items;
		item->previous = hashtable->items->previous;
		hashtable->items->previous->next = item;
		hashtable->items->previous = item;
	}
}

