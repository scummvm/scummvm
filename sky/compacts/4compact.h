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

#ifndef SKY4COMPACT_H
#define SKY4COMPACT_H




namespace Sky {

namespace SkyCompact {

uint16 jobs_u_to_l[] = {
	43-8+117*64,
	0
};

uint16 m_jobs_left[] = {
	2,
	16+270*64,
	65534,
	0,
	2,
	17+270*64,
	65534,
	0,
	2,
	18+270*64,
	65534,
	0,
	2,
	19+270*64,
	65534,
	0,
	2,
	20+270*64,
	65534,
	0,
	2,
	21+270*64,
	65534,
	0,
	2,
	22+270*64,
	65534,
	0,
	2,
	23+270*64,
	65534,
	0,
	0
};

uint16 jobs_l_to_u[] = {
	43-8+117*64,
	0
};

uint16 m_jobs_u_to_d[] = {
	43-8+270*64,
	42-8+270*64,
	41-8+270*64,
	0
};

Compact sarnie_s4 = {
	0,	// logic
	ST_BACKGROUND+ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	252,	// xcood
	219,	// ycood
	61*64,	// frame
	4+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	SARNIE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

uint16 m_jobs_l_to_d[] = {
	41-8+270*64,
	0
};

uint16 s4_talk_table[] = {
	4106,
	RET_OK,
	4107,
	RET_OK,
	ID_S4_FLOOR,
	STD_ADJOIN_FLOOR,
	ID_S4_L_EXIT,
	GET_TO_EL4,
	ID_CUPBOARD,
	GET_TO_CUPBOARD,
	ID_SARNIE,
	GET_TO_SHELVES,
	ID_SPANNER,
	GET_TO_SHELVES,
	ID_KNOB,
	GET_TO_KNOB,
	ID_CHUCK,
	GET_TO_CHUCK,
	ID_MONITORS,
	GET_TO_SCREENS,
	ID_BUTTONS,
	GET_TO_BUTTONS,
	ID_POSTCARD,
	GET_TO_POSTCARD,
	ID_NOTICE4,
	GET_TO_POSTCARD,
	ID_TV_SCREENS,
	GET_TO_MONITOR,
	ID_LAZER,
	GT_LAZER,
	65535
};

uint32 *l_talk_s4 = (uint32*)&r_talk_s4;

uint16 chip_list_sc4[] = {
	56,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	143,
	144,
	145,
	146,
	151,
	152,
	173,
	111,
	141,
	150,
	142,
	117,
	134,
	0
};

Compact r_talk_s4 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	s4_talk_table,	// getToTable
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
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

Compact monitors_s4 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	151*64,	// frame
	27+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	22,	// mouseSize_x
	12,	// mouseSize_y
	TV_1_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	MONITOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 m_jobs_down[] = {
	1,
	8+270*64,
	0,
	1,
	1,
	9+270*64,
	0,
	1,
	1,
	10+270*64,
	0,
	1,
	1,
	11+270*64,
	0,
	1,
	1,
	12+270*64,
	0,
	1,
	1,
	13+270*64,
	0,
	1,
	1,
	14+270*64,
	0,
	1,
	1,
	15+270*64,
	0,
	1,
	0
};

uint16 jobs_auto[32];

uint16 m_jobs_up[] = {
	1,
	0+270*64,
	0,
	65535,
	1,
	1+270*64,
	0,
	65535,
	1,
	2+270*64,
	0,
	65535,
	1,
	3+270*64,
	0,
	65535,
	1,
	4+270*64,
	0,
	65535,
	1,
	5+270*64,
	0,
	65535,
	1,
	6+270*64,
	0,
	65535,
	1,
	7+270*64,
	0,
	65535,
	0
};

uint16 m_jobs_right[] = {
	2,
	24+270*64,
	2,
	0,
	2,
	25+270*64,
	2,
	0,
	2,
	26+270*64,
	2,
	0,
	2,
	27+270*64,
	2,
	0,
	2,
	28+270*64,
	2,
	0,
	2,
	29+270*64,
	2,
	0,
	2,
	30+270*64,
	2,
	0,
	2,
	31+270*64,
	2,
	0,
	0
};

uint16 m_jobs_st_up[] = {
	270*64,
	1,
	1,
	36,
	0
};

uint16 m_jobs_st_down[] = {
	270*64,
	1,
	1,
	32,
	0
};

uint16 m_jobs_st_left[] = {
	270*64,
	1,
	1,
	34,
	0
};

uint16 m_jobs_st_right[] = {
	270*64,
	1,
	1,
	38,
	0
};

uint16 m_jobs_st_talk[] = {
	273*64,
	1,
	1,
	0,
	0
};

uint16 m_jobs_u_to_l[] = {
	43-8+270*64,
	0
};

uint16 m_jobs_u_to_r[] = {
	45-8+270*64,
	0
};

uint16 m_jobs_d_to_u[] = {
	47-8+270*64,
	46-8+270*64,
	45-8+270*64,
	0
};

uint16 m_jobs_d_to_l[] = {
	41-8+270*64,
	0
};

uint16 m_jobs_d_to_r[] = {
	47-8+270*64,
	0
};

uint16 m_jobs_l_to_u[] = {
	43-8+270*64,
	0
};

uint16 m_jobs_l_to_r[] = {
	41-8+270*64,
	40-8+270*64,
	47-8+270*64,
	0
};

uint16 m_jobs_r_to_u[] = {
	45-8+270*64,
	0
};

uint16 m_jobs_r_to_d[] = {
	47-8+270*64,
	0
};

uint16 m_jobs_r_to_l[] = {
	45-8+270*64,
	44-8+270*64,
	43-8+270*64,
	0
};

uint16 jobs_up[] = {
	2,
	0+117*64,
	0,
	65534,
	2,
	1+117*64,
	0,
	65534,
	2,
	2+117*64,
	0,
	65534,
	2,
	3+117*64,
	0,
	65534,
	2,
	4+117*64,
	0,
	65534,
	2,
	5+117*64,
	0,
	65534,
	2,
	6+117*64,
	0,
	65534,
	2,
	7+117*64,
	0,
	65534,
	0
};

uint16 jobs_down[] = {
	2,
	8+117*64,
	0,
	2,
	2,
	9+117*64,
	0,
	2,
	2,
	10+117*64,
	0,
	2,
	2,
	11+117*64,
	0,
	2,
	2,
	12+117*64,
	0,
	2,
	2,
	13+117*64,
	0,
	2,
	2,
	14+117*64,
	0,
	2,
	2,
	15+117*64,
	0,
	2,
	0
};

uint16 jobs_left[] = {
	4,
	16+117*64,
	65532,
	0,
	4,
	17+117*64,
	65532,
	0,
	4,
	18+117*64,
	65532,
	0,
	4,
	19+117*64,
	65532,
	0,
	4,
	20+117*64,
	65532,
	0,
	4,
	21+117*64,
	65532,
	0,
	4,
	22+117*64,
	65532,
	0,
	4,
	23+117*64,
	65532,
	0,
	0
};

uint16 jobs_right[] = {
	4,
	24+117*64,
	4,
	0,
	4,
	25+117*64,
	4,
	0,
	4,
	26+117*64,
	4,
	0,
	4,
	27+117*64,
	4,
	0,
	4,
	28+117*64,
	4,
	0,
	4,
	29+117*64,
	4,
	0,
	4,
	30+117*64,
	4,
	0,
	4,
	31+117*64,
	4,
	0,
	0
};

uint16 jobs_st_up[] = {
	117*64,
	1,
	1,
	36,
	0
};

uint16 jobs_st_down[] = {
	117*64,
	1,
	1,
	32,
	0
};

uint16 jobs_st_left[] = {
	117*64,
	1,
	1,
	34,
	0
};

uint16 jobs_st_right[] = {
	117*64,
	1,
	1,
	38,
	0
};

uint16 jobs_st_talk[] = {
	134*64,
	1,
	1,
	0,
	0
};

uint16 jobs_u_to_d[] = {
	43-8+117*64,
	42-8+117*64,
	41-8+117*64,
	0
};

uint16 jobs_u_to_r[] = {
	45-8+117*64,
	0
};

uint16 jobs_d_to_u[] = {
	47-8+117*64,
	46-8+117*64,
	45-8+117*64,
	0
};

uint16 jobs_d_to_l[] = {
	41-8+117*64,
	0
};

uint16 jobs_d_to_r[] = {
	47-8+117*64,
	0
};

uint16 jobs_l_to_d[] = {
	41-8+117*64,
	0
};

uint16 jobs_l_to_r[] = {
	41-8+117*64,
	40-8+117*64,
	47-8+117*64,
	0
};

uint16 jobs_r_to_u[] = {
	45-8+117*64,
	0
};

uint16 jobs_r_to_d[] = {
	47-8+117*64,
	0
};

uint16 jobs_r_to_l[] = {
	45-8+117*64,
	44-8+117*64,
	43-8+117*64,
	0
};

TurnTable jobsworth_turnTable0 = {
	{ // turnTableUp
		0,
		m_jobs_u_to_d,
		m_jobs_u_to_l,
		m_jobs_u_to_r,
		0
	},
	{ // turnTableDown
		m_jobs_d_to_u,
		0,
		m_jobs_d_to_l,
		m_jobs_d_to_r,
		0
	},
	{ // turnTableLeft
		m_jobs_l_to_u,
		m_jobs_l_to_d,
		0,
		m_jobs_l_to_r,
		0
	},
	{ // turnTableRight
		m_jobs_r_to_u,
		m_jobs_r_to_d,
		m_jobs_r_to_l,
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

TurnTable jobsworth_turnTable1 = {
	{ // turnTableUp
		0,
		jobs_u_to_d,
		jobs_u_to_l,
		jobs_u_to_r,
		0
	},
	{ // turnTableDown
		jobs_d_to_u,
		0,
		jobs_d_to_l,
		jobs_d_to_r,
		0
	},
	{ // turnTableLeft
		jobs_l_to_u,
		jobs_l_to_d,
		0,
		jobs_l_to_r,
		0
	},
	{ // turnTableRight
		jobs_r_to_u,
		jobs_r_to_d,
		jobs_r_to_l,
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

MegaSet jobsworth_megaSet0 = {
	0,	// gridWidth
	0,	// colOffset
	8,	// colWidth
	8,	// lastChr
	m_jobs_up,	// animUp
	m_jobs_down,	// animDown
	m_jobs_left,	// animLeft
	m_jobs_right,	// animRight
	m_jobs_st_up,	// standUp
	m_jobs_st_down,	// standDown
	m_jobs_st_left,	// standLeft
	m_jobs_st_right,	// standRight
	m_jobs_st_talk,	// standTalk
	&jobsworth_turnTable0,
};

MegaSet jobsworth_megaSet1 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	jobs_up,	// animUp
	jobs_down,	// animDown
	jobs_left,	// animLeft
	jobs_right,	// animRight
	jobs_st_up,	// standUp
	jobs_st_down,	// standDown
	jobs_st_left,	// standLeft
	jobs_st_right,	// standRight
	jobs_st_talk,	// standTalk
	&jobsworth_turnTable1
};

ExtCompact jobsworth_ext = {
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
	SP_COL_JOBS,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	jobs_auto,	// animScratch
	0,	// megaSet
	&jobsworth_megaSet0,
	&jobsworth_megaSet1,
	0,
	0
};

Compact jobsworth = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_MOUSE+ST_COLLISION,	// status
	0,	// sync
	0,	// screen
	ID_LOW_FLOOR,	// place
	0,	// getToTable
	416,	// xcood
	272,	// ycood
	73*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65516,	// mouseRel_y
	8,	// mouseSize_x
	28,	// mouseSize_y
	MEGA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	START_JOBS,	// baseSub
	0,	// baseSub_off
	&jobsworth_ext
};

Compact notice4 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	152,	// xcood
	232,	// ycood
	0,	// frame
	4196,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	8,	// mouseSize_x
	8,	// mouseSize_y
	NOTICE4_ACTION,	// actionScript
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

Compact buttons_s4 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	256,	// xcood
	195,	// ycood
	0,	// frame
	15+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	8,	// mouseSize_x
	15,	// mouseSize_y
	BUTTON_ACTION,	// actionScript
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

uint16 sc4_mouse_open[] = {
	JOBS,
	ID_JOEY,
	ID_POSTCARD,
	ID_NOTICE4,
	ID_LAZER,
	ID_CHUCK,
	ID_MONITORS,
	ID_TV_SCREENS,
	ID_S4_L_EXIT,
	ID_S4_FLOOR,
	ID_SARNIE,
	ID_SPANNER,
	ID_CUPBOARD,
	ID_KNOB,
	ID_BUTTONS,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact tv_screens_s4 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	56*64,	// frame
	4200,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	18,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	7,	// mouseSize_y
	TV_2_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	TV_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc4_logic[] = {
	ID_FOSTER,
	JOBS,
	ID_JOEY,
	ID_TV_SCREENS,
	ID_KNOB,
	ID_CHUCK,
	ID_LAZER,
	ID_SARNIE,
	ID_SPANNER,
	ID_CUPBOARD,
	ID_MONITORS,
	ID_LOADER,
	ID_TOP_LIFT,
	4344,
	ID_TOP_BARREL,
	ID_LIGHT1,
	ID_PANEL,
	0XFFFF,
	ID_MENU_LOGIC
};

Compact postcard = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	167,	// xcood
	224,	// ycood
	0,	// frame
	4197,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	POSTCARD_ACTION,	// actionScript
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

Compact knob_s4 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	302,	// xcood
	187,	// ycood
	58*64,	// frame
	13+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	14,	// mouseSize_y
	KNOB_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	KNOB_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact lazer_s4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	331,	// xcood
	210,	// ycood
	0,	// frame
	4214,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	350-331,	// mouseSize_x
	228-210,	// mouseSize_y
	LAZER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	LAZER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 fast_list_sc4[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	153,
	131,
	132,
	136,
	137,
	138,
	0
};

Compact sc4_left_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	188,	// xcood
	193,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	13,	// mouseSize_x
	61,	// mouseSize_y
	EL4_ACTION,	// actionScript
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

Compact spanner_s4 = {
	0,	// logic
	ST_BACKGROUND+ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	254,	// xcood
	224,	// ycood
	62*64,	// frame
	8,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	SPANNER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

uint16 joey_list_s4[] = {
	224,
	320,
	232,
	296,
	1,
	224,
	304,
	256,
	272,
	0,
	0
};

uint32 *c43 = (uint32*)&jobsworth;

Compact chuck_s4 = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	334,	// xcood
	222,	// ycood
	59*64,	// frame
	4198,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	17,	// mouseSize_x
	10,	// mouseSize_y
	CHUCK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	CHUCK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 r4_floor_table[] = {
	ID_S4_FLOOR,
	RET_OK,
	ID_S4_L_EXIT,
	GET_TO_EL4,
	ID_CUPBOARD,
	GET_TO_CUPBOARD,
	ID_SARNIE,
	GET_TO_SHELVES,
	ID_SPANNER,
	GET_TO_SHELVES,
	ID_KNOB,
	GET_TO_KNOB,
	ID_CHUCK,
	GET_TO_CHUCK,
	ID_MONITORS,
	GET_TO_SCREENS,
	ID_BUTTONS,
	GET_TO_BUTTONS,
	4106,
	GET_TO_TALK41,
	4107,
	GET_TO_TALK42,
	4136,
	S4_WALK_ON,
	ID_POSTCARD,
	GET_TO_POSTCARD,
	ID_NOTICE4,
	GET_TO_POSTCARD,
	ID_TV_SCREENS,
	GET_TO_MONITOR,
	ID_JOEY_PARK,
	GET_TO_JP2,
	ID_LAZER,
	GT_LAZER,
	65535
};

uint16 sc4_mouse[] = {
	JOBS,
	ID_JOEY,
	ID_POSTCARD,
	ID_NOTICE4,
	ID_LAZER,
	ID_CHUCK,
	ID_MONITORS,
	ID_TV_SCREENS,
	ID_S4_L_EXIT,
	ID_S4_FLOOR,
	ID_CUPBOARD,
	ID_KNOB,
	ID_BUTTONS,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc4_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	r4_floor_table,	// getToTable
	184,	// xcood
	232,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	183,	// mouseSize_x
	64,	// mouseSize_y
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

Compact cupboard_s4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE+ST_BACKGROUND+ST_MOUSE,	// status
	0,	// sync
	4,	// screen
	0,	// place
	0,	// getToTable
	248,	// xcood
	214,	// ycood
	57*64,	// frame
	3+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	20,	// mouseSize_x
	20,	// mouseSize_y
	CUPBOARD_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	CUPBOARD_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
