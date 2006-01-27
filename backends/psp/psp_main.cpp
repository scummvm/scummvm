/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 * Copyright (C) 2005 Joost Peters PSP Backend
 * Copyright (C) 2005 Thomas Mayer PSP Backend
 * Copyright (C) 2005 Paolo Costabel PSP Backend
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
 * $Header$
 *
 */
 
#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>

#define	REAL_MAIN
#include <common/stdafx.h>
#include <common/scummsys.h>
#include <base/engine.h>
#include <base/gameDetector.h>
#include <base/plugins.h>

#include <pspgu.h>
#include "./trace.h"


#define	USERSPACE_ONLY


/**
 * Define the module info section
 *
 * 2nd arg must 0x1000 so __init is executed in
 * kernelmode for our loaderInit function
 */
#ifndef USERSPACE_ONLY
PSP_MODULE_INFO("SCUMMVM-PSP", 0x1000, 1, 1);
#else
PSP_MODULE_INFO("SCUMMVM-PSP", 0, 1, 1);
#endif

/**
 * THREAD_ATTR_USER causes the thread that the startup
 * code (crt0.c) starts this program in to be in usermode
 * even though the module was started in kernelmode
 */
#ifndef USERSPACE_ONLY
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
#endif


#ifndef USERSPACE_ONLY
void MyExceptionHandler(PspDebugRegBlock *regs) {
	/* Do normal initial dump, setup screen etc */

	pspDebugScreenInit();

	pspDebugScreenSetBackColor(0x00FF0000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenClear();

	pspDebugScreenPrintf("Exception Details:\n");
	pspDebugDumpException(regs);

	while (1) ;
}

/**
 * Function that is called from _init in kernelmode before the
 * main thread is started in usermode.
 */
__attribute__ ((constructor))
void loaderInit() {
	pspKernelSetKernelPC();
	pspDebugInstallErrorHandler(MyExceptionHandler);
}
#endif

/* Exit callback */
SceKernelCallbackFunction exit_callback(int /*arg1*/, int /*arg2*/, void * /*common*/) {
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize /*size*/, void *arg) {
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (SceKernelCallbackFunction)exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();
	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


extern "C" int scummvm_main(int argc, char *argv[]);

int main(void)
{
	//PSPDebugTrace("Init debug screen\n");
	//pspDebugScreenInit();

	//PSPDebugTrace("Setup callbacks\n");
	SetupCallbacks();

	//check if the save directory exists
	SceUID fd = sceIoDopen(SCUMMVM_SAVEPATH);
	if (fd < 0) {
		//No? then let's create it.
		sceIoMkdir(SCUMMVM_SAVEPATH, 0777); 
	} else {
		//it exists, so close it again.
		sceIoDclose(fd);
	}

	static char *argv[] = { "scummvm", "--force-1x-overlay", NULL };
	static int argc = sizeof(argv)/sizeof(char *)-1;

	scummvm_main(argc, argv);
	
	sceKernelSleepThread();

	return 0;
}

