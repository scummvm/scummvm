/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2004 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
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
 * $Header$
 *
 */

#ifndef PORTDEFS_H
#define PORTDEFS_H

// Prevents error trying to call main() twice from within the program ;-).
#ifndef REAL_MAIN
	#define main scummvm_main
#endif /* REAL_MAIN */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <cctype>

//??
//#include <math.h>
//#include <time.h>

// GP32 SDK Includes
#include <gpfont.h>
#include <gpfont16.h>
#include <gpfont_port.h>
#include <gpgraphic.h>
#include <gpgraphic16.h>
#include <gpmm.h>
#include <gpmem.h>
#include <gpos_def.h>
#include <gpstdio.h>
#include <gpstdlib.h>
#include <gpdef.h>

#define size_t long unsigned int // SDK hack

// Undefine SDK defs.

#undef byte
#undef malloc
#undef calloc
#undef free

#undef stderr
#undef stdout
#undef stdin

// Redefine SDK defs.
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef int time_t;

extern int gpprintf(const char *fmt, ...);
#define printf gpprintf

extern void *gpmalloc(size_t size);
extern void *gpcalloc(size_t nitems, size_t size);
extern void gpfree(void *block);
extern char *gpstrdup(const char *s);

#define malloc gpmalloc
#define calloc gpcalloc //gm_calloc
//#define calloc gm_calloc
#define free gpfree

//	#define memset gm_memset
//	#define memcopy gm_memcopy
//
//	#define strcpy gm_strcpy	// uncomment?
//	#define strncpy gm_strncpy
//	#define strcat gm_strcat
//	#define sprintf gm_sprintf

#define strdup gpstrdup

#define assert(e) ((e) ? 0 : (printf("!AS: " #e " (%s, %d)\n", __FILE__, __LINE__)))
#define ASSERT assert

#define ENDLESSLOOP while (1)

#define FILE F_HANDLE
extern FILE *fstderr;
extern FILE *fstdout;
extern FILE *fstdin;

#define stderr fstderr
#define stdout fstdout
#define stdin fstdin

extern FILE *gpfopen(const char *filename, const char *mode);
extern int gpfclose(FILE *stream);
extern int gpfseek(FILE *stream, long offset, int whence);
extern size_t gpfread(void *ptr, size_t size, size_t n, FILE *stream);
extern size_t gpfwrite(const void *ptr, size_t size, size_t n, FILE*stream);
extern long gpftell(FILE *stream);
extern void gpclearerr(FILE *stream);
extern int gpfeof(FILE *stream);
extern char *gpfgets(char *s, int n, FILE *stream);
extern int gpfflush(FILE *stream);
extern char gpfgetc(FILE *stream);

#define fopen gpfopen
#define fclose gpfclose
#define fseek gpfseek
#define fread gpfread
#define fwrite gpfwrite
#define ftell gpftell

#undef clearerr
#define clearerr gpclearerr

#undef feof
#define feof gpfeof
#define fgets gpfgets
#define fgetc gpfgetc
#define getc gpfgetc

extern int gpfprintf(FILE *stream, const char *fmt, ...);
#define fprintf gpfprintf
#define fflush gpfflush

extern void gpexit(int code);
#define exit gpexit
//#define error printf

//extern time_t gptime(time_t *timer);
//#define time gptime

// MARK Debug Point.
#define MARK printf("MARK: %s, %s, %d", __FILE__, __FUNCTION__, __LINE__);

extern void *gpmemset (void *s, int c, size_t n);
extern void *gpmemcpy (void *dest, const void *src, size_t n);
//#define memset gpmemset
//#define memcpy gpmemcpy

// Missing stuff
int stricmp(const char *string1, const char *string2);
int strnicmp(const char *string1, const char *string2, int len);
inline float sin(float) { return 0; }
inline float cos(float) { return 0; }
inline float sqrt(float) { return 0; }
inline float atan2(float, float) { return 0; }

// EOF
#endif /* PORTDEFS_H */
