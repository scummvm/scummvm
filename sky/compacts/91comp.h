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

#ifndef SKY91COMP_H
#define SKY91COMP_H




namespace Sky {

namespace SkyCompact {

Compact decomp_obj = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	244,	// xcood
	281,	// ycood
	48*64,	// frame
	24586,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	20,	// mouseSize_x
	15,	// mouseSize_y
	DECOMP_OBJ_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DECOMP_OBJ_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_r91 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	370,	// xcood
	136,	// ycood
	111*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	26,	// mouseRel_y
	8,	// mouseSize_x
	109,	// mouseSize_y
	DOOR_R91_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_R91_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc91_mouse[] = {
	ID_BAG_91,
	ID_DECOMP_OBJ,
	ID_DECRYPT_OBJ,
	ID_EYEBALL_91,
	ID_DOOR_L91,
	ID_DOOR_L91F,
	ID_DOOR_R91,
	ID_DOOR_R91F,
	ID_DOOR_T91,
	ID_SC91_FLOOR,
	0XFFFF,
	ID_LINC_MENU_MOUSE
};

uint16 get_decomp[] = {
	48*64,
	244,
	281,
	0,
	244,
	281,
	1,
	244,
	281,
	2,
	244,
	281,
	3,
	244,
	281,
	4,
	244,
	281,
	5,
	244,
	281,
	6,
	244,
	281,
	7,
	0
};

uint16 eye_91_table[] = {
	4,
	4,
	4,
	4,
	4,
	4,
	5,
	5,
	6,
	6,
	6,
	6,
	7,
	7,
	8,
	8,
	9,
	9,
	10,
	10,
	11,
	11,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	4,
	4,
	4,
	4,
	4,
	4,
	5,
	5,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	8,
	8,
	9,
	9,
	10,
	10,
	10,
	11,
	11,
	11,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	4,
	4,
	4,
	4,
	4,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	8,
	8,
	8,
	9,
	9,
	10,
	10,
	11,
	11,
	11,
	11,
	11,
	11,
	12,
	12,
	12,
	4,
	4,
	4,
	4,
	5,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	7,
	8,
	8,
	8,
	9,
	9,
	10,
	10,
	10,
	10,
	11,
	11,
	11,
	11,
	11,
	11,
	4,
	4,
	4,
	5,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	7,
	8,
	8,
	8,
	8,
	9,
	9,
	10,
	10,
	10,
	10,
	10,
	11,
	11,
	11,
	4,
	4,
	5,
	5,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	7,
	7,
	7,
	8,
	8,
	8,
	8,
	9,
	9,
	9,
	10,
	10,
	10,
	10,
	10
};

Compact door_l91f = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	199,	// xcood
	136,	// ycood
	261*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	(int16) 65522,	// mouseRel_x
	263-136,	// mouseRel_y
	15,	// mouseSize_x
	10,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L91F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_r91f_anim[] = {
	112*64,
	374,
	136,
	0,
	374,
	136,
	1,
	374,
	136,
	2,
	374,
	136,
	3,
	374,
	136,
	4,
	374,
	136,
	5,
	374,
	136,
	6,
	374,
	136,
	7,
	374,
	136,
	8,
	374,
	136,
	9,
	0
};

Compact report_book = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	359,	// xcood
	277,	// ycood
	95*64,	// frame
	24596,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	15,	// mouseSize_y
	REPORT_BOOK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	REPORT_BOOK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 get_report[] = {
	95*64,
	359,
	277,
	0,
	359,
	277,
	1,
	359,
	277,
	1,
	359,
	277,
	2,
	359,
	277,
	3,
	359,
	277,
	4,
	359,
	277,
	5,
	359,
	277,
	6,
	359,
	277,
	7,
	0
};

uint16 chip_list_sc91[] = {
	IT_BAG_91+DISK_4,
	IT_DECOMP_OBJ+DISK_4,
	IT_DECRYPT_OBJ+DISK_4,
	IT_DOOR_L91+DISK_4,
	IT_DOOR_L91F+DISK_4,
	IT_DOOR_R91+DISK_4,
	IT_DOOR_R91F+DISK_4,
	IT_DOOR_T91+DISK_4,
	IT_DOOR_T91R+DISK_4,
	IT_EYE91_ZAP+DISK_4,
	IT_FOST_DIE91+DISK_4+0X8000,
	0
};

uint16 sc91_floor_table[] = {
	ID_SC91_FLOOR,
	RET_OK,
	ID_DOOR_L91,
	GET_TO_DOOR_L91,
	ID_DOOR_R91,
	GET_TO_DOOR_R91,
	ID_DOOR_T91,
	GET_TO_DOOR_T91,
	ID_BAG_91,
	GET_TO_BAG_91,
	ID_DECOMP_OBJ,
	GET_TO_DECOMP_OBJ,
	ID_DECRYPT_OBJ,
	GET_TO_DECRYPT_OBJ,
	ID_REPORT_BOOK,
	GET_TO_REPORT_BOOK,
	ID_EYEBALL_91,
	GET_TO_EYEBALL_91,
	65535
};

Compact sc91_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	91,	// screen
	0,	// place
	sc91_floor_table,	// getToTable
	153,	// xcood
	254,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	269,	// mouseSize_x
	53,	// mouseSize_y
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

Compact bag_91 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	315,	// xcood
	277,	// ycood
	47*64,	// frame
	24584,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	28,	// mouseSize_x
	15,	// mouseSize_y
	BAG_91_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	BAG_91_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_r91_anim[] = {
	111*64,
	370,
	136,
	0,
	370,
	136,
	1,
	370,
	136,
	2,
	370,
	136,
	3,
	370,
	136,
	4,
	370,
	136,
	5,
	370,
	136,
	6,
	370,
	136,
	7,
	370,
	136,
	8,
	370,
	136,
	9,
	0
};

uint16 rs_foster_91_95[] = {
	C_SCREEN,
	95,
	C_PLACE,
	ID_SC95_FLOOR,
	C_XCOOD,
	184,
	C_YCOOD,
	264,
	C_FRAME,
	46+182*64,
	C_DIR,
	3,
	65535
};

uint16 door_l91_anim[] = {
	260*64,
	204,
	136,
	0,
	204,
	136,
	1,
	204,
	136,
	2,
	204,
	136,
	3,
	204,
	136,
	4,
	204,
	136,
	5,
	204,
	136,
	6,
	204,
	136,
	7,
	204,
	136,
	8,
	204,
	136,
	9,
	0
};

uint16 pal91[] = {
	0,
	3584,
	8960,
	1044,
	7211,
	12812,
	5413,
	11066,
	15390,
	0,
	52,
	11520,
	0,
	39,
	7936,
	1550,
	5888,
	10,
	3616,
	9472,
	20,
	7211,
	15360,
	39,
	0,
	25,
	0,
	15928,
	11071,
	13873,
	10015,
	6188,
	8990,
	5135,
	2587,
	5903,
	8458,
	783,
	1303,
	4096,
	12032,
	0,
	30,
	5632,
	0,
	6183,
	26,
	5120,
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

uint16 get_decrypt[] = {
	131*64,
	277,
	281,
	0,
	277,
	281,
	1,
	277,
	281,
	1,
	277,
	281,
	2,
	277,
	281,
	3,
	277,
	281,
	4,
	277,
	281,
	5,
	277,
	281,
	6,
	0
};

Compact door_t91 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	276,	// xcood
	136,	// ycood
	31*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	26,	// mouseRel_y
	28,	// mouseSize_x
	95,	// mouseSize_y
	DOOR_T91_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_T91_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_l91 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	204,	// xcood
	136,	// ycood
	260*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	26,	// mouseRel_y
	9,	// mouseSize_x
	109,	// mouseSize_y
	DOOR_L91_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L91_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_91_92[] = {
	C_SCREEN,
	92,
	C_PLACE,
	ID_SLAB1,
	C_XCOOD,
	128,
	C_YCOOD,
	304,
	65535
};

uint16 fost_die91[] = {
	116*64,
	306,
	158,
	0,
	306,
	158,
	1,
	306,
	158,
	2,
	306,
	158,
	3,
	306,
	158,
	4,
	306,
	158,
	5,
	306,
	158,
	6,
	306,
	158,
	5,
	306,
	158,
	7,
	306,
	158,
	8,
	306,
	158,
	9,
	306,
	158,
	10,
	306,
	158,
	11,
	306,
	158,
	12,
	306,
	158,
	13,
	306,
	158,
	14,
	306,
	158,
	15,
	306,
	158,
	16,
	306,
	158,
	17,
	306,
	158,
	18,
	306,
	158,
	19,
	306,
	158,
	20,
	306,
	158,
	21,
	0
};

uint16 rs_foster_91_90[] = {
	C_SCREEN,
	90,
	C_PLACE,
	ID_SC90_FLOOR,
	C_XCOOD,
	384,
	C_YCOOD,
	256,
	65535
};

uint16 sc91_logic[] = {
	ID_BLUE_FOSTER,
	ID_BAG_91,
	ID_DECOMP_OBJ,
	ID_DECRYPT_OBJ,
	ID_DOOR_L91,
	ID_DOOR_L91F,
	ID_DOOR_R91,
	ID_DOOR_R91F,
	ID_DOOR_T91,
	ID_DOOR_T91R,
	ID_EYEBALL_90,
	ID_EYEBALL_91,
	0XFFFF,
	ID_LINC_MENU_LOGIC
};

Compact eyeball_91 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	232,	// xcood
	222,	// ycood
	6+91*64,	// frame
	24587,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	31,	// mouseSize_x
	29,	// mouseSize_y
	EYEBALL_91_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	EYEBALL_91_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 fast_list_sc91[] = {
	IT_BLUE_FOSTER+DISK_4,
	IT_LOGOFF+0X8000+DISK_4,
	IT_WINDOW+0X8000+DISK_4,
	IT_INFO_BUTTON+DISK_4,
	IT_LINK_ARROWS+DISK_4,
	IT_LINK_OBJECTS+DISK_4,
	IT_ENTER_TOP+DISK_4+0X8000,
	IT_EXIT_TOP+DISK_4+0X8000,
	IT_CROUCH_LEFT+DISK_4+0X8000,
	IT_CROUCH_RIGHT+DISK_4,
	IT_CROUCH_DOWN+DISK_4+0X8000,
	IT_EYEBALL+DISK_4,
	IT_GET_EYE+DISK_4+0X8000,
	IT_BLIND_EYE+DISK_4,
	IT_SEE_EYE+DISK_4,
	IT_SC91_LAYER_0+DISK_4,
	IT_SC91_LAYER_1+DISK_4,
	IT_SC91_GRID_1+DISK_4,
	0
};

Compact door_t91r = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	301,	// xcood
	136,	// ycood
	32*64,	// frame
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
	DOOR_T91R_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_l91f_anim[] = {
	261*64,
	199,
	136,
	0,
	199,
	136,
	1,
	199,
	136,
	2,
	199,
	136,
	3,
	199,
	136,
	4,
	199,
	136,
	5,
	199,
	136,
	6,
	199,
	136,
	7,
	199,
	136,
	8,
	199,
	136,
	9,
	0
};

Compact decrypt_obj = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	277,	// xcood
	281,	// ycood
	131*64,	// frame
	24585,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	25,	// mouseSize_x
	12,	// mouseSize_y
	DECRYPT_OBJ_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DECRYPT_OBJ_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_t91_anim[] = {
	31*64,
	276,
	136,
	0,
	276,
	136,
	1,
	276,
	136,
	2,
	276,
	136,
	3,
	276,
	136,
	4,
	276,
	136,
	5,
	276,
	136,
	6,
	276,
	136,
	7,
	276,
	136,
	8,
	276,
	136,
	9,
	0
};

uint16 door_t91r_anim[] = {
	32*64,
	301,
	136,
	0,
	301,
	136,
	1,
	301,
	136,
	2,
	301,
	136,
	3,
	301,
	136,
	4,
	301,
	136,
	5,
	301,
	136,
	6,
	301,
	136,
	7,
	301,
	136,
	8,
	301,
	136,
	9,
	0
};

uint16 eye91_zap_anim[] = {
	114*64,
	222,
	206,
	0,
	222,
	206,
	1,
	222,
	206,
	2,
	222,
	206,
	3,
	222,
	206,
	4,
	0
};

Compact door_r91f = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	91,	// screen
	0,	// place
	0,	// getToTable
	374,	// xcood
	136,	// ycood
	112*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	263-136,	// mouseRel_y
	30,	// mouseSize_x
	10,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_R91F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
