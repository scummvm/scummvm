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

#ifndef SKY73COMP_H
#define SKY73COMP_H




namespace Sky {

namespace SkyCompact {

uint16 gal_u_to_l[] = {
	35+90*64,
	0
};

Compact sc73_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	185,	// xcood
	162,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	205-185,	// mouseSize_x
	283-162,	// mouseSize_y
	SC73_EXIT_ACTION,	// actionScript
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

uint16 sc73_cham5_flash[] = {
	96*64,
	291,
	242,
	0,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	291,
	242,
	1,
	0
};

uint16 sc73_cham5_anim[] = {
	141*64,
	279,
	190,
	0,
	279,
	190,
	1,
	279,
	190,
	2,
	279,
	190,
	3,
	279,
	190,
	4,
	279,
	190,
	5,
	279,
	190,
	6,
	279,
	190,
	7,
	279,
	190,
	8,
	279,
	190,
	9,
	279,
	190,
	10,
	279,
	190,
	11,
	279,
	190,
	12,
	279,
	190,
	13,
	279,
	190,
	14,
	279,
	190,
	15,
	279,
	190,
	16,
	279,
	190,
	17,
	279,
	190,
	18,
	279,
	190,
	19,
	0
};

uint16 gal_auto[32];

uint16 gal_up[] = {
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
	0
};

uint16 gal_down[] = {
	2,
	8+90*64,
	0,
	2,
	2,
	9+90*64,
	0,
	2,
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
	0
};

uint16 gal_left[] = {
	4,
	16+90*64,
	65532,
	0,
	4,
	17+90*64,
	65532,
	0,
	4,
	18+90*64,
	65532,
	0,
	4,
	19+90*64,
	65532,
	0,
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
	0
};

uint16 gal_right[] = {
	4,
	24+90*64,
	4,
	0,
	4,
	25+90*64,
	4,
	0,
	4,
	26+90*64,
	4,
	0,
	4,
	27+90*64,
	4,
	0,
	4,
	28+90*64,
	4,
	0,
	4,
	29+90*64,
	4,
	0,
	4,
	30+90*64,
	4,
	0,
	4,
	31+90*64,
	4,
	0,
	0
};

uint16 gal_st_up[] = {
	90*64,
	1,
	0,
	36,
	0
};

uint16 gal_st_down[] = {
	90*64,
	1,
	0,
	32,
	0
};

uint16 gal_st_left[] = {
	90*64,
	1,
	0,
	34,
	0
};

uint16 gal_st_right[] = {
	90*64,
	1,
	0,
	38,
	0
};

uint16 gal_u_to_d[] = {
	35+90*64,
	34+90*64,
	33+90*64,
	0
};

uint16 gal_u_to_r[] = {
	37+90*64,
	0
};

uint16 gal_d_to_u[] = {
	39+90*64,
	38+90*64,
	37+90*64,
	0
};

uint16 gal_d_to_l[] = {
	33+90*64,
	0
};

uint16 gal_d_to_r[] = {
	39+90*64,
	0
};

uint16 gal_l_to_u[] = {
	35+90*64,
	0
};

uint16 gal_l_to_d[] = {
	33+90*64,
	0
};

uint16 gal_l_to_r[] = {
	33+90*64,
	32+90*64,
	39+90*64,
	0
};

uint16 gal_r_to_u[] = {
	37+90*64,
	0
};

uint16 gal_r_to_d[] = {
	39+90*64,
	0
};

uint16 gal_r_to_l[] = {
	37+90*64,
	36+90*64,
	35+90*64,
	0
};

TurnTable gallagher_turnTable0 = {
	{ // turnTableUp
		0,
		gal_u_to_d,
		gal_u_to_l,
		gal_u_to_r,
		0
	},
	{ // turnTableDown
		gal_d_to_u,
		0,
		gal_d_to_l,
		gal_d_to_r,
		0
	},
	{ // turnTableLeft
		gal_l_to_u,
		gal_l_to_d,
		0,
		gal_l_to_r,
		0
	},
	{ // turnTableRight
		gal_r_to_u,
		gal_r_to_d,
		gal_r_to_l,
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

MegaSet gallagher_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	gal_up,	// animUp
	gal_down,	// animDown
	gal_left,	// animLeft
	gal_right,	// animRight
	gal_st_up,	// standUp
	gal_st_down,	// standDown
	gal_st_left,	// standLeft
	gal_st_right,	// standRight
	0,	// standTalk
	&gallagher_turnTable0
};

ExtCompact gallagher_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	LEFT,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_COL_GALAG,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	gal_auto,	// animScratch
	0,	// megaSet
	&gallagher_megaSet0,
	0,
	0,
	0
};

Compact gallagher = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION,	// status
	0,	// sync
	73,	// screen
	ID_SC73_FLOOR,	// place
	0,	// getToTable
	408,	// xcood
	296,	// ycood
	34+90*64,	// frame
	20630,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	1,	// mouseRel_y
	32,	// mouseSize_x
	16,	// mouseSize_y
	SC73_CORPSE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	GALLAGHER_LOGIC73,	// baseSub
	0,	// baseSub_off
	&gallagher_ext
};

uint16 sc73_joey_fight1[] = {
	86*64,
	313,
	240,
	0,
	315,
	240,
	0,
	316,
	240,
	0,
	317,
	240,
	0,
	318,
	240,
	0,
	318,
	240,
	0,
	312,
	241,
	1,
	303,
	238,
	2,
	299,
	237,
	3,
	297,
	239,
	4,
	293,
	244,
	5,
	292,
	242,
	6,
	293,
	242,
	7,
	287,
	244,
	8,
	281,
	245,
	9,
	279,
	248,
	10,
	276,
	248,
	11,
	272,
	247,
	12,
	271,
	246,
	13,
	270,
	247,
	14,
	271,
	248,
	15,
	270,
	249,
	16,
	271,
	250,
	17,
	271,
	251,
	18,
	271,
	252,
	19,
	271,
	253,
	20,
	271,
	254,
	21,
	271,
	255,
	22,
	271,
	255,
	22,
	271,
	255,
	22,
	0
};

uint16 sc73_gall_fight2[] = {
	89*64,
	318,
	227,
	0,
	317,
	227,
	1,
	310,
	226,
	2,
	304,
	226,
	3,
	304,
	227,
	4,
	300,
	226,
	5,
	298,
	226,
	0,
	299,
	226,
	6,
	299,
	226,
	7,
	296,
	226,
	8,
	296,
	226,
	9,
	297,
	226,
	10,
	297,
	226,
	10,
	297,
	226,
	10,
	297,
	226,
	10,
	297,
	226,
	10,
	284,
	228,
	11,
	282,
	233,
	12,
	285,
	231,
	13,
	290,
	228,
	14,
	293,
	227,
	15,
	295,
	227,
	16,
	297,
	228,
	17,
	299,
	230,
	18,
	298,
	232,
	19,
	295,
	239,
	20,
	294,
	247,
	21,
	294,
	255,
	22,
	294,
	265,
	23,
	294,
	266,
	24,
	294,
	266,
	25,
	294,
	266,
	26,
	294,
	266,
	27,
	294,
	266,
	28,
	294,
	266,
	29,
	294,
	266,
	30,
	294,
	266,
	31,
	294,
	266,
	32,
	294,
	266,
	33,
	0
};

uint16 reset_72_73[] = {
	C_SCREEN,
	73,
	C_PLACE,
	ID_SC73_FLOOR,
	C_XCOOD,
	168,
	C_YCOOD,
	264,
	65535
};

uint16 sc73_joey_fight2[] = {
	88*64,
	271,
	253,
	0,
	271,
	255,
	1,
	271,
	255,
	1,
	271,
	255,
	1,
	271,
	252,
	2,
	271,
	255,
	1,
	271,
	255,
	1,
	271,
	251,
	3,
	271,
	255,
	1,
	271,
	255,
	1,
	271,
	255,
	1,
	271,
	255,
	4,
	271,
	255,
	5,
	271,
	254,
	6,
	271,
	250,
	7,
	271,
	250,
	7,
	269,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	268,
	255,
	8,
	0
};

uint16 sc73_floor_table[] = {
	ID_SC73_FLOOR,
	RET_OK,
	ID_SC73_EXIT,
	GT_SC73_EXIT,
	0,
	SC73_EXIT_WALK_ON,
	1,
	SC73_DOOR_WALK_ON,
	3,
	SC73_BIG_DOOR_WALK_ON,
	ID_SC73_CHAMBER4,
	GT_SC73_CHAMBER4,
	ID_SC73_CHAMBER5,
	GT_SC73_CHAMBER5,
	ID_SC73_BIG_DOOR,
	GT_SC73_BIG_DOOR,
	ID_SC73_SENSOR,
	GT_SC73_SENSOR,
	ID_SC73_DOOR,
	GT_SC73_DOOR,
	ID_SC73_LOCKED_DOOR,
	GT_SC73_LOCKED_DOOR,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	4,
	GT_SC73_JOEY_WAIT,
	5,
	GT_SC73_GALL_1,
	6,
	GT_SC73_GALL_2,
	ID_MEDI,
	GT_SC73_WRECKED_DROID,
	ID_GALLAGHER,
	GT_SC73_CORPSE,
	65535
};

Compact sc73_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	sc73_floor_table,	// getToTable
	188,	// xcood
	260,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	375-188,	// mouseSize_x
	327-260,	// mouseSize_y
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

uint16 sc73_bits_anim[] = {
	74*64,
	322,
	237,
	0,
	322,
	237,
	0,
	322,
	237,
	0,
	322,
	237,
	0,
	322,
	237,
	0,
	322,
	237,
	0,
	322,
	237,
	0,
	314,
	238,
	1,
	312,
	237,
	2,
	310,
	239,
	3,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	279,
	244,
	4,
	261,
	243,
	5,
	251,
	243,
	6,
	244,
	248,
	7,
	234,
	259,
	8,
	0
};

uint16 sc73_gall_fight1[] = {
	87*64,
	339,
	227,
	0,
	339,
	227,
	1,
	339,
	227,
	2,
	336,
	227,
	3,
	336,
	227,
	4,
	335,
	228,
	5,
	324,
	229,
	6,
	317,
	230,
	7,
	319,
	231,
	8,
	321,
	230,
	9,
	320,
	229,
	10,
	318,
	229,
	11,
	305,
	230,
	12,
	305,
	230,
	13,
	313,
	229,
	14,
	316,
	227,
	15,
	318,
	227,
	16,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	319,
	227,
	17,
	0
};

Compact sc73_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	362,	// xcood
	212,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	369-362,	// mouseSize_x
	270-212,	// mouseSize_y
	SC73_DOOR_ACTION,	// actionScript
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

uint16 sc73_cham4_anim[] = {
	140*64,
	225,
	190,
	0,
	225,
	190,
	1,
	225,
	190,
	2,
	225,
	190,
	3,
	225,
	190,
	4,
	225,
	190,
	5,
	225,
	190,
	6,
	225,
	190,
	7,
	225,
	190,
	8,
	225,
	190,
	9,
	225,
	190,
	10,
	225,
	190,
	11,
	225,
	190,
	12,
	225,
	190,
	13,
	225,
	190,
	14,
	225,
	190,
	15,
	225,
	190,
	16,
	225,
	190,
	17,
	225,
	190,
	18,
	225,
	190,
	19,
	0
};

Compact sc73_bits = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	322,	// xcood
	237,	// ycood
	74*64,	// frame
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
	SC73_BITS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc73_locked_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	162,	// xcood
	236,	// ycood
	0,	// frame
	181,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	178-162,	// mouseSize_x
	298-236,	// mouseSize_y
	SC73_LOCKED_DOOR_ACTION,	// actionScript
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

uint16 sc73_mouse_list[] = {
	ID_MEDI,
	ID_GALLAGHER,
	ID_SC73_CHAMBER4,
	ID_SC73_CHAMBER5,
	ID_SC73_SENSOR,
	ID_SC73_BIG_DOOR,
	ID_SC73_EXIT,
	ID_SC73_DOOR,
	ID_SC73_LOCKED_DOOR,
	ID_SC73_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc73_sensor = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	405,	// xcood
	251,	// ycood
	139*64,	// frame
	20570,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65532,	// mouseRel_y
	410-401,	// mouseSize_x
	271-247,	// mouseSize_y
	SC73_SENSOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC73_SENSOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc73_cham5_light = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	291,	// xcood
	242,	// ycood
	96*64,	// frame
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
	SC73_CHAM5_LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc73_joey_lunge[] = {
	85*64,
	254,
	240,
	0,
	256,
	241,
	1,
	260,
	242,
	2,
	266,
	243,
	3,
	274,
	243,
	3,
	282,
	243,
	3,
	290,
	242,
	2,
	296,
	241,
	1,
	302,
	240,
	0,
	306,
	240,
	0,
	310,
	240,
	0,
	0
};

uint16 sc73_get_board[] = {
	43*64,
	272,
	216,
	0,
	272,
	216,
	1,
	272,
	216,
	2,
	272,
	216,
	3,
	272,
	216,
	3,
	272,
	216,
	3,
	272,
	216,
	3,
	272,
	216,
	3,
	272,
	216,
	3,
	272,
	216,
	3,
	272,
	216,
	3,
	272,
	216,
	2,
	272,
	216,
	1,
	272,
	216,
	0,
	0
};

uint16 sc73_chip_list[] = {
	IT_SC73_LAYER_0+DISK_7,
	IT_SC73_LAYER_1+DISK_7,
	IT_SC73_LAYER_2+DISK_7,
	IT_SC73_GRID_1+DISK_7,
	IT_SC73_GRID_2+DISK_7,
	IT_MEDI+DISK_7,
	IT_MEDI_TALK+DISK_7,
	IT_GALLAGHER+DISK_7+0X8000,
	IT_GALL_TALK+DISK_7,
	IT_SC73_SENSOR+DISK_7,
	IT_SC73_CHAMBER3+DISK_7,
	IT_SC73_CHAMBER4+DISK_7,
	IT_SC73_CHAMBER5+DISK_7,
	IT_SC73_BIG_DOOR+DISK_7,
	IT_SC73_CHAM4_LIGHT+DISK_7,
	IT_SC73_CHAM5_LIGHT+DISK_7,
	IT_SC73_JOEY_LUNGE+DISK_7,
	IT_SC73_JOEY_FIGHT1+DISK_7,
	IT_SC73_JOEY_FIGHT2+DISK_7,
	IT_SC73_DEAD_GALL+DISK_7,
	IT_SC73_BITS+DISK_7,
	IT_SC73_BITS2+DISK_7,
	IT_SC73_SPRAY+DISK_7,
	0
};

Compact sc73_spray = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	312,	// xcood
	224,	// ycood
	76*64,	// frame
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
	SC73_SPRAY_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 reset_74_73[] = {
	C_SCREEN,
	73,
	C_PLACE,
	ID_SC73_FLOOR,
	C_XCOOD,
	384,
	C_YCOOD,
	264,
	65535
};

uint32 *grid73 = 0;

Compact sc73_bits2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	285,	// xcood
	248,	// ycood
	75*64,	// frame
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
	SC73_BITS2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc73_sensor_anim[] = {
	139*64,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	0,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	1,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	2,
	405,
	251,
	3,
	0
};

uint16 sc73_fast_list[] = {
	12+DISK_7,
	51+DISK_7,
	52+DISK_7,
	53+DISK_7,
	54+DISK_7,
	268+DISK_7,
	0
};

uint16 sc73_joey_list[] = {
	176,
	391,
	272,
	319,
	1,
	176,
	199,
	272,
	295,
	0,
	384,
	391,
	288,
	303,
	0,
	176,
	207,
	296,
	303,
	0,
	0
};

uint16 sc73_spray_anim[] = {
	76*64,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	312,
	224,
	0,
	306,
	235,
	1,
	311,
	226,
	2,
	315,
	225,
	3,
	325,
	220,
	4,
	336,
	224,
	5,
	348,
	234,
	6,
	0
};

Compact sc73_chamber4 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	225,	// xcood
	190,	// ycood
	140*64,	// frame
	20569,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	(int16) 65533,	// mouseRel_y
	253-224,	// mouseSize_x
	238-188,	// mouseSize_y
	SC73_CHAMBER4_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC73_CHAMBER4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc73_chamber3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	189,	// xcood
	190,	// ycood
	142*64,	// frame
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
	SC73_CHAMBER3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc73_palette[] = {
	0,
	1024,
	1284,
	2053,
	1799,
	2311,
	1801,
	2311,
	2825,
	2824,
	2824,
	2572,
	3082,
	2062,
	2830,
	2830,
	3852,
	3597,
	3853,
	3854,
	3087,
	3346,
	4622,
	3087,
	3856,
	3345,
	4625,
	4876,
	4878,
	3601,
	4880,
	5135,
	4624,
	4625,
	5907,
	4113,
	5902,
	5392,
	5138,
	5392,
	4631,
	4630,
	4633,
	5651,
	5397,
	4382,
	4370,
	6424,
	5654,
	5654,
	5656,
	6164,
	6939,
	5141,
	5657,
	4631,
	5659,
	5151,
	5397,
	6170,
	6172,
	6422,
	6681,
	6173,
	8986,
	5398,
	7955,
	6171,
	6685,
	7194,
	7965,
	5915,
	7955,
	8740,
	6426,
	6437,
	7959,
	6939,
	7202,
	7197,
	7711,
	8725,
	10275,
	6426,
	9493,
	7710,
	6944,
	9239,
	5664,
	9764,
	9242,
	10273,
	7197,
	10518,
	9500,
	7710,
	7211,
	8474,
	8481,
	10523,
	7709,
	8997,
	10521,
	11042,
	7455,
	10009,
	9001,
	9251,
	10268,
	11302,
	8225,
	8744,
	8227,
	7978,
	12059,
	7194,
	8493,
	8750,
	9501,
	9510,
	9766,
	7718,
	7216,
	11038,
	11816,
	8484,
	11297,
	11302,
	9254,
	12062,
	12324,
	9254,
	12833,
	11806,
	10024,
	11556,
	10284,
	10795,
	11813,
	8488,
	11312,
	12579,
	8999,
	7477,
	10546,
	9253,
	9012,
	12071,
	11566,
	11053,
	12580,
	13361,
	10282,
	13862,
	10783,
	10290,
	12584,
	12339,
	10287,
	13351,
	10796,
	11570,
	13864,
	13607,
	10797,
	12844,
	11057,
	9527,
	12849,
	13868,
	11567,
	13866,
	11570,
	11320,
	13617,
	12079,
	10553,
	14126,
	11568,
	13367,
	13875,
	12338,
	13113,
	15154,
	13616,
	13624,
	14898,
	13622,
	14140,
	15672,
	15161,
	15166,
	2825,
	2571,
	2568,
	2569,
	11,
	63,
	3596,
	2828,
	2829,
	2058,
	1802,
	2316,
	1540,
	4358,
	7,
	26,
	7936,
	0,
	35,
	9984,
	0,
	43,
	16128,
	0,
	5376,
	0,
	31,
	10752,
	0,
	52,
	15423,
	13,
	0,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
	9513,
	10535,
	10021,
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
	5653,
	4112,
	11573,
	12837,
	7462,
	7982,
	9748,
	3351,
	4128,
	6663,
	778,
	14649,
	12601,
	13105,
	10537,
	8749,
	10018,
	7196,
	5665,
	6934,
	2879,
	13835,
	1542,
	558,
	9730,
	0,
	11573,
	12837,
	7462,
	0,
	16128,
	0,
	16191,
	10559,
	10021,
	9513,
	10535,
	10021,
	9513,
	13863,
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

Compact sc73_chamber5 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	279,	// xcood
	190,	// ycood
	141*64,	// frame
	20569,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	(int16) 65533,	// mouseRel_y
	307-278,	// mouseSize_x
	238-188,	// mouseSize_y
	SC73_CHAMBER5_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC73_CHAMBER5_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc73_cham4_flash[] = {
	95*64,
	236,
	243,
	0,
	236,
	243,
	1,
	236,
	243,
	1,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	236,
	243,
	0,
	0
};

uint16 sc73_bits2_anim[] = {
	75*64,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	285,
	248,
	0,
	300,
	245,
	1,
	291,
	244,
	2,
	297,
	244,
	3,
	294,
	245,
	4,
	293,
	250,
	5,
	298,
	258,
	6,
	297,
	269,
	7,
	0
};

uint16 sc73_logic_list[] = {
	ID_FOSTER,
	ID_MEDI,
	ID_GALLAGHER,
	ID_KEN,
	ID_SC73_BIG_DOOR,
	ID_SC73_SENSOR,
	ID_SC73_CHAMBER3,
	ID_SC73_CHAMBER4,
	ID_SC73_CHAM4_LIGHT,
	ID_SC73_CHAMBER5,
	ID_SC73_CHAM5_LIGHT,
	ID_SC73_BITS,
	ID_SC73_BITS2,
	ID_SC73_SPRAY,
	ID_SC75_TONGS,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 reset_75_73[] = {
	C_SCREEN,
	73,
	C_PLACE,
	ID_SC73_FLOOR,
	C_XCOOD,
	408,
	C_YCOOD,
	296,
	65535
};

uint16 sc73_cham3_anim[] = {
	142*64,
	189,
	190,
	0,
	189,
	190,
	1,
	189,
	190,
	2,
	189,
	190,
	3,
	189,
	190,
	4,
	189,
	190,
	5,
	189,
	190,
	6,
	189,
	190,
	7,
	189,
	190,
	8,
	189,
	190,
	9,
	189,
	190,
	10,
	189,
	190,
	11,
	189,
	190,
	12,
	189,
	190,
	13,
	189,
	190,
	14,
	189,
	190,
	15,
	189,
	190,
	16,
	189,
	190,
	17,
	189,
	190,
	18,
	189,
	190,
	19,
	0
};

uint16 sc73_search[] = {
	44*64,
	293,
	252,
	0,
	293,
	252,
	1,
	293,
	252,
	2,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	3,
	293,
	252,
	2,
	293,
	252,
	1,
	293,
	252,
	0,
	0
};

Compact sc73_big_door = {
	0,	// logic
	ST_MOUSE+ST_SORT,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	384,	// xcood
	213,	// ycood
	138*64,	// frame
	181,	// cursorText
	SC73_BIG_DOOR_MOUSE_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65528,	// mouseRel_y
	398-380,	// mouseSize_x
	298-205,	// mouseSize_y
	SC73_BIG_DOOR_ACTION,	// actionScript
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

uint32 *gal = (uint32*)&gallagher;

Compact sc73_cham4_light = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	73,	// screen
	0,	// place
	0,	// getToTable
	236,	// xcood
	243,	// ycood
	95*64,	// frame
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
	SC73_CHAM4_LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
