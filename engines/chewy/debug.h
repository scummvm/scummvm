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

#ifndef CHEWY_DEBUG_H
#define CHEWY_DEBUG_H

namespace Chewy {

void get_room_nr();

void room_info();

void plot_auto_mov(int16 mode);

void go_auto_mov();

void plot_maus();
void plot_rect();
void get_auto_mov();
void start_detail();
void play_last_ani(int16 nr);

void set_z_ebene();
void get_phase(ObjMov *om);
//FIXME: was using Stream *
void test_load(void *handle, taf_info *Tt, int16 anz);

} // namespace Chewy

#endif
