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

#ifndef SKY32COMP_H
#define SKY32COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc32_gardening1[] = {
	58*64,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	7,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	7,
	272,
	168,
	6,
	272,
	168,
	6,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	11,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	7,
	272,
	168,
	6,
	1,
	1,
	5,
	0
};

uint16 reset_spunky_32[] = {
	C_SCREEN,
	32,
	C_PLACE,
	ID_SC32_FLOOR,
	C_XCOOD,
	240,
	C_YCOOD,
	288,
	C_FRAME,
	32+71*64,
	C_MOUSE_CLICK,
	MEGA_CLICK,
	C_ACTION_SCRIPT,
	MEGA_ACTION,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SPUNKY_LOGIC,
	C_BASE_SUB+2,
	0,
	C_DIR,
	DOWN,
	65535
};

uint16 sc32_vinc_anim[] = {
	56*64,
	218,
	188,
	0,
	218,
	189,
	1,
	218,
	189,
	2,
	218,
	188,
	3,
	219,
	189,
	4,
	219,
	189,
	5,
	218,
	188,
	6,
	218,
	189,
	7,
	218,
	189,
	1,
	218,
	188,
	8,
	219,
	189,
	5,
	219,
	189,
	5,
	218,
	189,
	7,
	218,
	188,
	8,
	218,
	188,
	9,
	218,
	188,
	9,
	218,
	188,
	3,
	219,
	189,
	5,
	218,
	188,
	10,
	219,
	189,
	5,
	219,
	189,
	4,
	219,
	189,
	5,
	219,
	189,
	4,
	219,
	189,
	5,
	218,
	188,
	10,
	218,
	188,
	6,
	218,
	189,
	7,
	218,
	189,
	1,
	218,
	189,
	2,
	218,
	188,
	11,
	218,
	188,
	3,
	218,
	188,
	0,
	0
};

uint16 reset_38_32[] = {
	C_SCREEN,
	32,
	C_PLACE,
	ID_SC32_FLOOR,
	C_XCOOD,
	400,
	C_YCOOD,
	224,
	C_FRAME,
	40+12*64,
	C_DIR,
	DOWN,
	65535
};

Compact sc32_plant_2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	336,	// xcood
	196,	// ycood
	0,	// frame
	16624,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	349-336,	// mouseSize_x
	225-196,	// mouseSize_y
	SC32_PLANT_2_ACTION,	// actionScript
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

uint16 sc32_use_card[] = {
	93*64,
	360,
	240,
	0,
	360,
	240,
	1,
	360,
	240,
	2,
	360,
	240,
	2,
	360,
	240,
	2,
	360,
	240,
	2,
	360,
	240,
	2,
	360,
	240,
	1,
	360,
	240,
	0,
	0
};

ExtCompact sc32_vincent_ext = {
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
	SP_COL_VINCENT32,	// spColour
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

Compact sc32_vincent = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	218,	// xcood
	188,	// ycood
	56*64,	// frame
	16457,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	238-218,	// mouseSize_x
	225-188,	// mouseSize_y
	SC32_VINCENT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC32_VINCENT_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc32_vincent_ext
};

uint32 *sc32_walk_grid = (uint32*)sc32_joey_list;

uint16 sc32_gard_turn_d[] = {
	59*64,
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
	0
};

uint16 sc32_lift_open[] = {
	100*64,
	378,
	175,
	14,
	378,
	175,
	13,
	378,
	175,
	12,
	378,
	175,
	11,
	378,
	175,
	10,
	378,
	175,
	9,
	378,
	175,
	8,
	378,
	175,
	7,
	378,
	175,
	6,
	378,
	175,
	5,
	378,
	175,
	4,
	378,
	175,
	3,
	378,
	175,
	2,
	378,
	175,
	1,
	378,
	175,
	0,
	0,
	0
};

uint16 sc32_lift_close[] = {
	100*64,
	378,
	175,
	0,
	378,
	175,
	1,
	378,
	175,
	2,
	378,
	175,
	3,
	378,
	175,
	4,
	378,
	175,
	5,
	378,
	175,
	6,
	378,
	175,
	7,
	378,
	175,
	8,
	378,
	175,
	9,
	378,
	175,
	10,
	378,
	175,
	11,
	378,
	175,
	12,
	378,
	175,
	13,
	378,
	175,
	14,
	0
};

uint16 sc32_mouse_list[] = {
	ID_SPUNKY,
	ID_DANIELLE,
	ID_SC32_VINCENT,
	ID_SC32_GARDENER,
	ID_SC32_TERMINAL,
	ID_SC32_BUZZER,
	ID_SC32_PLANT_1,
	ID_SC32_PLANT_2,
	ID_SC32_PLANT_3,
	ID_SC32_EXIT_31,
	ID_SC32_EXIT_33,
	ID_SC32_LIFT,
	ID_SC32_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc32_exit_31 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	32,	// screen
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
	245-162,	// mouseSize_y
	SC32_EXIT_31_ACTION,	// actionScript
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

uint16 sc32_use_com[] = {
	94*64,
	200,
	232,
	0,
	200,
	232,
	1,
	200,
	232,
	2,
	200,
	232,
	3,
	200,
	232,
	3,
	200,
	232,
	3,
	200,
	232,
	3,
	200,
	232,
	4,
	200,
	232,
	0,
	0
};

uint16 sc32_gardener_dn[] = {
	58*64,
	272,
	168,
	0,
	272,
	168,
	1,
	272,
	168,
	2,
	272,
	168,
	3,
	272,
	168,
	4,
	272,
	168,
	5,
	0
};

ExtCompact sc32_buzzer_ext = {
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
	SP_COL_BUZZER32,	// spColour
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

Compact sc32_buzzer = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	359,	// xcood
	175,	// ycood
	68*64,	// frame
	16623,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	365-357,	// mouseSize_x
	187-173,	// mouseSize_y
	SC32_BUZZER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC32_BUZZER_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc32_buzzer_ext
};

uint16 sc32_floor_table[] = {
	ID_SC32_FLOOR,
	RET_OK,
	ID_SC32_EXIT_31,
	GT_SC32_EXIT_31,
	0,
	SC32_EXIT_31_WALK_ON,
	ID_SC32_EXIT_33,
	GT_SC32_EXIT_33,
	1,
	SC32_EXIT_33_WALK_ON,
	ID_SC32_LIFT,
	GT_SC32_LIFT,
	ID_SC32_TERMINAL,
	GT_SC32_TERMINAL,
	ID_SC32_BUZZER,
	GT_SC32_BUZZER,
	ID_SC32_PLANT_1,
	GT_SC32_PLANT_1,
	ID_SC32_PLANT_2,
	GT_SC32_PLANT_2,
	ID_SC32_PLANT_3,
	GT_SC32_PLANT_3,
	ID_SC32_VINCENT,
	GT_SC32_VINCENT,
	ID_SC32_GARDENER,
	GT_SC32_GARDENER,
	ID_STD_LEFT_TALK,
	GT_SC32_LEFT_TALK,
	ID_STD_RIGHT_TALK,
	GT_SC32_RIGHT_TALK,
	ID_JOEY_PARK,
	GT_DANI_WAIT,
	ID_DANIELLE,
	GT_SC32_DANIELLE_AT_LIFT,
	65535
};

Compact sc32_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	sc32_floor_table,	// getToTable
	128,	// xcood
	216,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-128,	// mouseSize_x
	327-216,	// mouseSize_y
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

uint16 sc32_chip_list[] = {
	IT_SC32_LAYER_0+DISK_12,
	IT_SC32_LAYER_1+DISK_12,
	IT_SC32_LAYER_2+DISK_12,
	IT_DANIELLE+DISK_12,
	IT_DANI_CONV+DISK_12,
	IT_SPUNKY+DISK_12,
	IT_SNIFF_RIGHT+DISK_12,
	IT_PISS_RIGHT+DISK_12,
	IT_SC32_LIFT+DISK_12,
	IT_SC32_USE_CARD+DISK_12+0X8000,
	IT_SC32_USE_COM+DISK_12+0X8000,
	IT_SC32_VINCENT+DISK_12,
	IT_SC32_VINC_TALK+DISK_12,
	IT_SC32_GARDENER+DISK_12,
	IT_SC32_GARD_TURN+DISK_12+0X8000,
	IT_SC32_GARDEN_TALK+DISK_12+0X8000,
	68+DISK_12,
	0
};

uint16 sc32_palette[] = {
	0,
	1536,
	1798,
	2568,
	1798,
	2569,
	3337,
	2825,
	3083,
	3080,
	3092,
	2317,
	4108,
	2056,
	7692,
	3342,
	4878,
	2060,
	4110,
	6160,
	2317,
	4877,
	5132,
	2831,
	3594,
	4131,
	2323,
	4115,
	3855,
	5138,
	4363,
	7200,
	2574,
	4883,
	5906,
	2579,
	4880,
	4377,
	3095,
	4634,
	3085,
	9748,
	6165,
	4875,
	5653,
	4383,
	5899,
	4884,
	4131,
	3338,
	10516,
	5650,
	6172,
	3863,
	5654,
	5398,
	6424,
	5903,
	8484,
	3604,
	6172,
	6160,
	4634,
	5903,
	6444,
	3355,
	6421,
	6430,
	7192,
	6175,
	6163,
	6427,
	6942,
	6925,
	5658,
	6429,
	6938,
	4381,
	6674,
	10795,
	3093,
	7191,
	10018,
	3863,
	6946,
	9493,
	4635,
	7455,
	10777,
	4122,
	7709,
	10270,
	5404,
	6446,
	9231,
	6174,
	7974,
	8208,
	8478,
	7714,
	5660,
	12064,
	7468,
	7443,
	8738,
	7216,
	9746,
	5410,
	8480,
	7206,
	10019,
	8234,
	9238,
	7202,
	8487,
	8985,
	8481,
	7982,
	12567,
	5152,
	9256,
	8476,
	10277,
	9764,
	10273,
	8740,
	8751,
	9754,
	9766,
	8755,
	11544,
	7206,
	10026,
	9247,
	11049,
	9522,
	11547,
	8743,
	9526,
	12312,
	7975,
	11048,
	13354,
	7720,
	10796,
	13608,
	6697,
	10799,
	12837,
	8746,
	10552,
	10525,
	11822,
	11317,
	12063,
	10285,
	11319,
	14883,
	8236,
	12077,
	14639,
	7214,
	12084,
	11562,
	13105,
	12087,
	15143,
	7985,
	12857,
	12324,
	12852,
	12605,
	14627,
	10548,
	13117,
	12839,
	13878,
	13881,
	15406,
	10807,
	14393,
	16182,
	11066,
	14910,
	15409,
	13627,
	15418,
	16185,
	12350,
	15676,
	16189,
	14910,
	0,
	63,
	0,
	514,
	1282,
	1285,
	2056,
	3080,
	3084,
	3855,
	4623,
	4626,
	5397,
	6165,
	6168,
	7196,
	7964,
	7967,
	8738,
	9506,
	9509,
	10280,
	11304,
	11308,
	9238,
	6459,
	16169,
	12066,
	9791,
	16180,
	14639,
	16191,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	63,
	16130,
	512,
	0,
	14336,
	13364,
	11569,
	10029,
	9252,
	7203,
	7195,
	4883,
	2837,
	1804,
	770,
	519,
	14851,
	8495,
	10293,
	12056,
	4128,
	6439,
	8714,
	1299,
	2843,
	11264,
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
	11564,
	9781,
	12581,
	7202,
	7470,
	9494,
	4374,
	3610,
	4363,
	11835,
	13856,
	6438,
	7727,
	10258,
	2839,
	4129,
	6919,
	778,
	9533,
	13868,
	9501,
	5936,
	10527,
	6673,
	3107,
	6165,
	3591,
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
	512,
	63,
	16130,
	512,
	63,
	13826,
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

uint16 reset_33_32[] = {
	C_SCREEN,
	32,
	C_PLACE,
	ID_SC32_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	288,
	65535
};

ExtCompact sc32_gardener_ext = {
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
	SP_COL_GARDENER32,	// spColour
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

Compact sc32_gardener = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	272,	// xcood
	168,	// ycood
	5+58*64,	// frame
	16616,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	1,	// mouseRel_x
	18,	// mouseRel_y
	292-273,	// mouseSize_x
	220-186,	// mouseSize_y
	SC32_GARDENER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC32_GARDENER_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc32_gardener_ext
};

uint16 sc32_fast_list[] = {
	12+DISK_12,
	51+DISK_12,
	52+DISK_12,
	53+DISK_12,
	54+DISK_12,
	135+DISK_12,
	IT_SC32_GRID_1+DISK_12,
	IT_SC32_GRID_2+DISK_12,
	0
};

uint16 sc32_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_DOG_BARK_THING,
	ID_SC32_VINCENT,
	ID_SC32_GARDENER,
	ID_SC32_LIFT,
	ID_SC32_BUZZER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 reset_dani_32[] = {
	C_SCREEN,
	32,
	C_PLACE,
	ID_SC32_FLOOR,
	C_XCOOD,
	232,
	C_YCOOD,
	232,
	C_FRAME,
	32+55*64,
	C_MOUSE_CLICK,
	MEGA_CLICK,
	C_MOUSE_REL_X,
	65528,
	C_MOUSE_REL_Y,
	65493,
	C_MOUSE_SIZE_X,
	14,
	C_MOUSE_SIZE_Y,
	50,
	C_ACTION_SCRIPT,
	MEGA_ACTION,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	DANIELLE_LOGIC,
	C_BASE_SUB+2,
	0,
	C_DIR,
	DOWN,
	C_MEGA_SET,
	0,
	65535
};

Compact sc32_plant_3 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	398,	// xcood
	216,	// ycood
	0,	// frame
	16624,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	411-398,	// mouseSize_x
	243-216,	// mouseSize_y
	SC32_PLANT_3_ACTION,	// actionScript
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

Compact sc32_terminal = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	303,	// xcood
	181,	// ycood
	0,	// frame
	51,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	328-303,	// mouseSize_x
	226-181,	// mouseSize_y
	SC32_TERMINAL_ACTION,	// actionScript
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

uint16 sc32_gardener_up[] = {
	58*64,
	272,
	168,
	5,
	272,
	168,
	4,
	272,
	168,
	3,
	272,
	168,
	2,
	272,
	168,
	0,
	0
};

Compact sc32_plant_1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	263,	// xcood
	188,	// ycood
	0,	// frame
	16624,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	292-263,	// mouseSize_x
	217-188,	// mouseSize_y
	SC32_PLANT_1_ACTION,	// actionScript
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

uint16 sc32_gard_turn_u[] = {
	59*64,
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

uint16 sc32_joey_list[] = {
	144,
	431,
	224,
	327,
	1,
	376,
	431,
	224,
	239,
	0,
	128,
	287,
	248,
	279,
	0,
	0
};

uint16 sc32_gardening2[] = {
	58*64,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	272,
	168,
	13,
	272,
	168,
	14,
	272,
	168,
	5,
	272,
	168,
	6,
	272,
	168,
	7,
	272,
	168,
	8,
	272,
	168,
	8,
	272,
	168,
	9,
	272,
	168,
	10,
	272,
	168,
	11,
	272,
	168,
	12,
	1,
	1,
	5,
	0
};

uint16 reset_31_32[] = {
	C_SCREEN,
	32,
	C_PLACE,
	ID_SC32_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	224,
	65535
};

Compact sc32_lift = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	378,	// xcood
	175,	// ycood
	14+100*64,	// frame
	45+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	398-376,	// mouseSize_x
	232-173,	// mouseSize_y
	SC32_LIFT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC32_LIFT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc32_exit_33 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	32,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	260,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	140-128,	// mouseSize_x
	327-260,	// mouseSize_y
	SC32_EXIT_33_ACTION,	// actionScript
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
