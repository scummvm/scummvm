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

#ifndef SKY10COMP_H
#define SKY10COMP_H




namespace Sky {

namespace SkyCompact {

uint16 liyt_4_seq[] = {
	70*64,
	128,
	147,
	0,
	128,
	147,
	0,
	128,
	147,
	0,
	128,
	147,
	0,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	3,
	128,
	147,
	0,
	128,
	147,
	0,
	128,
	147,
	4,
	128,
	147,
	2,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	2,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	1,
	128,
	147,
	2,
	128,
	147,
	2,
	0
};

uint16 litebank_seq[] = {
	71*64,
	226,
	198,
	0,
	226,
	198,
	0,
	226,
	198,
	1,
	226,
	198,
	1,
	226,
	198,
	2,
	226,
	198,
	2,
	226,
	198,
	3,
	226,
	198,
	3,
	226,
	198,
	4,
	226,
	198,
	4,
	226,
	198,
	5,
	226,
	198,
	5,
	226,
	198,
	6,
	226,
	198,
	6,
	226,
	198,
	7,
	226,
	198,
	7,
	226,
	198,
	8,
	226,
	198,
	8,
	226,
	198,
	9,
	226,
	198,
	9,
	226,
	198,
	0,
	226,
	198,
	0,
	226,
	198,
	1,
	226,
	198,
	1,
	226,
	198,
	2,
	226,
	198,
	2,
	226,
	198,
	3,
	226,
	198,
	3,
	226,
	198,
	4,
	226,
	198,
	4,
	226,
	198,
	5,
	226,
	198,
	5,
	226,
	198,
	6,
	226,
	198,
	6,
	226,
	198,
	7,
	226,
	198,
	7,
	226,
	198,
	8,
	226,
	198,
	8,
	226,
	198,
	9,
	226,
	198,
	9,
	226,
	198,
	4,
	226,
	198,
	4,
	0
};

uint16 rs_foster_from_linc[] = {
	C_XCOOD,
	0XCF,
	C_YCOOD,
	0XE1,
	C_FRAME,
	0XF48,
	C_PLACE,
	ID_LINC_10,
	C_SCREEN,
	10,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_AR_PRIORITY+ST_GRID_PLOT+ST_COLLISION,
	65535
};

uint16 stand_10_seq[] = {
	62*64,
	207,
	225,
	0,
	207,
	225,
	1,
	207,
	225,
	2,
	207,
	225,
	3,
	207,
	224,
	4,
	207,
	227,
	5,
	207,
	228,
	6,
	212,
	228,
	7,
	216,
	228,
	8,
	223,
	226,
	9,
	0
};

uint16 pod_up[] = {
	58*64,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	216,
	0,
	204,
	215,
	0,
	204,
	214,
	0,
	204,
	213,
	0,
	204,
	212,
	0,
	204,
	211,
	0,
	204,
	210,
	0,
	204,
	209,
	0,
	204,
	208,
	0,
	204,
	207,
	0,
	204,
	206,
	0,
	204,
	205,
	0,
	204,
	204,
	0,
	204,
	203,
	0,
	204,
	202,
	0,
	204,
	201,
	0,
	204,
	200,
	0,
	204,
	199,
	0,
	204,
	198,
	0,
	204,
	197,
	0,
	204,
	196,
	0,
	204,
	195,
	0,
	204,
	194,
	0,
	204,
	193,
	0,
	204,
	192,
	0,
	204,
	191,
	0,
	204,
	190,
	0,
	204,
	189,
	0,
	204,
	188,
	0,
	204,
	187,
	0,
	204,
	186,
	0,
	204,
	185,
	0,
	204,
	184,
	0,
	204,
	183,
	0,
	204,
	182,
	0,
	204,
	181,
	0,
	0
};

uint16 rs_foster_11_10[] = {
	C_XCOOD,
	128,
	C_YCOOD,
	288,
	C_SCREEN,
	10,
	C_PLACE,
	ID_S10_FLOOR,
	65535
};

uint16 lift_10_close[] = {
	45*64,
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

uint16 s10_floor_table[] = {
	ID_S10_FLOOR,
	RET_OK,
	ID_LEFT_EXIT_10,
	GT_LEFT_EXIT_10,
	0,
	S10_RIGHT_ON,
	ID_LIFT_SLOT_10,
	GT_SLOT_10,
	ID_POD_LIGHT,
	GT_POD_LIGHT,
	ID_LINC_10,
	GT_LINC_10,
	ID_TERMINAL_10,
	GT_TERMINAL_10,
	ID_SCANNER_10,
	GT_SCANNER_10,
	ID_LIFT_10,
	GT_DOOR_10,
	65535
};

uint16 monitor_10_seq[] = {
	63*64,
	322,
	229,
	0,
	322,
	229,
	1,
	322,
	229,
	2,
	322,
	229,
	3,
	322,
	229,
	4,
	322,
	229,
	5,
	322,
	229,
	6,
	322,
	229,
	7,
	322,
	229,
	8,
	322,
	229,
	9,
	322,
	229,
	10,
	322,
	229,
	11,
	322,
	229,
	12,
	322,
	229,
	13,
	322,
	229,
	14,
	322,
	229,
	15,
	322,
	229,
	16,
	0
};

uint16 sit_10_seq[] = {
	61*64,
	210,
	228,
	0,
	207,
	228,
	1,
	207,
	228,
	2,
	207,
	227,
	3,
	207,
	224,
	4,
	207,
	225,
	5,
	207,
	225,
	6,
	207,
	225,
	7,
	207,
	225,
	8,
	207,
	225,
	8,
	0
};

Compact lift_slot_10 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	361,	// xcood
	245,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	366-361,	// mouseSize_x
	258-245,	// mouseSize_y
	SLOT_10_ACTION,	// actionScript
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

uint16 scanline_seq[] = {
	57*64,
	329,
	226,
	0,
	329,
	226,
	1,
	329,
	226,
	2,
	329,
	226,
	3,
	329,
	226,
	4,
	329,
	226,
	5,
	329,
	226,
	6,
	329,
	226,
	7,
	329,
	226,
	8,
	329,
	226,
	9,
	329,
	226,
	10,
	329,
	226,
	11,
	329,
	226,
	12,
	329,
	226,
	13,
	329,
	226,
	14,
	329,
	226,
	15,
	329,
	226,
	16,
	329,
	226,
	17,
	1,
	1,
	16,
	1,
	1,
	15,
	1,
	1,
	14,
	1,
	1,
	13,
	1,
	1,
	12,
	1,
	1,
	11,
	1,
	1,
	10,
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

Compact s10_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	s10_floor_table,	// getToTable
	152,	// xcood
	264,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	(312-264)-1,	// mouseSize_y
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

uint16 rs_foster_10_11[] = {
	C_XCOOD,
	384,
	C_YCOOD,
	272,
	C_SCREEN,
	11,
	C_PLACE,
	ID_S11_FLOOR,
	65535
};

Compact terminal_10 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	400,	// xcood
	265,	// ycood
	0,	// frame
	51,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	26,	// mouseSize_x
	308-265,	// mouseSize_y
	TERMINAL_10_ACTION,	// actionScript
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

uint16 linc_card_seq[] = {
	60*64,
	224,
	224,
	0,
	224,
	224,
	1,
	224,
	224,
	2,
	224,
	224,
	2,
	224,
	224,
	2,
	224,
	224,
	2,
	224,
	224,
	2,
	224,
	224,
	1,
	224,
	224,
	0,
	0
};

uint16 liyt_1_seq[] = {
	64*64,
	413,
	270,
	0,
	413,
	270,
	0,
	413,
	270,
	0,
	413,
	270,
	0,
	413,
	270,
	0,
	413,
	270,
	1,
	413,
	270,
	1,
	413,
	270,
	1,
	413,
	270,
	1,
	413,
	270,
	1,
	0
};

uint16 fsit_talk_seq[] = {
	72*64,
	61*64+8,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	2,
	207,
	225,
	2,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	2,
	207,
	225,
	2,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	2,
	207,
	225,
	2,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	2,
	207,
	225,
	2,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	2,
	207,
	225,
	2,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	3,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	1,
	207,
	225,
	1,
	207,
	225,
	2,
	207,
	225,
	2,
	207,
	225,
	0,
	207,
	225,
	0,
	207,
	225,
	3,
	207,
	225,
	3,
	0
};

Compact scanner_10 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	329,	// xcood
	226,	// ycood
	0,	// frame
	138,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	45,	// mouseRel_y
	25,	// mouseSize_x
	11,	// mouseSize_y
	SCANNER_10_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SCANNER_10_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact pod_light = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	239,	// xcood
	244,	// ycood
	59*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	6,	// mouseSize_y
	POD_LIGHT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	POD_LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact pod = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE+ST_FOREGROUND,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	204,	// xcood
	180,	// ycood
	58*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	366-361,	// mouseSize_x
	258-245,	// mouseSize_y
	SLOT_10_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	POD_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact fake_floor_10 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	356,	// xcood
	271,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
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

uint16 lift_10_open[] = {
	45*64,
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
	1,
	1,
	10,
	0
};

uint16 linc_10_table[] = {
	ID_S10_FLOOR,
	GT_FLOOR_FROM_CHAIR10,
	ID_LEFT_EXIT_10,
	GT_FLOOR_FROM_CHAIR10,
	ID_LIFT_SLOT_10,
	GT_FLOOR_FROM_CHAIR10,
	ID_POD_LIGHT,
	GT_FLOOR_FROM_CHAIR10,
	ID_TERMINAL_10,
	GT_FLOOR_FROM_CHAIR10,
	ID_SCANNER_10,
	GT_FLOOR_FROM_CHAIR10,
	ID_LIFT_10,
	GT_FLOOR_FROM_CHAIR10,
	ID_LINC_10,
	RET_OK,
	65535
};

Compact linc_10 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	linc_10_table,	// getToTable
	203,	// xcood
	222,	// ycood
	58*64,	// frame
	56,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	231-203,	// mouseSize_x
	269-222,	// mouseSize_y
	LINC_10_ACTION,	// actionScript
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

uint16 s10_logic[] = {
	ID_FOSTER,
	ID_SCANNER_10,
	ID_POD,
	ID_POD_LIGHT,
	ID_MONITOR_10,
	ID_LIYT_1,
	ID_LIYT_2,
	ID_LIYT_3,
	ID_LIYT_4,
	ID_LITEBANK,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 liyt_2_seq[] = {
	65*64,
	409,
	274,
	0,
	409,
	274,
	0,
	409,
	274,
	0,
	409,
	274,
	0,
	409,
	274,
	1,
	409,
	274,
	1,
	409,
	274,
	1,
	0
};

uint16 s10_chip_list[] = {
	45+DISK_1,
	46+DISK_1,
	55+DISK_1,
	57+DISK_1,
	58+DISK_1,
	59+DISK_1,
	60+DISK_1,
	61+DISK_1,
	62+DISK_1,
	63+DISK_1,
	64+DISK_1,
	65+DISK_1,
	66+DISK_1,
	70+DISK_1,
	71+DISK_1,
	30+DISK_1,
	31+DISK_1,
	32+DISK_1,
	72+DISK_1,
	0
};

uint16 pod_down[] = {
	58*64,
	204,
	180,
	0,
	204,
	180,
	1,
	204,
	181,
	1,
	204,
	182,
	1,
	204,
	183,
	1,
	204,
	184,
	1,
	204,
	185,
	1,
	204,
	186,
	1,
	204,
	187,
	1,
	204,
	188,
	1,
	204,
	189,
	1,
	204,
	190,
	1,
	204,
	191,
	1,
	204,
	192,
	1,
	204,
	193,
	1,
	204,
	194,
	1,
	204,
	195,
	1,
	204,
	196,
	1,
	204,
	197,
	1,
	204,
	198,
	1,
	204,
	199,
	1,
	204,
	200,
	1,
	204,
	201,
	1,
	204,
	202,
	1,
	204,
	203,
	1,
	204,
	204,
	1,
	204,
	205,
	1,
	204,
	206,
	1,
	204,
	207,
	1,
	204,
	208,
	1,
	204,
	209,
	1,
	204,
	210,
	1,
	204,
	211,
	1,
	204,
	212,
	1,
	204,
	213,
	1,
	204,
	214,
	1,
	204,
	215,
	1,
	204,
	216,
	1,
	204,
	216,
	1,
	204,
	216,
	1,
	0
};

Compact monitor_10 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	239,	// xcood
	244,	// ycood
	59*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	6,	// mouseSize_y
	POD_LIGHT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 liyt_3_seq[] = {
	66*64,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	0,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	1,
	325,
	170,
	0,
	325,
	170,
	0,
	0
};

Compact lift_10 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	358,	// xcood
	217,	// ycood
	45*64,	// frame
	139,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	62,	// mouseSize_y
	DOOR_10_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LIFT_10_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact litebank = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	239,	// xcood
	244,	// ycood
	59*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	6,	// mouseSize_y
	POD_LIGHT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 pod_light_seq[] = {
	59*64,
	239,
	244,
	0,
	239,
	244,
	0,
	239,
	244,
	1,
	239,
	244,
	1,
	239,
	244,
	1,
	239,
	244,
	1,
	239,
	244,
	0,
	239,
	244,
	0,
	239,
	244,
	0,
	239,
	244,
	0,
	0
};

uint32 *grid10 = 0;

uint16 card2_seq[] = {
	55*64,
	328,
	224,
	0,
	328,
	224,
	1,
	328,
	224,
	2,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	3,
	328,
	224,
	2,
	328,
	224,
	1,
	328,
	224,
	0,
	0
};

Compact liyt_3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	239,	// xcood
	244,	// ycood
	59*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	6,	// mouseSize_y
	POD_LIGHT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact liyt_2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	239,	// xcood
	244,	// ycood
	59*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	6,	// mouseSize_y
	POD_LIGHT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s10_pal[] = {
	0,
	1024,
	1540,
	1542,
	2054,
	2054,
	2056,
	2054,
	2056,
	2058,
	2566,
	2056,
	2058,
	2570,
	2570,
	2572,
	3594,
	2058,
	3084,
	3596,
	3084,
	3596,
	3596,
	3596,
	3596,
	4622,
	2060,
	3090,
	3594,
	3598,
	3600,
	3600,
	4112,
	4112,
	5646,
	2574,
	3602,
	4626,
	3088,
	4112,
	4114,
	5136,
	3608,
	4620,
	5136,
	4626,
	5136,
	3602,
	4118,
	6672,
	3088,
	4628,
	5140,
	4628,
	5142,
	5648,
	5650,
	4632,
	5652,
	6162,
	5652,
	5652,
	4630,
	5656,
	5648,
	6676,
	5652,
	6682,
	5140,
	5144,
	6170,
	5142,
	5146,
	5656,
	6168,
	5656,
	6172,
	5656,
	5662,
	6674,
	5144,
	5660,
	7704,
	5654,
	6680,
	6168,
	7704,
	5660,
	6684,
	7192,
	7196,
	6670,
	7704,
	6174,
	7192,
	7704,
	6682,
	7710,
	6682,
	7196,
	6682,
	6174,
	6684,
	8481,
	6682,
	7203,
	8470,
	5662,
	6689,
	8993,
	8474,
	7708,
	7713,
	7710,
	7205,
	8988,
	8988,
	8478,
	7713,
	8993,
	8483,
	8986,
	8990,
	8993,
	10012,
	7198,
	8993,
	10014,
	7710,
	8489,
	9494,
	6179,
	8997,
	10017,
	6181,
	8997,
	10531,
	8481,
	9507,
	11045,
	8481,
	10021,
	10529,
	6695,
	10025,
	10012,
	10021,
	10021,
	10023,
	8489,
	9517,
	10019,
	9001,
	9517,
	11045,
	10535,
	11049,
	10019,
	11561,
	11049,
	12073,
	10025,
	11563,
	11043,
	10539,
	11563,
	12069,
	8493,
	11565,
	11563,
	10031,
	11567,
	12587,
	11053,
	12077,
	13103,
	9519,
	12591,
	12071,
	12079,
	12595,
	13101,
	12083,
	13619,
	14129,
	11573,
	13621,
	14131,
	13111,
	14649,
	15157,
	14139,
	15163,
	15409,
	11067,
	15165,
	9509,
	8487,
	9507,
	11294,
	9518,
	11049,
	10019,
	8489,
	10524,
	2580,
	2056,
	8491,
	9984,
	32,
	2574,
	6152,
	4118,
	3343,
	15369,
	19,
	3855,
	3086,
	3341,
	4369,
	5135,
	4115,
	3854,
	3855,
	3857,
	7195,
	1055,
	1028,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	14129,
	11549,
	7220,
	12586,
	10011,
	6702,
	11300,
	8472,
	5929,
	9759,
	7190,
	5411,
	8474,
	5907,
	4638,
	6933,
	4625,
	3865,
	5648,
	3598,
	3091,
	4108,
	2570,
	2318,
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

uint16 s10_mouse[] = {
	ID_JOEY,
	ID_LEFT_EXIT_10,
	ID_LIFT_SLOT_10,
	ID_LINC_10,
	ID_POD_LIGHT,
	ID_TERMINAL_10,
	ID_SCANNER_10,
	ID_LIFT_10,
	ID_FAKE_FLOOR_10,
	ID_S10_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact liyt_4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	239,	// xcood
	244,	// ycood
	59*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	6,	// mouseSize_y
	POD_LIGHT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact left_exit_10 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	154,	// xcood
	226,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	171-154,	// mouseSize_x
	288-226,	// mouseSize_y
	EL10_ACTION,	// actionScript
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

Compact liyt_1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	10,	// screen
	0,	// place
	0,	// getToTable
	239,	// xcood
	244,	// ycood
	59*64,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	6,	// mouseSize_y
	POD_LIGHT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
