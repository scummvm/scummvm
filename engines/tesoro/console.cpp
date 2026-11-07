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
	registerCmd("showimage", WRAP_METHOD(Console, showimage));
	registerCmd("playanim", WRAP_METHOD(Console, playanim));
	registerCmd("rowdump", WRAP_METHOD(Console, rowdump));
}

Console::~Console() {
}

bool Console::rowdump(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: %s NAME.ANM IMAGE ROW [X]\n", argv[0]);
		return true;
	}
	Common::String name(argv[1]);
	name.toUppercase();
	const int x = argc > 4 ? atoi(argv[4]) : 20;
	debugPrintf("%s", g_engine->dumpRow(name.c_str(), atoi(argv[2]), atoi(argv[3]), x).c_str());
	return true;	// stay in the console so the output can be read
}

bool Console::showimage(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: %s NAME.ANM IMAGE_NUMBER\n", argv[0]);
		return true;
	}
	Common::String name(argv[1]);
	name.toUppercase();
	const uint index = atoi(argv[2]);

	g_engine->showImage(name.c_str(), index);
	return false;	// false closes the console, so the picture is visible
}

bool Console::playanim(int argc, const char **argv) {
	g_engine->playAnim(argv[1]);
	return false;
}


} // End of namespace Tesoro
