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

#include "mm/mm1/views/interactions/prisoners.h"
#include "mm/mm1/maps/map11.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Prisoner::Prisoner(const Common::String &name, const Common::String &line1,
		byte flag, Alignment freeAlignment, Alignment leaveAlignment) :
		TextView(name), _line1(line1), _flag(flag),
		_freeAlignment(freeAlignment), _leaveAlignment(leaveAlignment) {
	_bounds = getLineBounds(17, 24);
}

void Prisoner::draw() {
	clearSurface();
	writeString(0, 0, _line1);
	writeString(0, 3, "Options:  1) Set the prisoner free.\n");
	_textPos.x = 10;
	writeString("2) Torment the prisoner.\n");
	_textPos.x = 10;
	writeString("3) Leave without disturbing.");
}

bool Prisoner::msgFocus(const FocusMessage &msg) {
	Sound::sound2(SOUND_2);
	return TextView::msgFocus(msg);
}

bool Prisoner::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	if (msg.keycode < Common::KEYCODE_1 || msg.keycode > Common::KEYCODE_3)
		return true;

	Common::String line;
	int align;
	switch (msg.keycode) {
	case Common::KEYCODE_1:
		line = "The prisoner flees!";
		align = _freeAlignment;
		g_maps->clearSpecial();
		flee();
		break;

	case Common::KEYCODE_2:
		line = "The prisoner cowers!";
		align = _leaveAlignment;
		break;

	default:
		align = NEUTRAL;
		break;
	}

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];

		if (!(c._flags[1] & _flag)) {
			c._flags[1] |= _flag;
			if (align == c._alignment)
				c._worthiness += 32;
		}
	}

	if (align != NEUTRAL) {
		clearSurface();
		writeString(0, 1, line);
		Sound::sound(SOUND_2);
		delaySeconds(3);

	} else {
		close();
	}

	return true;
}

void Prisoner::timeout() {
	close();
}

/*------------------------------------------------------------------------*/

ChildPrisoner::ChildPrisoner() :
		Prisoner("ChildPrisoner", "A small child shivers fearfully as you\napproach.\n\n",
		CHARFLAG1_4, GOOD, EVIL) {
}

ManPrisoner::ManPrisoner() :
	Prisoner("ManPrisoner", "Man in shackles moans in agony!\n\n",
	CHARFLAG1_20, EVIL, GOOD) {
}

CloakedPrisoner::CloakedPrisoner() :
	Prisoner("CloakedPrisoner", "A mysterious cloaked figure, bound and\n"
		"gagged, lies motionless in the corner.\n"
		"\n",
		CHARFLAG1_40, EVIL, GOOD) {
}

DemonPrisoner::DemonPrisoner() :
	Prisoner("DemonPrisoner", "A vicious demon lunges at the party,\n"
		"restrained only by a collar and chain!\n"
		"\n",
		CHARFLAG1_10, EVIL, GOOD) {
}

MutatedPrisoner::MutatedPrisoner() :
	Prisoner("MutatedPrisoner", "Mutated creature chained to the floor\ngrunts as you approach.\n\n",
		CHARFLAG1_2, GOOD, EVIL) {
}

MaidenPrisoner::MaidenPrisoner() :
	Prisoner("MaidenPrisoner", "Inside a steel cage,a fair maiden weeps.\n\n",
		CHARFLAG1_8, GOOD, EVIL) {
}

void MaidenPrisoner::flee() {
	MM1::Maps::Map &map = *g_maps->_currentMap;
	map._walls[48] &= 0x7f;
}

/*------------------------------------------------------------------------*/

VirginPrisoner::VirginPrisoner() : TextView("VirginPrisoner") {
	setBounds(getLineBounds(20, 24));
}

void VirginPrisoner::draw() {
	clearSurface();
	writeString(0, 1, "A luscious virgin shackled to the wall!\na) Kiss her  b) Release her  c) Leave");
}

bool VirginPrisoner::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_a:
		g_events->close();
		g_events->send(SoundMessage("Try setting 1B,2J"));
		break;
	case Common::KEYCODE_b:
		g_events->close();
		static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap)->challenge();
		break;
	case Common::KEYCODE_c:
		g_events->close();
		break;
	default:
		return TextView::msgKeypress(msg);
	}

	return true;
}

bool VirginPrisoner::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		g_events->close();
		return true;
	} else {
		return TextView::msgAction(msg);
	}
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
