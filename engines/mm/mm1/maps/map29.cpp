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

#include "mm/mm1/maps/map29.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define ANSWER_OFFSET 67
#define BEAST_FLAG 110
// Og is the D4 special at (7,1): visited-array offset 1 * 16 + 7 = 23.
// Use the fixed cell index here rather than reading _data[53]. The overlay
// _data array is not loaded until Map29 itself is entered, while the Quest
// Log can be opened anywhere in the world.
#define OG_CELL_OFFSET (1 * MAP_W + 7)
#define OG_QUEST_LOG_FLAG 0x80

void Map29::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 4; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[55 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	switch (_walls[g_maps->_mapOffset]) {
	case 0xff:
		// The original overlay compares the cell wall byte with 0xff here.
		// 0x00 was a transcription error that made the intended 1-in-50
		// swamp encounter branch unreachable
		if (getRandomNumber(50) == 50) {
			int id1 = getRandomNumber(16);
			int monsterCount = getRandomNumber(id1 < 15 ? 13 : 4);

			enc.clearMonsters();
			for (int i = 0; i < monsterCount; ++i)
				enc.addMonster(id1, 11);

			enc._manual = true;
			enc._levelIndex = 80;
			enc.execute();

		} else {
			none160();
		}
		break;

	case 0xaa:
		if (g_maps->_forwardMask == DIRMASK_W)
			checkPartyDead();
		else
			// TODO: This key doesn't seem to be used by game
			g_events->addKeypress((Common::KeyCode)149);
		break;

	default:
		g_maps->clearSpecial();
		enc.execute();
		break;
	}
}

void Map29::special00() {
	visitedExit();

	send(SoundMessage(
		"Algary, enter (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(8, 15);
			g_maps->changeMapFromExit(0x203, 1);
		}
	));
}

void Map29::special01() {
	if (_data[BEAST_FLAG]) {
		g_maps->clearSpecial();
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[2] |= CHARFLAG2_4;
		}
	} else {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			if (c._flags[2] & CHARFLAG2_4)
				return;
		}

		send(SoundMessage(
			mapText("A huge winged beast exclaims,\n"
				"\"This swamp is quite dangerous. Fly you\n"
				"to safety (Y/N)?\""),
			[]() {
				g_maps->_mapPos = Common::Point(12, 12);
				g_maps->_currentMap->updateGame();
			},
			[]() {
				Game::Encounter &enc = g_globals->_encounters;
				Map29 &map = *static_cast<Map29 *>(g_maps->_currentMap);
				g_events->close();
				map[BEAST_FLAG]++;

				enc.clearMonsters();
				enc.addMonster(3, 12);
				for (int i = 1; i < 13; ++i)
					enc.addMonster(3, 5);

				enc._manual = true;
				enc._encounterType = Game::FORCE_SURPRISED;
				enc._levelIndex = 80;
				enc.execute();
			}
		));
	}
}

void Map29::special02() {
	if (!g_globals->_party.hasItem(B_QUEEN_IDOL_ID) ||
			!g_globals->_party.hasItem(W_QUEEN_IDOL_ID)) {
		begone();
	} else {
		g_events->addView("Chess");
	}
}

void Map29::special03() {
	SoundMessage msg(
		"Paul Pead and his men attack!",
		[]() {
			Game::Encounter &enc = g_globals->_encounters;
			int monsterCount = g_events->getRandomNumber(5) + 6;
			g_maps->clearSpecial();

			enc.clearMonsters();
			enc.addMonster(11, 12);
			for (int i = 1; i < monsterCount; ++i)
				enc.addMonster(1, 7);

			enc._manual = true;
			enc._encounterType = Game::FORCE_SURPRISED;
			enc._levelIndex = 80;
			enc.execute();
		}
	);
	msg._delaySeconds = 4;
	send(msg);
}

void Map29::chessAnswer(const Common::String &answer) {
	Common::String properAnswer;

	for (int i = 0; i < 22; ++i)
		properAnswer += _data[ANSWER_OFFSET + i] - 48;

	if (answer.equalsIgnoreCase(properAnswer)) {
		redrawGame();

		// Original MM1 behavior: the reward is committed immediately, the
		// Queen Idols vanish, and Og explains both the +25,000 EXP reward and
		// the Mt. Doom prisoner clue before the party is sent away.
		markOgQuestCompleted();

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._exp += 25000;
		}

		auto consumeIdol = [](byte itemId) {
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				int idx = c._backpack.indexOf(itemId);
				if (idx != -1) {
					c._backpack.removeAt(idx);
					return;
				}

				idx = c._equipped.indexOf(itemId);
				if (idx != -1) {
					c._equipped.removeAt(idx);
					return;
				}
			}
		};
		consumeIdol(B_QUEEN_IDOL_ID);
		consumeIdol(W_QUEEN_IDOL_ID);

		InfoMessage msg(
			0, 0, "YOU HAVE RESTORED MY SIGHT! (+25000 EXP)",
			0, 1, "I SEE AN IMPORTANT PRISONER IN A CASTLE ON MT. DOOM! HE HAS YOUR SIGHT.",
			[](const Common::KeyState &) {
				// Keep Og's reward/clue visible until the player dismisses it.
				// Only then perform the original forced move toward the Algary exit.
				g_events->focusedView()->close();
				g_maps->_mapPos.y = 7;
				g_maps->_currentMap->updateGame();
			}
		);
		msg._largeMessage = true;
		send(msg);
		Sound::sound(SOUND_3);
		Sound::sound(SOUND_3);

	} else {
		begone();
	}

}

void Map29::begone() {
	g_maps->_mapPos.y = 7;
	SoundMessage msg("Og says, \"Begone!\"",
		[]() {
			g_maps->_currentMap->updateGame();
		}
	);
	msg._delaySeconds = 2;
	send(msg);
}

bool Map29::ogQuestVisited() const {
	return (_visited[OG_CELL_OFFSET] & 0x7f) != 0;
}

bool Map29::ogQuestCompleted() const {
	return (_visited[OG_CELL_OFFSET] & OG_QUEST_LOG_FLAG) != 0;
}

void Map29::markOgQuestCompleted() {
	_visited[OG_CELL_OFFSET] |= OG_QUEST_LOG_FLAG;
}

} // namespace Maps
} // namespace MM1
} // namespace MM
