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

#include "mm/mm1/views_enh/interactions/giant.h"
#include "mm/mm1/maps/map30.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Giant::Giant() : Interaction("Giant", 8) {
	_title = STRING["maps.emap30.giant_title"];
	_animated = false;
}

bool Giant::msgFocus(const FocusMessage &msg) {
	PartyView::msgFocus(msg);

	_charSelected = false;
	addText(STRING["maps.map30.giant"]);
	return true;
}

void Giant::charSwitched(Character *priorChar) {
	Interaction::charSwitched(priorChar);
	if (_charSelected)
		return;
	_charSelected = true;

	MM1::Maps::Map30 &map = *static_cast<MM1::Maps::Map30 *>(g_maps->_currentMap);
	Common::String line = map.worthiness();
	addText(line);
	Sound::sound(SOUND_2);

	delaySeconds(5);
	redraw();
}

void Giant::timeout() {
	close();
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
