/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
 * Copyright (C) 2002 ph0x (GP32 port)
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 */


//#define __size_t  // SDK hack?
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	#include <ctype.h>


extern "C" {
	#include "gpfont.h"
	#include "gpfont_port.h"
	#include "gpgraphic.h"
	#include "gpmm.h"	
	#include "gpmem.h"	
	#include "gpos_def.h"
	#include "gpstdio.h"
	#include "gpstdlib.h"	
	#include "gpdef.h"
	//#include "defines.h"
}

#undef byte // SDK hack?

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32; 

	extern int gpprintf(const char *fmt, ...);
	#define printf gpprintf

	extern void *gpmalloc(size_t size);
	extern void *gpcalloc(size_t nitems, size_t size);
	extern void gpfree(void *block);
	#define malloc gpmalloc 
	#define calloc gpcalloc //gm_calloc
	#define free gpfree	
	/*#define memset gm_memset
	#define memcopy gm_memcopy

	#define strcpy gm_strcpy	// uncomment?
	#define strncpy gm_strncpy
	#define strcat gm_strcat
	#define sprintf gm_sprintf*/

	#define assert(e) ((e) ? 0 : (printf("!AS: " #e " (%s, %d)\n", __FILE__, __LINE__)))
	#define ASSERT assert

	#define ENDLESSLOOP while (1)

	#define FILE F_HANDLE
	#define stderr NULL	// hack...
	#define stdout stderr
	#define stdin stderr

	extern FILE *gpfopen(const char *filename, const char *mode);
	extern int gpfclose(FILE *stream);
	extern int gpfseek(FILE *stream, long offset, int whence);
	extern  size_t gpfread(void *ptr, size_t size, size_t n, FILE *stream);
	extern size_t gpfwrite(const void *ptr, size_t size, size_t n, FILE*stream);
	extern long gpftell(FILE *stream);
	extern void gpclearerr(FILE *stream);
	extern int gpfeof(FILE *stream);
	extern char *gpfgets(char *s, int n, FILE *stream);
	extern int gpfflush(FILE *stream);

	#define fopen gpfopen
	#define fclose gpfclose
	#define fseek gpfseek
	#define fread gpfread
	#define fwrite gpfwrite
	#define ftell gpftell
	#define clearerr gpclearerr
	#define feof gpfeof
	#define fgets gpfgets	
	
	extern int gpfprintf(FILE *stream, const char *fmt, ...);
	#define fprintf gpfprintf
	#define fflush gpfflush

	extern void gphalt(int code=0);
	#define exit gphalt
	//#define error printf

	#define time(x) (0) // fixme! (SIMON)

	// EOF
