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

#ifndef SKY40COMP_H
#define SKY40COMP_H




namespace Sky {

namespace SkyCompact {

Compact sc40_body_2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	263,	// xcood
	222,	// ycood
	0,	// frame
	16689,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	280-263,	// mouseSize_x
	248-222,	// mouseSize_y
	SC40_BODY_2_ACTION,	// actionScript
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

uint16 sc40_locker_shut[] = {
	14*64,
	1,
	1,
	3,
	1,
	1,
	3,
	1,
	1,
	3,
	1,
	1,
	2,
	1,
	1,
	2,
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

uint16 reset_39_40[] = {
	C_SCREEN,
	40,
	C_PLACE,
	ID_SC40_FLOOR,
	C_XCOOD,
	197,
	C_YCOOD,
	196,
	C_FRAME,
	25*64,
	C_DIR,
	RIGHT,
	65535
};

Compact sc40_body_3 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	286,	// xcood
	220,	// ycood
	0,	// frame
	16689,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	305-286,	// mouseSize_x
	249-220,	// mouseSize_y
	SC40_BODY_3_ACTION,	// actionScript
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

Compact sc40_cabinet = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	336,	// xcood
	280,	// ycood
	0,	// frame
	16682,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	357-336,	// mouseSize_x
	312-280,	// mouseSize_y
	SC40_CABINET_ACTION,	// actionScript
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

uint32 *sc40_walk_grid = (uint32*)sc40_fast_list;

uint16 sc40_palette[] = {
	0,
	768,
	515,
	1283,
	1539,
	1285,
	1544,
	1797,
	1544,
	2056,
	2056,
	1803,
	2570,
	3336,
	2057,
	2826,
	3082,
	2318,
	3087,
	2826,
	2830,
	3090,
	3338,
	2320,
	4109,
	6157,
	2315,
	4622,
	3597,
	2579,
	4114,
	6671,
	3084,
	4625,
	3851,
	3858,
	5392,
	7436,
	2829,
	5393,
	5135,
	3093,
	3613,
	5902,
	3348,
	5652,
	5391,
	4885,
	3872,
	8975,
	3086,
	4890,
	5903,
	3608,
	6166,
	4881,
	4122,
	4131,
	5903,
	5144,
	4390,
	6670,
	3865,
	6170,
	5395,
	4636,
	4900,
	8209,
	4630,
	7190,
	6422,
	5658,
	6940,
	5649,
	4127,
	6684,
	6933,
	7449,
	5415,
	6673,
	4382,
	7959,
	6163,
	5663,
	7454,
	7187,
	7196,
	7452,
	10007,
	4377,
	7457,
	6676,
	6432,
	8729,
	7956,
	4384,
	7711,
	8471,
	5408,
	7714,
	6680,
	6179,
	8480,
	7960,
	8478,
	9500,
	9238,
	5409,
	8228,
	7706,
	7203,
	9756,
	8473,
	8481,
	9504,
	7706,
	5416,
	9757,
	9500,
	6179,
	8486,
	9757,
	5412,
	9505,
	8477,
	6184,
	9255,
	9243,
	7717,
	10274,
	10011,
	6182,
	9509,
	9505,
	7208,
	11040,
	8733,
	6443,
	10530,
	10271,
	6951,
	9768,
	8994,
	7211,
	10534,
	9506,
	6701,
	11299,
	10529,
	8489,
	10542,
	9751,
	7469,
	10795,
	10524,
	7724,
	11813,
	11043,
	9258,
	12072,
	10779,
	8493,
	12328,
	10270,
	9007,
	11308,
	10535,
	9266,
	11568,
	11305,
	7732,
	12842,
	12328,
	9519,
	13611,
	11815,
	8246,
	13871,
	11811,
	10550,
	12852,
	12587,
	9017,
	14385,
	12585,
	11833,
	14899,
	13099,
	11836,
	15670,
	14384,
	12863,
	0,
	2560,
	2571,
	3595,
	3595,
	3346,
	4623,
	4367,
	3861,
	5652,
	5903,
	5144,
	8218,
	7705,
	7203,
	10273,
	8728,
	6952,
	10530,
	9503,
	6701,
	11558,
	10269,
	7728,
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

uint16 sc40_descend[] = {
	25*64,
	197,
	196,
	0,
	197,
	199,
	1,
	200,
	200,
	2,
	203,
	201,
	3,
	203,
	202,
	4,
	203,
	206,
	5,
	207,
	207,
	6,
	209,
	208,
	7,
	209,
	209,
	0,
	209,
	212,
	1,
	212,
	213,
	2,
	215,
	214,
	3,
	215,
	215,
	8,
	215,
	218,
	9,
	219,
	219,
	10,
	221,
	219,
	11,
	221,
	218,
	12,
	222,
	218,
	13,
	226,
	218,
	14,
	0
};

Compact sc40_locker_1 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	231,	// xcood
	208,	// ycood
	14*64,	// frame
	16681,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	251-208,	// mouseSize_y
	SC40_LOCKER_1_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC40_LOCKER_1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc40_floor_table[] = {
	ID_SC40_FLOOR,
	RET_OK,
	ID_SC40_EXIT_39,
	GT_SC40_EXIT_39,
	ID_SC40_CABINET,
	GT_SC40_CABINET,
	ID_SC40_TROLLEY,
	GT_SC40_TROLLEY,
	ID_SC40_LOCKER_1,
	GT_SC40_LOCKER_1,
	ID_SC40_LOCKER_2,
	GT_SC40_LOCKER_2,
	ID_SC40_LOCKER_3,
	GT_SC40_LOCKER_3,
	ID_SC40_LOCKER_4,
	GT_SC40_LOCKER_4,
	ID_SC40_LOCKER_5,
	GT_SC40_LOCKER_5,
	ID_SC40_BODY_1,
	GT_SC40_BODY_1,
	ID_SC40_BODY_2,
	GT_SC40_BODY_2,
	ID_SC40_BODY_3,
	GT_SC40_BODY_3,
	ID_SC40_BODY_4,
	GT_SC40_BODY_4,
	ID_SC40_BODY_5,
	GT_SC40_BODY_5,
	65535
};

Compact sc40_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	sc40_floor_table,	// getToTable
	168,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	415-168,	// mouseSize_x
	327-256,	// mouseSize_y
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

uint16 sc40_chip_list[] = {
	IT_SC40_LAYER_0+DISK_14,
	IT_SC40_LAYER_1+DISK_14,
	IT_SC40_LAYER_2+DISK_14,
	IT_SC40_GRID_1+DISK_14,
	IT_SC40_GRID_2+DISK_14,
	IT_SC40_LOCKER+DISK_14,
	IT_SC40_OPEN_DOOR+DISK_14,
	IT_SC40_CLOSE_DOOR+DISK_14,
	25+DISK_14,
	26+DISK_14,
	0
};

Compact sc40_body_5 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	342,	// xcood
	240,	// ycood
	0,	// frame
	16950,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	353-342,	// mouseSize_x
	249-240,	// mouseSize_y
	SC40_BODY_5_ACTION,	// actionScript
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

Compact sc40_locker_4 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	306,	// xcood
	208,	// ycood
	14*64,	// frame
	16681,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	251-208,	// mouseSize_y
	SC40_LOCKER_4_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC40_LOCKER_4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc40_close_door[] = {
	16*64,
	232,
	208,
	0,
	232,
	208,
	0,
	232,
	208,
	1,
	232,
	208,
	1,
	232,
	208,
	2,
	232,
	208,
	2,
	232,
	208,
	3,
	232,
	208,
	3,
	232,
	208,
	3,
	232,
	208,
	3,
	0
};

uint16 sc40_locker_open[] = {
	14*64,
	1,
	1,
	0,
	1,
	1,
	0,
	1,
	1,
	0,
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
	2,
	1,
	1,
	2,
	1,
	1,
	3,
	0
};

Compact sc40_locker_2 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	256,	// xcood
	208,	// ycood
	14*64,	// frame
	16681,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	251-208,	// mouseSize_y
	SC40_LOCKER_2_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC40_LOCKER_2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc40_locker_3 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	281,	// xcood
	208,	// ycood
	14*64,	// frame
	16681,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	251-208,	// mouseSize_y
	SC40_LOCKER_3_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC40_LOCKER_3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc40_fast_list[] = {
	12+DISK_14,
	51+DISK_14,
	52+DISK_14,
	53+DISK_14,
	54+DISK_14,
	135+DISK_14,
	0
};

Compact sc40_exit_39 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	211,	// xcood
	214,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	222-211,	// mouseSize_x
	268-214,	// mouseSize_y
	SC40_EXIT_39_ACTION,	// actionScript
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

Compact sc40_body_1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	236,	// xcood
	208,	// ycood
	0,	// frame
	16689,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	250-236,	// mouseSize_x
	245-208,	// mouseSize_y
	SC40_BODY_1_ACTION,	// actionScript
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

uint16 sc40_open_door[] = {
	15*64,
	240,
	208,
	0,
	240,
	208,
	0,
	240,
	208,
	0,
	240,
	208,
	0,
	240,
	208,
	1,
	240,
	208,
	1,
	240,
	208,
	2,
	240,
	208,
	2,
	240,
	208,
	3,
	240,
	208,
	3,
	0
};

Compact sc40_trolley = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	264,	// xcood
	264,	// ycood
	0,	// frame
	16683,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	336-264,	// mouseSize_x
	280-264,	// mouseSize_y
	SC40_TROLLEY_ACTION,	// actionScript
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

Compact sc40_locker_5 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	331,	// xcood
	208,	// ycood
	14*64,	// frame
	16681,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	251-208,	// mouseSize_y
	SC40_LOCKER_5_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC40_LOCKER_5_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc40_mouse_list[] = {
	ID_SC40_CABINET,
	ID_SC40_TROLLEY,
	ID_SC40_LOCKER_1,
	ID_SC40_LOCKER_2,
	ID_SC40_LOCKER_3,
	ID_SC40_LOCKER_4,
	ID_SC40_LOCKER_5,
	ID_SC40_BODY_1,
	ID_SC40_BODY_2,
	ID_SC40_BODY_3,
	ID_SC40_BODY_4,
	ID_SC40_BODY_5,
	ID_SC40_EXIT_39,
	ID_SC40_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc40_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_SC40_LOCKER_1,
	ID_SC40_LOCKER_2,
	ID_SC40_LOCKER_3,
	ID_SC40_LOCKER_4,
	ID_SC40_LOCKER_5,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

Compact sc40_body_4 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	40,	// screen
	0,	// place
	0,	// getToTable
	311,	// xcood
	219,	// ycood
	0,	// frame
	16689,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	328-311,	// mouseSize_x
	249-219,	// mouseSize_y
	SC40_BODY_4_ACTION,	// actionScript
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

uint16 sc40_ascend[] = {
	26*64,
	217,
	218,
	0,
	214,
	218,
	1,
	212,
	218,
	2,
	212,
	216,
	3,
	211,
	214,
	4,
	208,
	214,
	5,
	206,
	214,
	6,
	204,
	214,
	7,
	204,
	211,
	8,
	203,
	209,
	9,
	200,
	208,
	10,
	198,
	208,
	11,
	198,
	207,
	12,
	198,
	204,
	13,
	198,
	202,
	14,
	195,
	201,
	15,
	193,
	201,
	16,
	193,
	199,
	17,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
