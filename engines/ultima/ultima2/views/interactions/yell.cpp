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

#include "ultima/ultima2/views/interactions/yell.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

Yell::Yell() : Interaction("Yell") {
}

bool Yell::msgFocus(const FocusMessage &msg) {
	writeString("YELL WHAT?\n");
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Interaction::msgFocus(msg);
}

bool Yell::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii == 13) {
		writeString("\n");
		close();
		_G(logic)->resumeTurn();
	} else if (msg.ascii >= 32 && msg.ascii < 127) {
		writeString("%c", (char)msg.ascii);
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
