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

#ifndef SKY90COMP_H
#define SKY90COMP_H


#include "91comp.h"
#include "lincmenu.h"
#include "92comp.h"
#include "93comp.h"
#include "94comp.h"
#include "95comp.h"
#include "linc_gen.h"
#include "96comp.h"


namespace Sky {

namespace SkyCompact {

Compact door_r90f = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	364,	// xcood
	136,	// ycood
	259*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	255-136,	// mouseRel_y
	30,	// mouseSize_x
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
	DOOR_R90F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_r90 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	360,	// xcood
	136,	// ycood
	258*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	26,	// mouseRel_y
	8,	// mouseSize_x
	102,	// mouseSize_y
	DOOR_R90_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_R90_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc90_floor_table[] = {
	ID_SC90_FLOOR,
	RET_OK,
	ID_SC90_SMFLOOR,
	RET_FAIL,
	ID_DOOR_L90,
	RET_FAIL,
	ID_DOOR_R90,
	GET_TO_DOOR_R90,
	ID_JOIN_OBJECT,
	GET_TO_JOIN_OBJECT,
	ID_OSCILLATOR,
	RET_FAIL,
	ID_EYEBALL_90,
	GET_TO_EYEBALL_90,
	65535
};

Compact sc90_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	sc90_floor_table,	// getToTable
	153,	// xcood
	254,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	269,	// mouseSize_x
	53,	// mouseSize_y
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

uint16 sc90_smfloor_table[] = {
	ID_SC90_FLOOR,
	RET_FAIL,
	ID_SC90_SMFLOOR,
	RET_OK,
	ID_DOOR_L90,
	GET_TO_DOOR_L90,
	ID_DOOR_R90,
	RET_FAIL,
	ID_JOIN_OBJECT,
	RET_FAIL,
	ID_OSCILLATOR,
	GET_TO_OSCILLATOR,
	ID_EYEBALL_90,
	RET_FAIL,
	65535
};

uint16 eye90_zap_anim[] = {
	113*64,
	247,
	206,
	0,
	247,
	206,
	1,
	247,
	206,
	2,
	247,
	206,
	3,
	247,
	206,
	4,
	247,
	206,
	5,
	0
};

uint16 door_l90_anim[] = {
	45*64,
	212,
	136,
	0,
	212,
	136,
	1,
	212,
	136,
	2,
	212,
	136,
	3,
	212,
	136,
	4,
	212,
	136,
	5,
	212,
	136,
	6,
	212,
	136,
	7,
	212,
	136,
	8,
	212,
	136,
	9,
	212,
	136,
	10,
	212,
	136,
	11,
	212,
	136,
	12,
	212,
	136,
	13,
	212,
	136,
	14,
	212,
	136,
	15,
	212,
	136,
	16,
	212,
	136,
	17,
	212,
	136,
	18,
	212,
	136,
	19,
	212,
	136,
	20,
	212,
	136,
	21,
	212,
	136,
	22,
	0
};

Compact door_l90 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	211,	// xcood
	136,	// ycood
	45*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	26,	// mouseRel_y
	9,	// mouseSize_x
	102,	// mouseSize_y
	DOOR_L90_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L90_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 get_join[] = {
	22*64,
	336,
	272,
	0,
	336,
	272,
	1,
	336,
	272,
	2,
	336,
	272,
	3,
	336,
	272,
	4,
	336,
	272,
	5,
	0
};

uint16 sc90_mouse[] = {
	ID_JOIN_OBJECT,
	ID_OSCILLATOR,
	ID_EYEBALL_90,
	ID_DOOR_L90,
	ID_DOOR_L90F,
	ID_DOOR_R90,
	ID_DOOR_R90F,
	ID_SC90_SMFLOOR,
	ID_SC90_FLOOR,
	0XFFFF,
	ID_LINC_MENU_MOUSE
};

uint16 door_l90f_anim[] = {
	46*64,
	207,
	136,
	0,
	207,
	136,
	1,
	207,
	136,
	2,
	207,
	136,
	3,
	207,
	136,
	4,
	207,
	136,
	5,
	207,
	136,
	6,
	207,
	136,
	7,
	207,
	136,
	8,
	207,
	136,
	9,
	207,
	136,
	10,
	207,
	136,
	11,
	207,
	136,
	12,
	207,
	136,
	13,
	207,
	136,
	14,
	207,
	136,
	15,
	207,
	136,
	16,
	207,
	136,
	17,
	207,
	136,
	18,
	207,
	136,
	19,
	207,
	136,
	20,
	207,
	136,
	21,
	207,
	136,
	22,
	0
};

uint16 sc90_logic[] = {
	ID_BLUE_FOSTER,
	ID_JOIN_OBJECT,
	ID_OSCILLATOR,
	ID_DOOR_L90,
	ID_DOOR_L90F,
	ID_DOOR_R90,
	ID_DOOR_R90F,
	ID_EYEBALL_90,
	0XFFFF,
	ID_LINC_MENU_LOGIC
};

uint16 eye_90_table[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	2,
	3,
	4,
	4,
	5,
	5,
	6,
	6,
	6,
	7,
	7,
	8,
	8,
	9,
	10,
	11,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	12,
	0,
	0,
	0,
	0,
	0,
	1,
	2,
	3,
	3,
	4,
	4,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	8,
	8,
	9,
	9,
	10,
	11,
	12,
	12,
	12,
	12,
	12,
	12,
	0,
	0,
	1,
	1,
	2,
	2,
	3,
	3,
	4,
	4,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	8,
	8,
	9,
	9,
	10,
	10,
	11,
	11,
	12,
	12,
	12,
	1,
	1,
	2,
	2,
	3,
	3,
	4,
	4,
	5,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	8,
	8,
	9,
	9,
	10,
	10,
	11,
	12,
	12,
	2,
	3,
	3,
	4,
	4,
	4,
	5,
	5,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	7,
	8,
	8,
	8,
	9,
	9,
	10,
	11,
	3,
	3,
	4,
	4,
	4,
	5,
	5,
	5,
	5,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	6,
	7,
	7,
	7,
	7,
	8,
	8,
	8,
	9,
	9,
	10
};

Compact eyeball_90 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	270,	// xcood
	223,	// ycood
	6+91*64,	// frame
	24587,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	31,	// mouseSize_x
	29,	// mouseSize_y
	EYEBALL_90_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	EYEBALL_90_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc90_smfloor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	sc90_smfloor_table,	// getToTable
	206,	// xcood
	254,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	69,	// mouseSize_x
	11,	// mouseSize_y
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

Compact join_object = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	336,	// xcood
	272,	// ycood
	22*64,	// frame
	24598,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	16,	// mouseSize_y
	JOIN_OBJECT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	JOIN_OBJECT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_l90f = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	207,	// xcood
	136,	// ycood
	46*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	(int16) 65532,	// mouseRel_x
	255-136,	// mouseRel_y
	5,	// mouseSize_x
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
	DOOR_L90F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact oscillator = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	241,	// xcood
	259,	// ycood
	132*64,	// frame
	24592,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	18,	// mouseSize_x
	4,	// mouseSize_y
	OSCILLATOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	OSCILLATOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_90_91[] = {
	C_SCREEN,
	91,
	C_PLACE,
	ID_SC91_FLOOR,
	C_XCOOD,
	184,
	C_YCOOD,
	264,
	65535
};

uint16 rs_foster_90_94[] = {
	C_SCREEN,
	94,
	C_PLACE,
	ID_SC94_FLOOR,
	C_XCOOD,
	360,
	C_YCOOD,
	264,
	C_DIR,
	DOWN,
	65535
};

uint16 door_r90_anim[] = {
	258*64,
	360,
	136,
	0,
	360,
	136,
	1,
	360,
	136,
	2,
	360,
	136,
	3,
	360,
	136,
	4,
	360,
	136,
	5,
	360,
	136,
	6,
	360,
	136,
	7,
	360,
	136,
	8,
	360,
	136,
	9,
	360,
	136,
	10,
	360,
	136,
	11,
	360,
	136,
	12,
	360,
	136,
	13,
	360,
	136,
	14,
	360,
	136,
	15,
	360,
	136,
	16,
	360,
	136,
	17,
	360,
	136,
	18,
	360,
	136,
	19,
	360,
	136,
	20,
	360,
	136,
	21,
	360,
	136,
	22,
	0
};

uint16 door_r90f_anim[] = {
	259*64,
	364,
	136,
	0,
	364,
	136,
	1,
	364,
	136,
	2,
	364,
	136,
	3,
	364,
	136,
	4,
	364,
	136,
	5,
	364,
	136,
	6,
	364,
	136,
	7,
	364,
	136,
	8,
	364,
	136,
	9,
	364,
	136,
	10,
	364,
	136,
	11,
	364,
	136,
	12,
	364,
	136,
	13,
	364,
	136,
	14,
	364,
	136,
	15,
	364,
	136,
	16,
	364,
	136,
	17,
	364,
	136,
	18,
	364,
	136,
	19,
	364,
	136,
	20,
	364,
	136,
	21,
	364,
	136,
	22,
	0
};

uint16 get_oscill[] = {
	132*64,
	241,
	259,
	0,
	241,
	259,
	1,
	241,
	259,
	1,
	241,
	259,
	2,
	241,
	259,
	3,
	241,
	259,
	4,
	241,
	259,
	5,
	0
};

uint16 blind_eye[] = {
	89*64,
	268,
	221,
	0,
	268,
	221,
	1,
	268,
	221,
	2,
	268,
	221,
	3,
	268,
	221,
	4,
	268,
	221,
	5,
	268,
	221,
	6,
	268,
	221,
	7,
	0
};

uint16 see_eye[] = {
	90*64,
	270,
	223,
	0,
	270,
	223,
	1,
	270,
	223,
	2,
	270,
	223,
	3,
	270,
	223,
	4,
	270,
	223,
	5,
	270,
	223,
	6,
	270,
	223,
	7,
	270,
	223,
	8,
	270,
	223,
	9,
	0
};

uint16 pal90[] = {
	0,
	3584,
	8960,
	1044,
	7211,
	12812,
	5413,
	11066,
	15390,
	0,
	8,
	3072,
	0,
	16,
	5120,
	1036,
	5120,
	8,
	3104,
	9216,
	20,
	7208,
	15360,
	36,
	0,
	24,
	0,
	15159,
	9787,
	12077,
	8987,
	4390,
	7448,
	3850,
	1301,
	3335,
	7176,
	12,
	1044,
	4096,
	9216,
	0,
	28,
	5120,
	0,
	6180,
	24,
	7168,
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

uint16 pal90a[] = {
	0,
	365,
	382,
	415,
	415,
	415,
	12,
	77,
	78,
	607,
	2080,
	2624,
	3408,
	3696,
	4000,
	3984,
	879,
	546,
	3839,
	3311,
	2511,
	1930,
	855,
	565,
	965,
	113,
	80,
	3584,
	2304,
	1536,
	3737,
	895
};

uint16 pal90b[] = {
	0,
	346,
	363,
	380,
	398,
	415,
	10,
	11,
	12,
	13,
	1296,
	1840,
	2624,
	2912,
	3200,
	3984,
	14,
	546,
	3839,
	3311,
	2237,
	1657,
	854,
	565,
	660,
	97,
	64,
	2816,
	2048,
	1280,
	2935,
	15
};

uint16 chip_list_sc90[] = {
	IT_JOIN_OBJECT+DISK_4,
	IT_OSCILLATOR+DISK_4,
	IT_DOOR_L90+DISK_4,
	IT_DOOR_L90F+DISK_4,
	IT_DOOR_R90+DISK_4,
	IT_DOOR_R90F+DISK_4,
	IT_FOST_DIE90+DISK_4,
	0
};

uint16 get_eye[] = {
	18*64,
	276,
	147,
	0,
	276,
	147,
	1,
	276,
	147,
	2,
	276,
	147,
	2,
	276,
	147,
	2,
	276,
	147,
	3,
	276,
	147,
	0,
	0
};

uint16 fost_die90[] = {
	115*64,
	150,
	198,
	0,
	150,
	198,
	1,
	150,
	198,
	2,
	150,
	198,
	3,
	150,
	198,
	4,
	150,
	198,
	5,
	150,
	198,
	6,
	150,
	198,
	5,
	150,
	198,
	7,
	150,
	198,
	8,
	150,
	198,
	9,
	150,
	198,
	10,
	150,
	198,
	11,
	150,
	198,
	12,
	150,
	198,
	13,
	150,
	198,
	14,
	150,
	198,
	15,
	150,
	198,
	16,
	150,
	198,
	17,
	0
};

void *data_6[] = {
	0,
	&sc90_floor,
	&sc91_floor,
	&door_r90,
	&door_l91,
	&info_menu,
	&read_menu,
	&open_menu,
	&join_menu,
	&door_r91,
	&slab1,
	&door_l92,
	&door_r92,
	&sc93_floor,
	&door_l93,
	&sc90_smfloor,
	&slab2,
	&slab3,
	&slab4,
	&slab5,
	&slab6,
	&slab7,
	&slab8,
	&slab9,
	&door_r90f,
	&door_l91f,
	&persona,
	&sc94_floor,
	&join_object,
	&sc95_floor,
	&door_t91,
	&door_t91r,
	&door_l95,
	&door_l95f,
	&bridge_a,
	&bridge_b,
	&bridge_c,
	&bridge_d,
	&bridge_e,
	&bridge_f,
	&bridge_g,
	&bridge_h,
	shrug,
	&door_t95,
	&door_t95r,
	&door_r94,
	&door_r94r,
	&door_l94,
	&door_l94r,
	&blind_menu,
	&green_menu,
	&red_menu,
	&charon_menu,
	&decomp_menu,
	&orders_menu,
	&decrypt_menu,
	&persona_menu,
	&green_circle,
	&red_circle,
	&door_l90,
	&door_l90f,
	&bag_91,
	&decomp_obj,
	&decrypt_obj,
	anita_holo_talk,
	&oscillator,
	&adjust_book,
	&adjust_menu,
	&eyeball_90,
	0,
	0,
	0,
	0,
	&oscill_menu,
	&playbak_menu,
	&virus_menu,
	eye_90_table,
	0,
	0,
	0,
	rs_foster_s90,
	rs_foster_90_91,
	rs_foster_90_94,
	rs_foster_91_90,
	rs_foster_91_92,
	rs_foster_91_95,
	rs_foster_92_91,
	rs_foster_92_93,
	rs_foster_93_92,
	rs_foster_94_90,
	rs_foster_94_95,
	rs_foster_95_91,
	rs_foster_95_94,
	rs_foster_95_96,
	rs_foster_96_95,
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
	door_l90_anim,
	door_l90f_anim,
	door_r90_anim,
	door_r90f_anim,
	get_join,
	get_oscill,
	blind_eye,
	see_eye,
	door_l91_anim,
	door_l91f_anim,
	door_t91_anim,
	door_t91r_anim,
	0,
	grid90,
	get_decomp,
	get_decrypt,
	crouch_down,
	get_persona,
	get_adjust,
	door_l94_anim,
	door_l94r_anim,
	door_r94_anim,
	door_r94r_anim,
	door_l95_anim,
	door_l95f_anim,
	door_t95_anim,
	door_t95r_anim,
	grid91,
	grid92,
	pal90,
	pal90a,
	pal90b,
	pal91,
	pal92,
	pal93,
	pal94,
	pal95,
	pal96,
	holo1_a_anim,
	holo1_b_anim,
	fost_die91,
	&hologram_a,
	&hologram_pad,
	&report_book,
	&report_menu,
	get_report,
	0,
	chip_list_sc90,
	sc90_logic,
	sc90_mouse,
	fast_list_sc91,
	chip_list_sc91,
	sc91_logic,
	sc91_mouse,
	fast_list_sc92,
	chip_list_sc92,
	sc92_logic,
	sc92_mouse,
	fast_list_sc93,
	chip_list_sc93,
	sc93_logic,
	sc93_mouse,
	fast_list_sc94,
	chip_list_sc94,
	sc94_logic,
	sc94_mouse,
	fast_list_sc95,
	chip_list_sc95,
	sc95_logic,
	sc95_mouse,
	get_eye,
	crouch_left,
	crouch_right,
	&window_1,
	&window_2,
	&window_3,
	&window_4,
	window_logic,
	window_mouse,
	&hologram_b,
	holo3_anim,
	&door_r95,
	&door_r95f,
	door_r95_anim,
	door_r95f_anim,
	fast_list_sc96,
	chip_list_sc96,
	sc96_logic,
	sc96_mouse,
	&sc96_floor,
	&door_l96,
	&door_l96f,
	door_l96_anim,
	door_l96f_anim,
	grid93,
	grid94,
	grid95,
	grid96,
	&crystal,
	&virus,
	crystal_spin,
	crystal_break,
	virus_spin,
	guardian_up,
	&door_l93f,
	fost_die90,
	get_virus,
	&door_r91f,
	door_r91_anim,
	door_r91f_anim,
	&eyeball_91,
	eye_91_table,
	eye91_zap_anim,
	guardian_down,
	eye90_zap_anim,
	0,
	&guardian,
	crouch_right_a,
	crouch_right_b,
	&door_r92r,
	door_r92_anim,
	door_r92r_anim,
	&info_button,
	&weight,
	door_l93_anim,
	door_l93f_anim,
	enter_top,
	exit_top,
	head_module,
	file_module,
	size_module,
	auth_module,
	note_module,
	&door_l92f,
	door_l92_anim,
	door_l92f_anim,
	logon,
	logoff,
	weight_anim,
	&kill_menu,
	&orders2_menu,
	&report2_menu,
	&adjust2_menu,
	linc_menu_logic,
	linc_menu_mouse
};

uint32 *sec6_compacts = (uint32*)data_6;

} // namespace SkyCompact

} // namespace Sky

#endif
