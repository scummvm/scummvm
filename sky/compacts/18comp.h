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

#ifndef SKY18COMP_H
#define SKY18COMP_H




namespace Sky {

namespace SkyCompact {

uint16 steam_alert_seq[] = {
	86*64,
	258,
	242,
	0,
	258,
	242,
	1,
	258,
	243,
	2,
	257,
	239,
	3,
	254,
	235,
	4,
	253,
	234,
	5,
	251,
	233,
	6,
	251,
	233,
	7,
	0
};

uint16 sleep_seq[] = {
	75*64,
	250,
	231,
	0,
	250,
	231,
	0,
	250,
	231,
	0,
	250,
	231,
	1,
	250,
	231,
	1,
	250,
	231,
	1,
	250,
	231,
	2,
	250,
	231,
	2,
	250,
	231,
	2,
	250,
	231,
	3,
	250,
	231,
	3,
	250,
	231,
	3,
	250,
	231,
	4,
	250,
	231,
	4,
	250,
	231,
	4,
	250,
	231,
	5,
	250,
	231,
	5,
	250,
	231,
	5,
	250,
	231,
	6,
	250,
	231,
	6,
	250,
	231,
	6,
	250,
	231,
	7,
	250,
	231,
	7,
	250,
	231,
	7,
	0
};

Compact power_door = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE+ST_RECREATE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	146,	// xcood
	229,	// ycood
	61*64,	// frame
	8394,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	174-146,	// mouseRel_x
	233-229,	// mouseRel_y
	246-174,	// mouseSize_x
	268-233,	// mouseSize_y
	POWER_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	POWER_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 steam1_seq[] = {
	57*64,
	386,
	137,
	0,
	386,
	137,
	1,
	386,
	137,
	2,
	386,
	137,
	3,
	386,
	137,
	4,
	386,
	137,
	5,
	386,
	137,
	6,
	386,
	137,
	7,
	386,
	137,
	8,
	386,
	137,
	9,
	0
};

uint16 unpush_but_seq[] = {
	71*64,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

Compact right_lever = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	191,	// xcood
	236,	// ycood
	91*64,	// frame
	67,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	24,	// mouseSize_y
	RIGHT_LEVER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	RIGHT_LEVER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 steam2_seq[] = {
	58*64,
	382,
	137,
	0,
	382,
	137,
	1,
	382,
	137,
	2,
	382,
	137,
	3,
	382,
	137,
	4,
	0
};

uint16 blowoff_seq[] = {
	69*64,
	169,
	222,
	0,
	162,
	227,
	1,
	160,
	240,
	2,
	150,
	250,
	3,
	150,
	255,
	4,
	148,
	259,
	5,
	149,
	271,
	6,
	149,
	279,
	7,
	149,
	279,
	8,
	147,
	279,
	9,
	147,
	279,
	10,
	0
};

uint16 s18_mouse[] = {
	ID_JOEY,
	ID_MONITOR,
	ID_RIGHT_EXIT_S18,
	ID_POWER_PANEL,
	ID_POWER_SWITCH,
	ID_POWER_CHAIR,
	ID_S18_FLOOR,
	ID_LEFT_SKULL,
	ID_RIGHT_SKULL,
	ID_POWER_DOOR,
	ID_LEFT_LEVER,
	ID_RIGHT_LEVER,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 power_start_seq[] = {
	66*64,
	157,
	315,
	0,
	157,
	315,
	1,
	157,
	315,
	2,
	157,
	315,
	3,
	157,
	315,
	4,
	0
};

Compact power_switch = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	237,	// xcood
	222,	// ycood
	63*64,	// frame
	67,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	8,	// mouseSize_x
	8,	// mouseSize_y
	SWITCH_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	POWER_SWITCH_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 power_stop_seq[] = {
	68*64,
	157,
	315,
	0,
	157,
	315,
	1,
	157,
	315,
	2,
	157,
	315,
	3,
	157,
	315,
	0,
	157,
	315,
	4,
	157,
	315,
	5,
	157,
	315,
	6,
	157,
	315,
	7,
	157,
	315,
	8,
	0
};

uint16 s18_floor_table[] = {
	ID_S18_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S18_WALK_ON,
	1,
	GET_TO_TALK,
	ID_RIGHT_EXIT_S18,
	GET_TO_R_EXIT_S18,
	ID_POWER_PANEL,
	GET_TO_POWER_PANEL,
	ID_POWER_SWITCH,
	GET_TO_POWER_SWITCH,
	ID_POWER_CHAIR,
	GET_TO_POWER_CHAIR,
	ID_LEFT_SKULL,
	GET_TO_LEFT_SKULL,
	ID_RIGHT_SKULL,
	GET_TO_RIGHT_SKULL,
	ID_POWER_DOOR,
	GET_TO_POWER_DOOR,
	ID_RIGHT_LEVER,
	GET_TO_RIGHT_LEVER,
	ID_LEFT_LEVER,
	GET_TO_LEFT_LEVER,
	65535
};

Compact s18_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	s18_floor_table,	// getToTable
	128,	// xcood
	272,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	14+16,	// mouseSize_y
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

uint16 joey_list_s18[] = {
	156,
	360,
	280,
	304,
	1,
	0
};

uint16 nu_chip_18[] = {
	136+DISK_2,
	137+DISK_2,
	44+DISK_2,
	45+DISK_2,
	46+DISK_2,
	61+DISK_2,
	63+DISK_2,
	64+DISK_2,
	65+DISK_2,
	66+DISK_2,
	67+DISK_2,
	68+DISK_2,
	70+DISK_2+0X8000,
	71+DISK_2,
	88+DISK_2+0X8000,
	92+DISK_2+0X8000,
	90+DISK_2,
	91+DISK_2,
	89+DISK_2,
	72+DISK_2,
	85+DISK_2+0X8000,
	86+DISK_2+0X8000,
	74+DISK_2,
	76+DISK_2+0X8000,
	75+DISK_2,
	0
};

Compact power_chair = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	258,	// xcood
	253,	// ycood
	0,	// frame
	8908,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	24,	// mouseSize_y
	CHAIR_ACTION,	// actionScript
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

uint16 steam3_seq[] = {
	59*64,
	380,
	137,
	0,
	380,
	137,
	1,
	380,
	137,
	2,
	380,
	137,
	3,
	380,
	137,
	4,
	380,
	137,
	5,
	0
};

Compact left_skull = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	329,	// xcood
	247,	// ycood
	0,	// frame
	65,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	4,	// mouseSize_x
	5,	// mouseSize_y
	LEFT_SKULL_ACTION,	// actionScript
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

uint16 rs_gordon_end[] = {
	C_XCOOD,
	250,
	C_YCOOD,
	231,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_MOUSE,
	C_ACTION_SCRIPT,
	SAT_GORDON_ACTION,
	C_MOUSE_REL_X,
	10,
	C_MOUSE_REL_Y,
	244-231,
	C_MOUSE_SIZE_X,
	20,
	C_MOUSE_SIZE_Y,
	276-244,
	C_FRAME,
	75*64,
	65535
};

Compact right_skull = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	379,	// xcood
	247,	// ycood
	0,	// frame
	65,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	4,	// mouseSize_x
	5,	// mouseSize_y
	RIGHT_SKULL_ACTION,	// actionScript
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

uint16 power_spin_seq[] = {
	67*64,
	157,
	315,
	0,
	157,
	315,
	1,
	157,
	315,
	2,
	157,
	315,
	3,
	0
};

uint16 s18_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	57+DISK_2,
	58+DISK_2,
	60+DISK_2,
	0
};

uint16 steam4_seq[] = {
	60*64,
	380,
	137,
	0,
	380,
	137,
	1,
	380,
	137,
	2,
	380,
	137,
	3,
	380,
	137,
	4,
	0
};

Compact steam = {
	L_SCRIPT,	// logic
	ST_FOREGROUND+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	10,	// xcood
	10,	// ycood
	57*64,	// frame
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
	STEAM_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact socket = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE+ST_RECREATE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	185,	// xcood
	220,	// ycood
	65*64,	// frame
	66,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	8,	// mouseSize_x
	8,	// mouseSize_y
	SOCKET_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	POWER_PANEL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact power_motor = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	67*64,	// frame
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
	POWER_MOTOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 joey_unpush_seq[] = {
	89*64,
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

uint16 high_push_seq[] = {
	72*64,
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

Compact left_lever = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	181,	// xcood
	236,	// ycood
	90*64+1,	// frame
	67,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	24,	// mouseSize_y
	LEFT_LEVER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LEFT_LEVER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s18_pal[] = {
	0,
	1024,
	1540,
	1542,
	2054,
	2056,
	2058,
	2568,
	2570,
	2572,
	3084,
	3594,
	3084,
	4108,
	2570,
	3086,
	6158,
	2058,
	3598,
	4112,
	3598,
	3600,
	4112,
	4112,
	3606,
	5132,
	3088,
	4114,
	5650,
	2576,
	4626,
	6676,
	3600,
	4628,
	5652,
	4114,
	4630,
	5138,
	6162,
	5144,
	5132,
	5652,
	5146,
	5644,
	5654,
	5146,
	5652,
	6166,
	5660,
	6158,
	6166,
	5662,
	6158,
	6678,
	5656,
	6172,
	5656,
	6174,
	7184,
	5656,
	5665,
	6676,
	7192,
	6172,
	8474,
	4122,
	5671,
	7696,
	5658,
	6682,
	7198,
	7194,
	6689,
	8984,
	4636,
	6695,
	7700,
	7708,
	7198,
	8993,
	6684,
	7713,
	10526,
	5660,
	8483,
	8470,
	8990,
	7717,
	8986,
	7713,
	7717,
	11043,
	6174,
	8483,
	10019,
	7201,
	8999,
	8990,
	10019,
	8997,
	12071,
	6177,
	9001,
	8993,
	10021,
	9511,
	11045,
	7205,
	9011,
	11542,
	7717,
	9513,
	12587,
	6693,
	10025,
	11561,
	8999,
	10539,
	12579,
	7721,
	10029,
	13613,
	6185,
	10545,
	11041,
	10539,
	11051,
	12077,
	10027,
	11059,
	15649,
	6697,
	11567,
	14635,
	7211,
	11573,
	14115,
	7725,
	11567,
	12591,
	10543,
	12079,
	14641,
	9007,
	12605,
	13078,
	11057,
	12091,
	16161,
	7725,
	12599,
	13093,
	11569,
	12601,
	14113,
	10033,
	12593,
	12595,
	13617,
	13107,
	14643,
	10547,
	13109,
	13617,
	13107,
	13119,
	15649,
	9523,
	13627,
	16165,
	6199,
	13621,
	15669,
	10037,
	14139,
	14123,
	13623,
	14139,
	16173,
	8505,
	14655,
	14627,
	14135,
	14655,
	15145,
	13627,
	15679,
	16171,
	11581,
	15679,
	15665,
	14141,
	1073,
	2564,
	2570,
	3084,
	4108,
	3598,
	4112,
	5136,
	3088,
	4118,
	10,
	10,
	4630,
	5648,
	4626,
	5144,
	5132,
	5652,
	5146,
	7188,
	3606,
	6174,
	7184,
	6680,
	0,
	2111,
	2056,
	2572,
	3084,
	3594,
	3084,
	3596,
	4110,
	3600,
	4624,
	4624,
	4626,
	5140,
	6162,
	5140,
	6166,
	6166,
	5656,
	6682,
	7192,
	6684,
	7708,
	7708,
	63,
	2048,
	2056,
	3086,
	4622,
	4624,
	5140,
	6166,
	6166,
	6170,
	7708,
	7708,
	7713,
	8990,
	8993,
	8997,
	10023,
	9509,
	10025,
	11049,
	10539,
	11567,
	13099,
	11569,
	16128,
	2560,
	2570,
	2572,
	15116,
	11575,
	4628,
	6676,
	7192,
	7198,
	8990,
	8993,
	9511,
	11045,
	9001,
	11051,
	12073,
	11053,
	13109,
	14129,
	13623,
	15163,
	15669,
	14141,
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

uint16 rs_foster_to_18[] = {
	C_XCOOD,
	424,
	C_YCOOD,
	272,
	C_SCREEN,
	18,
	C_PLACE,
	ID_S18_FLOOR,
	65535
};

uint16 power_bang_seq[] = {
	62*64,
	179,
	216,
	0,
	173,
	207,
	1,
	172,
	205,
	2,
	171,
	204,
	3,
	169,
	204,
	4,
	167,
	203,
	5,
	164,
	201,
	6,
	158,
	198,
	7,
	155,
	196,
	8,
	150,
	192,
	9,
	145,
	189,
	10,
	138,
	183,
	11,
	134,
	177,
	12,
	134,
	174,
	13,
	138,
	170,
	14,
	139,
	167,
	15,
	0
};

uint16 s18_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_MONITOR,
	ID_STEAM,
	ID_POWER_DOOR,
	ID_LEFT_LEVER,
	ID_RIGHT_LEVER,
	ID_POWER_MOTOR,
	ID_POWER_PANEL,
	ID_POWER_SWITCH,
	ID_POWER_BANG,
	ID_LAMB,
	ID_LIFT_S7,
	ID_LIFT7_LIGHT,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 high_unpush_seq[] = {
	72*64,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 s18_chip_list[] = {
	173,
	111,
	44+DISK_2,
	45+DISK_2,
	46+DISK_2,
	61+DISK_2,
	63+DISK_2,
	64+DISK_2,
	65+DISK_2,
	66+DISK_2,
	67+DISK_2,
	68+DISK_2,
	70+DISK_2+0X8000,
	71+DISK_2,
	88+DISK_2+0X8000,
	92+DISK_2+0X8000,
	90+DISK_2,
	91+DISK_2,
	89+DISK_2,
	72+DISK_2,
	85+DISK_2+0X8000,
	86+DISK_2+0X8000,
	74+DISK_2,
	76+DISK_2+0X8000,
	75+DISK_2,
	0
};

Compact power_bang = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	18,	// screen
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
	POWER_BANG_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 push_but_seq[] = {
	71*64,
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

uint16 door_blast_seq[] = {
	61*64,
	146,
	229,
	0,
	146,
	229,
	1,
	146,
	229,
	2,
	146,
	229,
	3,
	0
};

uint16 rs_gordon_start[] = {
	C_XCOOD,
	264,
	C_YCOOD,
	272,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,
	C_ACTION_SCRIPT,
	SHOUT_ACTION,
	C_MOUSE_REL_X,
	65520,
	C_MOUSE_REL_Y,
	65488,
	C_MOUSE_SIZE_X,
	32,
	C_MOUSE_SIZE_Y,
	48,
	65535
};

uint16 monhit_seq[] = {
	76*64,
	250,
	231,
	0,
	250,
	231,
	1,
	250,
	231,
	2,
	250,
	231,
	3,
	0
};

uint16 spanner_seq[] = {
	70*64,
	239,
	195,
	0,
	239,
	195,
	0,
	239,
	195,
	1,
	239,
	195,
	1,
	239,
	195,
	2,
	239,
	195,
	3,
	239,
	195,
	4,
	239,
	195,
	5,
	239,
	195,
	6,
	239,
	195,
	6,
	239,
	195,
	6,
	239,
	195,
	7,
	239,
	195,
	8,
	239,
	195,
	9,
	239,
	195,
	7,
	239,
	195,
	10,
	239,
	195,
	10,
	239,
	195,
	11,
	239,
	195,
	12,
	0
};

uint16 joey_push_seq[] = {
	89*64,
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
	0
};

Compact right_exit_s18 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	0,	// place
	0,	// getToTable
	405,	// xcood
	221,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	414-405,	// mouseSize_x
	280-221,	// mouseSize_y
	ER18_ACTION,	// actionScript
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
