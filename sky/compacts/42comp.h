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

#ifndef SKY42COMP_H
#define SKY42COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc42_jdg_hammer[] = {
	169*64,
	269,
	186,
	0,
	269,
	186,
	1,
	269,
	186,
	2,
	269,
	186,
	3,
	269,
	186,
	4,
	269,
	186,
	4,
	269,
	186,
	4,
	269,
	186,
	4,
	269,
	186,
	5,
	269,
	186,
	6,
	269,
	186,
	7,
	269,
	186,
	8,
	269,
	186,
	9,
	269,
	186,
	8,
	269,
	186,
	7,
	269,
	186,
	9,
	269,
	186,
	8,
	269,
	186,
	7,
	269,
	186,
	9,
	269,
	186,
	10,
	269,
	186,
	11,
	269,
	186,
	11,
	269,
	186,
	8,
	269,
	186,
	5,
	269,
	186,
	4,
	269,
	186,
	4,
	269,
	186,
	4,
	269,
	186,
	4,
	269,
	186,
	12,
	269,
	186,
	2,
	269,
	186,
	1,
	269,
	186,
	0,
	0
};

uint16 reset_30_42[] = {
	C_SCREEN,
	42,
	C_XCOOD,
	424,
	C_YCOOD,
	304,
	65535
};

uint16 sc42_mouse_list[] = {
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc42_sign = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	42,	// screen
	0,	// place
	0,	// getToTable
	254,	// xcood
	137,	// ycood
	170*64,	// frame
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
	SC42_SIGN_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc42_fos_enter[] = {
	158*64,
	389,
	258,
	0,
	389,
	260,
	1,
	389,
	262,
	2,
	389,
	265,
	3,
	389,
	267,
	4,
	389,
	269,
	5,
	389,
	270,
	6,
	389,
	272,
	7,
	389,
	274,
	8,
	389,
	274,
	9,
	389,
	274,
	10,
	392,
	274,
	11,
	393,
	274,
	12,
	387,
	274,
	13,
	382,
	274,
	14,
	381,
	274,
	15,
	378,
	274,
	16,
	373,
	274,
	17,
	367,
	274,
	18,
	361,
	274,
	19,
	360,
	274,
	20,
	358,
	274,
	21,
	353,
	274,
	12,
	347,
	274,
	22,
	342,
	274,
	23,
	336,
	274,
	24,
	333,
	274,
	25,
	328,
	274,
	26,
	324,
	274,
	27,
	0
};

uint16 sc42_clerk_type[] = {
	151*64,
	310,
	234,
	0,
	310,
	234,
	1,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	4,
	310,
	234,
	0,
	310,
	234,
	1,
	310,
	234,
	2,
	310,
	234,
	5,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	5,
	310,
	234,
	5,
	310,
	234,
	6,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	7,
	310,
	234,
	3,
	310,
	234,
	5,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	5,
	310,
	234,
	5,
	310,
	234,
	0,
	310,
	234,
	6,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	2,
	310,
	234,
	7,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	5,
	310,
	234,
	6,
	310,
	234,
	2,
	310,
	234,
	0,
	310,
	234,
	3,
	310,
	234,
	5,
	310,
	234,
	0,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	5,
	310,
	234,
	5,
	310,
	234,
	6,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	0,
	310,
	234,
	7,
	310,
	234,
	5,
	310,
	234,
	0,
	310,
	234,
	1,
	310,
	234,
	2,
	310,
	234,
	3,
	310,
	234,
	4,
	310,
	234,
	3,
	310,
	234,
	0,
	310,
	234,
	1,
	0
};

ExtCompact sc42_judge_ext = {
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
	SP_COL_JUDGE42,	// spColour
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

Compact sc42_judge = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	42,	// screen
	0,	// place
	0,	// getToTable
	269,	// xcood
	186,	// ycood
	150*64,	// frame
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
	SC42_JUDGE_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc42_judge_ext
};

uint16 sc42_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SC42_JUDGE,
	ID_SC42_CLERK,
	ID_SC42_PROSECUTION,
	ID_SC42_JOBSWORTH,
	ID_SC31_GUARD,
	ID_SC42_SIGN,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc42_jdg_turn[] = {
	167*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	2,
	0
};

uint16 sc42_fast_list[] = {
	12+DISK_14,
	21+DISK_14,
	22+DISK_14,
	169+DISK_14,
	170+DISK_14+0X8000,
	0
};

uint32 *sc42_walk_grid = (uint32*)sc42_fast_list;

ExtCompact sc42_prosecution_ext = {
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
	SP_COL_PROS42,	// spColour
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

Compact sc42_prosecution = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	42,	// screen
	0,	// place
	0,	// getToTable
	196,	// xcood
	277,	// ycood
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
	SC42_PROSECUTION_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc42_prosecution_ext
};

uint16 sc42_fos_turn_lr[] = {
	161*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	2,
	0
};

uint16 sc42_chip_list[] = {
	20+DISK_14,
	IT_DANIELLE+DISK_12,
	150+DISK_14,
	151+DISK_14,
	152+DISK_14,
	153+DISK_14,
	154+DISK_14+0X8000,
	155+DISK_14+0X8000,
	156+DISK_14,
	157+DISK_14+0X8000,
	158+DISK_14+0X8000,
	159+DISK_14,
	160+DISK_14,
	161+DISK_14,
	162+DISK_14,
	163+DISK_14+0X8000,
	164+DISK_14+0X8000,
	165+DISK_14,
	166+DISK_14,
	167+DISK_14,
	168+DISK_14,
	0
};

ExtCompact sc42_clerk_ext = {
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
	SP_COL_CLERK42,	// spColour
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

Compact sc42_clerk = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	42,	// screen
	0,	// place
	0,	// getToTable
	310,	// xcood
	234,	// ycood
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
	SC42_CLERK_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc42_clerk_ext
};

uint16 reset_guard_42[] = {
	C_STATUS,
	ST_LOGIC+ST_RECREATE,
	C_SCREEN,
	42,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC42_BLUNT_LOGIC,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 sc42_jdg_return[] = {
	167*64,
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

uint16 sc42_fos_turn_rl[] = {
	161*64,
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

ExtCompact sc42_jobsworth_ext = {
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
	SP_COL_JOBS42,	// spColour
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

Compact sc42_jobsworth = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	42,	// screen
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
	SC42_JOBSWORTH_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc42_jobsworth_ext
};

uint16 reset_dani_42[] = {
	C_STATUS,
	ST_LOGIC+ST_RECREATE+ST_GRID_PLOT+ST_COLLISION,
	C_SCREEN,
	42,
	C_XCOOD,
	128,
	C_YCOOD,
	296,
	C_FRAME,
	38+55*64,
	C_DIR,
	RIGHT,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC42_DANI_LOGIC,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 sc42_sign_anim[] = {
	170*64,
	254,
	137,
	0,
	254,
	137,
	1,
	254,
	137,
	2,
	254,
	137,
	3,
	254,
	137,
	4,
	254,
	137,
	5,
	254,
	137,
	6,
	254,
	137,
	7,
	0
};

uint16 sc42_blunt_enter[] = {
	155*64,
	129,
	240,
	0,
	134,
	240,
	1,
	134,
	241,
	2,
	136,
	241,
	3,
	145,
	240,
	4,
	150,
	240,
	5,
	150,
	241,
	6,
	152,
	241,
	7,
	161,
	240,
	0,
	166,
	240,
	1,
	166,
	241,
	2,
	168,
	241,
	3,
	177,
	240,
	4,
	182,
	240,
	5,
	182,
	241,
	6,
	184,
	241,
	7,
	193,
	240,
	0,
	198,
	240,
	1,
	199,
	241,
	8,
	197,
	241,
	9,
	198,
	239,
	10,
	198,
	236,
	11,
	198,
	234,
	12,
	198,
	233,
	13,
	198,
	231,
	14,
	198,
	228,
	15,
	197,
	226,
	16,
	199,
	225,
	8,
	201,
	224,
	4,
	206,
	224,
	5,
	206,
	225,
	6,
	208,
	225,
	7,
	217,
	224,
	0,
	222,
	224,
	1,
	222,
	225,
	2,
	224,
	225,
	3,
	233,
	224,
	4,
	238,
	224,
	5,
	236,
	225,
	17,
	238,
	225,
	18,
	0
};

uint16 sc42_blunt_leave[] = {
	157*64,
	238,
	225,
	0,
	240,
	225,
	1,
	237,
	225,
	2,
	236,
	224,
	3,
	229,
	224,
	4,
	222,
	225,
	5,
	221,
	225,
	6,
	220,
	224,
	7,
	213,
	224,
	8,
	206,
	225,
	9,
	205,
	225,
	2,
	204,
	224,
	3,
	201,
	225,
	1,
	198,
	226,
	10,
	198,
	227,
	11,
	198,
	229,
	12,
	197,
	232,
	13,
	197,
	234,
	14,
	197,
	235,
	15,
	198,
	237,
	16,
	198,
	240,
	17,
	198,
	242,
	10,
	200,
	241,
	1,
	200,
	241,
	7,
	193,
	241,
	8,
	186,
	242,
	9,
	185,
	242,
	2,
	184,
	241,
	3,
	177,
	241,
	4,
	170,
	242,
	5,
	169,
	242,
	6,
	168,
	241,
	7,
	161,
	241,
	8,
	154,
	242,
	9,
	153,
	242,
	2,
	152,
	241,
	3,
	145,
	241,
	4,
	138,
	242,
	5,
	137,
	242,
	6,
	136,
	241,
	7,
	129,
	241,
	8,
	0
};

uint16 sc42_jobs_enter[] = {
	154*64,
	340,
	200,
	0,
	340,
	200,
	0,
	340,
	200,
	1,
	340,
	200,
	1,
	340,
	200,
	2,
	340,
	200,
	2,
	340,
	200,
	3,
	340,
	200,
	3,
	340,
	200,
	4,
	340,
	200,
	4,
	340,
	200,
	5,
	340,
	200,
	5,
	340,
	200,
	6,
	340,
	200,
	6,
	340,
	200,
	7,
	340,
	200,
	8,
	340,
	200,
	9,
	340,
	200,
	9,
	340,
	200,
	10,
	340,
	200,
	10,
	340,
	200,
	11,
	340,
	200,
	11,
	340,
	200,
	12,
	340,
	200,
	12,
	340,
	200,
	13,
	340,
	200,
	13,
	0
};

uint16 sc42_palette[] = {
	0,
	1024,
	0,
	8,
	1536,
	1024,
	10,
	2048,
	1024,
	522,
	2052,
	2562,
	526,
	2560,
	2052,
	1038,
	2050,
	3588,
	1544,
	3594,
	1028,
	1546,
	3078,
	1030,
	1042,
	4098,
	518,
	1552,
	2052,
	5126,
	1546,
	3088,
	2056,
	2066,
	5634,
	518,
	2574,
	3078,
	2570,
	2066,
	3078,
	4616,
	2576,
	4614,
	2568,
	2070,
	7172,
	1028,
	2570,
	3094,
	3084,
	2578,
	7688,
	1028,
	2582,
	3588,
	3596,
	3084,
	5140,
	1548,
	3084,
	5142,
	2060,
	3598,
	4112,
	3086,
	3094,
	7174,
	2056,
	3596,
	8472,
	1542,
	4112,
	6672,
	1550,
	3610,
	5640,
	2576,
	4114,
	3602,
	6672,
	2085,
	8454,
	2058,
	4118,
	4620,
	4114,
	4624,
	4628,
	4626,
	4122,
	4104,
	5140,
	4126,
	9990,
	1546,
	4624,
	6684,
	4112,
	5140,
	7702,
	2066,
	5136,
	6686,
	3092,
	5652,
	9490,
	2574,
	5144,
	8466,
	3600,
	5150,
	5128,
	6166,
	5150,
	6668,
	3606,
	5650,
	5662,
	5656,
	6166,
	7192,
	5142,
	6168,
	8982,
	2582,
	6174,
	5648,
	8472,
	6680,
	6166,
	6170,
	5157,
	5648,
	8984,
	7192,
	6682,
	6684,
	5671,
	7698,
	6170,
	7194,
	8988,
	5656,
	7198,
	10006,
	3098,
	7192,
	8997,
	6682,
	7205,
	6676,
	9502,
	7205,
	7706,
	6689,
	7723,
	7692,
	7201,
	7715,
	7198,
	9505,
	8485,
	10006,
	7198,
	8995,
	10012,
	7201,
	8990,
	11049,
	5667,
	9505,
	10533,
	7715,
	10021,
	10529,
	8485,
	10019,
	10029,
	9513,
	10533,
	11565,
	9511,
	11055,
	11047,
	11565,
	12081,
	12585,
	11055,
	13109,
	14637,
	12599,
	63,
	0,
	0,
	0,
	16191,
	0,
	16128,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
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
	12079,
	8754,
	11807,
	6172,
	5926,
	7695,
	2576,
	2325,
	2307,
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
	5420,
	8989,
	5646,
	2330,
	4623,
	2565,
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
	0,
	63,
	16128,
	0,
	63,
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

uint16 sc42_fos_leave[] = {
	163*64,
	324,
	275,
	0,
	328,
	275,
	1,
	333,
	275,
	2,
	336,
	275,
	3,
	336,
	275,
	4,
	339,
	275,
	5,
	345,
	275,
	6,
	348,
	275,
	7,
	348,
	275,
	8,
	350,
	275,
	9,
	359,
	275,
	10,
	365,
	275,
	11,
	365,
	275,
	12,
	365,
	275,
	13,
	370,
	275,
	14,
	378,
	275,
	15,
	384,
	275,
	16,
	386,
	275,
	17,
	391,
	275,
	18,
	389,
	275,
	19,
	389,
	275,
	20,
	389,
	275,
	21,
	389,
	272,
	22,
	389,
	270,
	23,
	389,
	269,
	24,
	389,
	267,
	25,
	389,
	265,
	26,
	389,
	262,
	27,
	389,
	260,
	28,
	389,
	258,
	29,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
