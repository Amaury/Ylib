#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include <stdlib.h>

/** @define free0 Memory liberation macro. */
#ifdef USE_BOEHM_GC
# define free0(p)	((void*)p ? (GC_FREE((void*)p), NULL) : NULL, p = NULL)
#else
# define free0(p)	((void*)p ? (free((void*)p), NULL) : NULL, p = NULL)
#endif // USE_BOEHM_GC
/** @define YMALLOC Memory allocation macro (for backward compatibility). */
#define YMALLOC(s)	malloc0(s)
/*! @define YCALLOC Memory allocation macro (for backward compatibility). */
#define YCALLOC(n, s)	calloc0(n, s)
/** @define YFREE Memory liberation macro (for backward compatibility). */
#define YFREE(p)	free0(p)

/**
 * @function	malloc0
 *		Memory allocation.
 * @param	size	Number of bytes to allocate.
 * @return	A pointer to the allocated data.
 * @throws	YEXCEPT_NOMEM if the allocation failed.
 */
void *malloc0(size_t size);
/**
 * @function	calloc0
 *		Memory allocation.
 * @param	nmemb	Number of elements to allocate.
 * @param	size	Number of bytes of each allocated element.
 * @return	A pointer to the allocated data.
 * @throws	YEXCEPT_NOMEM if the allocation failed.
 */
void *calloc0(size_t nmemb, size_t size);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */
