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

#ifndef GLK_COMPREHEND_GAME_H
#define GLK_COMPREHEND_GAME_H

#include "glk/comprehend/game_data.h"
#include "common/scummsys.h"

namespace Glk {
namespace Comprehend {

#define MAX_FILES 10

struct comprehend_game {
	const char *game_name;
	const char *short_name;

	const char *game_data_file;
	struct string_file string_files[MAX_FILES];
	const char *location_graphic_files[MAX_FILES];
	const char *item_graphic_files[MAX_FILES];
	const char *save_game_file_fmt;
	unsigned color_table;

	struct game_strings *strings;
	struct game_ops *ops;

	struct game_info *info;
};

void console_println(struct comprehend_game *game, const char *text);
int console_get_key(void);

struct item *get_item(struct comprehend_game *game, uint16 index);
void move_object(struct comprehend_game *game, struct item *item, int new_room);
void eval_function(struct comprehend_game *game, struct function *func,
		   struct word *verb, struct word *noun);

void comprehend_play_game(struct comprehend_game *game);
void game_save(struct comprehend_game *game);
void game_restore(struct comprehend_game *game);
void game_restart(struct comprehend_game *game);

} // namespace Comprehend
} // namespace Glk

#endif
