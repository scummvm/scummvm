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

#include "ultima/ultima1/views/dungeon_map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

bool DungeonMap::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_GAMEPLAY);
	return Dialog::msgFocus(msg);
}

bool DungeonMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Dialog::msgUnfocus(msg);
}

void DungeonMap::draw() {
	Dialog::draw();
	auto s = getSurface();

	// Write title showing the dungeon level
	s.writeString(Point(15, 0), "\x10 Level %2d \x11", _G(savegame)._dungeonLevel);

	// TODO: Dungeon view rendering
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
