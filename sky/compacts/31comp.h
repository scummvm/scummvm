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

#ifndef SKY31COMP_H
#define SKY31COMP_H




namespace Sky {

namespace SkyCompact {

uint16 reset_start_31[] = {
	C_SCREEN,
	31,
	C_PLACE,
	ID_SC31_FLOOR,
	C_XCOOD,
	280,
	C_YCOOD,
	224,
	C_FRAME,
	40+12*64,
	C_DIR,
	DOWN,
	65535
};

uint16 sc31_pull_rope[] = {
	31*64,
	225,
	226,
	0,
	225,
	226,
	1,
	225,
	226,
	1,
	225,
	226,
	2,
	225,
	226,
	3,
	225,
	226,
	4,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	6,
	225,
	226,
	7,
	225,
	226,
	8,
	225,
	226,
	9,
	225,
	226,
	9,
	225,
	226,
	9,
	225,
	226,
	9,
	225,
	226,
	10,
	225,
	226,
	11,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	5,
	225,
	226,
	12,
	225,
	226,
	13,
	225,
	226,
	8,
	225,
	226,
	9,
	0
};

uint16 sc31_guard_move[] = {
	129*64,
	334,
	177,
	0,
	333,
	179,
	1,
	333,
	180,
	2,
	334,
	182,
	3,
	334,
	185,
	4,
	334,
	187,
	5,
	334,
	188,
	6,
	334,
	190,
	7,
	333,
	193,
	8,
	333,
	195,
	1,
	333,
	196,
	2,
	334,
	198,
	3,
	334,
	201,
	4,
	334,
	203,
	5,
	334,
	204,
	6,
	334,
	206,
	7,
	333,
	209,
	8,
	333,
	211,
	1,
	333,
	212,
	2,
	334,
	214,
	3,
	334,
	217,
	4,
	334,
	219,
	5,
	334,
	220,
	6,
	334,
	222,
	7,
	334,
	222,
	9,
	334,
	222,
	10,
	334,
	222,
	10,
	334,
	222,
	10,
	334,
	222,
	10,
	334,
	222,
	10,
	334,
	222,
	11,
	333,
	228,
	12,
	333,
	238,
	13,
	0
};

Compact sc31_exit_39 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	347,	// xcood
	170,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	375-347,	// mouseSize_x
	232-170,	// mouseSize_y
	SC31_EXIT_39_ACTION,	// actionScript
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

uint16 sc31_plank_flick[] = {
	44*64,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
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
	7,
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
	7,
	1,
	1,
	0,
	0
};

uint16 sc31_fast_list[] = {
	12+DISK_12,
	51+DISK_12,
	52+DISK_12,
	53+DISK_12,
	54+DISK_12,
	135+DISK_12,
	IT_SC31_GRID_1+DISK_12,
	IT_SC31_GUARD_TALK+DISK_12,
	IT_SC31_DROP_ROPE+DISK_12,
	0
};

uint16 sc31_dog_swim[] = {
	133*64,
	338,
	286,
	0,
	334,
	285,
	1,
	328,
	285,
	2,
	324,
	285,
	3,
	322,
	286,
	4,
	314,
	288,
	5,
	311,
	290,
	6,
	307,
	295,
	7,
	309,
	299,
	8,
	313,
	301,
	9,
	317,
	305,
	10,
	322,
	309,
	11,
	331,
	312,
	12,
	341,
	310,
	13,
	351,
	307,
	14,
	352,
	304,
	15,
	354,
	302,
	16,
	356,
	300,
	17,
	359,
	299,
	18,
	358,
	296,
	19,
	354,
	293,
	20,
	346,
	289,
	21,
	341,
	288,
	22,
	0
};

uint16 sc31_bisc_drop[] = {
	105*64,
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
	2,
	0
};

uint16 reset_39_31[] = {
	C_SCREEN,
	31,
	C_PLACE,
	ID_SC31_FLOOR,
	C_XCOOD,
	368,
	C_YCOOD,
	224,
	65535
};

uint16 sc31_climb_plank[] = {
	103*64,
	250,
	276,
	0,
	248,
	276,
	1,
	244,
	276,
	2,
	244,
	276,
	3,
	240,
	276,
	4,
	239,
	275,
	5,
	235,
	275,
	6,
	235,
	275,
	7,
	234,
	274,
	8,
	233,
	274,
	9,
	232,
	276,
	10,
	233,
	278,
	11,
	0
};

uint16 sc31_bricks_up[] = {
	45*64,
	205,
	271,
	0,
	205,
	267,
	1,
	205,
	259,
	2,
	205,
	258,
	2,
	205,
	259,
	2,
	205,
	259,
	2,
	205,
	259,
	2,
	205,
	259,
	2,
	205,
	256,
	2,
	205,
	253,
	2,
	205,
	251,
	2,
	205,
	252,
	2,
	205,
	252,
	2,
	205,
	252,
	2,
	205,
	252,
	2,
	205,
	249,
	2,
	205,
	243,
	2,
	205,
	242,
	2,
	205,
	243,
	2,
	0
};

Compact sc31_plank = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	201,	// xcood
	268,	// ycood
	44*64,	// frame
	16605,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	27,	// mouseRel_x
	9,	// mouseRel_y
	258-228,	// mouseSize_x
	285-277,	// mouseSize_y
	SC31_PLANK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC31_PLANK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc31_lift_close[] = {
	86*64,
	266,
	179,
	8,
	266,
	179,
	7,
	266,
	179,
	6,
	266,
	179,
	5,
	266,
	179,
	4,
	266,
	179,
	3,
	266,
	179,
	2,
	266,
	179,
	1,
	266,
	179,
	0,
	0
};

uint16 sc31_get_bricks[] = {
	102*64,
	235,
	242,
	0,
	229,
	245,
	1,
	224,
	251,
	2,
	224,
	252,
	3,
	225,
	252,
	4,
	225,
	252,
	5,
	225,
	252,
	5,
	225,
	252,
	5,
	224,
	251,
	2,
	224,
	251,
	2,
	229,
	245,
	1,
	0
};

uint16 sc31_rope_pulled[] = {
	62*64,
	218,
	228,
	0,
	218,
	228,
	1,
	218,
	228,
	2,
	218,
	228,
	3,
	218,
	228,
	2,
	218,
	228,
	2,
	218,
	228,
	2,
	218,
	228,
	2,
	218,
	228,
	4,
	218,
	228,
	5,
	218,
	228,
	6,
	218,
	228,
	7,
	218,
	228,
	7,
	218,
	228,
	7,
	218,
	228,
	7,
	218,
	228,
	8,
	218,
	228,
	9,
	218,
	228,
	10,
	218,
	228,
	9,
	0
};

uint32 *sc31_walk_grid = (uint32*)sc31_joey_list;

Compact sc31_rope = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	218,	// xcood
	228,	// ycood
	62*64,	// frame
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
	SC31_ROPE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc31_palette[] = {
	0,
	1536,
	1542,
	2055,
	2570,
	1800,
	2823,
	2310,
	2314,
	2828,
	3850,
	1802,
	3341,
	3336,
	3596,
	3851,
	4619,
	2571,
	3596,
	2320,
	5390,
	3080,
	3359,
	1809,
	2840,
	4617,
	3342,
	3855,
	2065,
	8974,
	4106,
	4379,
	2578,
	4108,
	5410,
	3345,
	4371,
	4113,
	5394,
	5137,
	3342,
	6675,
	4363,
	4645,
	2326,
	5133,
	7711,
	2575,
	5137,
	3099,
	10003,
	4635,
	5902,
	4371,
	6163,
	5901,
	5397,
	5654,
	5398,
	2587,
	5646,
	5160,
	6423,
	5658,
	7697,
	3861,
	6678,
	4368,
	9240,
	6165,
	6429,
	5145,
	5157,
	7437,
	5143,
	6171,
	6424,
	3357,
	6416,
	5931,
	6426,
	5921,
	10002,
	4119,
	7964,
	8464,
	5402,
	7187,
	7212,
	3105,
	6942,
	7193,
	7452,
	7449,
	10785,
	4122,
	7458,
	9241,
	5150,
	6951,
	5397,
	11808,
	7967,
	11037,
	5148,
	6958,
	9489,
	6432,
	7976,
	8470,
	6179,
	8479,
	12067,
	5405,
	8488,
	9501,
	7972,
	8242,
	12052,
	5921,
	9256,
	11298,
	6948,
	9513,
	8734,
	10022,
	9517,
	13088,
	5668,
	9010,
	10010,
	9767,
	10282,
	12065,
	6695,
	9779,
	11037,
	9513,
	10790,
	13867,
	6438,
	10544,
	11806,
	8745,
	10550,
	14621,
	6697,
	10802,
	11812,
	9772,
	11818,
	14635,
	7468,
	11573,
	12578,
	9519,
	11828,
	14376,
	9518,
	12089,
	12833,
	11057,
	12092,
	12062,
	12594,
	12854,
	16168,
	8495,
	12853,
	14894,
	9778,
	13118,
	14117,
	11062,
	13882,
	15400,
	8759,
	13882,
	15919,
	10806,
	14392,
	15156,
	11321,
	14654,
	15407,
	13882,
	15679,
	15152,
	14909,
	15934,
	15925,
	15678,
	5140,
	9490,
	8485,
	13107,
	11565,
	10029,
	16191,
	12085,
	10031,
	14135,
	11563,
	8493,
	12593,
	12574,
	5681,
	14137,
	14620,
	8503,
	11567,
	16152,
	12093,
	13111,
	14104,
	10549,
	11053,
	14113,
	11573,
	12595,
	10025,
	7205,
	14137,
	13615,
	6703,
	12595,
	12075,
	10029,
	15167,
	12591,
	8493,
	10027,
	14618,
	8499,
	10025,
	16161,
	12603,
	13111,
	16169,
	11065,
	10027,
	8988,
	7201,
	6170,
	15636,
	10551,
	12601,
	14110,
	12597,
	13625,
	14125,
	11059,
	12597,
	13097,
	10031,
	10541,
	16161,
	11577,
	11057,
	15646,
	11575,
	9515,
	15130,
	8497,
	16191,
	13568,
	9773,
	10289,
	10782,
	5408,
	6435,
	7182,
	2067,
	3349,
	3587,
	264,
	1032,
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
	11826,
	10542,
	9509,
	7202,
	7451,
	5140,
	3351,
	1805,
	1799,
	1799,
	13575,
	10285,
	10036,
	12832,
	6433,
	6187,
	9487,
	2064,
	2591,
	11267,
	2056,
	8210,
	7441,
	6954,
	9494,
	4119,
	3866,
	4864,
	12548,
	13361,
	10281,
	8236,
	8734,
	12336,
	10288,
	13094,
	6687,
	6187,
	8719,
	2324,
	1819,
	2823,
	11573,
	13352,
	8231,
	8498,
	11033,
	3864,
	4133,
	7944,
	778,
	9276,
	13867,
	9500,
	6194,
	11297,
	7186,
	3620,
	6167,
	3848,
	2865,
	9995,
	1285,
	285,
	4865,
	0,
	12344,
	12840,
	7719,
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

Compact sc31_exit_30 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	31,	// screen
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
	290-162,	// mouseSize_y
	SC31_EXIT_30_ACTION,	// actionScript
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

uint16 sc31_put_bisc[] = {
	109*64,
	224,
	240,
	0,
	224,
	240,
	1,
	224,
	240,
	2,
	224,
	240,
	3,
	224,
	240,
	4,
	224,
	240,
	5,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	5,
	224,
	240,
	4,
	224,
	240,
	3,
	224,
	240,
	2,
	224,
	240,
	7,
	224,
	240,
	8,
	224,
	240,
	8,
	224,
	240,
	8,
	224,
	240,
	8,
	224,
	240,
	9,
	224,
	240,
	0,
	0
};

uint16 sc31_bricks_fall[] = {
	46*64,
	205,
	243,
	0,
	205,
	243,
	0,
	205,
	243,
	0,
	205,
	243,
	0,
	205,
	243,
	0,
	205,
	256,
	0,
	205,
	271,
	1,
	205,
	270,
	2,
	205,
	271,
	1,
	205,
	271,
	3,
	205,
	271,
	1,
	205,
	270,
	2,
	205,
	271,
	1,
	205,
	271,
	3,
	205,
	271,
	1,
	205,
	271,
	1,
	0
};

uint16 sc31_plank_raise[] = {
	44*64,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	5,
	1,
	1,
	4,
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
	0,
	0
};

uint16 sc31_bisc_placed[] = {
	107*64,
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
	105*64,
	0
};

uint16 sc31_joey_fall[] = {
	47*64,
	226,
	136,
	0,
	226,
	149,
	1,
	226,
	190,
	2,
	227,
	210,
	3,
	224,
	215,
	4,
	220,
	218,
	5,
	222,
	219,
	6,
	222,
	219,
	7,
	222,
	219,
	8,
	222,
	219,
	9,
	0
};

uint16 sc31_plank_drop[] = {
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
	2,
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
	2,
	1,
	1,
	2,
	1,
	1,
	4,
	1,
	1,
	5,
	0
};

uint16 sc31_lift_open[] = {
	86*64,
	266,
	179,
	0,
	266,
	179,
	1,
	266,
	179,
	2,
	266,
	179,
	3,
	266,
	179,
	4,
	266,
	179,
	5,
	266,
	179,
	6,
	266,
	179,
	7,
	266,
	179,
	8,
	0
};

uint16 sc31_get_board[] = {
	66*64,
	248,
	208,
	0,
	248,
	208,
	1,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	2,
	248,
	208,
	1,
	248,
	208,
	0,
	0
};

uint16 sc31_rope_lower[] = {
	63*64,
	218,
	228,
	0,
	218,
	228,
	0,
	218,
	228,
	1,
	218,
	228,
	2,
	218,
	228,
	3,
	218,
	228,
	4,
	218,
	228,
	5,
	218,
	228,
	5,
	218,
	228,
	6,
	218,
	228,
	7,
	0
};

uint16 sc31_drop_rope[] = {
	43*64,
	225,
	225,
	0,
	225,
	225,
	1,
	225,
	225,
	1,
	225,
	225,
	1,
	225,
	225,
	2,
	225,
	225,
	3,
	225,
	225,
	4,
	225,
	225,
	5,
	225,
	225,
	6,
	225,
	225,
	7,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	8,
	225,
	225,
	9,
	225,
	225,
	9,
	225,
	225,
	9,
	225,
	225,
	10,
	225,
	225,
	10,
	225,
	225,
	11,
	0
};

uint16 sc31_guard_reach[] = {
	130*64,
	328,
	242,
	0,
	328,
	242,
	1,
	328,
	242,
	2,
	328,
	242,
	3,
	328,
	242,
	3,
	328,
	242,
	3,
	328,
	242,
	2,
	328,
	242,
	1,
	328,
	242,
	0,
	1,
	1,
	131*64,
	0
};

uint16 reset_32_31[] = {
	C_SCREEN,
	31,
	C_PLACE,
	ID_SC31_FLOOR,
	C_XCOOD,
	OFF_RIGHT,
	C_YCOOD,
	256,
	65535
};

uint16 sc31_guard_blink[] = {
	110*64,
	328,
	176,
	0,
	328,
	176,
	1,
	328,
	176,
	0,
	1,
	1,
	101*64,
	0
};

uint16 sc31_use_card[] = {
	92*64,
	272,
	184,
	0,
	272,
	184,
	1,
	272,
	184,
	2,
	272,
	184,
	3,
	272,
	184,
	4,
	272,
	184,
	5,
	272,
	184,
	1,
	272,
	184,
	0,
	0
};

uint16 rs_guard_avail[] = {
	C_CURSOR_TEXT,
	544,
	C_MOUSE_CLICK,
	ADVISOR_188,
	C_ACTION_SCRIPT,
	SC31_GUARD_ACTION,
	C_MOUSE_REL_X,
	332-328,
	C_MOUSE_REL_Y,
	244-242,
	C_MOUSE_SIZE_X,
	353-332,
	C_MOUSE_SIZE_Y,
	275-244,
	65535
};

uint16 sc31_bricks_down[] = {
	45*64,
	205,
	243,
	2,
	205,
	243,
	2,
	205,
	249,
	2,
	205,
	252,
	2,
	205,
	253,
	2,
	205,
	256,
	2,
	205,
	259,
	2,
	205,
	259,
	2,
	205,
	267,
	1,
	205,
	271,
	0,
	0
};

uint16 sc31_rope_drop[] = {
	64*64,
	218,
	228,
	0,
	218,
	228,
	0,
	218,
	228,
	0,
	218,
	228,
	0,
	218,
	228,
	0,
	218,
	228,
	1,
	218,
	228,
	2,
	0
};

uint16 sc31_get_plank[] = {
	106*64,
	224,
	240,
	0,
	224,
	240,
	1,
	224,
	240,
	2,
	224,
	240,
	3,
	224,
	240,
	4,
	224,
	240,
	5,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	6,
	224,
	240,
	5,
	224,
	240,
	4,
	224,
	240,
	3,
	224,
	240,
	2,
	224,
	240,
	0,
	0
};

uint16 sc31_floor_table[] = {
	ID_SC31_FLOOR,
	RET_OK,
	ID_SC31_EXIT_30,
	GT_SC31_EXIT_30,
	0,
	SC31_EXIT_30_WALK_ON,
	ID_SC31_EXIT_32,
	GT_SC31_EXIT_32,
	1,
	SC31_EXIT_32_WALK_ON,
	ID_SC31_EXIT_39,
	GT_SC31_EXIT_39,
	3,
	SC31_EXIT_39_WALK_ON,
	ID_SC31_GUARD,
	GT_SC31_GUARD,
	ID_SC31_LIFT,
	GT_SC31_LIFT,
	ID_SC31_LIFT_SLOT,
	GT_SC31_LIFT_SLOT,
	ID_SC31_END_OF_ROPE,
	GT_SC31_END_OF_ROPE,
	ID_SC31_BRICKS,
	GT_SC31_BRICKS,
	ID_SC31_PLANK,
	GT_SC31_PLANK,
	ID_STD_LEFT_TALK,
	GT_SC31_LEFT_TALK,
	ID_STD_RIGHT_TALK,
	GT_SC31_RIGHT_TALK,
	ID_JOEY_PARK,
	GT_DANI_WAIT,
	ID_DANIELLE,
	GT_SC31_DANIELLE,
	ID_SC31_JOEY,
	GT_SC31_JOEY,
	65535
};

Compact sc31_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	sc31_floor_table,	// getToTable
	128,	// xcood
	224,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-128,	// mouseSize_x
	303-224,	// mouseSize_y
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

Compact sc31_end_of_rope = {
	0,	// logic
	ST_MOUSE+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	227,	// xcood
	226,	// ycood
	61*64,	// frame
	16604,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	3,	// mouseRel_y
	228-225,	// mouseSize_x
	270-229,	// mouseSize_y
	SC31_END_OF_ROPE_ACTION,	// actionScript
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

uint16 sc31_dog_fly[] = {
	104*64,
	235,
	246,
	0,
	240,
	207,
	1,
	243,
	169,
	2,
	247,
	139,
	3,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	281,
	136,
	4,
	323,
	136,
	5,
	323,
	136,
	6,
	323,
	168,
	7,
	323,
	209,
	8,
	323,
	250,
	9,
	323,
	259,
	10,
	323,
	259,
	11,
	323,
	259,
	12,
	323,
	259,
	13,
	323,
	259,
	14,
	323,
	259,
	15,
	323,
	259,
	16,
	323,
	259,
	17,
	323,
	259,
	18,
	323,
	259,
	19,
	323,
	259,
	20,
	323,
	259,
	21,
	323,
	259,
	22,
	323,
	259,
	23,
	323,
	259,
	24,
	323,
	259,
	25,
	323,
	259,
	26,
	323,
	259,
	27,
	323,
	259,
	24,
	323,
	259,
	28,
	323,
	259,
	29,
	0
};

Compact sc31_lift = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	266,	// xcood
	179,	// ycood
	67*64,	// frame
	45+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	294-266,	// mouseSize_x
	234-179,	// mouseSize_y
	SC31_LIFT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC31_LIFT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc31_chip_list[] = {
	IT_SC31_LAYER_0+DISK_12,
	IT_SC31_LAYER_1+DISK_12,
	IT_DANIELLE+DISK_12,
	IT_DANI_CONV+DISK_12,
	IT_SPUNKY+DISK_12,
	IT_BARK+DISK_12,
	IT_SNIFF_RIGHT+DISK_12,
	IT_PISS_RIGHT+DISK_12,
	IT_SC31_CLIMB_PLANK+DISK_12,
	IT_SC31_DOG_RISE+DISK_12,
	IT_SC31_DOG_SWIM+DISK_12,
	IT_SC31_LIFT+DISQ_12+0X8000,
	67+DISK_12,
	IT_SC31_HAND+DISK_12,
	IT_SC31_BISCUITS+DISK_12,
	IT_SC31_HOLD_ROPE+DISK_12,
	IT_SC31_PLANK+DISK_12,
	IT_SC31_BRICK_UP+DISK_12,
	IT_SC31_BRICK_FALL+DISK_12,
	IT_SC31_END_OF_ROPE+DISK_12,
	IT_SC31_ROPE_PULLED+DISK_12,
	IT_SC31_ROPE_LOWER+DISK_12,
	IT_SC31_ROPE_DROP+DISK_12,
	IT_SC31_GUARD_BLINK+DISK_12,
	IT_SC31_GUARD_REACH+DISK_12,
	IT_SC31_GUARD_TALK2+DISK_12+0X8000,
	48+DISK_12,
	66+DISK_12+0X8000,
	0
};

uint16 sc31_guard_chat[] = {
	101*64,
	328,
	176,
	0,
	328,
	176,
	0,
	328,
	176,
	1,
	328,
	176,
	1,
	328,
	176,
	2,
	328,
	176,
	2,
	328,
	176,
	3,
	328,
	176,
	3,
	328,
	176,
	2,
	328,
	176,
	2,
	328,
	176,
	0,
	328,
	176,
	0,
	328,
	176,
	4,
	328,
	176,
	4,
	328,
	176,
	3,
	328,
	176,
	3,
	328,
	176,
	0,
	328,
	176,
	0,
	328,
	176,
	1,
	328,
	176,
	1,
	328,
	176,
	2,
	328,
	176,
	2,
	328,
	176,
	3,
	328,
	176,
	3,
	328,
	176,
	5,
	328,
	176,
	5,
	328,
	176,
	3,
	328,
	176,
	3,
	328,
	176,
	0,
	328,
	176,
	0,
	328,
	176,
	2,
	328,
	176,
	2,
	328,
	176,
	4,
	328,
	176,
	4,
	328,
	176,
	1,
	328,
	176,
	1,
	328,
	176,
	2,
	328,
	176,
	2,
	328,
	176,
	0,
	328,
	176,
	0,
	328,
	176,
	4,
	328,
	176,
	4,
	328,
	176,
	2,
	328,
	176,
	2,
	328,
	176,
	3,
	328,
	176,
	3,
	328,
	176,
	5,
	328,
	176,
	5,
	328,
	176,
	0,
	328,
	176,
	0,
	328,
	176,
	1,
	328,
	176,
	1,
	328,
	176,
	4,
	328,
	176,
	4,
	328,
	176,
	2,
	328,
	176,
	2,
	328,
	176,
	3,
	328,
	176,
	3,
	328,
	176,
	5,
	328,
	176,
	5,
	328,
	176,
	0,
	328,
	176,
	0,
	328,
	176,
	4,
	328,
	176,
	4,
	0
};

Compact sc31_biscuits = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	230,	// xcood
	278,	// ycood
	107*64,	// frame
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
	SC31_BISCUITS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 reset_30_31[] = {
	C_SCREEN,
	31,
	C_PLACE,
	ID_SC31_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	248,
	65535
};

Compact sc31_bricks = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	205,	// xcood
	271,	// ycood
	45*64,	// frame
	16607,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	1,	// mouseRel_x
	0,	// mouseRel_y
	227-204,	// mouseSize_x
	284-271,	// mouseSize_y
	SC31_BRICKS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC31_BRICKS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc31_lower_rope[] = {
	32*64,
	225,
	226,
	0,
	225,
	226,
	1,
	225,
	226,
	2,
	225,
	226,
	3,
	225,
	226,
	4,
	225,
	226,
	5,
	225,
	226,
	6,
	225,
	226,
	7,
	225,
	226,
	8,
	225,
	226,
	9,
	225,
	226,
	10,
	225,
	226,
	11,
	225,
	226,
	12,
	225,
	226,
	13,
	225,
	226,
	14,
	225,
	226,
	15,
	0
};

uint16 sc31_bisc_raise[] = {
	105*64,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	2,
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
	0,
	0
};

Compact sc31_at_watcher = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	31,	// screen
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
	SC31_AT_WATCHER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc31_dog_rise[] = {
	132*64,
	359,
	303,
	0,
	360,
	301,
	1,
	360,
	299,
	2,
	362,
	298,
	3,
	361,
	295,
	3,
	356,
	292,
	4,
	348,
	288,
	5,
	343,
	287,
	6,
	341,
	286,
	7,
	0
};

uint16 rs_guard_chat[] = {
	C_MOUSE_CLICK,
	MEGA_CLICK,
	C_ACTION_SCRIPT,
	SC31_GUARD_CHATTING_ACTION,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC31_GUARD_CHATTING_LOGIC,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 sc31_mouse_list[] = {
	ID_SPUNKY,
	ID_DANIELLE,
	ID_SC31_GUARD,
	ID_SC31_END_OF_ROPE,
	ID_SC31_BRICKS,
	ID_SC31_PLANK,
	ID_SC31_LIFT,
	ID_SC31_LIFT_SLOT,
	ID_SC31_EXIT_30,
	ID_SC31_EXIT_32,
	ID_SC31_EXIT_39,
	ID_SC31_JOEY,
	ID_SC31_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc31_joey = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	0,	// frame
	466,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	250-222,	// mouseSize_x
	247-219,	// mouseSize_y
	SC31_JOEY_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC31_JOEY_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc31_lift_slot = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	297,	// xcood
	196,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	302-297,	// mouseSize_x
	209-196,	// mouseSize_y
	SC31_LIFT_SLOT_ACTION,	// actionScript
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

uint16 sc31_logic_list[] = {
	ID_DANIELLE,
	ID_SC31_GUARD,
	ID_SC31_LIFT,
	ID_SC31_END_OF_ROPE,
	ID_SC31_ROPE,
	ID_SC31_BRICKS,
	ID_FOSTER,
	ID_SC31_PLANK,
	ID_SC31_BISCUITS,
	ID_SPUNKY,
	ID_DOG_BARK_THING,
	ID_SC32_LIFT,
	ID_SC31_JOEY,
	16870,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

ExtCompact sc31_guard_ext = {
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
	SP_COL_GUARD31,	// spColour
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

Compact sc31_guard = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	328,	// xcood
	176,	// ycood
	101*64,	// frame
	53,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	6,	// mouseRel_x
	1,	// mouseRel_y
	353-334,	// mouseSize_x
	231-177,	// mouseSize_y
	SC31_GUARD_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC31_GUARD_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc31_guard_ext
};

Compact sc31_exit_32 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	31,	// screen
	0,	// place
	0,	// getToTable
	420,	// xcood
	162,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-420,	// mouseSize_x
	275-162,	// mouseSize_y
	SC31_EXIT_32_ACTION,	// actionScript
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

uint16 sc31_joey_list[] = {
	144,
	431,
	240,
	279,
	1,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
