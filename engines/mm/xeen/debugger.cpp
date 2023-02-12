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

#include "common/file.h"
#include "mm/utils/strings.h"
#include "mm/xeen/xeen.h"
#include "mm/xeen/debugger.h"
#include "mm/xeen/files.h"

namespace MM {
namespace Xeen {

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
	registerCmd("load", WRAP_METHOD(Debugger, cmdLoadOriginal));
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

bool Debugger::cmdLoadOriginal(int argc, const char **argv) {
	Combat &combat = *g_vm->_combat;
	FileManager &files = *g_vm->_files;
	Interface &intf = *g_vm->_interface;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;

	if (argc != 3) {
		debugPrintf("load <game path> <savegame slot>: Loads original save\n");
		return true;
	}

	// Loop through loading the sides' save archives
	SaveArchive *archives[2] = { files._xeenSave, files._darkSave };
	CCArchive *cc[2] = { files._xeenCc, files._darkCc };
	const char *prefix[2] = { "XEEN", "DARK" };

	Common::FSNode folder(argv[1]);

	for (int idx = 0; idx < 2; ++idx) {
		Common::FSNode fsNode = folder.getChild(
			Common::String::format("%s%.2d.SAV", prefix[idx], strToInt(argv[2])));
		Common::File f;

		if (f.open(fsNode)) {
			archives[idx]->load(f);
			f.close();
		} else {
			archives[idx]->reset(cc[idx]);
		}
	}

	// TODO: Figure out to set correct side from original saves
	files.setGameCc(_vm->getGameID() == GType_DarkSide ? 1 : 0);

	// Load the character roster and party
	files._currentSave->loadParty();

	// Reset any combat information from the previous game
	combat.reset();
	party._treasure.reset();

	// Load the new map
	map.clearMaze();
	map._loadCcNum = files._ccNum;
	map.load(party._mazeId);

	intf.drawParty(true);
	return false;
}

} // End of namespace Xeen
} // End of namespace MM
