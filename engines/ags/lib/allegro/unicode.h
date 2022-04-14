/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_LIB_ALLEGRO_UNICODE_H
#define AGS_LIB_ALLEGRO_UNICODE_H

#include "ags/lib/allegro/base.h"

namespace AGS3 {

#define U_ASCII         AL_ID('A','S','C','8')
#define U_ASCII_CP      AL_ID('A','S','C','P')
#define U_UNICODE       AL_ID('U','N','I','C')
#define U_UTF8          AL_ID('U','T','F','8')
#define U_CURRENT       AL_ID('c','u','r','.')


/* UTF-8 support functions
 */
extern int utf8_getc(const char *s);
extern int utf8_getx(char **s);
extern int utf8_setc(char *s, int c);
extern int utf8_width(const char *s);
extern int utf8_cwidth(int c);
extern int utf8_isok(int c);

/**
 * ASCII support functions
 */
extern int ascii_getc(const char *s);
extern int ascii_getx(char **s);
extern int ascii_setc(char *s, int c);
extern int ascii_width(const char *s);
extern int ascii_cwidth(int c);
extern int ascii_isok(int c);

/* ugetc: */
extern int (*ugetc)(const char *s);
/* ugetxc: */
extern int (*ugetx)(char **s);
/* ugetxc: */
extern int (*ugetxc)(const char * const *s);
/* usetc: */
extern int (*usetc)(char *s, int c);
/* uwidth: */
extern int (*uwidth)(const char *s);
/* ucwidth: */
extern int (*ucwidth)(int c);
/* uisok: */
extern int (*uisok)(int c);

/* set_uformat:
 *  Selects a new text encoding format.
 */
extern void set_uformat(int type);

enum { LC_CTYPE };
extern void setlocale(int type, const char *language);

/* get_uformat:
 *  Returns the current text encoding format.
 */
extern int get_uformat();
extern size_t ustrsize(const char *s);
/* &nicode string length
 */
extern int ustrlen(const char *s);
/* utolower:
 *  Unicode-aware version of the ANSI tolower() function.
 */
extern int utolower(int c);
/* utoupper:
 *  Unicode-aware version of the ANSI toupper() function.
 */
extern int utoupper(int c);
/* Unicode string compare
 */
extern int ustrcmp(const char *s1, const char *s2);
 /* ustricmp:
  *  Unicode-aware version of the DJGPP stricmp() function.
  */
extern int ustricmp(const char *s1, const char *s2);
/* ustrncmp:
 *  Unicode-aware version of the ANSI strncmp() function.
 */
extern int ustrncmp(const char *s1, const char *s2, int n);
/* ustrnicmp:
 *  Unicode-aware version of the DJGPP strnicmp() function.
 */
extern int ustrnicmp(const char *s1, const char *s2, int n);
/* uoffset:
 *  Returns the offset in bytes from the start of the string to the
 *  character at the specified index. If the index is negative, counts
 *  backward from the end of the string (-1 returns an offset to the
 *  last character).
 */
extern int uoffset(const char *s, int index);

/* ugetat:
 *  Returns the character from the specified index within the string.
 */
extern int ugetat(const char *s, int idx);

/* ustrlwr:
 *  Unicode-aware version of the ANSI strlwr() function.
 */
extern char *ustrlwr(char *s);
/* ustrupr:
 *  Unicode-aware version of the ANSI strupr() function.
 */
extern char *ustrupr(char *s);
/* ustrstr:
 *  Unicode-aware version of the ANSI strstr() function.
 */
extern const char *ustrstr(const char *s1, const char *s2);
/* usetat:
 *  Modifies the character at the specified index within the string,
 *  handling adjustments for variable width data. Returns how far the
 *  rest of the string was moved.
 */
int usetat(char *s, int index, int c);
/* ustrsizez:
 *  Returns the size of the specified string in bytes, including the
 *  trailing zero.
 */
extern int ustrsizez(const char *s);

extern int need_uconvert(const char *s, int type, int newtype);
extern int uvszprintf(char *buf, int size, const char *format, va_list args);

} // namespace AGS3

#endif
