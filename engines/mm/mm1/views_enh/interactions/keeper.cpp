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

#include "mm/mm1/views_enh/interactions/keeper.h"
#include "mm/mm1/maps/map54.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Keeper::Keeper() : Interaction("Keeper", 10) {
	_title = STRING["maps.emap54.keeper"];
	_animated = false;
}

bool Keeper::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);

	_pageNum = 0;
	addText(STRING["maps.map54.keeper1"]);
	return true;
}

void Keeper::viewAction() {
	MM1::Maps::Map54 &map = *static_cast<MM1::Maps::Map54 *>(g_maps->_currentMap);

	switch (++_pageNum) {
	case 1:
		addText(STRING["maps.emap54.keeper2"]);
		redraw();
		break;

	case 2: {
		uint32 perfTotal;
		_isWorthy = map.isWorthy(perfTotal);

		addText(Common::String::format(
			STRING["maps.emap54.keeper3"].c_str(), perfTotal));
		redraw();
		break;
	}

	case 3:
		addText(STRING[_isWorthy ? "maps.emap54.keeper5" :
			"maps.emap54.keeper4"].c_str());
		redraw();
		break;

	case 4:
		if (!_isWorthy) {
			leave();
		} else {
			addText(STRING["maps.map54.keeper6"]);
			redraw();
		}
		break;

	default:
		leave();
		break;
	}
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
