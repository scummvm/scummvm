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


// - Remove scummconsole.c
// - Delete files

// - libcartreset
// - Alternative controls - tap for left click, double for right
// - Inherit the Earth?
// - Stereo audio?
// - Software scaler?
// - 100% scale

// - Arrow keys cause key events when keyboard enabled - Done
// - Mouse cursor display - Done
// - Disable scaler on options menu - Done
// - Fix scale icons on top screen - Done
// - Fseek optimisation? - No need
// - Fix agi hack to be cleaner - done
// - Fix not typing looong words - Done
// - Show keyboard by default in AGI games
// - Fix mouse moving when cursor on keyboard screen - Done
// - Fix 'fit' thingy always appearing - Done
// - check cine backbuffer code - Done
// - Add long filename support - Done
// - New icons
// - Add key config for gob engine: Start:F1, Shift-numbers: F keys - Done
// - Fix [ds] appearing in game menu

// - Find out what's going wrong when you turn the console off
// - enable console when asserting

// - Alternative controls?


// - Fix 512x256 backbuffer to 320x240 - Done
// - Fix keyboard appearing on wrong screen - Done
// - Volume amplify option
// - Make save/restore game screen use scaler buffer


// 1.0.0!
// - Fix text on tabs on config screen
// - Remove ini file debug msg
// - Memory size for ite
// - Try discworld?

#include <nds.h>

#include "backends/platform/ds/osystem_ds.h"
#include "backends/plugins/ds/ds-provider.h"
#include "base/main.h"

namespace DS {

///////////////////
// Fast Ram
///////////////////

#define FAST_RAM_SIZE (22500)
#define ITCM_DATA	__attribute__((section(".itcm")))

u8 *fastRamPointer;
u8 fastRamData[FAST_RAM_SIZE] ITCM_DATA;

void *fastRamAlloc(int size) {
	void *result = (void *) fastRamPointer;
	fastRamPointer += size;
	if(fastRamPointer > fastRamData + FAST_RAM_SIZE) {
		warning("FastRam (ITCM) allocation failed");
		return malloc(size);
	}
	return result;
}

void fastRamReset() {
	fastRamPointer = &fastRamData[0];
}

} // End of namespace DS

/////////////////
// Main
/////////////////

int main(int argc, char **argv) {
#ifndef DISABLE_TEXT_CONSOLE
	videoSetModeSub(MODE_0_2D);
	consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 30, 0, false, true);
#endif

	g_system = new OSystem_DS();
	assert(g_system);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new DSPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	g_system->destroy();

	return res;
}
