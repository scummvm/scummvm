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

#include "glk/comprehend/game_oo.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/pics.h"
#include "common/md5.h"

namespace Glk {
namespace Comprehend {

enum OOToposRoomFlag {
	OO_ROOM_IN_SHIP = 1,
	OO_ROOM_FLAG_DARK = 2
};

enum OOToposFlag {
	OO_FLAG_9 = 9,
	OO_FLAG_13 = 13,
	OO_FLAG_22 = 22,
	OO_BRIGHT_ROOM = 25,
	OO_FLAG_WEARING_GOGGLES = 27,
	OO_FLAG_FLASHLIGHT_ON = 39,
	OO_FLAG_43 = 43,
	OO_FLAG_44 = 44,
	OO_FLAG_SUFFICIENT_FUEL = 51,
	OO_FLAG_REVERSE_VIDEO = 53,	// Effect of wearing goggles
	OO_FLAG_TOO_DARK = 55,
	OO_FLAG_TOO_BRIGHT = 56,
	OO_FLAG_58 = 58,
	OO_FLAG_59 = 59,
	OO_FLAG_READY_TO_DEPART = 60,
	OO_TRACTOR_BEAM = 71
};

enum OOToposItem {
	ITEM_SERUM_VIAL = 39
};

static const GameStrings OO_STRINGS = {
	EXTRA_STRING_TABLE(154)
};

OOToposGame::OOToposGame() : ComprehendGameV2(), _restartMode(RESTART_IMMEDIATE),
		_noFloodfill(UNSET), _lightOn(UNSET), _stringVal1(0), _stringVal2(0),
		_printComputerMsg(true), _shipNotWorking(false) {
	_gameDataFile = "g0";

	// Extra strings are (annoyingly) stored in the game binary
	Common::File f;
	if (!f.open("novel.exe"))
		error("novel.exe is a required file");

	Common::String md5 = Common::computeStreamMD5AsString(f, 1024);
	f.close();

	if (md5 == "3fc2072f6996b17d2f21f0a92e53cdcc") {
		// DOS version from if-archive
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x16564, 0x17640));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x17702, 0x18600));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x186b2, 0x19b80));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x19c62, 0x1a590));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x1a634, 0x1b080));
	} else if (md5 == "e26858f2aaa9dcc28f468b07902813c5") {
		// DOS version from graphicsmagician.com
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x164c4, 0x175a0));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x17662, 0x18560));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x18612, 0x19ae0));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x19bc2, 0x1a4f0));
		_stringFiles.push_back(StringFile("NOVEL.EXE", 0x1a594, 0x1afe0));
	} else {
		error("Unrecognised novel.exe encountered");
	}

	_locationGraphicFiles.push_back("RA");
	_locationGraphicFiles.push_back("RB");
	_locationGraphicFiles.push_back("RC");
	_locationGraphicFiles.push_back("RD");
	_locationGraphicFiles.push_back("RE");
	_itemGraphicFiles.push_back("OA");
	_itemGraphicFiles.push_back("OB");
	_itemGraphicFiles.push_back("OC");
	_itemGraphicFiles.push_back("OD");

	_colorTable = 1;
	_gameStrings = &OO_STRINGS;
	_titleGraphicFile = "t0";
}

void OOToposGame::beforeGame() {
	// Draw the title
	g_comprehend->drawPicture(TITLE_IMAGE);

	// Print game information
	console_println("Story by Michael and Muffy Berlyn, graphics by Raim und Redlich and Brian Poff");
	console_println("IBM version by Jeffrey A. Jay. Copyright 1987  POLARWARE, Inc.");
	g_comprehend->readChar();

	g_comprehend->glk_window_clear(g_comprehend->_bottomWindow);
}

int OOToposGame::roomIsSpecial(unsigned room_index, unsigned *roomDescString) {
	Room *room = &_rooms[room_index];

	// Is the room dark
	if ((room->_flags & OO_ROOM_FLAG_DARK) &&
	        !(_flags[OO_FLAG_FLASHLIGHT_ON])) {
		if (roomDescString)
			*roomDescString = 0xb3;
		return ROOM_IS_DARK;
	}

	// Is the room too bright
	if (room_index == OO_BRIGHT_ROOM &&
	        !_flags[OO_FLAG_WEARING_GOGGLES]) {
		if (roomDescString)
			*roomDescString = 0x1c;
		return ROOM_IS_TOO_BRIGHT;
	}

	return ROOM_IS_NORMAL;
}

void OOToposGame::beforeTurn() {
	ComprehendGameV2::beforeTurn();

	if (_flags[OO_FLAG_TOO_DARK]) {
		// Show placeholder room if room is too dark
		_currentRoom = 55;
		_updateFlags |= UPDATE_GRAPHICS;
	} else if (_flags[OO_FLAG_TOO_BRIGHT]) {
		// Show placeholder room if room is too bright
		_currentRoom = 54;
		_updateFlags |= UPDATE_GRAPHICS;
	} else {
		YesNo nff = _flags[OO_FLAG_REVERSE_VIDEO] ? YES : NO;

		if (_noFloodfill != nff) {
			_noFloodfill = nff;
			_updateFlags |= UPDATE_GRAPHICS | UPDATE_ROOM_DESC;

			if (_noFloodfill == YES)
				g_comprehend->_drawFlags |= IMAGEF_REVERSE;
			else
				g_comprehend->_drawFlags &= ~IMAGEF_REVERSE;
		}
	}
}

void OOToposGame::beforePrompt() {
	// Handle the computer console if in front of it
	computerConsole();
}

void OOToposGame::afterPrompt() {
	ComprehendGameV2::afterPrompt();

	// WORKAROUND: Allow for the Apple 2 password in the DOS version
	if (!scumm_stricmp(_inputLine, "vug957a"))
		strcpy(_inputLine, "tse957x");

	if (_currentRoom != _currentRoomCopy)
		_updateFlags |= UPDATE_GRAPHICS;
	_currentRoom = _currentRoomCopy;
}

void OOToposGame::handleSpecialOpcode() {
	switch (_specialOpcode) {
	case 1:
		// Update guard location
		randomizeGuardLocation();
		break;

	case 2:
		_restartMode = RESTART_IMMEDIATE;
		game_restart();
		break;

	case 3:
		_restartMode = RESTART_WITH_MSG;
		game_restart();
		break;

	case 4:
		_restartMode = RESTART_WITHOUT_MSG;
		game_restart();
		break;

	case 5:
		// Won the game
		g_comprehend->quitGame();
		break;

	case 6:
		// Save game
		game_save();
		break;

	case 7:
		// Restore game
		game_restore();
		break;

	case 8:
		// Computer response
		computerResponse();
		randomizeGuardLocation();
		break;

	case 9:
		// Checks the ship fuel
		checkShipFuel();
		randomizeGuardLocation();
		break;

	case 10:
		// Checks whether the ship is working
		checkShipWorking();
		break;

	default:
		break;
	}
}

bool OOToposGame::handle_restart() {
	_ended = false;

	if (_restartMode != RESTART_IMMEDIATE) {
		if (_restartMode == RESTART_WITH_MSG)
			console_println(stringLookup(_gameStrings->game_restart).c_str());

		if (tolower(console_get_key()) != 'r') {
			g_comprehend->quitGame();
			return false;
		}
	}

	loadGame();
	_updateFlags = UPDATE_ALL;
	return true;
}

void OOToposGame::synchronizeSave(Common::Serializer &s) {
	if (s.isSaving())
		_currentRoom = _currentRoomCopy;

	ComprehendGameV2::synchronizeSave(s);

	if (s.isLoading()) {
		_noFloodfill = UNSET;
		_currentRoomCopy = _currentRoom;

		beforeTurn();
	}
}

void OOToposGame::randomizeGuardLocation() {
	Item *item = get_item(22);
	if (_flags[OO_FLAG_13] && item->_room != _currentRoom) {
		if (getRandomNumber(255) > 128 && (_currentRoom == 3 || _currentRoom == 6))
			item->_room = _currentRoom;
	}
}

void OOToposGame::computerConsole() {
	if (_currentRoom == 57) {
		if (!_flags[OO_FLAG_9]) {
			// Mission Code:
			console_println(_strings2[129].c_str());
		} else if (!_flags[OO_FLAG_58]) {
			// Welcome back! I was wondering if you would be returning
			console_println(_strings2[131].c_str());
			_flags[OO_FLAG_58] = true;
			_printComputerMsg = true;
			checkShipWorking();
		} else if (_flags[OO_FLAG_59]) {
			checkShipDepart();
		} else if (_flags[OO_FLAG_43]) {
			// We can reach Mealy Sukas with the fuel we have left
			console_println(_strings2[142].c_str());
			_flags[OO_FLAG_59] = true;

			if (_flags[OO_FLAG_44])
				// The currency on Mealy Sukas is the 'frod'
				console_println(_strings2[144].c_str());
			else
				// Without evaluation data as to the current fuel prices
				console_println(_strings2[143].c_str());
		}
	}
}

void OOToposGame::computerResponse() {
	console_println(_strings2[145].c_str());
	if (_flags[OO_FLAG_43])
		console_println(_strings2[144].c_str());
	else
		console_println(_strings2[152].c_str());
}

void OOToposGame::checkShipWorking() {
	_stringVal1 = 164;
	_stringVal2 = 0;

	// Iterate through the ship's flags
	for (int idx = 42; idx < 51; ++idx, ++_stringVal1) {
		if (!_flags[idx]) {
			if (!_stringVal2) {
				// The following components are not installed
				printComputerMsg(_strings2[132].c_str());
				_stringVal2 = 1;
			}

			// Power Cylinder
			printComputerMsg(_strings[_stringVal1].c_str());
		}
	}

	_shipNotWorking = _stringVal2 != 0;
	if (!_shipNotWorking)
		// The ship is in working order
		printComputerMsg(_strings2[153].c_str());
}

void OOToposGame::checkShipFuel() {
	const byte ITEMS[7] = { 24, 27, 28, 29, 30, 31, 32 };
	_variables[0x4b] = 0;
	_stringVal1 = 68;
	_stringVal2 = 0;

	for (int idx = 168; idx < 175; ++idx, ++_stringVal1, ++_stringVal2) {
		if (_flags[idx]) {
			Item *item = get_item(ITEMS[_stringVal2] - 1);
			if (item->_room == ROOM_INVENTORY || (get_room(item->_room)->_flags & OO_ROOM_IN_SHIP) != 0) {
				Instruction varAdd(0x86, 0x4B, _stringVal1);
				execute_opcode(&varAdd, nullptr, nullptr);
			}
		}
	}

	// Computer: "Our current evaluation...
	Instruction strReplace(0xC9, 0x4B);
	execute_opcode(&strReplace, nullptr, nullptr);
	printComputerMsg(_strings2[146].c_str());

	FunctionState funcState;
	Instruction test(2, 75, 76);
	execute_opcode(&test, nullptr, nullptr);

	if (funcState._testResult) {
		// Computer: "We should now have enough
		_flags[OO_FLAG_SUFFICIENT_FUEL] = true;
		printComputerMsg(_strings2[151].c_str());
	} else {
		_flags[OO_FLAG_SUFFICIENT_FUEL] = false;
	}
}

void OOToposGame::checkShipDepart() {
	_printComputerMsg = false;
	checkShipWorking();
	checkShipFuel();
	_printComputerMsg = true;

	if (!_shipNotWorking && _flags[OO_FLAG_SUFFICIENT_FUEL]) {
		Item *item = get_item(ITEM_SERUM_VIAL - 1);
		if (item->_room == ROOM_INVENTORY || (get_room(item->_room)->_flags & OO_ROOM_IN_SHIP) != 0) {
			if (!_flags[OO_TRACTOR_BEAM]) {
				// I detect a tractor beam
				console_println(_strings2[77].c_str());
			} else if (!_flags[OO_FLAG_READY_TO_DEPART]) {
				// All systems check. Ready to depart
				_flags[OO_FLAG_22] = true;
				console_println(_strings2[79].c_str());
			} else {
				// Please close the airlock
				console_println(_strings2[76].c_str());
			}
		} else {
			// The serum vial is not aboard the ship
			console_println(_strings2[78].c_str());
		}
	}
}

void OOToposGame::printComputerMsg(const char *str) {
	if (_printComputerMsg)
		console_println(str);
}

} // namespace Comprehend
} // namespace Glk
