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

#ifndef SKY92COMP_H
#define SKY92COMP_H




namespace Sky {

namespace SkyCompact {

Compact bridge_e = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	340,	// xcood
	289,	// ycood
	4+44*64,	// frame
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
	BRIDGE_E_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_l92 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	152,	// xcood
	136,	// ycood
	54*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	26,	// mouseRel_y
	9,	// mouseSize_x
	153,	// mouseSize_y
	DOOR_L92_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L92_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 slab8_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB5,
	GET_TO_SLAB,
	ID_SLAB7,
	GET_TO_SLAB,
	ID_SLAB8,
	RET_OK,
	65535
};

Compact slab8 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab8_table,	// getToTable
	266,	// xcood
	248,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	42,	// mouseSize_x
	14,	// mouseSize_y
	SLAB_6_9_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB8_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 slab3_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB2,
	GET_TO_SLAB,
	ID_SLAB3,
	RET_OK,
	ID_SLAB6,
	GET_TO_SLAB,
	65535
};

uint16 slab2_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB1,
	GET_TO_SLAB,
	ID_SLAB2,
	RET_OK,
	ID_SLAB3,
	GET_TO_SLAB,
	ID_SLAB5,
	GET_TO_SLAB,
	65535
};

uint16 slab4_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB1,
	GET_TO_SLAB,
	ID_SLAB4,
	RET_OK,
	ID_SLAB5,
	GET_TO_SLAB,
	ID_SLAB7,
	GET_TO_SLAB,
	65535
};

Compact slab4 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab4_table,	// getToTable
	191,	// xcood
	267,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	51,	// mouseSize_x
	21,	// mouseSize_y
	SLAB_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_l92_anim[] = {
	54*64,
	152,
	136,
	0,
	152,
	136,
	1,
	152,
	136,
	2,
	152,
	136,
	3,
	152,
	136,
	4,
	152,
	136,
	5,
	152,
	136,
	6,
	152,
	136,
	7,
	152,
	136,
	8,
	152,
	136,
	9,
	152,
	136,
	10,
	152,
	136,
	11,
	152,
	136,
	12,
	152,
	136,
	13,
	152,
	136,
	14,
	152,
	136,
	15,
	152,
	136,
	16,
	152,
	136,
	17,
	152,
	136,
	18,
	152,
	136,
	19,
	152,
	136,
	20,
	152,
	136,
	21,
	152,
	136,
	22,
	0
};

uint16 door_r92_anim[] = {
	129*64,
	323,
	136,
	0,
	323,
	136,
	1,
	323,
	136,
	2,
	323,
	136,
	3,
	323,
	136,
	4,
	323,
	136,
	5,
	323,
	136,
	6,
	323,
	136,
	7,
	323,
	136,
	8,
	323,
	136,
	9,
	323,
	136,
	10,
	323,
	136,
	11,
	323,
	136,
	12,
	323,
	136,
	13,
	323,
	136,
	14,
	323,
	136,
	15,
	323,
	136,
	16,
	323,
	136,
	17,
	323,
	136,
	18,
	323,
	136,
	19,
	323,
	136,
	20,
	323,
	136,
	21,
	323,
	136,
	22,
	0
};

uint16 slab6_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB3,
	GET_TO_SLAB,
	ID_SLAB5,
	GET_TO_SLAB,
	ID_SLAB6,
	RET_OK,
	ID_SLAB9,
	GET_TO_SLAB,
	65535
};

Compact slab6 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab6_table,	// getToTable
	332,	// xcood
	267,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	53,	// mouseSize_x
	21,	// mouseSize_y
	SLAB_6_9_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB6_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_r92 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	323,	// xcood
	136,	// ycood
	129*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	26,	// mouseRel_y
	26,	// mouseSize_x
	86,	// mouseSize_y
	DOOR_R92_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_R92_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact bridge_h = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	325,	// xcood
	263,	// ycood
	7+44*64,	// frame
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
	BRIDGE_H_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact bridge_f = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	236,	// xcood
	268,	// ycood
	5+44*64,	// frame
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
	BRIDGE_F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 slab7_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB4,
	GET_TO_SLAB,
	ID_SLAB7,
	RET_OK,
	ID_SLAB8,
	GET_TO_SLAB,
	65535
};

Compact bridge_a = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	217,	// xcood
	295,	// ycood
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
	BRIDGE_A_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_r92r_anim[] = {
	130*64,
	348,
	136,
	0,
	348,
	136,
	1,
	348,
	136,
	2,
	348,
	136,
	3,
	348,
	136,
	4,
	348,
	136,
	5,
	348,
	136,
	6,
	348,
	136,
	7,
	348,
	136,
	8,
	348,
	136,
	9,
	348,
	136,
	10,
	348,
	136,
	11,
	348,
	136,
	12,
	348,
	136,
	13,
	348,
	136,
	14,
	348,
	136,
	15,
	348,
	136,
	16,
	348,
	136,
	17,
	348,
	136,
	18,
	348,
	136,
	19,
	348,
	136,
	20,
	348,
	136,
	21,
	348,
	136,
	22,
	0
};

Compact bridge_g = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	266,	// xcood
	263,	// ycood
	6+44*64,	// frame
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
	BRIDGE_G_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_l92f = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_SORT,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	147,	// xcood
	136,	// ycood
	55*64,	// frame
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
	DOOR_L92F_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact bridge_b = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	320,	// xcood
	295,	// ycood
	1+44*64,	// frame
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
	BRIDGE_B_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 slab9_table[] = {
	ID_SLAB6,
	GET_TO_SLAB,
	ID_SLAB9,
	RET_OK,
	ID_DOOR_R92,
	GET_TO_DOOR_R92,
	65535
};

uint16 slab1_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB1,
	RET_OK,
	ID_SLAB2,
	GET_TO_SLAB,
	ID_SLAB4,
	GET_TO_SLAB,
	ID_DOOR_L92,
	GET_TO_DOOR_L92,
	65535
};

Compact slab1 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab1_table,	// getToTable
	145,	// xcood
	294,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	79,	// mouseSize_x
	29,	// mouseSize_y
	SLAB_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact red_circle = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	2+62*64,	// frame
	24603,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	28,	// mouseSize_x
	6,	// mouseSize_y
	RED_CIRCLE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	RED_CIRCLE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact slab2 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab2_table,	// getToTable
	250,	// xcood
	294,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	76,	// mouseSize_x
	29,	// mouseSize_y
	SLAB_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc92_logic[] = {
	ID_BLUE_FOSTER,
	ID_BRIDGE_A,
	ID_BRIDGE_B,
	ID_BRIDGE_C,
	ID_BRIDGE_D,
	ID_BRIDGE_E,
	ID_BRIDGE_F,
	ID_BRIDGE_G,
	ID_BRIDGE_H,
	ID_SLAB1,
	ID_SLAB2,
	ID_SLAB3,
	ID_SLAB4,
	ID_SLAB5,
	ID_SLAB6,
	ID_SLAB7,
	ID_SLAB8,
	ID_SLAB9,
	ID_GREEN_CIRCLE,
	ID_RED_CIRCLE,
	ID_DOOR_L92,
	ID_DOOR_L92F,
	ID_DOOR_R92,
	ID_DOOR_R92R,
	ID_EYEBALL_90,
	0XFFFF,
	ID_LINC_MENU_LOGIC
};

uint16 chip_list_sc92[] = {
	IT_BRIDGES+DISK_4,
	IT_CIRCLES+DISK_4,
	IT_DOOR_L92+DISK_4,
	IT_DOOR_L92F+DISK_4,
	IT_DOOR_R92+DISK_4,
	IT_DOOR_R92R+DISK_4,
	0
};

uint16 pal92[] = {
	0,
	3584,
	8960,
	1044,
	7211,
	12812,
	5413,
	11066,
	15390,
	29,
	8192,
	0,
	36,
	10240,
	0,
	1550,
	5888,
	10,
	3616,
	9472,
	20,
	7211,
	15360,
	39,
	43,
	0,
	0,
	15928,
	11071,
	13873,
	10015,
	6188,
	8990,
	5135,
	2587,
	5903,
	8458,
	783,
	1303,
	4096,
	12032,
	0,
	30,
	5632,
	0,
	6183,
	12058,
	0,
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

uint16 fast_list_sc92[] = {
	IT_BLUE_FOSTER+DISK_4,
	IT_LOGOFF+0X8000+DISK_4,
	IT_WINDOW+0X8000+DISK_4,
	IT_INFO_BUTTON+DISK_4,
	IT_LINK_ARROWS+DISK_4,
	IT_LINK_OBJECTS+DISK_4,
	IT_ENTER_TOP+DISK_4+0X8000,
	IT_EXIT_TOP+DISK_4+0X8000,
	IT_CROUCH_RIGHT+DISK_4,
	IT_SC92_LAYER_0+DISK_4,
	IT_SC92_LAYER_1+DISK_4,
	IT_SC92_GRID_1+DISK_4,
	0
};

uint16 door_l92f_anim[] = {
	55*64,
	147,
	136,
	0,
	147,
	136,
	1,
	147,
	136,
	2,
	147,
	136,
	3,
	147,
	136,
	4,
	147,
	136,
	5,
	147,
	136,
	6,
	147,
	136,
	7,
	147,
	136,
	8,
	147,
	136,
	9,
	147,
	136,
	10,
	147,
	136,
	11,
	147,
	136,
	12,
	147,
	136,
	13,
	147,
	136,
	14,
	147,
	136,
	15,
	147,
	136,
	16,
	147,
	136,
	17,
	147,
	136,
	18,
	147,
	136,
	19,
	147,
	136,
	20,
	147,
	136,
	21,
	147,
	136,
	22,
	0
};

Compact slab9 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab9_table,	// getToTable
	319,	// xcood
	248,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	41,	// mouseSize_x
	14,	// mouseSize_y
	SLAB_6_9_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB9_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact door_r92r = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	348,	// xcood
	136,	// ycood
	130*64,	// frame
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
	DOOR_R92R_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 slab5_table[] = {
	ID_GREEN_CIRCLE,
	GET_TO_CIRCLE,
	ID_RED_CIRCLE,
	GET_TO_CIRCLE,
	ID_SLAB2,
	GET_TO_SLAB,
	ID_SLAB4,
	GET_TO_SLAB,
	ID_SLAB5,
	RET_OK,
	ID_SLAB6,
	GET_TO_SLAB,
	ID_SLAB8,
	GET_TO_SLAB,
	65535
};

Compact slab5 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab5_table,	// getToTable
	260,	// xcood
	267,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	55,	// mouseSize_x
	21,	// mouseSize_y
	SLAB_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB5_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact slab7 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab7_table,	// getToTable
	214,	// xcood
	248,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	43,	// mouseSize_x
	14,	// mouseSize_y
	SLAB_6_9_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB7_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc92_mouse[] = {
	ID_GREEN_CIRCLE,
	ID_RED_CIRCLE,
	ID_DOOR_L92,
	ID_DOOR_R92,
	ID_SLAB1,
	ID_SLAB2,
	ID_SLAB3,
	ID_SLAB4,
	ID_SLAB5,
	ID_SLAB6,
	ID_SLAB7,
	ID_SLAB8,
	ID_SLAB9,
	0XFFFF,
	ID_LINC_MENU_MOUSE
};

uint16 rs_foster_92_93[] = {
	C_SCREEN,
	93,
	C_PLACE,
	ID_SC93_FLOOR,
	C_XCOOD,
	184,
	C_YCOOD,
	264,
	C_FRAME,
	46+182*64,
	C_DIR,
	RIGHT,
	65535
};

Compact green_circle = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	62*64,	// frame
	24603,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	2,	// mouseRel_y
	28,	// mouseSize_x
	6,	// mouseSize_y
	GREEN_CIRCLE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	GREEN_CIRCLE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_92_91[] = {
	C_SCREEN,
	91,
	C_PLACE,
	ID_SC91_FLOOR,
	C_XCOOD,
	392,
	C_YCOOD,
	264,
	65535
};

Compact bridge_c = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	173,	// xcood
	289,	// ycood
	2+44*64,	// frame
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
	BRIDGE_C_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact bridge_d = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	92,	// screen
	0,	// place
	0,	// getToTable
	257,	// xcood
	289,	// ycood
	3+44*64,	// frame
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
	BRIDGE_D_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact slab3 = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	92,	// screen
	0,	// place
	slab3_table,	// getToTable
	350,	// xcood
	294,	// ycood
	0,	// frame
	0,	// cursorText
	SLAB_ON,	// mouseOn
	SLAB_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	70,	// mouseSize_x
	29,	// mouseSize_y
	SLAB_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SLAB3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
