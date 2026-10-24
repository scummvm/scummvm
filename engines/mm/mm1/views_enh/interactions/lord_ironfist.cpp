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

#include "mm/mm1/views_enh/interactions/lord_ironfist.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/maps/map43.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

LordIronfist::LordIronfist() : Interaction("LordIronfist", 18) {
	_title = "Lord Ironfist";
}

bool LordIronfist::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);
	clearButtons();

	if (_mode == ACCEPTING) {
		_mode = ACCEPTED_QUEST;

		addText(IRONFIST_QUESTS[g_globals->_party[0]._quest - 1]);

	} else {
		const Character &c = g_globals->_party[0];
		_mode = c._quest ? ACTIVE_QUEST : CAN_ACCEPT;


		if (_mode == CAN_ACCEPT) {
			Sound::sound(SOUND_2);

			addText("Lord Ironfist speaks: \"Your services are needed!\" Accept (Y/N)?");

			addButton("Accept", 'Y');
			addButton("Decline", 'N');

		} else {
			// There's an active quest, so check for completion
			MM1::Maps::Map43 &map = *static_cast<MM1::Maps::Map43 *>(g_maps->_currentMap);
			int questNum = g_globals->_party[0]._quest;
			Common::String line;

			if (questNum < 8)
				line = map.checkQuestComplete();
			else
				line = "\"Sorry, but since you are currently quested, i can't engage your services.\"";

			g_maps->_mapPos.x++;

			addText(Common::String::format("Lord Ironfist speaks: %s",
				line.c_str()
			));
		}
	}

	return true;
}

bool LordIronfist::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map43 &map = *static_cast<MM1::Maps::Map43 *>(g_maps->_currentMap);

	if (_mode == CAN_ACCEPT) {
		if (msg.keycode == Common::KEYCODE_y) {
			// Accept the quest
			close();
			map.acceptQuest();

			// Reshow the view to display what the quest is
			_mode = ACCEPTING;
			addView();

		} else if (msg.keycode == Common::KEYCODE_n) {
			close();
		}
	} else {
		close();
	}

	return true;
}

void LordIronfist::viewAction() {
	if (_mode != CAN_ACCEPT)
		close();
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
