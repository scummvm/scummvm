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

#include "mm/mm1/events.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/game/rest.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/maps/maps.h"

namespace MM {
namespace MM1 {
namespace Game {

void Rest::check() {
	int dangerRate = g_maps->_currentMap->dataByte(44);
	if (!dangerRate ||
		g_engine->getRandomNumber(dangerRate) != dangerRate) {
		// Rest allowed
		execute();
	} else {
		// Choose a random character, and make everyone
		// but them be asleep
		uint awakeIndex = g_engine->getRandomNumber(
			g_globals->_party.size() - 1);
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			if (!(c._condition & BAD_CONDITION) && i != awakeIndex)
				c._condition |= ASLEEP;
		}

		// Start an encounter
		g_globals->_encounters.execute();
	}
}

void Rest::execute() {
	// Reset active spells
	Common::fill(&g_globals->_activeSpells._arr[0],
		&g_globals->_activeSpells._arr[ACTIVE_SPELLS_COUNT], 0);

	// Rest the characters of the party
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c.rest();
	}

	g_events->send(InfoMessage(
		STRING["dialogs.game.rest.rest_complete"]
	));
}

} // namespace Game
} // namespace MM1
} // namespace MM
