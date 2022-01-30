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

#ifndef CHEWY_NGSDEFS_H
#define CHEWY_NGSDEFS_H

#include "common/events.h"

namespace Chewy {

#define MAXSPRITE 350
#define NOSPEICHER 0
#define MAXFONTBREITE 32
#define MAXFONTHOEHE 32

/*
#define master_clear outportb(0x00d,0)
#define flipflop_clear outportb(0x00c,0)
#define all_mask_set outportb(0x00f,0x0ff)
#define all_mask_clear outportb(0x00f,0)
#define read_status inportb(0x008)
#define set_kommando(komm) outportb(0x008,komm);
*/

#define CHECK 0x00
#define WRITE 0x04
#define READ 0x08
#define AUTOINIT_EIN 0x10
#define AUTOINIT_AUS 0x00
#define ADRESSDEC 0x20
#define ADRESSINC 0x00
#define REQUESTMODE 0x00
#define SINGLEMODE 0x40
#define BLOCKMODE 0x80
#define KASKAMODE 0xc0

#define MCGA_SPEICHER 64000L
#define SETZEN 0
#define UND 1
#define ODER 2
#define XODER 3

#define COPY_PAGE out->screen2screen

#define PAGE_0_1 0,0,316,199,0,0,0,80
#define PAGE_0_2 0,0,316,199,0,0,0,160*200
#define PAGE_0_3 0,0,316,199,0,0,0,160*200+80
#define PAGE_1_0 0,0,316,199,0,0,80,0
#define PAGE_1_2 0,0,316,199,0,0,80,160*200
#define PAGE_1_3 0,0,316,199,0,0,80,160*200+80
#define PAGE_2_0 0,0,316,199,0,0,160*200,0
#define PAGE_2_1 0,0,316,199,0,0,160*200,80
#define PAGE_2_3 0,0,316,199,0,0,160*200,160*200+80
#define PAGE_3_0 0,0,316,199,0,0,160*200+80,0
#define PAGE_3_1 0,0,316,199,0,0,160*200+80,80
#define PAGE_3_2 0,0,316,199,0,0,160*200+80,160*200
#define PAGE_R_L 0,0,316,399,0,0,80,0
#define PAGE_L_R 0,0,316,399,0,0,0,80
#define PAGE_D_U 0,0,635,199,0,0,160*200,0
#define PAGE_U_D 0,0,635,199,0,0,0,160*200

#define PAGEWA_0_1 0,0,316,199,0,0,0,80
#define PAGEWA_0_2 0,0,316,199,0,0,0,160
#define PAGEWA_0_3 0,0,316,199,0,0,0,240
#define PAGEWA_1_0 0,0,316,199,0,0,80,0
#define PAGEWA_1_2 0,0,316,199,0,0,80,160
#define PAGEWA_1_3 0,0,316,199,0,0,80,240
#define PAGEWA_2_0 0,0,316,199,0,0,160,0
#define PAGEWA_2_1 0,0,316,199,0,0,160,80
#define PAGEWA_2_3 0,0,316,199,0,0,160,240
#define PAGEWA_3_0 0,0,316,199,0,0,240,0
#define PAGEWA_3_1 0,0,316,199,0,0,240,80
#define PAGEWA_3_2 0,0,316,199,0,0,240,160

#define PAGESE_0_1 0,0,316,199,0,0,0,80*200
#define PAGESE_0_2 0,0,316,199,0,0,0,80*400
#define PAGESE_0_3 0,0,316,199,0,0,0,80*600
#define PAGESE_1_0 0,0,316,199,0,0,80*200,0
#define PAGESE_1_2 0,0,316,199,0,0,80*200,80*400
#define PAGESE_1_3 0,0,316,199,0,0,80*200,80*600
#define PAGESE_2_0 0,0,316,199,0,0,80*400,0
#define PAGESE_2_1 0,0,316,199,0,0,80*400,80*200
#define PAGESE_2_3 0,0,316,199,0,0,80*400,80*600
#define PAGESE_3_0 0,0,316,199,0,0,80*600,0
#define PAGESE_3_1 0,0,316,199,0,0,80*600,80*200
#define PAGESE_3_2 0,0,316,199,0,0,80*600,80*400

#define MODE640X400X256 0x100
#define MODE640X480X256 0x101
#define MODE800X600X16 0x102
#define MODE800X600X256 0x103
#define MODE1024X768X16 0x104
#define MODE1024X768X256 0x105
#define MODE1280X1024X16 0x106
#define MODE1280X1024X256 0x107

#define END_POOL 32767

#define MAXDIRS 50
#define MAXFNAMEN 50
#define GRAFIK 1
#define SPEICHER 2
#define DATEI 3
#define MOD15 10
#define TYPE_ANZ 27
#define PCXDATEI 0
#define TBFDATEI 1
#define TAFDATEI 2
#define TFFDATEI 3
#define VOCDATEI 4
#define TPFDATEI 5
#define TMFDATEI 6
#define MODDATEI 7
#define RAWDATEI 8
#define LBMDATEI 9
#define RDIDATEI 10
#define TXTDATEI 11
#define IIBDATEI 12
#define SIBDATEI 13
#define EIBDATEI 14
#define ATSDATEI 15
#define SAADATEI 16
#define FLCDATEI 17
#define AADDATEI 18
#define ADSDATEI 19
#define ADHDATEI 20
#define TGPDATEI 21
#define TVPDATEI 22
#define TTPDATEI 23
#define TAPDATEI 24
#define CFODATEI 25
#define TCFDATEI 26
#define NODATEI 255
#define SCREENSAVE 0
#define SPRITESAVE 1

#define MAUS 4
#define NICHT_BEWEGT 0
#define LINKS 1
#define RECHTS 2
#define HOCH 3
#define RUNTER 4

#define ALT 0x1000
#define SHIFT_LEFT Common::KEYCODE_LSHIFT
#define SHIFT_RIGHT Common::KEYCODE_RSHIFT
#define CURSOR_LEFT Common::KEYCODE_LEFT
#define CURSOR_RIGHT Common::KEYCODE_RIGHT
#define CURSOR_UP Common::KEYCODE_UP
#define CURSOR_DOWN Common::KEYCODE_DOWN
#define PAGE_UP Common::KEYCODE_PAGEUP
#define PAGE_DOWN Common::KEYCODE_PAGEDOWN
#define BS_KEY Common::KEYCODE_BACKSPACE
#define PAUSE_KEY Common::KEYCODE_PAUSE
#define PLUS Common::KEYCODE_PLUS
#define MINUS Common::KEYCODE_MINUS
#define TAB Common::KEYCODE_TAB
#define ESC Common::KEYCODE_ESCAPE
#define ENTER Common::KEYCODE_RETURN
#define SPACE Common::KEYCODE_SPACE
#define A_KEY Common::KEYCODE_a
#define B_KEY Common::KEYCODE_b
#define C_KEY Common::KEYCODE_c
#define D_KEY Common::KEYCODE_d
#define E_KEY Common::KEYCODE_e
#define F_KEY Common::KEYCODE_f
#define G_KEY Common::KEYCODE_g
#define H_KEY Common::KEYCODE_h
#define I_KEY Common::KEYCODE_i
#define J_KEY Common::KEYCODE_j
#define K_KEY Common::KEYCODE_k
#define L_KEY Common::KEYCODE_l
#define M_KEY Common::KEYCODE_m
#define N_KEY Common::KEYCODE_n
#define O_KEY Common::KEYCODE_o
#define P_KEY Common::KEYCODE_p
#define Q_KEY Common::KEYCODE_q
#define R_KEY Common::KEYCODE_r
#define S_KEY Common::KEYCODE_s
#define T_KEY Common::KEYCODE_t
#define U_KEY Common::KEYCODE_u
#define V_KEY Common::KEYCODE_v
#define W_KEY Common::KEYCODE_w
#define X_KEY Common::KEYCODE_x
#define Y_KEY Common::KEYCODE_y
#define Z_KEY Common::KEYCODE_z
#define F1_KEY Common::KEYCODE_F1
#define F2_KEY Common::KEYCODE_F2
#define F3_KEY Common::KEYCODE_F3
#define F4_KEY Common::KEYCODE_F4
#define F5_KEY Common::KEYCODE_F5
#define F6_KEY Common::KEYCODE_F6
#define F7_KEY Common::KEYCODE_F7
#define F8_KEY Common::KEYCODE_F8
#define F9_KEY Common::KEYCODE_F9
#define F10_KEY Common::KEYCODE_F10
#define F11_KEY Common::KEYCODE_F11
#define F12_KEY Common::KEYCODE_F12
#define KEY_1 Common::KEYCODE_1
#define KEY_2 Common::KEYCODE_2
#define KEY_3 Common::KEYCODE_3
#define KEY_4 Common::KEYCODE_4
#define KEY_5 Common::KEYCODE_5
#define KEY_6 Common::KEYCODE_6
#define KEY_7 Common::KEYCODE_7
#define KEY_8 Common::KEYCODE_8
#define KEY_9 Common::KEYCODE_9
#define KEY_0 Common::KEYCODE_0

#define MAUS_LINKS 255
#define MAUS_RECHTS 1
#define MAUS_MITTE 254

#define SOUND 5
#define BASE 0x220
#define JA 1
#define NEIN 0
#define ON 1
#define OFF 0
#define KEINE 0
#define SPEAKER 1
#define ADLIB 2
#define SOUNDBLASTER 3
#define SB_PRO 4
#define SB_16_ASP 5

#define NONE 0
#define MDA 1
#define CGA 2
#define EGAMono 3
#define EGAColor 4
#define VGAMono 5
#define VGAColor 6
#define MCGAMono 7
#define MCGAColor 8
#define I86 0
#define I186 1
#define I286 2
#define I386sx 3
#define I386dx 4
#define I486 5

} // namespace Chewy

#endif
