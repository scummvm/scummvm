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

static void MiscOptionsFormSave() {

	FieldType *fld1P;
	ControlType *cck1P, *cck2P, *cck3P, *cck4P, *cck5P, *cck6P, *cck7P, *cck8P;	
	FormPtr frmP;

	fld1P = (FieldType *)GetObjectPtr(MiscOptionsDebugLevelField);
	
	cck1P = (ControlType *)GetObjectPtr(MiscOptionsVibratorCheckbox);
	cck2P = (ControlType *)GetObjectPtr(MiscOptionsNoAutoOffCheckbox);
	cck3P = (ControlType *)GetObjectPtr(MiscOptionsStdPaletteCheckbox);
	cck4P = (ControlType *)GetObjectPtr(MiscOptionsDebugCheckbox);
	cck5P = (ControlType *)GetObjectPtr(MiscOptionsWriteIniCheckbox);
	cck6P = (ControlType *)GetObjectPtr(MiscOptionsAutoResetCheckbox);
	cck7P = (ControlType *)GetObjectPtr(MiscOptionsDemoCheckbox);
	cck8P = (ControlType *)GetObjectPtr(MiscOptionsFullscreenCheckbox);

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
	gPrefs->saveConfig = CtlGetValue(cck5P);
	gPrefs->autoReset = CtlGetValue(cck6P);
	gPrefs->demoMode = CtlGetValue(cck7P);
	gPrefs->fullscreen = CtlGetValue(cck8P);

	gPrefs->debugLevel = StrAToI(FldGetTextPtr(fld1P));
	
	FrmReturnToMain();
}

static void MiscOptionsFormInit() {

	FieldType *fld1P;
	FormPtr frmP;

	Char *levelP;
	MemHandle levelH;

	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsVibratorCheckbox), gPrefs->vibrator);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsNoAutoOffCheckbox), !gPrefs->autoOff);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsStdPaletteCheckbox), gPrefs->stdPalette);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsAutoResetCheckbox), gPrefs->autoReset);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsDebugCheckbox), gPrefs->debug);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsWriteIniCheckbox), gPrefs->saveConfig);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsDemoCheckbox), gPrefs->demoMode);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsFullscreenCheckbox), gPrefs->fullscreen);

	fld1P = (FieldType *)GetObjectPtr(MiscOptionsDebugLevelField);

	levelH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	levelP = (Char *)MemHandleLock(levelH);
	StrIToA(levelP, gPrefs->debugLevel);
	MemHandleUnlock(levelH);

	FldSetTextHandle(fld1P, levelH);
	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);
}

Boolean MiscOptionsFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			MiscOptionsFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
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