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

#pragma once
#include "eden.h"

Follower followerList[15] = {
//            char,                 X,  sx, sy,  ex,  ey,bank,
	{ PersonId::pidGregor,          5, 211,  9, 320, 176, 228,   0,  0 },
	{ PersonId::pidEloi,            4, 162, 47, 223, 176, 228, 112, 78 },
	{ PersonId::pidDina,            3,  55,  0, 172, 176, 228,  90, 16 },
	{ PersonId::pidChongOfChamaar,  4,   0,  5, 114, 176, 229,   0, 16 },
	{ PersonId::pidKommalaOfKoto,   3,   0, 15, 102, 176, 229,   0, 16 },
	{ PersonId::pidUlanOfUlele,     1,   0,  0, 129, 176, 230,   0, 16 },
	{ PersonId::pidCabukaOfCantura, 2,   0,  0, 142, 176, 230,   0, 16 },
	{ PersonId::pidFuggOfTamara,    0,   0, 17, 102, 176, 230,   0, 16 },
	{ PersonId::pidJabber,          2,   0,  6, 134, 176, 228,   0, 16 },
	{ PersonId::pidShazia,          1,  90, 17, 170, 176, 228,  50, 22 },
	{ PersonId::pidThugg,           0, 489,  8, 640, 176, 228, 160, 24 },
	{ PersonId::pidMungo,           5, 361,  0, 517, 176, 229,   0, 16 },
	{ PersonId::pidMonk,            0, 419, 22, 569, 176, 229, 100, 30 },
	{ PersonId::pidEve,             1, 300, 28, 428, 176, 229,   0, 38 },
	{ -1,                          -1,  -1, -1,  -1,  -1,  -1,  -1, -1 }
};

byte kLabyrinthPath[70] = {
// each nibble tells which direction to choose to exit the labyrinth
	0x11, 0x11, 0x11, 0x22, 0x33, 0x55, 0x25, 0x44, 0x25, 0x11, 0x11, 0x11,
	0x11, 0x35, 0x55, 0x45, 0x45, 0x44, 0x44, 0x34, 0x44, 0x34, 0x32, 0x52,
	0x33, 0x23, 0x24, 0x44, 0x24, 0x22, 0x54, 0x22, 0x54, 0x54, 0x44, 0x22,
	0x22, 0x42, 0x45, 0x22, 0x42, 0x45, 0x35, 0x11, 0x44, 0x34, 0x52, 0x11,
	0x44, 0x32, 0x55, 0x11, 0x11, 0x33, 0x11, 0x11, 0x53, 0x11, 0x11, 0x53,
	0x54, 0x24, 0x11, 0x22, 0x25, 0x33, 0x53, 0x54, 0x23, 0x44
};

char kDinoSpeedForCitaLevel[16] = { 1, 2, 3, 4, 4, 5, 6, 7, 8, 9 };

char kTabletView[12] = {          //TODO: make as struct?
	// opposite tablet id, video id
	Objects::obUnused10, 83,
	Objects::obUnused10, 84,
	Objects::obTablet4, 85,
	Objects::obTablet3, 86,
	Objects::obTablet6, 87,
	Objects::obTablet5, 85
};

// special character backgrounds for specific rooms
char kPersoRoomBankTable[84] = {
	// first entry is default bank, then pairs of [roomNum, bankNum], terminated by -1
	0,  3, 33, -1,
	21, 17, 35, -1,
	0,  2, 36, -1,
	22,  9, 38,  3, 39, -1,
	23,  8, 40, -1,
	0,  3, 41,  7, 42, -1,
	25, -1,
	27, 17, 45, -1,
	28, 26, 46, -1,
	29, 51, 48, -1,
	30, 53, 49, -1,
	0, 27, 50, -1,
	32, 17, 51, -1,
	52,  2, 52, -1,
	-3,  3, -3, -1,
	31, -1,
	24,  6, 43, -1,
	47, -1,
	0,  2, 64, -1,
	54,  3, 54, -1,
	27, -1,
	26, 17, 45, -1
};

// area transition descriptors
Goto gotos[130] = {
// area, oldarea, vid, time, valleyVid
	{  0,  1,   0,  2,  20 },
	{  0,  1, 162,  3, 168 },
	{  0,  2,   0,  2,  21 },
	{  0,  6,   0,  3, 108 },
	{  0,  9, 151,  3,   0 },
	{  0,  7, 106,  2, 101 },
	{  0, 10,  79,  3, 102 },
	{  0, 12,   0,  3,   0 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  1,  3,  58,  2, 104 },
	{  1,  4, 100,  4, 104 },
	{  1,  5, 107,  6, 104 },
	{  1,  6, 155,  8, 104 },
	{  1,  7, 165,  6, 104 },
	{  1,  8, 169,  6, 104 },
	{  1, 10, 111,  2, 104 },
	{  1, 11, 164,  4, 104 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  1,  3, 161,  3, 102 },
	{  1,  4, 163,  6, 102 },
	{  1,  5, 157,  9, 102 },
	{  1,  9, 160,  9, 102 },
	{  1, 10,  79,  3, 102 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  1,  3,   0,  3, 153 },			// 24
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  3,  1, 154,  2, 103 },
	{  3,  4, 100,  2, 103 },
	{  3,  5, 107,  4, 103 },
	{  3,  6, 155,  6, 103 },
	{  3,  7, 165,  8, 103 },
	{  3,  8, 169,  6, 103 },
	{  3, 10, 111,  4, 103 },
	{  3, 11, 164,  6, 103 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  3,  1, 162,  3,  22 },
	{  3,  4, 163,  6,  22 },
	{  3,  5, 157,  9,  22 },
	{  3,  9, 160,  9,  22 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  3,  1,   0,  3, 166 },			// 40
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  4,  1, 154,  4,  51 },
	{  4,  3,  58,  2,  51 },
	{  4,  5, 107,  2,  51 },
	{  4,  6, 155,  4,  51 },
	{  4,  7, 165,  6,  51 },
	{  4,  8, 169,  8,  51 },
	{  4, 10, 111,  6,  51 },
	{  4, 11, 164,  8,  51 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  4,  1, 162,  3, 109 },			// 51
	{  4,  3, 161,  6, 109 },
	{  4,  5, 157,  9, 109 },
	{  4,  9, 160,  9, 109 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  5,  1, 154,  6,  33 },
	{  5,  3,  58,  4,  33 },
	{  5,  4, 100,  2,  33 },
	{  5,  6, 155,  2,  33 },
	{  5,  7, 165,  4,  33 },
	{  5,  8, 169,  8,  33 },
	{  5, 10, 111,  8,  33 },
	{  5, 11, 164,  8,  33 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  5,  1, 162,  3,  99 },			// 65
	{  5,  3, 161,  6,  99 },
	{  5,  4, 163,  9,  99 },
	{  5,  9, 160,  9,  99 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  9,  1, 162,  3, 167 },			// 70
	{  9,  3, 161,  6, 167 },
	{  9,  4, 163,  9, 167 },
	{  9,  5, 157,  9, 167 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  6,  1, 154,  8, 105 },			// 75
	{  6,  3,  58,  6, 105 },
	{  6,  4, 100,  4, 105 },
	{  6,  5, 107,  2, 105 },
	{  6,  7, 165,  2, 105 },
	{  6,  8, 169, 10, 105 },
	{  6, 10, 111,  6, 105 },
	{  6, 11, 164,  8, 105 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  7,  1, 154,  4, 158 },			// 84
	{  7,  3,  58,  6, 158 },
	{  7,  4, 100,  6, 158 },
	{  7,  5, 107,  4, 158 },
	{  7,  6, 155,  2, 158 },
	{  7,  8, 169,  8, 158 },
	{  7, 10, 111,  4, 158 },
	{  7, 11, 164,  6, 158 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{  8,  1, 154,  2, 159 },			// 93
	{  8,  3,  58,  4, 159 },
	{  8,  4, 100,  6, 159 },
	{  8,  5, 107,  8, 159 },
	{  8,  6, 155, 10, 159 },
	{  8,  7, 165,  8, 159 },
	{  8, 10, 111,  6, 159 },
	{  8, 11, 164,  4, 159 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 10,  1, 154,  2,  77 },			// 102
	{ 10,  3,  58,  4,  77 },
	{ 10,  4, 100,  6,  77 },
	{ 10,  5, 107,  8,  77 },
	{ 10,  6, 155,  6,  77 },
	{ 10,  7, 165,  4,  77 },
	{ 10,  8, 169,  6,  77 },
	{ 10, 11, 164,  4,  77 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 11,  1, 154,  2,  80 },			// 111
	{ 11,  3,  58,  4,  80 },
	{ 11,  4, 100,  6,  80 },
	{ 11,  5, 107,  8,  80 },
	{ 11,  6, 155,  8,  80 },
	{ 11,  7, 165,  6,  80 },
	{ 11,  8, 169,  2,  80 },
	{ 11, 10, 111,  4,  80 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 12,  1, 154,  8,  56 },			// 120
	{ 12,  3,  58,  4,  56 },
	{ 12,  4, 100,  4,  56 },
	{ 12,  5, 107,  6,  56 },
	{ 12,  6, 155,  8,  56 },
	{ 12,  7, 165, 10,  56 },
	{ 12,  8, 169,  4,  56 },
	{ 12, 10, 111, 10,  56 },
	{ 12, 11, 164,  6,  56 },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
};

object_t _objects[42] = {
	//id,fl,loc,masklow,maskhi,ct
	{  1, 0,  3,      1,     0, 0},     // Eve's Way Stone
	{  2, 0,  3,      2,     0, 0},     // Thau's Seashell
	{  3, 0,  3,      4,     0, 0},     // Talisman of bravery
	{  4, 0,  3,      8,     0, 0},     // An old tooth. Very old! Whoever lost it most certainly has no further use for it!
	{  5, 0,  0,   0x10,     0, 0},     // Prism
	{  6, 0,  3,      0,     0, 0},     // Flute
	{  7, 0,  3, 0x4000,     0, 0},     // Apple
	{  8, 0,  4, 0x1000,     0, 0},     // Egg of Destiny
	{  9, 0,  3,  0x800,     0, 0},     // Root
	{ 10, 0,  3,      0,     0, 0},     // ???
	{ 11, 0,  6,      0,     0, 0},     // Mushroom
	{ 12, 0, 13,      0,     0, 0},     // Poisonous Mushroom
	{ 13, 0,  2,  0x400,     0, 0},     // Graa's Knife
	{ 14, 0, 22,      0,     0, 0},     // Empty Nest
	{ 15, 0, 26,      0,     0, 0},     // Full Nest
	{ 16, 0, 33,   0x20,     0, 0},     // Gold
	{ 17, 0,  3,      0,     0, 0},     // Sign of Shadow Mistress  (moon stone)
	{ 18, 0,  3,      0,     0, 0},     // Sign of Mother of all    (bag of soil)
	{ 19, 0, 40,      0,     0, 0},     // Sign of the life-giving  (sun star)
	{ 20, 0, 20,  0x200,     0, 0},     // King's Horn
	{ 21, 0,  3,      0,     0, 0},     // Golden Sword of Mashaar
	// Masks
	{ 22, 0,  3,   0x40,     0, 0},     // Mask of Death
	{ 23, 0,  3,   0x80,     0, 0},     // Mask of Bonding
	{ 24, 0,  3,  0x100,     0, 0},     // Mask of Birth
	// Objects of power
	{ 25, 0,  3,      0,     1, 0},     // Eye in the Storm
	{ 26, 0,  3,      0,     2, 0},     // Sky Hammer
	{ 27, 0,  3,      0,     4, 0},     // Fire in the Clouds
	{ 28, 0,  3,      0,     8, 0},     // Within and Without
	{ 29, 0,  3,      0,  0x10, 0},     // Eye in the Cyclone
	{ 30, 0,  3,      0,  0x20, 0},     // River that Winds
	// Musical instruments
	{ 31, 0,  3,      0,  0x40, 0},     // Trumpet
	{ 32, 0,  3,      0,  0x80, 0},     // -- unused (but still has a dialog line)
	{ 33, 0,  3,      0, 0x100, 0},     // Drum
	{ 34, 0,  3,      0, 0x200, 0},     // -- unused (but still has a dialog line)
	{ 35, 0,  3,      0, 0x400, 0},     // -- unused (but still has a dialog line)
	{ 36, 0,  3,      0, 0x800, 0},     // Ring
	// Tablets
	{ 37, 0,  3,      0,     0, 0},     // Tablet #1 (Mo)
	{ 38, 0, 42, 0x2000,     0, 0},     // Tablet #2 (Morkus' Lair)
	{ 39, 0,  3,      0,     0, 0},     // Tablet #3 (White Arch?)
	{ 40, 0,  3,      0,     0, 0},     // Tablet #4
	{ 41, 0,  3,      0,     0, 0},     // Tablet #5
	{ 42, 0,  3, 0x8000,     0, 0}      // Tablet #6 (Castra)
};

uint16 kObjectLocations[45] = {
	0x112, 0xFFFF,
	0x202, 0xFFFF,
	0x120, 0xFFFF,
	0x340, 0x44B, 0x548, 0x640, 0x717, 0x830, 0xFFFF,
	0x340, 0x44B, 0x548, 0x640, 0x717, 0x830, 0xFFFF,
	0, 0xFFFF,
	0x344, 0x53A, 0x831, 0xFFFF,
	0x331, 0x420, 0x54B, 0x637, 0x716, 0x840, 0xFFFF,
	0x834A, 0x8430, 0x8531, 0x644, 0x745, 0x838, 0xFFFF,
	0x510, 0xFFFF,
	0xC04, 0xFFFF,
	0xFFFF
};

perso_t kPersons[58] = {
	// room, aid, party mask,                            id,                                            flags,  X,bank,X, X,sprId,sprX,speed, X
	{ 0x103, 230, PersonMask::pmGregor, PersonId::pidGregor            ,                                                0,  0,  1, 0, 0,  0,   0, 0, 0 },
	{ 0x116, 231, PersonMask::pmDina  , PersonId::pidDina              ,                                                0,  4,  2, 0, 0,  3,   9, 0, 0 },
	{ 0x202, 232, PersonMask::pmTau   , PersonId::pidTau               ,                                                0,  8,  3, 0, 0,  0,   0, 0, 0 },
	{ 0x109, 233, PersonMask::pmMonk  , PersonId::pidMonk              ,                                                0, 12,  4, 0, 0,  6,  52, 0, 0 },
	{ 0x108, 234, PersonMask::pmJabber, PersonId::pidJabber            ,                                                0, 18,  5, 0, 0,  2,   0, 0, 0 },
	{ 0x103, 235, PersonMask::pmEloi  , PersonId::pidEloi              ,                                                0, 22,  6, 0, 0,  4,  20, 0, 0 },
	{ 0x301, 236, PersonMask::pmMungo , PersonId::pidMungo             ,                                                0, 28,  8, 0, 0, 11,  45, 0, 0 },
	{ 0x628, 237, PersonMask::pmEve   , PersonId::pidEve               ,                                                0, 30, 10, 0, 0,  7,  35, 0, 0 },
	{ 0x81A, 238, PersonMask::pmShazia, PersonId::pidShazia            ,                                                0, 34, 11, 0, 0,  1,  11, 0, 0 },
	{ 0x330, 239, PersonMask::pmLeader, PersonId::pidChongOfChamaar    ,                                                0, 38, 13, 0, 0, 10,   0, 0, 0 },
	{ 0x41B, 239, PersonMask::pmLeader, PersonId::pidUlanOfUlele       ,                                                0, 46, 15, 0, 0, 13,   0, 0, 0 },
	{ 0x53B, 239, PersonMask::pmLeader, PersonId::pidKommalaOfKoto     ,                                                0, 42, 14, 0, 0,  9,   0, 0, 0 },
	{ 0x711, 239, PersonMask::pmLeader, PersonId::pidCabukaOfCantura   ,                                                0, 50, 16, 0, 0, 14,   0, 0, 0 },
	{ 0xA02, 239, PersonMask::pmLeader, PersonId::pidMarindaOfEmbalmers,                                                0, 54, 17, 0, 0,  0,   0, 0, 0 },
	{ 0x628, 239, PersonMask::pmLeader, PersonId::pidFuggOfTamara      ,                                                0, 62, 18, 0, 0, 12,   0, 0, 0 },
	{ 0x801, 239, PersonMask::pmLeader, PersonId::pidChongOfChamaar    ,                                                0, 38, 13, 0, 0, 10,   0, 0, 0 },
	{ 0x41B,  10, PersonMask::pmQuest , PersonId::pidUlanOfUlele       , PersonFlags::pfType2                            , 46, 15, 0, 0, 13,   0, 0, 0 },
	{ 0x711,  11, PersonMask::pmQuest , PersonId::pidCabukaOfCantura   , PersonFlags::pfType2                            , 50, 16, 0, 0, 14,   0, 0, 0 },
	{ 0x106, 240, PersonMask::pmThugg , PersonId::pidThugg             ,                                                0, 64,  7, 0, 0,  0,  61, 0, 0 },
	{     0,  13,                    0, PersonId::pidNarrator          ,                                                0, 68, 12, 0, 0,  0,   0, 0, 0 },
	{ 0x902, 241, PersonMask::pmQuest , PersonId::pidNarrim            ,                                                0, 70, 19, 0, 0,  0,   0, 0, 0 },
	{ 0xC03, 244, PersonMask::pmMorkus, PersonId::pidMorkus            ,                                                0, 74, 20, 0, 0,  0,   0, 0, 0 },
	// dinos in each valley
	{ 0x332, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x329, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x33B, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x317, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x320, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType12                           ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x349, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x429, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x43B, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x422, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x432, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x522, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x534, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x515, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pftVelociraptor                    ,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x533, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x622, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x630, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x643, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x63A, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x737, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x739, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x74A, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x726, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x842, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pfType8        ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x822, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x828, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pftVelociraptor                    ,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x84B, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0xB03, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            , 58, 252, 0, 0,  0,   0, 0, 0 },
	// enemy dinos
	{ 0x311, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x410, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x51B, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x618, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x71B, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x81B, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0xFFFF, 0xFFFF, 0xFFFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFFFF, 0xFFFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{ 0x628, 237, PersonMask::pmEve   , PersonId::pidEve               ,                                                0, 80,  9, 0, 0,  8,  35, 0, 0 },
	{ 0x628, 237, PersonMask::pmEve   , PersonId::pidEve               ,                                                0, 78, 10, 0, 0,  7,  35, 0, 0 }
};

Citadel _citadelList[7] = {
	{   1, { 163, 182, 0, 0, 124, 147, 193, 0 }, {   0,   0, 0, 0,   0,   0,   0, 0 } },
	{  48, { 285, 286, 0, 0, 287, 288, 284, 0 }, { 114, 115, 0, 0, 116, 117, 113, 0 } },
	{  63, { 290, 291, 0, 0, 292, 293, 289, 0 }, { 119, 120, 0, 0, 121, 122, 118, 0 } },
	{  95, { 295, 296, 0, 0, 297, 298, 294, 0 }, { 124, 125, 0, 0, 126, 127, 123, 0 } },
	{ 127, { 300, 301, 0, 0, 302, 303, 299, 0 }, { 129, 130, 0, 0, 131, 132, 128, 0 } },
	{ 159, { 305, 306, 0, 0, 307, 308, 304, 0 }, { 134, 135, 0, 0, 136, 137, 133, 0 } },
	{ 255, { 310, 311, 0, 0, 312, 313, 309, 0 }, { 139, 140, 0, 0, 141, 142, 138, 0 } }
};

Rect _characterRects[19] = {   // TODO: just an array of int16s?
	{  93,  69, 223, 176},
	{ 102,  86, 162, 126},
	{  88, 103, 168, 163},
	{ 116,  66, 192, 176},
	{ 129,  92, 202, 153},
	{  60,  95, 160, 176},
	{ 155,  97, 230, 145},
	{ 100,  77, 156, 145},
	{ 110,  78, 170, 156},
	{  84,  76, 166, 162},
	{  57,  77, 125, 114},
	{  93,  69, 223, 175},
	{  93,  69, 223, 176},
	{  93,  69, 223, 176},
	{ 154,  54, 245, 138},
	{ 200,  50, 261, 116},
	{  70,  84, 162, 176},
	{ 125, 101, 222, 172},
	{ 188,  83, 251, 158}
};

byte _characterArray[20][5] = {   // TODO: struc?
	{    8, 15,   23, 25, 0xFF },
	{    0,  9, 0xFF,  0,    0 },
	{    0,  9, 0xFF,  0,    0 },
	{    0,  9, 0xFF,  0,    0 },
	{    0, 13, 0xFF,  0,    0 },
	{   16, 21, 0xFF,  0,    0 },
	{   11, 20, 0xFF,  0,    0 },
	{    0, 12, 0xFF,  0,    0 },
	{    0,  9, 0xFF,  0,    0 },
	{    0,  9, 0xFF,  0,    0 },
	{    5, 13, 0xFF,  0,    0 },
	{ 0xFF,  0,    0,  0,    0 },
	{    0,  8, 0xFF,  0,    0 },
	{ 0xFF,  0,    0,  0,    0 },
	{    0,  7, 0xFF,  0,    0 },
	{    0,  8, 0xFF,  0,    0 },
	{    8, 12, 0xFF,  0,    0 },
	{    0,  5, 0xFF,  0,    0 },
	{    0,  4, 0xFF,  0,    0 },
	{ 0xFF,  0,    0,  0,    0 }
};

Area kAreasTable[12] = {
	{ Areas::arMo           , AreaType::atCitadel,                           0,   0, 0,  1, 0, 0},
	{ Areas::arTausCave     , AreaType::atCave   ,                           0, 112, 0,  2, 0, 0},
	{ Areas::arChamaar      , AreaType::atValley ,                           0, 133, 0,  3, 0, 0},
	{ Areas::arUluru        , AreaType::atValley ,                           0, 187, 0,  4, 0, 0},
	{ Areas::arKoto         , AreaType::atValley , AreaFlags::HasVelociraptors, 236, 0,  5, 0, 0},
	{ Areas::arTamara       , AreaType::atValley ,                           0, 288, 0,  6, 0, 0},
	{ Areas::arCantura      , AreaType::atValley ,                           0, 334, 0,  7, 0, 0},
	{ Areas::arShandovra    , AreaType::atValley ,                           0, 371, 0,  8, 0, 0},
	{ Areas::arNarimsCave   , AreaType::atCave   ,                           0, 115, 0,  9, 0, 0},
	{ Areas::arEmbalmersCave, AreaType::atCave   ,                           0, 118, 0, 10, 0, 0},
	{ Areas::arWhiteArch    , AreaType::atCave   ,                           0, 122, 0, 11, 0, 0},
	{ Areas::arMoorkusLair  , AreaType::atCave   ,                           0, 127, 0, 12, 0, 0}
};

int16 tab_2CEF0[64] = {
	25, 257,  0,   0, 37, 258, 38, 259,  0,   0, 24, 260, 0, 0, 0, 0,
	0,   0, 53, 265,  0,   0,  0,   0,  0,   0,  0,   0, 0, 0, 0, 0,
	39, 261,  0,   0, 40, 262, 62, 263,  0,   0, 63, 264, 0, 0, 0, 0,
	18, 275,  0,   0, 35, 254, 36, 255, 19, 318, 23, 256, 0, 0, 0, 0
};

int16 tab_2CF70[64] = {
	65, 266,  0,   0, 66, 267, 67, 268,  0,   0, 68, 269, 0, 0, 0, 0,
	0,   0, 73, 274,  0,   0,  0,   0,  0,   0,  0,   0, 0, 0, 0, 0,
	69, 270,  0,   0, 70, 271, 71, 272,  0,   0, 72, 273, 0, 0, 0, 0,
	18, 275,  0,   0, 35, 254, 36, 255, 19, 318, 23, 256, 0, 0, 0, 0,
};

byte kActionCursors[299] = {
	3, 1, 2, 4, 5, 5, 5, 0, 5, 5,
	5, 5, 5, 3, 2, 5, 5, 5, 3, 2,
	4, 5, 7, 7, 4, 5, 5, 0, 0, 0,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 0, 0, 0, 0, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 0, 0,
	0, 0, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 0, 5, 6,
	6, 1, 6, 6, 0, 0, 6, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 6, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 0, 0, 6, 6,
	53, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0
};

byte mapMode[12] = { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 2, 0 };

// Cube faces to texture coords mapping
// each entry is num_polys(6) * num_faces_per_poly(2) * vertex_per_face(3) * uv(2)
byte cubeTextureCoords[3][6 * 2 * 3 * 2] = {
	{
		32, 32,  0, 32,  0,  0,
		32, 32,  0,  0, 32,  0,

		0, 32,  0,  0, 32,  0,
		0, 32, 32,  0, 32, 32,

		32, 32,  0, 32,  0,  0,
		32, 32,  0,  0, 32,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		0,  0, 32,  0, 32, 32,
		0,  0, 32, 32,  0, 32,

		0, 32,  0,  0, 32,  0,
		0, 32, 32,  0, 32, 32
	}, {
		32, 32,  0, 32,  0,  0,
		32, 32,  0,  0, 32,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		0, 32,  0,  0, 32,  0,
		0, 32, 32,  0, 32, 32,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0
	}, {
		30, 30,  2, 30,  2,  2,
		30, 30,  2,  2, 30,  2,

		2, 30,  2,  2, 30,  2,
		2, 30, 30,  2, 30, 30,

		30, 30,  2, 30,  2,  2,
		30, 30,  2,  2, 30,  2,

		30,  2, 30, 30,  2, 30,
		30,  2,  2, 30,  2,  2,

		2,  2, 30,  2, 30, 30,
		2,  2, 30, 30,  2, 30,

		2, 30,  2,  2, 30,  2,
		2, 30, 30,  2, 30, 30
	}
};
