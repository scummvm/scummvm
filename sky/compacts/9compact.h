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

#ifndef SKY9COMPACT_H
#define SKY9COMPACT_H




namespace Sky {

namespace SkyCompact {

uint16 s9_floor_table[] = {
	ID_S9_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	42,
	S9_WALK_ON,
	ID_DAD,
	GET_TO_DAD,
	ID_SON,
	GET_TO_SON,
	ID_LEFT_EXIT_S9,
	GET_TO_L_EXIT_S9,
	ID_LOBBY_DOOR,
	GET_TO_LOBBY_DOOR,
	ID_SCANNER,
	GET_TO_SCANNER,
	ID_LOBBY_SLOT,
	GET_TO_SCANNER,
	ID_LINC_S9,
	GT_LINC_S9,
	65535
};

Compact fake_floor_9 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	9,	// screen
	0,	// place
	s9_floor_table,	// getToTable
	386,	// xcood
	291,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	314-291,	// mouseSize_y
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

uint16 son_work_seq[] = {
	103*64,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	1,
	223,
	240,
	1,
	223,
	240,
	1,
	223,
	240,
	2,
	223,
	240,
	2,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	5,
	223,
	240,
	5,
	223,
	240,
	5,
	223,
	240,
	5,
	223,
	240,
	5,
	223,
	240,
	5,
	223,
	240,
	5,
	223,
	240,
	5,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	1,
	223,
	240,
	1,
	223,
	240,
	1,
	223,
	240,
	2,
	223,
	240,
	2,
	223,
	240,
	2,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	3,
	223,
	240,
	1,
	223,
	240,
	1,
	223,
	240,
	1,
	223,
	240,
	2,
	223,
	240,
	2,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	0,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	4,
	223,
	240,
	4,
	0
};

ExtCompact son_ext = {
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
	SP_COL_SON,	// spColour
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

Compact son = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	0XDF,	// xcood
	0XF0,	// ycood
	104*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	32,	// mouseSize_x
	16,	// mouseSize_y
	SON_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	SON_LOGIC,	// baseSub
	0,	// baseSub_off
	&son_ext
};

uint16 scandie_seq[] = {
	112*64,
	349,
	250,
	0,
	349,
	250,
	1,
	349,
	250,
	2,
	349,
	250,
	3,
	349,
	250,
	4,
	349,
	250,
	5,
	349,
	250,
	6,
	349,
	250,
	7,
	349,
	250,
	8,
	349,
	250,
	9,
	349,
	250,
	10,
	349,
	250,
	11,
	349,
	250,
	12,
	349,
	250,
	13,
	349,
	250,
	14,
	349,
	250,
	15,
	349,
	250,
	16,
	349,
	250,
	17,
	349,
	250,
	18,
	349,
	250,
	18,
	349,
	250,
	19,
	349,
	250,
	19,
	349,
	250,
	20,
	349,
	250,
	21,
	349,
	250,
	22,
	349,
	250,
	23,
	349,
	250,
	24,
	349,
	250,
	25,
	349,
	250,
	26,
	349,
	250,
	27,
	349,
	250,
	28,
	349,
	250,
	29,
	349,
	250,
	30,
	349,
	250,
	31,
	349,
	250,
	32,
	349,
	250,
	33,
	349,
	250,
	34,
	349,
	250,
	35,
	349,
	250,
	36,
	349,
	250,
	37,
	349,
	250,
	38,
	349,
	250,
	39,
	0
};

uint16 lobby_down_seq[] = {
	95*64,
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

uint16 s9_mouse[] = {
	ID_JOEY,
	ID_MONITOR,
	ID_LEFT_EXIT_S9,
	ID_LOBBY_DOOR,
	ID_SCANNER,
	ID_LOBBY_SLOT,
	ID_DAD,
	ID_SON,
	ID_FAKE_FLOOR_9,
	ID_LINC_S9,
	ID_S9_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 scan_seq[] = {
	107*64,
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
	1,
	1,
	10,
	1,
	1,
	11,
	1,
	1,
	12,
	1,
	1,
	13,
	1,
	1,
	14,
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

uint16 dad_work_seq[] = {
	96*64,
	270,
	230,
	0,
	270,
	230,
	0,
	270,
	230,
	1,
	270,
	230,
	1,
	270,
	230,
	2,
	270,
	230,
	2,
	270,
	230,
	3,
	270,
	230,
	3,
	270,
	230,
	4,
	270,
	230,
	4,
	270,
	230,
	5,
	270,
	230,
	5,
	270,
	230,
	6,
	270,
	230,
	6,
	270,
	230,
	7,
	270,
	230,
	7,
	270,
	230,
	6,
	270,
	230,
	6,
	270,
	230,
	7,
	270,
	230,
	7,
	270,
	230,
	6,
	270,
	230,
	6,
	270,
	230,
	5,
	270,
	230,
	5,
	270,
	230,
	4,
	270,
	230,
	4,
	270,
	230,
	3,
	270,
	230,
	3,
	0
};

Compact linc_s9 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	162,	// xcood
	284,	// ycood
	0,	// frame
	51,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	180-162,	// mouseSize_x
	322-284,	// mouseSize_y
	LINC_S9_ACTION,	// actionScript
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

uint16 dad_work3_seq[] = {
	98*64,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	1,
	342,
	230,
	2,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	1,
	342,
	230,
	2,
	342,
	230,
	0,
	342,
	230,
	0,
	342,
	230,
	0,
	0
};

ExtCompact dad_ext = {
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
	SP_COL_DAD,	// spColour
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

Compact dad = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	0X10E,	// xcood
	0XE6,	// ycood
	96*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	32,	// mouseSize_x
	16,	// mouseSize_y
	DAD_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	DAD_LOGIC,	// baseSub
	0,	// baseSub_off
	&dad_ext
};

uint16 dad_work2_seq[] = {
	97*64,
	307,
	230,
	0,
	307,
	230,
	0,
	307,
	230,
	1,
	307,
	230,
	1,
	307,
	230,
	1,
	307,
	230,
	1,
	307,
	230,
	1,
	307,
	230,
	0,
	307,
	230,
	2,
	307,
	230,
	2,
	307,
	230,
	3,
	307,
	230,
	3,
	307,
	230,
	4,
	307,
	230,
	5,
	307,
	230,
	5,
	307,
	230,
	6,
	307,
	230,
	6,
	307,
	230,
	7,
	307,
	230,
	7,
	307,
	230,
	8,
	307,
	230,
	8,
	307,
	230,
	7,
	307,
	230,
	7,
	307,
	230,
	6,
	307,
	230,
	6,
	307,
	230,
	5,
	307,
	230,
	5,
	307,
	230,
	9,
	307,
	230,
	3,
	307,
	230,
	3,
	0
};

Compact s9_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	9,	// screen
	0,	// place
	s9_floor_table,	// getToTable
	128,	// xcood
	288,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	37,	// mouseSize_y
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

Compact left_exit_s9 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	175,	// xcood
	244,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	304-244,	// mouseSize_y
	EL9_ACTION,	// actionScript
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

uint16 fans_seq[] = {
	93*64,
	188,
	199,
	0,
	188,
	199,
	1,
	188,
	199,
	2,
	188,
	199,
	3,
	188,
	199,
	4,
	188,
	199,
	5,
	188,
	199,
	6,
	188,
	199,
	7,
	188,
	199,
	8,
	188,
	199,
	9,
	188,
	199,
	10,
	188,
	199,
	11,
	188,
	199,
	12,
	188,
	199,
	13,
	188,
	199,
	14,
	188,
	199,
	15,
	0
};

Compact fans = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	93*64,	// frame
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
	FANS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact lobby_slot = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	384,	// xcood
	272,	// ycood
	0,	// frame
	36+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	12,	// mouseSize_y
	LOBBY_SLOT_ACTION,	// actionScript
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

Compact scanner = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	349,	// xcood
	250,	// ycood
	95*64,	// frame
	138,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	298-250,	// mouseRel_y
	32,	// mouseSize_x
	8,	// mouseSize_y
	SCANNER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SCANNER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 card_in_seq[] = {
	106*64,
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

uint16 lobby_up_seq[] = {
	95*64,
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
	0
};

Compact lobby_door = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_BACKGROUND+ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	9,	// screen
	0,	// place
	0,	// getToTable
	379,	// xcood
	241,	// ycood
	95*64,	// frame
	139,	// cursorText
	LOBBY_DOOR_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	62,	// mouseSize_y
	LOBBY_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOBBY_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_to_9[] = {
	C_XCOOD,
	152,
	C_YCOOD,
	296,
	C_SCREEN,
	9,
	C_PLACE,
	ID_S9_FLOOR,
	65535
};

uint16 rs_foster_10_9[] = {
	C_XCOOD,
	408,
	C_YCOOD,
	296,
	C_SCREEN,
	9,
	C_PLACE,
	ID_S9_FLOOR,
	65535
};

uint16 s9_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_FANS,
	ID_LOBBY_DOOR,
	ID_SCANNER,
	ID_DAD,
	ID_SON,
	ID_MONITOR,
	ID_LAMB,
	ID_LIFT_S7,
	ID_LIFT7_LIGHT,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 s9_chip_list[] = {
	173,
	111,
	93+DISK_2,
	95+DISK_2,
	96+DISK_2,
	97+DISK_2,
	98+DISK_2,
	99+DISK_2,
	106+DISK_2+0X8000,
	107+DISK_2+0X8000,
	110+DISK_2+0X8000,
	112+DISK_2+0X8000,
	74+DISK_2,
	92+DISK_2,
	47+DISK_2,
	48+DISK_2,
	0
};

uint16 s9_pal[] = {
	0,
	1024,
	1028,
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
	512,
	0,
	0,
	514,
	512,
	4,
	1538,
	1026,
	512,
	4,
	1026,
	1026,
	4,
	1540,
	514,
	2054,
	1540,
	516,
	2566,
	1540,
	1030,
	2566,
	2054,
	1538,
	522,
	2052,
	2060,
	2056,
	2566,
	2054,
	522,
	2056,
	1546,
	2058,
	2564,
	1038,
	4106,
	3078,
	2056,
	1036,
	2566,
	2064,
	2572,
	3078,
	2578,
	3084,
	3594,
	2566,
	2062,
	3080,
	3092,
	3084,
	2574,
	2574,
	2064,
	3592,
	3092,
	2064,
	3596,
	3088,
	2576,
	4106,
	2062,
	6414,
	4106,
	2064,
	6926,
	3594,
	3097,
	2578,
	4108,
	2578,
	6416,
	4622,
	3594,
	4624,
	4622,
	3600,
	3092,
	4108,
	4125,
	3092,
	4618,
	4127,
	6416,
	4620,
	3101,
	5908,
	4620,
	4127,
	3095,
	5134,
	4631,
	4628,
	5904,
	3600,
	5143,
	5902,
	4631,
	3097,
	5134,
	5153,
	4631,
	5136,
	5153,
	3609,
	5902,
	4129,
	6425,
	5902,
	4131,
	6937,
	6420,
	4628,
	5147,
	6928,
	6939,
	2075,
	6937,
	6412,
	5145,
	6935,
	5911,
	4125,
	7447,
	4626,
	6941,
	6935,
	4633,
	7453,
	7956,
	6427,
	6429,
	7449,
	7451,
	3103,
	7961,
	5140,
	7455,
	7963,
	6420,
	6432,
	8729,
	5908,
	7714,
	8735,
	7184,
	8224,
	8988,
	7710,
	7203,
	9498,
	8222,
	4901,
	9757,
	8477,
	8486,
	10014,
	7712,
	8743,
	11040,
	8736,
	9257,
	11040,
	10020,
	5676,
	11813,
	9763,
	10797,
	11560,
	10536,
	10290,
	12331,
	11563,
	11568,
	13102,
	12334,
	12341,
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
	0,
	0,
	16184,
	13620,
	11068,
	14130,
	12067,
	7218,
	11559,
	8214,
	4901,
	8735,
	7440,
	3103,
	7452,
	6922,
	2075,
	5651,
	3593,
	2578,
	3338,
	2055,
	1547,
	2311,
	1541,
	1032,
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
	16128,
	63,
	16191,
	16128,
	63,
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
