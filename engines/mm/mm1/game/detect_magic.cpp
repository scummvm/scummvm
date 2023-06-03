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

#include "mm/mm1/game/detect_magic.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

void DetectMagic::getMagicStrings() {
	Inventory &inv = g_globals->_currCharacter->_backpack;
	for (uint i = 0; i < inv.size(); ++i) {
		Common::String s;

		int itemId = inv[i]._id;
		bool flag = false;
		if (itemId < 12)
			flag = false;
		else if (itemId < 61)
			flag = true;
		else if (itemId < 66)
			flag = false;
		else if (itemId < 86)
			flag = true;
		else if (itemId < 93)
			flag = false;
		else if (itemId < 121)
			flag = true;
		else if (itemId < 128)
			flag = false;
		else if (itemId < 156)
			flag = true;
		else if (itemId < 158)
			flag = false;
		else if (itemId < 255)
			flag = true;
		else
			flag = false;

		if (flag) {
			_strings[i] = Common::String::format("Y (%d)", inv[i]._charges);
		} else {
			_strings[i] = "N";
		}
	}
}

} // namespace Game
} // namespace MM1
} // namespace MM
