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

#ifndef SKY93COMP_H
#define SKY93COMP_H




namespace Sky {

namespace SkyCompact {

uint16 get_adjust[] = {
	63*64,
	370,
	284,
	0,
	370,
	284,
	1,
	370,
	284,
	2,
	370,
	284,
	3,
	370,
	284,
	4,
	0
};

Compact door_l93 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	93,	// screen
	0,	// place
	0,	// getToTable
	205,	// xcood
	136,	// ycood
	133*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	26,	// mouseRel_y
	9,	// mouseSize_x
	109,	// mouseSize_y
	DOOR_L93_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L93_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc93_mouse[] = {
	ID_PERSONA,
	ID_ADJUST_BOOK,
	ID_DOOR_L93,
	ID_DOOR_L93F,
	ID_SC93_FLOOR,
	0XFFFF,
	ID_LINC_MENU_MOUSE
};

Compact adjust_book = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	93,	// screen
	0,	// place
	0,	// getToTable
	370,	// xcood
	284,	// ycood
	63*64,	// frame
	24596,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	15,	// mouseSize_x
	15,	// mouseSize_y
	ADJUST_BOOK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	ADJUST_BOOK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc93_floor_table[] = {
	ID_SC93_FLOOR,
	RET_OK,
	ID_DOOR_L93,
	GET_TO_DOOR_L93,
	ID_PERSONA,
	GET_TO_PERSONA,
	ID_ADJUST_BOOK,
	GET_TO_ADJUST_BOOK,
	65535
};

uint16 sc93_logic[] = {
	ID_BLUE_FOSTER,
	ID_PERSONA,
	ID_ADJUST_BOOK,
	ID_DOOR_L93,
	ID_DOOR_L93F,
	ID_EYEBALL_90,
	0XFFFF,
	ID_LINC_MENU_LOGIC
};

Compact door_l93f = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	93,	// screen
	0,	// place
	0,	// getToTable
	200,	// xcood
	136,	// ycood
	134*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	(int16) 65517,	// mouseRel_x
	263-136,	// mouseRel_y
	20,	// mouseSize_x
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
	DOOR_L93F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_93_92[] = {
	C_SCREEN,
	92,
	C_PLACE,
	ID_SLAB9,
	C_XCOOD,
	336,
	C_YCOOD,
	248,
	C_DIR,
	DOWN,
	65535
};

Compact sc93_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	93,	// screen
	0,	// place
	sc93_floor_table,	// getToTable
	153,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	269,	// mouseSize_x
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

uint16 chip_list_sc93[] = {
	IT_PERSONA+DISK_4,
	IT_ADJUST_BOOK+DISK_4,
	IT_DOOR_L93+DISK_4,
	IT_DOOR_L93F+DISK_4,
	0
};

uint16 door_l93f_anim[] = {
	134*64,
	200,
	136,
	0,
	200,
	136,
	1,
	200,
	136,
	2,
	200,
	136,
	3,
	200,
	136,
	4,
	200,
	136,
	5,
	200,
	136,
	6,
	200,
	136,
	7,
	200,
	136,
	8,
	200,
	136,
	9,
	200,
	136,
	10,
	200,
	136,
	11,
	200,
	136,
	12,
	200,
	136,
	13,
	200,
	136,
	14,
	200,
	136,
	15,
	200,
	136,
	16,
	200,
	136,
	17,
	200,
	136,
	18,
	200,
	136,
	19,
	200,
	136,
	20,
	200,
	136,
	21,
	200,
	136,
	22,
	0
};

uint16 get_persona[] = {
	51*64,
	342,
	250,
	0,
	342,
	250,
	1,
	342,
	250,
	2,
	342,
	250,
	3,
	342,
	250,
	4,
	342,
	250,
	5,
	0
};

uint16 fast_list_sc93[] = {
	IT_BLUE_FOSTER+DISK_4,
	IT_LOGOFF+0X8000+DISK_4,
	IT_WINDOW+0X8000+DISK_4,
	IT_INFO_BUTTON+DISK_4,
	IT_LINK_ARROWS+DISK_4,
	IT_LINK_OBJECTS+DISK_4,
	IT_CROUCH_RIGHT+DISK_4,
	IT_SC93_LAYER_0+DISK_4,
	IT_SC93_LAYER_1+DISK_4,
	IT_SC93_GRID_1+DISK_4,
	0
};

uint16 door_l93_anim[] = {
	133*64,
	205,
	136,
	0,
	205,
	136,
	1,
	205,
	136,
	2,
	205,
	136,
	3,
	205,
	136,
	4,
	205,
	136,
	5,
	205,
	136,
	6,
	205,
	136,
	7,
	205,
	136,
	8,
	205,
	136,
	9,
	205,
	136,
	10,
	205,
	136,
	11,
	205,
	136,
	12,
	205,
	136,
	13,
	205,
	136,
	14,
	205,
	136,
	15,
	205,
	136,
	16,
	205,
	136,
	17,
	205,
	136,
	18,
	205,
	136,
	19,
	205,
	136,
	20,
	205,
	136,
	21,
	205,
	136,
	22,
	0
};

Compact persona = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	93,	// screen
	0,	// place
	0,	// getToTable
	342,	// xcood
	250,	// ycood
	51*64,	// frame
	24594,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	16,	// mouseSize_x
	22,	// mouseSize_y
	PERSONA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	PERSONA_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 pal93[] = {
	0,
	3584,
	8960,
	1044,
	7211,
	12812,
	5413,
	11066,
	15390,
	45,
	10257,
	3840,
	33,
	7436,
	2560,
	1550,
	5888,
	10,
	3616,
	9472,
	20,
	7211,
	15360,
	39,
	25,
	8,
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
	5658,
	1792,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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

} // namespace SkyCompact

} // namespace Sky

#endif
