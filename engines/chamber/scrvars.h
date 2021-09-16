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

#ifndef CHAMBER_SCRVARS_H
#define CHAMBER_SCRVARS_H

namespace Chamber {

script_byte_vars_t script_byte_vars = {
	135,    /*  0 */
	0,    /*  1 */
	0,    /*  2 */
	0,    /*  3 */
	0,    /*  4 */
	0,    /*  5 */
	1,    /*  6 */
	0,    /*  7 */
	0,    /*  8 */
	0,    /*  9 */
	0,    /*  A */
	0,    /*  B */
	0,    /*  C */
	0,    /*  D */
	0,    /*  E */
	0,    /*  F */

	0,    /* 10 */
	0,    /* 11 */
	0,    /* 12 */
	0,    /* 13 */
	0,    /* 14 */
	0,    /* 15 */
	0,    /* 16 */
	0,    /* 17 */
	0,    /* 18 */
	0,    /* 19 */
	0,    /* 1A */
	0,    /* 1B */
	0,    /* 1C */
	0,    /* 1D */
	0,    /* 1E */
	0,    /* 1F */

	0,    /* 20 */
	0,    /* 21 */
	0,    /* 22 */
	0,    /* 23 */
	0xB,  /* 24 */
	0,    /* 25 */
	0,    /* 26 */
	0,    /* 27 */
	0,    /* 28 */
	0,    /* 29 */
	0,    /* 2A */
	0,    /* 2B */
	0,    /* 2C */
	0,    /* 2D */
	0,    /* 2E */
	0,    /* 2F */

	0,    /* 30 */
	0,    /* 31 */
	0,    /* 32 */
	0,    /* 33 */
	0,    /* 34 */
	0,    /* 35 */
	0,    /* 36 */
	0,    /* 37 */
	0,    /* 38 */
	0,    /* 39 */
	0,    /* 3A */
	0,    /* 3B */
	0,    /* 3C */
	0,    /* 3D */
	0,    /* 3E */
	0,    /* 3F */

	0,    /* 40 */
	0,    /* 41 */
	0,    /* 42 */
	0,    /* 43 */
	0,    /* 44 */
	0,    /* 45 */
	0,    /* 46 */
	1,    /* 47 */
	0,    /* 48 */
	0xFF, /* 49 */
	0,    /* 4A */
	0,    /* 4B */
	0,    /* 4C */
	0,    /* 4D */
	0,    /* 4E */
	0,    /* 4F */

	0,    /* 50 */
	0,    /* 51 */
	0,    /* 52 */
	0,    /* 53 */
	2,    /* 54 */
	1,    /* 55 */
	0,    /* 56 */
	0,    /* 57 */
	0,    /* 58 */
	0,    /* 59 */
	63,   /* 5A */
	0,    /* 5B */
	0,    /* 5C */
	0,    /* 5D */
	0,    /* 5E */
	0,    /* 5F */

	0,    /* 60 */
	0,    /* 61 */
	1,    /* 62 */
	0,    /* 63 */
	0,    /* 64 */
	0,    /* 65 */
	0,    /* 66 */
	0,    /* 67 */
	0,    /* 68 */
	0,    /* 69 */
	0,    /* 6A */
	0,    /* 6B */
	0,    /* 6C */
	/*{0,0,0,0}*/  /* 6D..70 */
};

script_word_vars_t script_word_vars = {
	{BE(0x9007), BE(0x9007), BE(0x9007), BE(0x9007), BE(0x9007), BE(0x9007)}, /* 00 .. 0B */
	BE(0),           /* 0C */
	BE(0),           /* 0E */
	BE(0),           /* 10 */
	{0},            /* 12 */
	BE(0),           /* A8 */
	BE(0),           /* AA */
	BE(0),           /* AC */
	BE(0),           /* AE */
	BE(0),           /* B0 */
	BE(0),           /* B2 */
	BE(0),           /* B4 */
	BE(0),           /* B6 */
	BE(0)            /* B8 */
};

item_t inventory_items[MAX_INV_ITEMS] = {
	{0x00,  0,  26, 109, 0xC0D9}, /*  0 : SKULL */
	{0x00,  0,  26, 109, 0xC0D9}, /*  1 : SKULL */
	{0x00,  0,  26, 109, 0xC0D9}, /*  2 : SKULL */
	{0x18,  0,  26, 109, 0xC0D9}, /*  3 : SKULL */
	{0x00,  0,  26, 109, 0xC0D9}, /*  4 : SKULL */
	{0x00,  0,  32, 104, 0xC0ED}, /*  5 : ROPE */
	{0x00,  0,  32, 104, 0xC0ED}, /*  6 : ROPE */
	{0x00,  0,  32, 104, 0xC0ED}, /*  7 : ROPE */
	{0x10,  0,  32, 104, 0xC0ED}, /*  8 : ROPE */
	{0x00,  0,  32, 104, 0xC0ED}, /*  9 : ROPE */
	{0x00,  0,  33, 106, 0xC0EE}, /* 10 : STONE FLY */
	{0x00,  0,  33, 106, 0xC0EE}, /* 11 : STONE FLY */
	{0x00,  0,  33, 106, 0xC0EE}, /* 12 : STONE FLY */
	{0x10,  0,  33, 106, 0xC0EE}, /* 13 : STONE FLY */
	{0x00,  0,  36, 107, 0xC0EC}, /* 14 : GOBLET */
	{0x00,  0,  36, 107, 0xC0EC}, /* 15 : GOBLET */
	{0x00,  0,  36, 107, 0xC0EC}, /* 16 : GOBLET */
	{0x10,  0,  36, 107, 0xC0EC}, /* 17 : GOBLET */
	{0x00,  0,  37, 108, 0xC0D3}, /* 18 : DAGGER */
	{0x00,  0,  37, 108, 0xC0D3}, /* 19 : DAGGER */
	{0x00,  0,  37, 108, 0xC0D3}, /* 20 : DAGGER */
	{0x10,  0,  37, 108, 0xC0D3}, /* 21 : DAGGER */
	{0x00,  0,  39, 105, 0xC0D6}, /* 22 : LANTERN */
	{0x00,  0,  39, 105, 0xC0D6}, /* 23 : LANTERN */
	{0x00,  0,  39, 105, 0xC0D6}, /* 24 : LANTERN */
	{0x10,  0,  39, 105, 0xC0D6}, /* 25 : LANTERN */
	{0x01,  0,  30, 136, 0xC126}, /* 26 : BLUE SPIDER */
	{0x01,  0,  30, 136, 0xC126}, /* 27 : BLUE SPIDER */
	{0x01,  0,  30, 136, 0xC126}, /* 28 : BLUE SPIDER */
	{0x01,  0,  30, 136, 0xC126}, /* 29 : BLUE SPIDER */
	{0x01,  0,  31, 135, 0xC126}, /* 30 : RED SPIDER */
	{0x01,  0,  31, 135, 0xC126}, /* 31 : RED SPIDER */
	{0x01,  0,  31, 135, 0xC126}, /* 32 : RED SPIDER */
	{0x01,  0,  31, 135, 0xC126}, /* 33 : RED SPIDER */
	{0x01, 38,  38, 120, 0xC132}, /* 34 : DIE */
	{0x01, 38,  38, 120, 0xC132}, /* 35 : DIE */
	{0x01, 38,  38, 120, 0xC132}, /* 36 : DIE */
	{0x01, 38,  38, 120, 0xC132}, /* 37 : DIE */
	{0x00,  0,  28, 132, 0xC0EF}, /* 38 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 39 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 40 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 41 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 42 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 43 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 44 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 45 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 46 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 47 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 48 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 49 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 50 : ZAPSTIK */
	{0x00,  0,  28, 132, 0xC0EF}, /* 51 : ZAPSTIK */
	{0x00,  0, 106, 142, 0xC2C0}, /* 52 : MASK */
	{0x00,  0,  34, 111, 0xC315}, /* 53 : WHISTLE */
	{0x00,  0,  35, 110, 0xC27E}, /* 54 : EGG */
	{0x00,  0,  40, 115, 0xC0D3}, /* 55 : SACRIFICIAL BLADE */
	{0x40, 65,  58, 117, 0xC127}, /* 56 : CHOPPER */
	{0x19,  0,  59, 116, 0xC2BD}, /* 57 : STATUE OF ZORQ */
	{0x00,  0, 105, 167, 0xC2BD}, /* 58 : STATUE OF SAURA */
	{0x00,  0, 107, 118, 0xC245}, /* 59 : FLASK */
	{0x00,  0, 108, 119, 0xC244}, /* 60 : BEAN */
	{0x35,  0,  35, 110, 0xC27E}, /* 61 : EGG */
	{0x35,  0,  35, 110, 0xC27E}, /* 62 : EGG */
};

pers_t pers_list[PERS_MAX] = {
	{ 0, 0, 44,  16,  0}, /*  0 : VORT */
	{ 5, 0, 49,  25,  6}, /*  1 : ASPIRANT */
	{ 0, 0, 49,  89, 19}, /*  2 : ASPIRANT */
	{ 0, 0, 49, 153, 15}, /*  3 : ASPIRANT */
	{ 0, 0, 49, 217, 11}, /*  4 : ASPIRANT */
	{59, 0, 51, 209,  0}, /*  5 : TURKEY */
	{70, 0, 52, 186,  0}, /*  6 : PRIESTESS */
	{71, 0, 52, 202,  0}, /*  7 : PRIESTESS */
	{ 2, 0, 42,  42,  0}, /*  8 : THE MASTER OF ORDEALS */
	{51, 0, 43,  52,  0}, /*  9 : PROTOZORQ */
	{51, 0, 43,  52,  0}, /* 10 : PROTOZORQ */
	{51, 0, 43,  52,  0}, /* 11 : PROTOZORQ */
	{51, 0, 43,  52,  0}, /* 12 : PROTOZORQ */
	{12, 0, 43,  52,  0}, /* 13 : PROTOZORQ */
	{ 5, 0, 43,  52,  0}, /* 14 : PROTOZORQ */
	{ 2, 0, 43,  52,  0}, /* 15 : PROTOZORQ */
	{ 2, 0, 43,  53,  0}, /* 16 : PROTOZORQ */
	{ 2, 0, 43,  54,  0}, /* 17 : PROTOZORQ */
	{ 5, 0, 43,  53,  0}, /* 18 : PROTOZORQ */
	{55, 0, 43,  36,  0}, /* 19 : PROTOZORQ */
	{56, 0, 43,  53,  0}, /* 20 : PROTOZORQ */
	{56, 0, 43,  54,  0}, /* 21 : PROTOZORQ */
	{ 0, 0, 43,  55,  0}, /* 22 : PROTOZORQ */
	{36, 0, 45, 130,  0}, /* 23 : THE POORMOUTH */
	{40, 0, 46,   2,  0}, /* 24 : KHELE */
	{41, 0, 47, 194,  0}, /* 25 : THE MISTRESS */
	{50, 0, 48, 138,  0}, /* 26 : DEILOS */
	{99, 0, 53, 236,  0}, /* 27 : SCI FI */
	{77, 0, 54,  58,  0}, /* 28 : NORMAJEEN */
	{77, 0, 55,  65,  0}, /* 29 : ASH */
	{59, 0, 56, 125,  0}, /* 30 : MONKEY */
	{ 0, 0, 57, 166,  0}, /* 31 : HARSSK */
	{ 0, 0, 58, 175,  0}, /* 32 : ZORQ */
	{ 0, 0, 59, 183,  0}, /* 33 : QRIICH */
	{ 0, 0, 44,   8,  0}, /* 34 : VORT */
	{ 0, 0, 44,   0,  0}, /* 35 : VORT */
	{59, 0, 52, 146,  0}, /* 36 : PRIESTESS */
	{63, 0, 52,  82,  0}, /* 37 : PRIESTESS */
	{ 0, 0, 93, 243,  0}, /* 38 : CADAVER */
	{59, 0, 51, 209,  0}, /* 39 : TURKEY */
	{59, 0, 51, 209,  0}, /* 40 : TURKEY */
};

} // End of namespace Chamber

#endif
