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

#ifndef SKY14COMP_H
#define SKY14COMP_H




namespace Sky {

namespace SkyCompact {

uint16 rs_foster_14_13[] = {
	C_XCOOD,
	456,
	C_YCOOD,
	240,
	C_SCREEN,
	13,
	C_PLACE,
	ID_S13_FLOOR,
	65535
};

uint16 rs_foster_13_14[] = {
	C_XCOOD,
	120,
	C_YCOOD,
	256,
	C_SCREEN,
	14,
	C_PLACE,
	ID_S14_FLOOR,
	65535
};

uint16 radman_l_to_r[] = {
	41+90*64,
	40+90*64,
	47+90*64,
	0
};

uint16 radman_auto[32];

uint16 radman_up[] = {
	2,
	0+90*64,
	0,
	65534,
	2,
	1+90*64,
	0,
	65534,
	2,
	2+90*64,
	0,
	65534,
	2,
	3+90*64,
	0,
	65534,
	2,
	4+90*64,
	0,
	65534,
	2,
	5+90*64,
	0,
	65534,
	2,
	6+90*64,
	0,
	65534,
	2,
	7+90*64,
	0,
	65534,
	2,
	8+90*64,
	0,
	65534,
	2,
	9+90*64,
	0,
	65534,
	0
};

uint16 radman_down[] = {
	2,
	10+90*64,
	0,
	2,
	2,
	11+90*64,
	0,
	2,
	2,
	12+90*64,
	0,
	2,
	2,
	13+90*64,
	0,
	2,
	2,
	14+90*64,
	0,
	2,
	2,
	15+90*64,
	0,
	2,
	2,
	16+90*64,
	0,
	2,
	2,
	17+90*64,
	0,
	2,
	2,
	18+90*64,
	0,
	2,
	2,
	19+90*64,
	0,
	2,
	0
};

uint16 radman_left[] = {
	4,
	20+90*64,
	65532,
	0,
	4,
	21+90*64,
	65532,
	0,
	4,
	22+90*64,
	65532,
	0,
	4,
	23+90*64,
	65532,
	0,
	4,
	24+90*64,
	65532,
	0,
	4,
	25+90*64,
	65532,
	0,
	4,
	26+90*64,
	65532,
	0,
	4,
	27+90*64,
	65532,
	0,
	4,
	28+90*64,
	65532,
	0,
	4,
	29+90*64,
	65532,
	0,
	0
};

uint16 radman_right[] = {
	4,
	30+90*64,
	4,
	0,
	4,
	31+90*64,
	4,
	0,
	4,
	32+90*64,
	4,
	0,
	4,
	33+90*64,
	4,
	0,
	4,
	34+90*64,
	4,
	0,
	4,
	35+90*64,
	4,
	0,
	4,
	36+90*64,
	4,
	0,
	4,
	37+90*64,
	4,
	0,
	4,
	38+90*64,
	4,
	0,
	4,
	39+90*64,
	4,
	0,
	0
};

uint16 radman_st_up[] = {
	90*64,
	1,
	0,
	44,
	0
};

uint16 radman_st_down[] = {
	90*64,
	1,
	0,
	40,
	0
};

uint16 radman_st_left[] = {
	90*64,
	1,
	0,
	42,
	0
};

uint16 radman_st_right[] = {
	90*64,
	1,
	0,
	46,
	0
};

uint16 radman_st_talk[] = {
	97*64,
	1,
	0,
	0,
	0
};

uint16 radman_u_to_d[] = {
	43+90*64,
	42+90*64,
	41+90*64,
	0
};

uint16 radman_u_to_l[] = {
	43+90*64,
	0
};

uint16 radman_u_to_r[] = {
	45+90*64,
	0
};

uint16 radman_d_to_u[] = {
	47+90*64,
	46+90*64,
	45+90*64,
	0
};

uint16 radman_d_to_l[] = {
	41+90*64,
	0
};

uint16 radman_d_to_r[] = {
	47+90*64,
	0
};

uint16 radman_l_to_u[] = {
	43+90*64,
	0
};

uint16 radman_l_to_d[] = {
	41+90*64,
	0
};

uint16 radman_r_to_u[] = {
	45+90*64,
	0
};

uint16 radman_r_to_d[] = {
	47+90*64,
	0
};

uint16 radman_r_to_l[] = {
	45+90*64,
	44+90*64,
	43+90*64,
	0
};

TurnTable radman_turnTable0 = {
	{ // turnTableUp
		0,
		radman_u_to_d,
		radman_u_to_l,
		radman_u_to_r,
		0
	},
	{ // turnTableDown
		radman_d_to_u,
		0,
		radman_d_to_l,
		radman_d_to_r,
		0
	},
	{ // turnTableLeft
		radman_l_to_u,
		radman_l_to_d,
		0,
		radman_l_to_r,
		0
	},
	{ // turnTableRight
		radman_r_to_u,
		radman_r_to_d,
		radman_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		0,
		0,
		0,
		0,
		0
	}
};

MegaSet radman_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	radman_up,	// animUp
	radman_down,	// animDown
	radman_left,	// animLeft
	radman_right,	// animRight
	radman_st_up,	// standUp
	radman_st_down,	// standDown
	radman_st_left,	// standLeft
	radman_st_right,	// standRight
	radman_st_talk,	// standTalk
	&radman_turnTable0
};

ExtCompact radman_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	0,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96,	// spWidth_xx
	SP_COL_RADMAN,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	radman_auto,	// animScratch
	0,	// megaSet
	&radman_megaSet0,
	0,
	0,
	0
};

Compact radman = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_MOUSE+ST_COLLISION,	// status
	0,	// sync
	14,	// screen
	ID_S14_FLOOR,	// place
	0,	// getToTable
	280,	// xcood
	256,	// ycood
	90*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65520,	// mouseRel_x
	(int16) 65488,	// mouseRel_y
	32,	// mouseSize_x
	48,	// mouseSize_y
	MEGA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	RADMAN_LOGIC,	// baseSub
	0,	// baseSub_off
	&radman_ext
};

Compact locker2 = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_BACKGROUND+ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	166,	// xcood
	192,	// ycood
	91*64,	// frame
	8254,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	8,	// mouseRel_x
	0,	// mouseRel_y
	17,	// mouseSize_x
	235-192,	// mouseSize_y
	LOCKER2_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOCKER2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 start_joey_14[] = {
	C_XCOOD,
	256,
	C_YCOOD,
	264,
	C_PLACE,
	ID_S14_FLOOR,
	C_SCREEN,
	14,
	C_MODE,
	0,
	C_BASE_SUB,
	JOEY_LOGIC,
	C_BASE_SUB+2,
	0,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,
	C_DIR,
	RIGHT,
	65535
};

Compact fact3_exit_left = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	126,	// xcood
	175,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	11,	// mouseSize_x
	272-175,	// mouseSize_y
	FACT3_ACTION,	// actionScript
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

uint16 rs_anita_to_14[] = {
	C_XCOOD,
	227,
	C_YCOOD,
	189,
	C_SCREEN,
	14,
	C_STATUS,
	ST_BACKGROUND+ST_RECREATE+ST_LOGIC+ST_MOUSE,
	C_BASE_SUB,
	ANITA_WORK,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 s14_logic[] = {
	ID_ANITA,
	ID_FOSTER,
	ID_JOEY,
	ID_RADMAN,
	ID_FOREMAN,
	ID_LOCKER1,
	ID_COAT,
	ID_LOCKER2,
	ID_LOCKER3,
	ID_LAMB,
	ID_LIFT_S7,
	ID_LIFT7_LIGHT,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 close_locker_seq[] = {
	92*64,
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

uint16 locker_close_seq[] = {
	91*64,
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

uint16 s14_floor_table[] = {
	ID_S14_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S14_LEFT_ON,
	1,
	S14_RIGHT_ON,
	ID_FACT3_L_EXIT,
	GT_FACT3_L_EXIT,
	ID_FACT3_R_EXIT,
	GT_FACT3_R_EXIT,
	ID_LOCKER1,
	GT_LOCKER1,
	ID_LOCKER2,
	GT_LOCKER2,
	ID_LOCKER3,
	GT_LOCKER3,
	ID_MACHINE,
	GT_MACHINE,
	ID_RAD_SCREEN,
	GT_RAD_SCREEN,
	ID_14_CONSOLE,
	GT_14_CONSOLE,
	ID_COAT,
	GT_COAT,
	ID_ANITA,
	GT_NU_ANITA,
	ID_STD_LEFT_TALK,
	GT_L_TALK_14,
	ID_STD_RIGHT_TALK,
	GT_R_TALK_14,
	65535
};

Compact s14_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	s14_floor_table,	// getToTable
	128,	// xcood
	216,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	71,	// mouseSize_y
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

uint16 rad_open_locker[] = {
	107*64,
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

uint16 change_back6[] = {
	105*64,
	162,
	190,
	0,
	162,
	190,
	1,
	162,
	190,
	2,
	162,
	190,
	3,
	162,
	190,
	4,
	162,
	190,
	5,
	162,
	190,
	6,
	162,
	190,
	7,
	162,
	190,
	8,
	162,
	190,
	8,
	162,
	190,
	9,
	162,
	190,
	9,
	162,
	190,
	10,
	162,
	190,
	11,
	162,
	190,
	12,
	162,
	190,
	13,
	162,
	190,
	14,
	162,
	190,
	14,
	162,
	190,
	15,
	162,
	190,
	16,
	162,
	190,
	17,
	162,
	190,
	18,
	162,
	190,
	19,
	162,
	190,
	19,
	162,
	190,
	20,
	162,
	190,
	20,
	162,
	190,
	20,
	162,
	190,
	19,
	162,
	190,
	19,
	162,
	190,
	21,
	0
};

uint16 change1_seq[] = {
	99*64,
	162,
	190,
	0,
	162,
	190,
	1,
	162,
	190,
	1,
	162,
	190,
	0,
	162,
	190,
	2,
	162,
	190,
	3,
	162,
	190,
	4,
	162,
	190,
	5,
	162,
	190,
	6,
	162,
	190,
	7,
	162,
	190,
	7,
	162,
	190,
	8,
	162,
	190,
	9,
	162,
	190,
	10,
	162,
	190,
	11,
	162,
	190,
	12,
	162,
	190,
	13,
	162,
	190,
	14,
	162,
	190,
	15,
	162,
	190,
	16,
	162,
	190,
	17,
	162,
	190,
	18,
	162,
	190,
	19,
	162,
	190,
	20,
	162,
	190,
	21,
	162,
	190,
	22,
	162,
	190,
	23,
	162,
	190,
	24,
	162,
	190,
	25,
	162,
	190,
	26,
	0
};

uint16 change_back3[] = {
	102*64,
	0XB1,
	0XC2,
	8,
	0XB1,
	0XC2,
	7,
	0XB1,
	0XC2,
	6,
	0XB1,
	0XC2,
	5,
	0XB1,
	0XC2,
	4,
	0XB1,
	0XC2,
	3,
	0XB1,
	0XC2,
	2,
	0XB1,
	0XC2,
	1,
	0XB1,
	0XC2,
	0,
	0
};

uint16 rs_foster_16_14[] = {
	C_XCOOD,
	416,
	C_YCOOD,
	256,
	C_SCREEN,
	14,
	C_PLACE,
	ID_S14_FLOOR,
	65535
};

Compact console_14 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	216,	// xcood
	204,	// ycood
	0,	// frame
	8371,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	250-216,	// mouseSize_x
	214-204,	// mouseSize_y
	CONSOLE_14_ACTION,	// actionScript
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

uint16 s14_pal[] = {
	0,
	1536,
	1542,
	1544,
	2052,
	1542,
	2056,
	3080,
	1544,
	2570,
	3082,
	2570,
	2576,
	3080,
	3084,
	3088,
	3594,
	3598,
	3600,
	5644,
	2572,
	4110,
	4112,
	4112,
	4116,
	7180,
	2572,
	4118,
	4622,
	4626,
	3612,
	6668,
	3602,
	3619,
	9484,
	3086,
	5654,
	5136,
	5142,
	4636,
	5648,
	5142,
	5146,
	5652,
	5654,
	4135,
	8462,
	4628,
	6172,
	6158,
	6168,
	5667,
	6668,
	4634,
	4649,
	7182,
	5656,
	6686,
	6670,
	5658,
	5667,
	6676,
	6682,
	7196,
	12562,
	2578,
	5163,
	6672,
	7196,
	6181,
	7190,
	6684,
	6183,
	8978,
	4124,
	6691,
	11544,
	4630,
	7201,
	7194,
	7198,
	7207,
	9998,
	5658,
	8485,
	10508,
	2590,
	7710,
	8990,
	4641,
	8476,
	8481,
	6177,
	6193,
	11024,
	4636,
	8478,
	10529,
	3105,
	8990,
	10522,
	6172,
	8481,
	10529,
	4129,
	8483,
	9500,
	4133,
	7721,
	12058,
	4126,
	9003,
	8972,
	8995,
	9509,
	9498,
	7205,
	10023,
	11026,
	7201,
	10529,
	12058,
	5665,
	10019,
	8990,
	9509,
	9515,
	11540,
	7201,
	10029,
	11534,
	4135,
	10023,
	9502,
	9511,
	10035,
	11018,
	8485,
	10021,
	11049,
	6185,
	10025,
	10021,
	7723,
	10543,
	12564,
	3627,
	10535,
	12585,
	4139,
	10537,
	10023,
	8493,
	11055,
	12566,
	4653,
	11053,
	13603,
	5163,
	11051,
	11051,
	8495,
	12085,
	13582,
	4143,
	11565,
	11559,
	9007,
	12083,
	13592,
	5681,
	12077,
	11565,
	12079,
	12591,
	12069,
	10033,
	12079,
	14125,
	4659,
	12591,
	12591,
	10547,
	13623,
	14102,
	6709,
	13105,
	13105,
	12597,
	14135,
	14643,
	13625,
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

uint32 *grid14 = 0;

uint16 change3_seq[] = {
	101*64,
	170,
	193,
	0,
	170,
	193,
	1,
	170,
	193,
	1,
	170,
	193,
	2,
	170,
	193,
	3,
	170,
	193,
	4,
	170,
	193,
	5,
	170,
	193,
	6,
	170,
	193,
	7,
	170,
	193,
	8,
	0
};

uint16 change_back4[] = {
	101*64,
	0XAA,
	0XC1,
	8,
	0XAA,
	0XC1,
	7,
	0XAA,
	0XC1,
	6,
	0XAA,
	0XC1,
	5,
	0XAA,
	0XC1,
	4,
	0XAA,
	0XC1,
	3,
	0XAA,
	0XC1,
	2,
	0XAA,
	0XC1,
	1,
	0XAA,
	0XC1,
	1,
	0XAA,
	0XC1,
	0,
	0
};

uint16 open_locker_seq[] = {
	92*64,
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

uint16 change4_seq[] = {
	102*64,
	177,
	194,
	0,
	177,
	194,
	1,
	177,
	194,
	2,
	177,
	194,
	3,
	177,
	194,
	4,
	177,
	194,
	5,
	177,
	194,
	6,
	177,
	194,
	7,
	177,
	194,
	8,
	0
};

uint16 foster_give_seq[] = {
	43*64,
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
	1,
	1,
	1,
	0,
	0
};

Compact locker3 = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_BACKGROUND+ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	187,	// xcood
	192,	// ycood
	91*64,	// frame
	8254,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	8,	// mouseRel_x
	0,	// mouseRel_y
	17,	// mouseSize_x
	235-192,	// mouseSize_y
	LOCKER3_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOCKER3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact rad_screen = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	292,	// xcood
	195,	// ycood
	0,	// frame
	9288,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	307-292,	// mouseSize_x
	245-195,	// mouseSize_y
	RAD_SCREEN_ACTION,	// actionScript
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

uint16 change5_seq[] = {
	103*64,
	171,
	194,
	0,
	171,
	194,
	1,
	171,
	194,
	2,
	171,
	194,
	3,
	171,
	194,
	4,
	171,
	194,
	5,
	0
};

Compact locker1 = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_BACKGROUND+ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	146,	// xcood
	192,	// ycood
	91*64,	// frame
	8254,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	8,	// mouseRel_x
	0,	// mouseRel_y
	17,	// mouseSize_x
	235-192,	// mouseSize_y
	LOCKER1_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOCKER1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact coat = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	0XAF,	// xcood
	0XCF,	// ycood
	0,	// frame
	8377,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	225-0XCF,	// mouseSize_y
	COAT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	COAT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 change_back5[] = {
	100*64,
	0XAA,
	0XC1,
	8,
	0XAA,
	0XC1,
	8,
	0XAA,
	0XC1,
	7,
	0XAA,
	0XC1,
	6,
	0XAA,
	0XC1,
	5,
	0XAA,
	0XC1,
	4,
	0
};

Compact machine = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	316,	// xcood
	174,	// ycood
	0,	// frame
	9290,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	359-316,	// mouseSize_x
	224-174,	// mouseSize_y
	MACHINE_ACTION,	// actionScript
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

uint16 anita_give_seq[] = {
	44*64,
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
	0,
	0
};

uint16 s14_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	22+DISK_6,
	23+DISK_6,
	92+DISK_6,
	27+DISK_6,
	0
};

Compact fact3_exit_right = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	14,	// screen
	0,	// place
	0,	// getToTable
	381,	// xcood
	184,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	403-381,	// mouseSize_x
	263-184,	// mouseSize_y
	FACT3_R_ACTION,	// actionScript
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

uint16 joey_list_14[] = {
	230,
	376,
	248,
	272,
	1,
	0
};

uint16 rad_close_locker[] = {
	107*64,
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

uint16 locker_open_seq[] = {
	91*64,
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

uint16 s14_mouse[] = {
	ID_JOEY,
	ID_FACT3_L_EXIT,
	ID_FACT3_R_EXIT,
	ID_ANITA,
	ID_RADMAN,
	ID_LOCKER1,
	ID_COAT,
	ID_LOCKER2,
	ID_LOCKER3,
	ID_MACHINE,
	ID_RAD_SCREEN,
	ID_14_CONSOLE,
	ID_S14_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 s14_chip_list[] = {
	173,
	111,
	136+DISK_2,
	137+DISK_2,
	90+DISK_6,
	96+DISK_6+0X8000,
	97+DISK_6,
	106+DISK_6,
	107+DISK_6+0X8000,
	28+DISK_6,
	98+DISK_6,
	43+DISK_6,
	44+DISK_6+0X8000,
	21+DISK_6,
	91+DISK_6,
	0
};

uint16 change_back2[] = {
	103*64,
	0XAB,
	0XC2,
	5,
	0XAB,
	0XC2,
	4,
	0XAB,
	0XC2,
	3,
	0XAB,
	0XC2,
	2,
	0XAB,
	0XC2,
	1,
	0XAB,
	0XC2,
	0,
	0
};

uint16 change2_seq[] = {
	100*64,
	170,
	193,
	0,
	170,
	193,
	1,
	170,
	193,
	2,
	170,
	193,
	3,
	170,
	193,
	4,
	170,
	193,
	5,
	170,
	193,
	6,
	170,
	193,
	7,
	170,
	193,
	8,
	170,
	193,
	8,
	0
};

uint16 change_back1[] = {
	104*64,
	0XB2,
	0XBF,
	9,
	0XB2,
	0XBF,
	8,
	0XB2,
	0XBF,
	7,
	0XB2,
	0XBF,
	6,
	0XB2,
	0XBF,
	5,
	0XB2,
	0XBF,
	4,
	0XB2,
	0XBF,
	4,
	0XB2,
	0XBF,
	3,
	0XB2,
	0XBF,
	2,
	0XB2,
	0XBF,
	1,
	0XB2,
	0XBF,
	0,
	0XB2,
	0XBF,
	0,
	0XB2,
	0XBF,
	0,
	0XB2,
	0XBF,
	0,
	0
};

uint16 change6_seq[] = {
	104*64,
	178,
	191,
	0,
	178,
	191,
	0,
	178,
	191,
	0,
	178,
	191,
	0,
	178,
	191,
	1,
	178,
	191,
	2,
	178,
	191,
	3,
	178,
	191,
	4,
	178,
	191,
	4,
	178,
	191,
	5,
	178,
	191,
	6,
	178,
	191,
	7,
	178,
	191,
	8,
	178,
	191,
	9,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
