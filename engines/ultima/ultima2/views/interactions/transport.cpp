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

#include "ultima/ultima2/views/interactions/transport.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

constexpr int HORSE_PRICE_INDEX = 4;

Transport::Transport() : Interaction("Transport") {
}

bool Transport::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	_price = _G(savegame).computeItemPrice(HORSE_PRICE_INDEX);
	writeString("WELCOME, TO THE TRANSPORT\nSHOPPE. I SELL YOU HORSE %.4d\nGOLD, DEAL, OK? --- ", _price);

	return Interaction::msgFocus(msg);
}

bool Transport::msgKeypress(const KeypressMessage &msg) {
	if (msg.ascii == 0)
		return true;

	char key = toupper(msg.ascii);
	writeString("%c\n", key);
	close();

	if (key != 'Y') {
		writeString("\nO.K. BYE THEN...\n");
	} else if (_G(logic)->trySpendGold(_price)) {
		writeString("\nRIDE SWIFTLY!\n");
		_G(savegame)._mount = Data::TILE_HORSE;
	}

	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
