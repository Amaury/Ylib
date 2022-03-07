/**
 * @header	y.h
 * @abstract	Basic definitions.
 * @discussion	This file contains all basic definitions used by "ylib". That
 *		includes types, characters and numbers definitions and basic
 *		macros.
 * @version	1.0.0 Jun 28 2002
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>
#include "ystatus.h"
#include "ymemory.h"
#include "ybin.h"
#include "ystr.h"
#include "yarray.h"
#include "ybase64.h"
#include "ycgi.h"
#include "ydaemon.h"
#include "ydom.h"
#include "yexception.h"
#include "yhash.h"
#include "yhashmap.h"
#include "yhashtable.h"
#include "yini.h"
#include "ylist.h"
#include "ylock.h"
#include "ylog.h"
#include "yqprintable.h"
#include "ysax.h"
#include "ytimer.h"
#include "yurl.h"
#include "yvalue.h"
#include "yresult.h"
#include "yvar.h"
#include "yjson.h"

#ifndef __FUNCTION__
# define __FUNCTION__	__func__
#endif /* __FUNCTION__ */

/** @define TAB Character definition. */
#define	TAB		'\t'
/** @define LF Character definition. */
#define	LF		'\n'
/** @define CR Character definition. */
#define CR		'\r'
/** @define SPACE Character definition. */
#define	SPACE		' '
/** @define EXCLAM Character definition. */
#define	EXCLAM		'!'
/** @define DQUOTE Character definition. */
#define	DQUOTE		'\"'
/** @define SHARP Character definition. */
#define	SHARP		'#'
/** @define DOLLAR Character definition. */
#define DOLLAR		'$'
/** @define PERCENT Character definition. */
#define PERCENT		'%'
/** @define AMP Character definition. */
#define	AMP		'&'
/** @define QUOTE Character definition. */
#define	QUOTE		'\''
/** @define LPAR Open parenthesis character definition. */
#define	LPAR		'('
/** @define RPAR Close parenthesis character definition. */
#define	RPAR		')'
/** @define ASTERISK Character definition. */
#define	ASTERISK	'*'
/** @define PLUS Character definition. */
#define PLUS		'+'
/** @define COMMA Character definition. */
#define COMMA		','
/** @define MINUS Character definition. */
#define	MINUS		'-'
/** @define DOT Character definition. */
#define	DOT		'.'
/** @define SLASH Character definition. */
#define	SLASH		'/'
/** @define COLON Character definition. */
#define	COLON		':'
/** @define SEMICOLON Character definition. */
#define SEMICOLON	';'
/** @define LT Open angle bracket character definition. */
#define	LT		'<'
/** @define EQ Character definition. */
#define	EQ		'='
/** @define GT Close angle bracket character definition. */
#define	GT		'>'
/** @define INTERROG Character definition. */
#define	INTERROG	'?'
/** @define AT Character definition. */
#define	AT		'@'
/** @define LBRACKET Open square bracket Character definition. */
#define	LBRACKET	'['
/** @define BACKSLASH Character definition. */
#define BACKSLASH	'\\'
/** @define RBRACKET Close square bracket character definition. */
#define	RBRACKET	']'
/** @define CARET Character definition. */
#define CARET		'^'
/** @define UNDERSCORE Character definition. */
#define UNDERSCORE	'_'
/** @define BACKQUOTE Character definition. */
#define BACKQUOTE	'`'
/** @define LBRACE Open curly bracket character definition. */
#define	LBRACE		'{'
/** @define PIPE Character definition. */
#define	PIPE		'|'
/** @define RBRACE Close curly bracket character definition. */
#define	RBRACE		'}'
/** @define TILDE Character definition. */
#define TILDE		'~'

/** @define YES String definition. */
#define YES		"yes"
/** @define NO String definition. */
#define NO		"no"

/** @define KB Definition of one KiloByte value. */
#define KB		1024
/** @define MB Definition of one MegaByte value. */
#define MB		1048576
/** @define GB Definition of one GigaByte value. */
#define GB		1073741824
/** @define TB Definition of one TeraByte value. */
#define TB		1099511627776
/** @define PB Definition of one PetaByte value. */
#define PB		1125899906842624
/** @define EB Defintion of one ExaByte value. */
#define EB		1152921504606846976
/** @define ZB Definition of one ZettaByte value. */
#define ZB		1180591620717411303424
/** @define YB Definition of one YottaByte value. */
#define YB		1208925819614629174706176

/** @define KILO Definition of one Kilo value. */
#define KILO		1000
/** @define MEGA Definition of one Mega value. */
#define MEGA		1000000
/** @define GIGA Definition of one Giga value. */
#define GIGA		1000000000
/** @define TERA Definition of one Tera value. */
#define TERA		1000000000000
/** @define PETA Definition of one Peta value. */
#define PETA		1000000000000000
/** @define EXA Definition of one Exa value. */
#define EXA		1000000000000000000
/** @define ZETTA Definition of one Zetta value. */
#define ZETTA		1000000000000000000000
/** @define YOTTA Definition of one Yotta value. */
#define YOTTA		1000000000000000000000000

/** @define STR_IS_TRUE Check if a string contains 'true', 'yes', 'on' or '1'. */
#define STR_IS_TRUE(s)	(s && (!strcasecmp(s, "true") || !strcasecmp(s, "yes") || !strcasecmp(s, "on") || !strcmp(s, "1")))
/** @define STR_IS_FALSE Check is a string is NULL, empty, or contains 'false', 'no', 'off' or '0'. */
#define STR_IS_FALSE(s)	(!s || !s[0] || !strcasecmp(s, "false") || !strcasecmp(s, "no") || !strcasecmp(s, "off") || !strcmp(s, "0"))

/* ********** ANSI ********** */
/** @define YANSI_CLEAR Sequence to clear the terminal. */
#define YANSI_CLEAR	"\033[H\033[J"
/** @define YANSI_RESET End an ANSI sequence. */
#define YANSI_RESET	"\x1b[0m"
/** @define YANSI_BOLD Start a bold string. */
#define YANSI_BOLD	"\x1b[1m"
/** @define YANSI_FAINT Start a faint text. */
#define YANSI_FAINT	"\x1b[2m"
/** @define YANSI_UNDERLINE Start an underlined text. */
#define YANSI_UNDERLINE	"\x1b[4m"
/** @define YANSI_NEGATIVE Start an inversed video text. */
#define YANSI_NEGATIVE	"\x1b[7m"
/** @define YANSI_RED . */
#define YANSI_RED	"\x1b[91m"
/** @define YANSI_GREEN . */
#define YANSI_GREEN	"\x1b[92m"
/** @define YANSI_YELLOW . */
#define YANSI_YELLOW	"\x1b[93m"
/** @define YANSI_BLUE . */
#define YANSI_BLUE	"\x1b[94m"
/** @define YANSI_MAGENTA . */
#define YANSI_MAGENTA	"\x1b[95m"
/** @define YANSI_CYAN . */
#define YANSI_CYAN	"\x1b[96m"

/* ********** STRING HELPERS ********** */
/** @define strlen0	Secured version of strlen. Returns 0 when the pointer is NULL. */
#define strlen0(s)	(s ? strlen(s) : 0)
/** @define empty0	Tell if a string is empty or not. */
#define empty0(s)	(!s || !s[0])

/* ********** MATHEMATICAL HELPERS ********** */
/** @define NEXT_POW2	Round a number to the next power of 2. */
#define NEXT_POW2(s)	((size_t)pow(2, ceil(log2(s))))
/**
 * @define	COMPUTE_SIZE
 * Compute a size by rounding a given number to the next power of 2, with a minimal value.
 */
#define COMPUTE_SIZE(s, minimal)	(((s) < minimal) ? minimal : NEXT_POW2((s)))

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

