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

#ifndef SKY13COMP_H
#define SKY13COMP_H




namespace Sky {

namespace SkyCompact {

uint16 foreman_l_to_r[] = {
	41-8+63*64,
	40-8+63*64,
	47-8+63*64,
	0
};

uint16 rs_foster[] = {
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_AR_PRIORITY+ST_GRID_PLOT+ST_COLLISION,
	65535
};

uint16 gears_seq[] = {
	48*64,
	284,
	291,
	0,
	284,
	291,
	1,
	284,
	291,
	2,
	0
};

Compact fact2_exit_left = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	126,	// xcood
	136,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	11,	// mouseSize_x
	300,	// mouseSize_y
	FACT2_ACTION,	// actionScript
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

Compact fact_console = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	0X1A2,	// xcood
	0XC3,	// ycood
	89*64,	// frame
	8371,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	10,	// mouseSize_y
	CONSOLE_13_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FACT_CONSOLE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact pipe1 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	56*64,	// frame
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
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 lite1_table[] = {
	ID_STD_RIGHT_TALK,
	GT_R_TALK_13,
	65535
};

Compact sensor = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	59*64,	// frame
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
	SENSOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 foreman_l_to_d[] = {
	41-8+63*64,
	0
};

uint16 foreman_st_up[] = {
	63*64,
	1,
	1,
	36,
	0
};

uint16 foreman_work_seq[] = {
	65*64,
	259,
	190,
	0,
	259,
	190,
	0,
	259,
	190,
	1,
	259,
	190,
	1,
	259,
	190,
	2,
	259,
	190,
	2,
	259,
	190,
	3,
	259,
	190,
	3,
	259,
	190,
	4,
	259,
	190,
	4,
	259,
	190,
	5,
	259,
	190,
	5,
	259,
	190,
	6,
	259,
	190,
	6,
	259,
	190,
	7,
	259,
	190,
	7,
	259,
	190,
	8,
	259,
	190,
	8,
	259,
	190,
	7,
	259,
	190,
	7,
	259,
	190,
	8,
	259,
	190,
	8,
	259,
	190,
	6,
	259,
	190,
	6,
	259,
	190,
	5,
	259,
	190,
	5,
	259,
	190,
	6,
	259,
	190,
	6,
	259,
	190,
	7,
	259,
	190,
	7,
	259,
	190,
	6,
	259,
	190,
	6,
	259,
	190,
	5,
	259,
	190,
	5,
	259,
	190,
	4,
	259,
	190,
	4,
	259,
	190,
	5,
	259,
	190,
	5,
	259,
	190,
	6,
	259,
	190,
	6,
	259,
	190,
	7,
	259,
	190,
	7,
	259,
	190,
	8,
	259,
	190,
	8,
	259,
	190,
	3,
	259,
	190,
	3,
	259,
	190,
	2,
	259,
	190,
	2,
	259,
	190,
	9,
	259,
	190,
	9,
	259,
	190,
	10,
	259,
	190,
	10,
	259,
	190,
	11,
	259,
	190,
	11,
	259,
	190,
	12,
	259,
	190,
	12,
	259,
	190,
	13,
	259,
	190,
	13,
	259,
	190,
	14,
	259,
	190,
	14,
	259,
	190,
	15,
	259,
	190,
	15,
	259,
	190,
	16,
	259,
	190,
	16,
	0
};

uint16 nu_s13_chip[] = {
	136+DISK_2,
	137+DISK_2,
	63+DISK_3,
	64+DISK_3,
	65+DISK_3,
	68+DISK_3,
	47+DISK_3,
	48+DISK_3,
	60+DISK_3,
	61+DISK_3,
	56+DISK_3,
	57+DISK_3,
	58+DISK_3,
	59+DISK_3,
	62+DISK_3,
	66+DISK_3,
	67+DISK_3,
	89+DISK_3,
	90+DISK_6,
	106+DISK_6,
	17+DISK_3,
	18+DISK_3,
	0
};

uint16 rs_storeman[] = {
	C_XCOOD,
	288,
	C_YCOOD,
	232,
	C_FRAME,
	63*64+34,
	C_PLACE,
	ID_S13_FLOOR,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	FOREMAN_STORE_CHECK,
	C_BASE_SUB+2,
	0,
	C_DIR,
	LEFT,
	C_LOGIC,
	L_SCRIPT,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_MOUSE,
	65535
};

uint16 foreman_left[] = {
	4,
	16+63*64,
	65532,
	0,
	4,
	17+63*64,
	65532,
	0,
	4,
	18+63*64,
	65532,
	0,
	4,
	19+63*64,
	65532,
	0,
	4,
	20+63*64,
	65532,
	0,
	4,
	21+63*64,
	65532,
	0,
	4,
	22+63*64,
	65532,
	0,
	4,
	23+63*64,
	65532,
	0,
	0
};

uint16 closecoat[] = {
	141*64,
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

uint16 pipe1_seq[] = {
	56*64,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	0XEC,
	0XC4,
	0,
	0
};

uint16 foreman_r_to_l[] = {
	45-8+63*64,
	44-8+63*64,
	43-8+63*64,
	0
};

Compact fact2_exit_right = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	432,	// xcood
	174,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	267-174,	// mouseSize_y
	FACT2_RIGHT_ACTION,	// actionScript
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

uint16 foreman_l_to_u[] = {
	43-8+63*64,
	0
};

Compact lite1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	13,	// screen
	0,	// place
	lite1_table,	// getToTable
	28,	// xcood
	36,	// ycood
	59*64,	// frame
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
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s13_floor_table[] = {
	ID_S13_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S13_LEFT_ON,
	1,
	S13_RIGHT_ON,
	3,
	ON_FROM_S15,
	4,
	GT_STORE_STOP,
	5,
	GT_TOUR_2,
	ID_LITE1,
	GT_LITE1,
	ID_FACT2_L_EXIT,
	GT_FACT2_L_EXIT,
	ID_FACT2_R_EXIT,
	GT_FACT2_R_EXIT,
	ID_F2_STORE_EXIT,
	GT_FACT2_STORE_EXIT,
	ID_COGS,
	GT_COGS,
	ID_STD_LEFT_TALK,
	GT_L_TALK_13,
	ID_STD_RIGHT_TALK,
	GT_R_TALK_13,
	ID_FACT2_SPY,
	GT_WINDOW,
	ID_SENSORS,
	GT_SENSORS,
	ID_FACT_CONSOLE,
	GT_FACT_CONSOLE,
	65535
};

uint16 fsearch_seq[] = {
	143*64,
	367,
	163,
	0,
	367,
	163,
	1,
	367,
	163,
	2,
	367,
	163,
	3,
	367,
	163,
	4,
	367,
	163,
	5,
	367,
	163,
	6,
	367,
	163,
	7,
	367,
	163,
	8,
	367,
	163,
	9,
	367,
	163,
	10,
	367,
	163,
	11,
	367,
	163,
	12,
	0
};

Compact fact2_store_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	203,	// xcood
	175,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	8,	// mouseSize_x
	238-175,	// mouseSize_y
	FACT2_STORE_ACTION,	// actionScript
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

Compact pipe4 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	59*64,	// frame
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
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 foreman_u_to_d[] = {
	43-8+63*64,
	42-8+63*64,
	41-8+63*64,
	0
};

uint16 foreman_right[] = {
	4,
	24+63*64,
	4,
	0,
	4,
	25+63*64,
	4,
	0,
	4,
	26+63*64,
	4,
	0,
	4,
	27+63*64,
	4,
	0,
	4,
	28+63*64,
	4,
	0,
	4,
	29+63*64,
	4,
	0,
	4,
	30+63*64,
	4,
	0,
	4,
	31+63*64,
	4,
	0,
	0
};

uint16 belts2_seq[] = {
	61*64,
	368,
	184,
	0,
	368,
	184,
	1,
	368,
	184,
	2,
	368,
	184,
	3,
	368,
	184,
	4,
	368,
	184,
	5,
	368,
	184,
	6,
	368,
	184,
	7,
	368,
	184,
	8,
	368,
	184,
	9,
	368,
	184,
	10,
	368,
	184,
	11,
	368,
	184,
	12,
	368,
	184,
	13,
	368,
	184,
	14,
	368,
	184,
	15,
	368,
	184,
	16,
	368,
	184,
	17,
	368,
	184,
	18,
	368,
	184,
	19,
	368,
	184,
	20,
	368,
	184,
	21,
	368,
	184,
	22,
	368,
	184,
	23,
	368,
	184,
	24,
	368,
	184,
	25,
	368,
	184,
	26,
	368,
	184,
	27,
	368,
	184,
	28,
	368,
	184,
	29,
	368,
	184,
	30,
	368,
	184,
	31,
	368,
	184,
	32,
	368,
	184,
	33,
	368,
	184,
	34,
	368,
	184,
	35,
	368,
	184,
	36,
	368,
	184,
	37,
	368,
	184,
	38,
	368,
	184,
	39,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	40,
	368,
	184,
	41,
	0
};

uint16 forfrisk_seq[] = {
	138*64,
	243,
	163,
	0,
	243,
	163,
	0,
	243,
	163,
	1,
	243,
	163,
	2,
	243,
	163,
	3,
	243,
	163,
	4,
	243,
	163,
	5,
	243,
	163,
	6,
	243,
	163,
	2,
	243,
	163,
	7,
	243,
	163,
	8,
	243,
	163,
	9,
	243,
	163,
	10,
	243,
	163,
	11,
	243,
	163,
	12,
	243,
	163,
	1,
	243,
	163,
	0,
	243,
	163,
	0,
	0
};

uint16 rs_foreman[] = {
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_MOUSE+ST_COLLISION,
	65535
};

uint16 pipe2_seq[] = {
	57*64,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	0X120,
	0XC9,
	0,
	0
};

uint16 rs_foster_13_12[] = {
	C_XCOOD,
	456,
	C_SCREEN,
	12,
	C_PLACE,
	ID_S12_FLOOR,
	65535
};

Compact s13_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	s13_floor_table,	// getToTable
	128,	// xcood
	216,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	71,	// mouseSize_y
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

Compact cogs = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	47*64,	// frame
	8255,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	1,	// mouseRel_x
	0,	// mouseRel_y
	21,	// mouseSize_x
	7,	// mouseSize_y
	COGS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 foreman_r_to_u[] = {
	45-8+63*64,
	0
};

Compact belts2 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	61*64,	// frame
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
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 foreman_d_to_l[] = {
	41-8+63*64,
	0
};

uint16 s13_chip_list[] = {
	173,
	111,
	136+DISK_2,
	137+DISK_2,
	63+DISK_3,
	64+DISK_3+0X8000,
	65+DISK_3+0X8000,
	68+DISK_3+0X8000,
	48+DISK_3,
	60+DISK_3,
	61+DISK_3,
	55+DISK_3+0X8000,
	62+DISK_3,
	66+DISK_3,
	67+DISK_3,
	89+DISK_3,
	138+DISK_3+0X8000,
	139+DISK_3+0X8000,
	143+DISK_3+0X8000,
	17+DISK_3,
	18+DISK_3,
	0
};

uint16 foreman_st_talk[] = {
	64*64,
	1,
	1,
	0,
	0
};

uint16 fact_console_dead[] = {
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
	0
};

uint16 foreman_st_down[] = {
	63*64,
	1,
	1,
	32,
	0
};

Compact lite2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	59*64,	// frame
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
	ANIMATE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact gears = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	48*64,	// frame
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
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 fost_cog_seq[] = {
	55*64,
	288,
	184,
	0,
	288,
	184,
	1,
	288,
	184,
	2,
	288,
	184,
	3,
	288,
	184,
	4,
	288,
	184,
	5,
	288,
	184,
	6,
	288,
	184,
	7,
	288,
	184,
	7,
	288,
	184,
	8,
	288,
	184,
	8,
	288,
	184,
	9,
	288,
	184,
	9,
	288,
	184,
	9,
	288,
	184,
	9,
	288,
	184,
	9,
	288,
	184,
	9,
	288,
	184,
	8,
	288,
	184,
	8,
	288,
	184,
	7,
	288,
	184,
	7,
	288,
	184,
	6,
	288,
	184,
	1,
	288,
	184,
	0,
	0
};

uint16 cogs_seq[] = {
	47*64,
	294,
	218,
	0,
	294,
	218,
	1,
	294,
	218,
	2,
	0
};

Compact pipe3 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	58*64,	// frame
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
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 lite2_seq[] = {
	67*64,
	417,
	206,
	0,
	417,
	206,
	0,
	417,
	206,
	0,
	417,
	206,
	0,
	417,
	206,
	0,
	417,
	206,
	0,
	417,
	206,
	1,
	417,
	206,
	1,
	417,
	206,
	1,
	417,
	206,
	1,
	0
};

Compact belts1 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	28,	// xcood
	36,	// ycood
	60*64,	// frame
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
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s13_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_LAMB,
	ID_LIFT_S7,
	ID_LIFT7_LIGHT,
	ID_FOREMAN,
	ID_TOP_BELT,
	ID_PIPES,
	ID_BOT_BELT,
	ID_WELDER,
	ID_COGS,
	ID_GEARS,
	ID_BELT1,
	ID_BELT2,
	ID_PIPE1,
	ID_PIPE2,
	ID_PIPE3,
	ID_PIPE4,
	ID_SENSOR,
	ID_LITE1,
	ID_LITE2,
	ID_FACT_CONSOLE,
	ID_COAT,
	ID_FACT2_SPY,
	177,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 s13_mouse[] = {
	ID_JOEY,
	ID_FOREMAN,
	ID_FACT2_L_EXIT,
	ID_FACT2_R_EXIT,
	ID_F2_STORE_EXIT,
	ID_COGS,
	ID_FACT2_SPY,
	ID_SENSORS,
	ID_FACT_CONSOLE,
	ID_S13_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sensors = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	212,	// xcood
	195,	// ycood
	0,	// frame
	8381,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	229-212,	// mouseSize_x
	241-195,	// mouseSize_y
	SENSORS_ACTION,	// actionScript
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

uint16 fact_console_seq[] = {
	89*64,
	418,
	196,
	0,
	418,
	196,
	1,
	418,
	196,
	2,
	418,
	196,
	3,
	418,
	196,
	3,
	418,
	196,
	3,
	418,
	196,
	3,
	418,
	196,
	3,
	418,
	196,
	3,
	418,
	196,
	4,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	6,
	418,
	196,
	7,
	418,
	196,
	7,
	418,
	196,
	7,
	418,
	196,
	8,
	418,
	196,
	8,
	418,
	196,
	8,
	418,
	196,
	8,
	418,
	196,
	8,
	418,
	196,
	9,
	418,
	196,
	10,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	11,
	418,
	196,
	12,
	418,
	196,
	13,
	418,
	196,
	14,
	418,
	196,
	15,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	17,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	18,
	418,
	196,
	19,
	418,
	196,
	20,
	418,
	196,
	21,
	418,
	196,
	22,
	418,
	196,
	23,
	418,
	196,
	24,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	25,
	418,
	196,
	26,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	27,
	418,
	196,
	28,
	418,
	196,
	29,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	5,
	418,
	196,
	30,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	31,
	418,
	196,
	32,
	418,
	196,
	33,
	418,
	196,
	34,
	418,
	196,
	35,
	418,
	196,
	35,
	418,
	196,
	35,
	418,
	196,
	35,
	418,
	196,
	35,
	418,
	196,
	35,
	418,
	196,
	36,
	418,
	196,
	37,
	418,
	196,
	38,
	418,
	196,
	39,
	418,
	196,
	40,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	418,
	196,
	16,
	0
};

uint16 foreman_down[] = {
	2,
	8+63*64,
	0,
	2,
	2,
	9+63*64,
	0,
	2,
	2,
	10+63*64,
	0,
	2,
	2,
	11+63*64,
	0,
	2,
	2,
	12+63*64,
	0,
	2,
	2,
	13+63*64,
	0,
	2,
	2,
	14+63*64,
	0,
	2,
	2,
	15+63*64,
	0,
	2,
	0
};

uint16 foreman_auto[32];

uint16 foreman_up[] = {
	2,
	0+63*64,
	0,
	65534,
	2,
	1+63*64,
	0,
	65534,
	2,
	2+63*64,
	0,
	65534,
	2,
	3+63*64,
	0,
	65534,
	2,
	4+63*64,
	0,
	65534,
	2,
	5+63*64,
	0,
	65534,
	2,
	6+63*64,
	0,
	65534,
	2,
	7+63*64,
	0,
	65534,
	0
};

uint16 foreman_st_left[] = {
	63*64,
	1,
	1,
	34,
	0
};

uint16 foreman_st_right[] = {
	63*64,
	1,
	1,
	38,
	0
};

uint16 foreman_u_to_l[] = {
	43-8+63*64,
	0
};

uint16 foreman_u_to_r[] = {
	45-8+63*64,
	0
};

uint16 foreman_d_to_u[] = {
	47-8+63*64,
	46-8+63*64,
	45-8+63*64,
	0
};

uint16 foreman_d_to_r[] = {
	47-8+63*64,
	0
};

uint16 foreman_r_to_d[] = {
	47-8+63*64,
	0
};

TurnTable foreman_turnTable0 = {
	{ // turnTableUp
		0,
		foreman_u_to_d,
		foreman_u_to_l,
		foreman_u_to_r,
		0
	},
	{ // turnTableDown
		foreman_d_to_u,
		0,
		foreman_d_to_l,
		foreman_d_to_r,
		0
	},
	{ // turnTableLeft
		foreman_l_to_u,
		foreman_l_to_d,
		0,
		foreman_l_to_r,
		0
	},
	{ // turnTableRight
		foreman_r_to_u,
		foreman_r_to_d,
		foreman_r_to_l,
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

MegaSet foreman_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	foreman_up,	// animUp
	foreman_down,	// animDown
	foreman_left,	// animLeft
	foreman_right,	// animRight
	foreman_st_up,	// standUp
	foreman_st_down,	// standDown
	foreman_st_left,	// standLeft
	foreman_st_right,	// standRight
	foreman_st_talk,	// standTalk
	&foreman_turnTable0
};

ExtCompact foreman_ext = {
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
	SP_COL_FOREMAN,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	foreman_auto,	// animScratch
	0,	// megaSet
	&foreman_megaSet0,
	0,
	0,
	0
};

Compact foreman = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_MOUSE+ST_COLLISION,	// status
	0,	// sync
	13,	// screen
	ID_S13_FLOOR,	// place
	0,	// getToTable
	408,	// xcood
	224+32,	// ycood
	63*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65520,	// mouseRel_x
	(int16) 65488,	// mouseRel_y
	24,	// mouseSize_x
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
	FOREMAN_LOGIC,	// baseSub
	0,	// baseSub_off
	&foreman_ext
};

uint16 lite1_seq[] = {
	66*64,
	417,
	203,
	0,
	417,
	203,
	0,
	417,
	203,
	0,
	417,
	203,
	1,
	417,
	203,
	1,
	417,
	203,
	1,
	0
};

uint16 pipe3_seq[] = {
	58*64,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	0X146,
	0XC4,
	0,
	0
};

uint16 opencoat_seq[] = {
	141*64,
	198,
	163,
	0,
	198,
	163,
	1,
	198,
	163,
	2,
	198,
	163,
	3,
	198,
	163,
	4,
	198,
	163,
	5,
	198,
	163,
	6,
	198,
	163,
	7,
	198,
	163,
	8,
	0
};

uint16 sensor_seq[] = {
	62*64,
	222,
	194,
	0,
	222,
	194,
	1,
	222,
	194,
	2,
	0
};

Compact fact2_spy_and_window = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	145,	// xcood
	213,	// ycood
	0,	// frame
	9218,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	171-145,	// mouseSize_x
	235-213,	// mouseSize_y
	WINDOW_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FACT2_SPY_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 belts1_seq[] = {
	60*64,
	217,
	174,
	0,
	217,
	174,
	1,
	217,
	174,
	2,
	217,
	174,
	3,
	217,
	174,
	4,
	217,
	174,
	5,
	217,
	174,
	6,
	217,
	174,
	7,
	217,
	174,
	8,
	217,
	174,
	9,
	217,
	174,
	10,
	217,
	174,
	11,
	217,
	174,
	12,
	217,
	174,
	13,
	217,
	174,
	14,
	217,
	174,
	15,
	217,
	174,
	16,
	217,
	174,
	17,
	217,
	174,
	18,
	217,
	174,
	19,
	217,
	174,
	20,
	217,
	174,
	21,
	217,
	174,
	22,
	217,
	174,
	23,
	217,
	174,
	24,
	217,
	174,
	25,
	217,
	174,
	26,
	217,
	174,
	27,
	217,
	174,
	28,
	217,
	174,
	29,
	217,
	174,
	30,
	217,
	174,
	31,
	217,
	174,
	32,
	217,
	174,
	33,
	217,
	174,
	34,
	217,
	174,
	35,
	217,
	174,
	36,
	217,
	174,
	37,
	217,
	174,
	38,
	217,
	174,
	39,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	40,
	217,
	174,
	41,
	0
};

Compact pipe2 = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_BACKGROUND+ST_LOGIC,	// status
	0,	// sync
	13,	// screen
	0,	// place
	0,	// getToTable
	0X120,	// xcood
	0XC9,	// ycood
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
	STD_FACT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s13_pal[] = {
	0,
	1024,
	1028,
	1028,
	1542,
	1028,
	1542,
	2054,
	1030,
	2054,
	2056,
	2056,
	2060,
	3078,
	2056,
	2062,
	2568,
	2570,
	2572,
	2568,
	3084,
	3084,
	4620,
	2060,
	3598,
	4620,
	2572,
	3598,
	4110,
	3598,
	4112,
	5648,
	3086,
	4114,
	4622,
	4112,
	4624,
	5650,
	3088,
	3102,
	6154,
	3600,
	3614,
	4620,
	5140,
	4630,
	5138,
	3094,
	5140,
	5138,
	5140,
	4634,
	6160,
	3604,
	4636,
	7696,
	4114,
	5654,
	8982,
	3600,
	6166,
	6160,
	5142,
	6164,
	9494,
	3088,
	5150,
	6674,
	5142,
	4135,
	5646,
	4634,
	6166,
	6168,
	6168,
	5667,
	7180,
	5656,
	4649,
	8462,
	4630,
	5157,
	8464,
	5142,
	6686,
	6158,
	7194,
	6177,
	10516,
	4116,
	6179,
	6676,
	6684,
	7706,
	6676,
	5662,
	7194,
	7196,
	6684,
	7713,
	8972,
	5658,
	5675,
	10002,
	3610,
	6693,
	7190,
	7710,
	7201,
	10010,
	5658,
	8476,
	7192,
	8478,
	8478,
	10008,
	6172,
	7713,
	9502,
	6686,
	7715,
	7710,
	8481,
	8489,
	9996,
	6686,
	8990,
	10524,
	6686,
	8481,
	8481,
	8995,
	10017,
	11032,
	4131,
	8489,
	8476,
	9507,
	9517,
	11020,
	7201,
	9515,
	10000,
	8483,
	9507,
	8995,
	9509,
	10531,
	11034,
	7715,
	10533,
	12060,
	3623,
	9005,
	9502,
	8489,
	10021,
	11559,
	7717,
	10023,
	9509,
	10535,
	10545,
	11534,
	8485,
	11047,
	11036,
	9511,
	10535,
	10537,
	7725,
	10539,
	10535,
	9517,
	11049,
	11051,
	11051,
	11055,
	11045,
	9007,
	11055,
	11049,
	11565,
	12077,
	11567,
	10545,
	12591,
	12591,
	11569,
	13619,
	13617,
	13111,
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

uint16 new_13_chip[] = {
	131+DISK_2,
	132+DISK_2,
	136+DISK_2,
	137+DISK_2,
	63+DISK_3,
	64+DISK_3+0X8000,
	65+DISK_3+0X8000,
	68+DISK_3+0X8000,
	48+DISK_3,
	60+DISK_3,
	61+DISK_3,
	55+DISK_3+0X8000,
	62+DISK_3,
	66+DISK_3,
	67+DISK_3,
	89+DISK_3,
	17+DISK_3,
	18+DISK_3,
	0
};

uint32 *grid13 = 0;

uint16 rs_foster_12_13[] = {
	C_XCOOD,
	120,
	C_YCOOD,
	280,
	C_SCREEN,
	13,
	C_PLACE,
	ID_S13_FLOOR,
	65535
};

uint16 s13_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	20+DISK_3,
	140+DISK_3,
	141+DISK_3,
	142+DISK_3,
	47+DISK_3,
	56+DISK_3,
	57+DISK_3,
	58+DISK_3,
	59+DISK_3,
	0
};

uint16 pipe4_seq[] = {
	59*64,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	348,
	241,
	0,
	0X14C,
	0X123,
	0,
	0
};

uint16 armsup_seq[] = {
	140*64,
	400,
	246,
	0,
	400,
	246,
	1,
	400,
	246,
	2,
	400,
	246,
	3,
	400,
	246,
	4,
	400,
	246,
	4,
	400,
	246,
	3,
	400,
	246,
	4,
	400,
	246,
	5,
	400,
	246,
	4,
	400,
	246,
	6,
	400,
	246,
	7,
	400,
	246,
	2,
	400,
	246,
	2,
	400,
	246,
	2,
	400,
	246,
	2,
	400,
	246,
	1,
	400,
	246,
	0,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
