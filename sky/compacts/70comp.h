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

#ifndef SKY70COMP_H
#define SKY70COMP_H




namespace Sky {

namespace SkyCompact {

Compact sc70_bar = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	70,	// screen
	0,	// place
	0,	// getToTable
	290,	// xcood
	170,	// ycood
	96*64,	// frame
	5,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	12,	// mouseSize_y
	SC70_BAR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC70_BAR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc70_pull_bar[] = {
	18*64,
	288,
	180,
	0,
	288,
	180,
	1,
	288,
	180,
	2,
	288,
	180,
	3,
	288,
	180,
	4,
	288,
	180,
	5,
	288,
	180,
	6,
	288,
	180,
	7,
	288,
	180,
	8,
	288,
	180,
	9,
	288,
	180,
	10,
	288,
	180,
	2,
	288,
	180,
	11,
	288,
	180,
	12,
	0
};

uint16 sc70_step_down[] = {
	15*64,
	322,
	194,
	0,
	323,
	194,
	1,
	323,
	195,
	2,
	323,
	195,
	3,
	329,
	195,
	4,
	337,
	194,
	5,
	343,
	196,
	6,
	343,
	197,
	7,
	343,
	201,
	8,
	347,
	202,
	9,
	349,
	203,
	10,
	349,
	204,
	11,
	349,
	207,
	12,
	352,
	208,
	13,
	355,
	209,
	14,
	355,
	210,
	15,
	355,
	214,
	16,
	359,
	215,
	9,
	361,
	216,
	10,
	361,
	217,
	11,
	361,
	220,
	12,
	364,
	221,
	13,
	367,
	222,
	14,
	367,
	223,
	17,
	367,
	226,
	18,
	371,
	227,
	19,
	373,
	227,
	20,
	373,
	226,
	21,
	374,
	226,
	22,
	378,
	226,
	0,
	0
};

uint16 sc70_palette[] = {
	0,
	1024,
	1285,
	1541,
	2054,
	1285,
	1542,
	1800,
	1542,
	1290,
	2309,
	1542,
	1544,
	2056,
	1287,
	1799,
	1543,
	1800,
	1799,
	3081,
	1285,
	1547,
	2052,
	1800,
	1802,
	2821,
	1542,
	1294,
	2565,
	1799,
	1801,
	2057,
	2568,
	1549,
	3078,
	1287,
	2314,
	2310,
	2057,
	1804,
	4103,
	1285,
	1551,
	2822,
	2056,
	1806,
	2565,
	2568,
	2313,
	3595,
	1799,
	2316,
	3333,
	1544,
	2316,
	2567,
	2314,
	2824,
	4362,
	1542,
	2061,
	4104,
	1287,
	2063,
	2822,
	2825,
	2316,
	3849,
	2056,
	2315,
	2573,
	3082,
	1810,
	3589,
	1801,
	2065,
	3334,
	2058,
	2317,
	2827,
	2571,
	1810,
	3591,
	2313,
	2063,
	4362,
	2056,
	2572,
	4108,
	1801,
	2320,
	2825,
	3339,
	2830,
	3846,
	2058,
	2572,
	3342,
	2571,
	2574,
	3084,
	2828,
	2577,
	3846,
	2570,
	2320,
	4619,
	1801,
	2322,
	3337,
	3339,
	2577,
	3080,
	3596,
	2577,
	3338,
	3851,
	3086,
	4105,
	2315,
	3086,
	4107,
	1804,
	2832,
	3851,
	3339,
	2579,
	5128,
	2313,
	3341,
	4876,
	2570,
	2834,
	3593,
	3596,
	3090,
	3591,
	4108,
	2834,
	4363,
	2572,
	3341,
	5391,
	2058,
	3343,
	7178,
	1287,
	3343,
	5132,
	2315,
	3089,
	5388,
	2570,
	3598,
	3853,
	3853,
	3601,
	4105,
	2830,
	3346,
	3851,
	4365,
	3598,
	8720,
	773,
	3600,
	4365,
	3597,
	3855,
	4110,
	4110,
	3857,
	4620,
	3342,
	3858,
	4362,
	3599,
	3604,
	6669,
	2060,
	3097,
	5642,
	3342,
	2846,
	5384,
	3599,
	3354,
	5643,
	3858,
	4888,
	8463,
	2319,
	0,
	2048,
	1799,
	3097,
	2058,
	1285,
	1542,
	1800,
	1542,
	1290,
	2309,
	2825,
	2570,
	2825,
	2825,
	2570,
	2828,
	2571,
	3084,
	3342,
	3853,
	3598,
	3597,
	4110,
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

uint16 sc70_floor_table[] = {
	ID_SC70_FLOOR,
	RET_OK,
	ID_SC70_DOOR,
	GT_SC70_DOOR,
	ID_SC70_CONTROL,
	GT_SC70_CONTROL,
	ID_SC70_PIT,
	GT_SC70_PIT,
	ID_SC70_IRIS,
	GT_SC70_PIT,
	ID_SC70_BAR,
	GT_SC70_PIT,
	ID_SC70_GRILL,
	GT_SC70_PIT,
	65535
};

uint16 sc70_enter_anim[] = {
	97*64,
	165,
	172,
	0,
	165,
	175,
	1,
	168,
	176,
	2,
	171,
	177,
	3,
	171,
	178,
	4,
	171,
	182,
	5,
	175,
	183,
	6,
	177,
	184,
	7,
	177,
	185,
	0,
	177,
	188,
	1,
	180,
	189,
	2,
	183,
	190,
	3,
	183,
	191,
	8,
	183,
	194,
	9,
	187,
	195,
	10,
	189,
	195,
	11,
	189,
	194,
	12,
	190,
	194,
	13,
	194,
	194,
	14,
	0
};

uint16 sc70_step_up[] = {
	14*64,
	372,
	226,
	0,
	369,
	226,
	1,
	366,
	226,
	2,
	364,
	226,
	3,
	364,
	224,
	4,
	363,
	222,
	5,
	360,
	222,
	6,
	358,
	222,
	7,
	356,
	222,
	8,
	356,
	219,
	9,
	355,
	217,
	10,
	352,
	216,
	11,
	350,
	216,
	12,
	350,
	215,
	13,
	350,
	212,
	14,
	350,
	210,
	15,
	347,
	209,
	16,
	345,
	209,
	17,
	345,
	207,
	18,
	345,
	204,
	19,
	344,
	201,
	20,
	341,
	200,
	21,
	339,
	200,
	22,
	337,
	200,
	23,
	336,
	196,
	24,
	336,
	194,
	25,
	333,
	194,
	26,
	325,
	195,
	27,
	325,
	194,
	28,
	325,
	194,
	29,
	324,
	194,
	0,
	0
};

Compact sc70_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	70,	// screen
	0,	// place
	sc70_floor_table,	// getToTable
	140,	// xcood
	230,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-140,	// mouseSize_x
	327-230,	// mouseSize_y
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

uint16 reset_68_70[] = {
	C_SCREEN,
	70,
	C_PLACE,
	ID_SC70_FLOOR,
	C_XCOOD,
	200,
	C_YCOOD,
	240,
	C_DIR,
	RIGHT,
	65535
};

uint16 sc70_exit_anim[] = {
	14*64,
	188,
	194,
	0,
	185,
	194,
	1,
	182,
	194,
	2,
	180,
	194,
	3,
	180,
	192,
	4,
	179,
	190,
	5,
	176,
	190,
	6,
	174,
	190,
	7,
	172,
	190,
	8,
	172,
	187,
	9,
	171,
	185,
	10,
	168,
	184,
	11,
	166,
	184,
	12,
	166,
	183,
	13,
	166,
	180,
	14,
	166,
	178,
	15,
	163,
	177,
	16,
	161,
	177,
	17,
	161,
	175,
	18,
	0
};

uint32 *grid70 = 0;

uint16 sc70_fast_list[] = {
	12+DISK_5,
	51+DISK_5,
	52+DISK_5,
	53+DISK_5,
	54+DISK_5,
	268+DISK_5,
	0
};

uint16 sc70_logic_list[] = {
	ID_MEDI,
	ID_SC67_DOOR,
	ID_SC68_DOOR,
	ID_SC70_PIT,
	ID_SC70_IRIS,
	ID_SC70_BAR,
	ID_SC70_CONTROL,
	ID_SC70_GRILL,
	ID_FOSTER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

ExtCompact sc70_iris_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	0,	// dir
	0,	// stopScript
	0,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

Compact sc70_iris = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	70,	// screen
	0,	// place
	0,	// getToTable
	231,	// xcood
	229,	// ycood
	3+95*64,	// frame
	20534,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	1,	// mouseRel_x
	1,	// mouseRel_y
	98,	// mouseSize_x
	18,	// mouseSize_y
	SC70_IRIS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC70_IRIS_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc70_iris_ext
};

uint16 sc70_pit_table[] = {
	ID_SC70_FLOOR,
	GT_SC70_FLOOR,
	ID_SC70_DOOR,
	GT_SC70_FLOOR,
	ID_SC70_CONTROL,
	GT_SC70_FLOOR,
	ID_SC70_PIT,
	GT_SC70_PIT,
	ID_SC70_IRIS,
	GT_SC70_IRIS,
	ID_SC70_BAR,
	GT_SC70_BAR,
	ID_SC70_GRILL,
	GT_SC70_GRILL,
	65535
};

Compact sc70_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	70,	// screen
	0,	// place
	0,	// getToTable
	178,	// xcood
	184,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	191-178,	// mouseSize_x
	242-184,	// mouseSize_y
	SC70_DOOR_ACTION,	// actionScript
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

Compact sc70_pit = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	70,	// screen
	0,	// place
	sc70_pit_table,	// getToTable
	237,	// xcood
	232,	// ycood
	117*64,	// frame
	20682,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	28,	// mouseRel_x
	1,	// mouseRel_y
	34,	// mouseSize_x
	7,	// mouseSize_y
	SC70_PIT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC70_PIT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc70_chip_list[] = {
	IT_SC70_LAYER_0+DISK_5,
	IT_SC70_LAYER_1+DISK_5,
	IT_SC70_LAYER_2+DISK_5,
	IT_SC70_GRID_1+DISK_5,
	IT_SC70_GRID_2+DISK_5,
	IT_SC70_IRIS+DISK_5,
	IT_SC70_BAR+DISK_5,
	IT_SC70_CONSOLE+DISK_5,
	IT_SC70_GRILL+DISK_5,
	IT_SC70_PIT+DISK_5,
	IT_SC70_STEP_UP+DISK_5,
	IT_SC70_STEP_DOWN+DISK_5,
	IT_SC70_PULL_BAR+DISK_5,
	IT_SC70_ENTER_ANIM+DISK_5,
	0
};

uint16 sc70_bar_anim[] = {
	96*64,
	290,
	170,
	0,
	290,
	170,
	1,
	290,
	170,
	2,
	290,
	170,
	3,
	0
};

Compact sc70_control = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	70,	// screen
	0,	// place
	0,	// getToTable
	365,	// xcood
	215,	// ycood
	115*64,	// frame
	20535,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	360-365,	// mouseRel_x
	212-215,	// mouseRel_y
	387-360,	// mouseSize_x
	252-212,	// mouseSize_y
	SC70_CONTROL_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC70_CONTROL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc70_pit_anim[] = {
	117*64,
	239,
	233,
	0,
	239,
	233,
	1,
	239,
	233,
	2,
	239,
	233,
	3,
	239,
	233,
	4,
	239,
	233,
	5,
	239,
	233,
	6,
	239,
	233,
	7,
	239,
	233,
	8,
	239,
	233,
	9,
	239,
	233,
	2,
	239,
	233,
	3,
	239,
	233,
	4,
	239,
	233,
	5,
	239,
	233,
	6,
	0
};

Compact sc70_grill = {
	0,	// logic
	ST_MOUSE+ST_BACKGROUND,	// status
	0,	// sync
	70,	// screen
	0,	// place
	0,	// getToTable
	265,	// xcood
	168,	// ycood
	116*64,	// frame
	20511,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	294-266,	// mouseSize_x
	173-168,	// mouseSize_y
	SC70_GRILL_ACTION,	// actionScript
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

uint16 sc70_consol_anim[] = {
	115*64,
	365,
	215,
	0,
	365,
	215,
	1,
	365,
	215,
	2,
	365,
	215,
	3,
	365,
	215,
	4,
	365,
	215,
	5,
	365,
	215,
	6,
	365,
	215,
	7,
	365,
	215,
	8,
	365,
	215,
	9,
	365,
	215,
	10,
	365,
	215,
	11,
	365,
	215,
	12,
	365,
	215,
	13,
	365,
	215,
	14,
	365,
	215,
	15,
	0
};

uint16 sc70_mouse_list[] = {
	ID_SC70_PIT,
	ID_SC70_IRIS,
	ID_SC70_BAR,
	ID_SC70_GRILL,
	ID_SC70_CONTROL,
	ID_SC70_DOOR,
	ID_SC70_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

} // namespace SkyCompact

} // namespace Sky

#endif
