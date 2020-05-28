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

#ifndef GLK_COMPREHEND_DUMP_GAME_DATA_H
#define GLK_COMPREHEND_DUMP_GAME_DATA_H

namespace Glk {
namespace Comprehend {

struct comprehend_game;
struct function_state;
struct instruction;

#define DUMP_STRINGS (1 << 0)
#define DUMP_EXTRA_STRINGS (1 << 1)
#define DUMP_ROOMS (1 << 2)
#define DUMP_ITEMS (1 << 3)
#define DUMP_DICTIONARY (1 << 4)
#define DUMP_WORD_PAIRS (1 << 5)
#define DUMP_ACTIONS (1 << 6)
#define DUMP_FUNCTIONS (1 << 7)
#define DUMP_REPLACE_WORDS (1 << 8)
#define DUMP_HEADER (1 << 9)
#define DUMP_ALL (~0)

void dump_instruction(struct comprehend_game *game,
                      struct function_state *func_state,
                      struct instruction *instr);
void dump_game_data(struct comprehend_game *game, unsigned flags);

} // namespace Comprehend
} // namespace Glk

#endif
