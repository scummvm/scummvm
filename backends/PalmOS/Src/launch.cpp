#include <PalmOS.h>
#include "StarterRsc.h"

#include "games.h"
#include "start.h"
#include "rumble.h"
#include "extend.h"
#include "globals.h"
#include "features.h"
#include "formUtil.h"

#include "modules.h"
#include "args.h"

/*
static Boolean checkPath(const Char *pathP) {
	FILE *tmpRef;

	if (!(tmpRef = fopen(pathP, "r"))) {
		return false;
	} else {
		fclose(tmpRef);
		return true;
	}
}
*/

#define BUILD_FILE(h,m)	\
	StrCopy(filename, "/Palm/Programs/ScummVM/Mods/");	\
	StrCat(filename, h);	\
	StrCat(filename, m);

#define CHECK_FILE() \
	e = VFSFileOpen(volRefNum, filename, vfsModeRead, &file);	\
	if (e)	\
		goto onError;	\
	else	\
	VFSFileClose(file);

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

static Err ModImport(UInt16 volRefNum, UInt8 engine) {
	const Char *files[] = {
		{ "scumm" },
		{ "simon" },
		{ "queen" },
		{ "sword1" },
		{ "sky" }
	};

	char filename[256];
	UInt16 dum1;
	UInt32 dum2;
	FileRef file;
	FormPtr ofmP, frmP;
	Err e = errNone;

	ofmP = FrmGetActiveForm();
	frmP = FrmInitForm(ImportForm);
	FrmSetActiveForm(frmP);
	FrmDrawForm(frmP);

#ifndef _DEBUG_ENGINE
	// In debug mode, the engine files are directly uploaded to the simulator
	BUILD_FILE(files[engine], ".engine");	// engine file ?
	CHECK_FILE();
	BUILD_FILE(files[engine], ".data");		// data file ?
	CHECK_FILE();
	BUILD_FILE("common", ".data")			// common data ?
	CHECK_FILE();

	BUILD_FILE("common", ".data");
	e = (e) ? e : VFSImportDatabaseFromFile(volRefNum, filename, &dum1, &dum2);
	BUILD_FILE(files[engine], ".data");
	e = (e) ? e : VFSImportDatabaseFromFile(volRefNum, filename, &dum1, &dum2);
	BUILD_FILE(files[engine], ".engine");
	e = (e) ? e : VFSImportDatabaseFromFile(volRefNum, filename, &dum1, &dum2);
#endif
	// if error, cleanup
	if (e) ModDelete();

onError:
	FrmEraseForm(frmP);
	FrmDeleteForm(frmP);
	if (e && ofmP) FrmSetActiveForm(ofmP);

	return e;
}

#undef DELET_FILE
#undef CHECK_FILE
#undef BUILD_FILE

Boolean StartScummVM() {
	Char **argvP;
	UInt8 lightspeed, argc	= 0;
	UInt32 stackSize;
	Boolean toLauncher;
	UInt8 engine;

	UInt16 musicDriver = sysInvalidRefNum; // for launch call
	UInt16 index = GamGetSelected();

	Char num[6];

	argvP = ArgsInit();

	if (index == dmMaxRecordIndex) {
		UInt16 whichButton;
		FormPtr frmP = FrmInitForm(EngineForm);
		whichButton = FrmDoDialog(frmP);
		FrmDeleteForm(frmP);

		if (whichButton == EngineCancelButton)
			return false;

		engine = (whichButton - Engine0Button);

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
			StrCopy(pathP,gameInfoP->pathP);
		else
			StrCat(pathP,gameInfoP->pathP);

/*		// path exists ?
		if (!checkPath(pathP)) {
			MemHandleUnlock(recordH);
			FrmCustomAlert(FrmErrorAlert,"The specified path was not found !",0,0);
			ArgsFree(argvP);
			return false;
		}
*/

		// ScummVM
		ArgsAdd(&argvP[argc], "-", NULL, &argc);

		// path
		ArgsAdd(&argvP[argc], "-p", pathP, &argc);

		// language
		if (gameInfoP->language > 0) {
			const Char *lang = "en\0de\0fr\0it\0pt\0es\0jp\0zh\0kr\0hb\0ru\0cz\0nl\0";
			ArgsAdd(&argvP[argc], "-q", (lang + (gameInfoP->language - 1) * 3), &argc);
		}

		// fullscreen ?
		if (gameInfoP->fullscreen)
			ArgsAdd(&argvP[argc], "-f", NULL, &argc);

		// aspect-ratio ?
		ArgsAdd(&argvP[argc], (gameInfoP->aspectRatio ? "--aspect-ratio" : "--no-aspect-ratio"), NULL, &argc);

		// gfx mode
		gVars->flipping.pageAddr1 = (UInt8 *)(BmpGetBits(WinGetBitmap(WinGetDisplayWindow())));
		gVars->flipping.pageAddr2 = gVars->flipping.pageAddr1; // default if not flipping mode
		gVars->filter = gameInfoP->filter;

		switch (gameInfoP->gfxMode)	{
			case 1:
				ArgsAdd(&argvP[argc], "-g", "flipping", &argc);
				gVars->flipping.pageAddr1 = (UInt8 *)WinScreenLock(winLockDontCare);
				WinScreenUnlock();
				break;
			case 2:
				ArgsAdd(&argvP[argc], "-g", "buffered", &argc);
				break;
			case 3:
				ArgsAdd(&argvP[argc], "-g", "wide", &argc);
				gVars->flipping.pageAddr1 = (UInt8 *)WinScreenLock(winLockDontCare);
				WinScreenUnlock();
				break;
			default:
				ArgsAdd(&argvP[argc], "-g", "normal", &argc);
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
			switch (gameInfoP->platform) {
				case 0:
					ArgsAdd(&argvP[argc], "--platform=", "amiga", &argc);
					break;
				case 1:
					ArgsAdd(&argvP[argc], "--platform=", "atari", &argc);
					break;
				case 2:
					ArgsAdd(&argvP[argc], "--platform=", "mac", &argc);
					break;
				case 3:
					ArgsAdd(&argvP[argc], "--platform=", "pc", &argc);
					break;
				case 4:
					ArgsAdd(&argvP[argc], "--platform=", "fmtowns", &argc);
					break;
				case 5:
					ArgsAdd(&argvP[argc], "--platform=", "windows", &argc);
					break;
			}
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
			// talk speed
			if (gameInfoP->talkSpeed) {
				StrIToA(num, gameInfoP->talkValue);
				ArgsAdd(&argvP[argc], "--talkspeed=", num, &argc);
			}
		}

		// music driver
		musicDriver =gameInfoP->musicInfo.sound.music;
		if (musicDriver) {
			switch (gameInfoP->musicInfo.sound.drvMusic) {
				case 0:	// NULL
					ArgsAdd(&argvP[argc], "-e", "null", &argc);
					break;

				case 1:	// built-in MIDI
					if (OPTIONS_TST(kOptDeviceZodiac))
						ArgsAdd(&argvP[argc], "-e", "zodiac", &argc);	// Tapwave Zodiac
					else if (OPTIONS_TST(kOptSonyPa1LibAPI))
						ArgsAdd(&argvP[argc], "-e", "ypa1", &argc);		// Pa1Lib devices
					else
						ArgsAdd(&argvP[argc], "-e", "null", &argc);		// error, no music driver
					break;

				case 2: // PC Speaker
					ArgsAdd(&argvP[argc], "-e", "pcspk", &argc);
					break;
				case 3: // IBM PCjr
					ArgsAdd(&argvP[argc], "-e", "pcjr", &argc);
				case 4: // FM Towns
					ArgsAdd(&argvP[argc], "-e", "towns", &argc);
				case 5: // AdLib
					ArgsAdd(&argvP[argc], "-e", "adlib", &argc);
			}
		}
		else	// NULL as default
			ArgsAdd(&argvP[argc], "-e", "null", &argc);

		// volume control
		StrIToA(num, gameInfoP->musicInfo.volume.master);
		ArgsAdd(&argvP[argc], "-o", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.sfx);
		ArgsAdd(&argvP[argc], "-s", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.music);
		ArgsAdd(&argvP[argc], "-m", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.speech);
		ArgsAdd(&argvP[argc], "-r", num, &argc);

		// output rate
		if (gameInfoP->musicInfo.sound.sfx) {
			UInt32 rates[] = {8000, 11025, 22050};
			StrIToA(num, rates[gameInfoP->musicInfo.sound.rate]);
			ArgsAdd(&argvP[argc], "--output-rate=", num, &argc);
		}

		// game name
		ArgsAdd(&argvP[argc], gameInfoP->gameP, NULL, &argc);

		// use sound
		if (!gameInfoP->musicInfo.sound.sfx) {
			OPTIONS_RST(kOptSonyPa1LibAPI);
			OPTIONS_RST(kOptPalmSoundAPI);
		}

		// others globals data
		gVars->CD.enable = gameInfoP->musicInfo.sound.CD;
		gVars->CD.driver = gameInfoP->musicInfo.sound.drvCD;
		gVars->CD.format = gameInfoP->musicInfo.sound.frtCD;
		gVars->CD.volume = gameInfoP->musicInfo.volume.audiocd;
		gVars->CD.defaultTrackLength = gameInfoP->musicInfo.sound.defaultTrackLength;
		gVars->CD.firstTrack = gameInfoP->musicInfo.sound.firstTrack;

		MemHandleUnlock(recordH);
	}

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
		// copy protection ?
		if (gPrefs->copyProtection)
			ArgsAdd(&argvP[argc], "--copy-protection", NULL, &argc);
		// demo mode ?
		if (gPrefs->demoMode)
			ArgsAdd(&argvP[argc], "--demo-mode", NULL, &argc);
	}

	if (argc > MAX_ARG)
		FrmCustomAlert(FrmErrorAlert, "Too many parameters.",0,0);

	stackSize = (gPrefs->setStack ? STACK_LARGER : STACK_DEFAULT);
	lightspeed= (gPrefs->lightspeed.enable ? gPrefs->lightspeed.mode : 255);
	toLauncher= (gPrefs->exitLauncher);

	// gVars values
	// (gVars->HRrefNum defined in checkHRmode on Clié)
	gVars->screenLocked	= false;
	gVars->volRefNum	= gPrefs->card.volRefNum;
	gVars->vibrator		= gPrefs->vibrator;
	gVars->stdPalette	= gPrefs->stdPalette;

	// user params
	HWR_RSTALL();

	if (!gPrefs->autoOff)
		HWR_SET(INIT_AUTOOFF);

	if (!gPrefs->arm) {
		OPTIONS_RST(kOptDeviceARM);
		OPTIONS_RST(kOptDeviceProcX86);
	} else {
		HWR_SET(INIT_ARM);
	}

	if (gVars->vibrator)
		HWR_SET(INIT_VIBRATOR);

	if (	musicDriver == 1 ||
			musicDriver == 3 ||
			musicDriver == 4 ||
			musicDriver == sysInvalidRefNum) {
		HWR_SET(INIT_PA1LIB);
	}

	if (ModImport(gVars->volRefNum, engine) != errNone) {
		FrmCustomAlert(FrmErrorAlert, "Error importing files:\nplease check that all required files are installed on the selected memory card, and you have enough free storage memory.", 0, 0);
		ArgsFree(argvP);
		return false;
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
