/**
 * @header	ybase64.h
 * @discussion	Definitions for base64 encoding.
 * @version	1.0.0 Aug 09 2002
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "y.h"

/**
 * @function	ybase64_encode
 *		Encode binary data to base64 form.
 * @param	bin	Binary data.
 * @return	The encoded string, or NULL if an error occurs.
 */
char *ybase64_encode(ybin_t bin);

/**
 * @function	ybase64_decode
 *		Decode base64 encoded string.
 * @param	pt	Base64 encoded string.
 * @return	The binary data.
 */
ybin_t ybase64_decode(const char *pt);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

