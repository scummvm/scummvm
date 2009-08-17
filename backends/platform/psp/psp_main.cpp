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


#define	USERSPACE_ONLY	//don't use kernel mode features

#ifndef USERSPACE_ONLY
#include <pspkernel.h>
#include <pspdebug.h>
#endif

#include <psppower.h>

#include <common/system.h>
#include <engines/engine.h>
#include <base/main.h>
#include <base/plugins.h>
#include "backends/platform/psp/powerman.h"


#include "osys_psp_gu.h"
#include "./trace.h"



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
int exit_callback(void) {
	sceKernelExitGame();
	return 0;
}

/* Function for handling suspend/resume */
void power_callback(int , int powerinfo) {
	if (powerinfo & PSP_POWER_CB_POWER_SWITCH || powerinfo & PSP_POWER_CB_SUSPENDING) {
		PowerMan.suspend();
	} else if (powerinfo & PSP_POWER_CB_RESUME_COMPLETE) {
		PowerMan.resume();
	}
}

/* Callback thread */
int CallbackThread(SceSize /*size*/, void *arg) {
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (SceKernelCallbackFunction)exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	/* Set up callbacks for PSPIoStream */

	cbid = sceKernelCreateCallback("Power Callback", (SceKernelCallbackFunction)power_callback, 0);
	if (cbid >= 0) {
		if(scePowerRegisterCallback(-1, cbid) < 0) {
			PSPDebugTrace("SetupCallbacks(): Couldn't register callback for power_callback\n");
		}
	} else {
		PSPDebugTrace("SetupCallbacks(): Couldn't create a callback for power_callback\n");
	}

	sceKernelSleepThreadCB();
	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
	int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

#undef main
int main(void) {
	PowerManager::instance();	// Setup power manager

	SetupCallbacks();

	//change clock rate to 333mhz
	scePowerSetClockFrequency(333, 333, 166);

	static const char *argv[] = { "scummvm", NULL };
	static int argc = sizeof(argv)/sizeof(char *)-1;

	g_system = new OSystem_PSP_GU();
	assert(g_system);

	int res = scummvm_main(argc, argv);

	g_system->quit();	// TODO: Consider removing / replacing this!

	PowerManager::destroy();	// get rid of PowerManager

	sceKernelSleepThread();

	return res;
}

