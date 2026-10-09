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

#include "ultima/ultima3/views/interactions/get_chest.h"
#include "ultima/ultima3/logic/chest_logic.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr byte FIRST_CHEST_CELL = 0x24;
constexpr byte LAST_CHEST_CELL = 0x27;
constexpr byte CELL_FLOOR = 0x20;

bool GetChest::keypress(const KeypressMessage &msg) {
	PlayerChooser::Result result = _chooser.handleKey(msg);
	if (result == PlayerChooser::PENDING)
		return false;
	if (result == PlayerChooser::CANCELLED)
		return true;

	const int slot = _chooser.slot();
	Data::Savegame &save = _G(savegame);

	if (!save.partyMember(slot).isAlive()) {
		_G(messages).print("Incapacitated!\n");
		g_engine->playSoundEffect(0xFF);
		return true;
	}

	byte cell = _G(map).cell(save._posX, save._posY);
	if (cell < FIRST_CHEST_CELL || cell > LAST_CHEST_CELL) {
		_G(messages).print("Not Here!\n");
		g_engine->playSoundEffect(0xFF);
		return true;
	}

	// The chest goes, leaving the ground it was on
	byte ground = (cell & 3) << 2;
	_G(map).setCell(save._posX, save._posY, ground ? ground : CELL_FLOOR);

	ChestLogic chest;
	chest.open(slot);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
