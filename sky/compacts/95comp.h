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

#ifndef SKY95COMP_H
#define SKY95COMP_H




namespace Sky {

namespace SkyCompact {

uint16 fast_list_sc95[] = {
	IT_BLUE_FOSTER+DISK_4,
	IT_LOGOFF+0X8000+DISK_4,
	IT_WINDOW+0X8000+DISK_4,
	IT_INFO_BUTTON+DISK_4,
	IT_LINK_ARROWS+DISK_4,
	IT_LINK_OBJECTS+DISK_4,
	IT_ENTER_TOP+DISK_4+0X8000,
	IT_EXIT_TOP+DISK_4+0X8000,
	IT_SC95_LAYER_0+DISK_4,
	IT_SC95_LAYER_1+DISK_4,
	IT_SC95_GRID_1+DISK_4,
	IT_GUARDIAN+DISK_4,
	IT_WEIGHT+DISK_4+0X8000,
	IT_DOOR_L95F+DISK_4,
	0
};

uint16 weight_anim[] = {
	103*64,
	322,
	136,
	0,
	315,
	136,
	1,
	312,
	178,
	2,
	312,
	175,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	2,
	312,
	176,
	3,
	312,
	176,
	4,
	312,
	176,
	5,
	312,
	176,
	5,
	312,
	176,
	5,
	312,
	176,
	6,
	312,
	176,
	7,
	0
};

uint16 sc95_floor_table[] = {
	ID_SC95_FLOOR,
	RET_OK,
	ID_DOOR_L95,
	GET_TO_DOOR_L95,
	ID_DOOR_R95,
	GET_TO_DOOR_R95,
	ID_DOOR_T95,
	GET_TO_DOOR_T95,
	ID_GUARDIAN,
	GET_TO_GUARDIAN,
	65535
};

uint16 door_r95_anim[] = {
	100*64,
	366,
	136,
	0,
	366,
	136,
	1,
	366,
	136,
	2,
	366,
	136,
	3,
	366,
	136,
	4,
	366,
	136,
	5,
	366,
	136,
	6,
	366,
	136,
	7,
	366,
	136,
	8,
	366,
	136,
	9,
	366,
	136,
	10,
	366,
	136,
	11,
	366,
	136,
	12,
	366,
	136,
	13,
	366,
	136,
	14,
	366,
	136,
	15,
	366,
	136,
	16,
	366,
	136,
	17,
	366,
	136,
	18,
	366,
	136,
	19,
	366,
	136,
	20,
	366,
	136,
	21,
	366,
	136,
	22,
	0
};

uint16 rs_foster_95_96[] = {
	C_SCREEN,
	96,
	C_PLACE,
	ID_SC96_FLOOR,
	C_XCOOD,
	184,
	C_YCOOD,
	256,
	65535
};

Compact door_r95f = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	370,	// xcood
	136,	// ycood
	101*64,	// frame
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
	DOOR_R95F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 pal95[] = {
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

uint16 chip_list_sc95[] = {
	IT_DOOR_L95+DISK_4,
	IT_DOOR_R95+DISK_4,
	IT_DOOR_R95F+DISK_4,
	IT_DOOR_T95+DISK_4,
	IT_DOOR_T95R+DISK_4,
	0
};

uint16 door_l95f_anim[] = {
	57*64,
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
	199,
	136,
	10,
	199,
	136,
	11,
	199,
	136,
	12,
	199,
	136,
	13,
	199,
	136,
	14,
	199,
	136,
	15,
	199,
	136,
	16,
	199,
	136,
	17,
	199,
	136,
	18,
	199,
	136,
	19,
	199,
	136,
	20,
	199,
	136,
	21,
	199,
	136,
	22,
	0
};

uint16 sc95_logic[] = {
	ID_BLUE_FOSTER,
	ID_DOOR_L95,
	ID_DOOR_L95F,
	ID_DOOR_R95,
	ID_DOOR_R95F,
	ID_DOOR_T95,
	ID_DOOR_T95R,
	ID_EYEBALL_90,
	ID_EYEBALL_91,
	ID_GUARDIAN,
	ID_WEIGHT,
	0XFFFF,
	ID_LINC_MENU_LOGIC
};

Compact weight = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	103*64,	// frame
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
	WEIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact guardian = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	324,	// xcood
	221,	// ycood
	102*64,	// frame
	24588,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	14,	// mouseRel_x
	0,	// mouseRel_y
	12,	// mouseSize_x
	50,	// mouseSize_y
	GUARDIAN_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	GUARDIAN_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_95_91[] = {
	C_SCREEN,
	91,
	C_PLACE,
	ID_SC91_FLOOR,
	C_XCOOD,
	288,
	C_YCOOD,
	256,
	C_DIR,
	DOWN,
	65535
};

uint16 sc95_mouse[] = {
	ID_GUARDIAN,
	ID_DOOR_L95,
	ID_DOOR_L95F,
	ID_DOOR_R95,
	ID_DOOR_R95F,
	ID_DOOR_T95,
	ID_SC95_FLOOR,
	0XFFFF,
	ID_LINC_MENU_MOUSE
};

Compact door_r95 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	366,	// xcood
	136,	// ycood
	100*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	26,	// mouseRel_y
	8,	// mouseSize_x
	110,	// mouseSize_y
	DOOR_R95_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_R95_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_95_94[] = {
	C_SCREEN,
	94,
	C_PLACE,
	ID_SC94_FLOOR,
	C_XCOOD,
	216,
	C_YCOOD,
	264,
	C_DIR,
	DOWN,
	65535
};

Compact door_t95 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	276,	// xcood
	136,	// ycood
	52*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	26,	// mouseRel_y
	28,	// mouseSize_x
	95,	// mouseSize_y
	DOOR_T95_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_T95_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_r95f_anim[] = {
	101*64,
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
	370,
	136,
	10,
	370,
	136,
	11,
	370,
	136,
	12,
	370,
	136,
	13,
	370,
	136,
	14,
	370,
	136,
	15,
	370,
	136,
	16,
	370,
	136,
	17,
	370,
	136,
	18,
	370,
	136,
	19,
	370,
	136,
	20,
	370,
	136,
	21,
	370,
	136,
	22,
	0
};

Compact door_l95 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	204,	// xcood
	136,	// ycood
	56*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	26,	// mouseRel_y
	9,	// mouseSize_x
	108,	// mouseSize_y
	DOOR_L95_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L95_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_l95f = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	199,	// xcood
	136,	// ycood
	57*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	(int16) 65526,	// mouseRel_x
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
	DOOR_L95F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_t95_anim[] = {
	52*64,
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
	276,
	136,
	10,
	276,
	136,
	11,
	276,
	136,
	12,
	276,
	136,
	13,
	276,
	136,
	14,
	276,
	136,
	15,
	276,
	136,
	16,
	276,
	136,
	17,
	276,
	136,
	18,
	276,
	136,
	19,
	276,
	136,
	20,
	276,
	136,
	21,
	276,
	136,
	22,
	0
};

uint16 door_t95r_anim[] = {
	53*64,
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
	301,
	136,
	10,
	301,
	136,
	11,
	301,
	136,
	12,
	301,
	136,
	13,
	301,
	136,
	14,
	301,
	136,
	15,
	301,
	136,
	16,
	301,
	136,
	17,
	301,
	136,
	18,
	301,
	136,
	19,
	301,
	136,
	20,
	301,
	136,
	21,
	301,
	136,
	22,
	0
};

uint16 guardian_down[] = {
	102*64,
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

Compact sc95_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	95,	// screen
	0,	// place
	sc95_floor_table,	// getToTable
	153,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	270,	// mouseSize_x
	52,	// mouseSize_y
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

uint16 guardian_up[] = {
	102*64,
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

Compact door_t95r = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	95,	// screen
	0,	// place
	0,	// getToTable
	301,	// xcood
	136,	// ycood
	53*64,	// frame
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
	DOOR_T95R_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_l95_anim[] = {
	56*64,
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
	204,
	136,
	10,
	204,
	136,
	11,
	204,
	136,
	12,
	204,
	136,
	13,
	204,
	136,
	14,
	204,
	136,
	15,
	204,
	136,
	16,
	204,
	136,
	17,
	204,
	136,
	18,
	204,
	136,
	19,
	204,
	136,
	20,
	204,
	136,
	21,
	204,
	136,
	22,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
