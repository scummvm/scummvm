#include <PalmOS.h>

#include "start.h"
#include "forms.h"

/***********************************************************************
 *
 * FUNCTION:    SystemInfoFormInit
 * FUNCTION:     SystemInfoFormHandleEvent
 *
 * DESCRIPTION: 
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void GetMemory(UInt32* storageMemoryP, UInt32* dynamicMemoryP, UInt32 *storageFreeP, UInt32 *dynamicFreeP) {
	UInt32		free, max;

	Int16		i;
	Int16		nCards;
	UInt16		cardNo;
	UInt16		heapID;

	UInt32		storageMemory = 0;
	UInt32		dynamicMemory = 0;
	UInt32		storageFree = 0;
	UInt32		dynamicFree = 0;

	// Iterate through each card to support devices with multiple cards.
	nCards = MemNumCards();		

	for (cardNo = 0; cardNo < nCards; cardNo++) {
		// Iterate through the RAM heaps on a card (excludes ROM).
		for (i=0; i< MemNumRAMHeaps(cardNo); i++) {
			// Obtain the ID of the heap.
			heapID = MemHeapID(cardNo, i);
			// Calculate the total memory and free memory of the heap.
			MemHeapFreeBytes(heapID, &free, &max);
			
			// If the heap is dynamic, increment the dynamic memory total.
			if (MemHeapDynamic(heapID)) {
				dynamicMemory += MemHeapSize(heapID);
				dynamicFree += free;

			// The heap is nondynamic (storage ?).
			} else {
				storageMemory += MemHeapSize(heapID);
				storageFree += free;
			}
		}
	}
	// Reduce the stats to KB.  Round the results.
	dynamicMemory = dynamicMemory / 1024L;
	storageMemory = storageMemory / 1024L;

	dynamicFree = dynamicFree / 1024L;
	storageFree = storageFree / 1024L;

	if (dynamicMemoryP) *dynamicMemoryP = dynamicMemory;
	if (storageMemoryP) *storageMemoryP = storageMemory;
	if (dynamicFreeP) *dynamicFreeP = dynamicFree;
	if (storageFreeP) *storageFreeP = storageFree;
}

static void SystemInfoFormInit() {
	FormPtr frmP;
	Coord x;
	UInt32 dm, sm, df, sf;
	Char num[10];

	GetMemory(&sm, &dm, &sf, &df);
	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);

	WinSetTextColor(255);
	FntSetFont(stdFont);
	
	StrIToA(num, dm);
	x = 149 - FntCharsWidth(num, StrLen(num));
	WinDrawChars(num, StrLen(num), x, 30);

	StrIToA(num, sm);
	x = 149 - FntCharsWidth(num, StrLen(num));
	WinDrawChars(num, StrLen(num), x, 42);

	StrIToA(num, df);
	x = 109 - FntCharsWidth(num, StrLen(num));
	WinDrawChars(num, StrLen(num), x, 30);

	StrIToA(num, sf);
	x = 109 - FntCharsWidth(num, StrLen(num));
	WinDrawChars(num, StrLen(num), x, 42);
}

Boolean SystemInfoFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			SystemInfoFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			// OK button only
			FrmReturnToMain();
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
