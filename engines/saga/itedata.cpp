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

byte ITE_ActorECSSpeechColor[ITE_ACTORCOUNT] = {
	kITEECSColorBrightWhite,
	8,
	5,
	0xc,
	0xf,
	5,
	0xf,
	0xf,
	0xf,
	0xf,
	5,
	0xa,
	0,
	0,
	8,
	0xe,
	0,
	0,
	0,
	0,
	0,
	0,
	0xe,
	0xc,
	0xe,
	0xd,
	0,
	0,
	0,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	8,
	8,
	8,
	8,
	0xe,
	0xe,
	0xe,
	0xe,
	0xc,
	0xc,
	0xc,
	0,
	0,
	8,
	8,
	8,
	0xc,
	0,
	0xa,
	0xe,
	0xa,
	0,
	0,
	0,
	0,
	0,
	0,
	0xb,
	0x9,
	0x9,
	8,
	0xa,
	0xc,
	0xf,
	0x9,
	0xa,
	0xa,
	0xc,
	0xf,
	0xf,
	0xa,
	8,
	0xa,
	0x5,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0x5,
	0xc,
	0xc,
	0x9,
	0xc,
	0x9,
	0xc,
	0xc,
	0,
	0,
	0,
	0xc,
	0x5,
	0xe,
	0xa,
	0,
	0,
	0,
	0,
	0xe,
	0xa,
	kITEECSColorBrightWhite,
	0xc,
	0xf,
	0xf,
	0x9,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	0x9,
	0x9,
	0,
	0,
	0,
	0,
	0,
	0xe,
	0xc,
	0xc,
	0xe,
	0x5,
	kITEECSColorBrightWhite,
	kITEECSColorBrightWhite,
	0xe,
	0xe,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0xc,
	0,
	0xc,
	kITEECSColorBrightWhite,
	8,
	5,
	0x8,
	0,
	0xf,
	5,
	0xe,
	0xa,
	0xc,
	0xc,
	0xa,
	0xe,
	0xd,
	0xd,
	0xe,
	0xe,
	8,
	8,
	8,
	8,
	8,
	8,
	0,
	0xe,
	0xc,
	0x9,
	0xc,
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
	// English
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
		"Cargando una partida guardada"
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
	},
	// Russian IHNM & ITE fan translations
	{
		// "Идти", "Осмотреть", "Взять", "Говорить", "Открыть",
		"\xC8\xE4\xF2\xE8", "\xCE\xF1\xEC\xEE\xF2\xF0\xE5\xF2\xFC", "\xC2\xE7\xFF\xF2\xFC", "\xC3\xEE\xE2\xEE\xF0\xE8\xF2\xFC", "\xCE\xF2\xEA\xF0\xFB\xF2\xFC",
		// "Закрыть", "Применить", "Дать", "Настройки", "Тест",
		"\xC7\xE0\xEA\xF0\xFB\xF2\xFC", "\xCF\xF0\xE8\xEC\xE5\xED\xE8\xF2\xFC", "\xC4\xE0\xF2\xFC", "\xCD\xE0\xF1\xF2\xF0\xEE\xE9\xEA\xE8", "\xD2\xE5\xF1\xF2",
		// "Demo", "Помощь", "Выйти из игры", "БЫСТ", "МЕДЛ",
		"Demo", "\xCF\xEE\xEC\xEE\xF9\xFC", "\xC2\xFB\xE9\xF2\xE8 \xE8\xE7 \xE8\xE3\xF0\xFB", "\xC1\xDB\xD1\xD2", "\xCC\xC5\xC4\xCB",
		// "ВКЛ", "ВЫКЛ", "Продолжить", "Чтение", "Запись",
		"\xC2\xCA\xCB", "\xC2\xDB\xCA\xCB", "\xCF\xF0\xEE\xE4\xEE\xEB\xE6\xE8\xF2\xFC", "\xD7\xF2\xE5\xED\xE8\xE5", "\xC7\xE0\xEF\xE8\xF1\xFC",
		// "Настройки игры", "Субтитры", "Музыка", "Звук", "Отмена",
		"\xCD\xE0\xF1\xF2\xF0\xEE\xE9\xEA\xE8 \xE8\xE3\xF0\xFB", "\xD1\xF3\xE1\xF2\xE8\xF2\xF0\xFB", "\xCC\xF3\xE7\xFB\xEA\xE0", "\xC7\xE2\xF3\xEA", "\xCE\xF2\xEC\xE5\xED\xE0",
		// "Выход", "OK", "СРД", "Мышь", "10%",
		"\xC2\xFB\xF5\xEE\xE4", "OK", "\xD1\xD0\xC4", "\xCC\xFB\xF8\xFC", "10%",
		"20%", "30%", "40%", "50%", "60%",
		// "70%", "80%", "90%", "МАКС", "Выйти из игры?",
		"70%", "80%", "90%", "\xCC\xC0\xCA\xD1", "\xC2\xFB\xE9\xF2\xE8 \xE8\xE7 \xE8\xE3\xF0\xFB?",
		// "Игра загружена!",
		"\xC8\xE3\xF0\xE0 \xE7\xE0\xE3\xF0\xF3\xE6\xE5\xED\xE0!",
		// "Введите имя записи",
		"\xC2\xE2\xE5\xE4\xE8\xF2\xE5 \xE8\xEC\xFF \xE7\xE0\xEF\xE8\xF1\xE8",
		// "Дать -> %s -> %s",
		"\xC4\xE0\xF2\xFC -> %s -> %s",
		// "Использовать %s >> %s",
		"\xC8\xF1\xEF\xEE\xEB\xFC\xE7\xEE\xE2\xE0\xF2\xFC %s >> %s", //two different lines for IHNM & ITE; check in saga.cpp, SagaEngine::getTextString()
		// "[Новая запись]",
		"[\xCD\xEE\xE2\xE0\xFF \xE7\xE0\xEF\xE8\xF1\xFC]",
		// "Я не могу это взять.",
		"\xDF \xED\xE5 \xEC\xEE\xE3\xF3 \xFD\xF2\xEE \xE2\xE7\xFF\xF2\xFC.",
		// "Ничего особенного.",
		"\xCD\xE8\xF7\xE5\xE3\xEE \xEE\xF1\xEE\xE1\xE5\xED\xED\xEE\xE3\xEE.",
		// "Здесь нечего открывать.",
		"\xC7\xE4\xE5\xF1\xFC \xED\xE5\xF7\xE5\xE3\xEE \xEE\xF2\xEA\xF0\xFB\xE2\xE0\xF2\xFC.",
		// "Здесь нечего закрыть.",
		"\xC7\xE4\xE5\xF1\xFC \xED\xE5\xF7\xE5\xE3\xEE \xE7\xE0\xEA\xF0\xFB\xF2\xFC.",
		// "Я не знаю, как это сделать.",
		"\xDF \xED\xE5 \xE7\xED\xE0\xFE, \xEA\xE0\xEA \xFD\xF2\xEE \xF1\xE4\xE5\xEB\xE0\xF2\xFC.",
		// "Субтитры",
		"\xD1\xF3\xE1\xF2\xE8\xF2\xF0\xFB",
		// "Что скажет Риф?",
		"\xD7\xF2\xEE \xF1\xEA\xE0\xE6\xE5\xF2 \xD0\xE8\xF4?",
		// " Загрузка "
		"       \xC7\xE0\xE3\xF0\xF3\xE7\xEA\xE0     "
	},
	{
		// Hebrew fan-translation
		"\xEC\xEA \xE0\xEC", "\xE4\xE1\xE8 \xE0\xEC", "\xE4\xF8\xED", "\xE3\xE1\xF8 \xF2\xED", "\xF4\xFA\xE7",
		"\xF1\xE2\xE5\xF8", "\xE4\xF4\xF2\xEC", "\xFA\xEF", "\xE0\xF4\xF9\xF8\xE5\xE9\xE5\xFA", "\xE1\xE3\xE9\xF7\xE4",
		"\xF0\xE9\xF1\xE9\xE5\xEF", "\xF2\xE6\xF8\xE4", "\xE9\xF6\xE9\xE0\xE4 \xEE\xE4\xEE\xF9\xE7\xF7", "\xEE\xE4\xE9\xF8\xE4", "\xE0\xE8\xE9\xFA",
		"\xF4\xF2\xE9\xEC", "\xEB\xE1\xE5\xE9", "\xE4\xEE\xF9\xEA \xEE\xF9\xE7\xF7", "\xE8\xF2\xE9\xF0\xE4", "\xF9\xEE\xE9\xF8\xE4",
		"\xE0\xF4\xF9\xF8\xE5\xE9\xE5\xFA \xEE\xF9\xE7\xF7", "\xEE\xE4\xE9\xF8\xE5\xFA \xF7\xF8\xE9\xE0\xE4", "\xEE\xE5\xE6\xE9\xF7\xE4", "\xF6\xEC\xE9\xEC", "\xE1\xE9\xE8\xE5\xEC",
		"\xE9\xF6\xE9\xE0\xE4", "\xE1\xF1\xE3\xF8", "\xE1\xE9\xF0\xE5\xF0\xE9", "\xEC\xE7\xE9\xF6\xE4", "10%",
		"20%", "30%", "40%", "50%", "60%",
		"70%", "80%", "90%", "\xEE\xE9\xF8\xE1\xE9", "\xEC\xF6\xE0\xFA \xEE\xE4\xEE\xF9\xE7\xF7?",
		"\xF0\xE8\xF2\xEF \xE1\xE4\xF6\xEC\xE7\xE4!", "\xE4\xE6\xE9\xF0\xE5 \xF9\xED \xEC\xEE\xF9\xE7\xF7 \xE4\xF9\xEE\xE5\xF8", "\xFA\xEF %s \xEC%s", "\xE4\xF4\xF2\xEC %s \xF2\xEC %s",
		"[\xEE\xF9\xE7\xF7 \xF9\xEE\xE5\xF8 \xE7\xE3\xF9]",
		"\xE0\xF0\xE9 \xEC\xE0 \xE9\xEB\xE5\xEC \xEC\xE4\xF8\xE9\xED \xE0\xFA \xE6\xE4.",
		"\xE0\xF0\xE9 \xEC\xE0 \xF8\xE5\xE0\xE4 \xF9\xE5\xED \xE3\xE1\xF8 \xEE\xE9\xE5\xE7\xE3 \xE1\xE6\xE4.",
		"\xE0\xE9\xEF \xEE\xF7\xE5\xED \xEC\xF4\xFA\xE5\xE7 \xE0\xFA \xE6\xE4.",
		"\xE0\xE9\xEF \xF4\xFA\xE7 \xEC\xF1\xE2\xE5\xF8.",
		"\xE0\xF0\xE9 \xEC\xE0 \xE9\xE5\xE3\xF2 \xEC\xF2\xF9\xE5\xFA \xE0\xFA \xE6\xE4.",
		"\xE4\xF6\xE2\xFA \xE3\xE5-\xF9\xE9\xE7",
		"\xEE\xE4 \xFA\xE2\xE5\xE1\xFA\xE5 \xF9\xEC \xF8\xE9\xF3?",
		"\xE8\xF2\xE9\xF0\xFA \xEE\xF9\xE7\xF7 \xF9\xEE\xE5\xF8"
	},
	// Chinese
	{
		// Note that the "Load Successful!" string is never used in ScummVM
		"\xa8\xab\xa6\x56" /* 走向, Walk to */,
		"\xac\x64\xac\xdd" /* 查看, Look at */,
		"\xae\xb3\xb0\x5f" /* 拿起, Pick up */,
		"\xa5\xe6\xbd\xcd" /* 交談, Talk to */,
		"\xa5\xb4\xb6\x7d" /* 打開, Open */,
		"\xc3\xf6\xb3\xac" /* 關閉, Close */,
		"\xa8\xcf\xa5\xce" /* 使用, Use */,
		"\xb5\xb9\xbb\x50" /* 給與, Give */,
		"\xbf\xef\xb6\xb5" /* 選項, Options */,
		"\xb4\xfa\xb8\xd5" /* 測試, Test */,
		"\xae\x69\xa5\xdc" /* 展示, Demo */,
		"\xa8\x44\xa7\x55" /* 求助, Help */,
		"\xb5\xb2\xa7\xf4" /* 結束, Quit game */,
		"\xa7\xd6\xb3\x74" /* 快速, Fast */,
		"\xbd\x77\xba\x43" /* 緩慢, Slow */,
		"\xb6\x7d" /* 開, On */,
		"\xc3\xf6" /* 關, Off */,
		"\xc4\x7e\xc4\xf2\xb9\x43\xc0\xb8" /* 繼續遊戲, Continue Playing */,
		"\xb8\xfc\xa4\x4a" /* 載入, Load */,
		"\xc0\x78\xa6\x73" /* 儲存, Save */,
		"\xb9\x43\xc0\xb8\xbf\xef\xb6\xb5" /* 遊戲選項, Game Options */,
		"\xb0\x54\xae\xa7\xb3\x74\xab\xd7" /* 訊息速度, Reading Speed */,
		"\xad\xb5\xbc\xd6" /* 音樂, Music */,
		"\xad\xb5\xae\xc4" /* 音效, Sound */,
		"\xa8\xfa\xae\xf8" /* 取消, Cancel */,
		"\xb5\xb2\xa7\xf4" /* 結束, Quit */,
		"\xa7\xb9\xb2\xa6" /* 完畢, OK */,
		"\xa4\xa4\xb5\xa5" /* 中等, Mid */,
		"\xba\x56\xc1\xe4" /* 敲鍵, Click */,
		"\xa2\xb0\xa2\xaf\xa2\x48" /* １０％ */,
		"\xa2\xb1\xa2\xaf\xa2\x48" /* ２０％ */,
		"\xa2\xb2\xa2\xaf\xa2\x48" /* ３０％ */,
		"\xa2\xb3\xa2\xaf\xa2\x48" /* ４０％ */,
		"\xa2\xb4\xa2\xaf\xa2\x48" /* ５０％ */,
		"\xa2\xb5\xa2\xaf\xa2\x48" /* ６０％ */,
		"\xa2\xb6\xa2\xaf\xa2\x48" /* ７０％ */,
		"\xa2\xb7\xa2\xaf\xa2\x48" /* ８０％ */,
		"\xa2\xb8\xa2\xaf\xa2\x48" /* ９０％ */,
		"\xb3\xcc\xa4\x6a" /* 最大, Max */,
		"\xad\x6e\xb5\xb2\xa7\xf4\xb9\x43\xc0\xb8\xb6\xdc\x3f" /* 要結束遊戲嗎?; Quit the Game? */,
		"\xb8\xfc\xa4\x4a\xa6\xa8\xa5\x5c" /* 載入成功, Load Successful! */,
		"\xbd\xd0\xbf\xe9\xa4\x4a\xa6\x73\xc0\xc9\xa6\x57\xba\xd9" /* 請輸入存檔名稱, Enter Save Game Name */,
		"\xB1N%s\xB5\xB9\xBBP%s", /* 將%s給與%s; Give %s to %s */
		"\xB1\x4e%s\xA8\xCF\xA5\xCE\xA6\x62%s", /* 將%s使用在%s; Use %s with %s */
		"[New Save Game]",
		"\xA7\xDA\xB5\x4C\xAA\x6B\xAE\xB3\xB0\x5F\xA8\xBA\xAD\xD3\xAA\x46\xA6\xE8\xA1\x45", /* 我無法拿起那個東西‧; I can't pick that up. */
		"\xA7\xDA\xAC\xDD\xA4\xA3\xA5\x58\xA6\xB3\xA4\xB0\xBB\xF2\xAF\x53\xAE\xED\xA4\xA7\xB3\x42\xA1\x45", /* 我看不出有什麼特殊之處‧; I see nothing special about it.  */
		"\xA8\x53\xA6\xB3\xA6\x61\xA4\xE8\xA5\x69\xA5\x48\xA5\xB4\xB6\x7D\xA1\x45", /* 沒有地方可以打開‧; There's no place to open it.  */
		"\xA8\x53\xA6\xB3\xA5\x69\xA5\x48\xC3\xF6\xB3\xAC\xAA\xBA\xA6\x61\xA4\xE8\xA1\x45", /* 沒有可以關閉的地方‧; There's no opening to close. */
		"\xA7\xDA\xA4\xA3\xAA\xBE\xB9\x44\xAD\x6E\xAB\xE7\xBB\xF2\xB0\xB5\xB3\x6F\xA5\xF3\xA8\xC6\xA1\x45", /* 我不知道要怎麼做這件事‧; I don't know how to do that.  */
		"Show Dialog", // String is not used in Chinese version
		"\xa7\x51\xa4\xd2\xaa\xba\xa6\x5e\xb5\xaa\xac\x4f\xa4\xb0\xbb\xf2\xa1\x48" /* 利夫的回答是什麼？; What is Rif's reply? */,
		"\xc5\xaa\xa8\xfa\xa6\x73\xc0\xc9" /* 讀取存檔; Loading a saved game */
	},
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
	// English
	{
		"screwdriver", "pliers", "c-clamp", "wood clamp", "level",
		"twine", "wood plane", "claw hammer", "tape measure", "hatchet",
		"shears", "ruler", "saw", "mallet", "paint brush"
	},
	// German
	{
		"Schraubendreher", "Zange", "Schraubzwinge", "Holzzwinge", "Wasserwaage",
		"Bindfaden", "Hobel", "Schusterhammer", "Bandma$", "Beil",
		"Schere", "Winkel", "S\204ge", "Hammer", "Pinsel"
	},
	// Italian
	{
		"cacciavite", "pinze", "morsa", "morsa da legno", "livella",
		"spago", "pialla", "martello", "metro a nastro", "accetta",
		"cesoie", "righello", "sega", "mazza", "pennello"
	},
	// Spanish
	{
		"", "", "", "", "",
		"", "", "", "", "",
		"", "", "", "", ""
	},
	// French
	{
		"tournevis", "pince", "\202tau \205 vis", "pince \205 bois", "niveau",
		"ficelle", "rabot \205 bois", "marteau", "m\212tre ruban", "hachette",
		"ciseaux", "r\212gle", "scie", "maillet", "pinceau"
	},
	// Japanese
	{
		// "ドライバー", "釘抜き", "Ｃ形クランプ", "木のクランプ", "レベル",
		"\x83h\x83\x89\x83\x43\x83o\x81[", "\x93\x42\x94\xB2\x82\xAB", "\x82\x62\x8C`\x83N\x83\x89\x83\x93\x83v", "\x96\xD8\x82\xCC\x83N\x83\x89\x83\x93\x83v", "\x83\x8C\x83x\x83\x8B",
		// "麻ヒモ", "かんな", "金づち", "巻尺", "斧",
		"\x96\x83\x83q\x83\x82", "\x82\xA9\x82\xF1\x82\xC8", "\x8B\xE0\x82\xC3\x82\xBF", "\x8A\xAA\x8E\xDA", "\x95\x80",
		// "ハサミ", "曲尺", "のこぎり", "木づち", "ペンキブラシ"
		"\x83n\x83T\x83~", "\x8B\xC8\x8E\xDA", "\x82\xCC\x82\xB1\x82\xAC\x82\xE8", "\x96\xD8\x82\xC3\x82\xBF", "\x83y\x83\x93\x83L\x83u\x83\x89\x83V"
	},
	// Russian
	{
		// "отвертка", "клещи", "струбцина", "деревянный зажим", "уровень",
		"\xEE\xF2\xE2\xE5\xF0\xF2\xEA\xE0", "\xEA\xEB\xE5\xF9\xE8", "\xF1\xF2\xF0\xF3\xE1\xF6\xE8\xED\xE0", "\xE4\xE5\xF0\xE5\xE2\xFF\xED\xED\xFB\xE9 \xE7\xE0\xE6\xE8\xEC", "\xF3\xF0\xEE\xE2\xE5\xED\xFC",
		// "шпагат", "рубанок", "молоток", "рулетка", "топорик",
		"\xF8\xEF\xE0\xE3\xE0\xF2", "\xF0\xF3\xE1\xE0\xED\xEE\xEA", "\xEC\xEE\xEB\xEE\xF2\xEE\xEA", "\xF0\xF3\xEB\xE5\xF2\xEA\xE0", "\xF2\xEE\xEF\xEE\xF0\xE8\xEA",
		// "ножницы", "линейка", "пила", "киянка", "кисть"
		"\xED\xEE\xE6\xED\xE8\xF6\xFB", "\xEB\xE8\xED\xE5\xE9\xEA\xE0", "\xEF\xE8\xEB\xE0", "\xEA\xE8\xFF\xED\xEA\xE0", "\xEA\xE8\xF1\xF2\xFC"
	},
	// Hebrew
	{
		"\xEE\xE1\xF8\xE2", "\xF6\xE1\xFA", "\xEB\xEC\xE9\xE1\xE4", "\xEE\xEC\xE7\xF6\xE9\xE9\xED", "\xF4\xEC\xF1",
		"\xE7\xE5\xE8 \xEE\xF9\xE9\xE7\xE4", "\xEE\xF7\xF6\xE5\xF2\xE4", "\xF4\xE8\xE9\xF9 \xF0\xE2\xF8\xE9\xED", "\xF1\xF8\xE8 \xEE\xE9\xE3\xE4", "\xF7\xF8\xE3\xE5\xED",
		"\xEE\xE6\xEE\xF8\xE4", "\xF1\xF8\xE2\xEC", "\xEE\xF1\xE5\xF8", "\xEE\xF7\xE1\xFA", "\xEE\xE1\xF8\xF9\xFA \xF6\xE1\xF2"
	},
	// Chinese
	{
		"\xc1\xb3\xb5\xb7\xb0\x5f\xa4\x6c" /* 螺絲起子, screwdriver */,
		"\xb9\x58\xa4\x6c" /* 鉗子, pliers */,
		"\xa2\xd1\xab\xac\xb9\x58" /* Ｃ型鉗, c-clamp */,
		"\xa4\xec\xb9\x58" /* 木鉗, wood clamp */,
		"\xa4\xf4\xa5\xad\xbb\xf6" /* 水平儀, level */,
		"\xb3\xc2\xbd\x75" /* 麻線, twine */,
		"\xa4\xec\xaa\x4f" /* 木板, wood plane */,
		"\xa9\xde\xb0\x76\xc2\xf1" /* 拔釘鎚, claw hammer */,
		"\xa5\xd6\xa4\xd8" /* 皮尺, tape measure */,
		"\xa9\xf2\xc0\x59" /* 斧頭, hatchet */,
		"\xb0\xc5\xa4\x4d" /* 剪刀, shears */,
		"\xaa\xbd\xa4\xd8" /* 直尺, ruler */,
		"\xbf\xf7\xa4\x6c" /* 鋸子, saw */,
		"\xa4\xec\xba\x6c" /* 木槌, mallet */,
		"\xaa\x6f\xba\xa3\xa8\xea" /* 油漆刷, paint brush */,
	},
};

// hints
const char *hintStr[][4] = {
	// English
	{
		"Check which pieces could fit in each corner first.",
		"Check which corner has the least number of pieces that can fit and start from there.",
		"Check each new corner and any new side for pieces that fit.",
		"I don't see anything out of place."
	},
	// German
	{
		"\232berpr\201fe zun\204chst, welche die Eckteile sein k\224nnten.",
		"Schau, in welche Ecke die wenigsten Teile passen, und fang dort an.",
		"Untersuche jede Ecke und jede Seite auf Teile, die dort passen k\224nnen.",
		"Ich sehe nichts an der falschen Stelle."
	},
	// Italian
	{
		"Controlla prima quali pezzi si inseriscono meglio in ogni angolo.",
		"Controlla quale angolo ha il minor numero di pezzi che combaciano, e parti da quello.",
		"Controlla ogni nuovo angolo e lato per ogni pezzo che combacia.",
		"Non vedo nulla fuori posto."
	},
	// Spanish
	{
		"",
		"",
		"",
		""
	},
	// French
	{
		"Trouve d'abord les pi\212ces qui correspondent aux coins.",
		"V\202rifie quel coin a le moins de pi\212ces qui correspondent et part de l\205.",
		"Evalue chaque coin et bord pour voir les pi\212ces qui collent.",
		"Je ne vois rien de mal plac\202."
	},
	// Japanese
	{
		// "まず、四隅に合うピースを\n捜してごらん。",
		"\x82\xDC\x82\xB8\x81\x41\x8El\x8B\xF7\x82\xC9\x8D\x87\x82\xA4\x83s\x81[\x83X\x82\xF0\n\x91{\x82\xB5\x82\xC4\x82\xB2\x82\xE7\x82\xF1\x81\x42",
		// "辺同士が合いそうなピースを\n捜してごらん。",
		"\x95\xD3\x93\xAF\x8Em\x82\xAA\x8D\x87\x82\xA2\x82\xBB\x82\xA4\x82\xC8\x83s\x81[\x83X\x82\xF0\n\x91{\x82\xB5\x82\xC4\x82\xB2\x82\xE7\x82\xF1\x81\x42",
		// "新しくできた空間と形が合う\nピースを捜してごらん。",
		"\x90V\x82\xB5\x82\xAD\x82\xC5\x82\xAB\x82\xBD\x8B\xF3\x8A\xD4\x82\xC6\x8C`\x82\xAA\x8D\x87\x82\xA4\n\x83s\x81[\x83X\x82\xF0\x91{\x82\xB5\x82\xC4\x82\xB2\x82\xE7\x82\xF1\x81\x42",
		// "置いたピースの位置は\nどれも合ってるよ。"
		"\x92u\x82\xA2\x82\xBD\x83s\x81[\x83X\x82\xCC\x88\xCA\x92u\x82\xCD\n\x82\xC7\x82\xEA\x82\xE0\x8D\x87\x82\xC1\x82\xC4\x82\xE9\x82\xE6\x81\x42"
	},
	// Russian
	{
		// "Сперва посмотри, какие фрагменты в какой угол можно поставить.",
		"\xD1\xEF\xE5\xF0\xE2\xE0 \xEF\xEE\xF1\xEC\xEE\xF2\xF0\xE8, \xEA\xE0\xEA\xE8\xE5 \xF4\xF0\xE0\xE3\xEC\xE5\xED\xF2\xFB \xE2 \xEA\xE0\xEA\xEE\xE9 \xF3\xE3\xEE\xEB \xEC\xEE\xE6\xED\xEE \xEF\xEE\xF1\xF2\xE0\xE2\xE8\xF2\xFC.",
		// "Посмотри, в какой угол подходит наименьшее количество фрагментов, и начни с него.",
		"\xCF\xEE\xF1\xEC\xEE\xF2\xF0\xE8, \xE2 \xEA\xE0\xEA\xEE\xE9 \xF3\xE3\xEE\xEB \xEF\xEE\xE4\xF5\xEE\xE4\xE8\xF2 \xED\xE0\xE8\xEC\xE5\xED\xFC\xF8\xE5\xE5 \xEA\xEE\xEB\xE8\xF7\xE5\xF1\xF2\xE2\xEE \xF4\xF0\xE0\xE3\xEC\xE5\xED\xF2\xEE\xE2, \xE8 \xED\xE0\xF7\xED\xE8 \xF1 \xED\xE5\xE3\xEE.",
		// "Смотри, какие фрагменты подойдут к новым углам и сторонам.",
		"\xD1\xEC\xEE\xF2\xF0\xE8, \xEA\xE0\xEA\xE8\xE5 \xF4\xF0\xE0\xE3\xEC\xE5\xED\xF2\xFB \xEF\xEE\xE4\xEE\xE9\xE4\xF3\xF2 \xEA \xED\xEE\xE2\xFB\xEC \xF3\xE3\xEB\xE0\xEC \xE8 \xF1\xF2\xEE\xF0\xEE\xED\xE0\xEC.",
		// "Не вижу, чтобы что-нибудь было не так."
		"\xCD\xE5 \xE2\xE8\xE6\xF3, \xF7\xF2\xEE\xE1\xFB \xF7\xF2\xEE-\xED\xE8\xE1\xF3\xE4\xFC \xE1\xFB\xEB\xEE \xED\xE5 \xF2\xE0\xEA."
	},
	// Hebrew
	{
		"\xE1\xE3\xF7\xE5 \xF7\xE5\xE3\xED \xE0\xE9\xEC\xE5 \xE7\xFA\xE9\xEB\xE5\xFA \xEE\xFA\xE0\xE9\xEE\xE5\xFA \xEC\xEB\xEC \xF4\xE9\xF0\xE4.",
		"\xE1\xE3\xF7\xE5 \xEC\xE0\xE9\xEC\xE5 \xF4\xE9\xF0\xE4 \xE9\xF9 \xE4\xEB\xE9 \xEE\xF2\xE8 \xE7\xFA\xE9\xEB\xE5\xFA \xF9\xF2\xF9\xE5\xE9\xE5\xFA \xEC\xE4\xFA\xE0\xE9\xED \xE5\xE4\xFA\xE7\xE9\xEC\xE5 \xEE\xF9\xED.",
		"\xE1\xE3\xF7\xE5 \xEB\xEC \xF4\xE9\xF0\xE4 \xE7\xE3\xF9\xE4 \xE5\xEB\xEC \xF6\xE3 \xE7\xE3\xF9 \xE0\xED \xE9\xF9 \xE7\xFA\xE9\xEB\xE4 \xEE\xFA\xE0\xE9\xEE\xE4.",
		"\xE0\xE9\xF0\xE9 \xF8\xE5\xE0\xE4 \xF9\xE5\xED \xE3\xE1\xF8 \xF9\xE0\xE9\xF0\xE5 \xE1\xEE\xF7\xE5\xEE\xE5."
	},
	// Chinese
	{
		"\xa5\xfd\xac\xdd\xac\xdd\xa8\xba\xa8\xc7\xb8\x48\xa4\xf9\xa5\x69\xa5\x48\xb1\xc6\xa6\x62\xa6\x55\xad\xd3\xa8\xa4\xa1\x45" /* 先看看那些碎片可以排在各個角‧; Check which pieces could fit in each corner first. */,
		"\xac\xdd\xac\xdd\xa8\xba\xa4\x40\xad\xd3\xa8\xa4\xaf\xe0\xb1\xc6\xb6\x69\xa5\x68\xaa\xba\xb8\x48\xa4\xf9\xb3\xcc\xa4\xd6\xa1\x41\xb4\x4e\xb1\x71\xa8\xba\xad\xd3\xa8\xa4\xb8\xa8\xb6\x7d\xa9\x6c\xa1\x45" /* 看看那一個角能排進去的碎片最少，就從那個角落開始‧; Check which corner has the least number of pieces that can fit and start from there. */,
		"\xc0\xcb\xac\x64\xa8\x43\xad\xd3\xb7\x73\xa5\x58\xb2\x7b\xaa\xba\xa8\xa4\xa5\x48\xa4\xce\xb7\x73\xa5\x58\xb2\x7b\xaa\xba\xc3\xe4\xa1\x41\xac\xdd\xac\xdd\xa6\xb3\xa8\x53\xa6\xb3\xb8\x48\xa4\xf9\xa7\x6b\xa6\x58\xb8\xd3\xb3\x42" /* 檢查每個新出現的角以及新出現的邊，看看有沒有碎片吻合該處; Check each new corner and any new side for pieces that fit. */,
		"\xa7\xda\xac\xdd\xa4\xa3\xa5\x58\xa8\xd3\xa6\xb3\xa8\xba\xa4\x40\xb6\xf4\xac\x4f\xa4\xa3\xbe\x41\xa6\x58\xaa\xba\xa1\x45" /* 我看不出來有那一塊是不適合的‧; I don't see anything out of place. */,
	},
};

const char *solicitStr[][NUM_SOLICIT_REPLIES] = {
	// English
	{
		"Hey, Fox! Would you like a hint?",
		"Would you like some help?",
		"Umm...Umm...",
		"Psst! want a hint?",
		"I would have done this differently, you know."
	},
	// German
	{
		"Hey, Fuchs! Brauchst Du \047nen Tip?",
		"M\224chtest Du etwas Hilfe?"
		"\231hm...\216hm..."
		"Psst! \047n Tip vielleicht?"
		"Ja, wei$t Du... ich h\204tte das anders gemacht."
	},
	// Italian
	{
		"Hey, Volpe! Serve un suggerimento?",
		"Hai bisogno di aiuto?",
		"Umm...Umm...",
		"Psst! Serve un aiutino?",
		"Io, sai, l'avrei fatto diversamente."
	},
	// Spanish
	{
		"",
		"",
		"",
		"",
		""
	},
	// French
	{
		"H\202, Renard! Tu veux un coup de pouce?",
		"T'as besoin d'aide?",
		"Umm...Umm...",
		"Psst! Un indice?",
		"Tu sais, j'aurais fait ça autrement."
	},
	// Japanese
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
	},
	// Russian
	{
		// "Эй, лис! Подсказка не нужна?",
		"\xDD\xE9, \xEB\xE8\xF1! \xCF\xEE\xE4\xF1\xEA\xE0\xE7\xEA\xE0 \xED\xE5 \xED\xF3\xE6\xED\xE0?",
		// "Хочешь помогу?",
		"\xD5\xEE\xF7\xE5\xF8\xFC \xEF\xEE\xEC\xEE\xE3\xF3?",
		// "М-м-м... М-м-м...",
		"\xCC-\xEC-\xEC... \xCC-\xEC-\xEC...",
		// "Пст! Хочешь подсказку?",
		"\xCF\xF1\xF2! \xD5\xEE\xF7\xE5\xF8\xFC \xEF\xEE\xE4\xF1\xEA\xE0\xE7\xEA\xF3?",
		// "Знаешь, а я бы сделал по-другому."
		"\xC7\xED\xE0\xE5\xF8\xFC, \xE0 \xFF \xE1\xFB \xF1\xE4\xE5\xEB\xE0\xEB \xEF\xEE-\xE4\xF0\xF3\xE3\xEE\xEC\xF3."
	},
	// Hebrew
	{
		"\xE4\xE9\xE9, \xF9\xE5\xF2\xEC! \xFA\xF8\xF6\xE4 \xF8\xEE\xE6?",
		"\xEE\xF2\xE5\xF0\xE9\xE9\xEF \xE1\xEE\xF2\xE8 \xF2\xE6\xF8\xE4?",
		"\xE4\xEE\xEE...\xE4\xEE\xEE...",
		"\xF4\xF1\xF1! \xF8\xE5\xF6\xE4 \xF8\xEE\xE6?",
		"\xE0\xF0\xE9 \xE4\xE9\xE9\xFA\xE9 \xF2\xE5\xF9\xE4 \xE6\xE0\xFA \xE0\xE7\xF8\xFA, \xE0\xFA\xE4 \xE9\xE5\xE3\xF2."
	},
	// Chinese
	{
		"\xbc\x4b\xa1\x41\xaa\xb0\xaf\x57\xa1\x49\xa7\x41\xbb\xdd\xa4\xa3\xbb\xdd\xad\x6e\xb4\xa3\xa5\xdc\xa1\x48" /* 嘿，狐狸！你需不需要提示？; Hey, Fox! Would you like a hint? */,
		"\xa7\x41\xbb\xdd\xad\x6e\xc0\xb0\xa7\x55\xb6\xdc\xa1\x48" /* 你需要幫助嗎？; Would you like some help? */,
		"\xb6\xe2\x2e\x2e\x2e\xb6\xe2\x2e\x2e\x2e" /* 嗯...嗯...; Umm...Umm... */,
		"\xb3\xde\xa1\x49\xad\x6e\xa4\xa3\xad\x6e\xb4\xa3\xa5\xdc\xa1\x48" /* 喂！要不要提示？; Psst! want a hint? */,
		"\xa7\xda\xb7\x7c\xa5\xce\xa4\xa3\xa6\x50\xaa\xba\xa4\xe8\xaa\x6b\xa8\xd3\xb0\xb5\xb3\x6f\xa5\xf3\xa8\xc6\xa1\x45" /* 我會用不同的方法來做這件事‧; I would have done this differently, you know. */,
	},
};

const char *sakkaStr[][NUM_SAKKA] = {
	// English
	{
		"Hey, you're not supposed to help the applicants!",
		"Guys! This is supposed to be a test!",
		"C'mon fellows, that's not in the rules!"
	},
	// German
	{
		"Hey, Du darfst dem Pr\201fling nicht helfen!",
		"Hallo?! Dies soll eine Pr\201fung sein!",
		"Also, Jungs. Schummeln steht nicht in den Regeln!"
	},
	// Italian
	{
		"Hey, non si dovrebbero aiutare i candidati!",
		"Ragazzi! Questo dovrebbe essere un test!",
		"Forza ragazzi, non si pu\225!"
	},
	// Spanish
	{
		"",
		"",
		""
	},
	// French
	{
		"H\202, vous n'\210tes pas suppos\202s aider les concurrents!",
		"Les gars! C'est suppos\202 être un test!",
		"Allez les gars, c'est pas dans les r\212gles!"
	},
	// Japanese
	{
		// "ねぇ、どう？",
		"\x82\xCB\x82\xA5\x81\x41\x82\xC7\x82\xA4\x81H",
		// "なかなかね！",
		"\x82\xC8\x82\xA9\x82\xC8\x82\xA9\x82\xCB\x81I",
		// "どうしたの！"
		"\x82\xC7\x82\xA4\x82\xB5\x82\xBD\x82\xCC\x81I"
	},
	// Russian
	{
		// "Эй, испытуемым помогать нельзя!",
		"\xDD\xE9, \xE8\xF1\xEF\xFB\xF2\xF3\xE5\xEC\xFB\xEC \xEF\xEE\xEC\xEE\xE3\xE0\xF2\xFC \xED\xE5\xEB\xFC\xE7\xFF!",
		// "Ребята! Это ведь испытание!",
		"\xD0\xE5\xE1\xFF\xF2\xE0! \xDD\xF2\xEE \xE2\xE5\xE4\xFC \xE8\xF1\xEF\xFB\xF2\xE0\xED\xE8\xE5!",
		// "Да хватит вам, это не по правилам!"
		"\xC4\xE0 \xF5\xE2\xE0\xF2\xE8\xF2 \xE2\xE0\xEC, \xFD\xF2\xEE \xED\xE5 \xEF\xEE \xEF\xF0\xE0\xE2\xE8\xEB\xE0\xEC!"
	},
	// Hebrew
	{
		"\xE4\xE9\xE9, \xE0\xFA\xE4 \xEC\xE0 \xE0\xEE\xE5\xF8 \xEC\xF1\xE9\xE9\xF2 \xEC\xEE\xE5\xF2\xEE\xE3\xE9\xED!",
		"\xE7\xE1\xF8'\xE4! \xE6\xE4 \xE0\xEE\xE5\xF8 \xEC\xE4\xE9\xE5\xFA \xEE\xE1\xE7\xEF!",
		"\xE1\xE7\xE9\xE9\xEB\xED, \xE6\xE4 \xEC\xE0 \xEE\xE5\xF4\xE9\xF2 \xE1\xE7\xE5\xF7\xE9\xED!"
	},
	// Chinese
	{
		"\xb3\xde\xa1\x41\xa7\x41\xa4\xa3\xa5\x69\xa5\x48\xc0\xb0\xa7\x55\xc0\xb3\xbc\x78\xaa\xba\xa4\x48\xa1\x49" /* 喂，你不可以幫助應徵的人！; Hey, you're not supposed to help the applicants! */,
		"\xa6\xd1\xa5\x53\xa1\x41\xb3\x6f\xa5\x69\xac\x4f\xb4\xfa\xc5\xe7\xad\xfe\xa1\x49" /* 老兄，這可是測驗哪！; Guys! This is supposed to be a test! */,
		"\xa1\x41\xaa\x42\xa4\xcd\xa1\x41\xb3\x6f\xa5\x69\xac\x4f\xa4\xa3\xa6\x58\xb3\x57\xa9\x77\xaa\xba\xb3\xe1\xa1\x49" /* ，朋友，這可是不合規定的喔！; C'mon fellows, that's not in the rules! */
	},
};

const char *whineStr[][NUM_WHINES] = {
	// English
	{
		"Aww, c'mon Sakka!",
		"One hint won't hurt, will it?",
		"Sigh...",
		"I think that clipboard has gone to your head, Sakka!",
		"Well, I don't recall any specific rule against hinting."
	},
	// German
	{
		"Och, sei nicht so, Sakka!"
		"EIN Tip wird schon nicht schaden, oder?",
		"Seufz..."
		"Ich glaube, Du hast ein Brett vor dem Kopf, Sakka!",
		"Hm, ich kann mich an keine Regel erinnern, die Tips verbietet."
	},
	// Italian
	{
		"Ooo, suvvia Sakka!",
		"Un indizio non guaster\205, no?",
		"Sigh...",
		"Credo che questa faccenda ti abbia dato alla testa, Sakka!",
		"Beh, non ricordo regole specifiche contro i suggerimenti."
	},
	// Spanish
	{
		"",
		"",
		"",
		"",
		""
	},
	// French
	{
		"Rohh, allez Sakka!",
		"Un indice ne peut pas faire de mal.",
		"Pfff...",
		"Je crois que ton carnet te monte \205 la t\210te, Sakka!",
		"Bon, je ne me souviens d'aucune r\212gle concernant les indices."
	},
	// Japanese
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
	},
	// Russian
	{
		// "Ой, Сакка, да брось!",
		"\xCE\xE9, \xD1\xE0\xEA\xEA\xE0, \xE4\xE0 \xE1\xF0\xEE\xF1\xFC!",
		// "Подсказочка лишней не будет, а?",
		"\xCF\xEE\xE4\xF1\xEA\xE0\xE7\xEE\xF7\xEA\xE0 \xEB\xE8\xF8\xED\xE5\xE9 \xED\xE5 \xE1\xF3\xE4\xE5\xF2, \xE0?",
		// "Эх...",
		"\xDD\xF5...",
		// "Мне кажется, этот узор ударил тебя в голову, Сакка!",
		"\xCC\xED\xE5 \xEA\xE0\xE6\xE5\xF2\xF1\xFF, \xFD\xF2\xEE\xF2 \xF3\xE7\xEE\xF0 \xF3\xE4\xE0\xF0\xE8\xEB \xF2\xE5\xE1\xFF \xE2 \xE3\xEE\xEB\xEE\xE2\xF3, \xD1\xE0\xEA\xEA\xE0!",
		// "Ну, я не помню ни одного правила, явно запрещающего подсказки."
		"\xCD\xF3, \xFF \xED\xE5 \xEF\xEE\xEC\xED\xFE \xED\xE8 \xEE\xE4\xED\xEE\xE3\xEE \xEF\xF0\xE0\xE2\xE8\xEB\xE0, \xFF\xE2\xED\xEE \xE7\xE0\xEF\xF0\xE5\xF9\xE0\xFE\xF9\xE5\xE3\xEE \xEF\xEE\xE4\xF1\xEA\xE0\xE7\xEA\xE8."
	},
	// Hebrew
	{
		"\xF0\xE5, \xE1\xE7\xE9\xE9\xE0\xFA \xF1\xE0\xF7\xE4!",
		"\xF8\xEE\xE6 \xE0\xE7\xE3 \xEC\xE0 \xE9\xEB\xE5\xEC \xEC\xE4\xE6\xE9\xF7, \xF0\xEB\xE5\xEF?",
		"\xF0\xE5 \xE8\xE5\xE1...",
		"\xF0\xF8\xE0\xE4 \xEC\xE9 \xF9\xEC\xE5\xE7 \xE4\xEB\xFA\xE9\xE1\xE4 \xF2\xEC\xE4 \xEC\xEA \xEC\xF8\xE0\xF9, \xF1\xE0\xF7\xE4!",
		"\xE8\xE5\xE1, \xE0\xF0\xE9 \xEC\xE0 \xE6\xE5\xEB\xF8 \xF9\xE9\xF9 \xE7\xE5\xF7 \xEE\xF4\xE5\xF8\xF9 \xF9\xEE\xFA\xF0\xE2\xE3 \xEC\xF8\xEE\xE6\xE9\xED."
	},
	// Chinese
	{
		"\xbe\xbe\xa1\x41\xa7\x4f\xb3\x6f\xbc\xcb\xa1\x41\xc2\xc4\xa5\x64\xa1\x49" /* 噢，別這樣，薩卡！; Aww, c'mon Sakka! */,
		"\xa4\x40\xad\xd3\xa4\x70\xb4\xa3\xa5\xdc\xa4\xa3\xb7\x7c\xab\xe7\xbc\xcb\xb9\xc0\xa1\x49" /* 一個小提示不會怎樣嘛！; One hint won't hurt, will it? */,
		"\xad\xfc\x2e\x2e\x2e" /* 唉...; Sigh... */,
		"\xa7\xda\xac\xdd\xa7\x41\xac\x4f\xa4\xd3\xb9\x4c\xa9\xf3\xb9\x78\xa9\x54\xa4\x46\xa1\x41\xc2\xc4\xa5\x5b\xa1\x49" /* 我看你是太過於頑固了，薩加！; I think that clipboard has gone to your head, Sakka! */,
		"\xb6\xe2\xa1\x41\xa7\xda\xa4\xa3\xb0\x4f\xb1\x6f\xa6\xb3\xa8\xba\xb1\xf8\xb3\x57\xa9\x77\xa4\xa3\xad\xe3\xb4\xa3\xa5\xdc\xa1\x45" /* 嗯，我不記得有那條規定不准提示‧; Well, I don't recall any specific rule against hinting. */,
	},
};

const char *optionsStr[][4] = {
	// English
	{
		"\"I'll do this puzzle later.\"",
		"\"Yes, I'd like a hint please.\"",
		"\"No, thank you, I'd like to try and solve it myself.\"",
		"I think the %s is in the wrong place."
	},
	// German
	{
		"\"Ich l\224se das Puzzle sp\204ter.\"",
		"\"Ja, ich m\224chte einen Tip, bitte.\"",
		"\"Nein danke, ich m\224chte das alleine l\224sen.\"",
		"Pssst... %s... falsche Stelle..."
	},
	// Italian
	{
		"\"Far\225 questo puzzle pi\227 tardi.\"",
		"\"Si, grazie. Ne avrei bisogno.\"",
		"\"No, grazie, voglio provare a risolverlo da solo.\"",
		"Penso che la tessera %s sia nel posto sbagliato."
	},
	// Spanish
	{
		"",
		"",
		"",
		""
	},
	// French
	{
		"\"Je r\202soudrai cette \202nigme plus tard.\"",
		"\"Oui, j'aimerais un indice s'il vous plait.\"",
		"\"Non merci, je voudrais r\202soudre cela par moi m\210me.\"",
		"Je crois que t'as mal plac\202 l'%s."
	},
	// Japanese
	{
		// "「後でやろうかな。」",
		"\x81u\x8C\xE3\x82\xC5\x82\xE2\x82\xEB\x82\xA4\x82\xA9\x82\xC8\x81\x42\x81v",
		// "「はい、ヒントをお願いします。」",
		"\x81u\x82\xCD\x82\xA2\x81\x41\x83q\x83\x93\x83g\x82\xF0\x82\xA8\x8A\xE8\x82\xA2\x82\xB5\x82\xDC\x82\xB7\x81\x42\x81v",
		// "「いいえ、自分で解きたいんです。」",
		"\x81u\x82\xA2\x82\xA2\x82\xA6\x81\x41\x8E\xA9\x95\xAA\x82\xC5\x89\xF0\x82\xAB\x82\xBD\x82\xA2\x82\xF1\x82\xC5\x82\xB7\x81\x42\x81v",
		// "%sの位置が違うんじゃないかな。"
		"%s\x82\xCC\x88\xCA\x92u\x82\xAA\x88\xE1\x82\xA4\x82\xF1\x82\xB6\x82\xE1\x82\xC8\x82\xA2\x82\xA9\x82\xC8\x81\x42"
	},
	// Russian
	{
		// "\"Я решу эту головоломку в другой раз.\"",
		"\"\xDF \xF0\xE5\xF8\xF3 \xFD\xF2\xF3 \xE3\xEE\xEB\xEE\xE2\xEE\xEB\xEE\xEC\xEA\xF3 \xE2 \xE4\xF0\xF3\xE3\xEE\xE9 \xF0\xE0\xE7.\"",
		// "\"Да, дайте мне подсказку, пожалуйста.\"",
		"\"\xC4\xE0, \xE4\xE0\xE9\xF2\xE5 \xEC\xED\xE5 \xEF\xEE\xE4\xF1\xEA\xE0\xE7\xEA\xF3, \xEF\xEE\xE6\xE0\xEB\xF3\xE9\xF1\xF2\xE0.\"",
		// "\"Нет, спасибо, я хочу догадаться сам.\"",
		"\"\xCD\xE5\xF2, \xF1\xEF\xE0\xF1\xE8\xE1\xEE, \xFF \xF5\xEE\xF7\xF3 \xE4\xEE\xE3\xE0\xE4\xE0\xF2\xFC\xF1\xFF \xF1\xE0\xEC.\"",
		// "Мне кажется, %s не на своем месте."
		"\xCC\xED\xE5 \xEA\xE0\xE6\xE5\xF2\xF1\xFF, %s \xED\xE5 \xED\xE0 \xF1\xE2\xEE\xE5\xEC \xEC\xE5\xF1\xF2\xE5."
	},
	// Hebrew
	{
		"\"\xE0\xF4\xFA\xE5\xF8 \xE0\xFA \xE4\xE7\xE9\xE3\xE4 \xE4\xE6\xE5 \xE0\xE7\xF8 \xEB\xEA.\"",
		"\"\xEB\xEF, \xE0\xF9\xEE\xE7 \xEC\xF8\xEE\xE6 \xE1\xE1\xF7\xF9\xE4.\"",
		"\"\xEC\xE0, \xFA\xE5\xE3\xE4 \xF8\xE1\xE4, \xE0\xF9\xEE\xE7 \xEC\xF0\xF1\xE5\xFA \xEC\xF4\xFA\xE5\xF8 \xE1\xF2\xF6\xEE\xE9.\"",
		"\xF0\xF8\xE0\xE4 \xEC\xE9 \xF9\xE4\xEE\xF7\xE5\xED \xF9\xE1\xE5 \xF9\xEE\xFA\xE9 \xE0\xFA \xE4%s \xEC\xE0 \xF0\xEB\xE5\xEF."
	},
	// Chinese
	{
		"\xb5\xa5\xa4\x40\xa4\x55\xa7\xda\xa6\x41\xa8\xd3\xb8\xd1\xa8\x4d\xb3\x6f\xad\xd3\xc1\xbc\xc3\x44\xa1\x45" /* 等一下我再來解決這個謎題‧; I'll do this puzzle later. */,
		"\xbd\xd0\xb5\xb9\xa7\xda\xa4\x40\xa8\xc7\xb4\xa3\xa5\xdc\xa1\x45" /* 請給我一些提示‧; Yes, I'd like a hint please. */,
		"\xa4\xa3\xa1\x41\xc1\xc2\xc1\xc2\xa7\x41\xa1\x45\xa7\xda\xb7\x51\xb8\xd5\xb8\xd5\xac\xdd\xa6\xdb\xa4\x76\xb8\xd1\xa8\x4d\xb3\x6f\xad\xd3\xb0\xdd\xc3\x44\xa1\x45" /* 不，謝謝你‧我想試試看自己解決這個問題‧; No, thank you, I'd like to try and solve it myself. */,
		"\xa7\xda\xc4\xb1\xb1\x6f\x25\x73\xa9\xf1\xbf\xf9\xa6\x61\xa4\xe8\xa4\x46\xa1\x45" /* 我覺得%s放錯地方了‧; I think the %s is in the wrong place. */,
	},
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
	{ { // Spanish
		0,		// cave voice 0
		""
		""
	},
	{
		1,		// cave voice 1
		""
		""
	},
	{
		2,		// cave voice 2
		""
		""
	},
	{
		3,		// cave voice 3
		""
		""
		""
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
	} },
	// -----------------------------------------------------
	{ {	// Russian fan translation
		0,		// cave voice 0
		// "Мы видим небо, видим землю, видим воду "
		"\xCC\xFB \xE2\xE8\xE4\xE8\xEC \xED\xE5\xE1\xEE, \xE2\xE8\xE4\xE8\xEC \xE7\xE5\xEC\xEB\xFE, \xE2\xE8\xE4\xE8\xEC \xE2\xEE\xE4\xF3 "
		// "и мы задумываемся - единственные ли мы?"
		"\xE8 \xEC\xFB \xE7\xE0\xE4\xF3\xEC\xFB\xE2\xE0\xE5\xEC\xF1\xFF - \xE5\xE4\xE8\xED\xF1\xF2\xE2\xE5\xED\xED\xFB\xE5 \xEB\xE8 \xEC\xFB?"
	},
	{
		1,		// cave voice 1
		// "Задолго до нашего существования "
		"\xC7\xE0\xE4\xEE\xEB\xE3\xEE \xE4\xEE \xED\xE0\xF8\xE5\xE3\xEE \xF1\xF3\xF9\xE5\xF1\xF2\xE2\xEE\xE2\xE0\xED\xE8\xFF "
		// "Землёй правили люди."
		"\xC7\xE5\xEC\xEB\xB8\xE9 \xEF\xF0\xE0\xE2\xE8\xEB\xE8 \xEB\xFE\xE4\xE8."
	},
	{
		2,		// cave voice 2
		// "Они создавали удивительные вещи "
		"\xCE\xED\xE8 \xF1\xEE\xE7\xE4\xE0\xE2\xE0\xEB\xE8 \xF3\xE4\xE8\xE2\xE8\xF2\xE5\xEB\xFC\xED\xFB\xE5 \xE2\xE5\xF9\xE8 "
		// "и сдвигали целые горы."
		"\xE8 \xF1\xE4\xE2\xE8\xE3\xE0\xEB\xE8 \xF6\xE5\xEB\xFB\xE5 \xE3\xEE\xF0\xFB."
	},
	{
		3,		// cave voice 3
		// "Они знали тайны полета, "
		"\xCE\xED\xE8 \xE7\xED\xE0\xEB\xE8 \xF2\xE0\xE9\xED\xFB \xEF\xEE\xEB\xE5\xF2\xE0, "
		// "тайны счастья и прочие тайны "
		"\xF2\xE0\xE9\xED\xFB \xF1\xF7\xE0\xF1\xF2\xFC\xFF \xE8 \xEF\xF0\xEE\xF7\xE8\xE5 \xF2\xE0\xE9\xED\xFB "
		// "за пределами нашего воображения."
		"\xE7\xE0 \xEF\xF0\xE5\xE4\xE5\xEB\xE0\xEC\xE8 \xED\xE0\xF8\xE5\xE3\xEE \xE2\xEE\xEE\xE1\xF0\xE0\xE6\xE5\xED\xE8\xFF."
	} },
	// -----------------------------------------------------
	{ { // Hebrew
		0,		// cave voice 0
		"\xE0\xF0\xE5 \xEE\xE1\xE9\xE8\xE9\xED \xE0\xEC \xE4\xF9\xEE\xE9\xE9\xED, \xE0\xEC \xE4\xE0\xF8\xF5, \xE0\xEC \xE4\xEE\xE9\xED, "
		"\xE5\xFA\xE5\xE4\xE9\xED \xE1\xEC\xE1\xF0\xE5: \xE4\xE0\xED \xE0\xF0\xE7\xF0\xE5 \xEC\xE1\xE3\xF0\xE5?"
	},
	{
		1,		// cave voice 1
		"\xE4\xF8\xE1\xE4 \xEC\xF4\xF0\xE9 \xF9\xE4\xE2\xF2\xF0\xE5 \xE4\xF0\xE4, \xE1\xF0\xE9 \xE4\xE0\xE3\xED \xF9\xEC\xE8\xE5 \xE1\xE0\xF8\xF5."
	},
	{
		2,		// cave voice 2
		"\xE4\xED \xF2\xF9\xE5 \xE3\xE1\xF8\xE9\xED \xEE\xE5\xF4\xEC\xE0\xE9\xED, \xE5\xE4\xE6\xE9\xE6\xE5 \xE4\xF8\xE9\xED \xF9\xEC\xEE\xE9\xED \xEE\xEE\xF7\xE5\xEE\xED. "
	},
	{
		3,		// cave voice 3
		"\xE4\xED \xE9\xE3\xF2\xE5 \xE0\xFA \xF1\xE5\xE3 \xE4\xFA\xF2\xE5\xF4\xE4, \xE0\xFA \xF1\xE5\xE3 \xE4\xE0\xE5\xF9\xF8, \xE5\xF1\xE5\xE3\xE5\xFA \xF0\xE5\xF1\xF4\xE9\xED "
		"\xEE\xF2\xE1\xF8 \xEC\xEE\xE4 \xF9\xE0\xF0\xE7\xF0\xE5 \xEE\xF1\xE5\xE2\xEC\xE9\xED \xEC\xE3\xEE\xE9\xE9\xEF."
	} },
	{ { // Chinese
		0,		// cave voice 0
		"\xa9\xef\xc0\x59\xb1\xe6\xa4\xd1\x2c\xad\xc1\xad\xba\xa8\xa3\xa6\x61\x2c\xa9\xf1\xb2\xb4\xb1\xe6\xa5\x68\xa7\xf3\xa6\xb3\xa4\x6a\xae\xfc\xaa\x65\xac\x79\x2c\xa7\xda\xad\xcc\xa4\xa3\xb8\x54\xc3\x68\xba\xc3\x3a\xa7\xda\xad\xcc\xac\x4f\xb6\xc8\xa6\x73\xaa\xba\xb1\xda\xc3\xfe\xb6\xdc\xa1\x48" /* 抬頭望天,俯首見地,放眼望去更有大海河流,我們不禁懷疑:我們是僅存的族類嗎？; We see the sky, we see the land, we see the water, */
	},
	{
		1,		// cave voice 1
		"\xab\xdc\xa4\x5b\xab\xdc\xa4\x5b\xa5\x48\xab\x65\x2c\xa7\xda\xad\xcc\xc1\xd9\xa8\x53\xa6\xb3\xa5\x58\xb2\x7b\xaa\xba\xae\xc9\xad\xd4\x2c\xa4\x48\xc3\xfe\xb4\x78\xba\xde\xa4\x46\xa6\x61\xb2\x79\x2e" /* 很久很久以前,我們還沒有出現的時候,人類掌管了地球.; Long before we came to exist, the humans ruled the Earth.  */
	},
	{
		2,		// cave voice 2
		"\xa5\x4c\xad\xcc\xbb\x73\xb3\x79\xaf\xab\xa9\x5f\xaa\xba\xbe\xb9\xa8\xe3\x2c\xa6\xd3\xa5\x42\xb2\xbe\xa5\x68\xa9\xd2\xa6\xb3\xaa\xba\xb0\xaa\xa4\x73\xc2\x4f\xc0\xad\x2e" /* 他們製造神奇的器具,而且移去所有的高山叢嶺.; They made marvelous things, and moved whole mountains. */
	},
	{
		3,		// cave voice 3
		"\xa5\x4c\xad\xcc\xa9\xfa\xa5\xd5\xad\xb8\xa6\xe6\xaa\xba\xaf\xb5\xb1\x4b\x2c\xa7\xd6\xbc\xd6\xaa\xba\xaf\xb5\xb1\x4b\x2c\xa5\x48\xa4\xce\xa8\xe4\xa5\xa6\xa7\xda\xad\xcc\xb5\x4c\xaa\x6b\xb7\x51\xb9\xb3\xaa\xba\xaf\xb5\xb1\x4b\x2e" /* 他們明白飛行的秘密,快樂的秘密,以及其它我們無法想像的秘密.; They knew the Secret of Flight, the Secret of Happiness, and other secrets beyond our imagining. */
	} },
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
	// -----------------------------------------------------
	{ { // Spanish
		4,		// cave voice 4
		""
		""

	},
	{
		5,		// cave voice 5
		""

	},
	{
		6,		// cave voice 6
		""
	} },
	// -----------------------------------------------------
	{ { // French fan translation
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
	} },
	{ { // Russian fan translation
		4,		// cave voice 4
		// "Люди также знали тайну жизни "
		"\xCB\xFE\xE4\xE8 \xF2\xE0\xEA\xE6\xE5 \xE7\xED\xE0\xEB\xE8 \xF2\xE0\xE9\xED\xF3 \xE6\xE8\xE7\xED\xE8 "
		// "и воспользовались этим, чтобы "
		"\xE8 \xE2\xEE\xF1\xEF\xEE\xEB\xFC\xE7\xEE\xE2\xE0\xEB\xE8\xF1\xFC \xFD\xF2\xE8\xEC, \xF7\xF2\xEE\xE1\xFB "
		// "наделить нас четырьмя великими дарами:"
		"\xED\xE0\xE4\xE5\xEB\xE8\xF2\xFC \xED\xE0\xF1 \xF7\xE5\xF2\xFB\xF0\xFC\xEC\xFF \xE2\xE5\xEB\xE8\xEA\xE8\xEC\xE8 \xE4\xE0\xF0\xE0\xEC\xE8:"
	},
	{
		5,		// cave voice 5
		// "Рассудительным умом, чувственным "
		"\xD0\xE0\xF1\xF1\xF3\xE4\xE8\xF2\xE5\xEB\xFC\xED\xFB\xEC \xF3\xEC\xEE\xEC, \xF7\xF3\xE2\xF1\xF2\xE2\xE5\xED\xED\xFB\xEC "
		// "сердцем, говорящим ртом и умелыми руками."
		"\xF1\xE5\xF0\xE4\xF6\xE5\xEC, \xE3\xEE\xE2\xEE\xF0\xFF\xF9\xE8\xEC \xF0\xF2\xEE\xEC \xE8 \xF3\xEC\xE5\xEB\xFB\xEC\xE8 \xF0\xF3\xEA\xE0\xEC\xE8."
	},
	{
		6,		// cave voice 6
		// "Мы их дети."
		"\xCC\xFB \xE8\xF5 \xE4\xE5\xF2\xE8."
	} },
	// -----------------------------------------------------
	{ { // Hebrew
		4,		// cave voice 4
		"\xE1\xF0\xE9 \xE4\xE0\xE3\xED \xE9\xE3\xF2\xE5 \xE2\xED \xE0\xFA \xF1\xE5\xE3 \xE4\xE7\xE9\xE9\xED, \xE5\xE4\xED "
		"\xE4\xF9\xFA\xEE\xF9\xE5 \xE1\xE5 \xEB\xE3\xE9 \xEC\xE4\xF2\xF0\xE9\xF7 \xEC\xF0\xE5 \xE0\xFA \xE0\xF8\xE1\xF2 \xE4\xEE\xFA\xF0\xE5\xFA \xE4\xE2\xE3\xE5\xEC\xE5\xFA:"
	},
	{
		5,		// cave voice 5
		"\xEE\xE5\xE7\xE5\xFA \xEC\xE7\xF9\xE5\xE1, \xEC\xE1\xE1\xE5\xFA \xEC\xE4\xF8\xE2\xE9\xF9, \xF4\xE9\xE5\xFA \xEC\xE3\xE1\xF8, \xE5\xE9\xE3\xE9\xE9\xED \xEC\xE4\xE5\xF9\xE9\xE8."
	},
	{
		6,		// cave voice 6
		"\xE0\xF0\xE7\xF0\xE5 \xE4\xE9\xE9\xF0\xE5 \xEC\xE4\xED \xEC\xE9\xEC\xE3\xE9\xED."
	} },
	{ { // Chinese
		4,		// cave voice 4
		"\xa4\x48\xc3\xfe\xa4\x5d\xaa\xbe\xb9\x44\xa5\xcd\xa9\x52\xaa\xba\xaf\xb5\xb1\x4b\x2c\xa6\xd3\xa5\x42\xa7\x51\xa5\xce\xa5\xa6\xb5\xb9\xa4\x46\xa7\xda\xad\xcc\xa5\x7c\xa4\x6a\xa4\xd1\xbd\xe1\xa1\x47" /* 人類也知道生命的秘密,而且利用它給了我們四大天賦：; The humans also knew the Secret of Life, and they used it to give us the Four Great Gifts: */
	},
	{
		5,		// cave voice 5
		"\xaf\xe0\xab\xe4\xaf\xe0\xb7\x51\xaa\xba\xb7\x4e\xa7\xd3\x2c\xb7\x50\xa8\xfc\xb1\xd3\xbe\x55\xaa\xba\xa4\xdf\xc6\x46\x2c\x20\xaf\xe0\xbb\xa1\xb5\xbd\xb9\x44\xaa\xba\xa4\x66\xa6\xde\x2c\xa5\x48\xa4\xce\xc6\x46\xa5\xa9\xa4\xe8\xab\x4b\xaa\xba\xc2\xf9\xa4\xe2\x2e" /* 能思能想的意志,感受敏銳的心靈, 能說善道的口舌,以及靈巧方便的雙手.; Thinking minds, feeling hearts, speaking mouths, and reaching hands. */
	},
	{
		6,		// cave voice 6
		"\xa7\xda\xad\xcc\xac\x4f\xa5\x4c\xad\xcc\xaa\xba\xa4\x6c\xae\x5d\x2e" /* 我們是他們的子孫.; We are their children. */
	} },
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
	// -----------------------------------------------------
	{ { // Spanish
		7,		// cave voice 7
		""
	},
	{
		8,		// cave voice 8
		""
		""
	},
	{
		9,		// cave voice 9
		""
		""

	} },
	// -----------------------------------------------------
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
	} },
	{ {	// Russian fan translation
		7,		// cave voice 7
		// "Они научили нас пользоваться "
		"\xCE\xED\xE8 \xED\xE0\xF3\xF7\xE8\xEB\xE8 \xED\xE0\xF1 \xEF\xEE\xEB\xFC\xE7\xEE\xE2\xE0\xF2\xFC\xF1\xFF "
		// "руками и разговаривать."
		"\xF0\xF3\xEA\xE0\xEC\xE8 \xE8 \xF0\xE0\xE7\xE3\xEE\xE2\xE0\xF0\xE8\xE2\xE0\xF2\xFC."
	},
	{
		8,		// cave voice 8
		// "Они показали нам удовольствие от размышлений."
		"\xCE\xED\xE8 \xEF\xEE\xEA\xE0\xE7\xE0\xEB\xE8 \xED\xE0\xEC \xF3\xE4\xEE\xE2\xEE\xEB\xFC\xF1\xF2\xE2\xE8\xE5 \xEE\xF2 \xF0\xE0\xE7\xEC\xFB\xF8\xEB\xE5\xED\xE8\xE9."
	},
	{
		9,		// cave voice 9
		// "Они любили нас и поведали "
		"\xCE\xED\xE8 \xEB\xFE\xE1\xE8\xEB\xE8 \xED\xE0\xF1 \xE8 \xEF\xEE\xE2\xE5\xE4\xE0\xEB\xE8 "
		// "бы нам тайну счастья, когда "
		"\xE1\xFB \xED\xE0\xEC \xF2\xE0\xE9\xED\xF3 \xF1\xF7\xE0\xF1\xF2\xFC\xFF, \xEA\xEE\xE3\xE4\xE0 "
		// "бы мы были готовы к этому."
		"\xE1\xFB \xEC\xFB \xE1\xFB\xEB\xE8 \xE3\xEE\xF2\xEE\xE2\xFB \xEA \xFD\xF2\xEE\xEC\xF3."
	} },
	// -----------------------------------------------------
	{ { // Hebrew
		7,		// cave voice 7
		"\xE4\xED \xEC\xE9\xEE\xE3\xE5 \xE0\xE5\xFA\xF0\xE5 \xEB\xE9\xF6\xE3 \xEC\xE4\xF9\xFA\xEE\xF9 \xE1\xE9\xE3\xE9\xE9\xED \xF9\xEC\xF0\xE5, \xE5\xEB\xE9\xF6\xE3 \xEC\xE3\xE1\xF8."
	},
	{
		8,		// cave voice 8
		"\xE4\xED \xE4\xF8\xE0\xE5 \xEC\xF0\xE5 \xE0\xFA \xE4\xE4\xF0\xE0\xE4 \xF9\xE1\xF9\xE9\xEE\xE5\xF9 \xE1\xEE\xE5\xE7\xF0\xE5."
	},
	{
		9,		// cave voice 9
		"\xE4\xED \xE0\xE4\xE1\xE5 \xE0\xE5\xFA\xF0\xE5, \xE5\xEB\xF9\xE4\xE9\xE9\xF0\xE5 \xEE\xE5\xEB\xF0\xE9\xED, \xE4\xED \xE1\xE5\xE5\xE3\xE0\xE9 \xE4\xE9\xE5 \xEE\xF2\xF0\xE9\xF7\xE9\xED \xEC\xF0\xE5 \xE0\xFA \xF1\xE5\xE3 \xE4\xE0\xE5\xF9\xF8."
	} },
	{ { // Chinese
		7,		// cave voice 7
		"\xa5\x4c\xad\xcc\xb1\xd0\xbe\xc9\xa7\xda\xad\xcc\xa6\x70\xa6\xf3\xa8\xcf\xa5\xce\xc2\xf9\xa4\xe2\x2c\xc1\xd9\xa6\xb3\xbb\xa1\xb8\xdc\xaa\xba\xa4\xe8\xaa\x6b\x2e" /* 他們教導我們如何使用雙手,還有說話的方法.; They taught us how to use our hands, and how to speak. */
	},
	{
		8,		// cave voice 8
		"\xa5\x4c\xad\xcc\xa8\xcf\xa7\xda\xad\xcc\xa9\xfa\xa5\xd5\xa8\xcf\xa5\xce\xb7\x4e\xa7\xd3\xaa\xba\xb3\xdf\xae\xae\x2e" /* 他們使我們明白使用意志的喜悅.; They showed us the joy of using our minds.  */
	},
	{
		9,		// cave voice 9
		"\xa5\x4c\xad\xcc\xb7\x52\xc5\x40\xa7\xda\xad\xcc\x2c\xb5\xa5\xa7\xda\xad\xcc\xb9\x77\xb3\xc6\xa6\x6e\xa4\x46\x2c\xa5\x4c\xad\xcc\xa4\x40\xa9\x77\xb7\x7c\xa7\x69\xb6\x44\xa7\xda\xad\xcc\xa7\xd6\xbc\xd6\xaa\xba\xaf\xb5\xb1\x4b\x2e" /* 他們愛護我們,等我們預備好了,他們一定會告訴我們快樂的秘密.; They loved us, and when we were ready, they surely would have given us the Secret of Happiness.  */
	} },
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
	// -----------------------------------------------------
	{ { // Spanish
		10,		// cave voice 10
		""
		""

	},
	{
		11,		// cave voice 11
		""
		""
	},
	{
		12,		// cave voice 12
		""
	},
	{
		13,		// cave voice 13
		""
		""
	} },
	// -----------------------------------------------------
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
	} },
	{ {	// Russian fan translation
		10,		// cave voice 10
		// "Теперь мы видим небо, землю "
		"\xD2\xE5\xEF\xE5\xF0\xFC \xEC\xFB \xE2\xE8\xE4\xE8\xEC \xED\xE5\xE1\xEE, \xE7\xE5\xEC\xEB\xFE "
		// "и воду, которые унаследовали, и "
		"\xE8 \xE2\xEE\xE4\xF3, \xEA\xEE\xF2\xEE\xF0\xFB\xE5 \xF3\xED\xE0\xF1\xEB\xE5\xE4\xEE\xE2\xE0\xEB\xE8, \xE8  "
		// "задумываемся... Почему они исчезли?"
		"\xE7\xE0\xE4\xF3\xEC\xFB\xE2\xE0\xE5\xEC\xF1\xFF... \xCF\xEE\xF7\xE5\xEC\xF3 \xEE\xED\xE8 \xE8\xF1\xF7\xE5\xE7\xEB\xE8?"
	},
	{
		11,		// cave voice 11
		// "Живы ли они и где сейчас? "
		"\xC6\xE8\xE2\xFB \xEB\xE8 \xEE\xED\xE8 \xE8 \xE3\xE4\xE5 \xF1\xE5\xE9\xF7\xE0\xF1? "
		// "На звёздах? В глубинах океана? В ветре?"
		"\xCD\xE0 \xE7\xE2\xB8\xE7\xE4\xE0\xF5? \xC2 \xE3\xEB\xF3\xE1\xE8\xED\xE0\xF5 \xEE\xEA\xE5\xE0\xED\xE0? \xC2 \xE2\xE5\xF2\xF0\xE5?"
	},
	{
		12,		// cave voice 12
		// "Мы задумываемся - хорошая их "
		"\xCC\xFB \xE7\xE0\xE4\xF3\xEC\xFB\xE2\xE0\xE5\xEC\xF1\xFF - \xF5\xEE\xF0\xEE\xF8\xE0\xFF \xE8\xF5 "
		// "постигла судьба или плохая?"
		"\xEF\xEE\xF1\xF2\xE8\xE3\xEB\xE0 \xF1\xF3\xE4\xFC\xE1\xE0 \xE8\xEB\xE8 \xEF\xEB\xEE\xF5\xE0\xFF?"
	},
	{
		13,		// cave voice 13
		// "И не разделим ли мы однажды ту же участь?"
		"\xC8 \xED\xE5 \xF0\xE0\xE7\xE4\xE5\xEB\xE8\xEC \xEB\xE8 \xEC\xFB \xEE\xE4\xED\xE0\xE6\xE4\xFB \xF2\xF3 \xE6\xE5 \xF3\xF7\xE0\xF1\xF2\xFC?"
	} },
	// -----------------------------------------------------
	{ { // Hebrew
		10,		// cave voice 10
		"\xE5\xEB\xF2\xFA \xE0\xF0\xE5 \xEE\xE1\xE9\xE8\xE9\xED \xE0\xEC \xE4\xF9\xEE\xE9\xE9\xED, \xE0\xEC \xE4\xE0\xF8\xF5, \xE0\xEC \xE4\xEE\xE9\xED \xF9\xF7\xE9\xE1\xEC\xF0\xE5 "
		"\xEE\xE4\xED \xEC\xF0\xE7\xEC\xE4, \xE5\xFA\xE5\xE4\xE9\xED: \xEE\xE3\xE5\xF2 \xE4\xED \xF2\xE6\xE1\xE5?"
	},
	{
		11,		// cave voice 11
		"\xE4\xE0\xED \xE4\xED \xF2\xE3\xE9\xE9\xEF \xE7\xE9\xE9\xED, \xE1\xEB\xE5\xEB\xE1\xE9\xED? \xE1\xEE\xF2\xEE\xF7\xE9 "
		"\xE4\xE0\xE5\xF7\xE9\xE9\xF0\xE5\xF1? \xE1\xFA\xE5\xEA \xE4\xF8\xE5\xE7?"
	},
	{
		12,		// cave voice 12
		"\xE0\xF0\xE7\xF0\xE5 \xFA\xE5\xE4\xE9\xED, \xE4\xE0\xED \xE2\xE5\xF8\xEC\xED \xE4\xE9\xE4 \xE8\xE5\xE1 \xE0\xE5 \xF8\xF2?"
	},
	{
		13,		// cave voice 13
		"\xE5\xE4\xE0\xED \xF0\xE7\xEC\xE5\xF7 \xF2\xEE\xED \xE0\xFA \xE0\xE5\xFA\xE5 \xE4\xE2\xE5\xF8\xEC \xE1\xE9\xE5\xED \xEE\xEF \xE4\xE9\xEE\xE9\xED?"
	} },
	{ { // Chinese
		10,		// cave voice 10
		"\xb5\x4d\xa6\xd3\xb2\x7b\xa6\x62\xa7\xda\xad\xcc\xac\xdd\xa8\xa3\xa7\xda\xad\xcc\xa9\xd2\xc4\x7e\xa9\xd3\xaa\xba\xa4\xd1\xaa\xc5\xa1\x42\xa4\x6a\xa6\x61\x2c\xa5\x48\xa4\xce\xae\xfc\xac\x76\x2c\xa7\xda\xad\xcc\xa4\x5d\xa6\x6e\xa9\x5f\xa1\x47\xa4\x48\xc3\xfe\xac\xb0\xa4\xb0\xbb\xf2\xc2\xf7\xb6\x7d\xa4\x46\xa1\x48" /* 然而現在我們看見我們所繼承的天空、大地,以及海洋,我們也好奇：人類為什麼離開了？; And now we see the sky, the land, and the water that we are heirs to, and we wonder: why did they leave?  */
	},
	{
		11,		// cave voice 11
		"\xa5\x4c\xad\xcc\xac\x4f\xa4\xa3\xac\x4f\xa4\xb4\xb5\x4d\xa6\xed\xa6\x62\xa4\xd1\xa4\x57\xaa\xba\xac\x50\xb2\x79\xb8\xcc\xa1\x48\xa6\xed\xa6\x62\xae\xfc\xac\x76\xaa\xba\xb2\x60\xb3\x42\xa1\x48\xc1\xd9\xac\x4f\xa9\x7e\xa6\xed\xa6\x62\xad\xb7\xa4\xa4\xa1\x48" /* 他們是不是仍然住在天上的星球裡？住在海洋的深處？還是居住在風中？; Do they live still, in the stars? In the oceans depths? In the wind?  */
	},
	{
		12,		// cave voice 12
		"\xa7\xda\xad\xcc\xb7\x51\xaa\xbe\xb9\x44\xa1\x47\xa5\x4c\xad\xcc\xaa\xba\xa9\x52\xb9\x42\xac\x4f\xa6\x6e\xac\x4f\xc3\x61\xa1\x48" /* 我們想知道：他們的命運是好是壞？; We wonder, was their fate good or evil?  */
	},
	{
		13,		// cave voice 13
		"\xac\x4f\xa4\xa3\xac\x4f\xb1\x4e\xa8\xd3\xa7\xda\xad\xcc\xa4\x5d\xb7\x7c\xbe\x44\xb9\x4a\xa6\x50\xbc\xcb\xaa\xba\xa9\x52\xb9\x42\xa1\x48" /* 是不是將來我們也會遭遇同樣的命運？ And will we also share the same fate one day?  */
	} },
};

const IntroCredit creditsValley[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Producer"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Produzent"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Produttore"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Producteur"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Producer"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xEE\xF4\xE9\xF7"},
	// "Продюсер"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xCF\xF0\xEE\xE4\xFE\xF1\xE5\xF0"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xbb\x73\xa7\x40\xc1\x60\xba\xca" /* 製作總監 */},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	// "Уолтер Хохбрюкнер"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xD3\xEE\xEB\xF2\xE5\xF0 \xD5\xEE\xF5\xE1\xF0\xFE\xEA\xED\xE5\xF0"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Walter Hochbrueckner"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Executive Producer"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Ausf\201hrender Produzent"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Produttore Esecutivo"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Producteur Executif"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Executive Producer"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xEE\xF4\xE9\xF7 \xE1\xF4\xE5\xF2\xEC"},
	// "Исполнительный продюсер"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xC8\xF1\xEF\xEE\xEB\xED\xE8\xF2\xE5\xEB\xFC\xED\xFB\xE9 \xEF\xF0\xEE\xE4\xFE\xF1\xE5\xF0"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xb0\xf5\xa6\xe6\xbb\x73\xa7\x40" /* 執行製作 */},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Robert McNally"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Robert McNally"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Robert McNally"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Robert McNally"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Robert McNally"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Robert McNally"},
	// "Роберт Макнелли"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xD0\xEE\xE1\xE5\xF0\xF2 \xCC\xE0\xEA\xED\xE5\xEB\xEB\xE8"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Robert McNally"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsHeader, "2nd Executive Producer"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsHeader, "Publisher"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsHeader, "Herausgeber"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Editore"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Editeur"},
	{Common::JA_JPN, kITECreditsNotWyrmKeep, kITECreditsHeader, "Publisher"},
	{Common::HE_ISR, kITECreditsNotWyrmKeep, kITECreditsHeader, "\xF2\xF8\xE9\xEB\xE4"},
	// "Издатель"
	{Common::RU_RUS, kITECreditsNotWyrmKeep, kITECreditsHeader, "\xC8\xE7\xE4\xE0\xF2\xE5\xEB\xFC"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xa5\x58\xab\x7e\xa4\x48" /* 出品人 */},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"},
	// "Джон Ван Канегем"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xC4\xE6\xEE\xED \xC2\xE0\xED \xCA\xE0\xED\xE5\xE3\xE5\xEC"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Jon Van Caneghem"},
};

const IntroCredit creditsTreeHouse1[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Game Design"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Spielentwurf"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Progetto"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Conception"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Game Design"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xF2\xE9\xF6\xE5\xE1 \xE4\xEE\xF9\xE7\xF7"},
	// "Дизайн игры"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xC4\xE8\xE7\xE0\xE9\xED \xE8\xE3\xF0\xFB"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xb9\x43\xc0\xb8\xb3\x5d\xad\x70" /* 遊戲設計 */},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	// "Талин, Джо Пирс, Роберт Макнелли"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xD2\xE0\xEB\xE8\xED, \xC4\xE6\xEE \xCF\xE8\xF0\xF1, \xD0\xEE\xE1\xE5\xF0\xF2 \xCC\xE0\xEA\xED\xE5\xEB\xEB\xE8"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Talin, Joe Pearce, Robert McNally"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "and Carolly Hauksdottir"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "und Carolly Hauksdottir"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "e Carolly Hauksdottir"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "et Carolly Hauksdottir"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "and Carolly Hauksdottir"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "and Carolly Hauksdottir"},
	// "и Кэролли Хёйхсдоттир"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xE8 \xCA\xFD\xF0\xEE\xEB\xEB\xE8 \xD5\xB8\xE9\xF5\xF1\xE4\xEE\xF2\xF2\xE8\xF0"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "\xa4\xce Carolly Hauksdottir" /* 及 Carolly Hauksdottir */},
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
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "and Bill Rotsler"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xFA\xF1\xF8\xE9\xE8 \xE5\xEB\xFA\xE9\xE1\xE4"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Robert Leh, Len Wein, and Bill Rotsler"},
	// "Сценарий и диалоги"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xD1\xF6\xE5\xED\xE0\xF0\xE8\xE9 \xE8 \xE4\xE8\xE0\xEB\xEE\xE3\xE8"},
	// "Роберт Ле, Лен Уэйн и Билл Ротслер"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xD0\xEE\xE1\xE5\xF0\xF2 \xCB\xE5, \xCB\xE5\xED \xD3\xFD\xE9\xED \xE8 \xC1\xE8\xEB\xEB \xD0\xEE\xF2\xF1\xEB\xE5\xF0"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xb5\x65\xad\xb1\xa4\xce\xb9\xef\xa5\xd5" /* 畫面及對白 */},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Robert Leh, Len Wein"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "\xa4\xce Bill Rotsler" /*  及 Bill Rotsler */},
};

const IntroCredit creditsTreeHouse2[] = {
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsHeader, "Art Direction"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xac\xfc\xa4\x75\xab\xfc\xbe\xc9" /* 美工指導 */},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Allison Hershey"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Art"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Grafiken"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Grafica"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Graphismes"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Art"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xE0\xEE\xF0\xE5\xFA"},
	// "Художники"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xD5\xF3\xE4\xEE\xE6\xED\xE8\xEA\xE8"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsText, "Ed Lacabanne, Glenn Price, April Lee,"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	{Common::JA_JPN, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price,"},
	{Common::HE_ISR, kITECreditsNotWyrmKeep, kITECreditsText, "Edward Lacabanne, Glenn Price, April Lee,"},
	// "Эдвард Лакабанн, Гленн Прайс, Эйприл Ли,"
	{Common::RU_RUS, kITECreditsNotWyrmKeep, kITECreditsText, "\xDD\xE4\xE2\xE0\xF0\xE4 \xCB\xE0\xEA\xE0\xE1\xE0\xED\xED, \xC3\xEB\xE5\xED\xED \xCF\xF0\xE0\xE9\xF1, \xDD\xE9\xEF\xF0\xE8\xEB \xCB\xE8,"},
	{Common::UNK_LANG, kITECreditsWyrmKeep, kITECreditsText, "Lisa Sample, Brian Dowrick, Reed Waller,"},
	{Common::EN_ANY, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey and Talin"},
	{Common::DE_DEU, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey und Talin"},
	{Common::IT_ITA, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey e Talin"},
	{Common::FR_FRA, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey et Talin"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsText, "Lisa Iennaco, Brian Dowrick, Reed"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsText, "Lisa Iennaco, Brian Dowrick, Reed"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsText, "Lisa Iennaco, Brian Dowrick, Reed"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsText, "Lisa Iennaco, Brian Dowrick, Reed"},
	{Common::HE_ISR, kITECreditsWyrmKeep, kITECreditsText, "Allison Hershey and Talin"},
	// "Лиза Айеннако, Брайан Доурик, Рид"
	{Common::RU_RUS, kITECreditsNotWyrmKeep, kITECreditsText, "\xCB\xE8\xE7\xE0 \xC0\xE9\xE5\xED\xED\xE0\xEA\xEE, \xC1\xF0\xE0\xE9\xE0\xED \xC4\xEE\xF3\xF0\xE8\xEA, \xD0\xE8\xE4"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsText, "Waller, Allison Hershey and Talin"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Waller, Allison Hershey und Talin"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Waller, Allison Hershey e Talin"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Waller, Allison Hershey et Talin"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "April Lee, Lisa Iennaco,"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Brian Dowrick, Reed Waller,"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Allison Hershey and Talin"},
	{Common::HE_ISR, kITECreditsNotWyrmKeep, kITECreditsText, "Waller, Allison Hershey and Talin"},
	// "Уоллер, Эллисон Херши и Талин"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xD3\xEE\xEB\xEB\xE5\xF0, \xDD\xEB\xEB\xE8\xF1\xEE\xED \xD5\xE5\xF0\xF8\xE8 \xE8 \xD2\xE0\xEB\xE8\xED"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsHeader, "Art Direction"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsHeader, "Grafische Leitung"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Direzione Grafica"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsHeader, "Directeur Artistique"},
	{Common::JA_JPN, kITECreditsNotWyrmKeep, kITECreditsHeader, "Art Direction"},
	{Common::HE_ISR, kITECreditsNotWyrmKeep, kITECreditsHeader, "\xF0\xE9\xE4\xE5\xEC \xE0\xEE\xF0\xE5\xFA\xE9"},
	// "Художественный руководитель"
	{Common::RU_RUS, kITECreditsNotWyrmKeep, kITECreditsHeader, "\xD5\xF3\xE4\xEE\xE6\xE5\xF1\xF2\xE2\xE5\xED\xED\xFB\xE9 \xF0\xF3\xEA\xEE\xE2\xEE\xE4\xE8\xF2\xE5\xEB\xFC"},
	{Common::EN_ANY, kITECreditsNotWyrmKeep, kITECreditsText, "Allison Hershey"},
	{Common::DE_DEU, kITECreditsNotWyrmKeep, kITECreditsText, "Allison Hershey"},
	{Common::IT_ITA, kITECreditsNotWyrmKeep, kITECreditsText, "Allison Hershey"},
	{Common::FR_FRA, kITECreditsNotWyrmKeep, kITECreditsText, "Allison Hershey"},
	{Common::JA_JPN, kITECreditsNotWyrmKeep, kITECreditsText, "Allison Hershey"},
	{Common::HE_ISR, kITECreditsNotWyrmKeep, kITECreditsText, "Allison Hershey"},
	// "Эллисон Херши"
	{Common::RU_RUS, kITECreditsNotWyrmKeep, kITECreditsText, "\xDD\xEB\xEB\xE8\xF1\xEE\xED \xD5\xE5\xF0\xF8\xE8"}
};

const IntroCredit creditsFairePath1[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Programming"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Programmiert von"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Programmazione"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Programmeurs"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Programming (original)"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xF4\xE9\xFA\xE5\xE7"},
	// "Программирование"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xCF\xF0\xEE\xE3\xF0\xE0\xEC\xEC\xE8\xF0\xEE\xE2\xE0\xED\xE8\xE5"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xb5\x7b\xa6\xa1\xb3\x5d\xad\x70" /* 程式設計 */},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Talin, Walter Hochbrueckner,"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Talin, Walter Hochbrueckner,"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Talin, Walter Hochbrueckner,"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Talin, Walter Hochbrueckner,"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Talin, Walter Hochbrueckner,"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Talin, Walter Hochbrueckner,"},
	// "Талин, Уолтер Хохбрюкнер,"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xD2\xE0\xEB\xE8\xED, \xD3\xEE\xEB\xF2\xE5\xF0 \xD5\xEE\xF5\xE1\xF0\xFE\xEA\xED\xE5\xF0,"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Joe Burks and Robert Wiggins"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Joe Burks und Robert Wiggins"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Joe Burks e Robert Wiggins"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Joe Burks et Robert Wiggins"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Joe Burks and Robert Wiggins"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Joe Burks and Robert Wiggins"},
	// "Джо Буркс и Роберт Виггинс"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xC4\xE6\xEE \xC1\xF3\xF0\xEA\xF1 \xE8 \xD0\xEE\xE1\xE5\xF0\xF2 \xC2\xE8\xE3\xE3\xE8\xED\xF1"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Joe Burks \xa4\xce Robert Wiggins" /* Joe Burks 及 Robert Wiggins */},
	{Common::EN_ANY, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsHeader, "Additional Programming"},
	{Common::FR_FRA, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsHeader, "Programmeur Additionnel"},
	{Common::HE_ISR, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsHeader, "\xF4\xE9\xFA\xE5\xE7 \xF0\xE5\xF1\xF3"},
	// "Дополнительное программирование"
	{Common::RU_RUS, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsHeader, "\xC4\xEE\xEF\xEE\xEB\xED\xE8\xF2\xE5\xEB\xFC\xED\xEE\xE5 \xEF\xF0\xEE\xE3\xF0\xE0\xEC\xEC\xE8\xF0\xEE\xE2\xE0\xED\xE8\xE5"},
	{Common::EN_ANY, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsText, "John Bolton"},
	{Common::FR_FRA, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsText, "John Bolton"},
	{Common::HE_ISR, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsText, "John Bolton"},
	// "Джон Болтон"
	{Common::RU_RUS, kITECreditsPCCD | kITECreditsWyrmKeep, kITECreditsText, "\xC4\xE6\xEE\xED \xC1\xEE\xEB\xF2\xEE\xED"},
	{Common::UNK_LANG, kITECreditsMac, kITECreditsHeader, "Macintosh Version"},
	{Common::UNK_LANG, kITECreditsMac, kITECreditsText, "Michael McNally and Robert McNally"},
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Music and Sound"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Musik und Sound"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Musica e Sonoro"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Musique et Sons"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Music and Sound (original)"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xEE\xE5\xE6\xE9\xF7\xE4 \xE5\xF6\xEC\xE9\xEC\xE9\xED"},
	// "Музыка и звук"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xCC\xF3\xE7\xFB\xEA\xE0 \xE8 \xE7\xE2\xF3\xEA"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xCC\xF3\xE7\xFB\xEA\xE0 \xE8 \xE7\xE2\xF3\xEA" "\xad\xb5\xbc\xd6\xa4\xce\xad\xb5\xae\xc4" /* 音樂及音效 */},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Matt Nathan"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Matt Nathan"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Matt Nathan"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Matt Nathan"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Matt Nathan"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Matt Nathan"},
	// "Мэтт Натан"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xCC\xFD\xF2\xF2 \xCD\xE0\xF2\xE0\xED"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Matt Nathan"},
};

const IntroCredit creditsFairePath2[] = {
	{Common::EN_ANY, kITECreditsAny, kITECreditsHeader, "Directed by"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsHeader, "Regie"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsHeader, "Regia"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Dirig\202 par"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsHeader, "Directed by"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xE1\xE9\xEE\xE5\xE9"},
	// "Директор проекта"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xC4\xE8\xF0\xE5\xEA\xF2\xEE\xF0 \xEF\xF0\xEE\xE5\xEA\xF2\xE0"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsHeader, "\xb9\x43\xc0\xb8\xbe\xc9\xba\x74" /* 遊戲導演 */},
	{Common::EN_ANY, kITECreditsAny, kITECreditsText, "Talin"},
	{Common::DE_DEU, kITECreditsAny, kITECreditsText, "Talin"},
	{Common::IT_ITA, kITECreditsAny, kITECreditsText, "Talin"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Talin"},
	{Common::JA_JPN, kITECreditsAny, kITECreditsText, "Talin"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "Talin"},
	{Common::ZH_TWN, kITECreditsAny, kITECreditsText, "Talin"},
	// "Талин"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xD2\xE0\xEB\xE8\xED"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsHeader, "Traduction Francaise"},
	{Common::FR_FRA, kITECreditsAny, kITECreditsText, "Ryfatwork, Momo le Yetty et Darknior"},
	// "Перевод на русский язык"
	{Common::RU_RUS, kITECreditsAny, kITECreditsHeader, "\xCF\xE5\xF0\xE5\xE2\xEE\xE4 \xED\xE0 \xF0\xF3\xF1\xF1\xEA\xE8\xE9 \xFF\xE7\xFB\xEA"},
	// "Бюро переводов Old-games.ru"
	{Common::RU_RUS, kITECreditsAny, kITECreditsText, "\xC1\xFE\xF0\xEE \xEF\xE5\xF0\xE5\xE2\xEE\xE4\xEE\xE2 Old-games.ru"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsHeader, "\xEE\xE4\xE3\xE5\xF8\xE4 \xF2\xE1\xF8\xE9\xFA"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "\xE0\xEC\xF2\xE3 \xE4\xEC\xF4\xF8\xE9\xEF, \xF2\xE5\xEE\xF8 \xEB\xE4\xEF \xE5\xF0\xE9\xE1 \xE1\xE0\xF8"},
	{Common::HE_ISR, kITECreditsAny, kITECreditsText, "\xEE\xE1\xE9\xFA \xE4\xF8\xF4\xFA\xF7\xE4 \xF2\xE1\xF8\xE9\xFA"},
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
