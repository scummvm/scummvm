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

#ifndef SKY65COMP_H
#define SKY65COMP_H




namespace Sky {

namespace SkyCompact {

uint32 *sc65_walk_grid = 0;

Compact sc65_exit_48 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	65,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	161,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	140-128,	// mouseSize_x
	327-161,	// mouseSize_y
	SC65_EXIT_48_ACTION,	// actionScript
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

uint16 sc65_floor_table[] = {
	ID_SC65_FLOOR,
	RET_OK,
	ID_SC65_POSTER1,
	GT_SC65_POSTER1,
	ID_SC65_POSTER2,
	GT_SC65_POSTER2,
	ID_SC65_SIGN,
	GT_SC65_SIGN,
	ID_SC65_EXIT_48,
	GT_SC65_EXIT_48,
	0,
	SC65_EXIT_48_WALK_ON,
	ID_SC65_EXIT_66,
	GT_SC65_EXIT_66,
	65535
};

Compact sc65_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	65,	// screen
	0,	// place
	sc65_floor_table,	// getToTable
	128,	// xcood
	272,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	375-128,	// mouseSize_x
	327-272,	// mouseSize_y
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

uint16 reset_48_65[] = {
	C_SCREEN,
	65,
	C_PLACE,
	ID_SC65_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	288,
	65535
};

uint16 sc65_chip_list[] = {
	IT_SC65_LAYER_0+DISK_13,
	IT_SC65_LAYER_1+DISK_13,
	IT_SC65_GRID_1+DISK_13,
	0
};

Compact sc65_exit_66 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	65,	// screen
	0,	// place
	0,	// getToTable
	376,	// xcood
	240,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	386-376,	// mouseSize_x
	306-240,	// mouseSize_y
	LINK_65_66,	// actionScript
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

uint16 sc65_mouse_list[] = {
	ID_SC65_POSTER1,
	ID_SC65_POSTER2,
	ID_SC65_SIGN,
	ID_SC65_EXIT_48,
	ID_SC65_EXIT_66,
	ID_SC65_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc65_fast_list[] = {
	12+DISK_13,
	51+DISK_13,
	52+DISK_13,
	53+DISK_13,
	54+DISK_13,
	0
};

Compact sc65_poster2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	65,	// screen
	0,	// place
	0,	// getToTable
	227,	// xcood
	175,	// ycood
	0,	// frame
	17386,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	246-227,	// mouseSize_x
	201-175,	// mouseSize_y
	SC65_POSTER2_ACTION,	// actionScript
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

Compact sc65_poster1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	65,	// screen
	0,	// place
	0,	// getToTable
	204,	// xcood
	175,	// ycood
	0,	// frame
	17386,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	224-204,	// mouseSize_x
	201-175,	// mouseSize_y
	SC65_POSTER1_ACTION,	// actionScript
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

Compact sc65_sign = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	65,	// screen
	0,	// place
	0,	// getToTable
	310,	// xcood
	177,	// ycood
	0,	// frame
	16677,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	347-310,	// mouseSize_x
	201-177,	// mouseSize_y
	SC65_SIGN_ACTION,	// actionScript
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

uint16 sc65_logic_list[] = {
	ID_FOSTER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc65_palette[] = {
	0,
	1280,
	1028,
	1540,
	1540,
	1541,
	1288,
	1540,
	1030,
	1544,
	1542,
	1543,
	2056,
	2565,
	1542,
	1799,
	3080,
	1030,
	2058,
	2053,
	1801,
	2058,
	2056,
	2314,
	2061,
	3846,
	1031,
	2571,
	3078,
	2314,
	2570,
	2571,
	2316,
	2320,
	4614,
	1033,
	3084,
	4618,
	1545,
	2830,
	3335,
	3083,
	3339,
	3852,
	2571,
	3596,
	4361,
	2059,
	2835,
	3334,
	3597,
	3343,
	3852,
	2318,
	3090,
	3339,
	3087,
	3092,
	5641,
	1547,
	3602,
	4105,
	3854,
	4110,
	3855,
	2577,
	3602,
	4109,
	3344,
	3604,
	4619,
	2832,
	3353,
	5127,
	4110,
	4116,
	4621,
	3856,
	4367,
	4114,
	3858,
	4880,
	5644,
	2576,
	4118,
	6412,
	2319,
	4625,
	6161,
	3088,
	4625,
	4628,
	3604,
	4630,
	4876,
	4370,
	5137,
	6418,
	3602,
	5652,
	7693,
	2065,
	5139,
	5908,
	3604,
	5651,
	6416,
	4116,
	5142,
	6933,
	3348,
	5653,
	5395,
	3864,
	5652,
	8214,
	2067,
	5654,
	5653,
	5654,
	6170,
	7434,
	3350,
	5403,
	5908,
	4377,
	6169,
	6935,
	5144,
	5665,
	6670,
	4378,
	6679,
	7959,
	4376,
	5924,
	6410,
	6426,
	7194,
	8468,
	3354,
	7196,
	9753,
	3864,
	6688,
	8726,
	6169,
	6950,
	9483,
	4378,
	7198,
	9243,
	2334,
	6442,
	8718,
	4893,
	7708,
	9246,
	6428,
	7715,
	7958,
	6943,
	7976,
	10766,
	2847,
	7721,
	8980,
	7711,
	8483,
	8481,
	8739,
	8487,
	11293,
	4386,
	9001,
	9493,
	8483,
	9514,
	11037,
	6182,
	10022,
	12325,
	2858,
	10542,
	12563,
	4903,
	10543,
	12830,
	5931,
	10794,
	11818,
	9002,
	12055,
	4159,
	16172,
	10760,
	63,
	16167,
	8960,
	57,
	13087,
	6912,
	45,
	10007,
	13878,
	11839,
	16175,
	10023,
	7999,
	16160,
	6167,
	4159,
	16144,
	2312,
	63,
	16129,
	0,
	63,
	15104,
	0,
	56,
	13568,
	0,
	50,
	12032,
	0,
	44,
	10496,
	0,
	38,
	8704,
	0,
	31,
	7168,
	0,
	25,
	5632,
	0,
	19,
	4096,
	13884,
	14655,
	16174,
	10038,
	13375,
	16159,
	5938,
	12095,
	16144,
	2093,
	10815,
	16128,
	38,
	8249,
	13056,
	29,
	6189,
	9984,
	20,
	4385,
	7168,
	13,
	2582,
	4096,
	13887,
	16191,
	16174,
	10047,
	16191,
	16159,
	5951,
	16191,
	16144,
	2111,
	16191,
	16128,
	56,
	12857,
	13056,
	45,
	10029,
	9984,
	33,
	6945,
	7168,
	22,
	4118,
	4096,
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
	2623,
	11569,
	10762,
	2609,
	12575,
	5130,
	13873,
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
