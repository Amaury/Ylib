/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	yvalue.h
 * @abstract	All definitions about values and units.
 * @discussion	Use these structures and functions to convert values
 *		depending of the desired unit.
 * @version	1.0 Aug 19 2003
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YVALUE_H__
#define __YVALUE_H__

/*! @define YVAL2PT Return a value in points. */
#define YVAL2PT(v)	yvalue_get(v, YUNIT_PT)
/*! @define YVAL2MM Return a value in millimeters. */
#define YVAL2MM(v)	yvalue_get(v, YUNIT_MM)
/*! @define YVAL2CM Return a value in centimeters. */
#define YVAL2CM(v)	yvalue_get(v, YUNIT_CM)
/*! @define YVAL2IN Return a value in inchs. */
#define YVAL2IN(v)	yvalue_get(v, YUNIT_IN)

/*!
 * @typedef	yunit_t
 *		Enumeration of known units.
 * @constant	YUNIT_PT	Screen point (in 72 dpi).
 * @constant	YUNIT_MM	Millimeter.
 * @constant	YUNIT_CM	Centimeter.
 * @constant	YUNIT_IN	Inch.
 */
typedef enum yunit_e
{
  YUNIT_PT = 0,
  YUNIT_MM,
  YUNIT_CM,
  YUNIT_IN
} yunit_t;

/*!
 * @typedef	yvalue_t
 *		Structure used to describe a value, depending
 *		of its unit.
 * @field	unit	Value's unit.
 * @field	value	Value's value.
 */
typedef struct yvalue_s
{
  yunit_t unit;
  float value;
} yvalue_t;

/*!
 * @function	yvalue_read
 *		Read a value from a character string.
 * @param	str		The character string to read.
 * @param	default_unit	The unit to use if not specified
 *				in the string.
 * @return	The readed value.
 */
yvalue_t yvalue_read(const char *str, yunit_t default_unit);

/*!
 * @function	yvalue_get
 *		Return a value in the desired unit.
 * @param	value	The value.
 * @param	unit	The unit.
 * @return	The value in the specified unit.
 */
float yvalue_get(const yvalue_t value, yunit_t unit);

#endif /* __YVALUE_H__ */
