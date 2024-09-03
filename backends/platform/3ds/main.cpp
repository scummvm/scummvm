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

#include "backends/platform/3ds/osystem.h"
#include "backends/plugins/3ds/3ds-provider.h"

#include <3ds.h>
#include <malloc.h>

enum {
	SYSTEM_MODEL_2DS = 3
};

// Set the size of the stack.
u32 __stacksize__ = 64 * 1024;

// Set the size of the linear heap to allow a larger application heap.
u32 __ctru_heap_size        = 0;
u32 __ctru_linear_heap_size = 10 * 1024 * 1024;

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

	g_system = new N3DS::OSystem_3DS();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new CTRPluginProvider());
#endif

	int res = scummvm_main(argc, argv);

	g_system->destroy();

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
