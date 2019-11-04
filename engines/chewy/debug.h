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

#ifndef CHEWY_DEBUG_H
#define CHEWY_DEBUG_H

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
//FIXME: was using FILE *
void test_load(void *handle, taf_info *Tt, int16 anz);

#endif
