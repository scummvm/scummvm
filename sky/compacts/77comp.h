/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$ 
 *
 */

#ifndef SKY77COMP_H
#define SKY77COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc77_fpushl_2[] = {
	74*64,
	1,
	1,
	5,
	1,
	1,
	7,
	1,
	1,
	8,
	1,
	1,
	9,
	1,
	1,
	0,
	0
};

uint16 sc77_palette[] = {
	0,
	768,
	1540,
	774,
	2307,
	1028,
	1541,
	1286,
	2054,
	1035,
	1796,
	1286,
	1037,
	1796,
	1798,
	1543,
	2825,
	1286,
	1039,
	1284,
	1798,
	2055,
	4362,
	1028,
	1801,
	3849,
	1286,
	2054,
	3593,
	2312,
	2064,
	4615,
	1542,
	2312,
	2315,
	3338,
	1302,
	2310,
	3082,
	1304,
	2052,
	2826,
	2066,
	2057,
	3339,
	2576,
	5129,
	1800,
	3082,
	5902,
	1544,
	1562,
	5638,
	2312,
	2578,
	2827,
	4110,
	2076,
	3078,
	3853,
	3346,
	2828,
	3596,
	2837,
	3340,
	4110,
	2839,
	3081,
	4368,
	2078,
	3080,
	3597,
	2586,
	5386,
	3086,
	4110,
	8723,
	2057,
	2845,
	5899,
	3598,
	3355,
	4107,
	4881,
	4371,
	7953,
	2315,
	4623,
	5653,
	4368,
	4110,
	4370,
	5394,
	4121,
	3854,
	4625,
	3358,
	4109,
	5651,
	3868,
	8205,
	2829,
	4885,
	8208,
	3854,
	4631,
	9747,
	2573,
	3619,
	4876,
	5909,
	4127,
	5905,
	5396,
	5138,
	6421,
	4884,
	4636,
	9746,
	3342,
	5653,
	5143,
	5910,
	4641,
	8719,
	4625,
	6420,
	5400,
	6679,
	6422,
	6684,
	6424,
	6171,
	8470,
	5397,
	4648,
	8975,
	4884,
	5917,
	6168,
	6681,
	5162,
	6160,
	7451,
	6175,
	10010,
	5908,
	6938,
	6431,
	7195,
	6688,
	9756,
	5656,
	5675,
	10515,
	5399,
	6690,
	6937,
	7709,
	7451,
	8993,
	7195,
	7451,
	10526,
	6426,
	6445,
	9239,
	7454,
	8221,
	8993,
	8224,
	8480,
	11554,
	7453,
	7728,
	10269,
	8482,
	8240,
	10528,
	8996,
	9250,
	13093,
	7969,
	9263,
	10018,
	10536,
	9780,
	13093,
	11567,
	2569,
	14860,
	14651,
	3854,
	3857,
	4624,
	39,
	2560,
	3339,
	5912,
	1560,
	2311,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	39,
	9984,
	0,
	0,
	14336,
	13364,
	11569,
	9261,
	8481,
	6687,
	6681,
	4626,
	2578,
	1803,
	770,
	519,
	13571,
	9517,
	9778,
	11805,
	5151,
	5926,
	8205,
	1808,
	2586,
	11267,
	2056,
	8210,
	7441,
	6954,
	9494,
	4119,
	3866,
	4864,
	13316,
	13364,
	11563,
	8751,
	9506,
	10024,
	8242,
	10527,
	6174,
	5927,
	7698,
	3600,
	2325,
	2308,
	13886,
	14637,
	9006,
	10036,
	12315,
	5152,
	6443,
	9997,
	2067,
	9533,
	13100,
	9244,
	5162,
	8476,
	5390,
	2072,
	3855,
	2308,
	2865,
	9995,
	1285,
	285,
	4865,
	0,
	11573,
	12837,
	7462,
	0,
	16128,
	0,
	16191,
	10047,
	63,
	16167,
	9984,
	63,
	16167,
	13824,
	13878,
	12077,
	8241,
	10527,
	5917,
	5925,
	7698,
	2865,
	9995,
	1285,
	285,
	16129,
	16191
};

Compact sc77_big_door = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	77,	// screen
	0,	// place
	0,	// getToTable
	380,	// xcood
	197,	// ycood
	28*64,	// frame
	181,	// cursorText
	SC77_BIG_DOOR_MOUSE_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	0,	// mouseRel_y
	392-375,	// mouseSize_x
	268-197,	// mouseSize_y
	SC77_BIG_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC77_BIG_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc77_fpushl_1[] = {
	74*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	1,
	1,
	4,
	1,
	1,
	5,
	1,
	1,
	6,
	0
};

uint16 sc77_fast_list[] = {
	12+DISK_10,
	51+DISK_10,
	52+DISK_10,
	53+DISK_10,
	54+DISK_10,
	268+DISK_10,
	0
};

uint16 sc77_logic_list[] = {
	ID_FOSTER,
	ID_KEN,
	ID_SC77_BIG_DOOR,
	ID_SC75_TONGS,
	ID_SC67_PULSE1,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc77_door_open[] = {
	28*64,
	380,
	197,
	0,
	380,
	197,
	1,
	380,
	197,
	2,
	380,
	197,
	3,
	380,
	197,
	4,
	380,
	197,
	5,
	380,
	197,
	6,
	380,
	197,
	7,
	0
};

uint16 sc77_chip_list[] = {
	IT_SC77_LAYER_0+DISK_10,
	IT_SC77_LAYER_1+DISK_10,
	IT_SC77_GRID_1+DISK_10,
	IT_KEN+DISK_10,
	IT_SC76_KEN_TALK+DISK_10,
	IT_SC77_BIG_DOOR+DISK_10,
	IT_SC77_FPUSHL+DISK_10,
	IT_SC77_FPUSHR+DISK_10,
	IT_SC77_KPUSHR+DISK_10,
	IT_SC77_STRETCH+DISK_10,
	102+DISK_10,
	0
};

uint32 *grid77 = 0;

uint16 reset_78_77[] = {
	C_SCREEN,
	77,
	C_PLACE,
	ID_SC77_FLOOR,
	C_XCOOD,
	408,
	C_YCOOD,
	264,
	65535
};

uint16 sc77_stretch[] = {
	87*64,
	361,
	224,
	0,
	361,
	224,
	1,
	361,
	224,
	2,
	361,
	224,
	1,
	361,
	224,
	0,
	361,
	224,
	0,
	361,
	224,
	1,
	361,
	224,
	2,
	361,
	224,
	2,
	361,
	224,
	1,
	361,
	224,
	1,
	361,
	224,
	1,
	361,
	224,
	2,
	361,
	224,
	2,
	361,
	224,
	2,
	361,
	224,
	0,
	361,
	224,
	1,
	361,
	224,
	2,
	361,
	224,
	2,
	361,
	224,
	1,
	361,
	224,
	0,
	361,
	224,
	1,
	361,
	224,
	3,
	361,
	224,
	4,
	361,
	224,
	3,
	361,
	224,
	1,
	361,
	224,
	0,
	361,
	224,
	3,
	361,
	224,
	4,
	361,
	224,
	3,
	361,
	224,
	4,
	361,
	224,
	2,
	361,
	224,
	3,
	361,
	224,
	5,
	361,
	224,
	6,
	361,
	224,
	7,
	361,
	224,
	6,
	361,
	224,
	7,
	361,
	224,
	6,
	361,
	224,
	3,
	361,
	224,
	1,
	361,
	224,
	3,
	361,
	224,
	1,
	361,
	224,
	0,
	361,
	224,
	1,
	361,
	224,
	3,
	361,
	224,
	7,
	361,
	224,
	3,
	361,
	224,
	5,
	361,
	224,
	6,
	361,
	224,
	5,
	361,
	224,
	5,
	361,
	224,
	7,
	361,
	224,
	6,
	361,
	224,
	3,
	361,
	224,
	0,
	361,
	224,
	0,
	361,
	224,
	3,
	361,
	224,
	6,
	361,
	224,
	7,
	361,
	224,
	8,
	361,
	224,
	9,
	0
};

uint16 sc77_joey_list[] = {
	184,
	391,
	264,
	279,
	1,
	376,
	391,
	264,
	271,
	0,
	0
};

uint16 sc77_mouse_list[] = {
	ID_KEN,
	ID_SC77_HAND_1,
	ID_SC77_HAND_2,
	ID_SC77_TANK_1,
	ID_SC77_TANK_2,
	ID_SC77_DOOR76,
	ID_SC77_BIG_DOOR,
	ID_SC77_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 reset_76_77[] = {
	C_SCREEN,
	77,
	C_PLACE,
	ID_SC77_FLOOR,
	C_XCOOD,
	152,
	C_YCOOD,
	264,
	65535
};

uint16 sc77_fpushr_2[] = {
	75*64,
	1,
	1,
	3,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 sc77_floor_table[] = {
	ID_SC77_FLOOR,
	RET_OK,
	ID_SC77_DOOR76,
	GT_SC77_DOOR76,
	0,
	SC77_DOOR76_WALK_ON,
	ID_SC77_BIG_DOOR,
	GT_SC77_BIG_DOOR,
	1,
	SC77_BIG_DOOR_WALK_ON,
	ID_SC77_TANK_1,
	GT_SC77_TANKS,
	ID_SC77_TANK_2,
	GT_SC77_TANKS,
	ID_SC77_HAND_1,
	GT_SC77_HAND_1,
	ID_SC77_HAND_2,
	GT_SC77_HAND_2,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	ID_KEN,
	GT_SC77_STUCK_KEN,
	65535
};

uint16 sc77_kpushr_2[] = {
	76*64,
	1,
	1,
	3,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

Compact sc77_tank_1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	77,	// screen
	0,	// place
	0,	// getToTable
	199,	// xcood
	157,	// ycood
	0,	// frame
	20568,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	281-199,	// mouseSize_x
	265-157,	// mouseSize_y
	SC77_TANKS_ACTION,	// actionScript
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

uint16 sc77_fpushr_1[] = {
	75*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	1,
	1,
	4,
	0
};

Compact sc77_tank_2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	77,	// screen
	0,	// place
	0,	// getToTable
	289,	// xcood
	157,	// ycood
	0,	// frame
	20568,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	365-289,	// mouseSize_x
	265-157,	// mouseSize_y
	SC77_TANKS_ACTION,	// actionScript
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

Compact sc77_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	77,	// screen
	0,	// place
	sc77_floor_table,	// getToTable
	188,	// xcood
	262,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	382-188,	// mouseSize_x
	280-262,	// mouseSize_y
	FLOOR_ACTION,	// actionScript
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

Compact sc77_hand_2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	77,	// screen
	0,	// place
	0,	// getToTable
	393,	// xcood
	236,	// ycood
	0,	// frame
	20571,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	400-393,	// mouseSize_x
	251-236,	// mouseSize_y
	SC77_HAND_2_ACTION,	// actionScript
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

Compact sc77_hand_1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	77,	// screen
	0,	// place
	0,	// getToTable
	364,	// xcood
	222,	// ycood
	0,	// frame
	20571,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	371-364,	// mouseSize_x
	233-222,	// mouseSize_y
	SC77_HAND_1_ACTION,	// actionScript
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

uint16 sc77_kpushr_1[] = {
	76*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	1,
	1,
	4,
	0
};

Compact sc77_door76 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	77,	// screen
	0,	// place
	0,	// getToTable
	174,	// xcood
	205,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	190-174,	// mouseSize_x
	270-205,	// mouseSize_y
	SC77_DOOR76_ACTION,	// actionScript
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
