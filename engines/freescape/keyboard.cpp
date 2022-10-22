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

#include "common/events.h"

#include "freescape/freescape.h"

#define AK_A 0x20
#define AK_B 0x35
#define AK_C 0x33
#define AK_D 0x22
#define AK_E 0x12
#define AK_F 0x23
#define AK_G 0x24
#define AK_H 0x25
#define AK_I 0x17
#define AK_J 0x26
#define AK_K 0x27
#define AK_L 0x28
#define AK_M 0x37
#define AK_N 0x36
#define AK_O 0x18
#define AK_P 0x19
#define AK_Q 0x10
#define AK_R 0x13
#define AK_S 0x21
#define AK_T 0x14
#define AK_U 0x16
#define AK_V 0x34
#define AK_W 0x11
#define AK_X 0x32
#define AK_Y 0x15
#define AK_Z 0x31

#define AK_0 0x0A
#define AK_1 0x01
#define AK_2 0x02
#define AK_3 0x03
#define AK_4 0x04
#define AK_5 0x05
#define AK_6 0x06
#define AK_7 0x07
#define AK_8 0x08
#define AK_9 0x09

#define AK_NP0 0x0F
#define AK_NP1 0x1D
#define AK_NP2 0x1E
#define AK_NP3 0x1F
#define AK_NP4 0x2D
#define AK_NP5 0x2E
#define AK_NP6 0x2F
#define AK_NP7 0x3D
#define AK_NP8 0x3E
#define AK_NP9 0x3F

#define AK_NPDIV 0x5C
#define AK_NPMUL 0x5D
#define AK_NPSUB 0x4A
#define AK_NPADD 0x5E
#define AK_NPDEL 0x3C
#define AK_NPLPAREN 0x5A
#define AK_NPRPAREN 0x5B

#define AK_F1 0x50
#define AK_F2 0x51
#define AK_F3 0x52
#define AK_F4 0x53
#define AK_F5 0x54
#define AK_F6 0x55
#define AK_F7 0x56
#define AK_F8 0x57
#define AK_F9 0x58
#define AK_F10 0x59

#define AK_UP 0x4C
#define AK_DN 0x4D
#define AK_LF 0x4F
#define AK_RT 0x4E

#define AK_SPC 0x40
#define AK_BS 0x41
#define AK_TAB 0x42
#define AK_ENT 0x43
#define AK_RET 0x44
#define AK_ESC 0x45
#define AK_DEL 0x46

#define AK_LSH 0x60
#define AK_RSH 0x61
#define AK_CAPSLOCK 0x62
#define AK_CTRL 0x63
#define AK_LALT 0x64
#define AK_RALT 0x65
#define AK_LAMI 0x66
#define AK_RAMI 0x67
#define AK_HELP 0x5F

namespace Freescape {

int FreescapeEngine::decodeAmigaAtariKey(int code) {
	switch (code) {
		case (AK_UP):
		return Common::KEYCODE_UP;

		case (AK_DN):
		return Common::KEYCODE_DOWN;

		case (AK_LF):
		return Common::KEYCODE_LEFT;

		case (AK_RT):
		return Common::KEYCODE_RIGHT;

		default:
		return code;
	}
}

int FreescapeEngine::decodeDOSKey(int index) {
	switch (index) {
		case 1:
			return 	Common::KEYCODE_r;
		case 2:
			return Common::KEYCODE_f;
		case 3:
			return Common::KEYCODE_UP;
		case 4:
			return Common::KEYCODE_DOWN;
		case 5:
			return Common::KEYCODE_q;
		case 6:
			return Common::KEYCODE_w;
		case 7:
			return Common::KEYCODE_p;
		case 8:
			return Common::KEYCODE_l;
		case 30:
			return Common::KEYCODE_SPACE;
		case 40:
			return Common::KEYCODE_d;
		default:
			error("Invalid key index: %x", index);
	}
	return 0;
}

} // End of namespace Freescape
