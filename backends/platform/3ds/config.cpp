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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/platform/3ds/config.h"
#include "backends/platform/3ds/osystem.h"
#include "backends/platform/3ds/options-dialog.h"
#include "common/config-manager.h"

namespace _3DS {

Config config;
static Common::String prefix = "3ds_";

static bool confGetBool(Common::String key, bool defaultVal) {
	if (ConfMan.hasKey(prefix + key)) {
		return ConfMan.getBool(prefix + key);
	}
	return defaultVal;
}

static void confSetBool(Common::String key, bool val) {
	ConfMan.setBool(prefix + key, val);
}

static int confGetInt(Common::String key, int defaultVal) {
	if (ConfMan.hasKey(prefix + key)) {
		return ConfMan.getInt(prefix + key);
	}
	return defaultVal;
}

static void confSetInt(Common::String key, int val) {
	ConfMan.setInt(prefix + key, val);
}

void loadConfig() {
	config.showCursor = confGetBool("showcursor", true);
	config.snapToBorder = confGetBool("snaptoborder", true);
	config.stretchToFit = confGetBool("stretchtofit", false);
	config.screen = confGetInt("screen", kScreenBoth);

	// Turn off the backlight of any screen not used
	if (R_SUCCEEDED(gspLcdInit())) {
		if (config.screen == kScreenTop) {
			GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_TOP);
			GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);
		} else if (config.screen == kScreenBottom) {
			GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
			GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_TOP);
		} else {
			GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
		}
		gspLcdExit();
	}

	OSystem_3DS *osys = dynamic_cast<OSystem_3DS *>(g_system);
	osys->updateConfig();
}

void saveConfig() {
	confSetBool("showcursor", config.showCursor);
	confSetBool("snaptoborder", config.snapToBorder);
	confSetBool("stretchtofit", config.stretchToFit);
	confSetInt("screen", config.screen);
	ConfMan.flushToDisk();
}

} // namespace _3DS
