/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/shared/gfx/status.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/core/game_state.h"

namespace Ultima {
namespace Shared {

EMPTY_MESSAGE_MAP(Status, Gfx::VisualItem);

void Status::draw() {
	Game *game = getRoot();
	GameState *gameState = getGameState();
	Gfx::VisualSurface s = getSurface();
	s.clear();

	// Write headers
	s.writeString("Hits:", TextPoint(0, 0), game->_textColor);
	s.writeString("Food:", TextPoint(0, 1), game->_textColor);
	s.writeString("Exp.:", TextPoint(0, 2), game->_textColor);
	s.writeString("Coin:", TextPoint(0, 3), game->_textColor);

	// Iterate through displaying the four values
	Character &c = gameState->_characters.front();
	const uint *vals[4] = { &c._hitPoints, &c._food, &c._experience, &c._coins };

	for (int idx = 0; idx < 4; ++idx) {
		uint value = MIN(*vals[idx], (uint)9999);
		s.writeString(Common::String::format("%4u", value), TextPoint(5, idx), game->_textColor);
	}
}

} // End of namespace Shared
} // End of namespace Ultima
