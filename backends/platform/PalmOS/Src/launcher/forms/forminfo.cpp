#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
#include "forms.h"

#include "base/version.h"
#include "globals.h"
#include "init_palmos.h"

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

static void VersionTabDraw() {
	WinDrawChars(gScummVMVersion, StrLen(gScummVMVersion), 47, 12 + 30);
	WinDrawChars(gScummVMBuildDate, StrLen(gScummVMBuildDate), 47, 24 + 30);
}

static void SystemTabDraw() {
	Coord x;
	UInt32 dm, sm, df, sf, stack;
	Char num[10];

	PalmGetMemory(&sm, &dm, &sf, 0);
	stack = GetStackSize();
	df = gVars->startupMemory;

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

		case frmCloseEvent:
			InfoFormSave();
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
