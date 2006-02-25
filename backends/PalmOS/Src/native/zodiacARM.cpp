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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "globals.h"
#include "extend.h"
#include "args.h"
#include "palmdefs.h"

#include "rumble.h"


#include <AdnDebugMgr.h>
//#define DEBUG_ARM

GlobalsDataType g_vars;
GlobalsDataPtr gVars = &g_vars;
UInt32 g_stackSize;

static void Go() {
	void *tmp;
	char **argvP;
	int argc;

#ifdef DEBUG_ARM
// Tell the debugger we want to enable full debugging 
	UInt32 flags = AdnDebugEnableGet(); 
	flags |= kAdnEnableMasterSwitch | kAdnEnableFullDebugging; 
	AdnDebugEnableSet(flags); 
// Tell the debugger where our code lives in memory: 
	AdnDebugNativeRegister(sysFileTApplication, appFileCreator, 'ARMC', 1);
#endif

	// get global struct
	FtrGet(appFileCreator, ftrVars, (UInt32 *)&tmp);
	MemMove(gVars, tmp, sizeof(GlobalsDataType));

	// init STDIO
	StdioSetCacheSize(0);
	StdioInit(gVars->VFS.volRefNum, "/PALM/Programs/ScummVM/scumm.log");
	if (gVars->indicator.showLED)
		StdioSetLedProc(DrawStatus);
	StdioSetCacheSize(gVars->VFS.cacheSize);
	gUnistdCWD = SCUMMVM_SAVEPATH;

	// get args
	FtrGet(appFileCreator, ftrArgsData, (UInt32 *)&argvP);
	FtrGet(appFileCreator, ftrArgsCount, (UInt32 *)&argc);

	// init system
	WinSetDrawWindow(WinGetDisplayWindow());
	if (HWR_INIT(INIT_VIBRATOR))	gVars->vibrator =	RumbleInit();

	// run ...
	extern int main(int, char **);
	DO_EXIT ( main(argc, argvP); )	

	// release 
	if (HWR_INIT(INIT_VIBRATOR))	RumbleRelease();
	StdioRelease();

#ifdef DEBUG_ARM
	AdnDebugNativeUnregister();
#endif
}

/* stack size */

Int8 *g_newStack, *g_newStackPos;
void *g_oldStack;

static asm void *StkSwap(void *newStack ,void *dummy) {	
	mov  r1, r13
	mov  r13, r0
	mov	 r0, r1
	bx	 lr
}

extern UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
	if (cmd == sysAppLaunchCmdNormalLaunch) {
		FtrGet(appFileCreator, ftrStack, &g_stackSize);
		if (!g_stackSize)
			g_stackSize = 32 * 1024;

		g_newStack = (Int8 *)malloc(g_stackSize + 8);
		g_newStackPos = (g_newStack + g_stackSize);
		g_newStackPos -= ((UInt32)g_newStackPos & 7);
		
		g_oldStack = StkSwap(g_newStackPos, 0);

		Go();

		StkSwap(g_oldStack, 0);
		free(g_newStack);
	}

	return 0;
}

