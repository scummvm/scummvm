#include <PalmOS.h>
#include <Sonyclie.h>
#include "StarterRsc.h"

#include <stdio.h>
#include <unistd.h>	
#include <stdlib.h>

#include "games.h"
#include "start.h"
#include "rumble.h"
#include "pa1lib.h"
#include "extend.h"
#include "globals.h"
#include "features.h"


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

static void initARM() {
	// init global ARM only
	MemSet(gVars->arm, sizeof(gVars->arm), 0);
	ARM(PNO_COPYRECT	).pnoPtr = _PnoInit(RSC_COPYRECT, &ARM(PNO_COPYRECT).pnoDesc);
	ARM(PNO_COSTUMEPROC3).pnoPtr = _PceInit(RSC_COSTUMEPROC3);
	ARM(PNO_DRAWSTRIP	).pnoPtr = _PceInit(RSC_DRAWSTRIP);
	ARM(PNO_BLIT		).pnoPtr = _PnoInit(RSC_BLIT, &ARM(PNO_BLIT).pnoDesc);
}

static void releaseARM() {
	_PnoFree(&ARM(PNO_BLIT			).pnoDesc, ARM(PNO_BLIT).pnoPtr);
	_PceFree(ARM(PNO_DRAWSTRIP		).pnoPtr);
	_PceFree(ARM(PNO_COSTUMEPROC3	).pnoPtr);
	_PnoFree(&ARM(PNO_COPYRECT		).pnoDesc, ARM(PNO_COPYRECT).pnoPtr);
}

static Boolean checkPath(const Char *pathP) {
	FILE *tmpRef;

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
	UInt16 musicDriver = sysInvalidRefNum; // for launch call

	UInt16 index = GamGetSelected();

	for(count = 0; count < MAX_ARG; count++)
		argvP[count] = NULL;

	if (index != dmMaxRecordIndex) {
		Char pathP[256];
		Char num[6];
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
		
/*		// path exists ?
		if (!checkPath(pathP)) {
			MemHandleUnlock(recordH);
			FrmCustomAlert(FrmErrorAlert,"The specified path was not found !",0,0);
			return false;
		}
*/
		// ScummVM
		AddArg(&argvP[argc], "-", NULL, &argc);

		// path
		AddArg(&argvP[argc], "-p", pathP, &argc);

		// language
		if (gameInfoP->language > 0) {
			const Char *lang = "en\0de\0fr\0it\0p\0es\0jp\0z\0kr\0hb\0ru\0cz\0nl\0";
			AddArg(&argvP[argc], "-q", (lang + (gameInfoP->language - 1) * 3), &argc);
		}

		// fullscreen ?
		if (gameInfoP->fullscreen)
			AddArg(&argvP[argc], "-f", NULL, &argc);

		// aspect-ratio ?
		AddArg(&argvP[argc], (gameInfoP->aspectRatio ? "--aspect-ratio" : "--no-aspect-ratio"), NULL, &argc);

		// copy protection ?
		if (gPrefs->copyProtection)
			AddArg(&argvP[argc], "--copy-protection", NULL, &argc);

		// gfx mode
		gVars->flipping.pageAddr1 = (UInt8 *)(BmpGetBits(WinGetBitmap(WinGetDisplayWindow())));
		gVars->flipping.pageAddr2 = gVars->flipping.pageAddr1; // default if not flipping mode
		gVars->filter = gameInfoP->filter;

		switch (gameInfoP->gfxMode)	{
			case 1:
				AddArg(&argvP[argc], "-g", "flipping", &argc);
				gVars->flipping.pageAddr1 = (UInt8 *)WinScreenLock(winLockDontCare);
				WinScreenUnlock();
				break;
			case 2:
				AddArg(&argvP[argc], "-g", "buffered", &argc);
				break;
			case 3:
				AddArg(&argvP[argc], "-g", "wide", &argc);
				gVars->flipping.pageAddr1 = (UInt8 *)WinScreenLock(winLockDontCare);
				WinScreenUnlock();
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
			switch (gameInfoP->platform) {
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
				case 4:
					AddArg(&argvP[argc], "--platform=", "fmtowns", &argc);
					break;
				case 5:
					AddArg(&argvP[argc], "--platform=", "windows", &argc);
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
		if (gPrefs->debug) {
			StrIToA(num, gPrefs->debugLevel);
			AddArg(&argvP[argc], "-d", num, &argc);
		}
		// demo mode ?
		if (gPrefs->demoMode)
			AddArg(&argvP[argc], "--demo-mode", NULL, &argc);

		// alternative intro ?
		if (gPrefs->altIntro)
			AddArg(&argvP[argc], "--alt-intro", NULL, &argc);

		// multi midi ?
		if (gameInfoP->musicInfo.sound.multiMidi)
			AddArg(&argvP[argc], "--multi-midi", NULL, &argc);

		// music driver
		musicDriver =gameInfoP->musicInfo.sound.music;
		if (musicDriver) {
			switch (gameInfoP->musicInfo.sound.drvMusic) {
				case 0:	// NULL
					AddArg(&argvP[argc], "-e", "null", &argc);
					break;

				case 1:	// built-in MIDI
					if (OPTIONS_TST(kOptDeviceZodiac))
						AddArg(&argvP[argc], "-e", "zodiac", &argc);	// Tapwave Zodiac
					else if (OPTIONS_TST(kOptSonyPa1LibAPI))
						AddArg(&argvP[argc], "-e", "ypa1", &argc);		// Pa1Lib devices
					else
						AddArg(&argvP[argc], "-e", "null", &argc);		// error, no music driver
					break;

				case 2: // PC Speaker
					AddArg(&argvP[argc], "-e", "pcspk", &argc);
					break;
				case 3: // IBM PCjr
					AddArg(&argvP[argc], "-e", "pcjr", &argc);
				case 4: // FM Towns
					AddArg(&argvP[argc], "-e", "towns", &argc);
			}		
		}
		else	// NULL as default
			AddArg(&argvP[argc], "-e", "null", &argc);

		// music tempo
		StrIToA(num, gameInfoP->musicInfo.sound.tempo);
		AddArg(&argvP[argc], "--tempo=", num, &argc);

		// volume control
		StrIToA(num, gameInfoP->musicInfo.volume.master);
		AddArg(&argvP[argc], "-o", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.sfx);
		AddArg(&argvP[argc], "-s", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.music);
		AddArg(&argvP[argc], "-m", num, &argc);
		StrIToA(num, gameInfoP->musicInfo.volume.speech);
		AddArg(&argvP[argc], "-r", num, &argc);

		// output rate
		if (gameInfoP->musicInfo.sound.sfx) {
			UInt32 rates[] = {8000, 11025, 22050};
			StrIToA(num, rates[gameInfoP->musicInfo.sound.rate]);
			AddArg(&argvP[argc], "--output-rate=", num, &argc);
		}

		// game name
		AddArg(&argvP[argc], gameInfoP->gameP, NULL, &argc);

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

	if (argc > MAX_ARG)
		FrmCustomAlert(FrmErrorAlert, "Too many parameters.",0,0);

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
	gVars->screenLocked = false;
	gVars->volRefNum = gPrefs->card.volRefNum;
	gVars->vibrator = gPrefs->vibrator;
	gVars->stdPalette = gPrefs->stdPalette;
	gVars->autoReset = gPrefs->autoReset;
	
	// user params
	if (!gPrefs->arm) {
		OPTIONS_RST(kOptDeviceARM);
		OPTIONS_RST(kOptDeviceProcX86);
	}

	if (gVars->vibrator)
		gVars->vibrator = RumbleInit();
		
	// create file for printf, warnings, etc...
	void DrawStatus(Boolean show);
	StdioInit(gVars->volRefNum, "/PALM/Programs/ScummVM/scumm.log", DrawStatus);
	gUnistdCWD = SCUMMVM_SAVEPATH;

	// TODO : move this to ypa1.cpp (?)
	void *sndStateOnFuncP = NULL,
		 *sndStateOffFuncP = NULL;

	if (musicDriver == 1 || musicDriver == 3 || musicDriver == 4 || musicDriver == sysInvalidRefNum) {

		Pa1Lib_Open();

		// Don't work on T4xx and T6xx series ?
		FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOnHandlerP, (UInt32*) &sndStateOnFuncP);
		FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOffHandlerP, (UInt32*) &sndStateOffFuncP);

		if (sndStateOnFuncP && sndStateOffFuncP) {
			((sndStateOnType)(sndStateOnFuncP))(aOutSndKindSp, 31, 31);
			((sndStateOnType)(sndStateOnFuncP))(aOutSndKindHp, 31, 31);
		}

		Pa1Lib_devHpVolume(31, 31);
		Pa1Lib_devSpVolume(31);
	}
	// -------------

	SavePrefs();	// free globals pref memory
	GlbOpen();
	initARM();
	
	// reset screen depth
	{
		UInt32 depth = 8;		
		WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	}

	DO_EXIT( main(argc, argvP); )
	
	// be sure to release feature memory
	FREE_FTR(ftrBufferOverlay)
	FREE_FTR(ftrBufferBackup)
	FREE_FTR(ftrBufferHotSwap)

	releaseARM();
	GlbClose();

	// TODO : move this to ypa1.cpp (?)
	if (musicDriver == 1 || musicDriver == 3 || musicDriver == 4 || musicDriver == sysInvalidRefNum) {
		if (sndStateOnFuncP && sndStateOffFuncP) {
			((sndStateOffType)(sndStateOffFuncP))(aOutSndKindSp);
			((sndStateOffType)(sndStateOffFuncP))(aOutSndKindHp);
		}

		Pa1Lib_Close();
	}
	// -------------

	// close log file
	StdioRelease();

	for(count = 0; count < MAX_ARG; count++)
		if (argvP[count])
			MemPtrFree(argvP[count]);
	
	if (gVars->vibrator)
		RumbleRelease();

	if (!autoOff) {
		SysSetAutoOffTime(autoOffDelay);SystemPreferencesChoice
		EvtResetAutoOffTimer();
	}

	return false;
}
