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

#ifndef CHEWY_EPISODE4_H
#define CHEWY_EPISODE4_H

namespace Chewy {

void switch_room(int16 nr);

void r66_entry(int16 eib_nr);
void r66_exit(int16 eib_nr);
void r66_talk1();
void r66_talk2();
void r66_talk3();
void r66_talk4();
int r66_proc2();
int r66_proc7();

void r67_entry();

void r67_exit();

void r67_setup_func();

int16 r67_use_grammo();

void r67_look_brief();

int16 r67_use_kommode();

void r67_kostuem_aad(int16 aad_nr);

int16 r67_talk_papagei();

void r68_entry();

void r68_exit();
void r68_setup_func();

void r68_look_kaktus();

void r68_talk_indigo();

void r68_talk_indigo(int16 aad_nr);

int16 r68_use_indigo();

void r68_talk_keeper();

int16 r68_use_papagei();

void r68_calc_diva();

int16 r68_use_keeper();

int16 r68_use_diva();

void r68_kostuem_aad(int16 aad_nr);

void r68_talk_papagei();

void r69_entry(int16 eib_nr);

void r69_exit(int16 eib_nr);
void r69_setup_func();

void r69_look_schild();

int16 r69_use_bruecke();

} // namespace Chewy

#endif
