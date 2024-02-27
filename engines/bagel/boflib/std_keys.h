
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

#ifndef BAGEL_BOFLIB_STD_KEYS_H
#define BAGEL_BOFLIB_STD_KEYS_H

#include "bagel/boflib/stdinc.h"

namespace Bagel {

// Boffo Key defs
//
#define BKEY_UNKNOWN    0x00000000

#define BKF_ALT     0x10000000
#define BKF_CMD     0x10000000      // Mac's Flower/Clover/Command key

#define BKF_CTRL    0x20000000
#define BKF_SHIFT   0x40000000

#define BKEY_0      '0'
#define BKEY_1      '1'
#define BKEY_2      '2'
#define BKEY_3      '3'
#define BKEY_4      '4'
#define BKEY_5      '5'
#define BKEY_6      '6'
#define BKEY_7      '7'
#define BKEY_8      '8'
#define BKEY_9      '9'

#define BKEY_a  (ULONG)'a'
#define BKEY_b  (ULONG)'b'
#define BKEY_c  (ULONG)'c'
#define BKEY_d  (ULONG)'d'
#define BKEY_e  (ULONG)'e'
#define BKEY_f  (ULONG)'f'
#define BKEY_g  (ULONG)'g'
#define BKEY_h  (ULONG)'h'
#define BKEY_i  (ULONG)'i'
#define BKEY_j  (ULONG)'j'
#define BKEY_k  (ULONG)'k'
#define BKEY_l  (ULONG)'l'
#define BKEY_m  (ULONG)'m'
#define BKEY_n  (ULONG)'n'
#define BKEY_o  (ULONG)'o'
#define BKEY_p  (ULONG)'p'
#define BKEY_q  (ULONG)'q'
#define BKEY_r  (ULONG)'r'
#define BKEY_s  (ULONG)'s'
#define BKEY_t  (ULONG)'t'
#define BKEY_u  (ULONG)'u'
#define BKEY_v  (ULONG)'v'
#define BKEY_w  (ULONG)'w'
#define BKEY_x  (ULONG)'x'
#define BKEY_y  (ULONG)'y'
#define BKEY_z  (ULONG)'z'

#define BKEY_A  (ULONG)'A'
#define BKEY_B  (ULONG)'B'
#define BKEY_C  (ULONG)'C'
#define BKEY_D  (ULONG)'D'
#define BKEY_E  (ULONG)'E'
#define BKEY_F  (ULONG)'F'
#define BKEY_G  (ULONG)'G'
#define BKEY_H  (ULONG)'H'
#define BKEY_I  (ULONG)'I'
#define BKEY_J  (ULONG)'J'
#define BKEY_K  (ULONG)'K'
#define BKEY_L  (ULONG)'L'
#define BKEY_M  (ULONG)'M'
#define BKEY_N  (ULONG)'N'
#define BKEY_O  (ULONG)'O'
#define BKEY_P  (ULONG)'P'
#define BKEY_Q  (ULONG)'Q'
#define BKEY_R  (ULONG)'R'
#define BKEY_S  (ULONG)'S'
#define BKEY_T  (ULONG)'T'
#define BKEY_U  (ULONG)'U'
#define BKEY_V  (ULONG)'V'
#define BKEY_W  (ULONG)'W'
#define BKEY_X  (ULONG)'X'
#define BKEY_Y  (ULONG)'Y'
#define BKEY_Z  (ULONG)'Z'

#define BKEY_PLUS   (ULONG)'+'
#define BKEY_MINUS  (ULONG)'-'
#define BKEY_PERIOD (ULONG)'.'

#define BKEY_ALT_a  (BKEY_a | BKF_ALT)
#define BKEY_ALT_b  (BKEY_b | BKF_ALT)
#define BKEY_ALT_c  (BKEY_c | BKF_ALT)
#define BKEY_ALT_d  (BKEY_d | BKF_ALT)
#define BKEY_ALT_e  (BKEY_e | BKF_ALT)
#define BKEY_ALT_f  (BKEY_f | BKF_ALT)
#define BKEY_ALT_g  (BKEY_g | BKF_ALT)
#define BKEY_ALT_h  (BKEY_h | BKF_ALT)
#define BKEY_ALT_i  (BKEY_i | BKF_ALT)
#define BKEY_ALT_j  (BKEY_j | BKF_ALT)
#define BKEY_ALT_k  (BKEY_k | BKF_ALT)
#define BKEY_ALT_l  (BKEY_l | BKF_ALT)
#define BKEY_ALT_m  (BKEY_m | BKF_ALT)
#define BKEY_ALT_n  (BKEY_n | BKF_ALT)
#define BKEY_ALT_o  (BKEY_o | BKF_ALT)
#define BKEY_ALT_p  (BKEY_p | BKF_ALT)
#define BKEY_ALT_q  (BKEY_q | BKF_ALT)
#define BKEY_ALT_r  (BKEY_r | BKF_ALT)
#define BKEY_ALT_s  (BKEY_s | BKF_ALT)
#define BKEY_ALT_t  (BKEY_t | BKF_ALT)
#define BKEY_ALT_u  (BKEY_u | BKF_ALT)
#define BKEY_ALT_v  (BKEY_v | BKF_ALT)
#define BKEY_ALT_w  (BKEY_w | BKF_ALT)
#define BKEY_ALT_x  (BKEY_x | BKF_ALT)
#define BKEY_ALT_y  (BKEY_y | BKF_ALT)
#define BKEY_ALT_z  (BKEY_z | BKF_ALT)

#define BKEY_ALT_A  (BKEY_A | BKF_ALT)
#define BKEY_ALT_B  (BKEY_B | BKF_ALT)
#define BKEY_ALT_C  (BKEY_C | BKF_ALT)
#define BKEY_ALT_D  (BKEY_D | BKF_ALT)
#define BKEY_ALT_E  (BKEY_E | BKF_ALT)
#define BKEY_ALT_F  (BKEY_F | BKF_ALT)
#define BKEY_ALT_G  (BKEY_G | BKF_ALT)
#define BKEY_ALT_H  (BKEY_H | BKF_ALT)
#define BKEY_ALT_I  (BKEY_I | BKF_ALT)
#define BKEY_ALT_J  (BKEY_J | BKF_ALT)
#define BKEY_ALT_K  (BKEY_K | BKF_ALT)
#define BKEY_ALT_L  (BKEY_L | BKF_ALT)
#define BKEY_ALT_M  (BKEY_M | BKF_ALT)
#define BKEY_ALT_N  (BKEY_N | BKF_ALT)
#define BKEY_ALT_O  (BKEY_O | BKF_ALT)
#define BKEY_ALT_P  (BKEY_P | BKF_ALT)
#define BKEY_ALT_Q  (BKEY_Q | BKF_ALT)
#define BKEY_ALT_R  (BKEY_R | BKF_ALT)
#define BKEY_ALT_S  (BKEY_S | BKF_ALT)
#define BKEY_ALT_T  (BKEY_T | BKF_ALT)
#define BKEY_ALT_U  (BKEY_U | BKF_ALT)
#define BKEY_ALT_V  (BKEY_V | BKF_ALT)
#define BKEY_ALT_W  (BKEY_W | BKF_ALT)
#define BKEY_ALT_X  (BKEY_X | BKF_ALT)
#define BKEY_ALT_Y  (BKEY_Y | BKF_ALT)
#define BKEY_ALT_Z  (BKEY_Z | BKF_ALT)

#define BKEY_BASE       0x00000100
#define BKEY_F1         (BKEY_BASE + 1)
#define BKEY_F2         (BKEY_BASE + 2)
#define BKEY_F3         (BKEY_BASE + 3)
#define BKEY_F4         (BKEY_BASE + 4)
#define BKEY_F5         (BKEY_BASE + 5)
#define BKEY_F6         (BKEY_BASE + 6)
#define BKEY_F7         (BKEY_BASE + 7)
#define BKEY_F8         (BKEY_BASE + 8)
#define BKEY_F9         (BKEY_BASE + 9)
#define BKEY_F10        (BKEY_BASE + 10)
#define BKEY_F11        (BKEY_BASE + 11)
#define BKEY_F12        (BKEY_BASE + 12)

#define BKEY_ALT_F1     (BKEY_F1 | BKF_ALT)
#define BKEY_ALT_F2     (BKEY_F2 | BKF_ALT)
#define BKEY_ALT_F3     (BKEY_F3 | BKF_ALT)
#define BKEY_ALT_F4     (BKEY_F4 | BKF_ALT)
#define BKEY_ALT_F5     (BKEY_F5 | BKF_ALT)
#define BKEY_ALT_F6     (BKEY_F6 | BKF_ALT)
#define BKEY_ALT_F7     (BKEY_F7 | BKF_ALT)
#define BKEY_ALT_F8     (BKEY_F8 | BKF_ALT)
#define BKEY_ALT_F9     (BKEY_F9 | BKF_ALT)
#define BKEY_ALT_F10    (BKEY_F10 | BKF_ALT)
#define BKEY_ALT_F11    (BKEY_F11 | BKF_ALT)
#define BKEY_ALT_F12    (BKEY_F12 | BKF_ALT)

#define BKEY_ESC        (ULONG)0x1B
#define BKEY_BACK       (ULONG)0x08
#define BKEY_ENTER      (ULONG)0x0D
#define BKEY_SPACE      (ULONG)' '
#define BKEY_PERIOD     (ULONG)'.'

#define BKEY_END        (BKEY_BASE + 14)
#define BKEY_HOME       (BKEY_BASE + 15)
#define BKEY_LEFT       (BKEY_BASE + 16)
#define BKEY_RIGHT      (BKEY_BASE + 17)
#define BKEY_UP         (BKEY_BASE + 18)
#define BKEY_DOWN       (BKEY_BASE + 19)
#define BKEY_INS        (BKEY_BASE + 20)
#define BKEY_DEL        (BKEY_BASE + 21)

#define BKEY_ALT_UP     (BKEY_UP | BKF_ALT)
#define BKEY_ALT_DOWN   (BKEY_DOWN | BKF_ALT)

#define BKEY_PAGEUP     (BKEY_BASE + 22)
#define BKEY_PAGEDOWN   (BKEY_BASE + 23)

#define BKEY_SCRL_LOCK  (BKEY_BASE + 24)

#define BKEY_SHIFT  BKF_SHIFT
#define BKEY_CTRL   BKF_CTRL
#define BKEY_ALT    BKF_ALT

} // namespace Bagel

#endif
