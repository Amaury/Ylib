/**
 * @header	yresult.h
 * @abstract	Management of function results.
 * @version	1.0.0, Feb 27 2022
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "y.h"

/* ********** MACROS DEFINTIONS ********** */

/** @define YRES	Return the ystatus of a yres_t-derived type. */
#define YRES_STATUS(y)	((y).status)
/** @define YRES_VAL	Return the value of a yres_t-derived type. */
#define YRES_VAL(y)	((y).value)
/** @define YRES_ASSERT	Check if a result has error. If yes, the program is stopped. */
#define YASSERT(y, ...)	((YRES_STATUS(y) == YENOERR) ? \
			 (y.value) : \
			 (YLOG_ADD(YLOG_ERR, __VA_ARGS__), exit(YRES_STATUS(y)), 0))
/** @define YRETURN	Define a result with an error code and a value. */
#define YRESULT(_type, _status, _value)	((_type){.status = _status, .value = _value})
/** @define YRETURN_ERR	Define a result with an error code (no value). */
#define YRESULT_ERR(_type, _status)	((_type){.status = _status})
/** @define YRETURN_VAL	Define a result with a value (no error). */
#define YRESULT_VAL(_type, _value)	((_type){.status = YENOERR, .value = _value})

/* ********** TYPE DEFINITIONS ********** */

/** @typedef yres_t Base type for result returned by functions. */
typedef union {
	ystatus_t status;
	ystatus_t value;
} yres_t;
/** @typedef yres_bool_t	Boolean result. */
typedef struct {
	ystatus_t status;
	bool      value;
} yres_bool_t;
/** @typedef yres_int_t	Integer result. */
typedef struct {
	ystatus_t status;
	int64_t   value;
} yres_int_t;
/** @typedef yres_float_t Floating-point number result. */
typedef struct {
	ystatus_t status;
	double    value;
} yres_float_t;
/** @typedef yres_pointer_t	Generic pointer result. */
typedef struct {
	ystatus_t status;
	void      *value;
} yres_pointer_t;
/** @typedef yres_str_t	String result. */
typedef struct {
	ystatus_t status;
	ystr_t    value;
} yres_str_t;
/** @typedef yres_array_t	Array result. */
typedef struct {
	ystatus_t status;
	yarray_t  value;
} yres_array_t;
/** @typedef yres_hashmap_t	Hashmap result. */
typedef struct {
	ystatus_t         status;
	struct yhashmap_s *value;
} yres_hashmap_t;
/** @typedef yres_var_t	yvar result. */
typedef struct {
	ystatus_t     status;
	struct yvar_s *value;
} yres_var_t;

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */
