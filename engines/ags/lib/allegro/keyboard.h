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

#include "common/keyboard.h"

namespace AGS3 {

#define KB_SHIFT_FLAG Common::KBD_SHIFT
#define KB_CTRL_FLAG Common::KBD_CTRL
#define KB_ALT_FLAG Common::KBD_ALT

#define KEY_LSHIFT Common::KEYCODE_LSHIFT
#define KEY_RSHIFT Common::KEYCODE_RSHIFT
#define KEY_ALT Common::KEYCODE_LALT
#define KEY_LCONTROL Common::KEYCODE_LCTRL
#define KEY_RCONTROL Common::KEYCODE_RCTRL
#define KEY_ALTGR 0
#define KEY_F9 Common::KEYCODE_F9

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

extern bool key[Common::KEYCODE_LAST];

extern int install_keyboard();
extern void remove_keyboard();

} // namespace AGS3

#endif
