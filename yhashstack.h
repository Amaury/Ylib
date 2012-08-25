/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	yhashstack.h
 * @abstract	All definitions about hash stacks.
 * @discussion  Hash stacks are composite structures that contain a list of
 *		hash tables, allowing to manipulate them as a unique hash table,
 *		without copying data.
 * @version     1.0 Aug 25 2012
 * @author      Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YHASHSTACK_H__
#define __YHASHSTACK_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "ydefs.h"
#include "yvect.h"
#include "yhashtable.h"

/* ************** TYPE DEFINITIONS **************** */

/*! @typedef yhashstack_t Same as a yvector. */
typedef yvect_t yhashstack_t;

/*! @typedef yhs_size_t Same as yv_size_t. */
typedef yv_size_t yhs_size_t;

/* ****************** FUNCTIONS **************** */
/**
 * @function	yhs_new
 *		Creates a new hash stack.
 * @param	size	The size of the created stack.
 * @return	The created hash stack.
 */
yhashstack_t yhs_new(yhs_size_t size);

/**
 * @function	yhs_delete
 *		Destroy a hash stack. The enclosed hash tables are NOT deleted.
 * @param	hashstack	Pointer to the hash stack.
 */
void yhs_delete(yhashstack_t *hashstack);

/**
 * @function	yhs_duplicate
 *		Duplicate a hash stack.
 * @param	hashstack	The hash stack.
 * @return	The duplicated hash stack. It has the same size and content.
 */
yhashstack_t yhs_duplicate(yhashstack_t hashstack);

/**
 * @function	yhs_get_last_hash
 *		Returns a pointer to the last hash table of a hash stack.
 * @param	hashstack	The hash stack.
 * @return	A pointer to the hash stack's last hash table.
 */
yhashtable_t *yhs_get_last_hash(yhashstack_t hashstack);

/**
 * @function	yhs_push_hash
 *		Add a hash table at the end of a hash stack, only if it doesn't
 *		already exists in the stack.
 * @param	hashstack	Pointer to the hash stack.
 * @param	hashtable	Pointer to the hashtable.
 */
void yhs_push_hash(yhashstack_t *hashstack, yhashtable_t *hashtable); 

/**
 * @function	yhs_pop_hash
 *		Remove the last hash table of a hash stack, and return it.
 * @param	hashstack	The hash stack.
 * @return	A pointer to the removed hash table.
 */
yhashtable_t *yhs_pop_hash(yhashstack_t hashstack);

/**
 * @function	yhs_search_from_string
 *		Search an element in a hash stack, from its string key.
 * @param	hashstack	The hash stack.
 * @param	key		Key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yhs_search_from_string(yhashstack_t hashstack, const char *key);

/**
 * @function	yhs_search_from_int
 *		Search an element in a hash stack, from its integer key.
 * @param	hashstack	The hash stack.
 * @param	key		Key used to index the element.
 * @return	A pointer to the element's data.
 */
void *yhs_search_from_int(yhashstack_t hashstack, size_t key);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YHASHSTACK_H__ */
