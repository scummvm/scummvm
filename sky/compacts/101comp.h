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

#ifndef SKY101COMP_H
#define SKY101COMP_H




namespace Sky {

namespace SkyCompact {

Compact cancel_button = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	419,	// xcood
	179,	// ycood
	150*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	CANCEL_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	15,	// mouseSize_x
	40,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact button_4 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	368,	// xcood
	199,	// ycood
	144*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact full_screen = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	0,	// frame
	0,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	1,	// mouseSize_x
	1,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FULL_SCREEN_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact button_5 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	385,	// xcood
	199,	// ycood
	145*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s101_logic[] = {
	ID_FOSTER,
	121,
	122,
	123,
	124,
	125,
	126,
	127,
	128,
	129,
	130,
	131,
	132,
	173,
	0XFFFF,
	ID_MENU_LOGIC
};

Compact button_7 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	367,	// xcood
	179,	// ycood
	147*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s101_mouse[] = {
	122,
	123,
	124,
	125,
	126,
	127,
	128,
	129,
	130,
	131,
	132,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact button_1 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	368,	// xcood
	219,	// ycood
	141*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s101_pal[] = {
	0,
	0,
	256,
	0,
	1,
	256,
	0,
	1,
	256,
	0,
	1,
	256,
	0,
	1,
	256,
	0,
	1,
	256,
	0,
	1,
	257,
	256,
	257,
	2,
	260,
	513,
	259,
	772,
	1025,
	259,
	1284,
	513,
	773,
	1282,
	1541,
	773,
	1288,
	1539,
	775,
	1798,
	2565,
	263,
	1294,
	2561,
	263,
	1800,
	1541,
	1801,
	2314,
	2561,
	1799,
	2822,
	2571,
	779,
	2318,
	3075,
	1801,
	2824,
	3081,
	2313,
	2828,
	3589,
	779,
	2832,
	2563,
	2829,
	3342,
	4101,
	2317,
	3852,
	4107,
	1295,
	3854,
	4105,
	1807,
	2843,
	3585,
	3343,
	4364,
	4621,
	1809,
	4372,
	4615,
	1811,
	4377,
	7427,
	271,
	4372,
	5131,
	3345,
	4884,
	3591,
	3861,
	3871,
	4097,
	3861,
	6160,
	4621,
	4373,
	5399,
	6921,
	2323,
	5399,
	6411,
	2325,
	6674,
	7949,
	2835,
	6676,
	6417,
	3864,
	6171,
	7437,
	2840,
	6681,
	6925,
	2842,
	6171,
	6417,
	2844,
	6683,
	8975,
	1304,
	5413,
	5897,
	3870,
	7193,
	10003,
	2325,
	7705,
	7953,
	3356,
	7711,
	6923,
	3360,
	7709,
	10513,
	2840,
	7711,
	6931,
	5664,
	7711,
	10518,
	3354,
	8223,
	6931,
	5666,
	8227,
	8461,
	3362,
	9247,
	8975,
	3362,
	8735,
	10010,
	4384,
	8737,
	11544,
	3870,
	9759,
	8465,
	6692,
	9765,
	9997,
	3366,
	9763,
	10006,
	3878,
	9765,
	11544,
	4898,
	10789,
	10515,
	5672,
	10279,
	11036,
	3882,
	10791,
	11034,
	3372,
	10795,
	11542,
	3886,
	11309,
	12058,
	5678,
	12335,
	12047,
	6190,
	12849,
	13071,
	4402,
	0,
	1,
	256,
	0,
	257,
	258,
	1797,
	2308,
	2059,
	3853,
	4364,
	4115,
	5909,
	6420,
	6172,
	8221,
	8476,
	8228,
	10278,
	10789,
	10540,
	12334,
	12845,
	12597,
	0,
	1281,
	256,
	13,
	5377,
	260,
	3357,
	9473,
	280,
	9773,
	1793,
	256,
	12,
	3585,
	256,
	20,
	6913,
	256,
	33,
	10241,
	256,
	46,
	13569,
	256,
	0,
	1,
	259,
	2816,
	1,
	276,
	7168,
	1,
	293,
	11524,
	2305,
	310,
	0,
	7937,
	256,
	48,
	13569,
	256,
	53,
	13569,
	256,
	47,
	6913,
	256,
	5120,
	2048,
	2056,
	3598,
	4110,
	4624,
	4630,
	4628,
	6164,
	6680,
	4638,
	4118,
	6678,
	6675,
	5662,
	8732,
	7705,
	7205,
	10274,
	9504,
	9259,
	11304,
	9010,
	11559,
	0,
	14336,
	13364,
	11569,
	9261,
	8481,
	6687,
	6937,
	4883,
	3603,
	2830,
	2827,
	2313,
	14345,
	10288,
	10034,
	11550,
	5407,
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
	13828,
	13878,
	12077,
	8753,
	9506,
	9767,
	8240,
	10527,
	5917,
	5925,
	7698,
	3857,
	3862,
	3848,
	14655,
	16168,
	10033,
	10294,
	11552,
	5662,
	5668,
	6926,
	2063,
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
	14655,
	14376,
	9004,
	0,
	16128,
	0,
	9767,
	7728,
	8993,
	5148,
	14354,
	1800,
	4631,
	13854,
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

Compact button_0 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	419,	// xcood
	219,	// ycood
	140*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact button_9 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	402,	// xcood
	179,	// ycood
	149*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact button_8 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	385,	// xcood
	179,	// ycood
	148*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact retina_scan = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	419,	// xcood
	239,	// ycood
	151*64,	// frame
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
	FS_RETINA_SCAN_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact button_6 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	402,	// xcood
	199,	// ycood
	146*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact button_3 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	402,	// xcood
	219,	// ycood
	143*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact button_2 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	101,	// screen
	0,	// place
	0,	// getToTable
	385,	// xcood
	219,	// ycood
	142*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	BUTTON_ACTION_101,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FS_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s101_chip_list[] = {
	101+DISK_1,
	140+DISK_1,
	141+DISK_1,
	142+DISK_1,
	143+DISK_1,
	144+DISK_1,
	145+DISK_1,
	146+DISK_1,
	147+DISK_1,
	148+DISK_1,
	149+DISK_1,
	150+DISK_1,
	151+DISK_1,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
