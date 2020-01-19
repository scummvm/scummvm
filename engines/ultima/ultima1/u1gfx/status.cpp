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

#include "ultima/ultima1/u1gfx/status.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

EMPTY_MESSAGE_MAP(Status, Shared::Gfx::VisualItem);

bool Status::isDirty() const {
	if (_isDirty)
		return true;

	const Ultima1Game *game = static_cast<const Ultima1Game *>(getGame());
	const Shared::Character &c = *game->_party._currentCharacter;

	return c._hitPoints != _hitPoints || c._food != _food || c._experience != _experience || c._coins != _coins;
}

void Status::draw() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	const Shared::Character &c = *game->_party._currentCharacter;

	// Copy over the character fields
	_hitPoints = c._hitPoints;
	_food = c._food;
	_experience = c._experience;
	_coins = c._coins;

	// Clear the status area
	Shared::Gfx::VisualSurface s = getSurface();
	s.clear();

	// Iterate through displaying the values
	const uint *vals[4] = { &_hitPoints, &_food, &_experience, &_coins };
	int count = game->isVGA() ? 3 : 4;

	for (int idx = 0; idx < count; ++idx) {
		// Write header
		s.writeString(game->_res->STATUS_TEXT[idx], TextPoint(0, idx), game->_textColor, game->_bgColor);

		uint value = MIN(*vals[idx], (uint)9999);
		s.writeString(Common::String::format("%4u", value), TextPoint(5, idx), game->_textColor, game->_bgColor);
	}

	_isDirty = false;
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
