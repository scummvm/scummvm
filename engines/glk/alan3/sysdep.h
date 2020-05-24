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

#ifndef GLK_ALAN3_SYSDEP
#define GLK_ALAN3_SYSDEP

/* System dependencies file for Alan Adventure Language system

  N.B. The test for symbols used here should really be of three types
  - processor name (like PC, x86, ...)
  - os name (DOS, WIN32, Solaris2, ...)
  - compiler name and version (DJGPP, CYGWIN, GCC271, THINK-C, ...)

  The set symbols should indicate if a feature is on or off like the GNU
  AUTOCONFIG package does.

  This is not completely done yet!
*/

#include "common/util.h"

namespace Glk {
namespace Alan3 {

#define GLK
#define HAVE_GLK

/*----------------------------------------------------------------------

  Below follows OS and compiler dependent settings. They should not be
  changed except for introducing new sections when porting to new
  environments.

 */

/***********************/
/* ISO character sets? */
/***********************/

/* Common case first */
#define ISO 1
#define NATIVECHARSET 0

#undef ISO
#define ISO 1
#undef NATIVECHARSET
#define NATIVECHARSET 0

/**************************/
/* Strings for file modes */
/**************************/
#define READ_MODE "rb"
#define WRITE_MODE "wb"


/****************/

/* don't need TERMIO */
#undef HAVE_TERMIO
/* don't need ANSI */
#undef HAVE_ANSI

#define USE_READLINE


/* Native character functions */
extern int isSpace(unsigned int c);      /* IN - Native character to test */
extern int isLower(unsigned int c);      /* IN - Native character to test */
extern int isUpper(unsigned int c);      /* IN - Native character to test */
extern int isLetter(unsigned int c);     /* IN - Native character to test */
extern int toLower(unsigned int c);      /* IN - Native character to convert */
extern int toUpper(unsigned int c);      /* IN - Native character to convert */
extern char *strlow(char str[]); /* INOUT - Native string to convert */
extern char *strupp(char str[]); /* INOUT - Native string to convert */

/* ISO character functions */
extern int isISOLetter(int c);  /* IN - ISO character to test */
extern char IsoToLowerCase(int c); /* IN - ISO character to convert */
extern char IsoToUpperCase(int c); /* IN - ISO character to convert */
extern char *stringLower(char str[]); /* INOUT - ISO string to convert */
extern char *stringUpper(char str[]); /* INOUT - ISO string to convert */
extern int compareStrings(char str1[], char str2[]); /* Case-insensitive compare */

/* ISO string conversion functions */
extern void toIso(char copy[],  /* OUT - Mapped string */
                  char original[], /* IN - string to convert */
                  int charset); /* IN - The current character set */

extern void fromIso(char copy[], /* OUT - Mapped string */
                    char original[]); /* IN - string to convert */

extern void toNative(char copy[], /* OUT - Mapped string */
                     char original[], /* IN - string to convert */
                     int charset); /* IN - current character set */

extern char *baseNameStart(char *fullPathName);

} // End of namespace Alan3
} // End of namespace Glk

#endif
