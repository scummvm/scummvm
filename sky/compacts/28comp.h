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

#ifndef SKY28COMP_H
#define SKY28COMP_H




namespace Sky {

namespace SkyCompact {

uint16 s28_chip_list[] = {
	136+DISK_9,
	137+DISK_9,
	86+DISK_8,
	131+DISK_2,
	132+DISK_2,
	63+DISK_9,
	64+DISK_9,
	65+DISK_9,
	66+DISK_9,
	67+DISK_9,
	13,
	69+DISK_9,
	18+DISK_8,
	0
};

uint16 s28_floor_table[] = {
	ID_S28_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S28_RIGHT_ON,
	1,
	S28_LEFT_ON,
	3,
	S28_UP_ON,
	ID_RIGHT_EXIT_28,
	GT_RIGHT_EXIT_28,
	ID_LEFT_EXIT_28,
	GT_LEFT_EXIT_28,
	ID_UP_EXIT_28,
	GT_UP_EXIT_28,
	ID_DUSTBIN_28,
	GT_DUSTBIN_28,
	ID_STD_LEFT_TALK,
	GT_L_TALK_28,
	ID_STD_RIGHT_TALK,
	GT_R_TALK_28,
	ID_LIFT_28,
	GT_LIFT_28,
	ID_SLOT_28,
	GT_SLOT_28,
	65535
};

uint16 s28_sml_table[] = {
	ID_S28_SML_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	ID_SMALL_R_28,
	GT_SML_R_28,
	ID_SMALL_L_28,
	GT_SML_L_28,
	4,
	SML_RIGHT_28,
	5,
	SML_LEFT_28,
	65535
};

Compact s28_sml_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	s28_sml_table,	// getToTable
	192,	// xcood
	168,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	304-192,	// mouseSize_x
	16,	// mouseSize_y
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

uint16 slot_28_anim[] = {
	18*64,
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
	1,
	1,
	1,
	0,
	0
};

Compact left_exit_28 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	127,	// xcood
	180,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	144-127,	// mouseSize_x
	308-180,	// mouseSize_y
	EL28_ACTION,	// actionScript
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

uint16 rs_foster_28_29[] = {
	C_XCOOD,
	120,
	C_YCOOD,
	272,
	C_SCREEN,
	29,
	C_PLACE,
	ID_S29_FLOOR,
	65535
};

Compact s28_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	s28_floor_table,	// getToTable
	128,	// xcood
	248,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	312-248,	// mouseSize_y
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

uint16 s28_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_REICH_DOOR_20,
	ID_INNER_R_DOOR,
	ID_GALLAGER_BEL,
	ID_LIFT_29,
	ID_LIFT_28,
	ID_CAT,
	ID_CATFOOD,
	0XFFFF,
	ID_MENU_LOGIC
};

Compact small_r_28 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	308,	// xcood
	137,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	339-308,	// mouseSize_x
	186-137,	// mouseSize_y
	R_28_SML_ACTION,	// actionScript
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

uint16 s28_pal[] = {
	0,
	512,
	514,
	1028,
	1540,
	773,
	1797,
	1794,
	1541,
	2568,
	2817,
	1542,
	2056,
	3592,
	1030,
	2059,
	2570,
	780,
	2065,
	3587,
	2570,
	2572,
	3085,
	1294,
	2323,
	4102,
	1803,
	2578,
	3594,
	3852,
	3088,
	3596,
	785,
	2837,
	3592,
	2064,
	2586,
	4099,
	4366,
	4112,
	4619,
	3598,
	3353,
	6150,
	2573,
	4116,
	5643,
	3598,
	4626,
	8461,
	1036,
	5394,
	6918,
	2063,
	3615,
	6407,
	3600,
	4375,
	5138,
	3860,
	4124,
	5388,
	1815,
	5144,
	5135,
	6164,
	4890,
	7698,
	3091,
	4135,
	8709,
	2578,
	5659,
	6671,
	3096,
	5405,
	6162,
	4888,
	4899,
	10767,
	1554,
	5412,
	8458,
	4629,
	6174,
	6672,
	6168,
	5918,
	10261,
	2581,
	5164,
	9224,
	3096,
	5922,
	7957,
	4890,
	5674,
	9997,
	3608,
	7197,
	8729,
	3357,
	6692,
	8468,
	5659,
	7198,
	11037,
	3865,
	7206,
	12050,
	3097,
	6952,
	8981,
	5918,
	6958,
	9231,
	5152,
	7717,
	8474,
	7967,
	7467,
	11797,
	4637,
	8229,
	10525,
	6943,
	7728,
	11023,
	6175,
	8745,
	11286,
	5153,
	8240,
	10770,
	7458,
	9253,
	12833,
	5408,
	8751,
	10776,
	6693,
	9011,
	11283,
	7972,
	9521,
	12566,
	6692,
	9771,
	13347,
	6180,
	9775,
	13343,
	5159,
	10543,
	12827,
	7462,
	10038,
	12567,
	8488,
	10287,
	11300,
	9770,
	10293,
	14107,
	6442,
	10805,
	12831,
	8236,
	11310,
	13608,
	6956,
	11058,
	13607,
	9516,
	11574,
	14369,
	7469,
	12082,
	14631,
	8239,
	12084,
	14122,
	8753,
	12854,
	14376,
	9523,
	13112,
	14379,
	10294,
	13623,
	14640,
	11574,
	0,
	5120,
	2832,
	4124,
	8972,
	3859,
	6170,
	11288,
	2068,
	6687,
	7699,
	7452,
	7715,
	10519,
	5666,
	10543,
	13595,
	7978,
	11317,
	14117,
	8753,
	12854,
	14376,
	11059,
	0,
	8448,
	4629,
	6687,
	10259,
	5403,
	8746,
	12061,
	6178,
	10543,
	12827,
	7462,
	10805,
	13599,
	9516,
	11574,
	14369,
	7469,
	12599,
	13858,
	10290,
	13112,
	14373,
	10294,
	0,
	512,
	514,
	1028,
	1540,
	773,
	1797,
	1794,
	1541,
	2568,
	7681,
	3091,
	5412,
	12298,
	3870,
	9011,
	13331,
	5159,
	10293,
	13595,
	6956,
	12599,
	14626,
	11574,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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

Compact up_exit_28 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	369,	// xcood
	196,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	394-369,	// mouseSize_x
	254-196,	// mouseSize_y
	EU28_ACTION,	// actionScript
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

Compact dustbin_28 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	326,	// xcood
	236,	// ycood
	0,	// frame
	12757,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	339-326,	// mouseSize_x
	253-236,	// mouseSize_y
	DUSTBIN_ACTION,	// actionScript
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

uint32 *grid28 = 0;

uint16 rs_31_28[] = {
	C_XCOOD,
	248,
	C_YCOOD,
	240,
	C_SCREEN,
	28,
	C_FRAME,
	12*64+40,
	C_DIR,
	DOWN,
	C_PLACE,
	ID_S28_FLOOR,
	65535
};

Compact slot_28 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	267,	// xcood
	217,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	8,	// mouseSize_y
	S28_SLOT_ACTION,	// actionScript
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

uint16 s28_mouse[] = {
	ID_JOEY,
	ID_LAMB,
	ID_RIGHT_EXIT_28,
	ID_LEFT_EXIT_28,
	ID_DUSTBIN_28,
	ID_UP_EXIT_28,
	ID_LIFT_28,
	ID_SLOT_28,
	ID_S28_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 lift_28_close[] = {
	69*64,
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

uint16 lift_28_open[] = {
	69*64,
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

Compact right_exit_28 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	429,	// xcood
	189,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	30,	// mouseSize_x
	306-189,	// mouseSize_y
	ER28_ACTION,	// actionScript
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

uint16 rs_lamb_28[] = {
	C_XCOOD,
	376,
	C_YCOOD,
	280,
	C_SCREEN,
	28,
	C_PLACE,
	ID_S28_FLOOR,
	C_LOGIC,
	L_SCRIPT,
	C_MODE,
	0,
	C_BASE_SUB,
	LAMB_BELL_LOGIC,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 rs_foster_29_28[] = {
	C_XCOOD,
	456,
	C_YCOOD,
	288,
	C_SCREEN,
	28,
	C_PLACE,
	ID_S28_FLOOR,
	65535
};

uint16 s28_sml_mouse[] = {
	ID_JOEY,
	ID_SMALL_L_28,
	ID_SMALL_R_28,
	ID_S28_SML_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact lift_28 = {
	L_SCRIPT,	// logic
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	237,	// xcood
	196,	// ycood
	69*64,	// frame
	45+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	268-0XF2,	// mouseSize_x
	236-0XB5,	// mouseSize_y
	LIFT_28_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LIFT_28_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact small_l_28 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	28,	// screen
	0,	// place
	0,	// getToTable
	173,	// xcood
	137,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	200-173,	// mouseSize_x
	186-137,	// mouseSize_y
	L_28_SML_ACTION,	// actionScript
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
