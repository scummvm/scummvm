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

#ifndef SKY75COMP_H
#define SKY75COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc75_floor_table[] = {
	ID_SC75_FLOOR,
	RET_OK,
	ID_SC75_BIG_DOOR,
	GT_SC75_BIG_DOOR,
	0,
	SC75_BIG_DOOR_WALK_ON,
	ID_SC75_DOOR,
	GT_SC75_DOOR,
	1,
	SC75_DOOR_WALK_ON,
	ID_SC75_NITRO_TANK,
	GT_SC75_NITRO_TANK,
	ID_SC75_LIVE_TANK,
	GT_SC75_LIVE_TANK,
	ID_SC75_CONSOLE,
	GT_SC75_CONSOLE,
	ID_SC75_TONGS,
	GT_SC75_TONGS,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	65535
};

uint16 sc75_usecard[] = {
	155*64,
	269,
	223,
	0,
	269,
	223,
	1,
	269,
	223,
	2,
	269,
	223,
	2,
	269,
	223,
	2,
	269,
	223,
	2,
	269,
	223,
	2,
	269,
	223,
	1,
	269,
	223,
	0,
	0
};

ExtCompact sc75_console_ext = {
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
	0,	// spWidth_xx
	0,	// spColour
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

Compact sc75_console = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	289,	// xcood
	215,	// ycood
	149*64,	// frame
	182,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65533,	// mouseRel_x
	(int16) 65532,	// mouseRel_y
	313-286,	// mouseSize_x
	244-211,	// mouseSize_y
	SC75_CONSOLE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC75_CONSOLE_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc75_console_ext
};

uint16 sc75_get_tongs[] = {
	154*64,
	352,
	216,
	0,
	352,
	216,
	1,
	352,
	216,
	2,
	352,
	216,
	3,
	352,
	216,
	3,
	352,
	216,
	3,
	352,
	216,
	3,
	352,
	216,
	2,
	352,
	216,
	1,
	352,
	216,
	0,
	0
};

uint16 sc75_joey_list[] = {
	168,
	391,
	272,
	311,
	1,
	0
};

uint16 sc75_freeze_ded2[] = {
	163*64,
	261,
	240,
	0,
	261,
	240,
	1,
	261,
	240,
	2,
	261,
	240,
	3,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	3,
	261,
	240,
	5,
	261,
	240,
	6,
	261,
	240,
	7,
	0
};

uint16 sc75_hand_tank[] = {
	166*64,
	336,
	216,
	0,
	336,
	216,
	1,
	336,
	216,
	2,
	336,
	216,
	3,
	336,
	216,
	4,
	336,
	216,
	5,
	336,
	216,
	6,
	336,
	216,
	6,
	336,
	216,
	4,
	336,
	216,
	3,
	336,
	216,
	2,
	336,
	216,
	1,
	336,
	216,
	0,
	0
};

Compact sc75_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	389,	// xcood
	204,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	404-389,	// mouseSize_x
	268-204,	// mouseSize_y
	SC75_DOOR_ACTION,	// actionScript
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

uint16 sc75_light2_anim[] = {
	152*64,
	335,
	207,
	0,
	335,
	207,
	1,
	335,
	207,
	2,
	335,
	207,
	2,
	335,
	207,
	3,
	335,
	207,
	4,
	335,
	207,
	5,
	335,
	207,
	5,
	335,
	207,
	6,
	335,
	207,
	6,
	335,
	207,
	0,
	335,
	207,
	0,
	335,
	207,
	1,
	335,
	207,
	2,
	335,
	207,
	2,
	335,
	207,
	4,
	335,
	207,
	4,
	335,
	207,
	5,
	335,
	207,
	6,
	335,
	207,
	6,
	335,
	207,
	0,
	0
};

Compact sc75_light1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	287,	// xcood
	207,	// ycood
	151*64,	// frame
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
	SC75_LIGHT1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_tongs_dead[] = {
	C_FRAME,
	10+268*64,
	65535
};

uint16 sc75_tank_anim[] = {
	165*64,
	331,
	230,
	0,
	331,
	230,
	1,
	331,
	230,
	2,
	331,
	230,
	3,
	331,
	230,
	4,
	331,
	230,
	5,
	331,
	230,
	6,
	331,
	230,
	7,
	331,
	230,
	8,
	331,
	230,
	9,
	331,
	230,
	10,
	331,
	230,
	11,
	331,
	230,
	12,
	331,
	230,
	13,
	331,
	230,
	14,
	331,
	230,
	15,
	331,
	230,
	16,
	331,
	230,
	17,
	331,
	230,
	18,
	331,
	230,
	19,
	331,
	230,
	20,
	331,
	230,
	21,
	331,
	230,
	22,
	331,
	230,
	23,
	331,
	230,
	24,
	331,
	230,
	25,
	331,
	230,
	26,
	331,
	230,
	27,
	331,
	230,
	28,
	331,
	230,
	29,
	0
};

uint16 reset_73_75[] = {
	C_SCREEN,
	75,
	C_PLACE,
	ID_SC75_FLOOR,
	C_XCOOD,
	160,
	C_YCOOD,
	264,
	65535
};

uint32 *grid75 = 0;

uint16 sc75_light1_anim[] = {
	151*64,
	287,
	207,
	0,
	287,
	207,
	0,
	287,
	207,
	0,
	287,
	207,
	0,
	287,
	207,
	0,
	287,
	207,
	0,
	287,
	207,
	0,
	287,
	207,
	0,
	287,
	207,
	1,
	287,
	207,
	1,
	287,
	207,
	1,
	287,
	207,
	1,
	287,
	207,
	1,
	287,
	207,
	1,
	287,
	207,
	1,
	287,
	207,
	1,
	0
};

uint16 sc75_fast_list[] = {
	12+DISK_10,
	51+DISK_10,
	52+DISK_10,
	53+DISK_10,
	54+DISK_10,
	IT_SC75_GRID_1+DISK_10,
	IT_SC75_GRID_2+DISK_10,
	IT_SC75_RPOCKET+DISK_10,
	IT_SC75_USECARD+DISK_10,
	268+DISK_10,
	0
};

Compact sc75_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	75,	// screen
	0,	// place
	sc75_floor_table,	// getToTable
	180,	// xcood
	260,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	399-180,	// mouseSize_x
	327-260,	// mouseSize_y
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

uint16 rs_tongs_empty[] = {
	C_FRAME,
	4+268*64,
	65535
};

Compact sc75_big_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	178,	// xcood
	182,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	190-178,	// mouseSize_x
	270-182,	// mouseSize_y
	SC75_BIG_DOOR_ACTION,	// actionScript
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

uint16 sc75_mouse_list[] = {
	ID_KEN,
	ID_SC75_CONSOLE,
	ID_SC75_TONGS,
	ID_SC75_LIVE_TANK,
	ID_SC75_NITRO_TANK,
	ID_SC75_BIG_DOOR,
	ID_SC75_DOOR,
	ID_SC75_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc75_live_tank = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	331,	// xcood
	230,	// ycood
	165*64,	// frame
	20629,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65527,	// mouseRel_x
	(int16) 65533,	// mouseRel_y
	380-322,	// mouseSize_x
	251-227,	// mouseSize_y
	SC75_LIVE_TANK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC75_LIVE_TANK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc75_freeze_it[] = {
	158*64,
	261,
	240,
	0,
	261,
	240,
	1,
	261,
	240,
	2,
	261,
	240,
	3,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	3,
	261,
	240,
	5,
	261,
	240,
	6,
	261,
	240,
	7,
	261,
	240,
	8,
	261,
	240,
	9,
	261,
	240,
	10,
	261,
	240,
	11,
	261,
	240,
	12,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	14,
	261,
	240,
	15,
	261,
	240,
	16,
	261,
	240,
	17,
	0
};

Compact sc75_tongs = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	382,	// xcood
	218,	// ycood
	153*64,	// frame
	20639,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	0,	// mouseRel_y
	3,	// mouseSize_x
	13,	// mouseSize_y
	SC75_TONGS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC75_TONGS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc75_chip_list[] = {
	IT_SC75_LAYER_0+DISK_10,
	IT_SC75_LAYER_1+DISK_10,
	IT_SC75_LAYER_2+DISK_10,
	IT_KEN+DISK_10,
	IT_SC76_KEN_TALK+DISK_10,
	IT_SC75_MONITOR+DISK_10,
	IT_SC75_CRASH+DISK_10,
	IT_SC75_TANK+DISK_10,
	IT_SC75_STEAM+DISK_10,
	IT_SC75_LIGHT1+DISK_10,
	IT_SC75_LIGHT2+DISK_10,
	IT_SC75_FREEZE_IT+DISK_10+0X8000,
	IT_SC75_FREEZE_TALK+DISK_10+0X8000,
	IT_SC75_FREEZE_IT2+DISK_10+0X8000,
	IT_SC75_FREEZE_DED+DISK_10+0X8000,
	IT_SC75_DEAD_TALK+DISK_10+0X8000,
	IT_SC75_FREEZE_DED2+DISK_10+0X8000,
	IT_SC75_TONGS+DISK_10,
	IT_SC75_GET_TONGS+DISK_10+0X8000,
	IT_SC75_GET_TISS+DISK_10+0X8000,
	IT_SC75_HAND_TANK+DISK_10+0X8000,
	0
};

uint16 sc75_nitro_anim[] = {
	150*64,
	210,
	255,
	0,
	210,
	255,
	1,
	210,
	255,
	1,
	210,
	255,
	2,
	210,
	255,
	2,
	210,
	255,
	2,
	210,
	255,
	3,
	210,
	255,
	3,
	210,
	255,
	3,
	210,
	255,
	4,
	210,
	255,
	4,
	210,
	255,
	4,
	210,
	255,
	5,
	210,
	255,
	5,
	210,
	255,
	5,
	210,
	255,
	6,
	210,
	255,
	6,
	210,
	255,
	6,
	210,
	255,
	7,
	210,
	255,
	7,
	210,
	255,
	8,
	0
};

uint16 rs_tongs_frozen[] = {
	C_FRAME,
	8+268*64,
	65535
};

uint16 sc75_crash_anim[] = {
	164*64,
	289,
	215,
	0,
	289,
	215,
	1,
	289,
	215,
	2,
	289,
	215,
	3,
	289,
	215,
	4,
	289,
	215,
	5,
	289,
	215,
	6,
	289,
	215,
	7,
	289,
	215,
	8,
	289,
	215,
	9,
	289,
	215,
	10,
	289,
	215,
	11,
	289,
	215,
	12,
	289,
	215,
	13,
	289,
	215,
	14,
	289,
	215,
	15,
	289,
	215,
	16,
	289,
	215,
	17,
	289,
	215,
	18,
	289,
	215,
	19,
	289,
	215,
	20,
	289,
	215,
	21,
	289,
	215,
	22,
	289,
	215,
	23,
	289,
	215,
	24,
	289,
	215,
	25,
	289,
	215,
	26,
	289,
	215,
	27,
	289,
	215,
	28,
	289,
	215,
	29,
	0
};

Compact sc75_light2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	335,	// xcood
	207,	// ycood
	152*64,	// frame
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
	SC75_LIGHT2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc75_mon_anim[] = {
	149*64,
	289,
	215,
	0,
	289,
	215,
	1,
	289,
	215,
	2,
	289,
	215,
	2,
	289,
	215,
	2,
	289,
	215,
	2,
	289,
	215,
	3,
	289,
	215,
	4,
	289,
	215,
	5,
	289,
	215,
	6,
	289,
	215,
	7,
	289,
	215,
	8,
	289,
	215,
	9,
	289,
	215,
	10,
	289,
	215,
	10,
	289,
	215,
	10,
	289,
	215,
	10,
	289,
	215,
	10,
	289,
	215,
	11,
	289,
	215,
	12,
	289,
	215,
	13,
	289,
	215,
	14,
	289,
	215,
	15,
	289,
	215,
	16,
	289,
	215,
	16,
	289,
	215,
	16,
	289,
	215,
	16,
	289,
	215,
	17,
	289,
	215,
	18,
	289,
	215,
	19,
	0
};

uint16 sc75_freeze_it2[] = {
	160*64,
	261,
	240,
	0,
	261,
	240,
	1,
	261,
	240,
	2,
	261,
	240,
	3,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	3,
	261,
	240,
	5,
	261,
	240,
	6,
	261,
	240,
	7,
	0
};

Compact sc75_nitro_tank = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	75,	// screen
	0,	// place
	0,	// getToTable
	210,	// xcood
	255,	// ycood
	150*64,	// frame
	20629,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65533,	// mouseRel_x
	2,	// mouseRel_y
	283-207,	// mouseSize_x
	307-257,	// mouseSize_y
	SC75_NITRO_TANK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC75_NITRO_TANK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc75_palette[] = {
	0,
	1024,
	1028,
	1285,
	1799,
	1799,
	2055,
	2570,
	2312,
	2569,
	2826,
	2826,
	3082,
	5132,
	2056,
	2830,
	3084,
	3341,
	3340,
	3856,
	3597,
	2582,
	3594,
	4622,
	3601,
	3856,
	4112,
	3095,
	3596,
	5136,
	4112,
	4118,
	4626,
	3863,
	4111,
	5395,
	4627,
	6676,
	3855,
	4629,
	4626,
	5908,
	5396,
	4885,
	6676,
	5904,
	6422,
	5140,
	5654,
	5142,
	6679,
	5402,
	5911,
	6422,
	6421,
	5400,
	7448,
	6424,
	4631,
	6683,
	5914,
	4889,
	6173,
	6422,
	6176,
	7705,
	6681,
	6426,
	7452,
	7957,
	7705,
	6681,
	7195,
	7195,
	7707,
	7193,
	8483,
	6426,
	8727,
	7194,
	7487,
	7453,
	7968,
	7709,
	8984,
	7709,
	7456,
	8221,
	7713,
	8990,
	7459,
	8221,
	8478,
	7964,
	6439,
	7973,
	8477,
	7204,
	8227,
	8477,
	7977,
	8738,
	7973,
	8735,
	8225,
	8231,
	8737,
	9506,
	8994,
	7714,
	8488,
	9505,
	7718,
	9256,
	9504,
	9002,
	9253,
	9510,
	9764,
	10020,
	10782,
	8742,
	10279,
	9002,
	9253,
	9767,
	10277,
	8233,
	10283,
	9770,
	10020,
	10534,
	10275,
	11054,
	10022,
	10535,
	9771,
	11817,
	10282,
	11563,
	10280,
	11044,
	9263,
	9774,
	11047,
	11048,
	10282,
	11049,
	9514,
	12589,
	10795,
	9773,
	13357,
	11560,
	11570,
	11820,
	11055,
	10028,
	11569,
	11568,
	11822,
	11311,
	11822,
	11312,
	12848,
	12585,
	11576,
	13616,
	12081,
	13105,
	12080,
	13354,
	11834,
	13363,
	12851,
	12337,
	13875,
	12853,
	12851,
	12597,
	13873,
	13624,
	13877,
	13623,
	13364,
	14392,
	14135,
	14902,
	14394,
	14908,
	15674,
	15420,
	16129,
	10502,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	5416,
	5916,
	7951,
	4106,
	3364,
	10514,
	5392,
	5166,
	14361,
	6434,
	6459,
	16146,
	0,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
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
	13886,
	14893,
	9263,
	10551,
	13341,
	5666,
	7217,
	11791,
	2582,
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
	10559,
	10021,
	9513,
	10535,
	10021,
	9513,
	13863,
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

uint16 rs_tongs_live[] = {
	C_FRAME,
	6+268*64,
	65535
};

uint16 reset_76_75[] = {
	C_SCREEN,
	75,
	C_PLACE,
	ID_SC75_FLOOR,
	C_XCOOD,
	416,
	C_YCOOD,
	272,
	65535
};

uint16 sc75_logic_list[] = {
	ID_KEN,
	ID_SC75_LIGHT1,
	ID_SC75_LIGHT2,
	ID_SC75_CONSOLE,
	ID_SC75_LIVE_TANK,
	ID_SC75_NITRO_TANK,
	ID_SC75_TONGS,
	ID_FOSTER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 rs_tongs_timer[] = {
	C_LOGIC,
	L_SCRIPT,
	C_STATUS,
	ST_LOGIC,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC75_TISSUE_LOGIC,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 sc75_freeze_ded[] = {
	161*64,
	261,
	240,
	0,
	261,
	240,
	1,
	261,
	240,
	2,
	261,
	240,
	3,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	4,
	261,
	240,
	3,
	261,
	240,
	5,
	261,
	240,
	6,
	261,
	240,
	7,
	261,
	240,
	8,
	261,
	240,
	9,
	261,
	240,
	10,
	261,
	240,
	11,
	261,
	240,
	12,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	13,
	261,
	240,
	14,
	261,
	240,
	14,
	261,
	240,
	14,
	261,
	240,
	14,
	261,
	240,
	14,
	261,
	240,
	14,
	261,
	240,
	15,
	261,
	240,
	16,
	261,
	240,
	17,
	261,
	240,
	18,
	261,
	240,
	19,
	0
};

uint16 sc75_rpocket[] = {
	156*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 sc75_get_tiss[] = {
	157*64,
	340,
	216,
	0,
	340,
	216,
	1,
	340,
	216,
	2,
	340,
	216,
	3,
	340,
	216,
	4,
	340,
	216,
	4,
	340,
	216,
	4,
	340,
	216,
	4,
	340,
	216,
	4,
	340,
	216,
	4,
	340,
	216,
	4,
	340,
	216,
	4,
	340,
	216,
	5,
	340,
	216,
	6,
	340,
	216,
	7,
	340,
	216,
	8,
	340,
	216,
	9,
	340,
	216,
	10,
	340,
	216,
	11,
	340,
	216,
	12,
	340,
	216,
	13,
	340,
	216,
	14,
	340,
	216,
	9,
	340,
	216,
	9,
	340,
	216,
	11,
	340,
	216,
	13,
	340,
	216,
	15,
	340,
	216,
	16,
	340,
	216,
	17,
	340,
	216,
	17,
	340,
	216,
	17,
	340,
	216,
	17,
	340,
	216,
	18,
	340,
	216,
	19,
	340,
	216,
	20,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
