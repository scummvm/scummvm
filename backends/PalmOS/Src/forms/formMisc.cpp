#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
#include "forms.h"
#include "globals.h"

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

static Boolean ScummVMTabSave() {
	FieldType *fld1P;
	ControlType *cckP[11];
	FormPtr frmP;

	fld1P = (FieldType *)GetObjectPtr(TabMiscScummVMDebugLevelField);
	
	cckP[3] = (ControlType *)GetObjectPtr(TabMiscScummVMDebugCheckbox);
	cckP[6] = (ControlType *)GetObjectPtr(TabMiscScummVMDemoCheckbox);
	cckP[9] = (ControlType *)GetObjectPtr(TabMiscScummVMCopyProtectionCheckbox);
	cckP[10]= (ControlType *)GetObjectPtr(TabMiscScummVMAltIntroCheckbox);

	frmP = FrmGetActiveForm();
	if (FldGetTextLength(fld1P) == 0 && CtlGetValue(cckP[3]) == 1) {
		TabSetActive(frmP, myTabP, 1);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabMiscScummVMDebugLevelField));
		FrmCustomAlert(FrmWarnAlert,"You must specified a debug level.",0,0);
		return false;
	}

	gPrefs->debug = CtlGetValue(cckP[3]);
	gPrefs->demoMode = CtlGetValue(cckP[6]);
	gPrefs->copyProtection = CtlGetValue(cckP[9]);
	gPrefs->altIntro = CtlGetValue(cckP[10]);

	gPrefs->debugLevel = StrAToI(FldGetTextPtr(fld1P));
	
	return true;
}

static void PalmOSTabSave() {
	ControlType *cckP[11];

	if (OPTIONS_TST(kOptDeviceARM)) {
		cckP[10]= (ControlType *)GetObjectPtr(TabMiscPalmOSARMCheckbox);
		gPrefs->arm = CtlGetValue(cckP[10]);
	}

	cckP[0] = (ControlType *)GetObjectPtr(TabMiscPalmOSVibratorCheckbox);
	cckP[1] = (ControlType *)GetObjectPtr(TabMiscPalmOSNoAutoOffCheckbox);
	cckP[2] = (ControlType *)GetObjectPtr(TabMiscPalmOSStdPaletteCheckbox);
	cckP[4] = (ControlType *)GetObjectPtr(TabMiscPalmOSLargerStackCheckbox);
	cckP[5] = (ControlType *)GetObjectPtr(TabMiscPalmOSExitLauncherCheckbox);
	cckP[6] = (ControlType *)GetObjectPtr(TabMiscPalmOSStylusClickCheckbox);

	gPrefs->vibrator = CtlGetValue(cckP[0]);
	gPrefs->autoOff = !CtlGetValue(cckP[1]);
	gPrefs->stdPalette = CtlGetValue(cckP[2]);
	gPrefs->setStack = CtlGetValue(cckP[4]);
	gPrefs->exitLauncher = CtlGetValue(cckP[5]);
	gPrefs->stylusClick = !CtlGetValue(cckP[6]);
}

static void ExtsTabSave() {
	ControlType *cckP[2];

	if (OPTIONS_TST(kOptLightspeedAPI)) {
		ListType *list1P = (ListType *)GetObjectPtr(TabMiscExtsLightspeedList);
		cckP[0] = (ControlType *)GetObjectPtr(TabMiscExtsLightspeedCheckbox);

		gPrefs->lightspeed.enable = CtlGetValue(cckP[0]);
		gPrefs->lightspeed.mode = LstGetSelection(list1P);
	}
	if (OPTIONS_TST(kOptGoLcdAPI)) {	
		cckP[1] = (ControlType *)GetObjectPtr(TabMiscExtsGolcdCheckbox);
		gPrefs->goLCD = CtlGetValue(cckP[1]);
	}
}

static void ScummVMTabInit() {
	FieldType *fld1P;
	Char *levelP;
	MemHandle levelH;

	CtlSetValue((ControlType *)GetObjectPtr(TabMiscScummVMDebugCheckbox), gPrefs->debug);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscScummVMDemoCheckbox), gPrefs->demoMode);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscScummVMCopyProtectionCheckbox), gPrefs->copyProtection);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscScummVMAltIntroCheckbox), gPrefs->altIntro);

	fld1P = (FieldType *)GetObjectPtr(TabMiscScummVMDebugLevelField);

	levelH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	levelP = (Char *)MemHandleLock(levelH);
	StrIToA(levelP, gPrefs->debugLevel);
	MemHandleUnlock(levelH);

	FldSetTextHandle(fld1P, levelH);
}

static void PalmOSTabInit() {
	if (OPTIONS_TST(kOptDeviceARM))
		CtlSetValue((ControlType *)GetObjectPtr(TabMiscPalmOSARMCheckbox), gPrefs->arm);

	CtlSetValue((ControlType *)GetObjectPtr(TabMiscPalmOSExitLauncherCheckbox), gPrefs->exitLauncher);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscPalmOSLargerStackCheckbox), gPrefs->setStack);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscPalmOSVibratorCheckbox), gPrefs->vibrator);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscPalmOSNoAutoOffCheckbox), !gPrefs->autoOff);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscPalmOSStdPaletteCheckbox), gPrefs->stdPalette);
	CtlSetValue((ControlType *)GetObjectPtr(TabMiscPalmOSStylusClickCheckbox), !gPrefs->stylusClick);
}

static void ExtsTabInit() {
	if (OPTIONS_TST(kOptLightspeedAPI)) {
		ListType *list1P = (ListType *)GetObjectPtr(TabMiscExtsLightspeedList);
		LstSetSelection(list1P, gPrefs->lightspeed.mode);
		CtlSetLabel((ControlType *)GetObjectPtr(TabMiscExtsLightspeedPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
		CtlSetValue((ControlType *)GetObjectPtr(TabMiscExtsLightspeedCheckbox), gPrefs->lightspeed.enable);
	}

	if (OPTIONS_TST(kOptGoLcdAPI))
		CtlSetValue((ControlType *)GetObjectPtr(TabMiscExtsGolcdCheckbox), gPrefs->goLCD);
}

static void MiscFormSave() {
	if (!ScummVMTabSave()) return;
	PalmOSTabSave();
	ExtsTabSave();
	
	TabDeleteTabs(myTabP);
	FrmReturnToMain();
}

static void MiscFormInit() {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();
	UInt8 extsCnt = 2;

	tabP = TabNewTabs(3);
	TabAddContent(&frmP, tabP, "PalmOS", TabMiscPalmOSForm);
	TabAddContent(&frmP, tabP, "ScummVM", TabMiscScummVMForm);
	TabAddContent(&frmP, tabP, "More ...", TabMiscExtsForm);

	if (!OPTIONS_TST(kOptDeviceARM)) {
		FrmRemoveObject(&frmP, FrmGetObjectIndex(frmP, TabMiscPalmOSARMCheckbox));
	}
	if (!OPTIONS_TST(kOptGoLcdAPI)) {
		FrmRemoveObject(&frmP, FrmGetObjectIndex(frmP, TabMiscExtsGolcdCheckbox));
		// move lightspeed
		TabMoveUpObject(frmP, TabMiscExtsLightspeedCheckbox, 12);
		TabMoveUpObject(frmP, TabMiscExtsLightspeedPopTrigger, 12);
		TabMoveUpObject(frmP, TabMiscExtsLightspeedList, 12);
		TabMoveUpObject(frmP, TabMiscExtsNothingLabel, 12);
		extsCnt--;
	}

	if (!OPTIONS_TST(kOptLightspeedAPI)) {
		FrmRemoveObject(&frmP, FrmGetObjectIndex(frmP, TabMiscExtsLightspeedCheckbox));
//		FrmRemoveObject(&frmP, FrmGetObjectIndex(frmP, TabMiscExtsLightspeedList)); // cannot remove this ?
		FrmRemoveObject(&frmP, FrmGetObjectIndex(frmP, TabMiscExtsLightspeedPopTrigger));
		TabMoveUpObject(frmP, TabMiscExtsNothingLabel, 12);
		extsCnt--;
	}
	
	if (extsCnt)
		FrmRemoveObject(&frmP, FrmGetObjectIndex(frmP, TabMiscExtsNothingLabel));

	PalmOSTabInit();
	ScummVMTabInit();
	ExtsTabInit();

	FrmDrawForm(frmP);
	TabSetActive(frmP, tabP, lastTab);

	myTabP = tabP;
}

Boolean MiscFormHandleEvent(EventPtr eventP) {
	FormPtr frmP = FrmGetActiveForm();
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			MiscFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case (MiscForm + 1) :
				case (MiscForm + 2) :
				case (MiscForm + 3) :
					lastTab = (eventP->data.ctlSelect.controlID - MiscForm - 1);
					TabSetActive(frmP, myTabP, lastTab);
					break;

				case TabMiscExtsLightspeedPopTrigger:
					FrmList(eventP, TabMiscExtsLightspeedList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabMiscExtsLightspeedList));
					break;

				case MiscOKButton:
					MiscFormSave();
					break;

				case MiscCancelButton:
					TabDeleteTabs(myTabP);
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
