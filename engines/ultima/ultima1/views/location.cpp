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

#include "ultima/ultima1/views/location.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

Location::Location() : Dialog("Location"), _map(this), _commands("LocationCommands", this), _stats(this) {
	_map.setBounds(TextRect(1, 1, 38, 19));
	_commands.setBounds(TextRect(0, 20, 29, 24));
	_stats.setBounds(TextRect(31, 21, 39, 24));
}

bool Location::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_OVERWORLD);
	return Dialog::msgFocus(msg);
}

bool Location::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Dialog::msgUnfocus(msg);
}

void Location::draw() {
	auto s = getSurface();
	s.clear();
	drawFrame();
	drawGameDividers();
}

bool Location::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		if (msg._stringValue == "MAP")
			_map.redraw();
		if (msg._stringValue == "STATS")
			_stats.redraw();
		return true;
	}

	return Dialog::msgGame(msg);
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
