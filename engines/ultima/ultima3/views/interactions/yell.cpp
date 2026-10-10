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

#include "ultima/ultima3/views/interactions/yell.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr int WORD_LENGTH = 9;
constexpr byte MARK_OF_SNAKE = 0x40;

// The place by the water where the right word carries the party across
constexpr byte CROSSING_X = 0x0A;
constexpr byte CROSSING_NEAR_Y = 0x38;
constexpr byte CROSSING_FAR_Y = 0x3B;

bool Yell::keypress(const KeypressMessage &msg) {
	Data::Savegame &save = _G(savegame);

	if (!_asking) {
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

		_G(messages).print("Word: ");
		_text.setup(WORD_LENGTH);
		_asking = true;
		return false;
	}

	if (!_text.handleKey(msg))
		return false;

	_G(messages).print("\n");

	// Only the right word, said by someone with the Mark of Snake at the
	// right place on the world map, does anything
	const Data::RosterEntry &e = save.partyMember(_players.slot());
	const bool atCrossing = save._location == Data::LOCATION_SOSARIA && save._posX == CROSSING_X &&
		(save._posY == CROSSING_NEAR_Y || save._posY == CROSSING_FAR_Y);

	if (_text.word() == "EVOCARE" && (e._marksAndCards & MARK_OF_SNAKE) && atCrossing) {
		save._posY ^= 3;
		_G(effects).flashViewport();
		g_engine->playSoundEffect(0xFD, 0xC0, 0x40);
	} else {
		_G(messages).print("No effect!\n");
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
