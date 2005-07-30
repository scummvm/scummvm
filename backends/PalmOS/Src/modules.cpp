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
#include "init_arm.h"
#include "init_palmos.h"
#include "init_sony.h"

#ifndef DISABLE_LIGHTSPEED
#include "lightspeed_public.h"
#endif

GlobalsDataPtr gVars;

void run(int argc, char *argv[]) {

	MathlibInit();
	gVars->HRrefNum	= SonyHRInit(8);
	if (gVars->HRrefNum == sysInvalidRefNum)
		PalmHRInit(8);
	gVars->slkRefNum= SilkInit(&(gVars->slkVersion));

	// create file for printf, warnings, etc...
	StdioInit(gVars->volRefNum, "/PALM/Programs/ScummVM/scumm.log", DrawStatus);
	gUnistdCWD = SCUMMVM_SAVEPATH;

	// init hardware
	if (HWR_INIT(INIT_PA1LIB))							Pa1libInit();
	if (HWR_INIT(INIT_VIBRATOR))	gVars->vibrator =	RumbleInit();
	if (HWR_INIT(INIT_ARM))								ARMInit();
														PalmInit(HWR_GET());

	if (!gVars->vibrator)
		HWR_RST(INIT_VIBRATOR);

	GlbOpen();

	DO_EXIT( main(argc, argv); )

	// be sure to release features memory
	FREE_FTR(ftrBufferOverlay)
	FREE_FTR(ftrBufferBackup)
	FREE_FTR(ftrBufferHotSwap)

	GlbClose();

									PalmRelease(HWR_GET());
	if (HWR_INIT(INIT_ARM))			ARMRelease();
	if (HWR_INIT(INIT_VIBRATOR))	RumbleRelease();
	if (HWR_INIT(INIT_PA1LIB))		Pa1libRelease();

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

static UInt32 ModulesPalmMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	switch (cmd)
		{
		case sysAppLaunchCmdNormalLaunch:
		{
			if (cmdPBP) {
				Char **argvP;
				UInt16 cardNo;
				LocalID dbID;

				LaunchParamType *lp = (LaunchParamType *)cmdPBP;

				gVars = lp->gVars;
				argvP = lp->args.argv;

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
						SysUIAppSwitch(cardNo, dbID, sysAppLaunchCustomDeleteEngine,0);
					else
						SysUIAppSwitch(cardNo, dbID, sysAppLaunchCmdNormalLaunch,0);
				}
			}
			break;
		}

		default:
			break;

		}

	return 0;
}

UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	return ModulesPalmMain(cmd, cmdPBP, launchFlags);
}
