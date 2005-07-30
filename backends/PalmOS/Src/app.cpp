#include <PalmOS.h>
#include <SonyClie.h>
#include <PalmNavigator.h>
#include "Pa1Lib.h"

#include "StarterRsc.h"
#include "palmdefs.h"
#include "start.h"
#include "globals.h"
#include "rumble.h"

#include "mathlib.h"
#include "formCards.h"
#include "games.h"
#include "extend.h"

#include "modules.h"
#include "init_mathlib.h"
#include "init_sony.h"
#include "init_palmos.h"

#ifndef DISABLE_TAPWAVE
#define __TWKEYS_H__
#include "tapwave.h"
#endif

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
	Err e = errNone;
	UInt32 depth = (OPTIONS_TST(kOptMode16Bit) && OPTIONS_TST(kOptDeviceOS5)) ? 16 : 8;

	// try to init Sony HR mode then Palm HR mode
	gVars->HRrefNum = SonyHRInit(depth);

	if (gVars->HRrefNum == sysInvalidRefNum)
		if (e = PalmHRInit(depth))
			FrmCustomAlert(FrmErrorAlert,"Your device doesn't seem to support Hi-Res or 256color mode.",0,0);

	return e;
}

static void AppStopHRMode() {
	if (gVars->HRrefNum != sysInvalidRefNum)
		SonyHRRelease(gVars->HRrefNum);
	else
		PalmHRRelease();
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
		FrmCustomAlert(FrmWarnAlert,"No skin found.\nScummVM will start in direct mode.",0,0);
		//FrmCustomAlert(FrmErrorAlert,"No skin found.\nPlease install a skin and restart ScummVM.",0,0);

	return err;
}

static Err AppStartCheckMathLib() {
	Err e = MathlibInit();

	switch (e) {
		case errNone:
			break;
		case sysErrLibNotFound:
			FrmCustomAlert(FrmErrorAlert,"Can't find MathLib !",0,0);
			break;
		default:
			FrmCustomAlert(FrmErrorAlert,"Can't open MathLib !",0,0);
			break;
	}

	return e;
}

static void AppStopMathLib() {
	MathlibRelease();
}

// Set the screen pitch for direct screen access
// available only before a game start
void WinScreenGetPitch() {
	if (OPTIONS_TST(kOptModeHiDensity)) {
		WinScreenGetAttribute(winScreenRowBytes, &(gVars->screenPitch));
		if (OPTIONS_TST(kOptMode16Bit))
			gVars->screenPitch /= 2;	// this value is used only in-game and in 8bit mode, so if we are in 16Bit 8bit = 16bit/2

		// FIXME : hack for TT3 simulator (and real ?) return 28 on landscape mode
		if (gVars->screenPitch < gVars->screenFullWidth)
			gVars->screenPitch = gVars->screenFullWidth;
	} else {
		gVars->screenPitch = gVars->screenFullWidth;
	}
}

void PINGetScreenDimensions() {
	UInt32 ftr;
	// if feature set, not set on Garmin iQue3600 ???
	if (!(FtrGet(sysFtrCreator, sysFtrNumInputAreaFlags, &ftr))) {
		if (ftr & grfFtrInputAreaFlagCollapsible) {

			Coord x, y;
			UInt16 curOrientation = SysGetOrientation();

			OPTIONS_SET(kOptCollapsible);
			// reset previous options if any
			OPTIONS_RST(kOptModeWide);
			OPTIONS_RST(kOptModeLandscape);

			PINSetInputTriggerState(pinInputTriggerEnabled);
			PINSetInputAreaState(pinInputAreaClosed);
			StatHide();

			WinGetDisplayExtent(&x, &y);
			gVars->screenFullWidth = x << 1;
			gVars->screenFullHeight = y << 1;

			OPTIONS_SET(kOptModeWide);

			if (curOrientation == sysOrientationLandscape ||
				curOrientation == sysOrientationReverseLandscape
					)
				OPTIONS_SET(kOptModeLandscape);

			StatShow();
			PINSetInputAreaState(pinInputAreaOpen);
			PINSetInputTriggerState(pinInputTriggerDisabled);
		}
	}
}

static void AppStartCheckScreenSize() {
	UInt32 version;
	UInt16 slkRefNum;

	gVars->screenWidth = 320;
	gVars->screenHeight = 320;

	gVars->screenFullWidth = gVars->screenWidth;
	gVars->screenFullHeight = gVars->screenHeight;

	// Sony HiRes+
	slkRefNum = SilkInit(&version);
	gVars->slkRefNum = slkRefNum;
	gVars->slkVersion = version;
	if (slkRefNum != sysInvalidRefNum) {
		if (version == vskVersionNum1) {
			SilkLibEnableResize(slkRefNum);
			SilkLibResizeDispWin(slkRefNum, silkResizeMax);
			HRWinGetWindowExtent(gVars->HRrefNum, &gVars->screenFullWidth, &gVars->screenFullHeight);
			SilkLibResizeDispWin(slkRefNum, silkResizeNormal);
			SilkLibDisableResize(slkRefNum);

		} else {
			VskSetState(slkRefNum, vskStateEnable, (gVars->slkVersion == vskVersionNum2 ? vskResizeVertically : vskResizeHorizontally));
			VskSetState(slkRefNum, vskStateResize, vskResizeNone);
			HRWinGetWindowExtent(gVars->HRrefNum, &gVars->screenFullWidth, &gVars->screenFullHeight);
			VskSetState(slkRefNum, vskStateResize, vskResizeMax);
			VskSetState(slkRefNum, vskStateEnable, vskResizeDisable);
			OPTIONS_SET((version == vskVersionNum3 ? kOptModeLandscape : kOptNone));
		}

		OPTIONS_SET(kOptModeWide);

	// Tapwave Zodiac and other DIA API compatible devices
	// get max screen size
	} else {
		PINGetScreenDimensions();
	}

	WinScreenGetPitch();
}

static void AppStopSilk() {
	if (gVars->slkRefNum != sysInvalidRefNum)
		SilkLibClose(gVars->slkRefNum);
}

#define max(id,value)	gVars->memory[id] = (gVars->memory[id] < value ? value : gVars->memory[id])
#define min(id,value)	gVars->memory[id] = (gVars->memory[id] > value ? value : gVars->memory[id])
#define threshold 		700

static void AppStartSetMemory() {
	UInt32 mem, def;
	GetMemory(0,0,0,&mem);
	def = (mem > threshold) ? (mem - threshold) * 1024 : 0;

	// default values
	gVars->memory[kMemScummOldCostGames] = (mem >= 550 + threshold) ? 550000 : def;
	gVars->memory[kMemScummNewCostGames] = (mem >= 2500 + threshold) ? 2500000 : def;
	gVars->memory[kMemSimon1Games] = (mem >= 1000 + threshold) ? 1000000 : def;
	gVars->memory[kMemSimon2Games] = (mem >= 2000 + threshold) ? 2000000 : def;

	// set min required values
	max(kMemScummOldCostGames, 450000);
	max(kMemScummNewCostGames, 450000);
	max(kMemSimon1Games, 500000);
	max(kMemSimon2Games, 500000);

	// set max required values
	min(kMemScummOldCostGames, 550000);
	min(kMemScummNewCostGames, 2500000);
	min(kMemSimon1Games, 1000000);
	min(kMemSimon2Games, 2000000);

}

#undef threshold
#undef min
#undef max

Err AppStart(void) {
	UInt16 dataSize, checkSize = 0;
	UInt32 ulProcessorType, manufacturer, version, depth;
	Boolean color;
	Err error;

#ifndef _DEBUG_ENGINE
	// delete old databases
 	ModDelete();
#endif

	// allocate global variables space
	dataSize = sizeof(GlobalsDataType);
	gVars = (GlobalsDataType *)MemPtrNew(dataSize);
	MemSet(gVars, dataSize, 0);

	gVars->indicator.on	= 255;
	gVars->indicator.off = 0;
	gVars->HRrefNum = sysInvalidRefNum;
	gVars->volRefNum = sysInvalidRefNum;
	gVars->slkRefNum = sysInvalidRefNum;
	gVars->options = kOptNone;

#ifndef DISABLE_TAPWAVE
	// Tapwave Zodiac libs ?
	if (!FtrGet(sysFileCSystem, sysFtrNumOEMCompanyID, &manufacturer))
		if (manufacturer == twCreatorID)
			OPTIONS_SET(kOptDeviceZodiac);
#endif

	// Hi-Density present ?
	if (!FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version))
		if (version >= 4)
			OPTIONS_SET(kOptModeHiDensity);

	// OS5 ?
	if (!FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version))
		if (version >= kOS5Version)
			OPTIONS_SET(kOptDeviceOS5);

	// ARM ?
 	if (!FtrGet(sysFileCSystem, sysFtrNumProcessorID, &ulProcessorType))
 		if (sysFtrNumProcessorIsARM(ulProcessorType))
 			OPTIONS_SET(kOptDeviceARM);
 		else if (ulProcessorType == sysFtrNumProcessorx86)
 			OPTIONS_SET(kOptDeviceProcX86);

	// 5Way Navigator
	if (!FtrGet(navFtrCreator, navFtrVersion, &version))
		if (version >= 1)
	 		OPTIONS_SET(kOpt5WayNavigator);

	// Palm Sound API ?
	if (!FtrGet(sysFileCSoundMgr, sndFtrIDVersion, &version))
		if (version >= 1)
			OPTIONS_SET(kOptPalmSoundAPI);

	// Sony Pa1 Sound API
	if (Pa1Lib_Open()) {
		OPTIONS_SET(kOptSonyPa1LibAPI);
		Pa1Lib_Close();
	}

	// check for 16bit mode
	if (!WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, &color))
		OPTIONS_SET(((depth & 0x8000) ? kOptMode16Bit : kOptNone));

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
		gPrefs->vibrator = RumbleExists();
		gPrefs->debug = false;
		gPrefs->exitLauncher = true;
		gPrefs->stdPalette = OPTIONS_TST(kOptDeviceOS5);

	} else {
		PrefGetAppPreferences(appFileCreator, appPrefID, gPrefs, &dataSize, true);
	}

	error = AppStartCheckMathLib();
	if (error) return (error);

	error = AppStartCheckHRmode();
	if (error) return (error);

//	error = AppStartLoadSkin();
//	if (error) return (error);
	bDirectMode = (AppStartLoadSkin() != errNone);

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

	AppStartCheckScreenSize();
	AppStartCheckNotify(); 		// not fatal error if not available
	AppStartSetMemory();		// set memory required by the different engines

	// force ARM option if bDirectMode
	if (!error)
		if (bDirectMode && OPTIONS_TST(kOptDeviceARM))
			gPrefs->arm = true;

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

	if (!bLaunched)
		MemPtrFree(gVars);
}
