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

#ifndef AGS_LIB_ALLEGRO_KEYBOARD_H
#define AGS_LIB_ALLEGRO_KEYBOARD_H

#include "ags/lib/allegro/alconfig.h"
#include "common/keyboard.h"

namespace AGS3 {

#define __allegro_KEY_MAX Common::KEYCODE_LAST

#define KB_SHIFT_FLAG Common::KBD_SHIFT
#define KB_CTRL_FLAG Common::KBD_CTRL
#define KB_ALT_FLAG Common::KBD_ALT

#define KEY_LSHIFT Common::KEYCODE_LSHIFT
#define KEY_RSHIFT Common::KEYCODE_RSHIFT
#define KEY_ALT Common::KEYCODE_LALT
#define KEY_LCONTROL Common::KEYCODE_LCTRL
#define KEY_RCONTROL Common::KEYCODE_RCTRL
#define KEY_SCRLOCK Common::KEYCODE_SCROLLOCK
#define KEY_ALTGR 0
#define KEY_F9 Common::KEYCODE_F9
#define KEY_A Common::KEYCODE_a

#define __allegro_KEY_LSHIFT Common::KEYCODE_LSHIFT
#define __allegro_KEY_RSHIFT Common::KEYCODE_RSHIFT
#define __allegro_KEY_LCONTROL Common::KEYCODE_LCTRL
#define __allegro_KEY_RCONTROL Common::KEYCODE_RCTRL
#define __allegro_KEY_ALT Common::KEYCODE_LALT

#define __allegro_KEY_F1 Common::KEYCODE_F1
#define __allegro_KEY_F2 Common::KEYCODE_F2
#define __allegro_KEY_F3 Common::KEYCODE_F3
#define __allegro_KEY_F4 Common::KEYCODE_F4
#define __allegro_KEY_F5 Common::KEYCODE_F5
#define __allegro_KEY_F6 Common::KEYCODE_F6
#define __allegro_KEY_F7 Common::KEYCODE_F7
#define __allegro_KEY_F8 Common::KEYCODE_F8
#define __allegro_KEY_F9 Common::KEYCODE_F9
#define __allegro_KEY_F10 Common::KEYCODE_F10
#define __allegro_KEY_F11 Common::KEYCODE_F11
#define __allegro_KEY_F12 Common::KEYCODE_F12

#define __allegro_KEY_HOME Common::KEYCODE_HOME
#define __allegro_KEY_UP Common::KEYCODE_UP
#define __allegro_KEY_PGUP Common::KEYCODE_PAGEUP
#define __allegro_KEY_LEFT Common::KEYCODE_LEFT
#define __allegro_KEY_RIGHT Common::KEYCODE_RIGHT
#define __allegro_KEY_END Common::KEYCODE_END
#define __allegro_KEY_DOWN Common::KEYCODE_DOWN
#define __allegro_KEY_PGDN Common::KEYCODE_PAGEDOWN
#define __allegro_KEY_INSERT Common::KEYCODE_INSERT
#define __allegro_KEY_DEL Common::KEYCODE_DELETE

#define __allegro_KEY_0_PAD Common::KEYCODE_KP0
#define __allegro_KEY_1_PAD Common::KEYCODE_KP1
#define __allegro_KEY_2_PAD Common::KEYCODE_KP2
#define __allegro_KEY_3_PAD Common::KEYCODE_KP3
#define __allegro_KEY_4_PAD Common::KEYCODE_KP4
#define __allegro_KEY_5_PAD Common::KEYCODE_KP5
#define __allegro_KEY_6_PAD Common::KEYCODE_KP6
#define __allegro_KEY_7_PAD Common::KEYCODE_KP7
#define __allegro_KEY_8_PAD Common::KEYCODE_KP8
#define __allegro_KEY_9_PAD Common::KEYCODE_KP9
#define __allegro_KEY_DEL_PAD Common::KEYCODE_KP_PERIOD

#define __allegro_KEY_PRTSCR Common::KEYCODE_PRINT
#define __allegro_KEY_PAUSE Common::KEYCODE_PAUSE
#define __allegro_KEY_ABNT_C1 94
#define __allegro_KEY_YEN 95
#define __allegro_KEY_KANA 96
#define __allegro_KEY_CONVERT 97
#define __allegro_KEY_NOCONVERT 98
#define __allegro_KEY_CIRCUMFLEX 100
#define __allegro_KEY_KANJI 102
#define __allegro_KEY_ALTGR 120
#define __allegro_KEY_LWIN Common::KEYCODE_LMETA
#define __allegro_KEY_RWIN Common::KEYCODE_RMETA
#define __allegro_KEY_MENU 123
#define __allegro_KEY_SCRLOCK Common::KEYCODE_SCROLLOCK
#define __allegro_KEY_NUMLOCK Common::KEYCODE_NUMLOCK
#define __allegro_KEY_CAPSLOCK Common::KEYCODE_CAPSLOCK

#define __allegro_KEY_0 Common::KEYCODE_0
#define __allegro_KEY_1 Common::KEYCODE_1
#define __allegro_KEY_2 Common::KEYCODE_2
#define __allegro_KEY_3 Common::KEYCODE_3
#define __allegro_KEY_4 Common::KEYCODE_4
#define __allegro_KEY_5 Common::KEYCODE_5
#define __allegro_KEY_6 Common::KEYCODE_6
#define __allegro_KEY_7 Common::KEYCODE_7
#define __allegro_KEY_8 Common::KEYCODE_8
#define __allegro_KEY_9 Common::KEYCODE_9

#define __allegro_KEY_A Common::KEYCODE_a
#define __allegro_KEY_B Common::KEYCODE_b
#define __allegro_KEY_C Common::KEYCODE_c
#define __allegro_KEY_D Common::KEYCODE_d
#define __allegro_KEY_E Common::KEYCODE_e
#define __allegro_KEY_F Common::KEYCODE_f
#define __allegro_KEY_G Common::KEYCODE_g
#define __allegro_KEY_H Common::KEYCODE_h
#define __allegro_KEY_I Common::KEYCODE_i
#define __allegro_KEY_J Common::KEYCODE_j
#define __allegro_KEY_K Common::KEYCODE_k
#define __allegro_KEY_L Common::KEYCODE_l
#define __allegro_KEY_M Common::KEYCODE_m
#define __allegro_KEY_N Common::KEYCODE_n
#define __allegro_KEY_O Common::KEYCODE_o
#define __allegro_KEY_P Common::KEYCODE_p
#define __allegro_KEY_Q Common::KEYCODE_q
#define __allegro_KEY_R Common::KEYCODE_r
#define __allegro_KEY_S Common::KEYCODE_s
#define __allegro_KEY_T Common::KEYCODE_t
#define __allegro_KEY_U Common::KEYCODE_u
#define __allegro_KEY_V Common::KEYCODE_v
#define __allegro_KEY_W Common::KEYCODE_w
#define __allegro_KEY_X Common::KEYCODE_x
#define __allegro_KEY_Y Common::KEYCODE_y
#define __allegro_KEY_Z Common::KEYCODE_z


#define __allegro_KEY_BACKSPACE Common::KEYCODE_BACKSPACE
#define __allegro_KEY_TAB Common::KEYCODE_TAB
#define __allegro_KEY_ENTER Common::KEYCODE_RETURN
#define __allegro_KEY_ENTER_PAD Common::KEYCODE_KP_ENTER
#define __allegro_KEY_ESC Common::KEYCODE_ESCAPE
#define __allegro_KEY_SPACE Common::KEYCODE_SPACE
#define __allegro_KEY_QUOTE Common::KEYCODE_QUOTE
#define __allegro_KEY_COMMA Common::KEYCODE_COMMA
#define __allegro_KEY_STOP 0
#define __allegro_KEY_SLASH Common::KEYCODE_SLASH
#define __allegro_KEY_SLASH_PAD Common::KEYCODE_KP_DIVIDE
#define __allegro_KEY_BACKSLASH Common::KEYCODE_BACKSLASH
#define __allegro_KEY_BACKSLASH2 Common::KEYCODE_BACKSLASH
#define __allegro_KEY_SEMICOLON Common::KEYCODE_SEMICOLON
#define __allegro_KEY_EQUALS Common::KEYCODE_EQUALS
#define __allegro_KEY_EQUALS_PAD Common::KEYCODE_KP_EQUALS
#define __allegro_KEY_OPENBRACE 123
#define __allegro_KEY_CLOSEBRACE 125
#define __allegro_KEY_PLUS_PAD Common::KEYCODE_KP_PLUS
#define __allegro_KEY_MINUS Common::KEYCODE_MINUS
#define __allegro_KEY_MINUS_PAD Common::KEYCODE_KP_MINUS
#define __allegro_KEY_COLON Common::KEYCODE_COLON
#define __allegro_KEY_COLON2 Common::KEYCODE_COLON
#define __allegro_KEY_ASTERISK Common::KEYCODE_ASTERISK
#define __allegro_KEY_AT Common::KEYCODE_AT

extern bool key[Common::KEYCODE_LAST];

AL_FUNC(bool, keyboard_needs_poll, (void));
AL_FUNC(int, poll_keyboard, (void));

extern int install_keyboard();
extern void remove_keyboard();
AL_FUNC(void, simulate_keypress, (int keycode));
AL_FUNC(void, simulate_ukeypress, (int keycode, int scancode));

AL_FUNC(bool, keypressed, (void));
AL_FUNC(int, readkey, (void));

} // namespace AGS3

#endif
