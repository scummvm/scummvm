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

#include "mm/mm1/views_enh/interactions/statue.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Statue::Statue() : Interaction("Statue", 33) {
	_title = "Statue";
	_animated = false;
}

bool Statue::msgGame(const GameMessage &msg) {
	if (msg._name == "STATUE") {
		_pageNum = 0;
		_statueNum = msg._value;
		addView(this);
		return true;
	}

	return false;
}

bool Statue::msgFocus(const FocusMessage &msg) {
	Common::String statueType = STATUE_NAMES[_statueNum];
	Common::String str = Common::String::format("On this stone statue of %s. A plaque reads...",
		statueType.c_str());

	addText(str);
	return true;
}

void Statue::viewAction() {
	switch (++_pageNum) {
	case 1:
		addText(STATUE_MESSAGES[_statueNum]);
		redraw();
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
