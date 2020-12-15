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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/game_tr2.h"
#include "glk/comprehend/pics.h"

namespace Glk {
namespace Comprehend {

enum RoomId {
	ROOM_CLAY_HUT = 7,
	ROOM_FIELD = 26
};

enum RoomFlag {
	ROOMFLAG_FOREST = 1 << 0,
	ROOMFLAG_WEREWOLF = 1 << 6,
	ROOMFLAG_VAMPIRE = 1 << 7
};

enum ItemId {
	ITEM_GOBLIN = 9,
	ITEM_SILVER_BULLET = 21,
	ITEM_BLACK_CAT = 23,
	ITEM_WEREWOLF = 33,
	ITEM_VAMPIRE = 38
};

struct TransylvaniaMonster {
	uint8 _object;
	uint8 _deadFlag;
	uint _roomAllowFlag;
	uint _minTurnsBefore;
	uint _randomness;
};


const TransylvaniaMonster TransylvaniaGame2::WEREWOLF = {
	ITEM_WEREWOLF, 7, ROOMFLAG_WEREWOLF, 10, 190
};

const TransylvaniaMonster TransylvaniaGame2::VAMPIRE = {
	ITEM_VAMPIRE, 5, ROOMFLAG_VAMPIRE, 0, 200
};

static const GameStrings TR_STRINGS = {
	EXTRA_STRING_TABLE(0x8a)
};


TransylvaniaGame2::TransylvaniaGame2() : ComprehendGameV2(),
		_miceReleased(false) {
	_gameDataFile = "g0";

	_locationGraphicFiles.push_back("RA");
	_locationGraphicFiles.push_back("RB");
	_locationGraphicFiles.push_back("RC");
	_itemGraphicFiles.push_back("OA");
	_itemGraphicFiles.push_back("OB");
	_itemGraphicFiles.push_back("OC");

	_titleGraphicFile = "t0";
	_gameStrings = &TR_STRINGS;
}

bool TransylvaniaGame2::updateMonster(const TransylvaniaMonster *monsterInfo) {
	Item *monster;
	Room *room;
	uint16 turn_count;

	room = &_rooms[_currentRoom];
	if (!(room->_flags & monsterInfo->_roomAllowFlag))
		return false;

	turn_count = _variables[VAR_TURN_COUNT];
	monster = get_item(monsterInfo->_object);

	if (monster->_room == _currentRoom) {
		// The monster is in the current room - leave it there
		return true;
	}

	if (!_flags[monsterInfo->_deadFlag] &&
	        turn_count > monsterInfo->_minTurnsBefore) {
		/*
		 * The monster is alive and allowed to move to the current
		 * room. Randomly decide whether on not to. If not, move
		 * it back to limbo.
		 */
		if (getRandomNumber(255) > monsterInfo->_randomness) {
			move_object(monster, _currentRoom);
			_variables[15] = turn_count + 1;
		} else {
			move_object(monster, ROOM_NOWHERE);
		}
	}

	return true;
}

bool TransylvaniaGame2::isMonsterInRoom(const TransylvaniaMonster *monsterInfo) {
	Item *monster = get_item(monsterInfo->_object);
	return monster->_room == _currentRoom;
}

int TransylvaniaGame2::roomIsSpecial(unsigned room_index,
                                    unsigned *roomDescString) {
	Room *room = &_rooms[room_index];

	if (room_index == 0x28) {
		if (roomDescString)
			*roomDescString = room->_stringDesc;
		return ROOM_IS_DARK;
	}

	return ROOM_IS_NORMAL;
}

void TransylvaniaGame2::beforeTurn() {
	Room *room;

	if (!isMonsterInRoom(&WEREWOLF) && !isMonsterInRoom(&VAMPIRE)) {
		if (_currentRoom == ROOM_CLAY_HUT) {
			Item *blackCat = get_item(ITEM_BLACK_CAT);
			if (blackCat->_room == _currentRoom && getRandomNumber(255) >= 128)
				console_println(_strings[109].c_str());
			goto done;

		} else if (_currentRoom == ROOM_FIELD) {
			Item *goblin = get_item(ITEM_GOBLIN);
			if (goblin->_room == _currentRoom)
				console_println(_strings[94 + getRandomNumber(3)].c_str());
			goto done;

		}
	}

	if (updateMonster(&WEREWOLF) || updateMonster(&VAMPIRE))
		goto done;

	room = &_rooms[_currentRoom];
	if ((room->_flags & ROOMFLAG_FOREST) && (_variables[VAR_TURN_COUNT] % 255) >= 4
			&& getRandomNumber(255) < 40) {
		int stringNum = _miceReleased ? 108 : 107;
		console_println(_strings[stringNum].c_str());

		// Until the mice are released, an eagle moves player to a random room
		if (!_miceReleased) {
			// Get new room to get moved to
			int roomNum = getRandomNumber(3) + 1;
			if (roomNum == _currentRoom)
				roomNum += 15;

			move_to(roomNum);

			// Make sure Werwolf and Vampire aren't present
			get_item(ITEM_WEREWOLF)->_room = 0xff;
			get_item(ITEM_VAMPIRE)->_room = 0xff;
		}
	}

done:
	ComprehendGameV2::beforeTurn();
}

void TransylvaniaGame2::synchronizeSave(Common::Serializer &s) {
	ComprehendGame::synchronizeSave(s);
	s.syncAsByte(_miceReleased);

	// As a post-step, ensure the vampire and werewolf aren't present
	get_item(ITEM_WEREWOLF)->_room = 0xff;
	get_item(ITEM_VAMPIRE)->_room = 0xff;
}

void TransylvaniaGame2::handleSpecialOpcode() {
	switch (_specialOpcode) {
	case 1:
		// Mice have been released
		_miceReleased = true;
		break;

	case 2:
		// Gun is fired. Drop the bullet in a random room
		get_item(ITEM_SILVER_BULLET)->_room = getRandomNumber(7) + 1;
		_updateFlags |= UPDATE_GRAPHICS;
		break;

	case 3:
	case 4:
		// Game over - failure
		console_println(_strings2[138].c_str());
		game_restart();
		break;

	case 5:
		// Won the game
		g_comprehend->showGraphics();
		g_comprehend->drawLocationPicture(40);
		game_restart();
		break;

	case 6:
		game_save();
		break;

	case 7:
		game_restore();
		break;

	case 8:
		// Restart game
		game_restart();
		break;

	case 9:
		// Show the Zin screen in reponse to doing
		// 'sing some enchanted evening' in his cabin.
		g_comprehend->showGraphics();
		g_comprehend->drawLocationPicture(41);
		console_get_key();
		_updateFlags |= UPDATE_GRAPHICS;
		break;

	default:
		break;
	}
}

#define READ_LINE do { \
	g_comprehend->readLine(buffer, sizeof(buffer)); \
	if (g_comprehend->shouldQuit()) return; \
	} while (strlen(buffer) == 0)

void TransylvaniaGame2::beforeGame() {
	char buffer[128];
	g_comprehend->setDisableSaves(true);

	// Draw the title
	g_comprehend->drawPicture(TITLE_IMAGE);

	// Print game information
	console_println("Story and graphics by Antonio Antiochia.");
	console_println("IBM version by Jeffrey A. Jay. Copyright 1987  POLARWARE, Inc.");
	g_comprehend->readChar();

	// Welcome to Transylvania - sign your name
	console_println(_strings[0x20].c_str());
	READ_LINE;

	// The player's name is stored in word 0
	_replaceWords[0] = Common::String(buffer);

	// And your next of kin - This isn't stored by the game
	console_println(_strings[0x21].c_str());
	READ_LINE;

	g_comprehend->setDisableSaves(false);
}

} // namespace Comprehend
} // namespace Glk
