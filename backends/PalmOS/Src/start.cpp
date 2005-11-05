/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include <PalmOS.h>
#include <SonyClie.h>

#include "palmdefs.h"
#include "start.h"
#include "games.h"
#include "globals.h"
#include "modules.h"
#include "skin.h"

#include "forms.h"

/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
GlobalsPreferencePtr gPrefs;
GlobalsDataPtr gVars;

Boolean bDirectMode = false;
Boolean bStartScumm = false;
Boolean bLaunched	= false;

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/

// Define the minimum OS version we support (3.5 for now).
#define kOurMinVersion	sysMakeROMVersion(3,5,0,sysROMStageRelease,0)
#define kPalmOS10Version	sysMakeROMVersion(1,0,0,sysROMStageRelease,0)

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

// Callback for ExgDBWrite to send data with Exchange Manager
static Err WriteDBData(const void* dataP, UInt32* sizeP, void* userDataP) {
	Err err;
	*sizeP = ExgSend((ExgSocketPtr)userDataP, (void*)dataP, *sizeP, &err);
	return err;
}

Err SendDatabase (UInt16 cardNo, LocalID dbID, Char *nameP, Char *descriptionP) {
	ExgSocketType exgSocket;
	Err err;

	// Create exgSocket structure
	MemSet(&exgSocket, sizeof(exgSocket), 0);
	exgSocket.description = descriptionP;
	exgSocket.name = nameP;

	// Start an exchange put operation
	err = ExgPut(&exgSocket);
	if (!err) {
		err = ExgDBWrite(WriteDBData, &exgSocket, NULL, dbID, cardNo);
		err = ExgDisconnect(&exgSocket, err);
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h 
 *                                for format)
 *              launchFlags     - flags that indicate if the application 
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags) {
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion) {
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {

			FrmAlert (RomIncompatibleAlert);

			// Palm OS 1.0 will continuously relaunch this app unless we switch to 
			// another safe one.
			if (romVersion <= kPalmOS10Version)
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);

		}
		return sysErrRomIncompatible;
	}

	return errNone;
}

///////////////////////////////////////////////////////////////////////


void SavePrefs() {
	if (gPrefs) {
		PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, gPrefs, sizeof (GlobalsPreferenceType), true);
		MemPtrFree(gPrefs);
		gPrefs = NULL;
	}
}

/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP) {
	UInt16 formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent) {
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId) {
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;

			case SkinsForm:
				FrmSetEventHandler(frmP, SkinsFormHandleEvent);
				break;

			case GameEditForm:
				FrmSetEventHandler(frmP, EditGameFormHandleEvent);
				break;

			case MiscForm:
				FrmSetEventHandler(frmP, MiscFormHandleEvent);
				break;

			case CardSlotForm:
				FrmSetEventHandler(frmP, CardSlotFormHandleEvent);
				break;

			case MusicForm:
				FrmSetEventHandler(frmP, MusicFormHandleEvent);
				break;

			case InfoForm:
				FrmSetEventHandler(frmP, InfoFormHandleEvent);
				break;

			default:
//				ErrFatalDisplay("Invalid Form Load Event");
				break;
		}
		return true;
		}
	
	return false;
}

/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppEventLoop(void) {
	UInt16 error;
	EventType event;

	do {
		EvtGetEvent(&event, evtNoWait);

		if(bStartScumm)
			bStartScumm = StartScummVM();

		if (! SysHandleEvent(&event))
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}

/***********************************************************************
 *
 * FUNCTION:    ScummVMPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppLaunchCmdNotify(UInt16 LaunchFlags, SysNotifyParamType * pData) {
	switch (pData->notifyType) {
		case sysNotifyVolumeMountedEvent:
			pData->handled = true;	// don't switch

			if (gPrefs) {	// gPrefs exists ? so we are in the palm selector
				CardSlotFormUpdate(); // redraw card list if needed

				if (gPrefs->card.volRefNum == sysInvalidRefNum) {
					VFSAnyMountParamType *notifyDetailsP = (VFSAnyMountParamType *)pData->notifyDetailsP;
					gPrefs->card.volRefNum = notifyDetailsP->volRefNum;

					if (FrmGetFormPtr(MainForm) == FrmGetActiveForm())
						if (gPrefs->card.volRefNum != sysInvalidRefNum) {
							CardSlotCreateDirs();
							FrmUpdateForm(MainForm, frmRedrawUpdateMSImport);
						}
				}
			}
			break;

		case sysNotifyVolumeUnmountedEvent:
			if (gPrefs) {
				CardSlotFormUpdate();

				if (gPrefs->card.volRefNum == (UInt16)pData->notifyDetailsP) {
					gPrefs->card.volRefNum = sysInvalidRefNum;

					if (FrmGetFormPtr(MainForm) == FrmGetActiveForm())
						FrmUpdateForm(MainForm, frmRedrawUpdateMS);
				}
			}
			break;

		case sysNotifyDisplayResizedEvent:
			if (gVars)
				if (FrmGetFormPtr(MainForm) == FrmGetActiveForm())
					SknApplySkin();
			break;

		case sonySysNotifyMsaEnforceOpenEvent:
			// what am i supposed to do here ???
			break;
	}
}

static UInt32 ScummVMPalmMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
	Err error;

	error = RomVersionCompatible (kOurMinVersion, launchFlags);
	if (error) return (error);

	switch (cmd) {
		case sysAppLaunchCmdNotify:
			AppLaunchCmdNotify(launchFlags, (SysNotifyParamType *) cmdPBP);
			break;

		case sysAppLaunchCustomEngineDelete:
#ifndef _DEBUG_ENGINE
			ModDelete();
#endif
			break;

		case sysAppLaunchCmdNormalLaunch:	
			error = AppStart();
			if (error) 
				goto end;

			if (!bDirectMode) {
				FrmGotoForm(MainForm);
			}else {
				GamUnselect();
				bStartScumm = true;
			}

			AppEventLoop();
end:
			AppStop();
			break;

		default:
			break;

	}
	return error;
}

/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/

UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
	return ScummVMPalmMain(cmd, cmdPBP, launchFlags);
}
