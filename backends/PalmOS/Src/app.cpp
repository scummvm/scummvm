#include <PalmOS.h>
#include <SonyClie.h>

#include "StarterRsc.h"
#include "start.h"
#include "globals.h"

#include "mathlib.h"
#include "formCards.h"
#include "games.h"

#define kOS5Version		sysMakeROMVersion(5,0,0,sysROMStageRelease,0)

/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err AppStartCheckHRmode()
{
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	UInt32 version;
	Err error = errNone;

	// test if sonyHR is present
	if (!(error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR) {		// HR available

			if ((error = SysLibFind(sonySysLibNameHR, &gVars->HRrefNum)))
				if (error == sysErrLibNotFound)							// couldn't find lib
					error = SysLibLoad( 'libr', sonySysFileCHRLib, &gVars->HRrefNum);

			if (!error) {	// Now we can use HR lib. Executes Open library.
				error = HROpen(gVars->HRrefNum);
				gVars->options |= optIsClieDevice;
			}
		}
	}
	// if not Hi-Density ?
	if (error && !(error = FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version))) {
		gVars->HRrefNum = sysInvalidRefNum;	// Not sony HR
		gVars->options &= ~optIsClieDevice;

		if (version<4)
			error = 1;
	}

	if (!error) { // Not, error processing
		UInt32 width, height, depth;
		Boolean color;

		width = hrWidth;
		height= hrHeight;
		depth = 8;
		color = true;

		if (gVars->HRrefNum != sysInvalidRefNum)
			error = HRWinScreenMode (gVars->HRrefNum, winScreenModeSet, &width, &height, &depth, &color );
		else
			error = WinScreenMode (winScreenModeSet, &width, &height, &depth, &color );

		// high-resolution mode entered from here if no error
		if (error != errNone)
			FrmCustomAlert(FrmErrorAlert,"Your device doesn't seems to support 256 colors mode.",0,0);

		// check for 16bit mode
		if (!WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, &color))
			gVars->options |= ((depth & 0x8000) ? optHas16BitMode : 0);
	}
	else
		FrmCustomAlert(FrmErrorAlert,"This device doesn't seems to support\nHi-Res mode.",0,0);

	return error;
}

static void AppStopHRMode() {
	if (gVars->HRrefNum != sysInvalidRefNum) {
			HRClose(gVars->HRrefNum);
			//SysLibRemove(gVars->HRrefNum);	// never call this !!
	}
}

static Err AppStartCheckNotify() {
	UInt32 romVersion;
	Err err;

	err = FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &romVersion); 
	if (!err) {
		UInt16 cardNo;
		LocalID dbID;

		err = SysCurAppDatabase(&cardNo, &dbID);
		if (!err) {
			SysNotifyRegister(cardNo, dbID, sysNotifyVolumeMountedEvent, NULL, sysNotifyNormalPriority, NULL);
			SysNotifyRegister(cardNo, dbID, sysNotifyVolumeUnmountedEvent, NULL, sysNotifyNormalPriority, NULL);
			SysNotifyRegister(cardNo, dbID, sonySysNotifyMsaEnforceOpenEvent, NULL, sysNotifyNormalPriority, NULL);
			SysNotifyRegister(cardNo, dbID, sysNotifyDisplayResizedEvent, NULL, sysNotifyNormalPriority, NULL);
		}
	}

	return err;
}

static Err AppStartLoadSkin() {
	Err err = errNone;

	// if skin defined, check if the db still exists
	if (gPrefs->skin.dbID) {
		UInt32 type, creator;

		// remember to check version for next revision of the skin
		err = DmDatabaseInfo (gPrefs->skin.cardNo, gPrefs->skin.dbID, gPrefs->skin.nameP, 0, 0, 0, 0, 0, 0, 0,0, &type, &creator);
		if (!err)
			if (type != 'skin' || creator != appFileCreator)
				err = dmErrInvalidParam;

		if (err)
			MemSet(&(gPrefs->skin),sizeof(SkinInfoType),0);
	}

	// No skin ? try to get the first one
	if (!gPrefs->skin.dbID) {
		DmSearchStateType stateInfo;

		err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, 'skin', appFileCreator, false, &gPrefs->skin.cardNo, &gPrefs->skin.dbID);
		if (!err)
			err = DmDatabaseInfo (gPrefs->skin.cardNo, gPrefs->skin.dbID, gPrefs->skin.nameP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	if (err)
		FrmCustomAlert(FrmErrorAlert,"No skin found.\nPlease install a skin and restart ScummVM.",0,0);
	
	return err;
}

static Err AppStartCheckMathLib()
{
	Err error = errNone;

	if ((error = SysLibFind(MathLibName, &MathLibRef)))
		if (error == sysErrLibNotFound)									// couldn't find lib
			error = SysLibLoad( LibType, MathLibCreator, &MathLibRef);

	if (!error) {// Now we can use lib. Executes Open library.
		error = MathLibOpen(MathLibRef, MathLibVersion);
		if (error)
			FrmCustomAlert(FrmErrorAlert,"Can't open MathLib !",0,0);

	} else {
		FrmCustomAlert(FrmErrorAlert,"Can't find MathLib.\nPlease install it first.",0,0);
	}

	return error;
}

static void AppStopMathLib() {
	UInt16 useCount;

	if (MathLibRef != sysInvalidRefNum) {
		MathLibClose(MathLibRef, &useCount);
		
		if (!useCount)
			SysLibRemove(MathLibRef);
	}
}

// Set the screen pitch for direct screen access
// avaliable only before a game start
void WinScreenGetPitch() {
	if (gVars->HRrefNum == sysInvalidRefNum) // TODO : better to check if Hi-Density feature is present (?)
		WinScreenGetAttribute(winScreenRowBytes, &(gVars->screenPitch));		
	else
		gVars->screenPitch = gVars->screenFullWidth;
}

void PINGetScreenDimensions() {
	UInt32 ftr;

	gVars->pinUpdate = false;

	// if feature set, not set on Garmin iQue3600
	if (!(FtrGet(sysFtrCreator, sysFtrNumInputAreaFlags, &ftr))) {
		if (ftr & grfFtrInputAreaFlagCollapsible) {

			RectangleType r;
			UInt16 curOrientation = SysGetOrientation();

			gVars->options |= optIsCollapsible;
			// reset previous options if any
			gVars->options &= ~optHasWideMode;
			gVars->options &= ~optHasLandscapeMode;
			gVars->options &= ~optIsLandscapeDisplay;

			PINSetInputAreaState(pinInputAreaClosed);
			StatHide();

			WinGetBounds(WinGetDisplayWindow(), &r);
			gVars->screenFullWidth = r.extent.x << 1;
			gVars->screenFullHeight = r.extent.y << 1;

			gVars->options |= optHasWideMode;
			
			if (curOrientation == sysOrientationLandscape ||
				curOrientation == sysOrientationReverseLandscape
					)
				gVars->options |= optHasLandscapeMode;
			
			StatShow();
			PINSetInputAreaState(pinInputAreaOpen);
		}
	}

	gVars->pinUpdate = true;
}

static Err AppStartCheckScreenSize() {
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error = errNone;

	gVars->screenWidth = 320;
	gVars->screenHeight = 320;

	gVars->screenFullWidth = gVars->screenWidth;
	gVars->screenFullHeight = gVars->screenHeight;

	// Sony HiRes+
	if (!(error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrSilk) {

			if ((error = SysLibFind(sonySysLibNameSilk, &gVars->slkRefNum)))
				if (error == sysErrLibNotFound)	
					error = SysLibLoad( sonySysFileTSilkLib, sonySysFileCSilkLib, &gVars->slkRefNum);

			if (!error) {
				error = FtrGet(sonySysFtrCreator, sonySysFtrNumVskVersion, &gVars->slkVersion);
				// Get screen size
				if (error) {
					// v1 = NR
				 	error = SilkLibOpen(gVars->slkRefNum);
					if(!error) {
						gVars->slkVersion = vskVersionNum1;
						SilkLibEnableResize(gVars->slkRefNum);
						SilkLibResizeDispWin(gVars->slkRefNum, silkResizeMax);
						HRWinGetWindowExtent(gVars->HRrefNum, &gVars->screenFullWidth, &gVars->screenFullHeight);
						SilkLibResizeDispWin(gVars->slkRefNum, silkResizeNormal);
						SilkLibDisableResize(gVars->slkRefNum);
					}
				} else {
					// v2 = NX/NZ
					// v3 = UX
				 	error = VskOpen(gVars->slkRefNum);
					if(!error) {
						VskSetState(gVars->slkRefNum, vskStateEnable, (gVars->slkVersion == vskVersionNum2 ? vskResizeVertically : vskResizeHorizontally));
						gVars->options |= (gVars->slkVersion == vskVersionNum2 ? optHasLandscapeMode : 0);
						VskSetState(gVars->slkRefNum, vskStateResize, vskResizeNone);
						HRWinGetWindowExtent(gVars->HRrefNum, &gVars->screenFullWidth, &gVars->screenFullHeight);
						VskSetState(gVars->slkRefNum, vskStateResize, vskResizeMax);
						VskSetState(gVars->slkRefNum, vskStateEnable, vskResizeDisable);
					}
				}
			}
		}

		if (error)
			gVars->slkRefNum = sysInvalidRefNum;
		else
			gVars->options |= optHasWideMode;
	}
	// Tapwave Zodiac and other DIA API compatible devies
	// get max screen size
	if (error)
		PINGetScreenDimensions();

	WinScreenGetPitch();
	return error;
}

static void AppStopSilk() {
	if (gVars->slkRefNum != sysInvalidRefNum)
		SilkLibClose(gVars->slkRefNum);
}

Err AppStart(void) {
	UInt16 dataSize, checkSize = 0;
	UInt32 ulProcessorType;
	UInt32 romVersion;
	Err error;

	// allocate global variables space
	dataSize = sizeof(GlobalsDataType);
	gVars = (GlobalsDataType *)MemPtrNew(dataSize);
	MemSet(gVars, dataSize, 0);

	gVars->indicator.on	= 255;
	gVars->indicator.off = 0;
	gVars->HRrefNum = sysInvalidRefNum;
	gVars->volRefNum = sysInvalidRefNum;
	gVars->slkRefNum = sysInvalidRefNum;
	gVars->skinSet = false;
	gVars->options = optNone;

	// OS5 ?
	if (!FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion))
		gVars->options |= ((romVersion >= kOS5Version) ? optIsOS5Device : 0);

	// ARM ?
 	if (!FtrGet(sysFileCSystem, sysFtrNumProcessorID, &ulProcessorType))
 		gVars->options |= ((sysFtrNumProcessorIsARM(ulProcessorType)) ? optIsARMDevice : 0);

	// allocate prefs space
	dataSize = sizeof(GlobalsPreferenceType);
	gPrefs	= (GlobalsPreferenceType *)MemPtrNew(dataSize);
	MemSet(gPrefs, dataSize, 0);

	// Read the saved preferences / saved-state information.
	if (PrefGetAppPreferences(appFileCreator, appPrefID, NULL, &checkSize, true) == noPreferenceFound || checkSize != dataSize) {
		// reset all elements
		MemSet(gPrefs, dataSize, 0);

		gPrefs->card.volRefNum = sysInvalidRefNum;

		gPrefs->autoOff = true;
		gPrefs->vibrator = CheckVibratorExists();
		gPrefs->debug = false;

		gPrefs->stdPalette = (gVars->options & optIsOS5Device);
		
		gPrefs->volume.speaker = 16;
		gPrefs->volume.headphone = 16;
		
		gPrefs->volume.master = 192;
		gPrefs->volume.music = 192;
		gPrefs->volume.sfx = 192;
		
		gPrefs->sound.tempo = 100;
		gPrefs->sound.defaultTrackLength = 10;
		gPrefs->sound.firstTrack = 1;
		
	} else {
		PrefGetAppPreferences(appFileCreator, appPrefID, gPrefs, &dataSize, true);
	}

	error = AppStartCheckMathLib();
	if (error) return (error);

	error = AppStartCheckHRmode();
	if (error) return (error);
	
	error = AppStartLoadSkin();
	if (error) return (error);

	if (gPrefs->card.volRefNum != sysInvalidRefNum) {	// if volref previously defined, check if it's a valid one
		VolumeInfoType volInfo;
		Err err = VFSVolumeInfo(gPrefs->card.volRefNum, &volInfo);
		if (err)
			gPrefs->card.volRefNum = sysInvalidRefNum;
	}
	else
		gPrefs->card.volRefNum = parseCards(); //parseCards(0);	// get first volref

	error = GamOpenDatabase();
	if (error) return (error);
	GamImportDatabase();

	AppStartCheckNotify(); // not fatal error if not avalaible
	AppStartCheckScreenSize();

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err AppStopCheckNotify()
{
	UInt32 romVersion;
	Err err;
	
	err = FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &romVersion); 
	if (!err) {
		UInt16 cardNo;
		LocalID dbID;

		err = SysCurAppDatabase(&cardNo, &dbID);
		if (!err) {
			SysNotifyUnregister(cardNo, dbID, sysNotifyVolumeUnmountedEvent, sysNotifyNormalPriority);
			SysNotifyUnregister(cardNo, dbID, sysNotifyVolumeMountedEvent, sysNotifyNormalPriority);
			// sonySysNotifyMsaEnforceOpenEvent
			SysNotifyUnregister(cardNo, dbID, sysNotifyDisplayResizedEvent, sysNotifyNormalPriority);
		}
	}
	
	return err;
}

void AppStop(void) {
	// Close all the open forms.
	FrmCloseAllForms();
	WinEraseWindow();
	WinPalette(winPaletteSetToDefault, 0, 256, NULL);

	// Close and move Game list database
	GamCloseDatabase(false);

	// Write the saved preferences / saved-state information.  This data 
	// will saved during a HotSync backup.
	SavePrefs();

	// stop all
	AppStopSilk();
	AppStopCheckNotify();
	AppStopMathLib();
	AppStopHRMode();

	// reset if needed
	if (gVars) {
		Boolean autoReset = gVars->autoReset;
		MemPtrFree(gVars);
		if (autoReset)
			SysReset();
	}
}
