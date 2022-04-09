/**
 * @header	yarray.h
 * @abstract	All definitions about yarrays.
 * @discussion	yarrays are bufferised arrays of pointers.
 * @version	1.0 Jun 13 2002
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include <stdlib.h>

/* ************** TYPE DEFINITIONS ************* */

/** @typedef yarray_t Array type definition. Always equivalent to (void**). */
typedef void** yarray_t;
/**
 * @typedef	yarray_function_t
 *		Function pointer, used to apply a procedure to an element.
 * @param	index		Index of the element in the array.
 * @param	data		Pointer to the data.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if OK.
 */
typedef ystatus_t (*yarray_function_t)(size_t index, void *data, void *user_data);


/* *********************** FUNCTIONS ******************* */

/**
 * @function	yarray_new
 *		Create a new yarray of the default size (4K).
 * @return	The created array.
 */
yarray_t yarray_new(void);
/**
 * @function	yarray_create
 *		Creates a new yarray of the given size.
 * @param	size	Size of the new yarray.
 * @return	The created yarray.
 */
yarray_t yarray_create(size_t size);
/**
 * @function	yarray_free
 *		Delete an yarray. Its content is NOT freed.
 * @param	a	The yarray.
 * @return	Always NULL.
 */
void *yarray_free(yarray_t a);
/**
 * @function	yarray_delete
 *		Delete an existing yarray.
 * @param	v	A pointer to the yarray.
 * @param	f	Pointer to a function that is called to delete each
 *			element of the yarray. Could be NULL.
 * @param	data	Pointer to data that could be given to the delete
 *			callback (see previous parameter). Could be NULL.
 */
void yarray_delete(yarray_t *v, yarray_function_t f, void *data);
/**
 * @function	yarray_trunc
 *		Truncate an existing yarray. The allocated memory doesn't
 *		change.
 * @param	v	The yarray.
 * @param	f	Pointer to a function that is called to delete each
 *			element of the yarray. Could be NULL ; otherwise, must
 *			have this prototype : void f(void *elem, void *data);
 * @param	data	Pointer to data that could be given to the delete
 *			callback (see previous parameter). Could be NULL.
 */
void yarray_trunc(yarray_t v, yarray_function_t f, void *data);
/**
 * @function	yarray_resize
 *		Set the minimum size of a yarray.
 * @param	v	A pointer to the yarray.
 * @param	sz	The minimum size for this yarray.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_resize(yarray_t *v, size_t sz);
/**
 * @function	yarray_length
 *		Return the length of a yarray (its used size).
 * @param	v	The yarray.
 * @return	The yarray's length.
 */
size_t yarray_length(const yarray_t v);
/**
 * @function	yarray_size
 *		Return the total size of a yarray.
 * @param	v	The yarray.
 * @return	The yarray's size.
 */
size_t yarray_size(const yarray_t v);
/**
 * @function	yarray_append
 *		Concatenate a yarray at the end of another one.
 * @param	dest	A pointer to the yarray to extend.
 * @param	src	A yarray.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_append(yarray_t *dest, const yarray_t src);
/**
 * @function	yarray_nappend
 *		Concatenate a given number of elements from a yarray
 *		at the end of another.
 * @param	dest	A pointer to the yarray to extend.
 * @param	src	A yarray.
 * @param	n	The number of elements to copy.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_nappend(yarray_t *dest, const yarray_t src, size_t n);
/**
 * @function	yarray_clone
 *		Duplicate a yarray.
 * @param	v	The yarray.
 * @result	The new yarray.
 */
yarray_t yarray_clone(const yarray_t v);
/**
 * @function	yarray_merge
 *		Merge two yarrays to create a new one.
 * @param	v1	The first yarray.
 * @param	v2	The second yarray.
 * @return	The new yarray.
 */
yarray_t yarray_merge(const yarray_t v1, const yarray_t v2);
/**
 * @function	yarray_add
 *		Add an element at the beginning of a yarray.
 * @param	v	A pointer to the yarray.
 * @param	e	A pointer to the element.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_add(yarray_t *v, void *e);
/**
 * @function	yarray_push
 *		Add an element at the end of a yarray.
 * @param	v	A pointer to the yarray.
 * @param	e	A pointer to the element.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_push(yarray_t *v, void *e);
/**
 * @function	yarray_push_multi
 *		Add multiple elements at the end of a yarray.
 * @param	v	A pointer to the yarray.
 * @param	n	The number of elements to add.
 * @param	...	The elements to add.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_push_multi(yarray_t *v, size_t n, ...);
/**
 * @function	yarray_insert
 *		Insert an element at the given offset of a yarray. All
 *		elements placed at this offset and after are shifted to the
 *		right.
 * @param	v	A pointer to the yarray.
 * @param	e	A pointer to the element.
 * @param	i	Offset of the element in the yarray. Must be less or
 *			equal to the yarray's used size.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_insert(yarray_t *v, void *e, size_t i);
/**
 * @function	yarray_set
 *		Replace the element at the given offset of a yarray.
 * @param	v	The array.
 * @param	e	A pointer to the element.
 * @param	i	Offset of the element in the yarray. Must be less or
 *			equal to the yarray's used size.
 * @param	f	Pointer to a function that is called to delete the existing
 *			element of the yarray. Could be NULL ; otherwise, must
 *			have this prototype : void f(void *elem, void *data);
 * @param	data	Pointer to data that could be given to the delete
 *			callback (see previous parameter). Could be NULL.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_set(yarray_t v, void *e, size_t i, yarray_function_t f, void *data);
/**
 * @function	yarray_get_first
 *		Returns the first element of a yarray.
 * @param	v	The yarray.
 * @return	A pointer to the first element, or NULL if no element was stored.
 */
void *yarray_get_first(const yarray_t v);
/**
 * @function	yarray_get_last
 *		Returns the last element of a yarray.
 * @param	v	The yarray.
 * @return	A pointer to the last element, or NULL if no element was stored.
 */
void *yarray_get_last(const yarray_t v);
/**
 * @function	yarray_get
 *		Returns the nth element of a yarray.
 * @param	v	The yarray.
 * @param	i	The position of the element, starting at 0.
 * @return	A pointer to the requested element, or NULL if it doesn't exist.
 */
void *yarray_get(const yarray_t v, size_t i);
/**
 * @function	yarray_shift
 *		Remove the first element of a yarray and return it.
 * @param	v	The yarray.
 * @return	A pointer to the removed element.
 */
void *yarray_shift(yarray_t v);
/**
 * @function	yarray_pop
 *		Remove the last element of a yarray and return it.
 * @param	v	The yarray.
 * @return	A pointer to the removed element.
 */
void *yarray_pop(yarray_t v);
/**
 * @function	yarray_extract
 *		Extract the element placed at the given offset of a yarray.
 *		All elements placed after the offset are shifted to the left.
 * @param	v	The yarray.
 * @param	i	Offset of the element in the yarray. Must be less than
 *			the yarray's used size.
 * @return	A pointer to the removed element.
 */
void *yarray_extract(yarray_t v, size_t i);
/**
 * @function	yarray_uniq
 *		Remove all values of a yarray to let only one entry of
 *		each value.
 * @param	v	The yarray.
 */
void yarray_uniq(yarray_t v);
/**
 * @function	yarray_sort
 *		Do a quick sort of all elements of a yarray. See qsort(3).
 * @param	v	The yarray.
 * @param	f	A pointer to the function used to compare elements.
 */
void yarray_sort(yarray_t v, int (*f)(const void*, const void*));
/**
 * @function	yarray_search
 *		Search the offset of an element in a yarray. WARNING: the
 *		yarray must be sorted (using yarray_sort()) because this
 *		function uses dichotomy.
 * @param	v	The yarray.
 * @param	e	The element to compare.
 * @param	f	A pointer to the function used to compare elements.
 * @return	The offset of the elment in the yarray, or (-1) if the
 *		element can't be found.
 */
long long int yarray_search(const yarray_t v, void *e, int (*f)(const void*, const void*));
/**
 * @function	yarray_foreach
 * 		Apply a function on every elements of an array.
 * @param	v		The yarray.
 * @param	func		Pointer to the executed function.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if OK.
 */
ystatus_t yarray_foreach(yarray_t v, yarray_function_t func, void *user_data);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

