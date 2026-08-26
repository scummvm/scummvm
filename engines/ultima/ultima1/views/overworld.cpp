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

#include "ultima/ultima1/views/overworld.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

Overworld::Overworld() : Dialog("Overworld"), _map(this), _commands(this), _stats(this) {
	_map.setBounds(TextRect(1, 1, 38, 19));
	_commands.setBounds(TextRect(0, 20, 29, 24));
	_stats.setBounds(TextRect(31, 21, 39, 24));
}

bool Overworld::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_OVERWORLD);
	return View::msgFocus(msg);
}

bool Overworld::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return View::msgUnfocus(msg);
}

void Overworld::draw() {
	auto s = getSurface();
	s.clear();
	drawFrame();
	drawGameDividers();
}

bool Overworld::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		_map.redraw();
		_stats.redraw();
		return true;
	}

	return false;
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
