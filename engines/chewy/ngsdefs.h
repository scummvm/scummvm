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

#ifndef CHEWY_NGSDEFS_H
#define CHEWY_NGSDEFS_H

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
#define OPENFEHLER 0
#define READFEHLER 1
#define WRITEFEHLER 2
#define CLOSEFEHLER 3
#define ZEIGERFEHLER 3
#define NOTPCX 7
#define PALETTEFEHLER 6
#define NOTTBF 7
#define NOTSBI 8
#define NOTVOC 9
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

#define CURSOR_LEFT 75
#define CURSOR_RIGHT 77
#define CURSOR_UP 72
#define CURSOR_DOWN 80
#define PAGE_UP 73
#define PAGE_DOWN 81
#define POS1_KEY 71
#define ENDE_KEY 79
#define EINFG_KEY 82
#define ENTF_KEY 83
#define BS_KEY 14
#define PAUSE_KEY 69
#define PLUS 78
#define MINUS 74
#define ALT 100
#define STRG 29
#define TAB 15
#define CAPS_LOCK 58
#define NUM_LOCK 69
#define SCROLL_LOCK 70
#define PRINT_SCREEN 55
#define SHIFT_LEFT 42
#define SHIFT_RIGHT 54
#define ESC 1
#define ENTER 28
#define SPACE 57
#define A_KEY 30
#define B_KEY 48
#define C_KEY 46
#define D_KEY 32
#define E_KEY 18
#define F_KEY 33
#define G_KEY 34
#define H_KEY 35
#define I_KEY 23
#define J_KEY 36
#define K_KEY 37
#define L_KEY 38
#define M_KEY 50
#define N_KEY 49
#define O_KEY 24
#define P_KEY 25
#define Q_KEY 16
#define R_KEY 19
#define S_KEY 31
#define T_KEY 20
#define U_KEY 22
#define V_KEY 47
#define W_KEY 17
#define X_KEY 45
#define Y_KEY 44
#define Z_KEY 21
#define F1_KEY 59
#define F2_KEY 60
#define F3_KEY 61
#define F4_KEY 62
#define F5_KEY 63
#define F6_KEY 64
#define F7_KEY 65
#define F8_KEY 66
#define F9_KEY 67
#define F10_KEY 68
#define F11_KEY 87
#define F12_KEY 88
#define KEY_1 2
#define KEY_2 3
#define KEY_3 4
#define KEY_4 5
#define KEY_5 6
#define KEY_6 7
#define KEY_7 8
#define KEY_8 9
#define KEY_9 10
#define KEY_0 11

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
