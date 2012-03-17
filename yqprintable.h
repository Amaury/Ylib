/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	yqprintable.h
 * @discussion	Definitions for quoted-printable encoding/decoding.
 * @version	1.0.0 Jun 13 2003
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YQPRINTABLE_H__
#define __YQPRINTABLE_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "ydefs.h"
#include "ystr.h"

/*!
 * @function	yqprintable_decode
 *		Decode quoted-printable encoded string.
 * @param	str	Quoted-printable encoded string.
 * @return	The binary data.
 */
ybin_t yqprintable_decode(const char *str);

/*!
 * @function	yqprintable_encode
 *		Encode binary data to quoted-printable string.
 * @param	bin	The binary data.
 * @return	The quoted-printable encoded string.
 */
ystr_t yqprintable_encode(const ybin_t bin);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YQPRINTABLE_H__ */
