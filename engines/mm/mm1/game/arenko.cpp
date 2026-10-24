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
#include "mm/mm1/maps/map28.h"


namespace MM {
namespace MM1 {
namespace Game {

void Arenko::giveGold() {
	Maps::Map28 &map = *static_cast<Maps::Map28 *>(g_maps->_currentMap);
	if (map.arenkoQuestCompleted())
		return;

	g_globals->_treasure.setGold((getRandomNumber(8) + 8) * 256);
	map.completeArenkoQuest();
	g_maps->clearSpecial();
	g_events->addAction(KEYBIND_SEARCH);
}

void Arenko::giveGems() {
	Maps::Map28 &map = *static_cast<Maps::Map28 *>(g_maps->_currentMap);
	if (map.arenkoQuestCompleted())
		return;

	g_globals->_treasure.setGems(getRandomNumber(50) + 200);
	map.completeArenkoQuest();
	g_maps->clearSpecial();
	g_events->addAction(KEYBIND_SEARCH);
}

void Arenko::giveItem() {
	Maps::Map28 &map = *static_cast<Maps::Map28 *>(g_maps->_currentMap);
	if (map.arenkoQuestCompleted())
		return;

	g_globals->_treasure._items[2] = getRandomNumber(22) + 196;
	map.completeArenkoQuest();
	g_maps->clearSpecial();
	g_events->addAction(KEYBIND_SEARCH);
}

} // namespace Game
} // namespace MM1
} // namespace MM
