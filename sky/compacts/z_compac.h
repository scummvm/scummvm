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

#ifndef SKYZ_COMPAC_H
#define SKYZ_COMPAC_H


#include "objects.h"
#include "85comp.h"
#include "101comp.h"
#include "102comp.h"


namespace Sky {

namespace SkyCompact {

uint16 fradman_up[] = {
	2,
	0+90*64,
	0,
	65534,
	2,
	1+90*64,
	0,
	65534,
	2,
	2+90*64,
	0,
	65534,
	2,
	3+90*64,
	0,
	65534,
	2,
	4+90*64,
	0,
	65534,
	2,
	5+90*64,
	0,
	65534,
	2,
	6+90*64,
	0,
	65534,
	2,
	7+90*64,
	0,
	65534,
	2,
	8+90*64,
	0,
	65534,
	2,
	9+90*64,
	0,
	65534,
	0
};

uint16 high_floor_table[] = {
	67,
	STAIRS_FROM_HIGH,
	70,
	STAIRS_FROM_HIGH,
	85,
	GET_TO_BAR,
	90,
	GET_TO_FEXIT,
	17,
	RET_OK,
	ID_NOTICE,
	GET_TO_NOTICE,
	0
};

Compact floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	high_floor_table,	// getToTable
	152,	// xcood
	208,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	248,	// mouseSize_x
	23,	// mouseSize_y
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

uint16 wjoey_d_to_u[] = {
	23+136*64,
	12+136*64,
	22+136*64,
	0
};

uint16 wjoey_st_left[] = {
	136*64,
	1,
	1,
	4,
	0
};

uint16 ljoey_r_to_d[] = {
	23+173*64,
	0
};

uint16 s2_talk_table[] = {
	21,
	RET_OK,
	22,
	RET_OK,
	116,
	GET_TO_EL2,
	ID_R_EXIT_S2,
	GET_TO_ER2,
	115,
	STD_ADJOIN_FLOOR,
	ID_LIGHT1,
	GET_TO_LIGHT1,
	ID_HOLE,
	GET_TO_HOLE,
	ID_DEAD_LOADER,
	GET_TO_TRANSPORTER,
	ID_TOP_LIFT,
	GET_TO_LIFTER,
	ID_ROBOT_SHELL,
	GET_TO_SHELL,
	ID_PANEL,
	GET_TO_LEDS,
	65535
};

Compact r_talk_s2 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	s2_talk_table,	// getToTable
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

uint16 fradman_u_to_d[] = {
	43+90*64,
	42+90*64,
	41+90*64,
	0
};

uint16 door_table[] = {
	17,
	STD_ADJOIN_FLOOR,
	67,
	STAIRS_FROM_HIGH,
	70,
	STAIRS_FROM_HIGH,
	85,
	GET_TO_BAR,
	90,
	GET_TO_FEXIT,
	ID_NOTICE,
	GET_TO_NOTICE,
	65535
};

uint16 mfoster_st_right[] = {
	13*64,
	1,
	1,
	46,
	0
};

uint16 bfoster_st_right[] = {
	182*64,
	1,
	0,
	46,
	0
};

uint16 seq3_pal[] = {
	0,
	1075,
	2932,
	3701,
	801,
	1586,
	2114,
	529,
	2916,
	785,
	1620,
	3154,
	818,
	529,
	3683,
	529,
	802,
	1587,
	802,
	1330,
	2643,
	2934,
	546,
	546,
	1092,
	3410,
	273,
	273,
	3410,
	2882,
	1057,
	3718
};

uint16 bfoster_u_to_r[] = {
	45+182*64,
	0
};

uint16 foster_l_to_t[] = {
	41+12*64,
	40+12*64,
	0
};

uint16 s9_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	56+DISK_2,
	150+DISK_2,
	102+DISK_2,
	103+DISK_2,
	104+DISK_2,
	105+DISK_2,
	100+DISK_2,
	108+DISK_2,
	109+DISK_2,
	0
};

uint16 wjoey_st_up[] = {
	136*64,
	1,
	1,
	0,
	0
};

uint16 bfoster_r_to_u[] = {
	45+182*64,
	0
};

uint16 monitor_left[] = {
	4,
	16+74*64,
	65532,
	0,
	4,
	17+74*64,
	65532,
	0,
	4,
	18+74*64,
	65532,
	0,
	4,
	19+74*64,
	65532,
	0,
	4,
	20+74*64,
	65532,
	0,
	4,
	21+74*64,
	65532,
	0,
	4,
	22+74*64,
	65532,
	0,
	4,
	23+74*64,
	65532,
	0,
	0
};

uint16 rs_lamb_to_three[] = {
	C_STATUS,
	ST_LOGIC,
	C_LOGIC,
	L_SCRIPT,
	65535
};

uint16 std_menu_logic[] = {
	18,
	47,
	48,
	63,
	64,
	65,
	66,
	IDO_CITYCARD,
	IDO_SHADES,
	IDO_PUTTY,
	IDO_LIGHTBULB,
	IDO_WD40,
	IDO_SKEY,
	IDO_ANITA_CARD,
	IDO_ANCHOR,
	IDO_MAGAZINE,
	IDO_TAPE,
	IDO_GLASS,
	IDO_ROPE,
	IDO_NEW_CABLE,
	IDO_TICKET,
	IDO_SECATEURS,
	IDO_PLASTER,
	IDO_BRICK,
	IDO_TONGS,
	IDO_DOG_FOOD,
	IDO_GALLCARD,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	33,
	32,
	31,
	30,
	29,
	28,
	27,
	26,
	25,
	24,
	23,
	0
};

uint16 but_7[] = {
	147*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 ljoey_up[] = {
	1,
	0+173*64,
	0,
	65535,
	1,
	1+173*64,
	0,
	65535,
	0
};

uint16 wjoey_r_to_d[] = {
	23+136*64,
	0
};

Compact small_door = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_BACKGROUND+ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	163,	// xcood
	248,	// ycood
	104*64,	// frame
	4148,	// cursorText
	SMALL_DOOR_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	9,	// mouseSize_x
	36,	// mouseSize_y
	SMALL_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SMALL_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 shrug_seq[] = {
	12*64,
	1,
	1,
	48,
	1,
	1,
	48,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	0
};

uint16 foster_up[] = {
	2,
	0+12*64,
	0,
	65534,
	2,
	1+12*64,
	0,
	65534,
	2,
	2+12*64,
	0,
	65534,
	2,
	3+12*64,
	0,
	65534,
	2,
	4+12*64,
	0,
	65534,
	2,
	5+12*64,
	0,
	65534,
	2,
	6+12*64,
	0,
	65534,
	2,
	7+12*64,
	0,
	65534,
	2,
	8+12*64,
	0,
	65534,
	2,
	9+12*64,
	0,
	65534,
	0
};

uint16 mfoster_d_to_l[] = {
	41+13*64,
	0
};

uint16 ljoey_down[] = {
	1,
	2+173*64,
	0,
	1,
	1,
	3+173*64,
	0,
	1,
	0
};

uint16 rs_blanks_linc[] = {
	C_FRAME,
	191*64,
	C_BASE_SUB,
	LINC_MENU_SCRIPT,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 mini_shrug_seq[] = {
	13*64,
	1,
	1,
	48,
	1,
	1,
	48,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	0
};

uint16 talk_table[] = {
	67,
	STD_ADJOIN_FLOOR,
	69,
	STAIRS_FROM_LOW,
	44,
	RET_OK,
	45,
	RET_OK,
	105,
	GET_TO_SMALL_DOOR,
	111,
	GET_TO_ER0,
	17,
	STAIRS_FROM_LOW
};

uint16 fradman_l_to_d[] = {
	41+90*64,
	0
};

uint16 low_floor_table[] = {
	105,
	GET_TO_SMALL_DOOR,
	111,
	GET_TO_ER0,
	69,
	STAIRS_FROM_LOW,
	17,
	STAIRS_FROM_LOW,
	ID_LOW_FLOOR,
	RET_OK,
	44,
	GET_TO_TALK1,
	45,
	GET_TO_TALK2,
	ID_UPLOAD,
	GET_TO_UPLOAD,
	ID_PRESS,
	GET_TO_PRESS,
	0
};

Compact low_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	low_floor_table,	// getToTable
	163,	// xcood
	272,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	237,	// mouseSize_x
	40,	// mouseSize_y
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

uint16 bfoster_d_to_u[] = {
	47+182*64,
	46+182*64,
	45+182*64,
	0
};

uint16 but_0[] = {
	140*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 but_4[] = {
	144*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint8 new_grid[120];

uint16 fradman_right[] = {
	4,
	30+90*64,
	4,
	0,
	4,
	31+90*64,
	4,
	0,
	4,
	32+90*64,
	4,
	0,
	4,
	33+90*64,
	4,
	0,
	4,
	34+90*64,
	4,
	0,
	4,
	35+90*64,
	4,
	0,
	4,
	36+90*64,
	4,
	0,
	4,
	37+90*64,
	4,
	0,
	4,
	38+90*64,
	4,
	0,
	4,
	39+90*64,
	4,
	0,
	0
};

uint16 bfoster_st_left[] = {
	182*64,
	1,
	0,
	42,
	0
};

uint16 sjoey_up[] = {
	1,
	0+86*64,
	0,
	65535,
	1,
	1+86*64,
	0,
	65535,
	0
};

uint16 s10_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	43+DISK_1,
	44+DISK_1,
	0
};

uint16 fradman_u_to_l[] = {
	43+90*64,
	0
};

uint16 ljoey_r_to_u[] = {
	22+173*64,
	0
};

uint16 foster_u_to_l[] = {
	43+12*64,
	0
};

uint16 lamb_auto[32];

uint16 lamb_up[] = {
	2,
	0+131*64,
	0,
	65534,
	2,
	1+131*64,
	0,
	65534,
	2,
	2+131*64,
	0,
	65534,
	2,
	3+131*64,
	0,
	65534,
	2,
	4+131*64,
	0,
	65534,
	2,
	5+131*64,
	0,
	65534,
	2,
	6+131*64,
	0,
	65534,
	2,
	7+131*64,
	0,
	65534,
	2,
	8+131*64,
	0,
	65534,
	2,
	9+131*64,
	0,
	65534,
	0
};

uint16 lamb_down[] = {
	2,
	10+131*64,
	0,
	2,
	2,
	11+131*64,
	0,
	2,
	2,
	12+131*64,
	0,
	2,
	2,
	13+131*64,
	0,
	2,
	2,
	14+131*64,
	0,
	2,
	2,
	15+131*64,
	0,
	2,
	2,
	16+131*64,
	0,
	2,
	2,
	17+131*64,
	0,
	2,
	2,
	18+131*64,
	0,
	2,
	2,
	19+131*64,
	0,
	2,
	0
};

uint16 lamb_left[] = {
	4,
	20+131*64,
	65532,
	0,
	4,
	21+131*64,
	65532,
	0,
	4,
	22+131*64,
	65532,
	0,
	4,
	23+131*64,
	65532,
	0,
	4,
	24+131*64,
	65532,
	0,
	4,
	25+131*64,
	65532,
	0,
	4,
	26+131*64,
	65532,
	0,
	4,
	27+131*64,
	65532,
	0,
	4,
	28+131*64,
	65532,
	0,
	4,
	29+131*64,
	65532,
	0,
	0
};

uint16 lamb_right[] = {
	4,
	30+131*64,
	4,
	0,
	4,
	31+131*64,
	4,
	0,
	4,
	32+131*64,
	4,
	0,
	4,
	33+131*64,
	4,
	0,
	4,
	34+131*64,
	4,
	0,
	4,
	35+131*64,
	4,
	0,
	4,
	36+131*64,
	4,
	0,
	4,
	37+131*64,
	4,
	0,
	4,
	38+131*64,
	4,
	0,
	4,
	39+131*64,
	4,
	0,
	0
};

uint16 lamb_st_up[] = {
	131*64,
	1,
	1,
	44,
	0
};

uint16 lamb_st_down[] = {
	131*64,
	1,
	1,
	40,
	0
};

uint16 lamb_st_left[] = {
	131*64,
	1,
	1,
	42,
	0
};

uint16 lamb_st_right[] = {
	131*64,
	1,
	1,
	46,
	0
};

uint16 lamb_st_talk[] = {
	132*64,
	1,
	1,
	0,
	0
};

uint16 lamb_u_to_d[] = {
	43+131*64,
	42+131*64,
	41+131*64,
	0
};

uint16 lamb_u_to_l[] = {
	43+131*64,
	0
};

uint16 lamb_u_to_r[] = {
	45+131*64,
	0
};

uint16 lamb_d_to_u[] = {
	47+131*64,
	46+131*64,
	45+131*64,
	0
};

uint16 lamb_d_to_l[] = {
	41+131*64,
	0
};

uint16 lamb_d_to_r[] = {
	47+131*64,
	0
};

uint16 lamb_l_to_u[] = {
	43+131*64,
	0
};

uint16 lamb_l_to_d[] = {
	41+131*64,
	0
};

uint16 lamb_l_to_r[] = {
	41+131*64,
	40+131*64,
	47+131*64,
	0
};

uint16 lamb_r_to_u[] = {
	45+131*64,
	0
};

uint16 lamb_r_to_d[] = {
	47+131*64,
	0
};

uint16 lamb_r_to_l[] = {
	45+131*64,
	44+131*64,
	43+131*64,
	0
};

TurnTable lamb_turnTable0 = {
	{ // turnTableUp
		0,
		lamb_u_to_d,
		lamb_u_to_l,
		lamb_u_to_r,
		0
	},
	{ // turnTableDown
		lamb_d_to_u,
		0,
		lamb_d_to_l,
		lamb_d_to_r,
		0
	},
	{ // turnTableLeft
		lamb_l_to_u,
		lamb_l_to_d,
		0,
		lamb_l_to_r,
		0
	},
	{ // turnTableRight
		lamb_r_to_u,
		lamb_r_to_d,
		lamb_r_to_l,
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

MegaSet lamb_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	lamb_up,	// animUp
	lamb_down,	// animDown
	lamb_left,	// animLeft
	lamb_right,	// animRight
	lamb_st_up,	// standUp
	lamb_st_down,	// standDown
	lamb_st_left,	// standLeft
	lamb_st_right,	// standRight
	lamb_st_talk,	// standTalk
	&lamb_turnTable0
};

ExtCompact lamb_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	0,	// dir
	STD_PLAYER_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_COL_LAMB,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	lamb_auto,	// animScratch
	0,	// megaSet
	&lamb_megaSet0,
	0,
	0,
	0
};

Compact lamb = {
	L_WAIT_SYNC,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,	// status
	0,	// sync
	12,	// screen
	ID_S12_FLOOR,	// place
	0,	// getToTable
	464,	// xcood
	280,	// ycood
	131*64,	// frame
	4,	// cursorText
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
	C_BASE_MODE,	// mode
	LAMB_FACTORY_START,	// baseSub
	0,	// baseSub_off
	&lamb_ext
};

uint16 bfoster_left[] = {
	4,
	20+182*64,
	65532,
	0,
	4,
	21+182*64,
	65532,
	0,
	4,
	22+182*64,
	65532,
	0,
	4,
	23+182*64,
	65532,
	0,
	4,
	24+182*64,
	65532,
	0,
	4,
	25+182*64,
	65532,
	0,
	4,
	26+182*64,
	65532,
	0,
	4,
	27+182*64,
	65532,
	0,
	4,
	28+182*64,
	65532,
	0,
	4,
	29+182*64,
	65532,
	0,
	0
};

uint16 fast_intro[] = {
	183+DISK_15,
	184+DISK_15,
	185+DISK_15,
	186+DISK_15,
	0
};

uint16 foster_st_right[] = {
	12*64,
	1,
	0,
	46,
	0
};

uint16 monitor_st_up[] = {
	74*64,
	1,
	1,
	36,
	0
};

uint16 sjoey_d_to_r[] = {
	12+86*64,
	0
};

uint16 foster_st_down[] = {
	12*64,
	1,
	0,
	40,
	0
};

uint16 mfoster_u_to_r[] = {
	45+13*64,
	0
};

uint16 sml_up_mid_get_seq[] = {
	22*64,
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
	3,
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

uint16 mfoster_u_to_d[] = {
	43+13*64,
	42+13*64,
	41+13*64,
	0
};

uint16 wjoey_u_to_d[] = {
	21+136*64,
	4+136*64,
	20+136*64,
	0
};

uint16 chip_intro[] = {
	187+DISK_15,
	188+DISK_15,
	195+DISK_15,
	196+DISK_15,
	197+DISK_15,
	0
};

uint16 fast_list_0[] = {
	64,
	65,
	66,
	12+0X000,
	51,
	52,
	53,
	13+0X8000,
	0
};

uint16 rs_right_arrow[] = {
	C_FRAME,
	49*64+1,
	C_BASE_SUB,
	MENU_SCRIPT,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 ljoey_st_left[] = {
	173*64,
	1,
	1,
	4,
	0
};

uint16 s28_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	0
};

uint16 monitor_right[] = {
	4,
	24+74*64,
	4,
	0,
	4,
	25+74*64,
	4,
	0,
	4,
	26+74*64,
	4,
	0,
	4,
	27+74*64,
	4,
	0,
	4,
	28+74*64,
	4,
	0,
	4,
	29+74*64,
	4,
	0,
	4,
	30+74*64,
	4,
	0,
	4,
	31+74*64,
	4,
	0,
	0
};

uint16 wjoey_st_down[] = {
	136*64,
	1,
	1,
	2,
	0
};

uint16 r2_floor_table[] = {
	115,
	RET_OK,
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
	ID_PANEL,
	GET_TO_LEDS,
	ID_HOLE,
	GET_TO_HOLE,
	ID_DEAD_LOADER,
	GET_TO_TRANSPORTER,
	ID_ROBOT_SHELL,
	GET_TO_SHELL,
	ID_JOEY_PARK,
	GET_TO_JP2,
	ID_PANEL,
	GT_PANEL2,
	ID_JUNK1,
	GT_JUNK1,
	ID_JUNK2,
	GT_JUNK2,
	0,
	S2_WALK_ON,
	1,
	ER0_WALK_ON,
	65535
};

Compact s2_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	2,	// screen
	0,	// place
	r2_floor_table,	// getToTable
	152,	// xcood
	224,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	240,	// mouseSize_x
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

uint16 fradman_down[] = {
	2,
	10+90*64,
	0,
	2,
	2,
	11+90*64,
	0,
	2,
	2,
	12+90*64,
	0,
	2,
	2,
	13+90*64,
	0,
	2,
	2,
	14+90*64,
	0,
	2,
	2,
	15+90*64,
	0,
	2,
	2,
	16+90*64,
	0,
	2,
	2,
	17+90*64,
	0,
	2,
	2,
	18+90*64,
	0,
	2,
	2,
	19+90*64,
	0,
	2,
	0
};

uint16 foster_u_to_d[] = {
	43+12*64,
	42+12*64,
	41+12*64,
	0
};

uint32 *c68 = (uint32*)&foster;

uint16 ljoey_d_to_r[] = {
	23+173*64,
	0
};

uint16 mfoster_r_to_u[] = {
	45+13*64,
	0
};

uint16 fast_list_sc3[] = {
	12,
	51,
	52,
	53,
	54,
	211+DISK_1,
	67+DISK_1,
	68+DISK_1,
	69+DISK_1,
	222+DISK_1,
	223+DISK_1,
	213+DISK_1,
	0
};

uint16 rs_left_arrow[] = {
	C_FRAME,
	49*64,
	C_BASE_SUB,
	MENU_SCRIPT,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 rs_l_arr_linc[] = {
	C_FRAME,
	190*64,
	C_BASE_SUB,
	LINC_MENU_SCRIPT,
	C_BASE_SUB+2,
	0,
	65535
};

Compact text_8 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	281*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

uint16 wjoey_l_to_d[] = {
	20+136*64,
	0
};

uint16 stair_table[] = {
	67,
	STD_ADJOIN_FLOOR,
	17,
	CLIMB_STAIRS,
	44,
	GET_TO_TALK1,
	45,
	GET_TO_TALK2,
	69,
	RET_OK,
	105,
	GET_TO_SMALL_DOOR,
	111,
	GET_TO_ER0,
	ID_UPLOAD,
	GET_TO_UPLOAD,
	ID_PRESS,
	GET_TO_PRESS,
	0
};

Compact stairs = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	stair_table,	// getToTable
	194,	// xcood
	306,	// ycood
	0,	// frame
	4147,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	12,	// mouseSize_x
	14,	// mouseSize_y
	STAIR_ACTION,	// actionScript
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

uint16 text_mouse[] = {
	18,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	33,
	47,
	48,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	63,
	64,
	65,
	66,
	IDO_CITYCARD,
	IDO_SHADES,
	IDO_PUTTY,
	IDO_LIGHTBULB,
	IDO_WD40,
	IDO_SKEY,
	IDO_ANITA_CARD,
	IDO_ANCHOR,
	IDO_MAGAZINE,
	IDO_TAPE,
	IDO_GLASS,
	IDO_ROPE,
	IDO_NEW_CABLE,
	IDO_TICKET,
	IDO_SECATEURS,
	IDO_PLASTER,
	IDO_BRICK,
	IDO_TONGS,
	IDO_DOG_FOOD,
	IDO_GALLCARD,
	46,
	0
};

uint16 monitor_d_to_r[] = {
	47-8+74*64,
	0
};

uint16 ljoey_l_to_d[] = {
	20+173*64,
	0
};

uint16 sjoey_st_right[] = {
	86*64,
	1,
	1,
	8,
	0
};

uint16 sjoey_st_down[] = {
	86*64,
	1,
	1,
	2,
	0
};

uint16 sjoey_st_left[] = {
	86*64,
	1,
	1,
	4,
	0
};

uint16 upstair_table[] = {
	67,
	DECEND,
	17,
	STD_ADJOIN_FLOOR,
	70,
	RET_OK,
	85,
	GET_TO_BAR,
	90,
	GET_TO_FEXIT,
	ID_NOTICE,
	GET_TO_NOTICE,
	0
};

uint16 sjoey_right[] = {
	1,
	8+86*64,
	1,
	0,
	1,
	9+86*64,
	1,
	0,
	1,
	10+86*64,
	1,
	0,
	1,
	11+86*64,
	1,
	0,
	0
};

uint16 ljoey_u_to_r[] = {
	22+173*64,
	0
};

uint16 mfoster_st_up[] = {
	13*64,
	1,
	1,
	44,
	0
};

uint16 mfoster_st_down[] = {
	13*64,
	1,
	1,
	40,
	0
};

uint16 foster_st_talk[] = {
	135*64,
	1,
	0,
	0,
	0
};

uint16 mfoster_l_to_r[] = {
	41+13*64,
	40+13*64,
	47+13*64,
	0
};

uint16 bar_table[] = {
	17,
	STD_ADJOIN_FLOOR,
	67,
	STAIRS_FROM_HIGH,
	70,
	STAIRS_FROM_HIGH,
	90,
	GET_TO_FEXIT,
	85,
	RET_OK,
	ID_NOTICE,
	GET_TO_NOTICE
};

uint16 foster_d_to_l[] = {
	41+12*64,
	0
};

Compact text_1 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	136,	// xcood
	327-16,	// ycood
	274*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

uint16 fradman_left[] = {
	4,
	20+90*64,
	65532,
	0,
	4,
	21+90*64,
	65532,
	0,
	4,
	22+90*64,
	65532,
	0,
	4,
	23+90*64,
	65532,
	0,
	4,
	24+90*64,
	65532,
	0,
	4,
	25+90*64,
	65532,
	0,
	4,
	26+90*64,
	65532,
	0,
	4,
	27+90*64,
	65532,
	0,
	4,
	28+90*64,
	65532,
	0,
	4,
	29+90*64,
	65532,
	0,
	0
};

uint16 s29_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	170+DISK_8,
	171+DISK_8,
	0
};

uint16 r1door_table[] = {
	95,
	STD_ADJOIN_FLOOR,
	97,
	GET_TO_R1_DOOR,
	ID_NOTICE2,
	GET_TO_NOTICE2,
	ID_SS_SIGN,
	GET_TO_SS_SIGN,
	65535
};

Compact r1_door = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_BACKGROUND+ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	1,	// screen
	0,	// place
	r1door_table,	// getToTable
	186,	// xcood
	177,	// ycood
	96*64,	// frame
	4148,	// cursorText
	OUT_EXIT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	64,	// mouseSize_y
	S1_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	R1EXIT_DOOR,	// baseSub
	0,	// baseSub_off
	0
};

Compact upstairs = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	upstair_table,	// getToTable
	164,	// xcood
	207,	// ycood
	0,	// frame
	4147,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	200-164,	// mouseSize_x
	211-207,	// mouseSize_y
	UPSTAIR_ACTION,	// actionScript
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

uint16 ljoey_u_to_d[] = {
	21+173*64,
	4+173*64,
	20+173*64,
	0
};

uint16 monitor_u_to_l[] = {
	43-8+74*64,
	0
};

uint16 fradman_st_right[] = {
	90*64,
	1,
	0,
	46,
	0
};

uint16 sml_door_table[] = {
	17,
	STAIRS_FROM_LOW,
	105,
	RET_OK,
	111,
	GET_TO_ER0,
	69,
	STAIRS_FROM_LOW,
	67,
	STD_ADJOIN_FLOOR,
	44,
	GET_TO_TALK1,
	45,
	GET_TO_TALK2
};

uint32 *l_talk_s2 = (uint32*)&r_talk_s2;

uint16 bfoster_r_to_l[] = {
	45+182*64,
	44+182*64,
	43+182*64,
	0
};

uint16 mfoster_left[] = {
	2,
	20+13*64,
	65534,
	0,
	2,
	21+13*64,
	65534,
	0,
	2,
	22+13*64,
	65534,
	0,
	2,
	23+13*64,
	65534,
	0,
	2,
	24+13*64,
	65534,
	0,
	2,
	25+13*64,
	65534,
	0,
	2,
	26+13*64,
	65534,
	0,
	2,
	27+13*64,
	65534,
	0,
	2,
	28+13*64,
	65534,
	0,
	2,
	29+13*64,
	65534,
	0,
	0
};

uint16 wjoey_up[] = {
	1,
	0+136*64,
	0,
	65535,
	1,
	1+136*64,
	0,
	65535,
	0
};

uint16 foster_r_to_u[] = {
	45+12*64,
	0
};

uint16 mfoster_d_to_u[] = {
	47+13*64,
	46+13*64,
	45+13*64,
	0
};

uint16 but_8[] = {
	148*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 but_5[] = {
	145*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 monitor_l_to_u[] = {
	43-8+74*64,
	0
};

uint16 mfoster_l_to_u[] = {
	43+13*64,
	0
};

uint16 bfoster_d_to_r[] = {
	47+182*64,
	0
};

uint16 wjoey_down[] = {
	1,
	2+136*64,
	0,
	1,
	1,
	3+136*64,
	0,
	1,
	0
};

Compact text_10 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	283*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

uint16 monitor_u_to_d[] = {
	43-8+74*64,
	42-8+74*64,
	41-8+74*64,
	0
};

Compact text_5 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	278*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

uint16 forklift1_cdt[] = {
	93*64,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	1,
	289,
	291,
	2,
	289,
	291,
	3,
	289,
	291,
	4,
	289,
	291,
	5,
	289,
	291,
	6,
	289,
	291,
	7,
	289,
	291,
	8,
	289,
	291,
	9,
	289,
	291,
	10,
	289,
	291,
	11,
	289,
	291,
	12,
	289,
	291,
	13,
	289,
	291,
	14,
	289,
	291,
	15,
	289,
	291,
	16,
	289,
	291,
	17,
	289,
	291,
	18,
	289,
	291,
	19,
	289,
	291,
	20,
	289,
	291,
	21,
	289,
	291,
	22,
	289,
	291,
	23,
	289,
	291,
	24,
	289,
	291,
	25,
	289,
	291,
	26,
	289,
	291,
	27,
	289,
	291,
	28,
	289,
	291,
	29,
	289,
	291,
	30,
	289,
	291,
	31,
	289,
	291,
	32,
	289,
	291,
	33,
	289,
	291,
	34,
	289,
	291,
	35,
	289,
	291,
	36,
	289,
	291,
	37,
	289,
	290,
	38,
	289,
	289,
	39,
	289,
	288,
	37,
	289,
	287,
	38,
	289,
	286,
	39,
	289,
	285,
	37,
	289,
	284,
	38,
	289,
	283,
	39,
	289,
	282,
	37,
	289,
	281,
	38,
	289,
	280,
	39,
	289,
	279,
	37,
	291,
	291,
	40,
	291,
	291,
	41,
	291,
	291,
	42,
	291,
	291,
	43,
	291,
	291,
	44,
	291,
	291,
	45,
	291,
	291,
	46,
	291,
	291,
	47,
	291,
	291,
	48,
	291,
	291,
	49,
	291,
	291,
	50,
	291,
	291,
	51,
	291,
	291,
	52,
	291,
	291,
	53,
	291,
	291,
	54,
	291,
	291,
	55,
	291,
	291,
	56,
	291,
	291,
	57,
	292,
	291,
	58,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	292,
	291,
	0,
	0
};

uint16 bfoster_up[] = {
	2,
	0+182*64,
	0,
	65534,
	2,
	1+182*64,
	0,
	65534,
	2,
	2+182*64,
	0,
	65534,
	2,
	3+182*64,
	0,
	65534,
	2,
	4+182*64,
	0,
	65534,
	2,
	5+182*64,
	0,
	65534,
	2,
	6+182*64,
	0,
	65534,
	2,
	7+182*64,
	0,
	65534,
	2,
	8+182*64,
	0,
	65534,
	2,
	9+182*64,
	0,
	65534,
	0
};

uint16 s11_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	16+DISK_1,
	0
};

uint16 fradman_d_to_u[] = {
	47+90*64,
	46+90*64,
	45+90*64,
	0
};

uint16 monitor_r_to_u[] = {
	45-8+74*64,
	0
};

uint16 bfoster_down[] = {
	2,
	10+182*64,
	0,
	2,
	2,
	11+182*64,
	0,
	2,
	2,
	12+182*64,
	0,
	2,
	2,
	13+182*64,
	0,
	2,
	2,
	14+182*64,
	0,
	2,
	2,
	15+182*64,
	0,
	2,
	2,
	16+182*64,
	0,
	2,
	2,
	17+182*64,
	0,
	2,
	2,
	18+182*64,
	0,
	2,
	2,
	19+182*64,
	0,
	2,
	0
};

uint16 ledge_table[] = {
	95,
	RET_OK,
	97,
	GET_TO_R1_DOOR,
	ID_NOTICE2,
	GET_TO_NOTICE2,
	ID_SS_SIGN,
	GET_TO_SS_SIGN,
	65535
};

uint16 ljoey_d_to_u[] = {
	23+173*64,
	12+173*64,
	22+173*64,
	0
};

uint16 foster_left[] = {
	4,
	20+12*64,
	65532,
	0,
	4,
	21+12*64,
	65532,
	0,
	4,
	22+12*64,
	65532,
	0,
	4,
	23+12*64,
	65532,
	0,
	4,
	24+12*64,
	65532,
	0,
	4,
	25+12*64,
	65532,
	0,
	4,
	26+12*64,
	65532,
	0,
	4,
	27+12*64,
	65532,
	0,
	4,
	28+12*64,
	65532,
	0,
	4,
	29+12*64,
	65532,
	0,
	0
};

uint16 sjoey_st_up[] = {
	86*64,
	1,
	1,
	0,
	0
};

uint16 fradman_l_to_r[] = {
	41+90*64,
	40+90*64,
	47+90*64,
	0
};

Compact outside_ledge = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	1,	// screen
	0,	// place
	ledge_table,	// getToTable
	192,	// xcood
	224+8,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	64,	// mouseSize_x
	33-8,	// mouseSize_y
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

uint16 fradman_st_up[] = {
	90*64,
	1,
	0,
	44,
	0
};

uint16 foster_l_to_u[] = {
	43+12*64,
	0
};

uint16 foster_u_to_t[] = {
	45+12*64,
	46+12*64,
	0
};

uint16 ljoey_d_to_l[] = {
	20+173*64,
	0
};

uint16 joey_auto[32];

uint16 ljoey_left[] = {
	2,
	4+173*64,
	65534,
	0,
	2,
	5+173*64,
	65534,
	0,
	2,
	6+173*64,
	65534,
	0,
	2,
	7+173*64,
	65534,
	0,
	2,
	8+173*64,
	65534,
	0,
	2,
	9+173*64,
	65534,
	0,
	2,
	10+173*64,
	65534,
	0,
	2,
	11+173*64,
	65534,
	0,
	0
};

uint16 ljoey_right[] = {
	2,
	12+173*64,
	2,
	0,
	2,
	13+173*64,
	2,
	0,
	2,
	14+173*64,
	2,
	0,
	2,
	15+173*64,
	2,
	0,
	2,
	16+173*64,
	2,
	0,
	2,
	17+173*64,
	2,
	0,
	2,
	18+173*64,
	2,
	0,
	2,
	19+173*64,
	2,
	0,
	0
};

uint16 ljoey_st_up[] = {
	173*64,
	1,
	1,
	0,
	0
};

uint16 ljoey_st_down[] = {
	173*64,
	1,
	1,
	2,
	0
};

uint16 ljoey_st_right[] = {
	173*64,
	1,
	1,
	12,
	0
};

uint16 ljoey_u_to_l[] = {
	21+173*64,
	0
};

uint16 ljoey_l_to_u[] = {
	21+173*64,
	0
};

uint16 ljoey_l_to_r[] = {
	20+173*64,
	2+173*64,
	23+173*64,
	0
};

uint16 ljoey_r_to_l[] = {
	22+173*64,
	0+173*64,
	21+173*64,
	0
};

uint16 wjoey_left[] = {
	2,
	4+136*64,
	65534,
	0,
	2,
	5+136*64,
	65534,
	0,
	2,
	6+136*64,
	65534,
	0,
	2,
	7+136*64,
	65534,
	0,
	2,
	8+136*64,
	65534,
	0,
	2,
	9+136*64,
	65534,
	0,
	2,
	10+136*64,
	65534,
	0,
	2,
	11+136*64,
	65534,
	0,
	0
};

uint16 wjoey_right[] = {
	2,
	12+136*64,
	2,
	0,
	2,
	13+136*64,
	2,
	0,
	2,
	14+136*64,
	2,
	0,
	2,
	15+136*64,
	2,
	0,
	2,
	16+136*64,
	2,
	0,
	2,
	17+136*64,
	2,
	0,
	2,
	18+136*64,
	2,
	0,
	2,
	19+136*64,
	2,
	0,
	0
};

uint16 wjoey_st_right[] = {
	136*64,
	1,
	1,
	12,
	0
};

uint16 wjoey_u_to_l[] = {
	21+136*64,
	0
};

uint16 wjoey_u_to_r[] = {
	22+136*64,
	0
};

uint16 wjoey_d_to_l[] = {
	20+136*64,
	0
};

uint16 wjoey_d_to_r[] = {
	23+136*64,
	0
};

uint16 wjoey_l_to_u[] = {
	21+136*64,
	0
};

uint16 wjoey_l_to_r[] = {
	20+136*64,
	2+136*64,
	23+136*64,
	0
};

uint16 wjoey_r_to_u[] = {
	22+136*64,
	0
};

uint16 wjoey_r_to_l[] = {
	22+136*64,
	0+136*64,
	21+136*64,
	0
};

uint16 sjoey_down[] = {
	1,
	2+86*64,
	0,
	1,
	1,
	3+86*64,
	0,
	1,
	0
};

uint16 sjoey_left[] = {
	1,
	4+86*64,
	65535,
	0,
	1,
	5+86*64,
	65535,
	0,
	1,
	6+86*64,
	65535,
	0,
	1,
	7+86*64,
	65535,
	0,
	0
};

uint16 sjoey_u_to_d[] = {
	14+86*64,
	4+86*64,
	13+86*64,
	0
};

uint16 sjoey_u_to_l[] = {
	14+86*64,
	0
};

uint16 sjoey_u_to_r[] = {
	15+86*64,
	0
};

uint16 sjoey_d_to_u[] = {
	12+86*64,
	8+86*64,
	15+86*64,
	0
};

uint16 sjoey_d_to_l[] = {
	13+86*64,
	0
};

uint16 sjoey_l_to_u[] = {
	14+86*64,
	0
};

uint16 sjoey_l_to_d[] = {
	13+86*64,
	0
};

uint16 sjoey_l_to_r[] = {
	13+86*64,
	2+86*64,
	12+86*64,
	0
};

uint16 sjoey_r_to_u[] = {
	15+86*64,
	0
};

uint16 sjoey_r_to_d[] = {
	12+86*64,
	0
};

uint16 sjoey_r_to_l[] = {
	15+86*64,
	0+86*64,
	14+86*64,
	0
};

TurnTable joey_turnTable0 = {
	{ // turnTableUp
		0,
		ljoey_u_to_d,
		ljoey_u_to_l,
		ljoey_u_to_r,
		0
	},
	{ // turnTableDown
		ljoey_d_to_u,
		0,
		ljoey_d_to_l,
		ljoey_d_to_r,
		0
	},
	{ // turnTableLeft
		ljoey_l_to_u,
		ljoey_l_to_d,
		0,
		ljoey_l_to_r,
		0
	},
	{ // turnTableRight
		ljoey_r_to_u,
		ljoey_r_to_d,
		ljoey_r_to_l,
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

TurnTable joey_turnTable1 = {
	{ // turnTableUp
		0,
		wjoey_u_to_d,
		wjoey_u_to_l,
		wjoey_u_to_r,
		0
	},
	{ // turnTableDown
		wjoey_d_to_u,
		0,
		wjoey_d_to_l,
		wjoey_d_to_r,
		0
	},
	{ // turnTableLeft
		wjoey_l_to_u,
		wjoey_l_to_d,
		0,
		wjoey_l_to_r,
		0
	},
	{ // turnTableRight
		wjoey_r_to_u,
		wjoey_r_to_d,
		wjoey_r_to_l,
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

TurnTable joey_turnTable2 = {
	{ // turnTableUp
		0,
		sjoey_u_to_d,
		sjoey_u_to_l,
		sjoey_u_to_r,
		0
	},
	{ // turnTableDown
		sjoey_d_to_u,
		0,
		sjoey_d_to_l,
		sjoey_d_to_r,
		0
	},
	{ // turnTableLeft
		sjoey_l_to_u,
		sjoey_l_to_d,
		0,
		sjoey_l_to_r,
		0
	},
	{ // turnTableRight
		sjoey_r_to_u,
		sjoey_r_to_d,
		sjoey_r_to_l,
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

MegaSet joey_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	ljoey_up,	// animUp
	ljoey_down,	// animDown
	ljoey_left,	// animLeft
	ljoey_right,	// animRight
	ljoey_st_up,	// standUp
	ljoey_st_down,	// standDown
	ljoey_st_left,	// standLeft
	ljoey_st_right,	// standRight
	0,	// standTalk
	&joey_turnTable0,
};

MegaSet joey_megaSet1 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	wjoey_up,	// animUp
	wjoey_down,	// animDown
	wjoey_left,	// animLeft
	wjoey_right,	// animRight
	wjoey_st_up,	// standUp
	wjoey_st_down,	// standDown
	wjoey_st_left,	// standLeft
	wjoey_st_right,	// standRight
	0,	// standTalk
	&joey_turnTable1,
};

MegaSet joey_megaSet2 = {
	0,	// gridWidth
	0,	// colOffset
	8,	// colWidth
	8,	// lastChr
	sjoey_up,	// animUp
	sjoey_down,	// animDown
	sjoey_left,	// animLeft
	sjoey_right,	// animRight
	sjoey_st_up,	// standUp
	sjoey_st_down,	// standDown
	sjoey_st_left,	// standLeft
	sjoey_st_right,	// standRight
	0,	// standTalk
	&joey_turnTable2
};

ExtCompact joey_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	3,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96,	// spWidth_xx
	SP_COL_JOEY,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	joey_auto,	// animScratch
	0,	// megaSet
	&joey_megaSet0,
	&joey_megaSet1,
	&joey_megaSet2,
	0
};

Compact joey = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	2,	// screen
	115,	// place
	0,	// getToTable
	320,	// xcood
	304,	// ycood
	173*64+12,	// frame
	3,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65526,	// mouseRel_x
	(int16) 65520,	// mouseRel_y
	20,	// mouseSize_x
	25,	// mouseSize_y
	SHOUT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	JOEY_LOGIC,	// baseSub
	0,	// baseSub_off
	&joey_ext
};

uint16 baby_logic_list[] = {
	ID_FOSTER,
	0
};

uint16 save_restore_mouse[] = {
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	20,
	0
};

uint16 fradman_r_to_l[] = {
	45+90*64,
	44+90*64,
	43+90*64,
	0
};

uint16 foster_right[] = {
	4,
	30+12*64,
	4,
	0,
	4,
	31+12*64,
	4,
	0,
	4,
	32+12*64,
	4,
	0,
	4,
	33+12*64,
	4,
	0,
	4,
	34+12*64,
	4,
	0,
	4,
	35+12*64,
	4,
	0,
	4,
	36+12*64,
	4,
	0,
	4,
	37+12*64,
	4,
	0,
	4,
	38+12*64,
	4,
	0,
	4,
	39+12*64,
	4,
	0,
	0
};

uint16 seq2_pal[] = {
	0,
	256,
	512,
	512,
	529,
	1024,
	1552,
	1296,
	1042,
	1552,
	2064,
	1568,
	2320,
	2080,
	2576,
	2337,
	2320,
	1842,
	2337,
	2849,
	3104,
	2848,
	2353,
	2866,
	2883,
	2882,
	3138,
	3394,
	3155,
	3411,
	3412,
	3428
};

uint16 seq1_pal[] = {
	0,
	291,
	309,
	546,
	1075,
	1093,
	1314,
	1586,
	1604,
	1622,
	2133,
	2097,
	2114,
	2390,
	2387,
	2626,
	2167,
	2662,
	2679,
	2900,
	3138,
	3156,
	2696,
	3377,
	3190,
	3464,
	3685,
	3650,
	3668,
	3857,
	3703,
	3736
};

uint16 bfoster_r_to_d[] = {
	47+182*64,
	0
};

uint16 low_get_seq[] = {
	200*64,
	315,
	267,
	0,
	315,
	267,
	1,
	315,
	267,
	2,
	315,
	267,
	3,
	1,
	1,
	3,
	1,
	1,
	3,
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

uint16 fradman_u_to_r[] = {
	45+90*64,
	0
};

uint16 rs_mega_alive[] = {
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,
	65535
};

Compact whole_screen = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	136,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	TEXT_EDIT,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	192,	// mouseSize_y
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

uint16 bfoster_u_to_l[] = {
	43+182*64,
	0
};

Compact bar = {
	0,	// logic
	ST_MOUSE+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	bar_table,	// getToTable
	143,	// xcood
	205,	// ycood
	86*64,	// frame
	4295,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	13+4,	// mouseSize_x
	3,	// mouseSize_y
	BAR_ACTION,	// actionScript
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

uint16 foster_st_left[] = {
	12*64,
	1,
	0,
	42,
	0
};

uint16 monitor_l_to_r[] = {
	41-8+74*64,
	40-8+74*64,
	47-8+74*64,
	0
};

uint16 rs_lamb_start_3[] = {
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,
	C_PLACE,
	ID_S29_FLOOR,
	C_SCREEN,
	29,
	C_XCOOD,
	240,
	C_YCOOD,
	232,
	C_MOUSE_REL_X,
	65520,
	C_MOUSE_REL_Y,
	65488,
	C_DIR,
	DOWN,
	65535
};

uint16 forklift2_cdt[] = {
	94*64,
	291,
	292,
	0,
	291,
	292,
	1,
	291,
	292,
	2,
	291,
	292,
	3,
	291,
	292,
	4,
	291,
	292,
	5,
	291,
	292,
	6,
	291,
	292,
	7,
	291,
	292,
	8,
	291,
	292,
	9,
	291,
	292,
	10,
	291,
	292,
	11,
	291,
	292,
	12,
	291,
	292,
	13,
	291,
	292,
	14,
	291,
	292,
	15,
	291,
	292,
	16,
	291,
	292,
	17,
	291,
	292,
	18,
	291,
	280,
	19,
	291,
	281,
	20,
	291,
	282,
	21,
	291,
	283,
	19,
	291,
	284,
	20,
	291,
	285,
	21,
	291,
	286,
	19,
	291,
	287,
	20,
	291,
	288,
	21,
	291,
	289,
	19,
	291,
	290,
	20,
	291,
	291,
	21,
	291,
	292,
	19,
	291,
	292,
	22,
	291,
	292,
	23,
	291,
	292,
	24,
	291,
	292,
	25,
	291,
	292,
	26,
	291,
	292,
	27,
	291,
	292,
	28,
	291,
	292,
	29,
	291,
	292,
	30,
	291,
	292,
	31,
	291,
	292,
	32,
	291,
	292,
	33,
	292,
	292,
	34,
	292,
	292,
	35,
	292,
	292,
	36,
	292,
	292,
	37,
	292,
	292,
	38,
	292,
	292,
	39,
	292,
	292,
	40,
	292,
	292,
	41,
	292,
	292,
	42,
	292,
	292,
	43,
	293,
	292,
	44,
	293,
	292,
	45,
	297,
	292,
	46,
	297,
	292,
	47,
	299,
	292,
	48,
	299,
	292,
	49,
	299,
	292,
	50,
	299,
	292,
	51,
	299,
	292,
	52,
	299,
	292,
	53,
	299,
	292,
	54,
	299,
	292,
	55,
	299,
	292,
	56,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	299,
	292,
	57,
	0
};

Compact talk2 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	talk_table,	// getToTable
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

Compact text_4 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	277*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

uint16 mfoster_d_to_r[] = {
	47+13*64,
	0
};

Compact door = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_BACKGROUND+ST_RECREATE+ST_LOGIC,	// status
	0,	// sync
	0,	// screen
	0,	// place
	door_table,	// getToTable
	400,	// xcood
	167,	// ycood
	89*64,	// frame
	4148,	// cursorText
	FEXIT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	16,	// mouseSize_x
	58,	// mouseSize_y
	FIRE_EXIT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FEXIT_DOOR,	// baseSub
	0,	// baseSub_off
	0
};

uint16 foster_d_to_u[] = {
	47+12*64,
	46+12*64,
	45+12*64,
	0
};

uint16 park_table[] = {
	ID_SLOT,
	GET_TO_SLOT,
	65535
};

uint16 minif_auto[32];

uint16 mfoster_up[] = {
	1,
	0+13*64,
	0,
	65535,
	1,
	1+13*64,
	0,
	65535,
	1,
	2+13*64,
	0,
	65535,
	1,
	3+13*64,
	0,
	65535,
	1,
	4+13*64,
	0,
	65535,
	1,
	5+13*64,
	0,
	65535,
	1,
	6+13*64,
	0,
	65535,
	1,
	7+13*64,
	0,
	65535,
	1,
	8+13*64,
	0,
	65535,
	1,
	9+13*64,
	0,
	65535,
	0
};

uint16 mfoster_down[] = {
	1,
	10+13*64,
	0,
	1,
	1,
	11+13*64,
	0,
	1,
	1,
	12+13*64,
	0,
	1,
	1,
	13+13*64,
	0,
	1,
	1,
	14+13*64,
	0,
	1,
	1,
	15+13*64,
	0,
	1,
	1,
	16+13*64,
	0,
	1,
	1,
	17+13*64,
	0,
	1,
	1,
	18+13*64,
	0,
	1,
	1,
	19+13*64,
	0,
	1,
	0
};

uint16 mfoster_right[] = {
	2,
	30+13*64,
	2,
	0,
	2,
	31+13*64,
	2,
	0,
	2,
	32+13*64,
	2,
	0,
	2,
	33+13*64,
	2,
	0,
	2,
	34+13*64,
	2,
	0,
	2,
	35+13*64,
	2,
	0,
	2,
	36+13*64,
	2,
	0,
	2,
	37+13*64,
	2,
	0,
	2,
	38+13*64,
	2,
	0,
	2,
	39+13*64,
	2,
	0,
	0
};

uint16 mfoster_st_left[] = {
	13*64,
	1,
	1,
	42,
	0
};

uint16 mfoster_u_to_l[] = {
	43+13*64,
	0
};

uint16 mfoster_l_to_d[] = {
	41+13*64,
	0
};

uint16 mfoster_r_to_d[] = {
	47+13*64,
	0
};

uint16 mfoster_r_to_l[] = {
	45+13*64,
	44+13*64,
	43+13*64,
	0
};

uint16 foster_down[] = {
	2,
	10+12*64,
	0,
	2,
	2,
	11+12*64,
	0,
	2,
	2,
	12+12*64,
	0,
	2,
	2,
	13+12*64,
	0,
	2,
	2,
	14+12*64,
	0,
	2,
	2,
	15+12*64,
	0,
	2,
	2,
	16+12*64,
	0,
	2,
	2,
	17+12*64,
	0,
	2,
	2,
	18+12*64,
	0,
	2,
	2,
	19+12*64,
	0,
	2,
	0
};

uint16 foster_st_up[] = {
	12*64,
	1,
	0,
	44,
	0
};

uint16 foster_u_to_r[] = {
	45+12*64,
	0
};

uint16 foster_d_to_r[] = {
	47+12*64,
	0
};

uint16 foster_l_to_d[] = {
	41+12*64,
	0
};

uint16 foster_l_to_r[] = {
	41+12*64,
	40+12*64,
	47+12*64,
	0
};

uint16 foster_r_to_d[] = {
	47+12*64,
	0
};

uint16 foster_r_to_l[] = {
	45+12*64,
	44+12*64,
	43+12*64,
	0
};

uint16 foster_t_to_u[] = {
	47+12*64,
	46+12*64,
	45+12*64,
	0
};

uint16 foster_t_to_l[] = {
	40+12*64,
	41+12*64,
	0
};

uint16 fradman_st_down[] = {
	90*64,
	1,
	0,
	40,
	0
};

uint16 fradman_st_left[] = {
	90*64,
	1,
	0,
	42,
	0
};

uint16 fradman_st_talk[] = {
	97*64,
	1,
	0,
	0,
	0
};

uint16 fradman_d_to_l[] = {
	41+90*64,
	0
};

uint16 fradman_d_to_r[] = {
	47+90*64,
	0
};

uint16 fradman_l_to_u[] = {
	43+90*64,
	0
};

uint16 fradman_r_to_u[] = {
	45+90*64,
	0
};

uint16 fradman_r_to_d[] = {
	47+90*64,
	0
};

uint16 bfoster_right[] = {
	4,
	30+182*64,
	4,
	0,
	4,
	31+182*64,
	4,
	0,
	4,
	32+182*64,
	4,
	0,
	4,
	33+182*64,
	4,
	0,
	4,
	34+182*64,
	4,
	0,
	4,
	35+182*64,
	4,
	0,
	4,
	36+182*64,
	4,
	0,
	4,
	37+182*64,
	4,
	0,
	4,
	38+182*64,
	4,
	0,
	4,
	39+182*64,
	4,
	0,
	0
};

uint16 bfoster_st_up[] = {
	182*64,
	1,
	0,
	44,
	0
};

uint16 bfoster_st_down[] = {
	182*64,
	1,
	0,
	40,
	0
};

uint16 bfoster_u_to_d[] = {
	43+182*64,
	42+182*64,
	41+182*64,
	0
};

uint16 bfoster_d_to_l[] = {
	41+182*64,
	0
};

uint16 bfoster_l_to_u[] = {
	43+182*64,
	0
};

uint16 bfoster_l_to_d[] = {
	41+182*64,
	0
};

uint16 bfoster_l_to_r[] = {
	41+182*64,
	40+182*64,
	47+182*64,
	0
};

TurnTable foster_turnTable0 = {
	{ // turnTableUp
		0,
		mfoster_u_to_d,
		mfoster_u_to_l,
		mfoster_u_to_r,
		0
	},
	{ // turnTableDown
		mfoster_d_to_u,
		0,
		mfoster_d_to_l,
		mfoster_d_to_r,
		0
	},
	{ // turnTableLeft
		mfoster_l_to_u,
		mfoster_l_to_d,
		0,
		mfoster_l_to_r,
		0
	},
	{ // turnTableRight
		mfoster_r_to_u,
		mfoster_r_to_d,
		mfoster_r_to_l,
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

TurnTable foster_turnTable1 = {
	{ // turnTableUp
		0,
		foster_u_to_d,
		foster_u_to_l,
		foster_u_to_r,
		foster_u_to_t
	},
	{ // turnTableDown
		foster_d_to_u,
		0,
		foster_d_to_l,
		foster_d_to_r,
		0
	},
	{ // turnTableLeft
		foster_l_to_u,
		foster_l_to_d,
		0,
		foster_l_to_r,
		foster_l_to_t
	},
	{ // turnTableRight
		foster_r_to_u,
		foster_r_to_d,
		foster_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		foster_t_to_u,
		0,
		foster_t_to_l,
		0,
		0
	},
};

TurnTable foster_turnTable2 = {
	{ // turnTableUp
		0,
		fradman_u_to_d,
		fradman_u_to_l,
		fradman_u_to_r,
		0
	},
	{ // turnTableDown
		fradman_d_to_u,
		0,
		fradman_d_to_l,
		fradman_d_to_r,
		0
	},
	{ // turnTableLeft
		fradman_l_to_u,
		fradman_l_to_d,
		0,
		fradman_l_to_r,
		0
	},
	{ // turnTableRight
		fradman_r_to_u,
		fradman_r_to_d,
		fradman_r_to_l,
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

TurnTable foster_turnTable3 = {
	{ // turnTableUp
		0,
		bfoster_u_to_d,
		bfoster_u_to_l,
		bfoster_u_to_r,
		0
	},
	{ // turnTableDown
		bfoster_d_to_u,
		0,
		bfoster_d_to_l,
		bfoster_d_to_r,
		0
	},
	{ // turnTableLeft
		bfoster_l_to_u,
		bfoster_l_to_d,
		0,
		bfoster_l_to_r,
		0
	},
	{ // turnTableRight
		bfoster_r_to_u,
		bfoster_r_to_d,
		bfoster_r_to_l,
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

MegaSet foster_megaSet0 = {
	0,	// gridWidth
	0,	// colOffset
	8,	// colWidth
	8,	// lastChr
	mfoster_up,	// animUp
	mfoster_down,	// animDown
	mfoster_left,	// animLeft
	mfoster_right,	// animRight
	mfoster_st_up,	// standUp
	mfoster_st_down,	// standDown
	mfoster_st_left,	// standLeft
	mfoster_st_right,	// standRight
	0,	// standTalk
	&foster_turnTable0,
};

MegaSet foster_megaSet1 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	foster_up,	// animUp
	foster_down,	// animDown
	foster_left,	// animLeft
	foster_right,	// animRight
	foster_st_up,	// standUp
	foster_st_down,	// standDown
	foster_st_left,	// standLeft
	foster_st_right,	// standRight
	foster_st_talk,	// standTalk
	&foster_turnTable1,
};

MegaSet foster_megaSet2 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	fradman_up,	// animUp
	fradman_down,	// animDown
	fradman_left,	// animLeft
	fradman_right,	// animRight
	fradman_st_up,	// standUp
	fradman_st_down,	// standDown
	fradman_st_left,	// standLeft
	fradman_st_right,	// standRight
	fradman_st_talk,	// standTalk
	&foster_turnTable2,
};

MegaSet foster_megaSet3 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	bfoster_up,	// animUp
	bfoster_down,	// animDown
	bfoster_left,	// animLeft
	bfoster_right,	// animRight
	bfoster_st_up,	// standUp
	bfoster_st_down,	// standDown
	bfoster_st_left,	// standLeft
	bfoster_st_right,	// standRight
	0,	// standTalk
	&foster_turnTable3
};

ExtCompact foster_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	0,	// dir
	STD_PLAYER_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_COL_FOSTER,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	minif_auto,	// animScratch
	0,	// megaSet
	&foster_megaSet0,
	&foster_megaSet1,
	&foster_megaSet2,
	&foster_megaSet3
};

Compact foster = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_AR_PRIORITY+ST_GRID_PLOT+ST_COLLISION,	// status
	0,	// sync
	SAFE_START_SCREEN,	// screen
	67,	// place
	0,	// getToTable
	160,	// xcood
	280,	// ycood
	13*64,	// frame
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
	BASE_INTRO,	// baseSub
	0,	// baseSub_off
	&foster_ext
};

uint16 monitor_up[] = {
	2,
	0+74*64,
	0,
	65534,
	2,
	1+74*64,
	0,
	65534,
	2,
	2+74*64,
	0,
	65534,
	2,
	3+74*64,
	0,
	65534,
	2,
	4+74*64,
	0,
	65534,
	2,
	5+74*64,
	0,
	65534,
	2,
	6+74*64,
	0,
	65534,
	2,
	7+74*64,
	0,
	65534,
	0
};

uint16 s0_fast_list[] = {
	12,
	51,
	52,
	53,
	13,
	55,
	22,
	158,
	14+0X8000,
	15+0X8000,
	71+0X8000,
	72+0X8000,
	0
};

uint16 s7_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	134+DISK_2,
	146+DISK_2,
	151+DISK_2,
	149+DISK_2,
	0
};

uint16 monitor_st_talk[] = {
	134*64,
	1,
	1,
	0,
	0
};

uint16 fast_list_sc90[] = {
	IT_BLUE_FOSTER+DISK_4,
	IT_LOGOFF+0X8000+DISK_4,
	IT_WINDOW+0X8000+DISK_4,
	IT_INFO_BUTTON+DISK_4,
	IT_LINK_ARROWS+DISK_4,
	IT_LINK_OBJECTS+DISK_4,
	IT_CROUCH_RIGHT+DISK_4,
	IT_EYEBALL+DISK_4,
	IT_GET_EYE+DISK_4+0X8000,
	IT_BLIND_EYE+DISK_4,
	IT_SEE_EYE+DISK_4,
	IT_EYE90_ZAP+DISK_4+0X8000,
	IT_SC90_LAYER_0+DISK_4,
	IT_SC90_LAYER_1+DISK_4,
	IT_SC90_GRID_1+DISK_4,
	0
};

uint16 but_2[] = {
	142*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 but_3[] = {
	143*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 monitor_st_down[] = {
	74*64,
	1,
	1,
	32,
	0
};

uint32 *zt = (uint32*)data_0;

uint16 but_9[] = {
	149*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

Compact text_11 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	284*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

Compact joey_park = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	park_table,	// getToTable
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

Compact coordinate_test = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	128+8,	// xcood
	136+8,	// ycood
	19*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	CLICK_DEBUG,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	64,	// mouseSize_x
	12,	// mouseSize_y
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

Compact text_2 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	275*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

Compact text_3 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	276*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

Compact text_6 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	279*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

Compact text_7 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	280*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

Compact text_9 = {
	0,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	282*64,	// frame
	0,	// cursorText
	TEXT_ON,	// mouseOn
	TEXT_OFF,	// mouseOff
	TEXT_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	12,	// mouseSize_y
	0,	// actionScript
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

uint16 rad_shrug_seq[] = {
	90*64,
	1,
	1,
	48,
	1,
	1,
	48,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	0
};

uint32 *talk1 = (uint32*)(&talk2);

uint16 rs_lamb_start_2[] = {
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,
	C_PLACE,
	ID_S7_FLOOR,
	C_SCREEN,
	7,
	C_XCOOD,
	256,
	C_YCOOD,
	224,
	C_MOUSE_REL_X,
	65520,
	C_MOUSE_REL_Y,
	65488,
	C_DIR,
	DOWN,
	65535
};

uint16 s6_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	24+DISK_2+0X8000,
	25+DISK_2+0X8000,
	0
};

uint16 swing_list[] = {
	85+DISK_1,
	86+DISK_1,
	87+DISK_1,
	88+DISK_1,
	0
};

Compact right_exit0 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	393,	// xcood
	250,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	9,	// mouseSize_x
	36,	// mouseSize_y
	ER0_ACTION,	// actionScript
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

uint16 rs_blank[] = {
	C_FRAME,
	50*64,
	C_BASE_SUB,
	MENU_SCRIPT,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 monitor_auto[32];

uint16 monitor_down[] = {
	2,
	8+74*64,
	0,
	2,
	2,
	9+74*64,
	0,
	2,
	2,
	10+74*64,
	0,
	2,
	2,
	11+74*64,
	0,
	2,
	2,
	12+74*64,
	0,
	2,
	2,
	13+74*64,
	0,
	2,
	2,
	14+74*64,
	0,
	2,
	2,
	15+74*64,
	0,
	2,
	0
};

uint16 monitor_st_left[] = {
	74*64,
	1,
	1,
	34,
	0
};

uint16 monitor_st_right[] = {
	74*64,
	1,
	1,
	38,
	0
};

uint16 monitor_u_to_r[] = {
	45-8+74*64,
	0
};

uint16 monitor_d_to_u[] = {
	47-8+74*64,
	46-8+74*64,
	45-8+74*64,
	0
};

uint16 monitor_d_to_l[] = {
	41-8+74*64,
	0
};

uint16 monitor_l_to_d[] = {
	41-8+74*64,
	0
};

uint16 monitor_r_to_d[] = {
	47-8+74*64,
	0
};

uint16 monitor_r_to_l[] = {
	45-8+74*64,
	44-8+74*64,
	43-8+74*64,
	0
};

TurnTable monitor_turnTable0 = {
	{ // turnTableUp
		0,
		monitor_u_to_d,
		monitor_u_to_l,
		monitor_u_to_r,
		0
	},
	{ // turnTableDown
		monitor_d_to_u,
		0,
		monitor_d_to_l,
		monitor_d_to_r,
		0
	},
	{ // turnTableLeft
		monitor_l_to_u,
		monitor_l_to_d,
		0,
		monitor_l_to_r,
		0
	},
	{ // turnTableRight
		monitor_r_to_u,
		monitor_r_to_d,
		monitor_r_to_l,
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

MegaSet monitor_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	monitor_up,	// animUp
	monitor_down,	// animDown
	monitor_left,	// animLeft
	monitor_right,	// animRight
	monitor_st_up,	// standUp
	monitor_st_down,	// standDown
	monitor_st_left,	// standLeft
	monitor_st_right,	// standRight
	monitor_st_talk,	// standTalk
	&monitor_turnTable0
};

ExtCompact monitor_ext = {
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
	SP_COL_MONITOR,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	monitor_auto,	// animScratch
	0,	// megaSet
	&monitor_megaSet0,
	0,
	0,
	0
};

Compact monitor = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	18,	// screen
	ID_S18_FLOOR,	// place
	0,	// getToTable
	250,	// xcood
	231,	// ycood
	75*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	260-250,	// mouseRel_x
	244-231,	// mouseRel_y
	280-260,	// mouseSize_x
	276-244,	// mouseSize_y
	SAT_GORDON_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	MONITOR_SLEEP,	// baseSub
	0,	// baseSub_off
	&monitor_ext
};

ExtCompact anita_ext = {
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
	SP_COL_ANITA,	// spColour
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

Compact anita = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	12,	// screen
	0,	// place
	0,	// getToTable
	0X157,	// xcood
	0XB3,	// ycood
	27*64,	// frame
	54,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	4,	// mouseRel_x
	4,	// mouseRel_y
	24,	// mouseSize_x
	48,	// mouseSize_y
	ANITA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	ANITA_WORK,	// baseSub
	0,	// baseSub_off
	&anita_ext
};

uint16 rs_r_arr_linc[] = {
	C_FRAME,
	1+190*64,
	C_BASE_SUB,
	LINC_MENU_SCRIPT,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 s5_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	17+DISK_2,
	18+DISK_2,
	113+DISK_2,
	0
};

uint16 but_e[] = {
	150*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 but_1[] = {
	141*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 but_6[] = {
	146*64,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	0,
	0
};

uint16 fast_intro_2[] = {
	189+DISK_15,
	190+DISK_15,
	191+DISK_15,
	0
};

uint16 fast_intro_3[] = {
	192+DISK_15,
	193+DISK_15,
	194+DISK_15,
	0
};

uint16 retina_scan_cdt[] = {
	151*64,
	378,
	302,
	0,
	378,
	302,
	0,
	378,
	302,
	1,
	378,
	302,
	2,
	378,
	302,
	3,
	378,
	302,
	4,
	378,
	302,
	5,
	378,
	302,
	6,
	378,
	302,
	7,
	378,
	302,
	7,
	378,
	302,
	7,
	378,
	302,
	7,
	378,
	302,
	7,
	378,
	302,
	7,
	378,
	302,
	7,
	378,
	302,
	8,
	378,
	302,
	9,
	378,
	302,
	10,
	378,
	302,
	11,
	378,
	302,
	12,
	378,
	302,
	13,
	378,
	302,
	13,
	378,
	302,
	14,
	378,
	302,
	15,
	378,
	302,
	16,
	378,
	302,
	17,
	0
};

Compact forklift_cpt = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	93*64,	// frame
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
	FORKLIFT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

void *data_0[] = {
	0,
	&joey,
	&joey_park,
	&foster,
	std_menu_logic,
	text_mouse,
	&gallcard_menu,
	rs_mega_alive,
	&citycard_menu,
	&shades_menu,
	&putty_menu,
	&lightbulb_menu,
	low_get_seq,
	mini_shrug_seq,
	sml_up_mid_get_seq,
	new_grid,
	&lamb,
	&floor,
	&coordinate_test,
	save_restore_mouse,
	&whole_screen,
	l_talk_s2,
	&r_talk_s2,
	&text_1,
	&text_2,
	&text_3,
	&text_4,
	&text_5,
	&text_6,
	&text_7,
	&text_8,
	&text_9,
	&text_10,
	&text_11,
	&wd40_menu,
	&skey_menu,
	&secateurs_menu,
	&rope_menu,
	&plaster_menu,
	&new_cable_menu,
	shrug_seq,
	rad_shrug_seq,
	&brick_menu,
	&tongs_menu,
	talk1,
	&talk2,
	&menu_bar,
	&left_arrow,
	&right_arrow,
	&dog_food_menu,
	0,
	&blank1,
	&blank2,
	&blank3,
	&blank4,
	&blank5,
	&blank6,
	&blank7,
	&blank8,
	&blank9,
	&blank10,
	&blank11,
	0,
	&crow_bar_menu,
	&sarnie_menu,
	&spanner_menu,
	&joeyb_menu,
	&low_floor,
	0,
	&stairs,
	&upstairs,
	&anita_card_menu,
	rs_lamb_to_three,
	rs_lamb_start_2,
	&anchor_menu,
	&magazine_menu,
	&tape_menu,
	&glass_menu,
	rs_lamb_start_3,
	&ticket_menu,
	s29_fast_list,
	s6_fast_list,
	fast_list_sc3,
	s9_fast_list,
	s10_fast_list,
	&bar,
	s11_fast_list,
	fast_list_0,
	s0_fast_list,
	s7_fast_list,
	&door,
	s28_fast_list,
	swing_list,
	0,
	0,
	&outside_ledge,
	0,
	&r1_door,
	0,
	0,
	0,
	0,
	fast_list_sc90,
	0,
	0,
	&small_door,
	sc85_fast_list,
	sc85_chip_list,
	sc85_logic_list,
	sc85_mouse_list,
	sc85_palette,
	&right_exit0,
	0,
	0,
	0,
	&s2_floor,
	0,
	s101_chip_list,
	s101_pal,
	s101_mouse,
	s101_logic,
	&full_screen,
	&cancel_button,
	&button_0,
	&button_1,
	&button_2,
	&button_3,
	&button_4,
	&button_5,
	&button_6,
	&button_7,
	&button_8,
	&button_9,
	rs_left_arrow,
	rs_right_arrow,
	rs_blank,
	&monitor,
	&anita,
	0,
	0,
	0,
	baby_logic_list,
	rs_l_arr_linc,
	rs_r_arr_linc,
	rs_blanks_linc,
	s5_fast_list,
	but_e,
	but_0,
	but_1,
	but_2,
	but_3,
	but_4,
	but_5,
	but_6,
	but_7,
	but_8,
	but_9,
	0,
	s102_chip_list,
	s102_pal,
	s102_logic,
	s102_mouse,
	&restart_butt,
	restart_seq,
	&restore_butt,
	restore_seq,
	seq1_pal,
	seq2_pal,
	seq3_pal,
	fast_intro,
	chip_intro,
	fast_intro_2,
	fast_intro_3,
	&retina_scan,
	retina_scan_cdt,
	&exit_butt,
	exit_seq,
	&forklift_cpt,
	forklift1_cdt,
	forklift2_cdt
};

uint32 *z_compact_table = (uint32*)data_0;

} // namespace SkyCompact

} // namespace Sky

#endif
