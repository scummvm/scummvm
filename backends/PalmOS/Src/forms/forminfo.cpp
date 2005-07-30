#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
#include "forms.h"

#include "version.h"

/***********************************************************************
 *
 * FUNCTION:    MiscOptionsFormSave
 * FUNCTION:    MiscOptionsFormInit
 * FUNCTION:    MiscOptionsFormHandleEvent
 *
 * DESCRIPTION: Misc. Options form functions
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static TabType *myTabP;
static UInt16 lastTab = 0;

static UInt32 GetStackSize() {
	MemPtr startPP, endPP;
	SysGetStackInfo(&startPP, &endPP);

	return ((Char *)endPP - (Char *)startPP) / 1024L;
}

void GetMemory(UInt32* storageMemoryP, UInt32* dynamicMemoryP, UInt32 *storageFreeP, UInt32 *dynamicFreeP) {
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

static void VersionTabDraw() {
	WinDrawChars(gScummVMVersion, StrLen(gScummVMVersion), 47, 12 + 30);
	WinDrawChars(gScummVMBuildDate, StrLen(gScummVMBuildDate), 47, 24 + 30);
}

static void SystemTabDraw() {
	Coord x;
	UInt32 dm, sm, df, sf, stack;
	Char num[10];

	GetMemory(&sm, &dm, &sf, &df);
	stack = GetStackSize();

	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));
	FntSetFont(stdFont);

	StrIToA(num, dm);
	x = 147 - FntCharsWidth(num, StrLen(num)) + 5;
	WinDrawChars(num, StrLen(num), x, 12 + 30);

	StrIToA(num, sm);
	x = 147 - FntCharsWidth(num, StrLen(num)) + 5;
	WinDrawChars(num, StrLen(num), x, 24 + 30);

	StrIToA(num, stack);
	x = 147 - FntCharsWidth(num, StrLen(num)) + 5;
	WinDrawChars(num, StrLen(num), x, 36 + 30);

	StrIToA(num, df);
	x = 107 - FntCharsWidth(num, StrLen(num)) + 5;
	WinDrawChars(num, StrLen(num), x, 12 + 30);

	StrIToA(num, sf);
	x = 107 - FntCharsWidth(num, StrLen(num)) + 5;
	WinDrawChars(num, StrLen(num), x, 24 + 30);

	StrCopy(num,"-");
	x = 107 - FntCharsWidth(num, StrLen(num)) + 5;
	WinDrawChars(num, StrLen(num), x, 36 + 30);
}

static void InfoFormSave() {
	TabDeleteTabs(myTabP);
	FrmReturnToMain();
}

static void AboutTabDraw() {
	MemHandle hTemp;
	BitmapPtr bmpTemp;

	hTemp = DmGetResource (bitmapRsc, 1200);
	if (hTemp) {
		bmpTemp = (BitmapType *)MemHandleLock(hTemp);
		WinDrawBitmap(bmpTemp,3,44);
		MemPtrUnlock(bmpTemp);
		DmReleaseResource(hTemp);
	}
}

static void InfoFormInit() {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();

	tabP = TabNewTabs(3);
	TabAddContent(&frmP, tabP, "About", TabInfoAboutForm, AboutTabDraw);
	TabAddContent(&frmP, tabP, "Version", TabInfoVersionForm, VersionTabDraw);
	TabAddContent(&frmP, tabP, "Memory", TabInfoSystemForm, SystemTabDraw);

	lastTab = 0;
	FrmDrawForm(frmP);
	TabSetActive(frmP, tabP, lastTab);

	myTabP = tabP;
}

Boolean InfoFormHandleEvent(EventPtr eventP) {
	FormPtr frmP = FrmGetActiveForm();
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			InfoFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case (InfoForm + 1) :
				case (InfoForm + 2) :
				case (InfoForm + 3) :
					lastTab = (eventP->data.ctlSelect.controlID - InfoForm - 1);
					TabSetActive(frmP, myTabP, lastTab);
					break;

				case InfoOKButton:
					InfoFormSave();
					break;
			}
			handled = true;
			break;

		default:
			break;
	}

	return handled;
}
