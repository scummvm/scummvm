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

#ifndef SKY26COMP_H
#define SKY26COMP_H




namespace Sky {

namespace SkyCompact {

Compact bio_door = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	181,	// xcood
	232,	// ycood
	93*64,	// frame
	181,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	13,	// mouseSize_x
	63,	// mouseSize_y
	BIO_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	BIO_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact plant_26 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	330,	// xcood
	211,	// ycood
	0,	// frame
	12785,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	353-330,	// mouseSize_x
	237-211,	// mouseSize_y
	PLANT_26_ACTION,	// actionScript
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

uint16 s26_floor_table[] = {
	ID_S26_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S26_RIGHT_ON,
	1,
	S26_LEFT_ON,
	ID_RIGHT_EXIT_26,
	GT_RIGHT_EXIT_26,
	ID_LEFT_EXIT_26,
	GT_LEFT_EXIT_26,
	ID_POSTER1,
	GT_POSTER,
	ID_POSTER2,
	GT_POSTER,
	ID_POSTER3,
	GT_POSTER,
	ID_POSTER4,
	GT_POSTER,
	ID_26_PLANT,
	GT_PLANT,
	ID_LEAFLET,
	GT_LEAFLET,
	ID_HOLO,
	GT_HOLO,
	ID_BIO_DOOR,
	GT_BIO_DOOR,
	65535
};

uint16 s26_mouse[] = {
	ID_JOEY,
	ID_BIO_DOOR,
	ID_LEFT_EXIT_26,
	ID_RIGHT_EXIT_26,
	ID_POSTER1,
	ID_POSTER2,
	ID_POSTER3,
	ID_POSTER4,
	ID_26_PLANT,
	ID_LEAFLET,
	ID_HOLO,
	ID_S26_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 s26_pal[] = {
	0,
	2560,
	2312,
	2314,
	3854,
	1800,
	3085,
	4873,
	1289,
	3086,
	4622,
	2570,
	3601,
	5640,
	2059,
	3344,
	3346,
	5901,
	3098,
	4870,
	3342,
	3857,
	3606,
	6927,
	3358,
	6152,
	3599,
	4122,
	4617,
	6673,
	4630,
	7696,
	3088,
	4629,
	6421,
	2836,
	3618,
	4362,
	8978,
	4886,
	4891,
	7956,
	5401,
	7702,
	3093,
	4641,
	8719,
	2579,
	4389,
	4621,
	6426,
	5915,
	7439,
	4885,
	5656,
	10526,
	2322,
	5414,
	6667,
	6679,
	5922,
	8717,
	4886,
	5414,
	5648,
	10008,
	7704,
	7954,
	5912,
	7957,
	6423,
	8985,
	5420,
	6923,
	7706,
	7709,
	7952,
	7193,
	6185,
	9486,
	4122,
	6680,
	10283,
	5145,
	7197,
	6690,
	10011,
	7968,
	11284,
	4632,
	6446,
	9741,
	6172,
	7457,
	10783,
	4380,
	7451,
	12333,
	4378,
	7964,
	9256,
	6432,
	7712,
	11557,
	5404,
	8994,
	10518,
	5663,
	7474,
	10254,
	6943,
	8483,
	12835,
	5149,
	8238,
	9491,
	7972,
	8498,
	10769,
	6691,
	8994,
	13866,
	4639,
	8752,
	9751,
	7208,
	9770,
	13342,
	5411,
	10279,
	14370,
	4900,
	9266,
	13086,
	6437,
	9774,
	10017,
	7724,
	10031,
	10012,
	5935,
	9528,
	15384,
	4646,
	9782,
	11549,
	9515,
	13354,
	14870,
	5673,
	10043,
	13339,
	8748,
	10553,
	11295,
	7476,
	10047,
	14878,
	6445,
	11324,
	11549,
	6199,
	13358,
	13860,
	9519,
	11070,
	11809,
	8247,
	12350,
	12060,
	10294,
	12348,
	13860,
	11314,
	12863,
	12832,
	11319,
	13884,
	15905,
	10035,
	13882,
	13098,
	11835,
	14140,
	15919,
	11064,
	15165,
	16177,
	13629,
	15677,
	16186,
	16191,
	0,
	3840,
	1800,
	2323,
	4613,
	2570,
	2838,
	6664,
	1548,
	3358,
	6664,
	2320,
	3618,
	8714,
	2579,
	4389,
	7437,
	4885,
	4649,
	9737,
	4117,
	5420,
	11787,
	3353,
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
	14851,
	9522,
	10806,
	12828,
	5410,
	6443,
	9485,
	1809,
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
	13375,
	14375,
	7724,
	9781,
	12823,
	2589,
	5679,
	6144,
	787,
	9533,
	13100,
	9244,
	5162,
	8476,
	5390,
	7936,
	0,
	39,
	2865,
	9995,
	1285,
	285,
	4865,
	0,
	13881,
	14634,
	9519,
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

uint16 helga_awake[] = {
	94*64,
	300,
	212,
	0,
	300,
	212,
	1,
	300,
	212,
	2,
	300,
	212,
	3,
	300,
	212,
	4,
	300,
	212,
	5,
	300,
	212,
	6,
	300,
	212,
	7,
	300,
	212,
	8,
	300,
	212,
	7,
	300,
	212,
	9,
	300,
	212,
	7,
	300,
	212,
	10,
	300,
	212,
	10,
	300,
	212,
	7,
	300,
	212,
	10,
	300,
	212,
	10,
	300,
	212,
	11,
	300,
	212,
	7,
	300,
	212,
	12,
	300,
	212,
	10,
	0
};

Compact right_exit_26 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	389,	// xcood
	232,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	398-389,	// mouseSize_x
	298-232,	// mouseSize_y
	ER26_ACTION,	// actionScript
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

uint16 wobble2[] = {
	96*64,
	378,
	217,
	0,
	378,
	217,
	1,
	378,
	217,
	2,
	378,
	217,
	3,
	378,
	217,
	4,
	378,
	217,
	5,
	378,
	217,
	6,
	378,
	217,
	7,
	378,
	217,
	8,
	378,
	217,
	9,
	378,
	217,
	10,
	378,
	217,
	11,
	378,
	217,
	12,
	378,
	217,
	13,
	378,
	217,
	14,
	378,
	217,
	15,
	378,
	217,
	16,
	378,
	217,
	17,
	378,
	217,
	18,
	378,
	217,
	19,
	378,
	217,
	20,
	378,
	217,
	21,
	378,
	217,
	22,
	378,
	217,
	23,
	0
};

uint16 helga_byeee[] = {
	97*64,
	415,
	217,
	0,
	415,
	217,
	1,
	415,
	217,
	2,
	415,
	217,
	1,
	415,
	217,
	3,
	415,
	217,
	1,
	415,
	217,
	4,
	415,
	217,
	5,
	415,
	217,
	6,
	415,
	217,
	7,
	415,
	217,
	8,
	415,
	217,
	9,
	415,
	217,
	10,
	0
};

Compact left_exit_26 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	181,	// xcood
	233,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	192-181,	// mouseSize_x
	295-233,	// mouseSize_y
	EL26_ACTION,	// actionScript
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

uint16 rs_joey_start_26[] = {
	C_XCOOD,
	344,
	C_YCOOD,
	312,
	C_SCREEN,
	26,
	C_PLACE,
	ID_S26_FLOOR,
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
	DOWN,
	65535
};

uint16 bio_door_open[] = {
	93*64,
	181,
	232,
	0,
	181,
	232,
	1,
	181,
	232,
	2,
	181,
	232,
	3,
	181,
	232,
	4,
	0
};

uint16 wobble1[] = {
	95*64,
	340,
	217,
	0,
	340,
	217,
	1,
	340,
	217,
	2,
	340,
	217,
	3,
	340,
	217,
	4,
	340,
	217,
	5,
	340,
	217,
	6,
	340,
	217,
	7,
	340,
	217,
	8,
	340,
	217,
	9,
	340,
	217,
	10,
	340,
	217,
	11,
	340,
	217,
	12,
	340,
	217,
	13,
	340,
	217,
	14,
	340,
	217,
	15,
	340,
	217,
	16,
	340,
	217,
	17,
	340,
	217,
	18,
	340,
	217,
	19,
	340,
	217,
	20,
	340,
	217,
	21,
	0
};

ExtCompact holo_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	DOWN,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96,	// spWidth_xx
	SP_COL_HOLO,	// spColour
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

Compact holo = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	226,	// xcood
	211,	// ycood
	0,	// frame
	12783,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	6,	// mouseRel_x
	32,	// mouseRel_y
	250-231,	// mouseSize_x
	251-244,	// mouseSize_y
	HOLO_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	HELGA_LOGIC,	// baseSub
	0,	// baseSub_off
	&holo_ext
};

Compact poster2 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	297,	// xcood
	184,	// ycood
	0,	// frame
	12741,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	329-297,	// mouseSize_x
	204-184,	// mouseSize_y
	POSTER2_ACTION,	// actionScript
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

uint16 s26_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	86+DISK_9,
	87+DISK_9,
	88+DISK_9,
	0
};

uint32 *grid26 = 0;

uint16 joey_list_s26[] = {
	208,
	360,
	280,
	321,
	1,
	0
};

uint16 return_leaflet[] = {
	87*64,
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

uint16 rs_foster_19_26[] = {
	C_XCOOD,
	416,
	C_YCOOD,
	288,
	C_SCREEN,
	26,
	C_PLACE,
	ID_S26_FLOOR,
	C_FRAME,
	12*64+42,
	65535
};

uint16 get_leaflet[] = {
	87*64,
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
	1,
	1,
	9,
	0
};

Compact poster3 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	262,	// xcood
	208,	// ycood
	0,	// frame
	12741,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	293-262,	// mouseSize_x
	225-208,	// mouseSize_y
	POSTER3_ACTION,	// actionScript
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

Compact leaflet = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	291,	// xcood
	266,	// ycood
	86*64,	// frame
	12784,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	309-291,	// mouseSize_x
	8,	// mouseSize_y
	LEAFLET_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LEAFLET_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s26_chip_list[] = {
	136+DISK_9,
	137+DISK_9,
	93+DISK_9,
	94+DISK_9,
	95+DISK_9,
	96+DISK_9,
	97+DISK_9,
	98+DISK_9,
	99+DISK_9,
	100+DISK_9,
	140+DISK_9,
	141+DISK_9,
	142+DISK_9,
	0
};

uint16 rs_foster_26_19[] = {
	C_XCOOD,
	136,
	C_YCOOD,
	264,
	C_SCREEN,
	19,
	C_PLACE,
	ID_S19_FLOOR,
	65535
};

Compact poster4 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	297,	// xcood
	208,	// ycood
	0,	// frame
	12741,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	329-297,	// mouseSize_x
	225-208,	// mouseSize_y
	POSTER4_ACTION,	// actionScript
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

uint16 s26_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_BURKE,
	ID_HOLO,
	ID_BIO_DOOR,
	ID_LEAFLET,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_LIFT_29,
	0XFFFF,
	ID_MENU_LOGIC
};

Compact poster1 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	0,	// getToTable
	262,	// xcood
	184,	// ycood
	0,	// frame
	12741,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	293-262,	// mouseSize_x
	204-184,	// mouseSize_y
	POSTER1_ACTION,	// actionScript
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

Compact s26_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	26,	// screen
	0,	// place
	s26_floor_table,	// getToTable
	128,	// xcood
	264,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
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

} // namespace SkyCompact

} // namespace Sky

#endif
