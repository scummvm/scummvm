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

#ifndef SKY47COMP_H
#define SKY47COMP_H




namespace Sky {

namespace SkyCompact {

uint16 reset_48_47[] = {
	C_SCREEN,
	47,
	C_PLACE,
	ID_SC47_FLOOR,
	C_XCOOD,
	OFF_RIGHT,
	C_YCOOD,
	280,
	65535
};

uint16 sc47_palette[] = {
	0,
	768,
	771,
	1028,
	1284,
	515,
	1030,
	1540,
	259,
	1284,
	1798,
	516,
	1032,
	1540,
	1541,
	1289,
	1283,
	1798,
	1289,
	2563,
	1028,
	1288,
	2566,
	516,
	1291,
	1539,
	1543,
	1543,
	2055,
	1287,
	1798,
	2824,
	1285,
	1545,
	3079,
	260,
	1293,
	3331,
	261,
	2055,
	3335,
	1285,
	1800,
	1800,
	2312,
	2057,
	3078,
	262,
	1548,
	3846,
	773,
	1802,
	2312,
	2312,
	2312,
	3592,
	773,
	1550,
	3076,
	1032,
	2312,
	3338,
	775,
	2060,
	3334,
	1287,
	1552,
	3588,
	1542,
	2059,
	2569,
	1801,
	2062,
	3844,
	775,
	2061,
	2312,
	2314,
	1807,
	2565,
	2569,
	1554,
	3332,
	1289,
	2064,
	3588,
	1544,
	2316,
	4362,
	775,
	2064,
	2822,
	2058,
	2066,
	3332,
	1802,
	1811,
	3587,
	2313,
	2068,
	2564,
	2571,
	2319,
	2823,
	2826,
	2573,
	3083,
	2315,
	2830,
	3080,
	3083,
	2070,
	2820,
	2828,
	2576,
	3336,
	2572,
	3087,
	3847,
	2571,
	3340,
	3340,
	3340,
	2830,
	3596,
	2829,
	2327,
	3846,
	3340,
	2329,
	3589,
	3597,
	3597,
	3853,
	3086,
	3093,
	5642,
	1804,
	2586,
	3844,
	3854,
	3854,
	7182,
	1034,
	3603,
	4362,
	3854,
	3856,
	3854,
	3344,
	2843,
	3845,
	3856,
	3856,
	4368,
	3856,
	4118,
	7693,
	1549,
	3612,
	8458,
	1293,
	3615,
	4869,
	4370,
	4631,
	4621,
	4627,
	4627,
	5139,
	4627,
	4884,
	7444,
	3603,
	4387,
	5642,
	5654,
	5151,
	5901,
	5398,
	5910,
	8982,
	4376,
	5672,
	8205,
	5659,
	6444,
	9999,
	4891,
	7472,
	12816,
	4640,
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

uint16 reset_45_47[] = {
	C_SCREEN,
	47,
	C_PLACE,
	ID_SC47_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	280,
	65535
};

uint16 sc47_mouse_list[] = {
	ID_SC47_EXIT_45,
	ID_SC47_EXIT_48,
	ID_SC47_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc47_exit_48 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	47,	// screen
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
	SC47_EXIT_48_ACTION,	// actionScript
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

uint32 *sc47_walk_grid = (uint32*)sc47_fast_list;

uint16 sc47_fast_list[] = {
	12+DISK_13,
	51+DISK_13,
	52+DISK_13,
	53+DISK_13,
	54+DISK_13,
	0
};

uint16 sc47_logic_list[] = {
	ID_FOSTER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

Compact sc47_exit_45 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	47,	// screen
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
	SC47_EXIT_45_ACTION,	// actionScript
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

uint16 sc47_chip_list[] = {
	IT_SC47_LAYER_0+DISK_13,
	0
};

uint16 sc47_floor_table[] = {
	ID_SC47_FLOOR,
	RET_OK,
	ID_SC47_EXIT_45,
	GT_SC47_EXIT_45,
	0,
	SC47_EXIT_45_WALK_ON,
	ID_SC47_EXIT_48,
	GT_SC47_EXIT_48,
	1,
	SC47_EXIT_48_WALK_ON,
	65535
};

Compact sc47_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	47,	// screen
	0,	// place
	sc47_floor_table,	// getToTable
	128,	// xcood
	272,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-128,	// mouseSize_x
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

} // namespace SkyCompact

} // namespace Sky

#endif
