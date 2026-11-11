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

#include "tesoro/console.h"
#include "tesoro/core/animbank.h"

namespace Tesoro {

Console::Console() : GUI::Debugger() {
	registerCmd("testCmd", WRAP_METHOD(Console, testCmd));
	registerCmd("fade", WRAP_METHOD(Console, fade));
	registerCmd("level", WRAP_METHOD(Console, level));
	registerCmd("hover", WRAP_METHOD(Console, hover));
}

Console::~Console() {
}

bool Console::testCmd(int argc, const char **argv) {
	debugPrintf("test\n");
	return false;
}



bool Console::fade(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s STEPS DELAY_MS\n", argv[0]);
		return true;
	}
	g_engine->fadeOut(atoi(argv[1]), atoi(argv[2]));
	return false;
}

bool Console::level(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s [LEVEL]\n", argv[0]);
		return true;
	}
	if (!g_engine->level(argv[1])) {
		debugPrintf("Cannot load %s\n", argv[1]);
		return true;
	}
	return false;
}

bool Console::hover(int argc, const char **argv) {
	if (argc < 2) {
		g_engine->setHoverText(nullptr);
		return false;
	}
	g_engine->setHoverText(argv[1]);
	return false;
}

} // End of namespace Tesoro
