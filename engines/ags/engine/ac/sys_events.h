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

#ifndef AGS_ENGINE_AC_SYS_EVENTS_H
#define AGS_ENGINE_AC_SYS_EVENTS_H

int  ags_getch ();
int  ags_kbhit ();
int  ags_iskeypressed (int keycode);

int  ags_misbuttondown (int but);
int  ags_mgetbutton();
void ags_domouse (int what);
int  ags_check_mouse_wheel ();

// Clears buffered keypresses and mouse clicks, if any
void ags_clear_input_buffer();
// Halts execution until any user input
// TODO: seriously not a good design, replace with event listening
void ags_wait_until_keypress();

#endif
