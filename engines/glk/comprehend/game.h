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

#ifndef GLK_ComprehendGame_H
#define GLK_ComprehendGame_H

#include "common/array.h"
#include "glk/comprehend/game_data.h"

namespace Glk {
namespace Comprehend {

#define ROOM_IS_NORMAL 0
#define ROOM_IS_DARK 1
#define ROOM_IS_TOO_BRIGHT 2

class ComprehendGame {
public:
	const char *game_name;
	const char *short_name;

	const char *game_data_file;
	Common::Array<string_file> string_files;
	Common::Array<const char *> location_graphic_files;
	Common::Array<const char *> item_graphic_files;
	const char *save_game_file_fmt;
	unsigned color_table;

	struct game_strings *strings;
	struct game_info *info;

public:
	ComprehendGame();
	virtual ~ComprehendGame();

	virtual void before_game() {}
	virtual void before_prompt() {}
	virtual bool before_turn() {
		return false;
	}
	virtual bool after_turn() {
		return false;
	}
	virtual int room_is_special(unsigned room_index,
		unsigned *room_desc_string) {
		return ROOM_IS_NORMAL;
	}
	virtual void handle_special_opcode(uint8 operand) {}
};

void console_println(ComprehendGame *game, const char *text);
int console_get_key(void);

struct item *get_item(ComprehendGame *game, uint16 index);
void move_object(ComprehendGame *game, struct item *item, int new_room);
void eval_function(ComprehendGame *game, struct function *func,
                   struct word *verb, struct word *noun);

void comprehend_play_game(ComprehendGame *game);
void game_save(ComprehendGame *game);
void game_restore(ComprehendGame *game);
void game_restart(ComprehendGame *game);

} // namespace Comprehend
} // namespace Glk

#endif
