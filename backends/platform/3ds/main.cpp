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

#include "backends/platform/3ds/osystem.h"
#include "backends/plugins/3ds/3ds-provider.h"

#include <3ds.h>
#include <malloc.h>

enum {
	SYSTEM_MODEL_2DS = 3
};

struct CommandLine {
	int argumentCount;
	char** argumentsValue;

	CommandLine(int argc, char** argv): argumentCount(argc), argumentsValue(argv) {}
};

static void mainThreadFunc(void *threadParams) {
	g_system = new _3DS::OSystem_3DS();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new CTRPluginProvider());
#endif

	CommandLine *commandLine = static_cast<CommandLine *>(threadParams);
	int res = scummvm_main(commandLine->argumentCount, commandLine->argumentsValue);

	g_system->destroy();

	threadExit(res);
};

int main(int argc, char *argv[]) {
	// Initialize basic libctru stuff
	cfguInit();
	gfxInitDefault();

	// 800px wide top screen is not available on old 2DS systems
	u8 systemModel = 0;
	CFGU_GetSystemModel(&systemModel);
	gfxSetWide(systemModel != SYSTEM_MODEL_2DS);

	romfsInit();
	osSetSpeedupEnable(true);
// 	consoleInit(GFX_TOP, NULL);
	gdbHioDevInit();
	gdbHioDevRedirectStdStreams(true, true, true);

#ifdef USE_LIBCURL
	const uint32 soc_sharedmem_size = 0x10000;
	void *soc_sharedmem = memalign(0x1000, soc_sharedmem_size);
	socInit((u32 *)soc_sharedmem, soc_sharedmem_size);
#endif

	// Start ScummVM in a separate thread to be able to set the stack size.
	// The default stack is not large enough.
	CommandLine commandLine(argc, argv);

	s32 mainThreadPriority = 0;
	svcGetThreadPriority(&mainThreadPriority, CUR_THREAD_HANDLE);

	Thread mainThread = threadCreate(&mainThreadFunc, &commandLine, 64 * 1024, mainThreadPriority, -2, false);
	threadJoin(mainThread, U64_MAX);
	int res = threadGetExitCode(mainThread);
	threadFree(mainThread);

	// Turn on both screen backlights before exiting.
	if (R_SUCCEEDED(gspLcdInit())) {
		GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
		gspLcdExit();
	}

#ifdef USE_LIBCURL
	socExit();
#endif
	gdbHioDevExit();
	romfsExit();
	gfxExit();
	cfguExit();
	return res;
}
