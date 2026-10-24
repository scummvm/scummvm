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

#include "mm/mm1/views_enh/interactions/hacker.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/maps/map36.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Hacker::Hacker() : Interaction("Hacker", 35) {
	_title = "Lord Hacker";
}

bool Hacker::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);
	return true;
}

bool Hacker::msgGame(const GameMessage &msg) {
	if (msg._name != "DISPLAY")
		return false;

	g_globals->_currCharacter = &g_globals->_party[0];
	_mode = g_globals->_currCharacter->_quest ? ACTIVE_QUEST : CAN_ACCEPT;

	if (_mode == CAN_ACCEPT)
		Sound::sound(SOUND_2);
	addView();
	clearButtons();

	if (_mode == CAN_ACCEPT) {
		addText("\"Your services are needed!\" Accept (Y/N)?");
		addButton("Accept", 'Y');
		addButton("Decline", 'N');

	} else {
		// There's an active quest, so check for completion
		MM1::Maps::Map36 &map = *static_cast<MM1::Maps::Map36 *>(g_maps->_currentMap);
		int questNum = g_globals->_party[0]._quest;
		Common::String line;

		if (questNum >= 15 && questNum <= 21)
			line = map.checkQuestComplete();
		else
			line = "\"Sorry, you're already quested.\"";

		g_maps->_mapPos.x--;
		map.redrawGame();

		addText(line);
	}

	return true;
}

bool Hacker::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map36 &map = *static_cast<MM1::Maps::Map36 *>(g_maps->_currentMap);

	if (_mode == CAN_ACCEPT) {
		if (msg.keycode == Common::KEYCODE_y) {
			map.acceptQuest();

			// Hacker's post-quest-7 branch is a punishment, not an eighth
			// quest. acceptQuest() intentionally leaves _quest at zero there.
			// Do not fall through to the normal quest ingredient lookup with
			// an invalid negative index; Classic already guards this case.
			if (!g_globals->_party[0]._quest) {
				close();
				return true;
			}

			_mode = ACCEPTED_QUEST;
			clearButtons();
			Common::String line = Common::String::format("Bring me %s",
				INGREDIENTS[g_globals->_party[0]._quest - 15]
			);
			addText(line);

			redraw();
			return true;

		} else if (msg.keycode == Common::KEYCODE_n) {
			close();
			return true;
		}
	}

	return false;
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
