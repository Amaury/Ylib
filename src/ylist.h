/**
 * @header	ylist.h
 * @abstract	Double-linked lists.
 * @version	1.0.0, Feb 24 2022
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "y.h"

/* ********** TYPE DEFINITIONS ********** */
/**
 * @typedef	ylist_t
 *		Double-linked list.
 * @field	first	First element of the list.
 * @field	last	Last element of the list.
 * @field	ptr	Pointer to the currently processed element.
 */
typedef struct ylist_s {
	struct ylist_elem_s *first;
	struct ylist_elem_s *last;
	struct ylist_elem_s *ptr;
} ylist_t;
/**
 * @typedef	ylist_elem_t
 *		Double-linked list element.
 * @field	data	Pointer to the list element.
 * @field	prev	Pointer to the previous element.
 * @field	next	Pointer to the next element.
 * @field	list	Pointer to the list which contains the element.
 */
typedef struct ylist_elem_s {
	void *data;
	struct ylist_elem_s *prev;
	struct ylist_elem_s *next;
	ylist_t *list;
} ylist_elem_t;
/**
 * @typedef	ylist_func_t
 *		Pointer to a function.
 * @param	elem_data	Pointer to the data in the list.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if OK.
 */
typedef ystatus_t (*ylist_func_t)(void *elem_data, void *user_data);

/* ********** FUNCTIONS ********** */
/**
 * @function	ylist_new
 *		Creates a new list.
 * @return	A pointer to the created list.
 */
ylist_t *ylist_new(void);
/**
 * @function	ylist_foreach
 *		Process a function on all elements of a list.
 * @param	list		Pointer to the list.
 * @param	func		Function to apply. The loop stops if the function returns false.
 * @param	user_data	Pointer to some user data.
 * @return	YENOERR if OK.
 */
ystatus_t ylist_foreach(ylist_t *list, ylist_func_t func, void *user_data);
/**
 * @function	ylist_rewind
 *		Put the internal pointer to the first element of the list.
 * @param	list	Pointer to the list.
 */
void ylist_rewind(ylist_t *list);
/**
 * @function	ylist_free
 *		Destroy a list and all its elements.
 * @param	list		Pointer to the list.
 * @param	delete_function	Callback used to free the list elements.
 * @param	user_data	Pointer given to the delete function.
 * @return	true if the list was successfully deleted.
 */
bool ylist_free(ylist_t *list, ylist_func_t delete_function, void *user_data);
/**
 * @function	ylist_push
 *		Add an element at the end of a list.
 * @param	list	Pointer to the list.
 * @param	data	Pointer to the added data.
 * @return	A pointer to the list element.
 */
ylist_elem_t *ylist_push(ylist_t *list, void *data);
/**
 * @function	ylist_add
 *		Add an element at the beginning of a list.
 * @param	list	Pointer to the list.
 * @param	data	Pointer to the added data.
 * @return	A pointer to the list element.
 */
ylist_elem_t *ylist_add(ylist_t *list, void *data);
/**
 * @function	ylist_shift
 *		Remove the first element of a list and return it.
 * @param	list	Pointer to the list.
 * @return	A pointer to the first element's data.
 */
void *ylist_shift(ylist_t *list);
/**
 * @function	ylist_pop
 *		Remove the last element of a list and return it.
 * @param	list	Pointer to the list.
 * @return	A pointer to the last element's data.
 */
void *ylist_pop(ylist_t *list);
/**
 * @function	ylist_elem_add_before
 *		Add some data before an existing element of a list.
 * @param	elem	The element.
 * @param	data	The data to add before the given element.
 * @return	A pointer to the list element.
 */
ylist_elem_t *ylist_elem_add_before(ylist_elem_t *elem, void *data);
/**
 * @function	ylist_elem_add_after
 *		Add some data after an existing element of a list.
 * @param	elem	The element.
 * @param	data	The data to add after the given element.
 * @return	A pointer to the list element.
 */
ylist_elem_t *ylist_elem_add_after(ylist_elem_t *elem, void *data);
/**
 * @function	ylist_swap
 *		Extract a list element from one list, and add it at the end of
 *		another list.
 * @param	elem	The element to swap.
 * @param	dest	The destination list.
 */
void ylist_swap(ylist_elem_t *elem, ylist_t *dest);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

