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

#undef fprintf
extern void fprintf(Common::WriteStream *ws, const char *fmt, ...);

/***********************/

/* Common case first */
#define ISO 1
#define NATIVECHARSET 0

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
                  int charset); /* IN - The current character set */

extern void fromIso(char copy[], /* OUT - Mapped string */
                    char original[]); /* IN - string to convert */

extern void toNative(char copy[], /* OUT - Mapped string */
                     char original[], /* IN - string to convert */
                     int charset); /* IN - current character set */

} // End of namespace Alan2
} // End of namespace Glk

#endif
