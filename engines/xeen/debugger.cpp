/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "xeen/xeen.h"
#include "xeen/debugger.h"

namespace Xeen {

static int strToInt(const char *s) {
	size_t size = strlen(s);

	if (size == 0)
		// No string at all
		return 0;

	if (toupper(s[size - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

/*------------------------------------------------------------------------*/

Debugger::Debugger(XeenEngine *vm) : GUI::Debugger(), _vm(vm),
		_spellId(-1), _invincible(false), _intangible(false), _superStrength(false) {
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("spell", WRAP_METHOD(Debugger, cmdSpell));
	registerCmd("spells", WRAP_METHOD(Debugger, cmdSpells));
	registerCmd("dump", WRAP_METHOD(Debugger, cmdDump));
	registerCmd("gold", WRAP_METHOD(Debugger, cmdGold));
	registerCmd("gems", WRAP_METHOD(Debugger, cmdGems));
	registerCmd("map", WRAP_METHOD(Debugger, cmdMap));
	registerCmd("pos", WRAP_METHOD(Debugger, cmdPos));
	registerCmd("invincible", WRAP_METHOD(Debugger, cmdInvincible));
	registerCmd("strength", WRAP_METHOD(Debugger, cmdSuperStrength));
	registerCmd("intangible", WRAP_METHOD(Debugger, cmdIntangible));
}

void Debugger::onFrame() {
	if (_spellId != -1) {
		// Cast any specified spell
		MagicSpell spellId = (MagicSpell)_spellId;
		_spellId = -1;
		Character *c = &_vm->_party->_activeParty[0];
		c->_currentSp = 99;
		_vm->_spells->castSpell(c, spellId);
	}

	GUI::Debugger::onFrame();
}

bool Debugger::cmdSpell(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: spell <spell-id>");
		return true;
	} else {
		int spellId = strToInt(argv[1]);
		if (spellId >= MS_AcidSpray && spellId <= MS_WizardEye) {
			_spellId = spellId;
			return false;
		}
	}

	return true;
}

bool Debugger::cmdSpells(int argc, const char **argv) {
	Party &party = *_vm->_party;

	for (uint charIdx = 0; charIdx < party._activeParty.size(); ++charIdx) {
		Character &c = party._activeParty[charIdx];
		Common::fill(c._spells, c._spells + SPELLS_PER_CLASS, true);
		c._currentSp = 9999;
	}

	party._gems += 1000;

	debugPrintf("Spells given to party.\n");
	return true;
}

bool Debugger::cmdDump(int argc, const char **argv) {
	File f;

	if (argc < 2) {
		debugPrintf("Format: dump <resource name>\n");
	} else {
		if (argc == 2)
			f.open(argv[1]);

		if (f.isOpen()) {
			Common::DumpFile df;
			df.open(argv[1]);

			size_t size = f.size();
			byte *data = new byte[size];

			if (f.read(data, size) == size) {
				df.write(data, size);

			} else {
				debugPrintf("Failed to read %zu bytes from '%s'\n", size, argv[1]);
			}

			f.close();
			df.close();
			delete[] data;
			debugPrintf("Saved\n");
		} else {
			debugPrintf("Could not find resource with that name\n");
		}
	}

	return true;
}

bool Debugger::cmdGold(int argc, const char **argv) {
	Party &party = *_vm->_party;
	if (argc == 1) {
		debugPrintf("Current gold: %d, bank: %d\n", party._gold, party._bankGold);
	} else {
		party._gold = strToInt(argv[1]);
		if (argc > 2)
			party._bankGold = strToInt(argv[2]);
	}

	return true;
}

bool Debugger::cmdGems(int argc, const char **argv) {
	Party &party = *_vm->_party;
	if (argc == 1) {
		debugPrintf("Current gems: %d, bank: %d\n", party._gems, party._bankGems);
	} else {
		party._gems = strToInt(argv[1]);
		if (argc > 2)
			party._bankGems = strToInt(argv[2]);
	}

	return true;
}

bool Debugger::cmdMap(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("map mapId [ xp, yp ] [ sideNum ]\n");
		return true;
	} else {
		Map &map = *g_vm->_map;
		Party &party = *g_vm->_party;

		int mapId = strToInt(argv[1]);
		int x = argc < 3 ? 8 : strToInt(argv[2]);
		int y = argc < 4 ? 8 : strToInt(argv[3]);

		if (argc == 5)
			map._loadCcNum = strToInt(argv[4]);
		map.load(mapId);
		party._mazePosition.x = x;
		party._mazePosition.y = y;
		party._mazeDirection = DIR_NORTH;
		return false;
	}
}

bool Debugger::cmdPos(int argc, const char **argv) {
	Party &party = *g_vm->_party;

	if (argc < 3) {
		debugPrintf("pos xp, yp\n");
		return true;
	} else {
		party._mazePosition.x = strToInt(argv[1]);
		party._mazePosition.y = strToInt(argv[2]);
		party._stepped = true;
		return false;
	}
}

bool Debugger::cmdInvincible(int argc, const char **argv) {
	_invincible = (argc < 2) || strcmp(argv[1], "off");
	debugPrintf("Invincibility is %s\n", _invincible ? "on" : "off");
	return true;
}

bool Debugger::cmdSuperStrength(int argc, const char **argv) {
	_superStrength = (argc < 2) || strcmp(argv[1], "off");
	debugPrintf("Super-powered attacks are %s\n", _superStrength ? "on" : "off");
	return true;
}

bool Debugger::cmdIntangible(int argc, const char **argv) {
	_intangible = (argc < 2) || strcmp(argv[1], "off");
	debugPrintf("Intangibility is %s\n", _intangible ? "on" : "off");
	return true;
}

} // End of namespace Xeen
