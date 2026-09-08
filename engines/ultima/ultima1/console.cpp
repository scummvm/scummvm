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

#include "ultima/ultima1/console.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/views/dialog.h"

namespace Ultima {
namespace Ultima1 {

Console::Console() : GUI::Debugger() {
	registerCmd("view", WRAP_METHOD(Console, cmdView));
	registerCmd("map", WRAP_METHOD(Console, cmdMap));
}

Console::~Console() {
}

bool Console::cmdView(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("view <view name>\n");
		return true;
	} else {
		auto *view = g_engine->findView(argv[1]);

		if (!view) {
			debugPrintf("Unknown view name\n");
			return true;
		} else if (dynamic_cast<Views::Dialog *>(view)) {
			g_engine->replaceView("Game", true);
			g_engine->addView(argv[1]);
			return false;
		} else {
			g_engine->replaceView(argv[1], true);
			return false;
		}
	}
}

bool Console::cmdMap(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("map <map num>\n");
		return true;
	} else {
		int location = atoi(argv[1]);

		if (location >= 41 && location < 49) {
			g_engine->send("Pillar", Shared::Messages::GameMessage("PILLAR", location - 41));
		} else {
			_G(map).load(location);
			_G(logic)->entering();
		}

		return false;
	}
}

} // namespace Ultima1
} // namespace Ultima
