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

#include "mm/mm1/views/locations/statue.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

#define PAGE1_SECONDS 10

Statue::Statue() : Location("Statue") {
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

bool Statue::msgKeypress(const KeypressMessage &msg) {
	if (!endDelay())
		leave();

	return true;
}

bool Statue::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		leave();
		return true;
	}

	return false;
}

void Statue::draw() {
	switch (_pageNum) {
	case 0:
		_bounds = getLineBounds(20, 24);
		clearSurface();

		writeString(0, 1, STRING["dialogs.statues.stone"]);
		writeString(STRING[Common::String::format(
			"dialogs.statues.names.%d", _statueNum)]);
		newLine();
		writeString(STRING["dialogs.statues.plaque"]);

		++_pageNum;
		delaySeconds(PAGE1_SECONDS);
		redraw();
		break;

	case 1:
		_bounds = getLineBounds(17, 24);
		clearSurface();

		writeString(0, 0, STRING[Common::String::format(
			"dialogs.statues.messages.%d", _statueNum)]);

		Sound::sound(SOUND_2);
		++_pageNum;
		redraw();
		break;

	default:
		leave();
		break;
	}
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
