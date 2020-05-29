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
#include "glk/comprehend/game_tr.h"
#include "glk/comprehend/util.h"

namespace Glk {
namespace Comprehend {

const tr_monster TransylvaniaGame::WEREWOLF = {
	0x21, 7, (1 << 6), 5, 5
};

const tr_monster TransylvaniaGame::VAMPIRE = {
	0x26, 5, (1 << 7), 0, 5
};

static struct game_strings tr_strings = {
    EXTRA_STRING_TABLE(0x8a)
};


TransylvaniaGame::TransylvaniaGame() : ComprehendGame() {
	_gameName = "Transylvania";
	_shortName = "tr";
	_gameDataFile = "tr.gda";

	_stringFiles.push_back(string_file("MA.MS1", 0x88));
	_stringFiles.push_back(string_file("MB.MS1", 0x88));
	_stringFiles.push_back(string_file("MC.MS1", 0x88));
	_stringFiles.push_back(string_file("MD.MS1", 0x88));
	_stringFiles.push_back(string_file("ME.MS1", 0x88));

    _locationGraphicFiles.push_back("RA.MS1");
	_locationGraphicFiles.push_back("RB.MS1");
	_locationGraphicFiles.push_back("RC.MS1");

	_itemGraphicFiles.push_back("OA.MS1");
	_itemGraphicFiles.push_back("OB.MS1");
	_itemGraphicFiles.push_back("OC.MS1");

	_savegameFileFormat = "G%d.MS0";
	_gameStrings = &tr_strings;
};

void TransylvaniaGame::update_monster(const tr_monster *monster_info) {
	struct item *monster;
	struct room *room;
	uint16 turn_count;

	room = &info->_rooms[info->_currentRoom];
	turn_count = info->_variables[VAR_TURN_COUNT];

	monster = get_item(this, monster_info->object);
	if (monster->room == info->_currentRoom) {
		/* The monster is in the current room - leave it there */
		return;
	}

	if ((room->flags & monster_info->room_allow_flag) &&
	    !info->_flags[monster_info->dead_flag] &&
	    turn_count > monster_info->min_turns_before) {
		/*
		 * The monster is alive and allowed to move to the current
		 * room. Randomly decide whether on not to. If not, move
		 * it back to limbo.
		 */
		if ((g_comprehend->getRandomNumber(0x7fffffff) % monster_info->randomness) == 0) {
			move_object(this, monster, info->_currentRoom);
			info->_variables[0xf] = turn_count + 1;
		} else {
			move_object(this, monster, ROOM_NOWHERE);
		}
	}
}

int TransylvaniaGame::room_is_special(unsigned room_index,
			      unsigned *room_desc_string)
{
	struct room *room = &info->_rooms[room_index];

	if (room_index == 0x28) {
		if (room_desc_string)
			*room_desc_string = room->string_desc;
		return ROOM_IS_DARK;
	}

	return ROOM_IS_NORMAL;
}

bool TransylvaniaGame::before_turn() {
	update_monster(&WEREWOLF);
	update_monster(&VAMPIRE);
	return false;
}

void TransylvaniaGame::handle_special_opcode(uint8 operand)
{
	switch (operand) {
	case 0x01:
		/*
		 * FIXME - Called when the mice are dropped and the cat chases
		 *         them.
		 */
		break;

	case 0x02:
		/* FIXME - Called when the gun is fired */
		break;

	case 0x06:
		game_save(this);
		break;

	case 0x07:
		game_restore(this);
		break;

	case 0x03:
		/* Game over - failure */
	case 0x05:
		/* Won the game */
	case 0x08:
		/* Restart game */
		game_restart(this);
		break;

	case 0x09:
		/*
		 * Show the Zin screen in reponse to doing 'sing some enchanted
		 * evening' in his cabin.
		 */
		draw_location_image(&info->_roomImages, 41);
		console_get_key();
		info->_updateFlags |= UPDATE_GRAPHICS;
		break;
	}
}

static void read_string(char *buffer, size_t size)
{
#ifdef TODO
	char *p;

	printf("> ");
	fgets(buffer, size, stdin);

	/* Remove trailing newline */
	p = strchr(buffer, '\n');
	if (p)
		*p = '\0';
#else
	error("TODO");
#endif
}

void TransylvaniaGame::before_game() {
	char buffer[128];

	/* Welcome to Transylvania - sign your name */
	console_println(this, info->_strings.strings[0x20]);
	read_string(buffer, sizeof(buffer));

	/*
	 * Transylvania uses replace word 0 as the player's name, the game
	 * data file stores a bunch of dummy characters, so the length is
	 * limited (the original game will break if you put a name in that
	 * is too long).
	 */
	if (!info->_replaceWords[0])
		info->_replaceWords[0] = xstrndup(buffer, strlen(buffer));
	else
		snprintf(info->_replaceWords[0],
			 strlen(info->_replaceWords[0]),
			 "%s", buffer);

	/* And your next of kin - This isn't store by the game */
	console_println(this, info->_strings.strings[0x21]);
	read_string(buffer, sizeof(buffer));
}

} // namespace Comprehend
} // namespace Glk
