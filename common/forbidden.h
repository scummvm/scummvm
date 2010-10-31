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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_FORBIDDEN_H
#define COMMON_FORBIDDEN_H


//
// Backend files may #define FORBIDDEN_SYMBOL_ALLOW_ALL if they
// have to access functions like fopen, fread etc.
// Regular code, esp. code in engines/, should never do that.
//

#ifndef FORBIDDEN_SYMBOL_ALLOW_ALL

#define FORBIDDEN_SYMBOL_REPLACEMENT	FORBIDDEN SYMBOL!


/*
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_printf
#undef printf
#define printf	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#undef fprintf
#define fprintf	FORBIDDEN_SYMBOL_REPLACEMENT
#endif
*/

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_FILE
#undef FILE
#define FILE	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fopen
#undef fopen
#define fopen(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fclose
#undef fclose
#define fclose(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fread
#undef fread
#define fread(a,b,c,d)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#undef fwrite
#define fwrite(a,b,c,d)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fseek
#undef fseek
#define fseek(a,b,c)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_ftell
#undef ftell
#define ftell(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_feof
#undef feof
#define feof(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fgetc
#undef fgetc
#define fgetc(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fputc
#undef fputc
#define fputc(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif


#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#undef setjmp
#define setjmp(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#undef longjmp
#define longjmp(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_system
#undef system
#define system(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif


/*
time_t

time

difftime

mktime

localtime

clock

gmtime

system

remove

setlocale

setvbuf
*/

#endif


#endif
