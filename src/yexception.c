#include <stdio.h>
#define __YEXCEPTION_C__
#include "yexception.h"

/* *** PRIVATE STRUCTURES *** */
/**
 * @struct	_yexcept_s
 * @field	stack		Exception stack.
 * @field	disabled	Tell if exceptions are disabled.
 * @field	filename	File name where the exception was raised.
 * @field	line		Line number where the exception was raised.
 */
struct _yexcept_s {
	yarray_t stack;
	bool disabled;
	char *filename;
	int line;
};

/**
 * @struct	_yexcept_stack_s
 *		Stack element.
 * @field	env		Jump buffer.
 * @field	status		Status of the exception.
 * @field	catched		True if the exception was catched.
 * @field	msg		Message of the raised exception.
 * @field	code		Code of the raised exception.
 * @field	filename	File name of the stack creation.
 * @field	line		Line number of the stack creation.
 */
struct _yexcept_stack_s {
	jmp_buf env;
	enum {
		YEXCEPT_CREATED = 0,
		YEXCEPT_RAISED,
		YEXCEPT_CATCHED
	} status;
	bool raised;
	bool catched;
	char *msg;
	int code;
	char *filename;
	int line;
};

/** Exceptions stack. */
_yexcept_t __g_yexception = {0};

/* Disable exceptions management. ythrow() will do nothing. */
void yexcept_disable(void) {
	__g_yexception.disabled = true;
}
/* Write the stack trace to stderr. */
void yexcept_stack_trace() {
	fprintf(stderr, "STACK TRACE:\n");
	size_t i;
	if (!__g_yexception.stack || !(i = yarray_length(__g_yexception.stack))) {
		fprintf(stderr, "Empty\n\n");
		fflush(stderr);
		return;
	}
	_yexcept_stack_t *elem;
	for (; i > 0; --i) {
		elem = __g_yexception.stack[i - 1];
		fprintf(stderr, "From file '%s' line %d\n", elem->filename, elem->line);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}
/* Add a level of exceptions handling. */
jmp_buf *_yexcept_start(char *filename, int line) {
	if (!__g_yexception.stack)
		__g_yexception.stack = yarray_new();
	_yexcept_stack_t *stack_elem = malloc0(sizeof(_yexcept_stack_t));
	stack_elem->line = line;
	stack_elem->filename = filename;
	yarray_push(&__g_yexception.stack, stack_elem);
	return (&stack_elem->env);
}
/* Throw an exception. */
void _yexcept_throw(char *msg, int code, char *filename, int line) {
	// do nothing if the exceptions management was disabled
	if (__g_yexception.disabled)
		return;
	while (true) {
		_yexcept_stack_t *elem = yarray_get_last(__g_yexception.stack);
		if (!elem) {
			fprintf(stderr, "Uncatched exception (file '%s' l. %d)\n"
			                "| Code: %d\n| Message: %s\n\n",
			       filename,  line, code, msg);
			fflush(stderr);
			yexcept_stack_trace();
			exit(code);
		}
		if (elem->status != YEXCEPT_CREATED) {
			yarray_pop(__g_yexception.stack);
			continue;
		}
		elem->status = YEXCEPT_RAISED;
		elem->msg = msg;
		elem->code = code;
		elem->filename = filename;
		elem->line = line;
		longjmp(elem->env, 1);
	}
}
/* Process an uncatched exception. */
void _yexcept_process_uncatched() {
	_yexcept_stack_t *elem = yarray_pop(__g_yexception.stack);
	if (!elem)
		return;
	// process only raised but not catched exceptions
	if (elem->status == YEXCEPT_CREATED ||
	    elem->status == YEXCEPT_CATCHED)
		return;
	_yexcept_stack_t *elem2 = yarray_get_last(__g_yexception.stack);
	if (!elem2) {
		fprintf(stderr, "Uncatched exception (file '%s' l. %d)\n"
		                "| Code: %d\n| Message: %s\n\n",
		        elem->filename, elem->line, elem->code, elem->msg);
		fflush(stderr);
		yexcept_stack_trace();
		exit(elem->code);
	}
	elem2->status = YEXCEPT_RAISED;
	elem2->msg = elem->msg;
	elem2->code = elem->code;
	elem2->filename = elem->filename;
	elem2->line = elem->line;
	free0(elem);
	longjmp(elem2->env, 1);
	return;
}
/* Check if an exception match a given code. */
bool _yexcept_catch(int code) {
	_yexcept_stack_t *elem = yarray_get_last(__g_yexception.stack);
	if (!elem || elem->status != YEXCEPT_RAISED ||
	    (code && elem->code != code))
		return (false);
	elem->status = YEXCEPT_CATCHED;
	return (true);
}
/* Tell if an exception has been created, but not raised nor catched. */
bool _yexcept_is_created() {
	_yexcept_stack_t *elem = yarray_get_last(__g_yexception.stack);
	if (!elem || elem->status != YEXCEPT_CREATED)
		return (false);
	return (true);
}

