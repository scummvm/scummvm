/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef STDLIB_H
#define STDLIB_H

#include "palmversion.h"

#ifdef PALMOS_68K
#include "MemGlue.h"
#endif

#if defined(COMPILE_ZODIAC) && defined(PALMOS_ARM)
#define MemGluePtrNew	MemPtrNew
#endif

extern ErrJumpBuf stdlib_errJumpBuf;

#define DO_EXIT( code ) \
	if (ErrSetJump(stdlib_errJumpBuf) == 0) { code }
	
#define atoi				StrAToI
#define atol				StrAToI
#define abs(a)				((a) < 0 ? -(a) : (a))
#define	malloc(a)			MemGluePtrNew(a)

#ifdef PALMOS_68K
#define qsort(a,b,c,d)		SysQSort((a), (b), (c), (CmpFuncPtr)(&d), 0);
#else
typedef  int 	(*_compare_function)(const void*, const void*);
extern "C" void qsort(void * table_base, size_t num_members, size_t member_size, _compare_function compare_members);
#endif

#define rand()				SysRandom(0)
#define abort()
#define strtoul(a,b,c)		((unsigned long)strtol(a,b,c))

void	*bsearch	(const void *key, const void *base, UInt32 nmemb, UInt32 size, int (*compar)(const void *, const void *));
MemPtr	 calloc		(UInt32 nelem, UInt32 elsize);
void	 exit		(Int16 status);
Err		 free		(MemPtr memP);
MemPtr	 realloc	(MemPtr oldP, UInt32 size);
long	 strtol		(const char *s, char **endptr, int base);

#endif
