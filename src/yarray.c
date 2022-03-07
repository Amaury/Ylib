#include <math.h>
#include <string.h>
#include <stdio.h>
#include "y.h"

/* ************ PRIVATE DEFINITIONS AND MACROS ************ */
/** @define _YARRAY_DEFAULT_SIZE DEfault size of yarrays. */
#define _YARRAY_DEFAULT_SIZE     256
/** @define _YARRAY_SIZE Compute the size of a new yarray's buffer. */
#define _YARRAY_SIZE(s)	COMPUTE_SIZE((s), _YARRAY_DEFAULT_SIZE)
/** @define _YARRAY_HEAD Get a pointer to a yarray's header. */
#define _YARRAY_HEAD(p)  ((yarray_head_t*)((void*)(p) - sizeof(yarray_head_t)))

/* ************ PRIVATE STRUCTURES AND TYPES ************** */
/**
 * @typedef	yarray_head_s
 *		Structure used for the head of yarrays.
 * @field	total	Total size of the yarray.
 * @field	used	Used size of the yarray.
 */
typedef struct {
	size_t total;
	size_t used;
} yarray_head_t;

/* ************ FUNCTIONS ************* */

/* Create a new yarray of the default size. */
yarray_t yarray_new() {
	return (yarray_create(_YARRAY_DEFAULT_SIZE));
}
/* Creates a new yarray of the given size. */
yarray_t yarray_create(size_t size) {
	void **nv;
	yarray_head_t *y;

	size = _YARRAY_SIZE(size);
	if (!(nv = (void**)malloc0((size * sizeof(void*)) + sizeof(yarray_head_t)))) {
		return (NULL);
	}
	y = (yarray_head_t*)nv;
	nv = (void**)((void*)nv + sizeof(yarray_head_t));
	y->total = size;
	y->used = 0;
	*nv = NULL;
	return ((yarray_t)nv);
}
/* Delete an yarray. Its content is not freed. */
void *yarray_free(yarray_t a) {
	yarray_delete(&a, NULL, NULL);
	return (NULL);
}
/* Delete an existing yarray. */
void yarray_delete(yarray_t *v, yarray_function_t f, void *data) {
	yarray_head_t *y;
	size_t i;

	if (!v || !*v)
		return;
	y = (yarray_head_t*)((void*)*v - sizeof(yarray_head_t));
	if (f) {
		for (i = 0; i < y->used; ++i)
			f(i, (*v)[i], data);
	}
	free0(y);
	*v = NULL;
}
/* Truncate an existing yarray. The allocated memory doesn't change. */
void yarray_trunc(yarray_t v, yarray_function_t f, void *data) {
	yarray_head_t *y;
	size_t i;

	if (!v)
		return;
	y = _YARRAY_HEAD(v);
	if (f) {
		for (i = 0; i < y->used; ++i) {
			f(i, v[i], data);
		}
	}
	y->used = 0;
	*v = NULL;
}
/* Set the minimum size of a yarray. */
ystatus_t yarray_resize(yarray_t *v, size_t sz) {
	yarray_head_t *y, *ny;
	void **nv;

	if (!v || !*v)
		return (YENOERR);
	y = _YARRAY_HEAD(v);
	if (sz < y->total)
		return (YENOERR);
	sz = _YARRAY_SIZE(sz);
	nv = (void**)malloc0((sz * sizeof(void*)) + sizeof(yarray_head_t));
	if (!nv)
		return (YENOMEM);
	ny = (yarray_head_t*)nv;
	nv = (void**)((void*)nv + sizeof(yarray_head_t));
	ny->total = sz;
	ny->used = y->used;
	memcpy(nv, **v, (y->used + 1) * sizeof(void*));
	free0(y);
	*v = nv;
	return (YENOERR);
}
/* Return the length of a yarray (its used size). */
size_t yarray_length(const yarray_t v) {
	if (!v)
		return (0);
	return (_YARRAY_HEAD(v)->used);
}
/* Return the size of a yarray (its allocated size). */
size_t yarray_size(const yarray_t v) {
	if (!v)
		return (0);
	return (_YARRAY_HEAD(v)->total);
}
/* Concatenate a yarray at the end of another one. */
ystatus_t yarray_append(yarray_t *dest, const yarray_t src) {
	size_t srcsz, arraysz, totalsz;
	yarray_head_t *y, *ny;
	void **nv;

	if (!src || !dest || !(srcsz = yarray_length(src)))
		return (YENOERR);
	if (!*dest) {
		if (!(*dest = yarray_clone(src)))
			return (YENOMEM);
		return (YENOERR);
	}
	y = _YARRAY_HEAD(*dest);
	if ((y->used + 1 + srcsz) <= y->total) {
		void *dest_pt = (void*)((size_t)*dest + (y->used * sizeof(void*)));
		memcpy(dest_pt, src, (srcsz + 1) * sizeof(void*));
		y->used += srcsz;
		return (YENOERR);
	}
	arraysz = y->used + srcsz;
	totalsz = _YARRAY_SIZE(arraysz);
	nv = (void**)malloc0((totalsz * sizeof(void*)) + sizeof(yarray_head_t));
	if (!nv)
		return (YENOMEM);
	ny = (yarray_head_t*)nv;
	nv = (void**)((void*)nv + sizeof(yarray_head_t));
	ny->total = totalsz;
	ny->used = arraysz;
	memcpy(nv, *dest, y->used * sizeof(void*));
	memcpy(nv + y->used, src, (srcsz + 1) * sizeof(void*));
	free0(y);
	*dest = nv;
	return (YENOERR);
}
/* Concatenate a given number of elements from a yarray at the end of another. */
ystatus_t yarray_nappend(yarray_t *dest, const yarray_t src, size_t n) {
	size_t arraysz, totalsz;
	yarray_head_t *y, *ny;
	void **nv;

	if (!src || !dest || !*dest || !n)
		return (YENOERR);
	y = _YARRAY_HEAD(src);
	n = (n < y->used) ? n : y->used;
	y = _YARRAY_HEAD(*dest);
	if ((y->used + 1 + n) <= y->total) {
		memcpy(*dest + y->used, src, n * sizeof(void*));
		y->used += n;
		(*dest)[y->used] = NULL;
		return (YENOERR);
	}
	arraysz = y->used + n;
	totalsz = _YARRAY_SIZE(arraysz);
	nv = (void**)malloc0((totalsz * sizeof(void*)) + sizeof(yarray_head_t));
	if (!nv)
		return (YENOMEM);
	ny = (yarray_head_t*)nv;
	nv = (void**)((void*)nv + sizeof(yarray_head_t));
	ny->total = totalsz;
	ny->used = arraysz;
	memcpy(nv, *dest, y->used * sizeof(void*));
	memcpy(nv + y->used, src, n * sizeof(void*));
	nv[ny->used] = NULL;
	free0(y);
	*dest = nv;
	return (YENOERR);
}
/* Duplicate a yarray. */
yarray_t yarray_clone(const yarray_t v) {
	yarray_head_t *y, *ny;
	void **nv;

	if (!v)
		return (yarray_new());
	y = _YARRAY_HEAD(v);
	nv = (void**)malloc0((y->total * sizeof(void*)) + sizeof(yarray_head_t));
	ny = (yarray_head_t*)nv;
	nv = (void**)((void*)nv + sizeof(yarray_head_t));
	ny->total = y->total;
	ny->used = y->used;
	memcpy(nv, v, (y->used + 1) * sizeof(void*));
	return (nv);
}
/* Merge 2 yarrays to create a new one. */
yarray_t yarray_merge(const yarray_t v1, const yarray_t v2) {
	yarray_t nv;

	nv = yarray_clone(v1);
	if (!nv)
		return (NULL);
	if (yarray_append(&nv, v2) != YENOERR) {
		yarray_free(nv);
		return (NULL);
	}
	return (nv);
}
/* Add an element at the beginning of a yarray. */
ystatus_t yarray_add(yarray_t *v, void *e) {
	size_t totalsz;
	int i;
	yarray_head_t *y, *ny;
	void **nv;

	if (!v || !*v)
		return (YENOERR);
	y = _YARRAY_HEAD(*v);
	if ((y->used + 2) <= y->total) {
		for (i = y->used; i >= 0; --i)
			(*v)[i + 1] = (*v)[i];
		(*v)[0] = e;
		y->used++;
		return (YENOERR);
	}
	totalsz = _YARRAY_SIZE(y->total + 2);
	nv = (void**)malloc0((totalsz * sizeof(void*)) + sizeof(yarray_head_t));
	if (!nv)
		return (YENOMEM);
	ny = (yarray_head_t*)nv;
	nv = (void**)((void*)nv + sizeof(yarray_head_t));
	ny->total = totalsz;
	ny->used = y->total + 1;
	nv[0] = e;
	memcpy((void*)((void*)nv + sizeof(void*)), *v, (y->used + 1) * sizeof(void*));
	free0(y);
	*v = nv;
	return (YENOERR);
}
/* Add an element at the end of a yarray. */
ystatus_t yarray_push(yarray_t *v, void *e) {
	size_t totalsz;
	yarray_head_t *y, *ny;
	void **nv;

	if (!v || !*v)
		return (YENOERR);
	y = _YARRAY_HEAD(*v);
	if ((y->used + 2) <= y->total) {
		(*v)[y->used] = e;
		(*v)[y->used + 1] = NULL;
		y->used++;
		return (YENOERR);
	}
	totalsz = _YARRAY_SIZE(y->total + 2);
	nv = (void**)malloc0((totalsz * sizeof(void*)) + sizeof(yarray_head_t));
	if (!nv)
		return (YENOMEM);
	ny = (yarray_head_t*)nv;
	nv = (void**)((void*)nv + sizeof(yarray_head_t));
	ny->total = totalsz;
	ny->used = y->used + 1;
	memcpy(nv, *v, y->used * sizeof(void*));
	nv[y->used] = e;
	nv[ny->used] = NULL;
	free0(y);
	*v = nv;
	return (YENOERR);
}
/* Add multiple elements at the end of a yarray. */
ystatus_t yarray_push_multi(yarray_t *v, size_t n, ...) {
	va_list p_list;

	if (!v || !*v)
		return (YENOERR);
	va_start(p_list, n);
	for (; n > 0; --n) {
		void *e = va_arg(p_list, void*);
		ystatus_t st = yarray_add(v, e);
		if (st != YENOERR)
			return (st);
	}
	va_end(p_list);
	return (YENOERR);
}
/*
 * Insert an element at the given offset of a yarray. All elements
 * placed at this offset and after are shifted.
 */
ystatus_t yarray_insert(yarray_t *v, void *e, size_t i) {
	if (!v || !*v)
		return (YENOERR);
	yarray_head_t *y = _YARRAY_HEAD(*v);
	ystatus_t st = yarray_resize(v, y->used + 2);
	if (st != YENOERR)
		return (st);
	if (i > y->used) {
		yarray_add(v, e);
		return (YENOERR);
	}
	for (size_t j = i; j < (y->used + 1); ++j)
		(*v)[j + 1] = (*v)[j];
	(*v)[i] = e;
	y->used++;
	return (YENOERR);
}
/* Replace the element at the given offset of a yarray. */
ystatus_t yarray_set(yarray_t v, void *e, size_t i, yarray_function_t f, void *data) {
	if (!e)
		return (YENOERR);
	if (!v)
		return (YEFAULT);
	yarray_head_t *y = _YARRAY_HEAD(v);
	if (i >= y->total)
		return (YEFAULT);
	if (!v[i]) {
		// no existing element
		v[i] = e;
		y->used++;
		return (YENOERR);
	}
	if (v[i] && f) {
		// remove the existing element
		ystatus_t st = f(i, v[i], data);
		if (st != YENOERR)
			return (st);
	}
	v[i] = e;
	return (YENOERR);
}
/* Returns the first element of a yarray. Equivalent to array[0]. */
void *yarray_get_first(const yarray_t v) {
	if (!v)
		return (NULL);
	yarray_head_t *y = _YARRAY_HEAD(v);
	if (!y->used)
		return (NULL);
	return (v[0]);
}
/* Returns the last element of a yarray. */
void *yarray_get_last(const yarray_t v) {
	if (!v)
		return (NULL);
	yarray_head_t *y = _YARRAY_HEAD(v);
	if (!y->used)
		return (NULL);
	return (v[y->used - 1]);
}
/* Returns the nth element of a yarray. */
void *yarray_get(const yarray_t v, size_t i) {
	if (!v)
		return (NULL);
	yarray_head_t *y = _YARRAY_HEAD(v);
	if (y->used <= i)
		return (NULL);
	return (v[i]);
}
/* Remove the first element of a yarray and return it. */
void *yarray_shift(yarray_t v) {
	if (!v)
		return (NULL);
	yarray_head_t *y = _YARRAY_HEAD(v);
	if (!y->used)
		return (NULL);
	void *res = *v;
	for (size_t i = 0; i < y->used; ++i)
		v[i] = v[i + 1];
	y->used--;
	return (res);
}
/* Remove the last element of a yarray and return it. */
void *yarray_pop(yarray_t v) {
	if (!v)
		return (NULL);
	yarray_head_t *y = _YARRAY_HEAD(v);
	if (!y->used)
		return (NULL);
	void *res = v[y->used - 1];
	v[y->used - 1] = NULL;
	y->used--;
	return (res);
}
/* Extract the element placed at the given offset of a yarray. */
void *yarray_extract(yarray_t v, size_t i) {
	if (!v)
		return (NULL);
	yarray_head_t *y = _YARRAY_HEAD(v);
	if (i >= y->used)
		return (NULL);
	void *res = v[i];
	for (size_t j = i; j < y->used; ++j)
		v[j] = v[j + 1];
	y->used--;
	return (res);
}
/* Remove all values of a yarray to let only one entry of each value. */
void yarray_uniq(yarray_t v) {
	if (!v)
		return ;
	yarray_head_t *y = _YARRAY_HEAD(v);
	for (size_t i = 0; i < y->used; ++i) {
		for (size_t j = i + 1; j < y->used; ++j) {
			if (v[i] == v[j]) {
				for (size_t k = j + 1; k < (y->used - 1); ++k)
					v[k] = v[k + 1];
				y->used--;
			}
		}
	}
}
/* Do a quick sort of all elements of a yarray. */
void yarray_sort(yarray_t v, int (*f)(const void*, const void*)) {
	if (!v || !f)
		return;
	yarray_head_t *y = _YARRAY_HEAD(v);
	qsort(v, y->used, sizeof(void*), f);
}
/* Search the offset of an element in a yarray. */
long long int yarray_search(const yarray_t v, void *e, int (*f)(const void*, const void*)) {
	if (!v || !f)
		return (-1);
	yarray_head_t *y = _YARRAY_HEAD(v);
	int o_start = 0;
	int o_end = y->used - 1;
	int o_pivot, cmp_res;
	for (; ; ) {
		o_pivot = (o_end + o_start) / 2;
		if (!(cmp_res = f(e, v[o_pivot])))
			return (o_pivot);
		if (cmp_res < 0) {
			if (o_pivot == (o_start + 1)) {
				if (!f(e, v[o_start]))
					return (o_start);
				else
					return (-1);
			}
			o_end = o_pivot;
		} else {
			if (o_pivot == (o_end - 1)) {
				if (!f(e, v[o_end]))
					return (o_end);
				else
					return (-1);
			}
			o_start = o_pivot;
		}
	}
}
/* Apply a function on every elements of an array. */
ystatus_t yarray_foreach(yarray_t v, yarray_function_t func, void *user_data) {
	size_t offset;
	yarray_head_t *y; 

	y = _YARRAY_HEAD(v);
	for (offset = 0; offset < y->used; ++offset) {
		ystatus_t st = func(offset, v[offset], user_data);
		if (st != YENOERR)
			return (st);
	}
	return (YENOERR);
}

