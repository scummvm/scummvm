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

#include "mm/mm1/maps/map06.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 446
#define VAL2 447
#define ENCOUNTER_COMPLETE 0x80
#define QUEST_ACTIVE_OFFSET (15 * MAP_W + 15)
#define QUEST_COMPLETE_OFFSET (15 * MAP_W + 13)

static const byte MONSTER_ID1[13] = {
	2, 1, 8, 9, 5, 9, 2, 7, 15, 4, 11, 1, 5
};
static const byte MONSTER_ID2[13] = {
	3, 4, 3, 4, 4, 3, 4, 3, 3, 4, 4, 3, 3
};

void Map06::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 27; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[78 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_data[VAL1]) {
		_data[VAL2]++;

		Common::Point &pos = g_maps->_mapPos;
		if (pos.y == 9) {
			if (pos.x == 13 || pos.x == 14)
				pos.x++;
			else
				pos.y++;
		} else if (pos.x < 6) {
			if (pos.y == 0) {
				if (pos.x == 0)
					pos.y++;
				else
					pos.x--;
			} else if (pos.y != 10 || pos.x == 3) {
				pos.y++;
			} else {
				pos.x++;
			}
		} else if (pos.x == 6) {
			if (pos.y == 0)
				pos.x--;
			else
				pos.y--;
		} else {
			pos.y++;
		}

		updateGame();
	} else {
		checkPartyDead();
	}
}

void Map06::special00() {
	visitedExit();
	send(SoundMessage(
		"Passage outside, exit (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(15, 11);
			g_maps->changeMapFromExit(0xa11, 2);
		}
	));
}

void Map06::special01() {
	visitedExit();
	send(SoundMessage(
		mapText("Shimmering blue and white portal,\nenter (Y/N)?"),
		[]() {
			g_maps->_mapPos = Common::Point(7, 0);
			g_maps->changeMapFromExit(0xc01, 1);
		}
	));
}

void Map06::special02() {
	if (_data[VAL1]) {
		g_maps->_mapPos.x--;
		slide();
	} else {
		checkPartyDead();
	}
}

void Map06::special03() {
	if (_data[VAL1]) {
		g_maps->_mapPos.y++;
		slide();
	} else {
		checkPartyDead();
	}
}

void Map06::special04() {
	if (_data[VAL2]) {
		_data[VAL2] = 0;
		send(SoundMessage(0, 1, "Splash! A pool of acid!"));

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._hpCurrent = MAX((int)c._hpCurrent - 15, 0);
			if (!c._hpCurrent) {
				if (!(c._condition & BAD_CONDITION))
					c._condition = UNCONSCIOUS;
			}
		}
	}
}

void Map06::special06() {
	send(SoundMessage(
		"Button on the wall, press it (Y/N)?",
		[]() {
			Map06 &map = *static_cast<Map06 *>(g_maps->_currentMap);
			byte &val1 = map[VAL1];
			val1 = val1 ? 0 : 1;

			// The original overlay byte is reloaded when a ScummVM save is
			// loaded. Remember the current visit's OFF state on the button
			// cell so a save/load does not reactivate the conveyors.
			byte &visited = map._visited[g_maps->_mapOffset];
			if (val1)
				visited &= ~VISITED_TRANSIENT_STATE;
			else
				visited |= VISITED_TRANSIENT_STATE;

			g_events->addKeypress(Common::KEYCODE_SPACE);
		}
	));
}

void Map06::special07() {
	g_maps->_mapPos.x = 10;
	g_maps->_mapPos.y = 11;
	updateGame();
	send(SoundMessage(17, 1, "Poof!"));
}

void Map06::special09() {
	send(SoundMessage(0, 1, mapText("A banner reads:\nCorridor of Endless Encounters!")));
}

void Map06::special10() {
	g_maps->_mapPos.x++;
	g_globals->_encounters.execute();
}

void Map06::special13() {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();

	// Encounter progress also initiates the side quest if the wizard
	// has not been visited first. A hidden quest remains hidden.
	_visited[QUEST_ACTIVE_OFFSET] |= ENCOUNTER_COMPLETE;

	// Record completion in the map visitation data, which is serialized in
	// savegames. This preserves progress when a game is saved and reloaded.
	byte &visited = _visited[g_maps->_mapOffset];
	if (visited & ENCOUNTER_COMPLETE) {
		checkPartyDead();
		return;
	}
	visited |= ENCOUNTER_COMPLETE;

	int monsterCount = getRandomNumber(5);

	enc._levelIndex = 20;
	int id1 = MONSTER_ID1[g_maps->_mapPos.y];
	int id2 = MONSTER_ID2[g_maps->_mapPos.y];

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(id1, id2);

	// The monster list above is scripted. Without manual mode execute()
	// discards it and generates an unrelated random encounter.
	enc._manual = true;
	enc.execute();
}

void Map06::special26() {
	if (completedEncounters() < 13) {
		// Talking to the wizard starts the tracked side quest
		_visited[QUEST_ACTIVE_OFFSET] |= ENCOUNTER_COMPLETE;
		send(SoundMessage(mapText("A crazed wizard exclaims,\n\"Encounter the 13, win the prize!\"")));
	} else {
		g_globals->_treasure.setGold(12000);
		g_globals->_treasure._items[1] = BRONZE_KEY_ID;
		g_globals->_treasure._items[2] = getRandomNumber(12) + 182;
		clearEncounterProgress();
		_visited[QUEST_ACTIVE_OFFSET] &= ~ENCOUNTER_COMPLETE;
		_visited[QUEST_COMPLETE_OFFSET] |= ENCOUNTER_COMPLETE;
		g_events->addAction(KEYBIND_SEARCH);
	}
}

int Map06::completedEncounters() const {
	int result = 0;

	// The thirteen challenges occupy map rows 0 through 12. Scanning
	// those rows avoids depending on overlay data when another map is active.
	for (int y = 0; y < 13; ++y) {
		for (int x = 0; x < MAP_W; ++x) {
			if (_visited[y * MAP_W + x] & ENCOUNTER_COMPLETE)
				++result;
		}
	}

	return result;
}

bool Map06::crazedWizardQuestActive() const {
	return (_visited[QUEST_ACTIVE_OFFSET] & ENCOUNTER_COMPLETE) ||
		completedEncounters() != 0;
}

bool Map06::crazedWizardQuestCompleted() const {
	return (_visited[QUEST_COMPLETE_OFFSET] & ENCOUNTER_COMPLETE) != 0;
}

void Map06::clearEncounterProgress() {
	for (int y = 0; y < 13; ++y) {
		for (int x = 0; x < MAP_W; ++x)
			_visited[y * MAP_W + x] &= ~ENCOUNTER_COMPLETE;
	}
}

void Map06::restoreVisitState() {
	const byte buttonOffset = _data[51 + 6];
	if (_visited[buttonOffset] & VISITED_TRANSIENT_STATE)
		_data[VAL1] = 0;
}

void Map06::slide() {
	_data[VAL2]++;

	SoundMessage msg(16, 1, "A slide!");
	msg._delaySeconds = 2;
	msg._callback = []() {
		g_maps->_currentMap->updateGame();
	};

	send(msg);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
