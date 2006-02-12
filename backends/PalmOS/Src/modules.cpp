#include <PalmOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "palmdefs.h"
#include "args.h"
#include "globals.h"
#include "modules.h"
#include "extend.h"
#include "features.h"

#include "rumble.h"
#include "init_mathlib.h"
#include "init_pa1lib.h"
#include "init_palmos.h"
#include "init_stuffs.h"
#include "init_sony.h"
#include "init_golcd.h"

#ifndef DISABLE_LIGHTSPEED
#include "lightspeed_public.h"
#endif

GlobalsDataPtr gVars;

#ifdef PALMOS_NATIVE

#include "endianutils.h"
#include <PNOLoader.h>

#ifdef COMPILE_ZODIAC
#	include <tapwave.h>
#	include <TwRuntime.h>
#else
#	include <TwDefs.h>
#endif

void run(int argc, char *argv[]) {
	// init args
	ArgsExportInit(argv, argc, true);

	// init system
	PalmHRInit(16);
	PalmInit(HWR_GET());
	void *__ptr = StuffsForceVG();

	gVars->screenPitch = StuffsGetPitch(gVars->screenFullWidth);

	// export global struct to ARM
	VARS_EXPORT();
	DO_VARS(_4B, 32, 0);
	DO_VARS(_2B, 16, (gVars->_4B * sizeof(UInt32)));
	FtrSet(appFileCreator, ftrVars , (UInt32)gVars);

	// run the module
#ifdef COMPILE_ZODIAC
	NativeFuncType *entry;
	TwLoadModule(0, 0, 0, 1, twLoadFlagTNA|twLoadFlagQuickRun, &entry);
#else
	UInt32 glue;
	FtrGet(twFtrCreator, twFtrAPIGlue, &glue);

	PnoDescriptor pno;
	PnoLoadFromResources(&pno, 'ARMC', 1, appFileCreator, 1);
	PnoCall(&pno, (void*)glue);
	PnoUnload(&pno);
#endif

	// reset globals
	DO_VARS(_4B, 32, 0);
	DO_VARS(_2B, 16, (gVars->_4B * sizeof(UInt32)));

	// release
	StuffsReleaseVG(__ptr);
	PalmRelease(HWR_GET());
	PalmHRRelease();

	// free args
	ArgsExportRelease(true);
	ArgsFree(argv);

	// release global struct
	FtrUnregister(appFileCreator, ftrVars);
	FtrUnregister(appFileCreator, ftrStack);
	MemPtrFree(gVars);

	// reset the palette if needed
	WinPalette(winPaletteSetToDefault, 0, 256, NULL);
}

#else

void run(int argc, char *argv[]) {

	MathlibInit();
	gVars->HRrefNum	= SonyHRInit(8);
	if (gVars->HRrefNum == sysInvalidRefNum)
		PalmHRInit(8);
	gVars->slkRefNum= SilkInit(&(gVars->slkVersion));
	gVars->screenPitch = StuffsGetPitch(gVars->screenFullWidth);

	// create file for printf, warnings, etc...
	StdioInit(gVars->VFS.volRefNum, "/PALM/Programs/ScummVM/scumm.log");
	if (gVars->indicator.showLED) StdioSetLedProc(DrawStatus);
	StdioSetCacheSize(gVars->VFS.cacheSize);
	gUnistdCWD = SCUMMVM_SAVEPATH;
	
	// init hardware
	if (HWR_INIT(INIT_GOLCD))							GoLCDInit(&gGoLcdH);
	if (HWR_INIT(INIT_PA1LIB))							Pa1libInit(gVars->palmVolume);
	if (HWR_INIT(INIT_VIBRATOR))	gVars->vibrator =	RumbleInit();
														PalmInit(HWR_GET());

	if (!gVars->vibrator)
		HWR_RST(INIT_VIBRATOR);

	GlbOpen();
	// be sure to have a VG
	void *__ptr = StuffsForceVG();

	DO_EXIT( main(argc, argv); )

	// be sure to release features memory
	FREE_FTR(ftrBufferOverlay)
	FREE_FTR(ftrBufferBackup)
	FREE_FTR(ftrBufferHotSwap)

	StuffsReleaseVG(__ptr);
	GlbClose();

									PalmRelease(HWR_GET());
	if (HWR_INIT(INIT_VIBRATOR))	RumbleRelease();
	if (HWR_INIT(INIT_PA1LIB))		Pa1libRelease();
	if (HWR_INIT(INIT_GOLCD))		GoLCDRelease(gGoLcdH);

	// close log file
	StdioRelease();

	PalmHRRelease();
	SonyHRRelease(gVars->HRrefNum);
	SilkRelease(gVars->slkRefNum);
	MathlibRelease();

	MemPtrFree(gVars);
	WinPalette(winPaletteSetToDefault, 0, 256, NULL);
//	ArgsFree(argvP);	// called in main(...)
}
#endif

static UInt32 ModulesPalmMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	UInt32 result = 0;

	switch (cmd) {
		case sysAppLaunchCustomEngineGetInfo: {
#ifdef PALMOS_NATIVE
			result = GET_MODEARM;
#else
#	if	!defined(DISABLE_SCUMM) || \
		!defined(DISABLE_SIMON) || \
		!defined(DISABLE_SWORD1)
			result = GET_DATACOMMON|GET_DATAENGINE|GET_MODE68K;
#	else
			result = GET_DATACOMMON|GET_MODE68K;
#	endif
#endif
			break;
		}

		case sysAppLaunchCmdNormalLaunch: {
			if (cmdPBP) {
				Char **argvP;
				UInt16 cardNo;
				LocalID dbID;

				LaunchParamType *lp = (LaunchParamType *)cmdPBP;
				
				gVars = lp->gVars;
				argvP = lp->args.argv;

				// get the free memory on the dynamic heap
				PalmGetMemory(0,0,0, &(gVars->startupMemory));

#ifndef DISABLE_LIGHTSPEED
				switch (lp->lightspeed) {
					case 0:
						LS_SetCPUSpeedHigh();
						break;
					case 1:
						LS_SetCPUSpeedNormal();
						break;
					case 2:
						LS_SetCPUSpeedLow();
						break;
				}
#endif
	//			MemPtrSetOwner(gVars, ownerID);
	//			ArgsSetOwner(argvP, ownerID);	// will be freed by main(...)
	//			MemPtrFree(lp);					// will be freed by the system on exit
				
				run(lp->args.argc, argvP);

				cardNo = 0;
				dbID = DmFindDatabase(0, "ScummVM");
				if (dbID) {
					if (lp->exitLauncher)
						SysUIAppSwitch(cardNo, dbID, sysAppLaunchCustomEngineDelete,0);
					else
						SysUIAppSwitch(cardNo, dbID, sysAppLaunchCmdNormalLaunch,0);
				}
			}
			break;
		}

		default:
			break;

		}

	return result;
}

UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
	return ModulesPalmMain(cmd, cmdPBP, launchFlags);
}
