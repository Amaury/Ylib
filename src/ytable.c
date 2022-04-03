#include "ytable.h"
#include "y.h"

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
typedef struct _ytable_element_s {
	void *data;
	const char *key;
	uint64_t hash_value;
} _ytable_element_t;

/* ********** DECLARATION OF PRIVATE FUNCTIONS ********** */
static ystatus_t _ytable_free_element_data(ytable_t *t, uint32_t element_offset);
static ystatus_t _ytable_instanciate(ytable_t *t);
static ystatus_t _ytable_instanciate_hashmap(ytable_t *t);
static ystatus_t _ytable_expand(ytable_t *t, uint16_t size, bool array_only);
static ystatus_t _ytable_add_element_to_hashmap(ytable_t *t, _ytable_element_t *element,
                                                uint32_t element_offset);
static yres_int_t _ytable_extract_element_from_hashmap(ytable_t *t, _ytable_element_t *element,
                                                       const char *key, uint64_t index);

/* ************ CREATION/DELETION FUNCTIONS ************* */
/* Create a new simple ytable. */
ytable_t *ytable_new(void) {
	ytable_t *table = malloc0(sizeof(ytable_t));
	if (!table)
		return (NULL);
	table->array_size = _YTABLE_DEFAULT_SIZE;
	return (table);
}
/* Create a new ytable by giving its size. */
ytable_t *ytable_create(size_t size, ytable_function_t delete_function, void *delete_data) {
	ytable_t *t = malloc0(sizeof(ytable_t));
	if (!t)
		return (NULL);
	*t = (ytable_t){
		.array_size = COMPUTE_SIZE(size, _YTABLE_DEFAULT_SIZE),
		.delete_function = delete_function,
		.delete_data = delete_data,
	};
	return (t);
}
/* Initialize a ytable. */
ytable_t *ytable_init(ytable_t *table, ytable_function_t delete_function, void *delete_data) {
	if (!table)
		return (NULL);
	*table = (ytable_t){
		.array_size = _YTABLE_DEFAULT_SIZE,
		.delete_function = delete_function,
		.delete_data = delete_data,
	};
	return (table);
}
/* Define the delete function. */
ytable_t *ytable_set_delete_function(ytable_t *table, ytable_function_t delete_function,
                                     void *delete_data) {
	if (table)
		return (NULL);
	table->delete_function = delete_function;
	table->delete_data = delete_data;
	return (table);
}
/* Destroy a ytable. */
void ytable_free(ytable_t *table) {
	if (!table)
		return;
	if (table->buckets) {
		for (uint32_t offset = 0; offset < table->array_size; ++offset) {
			free0(table->buckets[offset]);
		}
		free0(table->buckets);
	}
	if (table->elements && table->delete_function) {
		for (size_t offset = 0; offset < table->length; ++offset) {
			_ytable_element_t *e = &table->elements[offset];
			table->delete_function(e->hash_value, (char*)e->key, e->data,
			                       table->delete_data);
		}
	}
	free0(table->elements);
	free0(table);
}
/* Clone a ytable. */
ytable_t *ytable_clone(ytable_t *table) {
	if (!table)
		return (NULL);
	// create new table
	ytable_t *t = malloc0(sizeof(ytable_t));
	if (!t)
		return (NULL);
	*t = (ytable_t){
		.length = table->length,
		.array_size = table->array_size,
		.next_index = table->next_index,
		.delete_function = table->delete_function,
		.delete_data = table->delete_data,
	};
	if (!table->elements)
		return (t);
	// create array
	t->elements = calloc0(t->array_size, sizeof(_ytable_element_t));
	if (!t->elements) {
		free0(t);
		return (NULL);
	}
	// copy the array
	memcpy(t->elements, table->elements, (table->length * sizeof(_ytable_element_t)));
	// create buckets
	if (!table->buckets)
		return (t);
	t->buckets = calloc0(t->array_size, sizeof(uint32_t*));
	if (!t->buckets) {
		free0(t->elements);
		free0(t);
		return (NULL);
	}
	// copy buckets
	for (uint32_t offset = 0; offset < t->array_size; ++offset) {
		uint32_t *bucket = table->buckets[offset];
		if (!bucket)
			continue;
		uint32_t bucket_size = bucket[0];
		uint32_t bucket_length = bucket[1];
		uint32_t *new_bucket = calloc0(bucket_size + 2, sizeof(uint32_t));
		if (!new_bucket)
			return (t);
		memcpy(new_bucket, bucket, ((bucket_length + 2) * sizeof(uint32_t)));
		t->buckets[offset] = new_bucket;
	}
	return (t);
}

/* ********** ARRAY-LIKE FUNCTIONS ********** */
/* Add an element at the end of a ytable (used as an array). */
ystatus_t ytable_add(ytable_t *table, void *data) {
	if (!table)
		return (YEINVAL);
	// instanciate the array if needed
	RETURN_IF_ERR(_ytable_instanciate(table));
	// expand the array if needed
	RETURN_IF_ERR(_ytable_expand(table, 1, false));
	// add the element at the end of the array
	_ytable_element_t *element = &table->elements[table->length];
	*element = (_ytable_element_t){
		.data = data,
	};
	// increment counters
	++table->length;
	++table->next_index;
	return (YENOERR);
}
/* Add multiple elements at the end of a ytable (used as an array). */
ystatus_t ytable_madd(ytable_t *table, uint32_t count, ...) {
	if (!table)
		return (YEINVAL);
	if (!count)
		return (YENOERR);
	// instanciate the array if needed
	RETURN_IF_ERR(_ytable_instanciate(table));
	// expand the array if needed
	RETURN_IF_ERR(_ytable_expand(table, count, false));
	// loop on the elements to add
	va_list p_list;
	va_start(p_list, count);
	for (uint32_t n = 0; n < count; ++n) {
		// add the element at the end of the array
		void *data = va_arg(p_list, void*);
		table->elements[table->length] = (_ytable_element_t){
			.data = data,
		};
		// increment length counter
		++table->length;
	}
	// increment next index
	table->next_index += count;
	return (YENOERR);
}
/* Add an element at the beginning of a ytable (used as an array). */
ystatus_t ytable_push(ytable_t *table, void *data) {
	if (!table)
		return (YEINVAL);
	_ytable_element_t *element;
	// instanciate and expand the array if needed
	RETURN_IF_ERR(_ytable_instanciate(table));
	RETURN_IF_ERR(_ytable_expand(table, 1, false));
	// check if existing elements must be moved in the array
	if (!table->length)
		goto empty_array;
	// loop on array elements to move them, starting by the last one
	for (uint32_t new_offset = table->length; new_offset; --new_offset) {
		uint32_t old_offset = new_offset - 1;
		// move the element in the array
		table->elements[new_offset] = table->elements[old_offset];
		// if the element has a numeric key and this key is equal to the element's
		// new offset, remove it from the hashmap part of the table (the element
		// becomes like it has a numeric key from the beginning)
		_ytable_element_t *element = &table->elements[new_offset];
		if (_YTABLE_HAS_NUMERIC_KEY(element->hash_value) &&
		    _YTABLE_HASH_VALUE(element->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(table, element, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			element->hash_value = 0;
		}
	}
empty_array:
	// add the new element
	element = &table->elements[0];
	*element = (_ytable_element_t){
		.data = data,
	};
	++table->length;
	++table->next_index;
	return (YENOERR);
}
/* Add multiple elements at the beginning of a ytable (used as an array). */
ystatus_t ytable_mpush(ytable_t *table, uint32_t count, ...) {
	if (!table)
		return (YEINVAL);
	// instanciate and expand the array if needed
	RETURN_IF_ERR(_ytable_instanciate(table));
	RETURN_IF_ERR(_ytable_expand(table, count, false));
	// check if existing elements must be moved in the array
	if (!table->length)
		goto empty_array;
	// loop on array elements to move them, starting by the last one
	for (int64_t old_offset = (table->length - 1); old_offset >= 0; --old_offset) {
		uint32_t new_offset = old_offset + count;
		// move the element in the array
		table->elements[new_offset] = table->elements[old_offset];
		// if the element has a numeric key and this key is equal to the element's
		// new offset, remove it from the hashmap part of the table (the element
		// becomes like it has a numeric key from the beginning)
		_ytable_element_t *elem = &table->elements[new_offset];
		if (_YTABLE_HAS_NUMERIC_KEY(elem->hash_value) &&
		    _YTABLE_HASH_VALUE(elem->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(table, elem, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			elem->hash_value = 0;
		}
	}
empty_array: ;
	// add the new elements
	va_list p_list;
	va_start(p_list, count);
	for (uint32_t n = 0; n < count; ++n) {
		// add the element at the beginning of the array
		void *data = va_arg(p_list, void*);
		table->elements[n] = (_ytable_element_t){
			.data = data,
		};
	}
	table->length += count;
	table->next_index += count;
	return (YENOERR);
}
/* Remove the last element of a ytable and return it. */
void *ytable_pop(ytable_t *table) {
	if (!table || !table->length)
		return (NULL);
	--table->length;
	_ytable_element_t *element = &table->elements[table->length];
	// if the element is in the hashmap, remove it
	if (element->hash_value) {
		_ytable_extract_element_from_hashmap(table, element, NULL, 0);
	}
	return (element->data);
}
/* Remove the first element of a ytable and return it. */
void *ytable_shift(ytable_t *table) {
	if (!table || !table->length)
		return (NULL);
	_ytable_element_t *element = &table->elements[0];
	// if the element is in the hashmap, remove it
	if (element->hash_value) {
		_ytable_extract_element_from_hashmap(table, element, NULL, 0);
	}
	// loop on array elements to move them, starting by the second one
	void *data = element->data;
	for (uint32_t old_offset = 1; old_offset < table->length; ++old_offset) {
		uint32_t new_offset = old_offset - 1;
		// move the element in the array
		table->elements[new_offset] = table->elements[old_offset];
		element = &table->elements[new_offset];
		// if the element has a numeric key and this key is equal to the element's
		// new offset, remove it from the hashmap part of the table (the element
		// becomes likes it has a numeric key from the beginning)
		if (_YTABLE_HAS_NUMERIC_KEY(element->hash_value) &&
		    _YTABLE_HASH_VALUE(element->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(table, element, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			element->hash_value = 0;
		}
	}
	--table->length;
	return (data);
}

/* ********** INDEXED FUNCTIONS ********** */
/* Tell if a given index exists in a ytable. */
bool ytable_index_exists(ytable_t *table, uint64_t index) {
	yres_pointer_t res = ytable_get_index(table, index);
	if (YRES_STATUS(res) == YENOERR)
		return (true);
	return (false);
}
/* Tell if a given index exists and is set in a ytable. */
bool ytable_index_isset(ytable_t *table, uint64_t index) {
	yres_pointer_t res = ytable_get_index(table, index);
	if (YRES_STATUS(res) == YENOERR && YRES_VAL(res))
		return (true);
	return (false);
}
/* Return the value associated to the given index. */
yres_pointer_t ytable_get_index(ytable_t *table, uint64_t index) {
	if (!table)
		return (YRESULT_ERR(yres_pointer_t, YEINVAL));
	if (!table->length)
		return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
	// search for a direct index
	_ytable_element_t *elem;
	if (index < table->length) {
		elem = &table->elements[index];
		if (_YTABLE_HAS_NO_KEY(elem->hash_value))
			return (YRESULT_VAL(yres_pointer_t, elem->data));
	}
	// search for an hashed index
	if (!table->buckets)
		return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
	uint32_t modulo_value = MODULO_POW2(index, table->array_size);
	uint32_t *bucket = table->buckets[modulo_value];
	if (!bucket)
		return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t elem_offset = bucket[bucket_offset + 2];
		elem = &table->elements[elem_offset];
		// check if hash values are equal
		if (_YTABLE_HAS_NUMERIC_KEY(elem->hash_value) &&
		    _YTABLE_HASH_VALUE(elem->hash_value) == index)
			return (YRESULT_VAL(yres_pointer_t, elem->data));
	}
	// not found
	return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
}
/* Return the data value associated to the given index. */
void *ytable_get_index_data(ytable_t *table, uint64_t index) {
	yres_pointer_t res = ytable_get_index(table, index);
	RETURN_NULL_IF_ERR(YRES_STATUS(res));
	return (YRES_VAL(res));
}
/* Add an element in a ytable using an index. */
ystatus_t ytable_set_index(ytable_t *table, uint64_t index, void *data) {
	if (!table)
		return (YEINVAL);
	// if the given index is the next in numeric order, manage the table as an array
	if (index == table->next_index && index == table->length)
		return (ytable_add(table, data));
	// instanciate the array if needed
	RETURN_IF_ERR(_ytable_instanciate(table));
	// if the array is empty, no need to check if an element must be overwritten
	if (!table->length)
		goto empty_array;
	// check if the given index is corresponding to an array-like indexed element
	if (index >= table->length)
		goto not_array_element;
	_ytable_element_t *element = &table->elements[index];
	if (_YTABLE_HAS_NO_KEY(element->hash_value) ||
	    (_YTABLE_HAS_NUMERIC_KEY(element->hash_value) &&
	     _YTABLE_HASH_VALUE(element->hash_value) == index)) {
		// a previous element was found => overwrite
		// starts by removing the ancient element's data
		RETURN_IF_ERR(_ytable_free_element_data(table, index));
		// then overwrite it
		element->data = data;
		return (YENOERR);
	}
not_array_element:
	// search if an element already exists in the hashmap with the same hash value
	if (!table->length || !table->buckets)
		goto nobucket;
	uint64_t hash_value = index;
	uint32_t modulo_value = MODULO_POW2(hash_value, table->array_size);
	uint32_t *bucket = table->buckets[modulo_value];
	if (!bucket)
		goto nobucket;
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t elem_offset = bucket[bucket_offset + 2];
		element = &table->elements[elem_offset];
		// check if hash values are equal
		if (!_YTABLE_HAS_NUMERIC_KEY(element->hash_value) ||
		    _YTABLE_HASH_VALUE(element->hash_value) != hash_value)
			continue;
		// same hash value => overwrite
		// starts by removing the ancient element's data
		RETURN_IF_ERR(_ytable_free_element_data(table, elem_offset));
		// then overwrite it
		element->data = data;
		return (YENOERR);
	}
empty_array:
nobucket: ;
	bool hashed_element = (index != table->next_index || index != table->length) ?
	                      true : false;
	// expand the array if needed
	RETURN_IF_ERR(_ytable_expand(table, 1, hashed_element));
	// add the element at the end of the array
	element = &table->elements[table->length];
	*element = (_ytable_element_t){
		.data = data,
	};
	// if the element's index is different of the table's next index,
	// add it to the hashmap part of the table
	if (hashed_element) {
		element->hash_value = _YTABLE_SET_NUMERIC_KEY(index);
		RETURN_IF_ERR(_ytable_instanciate_hashmap(table));
		RETURN_IF_ERR(_ytable_add_element_to_hashmap(table, element, table->length));
	}
	// increment counters
	++table->length;
	index += 1;
	table->next_index = MAX(table->next_index, index);
	return (YENOERR);
}
/* Extract an element from its index and return its data. */
yres_pointer_t ytable_extract_index(ytable_t *table, uint64_t index) {
	if (!table)
		return (YRESULT_ERR(yres_pointer_t, YEINVAL));
	void *result_data;
	if (!table->length)
		return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
	// search for a direct index
	if (index >= table->length)
		goto not_array;
	_ytable_element_t *elem = &table->elements[index];
	if (_YTABLE_HAS_NO_KEY(elem->hash_value)) {
		// decrement array size
		--table->length;
		// check if it's the last element of the array
		if (index == table->length)
			return (YRESULT_VAL(yres_pointer_t, elem->data));
		// not the last element, shift all other elements
		result_data = elem->data;
		goto shift_elements;
	}
not_array: ;
	// extract the element from the hashmap
	yres_int_t res = _ytable_extract_element_from_hashmap(table, NULL, NULL, index);
	if (YRES_STATUS(res) != YENOERR)
		return (YRESULT_ERR(yres_pointer_t, YRES_STATUS(res)));
	index = YRES_VAL(res);
	elem = &table->elements[index];
	// decrement array size
	--table->length;
	// check if it's the last element of the array
	if (index == table->length)
		return (YRESULT_VAL(yres_pointer_t, elem->data));
	// not the last element, shift all other elements
	result_data = elem->data;
shift_elements:
	// shift elements in the array
	for (uint32_t new_offset = index; new_offset < table->length; ++new_offset) {
		uint32_t old_offset = new_offset + 1;
		elem = &table->elements[old_offset];
		// if the element has a numeric key and this key is equal to the element"s
		// new offset, remove it from the hashmap part of the table (the element
		// becomes likes it has a numeric key from the beginning)
		if (_YTABLE_HAS_NUMERIC_KEY(elem->hash_value) &&
		    _YTABLE_HASH_VALUE(elem->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(table, elem, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			elem->hash_value = 0;
		}
		// move the element in the array
		table->elements[new_offset] = table->elements[old_offset];
	}
	return (YRESULT_VAL(yres_pointer_t, result_data));
}
/* Extract an element from its index and return its data. */
void *ytable_extract_index_data(ytable_t *table, uint64_t index) {
	yres_pointer_t res = ytable_extract_index(table, index);
	RETURN_NULL_IF_ERR(YRES_STATUS(res));
	return (YRES_VAL(res));
}
/* Remove an element from its index. */
ystatus_t ytable_remove_index(ytable_t *table, uint64_t index) {
	if (!table)
		return (YEINVAL);
	if (!table->length)
		return (YEUNDEF);
	// check if the given index is corresponding to an array-like indexed element
	if (index >= table->length)
		goto not_array;
	_ytable_element_t *elem = &table->elements[index];
	if (_YTABLE_HAS_NO_KEY(elem->hash_value)) {
		// free the element's data
		RETURN_IF_ERR(_ytable_free_element_data(table, index));
		// decrement array size
		--table->length;
		// check if it's the last element of the array
		if (index == table->length)
			return (YENOERR);
		// not the last element, shift all other elements
		goto shift_elements;
	}
not_array: ;
	// remove the element from the hashmap
	yres_int_t res = _ytable_extract_element_from_hashmap(table, NULL, NULL, index);
	if (YRES_STATUS(res) != YENOERR)
		return (YRES_STATUS(res));
	index = YRES_VAL(res);
	// free the element's data
	RETURN_IF_ERR(_ytable_free_element_data(table, index));
	// decrement array size
	--table->length;
	// check if it's the last element of the array
	if (index == table->length)
		return (YENOERR);
shift_elements:
	// loop on the array
	for (uint32_t new_offset = index; new_offset < table->length; ++new_offset) {
		uint32_t old_offset = new_offset + 1;
		elem = &table->elements[old_offset];
		// if the element has a numeric key and this key is equal to the element"s
		// new offset, remove it from the hashmap part of the table (the element
		// becomes likes it has a numeric key from the beginning)
		if (_YTABLE_HAS_NUMERIC_KEY(elem->hash_value) &&
		    _YTABLE_HASH_VALUE(elem->hash_value) == new_offset) {
			// remove the element from the hashmap
			_ytable_extract_element_from_hashmap(table, elem, NULL, 0);
			// change the type of key => no type (not numeric, not a string)
			elem->hash_value = 0;
		}
		// move the element in the array
		table->elements[new_offset] = table->elements[old_offset];
	}
	return (YENOERR);
}

/* ********** KEYED FUNCTIONS ********** */
/* Tell if a given string key exists in a ytable. */
bool ytable_key_exists(ytable_t *table, const char *key) {
	yres_pointer_t res = ytable_get_key(table, key);
	if (YRES_STATUS(res) == YENOERR)
		return (true);
	return (false);
}
/* Tell if a given string key exists and is set in a ytable. */
bool ytable_key_isset(ytable_t *table, const char *key) {
	yres_pointer_t res = ytable_get_key(table, key);
	if (YRES_STATUS(res) == YENOERR && YRES_VAL(res) != NULL)
		return (true);
	return (false);
}
/* Return the value associated to the given string key. */
yres_pointer_t ytable_get_key(ytable_t *table, const char *key) {
	if (!table)
		return (YRESULT_ERR(yres_pointer_t, YEINVAL));
	if (!table->length || !table->buckets)
		return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
	// if the key is a numeric string, manage it as a numeric insert
	if (ys_is_numeric(key)) {
		uint64_t index = (uint64_t)atol(key);
		return (ytable_get_index(table, index));
	}
	// search for bucket
	uint64_t hash_value = yhash_compute(key);
	uint32_t modulo_value = MODULO_POW2(hash_value, table->array_size);
	uint32_t *bucket = table->buckets[modulo_value];
	if (!bucket)
		return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t elem_offset = bucket[bucket_offset + 2];
		_ytable_element_t *elem = &table->elements[elem_offset];
		// check if hash values are equal
		if (_YTABLE_HAS_STRING_KEY(elem->hash_value) &&
		    _YTABLE_HASH_VALUE(elem->hash_value) == hash_value &&
		    !strcmp0(elem->key, key))
			return (YRESULT_VAL(yres_pointer_t, elem->data));
	}
	// not found
	return (YRESULT_ERR(yres_pointer_t, YEUNDEF));
}
/* Return the data value associated to the given string key. */
void *ytable_get_key_data(ytable_t *table, const char *key) {
	yres_pointer_t res = ytable_get_key(table, key);
	RETURN_NULL_IF_ERR(YRES_STATUS(res));
	return (YRES_VAL(res));
}
/* Add an element in a ytable using a string key. */
ystatus_t ytable_set_key(ytable_t *table, const char *key, void *data) {
	if (!table || !key)
		return (YEINVAL);
	// if the key is a numeric string, manage it as a numeric insert
	if (ys_is_numeric(key)) {
		uint64_t index = (uint64_t)atol(key);
		return (ytable_set_index(table, index, data));
	}
	// instanciate the array if needed
	RETURN_IF_ERR(_ytable_instanciate(table));
	// compute the hash value
	uint64_t hash_value = yhash_compute(key);
	// if the array is empty, no need to check if an element must be overwritten
	if (!table->length || !table->buckets)
		goto nobucket;
	// search for bucket
	uint32_t modulo_value = MODULO_POW2(hash_value, table->array_size);
	uint32_t *bucket = table->buckets[modulo_value];
	if (!bucket)
		goto nobucket;
	// loop on bucket elements
	uint32_t bucket_length = bucket[1];
	for (uint32_t bucket_offset = 0; bucket_offset < bucket_length; ++bucket_offset) {
		uint32_t elem_offset = bucket[bucket_offset + 2];
		_ytable_element_t *elem = &table->elements[elem_offset];
		// check if keys are equal
		if (!_YTABLE_HAS_STRING_KEY(elem->hash_value) ||
		    _YTABLE_HASH_VALUE(elem->hash_value) != hash_value ||
		    strcmp0(elem->key, key)) {
			// keys are different
			continue;
		}
		// same keys => overwrite
		// starts by removing the ancient element's data
		RETURN_IF_ERR(_ytable_free_element_data(table, elem_offset));
		// then overwrite it
		elem->data = data;
		return (YENOERR);
	}
nobucket:
	// expand the array if needed
	RETURN_IF_ERR(_ytable_expand(table, 1, true));
	// add the element at the end of the array
	_ytable_element_t *element = &table->elements[table->length];
	*element = (_ytable_element_t){
		.data = data,
		.key = key,
		.hash_value = _YTABLE_SET_STRING_KEY(hash_value)
	};
	// add the element in the hashmap part of the table
	RETURN_IF_ERR(_ytable_instanciate_hashmap(table));
	RETURN_IF_ERR(_ytable_add_element_to_hashmap(table, element, table->length));
	// increment counter
	++table->length;
	return (YENOERR);
}

/* ********** GENERAL FUNCTIONS ********** */
/* Return the used length of a ytable. */
uint32_t ytable_length(ytable_t *table) {
	if (!table)
		return (0);
	return (table->length);
}
/* Tell if a ytable is used as an array (continuous list of elememnts). */
bool ytable_is_array(ytable_t *table) {
	if (!table || !table->buckets)
		return (true);
	return (false);
}
/* Apply a function on every elements of a ytable. */
ystatus_t ytable_foreach(ytable_t *table, ytable_function_t func, void *user_data) {
	if (!table)
		return (YEINVAL);
	if (!table->length || !func)
		return (YENOERR); 
	// loop on all elements
	for (uint32_t offset = 0; offset < table->length; ++offset) {
		_ytable_element_t *elem = &table->elements[offset];
		uint64_t hash = _YTABLE_HAS_NUMERIC_KEY(elem->hash_value) ?
		                _YTABLE_HASH_VALUE(elem->hash_value) : offset;

		RETURN_IF_ERR(func(hash, (char*)elem->key, elem->data, user_data));
	}
	return (YENOERR);
}

/* ********** PRIVATE FUNCTIONS ********** */
/* Delete the data pointed by an element. */
static ystatus_t _ytable_free_element_data(ytable_t *t, uint32_t element_offset) {
	if (!t->delete_function)
		return (YENOERR);
	_ytable_element_t *e = &t->elements[element_offset];
	return (t->delete_function(element_offset, (char*)e->key, e->data, t->delete_data));
}
/* Instanciate the buffers of a new empty ytable, if needed. */
static ystatus_t _ytable_instanciate(ytable_t *t) {
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
static ystatus_t _ytable_instanciate_hashmap(ytable_t *t) {
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
static ystatus_t _ytable_expand(ytable_t *t, uint16_t size, bool hash_expansion) {
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
static ystatus_t _ytable_add_element_to_hashmap(ytable_t *t, _ytable_element_t *element,
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
 * @return	YENOERR if the element was found and offset of the element in the array.
 *		YEUNDEF if the element wasn't found.
 *		Other error code if the function used to delete string key failed.
 */
static yres_int_t _ytable_extract_element_from_hashmap(ytable_t *t, _ytable_element_t *element,
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
			ystatus_t st = t->delete_function(0, (char*)elem->key, NULL,
			                                  t->delete_data);
			if (st != YENOERR)
				return (YRESULT_ERR(yres_int_t, st));
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

