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
#include "access/access.h"
#include "access/debugger.h"
#include "access/amazon/amazon_game.h"

namespace Access {

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

Debugger *Debugger::init(AccessEngine *vm) {
	switch (vm->getGameID()) {
	case kGameAmazon:
		return new Amazon::AmazonDebugger(vm);
	default:
		return new Debugger(vm);
	}
}

void Debugger::postEnter() {
	if (!_playMovieFile.empty()) {
		_vm->playMovie(_playMovieFile, Common::Point(0, 0));

		_playMovieFile.clear();
	}

	GUI::Debugger::postEnter();
}

/*------------------------------------------------------------------------*/

Debugger::Debugger(AccessEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("scene", WRAP_METHOD(Debugger, Cmd_LoadScene));
	registerCmd("cheat", WRAP_METHOD(Debugger, Cmd_Cheat));
	registerCmd("playmovie", WRAP_METHOD(Debugger, Cmd_PlayMovie));
	registerCmd("dumpscript", WRAP_METHOD(Debugger, Cmd_DumpScript));
	registerCmd("timers", WRAP_METHOD(Debugger, Cmd_Timers));
	registerCmd("flag", WRAP_METHOD(Debugger, Cmd_Flag));
	registerCmd("travel", WRAP_METHOD(Debugger, Cmd_Travel));
	registerCmd("ask", WRAP_METHOD(Debugger, Cmd_Ask));
	registerCmd("inventory", WRAP_METHOD(Debugger, Cmd_Inventory));
	registerCmd("everything", WRAP_METHOD(Debugger, Cmd_Everything));
}

Debugger::~Debugger() {
}

bool Debugger::Cmd_LoadScene(int argc, const char **argv) {
	switch (argc) {
	case 1:
		debugPrintf("Current scene is: %d\n\n", _vm->_player->_roomNumber);

		for (uint i = 0; i < _vm->_res->ROOMTBL.size(); i++)
			if (!_vm->_res->ROOMTBL[i]._desc.empty())
				debugPrintf("%d - %s\n", i, _vm->_res->ROOMTBL[i]._desc.c_str());
		return true;

	case 2: {
		int newRoom = strToInt(argv[1]);
		if (newRoom < 0 || newRoom >= (int)_vm->_res->ROOMTBL.size()) {
			debugPrintf("Invalid Room Number\n");
			return true;
		}
		if (_vm->_res->ROOMTBL[newRoom]._desc.empty()) {
			debugPrintf("Unused Room Number\n");
			return true;
		}

		_vm->_player->_roomNumber = newRoom;

		_vm->_room->_function = FN_CLEAR1;
		_vm->freeChar();
		_vm->_converseMode = 0;
		_vm->_scripts->_endFlag = true;
		_vm->_scripts->_returnCode = 0;

		return false;
		}
	default:
		debugPrintf("Current scene is: %d\n", _vm->_player->_roomNumber);
		debugPrintf("Usage: %s <scene number>\n", argv[0]);
		return true;
	}
}

bool Debugger::Cmd_Cheat(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Switches on/off the cheat mode.  Cheat mode:\n");
		debugPrintf(" - [Amazon] Skips guard on boat\n");
		debugPrintf(" - [MM] Allows travel to \"can't get there from here\" locations\n");
		return true;
	}

	_vm->_cheatFl = !_vm->_cheatFl;
	debugPrintf("Cheat is now %s\n", _vm->_cheatFl ? "ON" : "OFF");
	return true;
}

bool Debugger::Cmd_PlayMovie(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: playmovie <movie-file>\n");
		return true;
	}

	// play gets postponed until debugger is closed
	Common::String filename = argv[1];
	_playMovieFile = filename;

	return cmdExit(0, nullptr);
}

bool Debugger::Cmd_DumpScript(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <path>\n", argv[0]);
		debugPrintf("Dumps the currently loaded script data to the given path\n");
		return true;
	}

	Common::SeekableReadStream *data = _vm->_scripts->_data;
	if (!data) {
		debugPrintf("No script loaded\n");
		return true;
	}

	const Common::Path outpath = Common::Path(argv[1]);

	Common::DumpFile dumpFile;

	dumpFile.open(outpath);
	if (!dumpFile.isOpen()) {
		debugPrintf("Couldn't open %s\n", argv[1]);
		return true;
	}

	int64 oldpos = data->pos();
	data->seek(0);

	dumpFile.writeStream(data);
	dumpFile.close();

	data->seek(oldpos);
	return true;
}

bool Debugger::Cmd_Flag(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Usage: %s <flag number> [<flag value>]\n", argv[0]);
		debugPrintf("Prints or sets the value of the given flag\n");
		return true;
	}

	if (argc == 2) {
		int flagNum = strToInt(argv[1]);
		if (flagNum < 0 || flagNum >= 256) {
			debugPrintf("Invalid flag number\n");
			return true;
		}

		debugPrintf("Flag %d: %d\n", flagNum, _vm->_flags[flagNum]);
		return true;
	}

	int num = strToInt(argv[1]);
	if (num < 0 || num >= ARRAYSIZE(_vm->_flags)) {
		debugPrintf("Invalid flag number\n");
		return true;
	}

	int val = strToInt(argv[2]);
	if (val < 0 || val >= 256) {
		debugPrintf("Invalid flag val, must be byte\n");
		return true;
	}
	_vm->_flags[num] = val;

	debugPrintf("Flag %d set to %d\n", num, val);
	return true;
}

bool Debugger::Cmd_Timers(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Prints the current timers\n");
		return true;
	}

	debugPrintf("Timers:\n");
	for (uint i = 0; i < _vm->_timers.size(); ++i) {
		const TimerEntry te = _vm->_timers[i];
		debugPrintf("%d: init: %d timer: %d flag: %d\n", i, te._initTm, te._timer, te._flag);
	}

	return true;
}

bool Debugger::Cmd_Travel(int argc, const char **argv) {
	if (argc != 1 && argc != 3) {
		debugPrintf("Usage: %s [<travel number> <travel value>]\n", argv[0]);
		debugPrintf("Dump the travel table, or set a travel table entry to the given value\n");
		return true;
	}

	if (argc == 1) {
		debugPrintf("Travel table:\n");

		for (int i = 0; i < ARRAYSIZE(_vm->_travel); ++i) {
			if (!Martian::TRAVDATA[i])
				break;
			debugPrintf("%2d: %d (%s)\n", i, _vm->_travel[i], Martian::TRAVDATA[i]);
		}

		return true;
	}

	int num = strToInt(argv[1]);
	if (num < 0 || num >= ARRAYSIZE(_vm->_travel)) {
		debugPrintf("Invalid travel number\n");
		return true;
	}

	int val = strToInt(argv[2]);
	if (val < 0 || val >= 256) {
		debugPrintf("Invalid travel val, must be byte\n");
		return true;
	}
	_vm->_flags[num] = val;

	debugPrintf("Travel %d set to %d\n", num, val);
	return true;
}

bool Debugger::Cmd_Ask(int argc, const char **argv) {
	if (argc != 1 && argc != 3) {
		debugPrintf("Usage: %s [<ask number> <ask value>]\n", argv[0]);
		debugPrintf("Dump the ask table, or set an ask table entry to the given value\n");
		return true;
	}

	if (argc == 1) {
		debugPrintf("Ask table:\n");

		for (int i = 0; i < ARRAYSIZE(_vm->_ask); ++i) {
			if (!Martian::ASK_TBL[i])
				break;
			debugPrintf("%2d: %d (%s)\n", i, _vm->_ask[i], Martian::ASK_TBL[i]);
		}

		return true;
	}

	int num = strToInt(argv[1]);
	if (num < 0 || num >= ARRAYSIZE(_vm->_ask)) {
		debugPrintf("Invalid ask number\n");
		return true;
	}

	int val = strToInt(argv[2]);
	if (val < 0 || val >= 256) {
		debugPrintf("Invalid ask val, must be byte\n");
		return true;
	}
	_vm->_flags[num] = val;

	debugPrintf("Ask %d set to %d\n", num, val);
	return true;
}

bool Debugger::Cmd_Inventory(int argc, const char **argv) {
	if (argc != 1 && argc != 3) {
		debugPrintf("Usage: %s [<inv number> <state value>]\n", argv[0]);
		debugPrintf("Dump the list of inventory items and their state, or set the state of an item\n");
		return true;
	}

	static const char* STATE_NAMES[] = {
		"0 - Not Found",
		"1 - In Inv   ",
		"2 - Used	 ",
	};

	if (argc == 1) {
		debugPrintf("Inventory items:\n");

		for (int i = 0; i < (int)_vm->_inventory->_inv.size(); ++i) {
			const InventoryEntry &entry = _vm->_inventory->_inv[i];
			debugPrintf("%2d: %s  %s\n", i, STATE_NAMES[entry._value], entry._name.c_str());
		}
		return true;
	}

	int num = strToInt(argv[1]);
	if (num < 0 || num >= (int)_vm->_inventory->_inv.size()) {
		debugPrintf("Invalid inv number\n");
		return true;
	}

	int val = strToInt(argv[2]);
	if (val < 0 || val > 2) {
		debugPrintf("Invalid inv state val, must be 0/1/2\n");
		return true;
	}
	_vm->_inventory->_inv[num]._value = val;

	debugPrintf("Set item %d to %d\n", num, val);
	return true;
}

bool Debugger::Cmd_Everything(int argc, const char **argv) {
	if (argc != 2 || strcmp(argv[1], "please") != 0) {
		debugPrintf("Usage: %s please\n", argv[0]);
		debugPrintf("Gives you all items, travel locations, and ask subjects.\n");
		debugPrintf("Cannot be undone and may break your game, so you have to confirm with 'please'.\n");
		return true;
	}

	for (uint i = 0; i < _vm->_res->INVENTORY.size(); ++i)
		_vm->_inventory->_inv[i]._value = ITEM_IN_INVENTORY;

	for (uint i = 0; i < ARRAYSIZE(_vm->_travel); ++i)
		_vm->_travel[i] = 1;

	//
	// Turn off known-broken/cut locations that exist in the travel table
	// but you can't go there or going there directly will cause a crash.
	//
	const int INVALID_TRAVEL_LOCATIONS[] = {
		10, // RESTAURANT
		12, // LOVE SCENE
	};

	for (uint i = 0; i < ARRAYSIZE(INVALID_TRAVEL_LOCATIONS); ++i)
		_vm->_travel[INVALID_TRAVEL_LOCATIONS[i]] = 0;

	for (uint i = 0; i < ARRAYSIZE(_vm->_ask); ++i)
		_vm->_ask[i] = 1;

	debugPrintf("You now have everything, can go anywhere, and can ask anything.\n");

	return true;
}



/*------------------------------------------------------------------------*/

namespace Amazon {

AmazonDebugger::AmazonDebugger(AccessEngine *vm) : Debugger(vm) {
	registerCmd("chapter", WRAP_METHOD(AmazonDebugger, Cmd_StartChapter));
}

bool AmazonDebugger::Cmd_StartChapter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <chapter number>\n", argv[0]);
		return true;
	}

	// Build up a simple one line script to start the given chapter
	byte *chapterScript = (byte *)malloc(5);
	if (!chapterScript)
		error("malloc failed");
	chapterScript[0] = SCRIPT_START_BYTE;
	chapterScript[1] = ROOM_SCRIPT % 256;
	chapterScript[2] = ROOM_SCRIPT / 256;
	chapterScript[3] = 0x80 + 75;			// cmdChapter
	chapterScript[4] = strToInt(argv[1]);	// chapter number
	_vm->_scripts->setScript(new Resource(chapterScript, 5), true);

	return false;
}

} // End of namespace Amazon

} // End of namespace Access
