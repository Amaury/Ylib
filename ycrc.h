/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	ycrc.h
 * @abstract	All definitions about CRC computing.
 * @discussion	CRC are a method for checksum computing.
 * @version	1.0 Aug 13 2003
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YCRC_H__
#define __YCRC_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "ydefs.h"

/*! @typedef ycrc_t A CRC value is 32 bits wide. */
typedef u32_t ycrc_t;

/*!
 * @function	ycrc_init
 *		Initialize a CRC value before computing.
 * @return	A CRC value that must be used before computing.
 */
ycrc_t ycrc_init(void);

/*!
 * @function	ycrc_add_char
 *		Add a character value to the CRC computing.
 * @param	crc	A pointer to the CRC value.
 * @param	c	The character.
 */
void ycrc_add_char(ycrc_t *crc, char c);

/*!
 * @function	ycrc_add_str
 *		Add a string value to the CRC computing.
 * @param	crc	A pointer to the CRC value.
 * @param	s	The character string.
 */
void ycrc_add_str(ycrc_t *crc, char *s);

/*!
 * @function	ycrc_add_bin
 *		Add a binary value to the CRC computing.
 * @param	crc	A pointer to the CRC value.
 * @param	bin	A pointer to the binary value.
 */
void ycrc_add_bin(ycrc_t *crc, ybin_t *bin);

/*!
 * @function	ycrc_compute
 *		End the CRC computing.
 * @param	crc	A pointer to the CRC value.
 * @return	The calculated CRC value.
 */
ycrc_t ycrc_compute(ycrc_t *crc);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YCRC_H__ */
