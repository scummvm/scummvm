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

#ifndef SKY11COMP_H
#define SKY11COMP_H




namespace Sky {

namespace SkyCompact {

Compact slat_3 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	285,	// xcood
	236,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	3,	// mouseSize_x
	7,	// mouseSize_y
	SLAT_ACTION,	// actionScript
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

uint16 fallin_seq[] = {
	17*64,
	198,
	173,
	0,
	202,
	168,
	1,
	206,
	174,
	2,
	216,
	183,
	3,
	232,
	190,
	4,
	250,
	201,
	5,
	275,
	217,
	6,
	289,
	232,
	7,
	306,
	259,
	8,
	0
};

uint16 s11_mouse[] = {
	ID_JOEY,
	ID_CABLE_FALL,
	ID_SLOT_11,
	ID_LOCKER_11,
	ID_SLAT_1,
	ID_SLAT_2,
	ID_SLAT_3,
	ID_SLAT_4,
	ID_SLAT_5,
	ID_SOCCER_1,
	ID_SOCCER_2,
	ID_SOCCER_3,
	ID_SOCCER_4,
	ID_SOCCER_5,
	ID_RIGHT_EXIT_11,
	ID_S11_FLOOR,
	ID_SPY_11,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 s11_pal[] = {
	0,
	512,
	1026,
	1026,
	1028,
	1028,
	1028,
	1542,
	516,
	1540,
	1028,
	1542,
	1542,
	1540,
	1542,
	1542,
	1032,
	1544,
	2054,
	2054,
	2054,
	2054,
	2056,
	1544,
	2056,
	1544,
	2058,
	2568,
	2056,
	2570,
	2572,
	2564,
	2058,
	2568,
	2572,
	2570,
	2062,
	2056,
	3084,
	2572,
	2572,
	2572,
	3082,
	3596,
	3082,
	3084,
	3084,
	3596,
	3594,
	4108,
	2060,
	3088,
	3082,
	3086,
	3086,
	3086,
	3598,
	4106,
	4108,
	3596,
	3598,
	3086,
	3600,
	3598,
	4624,
	2062,
	4110,
	3084,
	4112,
	4110,
	4110,
	4110,
	4110,
	3600,
	4624,
	4114,
	5130,
	2576,
	4622,
	5650,
	2576,
	4624,
	4624,
	4624,
	4624,
	5138,
	3090,
	4116,
	5650,
	2578,
	4626,
	4114,
	4628,
	5138,
	5136,
	5138,
	4632,
	4620,
	4628,
	5648,
	4114,
	5142,
	5144,
	5132,
	5140,
	5650,
	5652,
	3606,
	5652,
	5138,
	5142,
	5142,
	5142,
	5654,
	5142,
	5656,
	4120,
	6164,
	4628,
	5146,
	5656,
	5656,
	6168,
	6678,
	5654,
	6170,
	6680,
	6164,
	4636,
	7190,
	5656,
	6174,
	7704,
	7192,
	5660,
	7706,
	6166,
	6689,
	7198,
	6684,
	6689,
	8474,
	7708,
	6177,
	8986,
	8476,
	6177,
	8990,
	6680,
	7717,
	9500,
	7196,
	7717,
	9502,
	7710,
	8485,
	10012,
	8481,
	6695,
	10014,
	7710,
	8487,
	10021,
	8478,
	8489,
	10531,
	8995,
	8491,
	11045,
	8990,
	9515,
	11557,
	9505,
	9517,
	11557,
	9511,
	9007,
	12069,
	10023,
	9007,
	12071,
	10021,
	10031,
	12585,
	11045,
	10543,
	13099,
	11045,
	10547,
	13611,
	11559,
	10549,
	4381,
	16128,
	40,
	6955,
	10240,
	31,
	26,
	16128,
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
	63,
	0,
	0,
	0,
	0,
	0,
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

Compact locker_11 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	301,	// xcood
	217,	// ycood
	26*64,	// frame
	12703,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	8,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	44,	// mouseSize_y
	LOCKER_11_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOCKER_11_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 locker_11_open[] = {
	26*64,
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
	0
};

uint16 bits2_seq[] = {
	25*64,
	198,
	182,
	0,
	199,
	182,
	1,
	208,
	191,
	2,
	214,
	221,
	3,
	219,
	245,
	4,
	0
};

Compact soccer_2 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	257,	// xcood
	217,	// ycood
	0,	// frame
	12703,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	271-257,	// mouseSize_x
	260-217,	// mouseSize_y
	SOCCER_1_ACTION,	// actionScript
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

uint16 usecard_11_seq[] = {
	28*64,
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

Compact bits = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	24*64,	// frame
	0,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	44,	// mouseRel_y
	16,	// mouseSize_x
	9,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	BITS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact cable_fall = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	21*64,	// frame
	49,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	44,	// mouseRel_y
	16,	// mouseSize_x
	9,	// mouseSize_y
	CABLE_11_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	CABLE_FALL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact soccer_4 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	291,	// xcood
	217,	// ycood
	0,	// frame
	12703,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	307-291,	// mouseSize_x
	260-217,	// mouseSize_y
	SOCCER_1_ACTION,	// actionScript
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

uint16 s11_floor_table[] = {
	ID_S11_FLOOR,
	RET_OK,
	ID_CABLE_FALL,
	GT_CABLE_11,
	ID_LOCKER_11,
	GT_LOCKER_11,
	ID_SLOT_11,
	GT_SLOT_11,
	ID_SOCCER_1,
	GT_SOCCER_1,
	ID_SOCCER_2,
	GT_SOCCER_2,
	ID_SOCCER_3,
	GT_SOCCER_3,
	ID_SOCCER_4,
	GT_SOCCER_4,
	ID_SOCCER_5,
	GT_SOCCER_5,
	ID_SLAT_1,
	GT_SOCCER_1,
	ID_SLAT_2,
	GT_SOCCER_2,
	ID_SLAT_3,
	GT_SOCCER_3,
	ID_SLAT_4,
	GT_SOCCER_4,
	ID_SLAT_5,
	GT_SOCCER_5,
	ID_RIGHT_EXIT_11,
	GT_RIGHT_EXIT_11,
	0,
	S11_LEFT_ON,
	65535
};

Compact soccer_1 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	238,	// xcood
	217,	// ycood
	0,	// frame
	12703,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	255-238,	// mouseSize_x
	260-217,	// mouseSize_y
	SOCCER_1_ACTION,	// actionScript
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

uint16 rs_foster_1_11[] = {
	C_XCOOD,
	320,
	C_YCOOD,
	288,
	C_SCREEN,
	11,
	C_PLACE,
	ID_S11_FLOOR,
	65535
};

uint16 s11_logic[] = {
	ID_FOSTER,
	ID_CABLE_FALL,
	ID_CABLE_FALL2,
	ID_SMASHED_WINDOW,
	ID_BITS,
	ID_BITS2,
	ID_SPY_11,
	ID_LOCKER_11,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 close_11_locker[] = {
	29*64,
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

uint16 s11_chip_list[] = {
	17+DISK_1,
	18+DISK_1,
	20+DISK_1,
	21+DISK_1,
	22+DISK_1,
	23+DISK_1,
	24+DISK_1,
	25+DISK_1,
	26+DISK_1,
	27+DISK_1,
	28+DISK_1,
	29+DISK_1,
	14+DISK_1,
	15+DISK_1,
	85+DISK_1,
	0
};

Compact slat_1 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	250,	// xcood
	236,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	3,	// mouseSize_x
	7,	// mouseSize_y
	SLAT_ACTION,	// actionScript
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

uint16 locker_11_close[] = {
	26*64,
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

Compact bits2 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	25*64,	// frame
	0,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	44,	// mouseRel_y
	16,	// mouseSize_x
	9,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	BITS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact slot_11 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	320,	// xcood
	236,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	3,	// mouseSize_x
	7,	// mouseSize_y
	SLOT_11_ACTION,	// actionScript
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

uint16 bits_seq[] = {
	24*64,
	213,
	170,
	0,
	229,
	168,
	1,
	240,
	170,
	2,
	250,
	174,
	3,
	266,
	180,
	4,
	0
};

Compact slat_4 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	302,	// xcood
	236,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	3,	// mouseSize_x
	7,	// mouseSize_y
	SLAT_ACTION,	// actionScript
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

uint16 fallin2_seq[] = {
	18*64,
	319,
	273,
	0,
	322,
	276,
	1,
	322,
	279,
	2,
	322,
	278,
	3,
	320,
	277,
	4,
	320,
	278,
	5,
	320,
	279,
	6,
	320,
	279,
	7,
	320,
	279,
	8,
	320,
	279,
	9,
	320,
	278,
	10,
	319,
	279,
	11,
	317,
	275,
	12,
	317,
	272,
	13,
	0
};

uint16 smashed_window_seq[] = {
	23*64,
	201,
	151,
	0,
	201,
	153,
	1,
	201,
	153,
	2,
	201,
	153,
	3,
	201,
	153,
	4,
	201,
	153,
	3,
	201,
	153,
	4,
	201,
	153,
	3,
	201,
	153,
	4,
	201,
	153,
	3,
	201,
	153,
	4,
	201,
	153,
	3,
	0
};

Compact spy_11 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	167,	// xcood
	138,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
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
	SPY11_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact soccer_3 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	273,	// xcood
	217,	// ycood
	0,	// frame
	12703,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	289-273,	// mouseSize_x
	260-217,	// mouseSize_y
	SOCCER_1_ACTION,	// actionScript
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

uint16 cable_fall1_seq[] = {
	21*64,
	195,
	178,
	0,
	195,
	178,
	0,
	195,
	178,
	0,
	195,
	178,
	1,
	195,
	178,
	2,
	195,
	178,
	3,
	195,
	178,
	4,
	195,
	178,
	5,
	195,
	178,
	6,
	195,
	178,
	7,
	195,
	178,
	8,
	195,
	178,
	9,
	195,
	178,
	10,
	195,
	178,
	11,
	195,
	178,
	12,
	195,
	178,
	13,
	195,
	178,
	14,
	195,
	178,
	15,
	195,
	178,
	16,
	195,
	178,
	17,
	195,
	178,
	18,
	195,
	178,
	19,
	195,
	178,
	20,
	195,
	178,
	21,
	195,
	178,
	22,
	0
};

Compact cable_fall2 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	22*64,	// frame
	49,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	44,	// mouseRel_y
	16,	// mouseSize_x
	9,	// mouseSize_y
	CABLE_11_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	CABLE2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact right_exit_11 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	359,	// xcood
	223,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	364-359,	// mouseSize_x
	282-223,	// mouseSize_y
	ER11_ACTION,	// actionScript
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

Compact smashed_window = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	23*64,	// frame
	49,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	44,	// mouseRel_y
	16,	// mouseSize_x
	9,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SMASHED_WINDOW_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact soccer_5 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	327,	// xcood
	217,	// ycood
	0,	// frame
	12703,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	343-327,	// mouseSize_x
	260-217,	// mouseSize_y
	SOCCER_1_ACTION,	// actionScript
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

uint32 *grid11 = 0;

uint16 open_11_locker[] = {
	29*64,
	237,
	184,
	0,
	237,
	184,
	0,
	237,
	184,
	1,
	237,
	184,
	2,
	237,
	184,
	3,
	237,
	184,
	4,
	237,
	184,
	5,
	237,
	184,
	6,
	0
};

Compact slat_2 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	267,	// xcood
	236,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	3,	// mouseSize_x
	7,	// mouseSize_y
	SLAT_ACTION,	// actionScript
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

uint16 fallin3_seq[] = {
	20*64,
	317,
	267,
	0,
	314,
	266,
	1,
	314,
	264,
	2,
	315,
	255,
	3,
	314,
	247,
	4,
	314,
	243,
	5,
	0
};

Compact s11_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	s11_floor_table,	// getToTable
	184,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	400-184,	// mouseSize_x
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

uint16 get_cable_11[] = {
	27*64,
	293,
	252,
	0,
	293,
	252,
	1,
	293,
	252,
	2,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	2,
	293,
	252,
	1,
	293,
	252,
	0,
	0
};

uint16 cant_open_locker[] = {
	29*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

Compact slat_5 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	11,	// screen
	0,	// place
	0,	// getToTable
	337,	// xcood
	236,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	3,	// mouseSize_x
	7,	// mouseSize_y
	SLAT_ACTION,	// actionScript
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

uint16 cable_fall2_seq[] = {
	22*64,
	209,
	242,
	0,
	209,
	242,
	0,
	209,
	242,
	0,
	209,
	242,
	0,
	209,
	242,
	0,
	209,
	242,
	0,
	209,
	242,
	0,
	209,
	242,
	1,
	209,
	242,
	2,
	209,
	242,
	3,
	209,
	242,
	3,
	209,
	242,
	3,
	209,
	242,
	3,
	209,
	242,
	4,
	209,
	242,
	5,
	209,
	242,
	6,
	209,
	242,
	6,
	209,
	242,
	6,
	209,
	242,
	7,
	209,
	242,
	8,
	209,
	242,
	9,
	209,
	242,
	10,
	209,
	242,
	10,
	209,
	242,
	10,
	209,
	242,
	10,
	209,
	242,
	11,
	209,
	242,
	12,
	209,
	242,
	13,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
