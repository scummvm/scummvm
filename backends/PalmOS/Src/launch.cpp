#include <PalmOS.h>
#include <Sonyclie.h>
#include "StarterRsc.h"
#include "stdio.h"
#include "games.h"
#include "start.h"
#include "vibrate.h"
#include "pa1lib.h"
#include "extend.h"

// need to move this on a .h file
#define sonySysFileCSystem      'SsYs'  /* Sony overall System */
#define sonySysFtrCreatorSystem     sonySysFileCSystem

#define sonySysFtrNumSystemBase         10000
#define sonySysFtrNumSystemAOutSndStateOnHandlerP   (sonySysFtrNumSystemBase + 4)
#define sonySysFtrNumSystemAOutSndStateOffHandlerP  (sonySysFtrNumSystemBase + 5)

typedef void (*sndStateOnType)(UInt8 /* kind */, UInt8 /* L volume 0-31 */, UInt8 /* R volume 0-31 */);
typedef void (*sndStateOffType)(UInt8 /* kind */);

/* kind */
#define aOutSndKindSp       (0) /* Speaker volume */
#define aOutSndKindHp       (2) /* HeadPhone volume */
////////////////////////////////////////////////////////////
#define MAX_ARG	25

static Boolean checkPath(const Char *pathP) {
	FileRef *tmpRef;

	if (!(tmpRef = fopen(pathP, "r"))) {
		return false;
	} else {
		fclose(tmpRef);
		return true;
	}
}

static void AddArg(Char **argvP, const Char *argP, const Char *parmP, UInt8 *countArgP)
{
	if (argP)
	{
		UInt16 len2 = 0;
		UInt16 len1 = StrLen(argP);

		if (len1 > 0)
		{	
			if (parmP)
				len2 = StrLen(parmP);

			(*countArgP)++;
			*argvP = (Char *)MemPtrNew(len1 + len2 + 1);
			StrCopy(*argvP, argP);
			
			if (parmP)
				StrCat(*argvP, parmP);
		}
	}
}

Boolean StartScummVM() {
	Char *argvP[MAX_ARG];
	UInt8 argc	= 0;
	UInt8 count;

	Boolean autoOff;
	UInt16 autoOffDelay;
	Boolean debug;
	UInt16 musicDriver = sysInvalidRefNum; // for launch call

	UInt16 index = GamGetSelected();

	for(count = 0; count < MAX_ARG; count++)
		argvP[count] = NULL;

	if (index != dmMaxRecordIndex) {
		Char pathP[256];
		Char num[4];
		MemHandle recordH;
		GameInfoType *gameInfoP;

		recordH = DmQueryRecord(gameDB,index);
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);

		// build path
		StrCopy(pathP,"/Palm/Programs/ScummVM/Games/");
		if (gameInfoP->pathP[0] == '/')
			StrCopy(pathP,gameInfoP->pathP);
		else
			StrCat(pathP,gameInfoP->pathP);
		
		// path exists ?
		if (!checkPath(pathP)) {
			MemHandleUnlock(recordH);
			FrmCustomAlert(FrmErrorAlert,"The specified path was not found !",0,0);
			return false;
		}

		// ScummVM
		AddArg(&argvP[argc], "-", NULL, &argc);

		// path
		AddArg(&argvP[argc], "-p", pathP, &argc);

		// language
		if (gameInfoP->language > 0) {
			const Char *lang = "en\0de\0fr\0it\0p\0es\0jp\0z\0kr\0hb\0ru\0";
			AddArg(&argvP[argc], "-q", (lang + (gameInfoP->language - 1) * 3), &argc);
		}

		// fullscreen ?
		if (gPrefs->fullscreen) {
			AddArg(&argvP[argc], "-f", NULL, &argc);
		}

		// gfx mode
		switch (gameInfoP->gfxMode)
		{
			case 1:
				AddArg(&argvP[argc], "-g", "flipping", &argc);
				break;
			case 2:
				AddArg(&argvP[argc], "-g", "dbuffer", &argc);
				break;
			case 3:
				AddArg(&argvP[argc], "-g", "wide", &argc);
				break;
			default:
				AddArg(&argvP[argc], "-g", "normal", &argc);
				break;
		}

		// load state
		if (gameInfoP->autoLoad) {
			StrIToA(num, gameInfoP->loadSlot);
			AddArg(&argvP[argc], "-x", num, &argc);
		}
		// boot script parameter
		if (gameInfoP->bootParam) {
			StrIToA(num, gameInfoP->bootValue);
			AddArg(&argvP[argc], "-b", num, &argc);
		}
		// not a PC version
		if (gameInfoP->setPlatform) {
			switch (gPrefs->sound.driver) {
				case 0:
					AddArg(&argvP[argc], "--platform=", "amiga", &argc);
					break;
				case 1:
					AddArg(&argvP[argc], "--platform=", "atari", &argc);
					break;
				case 2:
					AddArg(&argvP[argc], "--platform=", "mac", &argc);
					break;
				case 3:
					AddArg(&argvP[argc], "--platform=", "pc", &argc);
					break;
			}		
		}

		// subtitles
		if (!gameInfoP->subtitles) {
			AddArg(&argvP[argc], "-n", NULL, &argc);
		}
		// talk speed
		if (gameInfoP->talkSpeed) {
			StrIToA(num, gameInfoP->talkValue);
			AddArg(&argvP[argc], "--talkspeed=", num, &argc);
		}
		// debug level
		debug = gPrefs->debug;
		if (gPrefs->debug) {
			StrIToA(num, gPrefs->debugLevel);
			AddArg(&argvP[argc], "-d", num, &argc);
		}
		// demo mode ?
		if (gPrefs->demoMode)
			AddArg(&argvP[argc], "--demo-mode", NULL, &argc);

		// multi midi ?
		if (gPrefs->sound.multiMidi)
			AddArg(&argvP[argc], "--multi-midi", NULL, &argc);

		// music driver
		musicDriver = gPrefs->sound.music;
		if (musicDriver) {
			switch (gPrefs->sound.driver) {
				case 0:	// NULL
					AddArg(&argvP[argc], "-e", "null", &argc);
					break;
				case 1:	// yamaha Pa1
					AddArg(&argvP[argc], "-e", "ypa1", &argc);
					break;
			}		
		}
		else	// NULL as default
			AddArg(&argvP[argc], "-e", "null", &argc);

		// music tempo
		StrIToA(num, gPrefs->sound.tempo);
		AddArg(&argvP[argc], "--tempo=", num, &argc);

		// volume control
		StrIToA(num, gPrefs->volume.master);
		AddArg(&argvP[argc], "-o", num, &argc);
		StrIToA(num, gPrefs->volume.sfx);
		AddArg(&argvP[argc], "-s", num, &argc);
		StrIToA(num, gPrefs->volume.music);
		AddArg(&argvP[argc], "-m", num, &argc);

		// game name
		AddArg(&argvP[argc], gameInfoP->gameP, NULL, &argc);

		MemHandleUnlock(recordH);
	}

	gVars->skinSet = false;
	gVars->pinUpdate = false;
	GamCloseDatabase(false);
	FrmCloseAllForms();

	autoOff = gPrefs->autoOff;
	if (!autoOff) {
		autoOffDelay = SysSetAutoOffTime(0);
		EvtResetAutoOffTimer();
	}

	// gVars values
	//gVars->HRrefNum defined in checkHRmode on Clié OS4
	//gVars->logFile defined bellow, must be defined only if debug option is checked
	gVars->screenLocked = false;
	gVars->volRefNum = gPrefs->card.volRefNum;
	gVars->vibrator = gPrefs->vibrator;
	gVars->stdPalette = gPrefs->stdPalette;
	gVars->autoReset = gPrefs->autoReset;
	gVars->music.MP3 = gPrefs->sound.MP3;
	gVars->music.setDefaultTrackLength = gPrefs->sound.setDefaultTrackLength;
	gVars->music.defaultTrackLength = gPrefs->sound.defaultTrackLength;
	gVars->music.firstTrack = gPrefs->sound.firstTrack;

	// TODO : support tapwave rumble
	if (gVars->vibrator)
	{
		if (CheckVibratorExists()) {
			UInt16 cycle	= (SysTicksPerSecond())/2;
			UInt32 pattern	= 0xFF000000;
			UInt16 delay	= 1;
			UInt16 repeat	= 1;

			HwrVibrateAttributes(1, kHwrVibrateRate, &cycle);
			HwrVibrateAttributes(1, kHwrVibratePattern, &pattern);
			HwrVibrateAttributes(1, kHwrVibrateDelay, &delay);
			HwrVibrateAttributes(1, kHwrVibrateRepeatCount, &repeat);

		} else {
			gVars->vibrator = false;
		}
	}
	
	if (debug) {
		VFSFileDelete(gVars->volRefNum,"PALM/Programs/ScummVM/scumm.log");
		VFSFileCreate(gVars->volRefNum,"PALM/Programs/ScummVM/scumm.log");
		VFSFileOpen(gVars->volRefNum,"PALM/Programs/ScummVM/scumm.log",vfsModeWrite, &gVars->logFile);
	}

	void *sndStateOnFuncP = NULL,
		 *sndStateOffFuncP = NULL;

	if (musicDriver == 1 || musicDriver == sysInvalidRefNum) {

		Pa1Lib_Open();

		// Don't work on T4xx and T6xx series ?
		FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOnHandlerP, (UInt32*) &sndStateOnFuncP);
		FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOffHandlerP, (UInt32*) &sndStateOffFuncP);

		if (sndStateOnFuncP && sndStateOffFuncP) {
			((sndStateOnType)(sndStateOnFuncP))(aOutSndKindSp, gPrefs->volume.headphone, gPrefs->volume.headphone);
			((sndStateOnType)(sndStateOnFuncP))(aOutSndKindHp, gPrefs->volume.speaker, gPrefs->volume.speaker);

		}

		Pa1Lib_devHpVolume(gPrefs->volume.headphone, gPrefs->volume.headphone);
		Pa1Lib_devSpVolume(gPrefs->volume.speaker);
	}

	SavePrefs();	// free globals pref memory
	GlbOpen();
	main(argc, argvP);
	GlbClose();

	if (musicDriver == 1 || musicDriver == sysInvalidRefNum) {
		if (sndStateOnFuncP && sndStateOffFuncP) {
			((sndStateOffType)(sndStateOffFuncP))(aOutSndKindSp);
			((sndStateOffType)(sndStateOffFuncP))(aOutSndKindHp);
		}

		Pa1Lib_Close();
	}

	if (debug)
		VFSFileClose(gVars->logFile);

	for(count = 0; count < MAX_ARG; count++)
		if (argvP[count])
			MemPtrFree(argvP[count]);
	
	if (gVars->vibrator) {
		Boolean active = false;
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
	}

	if (!autoOff) {
		SysSetAutoOffTime(autoOffDelay);SystemPreferencesChoice
		EvtResetAutoOffTimer();
	}
	
	return false;
}
