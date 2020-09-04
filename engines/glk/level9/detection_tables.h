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

#include "engines/game.h"

/**
 * Unlike other ScummVM GLK subengines, Level9 has a detection table that was already
 * included in the Level9 interpreter. So it's used instead of a standard MD5-based list
 */

namespace Glk {
namespace Level9 {

/**
 * The following game database is modified for ScummVM based on the data obtained
 * from L9cut's l9data_d.h, and lets us find a game's name from its data CRC.
 * Entries marked "WANTED" in l9data_d.h, and file commentary, have been removed for brevity,
 * and the file has been reformatted (patchlevel data removed).
 *
 * The version of l9data_d.h used is 050 (22 Oct 2002).
 */
static const gln_game_table_t GLN_GAME_TABLE[] = {
	{0x8333, 0xb7, 0xe2ac, "adrianmole1", "Adrian Mole I", "pt. 1/Commodore 64"},
	{0x844d, 0x50, 0x5353, "adrianmole1", "Adrian Mole I", "pt. 2/Commodore 64"},
	{0x8251, 0x5f, 0x862a, "adrianmole1", "Adrian Mole I", "pt. 3/Commodore 64"},
	{0x7a78, 0x5e, 0x6ea3, "adrianmole1", "Adrian Mole I", "pt. 4/Commodore 64"},

	{0x7c6f, 0x0f, 0xba24, "adrianmole1", "Adrian Mole I", "pt. 1/Amstrad CPC"},

	{0x72fa, 0x8b, 0x6f12, "adrianmole1", "Adrian Mole I", "pt. 1/Spectrum"},
	{0x738e, 0x5b, 0x7e3d, "adrianmole1", "Adrian Mole I", "pt. 2/Spectrum"},
	{0x7375, 0xe5, 0x3f3e, "adrianmole1", "Adrian Mole I", "pt. 3/Spectrum"},
	{0x78d5, 0xe3, 0xcd7d, "adrianmole1", "Adrian Mole I", "pt. 4/Spectrum"},

	{0x3a31, 0xe5, 0x0bdb, "adrianmole1", "Adrian Mole I", "pt. 1/BBC"},
	{0x37f1, 0x77, 0xd231, "adrianmole1", "Adrian Mole I", "pt. 2/BBC"},
	{0x3900, 0x1c, 0x5d9a, "adrianmole1", "Adrian Mole I", "pt. 3/BBC"},
	{0x3910, 0xac, 0x07f9, "adrianmole1", "Adrian Mole I", "pt. 4/BBC"},
	{0x3ad6, 0xa7, 0x95d2, "adrianmole1", "Adrian Mole I", "pt. 5/BBC"},
	{0x38a5, 0x0f, 0xdefc, "adrianmole1", "Adrian Mole I", "pt. 6/BBC"},
	{0x361e, 0x7e, 0xfd9f, "adrianmole1", "Adrian Mole I", "pt. 7/BBC"},
	{0x3934, 0x75, 0xe141, "adrianmole1", "Adrian Mole I", "pt. 8/BBC"},
	{0x3511, 0xcc, 0xd829, "adrianmole1", "Adrian Mole I", "pt. 9/BBC"},
	{0x38dd, 0x31, 0x2534, "adrianmole1", "Adrian Mole I", "pt. 10/BBC"},
	{0x39c0, 0x44, 0x89df, "adrianmole1", "Adrian Mole I", "pt. 11/BBC"},
	{0x3a12, 0x8f, 0xc2bd, "adrianmole1", "Adrian Mole I", "pt. 12/BBC"},

	{0x7931, 0xb9, 0xc51b, "adrianmole2", "Adrian Mole II", "pt. 1/Commodore 64/Amstrad CPC"},
	{0x7cdf, 0xa5, 0x43e3, "adrianmole2", "Adrian Mole II", "pt. 2/Commodore 64/Amstrad CPC"},
	{0x7a0c, 0x97, 0x4bea, "adrianmole2", "Adrian Mole II", "pt. 3/Commodore 64/Amstrad CPC"},
	{0x7883, 0xe2, 0xee0e, "adrianmole2", "Adrian Mole II", "pt. 4/Commodore 64/Amstrad CPC"},

	{0x6841, 0x4a, 0x94e7, "adrianmole2", "Adrian Mole II", "pt. 1/Spectrum"},
	{0x6bc0, 0x62, 0xab3d, "adrianmole2", "Adrian Mole II", "pt. 2/Spectrum"},
	{0x692c, 0x21, 0x2015, "adrianmole2", "Adrian Mole II", "pt. 3/Spectrum"},
	{0x670a, 0x94, 0xa2a6, "adrianmole2", "Adrian Mole II", "pt. 4/Spectrum"},

	{0x593a, 0xaf, 0x30e9, "adrianmole2", "Adrian Mole II", "pt. 1/BBC"},
	{0x57e6, 0x8a, 0xc41a, "adrianmole2", "Adrian Mole II", "pt. 2/BBC"},
	{0x5819, 0xcd, 0x1ba0, "adrianmole2", "Adrian Mole II", "pt. 3/BBC"},
	{0x579b, 0xad, 0xa723, "adrianmole2", "Adrian Mole II", "pt. 4/BBC"},

	{0x765d, 0xcd, 0xfc02, "thearchers", "The Archers", "pt. 1/Commodore 64"},
	{0x6e58, 0x07, 0xbffc, "thearchers", "The Archers", "pt. 2/Commodore 64"},
	{0x7e98, 0x6a, 0x95e5, "thearchers", "The Archers", "pt. 3/Commodore 64"},
	{0x81e2, 0xd5, 0xb278, "thearchers", "The Archers", "pt. 4/Commodore 64"},

	{0x6ce5, 0x58, 0x46de, "thearchers", "The Archers", "pt. 1/Spectrum"},
	{0x68da, 0xc1, 0x3b8e, "thearchers", "The Archers", "pt. 2/Spectrum"},
	{0x6c67, 0x9a, 0x9a6a, "thearchers", "The Archers", "pt. 3/Spectrum"},
	{0x6d91, 0xb9, 0x12a7, "thearchers", "The Archers", "pt. 4/Spectrum"},

	{0x5834, 0x42, 0xcc9d, "thearchers", "The Archers", "pt. 1/BBC"},
	{0x56dd, 0x51, 0xe582, "thearchers", "The Archers", "pt. 2/BBC"},
	{0x5801, 0x53, 0xf2ef, "thearchers", "The Archers", "pt. 3/BBC"},
	{0x54a4, 0x01, 0xc0ab, "thearchers", "The Archers", "pt. 4/BBC"},

	{0x5323, 0xb7, 0x8af7, "adventurequest", "Adventure Quest", "Amstrad CPC/Spectrum"},
	{0x6e60, 0x83, 0x18e0, "adventurequest", "Adventure Quest /JoD", "Amiga/PC"},
	{0x6e5c, 0xf6, 0xd356, "adventurequest", "Adventure Quest /JoD", "ST"},
	{0x6970, 0xd6, 0xa820, "adventurequest", "Adventure Quest /JoD", "Spectrum 128"},
	{0x6968, 0x32, 0x0c01, "adventurequest", "Adventure Quest /JoD", "Amstrad CPC128/Spectrum +3"},
	{0x5b50, 0x66, 0x1800, "adventurequest", "Adventure Quest /JoD", "Amstrad CPC64"},
	{0x63b6, 0x2e, 0xef38, "adventurequest", "Adventure Quest /JoD", "Commodore 64"},
	{0x5b58, 0x50, 0x332e, "adventurequest", "Adventure Quest /JoD", "Atari"},
	{0x5ace, 0x11, 0xdc12, "adventurequest", "Adventure Quest /JoD", "Spectrum 48"},

	{0x76f4, 0x5e, 0x1fe5, "colossaladvjod", "Colossal Adventure /JoD", "Amiga/PC"},
	{0x76f4, 0x5a, 0xcf4b, "colossaladvjod", "Colossal Adventure /JoD", "ST"},
	{0x6f70, 0x40, 0xbd91, "colossaladvjod", "Colossal Adventure /JoD", "MSX"},
	{0x6f6e, 0x78, 0x28cd, "colossaladvjod", "Colossal Adventure /JoD", "Spectrum 128"},
	{0x6f4d, 0xcb, 0xe8f2, "colossaladvjod", "Colossal Adventure /JoD", "Amstrad CPC128[v1]/Spectrum +3"},
	{0x6f6a, 0xa5, 0x8dd2, "colossaladvjod", "Colossal Adventure /JoD", "Amstrad CPC128[v2]"},
	{0x5e31, 0x7c, 0xaa54, "colossaladvjod", "Colossal Adventure /JoD", "Amstrad CPC64"},
	{0x6c8e, 0xb6, 0x9be3, "colossaladvjod", "Colossal Adventure /JoD", "Commodore 64"},
	{0x5b16, 0x3b, 0xe2aa, "colossaladvjod", "Colossal Adventure /JoD", "Atari"},
	{0x5a8e, 0xf2, 0x7cca, "colossaladvjod", "Colossal Adventure /JoD", "Spectrum 48"},

	{0x630e, 0x8d, 0x7d7d, "dungeonadv", "Dungeon Adventure", "Amstrad CPC"},
	{0x630e, 0xbe, 0x3374, "dungeonadv", "Dungeon Adventure", "MSX"},
	{0x6de8, 0x4c, 0xd795, "dungeonadvjod", "Dungeon Adventure /JoD", "Spectrum 128"},
	{0x6dc0, 0x63, 0x5d95, "dungeonadvjod", "Dungeon Adventure /JoD", "Amstrad CPC128/Spectrum +3"},
	{0x58a6, 0x24, 0xb50f, "dungeonadvjod", "Dungeon Adventure /JoD", "Amstrad CPC64"},
	{0x6bd2, 0x65, 0xa41f, "dungeonadvjod", "Dungeon Adventure /JoD", "Commodore 64"},
	{0x593a, 0x80, 0x7a34, "dungeonadvjod", "Dungeon Adventure /JoD", "Atari"},
	{0x58a3, 0x38, 0x8ce4, "dungeonadvjod", "Dungeon Adventure /JoD", "Spectrum 48"},
	{0x6f0c, 0x95, 0x1f64, "dungeonadvjod", "Dungeon Adventure /JoD", "Amiga/PC/ST"},

	{0x63be, 0xd6, 0xcf5d, "emeraldisle", "Emerald Isle", "Atari/Commodore 64/Amstrad CPC/Spectrum"},
	{0x63be, 0x0a, 0x21ed, "emeraldisle", "Emerald Isle", "MSX *corrupt*"},
	{0x378c, 0x8d, 0x3a21, "emeraldisle", "Emerald Isle", "BBC"},

	{0x34b3, 0x20, 0xccda, "eriktheviking", "Erik the Viking", "BBC/Commodore 64"},
	{0x34b3, 0x53, 0x8f00, "eriktheviking", "Erik the Viking", "Spectrum"},
	{0x34b3, 0xc7, 0x9058, "eriktheviking", "Erik the Viking", "Amstrad CPC"},

	{0x5ff0, 0xf8, 0x3a13, "gnomeranger", "Gnome Ranger", "pt. 1 GD/Amstrad CPC/Spectrum +3"},
	{0x6024, 0x01, 0xaaa9, "gnomeranger", "Gnome Ranger", "pt. 2 GD/Amstrad CPC/Spectrum +3"},
	{0x6036, 0x3d, 0x6c6c, "gnomeranger", "Gnome Ranger", "pt. 3 GD/Amstrad CPC/Spectrum +3"},

	{0x52aa, 0xdf, 0x7b5b, "gnomeranger", "Gnome Ranger", "pt. 1 GD/Spectrum 128"},
	{0x6ffa, 0xdb, 0xdde2, "gnomeranger", "Gnome Ranger", "pt. 2 GD/Spectrum 128"},
	{0x723a, 0x69, 0x039b, "gnomeranger", "Gnome Ranger", "pt. 3 GD/Spectrum 128"},

	{0xb1a9, 0x80, 0x5fb7, "gnomeranger", "Gnome Ranger", "pt. 1/Amiga/ST"},
	{0xab9d, 0x31, 0xbe6d, "gnomeranger", "Gnome Ranger", "pt. 2/Amiga/ST"},
	{0xae28, 0x87, 0xb6b6, "gnomeranger", "Gnome Ranger", "pt. 3/Amiga/ST"},

	{0xb0ec, 0xc2, 0x0053, "gnomeranger", "Gnome Ranger", "pt. 1/ST[v1]"},
	{0xaf82, 0x83, 0x19f7, "gnomeranger", "Gnome Ranger", "pt. 2/ST[v1]"},

	{0xb1aa, 0xad, 0xaf47, "gnomeranger", "Gnome Ranger", "pt. 1/PC"},
	{0xb19e, 0x92, 0x8f96, "gnomeranger", "Gnome Ranger", "pt. 1/ST[v2]"},
	{0xab8b, 0xbf, 0x31e6, "gnomeranger", "Gnome Ranger", "pt. 2/PC/ST[v2]"},
	{0xae16, 0x81, 0x8741, "gnomeranger", "Gnome Ranger", "pt. 3/PC/ST[v2]"},

	{0xad41, 0xa8, 0x42c5, "gnomeranger", "Gnome Ranger", "pt. 1/Commodore 64 TO"},
	{0xa735, 0xf7, 0x2e08, "gnomeranger", "Gnome Ranger", "pt. 2/Commodore 64 TO"},
	{0xa9c0, 0x9e, 0x0d70, "gnomeranger", "Gnome Ranger", "pt. 3/Commodore 64 TO"},

	{0x908e, 0x0d, 0x58a7, "gnomeranger", "Gnome Ranger", "pt. 1/Commodore 64 Gfx"},
	{0x8f6f, 0x0a, 0x411a, "gnomeranger", "Gnome Ranger", "pt. 2/Commodore 64 Gfx"},
	{0x9060, 0xbb, 0xe75d, "gnomeranger", "Gnome Ranger", "pt. 3/Commodore 64 Gfx"},

	{0x8aab, 0xc0, 0xde5f, "gnomeranger", "Gnome Ranger", "pt. 1/Spectrum 48"},
	{0x8ac8, 0x9a, 0xc89b, "gnomeranger", "Gnome Ranger", "pt. 2/Spectrum 48"},
	{0x8a93, 0x4f, 0x10cc, "gnomeranger", "Gnome Ranger", "pt. 3/Spectrum 48"},

	{0x5a38, 0xf7, 0x876e, "ingridsback", "Ingrid's Back", "pt. 1 GD/Amstrad CPC/Spectrum +3"},
	{0x531a, 0xed, 0xcf3f, "ingridsback", "Ingrid's Back", "pt. 2 GD/Amstrad CPC/Spectrum +3"},
	{0x57e4, 0x19, 0xb354, "ingridsback", "Ingrid's Back", "pt. 3 GD/Amstrad CPC/Spectrum +3"},

	{0x76a0, 0x3a, 0xb803, "ingridsback", "Ingrid's Back", "pt. 1 GD/Spectrum 128"},
	{0x7674, 0x0b, 0xe92f, "ingridsback", "Ingrid's Back", "pt. 2 GD/Spectrum 128"},
	{0x765e, 0xba, 0x086d, "ingridsback", "Ingrid's Back", "pt. 3 GD/Spectrum 128"},

	{0xd19b, 0xad, 0x306d, "ingridsback", "Ingrid's Back", "pt. 1/PC"},
	{0xc5a5, 0xfe, 0x3c98, "ingridsback", "Ingrid's Back", "pt. 2/PC"},
	{0xd7ae, 0x9e, 0x1878, "ingridsback", "Ingrid's Back", "pt. 3/PC"},

	{0xd188, 0x13, 0xdc60, "ingridsback", "Ingrid's Back", "pt. 1/Amiga"},
	{0xc594, 0x03, 0xea95, "ingridsback", "Ingrid's Back", "pt. 2/Amiga"},
	{0xd79f, 0xb5, 0x1661, "ingridsback", "Ingrid's Back", "pt. 3/Amiga"},

	{0xd183, 0x83, 0xef72, "ingridsback", "Ingrid's Back", "pt. 1/ST"},
	{0xc58f, 0x65, 0xf337, "ingridsback", "Ingrid's Back", "pt. 2/ST"},
	{0xd79a, 0x57, 0x49c5, "ingridsback", "Ingrid's Back", "pt. 3/ST"},

	{0xb770, 0x03, 0x9a03, "ingridsback", "Ingrid's Back", "pt. 1/Commodore 64 TO"},
	{0xb741, 0xb6, 0x2aa5, "ingridsback", "Ingrid's Back", "pt. 2/Commodore 64 TO"},
	{0xb791, 0xa1, 0xd065, "ingridsback", "Ingrid's Back", "pt. 3/Commodore 64 TO"},

	{0x9089, 0xce, 0xc5e2, "ingridsback", "Ingrid's Back", "pt. 1/Commodore 64 Gfx"},
	{0x908d, 0x80, 0x30c7, "ingridsback", "Ingrid's Back", "pt. 2/Commodore 64 Gfx"},
	{0x909e, 0x9f, 0xdecc, "ingridsback", "Ingrid's Back", "pt. 3/Commodore 64 Gfx"},

	{0x8ab7, 0x68, 0xee57, "ingridsback", "Ingrid's Back", "pt. 1/Spectrum 48"},
	{0x8b1e, 0x84, 0x2538, "ingridsback", "Ingrid's Back", "pt. 2/Spectrum 48"},
	{0x8b1c, 0xa8, 0x9262, "ingridsback", "Ingrid's Back", "pt. 3/Spectrum 48"},

	{0x46ec, 0x64, 0x2300, "knightorc", "Knight Orc", "pt. 1 GD/Amstrad CPC/Spectrum +3"},
	{0x6140, 0x18, 0x4f66, "knightorc", "Knight Orc", "pt. 2 GD/Amstrad CPC/Spectrum +3"},
	{0x640e, 0xc1, 0xfc69, "knightorc", "Knight Orc", "pt. 3 GD/Amstrad CPC/Spectrum +3"},

	{0x74e0, 0x92, 0x885e, "knightorc", "Knight Orc", "pt. 1 GD/Spectrum 128"},
	{0x6dbc, 0x97, 0x6f55, "knightorc", "Knight Orc", "pt. 2 GD/Spectrum 128"},
	{0x7402, 0x07, 0x385f, "knightorc", "Knight Orc", "pt. 3 GD/Spectrum 128"},

	{0xbb93, 0x36, 0x6a05, "knightorc", "Knight Orc", "pt. 1/Amiga"},
	{0xbb6e, 0xad, 0x4d40, "knightorc", "Knight Orc", "pt. 1/ST"},
	{0xc58e, 0x4a, 0x4e9d, "knightorc", "Knight Orc", "pt. 2/Amiga/ST"},
	{0xcb9a, 0x0f, 0x0804, "knightorc", "Knight Orc", "pt. 3/Amiga/ST"},

	{0xbb6e, 0xa6, 0x9753, "knightorc", "Knight Orc", "pt. 1/PC"},
	{0xc58e, 0x43, 0xe9ce, "knightorc", "Knight Orc", "pt. 2/PC"},
	{0xcb9a, 0x08, 0x6c36, "knightorc", "Knight Orc", "pt. 3/PC"},

	{0x898a, 0x43, 0xfc8b, "knightorc", "Knight Orc", "pt. 1/Apple ]["},
	{0x8b9f, 0x61, 0x7288, "knightorc", "Knight Orc", "pt. 2/Apple ]["},
	{0x8af9, 0x61, 0x7542, "knightorc", "Knight Orc", "pt. 3/Apple ]["},

	{0x8970, 0x6b, 0x3c7b, "knightorc", "Knight Orc", "pt. 1/Commodore 64 Gfx"},
	{0x8b90, 0x4e, 0x098c, "knightorc", "Knight Orc", "pt. 2/Commodore 64 Gfx"},
	{0x8aea, 0x4e, 0xca54, "knightorc", "Knight Orc", "pt. 3/Commodore 64 Gfx"},

	{0x86d0, 0xb7, 0xadbd, "knightorc", "Knight Orc", "pt. 1/Spectrum 48"},
	{0x8885, 0x22, 0xe293, "knightorc", "Knight Orc", "pt. 2/Spectrum 48"},
	{0x87e5, 0x0e, 0xdc33, "knightorc", "Knight Orc", "pt. 3/Spectrum 48"},

	{0x4fd2, 0x9d, 0x799a, "lancelot", "Lancelot", "pt. 1 GD/BBC"},
	{0x4dac, 0xa8, 0x86ed, "lancelot", "Lancelot", "pt. 2 GD/BBC"},
	{0x4f96, 0x22, 0x30f8, "lancelot", "Lancelot", "pt. 3 GD/BBC"},

	{0x5c7a, 0x44, 0x460e, "lancelot", "Lancelot", "pt. 1 GD/Amstrad CPC/Spectrum +3"},
	{0x53a2, 0x1e, 0x2fae, "lancelot", "Lancelot", "pt. 2 GD/Amstrad CPC/Spectrum +3"},
	{0x5914, 0x22, 0x4a31, "lancelot", "Lancelot", "pt. 3 GD/Amstrad CPC/Spectrum +3"},

	{0x768c, 0xe8, 0x8fc6, "lancelot", "Lancelot", "pt. 1 GD/Spectrum 128"},
	{0x76b0, 0x1d, 0x0fcd, "lancelot", "Lancelot", "pt. 2 GD/Spectrum 128"},
	{0x765e, 0x4f, 0x3b73, "lancelot", "Lancelot", "pt. 3 GD/Spectrum 128"},

	{0xc0cf, 0x4e, 0xb7fa, "lancelot", "Lancelot", "pt. 1/Amiga/PC/ST"},
	{0xd5e9, 0x6a, 0x4192, "lancelot", "Lancelot", "pt. 2/Amiga/PC/ST"},
	{0xbb8f, 0x1a, 0x7487, "lancelot", "Lancelot", "pt. 3/Amiga/PC/ST"},

	{0xc0bd, 0x57, 0x6ef1, "lancelot", "Lancelot", "pt. 1/Mac"},
	{0xd5d7, 0x99, 0x770b, "lancelot", "Lancelot", "pt. 2/Mac"},
	{0xbb7d, 0x17, 0xbc42, "lancelot", "Lancelot", "pt. 3/Mac"},

	{0xb4c9, 0x94, 0xd784, "lancelot", "Lancelot", "pt. 1/Commodore 64 TO"},
	{0xb729, 0x51, 0x8ee5, "lancelot", "Lancelot", "pt. 2/Commodore 64 TO"},
	{0xb702, 0xe4, 0x1809, "lancelot", "Lancelot", "pt. 3/Commodore 64 TO"},

	{0x8feb, 0xba, 0xa800, "lancelot", "Lancelot", "pt. 1/Commodore 64 Gfx"},
	{0x8f6b, 0xfa, 0x0f7e, "lancelot", "Lancelot", "pt. 2/Commodore 64 Gfx"},
	{0x8f71, 0x2f, 0x0ddc, "lancelot", "Lancelot", "pt. 3/Commodore 64 Gfx"},

	{0x8ade, 0xf2, 0xfffb, "lancelot", "Lancelot", "pt. 1/Spectrum 48"},
	{0x8b0e, 0xfb, 0x0bab, "lancelot", "Lancelot", "pt. 2/Spectrum 48"},
	{0x8ab3, 0xc1, 0xcb62, "lancelot", "Lancelot", "pt. 3/Spectrum 48"},

	{0xbba4, 0x94, 0x0871, "lancelot", "Lancelot", "pt. 1/Amiga/PC *USA*"},
	{0xd0c0, 0x56, 0x8c48, "lancelot", "Lancelot", "pt. 2/Amiga/PC *USA*"},
	{0xb6ac, 0xc6, 0xaea0, "lancelot", "Lancelot", "pt. 3/Amiga/PC *USA*"},

	{0x8afc, 0x07, 0x8321, "lancelot", "Lancelot", "pt. 1/Commodore 64 Gfx *USA*"},
	{0x8aec, 0x13, 0x6791, "lancelot", "Lancelot", "pt. 2/Commodore 64 Gfx *USA*"},
	{0x8aba, 0x0d, 0x5602, "lancelot", "Lancelot", "pt. 3/Commodore 64 Gfx *USA*"},

	{0x5eb9, 0x30, 0xe99a, "lordsoftime", "Lords of Time", "Amstrad CPC"},
	{0x5eb9, 0x5d, 0xc098, "lordsoftime", "Lords of Time", "MSX"},
	{0x5eb9, 0x6e, 0xc689, "lordsoftime", "Lords of Time", "Spectrum"},
	{0x579e, 0x97, 0x9faa, "lordsoftimetmgd", "Lords of Time /T&M GD", "BBC"},
	{0x69fe, 0x56, 0xecfb, "lordsoftimetmgd", "Lords of Time /T&M GD", "Amstrad CPC/Spectrum +3"},
	{0x6f1e, 0xda, 0x2ce0, "lordsoftimetmgd", "Lords of Time /T&M GD", "Spectrum 128"},
	{0xb57c, 0x44, 0x7779, "lordsoftimetm", "Lords of Time /T&M", "PC"},
	{0xb579, 0x89, 0x3e89, "lordsoftimetm", "Lords of Time /T&M", "ST"},
	{0xb576, 0x2a, 0x7239, "lordsoftimetm", "Lords of Time /T&M", "Amiga"},
	{0xb563, 0x6a, 0x0c5c, "lordsoftimetm", "Lords of Time /T&M", "Mac"},
	{0xb38c, 0x37, 0x9f8e, "lordsoftimetm", "Lords of Time /T&M", "Commodore 64 TO"},
	{0x9070, 0x43, 0x45d4, "lordsoftimetm", "Lords of Time /T&M", "Commodore 64 Gfx"},
	{0x8950, 0xa1, 0xbb16, "lordsoftimetm", "Lords of Time /T&M", "Spectrum 48"},
	{0xb260, 0xe5, 0xc5b2, "lordsoftimetm", "Lords of Time /T&M", "PC/ST *USA*"},
	{0xb257, 0xf8, 0xfbd5, "lordsoftimetm", "Lords of Time /T&M", "Amiga *USA*"},
	{0x8d78, 0x3a, 0xba6e, "lordsoftimetm", "Lords of Time /T&M", "Commodore 64 Gfx *USA*"},

	{0x7410, 0x5e, 0x60be, "priceofmagik", "Price of Magik", "Spectrum 128"},
	{0x5aa4, 0xc1, 0x10a0, "priceofmagik", "Price of Magik", "Spectrum 48[v1]"},
	{0x5aa4, 0xc1, 0xeda4, "priceofmagik", "Price of Magik", "Spectrum 48[v2]"},
	{0x6fc6, 0x14, 0xf9b6, "priceofmagik", "Price of Magik", "Commodore 64"},
	{0x5aa4, 0xc1, 0xbbf4, "priceofmagik", "Price of Magik", "Amstrad CPC"},
	{0x5671, 0xbc, 0xff35, "priceofmagik", "Price of Magik", "BBC"},
	{0x6108, 0xdd, 0xefe7, "priceofmagiktmgd", "Price of Magik /T&M GD", "Spectrum 128"},
	{0x579a, 0x2a, 0x9373, "priceofmagiktmgd", "Price of Magik /T&M GD", "BBC"},
	{0x5a50, 0xa9, 0xa5fa, "priceofmagiktmgd", "Price of Magik /T&M GD", "Amstrad CPC/Spectrum +3"},
	{0xbac7, 0x7f, 0xddb2, "priceofmagiktm", "Price of Magik /T&M", "PC"},
	{0xbac4, 0x80, 0xa750, "priceofmagiktm", "Price of Magik /T&M", "ST"},
	{0xbaca, 0x3a, 0x221b, "priceofmagiktm", "Price of Magik /T&M", "Amiga"},
	{0xbab2, 0x87, 0x09f5, "priceofmagiktm", "Price of Magik /T&M", "Mac"},
	{0xb451, 0xa8, 0x2682, "priceofmagiktm", "Price of Magik /T&M", "Commodore 64 TO"},
	{0x8f51, 0xb2, 0x6c9a, "priceofmagiktm", "Price of Magik /T&M", "Commodore 64 Gfx"},
	{0x8a60, 0x2a, 0x29ed, "priceofmagiktm", "Price of Magik /T&M", "Spectrum 48"},
	{0xb7a0, 0x7e, 0x2226, "priceofmagiktm", "Price of Magik /T&M", "PC/ST *USA*"},
	{0xb797, 0x1f, 0x84a9, "priceofmagiktm", "Price of Magik /T&M", "Amiga *USA*"},
	{0x8c46, 0xf0, 0xcaf6, "priceofmagiktm", "Price of Magik /T&M", "Commodore 64 Gfx *USA*"},

	{0x506c, 0xf0, 0xba72, "redmoon", "Red Moon", "BBC/Commodore 64/Amstrad CPC/MSX"},
	{0x505d, 0x32, 0x2dcf, "redmoon", "Red Moon", "Spectrum"},
	{0x6da0, 0xb8, 0x3802, "redmoontmgd", "Red Moon /T&M GD", "Spectrum 128"},
	{0x6888, 0x8d, 0x7f6a, "redmoontmgd", "Red Moon /T&M GD", "Amstrad CPC/Spectrum +3"},
	{0x5500, 0x50, 0xca75, "redmoontmgd", "Red Moon /T&M GD", "BBC"},
	{0xa69e, 0x6c, 0xb268, "redmoontm", "Red Moon /T&M", "PC"},
	{0xa698, 0x41, 0xcaca, "redmoontm", "Red Moon /T&M", "ST"},
	{0xa692, 0xd1, 0x6a99, "redmoontm", "Red Moon /T&M", "Amiga"},
	{0xa67c, 0xb8, 0xff41, "redmoontm", "Red Moon /T&M", "Mac"},
	{0xa4e2, 0xa6, 0x016d, "redmoontm", "Red Moon /T&M", "Commodore 64 TO"},
	{0x903f, 0x6b, 0x603e, "redmoontm", "Red Moon /T&M", "Commodore 64 Gfx"},
	{0x8813, 0x11, 0x22de, "redmoontm", "Red Moon /T&M", "Spectrum 48"},
	{0xa3a4, 0xdf, 0x6732, "redmoontm", "Red Moon /T&M", "PC/ST *USA*"},
	{0xa398, 0x82, 0xd031, "redmoontm", "Red Moon /T&M", "Amiga *USA*"},
	{0x8d56, 0xd3, 0x146a, "redmoontm", "Red Moon /T&M", "Commodore 64 Gfx *USA*"},

	{0x60c4, 0x28, 0x0154, "returntoeden", "Return to Eden", "Amstrad CPC/Commodore 64[v1]"},
	{0x6064, 0x01, 0x5b3c, "returntoeden", "Return to Eden", "BBC[v1]"},
	{0x6064, 0x95, 0x510c, "returntoeden", "Return to Eden", "Commodore 64[v2]"},
	{0x6064, 0xda, 0xe610, "returntoeden", "Return to Eden", "Commodore 64[v2] *corrupt*"},
	{0x6064, 0xbd, 0x73ec, "returntoeden", "Return to Eden", "Atari *corrupt*"},
	{0x6047, 0x6c, 0x17ab, "returntoeden", "Return to Eden", "BBC[v2]"},
	{0x5ca1, 0x33, 0x1c43, "returntoeden", "Return to Eden", "Spectrum[v1]"},
	{0x5cb7, 0x64, 0x0790, "returntoeden", "Return to Eden", "Spectrum[v2]"},
	{0x5cb7, 0xfe, 0x3533, "returntoeden", "Return to Eden", "MSX"},
	{0x7d16, 0xe6, 0x5438, "returntoedensd", "Return to Eden /SD", "Amiga/ST"},
	{0x7d14, 0xe8, 0xfbab, "returntoedensd", "Return to Eden /SD", "PC"},
	{0x7cff, 0xf8, 0x6044, "returntoedensd", "Return to Eden /SD", "Amstrad CPC/Spectrum +3"},
	{0x7cf8, 0x24, 0x9c1c, "returntoedensd", "Return to Eden /SD", "Mac"},
	{0x7c55, 0x18, 0xdaee, "returntoedensd", "Return to Eden /SD", "Spectrum 128"},
	{0x772f, 0xca, 0x8602, "returntoedensd", "Return to Eden /SD", "Commodore 64"},
	{0x60f7, 0x68, 0xc2bc, "returntoedensd", "Return to Eden /SD", "Atari"},
	{0x639c, 0x8b, 0x06e2, "returntoedensd", "Return to Eden /SD", "Apple ]["},
	{0x5f43, 0xca, 0x828c, "returntoedensd", "Return to Eden /SD", "Spectrum 48"},

	{0x55ce, 0xa1, 0xba12, "scapeghost", "Scapeghost", "pt. 1 GD/BBC"},
	{0x54a6, 0xa9, 0xc9f3, "scapeghost", "Scapeghost", "pt. 2 GD/BBC"},
	{0x51bc, 0xe3, 0x89c3, "scapeghost", "Scapeghost", "pt. 3 GD/BBC"},

	{0x5cbc, 0xa5, 0x0dbe, "scapeghost", "Scapeghost", "pt. 1 GD/Amstrad CPC/Spectrum +3"},
	{0x5932, 0x4e, 0xb2f5, "scapeghost", "Scapeghost", "pt. 2 GD/Amstrad CPC/Spectrum +3"},
	{0x5860, 0x95, 0x3227, "scapeghost", "Scapeghost", "pt. 3 GD/Amstrad CPC/Spectrum +3"},

	{0x762e, 0x82, 0x8848, "scapeghost", "Scapeghost", "pt. 1 GD/Spectrum 128"},
	{0x5bd6, 0x35, 0x79ef, "scapeghost", "Scapeghost", "pt. 2 GD/Spectrum 128"},
	{0x6fa8, 0xa4, 0x62c2, "scapeghost", "Scapeghost", "pt. 3 GD/Spectrum 128"},

	{0xbeab, 0x2d, 0x94d9, "scapeghost", "Scapeghost", "pt. 1/Amiga"},
	{0xc132, 0x14, 0x7adc, "scapeghost", "Scapeghost", "pt. 1/Amiga *bak*"},
	{0xbe94, 0xcc, 0x04b8, "scapeghost", "Scapeghost", "pt. 1/PC/ST"},
	{0x99bd, 0x65, 0x032e, "scapeghost", "Scapeghost", "pt. 2/Amiga/PC/ST"},
	{0xbcb6, 0x7a, 0x7d4f, "scapeghost", "Scapeghost", "pt. 3/Amiga/PC/ST"},

	{0x9058, 0xcf, 0x9748, "scapeghost", "Scapeghost", "pt. 1/Commodore 64 Gfx"},
	{0x8f43, 0xc9, 0xeefd, "scapeghost", "Scapeghost", "pt. 2/Commodore 64 Gfx"},
	{0x90ac, 0x68, 0xb4a8, "scapeghost", "Scapeghost", "pt. 3/Commodore 64 Gfx"},

	{0x8a21, 0xf4, 0xd9e4, "scapeghost", "Scapeghost", "pt. 1/Spectrum 48"},
	{0x8a12, 0xe3, 0xc2ff, "scapeghost", "Scapeghost", "pt. 2/Spectrum 48"},
	{0x8a16, 0xcc, 0x4f3b, "scapeghost", "Scapeghost", "pt. 3/Spectrum 48"},

	{0x5fab, 0x5c, 0xa309, "snowball", "Snowball", "Amstrad CPC"},
	{0x5fab, 0x2f, 0x8aa2, "snowball", "Snowball", "MSX"},
	{0x7b31, 0x6e, 0x2e2b, "snowballsd", "Snowball /SD", "Amiga/ST"},
	{0x7b2f, 0x70, 0x6955, "snowballsd", "Snowball /SD", "Mac/PC/Spectrum 128"},
	{0x7b2f, 0x70, 0x6f6c, "snowballsd", "Snowball /SD", "Amstrad CPC/Spectrum +3"},
	{0x7363, 0x65, 0xa0ab, "snowballsd", "Snowball /SD", "Commodore 64"},
	{0x6bf8, 0x3f, 0xc9f7, "snowballsd", "Snowball /SD", "Atari"},
	{0x67a3, 0x9d, 0x1d05, "snowballsd", "Snowball /SD", "Apple ]["},
	{0x6541, 0x02, 0x2e6c, "snowballsd", "Snowball /SD", "Spectrum 48"},

	{0x772b, 0xcd, 0xa503, "worminparadise", "Worm in Paradise", "Spectrum 128"},
	{0x546c, 0xb7, 0x9420, "worminparadise", "Worm in Paradise", "Spectrum 48"},
	{0x6d84, 0xf9, 0x49ae, "worminparadise", "Worm in Paradise", "Commodore 64 *corrupt*"},
	{0x6d84, 0xc8, 0x943f, "worminparadise", "Worm in Paradise", "Commodore 64 *fixed*"},
	{0x6030, 0x47, 0x46ad, "worminparadise", "Worm in Paradise", "Amstrad CPC"},
	{0x5828, 0xbd, 0xe7cb, "worminparadise", "Worm in Paradise", "BBC"},
	{0x7cd9, 0x0c, 0x4df1, "worminparadise", "Worm in Paradise /SD", "Amiga/ST"},
	{0x7cd7, 0x0e, 0x4feb, "worminparadisesd", "Worm in Paradise /SD", "Amstrad CPC/Mac/PC/Spectrum 128/Spectrum +3"},
	{0x788d, 0x72, 0x888a, "worminparadisesd", "Worm in Paradise /SD", "Commodore 64"},
	{0x6161, 0xf3, 0xe6d7, "worminparadisesd", "Worm in Paradise /SD", "Atari"},
	{0x60dd, 0xf2, 0x5bb8, "worminparadisesd", "Worm in Paradise /SD", "Apple ]["},
	{0x5ebb, 0xf1, 0x4dec, "worminparadisesd", "Worm in Paradise /SD", "Spectrum 48"},

	{0x0000, 0x00, 0x0000, nullptr, nullptr, nullptr}
};

/**
 * The following patch database is obtained from L9cut's l9data_p.h, and
 * allows CRCs from patched games to be translated into original CRCs for
 * lookup in the game database above.  Some file commentary has been removed
 * for brevity, and unused patch fields deleted.
 *
 * The version of l9data_p.h used is 012 (22 May 2001).
 */
static const gln_patch_table_t GLN_PATCH_TABLE[] = {
	/* Price of Magik (Spectrum128) */
	{0x7410, 0x5e, 0x60be, 0x70, 0x6cef},

	/* Price of Magik (Commodore 64) */
	{0x6fc6, 0x14, 0xf9b6, 0x26, 0x3326},

	/* Price of Magik (Spectrum48) */
	{0x5aa4, 0xc1, 0xeda4, 0xd3, 0xed35},
	{0x5aa4, 0xc1, 0xeda4, 0xc1, 0x8a65},

	/* Colossal Adventure /JoD (Amiga/PC) */
	{0x76f4, 0x5e, 0x1fe5, 0xea, 0x1305},
	{0x76f4, 0x5e, 0x1fe5, 0xb5, 0x901f},
	{0x76f4, 0x5e, 0x1fe5, 0x5e, 0x6ea1},

	/* Colossal Adventure /JoD (ST) */
	{0x76f4, 0x5a, 0xcf4b, 0xe6, 0x012a},
	{0x76f4, 0x5a, 0xcf4b, 0xb1, 0x40b1},

	/* Adventure Quest /JoD (Amiga/PC) */
	{0x6e60, 0x83, 0x18e0, 0x4c, 0xcfb0},
	{0x6e60, 0x83, 0x18e0, 0xfa, 0x9b3b},
	{0x6e60, 0x83, 0x18e0, 0x83, 0x303d},

	/* Adventure Quest /JoD (ST) */
	{0x6e5c, 0xf6, 0xd356, 0xbf, 0xede7},
	{0x6e5c, 0xf6, 0xd356, 0x6d, 0x662d},

	/* Dungeon Adventure /JoD (Amiga/PC/ST) */
	{0x6f0c, 0x95, 0x1f64, 0x6d, 0x2443},
	{0x6f0c, 0x95, 0x1f64, 0x0c, 0x6066},
	{0x6f0c, 0x95, 0x1f64, 0x96, 0xdaca},
	{0x6f0c, 0x95, 0x1f64, 0x95, 0x848d},

	/* Colossal Adventure /JoD (Spectrum128) */
	{0x6f6e, 0x78, 0x28cd, 0xf8, 0xda5f},
	{0x6f6e, 0x78, 0x28cd, 0x77, 0x5b4e},

	/* Adventure Quest /JoD (Spectrum128) */
	{0x6970, 0xd6, 0xa820, 0x3b, 0x1870},
	{0x6970, 0xd6, 0xa820, 0xd5, 0x13c4},

	/* Dungeon Adventure /JoD (Spectrum128) */
	{0x6de8, 0x4c, 0xd795, 0xa2, 0x3eea},
	{0x6de8, 0x4c, 0xd795, 0x4b, 0xad30},

	/* Colossal Adventure /JoD (Amstrad CPC) */
	{0x6f4d, 0xcb, 0xe8f2, 0x4b, 0xb384},
	{0x6f4d, 0xcb, 0xe8f2, 0xca, 0x96e7},

	/* Adventure Quest /JoD (Amstrad CPC) */
	{0x6968, 0x32, 0x0c01, 0x97, 0xdded},
	{0x6968, 0x32, 0x0c01, 0x31, 0xe8c2},

	/* Dungeon Adventure /JoD (Amstrad CPC) */
	{0x6dc0, 0x63, 0x5d95, 0xb9, 0xc963},
	{0x6dc0, 0x63, 0x5d95, 0x62, 0x79f7},

	/* Colossal Adventure /JoD (Commodore 64) */
	{0x6c8e, 0xb6, 0x9be3, 0x36, 0x6971},
	{0x6c8e, 0xb6, 0x9be3, 0xb5, 0xeba0},

	/* Adventure Quest /JoD (Commodore 64) */
	{0x63b6, 0x2e, 0xef38, 0x93, 0x4e68},
	{0x63b6, 0x2e, 0xef38, 0x2d, 0x54dc},

	/* Dungeon Adventure /JoD (Commodore 64) */
	{0x6bd2, 0x65, 0xa41f, 0xbb, 0x4260},
	{0x6bd2, 0x65, 0xa41f, 0x64, 0xdf5a},

	/* Colossal Adventure /JoD (Spectrum48) */
	{0x5a8e, 0xf2, 0x7cca, 0x72, 0x8e58},
	{0x5a8e, 0xf2, 0x7cca, 0xf1, 0x0c89},
	{0x5a8e, 0xf2, 0x7cca, 0xf2, 0x2c96},

	/* Adventure Quest /JoD (Spectrum48) */
	{0x5ace, 0x11, 0xdc12, 0x76, 0x8663},
	{0x5ace, 0x11, 0xdc12, 0x10, 0xa757},
	{0x5ace, 0x11, 0xdc12, 0x11, 0xf118},

	/* Dungeon Adventure /JoD (Spectrum48) */
	{0x58a3, 0x38, 0x8ce4, 0x8e, 0xb61a},
	{0x58a3, 0x38, 0x8ce4, 0x37, 0x34c0},
	{0x58a3, 0x38, 0x8ce4, 0x38, 0xa1ee},

	/* Snowball /SD (Amiga/ST) */
	{0x7b31, 0x6e, 0x2e2b, 0xe5, 0x6017},

	/* Return to Eden /SD (Amiga/ST) */
	{0x7d16, 0xe6, 0x5438, 0x5d, 0xc770},

	/* Worm in Paradise /SD (Amiga/ST) */
	{0x7cd9, 0x0c, 0x4df1, 0x83, 0xe997},

	/* Snowball /SD (PC/Spectrum128) */
	{0x7b2f, 0x70, 0x6955, 0xe7, 0x0af4},
	{0x7b2f, 0x70, 0x6955, 0x70, 0x1179},

	/* Return to Eden /SD (PC) */
	{0x7d14, 0xe8, 0xfbab, 0x5f, 0xeab9},
	{0x7d14, 0xe8, 0xfbab, 0xe8, 0xe216},

	/* Return to Eden /SD (Amstrad CPC) */
	{0x7cff, 0xf8, 0x6044, 0x6f, 0xbb57},

	/* Return to Eden /SD (Spectrum128) */
	{0x7c55, 0x18, 0xdaee, 0x8f, 0x01fd},

	/* Worm in Paradise /SD (Amstrad CPC/PC/Spectrum128) */
	{0x7cd7, 0x0e, 0x4feb, 0x85, 0x4eae},
	{0x7cd7, 0x0e, 0x4feb, 0x0e, 0xb02c},

	/* Snowball /SD (Commodore 64) */
	{0x7363, 0x65, 0xa0ab, 0xdc, 0xca6a},

	/* Return to Eden /SD (Commodore 64) */
	{0x772f, 0xca, 0x8602, 0x41, 0x9bd0},

	/* Worm in Paradise /SD (Commodore 64) */
	{0x788d, 0x72, 0x888a, 0xe9, 0x4cce},

	/* Snowball /SD (Atari) */
	{0x6bf8, 0x3f, 0xc9f7, 0x96, 0x1908},

	/* Return to Eden /SD (Atari) */
	{0x60f7, 0x68, 0xc2bc, 0xdf, 0xd3ae},

	/* Worm in Paradise /SD (Atari) */
	{0x6161, 0xf3, 0xe6d7, 0x6a, 0xe232},

	/* Snowball /SD (Spectrum48) */
	{0x6541, 0x02, 0x2e6c, 0x79, 0xb80c},
	{0x6541, 0x02, 0x2e6c, 0x02, 0x028a},

	/* Return to Eden /SD (Spectrum48) */
	{0x5f43, 0xca, 0x828c, 0x41, 0x9f5e},
	{0x5f43, 0xca, 0x828c, 0xca, 0x6e1b},

	/* Worm in Paradise /SD (Spectrum48) */
	{0x5ebb, 0xf1, 0x4dec, 0x68, 0x4909},
	{0x5ebb, 0xf1, 0x4dec, 0xf1, 0xcc1a},

	/* Knight Orc, pt. 1 (Amiga) */
	{0xbb93, 0x36, 0x6a05, 0xad, 0xe52d},

	/* Knight Orc, pt. 1 (ST) */
	{0xbb6e, 0xad, 0x4d40, 0x24, 0x3bcd},

	/* Knight Orc, pt. 2 (Amiga/ST) */
	{0xc58e, 0x4a, 0x4e9d, 0xc1, 0xe2bf},

	/* Knight Orc, pt. 3 (Amiga/ST) */
	{0xcb9a, 0x0f, 0x0804, 0x86, 0x6487},

	/* Knight Orc, pt. 1 (PC) */
	{0xbb6e, 0xa6, 0x9753, 0x1d, 0x2e7f},
	{0xbb6e, 0xa6, 0x9753, 0xa6, 0x001d},

	/* Knight Orc, pt. 2 (PC) */
	{0xc58e, 0x43, 0xe9ce, 0xba, 0x5e4c},
	{0xc58e, 0x43, 0xe9ce, 0x43, 0xa8f0},

	/* Knight Orc, pt. 3 (PC) */
	{0xcb9a, 0x08, 0x6c36, 0x7f, 0xf0d4},
	{0xcb9a, 0x08, 0x6c36, 0x08, 0x2d08},

	/* Knight Orc, pt. 1 (Commodore 64 Gfx) */
	{0x8970, 0x6b, 0x3c7b, 0xe2, 0xb6f3},

	/* Knight Orc, pt. 1 (Spectrum48) */
	{0x86d0, 0xb7, 0xadbd, 0x2e, 0x43e1},

	/* Gnome Ranger, pt. 1 (Amiga/ST) */
	{0xb1a9, 0x80, 0x5fb7, 0xf7, 0x5c6c},

	/* Gnome Ranger, pt. 2 (Amiga/ST) */
	{0xab9d, 0x31, 0xbe6d, 0xa8, 0xcb96},

	/* Gnome Ranger, pt. 3 (Amiga/ST) */
	{0xae28, 0x87, 0xb6b6, 0xfe, 0x760c},

	/* Gnome Ranger, pt. 1 (PC) */
	{0xb1aa, 0xad, 0xaf47, 0x24, 0x5cfd},
	{0xb1aa, 0xad, 0xaf47, 0xad, 0xe0ed},

	/* Gnome Ranger, pt. 1 (ST-var) */
	{0xb19e, 0x92, 0x8f96, 0x09, 0x798c},

	/* Gnome Ranger, pt. 2 (PC/ST-var) */
	{0xab8b, 0xbf, 0x31e6, 0x36, 0x811c},
	{0xab8b, 0xbf, 0x31e6, 0xbf, 0x8ff3},

	/* Gnome Ranger, pt. 3 (PC/ST-var) */
	{0xae16, 0x81, 0x8741, 0xf8, 0x47fb},
	{0xae16, 0x81, 0x8741, 0x81, 0xc8eb},

	/* Gnome Ranger, pt. 1 (Commodore 64 TO) */
	{0xad41, 0xa8, 0x42c5, 0x1f, 0x7d1e},

	/* Gnome Ranger, pt. 2 (Commodore 64 TO) */
	{0xa735, 0xf7, 0x2e08, 0x6e, 0x780e},

	/* Gnome Ranger, pt. 3 (Commodore 64 TO) */
	{0xa9c0, 0x9e, 0x0d70, 0x15, 0x3e6b},

	/* Gnome Ranger, pt. 1 (Commodore 64 Gfx) */
	{0x908e, 0x0d, 0x58a7, 0x84, 0xab1d},

	/* Gnome Ranger, pt. 2 (Commodore 64 Gfx) */
	{0x8f6f, 0x0a, 0x411a, 0x81, 0x12bc},

	/* Gnome Ranger, pt. 3 (Commodore 64 Gfx) */
	{0x9060, 0xbb, 0xe75d, 0x32, 0x14e7},

	/* Lords of Time /T&M (PC) */
	{0xb57c, 0x44, 0x7779, 0xbb, 0x31a6},
	{0xb57c, 0x44, 0x7779, 0x44, 0xea72},

	/* Red Moon /T&M (PC) */
	{0xa69e, 0x6c, 0xb268, 0xe3, 0x4cef},
	{0xa69e, 0x6c, 0xb268, 0x6c, 0x3799},

	/* Price of Magik /T&M (PC) */
	{0xbac7, 0x7f, 0xddb2, 0xf6, 0x6ab3},
	{0xbac7, 0x7f, 0xddb2, 0x7f, 0x905c},

	/* Lords of Time /T&M (ST) */
	{0xb579, 0x89, 0x3e89, 0x00, 0xa2b7},

	/* Red Moon /T&M (ST) */
	{0xa698, 0x41, 0xcaca, 0xb8, 0xeeac},

	/* Price of Magik /T&M (ST) */
	{0xbac4, 0x80, 0xa750, 0xf7, 0xe030},

	/* Lords of Time /T&M (Amiga) */
	{0xb576, 0x2a, 0x7239, 0xa1, 0x2ea6},

	/* Red Moon /T&M (Amiga) */
	{0xa692, 0xd1, 0x6a99, 0x48, 0x50ff},

	/* Price of Magik /T&M (Amiga) */
	{0xbaca, 0x3a, 0x221b, 0xb1, 0x55bb},

	/* Lords of Time /T&M (Commodore 64 TO) */
	{0xb38c, 0x37, 0x9f8e, 0xae, 0xc6b1},

	/* Red Moon /T&M (Commodore 64 TO) */
	{0xa4e2, 0xa6, 0x016d, 0x1d, 0x31ab},

	/* Price of Magik /T&M (Commodore 64 TO) */
	{0xb451, 0xa8, 0x2682, 0x1f, 0x5de2},

	/* Lords of Time /T&M (Commodore 64 Gfx) */
	{0x9070, 0x43, 0x45d4, 0xba, 0x02eb},

	/* Red Moon /T&M (Commodore 64 Gfx) */
	{0x903f, 0x6b, 0x603e, 0xe2, 0x9f59},

	/* Price of Magik /T&M (Commodore 64 Gfx) */
	{0x8f51, 0xb2, 0x6c9a, 0x29, 0xde3b},

	/* Lords of Time /T&M (Spectrum48) */
	{0x8950, 0xa1, 0xbb16, 0x18, 0x2828},
	{0x8950, 0xa1, 0xbb16, 0xa1, 0x1ea2},

	/* Red Moon /T&M (Spectrum48) */
	{0x8813, 0x11, 0x22de, 0x88, 0x18b8},
	{0x8813, 0x11, 0x22de, 0x11, 0xd0cd},

	/* Price of Magik /T&M (Spectrum48) */
	{0x8a60, 0x2a, 0x29ed, 0xa1, 0x5e4d},

	/* Lancelot, pt. 1 (Amiga/PC/ST) */
	{0xc0cf, 0x4e, 0xb7fa, 0xc5, 0x4400},

	/* Lancelot, pt. 2 (Amiga/PC/ST) */
	{0xd5e9, 0x6a, 0x4192, 0xe1, 0x3b1e},

	/* Lancelot, pt. 3 (Amiga/PC/ST) */
	{0xbb8f, 0x1a, 0x7487, 0x91, 0x877d},

	/* Lancelot, pt. 1 (Commodore 64 TO) */
	{0xb4c9, 0x94, 0xd784, 0x0b, 0x203e},

	/* Lancelot, pt. 2 (Commodore 64 TO) */
	{0xb729, 0x51, 0x8ee5, 0xc8, 0xf1c9},

	/* Lancelot, pt. 3 (Commodore 64 TO) */
	{0xb702, 0xe4, 0x1809, 0x5b, 0x25b2},

	/* Lancelot, pt. 1 (Commodore 64 Gfx) */
	{0x8feb, 0xba, 0xa800, 0x31, 0x5bfa},

	/* Lancelot, pt. 2 (Commodore 64 Gfx) */
	{0x8f6b, 0xfa, 0x0f7e, 0x71, 0x75f2},

	/* Lancelot, pt. 3 (Commodore 64 Gfx) */
	{0x8f71, 0x2f, 0x0ddc, 0xa6, 0x3e87},

	/* Ingrid's Back, pt. 1 (PC) */
	{0xd19b, 0xad, 0x306d, 0x24, 0x4504},
	{0xd19b, 0xad, 0x306d, 0xad, 0x878e},

	/* Ingrid's Back, pt. 2 (PC) */
	{0xc5a5, 0xfe, 0x3c98, 0x75, 0x8950},
	{0xc5a5, 0xfe, 0x3c98, 0xfe, 0x8b7b},

	/* Ingrid's Back, pt. 3 (PC) */
	{0xd7ae, 0x9e, 0x1878, 0x15, 0xadb0},
	{0xd7ae, 0x9e, 0x1878, 0x9e, 0xaf9b},

	/* Ingrid's Back, pt. 1 (Amiga) */
	{0xd188, 0x13, 0xdc60, 0x8a, 0x755c},

	/* Ingrid's Back, pt. 2 (Amiga) */
	{0xc594, 0x03, 0xea95, 0x7a, 0xb5a8},

	/* Ingrid's Back, pt. 3 (Amiga) */
	{0xd79f, 0xb5, 0x1661, 0x2c, 0xbf5d},

	/* Ingrid's Back, pt. 1 (ST) */
	{0xd183, 0x83, 0xef72, 0xfa, 0xb04f},

	/* Ingrid's Back, pt. 2 (ST) */
	{0xc58f, 0x65, 0xf337, 0xdc, 0x900a},

	/* Ingrid's Back, pt. 3 (ST) */
	{0xd79a, 0x57, 0x49c5, 0xce, 0xe0f9},

	/* Ingrid's Back, pt. 1 (Commodore 64 TO) */
	{0xb770, 0x03, 0x9a03, 0x7a, 0xdc6a},

	/* Ingrid's Back, pt. 2 (Commodore 64 TO) */
	{0xb741, 0xb6, 0x2aa5, 0x2d, 0x5a6c},

	/* Ingrid's Back, pt. 3 (Commodore 64 TO) */
	{0xb791, 0xa1, 0xd065, 0x18, 0xaa0c},

	/* Ingrid's Back, pt. 1 (Commodore 64 Gfx) */
	{0x9089, 0xce, 0xc5e2, 0x44, 0xeff4},

	/* Ingrid's Back, pt. 2 (Commodore 64 Gfx) */
	{0x908d, 0x80, 0x30c7, 0xf6, 0x2a11},

	/* Ingrid's Back, pt. 3 (Commodore 64 Gfx) */
	{0x909e, 0x9f, 0xdecc, 0x15, 0xf4da},

	{0x0000, 0x00, 0x0000, 0x00, 0x0000},
};

const L9V1GameInfo L9_V1_GAMES[] = {
	{ 0x1a, 0x24, 301, { 0x0000, -0x004b, 0x0080, -0x002b, 0x00d0 }, 0x03b0, 0x0f80, 0x4857 }, /* Colossal Adventure */
	{ 0x20, 0x3b, 283, { -0x0583, 0x0000, -0x0508, -0x04e0, 0x0000 }, 0x0800, 0x1000, 0x39d1 }, /* Adventure Quest */
	{ 0x14, 0xff, 153, { -0x00d6, 0x0000, 0x0000, 0x0000, 0x0000 }, 0x0a20, 0x16bf, 0x420d }, /* Dungeon Adventure */
	{ 0x15, 0x5d, 252, { -0x3e70, 0x0000, -0x3d30, -0x3ca0, 0x0100 }, 0x4120, -0x3b9d, 0x3988 }, /* Lords of Time */
	{ 0x15, 0x6c, 284, { -0x00f0, 0x0000, -0x0050, -0x0050, -0x0050 }, 0x0300, 0x1930, 0x3c17 } /* Snowball */
};

const GlkDetectionEntry LEVEL9_GAMES[] = {
	DT_END_MARKER
};

} // End of namespace Level9
} // End of namespace Glk
