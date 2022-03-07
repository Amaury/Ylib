#include "ymemory.h"
#include "ystatus.h"

/* Allocate memory. */
void *malloc0(size_t size) {
	void *p;
#ifdef USE_BOEHM_GC
	p = GC_MALLOC(size);
#else
	p = calloc(1, size);
#endif
	if (!p) {
		return (NULL);
	}
	return (p);
}
/* Allocate memory. */
void *calloc0(size_t nmemb, size_t size) {
	void *p;
#ifdef USE_BOEHM_GC
	p = GC_MALLOC(n * s);
#else
	p = calloc(nmemb, size);
#endif
	if (!p) {
		return (NULL);
	}
	return (p);
}

