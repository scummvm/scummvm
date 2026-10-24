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
	_title = "Keeper";
	_animated = false;
}

bool Keeper::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);

	_pageNum = 0;
	addText("A metallic panel slides open, revealing a mysterious man in a white coat... (Press Space "
		"to communicate)");
	return true;
}

void Keeper::viewAction() {
	MM1::Maps::Map54 &map = *static_cast<MM1::Maps::Map54 *>(g_maps->_currentMap);

	switch (++_pageNum) {
	case 1:
		addText("In a serene voice, the data keeper says, \"Welcome to your inner sanctum Varnlings! I am "
			"very pleased that you've made it this far. You are to be commended. I've been monitoring "
			"your progress.\"");
		redraw();
		break;

	case 2: {
		uint32 perfTotal;
		_isWorthy = map.isWorthy(perfTotal);

		addText(Common::String::format(
			"Turning to the strange mechanical device, he inserts a flat object into a slot. Your "
				"party's current performance total=%u.", perfTotal));
		redraw();
		break;
	}

	case 3:
		addText(_isWorthy ?
			"Excellent rating! this is a rare occasion, for only a privileged few are given the "
				"opportunity for transfer to another V.A.R.N. (Vehicular Astropod Research Nacelle)." :
			"\"You are not yet worthy for transfer. Return after defeating the imposter.\"");
		redraw();
		break;

	case 4:
		if (!_isWorthy) {
			leave();
			map.sorpigalInn();
		} else {
			addText("Return now to the\n"
				"Inn of Sorpigal for rest and celebration then continue on to your new assignment at the "
				"Gates To Another World...\" (+50000 exp for your accomplishment!)");
			redraw();
		}
		break;

	default:
		leave();
		map.sorpigalInn();
		break;
	}
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
