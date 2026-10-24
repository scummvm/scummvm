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

#include "mm/mm1/views_enh/interactions/arenko.h"
#include "mm/mm1/maps/map28.h"
#include "mm/mm1/globals.h"

#define VAL1 63
#define VAL2 64

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Arenko::Arenko() : Interaction("Arenko", 9) {
	_title = "Arenko";
}

bool Arenko::msgFocus(const FocusMessage &msg) {
	Maps::Map28 &map = *static_cast<Maps::Map28 *>(g_maps->_currentMap);
	clearButtons();

	if (!map.arenkoQuestActive()) {
		addText("I am Arenko Guire and this is my grove. Climb all the trees, without leaving the area, "
			"and return.");
		map.startArenkoQuest();

	} else if (map[VAL1] < 19) {
		addText("Keep climbing.");

	} else {
		addText("Well done! What is your pleasure?");
		addButton("Gold", 'A');
		addButton("Gems", 'B');
		addButton("Item", 'C');
	}

	return true;
}

void Arenko::viewAction() {
	if (_buttons.empty()) {
		close();
	}
}

bool Arenko::msgKeypress(const KeypressMessage &msg) {
	if (!_buttons.empty()) {
		switch (msg.keycode) {
		case Common::KEYCODE_a:
			close();
			giveGold();
			return true;
		case Common::KEYCODE_b:
			close();
			giveGems();
			return true;
		case Common::KEYCODE_c:
			close();
			giveItem();
			return true;
		default:
			break;
		}
	}

	return Interaction::msgKeypress(msg);
}

bool Arenko::msgAction(const ActionMessage &msg) {
	if (_buttons.empty())
		return Interaction::msgAction(msg);
	return true;
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
