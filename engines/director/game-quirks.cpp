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

#include "director/director.h"

namespace Director {

static void quirkKidsBox() {
    // Kids Box opens with a 320x150 splash screen before switching to
    // a full screen 640x480 game window. If desktop mode is off, ScummVM
    // will pick a game window that fits the splash screen and then try
    // to squish the full size game window into it.
    g_director->_wmMode = Director::wmModeDesktop;
    // Game runs in 640x480; clipping it to this size ensures the main
    // game window takes up the full screen, and only the splash is windowed.
    g_director->_wmWidth = 640;
    g_director->_wmHeight = 480;
}

static void quirkLzone() {
	SearchMan.addSubDirectoryMatching(g_director->_gameDataDir, "win_data", 0, 2);
}

static void quirkMcLuhan() {
	// TODO. Read fonts from MCLUHAN/SYSTEM directory
	g_director->_extraSearchPath.push_back("mcluhan\\");
	g_director->_extraSearchPath.push_back("mcluhan-win\\");
	g_director->_wm = new Graphics::MacWindowManager();
	g_director->_wm->setEngine(g_director);
	g_director->_wm->_fontMan->loadWindowsFont(Common::String("MCLUHAN/SYSTEM/MCBOLD13.FON"));
	g_director->_wm->_fontMan->loadWindowsFont(Common::String("MCLUHAN/SYSTEM/MCLURG__.FON"));
	g_director->_wm->_fontMan->loadWindowsFont(Common::String("MCLUHAN/SYSTEM/MCL1N___.FON"));
}

struct Quirk {
	const char *target;
	Common::Platform platform;
	void (*quirk)();
} quirks[] = {
    { "kidsbox", Common::kPlatformMacintosh, &quirkKidsBox },
	{ "lzone", Common::kPlatformWindows, &quirkLzone },
	{ "mcluhan", Common::kPlatformWindows, &quirkMcLuhan },
	{ nullptr, Common::kPlatformUnknown, nullptr }
};

void DirectorEngine::gameQuirks(const char *target, Common::Platform platform) {
	for (auto q = quirks; q->target != nullptr; q++) {
		if (q->platform == Common::kPlatformUnknown || q->platform == platform)
			if (!strcmp(q->target, target)) {
				q->quirk();
				break;
			}
	}
}

} // End of namespace Director
