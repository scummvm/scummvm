#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
#include "forms.h"
#include "globals.h"

#ifndef DISABLE_LIGHTSPEED
#include "lightspeed_public.h"
#endif
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

	fld1P = (FieldType *)GetObjectPtr(TabScummVMDebugLevelField);

	cckP[3] = (ControlType *)GetObjectPtr(TabScummVMDebugCheckbox);
	cckP[6] = (ControlType *)GetObjectPtr(TabScummVMDemoCheckbox);
	cckP[9] = (ControlType *)GetObjectPtr(TabScummVMCopyProtectionCheckbox);
	cckP[10]= (ControlType *)GetObjectPtr(TabScummVMAltIntroCheckbox);

	frmP = FrmGetActiveForm();
	if (FldGetTextLength(fld1P) == 0 && CtlGetValue(cckP[3]) == 1) {
		TabSetActive(frmP, myTabP, 1);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabScummVMDebugLevelField));
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

	cckP[0] = (ControlType *)GetObjectPtr(TabPalmOSVibratorCheckbox);
	cckP[1] = (ControlType *)GetObjectPtr(TabPalmOSNoAutoOffCheckbox);
	cckP[2] = (ControlType *)GetObjectPtr(TabPalmOSStdPaletteCheckbox);
	cckP[3] = (ControlType *)GetObjectPtr(TabPalmOSLightspeedCheckbox);
	cckP[4] = (ControlType *)GetObjectPtr(TabPalmOSLargerStackCheckbox);
	cckP[5] = (ControlType *)GetObjectPtr(TabPalmOSExitLauncherCheckbox);
	cckP[10]= (ControlType *)GetObjectPtr(TabPalmOSARMCheckbox);

	gPrefs->vibrator = CtlGetValue(cckP[0]);
	gPrefs->autoOff = !CtlGetValue(cckP[1]);
	gPrefs->stdPalette = CtlGetValue(cckP[2]);
	gPrefs->lightspeed.enable = CtlGetValue(cckP[3]);
	gPrefs->setStack = CtlGetValue(cckP[4]);
	gPrefs->exitLauncher = CtlGetValue(cckP[5]);
	gPrefs->arm = CtlGetValue(cckP[10]);

#ifndef DISABLE_LIGHTSPEED
	if (LS_Installed()) {
		ListType *list1P = (ListType *)GetObjectPtr(TabPalmOSLightspeedList);
		cckP[6] = (ControlType *)GetObjectPtr(TabPalmOSLightspeedCheckbox);

		gPrefs->lightspeed.enable = CtlGetValue(cckP[6]);
		gPrefs->lightspeed.mode = LstGetSelection(list1P);
	}
#endif
}

static void ScummVMTabInit() {
	FieldType *fld1P;
	Char *levelP;
	MemHandle levelH;

	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMDebugCheckbox), gPrefs->debug);
	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMDemoCheckbox), gPrefs->demoMode);
	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMCopyProtectionCheckbox), gPrefs->copyProtection);
	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMAltIntroCheckbox), gPrefs->altIntro);

	fld1P = (FieldType *)GetObjectPtr(TabScummVMDebugLevelField);

	levelH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	levelP = (Char *)MemHandleLock(levelH);
	StrIToA(levelP, gPrefs->debugLevel);
	MemHandleUnlock(levelH);

	FldSetTextHandle(fld1P, levelH);
}

static void PalmOSTabInit() {
#ifndef DISABLE_LIGHTSPEED
	if (LS_Installed()) {
		ListType *list1P = (ListType *)GetObjectPtr(TabPalmOSLightspeedList);
		LstSetSelection(list1P, gPrefs->lightspeed.mode);
		CtlSetLabel((ControlType *)GetObjectPtr(TabPalmOSLightspeedPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
	}
#endif

	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSExitLauncherCheckbox), gPrefs->lightspeed.enable);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSExitLauncherCheckbox), gPrefs->exitLauncher);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSLargerStackCheckbox), gPrefs->setStack);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSVibratorCheckbox), gPrefs->vibrator);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSNoAutoOffCheckbox), !gPrefs->autoOff);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSStdPaletteCheckbox), gPrefs->stdPalette);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSARMCheckbox), gPrefs->arm);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSLightspeedCheckbox), gPrefs->lightspeed.enable);
}

static void MiscFormSave() {
	PalmOSTabSave();
	if (!ScummVMTabSave()) return;

	TabDeleteTabs(myTabP);
	FrmReturnToMain();
}

static void MiscFormInit() {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();

	tabP = TabNewTabs(2);
	TabAddContent(&frmP, tabP, "PalmOS", TabPalmOSForm);
	TabAddContent(&frmP, tabP, "ScummVM", TabScummVMForm);

	PalmOSTabInit();
	ScummVMTabInit();

	FrmDrawForm(frmP);
	TabSetActive(frmP, tabP, lastTab);

#ifndef DISABLE_LIGHTSPEED
	if (!LS_Installed())
#endif
	{
		FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabPalmOSLightspeedCheckbox));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabPalmOSLightspeedPopTrigger));
	}
#ifndef DISABLE_ARM
	if (!OPTIONS_TST(kOptDeviceARM))
#endif
	{	FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabPalmOSARMCheckbox)); }

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
					lastTab = (eventP->data.ctlSelect.controlID - MiscForm - 1);
					TabSetActive(frmP, myTabP, lastTab);

#ifndef DISABLE_LIGHTSPEED
					if (!LS_Installed())
#endif
					{
						FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabPalmOSLightspeedCheckbox));
						FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabPalmOSLightspeedPopTrigger));
					}
#ifndef DISABLE_ARM
					if (!OPTIONS_TST(kOptDeviceARM))
#endif
					{	FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabPalmOSARMCheckbox)); }
					break;

				case TabPalmOSLightspeedPopTrigger:
					FrmList(eventP, TabPalmOSLightspeedList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabPalmOSLightspeedList));
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
