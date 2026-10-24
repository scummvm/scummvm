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

#include "mm/mm1/views/interactions/keeper.h"
#include "mm/mm1/maps/map54.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Keeper::Keeper() : TextView("Keeper") {
	_bounds = getLineBounds(20, 24);
}

bool Keeper::msgFocus(const FocusMessage &msg) {
	_pageNum = 0;
	Sound::sound(SOUND_2);

	return TextView::msgFocus(msg);
}

void Keeper::draw() {
	MM1::Maps::Map54 &map = *static_cast<MM1::Maps::Map54 *>(g_maps->_currentMap);
	uint32 perfTotal;
	bool isWorthy;

	switch (_pageNum) {
	case 0:
		_bounds = getLineBounds(20, 24);
		clearSurface();
		writeString(0, 1, "A metallic panel slides open, revealing\n"
			"a mysterious man in a white coat...\n"
			"(Press Space to communicate)");
		break;

	case 1:
		_bounds = getLineBounds(0, 24);
		isWorthy = map.isWorthy(perfTotal);

		clearSurface();
		writeString(10, 0, ":::Inner Sanctum:::");
		writeString(0, 2, Common::String::format(
			"In a serene voice, the data keeper says,\n"
				"\"Welcome to your inner sanctum\n"
				"Varnlings! I am very pleased that you've\n"
				"made it this far. You are to be\n"
				"commended. I've been monitoring your\n"
				"progress.\" Turning to the strange\n"
				"mechanical device, he inserts a flat\n"
				"object into a slot. Your party's current\n"
				"performance total=%u", perfTotal));
		writeString(0, 3, isWorthy ?
			"Excellent rating! This is a rare        occasion, for only a privileged few are given the "
				"opportunity for transfer to\n"
				"another V.A.R.N. (Vehicular Astropod\n"
				"Research Nacelle). Return now to the\n"
				"Inn of Sorpigal for rest and celebration\n"
				"then continue on to your new assignment at the Gates To Another World...\"       (+50000 "
				"exp for your accomplishment!)\n"
				"----------------------------------------\n"
				"Please send your performance total to\n"
				"New World Computing, Inc.\n"
				"  P. O. Box 2068  Van Nuys, CA  91404" :
			"\"You are not yet worthy for transfer.   Return after defeating the imposter.\"");
		break;

	default:
		break;
	}
}

bool Keeper::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map54 &map = *static_cast<MM1::Maps::Map54 *>(g_maps->_currentMap);

	if (++_pageNum == 2) {
		close();
		map.sorpigalInn();
	} else {
		redraw();
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
