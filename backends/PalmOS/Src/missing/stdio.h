/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#ifndef __STDIO_H__
#define __STDIO_H__

#include "palmversion.h"
#include <stdarg.h>

typedef void (*LedProc)(Boolean show);

typedef struct {
	FileRef fileRef;
	UInt32 cacheSize, bufSize, bufPos;
	UInt8 *cache;
	UInt16 mode, err;
} FILE;

extern FILE	gStdioOutput;
typedef UInt32 size_t;

#ifdef stdin
#undef stdin
#undef stdout
#undef stderr
#endif

#ifdef SEEK_SET
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#endif

#define stdin		0
#define stdout		(&gStdioOutput)
#define stderr		(&gStdioOutput)

#define clearerr(a)
#define fflush(a)
#define vsnprintf(a,b,c,d)	vsprintf(a,c,d)
#define getc(a)				fgetc(a)

#define	SEEK_SET			vfsOriginBeginning 
#define	SEEK_CUR			vfsOriginCurrent  
#define	SEEK_END			vfsOriginEnd


UInt16		fclose	(FILE *stream);
UInt16		feof	(FILE *stream);
UInt16		ferror	(FILE *stream);
Char *		fgets	(Char *s, UInt32 n, FILE *stream);
Int16		fgetc	(FILE *stream);
FILE *		fopen	(const Char *filename, const Char *type);
UInt32		fread	(void *ptr, UInt32 size, UInt32 nitems, FILE *stream);
UInt32		fwrite	(const void *ptr, UInt32 size, UInt32 nitems, FILE *stream);
Int16		fseek	(FILE *stream, Int32 offset, Int32 whence);
Int32		ftell	(FILE *stream);

Int32	fprintf	(FILE *stream, const Char *formatStr, ...);
Int32	printf	(const Char* formatStr, ...);
Int32	sprintf	(Char* s, const Char* formatStr, ...);
Int32	snprintf(Char* s, UInt32 len, const Char* formatStr, ...);
Int32	vsprintf(Char* s, const Char* formatStr, _Palm_va_list argParam);

void	StdioInit			(UInt16 volRefNum, const Char *output);
void	StdioSetLedProc		(LedProc ledProc);
void	StdioSetCacheSize	(UInt32 s);
void	StdioRelease		();

#endif
