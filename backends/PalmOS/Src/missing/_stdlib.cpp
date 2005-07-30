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

#include <stdlib.h>

void *bsearch(const void *key, const void *base, UInt32 nmemb, UInt32 size, int (*compar)(const void *, const void *)) {
	Int32 position;

	if (SysBinarySearch(base, nmemb, size, (SearchFuncPtr)compar, key, 0, &position, true))
		return (void *)((UInt32)base + size * position);

	return NULL;
}

long strtol(const char *s, char **endptr, int base) {
	// WARNING : only base = 10 supported
	long val = StrAToI(s);

	if (endptr) {
		Char str[maxStrIToALen];
		StrIToA(str, val);

		if (StrNCompare(s, str, StrLen(str)) == 0)
			*endptr = (char *)s + StrLen(str);
	}

	return val;
}

MemPtr calloc(UInt32 nelem, UInt32 elsize) {
	MemPtr newP;
	UInt32 size = nelem*elsize;

	newP = MemGluePtrNew(size);

	if (newP)
		MemSet(newP,size,0);

	return newP;
}

Err free(MemPtr memP) {
	if (memP)
		return MemPtrFree(memP);
	return memErrInvalidParam;
}

MemPtr realloc(MemPtr oldP, UInt32 size) {

	if (oldP != NULL)
		if (MemPtrResize(oldP,size) == 0)
			return oldP;

	MemPtr	newP = MemPtrNew(size);

	if (oldP!=NULL)
	{
		MemMove(newP,oldP,MemPtrSize(oldP));
		MemPtrFree(oldP);
	}
	return newP;
}

ErrJumpBuf stdlib_errJumpBuf;
#define ERR_MAGIC	0xDADA

void exit(Int16 status) {
	EventType event;
	event.eType = keyDownEvent;
	event.data.keyDown.chr = vchrLaunch;
	event.data.keyDown.modifiers = commandKeyMask;
	EvtAddUniqueEventToQueue(&event, 0, true);

	ErrLongJump(stdlib_errJumpBuf, status == 0 ? 0xDADA : status);
}
