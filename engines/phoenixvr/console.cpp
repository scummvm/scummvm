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

#include "phoenixvr/console.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

Console::Console() : GUI::Debugger() {
	registerCmd("warp", WRAP_METHOD(Console, cmdWarp));
	registerCmd("script", WRAP_METHOD(Console, cmdScript));
	registerCmd("stop_all_sounds", WRAP_METHOD(Console, cmdStopAllSounds));
	registerCmd("next_level", WRAP_METHOD(Console, cmdNextLevel));
}

Console::~Console() {
}

bool Console::cmdWarp(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("warp <location.vr>\n");
		return true;
	}
	g_engine->goToWarp(argv[1]);
	return false;
}

bool Console::cmdScript(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("script <script.lst>\n");
		return true;
	}
	g_engine->setNextScript(argv[1]);
	return false;
}

bool Console::cmdStopAllSounds(int argc, const char **argv) {
	g_engine->stopAllSounds();
	return false;
}

bool Console::cmdNextLevel(int argc, const char **argv) {
	if (g_engine->setNextLevel())
		g_engine->stopAllSounds();
	return false;
}

} // End of namespace PhoenixVR
