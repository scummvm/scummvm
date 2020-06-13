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

namespace Glk {
namespace Comprehend {

#define OO_ROOM_FLAG_DARK 0x02

#define OO_BRIGHT_ROOM 0x19

#define OO_FLAG_WEARING_GOGGLES 0x1b
#define OO_FLAG_FLASHLIGHT_ON 0x27

OOToposGame::OOToposGame() : ComprehendGame() {
	_gameDataFile = "g0";

	// Extra strings are (annoyingly) stored in the game binary
	_stringFiles.push_back(StringFile("NOVEL.EXE", 0x16564, 0x17640));
	_stringFiles.push_back(StringFile("NOVEL.EXE", 0x17702, 0x18600));
	_stringFiles.push_back(StringFile("NOVEL.EXE", 0x186b2, 0x19b80));
	_stringFiles.push_back(StringFile("NOVEL.EXE", 0x19c62, 0x1a590));
	_stringFiles.push_back(StringFile("NOVEL.EXE", 0x1a634, 0x1b080));
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
}

int OOToposGame::roomIsSpecial(unsigned room_index,
                               unsigned *roomDescString) {
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
	// FIXME: Probably doesn't work correctly with restored games
	static bool flashlight_was_on = false, googles_were_worn = false;
	Room *room = &_rooms[_currentRoom];

	/*
	 * Check if the room needs to be redrawn because the flashlight
	 * was switch off or on.
	 */
	if (_flags[OO_FLAG_FLASHLIGHT_ON] != flashlight_was_on &&
	        (room->_flags & OO_ROOM_FLAG_DARK)) {
		flashlight_was_on = _flags[OO_FLAG_FLASHLIGHT_ON];
		_updateFlags |= UPDATE_GRAPHICS | UPDATE_ROOM_DESC;
	}

	/*
	 * Check if the room needs to be redrawn because the goggles were
	 * put on or removed.
	 */
	if (_flags[OO_FLAG_WEARING_GOGGLES] != googles_were_worn &&
	        _currentRoom == OO_BRIGHT_ROOM) {
		googles_were_worn = _flags[OO_FLAG_WEARING_GOGGLES];
		_updateFlags |= UPDATE_GRAPHICS | UPDATE_ROOM_DESC;
	}
}

void OOToposGame::handleSpecialOpcode(uint8 operand) {
	switch (operand) {
	case 0x03:
	// Game over - failure
	// fall through
	case 0x05:
	// Won the game
	// fall through
	case 0x04:
		// Restart game
		game_restart();
		break;

	case 0x06:
		// Save game
		game_save();
		break;

	case 0x07:
		// Restore game
		game_restore();
		break;
	}
}

} // namespace Comprehend
} // namespace Glk
