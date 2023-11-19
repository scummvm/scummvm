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

#include "mm/mm1/views_enh/interactions/scummvm.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

ScummVM::ScummVM() : Interaction("ScummVM", 38) {
	_title = STRING["maps.map55.title"];
	addText(STRING["maps.map55.message"]);
}

void ScummVM::viewAction() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._gold += 10000;
		c._gems = MIN((int)c._gems + 1000, 0xffff);
	}

	g_maps->_mapPos = Common::Point(8, 3);
	g_maps->changeMap(0x604, 1);
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
