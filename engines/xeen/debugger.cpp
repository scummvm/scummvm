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
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
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

Debugger::Debugger(XeenEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("spell", WRAP_METHOD(Debugger, cmdSpell));
	registerCmd("dump", WRAP_METHOD(Debugger, cmdDump));
	registerCmd("gold", WRAP_METHOD(Debugger, cmdGold));
	registerCmd("gems", WRAP_METHOD(Debugger, cmdGems));

	_spellId = -1;
}

void Debugger::update() {
	Party &party = *_vm->_party;
	Spells &spells = *_vm->_spells;

	if (_spellId != -1) {
		// Cast any specified spell
		MagicSpell spellId = (MagicSpell)_spellId;
		_spellId = -1;
		Character *c = &party._activeParty[0];
		c->_currentSp = 99;
		spells.castSpell(c, spellId);
	}

	onFrame();
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

bool Debugger::cmdDump(int argc, const char **argv) {
	File f;

	if (argc < 2) {
		debugPrintf("Format: dump <resource name>\n");
	} else {
		if (argc == 2)
			f.open(argv[1]);
		else
			f.open(argv[1], (ArchiveType)strToInt(argv[2]));

		if (f.isOpen()) {
			Common::DumpFile df;
			df.open(argv[1]);
			byte *data = new byte[f.size()];
			f.read(data, f.size());
			df.write(data, f.size());

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

} // End of namespace Xeen
