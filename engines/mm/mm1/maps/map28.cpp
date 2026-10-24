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

#include "mm/mm1/maps/map28.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/data/locations.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 63
#define VAL2 64

namespace {

enum ArenkoStorage {
	ARENKO_STATE_FIRST_OFFSET = 15 * MAP_W + 11,
	ARENKO_STATE_CELL_COUNT = 5,
	ARENKO_STATE_BIT = 0x80,
	ARENKO_REWARDED_STATE = 21
};

} // namespace

void Map28::load() {
	Map::load();
	restoreArenkoQuest();
	// Tree marks occupy only the western tree area, not the state cells.
	for (int i = 0; i < MAP_SIZE; ++i) {
		if (i % MAP_W < 7 && (_visited[i] & ARENKO_STATE_BIT) &&
				i != _data[51] && i != _data[52] && i != _data[53])
			_states[i] &= ~CELL_SPECIAL;
	}
}

byte Map28::arenkoStoredState() const {
	byte state = 0;
	for (int i = 0; i < ARENKO_STATE_CELL_COUNT; ++i) {
		if (_visited[ARENKO_STATE_FIRST_OFFSET + i] & ARENKO_STATE_BIT)
			state |= 1 << i;
	}
	return state;
}

void Map28::setArenkoStoredState(byte state) {
	for (int i = 0; i < ARENKO_STATE_CELL_COUNT; ++i) {
		byte &value = _visited[ARENKO_STATE_FIRST_OFFSET + i];
		value &= ~ARENKO_STATE_BIT;
		if (state & (1 << i))
			value |= ARENKO_STATE_BIT;
	}
}

void Map28::restoreArenkoQuest() {
	const byte state = arenkoStoredState();
	if (!state)
		return;

	_data[VAL2] = 1;
	_data[VAL1] = MIN((int)state - 1, 19);
	if (state >= ARENKO_REWARDED_STATE && _data[51] < MAP_SIZE)
		_states[_data[51]] &= ~CELL_SPECIAL;
}

bool Map28::arenkoQuestActive() const {
	return arenkoStoredState() != 0;
}

bool Map28::arenkoQuestCompleted() const {
	return arenkoStoredState() >= ARENKO_REWARDED_STATE;
}

int Map28::arenkoClimbs() const {
	const byte state = arenkoStoredState();
	return state ? MIN((int)state - 1, 19) : 0;
}

void Map28::startArenkoQuest() {
	if (!arenkoStoredState())
		setArenkoStoredState(1);
	_data[VAL1] = arenkoClimbs();
	_data[VAL2] = 1;
}

void Map28::recordArenkoClimb() {
	if (_visited[g_maps->_mapOffset] & ARENKO_STATE_BIT)
		return;
	_visited[g_maps->_mapOffset] |= ARENKO_STATE_BIT;
	if (arenkoQuestCompleted())
		return;
	if (!arenkoQuestActive())
		startArenkoQuest();

	const int climbed = MIN(arenkoClimbs() + 1, 19);
	_data[VAL1] = climbed;
	_data[VAL2] = 1;
	setArenkoStoredState(climbed + 1);
}

void Map28::completeArenkoQuest() {
	_data[VAL1] = 19;
	_data[VAL2] = 1;
	setArenkoStoredState(ARENKO_REWARDED_STATE);
}

void Map28::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 3; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[54 + i]) {			
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (g_maps->_mapPos.x >= 7) {
		g_maps->clearSpecial();
		g_globals->_encounters.execute();

	} else {
		send(SoundMessage(
			"Climb tree (Y/N)?",
			[]() {
				Map28 &map = *static_cast<Map28 *>(g_maps->_currentMap);

				if (!map[VAL2]) {
					g_events->send(SoundMessage("Nothing here."));

				} else {
					g_maps->clearSpecial();
					map.recordArenkoClimb();

					int val = g_events->getRandomNumber(10);
					switch (val) {
					case 1:
						g_events->send(SoundMessage("Nothing here."));
						break;

					case 2:
						for (uint i = 0; i < g_globals->_party.size(); ++i) {
							Character &c = g_globals->_party[i];
							c._food = MIN((int)c._food + 5, MAX_FOOD);
						}
						g_events->send(SoundMessage("+5 food"));
						break;

					case 3:
						map.setCondition(POISONED);
						map.reduceHpBase();
						g_events->send(SoundMessage("Poison thorns"));
						break;

					case 4:
						if (g_globals->_activeSpells._s.levitate) {
							g_events->send(SoundMessage("Nothing here."));
						} else {
							map.reduceHpBase();
							map.reduceHpBase();
							g_events->send(SoundMessage("You fell"));
						}
						break;

					case 5:
						g_events->send(SoundMessage("Lightning"));
						break;

					case 6:
						map.setCondition(DISEASED);
						map.reduceHpBase();
						g_events->send(SoundMessage("Lightning"));
						break;

					case 7:
						if (!g_globals->_activeSpells._s.psychic_protection) {
							for (uint i = 0; i < g_globals->_party.size(); ++i) {
								Character &c = g_globals->_party[i];
								c._sp._current = 0;
							}
						}

						g_events->send(SoundMessage("Flash"));
						break;

					case 8:
						g_events->send(SoundMessage("Poof!"));
						g_maps->_mapPos = Common::Point(
							g_events->getRandomNumber(15),
							g_events->getRandomNumber(15));
						map.updateGame();
						break;

					default:
						g_globals->_activeSpells._s.cursed = val;
						g_events->send(SoundMessage("Cursed"));
						break;
					}
				}
			}
		));
	}
}

void Map28::special00() {
	g_events->addView("Arenko");
}

void Map28::special01() {
	visitedExit();

	send(SoundMessage(
		"A cave, enter (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(8, 8);
			g_maps->changeMapFromExit(0x601, 1);
		}
	));
}

void Map28::special02() {
	send(SoundMessage("Roadsign: Algary S.9"));
}

void Map28::setCondition(byte condition) {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (getRandomNumber(3) == 2 && !(c._condition & BAD_CONDITION))
			c._condition = condition;
	}
}

void Map28::reduceHpBase() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._hpCurrent /= 2;
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
