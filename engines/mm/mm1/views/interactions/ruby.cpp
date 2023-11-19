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

#include "mm/mm1/views/interactions/ruby.h"
#include "mm/mm1/maps/map39.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Ruby::Ruby() : AnswerEntry("Ruby", Common::Point(14, 7), 12) {
	_bounds = getLineBounds(17, 24);
}

void Ruby::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map39.ruby1"]);
	AnswerEntry::draw();
}

void Ruby::answerEntered() {
	MM1::Maps::Map39 &map = *static_cast<MM1::Maps::Map39 *>(g_maps->_currentMap);
	close();
	map.riddleAnswered(_answer);
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
