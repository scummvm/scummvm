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

#ifndef SKY2COMPACT_H
#define SKY2COMPACT_H




namespace Sky {

namespace SkyCompact {

uint16 sml_loaderb_l_to_u[] = {
	24+158*64,
	0
};

uint16 joey_list_s2[] = {
	168,
	368,
	232,
	288,
	1,
	192,
	336,
	248,
	272,
	0,
	216,
	312,
	232,
	248,
	0,
	0
};

uint16 sml_loader_u_to_l[] = {
	24+158*64,
	0
};

uint32 *c4121 = (uint32*)&loader;

uint16 loader_right[] = {
	2,
	10+156*64,
	2,
	0,
	2,
	11+156*64,
	2,
	0,
	2,
	12+156*64,
	2,
	0,
	2,
	13+156*64,
	2,
	0,
	0
};

uint16 loaderb_st_left[] = {
	157*64,
	1,
	1,
	3,
	0
};

uint16 loader_r_to_d[] = {
	14+156*64,
	0
};

uint16 loader_st_down[] = {
	156*64,
	1,
	1,
	3,
	0
};

uint16 sml_loaderb_l_to_r[] = {
	20+159*64,
	159*64,
	19+159*64,
	0
};

uint16 loaderb_st_down[] = {
	157*64,
	1,
	1,
	0,
	0
};

uint16 sml_loaderb_r_to_u[] = {
	23+158*64,
	0
};

uint16 sml_loaderb_st_up[] = {
	158*64,
	1,
	1,
	0,
	0
};

uint16 loaderb_d_to_l[] = {
	12+157*64,
	0
};

uint16 sml_loaderb_st_right[] = {
	159*64,
	1,
	1,
	11,
	0
};

uint16 sml_loader_r_to_l[] = {
	23+158*64,
	0+158*64,
	24+158*64,
	0
};

uint16 sml_loader_r_to_u[] = {
	23+158*64,
	0
};

uint16 sml_loader_st_down[] = {
	158*64,
	1,
	1,
	3,
	0
};

uint16 loader_l_to_r[] = {
	17+156*64,
	3+156*64,
	14+156*64,
	0
};

uint16 sml_loaderb_d_to_l[] = {
	20+159*64,
	0
};

uint16 loaderb_r_to_u[] = {
	15+156*64,
	0
};

uint16 loader_left[] = {
	2,
	6+156*64,
	65534,
	0,
	2,
	7+156*64,
	65534,
	0,
	2,
	8+156*64,
	65534,
	0,
	2,
	9+156*64,
	65534,
	0,
	0
};

uint16 loaderb_right[] = {
	2,
	7+157*64,
	2,
	0,
	2,
	8+157*64,
	2,
	0,
	2,
	9+157*64,
	2,
	0,
	2,
	10+157*64,
	2,
	0,
	0
};

uint16 loaderb_r_to_d[] = {
	11+157*64,
	0
};

uint16 loader_d_to_u[] = {
	14+156*64,
	10+156*64,
	15+156*64,
	0
};

uint16 sml_loaderb_st_down[] = {
	159*64,
	1,
	1,
	0,
	0
};

uint16 sml_loaderb_up[] = {
	1,
	0+158*64,
	0,
	65535,
	1,
	1+158*64,
	0,
	65535,
	1,
	2+158*64,
	0,
	65535,
	0
};

uint16 sml_loader_d_to_r[] = {
	22+158*64,
	0
};

uint16 loader_st_up[] = {
	156*64,
	1,
	1,
	0,
	0
};

Compact right_exit_rm2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	388,	// xcood
	185,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	11,	// mouseSize_x
	71,	// mouseSize_y
	ER2_ACTION,	// actionScript
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

uint16 sml_loaderb_right[] = {
	1,
	11+159*64,
	1,
	0,
	1,
	12+159*64,
	1,
	0,
	1,
	13+159*64,
	1,
	0,
	1,
	14+159*64,
	1,
	0,
	1,
	15+159*64,
	1,
	0,
	1,
	16+159*64,
	1,
	0,
	1,
	17+159*64,
	1,
	0,
	1,
	18+159*64,
	1,
	0,
	0
};

uint16 loader_u_to_l[] = {
	16+156*64,
	0
};

uint16 sml_loader_st_right[] = {
	158*64,
	1,
	1,
	14,
	0
};

Compact panel = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	321,	// xcood
	187,	// ycood
	165*64,	// frame
	4186,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	12,	// mouseSize_x
	12,	// mouseSize_y
	PANEL2_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	PANEL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact alarm_flash = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	2,	// screen
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
	ALARM_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 loader_st_right[] = {
	156*64,
	1,
	1,
	10,
	0
};

uint16 sml_loader_l_to_u[] = {
	24+158*64,
	0
};

uint16 loader_auto[32];

uint16 loaderb_u_to_r[] = {
	15+156*64,
	0
};

uint16 sml_loader_u_to_r[] = {
	23+158*64,
	0
};

uint16 loaderb_d_to_r[] = {
	11+157*64,
	0
};

uint16 loaderb_u_to_l[] = {
	16+156*64,
	0
};

uint16 sml_loaderb_d_to_u[] = {
	19+159*64,
	11+159*64,
	23+158*64,
	0
};

uint16 loaderb_left[] = {
	2,
	3+157*64,
	65534,
	0,
	2,
	4+157*64,
	65534,
	0,
	2,
	5+157*64,
	65534,
	0,
	2,
	6+157*64,
	65534,
	0,
	0
};

uint16 sml_loaderb_r_to_d[] = {
	19+159*64,
	0
};

uint16 loader_l_to_u[] = {
	16+156*64,
	0
};

uint16 sml_loader_up[] = {
	1,
	0+158*64,
	0,
	65535,
	1,
	1+158*64,
	0,
	65535,
	1,
	2+158*64,
	0,
	65535,
	0
};

uint16 rs_test_jobs[] = {
	C_BASE_SUB,
	BASIC_JOBS,
	65535
};

uint16 loaderb_l_to_d[] = {
	12+157*64,
	0
};

uint16 sc2_mouse[] = {
	JOBS,
	ID_LOADER,
	ID_DEAD_LOADER,
	ID_JOEY,
	ID_ROBOT_SHELL,
	ID_JUNK1,
	ID_JUNK2,
	ID_LIGHT1,
	ID_PANEL,
	ID_TOP_LIFT,
	ID_HOLE,
	115,
	ID_L_EXIT_S2,
	ID_R_EXIT_S2,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 loaderb_up[] = {
	1,
	0+156*64,
	0,
	65535,
	1,
	1+156*64,
	0,
	65535,
	1,
	2+156*64,
	0,
	65535,
	0
};

uint16 loader_u_to_d[] = {
	16+156*64,
	6+156*64,
	17+156*64,
	0
};

uint16 sml_loader_u_to_d[] = {
	24+158*64,
	6+158*64,
	25+158*64,
	0
};

Compact hole = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	247,	// xcood
	252,	// ycood
	0,	// frame
	20+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	45,	// mouseSize_x
	19,	// mouseSize_y
	HOLE_ACTION,	// actionScript
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

uint16 loaderb_st_up[] = {
	156*64,
	1,
	1,
	0,
	0
};

uint16 sml_loader_down[] = {
	1,
	3+158*64,
	0,
	1,
	1,
	4+158*64,
	0,
	1,
	1,
	5+158*64,
	0,
	1,
	0
};

uint16 sml_loaderb_u_to_l[] = {
	24+158*64,
	0
};

uint16 loader_l_to_d[] = {
	17+156*64,
	0
};

uint16 *left_table2 = (uint16*)(&junk1);

uint16 sml_loaderb_u_to_d[] = {
	24+158*64,
	3+159*64,
	20+159*64,
	0
};

uint16 loader_u_to_r[] = {
	15+156*64,
	0
};

uint16 loaderb_l_to_r[] = {
	12+157*64,
	157*64,
	11+157*64,
	0
};

uint16 sml_loader_d_to_l[] = {
	25+158*64,
	0
};

uint16 loaderb_r_to_l[] = {
	15+156*64,
	0+156*64,
	17+156*64,
	0
};

uint16 top_lift_table[] = {
	ID_TOP_LIFT,
	RET_OK,
	115,
	LIFT_TO_FLOOR,
	ID_L_EXIT_S2,
	LIFT_TO_FLOOR,
	ID_R_EXIT_S2,
	LIFT_TO_FLOOR,
	21,
	LIFT_TO_FLOOR,
	22,
	LIFT_TO_FLOOR,
	ID_HOLE,
	LIFT_TO_FLOOR,
	ID_DEAD_LOADER,
	GET_TO_TRANSPORTER,
	ID_ROBOT_SHELL,
	GET_TO_SHELL,
	ID_PANEL,
	GT_PANEL2,
	ID_JUNK1,
	GT_JUNK1,
	ID_JUNK2,
	GT_JUNK2,
	65535
};

Compact robot_shell = {
	L_SCRIPT,	// logic
	ST_FOREGROUND+ST_MOUSE+ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	312,	// xcood
	294,	// ycood
	174*64,	// frame
	4546,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	16,	// mouseSize_y
	SHELL_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	JOEY_START,	// baseSub
	0,	// baseSub_off
	0
};

Compact junk1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	276,	// xcood
	293,	// ycood
	0,	// frame
	4546,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	306-276,	// mouseSize_x
	309-293,	// mouseSize_y
	JUNK1_ACTION,	// actionScript
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

uint16 loader_down[] = {
	1,
	3+156*64,
	0,
	1,
	1,
	4+156*64,
	0,
	1,
	1,
	5+156*64,
	0,
	1,
	0
};

uint16 loader_d_to_r[] = {
	14+156*64,
	0
};

uint16 top_lift_up[] = {
	148*64,
	244,
	250,
	6,
	244,
	250,
	5,
	244,
	250,
	4,
	244,
	250,
	3,
	244,
	250,
	2,
	244,
	250,
	1,
	244,
	250,
	0,
	0,
	0
};

uint16 sml_loader_st_up[] = {
	158*64,
	1,
	1,
	0,
	0
};

uint16 transporter_table[] = {
	115,
	STD_ADJOIN_FLOOR,
	ID_L_EXIT_S2,
	GET_TO_EL2,
	ID_R_EXIT_S2,
	GET_TO_ER2,
	ID_TOP_LIFT,
	GET_TO_LIFTER,
	21,
	GET_TO_TALK21,
	22,
	GET_TO_TALK22,
	ID_LIGHT1,
	GET_TO_LIGHT1,
	ID_HOLE,
	GET_TO_HOLE,
	ID_DEAD_LOADER,
	GET_TO_TRANSPORTER,
	ID_ROBOT_SHELL,
	GET_TO_SHELL,
	ID_PANEL,
	GT_PANEL2,
	ID_JUNK1,
	GT_JUNK1,
	ID_JUNK2,
	GT_JUNK2,
	65535
};

uint16 sml_loader_r_to_d[] = {
	22+158*64,
	0
};

uint16 loaderb_st_right[] = {
	157*64,
	1,
	1,
	7,
	0
};

uint16 chip_list_s2[] = {
	148,
	155,
	156+0X000,
	157+0X000,
	160,
	166+0X8000,
	173,
	111,
	117,
	134,
	150,
	227,
	0
};

uint32 *exit_table = (uint32*)(&junk1);

uint16 sml_loaderb_r_to_l[] = {
	23+158*64,
	0+158*64,
	24+158*64,
	0
};

Compact dead_loader = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	115,	// place
	transporter_table,	// getToTable
	0XD2,	// xcood
	0XF0,	// ycood
	156*64+6,	// frame
	4426,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65488,	// mouseRel_y
	20,	// mouseSize_x
	48,	// mouseSize_y
	TRANSPORTER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	DEAD_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 top_lift_2_up[] = {
	149*64,
	284,
	196,
	17,
	284,
	196,
	16,
	284,
	196,
	15,
	284,
	196,
	14,
	284,
	196,
	13,
	284,
	196,
	12,
	284,
	196,
	11,
	284,
	196,
	10,
	284,
	196,
	9,
	284,
	196,
	8,
	284,
	196,
	7,
	284,
	196,
	6,
	284,
	196,
	5,
	284,
	196,
	4,
	284,
	196,
	3,
	284,
	196,
	2,
	284,
	196,
	1,
	284,
	196,
	0,
	0,
	0
};

uint16 loaderb_l_to_u[] = {
	16+156*64,
	0
};

uint16 loaderb_down[] = {
	1,
	0+157*64,
	0,
	1,
	1,
	1+157*64,
	0,
	1,
	1,
	2+157*64,
	0,
	1,
	0
};

uint16 loader_r_to_u[] = {
	15+156*64,
	0
};

uint16 top_lift_2_down[] = {
	149*64,
	284,
	196,
	0,
	284,
	196,
	1,
	284,
	196,
	2,
	284,
	196,
	3,
	284,
	196,
	4,
	284,
	196,
	5,
	284,
	196,
	6,
	284,
	196,
	7,
	284,
	196,
	8,
	284,
	196,
	9,
	284,
	196,
	10,
	284,
	196,
	11,
	284,
	196,
	12,
	284,
	196,
	13,
	284,
	196,
	14,
	284,
	196,
	15,
	284,
	196,
	16,
	284,
	196,
	17,
	0
};

Compact left_exit_rm2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	left_table2,	// getToTable
	128,	// xcood
	164,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	150,	// mouseSize_y
	EL2_ACTION,	// actionScript
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

uint16 loader_up[] = {
	1,
	0+156*64,
	0,
	65535,
	1,
	1+156*64,
	0,
	65535,
	1,
	2+156*64,
	0,
	65535,
	0
};

uint16 loader_st_left[] = {
	156*64,
	1,
	1,
	6,
	0
};

uint16 loader_d_to_l[] = {
	17+156*64,
	0
};

uint16 loader_r_to_l[] = {
	15+156*64,
	0+156*64,
	16+156*64,
	0
};

uint16 loaderb_u_to_d[] = {
	17+156*64,
	3+157*64,
	12+157*64,
	0
};

uint16 loaderb_d_to_u[] = {
	11+157*64,
	7+157*64,
	15+156*64,
	0
};

uint16 sml_loader_left[] = {
	1,
	6+158*64,
	65535,
	0,
	1,
	7+158*64,
	65535,
	0,
	1,
	8+158*64,
	65535,
	0,
	1,
	9+158*64,
	65535,
	0,
	1,
	10+158*64,
	65535,
	0,
	1,
	11+158*64,
	65535,
	0,
	1,
	12+158*64,
	65535,
	0,
	1,
	13+158*64,
	65535,
	0,
	0
};

uint16 sml_loader_right[] = {
	1,
	14+158*64,
	1,
	0,
	1,
	15+158*64,
	1,
	0,
	1,
	16+158*64,
	1,
	0,
	1,
	17+158*64,
	1,
	0,
	1,
	18+158*64,
	1,
	0,
	1,
	19+158*64,
	1,
	0,
	1,
	20+158*64,
	1,
	0,
	1,
	21+158*64,
	1,
	0,
	0
};

uint16 sml_loader_st_left[] = {
	158*64,
	1,
	1,
	6,
	0
};

uint16 sml_loader_d_to_u[] = {
	22+158*64,
	14+158*64,
	23+158*64,
	0
};

uint16 sml_loader_l_to_d[] = {
	25+158*64,
	0
};

uint16 sml_loader_l_to_r[] = {
	25+158*64,
	3+158*64,
	22+158*64,
	0
};

uint16 sml_loaderb_down[] = {
	1,
	0+159*64,
	0,
	1,
	1,
	1+159*64,
	0,
	1,
	1,
	2+159*64,
	0,
	1,
	0
};

uint16 sml_loaderb_left[] = {
	1,
	3+159*64,
	65535,
	0,
	1,
	4+159*64,
	65535,
	0,
	1,
	5+159*64,
	65535,
	0,
	1,
	6+159*64,
	65535,
	0,
	1,
	7+159*64,
	65535,
	0,
	1,
	8+159*64,
	65535,
	0,
	1,
	9+159*64,
	65535,
	0,
	1,
	10+159*64,
	65535,
	0,
	0
};

uint16 sml_loaderb_st_left[] = {
	159*64,
	1,
	1,
	3,
	0
};

uint16 sml_loaderb_u_to_r[] = {
	23+158*64,
	0
};

uint16 sml_loaderb_d_to_r[] = {
	19+159*64,
	0
};

uint16 sml_loaderb_l_to_d[] = {
	20+159*64,
	0
};

TurnTable loader_turnTable0 = {
	{ // turnTableUp
		0,
		loader_u_to_d,
		loader_u_to_l,
		loader_u_to_r,
		0
	},
	{ // turnTableDown
		loader_d_to_u,
		0,
		loader_d_to_l,
		loader_d_to_r,
		0
	},
	{ // turnTableLeft
		loader_l_to_u,
		loader_l_to_d,
		0,
		loader_l_to_r,
		0
	},
	{ // turnTableRight
		loader_r_to_u,
		loader_r_to_d,
		loader_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		0,
		0,
		0,
		0,
		0
	},
};

TurnTable loader_turnTable1 = {
	{ // turnTableUp
		0,
		loaderb_u_to_d,
		loaderb_u_to_l,
		loaderb_u_to_r,
		0
	},
	{ // turnTableDown
		loaderb_d_to_u,
		0,
		loaderb_d_to_l,
		loaderb_d_to_r,
		0
	},
	{ // turnTableLeft
		loaderb_l_to_u,
		loaderb_l_to_d,
		0,
		loaderb_l_to_r,
		0
	},
	{ // turnTableRight
		loaderb_r_to_u,
		loaderb_r_to_d,
		loaderb_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		0,
		0,
		0,
		0,
		0
	},
};

TurnTable loader_turnTable2 = {
	{ // turnTableUp
		0,
		sml_loader_u_to_d,
		sml_loader_u_to_l,
		sml_loader_u_to_r,
		0
	},
	{ // turnTableDown
		sml_loader_d_to_u,
		0,
		sml_loader_d_to_l,
		sml_loader_d_to_r,
		0
	},
	{ // turnTableLeft
		sml_loader_l_to_u,
		sml_loader_l_to_d,
		0,
		sml_loader_l_to_r,
		0
	},
	{ // turnTableRight
		sml_loader_r_to_u,
		sml_loader_r_to_d,
		sml_loader_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		0,
		0,
		0,
		0,
		0
	},
};

TurnTable loader_turnTable3 = {
	{ // turnTableUp
		0,
		sml_loaderb_u_to_d,
		sml_loaderb_u_to_l,
		sml_loaderb_u_to_r,
		0
	},
	{ // turnTableDown
		sml_loaderb_d_to_u,
		0,
		sml_loaderb_d_to_l,
		sml_loaderb_d_to_r,
		0
	},
	{ // turnTableLeft
		sml_loaderb_l_to_u,
		sml_loaderb_l_to_d,
		0,
		sml_loaderb_l_to_r,
		0
	},
	{ // turnTableRight
		sml_loaderb_r_to_u,
		sml_loaderb_r_to_d,
		sml_loaderb_r_to_l,
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

MegaSet loader_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	loader_up,	// animUp
	loader_down,	// animDown
	loader_left,	// animLeft
	loader_right,	// animRight
	loader_st_up,	// standUp
	loader_st_down,	// standDown
	loader_st_left,	// standLeft
	loader_st_right,	// standRight
	0,	// standTalk
	&loader_turnTable0,
};

MegaSet loader_megaSet1 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	loaderb_up,	// animUp
	loaderb_down,	// animDown
	loaderb_left,	// animLeft
	loaderb_right,	// animRight
	loaderb_st_up,	// standUp
	loaderb_st_down,	// standDown
	loaderb_st_left,	// standLeft
	loaderb_st_right,	// standRight
	0,	// standTalk
	&loader_turnTable1,
};

MegaSet loader_megaSet2 = {
	0,	// gridWidth
	0,	// colOffset
	8,	// colWidth
	8,	// lastChr
	sml_loader_up,	// animUp
	sml_loader_down,	// animDown
	sml_loader_left,	// animLeft
	sml_loader_right,	// animRight
	sml_loader_st_up,	// standUp
	sml_loader_st_down,	// standDown
	sml_loader_st_left,	// standLeft
	sml_loader_st_right,	// standRight
	0,	// standTalk
	&loader_turnTable2,
};

MegaSet loader_megaSet3 = {
	0,	// gridWidth
	0,	// colOffset
	8,	// colWidth
	8,	// lastChr
	sml_loaderb_up,	// animUp
	sml_loaderb_down,	// animDown
	sml_loaderb_left,	// animLeft
	sml_loaderb_right,	// animRight
	sml_loaderb_st_up,	// standUp
	sml_loaderb_st_down,	// standDown
	sml_loaderb_st_left,	// standLeft
	sml_loaderb_st_right,	// standRight
	0,	// standTalk
	&loader_turnTable3
};

ExtCompact loader_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	2,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96,	// spWidth_xx
	SP_COL_LOADER,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	loader_auto,	// animScratch
	0*NEXT_MEGA_SET,	// megaSet
	&loader_megaSet0,
	&loader_megaSet1,
	&loader_megaSet2,
	&loader_megaSet3
};

Compact loader = {
	1,	// logic
	0,	// status
	0,	// sync
	2,	// screen
	115,	// place
	0,	// getToTable
	0XD2,	// xcood
	0XF0,	// ycood
	156*64+6,	// frame
	4426,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65488,	// mouseRel_y
	20,	// mouseSize_x
	48,	// mouseSize_y
	TRANS_ALIVE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LOADER_START,	// baseSub
	0,	// baseSub_off
	&loader_ext
};

Compact junk2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	332,	// xcood
	289,	// ycood
	0,	// frame
	4546,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	355-332,	// mouseSize_x
	315-289,	// mouseSize_y
	JUNK2_ACTION,	// actionScript
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

uint16 fast_list_sc2[] = {
	12,
	51,
	52,
	53,
	54,
	135+0X8000,
	153+0X8000,
	200+0X8000,
	174,
	201+0X8000,
	112,
	113,
	114,
	163,
	164,
	165,
	149,
	0
};

Compact light_one = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	216,	// xcood
	188,	// ycood
	163*64,	// frame
	4186,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	12,	// mouseSize_x
	8,	// mouseSize_y
	LIGHT1_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc2_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	JOBS,
	ID_CUPBOARD,
	ID_KNOB,
	ID_LAZER,
	ID_TOP_LIFT,
	4344,
	ID_TOP_BARREL,
	ID_LOADER,
	ID_UPLOAD,
	ID_DEAD_LOADER,
	ID_LIGHT1,
	ID_PANEL,
	ID_ALARM,
	ID_ROBOT_SHELL,
	ID_PRESS,
	0XFFFF,
	ID_MENU_LOGIC
};

Compact top_lift_2 = {
	L_SCRIPT,	// logic
	ST_SORT+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	284,	// xcood
	196,	// ycood
	149*64,	// frame
	19+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
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
	TOP_LIFT_2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact top_lift = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	top_lift_table,	// getToTable
	244,	// xcood
	250,	// ycood
	148*64,	// frame
	19+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
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
	TOP_LIFT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 top_lift_down[] = {
	148*64,
	244,
	250,
	0,
	244,
	250,
	1,
	244,
	250,
	2,
	244,
	250,
	3,
	244,
	250,
	4,
	244,
	250,
	5,
	244,
	250,
	6,
	0
};

Compact top_barrel = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	2,	// screen
	0,	// place
	0,	// getToTable
	262,	// xcood
	240,	// ycood
	155*64,	// frame
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
	TOP_BARREL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
