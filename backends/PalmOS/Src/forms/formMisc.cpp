#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
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
static TabType *myTabP;
static UInt16 lastTab = 0;

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

static Boolean ScummVMTabSave() {
	FieldType *fld1P;
	ControlType *cckP[11];
	FormPtr frmP;

	fld1P = (FieldType *)GetObjectPtr(TabScummVMDebugLevelField);
	
	cckP[3] = (ControlType *)GetObjectPtr(TabScummVMDebugCheckbox);
	cckP[6] = (ControlType *)GetObjectPtr(TabScummVMDemoCheckbox);
	cckP[7] = (ControlType *)GetObjectPtr(TabScummVMFullscreenCheckbox);
	cckP[8] = (ControlType *)GetObjectPtr(TabScummVMAspectRatioCheckbox);
	cckP[9] = (ControlType *)GetObjectPtr(TabScummVMCopyProtectionCheckbox);

	frmP = FrmGetActiveForm();
	if (FldGetTextLength(fld1P) == 0 && CtlGetValue(cckP[3]) == 1) {
		TabSetActive(frmP, myTabP, 1);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabScummVMDebugLevelField));
		FrmCustomAlert(FrmWarnAlert,"You must specified a debug level.",0,0);
		return false;
	}

	gPrefs->debug = CtlGetValue(cckP[3]);
	gPrefs->demoMode = CtlGetValue(cckP[6]);
	gPrefs->fullscreen = CtlGetValue(cckP[7]);
	gPrefs->aspectRatio = CtlGetValue(cckP[8]);
	gPrefs->copyProtection = CtlGetValue(cckP[9]);

	gPrefs->debugLevel = StrAToI(FldGetTextPtr(fld1P));
	
	return true;
}

static void PalmOSTabSave() {
	ControlType *cckP[11];

	cckP[0] = (ControlType *)GetObjectPtr(TabPalmOSVibratorCheckbox);
	cckP[1] = (ControlType *)GetObjectPtr(TabPalmOSNoAutoOffCheckbox);
	cckP[2] = (ControlType *)GetObjectPtr(TabPalmOSStdPaletteCheckbox);
	cckP[4] = (ControlType *)GetObjectPtr(TabPalmOSLargerStackCheckbox);
	cckP[5] = (ControlType *)GetObjectPtr(TabPalmOSAutoResetCheckbox);
	cckP[10]= (ControlType *)GetObjectPtr(TabPalmOSARMCheckbox);

	gPrefs->vibrator = CtlGetValue(cckP[0]);
	gPrefs->autoOff = !CtlGetValue(cckP[1]);
	gPrefs->stdPalette = CtlGetValue(cckP[2]);
	gPrefs->autoReset = CtlGetValue(cckP[5]);
	gPrefs->arm = CtlGetValue(cckP[10]);

	// Larger stack is a global data init at start up
	StackSize(CtlGetValue(cckP[4]) ? STACK_LARGER : STACK_DEFAULT);
	if (stackChanged)
		FrmCustomAlert(FrmInfoAlert,"You need to restart ScummVM in order for changes to take effect.",0,0);
}

static void ScummVMTabInit() {
	FieldType *fld1P;
	Char *levelP;
	MemHandle levelH;


	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMDebugCheckbox), gPrefs->debug);
	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMDemoCheckbox), gPrefs->demoMode);
	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMFullscreenCheckbox), gPrefs->fullscreen);
	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMAspectRatioCheckbox), gPrefs->aspectRatio);
	CtlSetValue((ControlType *)GetObjectPtr(TabScummVMCopyProtectionCheckbox), gPrefs->copyProtection);

	fld1P = (FieldType *)GetObjectPtr(TabScummVMDebugLevelField);

	levelH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	levelP = (Char *)MemHandleLock(levelH);
	StrIToA(levelP, gPrefs->debugLevel);
	MemHandleUnlock(levelH);

	FldSetTextHandle(fld1P, levelH);
}

static void PalmOSTabInit() {
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSLargerStackCheckbox), (StackSize(STACK_GET) == STACK_LARGER));

	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSVibratorCheckbox), gPrefs->vibrator);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSNoAutoOffCheckbox), !gPrefs->autoOff);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSStdPaletteCheckbox), gPrefs->stdPalette);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSAutoResetCheckbox), gPrefs->autoReset);
	CtlSetValue((ControlType *)GetObjectPtr(TabPalmOSARMCheckbox), gPrefs->arm);
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
					break;

				case TabPalmOSLargerStackCheckbox:
					stackChanged = !stackChanged;
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
