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

#include "ultima/ultima1/gfx/status.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/game_state.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

EMPTY_MESSAGE_MAP(Status, Shared::Gfx::VisualItem);

void Status::draw() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getRoot());
	Shared::GameState *gameState = getGameState();
	Shared::Gfx::VisualSurface s = getSurface();
	s.clear();

	// Iterate through displaying the four values
	Shared::Character &c = gameState->_characters.front();
	const uint *vals[4] = { &c._hitPoints, &c._food, &c._experience, &c._coins };

	for (int idx = 0; idx < 4; ++idx) {
		// Write header
		s.writeString(game->_res->STATUS_TEXT[idx], TextPoint(0, idx), game->_textColor);

		uint value = MIN(*vals[idx], (uint)9999);
		s.writeString(Common::String::format("%4u", value), TextPoint(5, idx), game->_textColor);
	}
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
