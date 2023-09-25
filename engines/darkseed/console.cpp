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

#include "darkseed/console.h"

namespace Darkseed {

Console::Console(TosText *tosText) : GUI::Debugger(), tosText(tosText) {
	registerCmd("tostext",   WRAP_METHOD(Console, Cmd_tostext));
}

Console::~Console() {
}

bool Console::Cmd_tostext(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: tostext <index>\n");
		return true;
	}

	uint16 textIdx = atoi(argv[1]);
	if (textIdx < tosText->getNumEntries()) {
		debugPrintf("%s\n", tosText->getText(textIdx).c_str());
	} else {
		debugPrintf("index too large!\n");
	}
	return true;
}

} // End of namespace Darkseed
