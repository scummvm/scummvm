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

#ifndef SKY33COMP_H
#define SKY33COMP_H




namespace Sky {

namespace SkyCompact {

Compact sc33_lock = {
	0,	// logic
	0,	// status
	0,	// sync
	33,	// screen
	0,	// place
	0,	// getToTable
	357,	// xcood
	244,	// ycood
	0,	// frame
	16640,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	360-357,	// mouseSize_x
	251-244,	// mouseSize_y
	SC33_LOCK_ACTION,	// actionScript
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

uint16 reset_32_33[] = {
	C_SCREEN,
	33,
	C_PLACE,
	ID_SC33_FLOOR,
	C_XCOOD,
	352,
	C_YCOOD,
	240,
	65535
};

uint16 sc33_palette[] = {
	0,
	1280,
	2311,
	1799,
	2053,
	2058,
	2824,
	3084,
	2827,
	3594,
	2056,
	5133,
	3342,
	3080,
	3852,
	3088,
	3082,
	2574,
	4108,
	2568,
	3855,
	4620,
	2570,
	5648,
	3854,
	4881,
	3086,
	4108,
	4376,
	4367,
	4106,
	4383,
	3601,
	5135,
	3084,
	5139,
	4106,
	5411,
	3601,
	4876,
	3612,
	6674,
	4124,
	3851,
	5653,
	4888,
	5132,
	4883,
	4883,
	5654,
	4372,
	4875,
	3622,
	7702,
	4890,
	4880,
	3864,
	5143,
	3605,
	9237,
	5390,
	7975,
	3347,
	5654,
	4374,
	7192,
	6675,
	6417,
	4631,
	5916,
	3855,
	10008,
	6161,
	3881,
	11032,
	5666,
	4879,
	7962,
	7192,
	4629,
	9242,
	6684,
	6418,
	6682,
	7956,
	4374,
	10523,
	6940,
	4630,
	11546,
	6432,
	8726,
	4378,
	6182,
	6673,
	6174,
	7195,
	5149,
	11037,
	7444,
	8495,
	6684,
	6949,
	6164,
	9503,
	6953,
	5650,
	11552,
	7715,
	7703,
	7455,
	7463,
	5655,
	12576,
	7467,
	11797,
	4637,
	8735,
	10528,
	6432,
	9239,
	10287,
	7457,
	8997,
	11546,
	5920,
	9243,
	6190,
	13093,
	8240,
	11284,
	6946,
	9508,
	10533,
	7205,
	10267,
	12082,
	6436,
	8754,
	9751,
	8743,
	9517,
	10782,
	8488,
	9267,
	7707,
	13609,
	10032,
	11548,
	9255,
	10793,
	13861,
	6438,
	10547,
	8990,
	13868,
	10295,
	12829,
	8746,
	11055,
	11048,
	10798,
	11062,
	13600,
	9260,
	11827,
	12585,
	11057,
	12088,
	13604,
	10033,
	13364,
	14380,
	10547,
	13109,
	12592,
	13365,
	13624,
	15149,
	10806,
	14131,
	14903,
	12088,
	14647,
	15412,
	12603,
	15417,
	15417,
	15420,
	15934,
	15923,
	15935,
	768,
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
	63,
	0,
	14336,
	13364,
	11569,
	10029,
	9252,
	7202,
	6939,
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
	11564,
	8239,
	10527,
	6174,
	5927,
	7698,
	3600,
	2325,
	2308,
	11831,
	13093,
	7206,
	7982,
	10261,
	3351,
	4130,
	7431,
	778,
	9277,
	14124,
	9757,
	5937,
	11040,
	6929,
	3109,
	6934,
	4103,
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
	63,
	63,
	16128,
	0,
	63,
	16128,
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

uint16 sc33_door_open[] = {
	97*64,
	1,
	1,
	2,
	1,
	1,
	3,
	0
};

uint32 *sc33_walk_grid = (uint32*)sc33_joey_list;

uint16 reset_34_33[] = {
	C_SCREEN,
	33,
	C_PLACE,
	ID_SC33_FLOOR,
	C_XCOOD,
	384,
	C_YCOOD,
	272,
	65535
};

Compact sc33_shed_door = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	33,	// screen
	0,	// place
	0,	// getToTable
	358,	// xcood
	222,	// ycood
	97*64,	// frame
	181,	// cursorText
	SC33_SHED_DOOR_MOUSE_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65521,	// mouseRel_x
	(int16) 65526,	// mouseRel_y
	366-343,	// mouseSize_x
	275-212,	// mouseSize_y
	SC33_SHED_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC33_SHED_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc33_push_door2[] = {
	96*64,
	328,
	224,
	0,
	328,
	224,
	1,
	328,
	224,
	2,
	328,
	224,
	3,
	328,
	224,
	4,
	328,
	224,
	5,
	328,
	224,
	5,
	328,
	224,
	5,
	328,
	224,
	6,
	328,
	224,
	0,
	0
};

Compact sc33_exit_32 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	33,	// screen
	0,	// place
	0,	// getToTable
	310,	// xcood
	170,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	330-310,	// mouseSize_x
	240-170,	// mouseSize_y
	SC33_EXIT_32_ACTION,	// actionScript
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

uint16 sc33_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_DOG_BARK_THING,
	ID_SC33_SHED_DOOR,
	ID_SC32_LIFT,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc33_floor_table[] = {
	ID_SC33_FLOOR,
	RET_OK,
	ID_SC33_EXIT_30,
	GT_SC33_EXIT_30,
	0,
	SC33_EXIT_30_WALK_ON,
	ID_SC33_EXIT_32,
	GT_SC33_EXIT_32,
	1,
	SC33_EXIT_32_WALK_ON,
	ID_SC33_SHED_DOOR,
	GT_SC33_SHED_DOOR,
	3,
	SC33_SHED_DOOR_WALK_ON,
	ID_SC33_LOCK,
	GT_SC33_LOCK,
	ID_STD_LEFT_TALK,
	GT_SC33_LEFT_TALK,
	ID_STD_RIGHT_TALK,
	GT_SC33_RIGHT_TALK,
	ID_JOEY_PARK,
	GT_DANI_WAIT,
	65535
};

Compact sc33_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	33,	// screen
	0,	// place
	sc33_floor_table,	// getToTable
	128,	// xcood
	224,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	358-128,	// mouseSize_x
	319-224,	// mouseSize_y
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

Compact sc33_exit_30 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	33,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	162,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	140-128,	// mouseSize_x
	327-162,	// mouseSize_y
	SC33_EXIT_30_ACTION,	// actionScript
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

uint16 sc33_joey_list[] = {
	144,
	319,
	232,
	319,
	1,
	0
};

uint16 sc33_push_door1[] = {
	96*64,
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
	4,
	1,
	1,
	4,
	1,
	1,
	4,
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

uint16 sc33_chip_list[] = {
	IT_SC33_LAYER_0+DISK_12,
	IT_SC33_LAYER_1+DISK_12,
	IT_SC33_LAYER_2+DISK_12,
	IT_DANIELLE+DISK_12,
	IT_DANI_CONV+DISK_12,
	IT_SPUNKY+DISK_12,
	IT_SNIFF_LEFT+DISK_12,
	IT_SC33_USE_CARD+DISK_12,
	IT_SC33_PUSH_DOOR+DISK_12,
	IT_SC33_SHED_DOOR+DISK_12,
	0
};

uint16 sc33_fast_list[] = {
	12+DISK_12,
	51+DISK_12,
	52+DISK_12,
	53+DISK_12,
	54+DISK_12,
	135+DISK_12,
	IT_SC33_GRID_1+DISK_12,
	IT_SC33_GRID_2+DISK_12,
	0
};

uint16 sc33_mouse_list[] = {
	ID_SC33_LOCK,
	ID_SC33_SHED_DOOR,
	ID_SPUNKY,
	ID_DANIELLE,
	ID_SC33_EXIT_30,
	ID_SC33_EXIT_32,
	ID_SC33_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc33_use_card[] = {
	95*64,
	328,
	224,
	0,
	328,
	224,
	1,
	328,
	224,
	2,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	2,
	328,
	224,
	4,
	328,
	224,
	0,
	0
};

uint16 reset_30_33[] = {
	C_SCREEN,
	33,
	C_PLACE,
	ID_SC33_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	248,
	65535
};

} // namespace SkyCompact

} // namespace Sky

#endif
