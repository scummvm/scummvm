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

#ifndef SKY76COMP_H
#define SKY76COMP_H




namespace Sky {

namespace SkyCompact {

uint16 ken_u_to_r[] = {
	45+61*64,
	0
};

uint16 sc76_logic_list[] = {
	ID_FOSTER,
	ID_KEN,
	ID_SC76_ANDROID_1,
	ID_SC76_ANDROID_2,
	ID_SC76_ANDROID_3,
	ID_SC76_LIGHT1,
	ID_SC76_LIGHT2,
	ID_SC76_LIGHT3,
	ID_SC76_LIGHT4,
	ID_SC76_LIGHT5,
	ID_SC76_LIGHT6,
	ID_SC76_LIGHT7,
	ID_SC76_LIGHT8,
	ID_SC76_LIGHT9,
	ID_SC76_BOARD_1,
	ID_SC76_BOARD_2,
	ID_SC76_BOARD_3,
	ID_SC76_CABINET_1,
	ID_SC76_CABINET_2,
	ID_SC76_CABINET_3,
	ID_SC75_TONGS,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint32 *grid76 = 0;

uint16 sc76_hatch_2[] = {
	68*64,
	271,
	204,
	0,
	271,
	204,
	0,
	266,
	204,
	1,
	266,
	204,
	1,
	263,
	204,
	2,
	263,
	204,
	2,
	263,
	212,
	3,
	263,
	212,
	3,
	263,
	216,
	4,
	263,
	216,
	4,
	262,
	219,
	5,
	262,
	219,
	5,
	256,
	222,
	6,
	256,
	222,
	6,
	251,
	224,
	7,
	251,
	224,
	7,
	248,
	224,
	8,
	248,
	224,
	8,
	247,
	226,
	9,
	247,
	226,
	9,
	247,
	228,
	10,
	247,
	228,
	10,
	247,
	228,
	11,
	247,
	228,
	11,
	0
};

uint16 sc76_cab2_close[] = {
	56*64,
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

uint16 reset_75_76[] = {
	C_SCREEN,
	76,
	C_PLACE,
	ID_SC76_FLOOR,
	C_XCOOD,
	160,
	C_YCOOD,
	264,
	65535
};

uint16 sc76_chip_list[] = {
	IT_SC76_LAYER_0+DISK_10,
	IT_SC76_LAYER_1+DISK_10,
	IT_SC76_LAYER_2+DISK_10,
	IT_SC76_GRID_1+DISK_10,
	IT_SC76_GRID_2+DISK_10,
	IT_KEN+DISK_10,
	IT_SC76_KEN_TALK+DISK_10,
	IT_SC76_LIGHT1+DISK_10,
	IT_SC76_LIGHT2+DISK_10,
	IT_SC76_LIGHT3+DISK_10,
	IT_SC76_LIGHT4+DISK_10,
	IT_SC76_LIGHT5+DISK_10,
	IT_SC76_LIGHT6+DISK_10,
	IT_SC76_LIGHT7+DISK_10,
	IT_SC76_LIGHT8+DISK_10,
	IT_SC76_LIGHT9+DISK_10,
	IT_SC76_CABINET_1+DISK_10,
	IT_SC76_CABINET_2+DISK_10,
	IT_SC76_CABINET_3+DISK_10,
	IT_SC76_BOARD_1+DISK_10,
	IT_SC76_BOARD_2+DISK_10,
	IT_SC76_BOARD_3+DISK_10,
	IT_SC76_OPEN_CAB+DISK_10,
	IT_SC76_LOW_GET+DISK_10,
	IT_SC76_HATCH_1+DISK_10,
	IT_SC76_ANDROID_2+DISK_10,
	IT_SC76_HATCH_2+DISK_10+0X8000,
	IT_SC76_ANDROID_3+DISK_10,
	IT_SC76_HATCH_3+DISK_10+0X8000,
	IT_SC76_PUNCH+DISK_10,
	IT_SC76_AND2_BABBLE+DISK_10,
	0
};

uint16 ken_right[] = {
	4,
	30+61*64,
	4,
	0,
	4,
	31+61*64,
	4,
	0,
	4,
	32+61*64,
	4,
	0,
	4,
	33+61*64,
	4,
	0,
	4,
	34+61*64,
	4,
	0,
	4,
	35+61*64,
	4,
	0,
	4,
	36+61*64,
	4,
	0,
	4,
	37+61*64,
	4,
	0,
	4,
	38+61*64,
	4,
	0,
	4,
	39+61*64,
	4,
	0,
	0
};

uint16 ken_st_up[] = {
	61*64,
	1,
	0,
	44,
	0
};

uint16 ken_d_to_l[] = {
	41+61*64,
	0
};

uint16 ken_auto[32];

uint16 ken_up[] = {
	2,
	0+61*64,
	0,
	65534,
	2,
	1+61*64,
	0,
	65534,
	2,
	2+61*64,
	0,
	65534,
	2,
	3+61*64,
	0,
	65534,
	2,
	4+61*64,
	0,
	65534,
	2,
	5+61*64,
	0,
	65534,
	2,
	6+61*64,
	0,
	65534,
	2,
	7+61*64,
	0,
	65534,
	2,
	8+61*64,
	0,
	65534,
	2,
	9+61*64,
	0,
	65534,
	0
};

uint16 ken_down[] = {
	2,
	10+61*64,
	0,
	2,
	2,
	11+61*64,
	0,
	2,
	2,
	12+61*64,
	0,
	2,
	2,
	13+61*64,
	0,
	2,
	2,
	14+61*64,
	0,
	2,
	2,
	15+61*64,
	0,
	2,
	2,
	16+61*64,
	0,
	2,
	2,
	17+61*64,
	0,
	2,
	2,
	18+61*64,
	0,
	2,
	2,
	19+61*64,
	0,
	2,
	0
};

uint16 ken_left[] = {
	4,
	20+61*64,
	65532,
	0,
	4,
	21+61*64,
	65532,
	0,
	4,
	22+61*64,
	65532,
	0,
	4,
	23+61*64,
	65532,
	0,
	4,
	24+61*64,
	65532,
	0,
	4,
	25+61*64,
	65532,
	0,
	4,
	26+61*64,
	65532,
	0,
	4,
	27+61*64,
	65532,
	0,
	4,
	28+61*64,
	65532,
	0,
	4,
	29+61*64,
	65532,
	0,
	0
};

uint16 ken_st_down[] = {
	61*64,
	1,
	0,
	40,
	0
};

uint16 ken_st_left[] = {
	61*64,
	1,
	0,
	42,
	0
};

uint16 ken_st_right[] = {
	61*64,
	1,
	0,
	46,
	0
};

uint16 ken_u_to_d[] = {
	43+61*64,
	42+61*64,
	41+61*64,
	0
};

uint16 ken_u_to_l[] = {
	43+61*64,
	0
};

uint16 ken_d_to_u[] = {
	47+61*64,
	46+61*64,
	45+61*64,
	0
};

uint16 ken_d_to_r[] = {
	47+61*64,
	0
};

uint16 ken_l_to_u[] = {
	43+61*64,
	0
};

uint16 ken_l_to_d[] = {
	41+61*64,
	0
};

uint16 ken_l_to_r[] = {
	41+61*64,
	40+61*64,
	47+61*64,
	0
};

uint16 ken_r_to_u[] = {
	45+61*64,
	0
};

uint16 ken_r_to_d[] = {
	47+61*64,
	0
};

uint16 ken_r_to_l[] = {
	45+61*64,
	44+61*64,
	43+61*64,
	0
};

TurnTable ken_turnTable0 = {
	{ // turnTableUp
		0,
		ken_u_to_d,
		ken_u_to_l,
		ken_u_to_r,
		0
	},
	{ // turnTableDown
		ken_d_to_u,
		0,
		ken_d_to_l,
		ken_d_to_r,
		0
	},
	{ // turnTableLeft
		ken_l_to_u,
		ken_l_to_d,
		0,
		ken_l_to_r,
		0
	},
	{ // turnTableRight
		ken_r_to_u,
		ken_r_to_d,
		ken_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		0,
		0,
		0,
		0,
		0
	}
};

MegaSet ken_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	ken_up,	// animUp
	ken_down,	// animDown
	ken_left,	// animLeft
	ken_right,	// animRight
	ken_st_up,	// standUp
	ken_st_down,	// standDown
	ken_st_left,	// standLeft
	ken_st_right,	// standRight
	0,	// standTalk
	&ken_turnTable0
};

ExtCompact ken_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	DOWN,	// dir
	STD_PLAYER_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_COL_KEN,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	ken_auto,	// animScratch
	0,	// megaSet
	&ken_megaSet0,
	0,
	0,
	0
};

Compact ken = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	76,	// screen
	ID_SC76_FLOOR,	// place
	0,	// getToTable
	256,	// xcood
	272,	// ycood
	40+61*64,	// frame
	3,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65528,	// mouseRel_x
	(int16) 65493,	// mouseRel_y
	14,	// mouseSize_x
	50,	// mouseSize_y
	SHOUT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	KEN_START_LOGIC,	// baseSub
	0,	// baseSub_off
	&ken_ext
};

uint16 sc76_floor_table[] = {
	ID_SC76_FLOOR,
	RET_OK,
	ID_SC76_DOOR75,
	GT_SC76_DOOR75,
	0,
	SC76_DOOR75_WALK_ON,
	ID_SC76_DOOR77,
	GT_SC76_DOOR77,
	1,
	SC76_DOOR77_WALK_ON,
	ID_SC76_ANDROID_1,
	GT_SC76_ANYTHING,
	ID_SC76_ANDROID_2,
	GT_SC76_ANYTHING,
	ID_SC76_ANDROID_3,
	GT_SC76_ANYTHING,
	ID_SC76_CONSOLE_1,
	GT_SC76_ANYTHING,
	ID_SC76_CONSOLE_2,
	GT_SC76_ANYTHING,
	ID_SC76_CONSOLE_3,
	GT_SC76_ANYTHING,
	ID_SC76_CABINET_1,
	GT_SC76_ANYTHING,
	ID_SC76_CABINET_2,
	GT_SC76_ANYTHING,
	ID_SC76_CABINET_3,
	GT_SC76_ANYTHING,
	ID_SC76_BOARD_1,
	GT_SC76_ANYTHING,
	ID_SC76_BOARD_2,
	GT_SC76_ANYTHING,
	ID_SC76_BOARD_3,
	GT_SC76_ANYTHING,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	65535
};

Compact sc76_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	sc76_floor_table,	// getToTable
	175,	// xcood
	255,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	391-175,	// mouseSize_x
	327-255,	// mouseSize_y
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

Compact sc76_light3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	278,	// xcood
	182,	// ycood
	31*64,	// frame
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
	SC76_LIGHT3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc76_light9 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	357,	// xcood
	239,	// ycood
	47*64,	// frame
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
	SC76_LIGHT9_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_light1_anim[] = {
	29*64,
	228,
	182,
	0,
	228,
	182,
	0,
	228,
	182,
	0,
	228,
	182,
	0,
	228,
	182,
	0,
	228,
	182,
	0,
	228,
	182,
	0,
	228,
	182,
	1,
	228,
	182,
	1,
	228,
	182,
	1,
	228,
	182,
	1,
	228,
	182,
	1,
	228,
	182,
	1,
	228,
	182,
	1,
	0
};

uint16 sc76_light2_anim[] = {
	30*64,
	232,
	182,
	0,
	232,
	182,
	0,
	232,
	182,
	0,
	232,
	182,
	1,
	232,
	182,
	2,
	232,
	182,
	0,
	232,
	182,
	0,
	232,
	182,
	0,
	232,
	182,
	0,
	232,
	182,
	0,
	232,
	182,
	0,
	232,
	182,
	0,
	0
};

Compact sc76_light6 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	327,	// xcood
	182,	// ycood
	44*64,	// frame
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
	SC76_LIGHT6_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc76_board_3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	356,	// xcood
	253,	// ycood
	60*64,	// frame
	6,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	365-355,	// mouseSize_x
	254-251,	// mouseSize_y
	SC76_BOARD_3_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_BOARD_3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_shut_cab[] = {
	63*64,
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
	0+62*64,
	1,
	1,
	2+62*64,
	1,
	1,
	1+62*64,
	1,
	1,
	0+62*64,
	1,
	1,
	8,
	1,
	1,
	6,
	1,
	1,
	4,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 sc76_light3_anim[] = {
	31*64,
	278,
	182,
	0,
	278,
	182,
	0,
	278,
	182,
	1,
	278,
	182,
	1,
	278,
	182,
	1,
	278,
	182,
	1,
	278,
	182,
	1,
	278,
	182,
	1,
	278,
	182,
	1,
	278,
	182,
	0,
	278,
	182,
	0,
	278,
	182,
	0,
	278,
	182,
	0,
	278,
	182,
	0,
	0
};

Compact sc76_light1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	228,	// xcood
	182,	// ycood
	29*64,	// frame
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
	SC76_LIGHT1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_low_get[] = {
	63*64,
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
	8,
	1,
	1,
	7,
	1,
	1,
	6,
	1,
	1,
	4,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 sc76_fostfall[] = {
	70*64,
	354,
	218,
	0,
	355,
	218,
	1,
	355,
	218,
	2,
	357,
	219,
	3,
	366,
	216,
	4,
	371,
	222,
	5,
	376,
	234,
	6,
	381,
	255,
	7,
	382,
	261,
	8,
	381,
	262,
	9,
	382,
	262,
	10,
	382,
	262,
	11,
	382,
	262,
	11,
	0
};

Compact sc76_door77 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	377,	// xcood
	208,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	392-377,	// mouseSize_x
	271-208,	// mouseSize_y
	SC76_DOOR77_ACTION,	// actionScript
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

Compact sc76_board_1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	248,	// xcood
	253,	// ycood
	58*64,	// frame
	6,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	257-247,	// mouseSize_x
	254-251,	// mouseSize_y
	SC76_BOARD_1_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_BOARD_1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc76_light8 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	304,	// xcood
	239,	// ycood
	46*64,	// frame
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
	SC76_LIGHT8_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc76_android_1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_BACKGROUND,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	224,	// xcood
	204,	// ycood
	66*64,	// frame
	20691,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65528,	// mouseRel_x
	0,	// mouseRel_y
	238-216,	// mouseSize_x
	243-204,	// mouseSize_y
	SC76_ANDROID_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_ANDROID_1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_cab1_open[] = {
	55*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	0
};

uint16 sc76_hatch_1[] = {
	66*64,
	224,
	204,
	0,
	224,
	204,
	0,
	224,
	204,
	1,
	224,
	204,
	1,
	224,
	204,
	2,
	224,
	204,
	2,
	224,
	204,
	3,
	224,
	204,
	3,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	4,
	224,
	204,
	3,
	224,
	204,
	3,
	224,
	204,
	2,
	224,
	204,
	2,
	224,
	204,
	1,
	224,
	204,
	1,
	224,
	204,
	0,
	224,
	204,
	0,
	0
};

Compact sc76_console_2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	299,	// xcood
	210,	// ycood
	0,	// frame
	182,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	316-299,	// mouseSize_x
	244-210,	// mouseSize_y
	SC76_CONSOLE_2_ACTION,	// actionScript
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

ExtCompact sc76_android_3_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	RIGHT,	// dir
	0,	// stopScript
	0,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_COL_ANDROID3,	// spColour
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

Compact sc76_android_3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	319,	// xcood
	204,	// ycood
	64*64,	// frame
	20691,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	340-319,	// mouseSize_x
	243-204,	// mouseSize_y
	SC76_ANDROID_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_ANDROID_3_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc76_android_3_ext
};

Compact sc76_console_1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	244,	// xcood
	210,	// ycood
	0,	// frame
	182,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	260-244,	// mouseSize_x
	244-210,	// mouseSize_y
	SC76_CONSOLE_1_ACTION,	// actionScript
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

uint16 sc76_fast_list[] = {
	12+DISK_10,
	51+DISK_10,
	52+DISK_10,
	53+DISK_10,
	54+DISK_10,
	268+DISK_10,
	0
};

ExtCompact sc76_android_2_ext = {
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
	SP_COL_ANDROID2,	// spColour
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

Compact sc76_android_2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	271,	// xcood
	204,	// ycood
	67*64,	// frame
	20691,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	289-271,	// mouseSize_x
	243-204,	// mouseSize_y
	SC76_ANDROID_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_ANDROID_2_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc76_android_2_ext
};

uint16 sc76_cab1_close[] = {
	55*64,
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

Compact sc76_light5 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	323,	// xcood
	182,	// ycood
	43*64,	// frame
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
	SC76_LIGHT5_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc76_board_2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	302,	// xcood
	253,	// ycood
	59*64,	// frame
	6,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	312-302,	// mouseSize_x
	254-251,	// mouseSize_y
	SC76_BOARD_2_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_BOARD_2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_light9_anim[] = {
	47*64,
	357,
	239,
	0,
	357,
	239,
	1,
	357,
	239,
	2,
	357,
	239,
	3,
	357,
	239,
	4,
	357,
	239,
	5,
	357,
	239,
	6,
	357,
	239,
	7,
	357,
	239,
	8,
	357,
	239,
	9,
	357,
	239,
	10,
	357,
	239,
	10,
	0
};

uint16 sc76_cab3_open[] = {
	57*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	0
};

Compact sc76_light7 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	247,	// xcood
	239,	// ycood
	45*64,	// frame
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
	SC76_LIGHT7_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc76_light4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	282,	// xcood
	182,	// ycood
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
	SC76_LIGHT4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_mouse_list[] = {
	ID_KEN,
	ID_SC76_ANDROID_1,
	ID_SC76_ANDROID_2,
	ID_SC76_ANDROID_3,
	ID_SC76_CONSOLE_1,
	ID_SC76_CONSOLE_2,
	ID_SC76_CONSOLE_3,
	ID_SC76_BOARD_1,
	ID_SC76_BOARD_2,
	ID_SC76_BOARD_3,
	ID_SC76_CABINET_1,
	ID_SC76_CABINET_2,
	ID_SC76_CABINET_3,
	ID_SC76_DOOR75,
	ID_SC76_DOOR77,
	ID_SC76_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc76_joey_list[] = {
	168,
	415,
	272,
	311,
	1,
	168,
	191,
	272,
	279,
	0,
	384,
	415,
	272,
	279,
	0,
	0
};

uint16 reset_77_76[] = {
	C_SCREEN,
	76,
	C_PLACE,
	ID_SC76_FLOOR,
	C_XCOOD,
	408,
	C_YCOOD,
	272,
	65535
};

uint16 sc76_light5_anim[] = {
	43*64,
	323,
	182,
	0,
	323,
	182,
	0,
	323,
	182,
	0,
	323,
	182,
	0,
	323,
	182,
	0,
	323,
	182,
	0,
	323,
	182,
	1,
	323,
	182,
	1,
	323,
	182,
	1,
	323,
	182,
	1,
	323,
	182,
	1,
	323,
	182,
	1,
	323,
	182,
	1,
	323,
	182,
	0,
	0
};

uint16 sc76_light4_anim[] = {
	32*64,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	0,
	282,
	182,
	1,
	282,
	182,
	2,
	282,
	182,
	0,
	0
};

uint16 sc76_hatch_3[] = {
	65*64,
	319,
	204,
	0,
	319,
	204,
	0,
	317,
	204,
	1,
	317,
	204,
	1,
	315,
	204,
	2,
	315,
	204,
	2,
	312,
	213,
	3,
	312,
	213,
	3,
	316,
	216,
	4,
	316,
	216,
	4,
	315,
	219,
	5,
	315,
	219,
	5,
	308,
	222,
	6,
	308,
	222,
	7,
	305,
	224,
	8,
	305,
	224,
	8,
	302,
	221,
	9,
	302,
	221,
	9,
	303,
	220,
	10,
	303,
	220,
	10,
	301,
	220,
	11,
	302,
	220,
	12,
	307,
	220,
	13,
	0
};

Compact sc76_cabinet_1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	245,	// xcood
	246,	// ycood
	55*64,	// frame
	20692,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	261-244,	// mouseSize_x
	265-245,	// mouseSize_y
	SC76_CABINET_1_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_CABINET_1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_punch[] = {
	69*64,
	307,
	220,
	0,
	312,
	220,
	1,
	311,
	221,
	2,
	312,
	221,
	3,
	318,
	221,
	4,
	326,
	220,
	5,
	331,
	220,
	6,
	331,
	221,
	7,
	337,
	220,
	0,
	337,
	220,
	0,
	324,
	223,
	8,
	323,
	224,
	9,
	320,
	224,
	10,
	330,
	224,
	11,
	331,
	222,
	12,
	331,
	222,
	13,
	332,
	221,
	14,
	337,
	220,
	0,
	337,
	220,
	0,
	337,
	220,
	15,
	0
};

uint16 sc76_cab2_open[] = {
	56*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	0
};

uint16 sc76_light7_anim[] = {
	45*64,
	247,
	239,
	0,
	247,
	239,
	1,
	247,
	239,
	2,
	247,
	239,
	3,
	247,
	239,
	4,
	247,
	239,
	5,
	247,
	239,
	6,
	247,
	239,
	7,
	247,
	239,
	8,
	247,
	239,
	9,
	247,
	239,
	9,
	247,
	239,
	9,
	247,
	239,
	9,
	0
};

Compact sc76_cabinet_3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	355,	// xcood
	246,	// ycood
	57*64,	// frame
	20692,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	1,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	370-356,	// mouseSize_x
	265-245,	// mouseSize_y
	SC76_CABINET_3_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_CABINET_3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc76_console_3 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	350,	// xcood
	210,	// ycood
	0,	// frame
	182,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	369-350,	// mouseSize_x
	244-210,	// mouseSize_y
	SC76_CONSOLE_3_ACTION,	// actionScript
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

uint16 sc76_open_cab[] = {
	63*64,
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
	0+62*64,
	1,
	1,
	1+62*64,
	1,
	1,
	2+62*64,
	1,
	1,
	2+62*64,
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

uint16 sc76_palette[] = {
	0,
	1024,
	1540,
	1029,
	1540,
	1542,
	2054,
	2056,
	1799,
	2056,
	2570,
	2825,
	0,
	3072,
	3082,
	2826,
	3086,
	3085,
	3087,
	3084,
	3853,
	3599,
	3342,
	4623,
	4111,
	4368,
	4113,
	3859,
	3856,
	4626,
	4626,
	4116,
	5395,
	4882,
	4626,
	5140,
	4882,
	5911,
	4626,
	5141,
	4887,
	5910,
	5653,
	5653,
	5654,
	5147,
	5654,
	6167,
	5911,
	5914,
	6937,
	6679,
	6425,
	6937,
	6933,
	8478,
	6166,
	6174,
	7448,
	7449,
	7194,
	6939,
	7707,
	7197,
	8220,
	7450,
	7196,
	6689,
	7454,
	6435,
	7195,
	7966,
	7710,
	9761,
	7195,
	8474,
	10530,
	6938,
	8732,
	9248,
	7965,
	8222,
	7971,
	7969,
	7969,
	7970,
	8483,
	8737,
	9763,
	8224,
	7470,
	7452,
	9765,
	7724,
	8990,
	9506,
	9251,
	11299,
	8224,
	8238,
	8477,
	9765,
	9253,
	10789,
	8482,
	10273,
	11816,
	8226,
	8497,
	9758,
	9766,
	9003,
	9254,
	10535,
	10274,
	10027,
	9766,
	9008,
	11554,
	8997,
	11044,
	10019,
	10536,
	10536,
	9510,
	11049,
	9521,
	10788,
	10792,
	11558,
	12067,
	9511,
	11048,
	9769,
	11563,
	10033,
	10023,
	9519,
	10543,
	13096,
	9511,
	11050,
	10027,
	12077,
	11563,
	12841,
	10537,
	12585,
	11045,
	11565,
	11310,
	11565,
	11054,
	12073,
	11054,
	10033,
	11820,
	13104,
	11051,
	12331,
	12083,
	11568,
	13101,
	11559,
	12593,
	12590,
	12340,
	12082,
	12339,
	13617,
	12079,
	13360,
	13110,
	12595,
	13106,
	13364,
	13109,
	14129,
	13622,
	13623,
	14131,
	14136,
	14135,
	14649,
	14902,
	14649,
	15159,
	15161,
	14909,
	15676,
	15933,
	15935,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
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
	15149,
	9520,
	10809,
	13854,
	5924,
	7988,
	12817,
	2841,
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
	63,
	0,
	0,
	0,
	0,
	0,
	13824,
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

Compact sc76_light2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	232,	// xcood
	182,	// ycood
	30*64,	// frame
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
	SC76_LIGHT2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_light8_anim[] = {
	46*64,
	304,
	239,
	0,
	304,
	239,
	0,
	304,
	239,
	1,
	304,
	239,
	1,
	304,
	239,
	2,
	304,
	239,
	3,
	304,
	239,
	4,
	304,
	239,
	5,
	304,
	239,
	6,
	304,
	239,
	7,
	304,
	239,
	8,
	304,
	239,
	9,
	304,
	239,
	10,
	304,
	239,
	11,
	0
};

uint16 sc76_cab3_close[] = {
	57*64,
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

Compact sc76_cabinet_2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	301,	// xcood
	246,	// ycood
	56*64,	// frame
	20692,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	1,	// mouseRel_x
	1,	// mouseRel_y
	316-302,	// mouseSize_x
	265-245,	// mouseSize_y
	SC76_CABINET_2_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC76_CABINET_2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc76_light6_anim[] = {
	44*64,
	327,
	182,
	0,
	327,
	182,
	1,
	327,
	182,
	2,
	327,
	182,
	2,
	327,
	182,
	2,
	327,
	182,
	2,
	0
};

Compact sc76_door75 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	76,	// screen
	0,	// place
	0,	// getToTable
	174,	// xcood
	210,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	190-174,	// mouseSize_x
	275-210,	// mouseSize_y
	SC76_DOOR75_ACTION,	// actionScript
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
