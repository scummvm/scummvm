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

#ifndef M4_PLATFORM_KEYS_H
#define M4_PLATFORM_KEYS_H

#include "common/events.h"
#include "m4/m4_types.h"

namespace M4 {

enum {
	KEY_CTRL_A = (Common::KBD_CTRL << 16) | Common::KEYCODE_a,
	KEY_CTRL_B = (Common::KBD_CTRL << 16) | Common::KEYCODE_b,
	KEY_CTRL_C = (Common::KBD_CTRL << 16) | Common::KEYCODE_c,
	KEY_CTRL_D = (Common::KBD_CTRL << 16) | Common::KEYCODE_d,
	KEY_CTRL_E = (Common::KBD_CTRL << 16) | Common::KEYCODE_e,
	KEY_CTRL_F = (Common::KBD_CTRL << 16) | Common::KEYCODE_f,
	KEY_CTRL_G = (Common::KBD_CTRL << 16) | Common::KEYCODE_g,
	KEY_CTRL_H = (Common::KBD_CTRL << 16) | Common::KEYCODE_h,
	KEY_CTRL_I = (Common::KBD_CTRL << 16) | Common::KEYCODE_i,
	KEY_CTRL_J = (Common::KBD_CTRL << 16) | Common::KEYCODE_j,
	KEY_CTRL_K = (Common::KBD_CTRL << 16) | Common::KEYCODE_k,
	KEY_CTRL_L = (Common::KBD_CTRL << 16) | Common::KEYCODE_l,
	KEY_CTRL_M = (Common::KBD_CTRL << 16) | Common::KEYCODE_m,
	KEY_CTRL_N = (Common::KBD_CTRL << 16) | Common::KEYCODE_n,
	KEY_CTRL_O = (Common::KBD_CTRL << 16) | Common::KEYCODE_o,
	KEY_CTRL_P = (Common::KBD_CTRL << 16) | Common::KEYCODE_p,
	KEY_CTRL_Q = (Common::KBD_CTRL << 16) | Common::KEYCODE_q,
	KEY_CTRL_R = (Common::KBD_CTRL << 16) | Common::KEYCODE_r,
	KEY_CTRL_S = (Common::KBD_CTRL << 16) | Common::KEYCODE_s,
	KEY_CTRL_T = (Common::KBD_CTRL << 16) | Common::KEYCODE_t,
	KEY_CTRL_U = (Common::KBD_CTRL << 16) | Common::KEYCODE_u,
	KEY_CTRL_V = (Common::KBD_CTRL << 16) | Common::KEYCODE_v,
	KEY_CTRL_W = (Common::KBD_CTRL << 16) | Common::KEYCODE_w,
	KEY_CTRL_X = (Common::KBD_CTRL << 16) | Common::KEYCODE_x,
	KEY_CTRL_Y = (Common::KBD_CTRL << 16) | Common::KEYCODE_y,
	KEY_CTRL_Z = (Common::KBD_CTRL << 16) | Common::KEYCODE_z,

	KEY_ALT_A = (Common::KBD_ALT << 16) | Common::KEYCODE_a,
	KEY_ALT_B = (Common::KBD_ALT << 16) | Common::KEYCODE_b,
	KEY_ALT_C = (Common::KBD_ALT << 16) | Common::KEYCODE_c,
	KEY_ALT_D = (Common::KBD_ALT << 16) | Common::KEYCODE_d,
	KEY_ALT_E = (Common::KBD_ALT << 16) | Common::KEYCODE_e,
	KEY_ALT_F = (Common::KBD_ALT << 16) | Common::KEYCODE_f,
	KEY_ALT_G = (Common::KBD_ALT << 16) | Common::KEYCODE_g,
	KEY_ALT_H = (Common::KBD_ALT << 16) | Common::KEYCODE_h,
	KEY_ALT_I = (Common::KBD_ALT << 16) | Common::KEYCODE_i,
	KEY_ALT_J = (Common::KBD_ALT << 16) | Common::KEYCODE_j,
	KEY_ALT_K = (Common::KBD_ALT << 16) | Common::KEYCODE_k,
	KEY_ALT_L = (Common::KBD_ALT << 16) | Common::KEYCODE_l,
	KEY_ALT_M = (Common::KBD_ALT << 16) | Common::KEYCODE_m,
	KEY_ALT_N = (Common::KBD_ALT << 16) | Common::KEYCODE_n,
	KEY_ALT_O = (Common::KBD_ALT << 16) | Common::KEYCODE_o,
	KEY_ALT_P = (Common::KBD_ALT << 16) | Common::KEYCODE_p,
	KEY_ALT_Q = (Common::KBD_ALT << 16) | Common::KEYCODE_q,
	KEY_ALT_R = (Common::KBD_ALT << 16) | Common::KEYCODE_r,
	KEY_ALT_S = (Common::KBD_ALT << 16) | Common::KEYCODE_s,
	KEY_ALT_T = (Common::KBD_ALT << 16) | Common::KEYCODE_t,
	KEY_ALT_U = (Common::KBD_ALT << 16) | Common::KEYCODE_u,
	KEY_ALT_V = (Common::KBD_ALT << 16) | Common::KEYCODE_v,
	KEY_ALT_W = (Common::KBD_ALT << 16) | Common::KEYCODE_w,
	KEY_ALT_X = (Common::KBD_ALT << 16) | Common::KEYCODE_x,
	KEY_ALT_Y = (Common::KBD_ALT << 16) | Common::KEYCODE_y,
	KEY_ALT_Z = (Common::KBD_ALT << 16) | Common::KEYCODE_z,

	KEY_ALT_0 = (Common::KBD_ALT << 16) | Common::KEYCODE_0,
	KEY_ALT_1 = (Common::KBD_ALT << 16) | Common::KEYCODE_1,
	KEY_ALT_2 = (Common::KBD_ALT << 16) | Common::KEYCODE_2,
	KEY_ALT_3 = (Common::KBD_ALT << 16) | Common::KEYCODE_3,
	KEY_ALT_4 = (Common::KBD_ALT << 16) | Common::KEYCODE_4,
	KEY_ALT_5 = (Common::KBD_ALT << 16) | Common::KEYCODE_5,
	KEY_ALT_6 = (Common::KBD_ALT << 16) | Common::KEYCODE_6,
	KEY_ALT_7 = (Common::KBD_ALT << 16) | Common::KEYCODE_7,
	KEY_ALT_8 = (Common::KBD_ALT << 16) | Common::KEYCODE_8,
	KEY_ALT_9 = (Common::KBD_ALT << 16) | Common::KEYCODE_9,

	KEY_PAD_MINUS = Common::KEYCODE_KP_MINUS,
	KEY_PAD_PLUS = Common::KEYCODE_KP_PLUS,
	KEY_PAD_TIMES = Common::KEYCODE_KP_MULTIPLY,
	KEY_PAD_DIVIDE = Common::KEYCODE_KP_DIVIDE,
};

enum {
	KEY_PAGE_UP = Common::KEYCODE_PAGEUP,
	KEY_PAGE_DOWN = Common::KEYCODE_PAGEDOWN,
	KEY_HOME = Common::KEYCODE_HOME,
	KEY_END = Common::KEYCODE_END,
	KEY_DELETE = Common::KEYCODE_DELETE,
	KEY_BACKSP = Common::KEYCODE_BACKSPACE,
	KEY_TAB = Common::KEYCODE_TAB,
	KEY_SHFTTAB = (Common::KBD_SHIFT << 16) | Common::KEYCODE_TAB,
	KEY_RETURN = Common::KEYCODE_RETURN,
	KEY_ESCAPE = Common::KEYCODE_ESCAPE,
	KEY_SPACE = Common::KEYCODE_SPACE,
	KEY_CLEAR = Common::KEYCODE_CLEAR,

	KEY_LEFT = Common::KEYCODE_LEFT,
	KEY_RIGHT = Common::KEYCODE_RIGHT,
	KEY_UP = Common::KEYCODE_UP,
	KEY_DOWN = Common::KEYCODE_DOWN,

	KEY_ALT_UP = (Common::KBD_ALT << 16) | Common::KEYCODE_UP,
	KEY_ALT_DOWN = (Common::KBD_ALT << 16) | Common::KEYCODE_DOWN,
	KEY_ALT_LEFT = (Common::KBD_ALT << 16) | Common::KEYCODE_LEFT,
	KEY_ALT_RIGHT = (Common::KBD_ALT << 16) | Common::KEYCODE_RIGHT,

	KEY_CTRL_UP = (Common::KBD_CTRL << 16) | Common::KEYCODE_UP,
	KEY_CTRL_DOWN = (Common::KBD_CTRL << 16) | Common::KEYCODE_DOWN,
	KEY_CTRL_LEFT = (Common::KBD_CTRL << 16) | Common::KEYCODE_LEFT,
	KEY_CTRL_RIGHT = (Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT,

	KEY_F1 = Common::KEYCODE_F1,
	KEY_F2 = Common::KEYCODE_F2,
	KEY_F3 = Common::KEYCODE_F3,
	KEY_F4 = Common::KEYCODE_F4,

	KEY_F5 = Common::KEYCODE_F5,
	KEY_F6 = Common::KEYCODE_F6,
	KEY_F7 = Common::KEYCODE_F7,
	KEY_F8 = Common::KEYCODE_F8,
	KEY_F9 = Common::KEYCODE_F9,
	KEY_F10 = Common::KEYCODE_F10,
	KEY_F11 = Common::KEYCODE_F11,
	KEY_F12 = Common::KEYCODE_F12
};

} // namespace M4

#endif
