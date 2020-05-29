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
#include "glk/comprehend/graphics.h"

namespace Glk {
namespace Comprehend {

#define OO_ROOM_FLAG_DARK 0x02

#define OO_BRIGHT_ROOM 0x19

#define OO_FLAG_WEARING_GOGGLES 0x1b
#define OO_FLAG_FLASHLIGHT_ON 0x27

OOToposGame::OOToposGame() : comprehend_game() {
	game_name = "Oo-Topos";
	short_name = "oo";
	game_data_file = "g0";

	// Extra strings are (annoyingly) stored in the game binary
	string_files.push_back(string_file("NOVEL.EXE", 0x16564, 0x17640));
	string_files.push_back(string_file("NOVEL.EXE", 0x17702, 0x18600));
	string_files.push_back(string_file("NOVEL.EXE", 0x186b2, 0x19b80));
	string_files.push_back(string_file("NOVEL.EXE", 0x19c62, 0x1a590));
	string_files.push_back(string_file("NOVEL.EXE", 0x1a634, 0x1b080));
	location_graphic_files.push_back("RA");
	location_graphic_files.push_back("RB");
	location_graphic_files.push_back("RC");
	location_graphic_files.push_back("RD");
	location_graphic_files.push_back("RE");
	item_graphic_files.push_back("OA");
	item_graphic_files.push_back("OB");
	item_graphic_files.push_back("OC");
	item_graphic_files.push_back("OD");

	save_game_file_fmt = "G%d";
	color_table = 1;
}

int OOToposGame::room_is_special(unsigned room_index,
                              unsigned *room_desc_string) {
	room *room = &info->rooms[room_index];

	/* Is the room dark */
	if ((room->flags & OO_ROOM_FLAG_DARK) &&
	    !(info->flags[OO_FLAG_FLASHLIGHT_ON])) {
		if (room_desc_string)
			*room_desc_string = 0xb3;
		return ROOM_IS_DARK;
	}

	/* Is the room too bright */
	if (room_index == OO_BRIGHT_ROOM &&
	    !info->flags[OO_FLAG_WEARING_GOGGLES]) {
		if (room_desc_string)
			*room_desc_string = 0x1c;
		return ROOM_IS_TOO_BRIGHT;
	}

	return ROOM_IS_NORMAL;
}

bool OOToposGame::before_turn() {
	/* FIXME - probably doesn't work correctly with restored games */
	static bool flashlight_was_on = false, googles_were_worn = false;
	struct room *room = &info->rooms[info->current_room];

	/* 
	 * Check if the room needs to be redrawn because the flashlight
	 * was switch off or on.
	 */
	if (info->flags[OO_FLAG_FLASHLIGHT_ON] != flashlight_was_on &&
	    (room->flags & OO_ROOM_FLAG_DARK)) {
		flashlight_was_on = info->flags[OO_FLAG_FLASHLIGHT_ON];
		info->update_flags |= UPDATE_GRAPHICS | UPDATE_ROOM_DESC;
	}

	/*
	 * Check if the room needs to be redrawn because the goggles were
	 * put on or removed.
	 */
	if (info->flags[OO_FLAG_WEARING_GOGGLES] != googles_were_worn &&
	    info->current_room == OO_BRIGHT_ROOM) {
		googles_were_worn = info->flags[OO_FLAG_WEARING_GOGGLES];
		info->update_flags |= UPDATE_GRAPHICS | UPDATE_ROOM_DESC;
	}

	return false;
}

void OOToposGame::handle_special_opcode(uint8 operand) {
	switch (operand) {
	case 0x03:
		/* Game over - failure */
	case 0x05:
		/* Won the game */
	case 0x04:
		/* Restart game */
		game_restart(this);
		break;

	case 0x06:
		/* Save game */
		game_save(this);
		break;

	case 0x07:
		/* Restore game */
		game_restore(this);
		break;
	}
}

} // namespace Comprehend
} // namespace Glk
