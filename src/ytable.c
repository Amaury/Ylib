#include "ytable.h"

/* ************ PRIVATE DEFINITIONS AND MACROS ************ */
/** @define _YTABLE_DEFAULT_SIZE DEfault size of yvectors. */
#define _YTABLE_DEFAULT_SIZE	256
/** @define _YTABLE_ROUND_SIZE	Round the size to the next power of 2. */
#define _YTABLE_ROUND_SIZE(s)	((size_t)pow(2, ceil(log2(s))))
/** @define _YTABLE_SIZE Compute the size of a new yarray's buffer. */
#define _YARRAY_SIZE(s)	(((s) < _YTABLE_DEFAULT_SIZE) ? \
			 _YTABLE_DEFAULT_SIZE : \
			 _YTABLE_ROUND_SIZE((s)))

/* ************ PRIVATE STRUCTURES AND TYPES ************** */
/**
 * @typedef	_ytable_element_t
 *		Storage structure of an element.
 * @field	data	Pointer to the stored data.
 */
typedef struct {
	void *data;
	enum {
		YTABLE_KEY_INT = 0,
		YTABLE_KEY_STRING,
	} key_type;
	char *key;
	size_t hash_value;
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
	size_t length;
	size_t array_size;
	size_t next_index;
	_ytable_element_t *elements;
	void **buckets;
	ytable_function_t *delete_function;
	void *delete_data;
} _ytable_t;

/* ************ FUNCTIONS ************* */
/* Create a new simple ytable. */
ytable_t *ytable_new(void) {
	_ytable_t *table = malloc0(sizeof(_ytable_t));
	if (!table)
		return (NULL);
	table = (_ytable_t){
		.array_size = _YTABLE_DEFAULT_SIZE,
		.elements = malloc0(_YTABLE_DEFAULT_SIZE * sizeof(_table_element_t)),
	};
	if (!table->elements) {
		free0(table);
		return (NULL);
	}
	return ((ytable_t*)table);
}
/* Create a new ytable by giving its size. */
ytable_t *ytable_create(size_t size, ytable_function_t delete_function, void *delete_data) {
	_ytable_t *table = malloc0(sizeof(_ytable_t));
	if (!table)
		return (NULL);
	size_t array_size = size ? size : _YTABLE_DEFAULT_SIZE;
	table = (_ytable_t){
		.array_size = array_size,
		.elements = malloc0(array_size * sizeof(_table_element_t)),
		.delete_function = delete_function,
		.delete_data = delete_data,
	};
	if (!table->elements) {
		free0(table);
		return (NULL);
	}
	return ((ytable_t*)table);
}
/* Define the delete function. */
ytable_t *ytable_set_delete_function(ytable_t *table, ytable_function_t delete_function, void *delete_data) {
	if (table)
		return (NULL);
	_ytable_t *t = (_ytable_t*)table;
	t->delete_function = delete_function;
	t->delete_data = delete_data;
	return (table);
}
