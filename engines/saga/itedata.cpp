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

// Actor and Object data tables
#include "saga/saga.h"
#include "saga/itedata.h"
#include "saga/sndres.h"

namespace Saga {

ActorTableData ITE_ActorTable[ITE_ACTORCOUNT] = {
	// Original used so called permanent actors for first three and that was designed by
	// EXTENDED object flag. They contained frames in more than one resource. We use
	// different technique here see "Appending to sprite list" in loadActorResources()

//       flags     name scene    x     y    z  spr  frm scp  col
//    ------------ ---- ----  ---- ----- ---- ---- ---- --- ---- -- -- --
	{ kProtagonist | kExtended,
					 0,   1,    0,    0,   0,  37, 135,  0,   1,  0, 0, 0},	// map party
	// spr and frm numbers taken from permanent actors list
	{ kFollower | kExtended,
					 1,   0,    0,    0,   0,  45, 177,  1, 132,  0, 0, 0},	// Okk
	{ kFollower | kExtended,
					 2,   0,    0,    0,   0,  48, 143,  2, 161,  0, 0, 0},	// Eeah
	{ 0,             3,   0,  240,  480,   0, 115, 206,  0,  25,  0, 0, 0},	// albino ferret
	{ 0,             4,  17,  368,  400,   0, 115, 206,  4,  49,  0, 0, 0},	// moneychanger
	{ 0,             5,  11,  552,  412,   0,  54, 152,  1, 171,  0, 0, 0},	// Sist
	{ 0,            17,   2, 1192,  888,   0,  57, 153, 17,  49,  0, 0, 0},	// worker ferret 1
	{ 0,            17,   2,  816, 1052,   0,  57, 153, 18,  49,  0, 0, 0},	// worker ferret 2
	{ 0,            17,   2,  928,  932,   0,  58, 153, 19,  49,  0, 0, 0},	// worker ferret 3
	{ 0,            17,   2, 1416, 1160,   0,  58, 153, 20,  49,  0, 0, 0},	// worker ferret 4
	{ 0,            19,  49, 1592, 1336,   0,  92, 175, 15, 162,  0, 0, 0},	// faire merchant 1 (bear)
	{ 0,            20,  49,  744,  824,   0,  63, 156, 19, 112,  0, 4, 4},	// faire merchant 2 (ferret)
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire merchant 3
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire merchant 4
	{ 0,             9,  49, 1560, 1624,   0,  94, 147, 18, 132,  0, 4, 4},	// faire goer 1a (rat)
	{ 0,            56,  49, 1384,  792,   0,  95, 193, 20,  72,  0, 0, 0},	// faire goer 1b (otter)
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 2a
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 2b
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 3a
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 3b
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 4a
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 4b
	{ 0,            18,  32,  764,  448,   0,  55, 150,  0,  48, 10, 4, 4},	// Scorry
	{ 0,            35,  32,    0,    0,   0,  56, 151,  0, 112,  0, 0, 0},	// grand puzzler
	{ 0,            36,  32,    0,    0,   0, 105, 142,  0, 155,  0, 0, 0},	// Rhene
	{ 0,            32,  32,    0,    0,   0,  91, 190,  0,  98,  0, 0, 0},	// elk captain
	{ 0,            31,  32,    0,    0,   0,  90, 189,  0, 171,  0, 0, 0},	// elk guard 1
	{ 0,            31,  32,    0,    0,   0,  90, 189,  0, 171,  0, 0, 0},	// elk guard 2
	{ 0,            31,  32,    0,    0,   0,  90, 189,  0, 171,  0, 0, 0},	// elk guard 3
	{ 0,            31,  32,    0,    0,   0,  79, 172,  0,  18,  0, 0, 0},	// boar sergeant
	{ 0,            21,  50,  664,  400,   0,  76, 171,  2,  74,  0, 4, 4},	// boar sentry 1
	{ 0,            21,  50,  892,  428,   0,  76, 171,  2,  74,  0, 4, 4},	// boar sentry 2
	{ 0,             9,  51,  904,  936,   0,  51, 145, 35,   5,  0, 0, 0},	// hall rat 1
	{ 0,             9,  51,  872,  840,   0,  51, 145, 36,   5,  0, 0, 0},	// hall rat 2
	{ 0,             9,  51, 1432,  344,   0,  51, 145, 37,   5,  0, 0, 0},	// hall rat 3
	{ 0,             9,  51,  664,  472,   0,  51, 145, 38,   5,  0, 0, 0},	// hall rat 4
	{ 0,            10,  51, 1368, 1464,   0,  80, 146, 39, 147,  0, 0, 0},	// book rat 1
	{ 0,            10,  51, 1416, 1624,   0,  80, 146, 40, 147,  0, 0, 0},	// book rat 2
	{ 0,            10,  51, 1752,  120,   0,  80, 146, 41, 147,  0, 0, 0},	// book rat 3
	{ 0,            10,  51,  984,  408,   0,  80, 146, 42, 147,  0, 0, 0},	// book rat 4
	{ 0,            14,  52,  856,  376,   0,  82, 174,  8,  73,  0, 0, 0},	// grounds servant 1
	{ 0,            14,  52,  808,  664,   0,  82, 174,  9,  73,  0, 0, 0},	// grounds servant 2
	{ 0,            14,  52,  440,  568,   0,  82, 174, 10,  73,  0, 0, 0},	// grounds servant 3
	{ 0,            14,  52,  392,  776,   0,  82, 174, 11,  73,  0, 0, 0},	// grounds servant 4
	{ 0,            21,   4,  240,  384,   0,  79, 172,  0,  18,  0, 2, 2},	// boar sentry 3 (by doorway)
	{ 0,            23,   4,  636,  268,   0,  77, 173,  0,  74,  0, 4, 4},	// boar courtier
	{ 0,            22,   4,  900,  320,   0,  78, 179,  0,  60,  0, 4, 4},	// boar king
	{ 0,            14,   4,  788,  264,   0,  75, 170,  0, 171,  0, 2, 2},	// boar servant 1
	{ 0,            14,   4, 1088,  264,   0,  75, 170,  0, 171,  0, 6, 6},	// boar servant 2
	{ 0,            24,  19,  728,  396,   0,  65, 181, 47, 146,  0, 6, 6},	// glass master
	{ 0,            24,  21,  -20,  -20,   0,  66, 182,  0, 146,  0, 4, 4},	// glass master (with orb)
	{ kCycle,       25,  19,  372,  464,   0,  67, 183, 73, 146,  0, 2, 2},	// glass worker
	{ 0,            26,   5,  564,  476,  27,  53, 149,  1,   5,  0, 4, 4},	// door rat
	{ kCycle,       27,  31,  868,  344,   0,  81, 180,  0, 171,  0, 4, 4},	// bees
	{ 0,            28,  73,  568,  380,   0,  83, 176, 30, 120,  0, 4, 4},	// fortune teller
	{ 0,            14,   7,  808,  480,   0,  82, 174,  9,  73,  0, 0, 0},	// orb messenger
	{ 0,            29,  10,  508,  432,   0,  84, 186,  6, 112,  0, 4, 4},	// elk king
	{ 0,            33,  10,  676,  420,   0,  86, 184,  6, 171,  0, 4, 4},	// elk chancellor
	{ 0,            30,  10,  388,  452,   0,  88, 185,  6, 171,  0, 4, 4},	// elk courtier 1
	{ 0,            30,  10,  608,  444,   0,  89, 185,  6, 171,  0, 4, 4},	// elk courtier 2
	{ 0,            31,  10,  192,  468,   0,  90, 189,  6, 171,  0, 4, 4},	// elk throne guard 1
	{ 0,            31,  10,  772,  432,   0,  90, 189,  6, 171,  0, 4, 4},	// elk throne guard 2
	{ 0,            14,  10, 1340,  444,   0,  87, 188,  6, 171,  0, 4, 4},	// elk servant
	{ 0,            20,  18,  808,  360,   7,  60, 154, 64,  88,  0, 4, 4},	// hardware ferret
	{ 0,            34,  49, 1128, 1256,   0,  96, 191, 16,  35,  0, 4, 4},	// porcupine
	{ 0,            34,  49, 1384,  792,   0,  93, 192, 17,  66,  0, 4, 4},	// faire ram
	{ 0,            24,  21,    0,  -40,   0,  65, 181, 50, 146,  0, 6, 6},	// glass master 2
	{ 0,             3,  21,    0,  -40,   0,  64, 158, 49, 112,  0, 0, 0},	// Sakka
	{ 0,            17,  21,    0,  -40,   0,  62, 157, 74,  48,  0, 0, 0},	// lodge ferret 1
	{ 0,            17,  21,    0,  -40,   0,  62, 157, 74,  49,  0, 0, 0},	// lodge ferret 2
	{ 0,            17,  21,    0,  -40,   0,  62, 157, 74,  50,  0, 0, 0},	// lodge ferret 3
	{ 0,            12, 244, 1056,  504,   0, 107, 167, 21, 124,  0, 6, 6},	// Elara
	{ 0,             8,  33,  248,  440,   0,  68, 169, 14, 112,  0, 0, 0},	// Tycho
	{ 0,            11,  23,  308,  424,   0, 106, 166,  6,  48,  0, 2, 2},	// Alamma
	{ 0,            17,   2, 1864, 1336,   0,  58, 153, 21,  49,  0, 0, 0},	// worker ferret 5
	{ 0,            17,   2,  760,  216,   0,  58, 153, 22,  49,  0, 0, 0},	// worker ferret 6
	{ 0,            44,  29,    0,    0,   0,  72, 159,  0, 112,  0, 0, 0},	// Prince
	{ 0,            45,  29,    0,    0,   0,  71, 163,  0, 146,  0, 6, 6},	// harem girl 1
	{ 0,            45,  29,    0,    0,   0,  71, 163,  0, 124,  0, 2, 2},	// harem girl 2
	{ 0,            45,  29,    0,    0,   0,  71, 163,  0, 169,  0, 0, 0},	// harem girl 3
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0},	// dog sergeant
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0},	// throne dog guard 1
	{ 0,             7, 257,  552,  408,   0,  70, 165,  0,   4,  0, 2, 2},	// throne dog guard 2
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0},	// throne dog guard 3
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0},	// throne dog guard 4
	{ 0,             7, 257,  712,  380,   0,  69, 164,  0,   4,  0, 4, 4},	// throne dog guard 5
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0},	// throne dog guard 6
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0},	// throne dog guard 7
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0},	// throne dog guard 8
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0},	// throne dog guard 9
	{ 0,             7,   0,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0},	// throne dog guard 10
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0},	// throne dog guard 11
	{ 0,            47,  30,    0,    0,   0, 102, 199,  1, 186,  0, 0, 0},	// old wolf ferryman
	{ 0,            48,  69,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0},	// cat village wildcat
	{ 0,            49,  69,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0},	// cat village attendant
	{ 0,            50,  69,    0,    0,   0, 111, 203, 16,  67,  0, 0, 0},	// cat village Prowwa
	{ 0,            51,  20,    0,    0,   0, 112, 204, 15,  26,  0, 0, 0},	// Prowwa hut Mirrhp
	{ 0,            50,  20,    0,    0,   0, 111, 203, 14,  67,  0, 0, 0},	// Prowwa hut Prowwa
	{ 0,            49,  20,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0},	// Prowwa hut attendant
	{ 0,            48, 256,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0},	// wildcat sentry
	{ 0,            21,  32,    0,    0,   0,  76, 171,  0, 171,  0, 0, 0},	// boar warrior 1
	{ 0,            21,  32,    0,    0,   0,  76, 171,  0, 171,  0, 0, 0},	// boar warrior 2
	{ 0,            21,  32,    0,    0,   0,  76, 171,  0, 171,  0, 0, 0},	// boar warrior 3
	{ 0,            52,  15,  152,  400,   0, 108, 168, 19,  48, 10, 2, 2},	// Alamma's voice
	{ 0,            47, 251,  640,  360,   0, 113, 205,  5, 186, 10, 2, 2},	// ferry on ocean
	{ 0,            41,  75,  152,  400,   0, 100, 197,  5,  81,  0, 0, 0},	// Shiala
	{ 0,            44,   9,    0,    0,   0,  73, 160, 54, 112,  0, 0, 0},	// Prince (asleep)
	{ 0,             0,  22,  -20,  -20,   0, 118, 209,  0, 171,  0, 0, 0},	// Rif and Eeah (at rockslide)
	{ 0,             1,  22,    0,    0,   0, 119, 210,  0, 171,  0, 0, 0},	// Okk (at rockslide)
	{ 0,             0,  22,  -20,  -20,   0, 118, 209,  0, 171,  0, 0, 0},	// Rif and Eeah (at rockslide w. rope)
	{ 0,             1,  22,    0,    0,   0, 119, 210,  0, 171,  0, 0, 0},	// Okk (at rockslide w. rope)
	{ 0,            53,  42,  640,  400,   0, 104, 201,  8, 141,  0, 0, 0},	// Kylas Honeyfoot
	{ 0,            54,  21,  -20,  -20,   0, 120, 211, 48, 238,  0, 0, 0},	// Orb of Hands
	{ 0,             0,   4,  -20,  -20,   0,  42, 140,  0,   1,  0, 0, 0},	// Rif (muddy)
	{ 0,            26,   5,  -20,  -20,  27,  52, 148,  1,   5,  0, 4, 4},	// door rat (standing)
	{ 0,            36,   4,  -20,  -20,   0, 116, 207,  0, 155,  0, 0, 0},	// boar with Rhene 1
	{ 0,            36,   0,  -20,  -20,   0, 117, 208,  0, 155,  0, 0, 0},	// boar with Rhene 2
	{ 0,            46, 252,  -20,  -20,   0,  74, 162, 29,  34,  0, 0, 0},	// dog jailer
	{ 0,             0,  32,  -20,  -20,   0,  41, 137,  0,   1,  0, 0, 0},	// Rif (tourney)
	{ 0,             0, 259,  -20,  -20,   0,  44, 138,  0,   1,  0, 0, 0},	// cliff rat
	{ 0,             0,   5,  -20,  -20,   0,  43, 139,  0,   1,  0, 0, 0},	// Rif (cloaked)
	{ 0,             0,  31,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0},	// Rif (oak tree scene)
	{ 0,             0, 252,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0},	// Rif (jail cell scene)
	{ 0,             0,  15,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0},	// Rif (outside Alamma's)
	{ 0,             0,  20,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0},	// Rif (sick tent)
	{ 0,             0,  25,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0},	// Rif (gem room)
	{ 0,             0, 272,  -20,  -20,   0,  40, 141,  0,   1,  0, 0, 0},	// Rif (dragon maze)
	{ 0,             0,  50,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0},	// Rif (boar entry gate)
	{ 0,            50,  71,  -20,  -20,   0, 111, 203,  0,  67,  0, 0, 0},	// Prowwa (dog castle back)
	{ 0,            50, 274,  -20,  -20,   0, 111, 203,  0,  67,  0, 0, 0},	// Prowwa (cat festival)
	{ 0,            50, 274,  -20,  -20,   0, 110, 212,  0, 171,  0, 0, 0},	// cat festival dancer 1
	{ 0,            50, 274,  -20,  -20,   0, 110, 212,  0, 171,  0, 0, 0},	// cat festival dancer 2
	{ 0,            50, 274,  -20,  -20,   0, 110, 212,  0, 171,  0, 0, 0},	// cat festival dancer 3
	{ 0,            57, 272,  909,  909,  48, 121, 213,  0, 171,  0, 0, 0},	// komodo dragon
	{ 0,            58,  15,  -20,  -20,   0, 122, 214,  0, 171,  0, 0, 0},	// letter from Elara
	{ 0,            37, 246,  -20,  -20,   0,  97, 194,  0, 141,  0, 0, 0},	// Gar (wolves' cage)
	{ 0,            38, 246,  -20,  -20,   0,  98, 195,  0,  27,  0, 0, 0},	// Wrah (wolves' cage)
	{ 0,            59, 246,  -20,  -20,   0, 103, 200,  0,  26,  0, 0, 0},	// Chota (wolves' cage)
	{ 0,            41, 245,  -20,  -20,   0, 100, 197,  0,  81,  0, 0, 0},	// Shiala (wolves' cage)
	{ 0,            47, 250,  640,  360,   0, 114, 205,  0, 186, 10, 2, 2},	// ferry on ocean
	{ 0,             0, 278,  -20,  -20,   0,  40, 141,  0,   1,  0, 0, 0},	// Rif (falling in tunnel trap door)
	{ 0,             0, 272,  -20,  -20,   0,  40, 141,  0,   1,  0, 0, 0},	// Rif (falling in dragon maze)
	{ 0,            41,  77,  -20,  -20,   0, 100, 197, 24,  81,  0, 0, 0},	// Shiala (grotto)
	{ 0,            37, 261,  -20,  -20,   0,  97, 194,  0, 141,  0, 0, 0},	// Gar (ambush)
	{ 0,            38, 261,  -20,  -20,   0,  98, 195,  0,  27,  0, 0, 0},	// Wrah (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0},	// dark claw wolf (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0},	// dark claw wolf (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0},	// dark claw wolf (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0},	// dark claw wolf (ambush)
	{ 0,            59, 279,  -20,  -20,   0, 103, 200,  0,  26,  0, 0, 0},	// Chota (top of dam)
	{ 0,            38, 279,  -20,  -20,   0,  98, 195,  0,  27,  0, 0, 0},	// Wrah (top of dam)
	{ 0,            42,  77,  -20,  -20,   0, 101, 198, 25, 171,  0, 0, 0},	// Shiala's spear
	{ 0,            59, 281,  -20,  -20,   0, 103, 200, 26,  26,  0, 0, 0},	// Chota (lab)
	{ 0,            59, 279,  -20,  -20,   0, 123, 215,  0,   1,  0, 0, 0},	// Rif (finale)
	{ 0,            59, 279,  -20,  -20,   0, 123, 215,  0, 132,  0, 0, 0},	// Okk (finale)
	{ 0,            59, 279,  -20,  -20,   0, 123, 215,  0, 161,  0, 0, 0},	// Eeah (finale)
	{ 0,            54, 279,  -20,  -20,   0, 120, 211,  0, 133,  0, 6, 6},	// Orb of Storms (top of dam)
	{ 0,            44,   9,  -20,  -20,   0, 124, 161,  0, 171,  0, 6, 6},	// Prince's snores
	{ 0,             7, 255,  588,  252,   0,  70, 165,  0,   3,  0, 2, 2},	// hall dog guard 1
	{ 0,             7, 255,  696,  252,   0,  70, 165,  0,   5,  0, 6, 6},	// hall dog guard 2
	{ 0,            36,   4,    0,    0,   0, 105, 142,  0, 155,  0, 0, 0},	// Rhene
	{ 0,            44, 272, 1124, 1124, 120,  72, 159,  0, 112,  0, 0, 0},	// Prince (dragon maze)
	{ 0,             7, 272, 1124, 1108, 120,  70, 165,  0,   4,  0, 0, 0},	// dog heckler 1 (dragon maze)
	{ 0,             7, 272, 1108, 1124, 120,  70, 165,  0,   4,  0, 0, 0},	// dog heckler 2 (dragon maze)
	{ 0,            29, 288,  508,  432,   0,  85, 187,  0, 112,  0, 4, 4},	// elk king (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0,  99,  0, 4, 4},	// crowd voice 1 (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0,  98,  0, 4, 4},	// crowd voice 2 (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0, 104,  0, 4, 4},	// crowd voice 3 (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0,  99,  0, 4, 4},	// crowd voice 4 (finale)
	{ 0,            36, 288,    0,    0,   0, 105, 142,  0, 155,  0, 0, 0},	// Rhene (finale)
	{ 0,             1,  27,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0},	// Okk (temple gate)
	{ 0,             1, 252,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0},	// Okk (jail cell)
	{ 0,             1,  25,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0},	// Okk (gem room)
	{ 0,             1, 259,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0},	// Okk (cliff)
	{ 0,             1, 279,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0},	// Okk (dam top)
	{ 0,             1, 273,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0},	// Okk (human ruins)
	{ 0,             1,  26,  -20,  -20,   0,   8, 178,  0, 171,  0, 0, 0},	// puzzle pieces
	{ 0,             1,   0,  -20,  -20,   0,   0,   0,  0,  50,  0, 0, 0},	// poker dog 1
	{ 0,             1,   0,  -20,  -20,   0,   0,   0,  0,  82,  0, 0, 0},	// poker dog 2
	{ 0,             1,   0,  -20,  -20,   0,   0,   0,  0,  35,  0, 0, 0},	// poker dog 3
	{ 0,             9,  74,  -20,  -20,   0,  51, 145,  0,   5,  0, 0, 0}	// sundial rat
};


ObjectTableData ITE_ObjectTable[ITE_OBJECTCOUNT] = {
	{  8,  49, 1256,  760,  0,  9,  5, kObjNotFlat }, // Magic Hat
	{  9,  52, 1080, 1864,  0, 68,  4, kObjUseWith }, // Berries
	{ 10, 259,  744,  524,  0, 79, 42, kObjUseWith }, // Card Key
	{ 11,   0,  480,  480,  0, 69,  6, 0           }, // Foot Print
	{ 12,   0,  480,  480,  0, 13, 38, kObjUseWith }, // Power Cell
	{ 13,  28,  640,  412, 40, 14, 15, kObjUseWith }, // Digital Clock
	{ 14,   0,  480,  480,  0, 15, 41, kObjUseWith }, // Oil Lamp
	{ 15,  24,  868,  456, 35, 46, 13, kObjUseWith }, // Magnetic Key
	{ 16,   0,  480,  480,  0, 17,  7, kObjUseWith }, // Plaster
	{ 17, 249,  320,  476, 45, 18, 44, 0           }, // Trophy
	{ 18,   0,  480,  480,  0, 19, 20, 0           }, // Coins
	{ 19,  19,  600,  480,  0, 20,  8, 0           }, // Lens Fragments
	{ 20,   0, 1012,  568, 80, 44, 10, kObjUseWith }, // Key to jail cell
	{ 21,   0,  480,  480,  0, 22,  9, 0           }, // Remade lens
	{ 22,   0,  480,  480,  0, 23, 21, 0           }, // Tycho's Map
	{ 23,   0,  480,  480,  0, 24, 23, 0           }, // Silver Medallion
	{ 24,   0,  480,  480,  0, 25, 24, 0           }, // Mud in Fur
	{ 25,   0,  480,  480,  0, 26, 25, 0           }, // Gold Ring
	{ 27,  13, 1036,  572, 40, 47, 14, kObjUseWith }, // Screwdriver
	{ 28,   0,  480,  480,  0, 29, 26, 0           }, // Apple Token
	{ 29,   0,  480,  480,  0, 30, 22, kObjUseWith }, // Letter from Elara
	{ 30,   0,  164,  440,  0, 31, 16, kObjUseWith }, // Spoon
	{ 32,   0,  480,  480,  0, 33, 43, 0           }, // Catnip
	{ 33,  31,  580,  392,  0, 45, 11, 0           }, // Twigs
	{ 35,   0,  468,  480,  0, 36, 12, kObjUseWith }, // Empty Bowl (also bowl of honey)
	{ 37,   0,  480,  480,  0, 38, 45, kObjUseWith }, // Needle and Thread
	{ 38,  25,  332,  328,  0, 48, 19, 0           }, // Rock Crystal
	{ 39,   0,  480,  480,  0, 40,  0, kObjUseWith }, // Salve
	{ 40, 269,  644,  416,  0, 41, 39, kObjNotFlat }, // Electrical Cable
	{ 41,  12,  280,  516,  0, 43, 17, kObjUseWith }, // Piece of flint
	{ 42,   5,  876,  332, 32, 65, 18, 0           }, // Rat Cloak
	{ 43,  52,  556, 1612,  0, 49, 28, kObjUseWith |
									   kObjNotFlat }, // Bucket
	{ 48,  52,  732,  948,  0, 50, 27, kObjUseWith }, // Cup
	{ 49,  52,  520, 1872,  0, 53, 29, 0           }, // Fertilizer
	{ 50,  52, 1012, 1268,  0, 52, 30, 0           }, // Feeder
	{ 51, 252,  -20,  -20,  0, 71, 32, kObjUseWith |
									   kObjNotFlat }, // Bowl in jail cell
	{ 53, 252, 1148,  388,  0, 70, 33, 0           }, // Loose stone block in jail cell
	{ 26,  12,  496,  368,  0, 76, 31, 0           }, // Coil of Rope from Quarry
	{ 54, 281,  620,  352,  0, 80, 46, 0           }  // Orb of Storms in Dam Lab
};

IteFxTable ITE_SfxTable[ITE_SFXCOUNT] = {
	{ 14,	127 },	// Door open
	{ 15,   127 },	// Door close
	{ 16,    63 },	// Rush water (floppy volume: 127)
	{ 16,    26 },	// Rush water (floppy volume: 40)
	{ 17,    64 },	// Cricket
	{ 18,    84 },	// Porticullis (floppy volume: 127)
	{ 19,    64 },	// Clock 1
	{ 20,    64 },	// Clock 2
	{ 21,    64 },	// Dam machine
	{ 21,    40 },	// Dam machine
	{ 22,    64 },	// Hum 1
	{ 23,    64 },	// Hum 2
	{ 24,    64 },	// Hum 3
	{ 25,    64 },	// Hum 4
	// Note: the following effect was set to 51 for
	// some unknown reason
	{ 26,	 32 },	// Stream
	{ 27,    42 },	// Surf (floppy volume: 127)
	{ 27,    32 },	// Surf (floppy volume: 64)
	{ 28,    64 },	// Fire loop (floppy volume: 96)
	{ 29,    84 },	// Scraping (floppy volume: 127)
	{ 30,    64 },	// Bee swarm (floppy volume: 96)
	{ 30,    26 },	// Bee swarm (floppy volume: 40)
	{ 31,    64 },	// Squeaky board
	{ 32,   127 },	// Knock
	{ 33,    32 },	// Coins (floppy volume: 48)
	{ 34,    84 },	// Storm (floppy volume: 127)
	{ 35,	 84 },	// Door close 2 (floppy volume: 127)
	{ 36,    84 },  // Arcweld (floppy volume: 127)
	{ 37,	127 },	// Retract orb
	{ 38,   127 },	// Dragon
	{ 39,   127 },	// Snores
	{ 40,   127 },	// Splash
	{ 41,   127 },	// Lobby door
	{ 42,    26 },	// Chirp loop (floppy volume: 40)
	{ 43,    96 },	// Door creak
	{ 44,    64 },	// Spoon dig
	{ 45,    96 },	// Crow
	{ 46,    42 },	// Cold wind (floppy volume: 64)
	{ 47,    96 },	// Tool sound 1
	{ 48,   127 },	// Tool sound 2
	{ 49,    64 },	// Tool sound 3
	{ 50,    96 },	// Metal door
	{ 51,	 32 },	// Water loop S
	{ 52,	 32 },	// Water loop L (floppy volume: 64)
	{ 53,	127 },	// Door open 2
	{ 54,	 64 },	// Jail door
	{ 55,	 53 },	// Killing fire (floppy volume: 80)
  //{ 56,	  0 },	// Dummy FX
	// Crowd effects, which exist only in the CD version
	{ 57,	 64 },
	{ 58,    64 },
	{ 59,    64 },
	{ 60,    64 },
	{ 61,    64 },
	{ 62,    64 },
	{ 63,    64 },
	{ 64,    64 },
	{ 65,    64 },
	{ 66,    64 },
	{ 67,    64 },
	{ 68,    64 },
	{ 69,    64 },
	{ 70,    64 },
	{ 71,    64 },
	{ 72,    64 },
	{ 73,    64 }
};

const char *ITEinterfaceTextStrings[][53] = {
	{
		// Note that the "Load Successful!" string is never used in ScummVM
		"Walk to", "Look At", "Pick Up", "Talk to", "Open",
		"Close", "Use", "Give", "Options", "Test",
		"Demo", "Help", "Quit Game", "Fast", "Slow",
		"On", "Off", "Continue Playing", "Load", "Save",
		"Game Options", "Reading Speed", "Music", "Sound", "Cancel",
		"Quit", "OK", "Mid", "Click", "10%",
		"20%", "30%", "40%", "50%", "60%",
		"70%", "80%", "90%", "Max", "Quit the Game?",
		"Load Successful!", "Enter Save Game Name", "Give %s to %s", "Use %s with %s",
		"[New Save Game]",
		"I can't pick that up.",
		"I see nothing special about it.",
		"There's no place to open it.",
		"There's no opening to close.",
		"I don't know how to do that.",
		"Show Dialog",
		"What is Rif's reply?",
		"Loading a saved game"
	},
	// German
	{
		"Gehe zu", "Schau an", "Nimm", "Rede mit", "\231ffne",
		"Schlie$e", "Benutze", "Gib", "Optionen", "Test",
		"Demo", "Hilfe", "Spiel beenden", "S", "L",
		"An", "Aus", "Weiterspielen", "Laden", "Sichern",
		"Spieleoptionen", "Lesegeschw.", "Musik", "Sound", "Abbr.",
		"Beenden", NULL, "M", "Klick", "10%",
		"20%", "30%", "40%", "50%", "60%",
		"70%", "80%", "90%", "Max", "Spiel beenden?",
		"Spielstand geladen!", "Bitte Namen eingeben", "Gib %s zu %s", "Benutze %s mit %s",
		"[Neuer Spielstand]",
		"Das kann ich nicht aufnehmen.",
		"Ich sehe nichts besonderes.",
		"Das kann man nicht \224ffnen.",
		"Hier ist keine \231ffnung zum Schlie$en.",
		"Ich wei$ nicht, wie ich das machen soll.",
		"Text zeigen",
		"Wie lautet die Antwort?",
		"Spielstand wird geladen"
	},
	// Italian fan translation
	{
		"Vai verso", "Guarda", "Prendi", "Parla con", "Apri",
		"Chiudi", "Usa", "Dai", "Opzioni", "Test",
		"Demo", "Aiuto", "Uscire", "Veloce", "Lento",
		"On", "Off", "Continua il Gioco", "Carica", "Salva",
		"Controlli", "Velocit\205 testo", "Musica", "Suoni", "Annulla",
		"Fine", "OK", "Med", "Click", "10%",
		"20%", "30%", "40%", "50%", "60%",
		"70%", "80%", "90%", "Max", "Terminare il Gioco?",
		"Caricamento OK!", "Immettere un nome", "Dai %s a %s", "Usa %s con %s",
		"[Nuovo Salvataggio]",
		"Non posso raccoglierlo.",
		"Non ci vedo nulla di speciale.",
		"Non c'\212 posto per aprirlo.",
		"Nessuna apertura da chiudere.",
		"Non saprei come farlo.",
		"Dialoghi",
		"Come risponderebbe Rif?",
		"Vuoi davvero caricare il gioco?"
	},
	// Spanish IHNM
	{
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, "Salir del Programa?",
		"Load Successful!", "Introduzca Nombre Partida", "Dar %s a %s", "Usar %s con %s",
		// Original uses "Partida Grabada" here (saved game), but "nueva partida" (new save
		// game) makes more sense (according to jvprat)
		"[Nueva partida]",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"Cardango una partida guardada"
	},
	// Russian IHNM
	{
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, "\xC2\xFB\xE9\xF2\xE8 \xE8\xE7 \xE8\xE3\xF0\xFB?",
		"Load Successful!",
		"\xC2\xE2\xE5\xE4\xE8\xF2\xE5 \xE8\xEC\xFF \xE7\xE0\xEF\xE8\xF1\xE8",
		"\xC4\xE0\xF2\xFC %s >> %s",
		"\xC8\xF1\xEF\xEE\xEB\xFC\xE7\xEE\xE2\xE0\xF2\xFC %s >> %s",
		"[\xCD\xEE\xE2\xE0\xFF \xE7\xE0\xEF\xE8\xF1\xFC]",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"       \xC7\xE0\xE3\xF0\xF3\xE7\xEA\xE0     "
	},
	// French fan translation : ACTIONS
	{
		"Aller vers", "Examiner", "Prendre", "Parler \205", "Ouvrir",
		"Fermer", "Utiliser", "Donner", "Options", "Test",
		"Demo", "Aide", "Quitter le Jeu", "Vite", "Lent",
		"On", "Off", "Continuer \205 jouer", "Charger", "Sauver",
		"Options du Jeu", "Vitesse texte", "Musique", "Son", "Annuler",
		"Quitter", "OK", "Mid", "Click", "10%",
		"20%", "30%", "40%", "50%", "60%",
		"70%", "80%", "90%", "Max", "Quitter le jeu?",
		"Chargement OK!", "Nommer la sauvegarde", "Donner %s \205 %s", "Utiliser %s avec %s",
		"[Nouvelle sauvegarde]",
		"Je ne peux pas prendre ça.",
		"Je ne vois rien de sp\202cial.",
		"Il n'y a pas la place pour l'ouvrir.",
		"Il n'y a pas d'ouverture \205 fermer.",
		"Je ne sais pas comment l'utiliser.",
		"Sous-titre",
		"Que r\202pond Rif?",
		"Charger une sauvegarde"
	},
	// Japanese
	{
		// "歩　く", "見　る", "拾　う", "話　す", "開ける",
		"\x95\xE0\x81@\x82\xAD", "\x8C\xA9\x81@\x82\xE9", "\x8F\x45\x81@\x82\xA4", "\x98\x62\x81@\x82\xB7", "\x8AJ\x82\xAF\x82\xE9",
		// "閉める", "使　う", "与える", "Options", "Test",
		"\x95\xC2\x82\xDF\x82\xE9", "\x8Eg\x81@\x82\xA4", "\x97^\x82\xA6\x82\xE9", "Options", "Test",
		// "Demo", "Help", "ゲーム終了", "速い", "遅い",
		"Demo", "Help", "\x83Q\x81[\x83\x80\x8FI\x97\xB9", "\x91\xAC\x82\xA2", "\x92x\x82\xA2",
		// "オン", "オフ", "ゲーム再開", "ロード", "セーブ",
		"\x83I\x83\x93", "\x83I\x83t", "\x83Q\x81[\x83\x80\x8D\xC4\x8AJ", "\x83\x8D\x81[\x83h", "\x83Z\x81[\x83u",
		// "ゲームオプション", "表示速度", "音楽", "効果音", "無効",
		"\x83Q\x81[\x83\x80\x83I\x83v\x83V\x83\x87\x83\x93", "\x95""\\""\x8e\xa6\x91\xac\x93""x", "\x89\xB9\x8Ay", "\x8C\xF8\x89\xCA\x89\xB9", "\x96\xB3\x8C\xF8",
		// "終了", "ＯＫ", "普通", "クリック", "１０％",
		"\x8FI\x97\xB9", "\x82n\x82j", "\x95\x81\x92\xCA", "\x83N\x83\x8A\x83\x62\x83N", "\x82P\x82O\x81\x93",
		// "２０％", "３０％", "４０％", "５０％", "６０％",
		"\x82Q\x82O\x81\x93", "\x82R\x82O\x81\x93", "\x82S\x82O\x81\x93", "\x82T\x82O\x81\x93", "\x82U\x82O\x81\x93",
		// "７０％", "８０％", "９０％", "最大", "ゲームを中断しますか？",
		"\x82V\x82O\x81\x93", "\x82W\x82O\x81\x93", "\x82X\x82O\x81\x93", "\x8D\xC5\x91\xE5", "\x83Q\x81[\x83\x80\x82\xF0\x92\x86\x92\x66\x82\xB5\x82\xDC\x82\xB7\x82\xA9\x81H",
		// "?", "ゲーム名は？", "%sに%sを与える", "%sを%sに使う",
		"?", "\x83Q\x81[\x83\x80\x96\xBC\x82\xCD\x81H", "%s\x82\xC9%s\x82\xF0\x97^\x82\xA6\x82\xE9", "%s\x82\xF0%s\x82\xC9\x8Eg\x82\xA4",
		"[New Save Game]",
		// "拾えないね。",
		"\x8F\x45\x82\xA6\x82\xC8\x82\xA2\x82\xCB\x81\x42",
		// "変わったものはないね。",
		"\x95\xCF\x82\xED\x82\xC1\x82\xBD\x82\xE0\x82\xCC\x82\xCD\x82\xC8\x82\xA2\x82\xCB\x81\x42",
		// "どこも開きそうにないよ。",
		"\x82\xC7\x82\xB1\x82\xE0\x8AJ\x82\xAB\x82\xBB\x82\xA4\x82\xC9\x82\xC8\x82\xA2\x82\xE6\x81\x42",
		// "どこも閉められない。",
		"\x82\xC7\x82\xB1\x82\xE0\x95\xC2\x82\xDF\x82\xE7\x82\xEA\x82\xC8\x82\xA2\x81\x42",
		// "どうすればいいか分からないね。",
		"\x82\xC7\x82\xA4\x82\xB7\x82\xEA\x82\xCE\x82\xA2\x82\xA2\x82\xA9\x95\xAA\x82\xA9\x82\xE7\x82\xC8\x82\xA2\x82\xCB\x81\x42",
		"?",
		// "リフの返事は？",
		"\x83\x8A\x83t\x82\xCC\x95\xD4\x8E\x96\x82\xCD\x81H",
		"?"
	}
};

const RawPoint pieceOrigins[PUZZLE_PIECES] = {
	{ 268,  18 },
	{ 270,  51 },
	{  19,  51 },
	{  73,   0 },
	{   0,  34 },
	{ 215,   0 },
	{ 159,   0 },
	{   9,  69 },
	{ 288,  18 },
	{ 112,   0 },
	{  27,  88 },
	{  43,   0 },
	{   0,   0 },
	{ 262,   0 },
	{ 271, 103 }
};

// Objects
const char *pieceNames[][PUZZLE_PIECES] = {
	{
		"screwdriver", "pliers", "c-clamp", "wood clamp", "level",
		"twine", "wood plane", "claw hammer", "tape measure", "hatchet",
		"shears", "ruler", "saw", "mallet", "paint brush"
	},
	{
		"Schraubendreher", "Zange", "Schraubzwinge", "Holzzwinge", "Wasserwaage",
		"Bindfaden", "Hobel", "Schusterhammer", "Bandma$", "Beil",
		"Schere", "Winkel", "S\204ge", "Hammer", "Pinsel"
	},
	{
		"cacciavite", "pinze", "morsa", "morsa da legno", "livella",
		"spago", "pialla", "martello", "metro a nastro", "accetta",
		"cesoie", "righello", "sega", "mazza", "pennello"
	},
	{
		"tournevis", "pince", "\202tau \205 vis", "pince \205 bois", "niveau",
		"ficelle", "rabot \205 bois", "marteau", "m\212tre ruban", "hachette",
		"ciseaux", "r\212gle", "scie", "maillet", "pinceau"
	},
	{
		// "ドライバー", "釘抜き", "Ｃ形クランプ", "木のクランプ", "レベル",
		"\x83h\x83\x89\x83\x43\x83o\x81[", "\x93\x42\x94\xB2\x82\xAB", "\x82\x62\x8C`\x83N\x83\x89\x83\x93\x83v", "\x96\xD8\x82\xCC\x83N\x83\x89\x83\x93\x83v", "\x83\x8C\x83x\x83\x8B",
		// "麻ヒモ", "かんな", "金づち", "巻尺", "斧",
		"\x96\x83\x83q\x83\x82", "\x82\xA9\x82\xF1\x82\xC8", "\x8B\xE0\x82\xC3\x82\xBF", "\x8A\xAA\x8E\xDA", "\x95\x80",
		// "ハサミ", "曲尺", "のこぎり", "木づち", "ペンキブラシ"
		"\x83n\x83T\x83~", "\x8B\xC8\x8E\xDA", "\x82\xCC\x82\xB1\x82\xAC\x82\xE8", "\x96\xD8\x82\xC3\x82\xBF", "\x83y\x83\x93\x83L\x83u\x83\x89\x83V"
	}
};

// hints
const char *hintStr[][4] = {
	{
		"Check which pieces could fit in each corner first.",
		"Check which corner has the least number of pieces that can fit and start from there.",
		"Check each new corner and any new side for pieces that fit.",
		"I don't see anything out of place."
	},
	{
		"\232berpr\201fe zun\204chst, welche die Eckteile sein k\224nnten.",
		"Schau, in welche Ecke die wenigsten Teile passen, und fang dort an.",
		"Untersuche jede Ecke und jede Seite auf Teile, die dort passen k\224nnen.",
		"Ich sehe nichts an der falschen Stelle."
	},
	{
		"Controlla prima quali pezzi si inseriscono meglio in ogni angolo.",
		"Controlla quale angolo ha il minor numero di pezzi che combaciano, e parti da quello.",
		"Controlla ogni nuovo angolo e lato per ogni pezzo che combacia.",
		"Non vedo nulla fuori posto."
	},
	{
		"Trouve d'abord les pi\212ces qui correspondent aux coins.",
		"V\202rifie quel coin a le moins de pi\212ces qui correspondent et part de l\205.",
		"Evalue chaque coin et bord pour voir les pi\212ces qui collent.",
		"Je ne vois rien de mal plac\202."
	},
	{
		// "まず、四隅に合うピースを\n捜してごらん。",
		"\x82\xDC\x82\xB8\x81\x41\x8El\x8B\xF7\x82\xC9\x8D\x87\x82\xA4\x83s\x81[\x83X\x82\xF0\n\x91{\x82\xB5\x82\xC4\x82\xB2\x82\xE7\x82\xF1\x81\x42",
		// "辺同士が合いそうなピースを\n捜してごらん。",
		"\x95\xD3\x93\xAF\x8Em\x82\xAA\x8D\x87\x82\xA2\x82\xBB\x82\xA4\x82\xC8\x83s\x81[\x83X\x82\xF0\n\x91{\x82\xB5\x82\xC4\x82\xB2\x82\xE7\x82\xF1\x81\x42",
		// "新しくできた空間と形が合う\nピースを捜してごらん。",
		"\x90V\x82\xB5\x82\xAD\x82\xC5\x82\xAB\x82\xBD\x8B\xF3\x8A\xD4\x82\xC6\x8C`\x82\xAA\x8D\x87\x82\xA4\n\x83s\x81[\x83X\x82\xF0\x91{\x82\xB5\x82\xC4\x82\xB2\x82\xE7\x82\xF1\x81\x42",
		// "置いたピースの位置は\nどれも合ってるよ。"
		"\x92u\x82\xA2\x82\xBD\x83s\x81[\x83X\x82\xCC\x88\xCA\x92u\x82\xCD\n\x82\xC7\x82\xEA\x82\xE0\x8D\x87\x82\xC1\x82\xC4\x82\xE9\x82\xE6\x81\x42"
	}
};

const char *solicitStr[][NUM_SOLICIT_REPLIES] = {
	{
		"Hey, Fox! Would you like a hint?",
		"Would you like some help?",
		"Umm...Umm...",
		"Psst! want a hint?",
		"I would have done this differently, you know."
	},
	{
		"Hey, Fuchs! Brauchst Du \047nen Tip?",
		"M\224chtest Du etwas Hilfe?"
		"\231hm...\216hm..."
		"Psst! \047n Tip vielleicht?"
		"Ja, wei$t Du... ich h\204tte das anders gemacht."
	},
	{
		"Hey, Volpe! Serve un suggerimento?",
		"Hai bisogno di aiuto?",
		"Umm...Umm...",
		"Psst! Serve un aiutino?",
		"Io, sai, l'avrei fatto diversamente."
	},
	{
		"H\202, Renard! Tu veux un coup de pouce?",
		"T'as besoin d'aide?",
		"Umm...Umm...",
		"Psst! Un indice?",
		"Tu sais, j'aurais fait ça autrement."
	},
	{
		// "ねぇ君！\nヒント欲しい？",
		"\x82\xCB\x82\xA5\x8CN\x81I\n\x83q\x83\x93\x83g\x97~\x82\xB5\x82\xA2\x81H",
		// "手助けしようか？",
		"\x8E\xE8\x8F\x95\x82\xAF\x82\xB5\x82\xE6\x82\xA4\x82\xA9\x81H",
		// "フム・・・",
		"\x83t\x83\x80\x81\x45\x81\x45\x81\x45",
		// "ちょっと！\nヒント欲しい？",
		"\x82\xBF\x82\xE5\x82\xC1\x82\xC6\x81I\n\x83q\x83\x93\x83g\x97~\x82\xB5\x82\xA2\x81H",
		// "私なら違う方法で\nやってるだろうな。"
		"\x8E\x84\x82\xC8\x82\xE7\x88\xE1\x82\xA4\x95\xFB\x96@\x82\xC5\n\x82\xE2\x82\xC1\x82\xC4\x82\xE9\x82\xBE\x82\xEB\x82\xA4\x82\xC8\x81\x42"
	}
};

const char *sakkaStr[][NUM_SAKKA] = {
	{
		"Hey, you're not supposed to help the applicants!",
		"Guys! This is supposed to be a test!",
		"C'mon fellows, that's not in the rules!"
	},
	{
		"Hey, Du darfst dem Pr\201fling nicht helfen!",
		"Hallo?! Dies soll eine Pr\201fung sein!",
		"Also, Jungs. Schummeln steht nicht in den Regeln!"
	},
	{
		"Hey, non si dovrebbero aiutare i candidati!",
		"Ragazzi! Questo dovrebbe essere un test!",
		"Forza ragazzi, non si pu\225!"
	},
	{
		"H\202, vous n'\210tes pas suppos\202s aider les concurrents!",
		"Les gars! C'est suppos\202 être un test!",
		"Allez les gars, c'est pas dans les r\212gles!"
	},
	{
		// "ねぇ、どう？",
		"\x82\xCB\x82\xA5\x81\x41\x82\xC7\x82\xA4\x81H",
		// "なかなかね！",
		"\x82\xC8\x82\xA9\x82\xC8\x82\xA9\x82\xCB\x81I",
		// "どうしたの！"
		"\x82\xC7\x82\xA4\x82\xB5\x82\xBD\x82\xCC\x81I"
	}
};

const char *whineStr[][NUM_WHINES] = {
	{
		"Aww, c'mon Sakka!",
		"One hint won't hurt, will it?",
		"Sigh...",
		"I think that clipboard has gone to your head, Sakka!",
		"Well, I don't recall any specific rule against hinting."
	},
	{
		"Och, sei nicht so, Sakka!"
		"EIN Tip wird schon nicht schaden, oder?",
		"Seufz..."
		"Ich glaube, Du hast ein Brett vor dem Kopf, Sakka!",
		"Hm, ich kann mich an keine Regel erinnern, die Tips verbietet."
	},
	{
		"Ooo, suvvia Sakka!",
		"Un indizio non guaster\205, no?",
		"Sigh...",
		"Credo che questa faccenda ti abbia dato alla testa, Sakka!",
		"Beh, non ricordo regole specifiche contro i suggerimenti."
	},
	{
		"Rohh, allez Sakka!",
		"Un indice ne peut pas faire de mal.",
		"Pfff...",
		"Je crois que ton carnet te monte \205 la t\210te, Sakka!",
		"Bon, je ne me souviens d'aucune r\212gle concernant les indices."
	},
	{
		// "アゥ、\nちょっと、サッカ！",
		"\x83\x41\x83\x44\x81\x41\n\x82\xBF\x82\xE5\x82\xC1\x82\xC6\x81\x41\x83T\x83\x62\x83J\x81I",
		// "ヒントの一つ位\nいいかい？",
		"\x83q\x83\x93\x83g\x82\xCC\x88\xEA\x82\xC2\x88\xCA\n\x82\xA2\x82\xA2\x82\xA9\x82\xA2\x81H",
		// "フー・・・",
		"\x83t\x81[\x81\x45\x81\x45\x81\x45",
		// "サッカの頭の中では\nもう完成してるんだ！",
		"\x83T\x83\x62\x83J\x82\xCC\x93\xAA\x82\xCC\x92\x86\x82\xC5\x82\xCD\n\x82\xE0\x82\xA4\x8A\xAE\x90\xAC\x82\xB5\x82\xC4\x82\xE9\x82\xF1\x82\xBE\x81I",
		// "ああ、ヒントに対抗できる\nルールが思い出せない。"
		"\x82\xA0\x82\xA0\x81\x41\x83q\x83\x93\x83g\x82\xC9\x91\xCE\x8DR\x82\xC5\x82\xAB\x82\xE9\n\x83\x8B\x81[\x83\x8B\x82\xAA\x8Ev\x82\xA2\x8Fo\x82\xB9\x82\xC8\x82\xA2\x81\x42"
	}
};

const char *optionsStr[][4] = {
	{
		"\"I'll do this puzzle later.\"",
		"\"Yes, I'd like a hint please.\"",
		"\"No, thank you, I'd like to try and solve it myself.\"",
		"I think the %s is in the wrong place."
	},
	{
		"\"Ich l\224se das Puzzle sp\204ter.\"",
		"\"Ja, ich m\224chte einen Tip, bitte.\"",
		"\"Nein danke, ich m\224chte das alleine l\224sen.\"",
		"Pssst... %s... falsche Stelle..."
	},
	{
		"\"Far\225 questo puzzle pi\227 tardi.\"",
		"\"Si, grazie. Ne avrei bisogno.\"",
		"\"No, grazie, voglio provare a risolverlo da solo.\"",
		"Penso che la tessera %s sia nel posto sbagliato."
	},
	{
		"\"Je r\202soudrai cette \202nigme plus tard.\"",
		"\"Oui, j'aimerais un indice s'il vous plait.\"",
		"\"Non merci, je voudrais r\202soudre cela par moi m\210me.\"",
		"Je crois que t'as mal plac\202 l'%s."
	},
	{
		// "「後でやろうかな。」",
		"\x81u\x8C\xE3\x82\xC5\x82\xE2\x82\xEB\x82\xA4\x82\xA9\x82\xC8\x81\x42\x81v",
		// "「はい、ヒントをお願いします。」",
		"\x81u\x82\xCD\x82\xA2\x81\x41\x83q\x83\x93\x83g\x82\xF0\x82\xA8\x8A\xE8\x82\xA2\x82\xB5\x82\xDC\x82\xB7\x81\x42\x81v",
		// "「いいえ、自分で解きたいんです。」",
		"\x81u\x82\xA2\x82\xA2\x82\xA6\x81\x41\x8E\xA9\x95\xAA\x82\xC5\x89\xF0\x82\xAB\x82\xBD\x82\xA2\x82\xF1\x82\xC5\x82\xB7\x81\x42\x81v",
		// "%sの位置が違うんじゃないかな。"
		"%s\x82\xCC\x88\xCA\x92u\x82\xAA\x88\xE1\x82\xA4\x82\xF1\x82\xB6\x82\xE1\x82\xC8\x82\xA2\x82\xA9\x82\xC8\x81\x42"
	}
};

const IntroDialogue introDialogueCave1[][4] = {
	{ { // English
		0,		// cave voice 0
		"We see the sky, we see the land, we see the water, "
		"and we wonder: Are we the only ones?"
	},
	{
		1,		// cave voice 1
		"Long before we came to exist, the humans ruled the "
		"Earth."
	},
	{
		2,		// cave voice 2
		"They made marvelous things, and moved whole "
		"mountains."
	},
	{
		3,		// cave voice 3
		"They knew the Secret of Flight, the Secret of "
		"Happiness, and other secrets beyond our imagining."
	} },
	// -----------------------------------------------------
	{ { // German
		0,		// cave voice 0
		"Um uns sind der Himmel, das Land und die Seen; und "
		"wir fragen uns - sind wir die einzigen?"
	},
	{
		1,		// cave voice 1
		"Lange vor unserer Zeit herrschten die Menschen "
		"\201ber die Erde."
	},
	{
		2,		// cave voice 2
		"Sie taten wundersame Dinge und versetzten ganze "
		"Berge."
	},
	{
		3,		// cave voice 3
		"Sie kannten das Geheimnis des Fluges, das Geheimnis "
		"der Fr\224hlichkeit und andere Geheimnisse, die "
		"unsere Vorstellungskraft \201bersteigen."
	} },
	// -----------------------------------------------------
	{ { // Italian fan translation
		0,		// cave voice 0
		"Guardiamo il cielo, guardiamo la terra, guardiamo "
		"l'acqua, e ci chiediamo: Siamo forse soli?"
	},
	{
		1,		// cave voice 1
		"Molto tempo prima che noi esistessimo, gli Umani "
		"dominavano la terra."
	},
	{
		2,		// cave voice 2
		"Fecero cose meravigliose, e mossero intere "
		"montagne."
	},
	{
		3,		// cave voice 3
		"Conoscevano il Segreto del Volo, il Segreto della "
		"Felicit\205, ed altri segreti oltre ogni nostra "
		"immaginazione."
	} },
	// -----------------------------------------------------
	{ { // French fan translation
		0,		// cave voice 0
		"Nous voyons le ciel, nous voyons les terres, "
		"nous voyons la mer et nous nous demandons: "
		"Sommes-nous vraiment seuls?"
	},
	{
		1,		// cave voice 1
		"Bien avant notre av\212nement, les humains "
		"r\202gnaient sur Terre."
	},
	{
		2,		// cave voice 2
		"Ils firent des choses merveilleuses, et "
		"d\202plac\212rent des montagnes."
	},
	{
		3,		// cave voice 3
		"Ils savaient comment Voler, poss\202daient le "
		"secret du Bonheur et d'autres secrets au "
		"del\205 de notre imagination."
	} },
	// -----------------------------------------------------
	{ { // Japanese PC-98 version
		0,		// cave voice 0
		// "私達は空を眺め、地上を眺め、海を眺め、こう思います："
		"\x8E\x84\x92\x42\x82\xCD\x8B\xF3\x82\xF0\x92\xAD\x82\xDF\x81\x41\x92n\x8F\xE3\x82\xF0\x92\xAD\x82\xDF\x81\x41\x8A\x43\x82\xF0\x92\xAD\x82\xDF\x81\x41\x82\xB1\x82\xA4\x8Ev\x82\xA2\x82\xDC\x82\xB7\x81\x46"
		// "この世に存在しているのは私達だけなのだろうか？"
		"\x82\xB1\x82\xCC\x90\xA2\x82\xC9\x91\xB6\x8D\xDD\x82\xB5\x82\xC4\x82\xA2\x82\xE9\x82\xCC\x82\xCD\x8E\x84\x92\x42\x82\xBE\x82\xAF\x82\xC8\x82\xCC\x82\xBE\x82\xEB\x82\xA4\x82\xA9\x81H"
	},
	{
		1,		// cave voice 1
		// "私達が存在する以前のはるか昔、"
		"\x8E\x84\x92\x42\x82\xAA\x91\xB6\x8D\xDD\x82\xB7\x82\xE9\x88\xC8\x91O\x82\xCC\x82\xCD\x82\xE9\x82\xA9\x90\xCC\x81\x41"
		// "人間達が地球を支配していました。"
		"\x90l\x8A\xD4\x92\x42\x82\xAA\x92n\x8B\x85\x82\xF0\x8Ex\x94z\x82\xB5\x82\xC4\x82\xA2\x82\xDC\x82\xB5\x82\xBD\x81\x42"
	},
	{
		2,		// cave voice 2
		// "彼らはあらゆる努力を払い、驚異的な物を創造しました。"
		"\x94\xDE\x82\xE7\x82\xCD\x82\xA0\x82\xE7\x82\xE4\x82\xE9\x93w\x97\xCD\x82\xF0\x95\xA5\x82\xA2\x81\x41\x8B\xC1\x88\xD9\x93I\x82\xC8\x95\xA8\x82\xF0\x91n\x91\xA2\x82\xB5\x82\xDC\x82\xB5\x82\xBD\x81\x42"
	},
	{
		3,		// cave voice 3
		// "彼らは、飛行の秘密や、幸福の秘密、他にも、"
		"\x94\xDE\x82\xE7\x82\xCD\x81\x41\x94\xF2\x8Ds\x82\xCC\x94\xE9\x96\xA7\x82\xE2\x81\x41\x8DK\x95\x9F\x82\xCC\x94\xE9\x96\xA7\x81\x41\x91\xBC\x82\xC9\x82\xE0\x81\x41"
		// "我々の想像を越えた多くの秘密を知っていたのです。"
		"\x89\xE4\x81X\x82\xCC\x91z\x91\x9C\x82\xF0\x89z\x82\xA6\x82\xBD\x91\xBD\x82\xAD\x82\xCC\x94\xE9\x96\xA7\x82\xF0\x92m\x82\xC1\x82\xC4\x82\xA2\x82\xBD\x82\xCC\x82\xC5\x82\xB7\x81\x42"
	} }
};

const IntroDialogue introDialogueCave2[][3] = {
	{ { // English
		4,		// cave voice 4
		"The humans also knew the Secret of Life, and they "
		"used it to give us the Four Great Gifts:"
	},
	{
		5,		// cave voice 5
		"Thinking minds, feeling hearts, speaking mouths, and "
		"reaching hands."
	},
	{
		6,		// cave voice 6
		"We are their children."
	} },
	// -----------------------------------------------------
	{ { // German
		4,		// cave voice 4
		"Au$erdem kannten die Menschen das Geheimnis des "
		"Lebens. Und sie nutzten es, um uns die vier gro$en "
		"Geschenke zu geben -"
	},
	{
		5,		// cave voice 5
		"den denkenden Geist, das f\201hlende Herz, den "
		"sprechenden Mund und die greifende Hand."
	},
	{
		6,		// cave voice 6
		"Wir sind ihre Kinder."
	} },
	// -----------------------------------------------------
	{ { // Italian fan translation
		4,		// cave voice 4
		"Gli Umani conoscevano anche il Segreto della Vita, "
		"e lo usarono per darci i Quattro Grandi Doni:"

	},
	{
		5,		// cave voice 5
		"Il pensiero, le emozioni, la parola e la manualit\205."

	},
	{
		6,		// cave voice 6
		"Siamo i loro figli."
	} },
	{ { // Fench fan translation
		4,		// cave voice 4
		"Les humains connaissaient aussi le secret de "
		"la Vie, et l'utilis\212rent pour nous offrir "
		"Quatres Dons:"
	},
	{
		5,		// cave voice 5
		"La Pens\202e, les Sentiments, la Parole et, "
		"l'Habilet\202 manuelle."
	},
	{
		6,		// cave voice 6
		"Nous sommes leurs enfants."
	} },
	// -----------------------------------------------------
	{ { // Japanese PC-98 version
		4,		// cave voice 4
		// "人間達は生活の秘密も知っていました。　"
		"\x90l\x8A\xD4\x92\x42\x82\xCD\x90\xB6\x8A\x88\x82\xCC\x94\xE9\x96\xA7\x82\xE0\x92m\x82\xC1\x82\xC4\x82\xA2\x82\xDC\x82\xB5\x82\xBD\x81\x42\x81@"
		// "彼らはそれを使って、私達に４つの偉大な贈り物・・・"
		"\x94\xDE\x82\xE7\x82\xCD\x82\xBB\x82\xEA\x82\xF0\x8Eg\x82\xC1\x82\xC4\x81\x41\x8E\x84\x92\x42\x82\xC9\x82S\x82\xC2\x82\xCC\x88\xCC\x91\xE5\x82\xC8\x91\xA1\x82\xE8\x95\xA8\x81\x45\x81\x45\x81\x45"
	},
	{
		5,		// cave voice 5
		// "「考える心や、感じる心、言葉を言う口や、"
		"\x81u\x8Dl\x82\xA6\x82\xE9\x90S\x82\xE2\x81\x41\x8A\xB4\x82\xB6\x82\xE9\x90S\x81\x41\x8C\xBE\x97t\x82\xF0\x8C\xBE\x82\xA4\x8C\xFB\x82\xE2\x81\x41"
		// "物をつかむ手」を授けました。"
		"\x95\xA8\x82\xF0\x82\xC2\x82\xA9\x82\xDE\x8E\xE8\x81v\x82\xF0\x8E\xF6\x82\xAF\x82\xDC\x82\xB5\x82\xBD\x81\x42"
	},
	{
		6,		// cave voice 6
		// "私達は彼らの子供なのです。"
		"\x8E\x84\x92\x42\x82\xCD\x94\xDE\x82\xE7\x82\xCC\x8Eq\x8B\x9F\x82\xC8\x82\xCC\x82\xC5\x82\xB7\x81\x42"
	} }
};

const IntroDialogue introDialogueCave3[][3] = {
	{ { // English
		7,		// cave voice 7
		"They taught us how to use our hands, and how to "
		"speak."
	},
	{
		8,		// cave voice 8
		"They showed us the joy of using our minds."
	},
	{
		9,		// cave voice 9
		"They loved us, and when we were ready, they surely "
		"would have given us the Secret of Happiness."
	} },
	// -----------------------------------------------------
	{ { // German
		7,		// cave voice 7
		"Sie lehrten uns zu sprechen und unsere H\204nde zu "
		"benutzen."
	},
	{
		8,		// cave voice 8
		"Sie zeigten uns die Freude am Denken."
	},
	{
		9,		// cave voice 9
		"Sie liebten uns, und w\204ren wir bereit gewesen, "
		"h\204tten sie uns sicherlich das Geheimnis der "
		"Fr\224hlichkeit offenbart."
	} },
	// -----------------------------------------------------
	{ { // Italian fan translation
		7,		// cave voice 7
		"Ci insegnarono come usare le mani e come parlare. "

	},
	{
		8,		// cave voice 8
		"Ci mostrarono le gioie che l'uso della mente "
		"pu\225 dare. "
	},
	{
		9,		// cave voice 9
		"Ci amarono, ed una volta pronti, ci avrebbero "
		"sicuramente svelato il Segreto della Felicit\205."

	} },
	{ { // French fan translation
		7,		// cave voice 7
		"Ils nous apprirent \205 utiliser nos mains, et \205 "
		"parler."
	},
	{
		8,		// cave voice 8
		"Ils nous apprirent les joies de la pens\202e."
	},
	{
		9,		// cave voice 9
		"Ils nous aim\212rent, et le moment venu, ils "
		"nous auraient s\223rement livr\202 le Secret du "
		"Bonheur."
	} },
	// -----------------------------------------------------
	{ { // Japanese PC-98 version
		7,		// cave voice 7
		// "彼らは手の使い方や、話の仕方を教えてくれました。"
		"\x94\xDE\x82\xE7\x82\xCD\x8E\xE8\x82\xCC\x8Eg\x82\xA2\x95\xFB\x82\xE2\x81\x41\x98\x62\x82\xCC\x8E\x64\x95\xFB\x82\xF0\x8B\xB3\x82\xA6\x82\xC4\x82\xAD\x82\xEA\x82\xDC\x82\xB5\x82\xBD\x81\x42"
	},
	{
		8,		// cave voice 8
		// "心で喜ぶことを教えてくれました。"
		"\x90S\x82\xC5\x8A\xEC\x82\xD4\x82\xB1\x82\xC6\x82\xF0\x8B\xB3\x82\xA6\x82\xC4\x82\xAD\x82\xEA\x82\xDC\x82\xB5\x82\xBD\x81\x42"
	},
	{
		9,		// cave voice 9
		// "彼らは私達を可愛がりました。　"
		"\x94\xDE\x82\xE7\x82\xCD\x8E\x84\x92\x42\x82\xF0\x89\xC2\x88\xA4\x82\xAA\x82\xE8\x82\xDC\x82\xB5\x82\xBD\x81\x42\x81@"
		// "私達さえその気があったら、"
		"\x8E\x84\x92\x42\x82\xB3\x82\xA6\x82\xBB\x82\xCC\x8B\x43\x82\xAA\x82\xA0\x82\xC1\x82\xBD\x82\xE7\x81\x41"
		// "幸福の秘密も教えてくれていたことでしょう。"
		"\x8DK\x95\x9F\x82\xCC\x94\xE9\x96\xA7\x82\xE0\x8B\xB3\x82\xA6\x82\xC4\x82\xAD\x82\xEA\x82\xC4\x82\xA2\x82\xBD\x82\xB1\x82\xC6\x82\xC5\x82\xB5\x82\xE5\x82\xA4\x81\x42"
	}
	}
};

const IntroDialogue introDialogueCave4[][4] = {
	{ { // English
		10,		// cave voice 10
		"And now we see the sky, the land, and the water that "
		"we are heirs to, and we wonder: why did they leave?"
	},
	{
		11,		// cave voice 11
		"Do they live still, in the stars? In the oceans "
		"depths? In the wind?"
	},
	{
		12,		// cave voice 12
		"We wonder, was their fate good or evil?"
	},
	{
		13,		// cave voice 13
		"And will we also share the same fate one day?"
	} },
	// -----------------------------------------------------
	{ { // German
		10,		// cave voice 10
		"Und nun sehen wir den Himmel, das Land und die "
		"Seen - unser Erbe. Und wir fragen uns - warum "
		"verschwanden sie?"
	},
	{
		11,		// cave voice 11
		"Leben sie noch in den Sternen? In den Tiefen des "
		"Ozeans? Im Wind?"
	},
	{
		12,		// cave voice 12
		"Wir fragen uns - war ihr Schicksal gut oder b\224se?"
	},
	{
		13,		// cave voice 13
		"Und wird uns eines Tages das gleiche Schicksal "
		"ereilen?"
	} },
	// -----------------------------------------------------
	{ { // Italian fan translation
		10,		// cave voice 10
		"Ed ora che guardiamo il cielo, la terra e l'acqua "
		"che abbiamo ereditato, pensiamo: Perch\202 partirono?"

	},
	{
		11,		// cave voice 11
		"Vivono ancora, nelle stelle? Nelle profondit\205 "
		"dell'oceano? Nel vento?"
	},
	{
		12,		// cave voice 12
		"Ci domandiamo, il loro destino fu felice o nefasto?"
	},
	{
		13,		// cave voice 13
		"E un giorno, condivideremo anche noi lo stesso "
		"destino?"
	} },
	{ { // French fan translation
		10,		// cave voice 10
		"Aujourd'hui nous voyons le ciel, les terres, et "
		"l'eau dont nous sommes les h\202ritiers, Et "
		"nous nous demandons: pourquoi sont-ils partis?" // Partis \205 la ligne ?????
	},
	{
		11,		// cave voice 11
		"Vivent-ils encore, dans les \202toiles? dans les "
		"profondeurs des oc\202ans? dans le vent?"
	},
	{
		12,		// cave voice 12
		"Nous nous demandons: leur destin f\223t-il bon "
		"ou mauvais?"
	},
	{
		13,		// cave voice 13
		"Et aurons-nous un jour un sort identique?"
	} },
	// -----------------------------------------------------
	{ { // Japanese PC-98 version
		10,		// cave voice 10
		// "今こうして、自分達に受け継がれた空を眺め、地上を眺め、"
		"\x8D\xA1\x82\xB1\x82\xA4\x82\xB5\x82\xC4\x81\x41\x8E\xA9\x95\xAA\x92\x42\x82\xC9\x8E\xF3\x82\xAF\x8Cp\x82\xAA\x82\xEA\x82\xBD\x8B\xF3\x82\xF0\x92\xAD\x82\xDF\x81\x41\x92n\x8F\xE3\x82\xF0\x92\xAD\x82\xDF\x81\x41"
		// "海を眺めながら、こう思うのです："
		"\x8A\x43\x82\xF0\x92\xAD\x82\xDF\x82\xC8\x82\xAA\x82\xE7\x81\x41\x82\xB1\x82\xA4\x8Ev\x82\xA4\x82\xCC\x82\xC5\x82\xB7\x81\x46"
		// "彼らはなぜ去ってしまったのだろうか？"
		"\x94\xDE\x82\xE7\x82\xCD\x82\xC8\x82\xBA\x8B\x8E\x82\xC1\x82\xC4\x82\xB5\x82\xDC\x82\xC1\x82\xBD\x82\xCC\x82\xBE\x82\xEB\x82\xA4\x82\xA9\x81H"
	},
	{
		11,		// cave voice 11
		// "まだ生きているのだろうか？　星達の中で？　"
		"\x82\xDC\x82\xBE\x90\xB6\x82\xAB\x82\xC4\x82\xA2\x82\xE9\x82\xCC\x82\xBE\x82\xEB\x82\xA4\x82\xA9\x81H\x81@\x90\xAF\x92\x42\x82\xCC\x92\x86\x82\xC5\x81H\x81@"
		// "大海の底で？　風の中で？"
		"\x91\xE5\x8A\x43\x82\xCC\x92\xEA\x82\xC5\x81H\x81@\x95\x97\x82\xCC\x92\x86\x82\xC5\x81H"
	},
	{
		12,		// cave voice 12
		// "彼らは幸運に導かれたのだろうか？"
		"\x94\xDE\x82\xE7\x82\xCD\x8DK\x89^\x82\xC9\x93\xB1\x82\xA9\x82\xEA\x82\xBD\x82\xCC\x82\xBE\x82\xEB\x82\xA4\x82\xA9\x81H"
	},
	{
		13,		// cave voice 13
		// "それとも悲運に導かれたのだろうか？　そして私達も、"
		"\x82\xBB\x82\xEA\x82\xC6\x82\xE0\x94\xDF\x89^\x82\xC9\x93\xB1\x82\xA9\x82\xEA\x82\xBD\x82\xCC\x82\xBE\x82\xEB\x82\xA4\x82\xA9\x81H\x81@\x82\xBB\x82\xB5\x82\xC4\x8E\x84\x92\x42\x82\xE0\x81\x41"
		// "ある日同じ運命に導かれるのだろうか？"
		"\x82\xA0\x82\xE9\x93\xFA\x93\xAF\x82\xB6\x89^\x96\xBD\x82\xC9\x93\xB1\x82\xA9\x82\xEA\x82\xE9\x82\xCC\x82\xBE\x82\xEB\x82\xA4\x82\xA9\x81H"
	}
	}
};

const IntroCredit creditsValley[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Producer"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Produzent"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Produttore"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Producteur"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Producer"},
	{Common::UNK_LANG, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Executive Producer"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Ausf\201hrender Produzent"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Produttore Esecutivo"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Producteur Executif"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Executive Producer"},
	{Common::UNK_LANG, kITECreditsAny, kITECreditsText, "Robert McNally"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsHeader, "2nd Executive Producer"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsHeader, "Publisher"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsHeader, "Herausgeber"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Editore"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Editeur"},
	{Common::JA_JPN, kITECreditsNotWyrmKeep, kITECreditsHeader, "Publisher"},
	{Common::UNK_LANG, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"}
};

const IntroCredit creditsTreeHouse1[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Game Design"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Spielentwurf"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Progetto"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Conception"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Game Design"},
	{Common::UNK_LANG, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "and Carolly Hauksdottir"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "und Carolly Hauksdottir"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "e Carolly Hauksdottir"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "et Carolly Hauksdottir"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "and Carolly Hauksdottir"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Screenplay and Dialog"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Robert Leh, Len Wein, and Bill Rotsler"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Geschichte und Dialoge"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Robert Leh, Len Wein und Bill Rotsler"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Sceneggiatura e Dialoghi"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Robert Leh, Len Wein e Bill Rotsler"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Sc\202nario et Dialogues"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Robert Leh, Len Wein et Bill Rotsler"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Screenplay and Dialog"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Robert Leh, Len Wein"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "and Bill Rotsler"}
};

const IntroCredit creditsTreeHouse2[] = {
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsHeader, "Art Direction"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Art"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Grafiken"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Grafica"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Graphismes"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Art"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsText, "Ed Lacabanne, Glenn Price, April Lee,"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::JA_JPN, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price,"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsText, "Lisa Sample, Brian Dowrick, Reed Waller,"},
	{Common::EN_ANY, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey and Talin"},
	{Common::DE_DEU, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey und Talin"},
	{Common::IT_ITA, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey e Talin"},
	{Common::FR_FRA, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey et Talin"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsText, "Lisa Iennaco, Brian Dowrick, Reed"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsText, "Waller, Allison Hershey and Talin"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Waller, Allison Hershey und Talin"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Waller, Allison Hershey e Talin"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Waller, Allison Hershey et Talin"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "April Lee, Lisa Iennaco,"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Brian Dowrick, Reed Waller,"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Allison Hershey and Talin"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsHeader, "Art Direction"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsHeader, "Grafische Leitung"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Direzione Grafica"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Directeur Artistique"},
	{Common::JA_JPN, kITECreditsNotWyrmKeep, kITECreditsHeader, "Art Direction"},
	{Common::UNK_LANG, kITECreditsNotWyrmKeep, kITECreditsText, "Allison Hershey"}
};

const IntroCredit creditsFairePath1[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Programming"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Programmiert von"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Programmazione"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Programmeurs"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Programming (original)"},
	{Common::UNK_LANG, kITECreditsAny, kITECreditsText, "Talin, Walter Hochbrueckner,"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Joe Burks and Robert Wiggins"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Joe Burks und Robert Wiggins"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Joe Burks e Robert Wiggins"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Joe Burks et Robert Wiggins"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Joe Burks and Robert Wiggins"},
	{Common::EN_ANY, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsHeader, "Additional Programming"},
	{Common::FR_FRA, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsHeader, "Programmeur Additionnel"},
	{Common::EN_ANY, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsText, "John Bolton"},
	{Common::FR_FRA, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsText, "John Bolton"},
	{Common::UNK_LANG, kITECreditsMac, kITECreditsHeader, "Macintosh Version"},
	{Common::UNK_LANG, kITECreditsMac, kITECreditsText, "Michael McNally and Robert McNally"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Music and Sound"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Musik und Sound"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Musica e Sonoro"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Musique et Sons"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Music and Sound (original)"},
	{Common::UNK_LANG, kITECreditsAny, kITECreditsText, "Matt Nathan"}
};

const IntroCredit creditsFairePath2[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Directed by"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Regie"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Regia"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Dirig\202 par"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Directed by"},
	{Common::UNK_LANG, kITECreditsAny, kITECreditsText, "Talin"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Traduction Francaise"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Ryfatwork, Momo le Yetty et Darknior"}
};

const IntroCredit creditsTent[6] = {
	{Common::JA_JPN, kITECreditsPC98, kITECreditsHeader, "NEC PC-9821 version by"},
	{Common::JA_JPN, kITECreditsPC98, kITECreditsText, "STARCRAFT,Inc."},
	{Common::JA_JPN, kITECreditsPC98, kITECreditsHeader, "Programming"},
	{Common::JA_JPN, kITECreditsPC98, kITECreditsText, "Toshio Sato"},
	{Common::JA_JPN, kITECreditsPC98, kITECreditsHeader, "Music and Sound"},
	{Common::JA_JPN, kITECreditsPC98, kITECreditsText, "Takeshi Abo"}
};

} // End of namespace Saga
