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

#ifndef SKY27COMP_H
#define SKY27COMP_H




namespace Sky {

namespace SkyCompact {

uint16 s27_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_BURKE,
	ID_SCANNER_27,
	ID_HELMET_COLE,
	ID_MEDI_COMP,
	ID_BODY,
	ID_GLASS_SLOT,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_LIFT_29,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 scan_27_seq[] = {
	159*64,
	308,
	212,
	0,
	308,
	212,
	1,
	0
};

uint16 body_table[] = {
	ID_CHAIR_27,
	GT_CHAIR_27,
	ID_GAS,
	GT_GAS,
	ID_STD_RIGHT_TALK,
	GT_R_TALK_27,
	65535
};

uint16 glass_scan[] = {
	212*64,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	2,
	194,
	264,
	3,
	194,
	264,
	4,
	194,
	264,
	5,
	194,
	264,
	6,
	194,
	264,
	7,
	194,
	264,
	8,
	194,
	264,
	9,
	194,
	264,
	10,
	194,
	264,
	11,
	194,
	264,
	12,
	194,
	264,
	13,
	194,
	264,
	14,
	194,
	264,
	15,
	194,
	264,
	16,
	194,
	264,
	17,
	194,
	264,
	18,
	194,
	264,
	19,
	194,
	264,
	20,
	194,
	264,
	21,
	194,
	264,
	22,
	194,
	264,
	23,
	194,
	264,
	24,
	194,
	264,
	25,
	194,
	264,
	26,
	194,
	264,
	27,
	194,
	264,
	28,
	194,
	264,
	29,
	194,
	264,
	30,
	194,
	264,
	31,
	194,
	264,
	32,
	194,
	264,
	7,
	194,
	264,
	8,
	194,
	264,
	9,
	194,
	264,
	10,
	194,
	264,
	11,
	194,
	264,
	33,
	194,
	264,
	34,
	194,
	264,
	35,
	194,
	264,
	36,
	194,
	264,
	37,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	9,
	194,
	264,
	9,
	194,
	264,
	9,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	9,
	194,
	264,
	9,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	38,
	194,
	264,
	39,
	194,
	264,
	40,
	194,
	264,
	41,
	194,
	264,
	42,
	0
};

uint16 burke_auto[32];

uint16 put_glass_seq[] = {
	203*64,
	241,
	250,
	0,
	236,
	254,
	1,
	235,
	262,
	2,
	233,
	262,
	3,
	233,
	262,
	3,
	231,
	262,
	4,
	229,
	262,
	5,
	227,
	262,
	6,
	227,
	262,
	6,
	227,
	262,
	6,
	231,
	262,
	4,
	233,
	262,
	3,
	234,
	262,
	7,
	234,
	262,
	8,
	234,
	262,
	8,
	234,
	262,
	8,
	234,
	262,
	8,
	234,
	262,
	8,
	234,
	262,
	8,
	234,
	262,
	8,
	234,
	262,
	7,
	230,
	262,
	9,
	225,
	262,
	10,
	224,
	262,
	11,
	226,
	262,
	12,
	227,
	262,
	13,
	232,
	262,
	14,
	234,
	262,
	7,
	235,
	262,
	2,
	236,
	254,
	1,
	241,
	250,
	15,
	0
};

uint16 burke_u_to_d[] = {
	43-8+150*64,
	42-8+150*64,
	41-8+150*64,
	0
};

uint16 burke_l_to_u[] = {
	43-8+150*64,
	0
};

Compact helmet_cole = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE+ST_FOREGROUND,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	232,	// xcood
	207,	// ycood
	162*64,	// frame
	12798,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	253-232,	// mouseSize_x
	226-207,	// mouseSize_y
	HELMET_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	HELMET_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_27_26[] = {
	C_XCOOD,
	160,
	C_YCOOD,
	288,
	C_SCREEN,
	26,
	C_PLACE,
	ID_S26_FLOOR,
	65535
};

uint16 burke_d_to_u[] = {
	47-8+150*64,
	46-8+150*64,
	45-8+150*64,
	0
};

uint16 burke_st_talk[] = {
	151*64,
	1,
	1,
	0,
	0
};

uint16 out_chair_seq[] = {
	161*64,
	233,
	235,
	0,
	231,
	235,
	1,
	230,
	235,
	2,
	230,
	235,
	3,
	230,
	234,
	4,
	230,
	234,
	5,
	230,
	236,
	6,
	237,
	236,
	7,
	240,
	236,
	8,
	246,
	234,
	9,
	246,
	235,
	10,
	248,
	235,
	11,
	254,
	234,
	12,
	258,
	234,
	13,
	0
};

uint16 burke_st_right[] = {
	150*64,
	1,
	1,
	38,
	0
};

uint16 burke_u_to_l[] = {
	43-8+150*64,
	0
};

uint16 hand_op_seq[] = {
	210*64,
	216,
	240,
	0,
	216,
	240,
	1,
	216,
	240,
	2,
	216,
	240,
	3,
	216,
	240,
	4,
	216,
	240,
	3,
	216,
	240,
	4,
	216,
	240,
	2,
	216,
	240,
	1,
	216,
	240,
	5,
	216,
	240,
	6,
	216,
	240,
	5,
	216,
	240,
	2,
	216,
	240,
	4,
	216,
	240,
	3,
	216,
	240,
	4,
	216,
	240,
	2,
	216,
	240,
	3,
	216,
	240,
	2,
	216,
	240,
	1,
	216,
	240,
	2,
	216,
	240,
	3,
	216,
	240,
	4,
	216,
	240,
	5,
	216,
	240,
	6,
	216,
	240,
	5,
	216,
	240,
	6,
	216,
	240,
	5,
	216,
	240,
	1,
	216,
	240,
	2,
	216,
	240,
	7,
	216,
	240,
	8,
	216,
	240,
	8,
	216,
	240,
	9,
	216,
	240,
	10,
	216,
	240,
	11,
	216,
	240,
	12,
	216,
	240,
	13,
	216,
	240,
	14,
	216,
	240,
	15,
	0
};

uint16 helmet_up[] = {
	162*64,
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

uint16 hand_in_seq[] = {
	209*64,
	216,
	240,
	0,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	2,
	216,
	240,
	2,
	216,
	240,
	3,
	216,
	240,
	3,
	216,
	240,
	4,
	216,
	240,
	4,
	216,
	240,
	4,
	216,
	240,
	4,
	216,
	240,
	4,
	216,
	240,
	4,
	216,
	240,
	4,
	216,
	240,
	4,
	0
};

uint16 burke_r_to_u[] = {
	45-8+150*64,
	0
};

uint16 burke_left[] = {
	4,
	16+150*64,
	65532,
	0,
	4,
	17+150*64,
	65532,
	0,
	4,
	18+150*64,
	65532,
	0,
	4,
	19+150*64,
	65532,
	0,
	4,
	20+150*64,
	65532,
	0,
	4,
	21+150*64,
	65532,
	0,
	4,
	22+150*64,
	65532,
	0,
	4,
	23+150*64,
	65532,
	0,
	0
};

uint16 burke_st_up[] = {
	150*64,
	1,
	1,
	36,
	0
};

uint16 burke_st_left[] = {
	150*64,
	1,
	1,
	34,
	0
};

uint16 chair_table[] = {
	ID_CHAIR_27,
	RET_OK,
	ID_S27_FLOOR,
	GT_FLOOR_FROM_CHAIR,
	ID_RIGHT_EXIT_27,
	GT_FLOOR_FROM_CHAIR,
	ID_STD_LEFT_TALK,
	GT_FLOOR_FROM_CHAIR,
	ID_CHART1,
	GT_FLOOR_FROM_CHAIR,
	ID_CHART2,
	GT_FLOOR_FROM_CHAIR,
	ID_GAS,
	GT_FLOOR_FROM_CHAIR,
	ID_SCANNER_27,
	GT_FLOOR_FROM_CHAIR,
	ID_MEDI_COMP,
	GT_FLOOR_FROM_CHAIR,
	ID_BODY,
	GT_FLOOR_FROM_CHAIR,
	ID_HELMET_COLE,
	GT_FLOOR_FROM_CHAIR,
	65535
};

Compact gas = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	276,	// xcood
	269,	// ycood
	0,	// frame
	12797,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	294-276,	// mouseSize_x
	326-269,	// mouseSize_y
	GAS_ACTION,	// actionScript
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

uint16 lung_seq[] = {
	157*64,
	308,
	212,
	0,
	308,
	212,
	1,
	308,
	212,
	2,
	308,
	212,
	3,
	308,
	212,
	4,
	308,
	212,
	5,
	308,
	212,
	6,
	308,
	212,
	7,
	308,
	212,
	8,
	308,
	212,
	9,
	308,
	212,
	10,
	308,
	212,
	11,
	308,
	212,
	12,
	308,
	212,
	13,
	308,
	212,
	14,
	308,
	212,
	15,
	308,
	212,
	14,
	308,
	212,
	13,
	308,
	212,
	12,
	308,
	212,
	11,
	308,
	212,
	16,
	308,
	212,
	11,
	308,
	212,
	16,
	308,
	212,
	13,
	308,
	212,
	12,
	308,
	212,
	15,
	308,
	212,
	14,
	308,
	212,
	15,
	308,
	212,
	12,
	308,
	212,
	13,
	308,
	212,
	16,
	308,
	212,
	9,
	308,
	212,
	17,
	308,
	212,
	5,
	308,
	212,
	18,
	308,
	212,
	19,
	308,
	212,
	20,
	308,
	212,
	21,
	0
};

uint16 burke_l_to_r[] = {
	41-8+150*64,
	40-8+150*64,
	47-8+150*64,
	0
};

uint16 glass_slot_open[] = {
	204*64,
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
	0
};

ExtCompact body_ext = {
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
	96,	// spWidth_xx
	SP_COL_BODY,	// spColour
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

Compact body = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	body_table,	// getToTable
	353,	// xcood
	283,	// ycood
	169*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	10,	// mouseSize_y
	BODY_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	BODY_SSS,	// baseSub
	0,	// baseSub_off
	&body_ext
};

uint16 glove_off_seq[] = {
	154*64,
	328,
	261,
	6,
	328,
	261,
	5,
	328,
	261,
	4,
	328,
	261,
	3,
	328,
	261,
	2,
	328,
	261,
	1,
	328,
	261,
	0,
	0
};

uint16 rs_small_burke[] = {
	C_MOUSE_REL_X,
	0,
	C_MOUSE_REL_Y,
	0,
	65535
};

uint16 burke_d_to_l[] = {
	41-8+150*64,
	0
};

uint16 rs_foster_26_27[] = {
	C_XCOOD,
	392,
	C_YCOOD,
	256,
	C_SCREEN,
	27,
	C_PLACE,
	ID_S27_FLOOR,
	65535
};

uint32 *grid27 = 0;

uint16 burke_up[] = {
	2,
	0+150*64,
	0,
	65534,
	2,
	1+150*64,
	0,
	65534,
	2,
	2+150*64,
	0,
	65534,
	2,
	3+150*64,
	0,
	65534,
	2,
	4+150*64,
	0,
	65534,
	2,
	5+150*64,
	0,
	65534,
	2,
	6+150*64,
	0,
	65534,
	2,
	7+150*64,
	0,
	65534,
	0
};

uint16 operate_seq[] = {
	155*64,
	326,
	260,
	0,
	326,
	260,
	0,
	326,
	260,
	1,
	326,
	260,
	1,
	326,
	260,
	1,
	326,
	260,
	1,
	326,
	260,
	1,
	326,
	260,
	2,
	326,
	260,
	3,
	326,
	260,
	3,
	326,
	260,
	4,
	326,
	260,
	4,
	326,
	260,
	5,
	326,
	260,
	6,
	326,
	260,
	6,
	326,
	260,
	6,
	326,
	260,
	7,
	326,
	260,
	7,
	326,
	260,
	7,
	326,
	260,
	7,
	326,
	260,
	8,
	326,
	260,
	8,
	326,
	260,
	8,
	326,
	260,
	8,
	326,
	260,
	7,
	326,
	260,
	7,
	326,
	260,
	6,
	326,
	260,
	6,
	326,
	260,
	4,
	326,
	260,
	4,
	326,
	260,
	5,
	326,
	260,
	5,
	326,
	260,
	9,
	326,
	260,
	9,
	326,
	260,
	10,
	326,
	260,
	10,
	326,
	260,
	11,
	326,
	260,
	12,
	326,
	260,
	11,
	326,
	260,
	11,
	326,
	260,
	0,
	0
};

uint16 s27_floor_table[] = {
	ID_S27_FLOOR,
	RET_OK,
	ID_RIGHT_EXIT_27,
	GT_RIGHT_EXIT_27,
	ID_JOEY_PARK,
	GET_TO_JP2,
	ID_STD_LEFT_TALK,
	GT_L_TALK_27,
	ID_STD_RIGHT_TALK,
	GT_R_TALK_27,
	0,
	S27_RIGHT_ON,
	ID_CHART1,
	GT_CHART1,
	ID_CHART2,
	GT_CHART2,
	ID_GAS,
	GT_GAS,
	ID_SCANNER_27,
	GT_SCANNER_27,
	ID_MEDI_COMP,
	GT_MEDI_COMP,
	ID_CHAIR_27,
	GT_CHAIR_27,
	ID_HELMET_COLE,
	GT_HELMET,
	ID_BODY,
	GT_BODY,
	65535
};

uint16 helmet_down[] = {
	162*64,
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
	0
};

uint32 *heart_seq = (uint32*)lung_seq;

uint16 squirm_seq[] = {
	164*64,
	230,
	234,
	0,
	230,
	234,
	1,
	230,
	234,
	2,
	230,
	234,
	3,
	230,
	234,
	4,
	230,
	234,
	4,
	230,
	234,
	2,
	230,
	234,
	2,
	230,
	234,
	5,
	230,
	234,
	6,
	230,
	234,
	5,
	230,
	234,
	2,
	230,
	234,
	5,
	230,
	234,
	6,
	230,
	234,
	5,
	230,
	234,
	2,
	230,
	234,
	1,
	230,
	234,
	3,
	230,
	234,
	4,
	230,
	234,
	3,
	230,
	234,
	2,
	230,
	234,
	5,
	230,
	234,
	2,
	230,
	234,
	3,
	230,
	234,
	4,
	230,
	234,
	2,
	230,
	234,
	5,
	230,
	234,
	6,
	230,
	234,
	5,
	230,
	234,
	2,
	230,
	234,
	2,
	230,
	234,
	3,
	230,
	234,
	2,
	230,
	234,
	3,
	230,
	234,
	4,
	230,
	234,
	3,
	230,
	234,
	4,
	230,
	234,
	3,
	230,
	234,
	5,
	230,
	234,
	6,
	230,
	234,
	5,
	230,
	234,
	6,
	230,
	234,
	5,
	230,
	234,
	2,
	230,
	234,
	0,
	230,
	234,
	0,
	230,
	234,
	7,
	0
};

uint16 burke_r_to_d[] = {
	47-8+150*64,
	0
};

uint16 glove_on_seq[] = {
	154*64,
	328,
	261,
	0,
	328,
	261,
	1,
	328,
	261,
	2,
	328,
	261,
	3,
	328,
	261,
	4,
	328,
	261,
	5,
	328,
	261,
	6,
	0
};

Compact chart2 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	395,	// xcood
	201,	// ycood
	0,	// frame
	12778,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	416-395,	// mouseSize_x
	251-201,	// mouseSize_y
	CHART2_ACTION,	// actionScript
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

uint16 burke_r_to_l[] = {
	45-8+150*64,
	44-8+150*64,
	43-8+150*64,
	0
};

Compact chart1 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	263,	// xcood
	193,	// ycood
	0,	// frame
	12778,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	289-263,	// mouseSize_x
	241-193,	// mouseSize_y
	CHART1_ACTION,	// actionScript
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

uint16 rs_big_burke[] = {
	C_MOUSE_REL_X,
	65520,
	C_MOUSE_REL_Y,
	65488,
	65535
};

uint16 burke_down[] = {
	2,
	8+150*64,
	0,
	2,
	2,
	9+150*64,
	0,
	2,
	2,
	10+150*64,
	0,
	2,
	2,
	11+150*64,
	0,
	2,
	2,
	12+150*64,
	0,
	2,
	2,
	13+150*64,
	0,
	2,
	2,
	14+150*64,
	0,
	2,
	2,
	15+150*64,
	0,
	2,
	0
};

uint16 medi_comp_seq[] = {
	165*64,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	2,
	194,
	264,
	3,
	194,
	264,
	4,
	194,
	264,
	5,
	194,
	264,
	6,
	194,
	264,
	7,
	194,
	264,
	8,
	194,
	264,
	9,
	194,
	264,
	10,
	194,
	264,
	11,
	194,
	264,
	12,
	194,
	264,
	13,
	194,
	264,
	14,
	194,
	264,
	15,
	194,
	264,
	16,
	194,
	264,
	17,
	194,
	264,
	18,
	194,
	264,
	17,
	194,
	264,
	19,
	194,
	264,
	20,
	194,
	264,
	21,
	194,
	264,
	22,
	194,
	264,
	23,
	194,
	264,
	24,
	194,
	264,
	25,
	194,
	264,
	26,
	194,
	264,
	27,
	194,
	264,
	28,
	194,
	264,
	29,
	194,
	264,
	30,
	194,
	264,
	31,
	194,
	264,
	30,
	194,
	264,
	31,
	194,
	264,
	30,
	194,
	264,
	31,
	194,
	264,
	30,
	194,
	264,
	31,
	194,
	264,
	30,
	194,
	264,
	31,
	194,
	264,
	30,
	194,
	264,
	32,
	194,
	264,
	30,
	194,
	264,
	32,
	194,
	264,
	30,
	194,
	264,
	32,
	194,
	264,
	30,
	0
};

uint16 helmet_op_seq[] = {
	163*64,
	232,
	207,
	0,
	232,
	207,
	1,
	232,
	207,
	1,
	232,
	207,
	2,
	232,
	207,
	2,
	232,
	207,
	3,
	232,
	207,
	3,
	232,
	207,
	4,
	232,
	207,
	4,
	232,
	207,
	5,
	232,
	207,
	5,
	232,
	207,
	5,
	232,
	207,
	5,
	232,
	207,
	6,
	232,
	207,
	6,
	232,
	207,
	3,
	232,
	207,
	3,
	232,
	207,
	2,
	232,
	207,
	2,
	232,
	207,
	1,
	232,
	207,
	1,
	232,
	207,
	7,
	232,
	207,
	7,
	232,
	207,
	7,
	232,
	207,
	7,
	232,
	207,
	1,
	232,
	207,
	1,
	232,
	207,
	2,
	232,
	207,
	2,
	232,
	207,
	3,
	232,
	207,
	3,
	232,
	207,
	4,
	232,
	207,
	4,
	232,
	207,
	5,
	232,
	207,
	5,
	232,
	207,
	5,
	232,
	207,
	5,
	232,
	207,
	6,
	232,
	207,
	6,
	232,
	207,
	3,
	232,
	207,
	3,
	232,
	207,
	2,
	232,
	207,
	2,
	232,
	207,
	1,
	232,
	207,
	1,
	232,
	207,
	7,
	232,
	207,
	7,
	0
};

uint16 gas_seq[] = {
	153*64,
	288,
	256,
	0,
	288,
	256,
	1,
	288,
	256,
	2,
	288,
	256,
	3,
	288,
	256,
	4,
	288,
	256,
	4,
	288,
	256,
	4,
	288,
	256,
	3,
	288,
	256,
	5,
	288,
	256,
	6,
	288,
	256,
	7,
	288,
	256,
	8,
	288,
	256,
	9,
	288,
	256,
	10,
	288,
	256,
	10,
	288,
	256,
	10,
	288,
	256,
	10,
	288,
	256,
	11,
	288,
	256,
	12,
	288,
	256,
	12,
	288,
	256,
	12,
	288,
	256,
	12,
	288,
	256,
	7,
	288,
	256,
	6,
	288,
	256,
	5,
	288,
	256,
	3,
	288,
	256,
	4,
	288,
	256,
	4,
	288,
	256,
	4,
	288,
	256,
	3,
	288,
	256,
	2,
	288,
	256,
	1,
	288,
	256,
	0,
	0
};

uint16 burke_l_to_d[] = {
	41-8+150*64,
	0
};

uint16 glass_slot_close[] = {
	204*64,
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

uint16 s27_mouse[] = {
	ID_JOEY,
	ID_BURKE,
	ID_RIGHT_EXIT_27,
	ID_CHART1,
	ID_CHART2,
	ID_GAS,
	ID_SCANNER_27,
	ID_MEDI_COMP,
	ID_CHAIR_27,
	ID_BODY,
	ID_HELMET_COLE,
	ID_S27_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 burke_right[] = {
	4,
	24+150*64,
	4,
	0,
	4,
	25+150*64,
	4,
	0,
	4,
	26+150*64,
	4,
	0,
	4,
	27+150*64,
	4,
	0,
	4,
	28+150*64,
	4,
	0,
	4,
	29+150*64,
	4,
	0,
	4,
	30+150*64,
	4,
	0,
	4,
	31+150*64,
	4,
	0,
	0
};

uint16 burke_st_down[] = {
	150*64,
	1,
	1,
	32,
	0
};

uint16 burke_u_to_r[] = {
	45-8+150*64,
	0
};

uint16 burke_d_to_r[] = {
	47-8+150*64,
	0
};

TurnTable burke_turnTable0 = {
	{ // turnTableUp
		0,
		burke_u_to_d,
		burke_u_to_l,
		burke_u_to_r,
		0
	},
	{ // turnTableDown
		burke_d_to_u,
		0,
		burke_d_to_l,
		burke_d_to_r,
		0
	},
	{ // turnTableLeft
		burke_l_to_u,
		burke_l_to_d,
		0,
		burke_l_to_r,
		0
	},
	{ // turnTableRight
		burke_r_to_u,
		burke_r_to_d,
		burke_r_to_l,
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

MegaSet burke_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	burke_up,	// animUp
	burke_down,	// animDown
	burke_left,	// animLeft
	burke_right,	// animRight
	burke_st_up,	// standUp
	burke_st_down,	// standDown
	burke_st_left,	// standLeft
	burke_st_right,	// standRight
	burke_st_talk,	// standTalk
	&burke_turnTable0
};

ExtCompact burke_ext = {
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
	SP_COL_BURKE,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	burke_auto,	// animScratch
	0,	// megaSet
	&burke_megaSet0,
	0,
	0,
	0
};

Compact burke = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_MOUSE+ST_GRID_PLOT,	// status
	0,	// sync
	27,	// screen
	ID_S27_FLOOR,	// place
	0,	// getToTable
	344,	// xcood
	304,	// ycood
	150*64,	// frame
	12694,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65520,	// mouseRel_x
	(int16) 65488,	// mouseRel_y
	32,	// mouseSize_x
	48,	// mouseSize_y
	MEGA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	BURKE_LOGIC,	// baseSub
	0,	// baseSub_off
	&burke_ext
};

uint16 shake_seq[] = {
	152*64,
	325,
	256,
	0,
	325,
	256,
	1,
	325,
	256,
	2,
	325,
	256,
	3,
	325,
	256,
	4,
	325,
	256,
	5,
	325,
	256,
	2,
	325,
	256,
	3,
	325,
	256,
	4,
	325,
	256,
	2,
	325,
	256,
	4,
	325,
	256,
	2,
	325,
	256,
	4,
	325,
	256,
	6,
	325,
	256,
	7,
	325,
	256,
	8,
	325,
	256,
	9,
	325,
	256,
	10,
	0
};

uint16 hands_out_seq[] = {
	213*64,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	1,
	216,
	240,
	1,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	0,
	216,
	240,
	1,
	216,
	240,
	2,
	216,
	240,
	3,
	216,
	240,
	4,
	0
};

uint16 s27_pal[] = {
	0,
	1280,
	1286,
	1545,
	1797,
	1800,
	2061,
	2054,
	2570,
	2570,
	2823,
	2571,
	2577,
	3079,
	2829,
	3343,
	2824,
	3342,
	4108,
	3594,
	3598,
	4109,
	4879,
	3085,
	4111,
	4364,
	3345,
	4373,
	5640,
	3600,
	4881,
	6416,
	2065,
	4627,
	4111,
	4373,
	5138,
	6420,
	3349,
	5651,
	7187,
	4370,
	5398,
	5140,
	5654,
	5401,
	7442,
	3348,
	6421,
	5136,
	4635,
	5151,
	5906,
	5911,
	6171,
	5391,
	5951,
	7191,
	6164,
	6426,
	5410,
	6932,
	5913,
	6175,
	8213,
	4122,
	6940,
	6426,
	6941,
	7962,
	7699,
	5915,
	6434,
	8983,
	4380,
	7963,
	8217,
	7453,
	8732,
	7703,
	6944,
	7717,
	10259,
	6427,
	8477,
	8735,
	6687,
	8992,
	8217,
	7458,
	7465,
	8988,
	7712,
	8486,
	8469,
	8483,
	9760,
	9755,
	6946,
	7981,
	8989,
	6952,
	9256,
	10782,
	8226,
	9509,
	10786,
	6437,
	10017,
	9252,
	7722,
	10277,
	10020,
	8231,
	9006,
	11296,
	6696,
	10536,
	9510,
	8237,
	9520,
	12066,
	6442,
	10795,
	11556,
	7467,
	11816,
	12578,
	9256,
	11561,
	11046,
	10283,
	11568,
	11803,
	9770,
	11058,
	13087,
	10025,
	12076,
	13095,
	7215,
	11823,
	11305,
	10801,
	11316,
	11561,
	9266,
	10807,
	13352,
	9520,
	12591,
	13612,
	8497,
	12593,
	12585,
	9780,
	11831,
	13866,
	9267,
	13616,
	14124,
	11313,
	13107,
	12846,
	10550,
	13617,
	14385,
	9781,
	13621,
	14640,
	11827,
	14131,
	13875,
	11065,
	14393,
	15401,
	10550,
	14646,
	14127,
	12855,
	14649,
	14388,
	12603,
	14909,
	16170,
	11579,
	15418,
	15673,
	14141,
	15932,
	16187,
	15679,
	0,
	2048,
	2570,
	2827,
	3082,
	2829,
	3598,
	3342,
	3856,
	4111,
	4364,
	4115,
	4627,
	4111,
	4373,
	5138,
	4884,
	4886,
	5652,
	5910,
	5911,
	6680,
	6937,
	5913,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	6688,
	10768,
	6437,
	5138,
	5908,
	5911,
	3340,
	2827,
	2571,
	12076,
	10023,
	8231,
	4377,
	6408,
	6941,
	4881,
	7440,
	3348,
	2061,
	4358,
	1802,
	9760,
	16155,
	16191,
	16189,
	13113,
	11575,
	12072,
	7970,
	6440,
	8213,
	3345,
	2841,
	4358,
	517,
	522,
	1285,
	16133,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	14399,
	13364,
	12338,
	11820,
	10028,
	8226,
	7452,
	5142,
	3605,
	1804,
	1799,
	1799,
	14343,
	9518,
	10035,
	11802,
	4383,
	6441,
	9226,
	786,
	3360,
	11264,
	2056,
	8210,
	7441,
	6954,
	9494,
	4119,
	3866,
	4864,
	12548,
	13361,
	10281,
	8236,
	8734,
	10284,
	9264,
	10271,
	5917,
	5664,
	6159,
	2319,
	2064,
	2052,
	13631,
	14376,
	6694,
	6189,
	8975,
	1805,
	1048,
	3586,
	0,
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
	13631,
	14376,
	6694,
	0,
	16128,
	0,
	16191,
	16191,
	63,
	16191,
	16128,
	63,
	16191,
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

uint16 s27_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	147+DISK_8,
	148+DISK_8,
	149+DISK_8,
	169+DISK_8,
	204+DISK_8,
	209+DISK_8,
	211+DISK_8,
	213+DISK_8,
	163+DISK_8+0X000,
	0
};

Compact medi_comp = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_SORT+ST_LOGIC,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	194,	// xcood
	264,	// ycood
	165*64,	// frame
	12668,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	235-194,	// mouseSize_x
	301-261,	// mouseSize_y
	MEDI_COMP_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	MEDIC_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact glass_slot = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	224,	// xcood
	285,	// ycood
	204*64,	// frame
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
	0,	// mode
	GLASS_SLOT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 in_chair_seq[] = {
	160*64,
	252,
	234,
	0,
	249,
	234,
	1,
	240,
	235,
	2,
	233,
	236,
	3,
	229,
	236,
	4,
	230,
	236,
	5,
	230,
	234,
	6,
	230,
	234,
	7,
	230,
	235,
	8,
	230,
	235,
	9,
	231,
	235,
	10,
	233,
	235,
	11,
	0
};

uint16 s27_chip_list[] = {
	150+DISK_8,
	151+DISK_8,
	152+DISK_8,
	153+DISK_8,
	154+DISK_8,
	155+DISK_8,
	159+DISK_8,
	162+DISK_8+0X000,
	164+DISK_8,
	165+DISK_8,
	167+DISK_8,
	168+DISK_8,
	143+DISK_8,
	144+DISK_8,
	145+DISK_8,
	146+DISK_8,
	0
};

uint16 kidney_seq[] = {
	158*64,
	308,
	212,
	0,
	308,
	212,
	1,
	308,
	212,
	2,
	308,
	212,
	3,
	308,
	212,
	4,
	308,
	212,
	3,
	308,
	212,
	5,
	308,
	212,
	6,
	308,
	212,
	7,
	308,
	212,
	8,
	308,
	212,
	9,
	308,
	212,
	10,
	308,
	212,
	11,
	308,
	212,
	12,
	308,
	212,
	13,
	308,
	212,
	14,
	308,
	212,
	15,
	308,
	212,
	16,
	308,
	212,
	15,
	308,
	212,
	16,
	308,
	212,
	15,
	308,
	212,
	16,
	308,
	212,
	15,
	308,
	212,
	16,
	308,
	212,
	15,
	308,
	212,
	16,
	308,
	212,
	15,
	308,
	212,
	16,
	308,
	212,
	15,
	308,
	212,
	14,
	308,
	212,
	13,
	308,
	212,
	12,
	308,
	212,
	11,
	308,
	212,
	10,
	308,
	212,
	9,
	308,
	212,
	8,
	308,
	212,
	7,
	308,
	212,
	6,
	308,
	212,
	5,
	308,
	212,
	3,
	308,
	212,
	4,
	308,
	212,
	3,
	308,
	212,
	4,
	308,
	212,
	2,
	308,
	212,
	1,
	308,
	212,
	0,
	0
};

Compact scanner_27 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	308,	// xcood
	212,	// ycood
	159*64,	// frame
	12799,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	329-308,	// mouseSize_x
	245-212,	// mouseSize_y
	SCANNER_27_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	SCANNER_27_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact s27_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	s27_floor_table,	// getToTable
	128,	// xcood
	240,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	4242,	// mouseSize_y
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

uint16 fprint_seq[] = {
	211*64,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	194,
	264,
	0,
	194,
	264,
	1,
	0
};

Compact right_exit_27 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	0,	// getToTable
	363,	// xcood
	202,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	382-363,	// mouseSize_x
	267-202,	// mouseSize_y
	ER27_ACTION,	// actionScript
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

Compact chair_27 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	27,	// screen
	0,	// place
	chair_table,	// getToTable
	230,	// xcood
	236,	// ycood
	0,	// frame
	12752,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	256-230,	// mouseSize_x
	283-236,	// mouseSize_y
	CHAIR_27_ACTION,	// actionScript
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
