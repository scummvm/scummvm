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
	registerCmd("scale", WRAP_METHOD(Console, scale));
	registerCmd("fade", WRAP_METHOD(Console, fade));
}

Console::~Console() {
}

bool Console::testCmd(int argc, const char **argv) {
	debugPrintf("test\n");
	return false;
}

bool Console::scale(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s SCALEX [SCALEY]   (0x10000 is normal size)\n", argv[0]);
		return true;
	}
	const int32 sx = strtol(argv[1], nullptr, 0);	// base 0 reads 0x8000 and -0x10000
	const int32 sy = argc > 2 ? strtol(argv[2], nullptr, 0) : sx;
	g_engine->setScale(sx, sy);
	return false;	// close the console so the picture is visible
}

bool Console::fade(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s STEPS DELAY_MS\n", argv[0]);
		return true;
	}
	g_engine->fadeOut(atoi(argv[1]), atoi(argv[2]));
	return false;
}

} // End of namespace Tesoro
