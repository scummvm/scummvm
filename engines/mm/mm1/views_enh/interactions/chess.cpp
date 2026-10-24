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

#include "mm/mm1/views_enh/interactions/chess.h"
#include "mm/mm1/maps/map29.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Chess::Chess() : InteractionQuery("Chess", 23, 13) {
	_title = "Og";
	addText("Painted on these grounds is a black &\n"
		"white checkered motif covered with idols similar to the ones you carry. A large being "
		"approaches... Og speaks, \"Queen to queens level 3\"");
}

void Chess::answerEntered() {
	MM1::Maps::Map29 &map = *static_cast<MM1::Maps::Map29 *>(g_maps->_currentMap);
	map.chessAnswer(_answer);
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
