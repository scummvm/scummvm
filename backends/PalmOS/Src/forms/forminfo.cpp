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

static void VersionTabPreInit(FormType *frmP) {
	FrmNewLabel(&frmP, 4210, gScummVMVersion, 49, 12, stdFont);
	FrmNewLabel(&frmP, 4211, gScummVMBuildDate, 49, 24, stdFont);
}

static void SystemTabPreInit(FormType *frmP) {
	Coord x;
	UInt32 dm, sm, df, sf, stack;
	Char num[10];

	GetMemory(&sm, &dm, &sf, &df);
	stack = GetStackSize();
	
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));
	FntSetFont(stdFont);
	
	StrIToA(num, dm);
	x = 149 - FntCharsWidth(num, StrLen(num)) + 5;
	FrmNewLabel(&frmP, 4308, num, x, 12, stdFont);

	StrIToA(num, sm);
	x = 149 - FntCharsWidth(num, StrLen(num)) + 5;
	FrmNewLabel(&frmP, 4309, num, x, 24, stdFont);

	StrIToA(num, stack);
	x = 149 - FntCharsWidth(num, StrLen(num)) + 5;
	FrmNewLabel(&frmP, 4310, num, x, 36, stdFont);

	StrIToA(num, df);
	x = 109 - FntCharsWidth(num, StrLen(num)) + 5;
	FrmNewLabel(&frmP, 4311, num, x, 12, stdFont);

	StrIToA(num, sf);
	x = 109 - FntCharsWidth(num, StrLen(num)) + 5;
	FrmNewLabel(&frmP, 4312, num, x, 24, stdFont);

	StrCopy(num,"-");
	x = 109 - FntCharsWidth(num, StrLen(num)) + 5;
	FrmNewLabel(&frmP, 4313, num, x, 36, stdFont);
}

static void InfoFormSave() {
	TabDeleteTabs(myTabP);
	FrmReturnToMain();
}

static void AboutTabInit(Boolean draw) {
	if (!draw) {
		RectangleType r = {2, 40, 154, 45};
		WinSetBackColor(UIColorGetTableEntryIndex(UIFormFill));
		WinEraseRectangle(&r, 0);
		
	} else {
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
}

static void InfoFormInit() {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();

	tabP = TabNewTabs(3);
	TabAddContent(&frmP, tabP, "About", TabInfoAboutForm);
	TabAddContent(&frmP, tabP, "Version", TabInfoVersionForm, VersionTabPreInit);
	TabAddContent(&frmP, tabP, "Memory", TabInfoSystemForm, SystemTabPreInit);

	FrmDrawForm(frmP);
	TabSetActive(frmP, tabP, 0);

	AboutTabInit(true);

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
					if (lastTab == 0) AboutTabInit(false);

					lastTab = (eventP->data.ctlSelect.controlID - InfoForm - 1);
					TabSetActive(frmP, myTabP, lastTab);
				
					if (lastTab == 0) AboutTabInit(true);
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
