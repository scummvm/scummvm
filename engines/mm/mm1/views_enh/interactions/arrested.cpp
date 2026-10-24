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

#include "mm/mm1/views_enh/interactions/arrested.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Arrested::Arrested() : Interaction("Arrested", 26), Game::Arrested() {
	_title = "Town Guards";
}

bool Arrested::msgFocus(const FocusMessage &msg) {
	addText("Town guards: you're under arrest!");
	clearButtons();
	addButton("\x01""37Attack", 'A');
	addButton("\x01""37Bribe", 'B');
	addButton("\x01""37Run", 'R');
	addButton("\x01""37Surrender", 'S');

	return true;
}

bool Arrested::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_a:
		attack();
		break;
	case Common::KEYCODE_b:
		bribe();
		break;
	case Common::KEYCODE_r:
		run();
		break;
	case Common::KEYCODE_s:
		surrender();
		break;
	default:
		return Interaction::msgKeypress(msg);
	}

	return true;
}

void Arrested::surrender(int numYears) {
	Game::Arrested::surrender(numYears);

	// Display sentence
	Common::String str = Common::String::format(
		"High court sentence: %d years", numYears);

	SoundMessage msg(str);
	msg._delaySeconds = 3;
	send(msg);
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
