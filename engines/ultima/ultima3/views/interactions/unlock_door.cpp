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

#include "ultima/ultima3/views/interactions/unlock_door.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr byte CELL_LOCKED_DOOR = 0xB8;

bool UnlockDoor::keypress(const KeypressMessage &msg) {
	Data::Savegame &save = _G(savegame);

	if (!_chosen) {
		DirectionChooser::Result result = _directions.handleKey(msg);
		if (result == DirectionChooser::PENDING)
			return false;
		if (result == DirectionChooser::CANCELLED)
			return true;

		// Doors are only found to the sides
		Direction dir = _directions.direction();
		_doorX = save._posX + (dir == DIR_EAST ? 1 : (dir == DIR_WEST ? -1 : 0));
		_doorY = save._posY;

		if ((dir != DIR_EAST && dir != DIR_WEST) || _G(map).cell(_doorX, _doorY) != CELL_LOCKED_DOOR) {
			_G(messages).print("Not Here!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		_chosen = true;
		_G(messages).print("Whose key? ");
		return false;
	}

	PlayerChooser::Result result = _players.handleKey(msg);
	if (result == PlayerChooser::PENDING)
		return false;
	if (result == PlayerChooser::CANCELLED)
		return true;

	Data::RosterEntry &e = save.partyMember(_players.slot());
	if (e._keys == 0) {
		_G(messages).print("None Left!\n");
		g_engine->playSoundEffect(0xFE);
		return true;
	}

	// The door opens onto the same kind of ground the party stands on
	e._keys = Data::toBcd(Data::fromBcd(e._keys) - 1);
	_G(map).setCell(_doorX, _doorY, _G(map).tile(save._posX, save._posY) << 2);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
