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

#ifndef SKY3COMPACT_H
#define SKY3COMPACT_H




namespace Sky {

namespace SkyCompact {

uint16 rs_low_lift2[] = {
	C_XCOOD,
	292,
	C_YCOOD,
	210,
	C_FRAME,
	203*64,
	C_BASE_SUB+2,
	0,
	C_LOGIC,
	L_SCRIPT,
	C_STATUS,
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_NO_VMASK,
	65535
};

Compact low_lift_2 = {
	L_SCRIPT,	// logic
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_NO_VMASK,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	292,	// xcood
	210,	// ycood
	203*64,	// frame
	19+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	57,	// mouseRel_y
	48,	// mouseSize_x
	21,	// mouseSize_y
	TOP_LIFT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOW_LIFT2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_low_lift3[] = {
	C_XCOOD,
	275,
	C_YCOOD,
	286,
	C_FRAME,
	204*64,
	C_BASE_SUB+2,
	0,
	C_LOGIC,
	L_SCRIPT,
	C_STATUS,
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_NO_VMASK,
	65535
};

uint16 liftc3_down[] = {
	204*64,
	298,
	141,
	22,
	298,
	141,
	21,
	294,
	141,
	20,
	289,
	141,
	19,
	285,
	141,
	18,
	280,
	141,
	17,
	275,
	141,
	16,
	275,
	146,
	16,
	275,
	151,
	15,
	275,
	156,
	15,
	275,
	161,
	14,
	275,
	166,
	14,
	275,
	171,
	13,
	275,
	176,
	13,
	275,
	181,
	12,
	275,
	186,
	12,
	275,
	191,
	11,
	275,
	196,
	11,
	275,
	201,
	10,
	275,
	206,
	10,
	275,
	211,
	9,
	275,
	216,
	9,
	275,
	221,
	8,
	275,
	226,
	8,
	275,
	231,
	7,
	275,
	236,
	7,
	275,
	241,
	6,
	275,
	246,
	6,
	275,
	251,
	5,
	275,
	256,
	5,
	275,
	261,
	4,
	275,
	266,
	4,
	275,
	271,
	3,
	275,
	276,
	2,
	275,
	281,
	1,
	275,
	286,
	0,
	0,
	0
};

uint16 joey_list_s3[] = {
	296,
	360,
	248,
	321,
	1,
	0
};

uint16 liftc1_down[] = {
	202*64,
	250,
	141,
	15,
	250,
	146,
	14,
	250,
	151,
	14,
	250,
	155,
	13,
	250,
	160,
	13,
	250,
	164,
	12,
	250,
	169,
	12,
	250,
	173,
	11,
	250,
	178,
	11,
	250,
	182,
	10,
	250,
	187,
	10,
	250,
	191,
	9,
	250,
	196,
	9,
	250,
	200,
	8,
	250,
	205,
	8,
	250,
	209,
	7,
	250,
	214,
	7,
	250,
	218,
	6,
	250,
	223,
	6,
	250,
	227,
	5,
	250,
	232,
	5,
	250,
	236,
	4,
	250,
	241,
	4,
	250,
	245,
	3,
	250,
	250,
	3,
	250,
	254,
	2,
	250,
	259,
	2,
	250,
	263,
	1,
	250,
	268,
	0,
	0,
	0
};

uint16 shades_l_to_r[] = {
	41-8+217*64,
	40-8+217*64,
	47-8+217*64,
	0
};

uint16 shades_st_talk[] = {
	43*64,
	1,
	1,
	0,
	0
};

uint16 shades_st_right[] = {
	217*64,
	1,
	1,
	38,
	0
};

Compact eye_bolt = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_NO_VMASK,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	0,	// frame
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
	EYE_BOLT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact lights1 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	10,	// xcood
	10,	// ycood
	212*64,	// frame
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
	LIGHTS1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_low_barrel[] = {
	C_XCOOD,
	10,
	C_YCOOD,
	10,
	C_FRAME,
	202*64,
	C_STATUS,
	ST_LOGIC+ST_RECREATE+ST_NO_VMASK,
	C_LOGIC,
	L_SCRIPT,
	C_BASE_SUB+2,
	0,
	65535
};

Compact convey = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	0,	// frame
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
	CONVEY_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 search_seq[] = {
	169*64,
	289,
	231,
	0,
	289,
	231,
	1,
	289,
	231,
	2,
	289,
	231,
	3,
	289,
	231,
	4,
	289,
	231,
	5,
	289,
	231,
	4,
	289,
	231,
	6,
	289,
	231,
	7,
	289,
	231,
	8,
	289,
	231,
	8,
	289,
	231,
	8,
	289,
	231,
	8,
	289,
	231,
	8,
	289,
	231,
	8,
	289,
	231,
	8,
	289,
	231,
	7,
	289,
	231,
	6,
	289,
	231,
	4,
	289,
	231,
	1,
	289,
	231,
	0,
	0
};

ExtCompact eye_ball_ext = {
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
	0
};

Compact eye_ball = {
	L_SCRIPT,	// logic
	ST_FOREGROUND+ST_RECREATE+ST_LOGIC+ST_NO_VMASK+ST_MOUSE,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	201,	// xcood
	165,	// ycood
	213*64,	// frame
	4218,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	25,	// mouseSize_x
	70,	// mouseSize_y
	EYE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	EYE_BALL_LOGIC,	// baseSub
	0,	// baseSub_off
	&eye_ball_ext
};

uint32 *sh = (uint32*)&shades;

uint16 liftc3_up[] = {
	204*64,
	275,
	286,
	0,
	275,
	281,
	1,
	275,
	276,
	2,
	275,
	271,
	3,
	275,
	266,
	4,
	275,
	261,
	4,
	275,
	256,
	5,
	275,
	251,
	5,
	275,
	246,
	6,
	275,
	241,
	6,
	275,
	236,
	7,
	275,
	231,
	7,
	275,
	226,
	8,
	275,
	221,
	8,
	275,
	216,
	9,
	275,
	211,
	9,
	275,
	206,
	10,
	275,
	201,
	10,
	275,
	196,
	11,
	275,
	191,
	11,
	275,
	186,
	12,
	275,
	181,
	12,
	275,
	176,
	13,
	275,
	171,
	13,
	275,
	166,
	14,
	275,
	161,
	14,
	275,
	156,
	15,
	275,
	151,
	15,
	275,
	146,
	16,
	275,
	141,
	16,
	280,
	141,
	17,
	285,
	141,
	18,
	289,
	141,
	19,
	294,
	141,
	20,
	298,
	141,
	21,
	298,
	141,
	22,
	0
};

uint16 shades_l_to_u[] = {
	43-8+217*64,
	0
};

uint16 shades_u_to_d[] = {
	43-8+217*64,
	42-8+217*64,
	41-8+217*64,
	0
};

uint16 s3_mouse[] = {
	ID_FURNACE_DOOR,
	ID_STEVE_SPY,
	ID_JOEY,
	ID_SLOT,
	ID_SMOULDER,
	ID_EYE_BALL,
	ID_FURNACE,
	ID_S3_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 shades_d_to_u[] = {
	47-8+217*64,
	46-8+217*64,
	45-8+217*64,
	0
};

MegaSet smoulder_megaSet0 = {
	7,	// gridWidth
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
	0
};

ExtCompact smoulder_ext = {
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
	&smoulder_megaSet0,
	0,
	0,
	0
};

Compact smoulder = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	373+24,	// xcood
	289,	// ycood
	0,	// frame
	4220,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65512,	// mouseRel_x
	(int16) 65532,	// mouseRel_y
	60,	// mouseSize_x
	10,	// mouseSize_y
	SMOULDER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SMOULDER_LOGIC,	// baseSub
	0,	// baseSub_off
	&smoulder_ext
};

Compact low_lift_3 = {
	L_SCRIPT,	// logic
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_NO_VMASK,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	275,	// xcood
	286,	// ycood
	204*64,	// frame
	19+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	57,	// mouseRel_y
	48,	// mouseSize_x
	21,	// mouseSize_y
	TOP_LIFT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOW_LIFT3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 shades_u_to_r[] = {
	45-8+217*64,
	0
};

Compact furnace = {
	L_SCRIPT,	// logic
	ST_SORT+ST_LOGIC+ST_MOUSE+ST_RECREATE,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	226,	// xcood
	240,	// ycood
	211*64,	// frame
	4221,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	40,	// mouseSize_y
	FURNACE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FURNACE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 liftc1_up[] = {
	202*64,
	250,
	268,
	0,
	250,
	263,
	1,
	250,
	259,
	2,
	250,
	254,
	2,
	250,
	250,
	3,
	250,
	245,
	3,
	250,
	241,
	4,
	250,
	236,
	4,
	250,
	232,
	5,
	250,
	227,
	5,
	250,
	223,
	6,
	250,
	218,
	6,
	250,
	214,
	7,
	250,
	209,
	7,
	250,
	205,
	8,
	250,
	200,
	8,
	250,
	196,
	9,
	250,
	191,
	9,
	250,
	187,
	10,
	250,
	182,
	10,
	250,
	178,
	11,
	250,
	173,
	11,
	250,
	169,
	12,
	250,
	164,
	12,
	250,
	160,
	13,
	250,
	155,
	13,
	250,
	151,
	14,
	250,
	146,
	14,
	250,
	141,
	15,
	0
};

uint16 shades_r_to_d[] = {
	47-8+217*64,
	0
};

uint16 s3_logic[] = {
	ID_STEVE_SPY,
	ID_FOSTER,
	ID_LOW_LIFT,
	4350,
	4351,
	ID_CONVEY,
	ID_FURNACE,
	ID_LOW_BARREL,
	ID_JOEY_FLY,
	ID_JOEY,
	ID_LIGHTS1,
	ID_FURNACE_DOOR,
	ID_EYE_BALL,
	ID_SHADES,
	ID_EYE_BOLT,
	ID_SMOULDER,
	105,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 shades_st_up[] = {
	217*64,
	1,
	1,
	36,
	0
};

uint16 smoulder_seq[] = {
	225*64,
	373,
	269,
	0,
	373,
	269,
	1,
	373,
	269,
	1,
	373,
	269,
	2,
	373,
	269,
	2,
	373,
	269,
	3,
	373,
	269,
	3,
	373,
	269,
	4,
	373,
	269,
	4,
	373,
	269,
	0,
	0
};

uint16 shades_auto[32];

uint16 shades_up[] = {
	2,
	0+217*64,
	0,
	65534,
	2,
	1+217*64,
	0,
	65534,
	2,
	2+217*64,
	0,
	65534,
	2,
	3+217*64,
	0,
	65534,
	2,
	4+217*64,
	0,
	65534,
	2,
	5+217*64,
	0,
	65534,
	2,
	6+217*64,
	0,
	65534,
	2,
	7+217*64,
	0,
	65534,
	0
};

uint16 shades_down[] = {
	2,
	8+217*64,
	0,
	2,
	2,
	9+217*64,
	0,
	2,
	2,
	10+217*64,
	0,
	2,
	2,
	11+217*64,
	0,
	2,
	2,
	12+217*64,
	0,
	2,
	2,
	13+217*64,
	0,
	2,
	2,
	14+217*64,
	0,
	2,
	2,
	15+217*64,
	0,
	2,
	0
};

uint16 shades_left[] = {
	4,
	16+217*64,
	65532,
	0,
	4,
	17+217*64,
	65532,
	0,
	4,
	18+217*64,
	65532,
	0,
	4,
	19+217*64,
	65532,
	0,
	4,
	20+217*64,
	65532,
	0,
	4,
	21+217*64,
	65532,
	0,
	4,
	22+217*64,
	65532,
	0,
	4,
	23+217*64,
	65532,
	0,
	0
};

uint16 shades_right[] = {
	4,
	24+217*64,
	4,
	0,
	4,
	25+217*64,
	4,
	0,
	4,
	26+217*64,
	4,
	0,
	4,
	27+217*64,
	4,
	0,
	4,
	28+217*64,
	4,
	0,
	4,
	29+217*64,
	4,
	0,
	4,
	30+217*64,
	4,
	0,
	4,
	31+217*64,
	4,
	0,
	0
};

uint16 shades_st_down[] = {
	217*64,
	1,
	1,
	32,
	0
};

uint16 shades_st_left[] = {
	217*64,
	1,
	1,
	34,
	0
};

uint16 shades_u_to_l[] = {
	43-8+217*64,
	0
};

uint16 shades_d_to_l[] = {
	41-8+217*64,
	0
};

uint16 shades_d_to_r[] = {
	47-8+217*64,
	0
};

uint16 shades_l_to_d[] = {
	41-8+217*64,
	0
};

uint16 shades_r_to_u[] = {
	45-8+217*64,
	0
};

uint16 shades_r_to_l[] = {
	45-8+217*64,
	44-8+217*64,
	43-8+217*64,
	0
};

TurnTable shades_turnTable0 = {
	{ // turnTableUp
		0,
		shades_u_to_d,
		shades_u_to_l,
		shades_u_to_r,
		0
	},
	{ // turnTableDown
		shades_d_to_u,
		0,
		shades_d_to_l,
		shades_d_to_r,
		0
	},
	{ // turnTableLeft
		shades_l_to_u,
		shades_l_to_d,
		0,
		shades_l_to_r,
		0
	},
	{ // turnTableRight
		shades_r_to_u,
		shades_r_to_d,
		shades_r_to_l,
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

MegaSet shades_megaSet0 = {
	0,	// gridWidth
	0,	// colOffset
	8,	// colWidth
	8,	// lastChr
	shades_up,	// animUp
	shades_down,	// animDown
	shades_left,	// animLeft
	shades_right,	// animRight
	shades_st_up,	// standUp
	shades_st_down,	// standDown
	shades_st_left,	// standLeft
	shades_st_right,	// standRight
	shades_st_talk,	// standTalk
	&shades_turnTable0
};

ExtCompact shades_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	0,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96,	// spWidth_xx
	SP_COL_SHADES,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	shades_auto,	// animScratch
	0,	// megaSet
	&shades_megaSet0,
	0,
	0,
	0
};

Compact shades = {
	L_SCRIPT,	// logic
	ST_GRID_PLOT+ST_LOGIC,	// status
	0,	// sync
	3,	// screen
	ID_S3_FLOOR,	// place
	0,	// getToTable
	424,	// xcood
	280,	// ycood
	217*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	MEGA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	SHADES_LOGIC,	// baseSub
	0,	// baseSub_off
	&shades_ext
};

uint16 liftc2_down[] = {
	203*64,
	292,
	141,
	21,
	292,
	141,
	20,
	292,
	141,
	19,
	292,
	141,
	18,
	292,
	141,
	17,
	292,
	141,
	16,
	292,
	141,
	15,
	292,
	141,
	14,
	292,
	141,
	13,
	292,
	141,
	12,
	292,
	141,
	11,
	292,
	141,
	10,
	292,
	141,
	9,
	292,
	141,
	8,
	292,
	146,
	8,
	292,
	151,
	7,
	292,
	156,
	7,
	292,
	160,
	6,
	292,
	165,
	6,
	292,
	169,
	5,
	292,
	174,
	5,
	292,
	178,
	4,
	292,
	183,
	4,
	292,
	187,
	3,
	292,
	192,
	3,
	292,
	196,
	2,
	292,
	201,
	2,
	292,
	205,
	1,
	292,
	210,
	0,
	0,
	0
};

uint16 rs_start_joey_fly[] = {
	C_STATUS,
	ST_SORT+ST_LOGIC+ST_RECREATE,
	65535
};

Compact steve_watch = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	244,	// xcood
	196,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	1,	// mouseSize_x
	1,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	STEVE_SPY_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 liftc2_up[] = {
	203*64,
	292,
	210,
	0,
	292,
	205,
	1,
	292,
	201,
	2,
	292,
	196,
	2,
	292,
	192,
	3,
	292,
	187,
	3,
	292,
	183,
	4,
	292,
	178,
	4,
	292,
	174,
	5,
	292,
	169,
	5,
	292,
	165,
	6,
	292,
	160,
	6,
	292,
	156,
	7,
	292,
	151,
	7,
	292,
	146,
	8,
	292,
	141,
	8,
	292,
	141,
	9,
	292,
	141,
	10,
	292,
	141,
	11,
	292,
	141,
	12,
	292,
	141,
	13,
	292,
	141,
	14,
	292,
	141,
	15,
	292,
	141,
	16,
	292,
	141,
	17,
	292,
	141,
	18,
	292,
	141,
	19,
	292,
	141,
	20,
	292,
	141,
	21,
	0
};

Compact furnace_door = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	0X18E,	// xcood
	0XDE,	// ycood
	215*64,	// frame
	181,	// cursorText
	FURNACE_EXIT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	60,	// mouseSize_y
	FURNACE_D_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FURNACE_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact slot = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	385,	// xcood
	233,	// ycood
	0,	// frame
	4219,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	15,	// mouseSize_y
	SLOT_ACTION,	// actionScript
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

uint16 chip_list_sc3[] = {
	173,
	111,
	227,
	202+DISK_1,
	203+DISK_1,
	204+DISK_1,
	205+DISK_1,
	206+DISK_1,
	207+DISK_1,
	208+DISK_1,
	212+DISK_1,
	216+DISK_1+0X8000,
	215+DISK_1,
	209+0X8000+DISK_1,
	210+0X8000+DISK_1,
	217+DISK_1,
	214+DISK_1,
	225+DISK_1,
	218+DISK_1+0X8000,
	169+DISK_1+0X8000,
	0
};

Compact low_lift = {
	L_SCRIPT,	// logic
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_NO_VMASK,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	250,	// xcood
	268,	// ycood
	202*64,	// frame
	19+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	57,	// mouseRel_y
	48,	// mouseSize_x
	21,	// mouseSize_y
	TOP_LIFT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LOW_LIFT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact joey_fly = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	209*64,	// frame
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
	FLY_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_low_lift[] = {
	C_XCOOD,
	250,
	C_YCOOD,
	268,
	C_FRAME,
	202*64,
	C_BASE_SUB+2,
	0,
	C_LOGIC,
	L_SCRIPT,
	C_STATUS,
	ST_SORT+ST_LOGIC+ST_RECREATE+ST_NO_VMASK,
	65535
};

uint16 r3_floor_table[] = {
	ID_S3_FLOOR,
	RET_OK,
	ID_FURNACE_DOOR,
	GET_TO_FURNACE_DOOR,
	ID_SLOT,
	GET_TO_SLOT,
	ID_SMOULDER,
	GET_TO_BODY,
	ID_EYE_BALL,
	GET_TO_EYE,
	ID_FURNACE,
	GET_TO_FURNACE,
	ID_JOEY_PARK,
	GET_TO_JP2,
	65535
};

Compact s3_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	3,	// screen
	0,	// place
	r3_floor_table,	// getToTable
	176,	// xcood
	240,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	256,	// mouseSize_x
	79,	// mouseSize_y
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

uint16 rs_joey_to_furnace[] = {
	C_XCOOD,
	344,
	C_YCOOD,
	264,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,
	C_PLACE,
	ID_S3_FLOOR,
	C_SCREEN,
	3,
	C_MODE,
	0,
	C_BASE_SUB,
	JOEY_LOGIC,
	C_BASE_SUB+2,
	0,
	C_DIR,
	RIGHT,
	C_FRAME,
	173*64+12,
	65535
};

Compact low_barrel = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE+ST_NO_VMASK,	// status
	0,	// sync
	3,	// screen
	0,	// place
	0,	// getToTable
	10,	// xcood
	10,	// ycood
	202*64,	// frame
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
	LOW_BARREL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
