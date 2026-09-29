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
#include "ultima/ultima2/views/overworld_map.h"
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
	if (msg.keycode == Common::KEYCODE_RETURN) {
		writeString("\n");
		close();

		Data::Savegame &sg = _G(savegame);
		if (_text.equalsIgnoreCase("scummvm") && dynamic_cast<Views::OverworldMap *>(g_engine->focusedView()) != nullptr &&
				sg._mount == 0) {
			writeString("The universe whisks you away.\n");

			// A parting gift
			sg._items[Data::ITEM_HELM] = MIN(sg._items[Data::ITEM_HELM] + 1, Data::MAX_BCD_BYTE);

			// Show the ScummVM secret map
			sg._overworldReturnX = sg._mapX;
			sg._overworldReturnY = sg._mapY;
			sg._mapX = 31;
			sg._mapY = 62;
			sg._mapType = 2;
			_G(map).load(sg._mapEra, 9);
			_G(logic)->entering();
		}

		_G(logic)->resumeTurn();
	} else if (Common::isPrint(msg.ascii)) {
		writeString("%c", (char)msg.ascii);
		_text += msg.ascii;
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
