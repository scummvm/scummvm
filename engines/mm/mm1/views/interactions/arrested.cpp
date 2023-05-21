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

#include "mm/mm1/views/interactions/arrested.h"
#include "mm/mm1/maps/map04.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Arrested::Arrested() : TextView("Arrested") {
	_bounds = getLineBounds(20, 24);
}

bool Arrested::msgFocus(const FocusMessage &msg) {
	return TextView::msgFocus(msg);
}

void Arrested::draw() {
	clearSurface();
	Sound::sound(SOUND_2);

	writeString(0, 1, STRING["maps.map04.guards"]);
}

bool Arrested::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		attack();
		break;
	case Common::KEYCODE_b:
		bribe();
		break;
	case Common::KEYCODE_c:
		run();
		break;
	case Common::KEYCODE_d:
		surrender();
		break;
	default:
		break;
	}

	return true;
}

void Arrested::surrender(int numYears) {
	Game::Arrested::surrender(numYears);

	// Display sentence
	clearSurface();
	Sound::sound(SOUND_2);
	writeString(0, 1, STRING["maps.map04.sentence"]);
	writeNumber(numYears);

	delaySeconds(3);
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
