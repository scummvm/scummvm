#include <PalmOS.h>

#include "start.h"
#include "forms.h"

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
static Boolean stackChanged = false;
static UInt8 tabNum;

#define TAB_START	MiscOptionsTabPalmPushButton
#define TAB_COUNT	2


static UInt32 StackSize(UInt32 newSize) {
	MemHandle pref = DmGetResource('pref',0);
	UInt32 size = 0;
	
	if (pref) {
		SysAppPrefsType *data = (SysAppPrefsType *)MemHandleLock(pref);
		size = data->stackSize;

		if (newSize) {
			SysAppPrefsType newData;
			MemMove(&newData, data, sizeof(SysAppPrefsType));
			newData.stackSize = newSize;
			DmWrite(data, 0, &newData, sizeof(SysAppPrefsType));
		}

		MemPtrUnlock(data);
		DmReleaseResource(pref);
	}

	return size;
}

static void MiscOptionsFormSave() {

	FieldType *fld1P;
	ControlType *cck1P, *cck2P, *cck3P, *cck4P, *cck5P, *cck6P, *cck7P, *cck8P, *cck9P, *cck10P;	
	FormPtr frmP;

	fld1P = (FieldType *)GetObjectPtr(MiscOptionsDebugLevelField);
	
	cck1P = (ControlType *)GetObjectPtr(MiscOptionsVibratorCheckbox);
	cck2P = (ControlType *)GetObjectPtr(MiscOptionsNoAutoOffCheckbox);
	cck3P = (ControlType *)GetObjectPtr(MiscOptionsStdPaletteCheckbox);
	cck4P = (ControlType *)GetObjectPtr(MiscOptionsDebugCheckbox);
	cck5P = (ControlType *)GetObjectPtr(MiscOptionsLargerStackCheckbox);
	cck6P = (ControlType *)GetObjectPtr(MiscOptionsAutoResetCheckbox);
	cck7P = (ControlType *)GetObjectPtr(MiscOptionsDemoCheckbox);
	cck8P = (ControlType *)GetObjectPtr(MiscOptionsFullscreenCheckbox);
	cck9P = (ControlType *)GetObjectPtr(MiscOptionsAspectRatioCheckbox);
	cck10P= (ControlType *)GetObjectPtr(MiscOptionsCopyProtectionCheckbox);

	frmP = FrmGetActiveForm();

	if (FldGetTextLength(fld1P) == 0 && CtlGetValue(cck4P) == 1) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a debug level.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, MiscOptionsDebugLevelField));
		return;
	}

	gPrefs->vibrator = CtlGetValue(cck1P);
	gPrefs->autoOff = !CtlGetValue(cck2P);
	gPrefs->stdPalette = CtlGetValue(cck3P);
	gPrefs->debug = CtlGetValue(cck4P);
	gPrefs->autoReset = CtlGetValue(cck6P);
	gPrefs->demoMode = CtlGetValue(cck7P);
	gPrefs->fullscreen = CtlGetValue(cck8P);
	gPrefs->aspectRatio = CtlGetValue(cck9P);
	gPrefs->copyProtection = CtlGetValue(cck10P);

	gPrefs->debugLevel = StrAToI(FldGetTextPtr(fld1P));

	// Larger stack is a global data init at start up
	StackSize(CtlGetValue(cck5P) ? STACK_LARGER : STACK_DEFAULT);
	if (stackChanged)
		FrmCustomAlert(FrmInfoAlert,"You need to restart ScummVM in order for changes to take effect.",0,0);
	
	FrmReturnToMain();
}

static void FrmSetTabSize(const FormPtr frmP, UInt16 objID, Coord newY, Coord newH) {
	RectangleType r;
	UInt16 index;
	
	index = FrmGetObjectIndex (frmP, objID);
	FrmGetObjectBounds(frmP, index, &r);
	r.topLeft.y	= newY;
	r.extent.y	= newH;
	FrmSetObjectBounds(frmP, index, &r);
}

static Boolean FrmSelectTab(const FormPtr frmP, UInt16 objID) {
	FrmSetControlValue(frmP, FrmGetObjectIndex(frmP, objID), 0);

	if (tabNum != (objID - TAB_START)) {
		UInt8 color;
		
		FrmSetTabSize(frmP, (TAB_START + tabNum), 18, 10);
		FrmSetTabSize(frmP, objID, 16, 12);

		tabNum = objID - TAB_START;
		WinScreenLock(winLockDontCare);
		FrmDrawForm(frmP);
		color = UIColorGetTableEntryIndex(UIObjectFrame);
		WinSetForeColor(color);
		WinDrawLine(1, 28, 154,28);
		WinScreenUnlock();

		return true;
	}
		
	return false;
}

static void FrmShowHide(const FormPtr frmP, UInt16 idStart, UInt16 idEnd, Boolean show) {
	UInt16 item, index;
	Coord y = 35;
	
	for (item = idStart; item <= idEnd;	item++) {
		index = FrmGetObjectIndex (frmP, item);
		FrmSetObjectPosition(frmP, index, 4, y);
		y += 12;

		if (show)
			FrmShowObject(frmP, index);
		else
			FrmHideObject(frmP, index);
	}
}

static void MiscOptionsShowPalmOS(const FormPtr frmP, Boolean show) {
	FrmShowHide(frmP, MiscOptionsTabTitlePalmLabel, MiscOptionsLargerStackCheckbox, show);
}

static void MiscOptionsShowScummVM(const FormPtr frmP, Boolean show) {
	FrmShowHide(frmP, MiscOptionsTabTitleScummLabel, MiscOptionsCopyProtectionCheckbox, show);

	if (show)
		FrmShowObject(frmP, FrmGetObjectIndex (frmP, MiscOptionsDebugLevelField));
	else
		FrmHideObject(frmP, FrmGetObjectIndex (frmP, MiscOptionsDebugLevelField));
}

static void MiscOptionsFormInit() {

	FieldType *fld1P;
	FormPtr frmP;
	UInt16 item;
	UInt8 color;

	Char *levelP;
	MemHandle levelH;

	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsLargerStackCheckbox), (StackSize(STACK_GET) == STACK_LARGER));

	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsVibratorCheckbox), gPrefs->vibrator);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsNoAutoOffCheckbox), !gPrefs->autoOff);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsStdPaletteCheckbox), gPrefs->stdPalette);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsAutoResetCheckbox), gPrefs->autoReset);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsDebugCheckbox), gPrefs->debug);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsDemoCheckbox), gPrefs->demoMode);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsFullscreenCheckbox), gPrefs->fullscreen);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsAspectRatioCheckbox), gPrefs->aspectRatio);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsCopyProtectionCheckbox), gPrefs->copyProtection);
	
	fld1P = (FieldType *)GetObjectPtr(MiscOptionsDebugLevelField);

	levelH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	levelP = (Char *)MemHandleLock(levelH);
	StrIToA(levelP, gPrefs->debugLevel);
	MemHandleUnlock(levelH);

	FldSetTextHandle(fld1P, levelH);

	frmP = FrmGetActiveForm();
	// set tab size
	for (item = TAB_START; item < (TAB_START + TAB_COUNT); item++)
		FrmSetTabSize(frmP, item, 18, 10);

	tabNum = 0;
	FrmSetTabSize(frmP, (TAB_START + tabNum), 16, 12);
	MiscOptionsShowScummVM(frmP, false);
	MiscOptionsShowPalmOS(frmP, true);
	FrmSetObjectPosition(frmP, FrmGetObjectIndex (frmP, MiscOptionsDebugLevelField), 103, 35 + 12 * 3);

	FrmDrawForm(frmP);

	color = UIColorGetTableEntryIndex(UIObjectFrame);
	WinSetForeColor(color);
	WinDrawLine(1, 28, 154,28);
}

Boolean MiscOptionsFormHandleEvent(EventPtr eventP) {
	FormPtr frmP = FrmGetActiveForm();
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			MiscOptionsFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case MiscOptionsTabPalmPushButton:
					if (FrmSelectTab(frmP, MiscOptionsTabPalmPushButton))
						MiscOptionsShowScummVM(frmP, false);
						MiscOptionsShowPalmOS(frmP, true);
					break;

				case MiscOptionsTabScummPushButton:
					if (FrmSelectTab(frmP, MiscOptionsTabScummPushButton))
						MiscOptionsShowPalmOS(frmP, false);
						MiscOptionsShowScummVM(frmP, true);
					break;
				
				case MiscOptionsLargerStackCheckbox:
					stackChanged = !stackChanged;
					break;

				case MiscOptionsOKButton:
					MiscOptionsFormSave();
					break;

				case MiscOptionsCancelButton:
					FrmReturnToMain();
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
