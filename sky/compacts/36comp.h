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

#ifndef SKY36COMP_H
#define SKY36COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc36_get_glass[] = {
	96*64,
	242,
	226,
	0,
	242,
	226,
	1,
	242,
	226,
	2,
	242,
	226,
	3,
	242,
	226,
	4,
	242,
	226,
	4,
	242,
	226,
	4,
	242,
	226,
	4,
	250,
	226,
	5,
	255,
	226,
	6,
	254,
	226,
	7,
	256,
	226,
	8,
	258,
	226,
	9,
	259,
	226,
	10,
	0
};

uint16 sc36_mouse_list[] = {
	ID_SC36_BABS,
	ID_SC36_BARMAN,
	ID_SC36_GALLAGHER,
	ID_SC36_COLSTON,
	ID_SC36_JUKEBOX,
	ID_SC36_BAND,
	ID_SC36_GLASS,
	ID_SC36_SENSOR,
	ID_SC36_DOOR,
	ID_SC36_EXIT_30,
	ID_SC36_FLOOR,
	ID_SC36_LOW_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 babs_d_to_u[] = {
	47+106*64,
	46+106*64,
	45+106*64,
	0
};

uint16 bar_drink[] = {
	32*64,
	391,
	240,
	1,
	391,
	240,
	2,
	391,
	240,
	3,
	391,
	240,
	2,
	391,
	240,
	23,
	391,
	240,
	24,
	391,
	240,
	25,
	391,
	240,
	26,
	391,
	240,
	26,
	391,
	240,
	27,
	391,
	240,
	28,
	391,
	240,
	29,
	391,
	240,
	30,
	391,
	240,
	30,
	391,
	240,
	31,
	391,
	240,
	32,
	391,
	240,
	32,
	391,
	240,
	33,
	391,
	240,
	34,
	391,
	240,
	34,
	391,
	240,
	34,
	391,
	240,
	34,
	391,
	240,
	34,
	391,
	240,
	0,
	0
};

uint16 babs_l_to_u[] = {
	43+106*64,
	0
};

uint16 bar_put_cloth[] = {
	32*64,
	391,
	240,
	15,
	391,
	240,
	14,
	391,
	240,
	13,
	391,
	240,
	8,
	391,
	240,
	12,
	391,
	240,
	12,
	391,
	240,
	12,
	391,
	240,
	12,
	391,
	240,
	8,
	391,
	240,
	9,
	391,
	240,
	0,
	0
};

uint16 babs_u_to_l[] = {
	43+106*64,
	0
};

uint16 sc36_col_down2[] = {
	98*64,
	272,
	213,
	0,
	272,
	213,
	1,
	272,
	213,
	2,
	272,
	213,
	3,
	272,
	213,
	4,
	272,
	213,
	5,
	272,
	213,
	6,
	272,
	213,
	7,
	272,
	213,
	8,
	272,
	213,
	9,
	272,
	213,
	10,
	272,
	213,
	11,
	272,
	213,
	12,
	272,
	213,
	13,
	272,
	213,
	14,
	272,
	213,
	15,
	272,
	213,
	16,
	0
};

uint16 babs_left[] = {
	4,
	20+106*64,
	65532,
	0,
	4,
	21+106*64,
	65532,
	0,
	4,
	22+106*64,
	65532,
	0,
	4,
	23+106*64,
	65532,
	0,
	4,
	24+106*64,
	65532,
	0,
	4,
	25+106*64,
	65532,
	0,
	4,
	26+106*64,
	65532,
	0,
	4,
	27+106*64,
	65532,
	0,
	4,
	28+106*64,
	65532,
	0,
	4,
	29+106*64,
	65532,
	0,
	0
};

uint16 babs_u_to_t[] = {
	43+106*64,
	42+106*64,
	48+106*64,
	0
};

uint16 sc36_col_up1[] = {
	101*64,
	249,
	211,
	0,
	247,
	211,
	1,
	246,
	211,
	2,
	247,
	211,
	3,
	249,
	211,
	4,
	249,
	211,
	5,
	251,
	211,
	6,
	250,
	211,
	7,
	252,
	211,
	8,
	256,
	212,
	9,
	259,
	211,
	10,
	258,
	211,
	11,
	260,
	211,
	12,
	265,
	211,
	5,
	267,
	211,
	6,
	266,
	211,
	7,
	268,
	211,
	8,
	272,
	212,
	9,
	275,
	211,
	10,
	274,
	211,
	11,
	276,
	211,
	12,
	281,
	211,
	5,
	283,
	211,
	6,
	282,
	211,
	7,
	284,
	211,
	8,
	288,
	212,
	9,
	288,
	212,
	3,
	287,
	212,
	13,
	288,
	213,
	14,
	287,
	214,
	15,
	287,
	215,
	16,
	287,
	216,
	17,
	287,
	217,
	18,
	287,
	218,
	19,
	287,
	219,
	20,
	287,
	220,
	13,
	288,
	221,
	14,
	287,
	222,
	15,
	287,
	223,
	16,
	287,
	224,
	17,
	287,
	225,
	18,
	287,
	226,
	19,
	287,
	227,
	20,
	287,
	228,
	13,
	288,
	229,
	14,
	287,
	230,
	15,
	287,
	231,
	16,
	287,
	232,
	21,
	287,
	233,
	22,
	0
};

uint16 sc36_fos_down2[] = {
	93*64,
	298,
	228,
	0,
	298,
	228,
	1,
	298,
	228,
	2,
	298,
	228,
	3,
	298,
	228,
	4,
	298,
	228,
	5,
	298,
	228,
	6,
	298,
	228,
	7,
	298,
	228,
	8,
	298,
	228,
	9,
	298,
	228,
	10,
	298,
	228,
	11,
	298,
	228,
	12,
	298,
	228,
	13,
	298,
	228,
	14,
	298,
	228,
	15,
	298,
	228,
	16,
	298,
	228,
	17,
	298,
	228,
	18,
	298,
	228,
	19,
	298,
	228,
	20,
	298,
	228,
	21,
	298,
	228,
	22,
	298,
	228,
	23,
	298,
	228,
	24,
	298,
	228,
	25,
	298,
	228,
	26,
	298,
	228,
	27,
	298,
	228,
	28,
	298,
	228,
	29,
	298,
	228,
	30,
	298,
	228,
	31,
	298,
	228,
	32,
	298,
	228,
	33,
	0
};

uint16 sc36_col_deal[] = {
	47*64,
	128,
	136,
	0,
	128,
	136,
	3,
	128,
	136,
	4,
	128,
	136,
	5,
	128,
	136,
	6,
	128,
	136,
	7,
	128,
	136,
	7,
	128,
	136,
	8,
	128,
	136,
	0,
	0
};

uint16 sc36_palette[] = {
	0,
	512,
	1540,
	518,
	520,
	2564,
	1538,
	2566,
	2050,
	1540,
	1034,
	3590,
	526,
	1542,
	3078,
	20,
	5634,
	512,
	2566,
	4614,
	2050,
	2054,
	2574,
	3078,
	2054,
	1040,
	5640,
	2564,
	3602,
	3078,
	2058,
	2574,
	4104,
	2566,
	5650,
	1540,
	2566,
	2580,
	2572,
	3076,
	3612,
	3594,
	3078,
	2584,
	4620,
	3086,
	3602,
	5132,
	3594,
	1560,
	8462,
	3592,
	3614,
	3088,
	2582,
	1550,
	9998,
	4620,
	2576,
	6672,
	4108,
	4632,
	5134,
	3094,
	1554,
	10000,
	4104,
	4643,
	4112,
	4110,
	2074,
	10000,
	5132,
	3602,
	6162,
	4614,
	3115,
	8466,
	5646,
	5650,
	5648,
	5130,
	3623,
	7700,
	5134,
	4641,
	6164,
	5128,
	4655,
	7700,
	5654,
	6162,
	6162,
	5126,
	3127,
	11030,
	5648,
	5155,
	4122,
	4636,
	3608,
	11030,
	6674,
	4630,
	8982,
	6158,
	6183,
	7190,
	6154,
	3123,
	12056,
	4133,
	3608,
	12056,
	6166,
	5665,
	4126,
	6166,
	4645,
	10522,
	6674,
	8491,
	7190,
	7702,
	5144,
	10012,
	7184,
	3119,
	13086,
	7188,
	6187,
	7710,
	7706,
	7198,
	8988,
	5675,
	5660,
	12062,
	7700,
	7219,
	10014,
	8988,
	7196,
	8995,
	8472,
	5169,
	14625,
	6699,
	7201,
	11553,
	8984,
	6711,
	13093,
	9498,
	5687,
	15143,
	10017,
	8487,
	11557,
	9502,
	9523,
	10533,
	10019,
	8495,
	13607,
	10526,
	9527,
	10027,
	10533,
	10033,
	11051,
	11045,
	10037,
	12077,
	11559,
	9011,
	15661,
	11559,
	11063,
	12591,
	12075,
	10549,
	15151,
	12589,
	12085,
	12593,
	13101,
	13115,
	13109,
	13621,
	14641,
	13113,
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
	13312,
	15413,
	8996,
	4912,
	9236,
	2060,
	528,
	1539,
	12602,
	12585,
	6948,
	6696,
	7951,
	1808,
	2326,
	4097,
	5144,
	4104,
	11020,
	14126,
	7195,
	11303,
	2056,
	7188,
	24,
	2052,
	8220,
	3120,
	8204,
	2052,
	4120,
	8216,
	3076,
	1048,
	1032,
	2052,
	3084,
	3080,
	6156,
	6180,
	9244,
	8216,
	2088,
	7180,
	4108,
	1060,
	3080,
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
	0,
	14336,
	13364,
	10795,
	9261,
	9762,
	6174,
	5662,
	4880,
	2319,
	1802,
	770,
	519,
	14851,
	10545,
	10034,
	11037,
	4894,
	5668,
	7435,
	1295,
	2326,
	11265,
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
	10781,
	5401,
	4898,
	6670,
	2062,
	530,
	1024,
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

uint16 babs_auto[32];

uint16 babs_up[] = {
	2,
	0+106*64,
	0,
	65534,
	2,
	1+106*64,
	0,
	65534,
	2,
	2+106*64,
	0,
	65534,
	2,
	3+106*64,
	0,
	65534,
	2,
	4+106*64,
	0,
	65534,
	2,
	5+106*64,
	0,
	65534,
	2,
	6+106*64,
	0,
	65534,
	2,
	7+106*64,
	0,
	65534,
	2,
	8+106*64,
	0,
	65534,
	2,
	9+106*64,
	0,
	65534,
	0
};

uint16 babs_down[] = {
	2,
	10+106*64,
	0,
	2,
	2,
	11+106*64,
	0,
	2,
	2,
	12+106*64,
	0,
	2,
	2,
	13+106*64,
	0,
	2,
	2,
	14+106*64,
	0,
	2,
	2,
	15+106*64,
	0,
	2,
	2,
	16+106*64,
	0,
	2,
	2,
	17+106*64,
	0,
	2,
	2,
	18+106*64,
	0,
	2,
	2,
	19+106*64,
	0,
	2,
	0
};

uint16 babs_right[] = {
	4,
	30+106*64,
	4,
	0,
	4,
	31+106*64,
	4,
	0,
	4,
	32+106*64,
	4,
	0,
	4,
	33+106*64,
	4,
	0,
	4,
	34+106*64,
	4,
	0,
	4,
	35+106*64,
	4,
	0,
	4,
	36+106*64,
	4,
	0,
	4,
	37+106*64,
	4,
	0,
	4,
	38+106*64,
	4,
	0,
	4,
	39+106*64,
	4,
	0,
	0
};

uint16 babs_st_up[] = {
	106*64,
	1,
	0,
	44,
	0
};

uint16 babs_st_down[] = {
	106*64,
	1,
	0,
	40,
	0
};

uint16 babs_st_left[] = {
	106*64,
	1,
	0,
	42,
	0
};

uint16 babs_st_right[] = {
	106*64,
	1,
	0,
	46,
	0
};

uint16 babs_st_talk[] = {
	107*64,
	1,
	0,
	0,
	0
};

uint16 babs_u_to_d[] = {
	43+106*64,
	42+106*64,
	41+106*64,
	0
};

uint16 babs_u_to_r[] = {
	45+106*64,
	0
};

uint16 babs_d_to_l[] = {
	41+106*64,
	0
};

uint16 babs_d_to_r[] = {
	47+106*64,
	0
};

uint16 babs_d_to_t[] = {
	48+106*64,
	0
};

uint16 babs_l_to_d[] = {
	41+106*64,
	0
};

uint16 babs_l_to_r[] = {
	41+106*64,
	40+106*64,
	47+106*64,
	0
};

uint16 babs_l_to_t[] = {
	48+106*64,
	0
};

uint16 babs_r_to_u[] = {
	45+106*64,
	0
};

uint16 babs_r_to_d[] = {
	47+106*64,
	0
};

uint16 babs_r_to_l[] = {
	45+106*64,
	44+106*64,
	43+106*64,
	0
};

uint16 babs_r_to_t[] = {
	47+106*64,
	40+106*64,
	48+106*64,
	0
};

uint16 babs_t_to_u[] = {
	48+106*64,
	42+106*64,
	43+106*64,
	0
};

uint16 babs_t_to_d[] = {
	48+106*64,
	0
};

uint16 babs_t_to_l[] = {
	48+106*64,
	0
};

uint16 babs_t_to_r[] = {
	48+106*64,
	40+106*64,
	47+106*64,
	0
};

TurnTable sc36_babs_turnTable0 = {
	{ // turnTableUp
		0,
		babs_u_to_d,
		babs_u_to_l,
		babs_u_to_r,
		babs_u_to_t
	},
	{ // turnTableDown
		babs_d_to_u,
		0,
		babs_d_to_l,
		babs_d_to_r,
		babs_d_to_t
	},
	{ // turnTableLeft
		babs_l_to_u,
		babs_l_to_d,
		0,
		babs_l_to_r,
		babs_l_to_t
	},
	{ // turnTableRight
		babs_r_to_u,
		babs_r_to_d,
		babs_r_to_l,
		0,
		babs_r_to_t
	},
	{ // turnTableTalk
		babs_t_to_u,
		babs_t_to_d,
		babs_t_to_l,
		babs_t_to_r,
		0
	}
};

MegaSet sc36_babs_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	babs_up,	// animUp
	babs_down,	// animDown
	babs_left,	// animLeft
	babs_right,	// animRight
	babs_st_up,	// standUp
	babs_st_down,	// standDown
	babs_st_left,	// standLeft
	babs_st_right,	// standRight
	babs_st_talk,	// standTalk
	&sc36_babs_turnTable0
};

ExtCompact sc36_babs_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	TALK,	// dir
	STD_PLAYER_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_COL_BABS36,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	babs_auto,	// animScratch
	0,	// megaSet
	&sc36_babs_megaSet0,
	0,
	0,
	0
};

Compact sc36_babs = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_SORT+ST_RECREATE+ST_GRID_PLOT+ST_COLLISION,	// status
	0,	// sync
	36,	// screen
	ID_SC36_FLOOR,	// place
	0,	// getToTable
	344,	// xcood
	264,	// ycood
	107*64,	// frame
	54,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65526,	// mouseRel_x
	(int16) 65491,	// mouseRel_y
	17,	// mouseSize_x
	50,	// mouseSize_y
	MEGA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC36_BABS_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc36_babs_ext
};

uint16 sc36_col_down4[] = {
	100*64,
	288,
	233,
	0,
	287,
	233,
	1,
	288,
	232,
	2,
	288,
	231,
	3,
	288,
	229,
	4,
	287,
	229,
	5,
	287,
	228,
	6,
	288,
	227,
	7,
	288,
	225,
	8,
	287,
	225,
	9,
	288,
	224,
	10,
	288,
	223,
	3,
	288,
	221,
	4,
	287,
	221,
	5,
	287,
	220,
	6,
	288,
	219,
	7,
	288,
	217,
	8,
	287,
	217,
	9,
	288,
	216,
	10,
	288,
	215,
	3,
	288,
	213,
	4,
	287,
	213,
	5,
	287,
	212,
	6,
	288,
	211,
	11,
	290,
	211,
	12,
	287,
	211,
	13,
	283,
	211,
	14,
	283,
	211,
	15,
	282,
	212,
	16,
	279,
	211,
	17,
	275,
	211,
	18,
	275,
	211,
	19,
	274,
	211,
	12,
	271,
	211,
	13,
	267,
	211,
	14,
	267,
	211,
	15,
	266,
	212,
	16,
	263,
	211,
	17,
	259,
	211,
	18,
	259,
	211,
	19,
	258,
	211,
	12,
	255,
	211,
	13,
	251,
	211,
	14,
	251,
	211,
	15,
	250,
	212,
	16,
	247,
	211,
	17,
	249,
	211,
	20,
	249,
	211,
	20,
	0
};

uint16 sc36_logic_list[] = {
	ID_FOSTER,
	ID_SC36_BABS,
	ID_SC36_BARMAN,
	ID_SC36_COLSTON,
	ID_SC36_GALLAGHER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_SC30_HENRI,
	ID_SC36_COL_FEET,
	ID_SC36_GAL_LEGS,
	ID_SC36_CARDS,
	ID_SC36_GLASS,
	ID_SC36_BAND,
	ID_SC36_JUKEBOX,
	ID_SC36_JUKE_LIGHT,
	ID_SC36_DOOR,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 reset_30_36[] = {
	C_SCREEN,
	36,
	C_PLACE,
	ID_SC36_FLOOR,
	C_XCOOD,
	168,
	C_YCOOD,
	280,
	C_FRAME,
	40+12*64,
	C_DIR,
	DOWN,
	65535
};

uint16 sc36_doorshut[] = {
	144*64,
	375,
	183,
	5,
	375,
	183,
	4,
	375,
	183,
	3,
	375,
	183,
	2,
	375,
	183,
	1,
	375,
	183,
	0,
	0
};

uint16 sc36_fos_up2[] = {
	95*64,
	296,
	208,
	0,
	296,
	208,
	1,
	296,
	208,
	2,
	296,
	208,
	3,
	296,
	208,
	4,
	296,
	208,
	5,
	296,
	208,
	6,
	296,
	208,
	7,
	296,
	208,
	8,
	296,
	208,
	9,
	296,
	208,
	10,
	296,
	208,
	11,
	296,
	208,
	12,
	296,
	208,
	13,
	296,
	208,
	14,
	296,
	208,
	15,
	296,
	208,
	16,
	296,
	208,
	17,
	296,
	208,
	18,
	296,
	208,
	19,
	296,
	208,
	20,
	296,
	208,
	21,
	296,
	208,
	22,
	0
};

uint16 bar_get_cloth[] = {
	32*64,
	391,
	240,
	9,
	391,
	240,
	8,
	391,
	240,
	12,
	391,
	240,
	12,
	391,
	240,
	12,
	391,
	240,
	12,
	391,
	240,
	8,
	391,
	240,
	13,
	391,
	240,
	14,
	391,
	240,
	15,
	0
};

Compact sc36_juke_light = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	233,	// xcood
	212,	// ycood
	68*64,	// frame
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
	SC36_JUKE_LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc36_low_floor_table[] = {
	ID_SC36_FLOOR,
	GT_SC36_FLOOR,
	ID_SC36_LOW_FLOOR,
	RET_OK,
	ID_SC36_EXIT_30,
	GT_SC36_FLOOR,
	ID_SC36_DOOR,
	GT_SC36_DOOR,
	ID_SC36_SENSOR,
	GT_SC36_SENSOR,
	ID_SC36_BAND,
	GT_SC36_BAND,
	ID_SC36_JUKEBOX,
	GT_SC36_JUKEBOX,
	0,
	SC36_DOOR_WALK_ON,
	ID_SC36_BARMAN,
	GT_SC36_FLOOR,
	ID_SC36_COLSTON,
	GT_SC36_FLOOR,
	ID_SC36_GALLAGHER,
	GT_SC36_FLOOR,
	ID_SC36_GLASS,
	GT_SC36_FLOOR,
	ID_STD_LEFT_TALK,
	GT_SC36_FLOOR,
	ID_STD_RIGHT_TALK,
	GT_SC36_FLOOR,
	ID_SC36_BABS,
	GT_SC36_FLOOR,
	65535
};

Compact sc36_low_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	sc36_low_floor_table,	// getToTable
	248,	// xcood
	200,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	383-248,	// mouseSize_x
	255-200,	// mouseSize_y
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

uint16 sc36_floor_table[] = {
	ID_SC36_FLOOR,
	RET_OK,
	ID_SC36_LOW_FLOOR,
	GT_SC36_LOW_FLOOR,
	ID_SC36_EXIT_30,
	GT_SC36_EXIT_30,
	0,
	SC36_EXIT_30_WALK_ON,
	ID_SC36_DOOR,
	GT_SC36_LOW_FLOOR,
	ID_SC36_SENSOR,
	GT_SC36_LOW_FLOOR,
	ID_SC36_BAND,
	GT_SC36_LOW_FLOOR,
	ID_SC36_JUKEBOX,
	GT_SC36_LOW_FLOOR,
	ID_SC36_BARMAN,
	GT_SC36_BARMAN,
	ID_SC36_COLSTON,
	GT_SC36_COLSTON,
	ID_SC36_GALLAGHER,
	GT_SC36_GALLAGHER,
	ID_SC36_GLASS,
	GT_SC36_GLASS,
	ID_STD_LEFT_TALK,
	GT_SC36_LEFT_TALK,
	ID_STD_RIGHT_TALK,
	GT_SC36_RIGHT_TALK,
	3,
	MOVE_BABS,
	ID_SC36_BABS,
	GT_BABS_AMIGA,
	65535
};

uint16 sc36_jukebox_on[] = {
	68*64,
	233,
	212,
	0,
	233,
	212,
	1,
	233,
	212,
	2,
	233,
	212,
	3,
	0
};

Compact sc36_door = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	375,	// xcood
	183,	// ycood
	144*64,	// frame
	181,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	378-374,	// mouseSize_x
	214-182,	// mouseSize_y
	SC36_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC36_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc36_col_down3[] = {
	99*64,
	283,
	234,
	0,
	283,
	234,
	1,
	283,
	234,
	2,
	283,
	234,
	3,
	283,
	234,
	4,
	283,
	234,
	5,
	283,
	234,
	6,
	283,
	234,
	7,
	283,
	234,
	8,
	283,
	234,
	9,
	283,
	234,
	10,
	283,
	234,
	11,
	283,
	234,
	12,
	283,
	234,
	13,
	283,
	234,
	14,
	283,
	234,
	15,
	283,
	234,
	16,
	283,
	234,
	17,
	283,
	234,
	18,
	283,
	234,
	19,
	283,
	234,
	20,
	283,
	234,
	21,
	283,
	234,
	22,
	283,
	234,
	23,
	283,
	234,
	24,
	283,
	234,
	25,
	283,
	234,
	26,
	283,
	234,
	27,
	283,
	234,
	28,
	283,
	234,
	29,
	0
};

ExtCompact sc36_jukebox_ext = {
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
	96+32,	// spWidth_xx
	SP_COL_JUKEBOX36,	// spColour
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

Compact sc36_jukebox = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	233,	// xcood
	222,	// ycood
	86*64,	// frame
	16649,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65521,	// mouseRel_y
	241-229,	// mouseSize_x
	236-207,	// mouseSize_y
	SC36_JUKEBOX_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC36_JUKEBOX_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc36_jukebox_ext
};

uint16 sc36_juke_kicked[] = {
	68*64,
	233,
	212,
	0,
	233,
	212,
	2,
	233,
	212,
	0,
	233,
	212,
	0,
	233,
	212,
	1,
	233,
	212,
	3,
	233,
	212,
	0,
	233,
	212,
	2,
	233,
	212,
	2,
	233,
	212,
	1,
	233,
	212,
	3,
	233,
	212,
	0,
	233,
	212,
	3,
	233,
	212,
	0,
	233,
	212,
	2,
	233,
	212,
	0,
	233,
	212,
	1,
	233,
	212,
	3,
	233,
	212,
	2,
	233,
	212,
	3,
	0
};

uint16 sc36_fos_up1[] = {
	94*64,
	298,
	229,
	0,
	298,
	229,
	1,
	298,
	229,
	2,
	298,
	229,
	3,
	298,
	229,
	4,
	298,
	229,
	5,
	298,
	229,
	6,
	298,
	229,
	7,
	298,
	229,
	8,
	298,
	229,
	9,
	298,
	229,
	10,
	298,
	229,
	11,
	298,
	229,
	12,
	298,
	229,
	13,
	298,
	229,
	14,
	298,
	229,
	15,
	298,
	229,
	16,
	298,
	229,
	17,
	298,
	229,
	18,
	298,
	229,
	19,
	298,
	229,
	20,
	298,
	229,
	21,
	298,
	229,
	22,
	298,
	229,
	23,
	298,
	229,
	24,
	298,
	229,
	25,
	298,
	229,
	26,
	298,
	229,
	27,
	298,
	229,
	28,
	298,
	229,
	29,
	298,
	229,
	30,
	298,
	229,
	31,
	298,
	229,
	32,
	298,
	229,
	33,
	298,
	229,
	34,
	298,
	229,
	35,
	0
};

uint16 sc36_use_jukebox[] = {
	67*64,
	240,
	210,
	0,
	240,
	210,
	1,
	240,
	210,
	2,
	240,
	210,
	3,
	240,
	210,
	4,
	240,
	210,
	4,
	240,
	210,
	4,
	240,
	210,
	4,
	240,
	210,
	5,
	240,
	210,
	6,
	240,
	210,
	0,
	0
};

uint16 sc36_col_think[] = {
	47*64,
	219,
	226,
	0,
	219,
	226,
	3,
	219,
	226,
	4,
	219,
	226,
	5,
	219,
	226,
	9,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	10,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	9,
	219,
	226,
	10,
	219,
	226,
	11,
	219,
	226,
	12,
	219,
	226,
	13,
	219,
	226,
	14,
	219,
	226,
	15,
	219,
	226,
	12,
	219,
	226,
	13,
	219,
	226,
	14,
	219,
	226,
	15,
	219,
	226,
	12,
	219,
	226,
	12,
	219,
	226,
	13,
	219,
	226,
	14,
	219,
	226,
	15,
	219,
	226,
	12,
	219,
	226,
	13,
	219,
	226,
	14,
	219,
	226,
	15,
	219,
	226,
	12,
	219,
	226,
	0,
	0
};

uint16 bar_wipe2[] = {
	32*64,
	391,
	240,
	16,
	391,
	240,
	17,
	391,
	240,
	18,
	391,
	240,
	19,
	391,
	240,
	20,
	391,
	240,
	21,
	391,
	240,
	20,
	391,
	240,
	19,
	391,
	240,
	20,
	391,
	240,
	21,
	391,
	240,
	22,
	391,
	240,
	16,
	391,
	240,
	17,
	391,
	240,
	18,
	391,
	240,
	19,
	391,
	240,
	20,
	391,
	240,
	19,
	391,
	240,
	18,
	391,
	240,
	17,
	391,
	240,
	16,
	391,
	240,
	22,
	391,
	240,
	21,
	391,
	240,
	22,
	391,
	240,
	16,
	391,
	240,
	17,
	391,
	240,
	18,
	391,
	240,
	19,
	391,
	240,
	20,
	391,
	240,
	21,
	391,
	240,
	22,
	391,
	240,
	16,
	391,
	240,
	17,
	391,
	240,
	18,
	391,
	240,
	19,
	391,
	240,
	20,
	391,
	240,
	19,
	391,
	240,
	18,
	391,
	240,
	17,
	391,
	240,
	18,
	391,
	240,
	19,
	391,
	240,
	20,
	391,
	240,
	21,
	391,
	240,
	22,
	391,
	240,
	16,
	0
};

Compact sc36_gal_legs = {
	0,	// logic
	ST_SORT+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	202,	// xcood
	262,	// ycood
	59*64,	// frame
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
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

uint16 sc36_col_up2[] = {
	102*64,
	283,
	234,
	0,
	283,
	234,
	1,
	283,
	234,
	2,
	283,
	234,
	3,
	283,
	234,
	4,
	283,
	234,
	5,
	283,
	234,
	6,
	283,
	234,
	7,
	283,
	234,
	8,
	283,
	234,
	9,
	283,
	234,
	10,
	283,
	234,
	11,
	283,
	234,
	12,
	283,
	234,
	13,
	283,
	234,
	14,
	283,
	234,
	15,
	283,
	234,
	16,
	283,
	234,
	17,
	283,
	234,
	18,
	283,
	234,
	19,
	283,
	234,
	20,
	283,
	234,
	21,
	283,
	234,
	22,
	283,
	234,
	23,
	283,
	234,
	24,
	283,
	234,
	25,
	283,
	234,
	26,
	0
};

Compact sc36_cards = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	224,	// xcood
	252,	// ycood
	60*64,	// frame
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
	SC36_CARDS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc36_reach_glass[] = {
	96*64,
	242,
	226,
	0,
	242,
	226,
	1,
	242,
	226,
	2,
	242,
	226,
	3,
	242,
	226,
	4,
	242,
	226,
	4,
	242,
	226,
	4,
	242,
	226,
	4,
	242,
	226,
	2,
	242,
	226,
	1,
	242,
	226,
	0,
	0
};

uint16 sc36_juke_break[] = {
	86*64,
	233,
	222,
	7,
	233,
	222,
	1,
	233,
	222,
	4,
	233,
	222,
	6,
	233,
	222,
	2,
	233,
	222,
	5,
	233,
	222,
	0,
	233,
	222,
	3,
	0
};

uint16 sc36_fos_down1[] = {
	92*64,
	296,
	208,
	0,
	296,
	208,
	1,
	296,
	208,
	2,
	296,
	208,
	3,
	296,
	208,
	4,
	296,
	208,
	5,
	296,
	208,
	6,
	296,
	208,
	7,
	296,
	208,
	8,
	296,
	208,
	9,
	296,
	208,
	10,
	296,
	208,
	11,
	296,
	208,
	12,
	296,
	208,
	13,
	296,
	208,
	14,
	296,
	208,
	15,
	296,
	208,
	16,
	296,
	208,
	17,
	296,
	208,
	18,
	296,
	208,
	19,
	296,
	208,
	20,
	296,
	208,
	21,
	296,
	208,
	22,
	0
};

uint16 reset_37_36[] = {
	C_SCREEN,
	36,
	C_PLACE,
	ID_SC36_LOW_FLOOR,
	C_XCOOD,
	392,
	C_YCOOD,
	208,
	C_FRAME,
	42+13*64,
	65535
};

Compact sc36_glass = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	242,	// xcood
	247,	// ycood
	66*64,	// frame
	200,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	246-241,	// mouseSize_x
	253-246,	// mouseSize_y
	SC36_GLASS_ACTION,	// actionScript
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

Compact sc36_col_feet = {
	0,	// logic
	ST_SORT+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	219,	// xcood
	265,	// ycood
	48*64,	// frame
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
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

uint16 sc36_col_blink2[] = {
	47*64,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	16,
	219,
	226,
	17,
	219,
	226,
	18,
	219,
	226,
	16,
	0
};

uint16 sc36_jukebox_off[] = {
	68*64,
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

Compact sc36_band = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	271,	// xcood
	161,	// ycood
	31*64,	// frame
	16653,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	5,	// mouseRel_y
	322-271,	// mouseSize_x
	187-166,	// mouseSize_y
	SC36_BAND_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC36_BAND_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc36_sensor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	371,	// xcood
	191,	// ycood
	0,	// frame
	16651,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	373-371,	// mouseSize_x
	197-191,	// mouseSize_y
	SC36_SENSOR_ACTION,	// actionScript
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

uint16 sc36_col_drink[] = {
	69*64,
	219,
	226,
	0,
	219,
	226,
	1,
	219,
	226,
	2,
	219,
	226,
	3,
	219,
	226,
	3,
	219,
	226,
	4,
	219,
	226,
	5,
	219,
	226,
	6,
	219,
	226,
	7,
	219,
	226,
	7,
	219,
	226,
	7,
	219,
	226,
	7,
	219,
	226,
	7,
	219,
	226,
	6,
	219,
	226,
	5,
	219,
	226,
	4,
	219,
	226,
	3,
	219,
	226,
	3,
	219,
	226,
	2,
	219,
	226,
	1,
	219,
	226,
	8,
	0
};

uint16 reset_juke_light[] = {
	C_LOGIC,
	L_SCRIPT,
	C_FRAME,
	68*64,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 sc36_gal_look1[] = {
	58*64,
	202,
	238,
	6,
	202,
	238,
	7,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	8,
	202,
	238,
	7,
	202,
	238,
	6,
	0
};

uint16 reset_colston[] = {
	C_LOGIC,
	L_SCRIPT,
	C_STATUS,
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,
	C_XCOOD,
	219,
	C_YCOOD,
	226,
	C_FRAME,
	47*64,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC36_COLSTON_LOGIC,
	C_BASE_SUB+2,
	0,
	C_REQUEST,
	0,
	65535
};

uint16 sc36_chip_list[] = {
	13+DISK_13,
	IT_SC36_LAYER_0+DISK_13,
	IT_SC36_LAYER_1+DISK_13,
	IT_SC36_LAYER_2+DISK_13,
	IT_SC36_LAYER_3+DISK_13,
	IT_SC36_GRID_1+DISK_13,
	IT_SC36_GRID_2+DISK_13,
	IT_SC36_GRID_3+DISK_13,
	IT_SC36_DOOR+DISK_13,
	IT_SC36_BABS+DISK_13,
	IT_SC36_BABS_TALK+DISK_13,
	IT_SC36_PRESS_PLATE+DISK_13,
	IT_SC36_USE_JUKEBOX+DISK_13,
	IT_SC36_JUKEBOX+DISK_13,
	IT_SC36_JUKE_LIGHT+DISK_13,
	IT_SC36_BAND+DISK_13,
	IT_SC36_BARMAN+DISK_13,
	IT_BARMAN_TALK+DISK_13,
	IT_SC36_COLSTON+DISK_13,
	IT_SC36_COL_FEET+DISK_13,
	IT_SC36_COL_TALK1+DISK_13,
	IT_SC36_COL_TALK2+DISK_13,
	IT_SC36_COL_DRINK+DISK_13,
	IT_SC36_COL_DOWN4+DISK_13,
	IT_SC36_COL_UP1+DISK_13,
	IT_SC36_COL_KICK+DISK_13,
	IT_SC36_GALLAGHER+DISK_13,
	IT_SC36_GAL_LEGS+DISK_13,
	IT_SC36_GAL_TALK+DISK_13,
	IT_SC36_CARDS+DISK_13,
	IT_SC36_GLASS+DISK_13,
	0
};

uint16 sc36_juke_stuck[] = {
	86*64,
	233,
	222,
	7,
	233,
	222,
	3,
	0
};

uint16 sc36_col_blink1[] = {
	47*64,
	219,
	226,
	0,
	219,
	226,
	1,
	219,
	226,
	2,
	219,
	226,
	0,
	0
};

uint32 *babs = (uint32*)&sc36_babs;

uint16 bar_wipe[] = {
	32*64,
	391,
	240,
	16,
	391,
	240,
	17,
	391,
	240,
	18,
	391,
	240,
	19,
	391,
	240,
	20,
	391,
	240,
	21,
	391,
	240,
	22,
	0
};

uint16 sc36_gal_look2[] = {
	58*64,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	202,
	238,
	0,
	0
};

uint16 sc36_juke_sing[] = {
	86*64,
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
	0,
	1,
	1,
	3,
	1,
	1,
	4,
	1,
	1,
	0,
	1,
	1,
	6,
	1,
	1,
	0,
	1,
	1,
	3,
	1,
	1,
	6,
	1,
	1,
	5,
	1,
	1,
	7,
	1,
	1,
	6,
	1,
	1,
	0,
	1,
	1,
	7,
	1,
	1,
	4,
	1,
	1,
	2,
	1,
	1,
	5,
	1,
	1,
	3,
	1,
	1,
	7,
	1,
	1,
	4,
	0
};

ExtCompact sc36_colston_ext = {
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
	96+32,	// spWidth_xx
	SP_COL_COLSTON36,	// spColour
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

Compact sc36_colston = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	219,	// xcood
	226,	// ycood
	47*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	240-219,	// mouseSize_x
	252-226,	// mouseSize_y
	SC36_COLSTON_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC36_COLSTON_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc36_colston_ext
};

uint16 sc36_col_down1[] = {
	97*64,
	220,
	226,
	0,
	219,
	227,
	1,
	221,
	223,
	2,
	222,
	219,
	3,
	222,
	217,
	4,
	222,
	217,
	4,
	222,
	217,
	4,
	223,
	217,
	5,
	225,
	217,
	6,
	224,
	217,
	7,
	233,
	218,
	8,
	236,
	217,
	9,
	241,
	217,
	10,
	240,
	217,
	11,
	243,
	218,
	12,
	252,
	218,
	13,
	257,
	217,
	14,
	256,
	217,
	15,
	260,
	218,
	16,
	268,
	217,
	17,
	273,
	217,
	18,
	272,
	217,
	19,
	275,
	218,
	20,
	279,
	217,
	21,
	277,
	216,
	22,
	276,
	215,
	23,
	277,
	213,
	24,
	277,
	211,
	25,
	277,
	208,
	26,
	277,
	207,
	27,
	277,
	210,
	28,
	0
};

ExtCompact sc36_gallagher_ext = {
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
	96+32,	// spWidth_xx
	SP_COL_GALLAGHER36,	// spColour
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

Compact sc36_gallagher = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	202,	// xcood
	238,	// ycood
	58*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	0,	// mouseRel_y
	217-201,	// mouseSize_x
	268-238,	// mouseSize_y
	SC36_GALLAGHER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC36_GALLAGHER_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc36_gallagher_ext
};

uint16 sc36_col_up3[] = {
	103*64,
	272,
	213,
	0,
	272,
	213,
	1,
	272,
	213,
	2,
	272,
	213,
	3,
	272,
	213,
	4,
	272,
	213,
	5,
	272,
	213,
	6,
	272,
	213,
	7,
	272,
	213,
	8,
	272,
	213,
	9,
	272,
	213,
	10,
	272,
	213,
	11,
	272,
	213,
	12,
	272,
	213,
	13,
	272,
	213,
	14,
	272,
	213,
	15,
	0
};

uint16 sc36_band_anim[] = {
	31*64,
	276,
	164,
	0,
	276,
	164,
	1,
	276,
	164,
	2,
	276,
	164,
	3,
	276,
	164,
	4,
	276,
	164,
	5,
	276,
	164,
	6,
	276,
	164,
	7,
	276,
	164,
	8,
	276,
	164,
	9,
	276,
	164,
	10,
	276,
	164,
	11,
	276,
	164,
	12,
	276,
	164,
	13,
	276,
	164,
	14,
	276,
	164,
	15,
	276,
	164,
	16,
	276,
	164,
	17,
	276,
	164,
	18,
	276,
	164,
	19,
	0
};

uint16 sc36_col_kick[] = {
	105*64,
	249,
	211,
	0,
	248,
	211,
	1,
	245,
	211,
	2,
	243,
	212,
	3,
	241,
	212,
	4,
	242,
	212,
	5,
	247,
	211,
	6,
	241,
	213,
	7,
	243,
	212,
	8,
	247,
	211,
	9,
	249,
	211,
	0,
	249,
	211,
	0,
	249,
	211,
	0,
	0
};

Compact sc36_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	sc36_floor_table,	// getToTable
	184,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-184,	// mouseSize_x
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

uint32 *sc36_walk_grid = (uint32*)sc36_fast_list;

uint16 sc36_dooropen[] = {
	144*64,
	375,
	183,
	0,
	375,
	183,
	1,
	375,
	183,
	2,
	375,
	183,
	3,
	375,
	183,
	4,
	375,
	183,
	5,
	0
};

uint16 bar_get_drink[] = {
	32*64,
	391,
	240,
	0,
	391,
	240,
	1,
	391,
	240,
	1,
	391,
	240,
	2,
	391,
	240,
	3,
	391,
	240,
	3,
	391,
	240,
	3,
	391,
	240,
	4,
	391,
	240,
	5,
	391,
	240,
	6,
	391,
	240,
	6,
	391,
	240,
	6,
	391,
	240,
	7,
	391,
	240,
	7,
	391,
	240,
	7,
	391,
	240,
	7,
	391,
	240,
	7,
	391,
	240,
	7,
	391,
	240,
	7,
	391,
	240,
	6,
	391,
	240,
	5,
	391,
	240,
	4,
	391,
	240,
	8,
	391,
	240,
	8,
	391,
	240,
	8,
	391,
	240,
	8,
	391,
	240,
	9,
	391,
	240,
	0,
	0
};

uint16 sc36_gal_deal[] = {
	58*64,
	202,
	238,
	0,
	202,
	238,
	1,
	202,
	238,
	2,
	202,
	238,
	3,
	202,
	238,
	4,
	202,
	238,
	4,
	202,
	238,
	5,
	202,
	238,
	6,
	202,
	238,
	6,
	0
};

uint16 bar_blink[] = {
	32*64,
	391,
	240,
	10,
	391,
	240,
	11,
	391,
	240,
	0,
	0
};

uint16 sc36_press_plate[] = {
	70*64,
	360,
	186,
	0,
	360,
	186,
	0,
	360,
	186,
	1,
	360,
	186,
	2,
	360,
	186,
	3,
	360,
	186,
	4,
	360,
	186,
	5,
	360,
	186,
	5,
	360,
	186,
	5,
	360,
	186,
	5,
	0
};

uint16 reset_jukebox[] = {
	C_LOGIC,
	L_SCRIPT,
	C_STATUS,
	ST_MOUSE+ST_LOGIC+ST_RECREATE,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC36_JUKEBOX_LOGIC,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 sc36_fast_list[] = {
	12+DISK_13,
	51+DISK_13,
	52+DISK_13,
	53+DISK_13,
	54+DISK_13,
	135+DISK_13,
	IT_SC36_GET_GLASS+DISK_13,
	0
};

uint16 sc36_col_up4[] = {
	104*64,
	277,
	211,
	0,
	278,
	211,
	1,
	277,
	210,
	2,
	277,
	209,
	3,
	278,
	209,
	4,
	278,
	209,
	5,
	278,
	209,
	5,
	277,
	210,
	6,
	277,
	212,
	7,
	278,
	213,
	8,
	279,
	216,
	9,
	279,
	217,
	10,
	279,
	217,
	11,
	271,
	217,
	12,
	271,
	218,
	13,
	270,
	218,
	14,
	264,
	217,
	15,
	255,
	217,
	16,
	255,
	218,
	17,
	253,
	217,
	18,
	247,
	217,
	19,
	239,
	217,
	20,
	239,
	218,
	21,
	239,
	218,
	22,
	233,
	217,
	23,
	223,
	217,
	24,
	223,
	217,
	25,
	222,
	217,
	26,
	222,
	217,
	26,
	222,
	217,
	26,
	222,
	219,
	27,
	221,
	223,
	28,
	219,
	227,
	29,
	220,
	226,
	30,
	0
};

ExtCompact sc36_barman_ext = {
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
	96+32,	// spWidth_xx
	SP_COL_BARMAN36,	// spColour
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

Compact sc36_barman = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	391,	// xcood
	240,	// ycood
	32*64,	// frame
	16619,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	415-391,	// mouseSize_x
	266-240,	// mouseSize_y
	SC36_BARMAN_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC36_BARMAN_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc36_barman_ext
};

Compact sc36_exit_30 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	36,	// screen
	0,	// place
	0,	// getToTable
	171,	// xcood
	230,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	182-171,	// mouseSize_x
	296-230,	// mouseSize_y
	SC36_EXIT_30_ACTION,	// actionScript
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
