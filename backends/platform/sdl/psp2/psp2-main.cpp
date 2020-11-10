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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include <psp2/appmgr.h>

#include "common/scummsys.h"
#include "backends/platform/sdl/psp2/psp2.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

int _newlib_heap_size_user = 192 * 1024 * 1024;
char boot_params[1024];

int main(int argc, char *argv[]) {

#ifdef __PSP2_DEBUG__
	psp2shell_init(3333, 10);
#endif

	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);

	// Create our OSystem instance
	g_system = new OSystem_PSP2();
	assert(g_system);

	// Pre initialize the backend
	g_system->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
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

#ifdef __PSP2_DEBUG__
	psp2shell_exit();
#endif

	return res;
}
