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

#ifndef SKY41COMP_H
#define SKY41COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc41_mouse_list[] = {
	ID_SC41_EXIT_39,
	ID_SC41_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc41_heat_2_anim[] = {
	72*64,
	265,
	244,
	0,
	265,
	244,
	1,
	265,
	244,
	2,
	265,
	244,
	3,
	265,
	244,
	4,
	265,
	244,
	5,
	265,
	244,
	6,
	265,
	244,
	7,
	265,
	244,
	8,
	265,
	244,
	9,
	0
};

Compact sc41_heat_1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	41,	// screen
	0,	// place
	0,	// getToTable
	211,	// xcood
	294,	// ycood
	71*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC41_HEAT_1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 reset_39_41[] = {
	C_SCREEN,
	41,
	C_PLACE,
	ID_SC41_FLOOR,
	C_XCOOD,
	336,
	C_YCOOD,
	176,
	C_MEGA_SET,
	0,
	C_FRAME,
	40+13*64,
	C_DIR,
	DOWN,
	65535
};

uint16 sc41_heat_1_anim[] = {
	71*64,
	211,
	294,
	0,
	211,
	294,
	1,
	211,
	294,
	2,
	211,
	294,
	3,
	211,
	294,
	4,
	211,
	294,
	5,
	211,
	294,
	6,
	211,
	294,
	7,
	211,
	294,
	8,
	211,
	294,
	9,
	0
};

uint32 *sc41_walk_grid = (uint32*)sc41_fast_list;

uint16 sc41_fast_list[] = {
	13+DISK_14,
	0
};

Compact sc41_heat_2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	41,	// screen
	0,	// place
	0,	// getToTable
	265,	// xcood
	244,	// ycood
	72*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC41_HEAT_2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc41_chip_list[] = {
	IT_SC41_LAYER_0+DISK_14,
	IT_SC41_LAYER_1+DISK_14,
	IT_SC41_LAYER_2+DISK_14,
	IT_SC41_GRID_1+DISK_14,
	IT_SC41_GRID_2+DISK_14,
	71+DISK_14,
	72+DISK_14,
	74+DISK_14,
	0
};

Compact sc41_heat_3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	41,	// screen
	0,	// place
	0,	// getToTable
	390,	// xcood
	294,	// ycood
	74*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC41_HEAT_3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc41_exit_39 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	41,	// screen
	0,	// place
	0,	// getToTable
	325,	// xcood
	190,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	342-325,	// mouseSize_x
	205-190,	// mouseSize_y
	SC41_EXIT_39_ACTION,	// actionScript
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

uint16 sc41_floor_table[] = {
	ID_SC41_FLOOR,
	RET_OK,
	ID_SC41_EXIT_39,
	GT_SC41_EXIT_39,
	0,
	SC41_EXIT_39_WALK_ON,
	65535
};

Compact sc41_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	41,	// screen
	0,	// place
	sc41_floor_table,	// getToTable
	328,	// xcood
	208,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	343-328,	// mouseSize_x
	223-208,	// mouseSize_y
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

uint16 sc41_heat_3_anim[] = {
	74*64,
	390,
	294,
	0,
	390,
	294,
	1,
	390,
	294,
	2,
	390,
	294,
	3,
	390,
	294,
	4,
	390,
	294,
	5,
	390,
	294,
	6,
	390,
	294,
	7,
	390,
	294,
	8,
	390,
	294,
	9,
	0
};

uint16 sc41_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_SC41_HEAT_1,
	ID_SC41_HEAT_2,
	ID_SC41_HEAT_3,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc41_palette[] = {
	0,
	512,
	514,
	772,
	1539,
	1028,
	1540,
	2056,
	1030,
	1544,
	2566,
	1542,
	1546,
	2054,
	2056,
	1548,
	4102,
	1028,
	1040,
	2054,
	2568,
	1548,
	3080,
	1544,
	2060,
	3592,
	1544,
	1554,
	3590,
	2056,
	2574,
	5130,
	1544,
	2068,
	4104,
	2570,
	3084,
	4620,
	2058,
	2580,
	5638,
	2056,
	3086,
	4620,
	2572,
	3598,
	6158,
	2570,
	3094,
	7690,
	1544,
	3096,
	5128,
	3086,
	3098,
	7176,
	3082,
	3098,
	5642,
	3086,
	4116,
	5134,
	4112,
	2593,
	7176,
	2062,
	4118,
	8462,
	3082,
	3612,
	6154,
	3600,
	3614,
	6666,
	3600,
	3107,
	6664,
	3090,
	4124,
	6672,
	4114,
	4636,
	7694,
	3090,
	3619,
	6158,
	4628,
	3619,
	10000,
	2062,
	3623,
	8970,
	3088,
	5146,
	8978,
	3600,
	4643,
	7692,
	4628,
	5153,
	7184,
	5142,
	4139,
	8972,
	4628,
	4141,
	10506,
	3090,
	6172,
	11030,
	2578,
	5159,
	12046,
	3088,
	5669,
	10000,
	3606,
	5669,
	8468,
	5656,
	5163,
	12560,
	3090,
	6689,
	10006,
	4632,
	6183,
	8980,
	5658,
	5169,
	12046,
	3606,
	6189,
	9486,
	6170,
	5679,
	8466,
	7196,
	6189,
	12562,
	4118,
	6697,
	8984,
	7708,
	6191,
	10004,
	6172,
	6699,
	10006,
	6684,
	6195,
	12048,
	5658,
	7719,
	12570,
	5146,
	6705,
	13590,
	4634,
	7215,
	9496,
	7713,
	7217,
	11546,
	7198,
	7727,
	13594,
	5148,
	8489,
	13086,
	5662,
	8491,
	11548,
	7201,
	8999,
	13089,
	6177,
	8501,
	12058,
	8483,
	9009,
	13598,
	7203,
	10027,
	12581,
	8485,
	9523,
	13601,
	8999,
	10039,
	13603,
	10027,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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
	11573,
	12837,
	7462,
	7982,
	9748,
	3351,
	4128,
	6663,
	778,
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
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	13887,
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

} // namespace SkyCompact

} // namespace Sky

#endif
