/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
#include <PmPalmOSNVFS.h>
#include "StarterRsc.h"

#include "games.h"
#include "start.h"
#include "rumble.h"
#include "globals.h"
#include "features.h"
#include "formUtil.h"
#include "formCards.h"
#include "palmdefs.h"

#include "init_palmos.h"
#include "init_stuffs.h"

#include "modules.h"
#include "args.h"


#define BUILD_ERROR(m)	\
	{	StrCopy(msg, m);	\
		StrCat(msg, "\n\nPlease check that all required files are installed on your card, and you have enough free storage memory.");	\
		goto onError; }

#define BUILD_FILE(h,m)	\
	StrCopy(filename, "/Palm/Programs/ScummVM/Mods/");	\
	StrCat(filename, h);	\
	StrCat(filename, m);

#define FIND_FILE() \
	if (*volRefNum == vfsInvalidVolRef) \
		*volRefNum = ModFind(filename);


#define CHECK_FILE(m) \
	e = VFSFileOpen(*volRefNum, filename, vfsModeRead, &file);	\
	if (e)	\
		BUILD_ERROR(m)	\
	else	\
		VFSFileClose(file);

#define IMPRT_FILE(m) \
	e = VFSImportDatabaseFromFile(*volRefNum, filename, &cardNo, &dbID);	\
	if (e)	\
		BUILD_ERROR(m)

#define DELET_FILE(f) \
	del_dbID = DmFindDatabase(0, f);	\
	if (del_dbID)	\
		DmDeleteDatabase(0, del_dbID);

void ModDelete() {
	LocalID del_dbID;

	DELET_FILE("Glbs::Common");
	DELET_FILE("Glbs::Engine");
	DELET_FILE("ScummVM-Engine");
}

UInt16 ModFind(const Char *f) {
	Err e;
	UInt16 volRefNum;
	FileRef r;
	UInt32 volIterator = vfsIteratorStart|vfsIncludePrivateVolumes;
	while (volIterator != vfsIteratorStop) {
		e = VFSVolumeEnumerate(&volRefNum, &volIterator);

		if (!e)	e = VFSFileOpen(volRefNum, f, vfsModeRead, &r);
		if (!e) e = VFSFileClose(r);
		if (!e) break;
	}

	return volRefNum;
}

static void ModSetStack(UInt32 newSize, UInt16 cardNo, LocalID dbID) {
	DmOpenRef dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadWrite);

	if (dbRef) {
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

		DmCloseDatabase(dbRef);
	}
}

static Err ModImport(UInt16 *volRefNum, UInt8 engine, Boolean *armP) {
#ifndef _DEBUG_ENGINE
	char filename[256];
	UInt16 cardNo;
	LocalID dbID;
	UInt32 result;
	FileRef file;
#endif
	char msg[256];
	FormPtr ofmP, frmP;
	Err e = errNone;

	ofmP = FrmGetActiveForm();
	frmP = FrmInitForm(ImportForm);
	FrmSetActiveForm(frmP);
	FrmDrawForm(frmP);

	// In debug mode, the engine files are directly uploaded to the simulator
#ifndef _DEBUG_ENGINE
	// engine file ?
	BUILD_FILE(engines[engine].fileP, ".engine");
	FIND_FILE ();
	CHECK_FILE("ScummVM engine file was not found !");
	IMPRT_FILE("Cannot import engine file !");

	// need more files ?
	dbID = DmFindDatabase(0, "ScummVM-Engine");	// be sure to have the correct dbID
	e = SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCustomEngineGetInfo, 0, &result);
	*armP = ((result & GET_MODEARM) == GET_MODEARM);

/*	ARM ONLY FOR NOW, NOT REQUIRED
	// common file ?
	if (!e && (result & GET_DATACOMMON)) {
		BUILD_FILE("common", ".data");
		CHECK_FILE("Common data file was not found !");
		IMPRT_FILE("Cannot import common data file !");
	}
	// data file ?
	if (!e && (result & GET_DATAENGINE)) {
		BUILD_FILE(engines[engine].fileP, ".data");
		CHECK_FILE("Engine data file was not found !");
		IMPRT_FILE("Cannot import engine data file !");
	}
*/
#endif
	// if error, cleanup
	if (e) ModDelete();

onError:
	FrmEraseForm(frmP);
	FrmDeleteForm(frmP);
	if (e) {
		if (ofmP) FrmSetActiveForm(ofmP);
		FrmCustomAlert(FrmErrorAlert, msg, 0, 0);
	}

	return e;
}

#undef DELET_FILE
#undef CHECK_FILE
#undef BUILD_FILE

Boolean StartScummVM(Int16 engine) {
	Char **argvP;
	UInt8 lightspeed, argc	= 0;
	UInt32 stackSize;
	Boolean toLauncher, direct, isARM;
	Char num[6];

	argvP = ArgsInit();
	direct = false;

	// start command line (exec name)
	ArgsAdd(&argvP[argc], "-", NULL, &argc);
	// standard path
	ArgsAdd(&argvP[argc], "--themepath=", "/PALM/Programs/ScummVM/Themes", &argc);

	UInt16 index = GamGetSelected();
	// no game selected
	if (index == dmMaxRecordIndex) {
		if (engine == NO_ENGINE) {
			// free args
			ArgsFree(argvP);
			return false;
		}

		// default values
		if (bDirectMode)
			gPrefs->card.volRefNum = parseCards();	// always use the first removable card available (?)

		gVars->filter		= true;
		gVars->palmVolume	= 50;
		gVars->fmQuality	= FM_QUALITY_INI;
		direct = true;

	// somthing selected
	} else {
		Char pathP[256];
		MemHandle recordH;
		GameInfoType *gameInfoP;

		recordH = DmQueryRecord(gameDB,index);
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);
		engine = gameInfoP->engine;

		// build path
		StrCopy(pathP,"/Palm/Programs/ScummVM/Games/");
		if (gameInfoP->pathP[0] == '/')
			StrCopy(pathP, gameInfoP->pathP);
		else if (!(gameInfoP->pathP[0] == '.' && StrLen(gameInfoP->pathP) == 1))
			StrCat(pathP, gameInfoP->pathP);

		// path
		ArgsAdd(&argvP[argc], "-p", pathP, &argc);

		// language
		if (gameInfoP->language > 0) {
			const Char *lang = "zh\0cz\0gb\0en\0fr\0de\0hb\0it\0jp\0kr\0pl\0pt\0ru\0es\0se\0";
			ArgsAdd(&argvP[argc], "-q", (lang + (gameInfoP->language - 1) * 3), &argc);
		}

		// fullscreen ?
		if (gameInfoP->fullscreen)
			ArgsAdd(&argvP[argc], "-f", NULL, &argc);

		// aspect-ratio ?
		ArgsAdd(&argvP[argc], (gameInfoP->aspectRatio ? "--aspect-ratio" : "--no-aspect-ratio"), NULL, &argc);

		// gfx mode
		gVars->filter = gameInfoP->filter;

		switch (gameInfoP->renderMode) {
			case 1:
				ArgsAdd(&argvP[argc], "--render-mode=", "amiga", &argc);
				break;
			case 2:
				ArgsAdd(&argvP[argc], "--render-mode=", "cga", &argc);
				break;
			case 3:
				ArgsAdd(&argvP[argc], "--render-mode=", "ega", &argc);
				break;
			case 4:
				ArgsAdd(&argvP[argc], "--render-mode=", "hercAmber", &argc);
				break;
			case 5:
				ArgsAdd(&argvP[argc], "--render-mode=", "hercGreen", &argc);
				break;
		}

		switch (gameInfoP->gfxMode)	{
			case 1:
				ArgsAdd(&argvP[argc], "-g", "wide", &argc);
				break;
			default:
				ArgsAdd(&argvP[argc], "-g", "1x", &argc);
				break;
		}

		// load state
		if (gameInfoP->autoLoad) {
			StrIToA(num, gameInfoP->loadSlot);
			ArgsAdd(&argvP[argc], "-x", num, &argc);
		}
		// boot script parameter
		if (gameInfoP->bootParam) {
			StrIToA(num, gameInfoP->bootValue);
			ArgsAdd(&argvP[argc], "-b", num, &argc);
		}
		// not a PC version
		if (gameInfoP->setPlatform) {
			static const char *platform[] = {
				"3do",
				"acorn",
				"amiga",
				"2gs",
				"atari",
				"c64",
				"pc",
				"fmtowns",
				"linux",
				"mac",
				"nes",
				"segacd",
				"windows"
			};
			ArgsAdd(&argvP[argc], "--platform=", platform[gameInfoP->platform], &argc);
		}

		// subtitles
		if (gameInfoP->subtitles)
			ArgsAdd(&argvP[argc],  "-n", NULL, &argc);

		// multi midi ?
		if (gameInfoP->musicInfo.sound.multiMidi)
			ArgsAdd(&argvP[argc], "--multi-midi", NULL, &argc);

		if (engine == ENGINE_SCUMM) {
			// music tempo
			StrIToA(num, gameInfoP->musicInfo.sound.tempo);
			ArgsAdd(&argvP[argc], "--tempo=", num, &argc);
		}

		// talk speed
		if (gameInfoP->talkSpeed) {
			StrIToA(num, gameInfoP->talkValue);
			ArgsAdd(&argvP[argc], "--talkspeed=", num, &argc);
		}

		// music driver
		if (gameInfoP->musicInfo.sound.music) {
			static char *drv[] = {
				"auto",
				"null",
				"adlib",
				"towns",
				"pcjr",
				"native",
				"pcspk"
			};

			if (StrCompare(drv[gameInfoP->musicInfo.sound.drvMusic], "native") == 0) {
				if (OPTIONS_TST(kOptDeviceZodiac))
					ArgsAdd(&argvP[argc], "-e", "zodiac", &argc);	// Tapwave Zodiac
				else if (OPTIONS_TST(kOptSonyPa1LibAPI))
					ArgsAdd(&argvP[argc], "-e", "ypa1", &argc);		// Pa1Lib devices
				else
					ArgsAdd(&argvP[argc], "-e", "auto", &argc);		// no driver, switch to auto
			} else {
				ArgsAdd(&argvP[argc], "-e", drv[gameInfoP->musicInfo.sound.drvMusic], &argc);
			}

			// output rate
			UInt32 rates[] = {4000, 8000, 11025, 22050, 44100};
			StrIToA(num, rates[gameInfoP->musicInfo.sound.rate]);
			ArgsAdd(&argvP[argc], "--output-rate=", num, &argc);

			// FM quality
			gVars->fmQuality = gameInfoP->fmQuality;

		} else {
			ArgsAdd(&argvP[argc], "-e", "null", &argc);
		}

		// volume control
		StrIToA(num, gameInfoP->musicInfo.volume.sfx);
		ArgsAdd(&argvP[argc], "-s", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.music);
		ArgsAdd(&argvP[argc], "-m", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.speech);
		ArgsAdd(&argvP[argc], "-r", num, &argc);

		// game name
		ArgsAdd(&argvP[argc], gameInfoP->gameP, NULL, &argc);

		gVars->palmVolume = gameInfoP->musicInfo.sound.music ? gameInfoP->musicInfo.volume.palm : 0;

		MemHandleUnlock(recordH);
	} // end no game / game selected

	// common command line options

	// debug level
	if (gPrefs->debug) {
		StrIToA(num, gPrefs->debugLevel);
		ArgsAdd(&argvP[argc], "-d", num, &argc);
	}

	if (engine == ENGINE_QUEEN || engine == ENGINE_SKY) {
		// alternative intro ?
		if (gPrefs->altIntro)
			ArgsAdd(&argvP[argc], "--alt-intro", NULL, &argc);
	}

	if (engine == ENGINE_SCUMM) {
		// demo mode ?
		if (gPrefs->demoMode)
			ArgsAdd(&argvP[argc], "--demo-mode", NULL, &argc);
	}

	// copy protection ?
	if (gPrefs->copyProtection)
		ArgsAdd(&argvP[argc], "--copy-protection", NULL, &argc);

	// exceed max args ?
	if (argc > MAX_ARG)
		FrmCustomAlert(FrmErrorAlert, "Too many parameters.",0,0);

	// set some common options
	stackSize = (gPrefs->setStack ? STACK_LARGER : STACK_DEFAULT);
	lightspeed= (gPrefs->lightspeed.enable ? gPrefs->lightspeed.mode : 255);
	toLauncher= (gPrefs->exitLauncher);

	// gVars values
	// (gVars->HRrefNum defined in checkHRmode on Clie)
#ifndef _DEBUG_ENGINE
	gVars->VFS.volRefNum	= (gPrefs->card.autoDetect ? vfsInvalidVolRef : gPrefs->card.volRefNum);
#else
	gVars->VFS.volRefNum	= gPrefs->card.volRefNum;
#endif
	gVars->vibrator			= gPrefs->vibrator;
	gVars->stdPalette		= gPrefs->stdPalette;
	gVars->VFS.cacheSize	= (gPrefs->card.useCache ? gPrefs->card.cacheSize : 0);
	gVars->indicator.showLED= gPrefs->card.showLED;
	gVars->stylusClick		= gPrefs->stylusClick;
	gVars->autoSave			= (gPrefs->autoSave ? gPrefs->autoSavePeriod : -1);
	gVars->advancedMode		= gPrefs->advancedMode;
	gVars->arrowKeys		= gPrefs->arrowKeys;

	// user params
	HWR_RSTALL();

	if (gPrefs->goLCD)
		HWR_SET(INIT_GOLCD);
	else
		OPTIONS_RST(kOptGoLcdAPI);

	if (!gPrefs->autoOff)
		HWR_SET(INIT_AUTOOFF);

	if (gVars->vibrator)
		HWR_SET(INIT_VIBRATOR);
/* ????
	if (	musicDriver == 1 ||
			musicDriver == 3 ||
			musicDriver == 4 ||
			musicDriver == sysInvalidRefNum) {
		HWR_SET(INIT_PA1LIB);
	}
*/
	if (ModImport(&gVars->VFS.volRefNum, engine, &isARM) != errNone) {
		if (bDirectMode) {
			// and force exit if nothing selected
			EventType event;
			event.eType = keyDownEvent;
			event.data.keyDown.chr = vchrLaunch;
			event.data.keyDown.modifiers = commandKeyMask;
			EvtAddUniqueEventToQueue(&event, 0, true);
		}
		ArgsFree(argvP);
		return false;
	}

	// reset mode if screen rotation occurred (DIA only)
	if (!direct && OPTIONS_TST(kOptCollapsible)) {
		UInt8 mode = PalmScreenSize(0,0, &(gVars->screenFullWidth), &(gVars->screenFullHeight));
		OPTIONS_RST(kOptModeLandscape);
		OPTIONS_SET((mode == PALM_LANDSCAPE) ? kOptModeLandscape : kOptNone);
	}

	// free and save globals pref memory
	GamCloseDatabase(false);
	FrmCloseAllForms();
	SavePrefs();

	{
		UInt16 cardNo;
		UInt32 dbID;

		LaunchParamType *cmdPBP = (LaunchParamType *)MemPtrNew(sizeof(LaunchParamType));

		MemPtrSetOwner(cmdPBP, 0);
		MemPtrSetOwner(gVars, 0);
		ArgsSetOwner(argvP, 0);

		cardNo = 0;
		dbID = DmFindDatabase(0, "ScummVM-Engine");

		if (isARM)
			FtrSet(appFileCreator, ftrStack , (stackSize * 4));
		else
			ModSetStack(stackSize, cardNo, dbID);

		cmdPBP->args.argc = argc;
		cmdPBP->args.argv = argvP;
		cmdPBP->gVars = gVars;
		cmdPBP->lightspeed = lightspeed;
		cmdPBP->exitLauncher = toLauncher;

		SysUIAppSwitch(cardNo, dbID, sysAppLaunchCmdNormalLaunch, cmdPBP);
		bLaunched = true;
	}

	return false;
}
