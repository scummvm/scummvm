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

#ifndef SKY96COMP_H
#define SKY96COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc96_mouse[] = {
	ID_CRYSTAL,
	ID_VIRUS,
	ID_DOOR_L96,
	ID_SC96_FLOOR,
	0XFFFF,
	ID_LINC_MENU_MOUSE
};

uint16 door_l96f_anim[] = {
	99*64,
	190,
	136,
	0,
	190,
	136,
	1,
	190,
	136,
	2,
	190,
	136,
	3,
	190,
	136,
	4,
	190,
	136,
	5,
	190,
	136,
	6,
	190,
	136,
	7,
	190,
	136,
	8,
	190,
	136,
	9,
	190,
	136,
	10,
	190,
	136,
	11,
	190,
	136,
	12,
	190,
	136,
	13,
	190,
	136,
	14,
	190,
	136,
	15,
	190,
	136,
	16,
	190,
	136,
	17,
	190,
	136,
	18,
	190,
	136,
	19,
	190,
	136,
	20,
	190,
	136,
	21,
	190,
	136,
	22,
	0
};

uint16 sc96_floor_table[] = {
	ID_SC96_FLOOR,
	RET_OK,
	ID_DOOR_L96,
	GET_TO_DOOR_L96,
	ID_CRYSTAL,
	GET_TO_CRYSTAL,
	ID_VIRUS,
	GET_TO_VIRUS,
	65535
};

Compact sc96_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	96,	// screen
	0,	// place
	sc96_floor_table,	// getToTable
	195,	// xcood
	240,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	181,	// mouseSize_x
	50,	// mouseSize_y
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

uint16 sc96_logic[] = {
	ID_BLUE_FOSTER,
	ID_DOOR_L96,
	ID_DOOR_L96F,
	ID_CRYSTAL,
	ID_VIRUS,
	ID_EYEBALL_90,
	0XFFFF,
	ID_LINC_MENU_LOGIC
};

Compact crystal = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	96,	// screen
	0,	// place
	0,	// getToTable
	261,	// xcood
	227,	// ycood
	106*64,	// frame
	24604,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	4,	// mouseRel_y
	50,	// mouseSize_x
	35,	// mouseSize_y
	CRYSTAL_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	CRYSTAL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_96_95[] = {
	C_SCREEN,
	95,
	C_PLACE,
	ID_SC95_FLOOR,
	C_XCOOD,
	384,
	C_YCOOD,
	264,
	65535
};

uint16 crystal_break[] = {
	107*64,
	261,
	227,
	0,
	261,
	227,
	1,
	261,
	227,
	2,
	261,
	227,
	3,
	261,
	227,
	4,
	261,
	227,
	5,
	261,
	227,
	6,
	261,
	227,
	7,
	261,
	227,
	8,
	261,
	227,
	9,
	261,
	227,
	10,
	0
};

uint16 pal96[] = {
	0,
	3584,
	8960,
	1044,
	7211,
	12812,
	5413,
	11066,
	15390,
	23,
	286,
	1792,
	0,
	274,
	7685,
	1036,
	5120,
	8,
	3104,
	9216,
	20,
	7208,
	15360,
	36,
	4098,
	42,
	0,
	15416,
	10300,
	13360,
	9244,
	6188,
	8220,
	5132,
	2072,
	5132,
	7176,
	12,
	1046,
	4096,
	9216,
	0,
	28,
	5120,
	0,
	8747,
	1044,
	13856,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
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
	16145,
	4415,
	16191,
	16145,
	4415,
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

Compact door_l96f = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_SORT,	// status
	0,	// sync
	96,	// screen
	0,	// place
	0,	// getToTable
	190,	// xcood
	136,	// ycood
	99*64,	// frame
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
	DOOR_L96F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact virus = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	96,	// screen
	0,	// place
	0,	// getToTable
	282,	// xcood
	250,	// ycood
	108*64,	// frame
	24605,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	11,	// mouseSize_x
	11,	// mouseSize_y
	VIRUS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	VIRUS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 crystal_spin[] = {
	106*64,
	261,
	227,
	0,
	261,
	227,
	1,
	261,
	227,
	2,
	261,
	227,
	3,
	261,
	227,
	4,
	261,
	227,
	5,
	261,
	227,
	6,
	261,
	227,
	7,
	261,
	227,
	8,
	261,
	227,
	9,
	0
};

uint16 virus_spin[] = {
	108*64,
	282,
	250,
	0,
	282,
	250,
	1,
	282,
	250,
	2,
	282,
	250,
	3,
	282,
	250,
	4,
	282,
	250,
	5,
	282,
	250,
	6,
	282,
	250,
	7,
	282,
	250,
	8,
	282,
	250,
	9,
	0
};

Compact door_l96 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	96,	// screen
	0,	// place
	0,	// getToTable
	198,	// xcood
	136,	// ycood
	98*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	26,	// mouseRel_y
	7,	// mouseSize_x
	102,	// mouseSize_y
	DOOR_L96_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L96_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 chip_list_sc96[] = {
	IT_DOOR_L96+DISK_4,
	IT_DOOR_L96F+DISK_4,
	IT_CRYSTAL_SPIN+DISK_4,
	IT_CRYSTAL_BREAK+DISK_4,
	IT_VIRUS_SPIN+DISK_4,
	IT_GET_VIRUS+DISK_4,
	0
};

uint16 get_virus[] = {
	110*64,
	282,
	250,
	0,
	282,
	250,
	1,
	282,
	250,
	2,
	282,
	250,
	3,
	282,
	250,
	4,
	282,
	250,
	5,
	282,
	250,
	6,
	0
};

uint16 door_l96_anim[] = {
	98*64,
	198,
	136,
	0,
	198,
	136,
	1,
	198,
	136,
	2,
	198,
	136,
	3,
	198,
	136,
	4,
	198,
	136,
	5,
	198,
	136,
	6,
	198,
	136,
	7,
	198,
	136,
	8,
	198,
	136,
	9,
	198,
	136,
	10,
	198,
	136,
	11,
	198,
	136,
	12,
	198,
	136,
	13,
	198,
	136,
	14,
	198,
	136,
	15,
	198,
	136,
	16,
	198,
	136,
	17,
	198,
	136,
	18,
	198,
	136,
	19,
	198,
	136,
	20,
	198,
	136,
	21,
	198,
	136,
	22,
	0
};

uint16 fast_list_sc96[] = {
	IT_BLUE_FOSTER+DISK_4,
	IT_LOGOFF+0X8000+DISK_4,
	IT_WINDOW+0X8000+DISK_4,
	IT_INFO_BUTTON+DISK_4,
	IT_LINK_ARROWS+DISK_4,
	IT_LINK_OBJECTS+DISK_4,
	IT_CROUCH_RIGHT+DISK_4,
	IT_SC96_LAYER_0+DISK_4,
	IT_SC96_LAYER_1+DISK_4,
	IT_SC96_GRID_1+DISK_4,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
