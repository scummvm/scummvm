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

#ifndef SKY29COMP_H
#define SKY29COMP_H


#include "19comp.h"
#include "23comp.h"
#include "25comp.h"
#include "24comp.h"
#include "28comp.h"
#include "26comp.h"
#include "27comp.h"
#include "20comp.h"
#include "22comp.h"
#include "21comp.h"
#include "11comp.h"
#include "10comp.h"


namespace Sky {

namespace SkyCompact {

uint16 rs_joey_7_29[] = {
	C_XCOOD,
	240,
	C_YCOOD,
	240,
	C_SCREEN,
	29,
	C_PLACE,
	ID_S29_FLOOR,
	C_LOGIC,
	L_SCRIPT,
	C_BASE_SUB,
	JOEY_OUT_OF_LIFT,
	C_BASE_SUB+2,
	0,
	C_FRAME,
	136*64+2,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_COLLISION+ST_MOUSE,
	C_DIR,
	DOWN,
	C_MEGA_SET,
	1*NEXT_MEGA_SET,
	C_FRAME,
	136*64+2,
	65535
};

uint16 s29_floor_table[] = {
	ID_S29_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	ID_LIFT_29,
	GT_29_LIFT,
	ID_S29_CARD_SLOT,
	GT_29_CARD_SLOT,
	ID_LIFT_WAIT,
	GT_LIFT_WAIT,
	ID_RIGHT_EXIT_29,
	GT_RIGHT_EXIT_29,
	ID_LEFT_EXIT_29,
	GT_LEFT_EXIT_29,
	1,
	S29_RIGHT_ON,
	3,
	S29_LEFT_ON,
	ID_LIFT_WAIT,
	GT_LIFT_WAIT,
	ID_STD_LEFT_TALK,
	GT_L_TALK_29,
	ID_STD_RIGHT_TALK,
	GT_R_TALK_29,
	ID_CABLE_29,
	GT_CABLE_29,
	65535
};

Compact s29_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	s29_floor_table,	// getToTable
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
	48,	// mouseSize_y
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

uint32 *grid29 = 0;

uint16 card_slot_29_anim[] = {
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
	1,
	1,
	1,
	0,
	0
};

uint16 s29_lift_open[] = {
	17*64,
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

uint16 rs_19_sml_28[] = {
	C_XCOOD,
	192,
	C_YCOOD,
	176,
	C_SCREEN,
	28,
	C_PLACE,
	ID_S28_SML_FLOOR,
	C_FRAME,
	44+13*64,
	65535
};

uint16 sml_joey_list_28[] = {
	222,
	284,
	166,
	186,
	1,
	0
};

Compact s29_card_slot = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	0,	// getToTable
	258,	// xcood
	219,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	8,	// mouseSize_y
	S29_SLOT_ACTION,	// actionScript
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

uint16 s29_sml_mouse[] = {
	ID_JOEY,
	ID_SMALL_R_29,
	ID_SMALL_L_29,
	ID_S29_SML_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 s29_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_LIFT_29,
	ID_LIFT29_LIGHT,
	ID_CABLE_29,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 s29_pal[] = {
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

Compact small_l_29 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	0,	// getToTable
	156,	// xcood
	136,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	179-156,	// mouseSize_x
	188-136,	// mouseSize_y
	L_29_SML_ACTION,	// actionScript
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

uint16 rs_foster_7_29[] = {
	C_XCOOD,
	240,
	C_YCOOD,
	232,
	C_SCREEN,
	29,
	C_FRAME,
	12*64+40,
	C_DIR,
	DOWN,
	C_PLACE,
	ID_S29_FLOOR,
	65535
};

uint16 rs_28_sml_29[] = {
	C_XCOOD,
	168,
	C_YCOOD,
	176,
	C_SCREEN,
	29,
	C_PLACE,
	ID_S29_SML_FLOOR,
	65535
};

uint16 s29_chip_list[] = {
	136+DISK_8,
	137+DISK_8,
	131+DISK_2,
	132+DISK_2,
	86+DISK_8,
	17+DISK_8,
	18+DISK_8,
	20+DISK_8,
	76+DISK_8,
	14+DISK_8,
	15+DISK_8,
	16+DISK_8,
	13,
	0
};

uint16 s29_lift_close[] = {
	17*64,
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

uint16 rs_29_sml_23[] = {
	C_XCOOD,
	184,
	C_YCOOD,
	248,
	C_SCREEN,
	23,
	C_PLACE,
	ID_S23_FLOOR,
	C_FRAME,
	44+12*64,
	65535
};

uint16 rs_lamb_start_bel[] = {
	C_XCOOD,
	360,
	C_YCOOD,
	264,
	C_SCREEN,
	28,
	C_PLACE,
	ID_S28_FLOOR,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,
	65535
};

uint16 s29_mouse[] = {
	ID_JOEY,
	ID_LIFT_29,
	ID_LAMB,
	ID_S29_CARD_SLOT,
	ID_RIGHT_EXIT_29,
	ID_LEFT_EXIT_29,
	ID_CABLE_29,
	ID_S29_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact lift_29 = {
	L_SCRIPT,	// logic
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	0,	// getToTable
	0XE5,	// xcood
	0XC5,	// ycood
	17*64,	// frame
	45+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	268-0XF5,	// mouseSize_x
	236-0XB5,	// mouseSize_y
	LIFT_29_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	S29_LIFT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact lift29_light = {
	L_SCRIPT,	// logic
	ST_RECREATE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	0,	// getToTable
	0XEE,	// xcood
	0XBE,	// ycood
	20*64,	// frame
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

uint16 joey_list_s29[] = {
	156,
	360,
	276,
	304,
	1,
	0
};

Compact right_exit_29 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	0,	// getToTable
	397,	// xcood
	201,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	414-397,	// mouseSize_x
	262-201,	// mouseSize_y
	ER29_ACTION,	// actionScript
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

uint16 rs_foster_23_29[] = {
	C_XCOOD,
	440,
	C_YCOOD,
	264,
	C_SCREEN,
	29,
	C_PLACE,
	ID_S29_FLOOR,
	65535
};

Compact left_exit_29 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	29,	// screen
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
	300-180,	// mouseSize_y
	EL29_ACTION,	// actionScript
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

uint16 lamb_card_29[] = {
	76*64,
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

Compact cable_29 = {
	L_SCRIPT,	// logic
	ST_RECREATE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	0,	// getToTable
	335,	// xcood
	236,	// ycood
	171*64,	// frame
	49,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	16,	// mouseRel_y
	45,	// mouseSize_x
	20,	// mouseSize_y
	CABLE_29_ACTION,	// actionScript
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

uint16 get_cable_29_seq[] = {
	170*64,
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

Compact small_r_29 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	0,	// getToTable
	319,	// xcood
	137,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	348-319,	// mouseSize_x
	186-137,	// mouseSize_y
	R_29_SML_ACTION,	// actionScript
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

uint16 s29_sml_table[] = {
	ID_S29_SML_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	ID_SMALL_R_29,
	GT_SML_R_29,
	ID_SMALL_L_29,
	GT_SML_L_29,
	4,
	SML_RIGHT_29,
	5,
	SML_LEFT_29,
	65535
};

Compact s29_sml_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	29,	// screen
	0,	// place
	s29_sml_table,	// getToTable
	176,	// xcood
	168,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320-176,	// mouseSize_x
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

uint16 rs_23_sml_29[] = {
	C_XCOOD,
	328,
	C_YCOOD,
	168,
	C_SCREEN,
	29,
	C_PLACE,
	ID_S29_SML_FLOOR,
	C_FRAME,
	44+13*64,
	65535
};

uint16 rs_29_sml_28[] = {
	C_XCOOD,
	312,
	C_YCOOD,
	168,
	C_SCREEN,
	28,
	C_PLACE,
	ID_S28_SML_FLOOR,
	65535
};

uint16 sml_joey_list_29[] = {
	202,
	294,
	166,
	186,
	1,
	0
};

void *data_3[] = {
	s29_chip_list,
	&useless_char,
	grid29,
	s29_pal,
	&s29_floor,
	s29_mouse,
	s29_logic,
	rs_foster_7_29,
	&lift_29,
	s29_lift_open,
	s29_lift_close,
	&s29_card_slot,
	card_slot_29_anim,
	&lift29_light,
	rs_joey_7_29,
	joey_list_s29,
	&right_exit_29,
	s23_chip_list,
	s23_fast_list,
	grid23,
	s23_pal,
	&s23_floor,
	s23_mouse,
	s23_logic,
	rs_foster_29_23,
	&left_exit_23,
	rs_foster_23_29,
	&anchor_exit_23,
	s25_chip_list,
	s25_fast_list,
	grid25,
	s25_pal,
	&s25_floor,
	s25_mouse,
	s25_logic,
	rs_foster_23_25,
	&anchor_exit_25,
	rs_foster_25_23,
	joey_list_s25,
	&travel_exit_23,
	s24_fast_list,
	s24_chip_list,
	grid24,
	s24_pal,
	&s24_floor,
	s24_mouse,
	s24_logic,
	rs_foster_23_24,
	&left_exit_24,
	rs_foster_24_23,
	&left_exit_29,
	0,
	s28_chip_list,
	grid28,
	s28_pal,
	&s28_floor,
	s28_mouse,
	s28_logic,
	rs_foster_29_28,
	&right_exit_28,
	rs_foster_28_29,
	&left_exit_28,
	s19_fast_list,
	s19_chip_list,
	grid19,
	s19_pal,
	&s19_floor,
	s19_mouse,
	s19_logic,
	rs_foster_28_19,
	&right_exit_19,
	rs_foster_19_28,
	joey_list_s19,
	&left_exit_19,
	s26_fast_list,
	s26_chip_list,
	grid26,
	s26_pal,
	&s26_floor,
	s26_mouse,
	s26_logic,
	rs_foster_19_26,
	&right_exit_26,
	rs_foster_26_19,
	&dustbin_28,
	rs_joey_start_26,
	joey_list_s26,
	&poster1,
	&poster2,
	&poster3,
	&poster4,
	&plant_26,
	&leaflet,
	&holo,
	&bin_23,
	&sculpture,
	&link_23,
	&wreck_23,
	&london_poster,
	&new_york_poster,
	&mural,
	0,
	&left_exit_26,
	s27_fast_list,
	s27_chip_list,
	grid27,
	s27_pal,
	&s27_floor,
	s27_mouse,
	s27_logic,
	rs_foster_26_27,
	&right_exit_27,
	rs_foster_27_26,
	&chart1,
	&chart2,
	&gas,
	&scanner_27,
	&pidgeons,
	0,
	&burke,
	glove_on_seq,
	glove_off_seq,
	operate_seq,
	gas_seq,
	shake_seq,
	scan_27_seq,
	heart_seq,
	lung_seq,
	kidney_seq,
	&chair_27,
	in_chair_seq,
	out_chair_seq,
	&helmet_cole,
	helmet_down,
	helmet_up,
	helmet_op_seq,
	squirm_seq,
	&medi_comp,
	medi_comp_seq,
	rs_big_burke,
	rs_small_burke,
	&body,
	&anchor,
	anchor_seq,
	anchor_leave_seq,
	anchor_return_seq,
	&hook,
	&statue_25,
	hook_seq,
	&lazer_25,
	lazer_25_seq,
	&spark_25,
	spark_25_seq,
	get_anchor_seq,
	&trevor,
	trevor_relax,
	trevor_unrelax,
	joey_list_s24,
	trevor_scratch,
	&up_exit_28,
	s20_fast_list,
	s20_chip_list,
	grid20,
	s20_pal,
	&s20_floor,
	s20_mouse,
	s20_logic,
	rs_all_28_20,
	&down_exit_20,
	rs_foster_20_28,
	joey_list_20,
	&reich_door_20,
	reich_20_open,
	reich_20_close,
	&reich_slot,
	s22_fast_list,
	s22_chip_list,
	grid22,
	s22_pal,
	&s22_floor,
	s22_mouse,
	s22_logic,
	rs_foster_20_22,
	&right_exit_22,
	rs_foster_22_20,
	usecard_seq,
	&lamb_door_20,
	lamb_20_open,
	lamb_20_close,
	&lamb_slot,
	s21_fast_list,
	s21_chip_list,
	grid21,
	s21_pal,
	&s21_floor,
	s21_mouse,
	s21_logic,
	rs_foster_20_21,
	&left_exit_21,
	rs_foster_21_20,
	&shrub_1,
	&shrub_2,
	&shrub_3,
	rs_joey_start_20,
	&lamb_bed,
	&lamb_tv,
	&fish_tank,
	&fish_poster,
	&pillow,
	remove_pillow,
	replace_pillow,
	fost_remove_pill,
	fost_replace_pill,
	&magazine,
	rs_magazine,
	fish_seq,
	get_magazine_seq,
	&reich_chair,
	&cabinet,
	&cert,
	&reich_picture,
	&fish_food,
	&lambs_books,
	&lambs_chair,
	&dispensor,
	push_dispensor,
	&cat_food,
	cat_food_seq,
	&video,
	&cassette,
	put_video_in,
	take_video_out,
	video_in,
	video_out,
	take_cassette_seq,
	more_cassette_seq,
	&big_pict1,
	&video_screen,
	&big_pict2,
	&big_pict3,
	cat1_seq,
	cat2_seq,
	cat3_seq,
	cat4_seq,
	bend_down,
	watch_film,
	&cat,
	cat_walk_seq,
	cat_eat_seq,
	helga_awake,
	helga_byeee,
	wobble1,
	wobble2,
	&bio_door,
	bio_door_open,
	tank2_seq,
	tank3_seq,
	&sales_chart,
	&gallager_bel,
	gal_look_up,
	gal_look_down,
	trev_get_mag,
	trev_read_mag,
	trev_turn_page,
	mag_out,
	mag_away,
	&fake_floor_22,
	&reich_window,
	&lamb_window,
	&fake_floor_21,
	&inner_lamb_door,
	inner_lamb_open,
	inner_lamb_close,
	get_leaflet,
	return_leaflet,
	mag_back_seq,
	give_mag_seq,
	&ticket,
	ticket_seq,
	get_ticket,
	&globe,
	globe_spin,
	push_globe,
	&inner_reich_door,
	inner_reich_open,
	inner_reich_close,
	&glass_slot,
	glass_slot_open,
	glass_slot_close,
	put_glass_seq,
	hand_in_seq,
	glass_scan,
	hand_op_seq,
	fprint_seq,
	hands_out_seq,
	rs_lamb_28,
	lamb_card_seq2,
	lamb_card_29,
	lamb_sit_seq,
	lamb_sit_up,
	&cable_29,
	get_cable_29_seq,
	0,
	s11_chip_list,
	grid11,
	s11_pal,
	&s11_floor,
	s11_mouse,
	s11_logic,
	rs_foster_1_11,
	fallin_seq,
	fallin2_seq,
	fallin3_seq,
	&cable_fall,
	cable_fall1_seq,
	cable_fall2_seq,
	&cable_fall2,
	&smashed_window,
	smashed_window_seq,
	&bits,
	bits_seq,
	&bits2,
	bits2_seq,
	get_cable_11,
	&spy_11,
	&locker_11,
	locker_11_open,
	locker_11_close,
	&slot_11,
	usecard_11_seq,
	open_11_locker,
	close_11_locker,
	&soccer_1,
	&soccer_2,
	&soccer_3,
	&soccer_4,
	&soccer_5,
	cant_open_locker,
	&slat_1,
	&slat_2,
	&slat_3,
	&slat_4,
	&slat_5,
	&right_exit_11,
	0,
	s10_chip_list,
	grid10,
	s10_pal,
	&s10_floor,
	s10_mouse,
	s10_logic,
	rs_foster_11_10,
	rs_foster_10_11,
	&left_exit_10,
	&lift_10,
	lift_10_open,
	&lift_slot_10,
	card2_seq,
	scanline_seq,
	&scanner_10,
	&pod,
	pod_down,
	pod_up,
	&linc_10,
	&pod_light,
	pod_light_seq,
	linc_card_seq,
	sit_10_seq,
	stand_10_seq,
	&monitor_10,
	monitor_10_seq,
	&liyt_1,
	liyt_1_seq,
	&liyt_2,
	liyt_2_seq,
	&liyt_3,
	liyt_3_seq,
	&liyt_4,
	liyt_4_seq,
	&litebank,
	litebank_seq,
	fsit_talk_seq,
	&terminal_10,
	lift_10_close,
	&fake_floor_10,
	rs_lamb_start_bel,
	0,
	rs_foster_from_linc,
	&small_exit_23,
	&small_r_29,
	&small_l_29,
	&small_r_28,
	&small_l_28,
	&top_right_19,
	&s29_sml_floor,
	rs_23_sml_29,
	s29_sml_mouse,
	rs_29_sml_23,
	&s28_sml_floor,
	rs_29_sml_28,
	s28_sml_mouse,
	rs_28_sml_29,
	rs_28_sml_19,
	rs_19_sml_28,
	sml_joey_list_29,
	sml_joey_list_28,
	&lift_28,
	lift_28_open,
	lift_28_close,
	&slot_28,
	slot_28_anim,
	rs_31_28
};

} // namespace SkyCompact

} // namespace Sky

#endif
