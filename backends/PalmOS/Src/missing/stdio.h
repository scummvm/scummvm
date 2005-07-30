/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 *
 * $Header$
 *
 */

#ifndef __STDIO_H__
#define __STDIO_H__

#include <PalmOS.h>
#include <VFSMgr.h>
#include <stdarg.h>

typedef void (*LedProc)(Boolean show);

extern FileRef	gStdioOutput;

typedef FileRef FILE;
typedef UInt32 size_t;

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

UInt16		fclose	(FileRef *stream);
UInt16		feof	(FileRef *stream);
Char *		fgets	(Char *s, UInt32 n, FileRef *stream);
Int16		fgetc	(FileRef *stream);
FileRef *	fopen	(const Char *filename, const Char *type);
UInt32		fread	(void *ptr, UInt32 size, UInt32 nitems, FileRef *stream);
UInt32		fwrite	(const void *ptr, UInt32 size, UInt32 nitems, FileRef *stream);
Int32		fseek	(FileRef *stream, Int32 offset, Int32 whence);
UInt32		ftell	(FileRef *stream);

Int32	fprintf	(FileRef *stream, const Char *formatStr, ...);
Int32	printf	(const Char* formatStr, ...);
Int32	sprintf	(Char* s, const Char* formatStr, ...);
Int32	snprintf(Char* s, UInt32 len, const Char* formatStr, ...);
Int32	vsprintf(Char* s, const Char* formatStr, _Palm_va_list argParam);

void	StdioInit	(UInt16 volRefNum, const Char *output, LedProc ledProc);
void	StdioRelease();

#endif
