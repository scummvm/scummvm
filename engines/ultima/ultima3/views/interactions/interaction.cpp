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

#include "ultima/ultima3/views/interactions/interaction.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

PlayerChooser::Result PlayerChooser::handleKey(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		_G(messages).print("\n");
		return CANCELLED;
	}

	int number = msg.ascii - '0';
	if (number < 0 || number > Data::PARTY_MAX) {
		g_engine->playSoundEffect(0xFE);
		return PENDING;
	}

	_G(messages).putChar('0' + number);
	Result result = CHOSEN;

	if (number == 0) {
		result = CANCELLED;
	} else if (number > _G(savegame)._partySize) {
		_G(messages).print("\nNo one there!");
		g_engine->playSoundEffect(0xFE);
		result = CANCELLED;
	}

	_slot = number - 1;
	_G(messages).print("\n");
	return result;
}

DirectionChooser::Result DirectionChooser::handleKey(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_UP:
		_dir = DIR_NORTH;
		_G(messages).print("North\n");
		return CHOSEN;
	case Common::KEYCODE_DOWN:
		_dir = DIR_SOUTH;
		_G(messages).print("South\n");
		return CHOSEN;
	case Common::KEYCODE_RIGHT:
		_dir = DIR_EAST;
		_G(messages).print("East\n");
		return CHOSEN;
	case Common::KEYCODE_LEFT:
		_dir = DIR_WEST;
		_G(messages).print("West\n");
		return CHOSEN;
	case Common::KEYCODE_ESCAPE:
		_G(messages).print("\n");
		return CANCELLED;
	default:
		g_engine->playSoundEffect(0xFE);
		return PENDING;
	}
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
