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

#ifndef SKY34COMP_H
#define SKY34COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc34_fast_list[] = {
	12+DISK_14,
	51+DISK_14,
	52+DISK_14,
	53+DISK_14,
	54+DISK_14,
	135+DISK_14,
	0
};

uint16 sc34_mouse_list[] = {
	ID_SC34_SECATEURS,
	ID_SC34_TKT_MACHINE,
	ID_SC34_MAP,
	ID_SC34_BRICKS,
	ID_SC34_DOOR,
	ID_SC34_FLOOR,
	ID_SC32_LIFT,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc34_secateurs = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	34,	// screen
	0,	// place
	0,	// getToTable
	259,	// xcood
	229,	// ycood
	104*64,	// frame
	180,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	269-259,	// mouseSize_x
	232-229,	// mouseSize_y
	SC34_SECATEURS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC34_SECATEURS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint32 *sc34_walk_grid = (uint32*)sc34_fast_list;

uint16 sc34_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_SC34_SECATEURS,
	ID_SC32_LIFT,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc34_get_secs[] = {
	105*64,
	248,
	208,
	0,
	248,
	208,
	1,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	1,
	248,
	208,
	0,
	0
};

uint16 sc34_chip_list[] = {
	IT_SC34_LAYER_0+DISK_14,
	IT_SC34_LAYER_1+DISK_14,
	IT_SC34_GRID_1+DISK_14,
	IT_SC34_SECATEURS+DISK_14,
	IT_SC34_GET_SECS+DISK_14,
	IT_SC34_STAIRS1+DISK_14,
	IT_SC34_STAIRS2+DISK_14,
	0
};

uint16 reset_33_34[] = {
	C_SCREEN,
	34,
	C_PLACE,
	ID_SC34_FLOOR,
	C_XCOOD,
	168,
	C_YCOOD,
	272,
	65535
};

uint16 sc34_stairs2[] = {
	103*64,
	375,
	251,
	0,
	375,
	251,
	1,
	378,
	251,
	2,
	382,
	251,
	3,
	379,
	251,
	4,
	376,
	251,
	5,
	374,
	251,
	6,
	374,
	249,
	7,
	373,
	247,
	8,
	370,
	247,
	9,
	367,
	247,
	10,
	364,
	247,
	11,
	364,
	244,
	12,
	363,
	242,
	13,
	359,
	241,
	14,
	357,
	241,
	15,
	357,
	239,
	16,
	356,
	236,
	17,
	355,
	234,
	18,
	351,
	233,
	19,
	348,
	233,
	20,
	347,
	231,
	21,
	346,
	228,
	22,
	345,
	225,
	23,
	342,
	224,
	24,
	340,
	224,
	25,
	337,
	224,
	26,
	336,
	220,
	27,
	336,
	218,
	28,
	333,
	218,
	29,
	325,
	219,
	30,
	325,
	218,
	31,
	325,
	218,
	32,
	324,
	218,
	3,
	0
};

uint16 sc34_floor_table[] = {
	ID_SC34_FLOOR,
	RET_OK,
	ID_SC34_DOOR,
	GT_SC34_DOOR,
	0,
	SC34_DOOR_WALK_ON,
	ID_SC34_SECATEURS,
	GT_SC34_SECATEURS,
	ID_SC34_TKT_MACHINE,
	GT_SC34_TKT_MACHINE,
	ID_SC34_MAP,
	GT_SC34_MAP,
	ID_SC34_BRICKS,
	GT_SC34_BRICKS,
	65535
};

Compact sc34_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	34,	// screen
	0,	// place
	sc34_floor_table,	// getToTable
	208,	// xcood
	248,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	343-208,	// mouseSize_x
	311-248,	// mouseSize_y
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

Compact sc34_bricks = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	34,	// screen
	0,	// place
	0,	// getToTable
	378,	// xcood
	190,	// ycood
	0,	// frame
	16648,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	410-378,	// mouseSize_x
	272-190,	// mouseSize_y
	SC34_BRICKS_ACTION,	// actionScript
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

uint16 sc34_stairs1[] = {
	102*64,
	314,
	218,
	0,
	315,
	218,
	1,
	315,
	219,
	2,
	315,
	219,
	3,
	321,
	219,
	4,
	329,
	218,
	5,
	335,
	220,
	6,
	335,
	221,
	7,
	336,
	226,
	8,
	340,
	227,
	9,
	342,
	228,
	10,
	343,
	229,
	11,
	345,
	232,
	12,
	348,
	233,
	13,
	351,
	234,
	14,
	353,
	235,
	15,
	354,
	239,
	16,
	358,
	240,
	9,
	360,
	241,
	10,
	362,
	242,
	11,
	363,
	245,
	17,
	366,
	246,
	18,
	369,
	247,
	19,
	369,
	248,
	20,
	370,
	251,
	21,
	373,
	252,
	22,
	375,
	252,
	23,
	375,
	251,
	24,
	376,
	251,
	25,
	380,
	251,
	0,
	380,
	251,
	26,
	380,
	251,
	27,
	380,
	251,
	28,
	380,
	251,
	29,
	380,
	251,
	30,
	380,
	251,
	31,
	379,
	251,
	32,
	379,
	251,
	32,
	380,
	251,
	33,
	380,
	251,
	34,
	380,
	251,
	28,
	380,
	251,
	26,
	380,
	251,
	0,
	0
};

Compact sc34_tkt_machine = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	34,	// screen
	0,	// place
	0,	// getToTable
	217,	// xcood
	195,	// ycood
	0,	// frame
	16646,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	270-217,	// mouseSize_x
	226-195,	// mouseSize_y
	SC34_TKT_MACHINE_ACTION,	// actionScript
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

Compact sc34_map = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	34,	// screen
	0,	// place
	0,	// getToTable
	276,	// xcood
	185,	// ycood
	0,	// frame
	16647,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	321-276,	// mouseSize_x
	220-185,	// mouseSize_y
	SC34_MAP_ACTION,	// actionScript
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

uint16 sc34_palette[] = {
	0,
	512,
	514,
	517,
	770,
	1283,
	1029,
	1286,
	773,
	1284,
	2056,
	1028,
	1287,
	1543,
	1543,
	1797,
	2571,
	1542,
	1800,
	4873,
	772,
	2311,
	2829,
	1801,
	2569,
	2826,
	2825,
	2318,
	3849,
	1546,
	2826,
	2829,
	4108,
	2830,
	5132,
	2058,
	3596,
	4364,
	2315,
	3342,
	4367,
	3085,
	3597,
	5906,
	2316,
	3603,
	3853,
	4112,
	4110,
	3861,
	4879,
	3858,
	5647,
	3342,
	4625,
	7182,
	2573,
	4367,
	4888,
	4625,
	4376,
	6925,
	3343,
	5136,
	4887,
	5395,
	5140,
	7955,
	2319,
	4633,
	4624,
	6676,
	4636,
	5646,
	5652,
	5651,
	6935,
	4628,
	6166,
	5649,
	5654,
	5656,
	5139,
	6679,
	5154,
	6414,
	5655,
	5910,
	8477,
	4373,
	6172,
	5912,
	5659,
	6174,
	6419,
	6681,
	5668,
	5904,
	7962,
	6684,
	6683,
	6429,
	6937,
	9506,
	4633,
	6691,
	7703,
	7452,
	6945,
	6939,
	7199,
	7708,
	10528,
	5148,
	8732,
	8218,
	7966,
	8735,
	8726,
	7198,
	8221,
	8483,
	8736,
	7720,
	7450,
	11039,
	8223,
	8742,
	7969,
	8990,
	6945,
	12832,
	9760,
	9499,
	8481,
	9251,
	8221,
	8997,
	8997,
	8740,
	8488,
	10018,
	9509,
	7976,
	9769,
	11037,
	8483,
	9511,
	9510,
	10534,
	11048,
	9499,
	10025,
	10020,
	11312,
	10024,
	11302,
	11046,
	9260,
	10791,
	10035,
	10798,
	11818,
	11308,
	12588,
	12077,
	12582,
	11051,
	11563,
	11575,
	13359,
	12592,
	10794,
	15151,
	12081,
	12081,
	14129,
	12596,
	12081,
	15154,
	13366,
	13619,
	13877,
	13619,
	13882,
	14646,
	14392,
	14907,
	14393,
	14906,
	15677,
	15419,
	0,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
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
	0,
	63,
	16128,
	0,
	63,
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

Compact sc34_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	34,	// screen
	0,	// place
	0,	// getToTable
	188,	// xcood
	215,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	206-188,	// mouseSize_x
	275-215,	// mouseSize_y
	SC34_DOOR_ACTION,	// actionScript
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
