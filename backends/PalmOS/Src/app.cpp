/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 * $URL$
 * $Id$
 *
 */

#include <PalmOS.h>
#include <SonyClie.h>

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
#include "init_stuffs.h"

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

	if (gVars->HRrefNum == sysInvalidRefNum) {
		if (e = PalmHRInit(depth))
			FrmCustomAlert(FrmErrorAlert,"Your device doesn't seem to support Hi-Res or 256color mode.",0,0);
	} else {
		OPTIONS_SET(kOptDeviceClie);
	}

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

		// check if the DB still exists
		DmSearchStateType state;
		UInt16 cardNo;
		LocalID dbID;
		Boolean found = false;
		err = DmGetNextDatabaseByTypeCreator(true, &state, 'skin', appFileCreator, false, &cardNo, &dbID);
		while (!err && dbID && !found) {
			found = (cardNo == gPrefs->skin.cardNo && dbID == gPrefs->skin.dbID);
			err = DmGetNextDatabaseByTypeCreator(false, &state, 'skin', appFileCreator, false, &cardNo, &dbID);
		}

		if (found) {
			// remember to check version for next revision of the skin
			err = DmDatabaseInfo (gPrefs->skin.cardNo, gPrefs->skin.dbID, gPrefs->skin.nameP, 0, 0, 0, 0, 0, 0, 0,0, &type, &creator);
			if (!err)
				if (type != 'skin' || creator != appFileCreator)
					err = dmErrInvalidParam;
		}
		
		if (!found || err)
			MemSet(&(gPrefs->skin),sizeof(SkinInfoType),0);
	}

	// No skin ? try to get the first one
	if (!gPrefs->skin.dbID) {
		DmSearchStateType stateInfo;

		err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, 'skin', appFileCreator, false, &gPrefs->skin.cardNo, &gPrefs->skin.dbID);
		if (!err)
			err = DmDatabaseInfo (gPrefs->skin.cardNo, gPrefs->skin.dbID, gPrefs->skin.nameP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	
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

static void AppStartCheckScreenSize() {
	Coord sw, sh, fw, fh;
	UInt8 mode;

	OPTIONS_RST(kOptCollapsible);
	OPTIONS_RST(kOptModeWide);
	OPTIONS_RST(kOptModeLandscape);

	// we are on a sony device
	if (OPTIONS_TST(kOptDeviceClie)) {
		mode = SonyScreenSize(gVars->HRrefNum, &sw, &sh, &fw, &fh);
		if (mode) {
			OPTIONS_SET(kOptModeWide);
			OPTIONS_SET((mode == SONY_LANDSCAPE) ? kOptModeLandscape : kOptNone);
		}
	} else {
		mode = PalmScreenSize(&sw, &sh, &fw, &fh);
		if (mode) {
			OPTIONS_SET(kOptCollapsible);
			OPTIONS_SET(kOptModeWide);
			OPTIONS_SET((mode == PALM_LANDSCAPE) ? kOptModeLandscape : kOptNone);
		}
	}

	gVars->screenWidth = sw;
	gVars->screenHeight = sh;

	gVars->screenFullWidth = fw;
	gVars->screenFullHeight = fh;
}

#define max(id,value)	gVars->memory[id] = (gVars->memory[id] < value ? value : gVars->memory[id])
#define min(id,value)	gVars->memory[id] = (gVars->memory[id] > value ? value : gVars->memory[id])
#define threshold 		700

static void AppStartSetMemory() {
	UInt32 mem, def;
	PalmGetMemory(0,0,0,&mem);
	def = (mem > threshold) ? (mem - threshold) * 1024 : 0;
	gVars->startupMemory = mem;

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
	gVars->VFS.volRefNum = vfsInvalidVolRef;
	gVars->slkRefNum = sysInvalidRefNum;
	gVars->options = kOptNone;

	// set memory required by the differents engines
	AppStartSetMemory();
	StuffsGetFeatures();

	// allocate prefs space
	dataSize = sizeof(GlobalsPreferenceType);
	gPrefs	= (GlobalsPreferenceType *)MemPtrNew(dataSize);
	MemSet(gPrefs, dataSize, 0);

	// Read the saved preferences / saved-state information.
	if (PrefGetAppPreferences(appFileCreator, appPrefID, NULL, &checkSize, true) == noPreferenceFound || checkSize != dataSize) {
		// reset all elements
		MemSet(gPrefs, dataSize, 0);

		gPrefs->card.volRefNum = vfsInvalidVolRef;
		gPrefs->card.cacheSize = 4096;
		gPrefs->card.useCache = true;
		gPrefs->card.showLED = true;

		gPrefs->autoOff = true;
		gPrefs->vibrator = RumbleExists();
		gPrefs->debug = false;
		gPrefs->exitLauncher = true;
		gPrefs->stdPalette = OPTIONS_TST(kOptDeviceOS5);
		gPrefs->stylusClick = true;
		
	} else {
		PrefGetAppPreferences(appFileCreator, appPrefID, gPrefs, &dataSize, true);
	}

	if (!OPTIONS_TST(kOptDeviceARM)) {
		error = AppStartCheckMathLib();
		if (error) return (error);
	}

	error = AppStartCheckHRmode();
	if (error) return (error);
	
	bDirectMode = (AppStartLoadSkin() != errNone);

	// if volref previously defined, check if it's a valid one
	if (gPrefs->card.volRefNum != vfsInvalidVolRef) {
		VolumeInfoType volInfo;
		Err err = VFSVolumeInfo(gPrefs->card.volRefNum, &volInfo);
		if (err)
			gPrefs->card.volRefNum = parseCards();
	}
	else
		gPrefs->card.volRefNum = parseCards();
	if (gPrefs->card.volRefNum != vfsInvalidVolRef)
		CardSlotCreateDirs();

	// open games database
	error = GamOpenDatabase();
	if (error) return (error);
	GamImportDatabase();

	AppStartCheckScreenSize();
	AppStartCheckNotify(); 		// not fatal error if not avalaible

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
	AppStopCheckNotify();
	if (!OPTIONS_TST(kOptDeviceARM))
		AppStopMathLib();
	AppStopHRMode();

	if (!bLaunched)
		MemPtrFree(gVars);
}
