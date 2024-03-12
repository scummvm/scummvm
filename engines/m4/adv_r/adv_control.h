
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

#ifndef M4_ADV_R_ADV_CONTROL_H
#define M4_ADV_R_ADV_CONTROL_H

#include "common/stream.h"
#include "m4/m4_types.h"

namespace M4 {

bool kernel_section_startup();
void player_set_commands_allowed(bool t_or_f);
void game_pause(bool flag);
bool this_is_a_walkcode(int32 x, int32 y);
int32 get_screen_depth(int32 x, int32 y);
int32 get_screen_color(int32 x, int32 y);
void update_mouse_pos_dialog();

} // End of namespace M4

#endif
