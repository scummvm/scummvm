/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "stdlib.h"
#include "MemGlue.h"
///////////////////////////////////////////////////////////////////////////////
/*void qsort(void *base, UInt32 nmemb, UInt32 size, ComparF *compar) {
	
	SysQSort(base, nmemb, size, compar);
}*/
///////////////////////////////////////////////////////////////////////////////
void *bsearch(const void *key, const void *base, UInt32 nmemb, 
				UInt32 size, int (*compar)(const void *, const void *)) {
	UInt32 i;

	for (i=0; i<nmemb; i++) 
		if (compar(key, (void*)((UInt32)base + size * i)) == 0)
			return (void*)((UInt32)base + size * i);
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
static DmOpenRef gExtMemory = NULL;

static UInt16 MemFindHeapID(UInt32 size)
{
	UInt32  nFree, maxChunk ;
	UInt16 maxCards = 1;	//MemNumCards(); process only first card for now
	UInt16 heapID = -1;		// no heap avaliable

	UInt16 cardNo;
	UInt16 maxHeaps, heapIndex;

	for (cardNo = 0; cardNo < maxCards; cardNo++)
	{
		if (MemNumRAMHeaps(cardNo) > 0)
		{
			maxHeaps =  MemNumHeaps(cardNo);
			for (heapIndex = 0; heapIndex < maxHeaps; heapIndex++)
			{
				// Obtain the ID of the heap.
				heapID = MemHeapID(cardNo, heapIndex);
				
				if (!(MemHeapFlags(heapID) & memHeapFlagReadOnly))
				{
					MemHeapFreeBytes( heapID, &nFree, &maxChunk );
					if (maxChunk > size)
						return heapID;
				}
			}
		}
	}

	return heapID;
}

void MemExtInit()
{
	if (!gExtMemory)
	{
		LocalID localID = DmFindDatabase(0, "ScummVM-Memory");
		if (localID) DmDeleteDatabase(0, localID);
		
		if (DmCreateDatabase (0, "ScummVM-Memory", 'ScVM', 'DATA', false) != errNone)
			return;

		localID = DmFindDatabase(0, "ScummVM-Memory");
		gExtMemory = DmOpenDatabase(0, localID, dmModeReadWrite|dmModeExclusive);
	}
}

void MemExtCleanup()
{
	if (gExtMemory) {
		DmCloseDatabase(gExtMemory);
		LocalID localID = DmFindDatabase(0, "ScummVM-Memory");
		if (localID)
			DmDeleteDatabase(0, localID);
	}
}
//#define USE_EXTENDEDMEM
#ifdef USE_EXTENDEDMEM

MemPtr calloc(UInt32 nelem, UInt32 elsize)
{
	UInt32 size = nelem*elsize;
	MemPtr newP = NULL;
	UInt16 heapID = MemFindHeapID(size);

	if (heapID != NO_HEAP_FOUND)
	{
		if (MemHeapDynamic(heapID) && size < 65536-8) // 8 = chunk header size
			newP = MemPtrNew(size);
		else
		{
			SysAppInfoPtr appInfoP;
			UInt16 ownerID, large, nmovable;
			UInt16 attr;

			ownerID = ((SysAppInfoPtr)SysGetAppInfo(&appInfoP, &appInfoP))->memOwnerID; 
			large	= ((size > 65536-8) ? memNewChunkFlagAllowLarge : 0);
			nmovable= (MemHeapDynamic(heapID) ? memNewChunkFlagNonMovable : memNewChunkFlagPreLock);
			attr	=	ownerID|large|nmovable;
			
			//MEMORY_RESERVE_ACCESS
			newP = MemChunkNew(heapID, size, attr);
			//MEMORY_RELEASE_ACCESS

			if (newP && MemPtrDataStorage(newP)) {		// if storage heap ?
				if (!gExtMemory) {						// if memory DB doesn't exist
					MemChunkFree(newP);
					return NULL;
				}

				UInt16 index = dmMaxRecordIndex;	// used for record purpose
				MemHandle newH = MemPtrRecoverHandle(newP);					// exists
				if (DmAttachRecord(gExtMemory, &index, newH, NULL) != errNone) // attach to DB
				{
					MemChunkFree(newP);	// error
					return NULL;
				}
			}
		}
	}

	if (newP)
		MemSet(newP,size,0);

	return newP;
}

#else

MemPtr calloc(UInt32 nelem, UInt32 elsize)
{
	UInt32 size = nelem*elsize;
	MemPtr newP = NULL;

/*	if (size < 65536-8) 		// 8 = chunk header size
		newP = MemPtrNew(size);
	else*/
/*	{
		SysAppInfoPtr appInfoP;
		UInt16 ownerID;
		UInt16 attr;

		ownerID = ((SysAppInfoPtr)SysGetAppInfo(&appInfoP, &appInfoP))->memOwnerID; 
		attr	=	ownerID|memNewChunkFlagAllowLarge|memNewChunkFlagNonMovable;

		newP = MemChunkNew(0, size, attr);
	}
*/
	newP = MemGluePtrNew(size);

	if (newP)
		MemSet(newP,size,0);

	return newP;
}

#endif
///////////////////////////////////////////////////////////////////////////////
#ifdef USE_EXTENDEDMEM
Err free(MemPtr memP)
{
	Err err = memErrInvalidParam;

	if (!memP)
		return err;

	if (MemPtrDataStorage(memP)) {		// if storage heap ?
		if (gExtMemory) {				// if memory DB exists
			DmOpenRef where;
			MemHandle newH = MemPtrRecoverHandle(memP);
			UInt16 index = DmSearchRecord(newH, &where);
			err = DmRemoveRecord(gExtMemory, index);
		}
	}
	else
		err = MemChunkFree(memP);

	return err;
}
#else
Err free(MemPtr memP)
{
	if (memP)
		return MemPtrFree(memP);
	
	return memErrInvalidParam;
}
#endif
///////////////////////////////////////////////////////////////////////////////
MemPtr realloc(MemPtr oldP, UInt32 size)
{
	
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
///////////////////////////////////////////////////////////////////////////////
void exit(Int16 status)
{
	// need to change this
	EventType event;
	event.eType = keyDownEvent;
	event.data.keyDown.chr = vchrLaunch;
	event.data.keyDown.modifiers = commandKeyMask;
	EvtAddEventToQueue (&event);
}