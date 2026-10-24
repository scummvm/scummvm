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

#include "mm/mm1/maps/map34.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 1238

void Map34::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 23; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[74 + i]) {			
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	// All other cells on the map are encounters
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map34::special00() {
	g_events->addView("ChildPrisoner");
}

void Map34::special01() {
	visitedExit();

	send(SoundMessage(
		"A passage leads outside, take it (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(7, 15);
			g_maps->changeMapFromExit(0xf01, 2);
		}
	));
}

void Map34::special02() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if ((c._flags[13] & CHARFLAG13_ALAMAR) || c.hasItem(EYE_OF_GOROS_ID)) {
			g_maps->clearSpecial();
			g_globals->_encounters.execute();
			return;
		}
	}

	Sound::sound2(SOUND_2);

	InfoMessage msg(0, 0, mapText("Suspended in a forcefield is the body\n"
		"of a man enclosed in an iron suit.\n"
		"Telepathic communication: \"I am the true\n"
		"King Alamar, imprisoned by a demonic\n"
		"alien being! Confront him with this Eye\n"
		"of Goros and you shall see his true\n"
		"self. The fate of the realm is at stake\""));
	msg._largeMessage = true;
	msg._keyCallback = [](const Common::KeyState &) {
		g_events->close();
		g_globals->_treasure._items[2] = EYE_OF_GOROS_ID;
		g_events->addAction(KEYBIND_SEARCH);
	};

	send(msg);
}

void Map34::special03() {
	send(SoundMessage(mapText("Etched in silver, message D reads:\n//sv/21;-22r,;du1rs0")));
}

void Map34::special04() {
	send(SoundMessage(mapText("Written in blue blood:\nThe canine has the key!")));
}

void Map34::special05() {
	send(SoundMessage(mapText("Etched in silver:\nMessage interleave 'fedbac'")));
}

void Map34::special06() {
	send(SoundMessage(mapText("Etched in gold:\nMessage interleave '8,5,3,9,1,4,6,2,7'")));
}

void Map34::special07() {
	if (g_globals->_party.hasItem(GOLD_KEY_ID)) {
		checkPartyDead();
	} else {
		send(SoundMessage("A golden door repels you!"));
		g_maps->_mapPos.y--;
		updateGame();
	}
}

void Map34::special08() {
	send(SoundMessage(mapText("A gruesome statue says:\n\"Start at 15-7 and walk to Doom!\"")));
}

void Map34::special09() {
	send(SoundMessage(mapText("A banner reads:\nThe endless spiral!!!")));
}

void Map34::special10() {
	g_maps->_mapPos.x++;
	g_maps->_mapPos.y--;
	updateGame();
}

void Map34::special12() {
	g_maps->_mapPos.x--;
	g_maps->_mapPos.y++;
	updateGame();
}

void Map34::special13() {
	send(SoundMessage(mapText("A sign above the door reads:\nMonster breeding grounds, keep out!")));
}

void Map34::special16() {
	send(SoundMessage("A loud machine pulsates rhythmically."));
}

void Map34::special17() {
	send(SoundMessage(mapText("A metalic box echoes:\n\"Intruder alert!\"")));
	_data[MAP_47] = 9;
	_data[VAL1] = 9;
}

void Map34::special18() {
	send(SoundMessage(mapText("Scratched on the wall:\nSearch after the canine rewards you!")));
}

void Map34::special19() {
	send(SoundMessage(mapText("Scrawled on the wall:\n"
		"For the world is hollow and i have\n"
		"touched the sky!")));
}

void Map34::special20() {
	if (_data[VAL1]) {
		g_maps->clearSpecial();
		g_globals->_encounters.execute();
	} else {
		none160();
	}
}

void Map34::special21() {
	send(SoundMessage(mapText("Written on the wall:\nJump three times to reach center.")));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
