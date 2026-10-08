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

#include "ultima/ultima3/views/interactions/look.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

bool Look::keypress(const KeypressMessage &msg) {
	DirectionChooser::Result result = _chooser.handleKey(msg);
	if (result == DirectionChooser::PENDING)
		return false;
	if (result == DirectionChooser::CANCELLED)
		return true;

	const Data::Savegame &save = _G(savegame);
	int x = save._posX, y = save._posY;
	switch (_chooser.direction()) {
	case DIR_NORTH: --y; break;
	case DIR_SOUTH: ++y; break;
	case DIR_EAST: ++x; break;
	default: --x; break;
	}

	_G(messages).print("->");
	_G(messages).print(Data::NAMES[_G(map).tile(x, y)]);
	_G(messages).print("\n");
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
