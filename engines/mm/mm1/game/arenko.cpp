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

#include "mm/mm1/game/arenko.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace Game {

void Arenko::giveGold() {
	g_globals->_treasure.setGold((getRandomNumber(8) + 8) * 256);
	g_maps->clearSpecial();
	g_events->addAction(KEYBIND_SEARCH);
}

void Arenko::giveGems() {
	g_globals->_treasure.setGems(getRandomNumber(50) + 200);
	g_maps->clearSpecial();
	g_events->addAction(KEYBIND_SEARCH);
}

void Arenko::giveItem() {
	g_globals->_treasure._items[2] = getRandomNumber(22) + 196;
	g_maps->clearSpecial();
	g_events->addAction(KEYBIND_SEARCH);
}

} // namespace Game
} // namespace MM1
} // namespace MM
