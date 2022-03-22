#include "ytable.h"

/* ************ PRIVATE DEFINITIONS AND MACROS ************ */
/** @define _YTABLE_DEFAULT_SIZE Default size of yvectors. */
#define _YTABLE_DEFAULT_SIZE		8
/** @define _YTABLE_SIZE Compute the size of a new yarray's buffer. */
#define _YARRAY_SIZE(s)			COMPUTE_SIZE((s), _YTABLE_DEFAULT_SIZE)
/** @define _YTABLE_MAX_LOAD_FACTOR Maximum load factor of a hash table before increasing it. */
#define _YTABLE_MAX_LOAD_FACTOR		0.7
/** @define _YTABLE_MIN_LOAD_FACTOR Minimum load factor of a hash table before reducing it. */
#define _YTABLE_MIN_LOAD_FACTOR		0.25
/** @define _YTABLE_DEFAULT_BUCKET_SIZE Default size for buckets. */
#define _YTABLE_DEFAULT_BUCKET_SIZE	4
/** @define _YTABLE_HAS_NUMERIC_KEY	Returns 1 if an element has a numeric key. */
#define _YTABLE_HAS_NUMERIC_KEY(h)	((h) & ((uint64_t)1 << 63)) // 0b10...00 (64 bits)
/** @define _YTABLE_HAS_STRING_KEY	Returns 1 if an element has a string key. */
#define _YTABLE_HAS_STRING_KEY(h)	((h) & ((uint64_t)1 << 62)) // 0b01...00 (64 bits)
/** @define _YTABLE_HAS_NO_KEY		Returns 1 if an element has no defined key. */
#define _YTABLE_HAS_NO_KEY(h)		(!((h) & ((uint64_t)3 << 62))) // 0b11...00 (64 bits)
/** @define _YTABLE_HASH_VALUE		Extract the real hash value from a hash field. */
#define _YTABLE_HASH_VALUE(h)		((h) & 0x3FFFFFFFFFFFFFFF) // 0b0011...11 (64 bits)
/** @define _YTABLE_SET_NUMERIC_KEY	Set the numeric key bit to a hash value. */
#define _YTABLE_SET_NUMERIC_KEY(h)	(_YTABLE_HASH_VALUE(h) | ((uint64_t)1 << 63)) // 0b10..00
/** @define _YTABLE_SET_STRING_KEY	Set the string key bit to a hash value. */
#define _YTABLE_SET_STRING_KEY(h)	(_YTABLE_HASH_VALUE(h) | ((uint64_t)1 << 62)) // 0b01..00
/** @define _YTABLE_SET_NO_KEY		Set no key bit to a hash value. */
#define _YTABLE_SET_NO_KEY		(_YTABLE_HASH_VALUE(h))

/* ************ PRIVATE STRUCTURES AND TYPES ************** */
/**
 * @typedef	_ytable_element_t
 *		Storage structure of an element.
 * @field	data		Pointer to the stored data.
 * @field	key		String key.
 * @field	hash_value	- 64th bit: 1 if numeric key (0 if keyless array-like)
 *				- 63rd bit: 1 if string key (0 if keyless array-like)
 *				- 62 bits: numéric key or computed hash from the string key
 */
typedef struct {
	void *data;
	const char *key;
	uint64_t hash_value;
} _ytable_element_t;
/**
 * @typedef	_ytable_t
 *		Real ytable _structure.
 * @field	length		Number of stored elements.
 * @field	array_size	Allocated size of the array.
 * @field	next_index	Next numeric index.
 * @field	elements	Array of table's elements.
 * @field	buckets		Array of bucket lists.
 * @field	delete_function	Pointer to a function used to delete elements.
 * @field	delete_data	Pointer to data pass to the delete function.
 */
typedef struct {
	uint32_t length;
	uint32_t array_size;
	uint64_t next_index;
	_ytable_element_t *elements;
	uint32_t **buckets;
	ytable_function_t delete_function;
	void *delete_data;
} _ytable_t;

/* ********** DECLARATION OF PRIVATE FUNCTIONS ********** */
static ystatus_t _ytable_free_element_data(_ytable_t *t, uint32_t element_offset);
static ystatus_t _ytable_instanciate(_ytable_t *t);
static ystatus_t _ytable_instanciate_hashmap(_ytable_t *t);
static ystatus_t _ytable_expand(_ytable_t *t, uint16_t size, bool array_only);
static ystatus_t _ytable_add_element_to_hashmap(_ytable_t *t, _ytable_element_t *element,
                                                uint32_t element_offset);
static yres_int_t _ytable_extract_element_from_hashmap(_ytable_t *t, _ytable_element_t *element,
                                                       const char *key, uint64_t index);

/* ************ CREATION/DELETION FUNCTIONS ************* */
/* Create a new simple ytable. */
ytable_t *ytable_new(void) {
	_ytable_t *table = malloc0(sizeof(_ytable_t));
	if (!table)
		return (NULL);
	table->array_size = _YTABLE_DEFAULT_SIZE;
	return ((ytable_t*)table);
}
/* Create a new ytable by giving its size. */
ytable_t *ytable_create(size_t size, ytable_function_t delete_function, void *delete_data) {
	_ytable_t *t = malloc0(sizeof(_ytable_t));
	if (!t)
		return (NULL);
	*t = (_ytable_t){
		.array_size = COMPUTE_SIZE(size, _YTABLE_DEFAULT_SIZE),
		.delete_function = delete_function,
		.delete_data = delete_data,
	};
	return ((ytable_t*)t);
}
/* Define the delete function. */
ytable_t *ytable_set_delete_function(ytable_t *table, ytable_function_t delete_function,
                                     void *delete_data) {
	if (table)
		return (NULL);
	_ytable_t *t = (_ytable_t*)table;
	t->delete_function = delete_function;
	t->delete_data = delete_data;
	return (table);
}
/* Destroy a ytable. */
void ytable_free(ytable_t *table) {
	if (!table)
		return;
	_ytable_t *t = (_ytable_t*)table;
	if (t->buckets) {
		for (uint32_t offset = 0; offset < t->array_size; ++offset) {
			free0(t->buckets[offset]);
		}
		free0(t->buckets);
	}
	if (t->elements && t->delete_function) {
		for (size_t offset = 0; offset < t->length; ++offset) {
			_ytable_element_t *e = &t->elements[offset];
			t->delete_function(e->hash_value, (char*)e->key, e->data,
			                   t->delete_data);
		}
	}
	free0(t->elements);
	free0(t);
}

/* ********** ARRAY-LIKE FUNCTIONS ********** */
/* Add an element at the end of a ytable (used as an array). */
ystatus_t ytable_add(ytable_t *table, void *data) {
	if (!table)
		return (YEINVAL);
	_ytable_t *t = (_ytable_t*)table;
	// if the element's offset would be different than the table's next index,
	// do as it has a numeric key
	if (t->length != t->next_index) {
		return (ytable_set_index(table, t->next_index, data));
	}
	// instanciate the array if needed
	RETURN_IF_ERR(_ytable_instanciate(t));
	// expand the array if needed
	RETURN_IF_ERR(_ytable_expand(t, 1, false));
	// add the element at the end of the array
	_ytable_element_t *element = &t->elements[t->length];
	*element = (_ytable_element_t){0};
	element->data = data;
	// increment counters
	++t->length;
	++t->next_index;
	return (YENOERR);
}
/* Add an element at the beginning of a ytable (used as an array). */
ystatus_t ytable_push(ytable_t *table, void *data) {
	if (!table)
		return (YEINVAL);
	_ytable_t *t = (_ytable_t*)table;
	_ytable_element_t *element;
	// instanciate and expand the array if needed
	RETURN_IF_ERR(_ytable_instanciate(t));
	RETURN_IF_ERR(_ytable_expand(t, 1, false));
	// check if existing elements must be moved in the array
	if (!t->length)
		goto empty_array;
	// loop on array elements to move them, starting by the last one
	for (uint32_t new_offset = t->length; new_offset; --new_offset) {
		uint32_t old_offset = offset - 1;
		// move the element in the array
		t->elements[new_offset] = t->elements[old_offset];
		// if the element has a numeric key and this key is equal to the element's
		// new offset, remove it from the hashmap part of the table (the element
		// becomes like it has a numeric key from the beginning)
		_ytable_element_t *element = &t->elements[new_offset];
		if (_YTABLE_HAS_NUMERIC_KEY(element->hash_value) &&
		    _YTABLE_HASH_VALUE(element->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(t, element, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			element->hash_value = 0;
		}
	}
empty_array:
	// add the new element
	element = &t->elements[0];
	*element = (_ytable_element_t){0};
	element->data = data;
	++t->length;
	++t->next_index;
	return (YENOERR);
}
/* Remove the last element of a ytable and return it. */
void *ytable_pop(ytable_t *table) {
	_ytable_t *t = (_ytable_t*)table;
	if (!table || !t->length)
		return (NULL);
	--t->length;
	_ytable_element_t *element = &t->elements[t->length];
	// if the element is in the hashmap, remove it
	if (element->hash_value) {
		_ytable_extract_element_from_hashmap(t, element, NULL, 0);
	}
	return (element->data);
}
/* Remove the first element of a ytable and return it. */
void *ytable_shift(ytable_t *table) {
	_ytable_t *t = (_ytable_t*)table;
	if (!table || !t->length)
		return (NULL);
	_ytable_element_t *element = &t->elements[0];
	// if (the element is in the hashmap, remove it
	if (element->hash_value) {
		_ytable_extract_element_from_hashmap(t, element, NULL, 0);
	}
	// loop on array elements to move them, starting by the second one
	void *data = element->data;
	for (uint32_t offset = 1; offset < t->length; ++offset) {
		uint32_t new_offset = offset - 1;
		// move the element in the array
		t->elements[new_offset] = t->elements[offset];
		element = &t->elements[new_offset];
		// if the element has a numeric key and this key is equal to the element's
		// new offset, remove it from the hashmap part of the table (the element
		// becomes likes it has a numeric key from the beginning)
		if (_YTABLE_HAS_NUMERIC_KEY(element->hash_value) &&
		    _YTABLE_HASH_VALUE(element->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(t, element, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			element->hash_value = 0;
		}
	}
	--t->length;
	return (data);
}

/* ********** INDEXED FUNCTIONS ********** */
/* Tell if a given key exists in a ytable. */
bool ytable_index_exists(ytable_t *table, uint64_t index) {
	_ytable_t *t = (_ytable_t*)table;
	if (!table || !t->length)
		return (false);
	// search for a direct index
	_ytable_element_t *elem = &t->elements[index];
	if (_YTABLE_HAS_NO_KEY(elem->hash_value))
		return (true);
	// search for an hashed index
	if (!t->buckets)
		return (false);
	uint32_t modulo_value = MODULO_POW2(index, t->array_size);
	uint32_t *bucket = t->buckets[modulo_value];
	if (!bucket)
		return (false);
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t compare_offset = bucket[bucket_offset + 2];
		elem = &t->elements[compare_offset];
		// check if hash values are equal
		if (_YTABLE_HAS_NUMERIC_KEY(elem->hash_value) &&
		    _YTABLE_HASH_VALUE(elem->hash_value) == index)
			return (true);
	}
	// not found
	return (false);
}
/* Add an element in a ytable using an index. */
ystatus_t ytable_set_index(ytable_t *table, uint64_t index, void *data) {
	if (!table)
		return (YEINVAL);
	_ytable_t *t = (_ytable_t*)table;
	// if the given index is the next in numeric order, manage the table as an array
	if (index == t->next_index && index == t->length)
		return (ytable_add(table, data));
	// instanciate the array if needed
	RETURN_IF_ERR(_ytable_instanciate(t));
	// if the array is empty, no need to check if an element must be overwritten
	if (!t->length)
		goto empty_array;
	// check if the given index is corresponding to an array-like indexed element
	if (index >= t->length)
		goto not_array_element;
	_ytable_element_t *element = &t->elements[index];
	if (_YTABLE_HAS_NO_KEY(element->hash_value) ||
	    (_YTABLE_HAS_NUMERIC_KEY(element->hash_value) &&
	     _YTABLE_HASH_VALUE(element->hash_value) == index)) {
		// a previous element was found => overwrite
		// starts by removing the ancient element's data
		RETURN_IF_ERR(_ytable_free_element_data(t, index));
		// then overwrite it
		element->data = data;
		return (YENOERR);
	}
not_array_element:
	// search if an element already exists in the hashmap with the same hash value
	if (!t->length || !t->buckets)
		goto nobucket;
	uint64_t hash_value = index;
	uint32_t modulo_value = MODULO_POW2(hash_value, t->array_size);
	uint32_t *bucket = t->buckets[modulo_value];
	if (!bucket)
		goto nobucket;
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t compare_offset = bucket[bucket_offset + 2];
		element = &t->elements[compare_offset];
		// check if hash values are equal
		if (!_YTABLE_HAS_NUMERIC_KEY(element->hash_value) ||
		    _YTABLE_HASH_VALUE(element->hash_value) != hash_value)
			continue;
		// same hash value => overwrite
		// starts by removing the ancient element's data
		RETURN_IF_ERR(_ytable_free_element_data(t, compare_offset));
		// then overwrite it
		element->data = data;
		return (YENOERR);
	}
empty_array:
nobucket: ;
	bool hashed_element = (index != t->next_index || index != t->length) ? true : false;
	// expand the array if needed
	RETURN_IF_ERR(_ytable_expand(t, 1, hashed_element));
	// add the element at the end of the array
	element = &t->elements[t->length];
	*element = (_ytable_element_t){0};
	element->data = data;
	// if the element's index is different of the table's next index,
	// add it to the hashmap part of the table
	if (hashed_element) {
		element->hash_value = _YTABLE_SET_NUMERIC_KEY(index);
		RETURN_IF_ERR(_ytable_instanciate_hashmap(t));
		RETURN_IF_ERR(_ytable_add_element_to_hashmap(t, element, t->length));
	}
	// increment counters
	++t->length;
	index += 1;
	t->next_index = MAX(t->next_index, index);
	return (YENOERR);
}
/* Extract an element from its index and return it. */
void *ytable_extract_index(ytable_t *table, uint64_t index) {
	_ytable_t *t = (_ytable_t*)table;
	if (!table || !t->length)
		return (NULL);
	// search for a direct index
	_ytable_element_t *elem = &t->elements[index];
	if (!_YTABLE_HAS_NO_KEY(elem->hash_value))
		goto not_array;
	// decrement array size
	--t->length;
	// check if it's the last element of the array
	if (index == t->length)
		return (elem->data);
	// not the last element, shift all other elements
	void *data = elem->data;
	goto shift_elements;
not_array: ;
	// extract the element from the hashmap
	yres_int_t res = _ytable_extract_element_from_hashmap(t, NULL, NULL, index);
	if (YRES_STATUS(res) != YENOERR)
		return (NULL);
	index = YRES_VAL(res);
	elem = &t->elements[ndex];
	// decrement array size
	--t->length;
	// check if its the last element of the array
	if (res.value == t->length)
		return (elem->data);
	// not the last element, shift all other elements
	data = elem->data;
shift_elements:
	// shift elements in the array
	for (uint32_t new_offset = index; new_offset < t->length; ++new_offset) {
		uint32_t old_offset = new_offset + 1;
		elem = &t->elements[new_offset];
		// if the element has a numeric key and this key is equal to the element"s
		// new offset, remove it from the hashmap part of the table (the element
		// becomes likes it has a numeric key from the beginning)
		if (_YTABLE_HAS_NUMERIC_KEY(element->hash_value) &&
		    _YTABLE_HASH_VALUE(element->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(t, element, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			element->hash_value = 0;
		}
		// move the element in the array
		t->elements[new_offset] = t->elements[old_offset];
	}
	return (data);
}
/* Remove an element from its index. */
ystatus_t ytable_remove_index(ytable_t *table, uint64_t index) {
	uint32_t final_offset;
	if (!table)
		return (YEINVAL);
	_ytable_t *t = (_ytable_t*)table;
	if (!t->length)
		return (YEUNDEF);
	// check if the given index is corresponding to an array-like indexed element
	if (index >= t->length)
		goto not_array_element;
	_ytable_element_t *element = &t->elements[index];
	if (_YTABLE_HAS_NO_KEY(element->hash_value)) {
		RETURN_IF_ERR(_ytable_free_element_data(t, index));
		final_offset = index;
		goto shift_elements;
	}
not_array_element:
	// search for an element in the hashmap with the same hash value
	if (!t->buckets)
		return (YEUNDEF);
	uint32_t = modulo_value = MODULO_POW2(index, t->array_size);
	uint32_t *bucket = t->buckets[modulo_value];
	if (!bucket)
		return (YEUNDEF);
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t compare_offset = bucket[bucket_offset + 2];
		element = &t->elements[compare_offset];
		// check if hash values are equal
		if (!_YTABLE_HAS_NUMERIC_KEY(element->hash_value) ||
		    _YTABLE_HASH_VALUE(element->hash_value) != index)
			continue;
		// same hash value => remove
		RETURN_IF_ERR(_ytable_free_element_data(t, compare_offset));
		final_offset = compare_offset;
		goto shift_elements;
	}
	return (YENOERR);
shift_elements:
	for (uint32_t loop_offset = final_offset; loop_offset > final_offset; --loop_offset) {
		_ytable_element_t *shift_element = &t->elements[loop_offset];
		if (!_YTABLE_HAS_NO_KEY(shift_elem->hash_value)) {

		}
		t->elements[loop_offset - 1] = t->elements
	}
}

/* ********** KEYED FUNCTIONS ********** */
/* Tell if a given key exists in a ytable. */
bool ytable_key_exists(ytable_t *table, const char *key) {
	_ytable_t *t = (_ytable_t*)table;
	if (!table || !t->length || !t->buckets)
		return (false);
	// search for bucket
	uint64_t hash_value = yhash_compute(key);
	uint32_t modulo_value = MODULO_POW2(hash_value, t->array_size);
	uint32_t *bucket = t->buckets[modulo_value];
	if (!bucket)
		return (false);
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t compare_offset = bucket[bucket_offset + 2];
		_ytable_element_t *elem = &t->elements[compare_offset];
		// check if hash values are equal
		if (_YTABLE_HAS_STRING_KEY(elem->hash_value) &&
		    _YTABLE_HASH_VALUE(elem->hash_value) == hash_value &&
		    !strcmp0(elem->key, key))
			return (true);
	}
	// not found
	return (false);
}
/* Add an element in a ytable using a string key. */
ystatus_t ytable_set_key(ytable_t *table, const char *key, void *data) {
	if (!table || !key)
		return (YEINVAL);
	_ytable_t *t = (_ytable_t*)table;
	// if the key is a numeric string, manage it as a numeric insert
	if (ys_is_numeric(key)) {
		uint64_t index = (uint64_t)atol(key);
		return (ytable_set_index(table, index, data));
	}
	// instanciate the array if needed
	RETURN_IF_ERR(_ytable_instanciate(t));
	// compute the hash value
	uint64_t hash_value = yhash_compute(key);
	// if the array is empty, no need to check if an element must be overwritten
	if (!t->length || !t->buckets)
		goto nobucket;
	uint32_t modulo_value = MODULO_POW2(hash_value, t->array_size);
	uint32_t *bucket = t->buckets[modulo_value];
	if (!bucket)
		goto nobucket;
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t elem_offset = bucket[bucket_offset + 2];
		_ytable_element_t *elem = &t->elements[elem_offset];
		// check if keys are equal
		if (!_YTABLE_HAS_STRING_KEY(elem->hash_value) ||
		    _YTABLE_HASH_VALUE(elem->hash_value) != hash_value ||
		    strcmp0(elem->key, key)) {
			// keys are different
			continue;
		}
		// same keys => overwrite
		// starts by removing the ancient element's data
		RETURN_IF_ERR(_ytable_free_element_data(t, elem_offset));
		// then overwrite it
		elem->data = data;
		return (YENOERR);
	}
nobucket:
	// expand the array if needed
	RETURN_IF_ERR(_ytable_expand(t, 1, true));
	// add the element at the end of the array
	_ytable_element_t *element = &t->elements[t->length];
	*element = (_ytable_element_t){
		.data = data,
		.key = key,
		.hash_value = _YTABLE_SET_STRING_KEY(hash_value)
	};
	// add the element in the hashmap part of the table
	RETURN_IF_ERR(_ytable_instanciate_hashmap(t));
	RETURN_IF_ERR(_ytable_add_element_to_hashmap(t, element, t->length));
	// increment counters
	++t->length;
	return (YENOERR);
}

/* ********** GENERAL FUNCTIONS ********** */
/* Return the used length of a ytable. */
uint32_t ytable_length(ytable_t *table) {
	_ytable_t *t = (_ytable_t*)table;
	if (!t)
		return (0);
	return (t->length);
}
/* Apply a function on every elements of a ytable. */
ystatus_t ytable_foreach(ytable_t *table, ytable_function_t func, void *user_data) {
	_ytable_t *t = (_ytable_t*)table;
	if (!t)
		return (YEINVAL);
	if (!t->length || !func)
		return (YENOERR); 
	// loop on all elements
	for (uint32_t offset = 0; offset < t->length; ++offset) {
		_ytable_element_t *elem = &t->elements[offset];
		uint64_t hash = _YTABLE_HAS_NUMERIC_KEY(elem->hash_value) ?
		                _YTABLE_HASH_VALUE(elem->hash_value) : offset;

		RETURN_IF_ERR(func(hash, (char*)elem->key, elem->data, user_data));
	}
	return (YENOERR);
}

/* ********** PRIVATE FUNCTIONS ********** */
/* Delete the data pointed by an element. */
static ystatus_t _ytable_free_element_data(_ytable_t *t, uint32_t element_offset) {
	if (!t->delete_function)
		return (YENOERR);
	_ytable_element_t *e = &t->elements[element_offset];
	return (t->delete_function(element_offset, (char*)e->key, e->data, t->delete_data));
}
/* Instanciate the buffers of a new empty ytable, if needed. */
static ystatus_t _ytable_instanciate(_ytable_t *t) {
	if (t->elements)
		return (YENOERR);
	if (!t->array_size)
		return (YEINVAL);
	t->elements = calloc0(t->array_size, sizeof(_ytable_element_t));
	if (!t->elements)
		return (YENOMEM);
	return (YENOERR);
}
/* Instanciate the hashmap of a ytable, if needed. */
static ystatus_t _ytable_instanciate_hashmap(_ytable_t *t) {
	if (t->buckets)
		return (YENOERR);
	t->buckets = calloc0(t->array_size, sizeof(uint32_t*));
	if (!t->buckets)
		return (YENOERR);
	return (YENOERR);
}
/*
 * @function	_ytable_expand
 *		Expand the buffers of a ytable if needed.
 * @param	t		Pointer to the table.
 * @param	size		Number of elements to add.
 * @param	hash_expansion	True if the expansion is for a hashed element.
 * @return	YENOERR if OK.
 */
static ystatus_t _ytable_expand(_ytable_t *t, uint16_t size, bool hash_expansion) {
	uint32_t new_length = t->length + size;
	uint32_t new_array_size;
	// checkings
	if (!hash_expansion) {
		// expansion of the array, not of the hashmap
		if (new_length <= t->array_size)
			return (YENOERR);
		new_array_size = COMPUTE_SIZE(new_length, _YTABLE_DEFAULT_SIZE);
	} else {
		// expansion of the hashmap
		float load_factor = (float)new_length / MAX(t->array_size, _YTABLE_DEFAULT_SIZE);
		if (load_factor <= _YTABLE_MAX_LOAD_FACTOR)
			return (YENOERR);
		// compute the new array_size
		new_array_size = COMPUTE_SIZE(new_length * 2, _YTABLE_DEFAULT_SIZE);
	}
	// create the new list of elements
	_ytable_element_t *elements = calloc0(new_array_size, sizeof(_ytable_element_t));
	if (!elements)
		return (YENOMEM);
	memcpy(elements, t->elements, t->length * sizeof(_ytable_element_t));
	free0(t->elements);
	t->elements = elements;
	// if there was some buckets, they are freed and they recreated
	if (t->buckets) {
		// free all buckets
		for (uint32_t offset = 0; offset < t->array_size; ++offset) {
			uint32_t *pt = t->buckets[offset];
			free0(pt);
		}
		free0(t->buckets);
		// create new bucket list
		t->buckets = (uint32_t**)calloc0(new_array_size, sizeof(uint32_t*));
		if (!t->buckets)
			return (YENOMEM);
		// loop on data to rehash them
		for (uint32_t offset = 0; offset < t->length; ++offset) {
			_ytable_element_t *element = &t->elements[offset];
			if (!element)
				continue;
			// find the bucket
			uint64_t hash_value = _YTABLE_HASH_VALUE(element->hash_value);
			uint32_t modulo_value = MODULO_POW2(hash_value, new_array_size);
			uint32_t *bucket = t->buckets[modulo_value];
			// check if the bucket exists
			if (!bucket) {
				// it doesn't exist, create it
				size_t bucket_size = _YTABLE_DEFAULT_BUCKET_SIZE + 2;
				uint32_t *new_bucket = calloc0(bucket_size, sizeof(uint32_t));
				if (!new_bucket)
					return (YENOMEM);
				new_bucket[0] = _YTABLE_DEFAULT_BUCKET_SIZE;
				new_bucket[1] = 1;
				new_bucket[2] = offset;
				t->buckets[modulo_value] = new_bucket;
				continue;
			}
			// the bucket exists, try to add the element to it
			uint32_t bucket_size = bucket[0];
			uint32_t bucket_length = bucket[1];
			if (bucket_length < bucket_size) {
				// there is still some free space in the bucket
				bucket[bucket_length + 2] = offset;
				++bucket[1];
				continue;
			}
			// the bucket is full
			uint32_t new_bucket_size = NEXT_POW2(bucket_size + 1);
			uint32_t *new_bucket = calloc(new_bucket_size + 2, sizeof(uint32_t));
			if (!new_bucket)
				return (YENOMEM);
			memcpy(new_bucket, bucket, (bucket_length + 2) * sizeof(uint32_t));
			new_bucket[0] = new_bucket_size;
			new_bucket[1] = bucket_length + 1;
			new_bucket[bucket_length + 2] = offset;
			free0(bucket);
			t->buckets[modulo_value] = new_bucket;
		}
	}
	// update array size
	t->array_size = new_array_size;
	return (YENOERR);
}
/*
 * Add an element to the hashmap part of the ytable.
 * Before calling this function, be sure that no element already exist with the same key.
 * @param	t		Pointer to the table.
 * @param	element		Pointer to the element in the table's array.
 * @param	element_offset	Offset of the element in the table's array.
 * @return	YENOERR if OK.
 */
static ystatus_t _ytable_add_element_to_hashmap(_ytable_t *t, _ytable_element_t *element,
                                                uint32_t element_offset) {
	// check if the ytable has a bucket list
	if (!t->buckets) {
		// no, create it
		t->buckets = calloc(t->array_size, sizeof(uint32_t));
		if (!t->buckets)
			return (YENOMEM);
	}
	// check if the bucket exists
	uint64_t hash_value = _YTABLE_HASH_VALUE(element->hash_value);
	uint32_t modulo_value = MODULO_POW2(hash_value, t->array_size);
	uint32_t *bucket = t->buckets[modulo_value];
	if (!bucket) {
		// it doesn't exist, create it
		size_t bucket_size = _YTABLE_DEFAULT_BUCKET_SIZE + 2;
		uint32_t *new_bucket = calloc0(bucket_size, sizeof(uint32_t));
		if (!new_bucket)
			return (YENOMEM);
		new_bucket[0] = _YTABLE_DEFAULT_BUCKET_SIZE;
		new_bucket[1] = 1;
		new_bucket[2] = element_offset;
		t->buckets[modulo_value] = new_bucket;
		return (YENOERR);
	}
#if 0
	// the bucket exists, check if the element should be overwritten
	uint32_t bucket_size = bucket[0];
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t compare_offset = bucket[bucket_offset + 2];
		_ytable_element_t *compare_element = &t->elements[compare_offset];
		if (_YTABLE_HASH_VALUE(compare_element->hash_value) !=
		    _YTABLE_HASH_VALUE(element->hash_value)) {
			// hash values are different
			continue;
		}
		if ((_YTABLE_HAS_NUMERIC_KEY(compare_element->hash_value) &&
		     _YTABLE_HAS_NUMERIC_KEY(element->hash_value)) ||
		    (_YTABLE_HAS_STRING_KEY(compare_element->hash_value) &&
		     _YTABLE_HAS_STRING_KEY(element->hash_value) &&
		     !strcmp0(compare_element->key, element->key))) {
			// same numeric or string key => overwrite
			// start by removing the ancient element's data
			RETURN_IF_ERR(_ytable_free_element_data(t, compare_offset));
			// then overwrite it
			compare_element->data = data;
			return (YENOERR);
		}
	}
#endif
	// check the bucket size
	uint32_t bucket_size = bucket[0];
	uint32_t bucket_length = bucket[1];
	if (bucket_length < bucket_size) {
		// there is still some free space in the bucket
		bucket[bucket_length + 2] = element_offset;
		++bucket[1];
		return (YENOERR);
	}
	// the bucket is full
	uint32_t new_bucket_size = NEXT_POW2(bucket_size + 1);
	uint32_t *new_bucket = calloc(new_bucket_size + 2, sizeof(uint32_t));
	if (!new_bucket)
		return (YENOMEM);
	memcpy(new_bucket, bucket, (bucket_length + 2) * sizeof(uint32_t));
	free0(bucket);
	new_bucket[0] = new_bucket_size;
	new_bucket[1] = bucket_length + 1;
	new_bucket[bucket_length + 2] = element_offset;
	t->buckets[modulo_value] = new_bucket;
	return (YENOERR);
}
/*
 * Extract an element from the hashmap part of a ytable.
 * The element itself (in the array part of the ytable) is not modified or removed.
 * It should be removed afterwards.
 * @param	t	Pointer to the table.
 * @param	element	Pointer to the element to remove. Could be NULL.
 * @param	key	String key. Not used if the element is given. Could be NULL.
 * @param	index	Numeric index. Not used if the element or the key is given.
 * @return	YENOERR if the elemnt was found and offset of the element in the array.
 */
static yres_int_t _ytable_extract_element_from_hashmap(_ytable_t *t, _ytable_element_t *element,
                                                       const char *key, uint64_t index) {
	yres_int_t result_undef = YRESULT_ERR(yres_int_t, YEUNDEF);
	// check if the ytable has a bucket list
	if (!t->buckets)
		return (result_undef);
	// check if the bucket exists
	uint64_t hash_value = element ? _YTABLE_HASH_VALUE(element->hash_value) :
	                      (key ? yhash_compute(key) : index);
	uint64_t element_hash_value = element ? element->hash_value :
	                              (key ? _YTABLE_SET_STRING_KEY(hash_value) :
	                               _YTABLE_SET_NUMERIC_KEY(hash_value));
	const char *element_key = element ? element->key : (key ? key : NULL);
	uint32_t modulo_value = MODULO_POW2(hash_value, t->array_size);
	uint32_t *bucket = t->buckets[modulo_value];
	if (!bucket)
		return (result_undef);
	// the bucket exists, loop on it
	//TODO: shrink the size of the bucket if needed
	//uint32_t bucket_size = bucket[0];
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t elem_offset = bucket[bucket_offset + 2];
		_ytable_element_t *elem = &t->elements[elem_offset];
		// check keys
		if (elem->hash_value != element_hash_value || strcmp0(element_key, elem->key)) {
			// hash values are different
			// => not the same type of keys (one is numeric and the other is string)
			// => or not the same index keys (for numeric keys)
			// => or not the same string keys
			continue;
		}
		// free the string key if needed
		if (_YTABLE_HAS_STRING_KEY(elem->hash_value) && t->delete_function) {
			ystatus_t st = t->delete_function(0, elem->key, NULL, t->delete_data);
			if (st != YENOERR)
				return (YRESULT_ERR(yres_int, st));
		}
		// prepare the result value
		yres_int_t result = YRESULT_VAL(yres_int_t, elem_offset);
		// same numeric or string key => remove it
		// check if it was the only element in the bucket
		if (bucket_length == 1) {
			free0(bucket);
			t->buckets[modulo_value] = NULL;
			return (result);
		}
		// decrements the used size of the bucket
		--bucket[1];
		// check if it's the last element in the bucket
		if (bucket_offset == (bucket_length - 1)) {
			return (result);
		}
		// it's an element in the middle of the bucket
		memmove(&bucket[bucket_offset + 2], &bucket[bucket_offset + 3],
			(bucket_length - bucket_offset - 1) * sizeof(uint32_t));
		return (result);
	}
	return (result_undef);
}

