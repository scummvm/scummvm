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

#ifndef SKY44COMP_H
#define SKY44COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc44_chip_list[] = {
	IT_SC44_LAYER_0+DISK_13,
	145+DISK_13,
	0
};

uint32 *sc44_walk_grid = (uint32*)sc44_fast_list;

Compact sc44_rubble = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	44,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	223,	// ycood
	0,	// frame
	17383,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	250-128,	// mouseSize_x
	310-223,	// mouseSize_y
	SC44_RUBBLE_ACTION,	// actionScript
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

uint16 reset_45_44[] = {
	C_SCREEN,
	44,
	C_PLACE,
	ID_SC44_FLOOR,
	C_XCOOD,
	OFF_RIGHT,
	C_YCOOD,
	280,
	65535
};

Compact sc44_grill = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	44,	// screen
	0,	// place
	0,	// getToTable
	319,	// xcood
	179,	// ycood
	0,	// frame
	17382,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	341-319,	// mouseSize_x
	198-179,	// mouseSize_y
	SC44_GRILL_ACTION,	// actionScript
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

uint16 sc44_floor_table[] = {
	ID_SC44_FLOOR,
	RET_OK,
	ID_SC44_RUBBLE,
	GT_SC44_RUBBLE,
	ID_SC44_GRILL,
	GT_SC44_GRILL,
	ID_SC44_EXIT_45,
	GT_SC44_EXIT_45,
	0,
	SC44_EXIT_45_WALK_ON,
	65535
};

uint16 sc44_drop_down[] = {
	145*64,
	316,
	186,
	0,
	317,
	187,
	1,
	317,
	189,
	2,
	317,
	188,
	3,
	316,
	196,
	4,
	317,
	197,
	5,
	317,
	197,
	6,
	315,
	228,
	7,
	313,
	229,
	8,
	316,
	226,
	9,
	0
};

Compact sc44_exit_45 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	44,	// screen
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
	SC44_EXIT_45_ACTION,	// actionScript
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

uint16 sc44_fast_list[] = {
	12+DISK_13,
	51+DISK_13,
	52+DISK_13,
	53+DISK_13,
	54+DISK_13,
	0
};

uint16 sc44_palette[] = {
	0,
	768,
	771,
	772,
	1283,
	515,
	1028,
	1540,
	1028,
	1285,
	1283,
	1285,
	1032,
	2052,
	1540,
	1287,
	1539,
	1030,
	1287,
	2309,
	773,
	1541,
	1799,
	1797,
	1289,
	2821,
	1028,
	1544,
	2052,
	1542,
	1544,
	3080,
	773,
	1798,
	1542,
	2055,
	1292,
	2565,
	1030,
	1801,
	2565,
	1542,
	1294,
	1795,
	1800,
	1801,
	1799,
	2312,
	1549,
	2564,
	1544,
	1804,
	2307,
	2312,
	1549,
	2822,
	1287,
	2312,
	2824,
	1799,
	2061,
	3076,
	1544,
	1806,
	3843,
	1030,
	2059,
	2569,
	2057,
	1553,
	3588,
	1287,
	1808,
	2307,
	2825,
	2568,
	3082,
	2057,
	2063,
	4100,
	1287,
	2063,
	4358,
	1032,
	2318,
	2565,
	2570,
	1810,
	4355,
	1544,
	1810,
	4101,
	1289,
	2318,
	2824,
	2059,
	2573,
	3079,
	2570,
	1812,
	4099,
	1801,
	3082,
	4874,
	1032,
	2826,
	4620,
	1289,
	2575,
	3590,
	2570,
	2829,
	5385,
	1032,
	3084,
	4362,
	1546,
	2828,
	3852,
	2059,
	3086,
	5898,
	1032,
	3085,
	4109,
	2316,
	3340,
	5644,
	1289,
	2833,
	3848,
	2827,
	3087,
	3597,
	3085,
	3597,
	4877,
	2060,
	3342,
	4110,
	2829,
	2585,
	4868,
	1549,
	3597,
	4623,
	2573,
	3345,
	3853,
	3342,
	2587,
	3332,
	4112,
	3854,
	3854,
	3854,
	2589,
	4100,
	3599,
	3602,
	4620,
	3598,
	3856,
	3856,
	3856,
	2844,
	7429,
	1036,
	4114,
	4365,
	3856,
	4113,
	4113,
	4113,
	3861,
	4619,
	4625,
	4625,
	4881,
	4370,
	4124,
	8458,
	1550,
	4884,
	5906,
	4114,
	4886,
	5649,
	5141,
	4896,
	5900,
	5398,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
	8481,
	8487,
	10017,
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
	8511,
	10017,
	8481,
	8487,
	10017,
	8481,
	13863,
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

uint16 sc44_logic_list[] = {
	ID_FOSTER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

Compact sc44_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	44,	// screen
	0,	// place
	sc44_floor_table,	// getToTable
	208,	// xcood
	272,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-208,	// mouseSize_x
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

uint16 sc44_mouse_list[] = {
	ID_SC44_GRILL,
	ID_SC44_RUBBLE,
	ID_SC44_EXIT_45,
	ID_SC44_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 reset_37_44[] = {
	C_SCREEN,
	44,
	C_PLACE,
	ID_SC44_FLOOR,
	C_XCOOD,
	316,
	C_YCOOD,
	186,
	C_FRAME,
	145*64,
	65535
};

} // namespace SkyCompact

} // namespace Sky

#endif
