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

#ifndef SKY45COMP_H
#define SKY45COMP_H




namespace Sky {

namespace SkyCompact {

uint16 reset_44_45[] = {
	C_SCREEN,
	45,
	C_PLACE,
	ID_SC45_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	280,
	65535
};

uint16 sc45_logic_list[] = {
	ID_FOSTER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc45_floor_table[] = {
	ID_SC45_FLOOR,
	RET_OK,
	ID_SC45_EXIT_44,
	GT_SC45_EXIT_44,
	0,
	SC45_EXIT_44_WALK_ON,
	ID_SC45_EXIT_46,
	GT_SC45_EXIT_46,
	1,
	SC45_EXIT_46_WALK_ON,
	ID_SC45_EXIT_47,
	GT_SC45_EXIT_47,
	3,
	SC45_EXIT_47_WALK_ON,
	65535
};

Compact sc45_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	45,	// screen
	0,	// place
	sc45_floor_table,	// getToTable
	128,	// xcood
	264,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	457-128,	// mouseSize_x
	327-264,	// mouseSize_y
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

Compact sc45_exit_47 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	45,	// screen
	0,	// place
	0,	// getToTable
	328,	// xcood
	181,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	378-328,	// mouseSize_x
	261-181,	// mouseSize_y
	SC45_EXIT_47_ACTION,	// actionScript
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

uint16 reset_46_45[] = {
	C_SCREEN,
	45,
	C_PLACE,
	ID_SC45_FLOOR,
	C_XCOOD,
	OFF_RIGHT,
	C_YCOOD,
	280,
	65535
};

Compact sc45_exit_46 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	45,	// screen
	0,	// place
	0,	// getToTable
	420,	// xcood
	161,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-420,	// mouseSize_x
	327-161,	// mouseSize_y
	SC45_EXIT_46_ACTION,	// actionScript
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

uint16 sc45_palette[] = {
	0,
	768,
	771,
	1028,
	2052,
	514,
	773,
	1539,
	1028,
	1284,
	1798,
	771,
	1032,
	2307,
	771,
	1287,
	1539,
	1541,
	1541,
	1285,
	1798,
	1033,
	2562,
	1028,
	1289,
	2563,
	773,
	1543,
	1543,
	1543,
	1290,
	2819,
	773,
	1800,
	1542,
	2055,
	1545,
	2823,
	1285,
	1293,
	1795,
	1800,
	1546,
	2052,
	2055,
	1548,
	1796,
	2312,
	2057,
	3079,
	1542,
	1550,
	3842,
	773,
	1547,
	2563,
	2055,
	1550,
	2308,
	2312,
	2312,
	2824,
	1288,
	2312,
	3338,
	775,
	2059,
	3335,
	1287,
	1550,
	4102,
	1030,
	2059,
	2569,
	2057,
	2058,
	3595,
	1032,
	1804,
	3082,
	1545,
	1807,
	3331,
	1800,
	2316,
	3848,
	1287,
	2064,
	2308,
	2314,
	1553,
	2310,
	3082,
	2316,
	4362,
	775,
	2824,
	4107,
	1544,
	2318,
	2822,
	2314,
	2318,
	4616,
	1032,
	1811,
	3331,
	2314,
	2571,
	2571,
	2571,
	2068,
	3588,
	2569,
	2573,
	5387,
	775,
	2828,
	3850,
	1802,
	3337,
	3596,
	2059,
	2573,
	5646,
	1032,
	3083,
	2827,
	3339,
	3085,
	4107,
	1547,
	2072,
	3588,
	2571,
	3340,
	3340,
	3340,
	2832,
	3081,
	3597,
	2831,
	3596,
	3085,
	3344,
	3595,
	3597,
	3597,
	3341,
	3854,
	3344,
	5390,
	1803,
	2586,
	3844,
	3342,
	3599,
	3599,
	3599,
	3856,
	3598,
	4112,
	3350,
	6664,
	1291,
	3603,
	7180,
	1035,
	3856,
	4112,
	3857,
	4113,
	3857,
	4369,
	4624,
	4626,
	4625,
	3358,
	6663,
	2831,
	4627,
	5651,
	4115,
	4884,
	5140,
	5397,
	5143,
	8977,
	1807,
	4894,
	6668,
	5144,
	63,
	15872,
	0,
	62,
	15616,
	0,
	61,
	15360,
	0,
	60,
	15104,
	0,
	59,
	14848,
	0,
	58,
	14592,
	0,
	57,
	14336,
	0,
	56,
	14080,
	0,
	55,
	13824,
	0,
	54,
	13568,
	0,
	53,
	13312,
	0,
	51,
	13056,
	0,
	50,
	12800,
	0,
	49,
	12544,
	0,
	48,
	12288,
	0,
	47,
	12032,
	0,
	46,
	11776,
	0,
	45,
	11520,
	0,
	44,
	11264,
	0,
	43,
	11008,
	0,
	42,
	10752,
	0,
	41,
	10496,
	0,
	40,
	10240,
	0,
	39,
	9984,
	0,
	38,
	9728,
	0,
	37,
	9472,
	0,
	36,
	9216,
	0,
	35,
	8960,
	0,
	34,
	8704,
	0,
	33,
	8448,
	0,
	32,
	8192,
	0,
	31,
	7936,
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
	1087,
	0,
	4,
	768,
	0,
	3,
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

uint16 sc45_fast_list[] = {
	12+DISK_13,
	51+DISK_13,
	52+DISK_13,
	53+DISK_13,
	54+DISK_13,
	0
};

uint32 *sc45_walk_grid = (uint32*)sc45_fast_list;

uint16 reset_47_45[] = {
	C_SCREEN,
	45,
	C_PLACE,
	ID_SC45_FLOOR,
	C_XCOOD,
	392,
	C_YCOOD,
	248,
	65535
};

Compact sc45_exit_44 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	45,	// screen
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
	SC45_EXIT_44_ACTION,	// actionScript
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

uint16 sc45_chip_list[] = {
	IT_SC45_LAYER_0+DISK_13,
	IT_SC45_LAYER_1+DISK_13,
	IT_SC45_GRID_1+DISK_13,
	0
};

uint16 sc45_mouse_list[] = {
	ID_SC45_EXIT_44,
	ID_SC45_EXIT_46,
	ID_SC45_EXIT_47,
	ID_SC45_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

} // namespace SkyCompact

} // namespace Sky

#endif
