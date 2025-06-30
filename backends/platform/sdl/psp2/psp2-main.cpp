/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include <psp2/appmgr.h>
#include <psp2/kernel/threadmgr/callback.h>

#include "common/scummsys.h"
#include "backends/platform/sdl/psp2/psp2.h"
#include "backends/platform/sdl/psp2/powerman.h"
#include "backends/plugins/psp2/psp2-provider.h"
#include "base/main.h"
#include "engines/engine.h"

int _newlib_heap_size_user = 192 * 1024 * 1024;
char boot_params[1024];


int power_callback(int notifyId, int notifyCount, int powerInfo, void *common) {
	debug(9, "power_callback %x", powerInfo);

	if ((powerInfo & SCE_POWER_CB_APP_RESUME) ||
		(powerInfo & SCE_POWER_CB_APP_RESUMING)) {
		debug(2, "App resuming");
		PowerMan.resume();
	} else if ((powerInfo & SCE_POWER_CB_BUTTON_PS_PRESS) ||
		(powerInfo & SCE_POWER_CB_APP_SUSPEND) ||
		(powerInfo & SCE_POWER_CB_SYSTEM_SUSPEND)) {
		debug(2, "App on background");
		PowerMan.suspend();
	}

	return 0;
}

int callbacks_thread(unsigned int args, void* arg) {
	// Add a callback to pause/resume games when the system is going to sleep
	// or when the user press the PS button. It would crash overwise.
	int cbid = sceKernelCreateCallback("Power Callback", 0, power_callback, NULL);
	scePowerRegisterCallback(cbid);
	for (;;) {
		sceKernelDelayThreadCB(10000000);
	}

	return 0;
}

int main(int argc, char *argv[]) {

	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);

	// Create our OSystem instance
	g_system = new OSystem_PSP2();
	assert(g_system);

	// Pre initialize the backend
	g_system->init();

	PowerManager::instance();	// Setup power manager

	// Starting power callbacks handler
	SceUID thid = sceKernelCreateThread("callbackThread", callbacks_thread, 0x10000100, 0x10000, 0, 0, NULL);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	} else {
		warning("Cannot create power callback thread ! Suspend may not work !");
	}

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new PSP2PluginProvider());
#endif

	sceAppMgrGetAppParam(boot_params);
	int res;
	if (strstr(boot_params,"psgm:play"))
	{
		char *path_param = strstr(boot_params, "&path=");
		char *gameid_param = strstr(boot_params, "&game_id=");
		if (path_param != NULL && gameid_param != NULL)
		{
			char path[256];
			char game_id[64];

			if (gameid_param > path_param)
			{
				// handle case where gameid param follows path param
				path_param += 6;
				memcpy(path, path_param, gameid_param - path_param);
				path[gameid_param-path_param] = 0;
				snprintf(game_id, 64, gameid_param + 9);
			}
			else
			{
				// handle case where path param follows gameid param
				gameid_param += 9;
				memcpy(game_id, gameid_param, path_param - gameid_param);
				game_id[path_param-gameid_param] = 0;
				snprintf(path, 256, path_param + 6);
			}

			const char* args[4];
			args[0] = "ux0:app/VSCU00001/eboot.bin";
			args[1] = "-p";
			args[2] = path;
			args[3] = game_id;

			res = scummvm_main(4, args);
			goto exit;
		}
	}

	// Invoke the actual ScummVM main entry point:
	res = scummvm_main(argc, argv);

exit:
	// Free OSystem
	g_system->destroy();

	PowerManager::destroy();	// get rid of PowerManager

	return res;
}
