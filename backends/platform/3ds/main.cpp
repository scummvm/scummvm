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

#include "osystem.h"
#include <3ds.h>

int main(int argc, char *argv[]) {
	// Initialize basic libctru stuff
	gfxInitDefault();
	cfguInit();
	osSetSpeedupEnable(true);
// 	consoleInit(GFX_TOP, NULL);

	g_system = new _3DS::OSystem_3DS();
	assert(g_system);

	// Invoke the actual ScummVM main entry point
// 	if (argc > 2)
// 		res = scummvm_main(argc-2, &argv[2]);
// 	else
// 		res = scummvm_main(argc, argv);
//	scummvm_main(0, nullptr);

	int res = scummvm_main(argc, argv);

	g_system->destroy();

	// Turn on both screen backlights before exiting.
	if (R_SUCCEEDED(gspLcdInit())) {
		GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
		gspLcdExit();
	}

	cfguExit();
	gfxExit();
	return res;
}
