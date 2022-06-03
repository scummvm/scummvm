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

static void quirkLzone() {
	SearchMan.addSubDirectoryMatching(g_director->_gameDataDir, "win_data", 0, 2);
}

static void quirkMediaband() {
	// Meet Mediaband could have up to 5 levels of directories
	SearchMan.addDirectory(g_director->_gameDataDir.getPath(), g_director->_gameDataDir, 0, 5);
}

static void quirkWarlock() {
	// Meet Mediaband could have up to 5 levels of directories
	SearchMan.addDirectory(g_director->_gameDataDir.getPath(), g_director->_gameDataDir, 0, 5);
}

static void quirkMcLuhan() {
	// TODO. Read fonts from MCLUHAN/SYSTEM directory
	g_director->_extraSearchPath.push_back("mcluhan\\");
}

struct Quirk {
	const char *target;
	Common::Platform platform;
	void (*quirk)();
} quirks[] = {
	{ "lzone", Common::kPlatformWindows, &quirkLzone },
	{ "mediaband", Common::kPlatformUnknown, &quirkMediaband },
	{ "warlock", Common::kPlatformUnknown, &quirkWarlock },
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
