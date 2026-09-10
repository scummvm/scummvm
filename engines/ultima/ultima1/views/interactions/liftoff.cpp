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

#include "ultima/ultima1/views/interactions/liftoff.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

bool LiftOff::msgFocus(const FocusMessage &msg) {
	setBounds(Common::Rect(0, 0, 0, 0));

	_counter = 10;
	showCounter();
	delaySeconds(1);
	return true;
}

void LiftOff::showCounter() {
	writeString("%d", _counter);
	if (_counter != 1)
		writeString("..");
}

void LiftOff::timeout() {
	if (--_counter == 0) {
		finish();
	} else {
		showCounter();
		delaySeconds(1);
	}
}

bool LiftOff::msgAction(const ActionMessage &msg) {
	skip();
	return true;
}

bool LiftOff::msgKeypress(const KeypressMessage &msg) {
	skip();
	return true;
}

void LiftOff::skip() {
	cancelDelay();
	finish();
}

void LiftOff::finish() {
	writeString("\n");
	writeString("Thou hast lifted off!\n");

	close();
	_G(map).load(Data::MAP_SPACE);
	_G(logic)->entering();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
