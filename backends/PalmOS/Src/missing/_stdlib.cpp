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

#include <stdlib.h>

#define memNewChunkFlagAllowLarge	0x1000 
extern "C" SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP) SYS_TRAP(sysTrapSysGetAppInfo);

void *bsearch(const void *key, const void *base, UInt32 nmemb, UInt32 size, int (*compar)(const void *, const void *)) {
#ifdef PALMOS_68K
	Int32 position;
	if (SysBinarySearch(base, nmemb, size, (SearchFuncPtr)compar, key, 0, &position, true))
		return (void *)((UInt32)base + size * position);
#else
	for (int i = 0; i < nmemb; i++) 
		if (compar(key, (void*)((UInt32)base + size * i)) == 0)
			return (void*)((UInt32)base + size * i);
#endif

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

MemPtr __malloc(UInt32 size) {
	MemPtr newP = NULL;

	if (size <= 65000) {
		newP = MemPtrNew(size);
	} else {
		SysAppInfoPtr appInfoP;
		UInt16 ownerID;
		UInt16 attr;

		ownerID = ((SysAppInfoPtr)SysGetAppInfo(&appInfoP, &appInfoP))->memOwnerID; 
		attr	= ownerID|memNewChunkFlagAllowLarge|memNewChunkFlagNonMovable;

		newP = MemChunkNew(0, size, attr);
	}

	return newP;
}

MemPtr calloc(UInt32 nelem, UInt32 elsize) {
	MemPtr newP;
	UInt32 size = (nelem * elsize);

	newP = malloc(size);	// was MemGluePtrNew

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
		if (MemPtrResize(oldP, size) == 0)
			return oldP;

	MemPtr	newP = malloc(size);	// was MemPtrNew

	if (oldP!=NULL) {
		MemMove(newP,oldP,MemPtrSize(oldP));
		MemPtrFree(oldP);
	}

	return newP;
}

ErrJumpBuf stdlib_errJumpBuf;
#define ERR_MAGIC	0xDADA

void exit(Int16 status) {
#if (defined(PALMOS_ARM) && defined(COMPILE_ZODIAC))
	SysEventType event;
	event.eType = sysEventKeyDownEvent;
#else
	EventType event;
	event.eType = keyDownEvent;
#endif
	event.data.keyDown.chr = vchrLaunch;
	event.data.keyDown.modifiers = commandKeyMask;
	EvtAddUniqueEventToQueue(&event, 0, true);

	ErrLongJump(stdlib_errJumpBuf, status == 0 ? 0xDADA : status);
}
