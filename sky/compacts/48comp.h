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

#ifndef SKY48COMP_H
#define SKY48COMP_H




namespace Sky {

namespace SkyCompact {

Compact sc48_eyes = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	48,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	0,	// frame
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
	SC48_EYES_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc48_mouse_list[] = {
	ID_SC48_SOCKET,
	ID_SC48_HOLE,
	ID_SC48_EXIT_47,
	ID_SC48_EXIT_65,
	ID_SC48_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc48_chip_list[] = {
	IT_SC48_LAYER_0+DISK_13,
	71+DISK_13,
	72+DISK_13,
	74+DISK_13,
	75+DISK_13,
	0
};

uint16 sc48_floor_table[] = {
	ID_SC48_FLOOR,
	RET_OK,
	ID_SC48_EXIT_47,
	GT_SC48_EXIT_47,
	0,
	SC48_EXIT_47_WALK_ON,
	ID_SC48_EXIT_65,
	GT_SC48_EXIT_65,
	1,
	SC48_EXIT_65_WALK_ON,
	ID_SC48_SOCKET,
	GT_SC48_SOCKET,
	ID_SC48_HOLE,
	GT_SC48_HOLE,
	65535
};

Compact sc48_socket = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	48,	// screen
	0,	// place
	0,	// getToTable
	312,	// xcood
	246,	// ycood
	71*64,	// frame
	16552,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	1,	// mouseRel_y
	319-317,	// mouseSize_x
	248-247,	// mouseSize_y
	SC48_SOCKET_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC48_SOCKET_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc48_put_bulb[] = {
	72*64,
	304,
	224,
	0,
	304,
	224,
	1,
	304,
	224,
	2,
	304,
	224,
	3,
	304,
	224,
	3,
	304,
	224,
	3,
	304,
	224,
	3,
	304,
	224,
	3,
	304,
	224,
	2,
	304,
	224,
	1,
	304,
	224,
	0,
	0
};

uint16 sc48_palette[] = {
	0,
	512,
	1026,
	1026,
	1028,
	1028,
	1030,
	1028,
	1030,
	1032,
	1538,
	1030,
	1542,
	2054,
	1030,
	2052,
	1030,
	2056,
	1544,
	2566,
	1030,
	2054,
	2566,
	1542,
	1548,
	2564,
	2054,
	2054,
	2570,
	1032,
	2056,
	2568,
	1544,
	1550,
	1540,
	2570,
	2058,
	4104,
	1030,
	2062,
	2050,
	2570,
	2570,
	2568,
	2570,
	2064,
	4100,
	1544,
	2062,
	2058,
	2572,
	2574,
	3078,
	2570,
	2066,
	3588,
	2058,
	2572,
	2572,
	2572,
	2068,
	3588,
	2570,
	2574,
	3084,
	3084,
	2070,
	3076,
	3596,
	2580,
	3588,
	2572,
	2072,
	3588,
	3596,
	3596,
	3084,
	3598,
	3598,
	6156,
	1034,
	3598,
	3086,
	3600,
	2586,
	4612,
	3596,
	3600,
	6670,
	1546,
	3602,
	4618,
	3086,
	3092,
	3598,
	3600,
	3602,
	3598,
	4112,
	3098,
	4614,
	3088,
	4112,
	5136,
	4110,
	3100,
	4614,
	3600,
	4114,
	4114,
	4114,
	4624,
	4626,
	4114,
	4626,
	5138,
	3602,
	4628,
	4112,
	4628,
	3617,
	5126,
	4626,
	5138,
	5138,
	5138,
	5140,
	6672,
	4624,
	5140,
	4628,
	4630,
	5650,
	6164,
	4116,
	5652,
	5652,
	5652,
	5652,
	5654,
	5142,
	5654,
	5654,
	5656,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	8244,
	11296,
	2056,
	1052,
	4868,
	514,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	8747,
	10276,
	8479,
	7461,
	8991,
	7194,
	6176,
	7706,
	6166,
	4891,
	6421,
	4881,
	3862,
	4881,
	3853,
	2833,
	3597,
	2825,
	1804,
	2313,
	1797,
	1031,
	16133,
	63,
	0,
	14336,
	13364,
	11569,
	8493,
	7710,
	5916,
	5654,
	3855,
	2063,
	1289,
	770,
	517,
	13571,
	9517,
	9778,
	11805,
	5151,
	5926,
	8205,
	1808,
	2586,
	10243,
	0,
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
	63,
	16191,
	16128,
	63,
	16191,
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

uint16 sc48_fast_list[] = {
	12+DISK_13,
	51+DISK_13,
	52+DISK_13,
	53+DISK_13,
	54+DISK_13,
	0
};

Compact sc48_hole = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC,	// status
	0,	// sync
	48,	// screen
	0,	// place
	0,	// getToTable
	362,	// xcood
	203,	// ycood
	0,	// frame
	17438,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	383-362,	// mouseSize_x
	253-203,	// mouseSize_y
	SC48_HOLE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC48_HOLE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 reset_47_48[] = {
	C_SCREEN,
	48,
	C_PLACE,
	ID_SC48_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	280,
	65535
};

uint16 sc48_logic_list[] = {
	ID_FOSTER,
	ID_SC48_SOCKET,
	ID_SC48_HOLE,
	ID_SC48_EYES,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 reset_65_48[] = {
	C_SCREEN,
	48,
	C_PLACE,
	ID_SC48_FLOOR,
	C_XCOOD,
	OFF_RIGHT,
	C_YCOOD,
	280,
	65535
};

uint16 sc48_eyes_anim[] = {
	74*64,
	358,
	201,
	0,
	358,
	201,
	1,
	358,
	201,
	2,
	358,
	201,
	3,
	358,
	201,
	3,
	358,
	201,
	4,
	358,
	201,
	5,
	358,
	201,
	6,
	358,
	201,
	5,
	358,
	201,
	4,
	358,
	201,
	7,
	358,
	201,
	8,
	358,
	201,
	9,
	358,
	201,
	10,
	358,
	201,
	11,
	0
};

Compact sc48_exit_47 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	48,	// screen
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
	SC48_EXIT_47_ACTION,	// actionScript
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

Compact sc48_exit_65 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	48,	// screen
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
	448-420,	// mouseSize_x
	327-161,	// mouseSize_y
	SC48_EXIT_65_ACTION,	// actionScript
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

uint32 *sc48_walk_grid = (uint32*)sc48_fast_list;

Compact sc48_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	48,	// screen
	0,	// place
	sc48_floor_table,	// getToTable
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

uint16 sc48_light_pal[] = {
	0,
	1280,
	1797,
	1797,
	1799,
	1799,
	1801,
	1799,
	1801,
	1803,
	2309,
	1801,
	2313,
	2825,
	1801,
	2823,
	1801,
	2827,
	2315,
	3337,
	1801,
	2825,
	3337,
	2313,
	2319,
	3335,
	2825,
	2825,
	3341,
	1803,
	2827,
	3339,
	2315,
	2321,
	2311,
	3341,
	2829,
	4875,
	1801,
	2833,
	2821,
	3341,
	3341,
	3339,
	3341,
	2835,
	4871,
	2315,
	2833,
	2829,
	3343,
	3345,
	3849,
	3341,
	2837,
	4359,
	2829,
	3343,
	3343,
	3343,
	2839,
	4359,
	3341,
	3345,
	3855,
	3855,
	2841,
	3847,
	4367,
	3351,
	4359,
	3343,
	2843,
	4359,
	4367,
	4367,
	3855,
	4369,
	4369,
	6927,
	1805,
	4369,
	3857,
	4371,
	3357,
	5383,
	4367,
	4371,
	7441,
	2317,
	4373,
	5389,
	3857,
	3863,
	4369,
	4371,
	4373,
	4369,
	4883,
	3869,
	5385,
	3859,
	4883,
	5907,
	4881,
	3871,
	5385,
	4371,
	4885,
	4885,
	4885,
	5395,
	5397,
	4885,
	5397,
	5909,
	4373,
	5399,
	4883,
	5399,
	4388,
	5897,
	5397,
	5909,
	5909,
	5909,
	5911,
	7443,
	5395,
	5911,
	5399,
	5401,
	6421,
	6935,
	4887,
	6423,
	6423,
	6423,
	6423,
	6425,
	5913,
	6425,
	6425,
	6427,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	9015,
	12067,
	2827,
	1823,
	5639,
	1285,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	9518,
	11047,
	9250,
	8232,
	9762,
	7965,
	6947,
	8477,
	6937,
	5662,
	7192,
	5652,
	4633,
	5652,
	4624,
	3604,
	4368,
	3596,
	2575,
	3084,
	2568,
	1802,
	16136,
	831,
	771,
	15107,
	14135,
	12340,
	9264,
	8481,
	6687,
	6425,
	4626,
	2834,
	1548,
	1027,
	774,
	14340,
	10288,
	10549,
	12576,
	5922,
	6697,
	8976,
	2579,
	3357,
	11014,
	771,
	8981,
	8212,
	7725,
	10265,
	4890,
	4637,
	5635,
	14087,
	14135,
	12334,
	9522,
	10277,
	10795,
	9013,
	11298,
	6945,
	6698,
	8469,
	4371,
	3096,
	3079,
	12344,
	13608,
	8233,
	8753,
	10519,
	4122,
	4899,
	7434,
	1549,
	10303,
	13871,
	10015,
	5933,
	9247,
	6161,
	2843,
	4626,
	3079,
	3636,
	10766,
	2056,
	1056,
	5636,
	771,
	12344,
	13608,
	8233,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	16191,
	16131,
	831,
	0,
	16128,
	16191
};

} // namespace SkyCompact

} // namespace Sky

#endif
