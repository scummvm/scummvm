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

#ifndef SKY20COMP_H
#define SKY20COMP_H




namespace Sky {

namespace SkyCompact {

uint16 gal_look_up[] = {
	90*64,
	1,
	1,
	0,
	11,
	1,
	1,
	1,
	1,
	2,
	0
};

uint16 rs_joey_start_20[] = {
	C_XCOOD,
	168,
	C_YCOOD,
	296,
	C_SCREEN,
	20,
	C_PLACE,
	ID_S20_FLOOR,
	C_LOGIC,
	L_SCRIPT,
	C_BASE_SUB,
	JOEY_LOGIC,
	C_BASE_SUB+2,
	0,
	C_FRAME,
	136*64+2,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_COLLISION+ST_MOUSE+ST_GRID_PLOT,
	C_DIR,
	UP,
	65535
};

uint16 s20_pal[] = {
	0,
	512,
	2,
	514,
	771,
	5,
	1028,
	1027,
	1540,
	1032,
	1026,
	8,
	1797,
	1795,
	2054,
	2822,
	1793,
	1290,
	2059,
	3590,
	776,
	3081,
	2821,
	2569,
	2576,
	2822,
	2062,
	2580,
	2564,
	1552,
	3340,
	4108,
	2828,
	3601,
	3591,
	2320,
	2840,
	3076,
	1300,
	3349,
	4615,
	3598,
	3356,
	5124,
	3344,
	4880,
	3595,
	2070,
	5903,
	5380,
	4112,
	4376,
	4873,
	2325,
	4122,
	7179,
	1809,
	5143,
	8202,
	1296,
	6417,
	4871,
	2328,
	4632,
	5138,
	4629,
	5399,
	5389,
	3095,
	4637,
	7181,
	2069,
	7187,
	4613,
	3610,
	6421,
	6926,
	4116,
	4642,
	5640,
	2587,
	5407,
	6923,
	5141,
	7701,
	9480,
	1556,
	6429,
	6156,
	4890,
	7445,
	8722,
	3606,
	6428,
	6932,
	3612,
	6431,
	6160,
	3871,
	5926,
	6669,
	2849,
	6941,
	7448,
	4637,
	6436,
	8979,
	3866,
	8988,
	6408,
	4898,
	6697,
	9743,
	4380,
	7711,
	8474,
	4896,
	7207,
	8469,
	3364,
	9501,
	9233,
	5663,
	9245,
	11031,
	4381,
	7719,
	11033,
	5406,
	9762,
	8977,
	3112,
	8740,
	8475,
	7460,
	9255,
	8978,
	5415,
	8493,
	12311,
	4641,
	8746,
	10011,
	7715,
	9764,
	8736,
	7209,
	9768,
	11289,
	6437,
	9264,
	12310,
	6692,
	11045,
	10265,
	4907,
	10539,
	10005,
	8489,
	10538,
	12573,
	5672,
	10285,
	12576,
	7463,
	11566,
	13080,
	6697,
	11056,
	11805,
	9003,
	10802,
	13601,
	7721,
	11572,
	12570,
	9773,
	11319,
	13088,
	8750,
	11831,
	13604,
	8241,
	12598,
	14120,
	9266,
	13365,
	14635,
	10036,
	13879,
	14638,
	12345,
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

Compact lamb_door_20 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	291,	// xcood
	229,	// ycood
	186*64,	// frame
	181,	// cursorText
	LAMB_20_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	273-252,	// mouseSize_x
	287-228,	// mouseSize_y
	LAMB_DOOR_20_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LAMB_DOOR_20_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact reich_window = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	194,	// xcood
	239,	// ycood
	0,	// frame
	60,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	223-194,	// mouseSize_x
	255-239,	// mouseSize_y
	REICH_WINDOW_ACTION,	// actionScript
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

uint16 usecard_seq[] = {
	192*64,
	258,
	173,
	0,
	258,
	173,
	1,
	258,
	173,
	2,
	258,
	173,
	3,
	258,
	173,
	3,
	258,
	173,
	3,
	258,
	173,
	2,
	258,
	173,
	1,
	258,
	173,
	0,
	0
};

uint16 s20_chip_list[] = {
	185+DISK_9,
	186+DISK_9,
	192+DISK_9,
	90+DISK_9,
	131+DISK_2,
	132+DISK_2,
	217+DISK_9,
	0
};

Compact down_exit_20 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	312,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_DOWN_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	193-128,	// mouseSize_x
	20,	// mouseSize_y
	ED20_ACTION,	// actionScript
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

uint16 s20_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	180+DISK_9,
	181+DISK_9,
	182+DISK_9,
	183+DISK_9,
	184+DISK_9,
	0
};

uint16 rs_all_28_20[] = {
	C_XCOOD,
	160,
	C_YCOOD,
	320,
	C_SCREEN,
	20,
	C_PLACE,
	ID_S20_FLOOR,
	65535
};

uint16 s20_floor_table[] = {
	ID_S20_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S20_START_ON,
	1,
	S20_REICH_ON,
	3,
	S20_LAMB_ON,
	ID_DOWN_EXIT_20,
	GT_DOWN_EXIT_20,
	ID_REICH_DOOR_20,
	GT_REICH_DOOR_20,
	ID_LAMB_DOOR_20,
	GT_LAMB_DOOR_20,
	ID_REICH_SLOT,
	GT_REICH_SLOT,
	ID_LAMB_SLOT,
	GT_LAMB_SLOT,
	ID_SHRUB_1,
	GT_SHRUB_1,
	ID_SHRUB_2,
	GT_SHRUB_2,
	ID_SHRUB_3,
	GT_SHRUB_3,
	ID_GALLAGER_BEL,
	GT_GALLAGER_BEL,
	ID_REICH_WINDOW,
	GT_REICH_WINDOW,
	ID_LAMB_WINDOW,
	GT_LAMB_WINDOW,
	ID_STD_LEFT_TALK,
	GT_L_TALK_20,
	ID_STD_RIGHT_TALK,
	GT_R_TALK_20,
	65535
};

Compact s20_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	s20_floor_table,	// getToTable
	128,	// xcood
	288,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	4242,	// mouseSize_x
	4242,	// mouseSize_y
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

uint16 lamb_20_open[] = {
	186*64,
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
	5,
	1,
	1,
	6,
	1,
	1,
	7,
	1,
	1,
	8,
	1,
	1,
	9,
	1,
	1,
	10,
	0
};

Compact lamb_window = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	342,	// xcood
	240,	// ycood
	0,	// frame
	60,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	368-342,	// mouseSize_x
	257-240,	// mouseSize_y
	LAMB_WINDOW_ACTION,	// actionScript
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

uint32 *grid20 = 0;

uint16 gal_look_down[] = {
	90*64,
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

Compact lamb_slot = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	323,	// xcood
	256,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	7,	// mouseSize_x
	11,	// mouseSize_y
	LAMB_SLOT_ACTION,	// actionScript
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

uint16 lamb_20_close[] = {
	186*64,
	1,
	1,
	9,
	1,
	1,
	8,
	1,
	1,
	7,
	1,
	1,
	6,
	1,
	1,
	5,
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

Compact shrub_3 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	396,	// xcood
	257,	// ycood
	0,	// frame
	12659,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	421-396,	// mouseSize_x
	298-257,	// mouseSize_y
	SHRUB_3_ACTION,	// actionScript
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

uint16 reich_20_close[] = {
	185*64,
	1,
	1,
	9,
	1,
	1,
	8,
	1,
	1,
	7,
	1,
	1,
	6,
	1,
	1,
	5,
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

Compact reich_slot = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	232,	// xcood
	256,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	7,	// mouseSize_x
	11,	// mouseSize_y
	REICH_SLOT_ACTION,	// actionScript
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

Compact shrub_2 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	249,	// xcood
	303,	// ycood
	0,	// frame
	12659,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	310-249,	// mouseSize_x
	4242,	// mouseSize_y
	SHRUB_2_ACTION,	// actionScript
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

uint16 joey_list_20[] = {
	208,
	352,
	296,
	330,
	1,
	0
};

uint16 reich_20_open[] = {
	185*64,
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
	5,
	1,
	1,
	6,
	1,
	1,
	7,
	1,
	1,
	8,
	1,
	1,
	9,
	1,
	1,
	10,
	0
};

Compact shrub_1 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	156,	// xcood
	251,	// ycood
	0,	// frame
	12659,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	193-156,	// mouseSize_x
	294-251,	// mouseSize_y
	SHRUB_1_ACTION,	// actionScript
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

uint16 s20_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_LAMB,
	ID_REICH_DOOR_20,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_INNER_R_DOOR,
	ID_LIFT_29,
	ID_GALLAGER_BEL,
	ID_CAT,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 lamb_card_seq2[] = {
	217*64,
	276,
	231,
	0,
	276,
	231,
	1,
	276,
	231,
	2,
	276,
	231,
	3,
	276,
	231,
	4,
	276,
	231,
	5,
	276,
	231,
	6,
	276,
	231,
	1,
	276,
	231,
	0,
	0
};

Compact reich_door_20 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	252,	// xcood
	228,	// ycood
	185*64,	// frame
	181,	// cursorText
	REICH_20_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	273-252,	// mouseSize_x
	287-228,	// mouseSize_y
	REICH_DOOR_20_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	REICH_DOOR_20_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_20_28[] = {
	C_XCOOD,
	408,
	C_YCOOD,
	248,
	C_SCREEN,
	28,
	C_PLACE,
	ID_S28_FLOOR,
	65535
};

ExtCompact gallager_bel_ext = {
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
	0,	// request
	96,	// spWidth_xx
	SP_COL_GALAG,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	0,	// animScratch
	0,	// megaSet
	0,
	0,
	0,
	0
};

Compact gallager_bel = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_MOUSE+ST_FOREGROUND,	// status
	0,	// sync
	20,	// screen
	0,	// place
	0,	// getToTable
	346,	// xcood
	282,	// ycood
	90*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	369-346,	// mouseSize_x
	320-282,	// mouseSize_y
	GAL_BEL_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	GAL_LOGIC,	// baseSub
	0,	// baseSub_off
	&gallager_bel_ext
};

uint16 s20_mouse[] = {
	ID_JOEY,
	ID_LAMB,
	ID_DOWN_EXIT_20,
	ID_REICH_DOOR_20,
	ID_REICH_SLOT,
	ID_LAMB_SLOT,
	ID_LAMB_DOOR_20,
	ID_SHRUB_1,
	ID_SHRUB_2,
	ID_SHRUB_3,
	ID_GALLAGER_BEL,
	ID_REICH_WINDOW,
	ID_LAMB_WINDOW,
	ID_S20_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

} // namespace SkyCompact

} // namespace Sky

#endif
