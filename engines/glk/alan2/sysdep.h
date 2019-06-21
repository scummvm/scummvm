/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GLK_ALAN2_SYSDEP
#define GLK_ALAN2_SYSDEP

/* System dependencies file for Alan Adventure Language system
 *
 * N.B.The test for symbols used here should really be of three types
 * - processor name(like PC, x86, ...)
 * - os name(DOS, WIN32, Solaris2, ...)
 * - compiler name andversion(DJGPP, CYGWIN, GCC271, THINK-C, ...)
 *
 * The set symbols should indicate if a feature is on or off like the GNU
 * AUTOCONFIG package does.
 *
 * This is not completely done yet!
 */

#include "common/scummsys.h"
#include "common/stream.h"

namespace Glk {
namespace Alan2 {

#define GLK
#define __win__

#ifdef GLK
#undef isdigit
#define isdigit Common::isDigit
#undef stricmp
#define stricmp scumm_stricmp
#undef strdup
#define strdup scumm_strdup
#undef rand
#define rand() g_vm->getRandomNumber(0x7fffffff)
#undef fprintf
extern void fprintf(Common::WriteStream *ws, const char *fmt, ...);
#endif

/* Place definitions of OS and compiler here if necessary */
#ifdef AZTEC_C
#define __amiga__
#endif

#ifndef __sun__
#ifdef sun
#define __sun__
#endif
#endif

#ifdef _INCLUDE_HPUX_SOURCE
#define __hp__
#endif

#ifndef __unix__
#ifdef unix
#define __unix__
#endif
#endif

#ifdef vax
#define __vms__
#endif

#ifdef THINK_C
#define __mac__
#endif

#ifdef __MWERKS__
#ifdef macintosh
#define __mac__
#else
#define __dos__
#endif
#endif

#ifdef DOS
#define __dos__
#endif

#ifdef __BORLANDC__
#define __dos__
#endif

#ifdef __CYGWIN__
#define __win__
#endif

#ifdef __MINGW32__
#define __win__
#endif

#ifdef __PACIFIC__
#define  __dos__
#define HAVE_SHORT_FILENAMES
#endif


/*----------------------------------------------------------------------

  Below follows OS and compiler dependent settings. They should not be
  changed except for introducing new sections when porting to new
  environments.

 */

/************/
/* Includes */
/************/

#ifndef GLK
#include <stdio.h>
#include <ctype.h>
#endif

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#endif

#ifdef __vms__
/* Our VAXC doesn't define __STDC__ */
#include <stdlib.h>
#include <string.h>
#endif


#ifdef __mac__
#include <stdlib.h>
#include <string.h>
#include <unix.h>
#endif

#ifdef __MWERKS__
#define strdup _strdup
#endif

/***********************/
/* ISO character sets? */
/***********************/

/* Common case first */
#define ISO 1
#define NATIVECHARSET 0

#ifdef GLK
#undef ISO
#define ISO 1
#undef NATIVECHARSET
#define NATIVECHARSET 0
#else	/* Glk is ISO, no matter what the OS */

#ifdef __dos__
#undef ISO
#define ISO 0
#undef NATIVECHARSET
#define NATIVECHARSET 2
#endif

#ifdef __win__
#undef ISO
#define ISO 1
#undef NATIVECHARSET
#define NATIVECHARSET 2
#endif

#ifdef __mac__
#undef ISO
#define ISO 0
#undef NATIVECHARSET
#define NATIVECHARSET 1
#endif

#endif

/**************************/
/* Strings for file modes */
/**************************/
#define READ_MODE "r"
#define WRITE_MODE "w"

#ifdef __mac__
/* File open mode (binary) */
#undef READ_MODE
#define READ_MODE "rb"
#undef WRITE_MODE
#define WRITE_MODE "wb"
#endif

#ifdef __dos__
/* File open mode (binary) */
#undef READ_MODE
#define READ_MODE "rb"
#undef WRITE_MODE
#define WRITE_MODE "wb"
#endif

#ifdef __win__
/* File open mode (binary) */
#undef READ_MODE
#define READ_MODE "rb"
#undef WRITE_MODE
#define WRITE_MODE "wb"
#endif

/*****************/
/* Byte ordering */
/*****************/

#ifdef __dos__
#define REVERSED
#endif

#ifdef __vms__
#define REVERSED
#endif

#ifdef __win__
#ifndef REVERSED
#define REVERSED
#endif
#endif


/****************************/
/* Allocates cleared bytes? */
/****************************/

#ifdef __CYGWIN__
#define NOTCALLOC
#endif

#ifdef __MINGW32__
#define NOTCALLOC
#endif

#ifdef __unix__
#define NOTCALLOC
#endif


/****************/
/* Have termio? */
/****************/

#ifdef GLK
/* don't need TERMIO */
#else

#ifdef __CYGWIN__
#define HAVE_TERMIO
#endif

#ifdef __unix__
#define HAVE_TERMIO
#endif

#endif

/*******************************/
/* Is ANSI control available? */
/*******************************/

#ifdef GLK
/* don't need ANSI */
#else

#ifdef __CYGWIN__
#define HAVE_ANSI
#endif

#endif

/******************************/
/* Use the READLINE function? */
/******************************/

#ifdef GLK
/* Glk always uses readline(), no matter what the OS */
#define USE_READLINE
#else

#ifdef __unix__
#define USE_READLINE
#endif

#ifdef x__dos__
#define USE_READLINE
#endif

#ifdef __win__
#define USE_READLINE
#endif

#endif

/* Special cases and definition overrides */
#ifdef __unix__
#define MULTI
#endif




#ifdef __vms__

#define MULTI

extern char *strdup(char str[]);

/* Cheat implementation of strftime */
extern size_t strftime (char *, size_t, const char *, const struct tm *);

#endif

#ifdef __mac__

extern char *strdup(char *str);

#endif


#ifdef __dos__

/* Return codes */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE  1

#endif


/* Native character functions */
extern int isSpace(int c);      /* IN - Native character to test */
extern int isLower(int c);      /* IN - Native character to test */
extern int isUpper(int c);      /* IN - Native character to test */
extern int isLetter(int c);     /* IN - Native character to test */
extern int toLower(int c);      /* IN - Native character to convert */
extern int toUpper(int c);      /* IN - Native character to convert */
extern char *strlow(char str[]); /* INOUT - Native string to convert */
extern char *strupp(char str[]); /* INOUT - Native string to convert */

/* ISO character functions */
extern int isISOLetter(int c);  /* IN - ISO character to test */
extern char toLowerCase(int c); /* IN - ISO character to convert */
extern char toUpperCase(int c); /* IN - ISO character to convert */
extern char *stringLower(char str[]); /* INOUT - ISO string to convert */
extern char *stringUpper(char str[]); /* INOUT - ISO string to convert */

/* ISO string conversion functions */
extern void toIso(char copy[],  /* OUT - Mapped string */
		  char original[], /* IN - string to convert */
		  int charset);	/* IN - The current character set */

extern void fromIso(char copy[], /* OUT - Mapped string */
		    char original[]); /* IN - string to convert */

extern void toNative(char copy[], /* OUT - Mapped string */
		     char original[], /* IN - string to convert */
		     int charset); /* IN - current character set */

} // End of namespace Alan2
} // End of namespace Glk

#endif
