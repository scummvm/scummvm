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

#include "ultima/ultima3/views/interactions/steal_chest.h"
#include "ultima/ultima3/logic/chest_logic.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr byte FIRST_COUNTER_CELL = 0x94;
constexpr byte LAST_COUNTER_CELL = 0xE4;
constexpr byte CELL_CHEST = 0x24;
constexpr byte CELL_FLOOR = 0x20;
constexpr byte CELL_GUARD = 0x48;
constexpr byte FLAGS_HOSTILE = 0xC0;

bool StealChest::keypress(const KeypressMessage &msg) {
	Data::Savegame &save = _G(savegame);

	if (!_chosen) {
		PlayerChooser::Result result = _players.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		if (!save.partyMember(_players.slot()).isAlive()) {
			_G(messages).print("Incapacitated!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		_chosen = true;
		_G(messages).print("Direct? ");
		return false;
	}

	DirectionChooser::Result result = _directions.handleKey(msg);
	if (result == DirectionChooser::PENDING)
		return false;
	if (result == DirectionChooser::CANCELLED)
		return true;

	const int slot = _players.slot();
	int dx = 0, dy = 0;
	switch (_directions.direction()) {
	case DIR_NORTH: dy = -1; break;
	case DIR_SOUTH: dy = 1; break;
	case DIR_EAST: dx = 1; break;
	default: dx = -1; break;
	}

	ChestLogic chest;
	bool stolen = false;

	// The chest has to be a couple of steps beyond the counter beside the party
	if (chest.evadesTrap(slot)) {
		byte counter = _G(map).cell(save._posX + dx, save._posY + dy);

		if (counter >= FIRST_COUNTER_CELL && counter <= LAST_COUNTER_CELL) {
			int x = save._posX + dx * 3, y = save._posY + dy * 3;

			if (_G(map).cell(x, y) == CELL_CHEST) {
				_G(map).setCell(x, y, CELL_FLOOR);
				chest.loot(slot);
				stolen = true;
			}
		}
	}

	if (stolen)
		return true;

	if (Graphics::Views::g_events->getRandomNumber(254) & 3) {
		_G(messages).print("Failed!\n");
		return true;
	}

	// Caught in the act, the guards turn hostile
	Data::Creatures &creatures = _G(map)._creatures;
	for (int i = 0; i < Data::CREATURE_COUNT; ++i) {
		if (creatures._tile[i] == CELL_GUARD)
			creatures._flags[i] = FLAGS_HOSTILE;
	}

	_G(messages).print("Watch out!\n");
	g_engine->playSoundEffect(0xFA);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
