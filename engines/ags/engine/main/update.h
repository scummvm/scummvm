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

#ifndef AGS_ENGINE_MAIN_UPDATE_H
#define AGS_ENGINE_MAIN_UPDATE_H

namespace AGS3 {

// Update MoveList of certain index, save current position;
// *resets* mslot to zero if path is complete.
// returns "need_to_fix_sprite" value, which may be 0,1,2;
// TODO: find out what this return value means, and refactor.
// TODO: do not reset mslot in this function, reset externally instead.
int do_movelist_move(short &mslot, int &pos_x, int &pos_y);
// Recalculate derived (non-serialized) values in movelists
void restore_movelists();
// Update various things on the game frame (historical code mess...)
void update_stuff();

} // namespace AGS3

#endif
