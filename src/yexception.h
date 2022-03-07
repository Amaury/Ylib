/**
 * @header	yexception.h
 * @abstract	Exceptions handling.
 * @discussion	Exceptions are used for error management, and provide non-local goto.
 *		<p>
 *		Code is written in a ytry block. Exceptions are thrown using the ythrow()
 *		instruction. They are catched using ycatch() blocks.
 *		</p>
 *		<p>
 *		An exception has a string message and a numerical code. Catching is done
 *		for a given code. All exceptions (regardless of their codes) could be catch
 *		with a yexcept block.
 *		</p>
 *		<p>
 *		If an exception is not catched by a ycatch() or a yexcept block following
 *		the ytry block, it is thrown to the upper-level block.
 *		If an exception is not catchable, it is written to stderr and the program
 *		is terminated (with an exit value equals to the exception code).
 *		</p>
 *		<p>
 *		Any positive or negative integer value could be used as an exception code.
 *		It is recommended to use errors codes from ystatus.h when possible. If you
 *		define your own codes, always use positive integers greater than 1.
 *		</p>
 *		<p>
 *		Warning: Inside ytry, ycatch and yexcept blocks, there must be no statement
 *		that could get the processing out of the block (i.e. no return, no goto).
 *		</p>
 *		Example:
 *		<code>
 *			#define EXCEPT_A 1
 *			#define EXCEPT_B 2
 *			void go() {
 *				ytry {
 *					ythrow("bla bla", EXCEPT_B);
 *				} ycatch(EXCEPT_A) {
 *					printf("A exception catched: %s.\n", YEXCEPT_MSG);
 *				}
 *			}
 *			ytry {
 *				go();
 *			} ycatch(EXCEPT_A) {
 *				// will never come here, because EXCEPT_A exceptions are
 *				// catched in the go() function
 *			} ycatch(EXCEPT_B) {
 *				printf("B exception from file '%s' line '%d'.\n",
 *				       EXCEPT_FILE, EXCEPT_LINE);
 *			} yexcept {
 *				printf("Some exception raised '%s' (%d).\n",
 *				       EXCEPT_MSG, EXCEPT_CODE);
 *			}
 *		</code>
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include <setjmp.h>
#include "y.h"

/**
 * @typedef	_yexcept_t
 *		Must not be used directly.
 */
typedef struct _yexcept_s _yexcept_t;

/**
 * @typedef	_yexcept_stack_t
 *		Stack element.
 * @field	env		Jump buffer.
 * @field	status		Status of the exception.
 * @field	catched		True if the exception was catched.
 * @field	msg		Message of the raised exception.
 * @field	code		Code of the raised exception.
 * @field	filename	File name of the stack creation.
 * @field	line		Line number of the stack creation.
 */
typedef struct _yexcept_stack_s _yexcept_stack_t;

#ifndef __Y_EXCEPTION_C__
extern _yexcept_t __g_yexception;
#endif

/** @define YEXCEPT_MSG Message from the thrown exception. */
#define YEXCEPT_MSG	(((_yexcept_stack_t*)yarray_get_last(__g_yexception.stack))->msg)

/** @define YEXCEPT_CODE Code of the thrown exception. */
#define YEXCEPT_CODE	(((_yexcept_stack_t*)yarray_get_last(__g_yexception.stack))->code)

/** @define YEXCEPT_LINE Line of the thrown exception. */
#define YEXCEPT_LINE	(((_yexcept_stack_t*)yarray_get_last(__g_yexception.stack))->line)

/** @define YEXCEPT_FILE Filename of the thrown exception. */
#define YEXCEPT_FILE	(((_yexcept_stack_t*)yarray_get_last(__g_yexception.stack))->filename)

/** @define ytry	Open an exception environment. */
#define ytry	_Pragma("GCC diagnostic ignored \"-Wdangling-else\""); \
		if (setjmp(*_yexcept_start(__FILE__, __LINE__)) >= 0) \
			for (int loops = 0; true; ++loops) \
				if (loops >= 2) { \
					/* uncatched exception */ \
					_yexcept_process_uncatched(); \
					break; \
				} else if (!loops && _yexcept_is_created()) \
					/* code inside 'ytry' block */

/**
 * @define ycatch	Catch an exception.
 * @param	int	code	Code of the exception.
 */
#define ycatch(code)		else if (_yexcept_catch(((int)code)))  \
					/* code inside 'ycatch' block */

/** @define yexcept Catch an uncatched exception. */
#define yexcept			else if (_yexcept_catch(0)) \
					/* code inside 'yexcept' block */

/**
 * @define ythrow	Throw an exception.
 * @param	string	m	Message of the exception.
 * @param	int	c	Code of the exception.
 */
#define ythrow(m, c)	_yexcept_throw(m, c, __FILE__, __LINE__)

/**
 * @function	yexcept_disable
 *		Disable exceptions management. ythrow will do nothing.
 */
void yexcept_disable(void);
/**
 * @function	yexcept_stack_trace
 *		Write the stack trace to stderr.
 */
void yexcept_stack_trace(void);
/**
 * @function	_yexcept_start
 *		Add a level of exceptions handling.
 * @param	filename	Name of the file where the 'try' was added.
 * @param	line		Number of the line where the 'try' was added.
 * @return	A pointer to the allocated environment.
 */
jmp_buf *_yexcept_start(char *filename, int line);
/**
 * @function	_yexcept_throw
 *		Store all needed information when an exception is raised.
 *		THIS FUNCTION IS CALLED AUTOMATICALLY. IT MUST NOT BE CALLED BY USERS.
 * @param	msg		Exception's message.
 * @param	code		Exception's code.
 * @param	filename	Name of the file where the exception was raised.
 * @param	line		Line of the file where the exception was raised.
 */
void _yexcept_throw(char *msg, int code, char *filename, int line);
/**
 * @function	_yexcept_process_uncatched
 *		Look at the last exception and check if it was catched or not.
 *		THIS FUNCTION IS CALLED AUTOMATICALLY. IT MUST NOT BE CALLED BY USERS.
 */
void _yexcept_process_uncatched(void);
/**
 * @function	_yexcept_catch
 *		Check if an exception match a given code.
 * @param	code	Code to compare. 0 for the catchall.
 * @return	True if this is the code of the exception.
 */
bool _yexcept_catch(int code);
/**
 * @function	_yexcept_is_created
 *		Tell if an exception has been created, but not raised nor catched.
 *		THIS FUNCTION IS CALLED AUTOMATICALLY. IT MUST NOT BE CALLED BY USERS.
 * @return	True if an exception has been created.
 */
bool _yexcept_is_created(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

