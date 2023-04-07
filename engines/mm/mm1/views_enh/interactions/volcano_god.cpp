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

#include "mm/mm1/views_enh/interactions/volcano_god.h"
#include "mm/mm1/maps/map11.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

VolcanoGod::VolcanoGod() : InteractionQuery("VolcanoGod", 8, 10) {
	_title = STRING["maps.emap11.volcano_god"];
}

bool VolcanoGod::msgFocus(const FocusMessage &msg) {
	InteractionQuery::msgFocus(msg);
	_showEntry = false;
	setMode(CHOOSE_OPTION);
	return true;
}

bool VolcanoGod::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_a:
		challenge();
		break;
	case Common::KEYCODE_b:
		riddle();
		break;
	case Common::KEYCODE_c:
		clue();
		break;
	default:
		return InteractionQuery::msgKeypress(msg);
	}

	return true;
}

bool VolcanoGod::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		g_events->close();
		return true;
	} else {
		return InteractionQuery::msgAction(msg);
	}
}

void VolcanoGod::setMode(Mode newMode) {
	clearButtons();
	_mode = newMode;

	switch (_mode) {
	case CHOOSE_OPTION:
		addText(STRING["maps.emap11.god_text"]);
		addButton(STRING["maps.emap11.god_a"], 'A');
		addButton(STRING["maps.emap11.god_b"], 'B');
		addButton(STRING["maps.emap11.god_c"], 'C');
		break;
	case ENTER_RESPONSE:
		addText(STRING["maps.emap11.question"]);
		_showEntry = true;
		break;
	}

	redraw();
}


void VolcanoGod::challenge() {
	MM1::Maps::Map11 &map = *static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap);
	close();
	map.challenge();
}

void VolcanoGod::riddle() {
	Sound::sound(SOUND_2);
	setMode(ENTER_RESPONSE);
	redraw();
}

void VolcanoGod::clue() {
	MM1::Maps::Map11 &map = *static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap);
	close();
	map.clue();
}

void VolcanoGod::answerEntered() {
	MM1::Maps::Map11 &map = *static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap);
	map.riddleAnswer(_answer);
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
