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

#ifndef SKY67COMP_H
#define SKY67COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc67_logic_list[] = {
	ID_FOSTER,
	ID_MEDI,
	ID_SC67_DOOR,
	ID_SC68_DOOR,
	ID_SC67_PULSE1,
	ID_SC67_PULSE2,
	ID_SC67_PULSE3,
	ID_SC67_PULSE4,
	ID_SC67_ROCK,
	ID_SC67_BRICK,
	ID_SC67_PLASTER,
	ID_SC67_CLOT,
	ID_SC67_CROWBAR,
	ID_SC67_MEND,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

Compact sc67_vein = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	351,	// xcood
	204,	// ycood
	0,	// frame
	20501,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	361-351,	// mouseSize_x
	234-204,	// mouseSize_y
	SC67_VEIN_ACTION,	// actionScript
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

uint16 sc67_mouse_list[] = {
	ID_MEDI,
	ID_SC67_PLASTER,
	ID_SC67_BRICK,
	ID_SC67_BRICKWORK,
	ID_SC67_CLOT,
	ID_SC67_VEIN,
	ID_SC67_CROWBAR,
	ID_SC67_DOOR,
	ID_SC67_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc67_rub_head[] = {
	143*64,
	253,
	173,
	0,
	253,
	173,
	1,
	253,
	173,
	2,
	253,
	173,
	3,
	253,
	173,
	4,
	253,
	173,
	4,
	253,
	173,
	5,
	253,
	173,
	5,
	253,
	173,
	4,
	253,
	173,
	4,
	253,
	173,
	5,
	253,
	173,
	5,
	253,
	173,
	4,
	253,
	173,
	4,
	253,
	173,
	4,
	253,
	173,
	3,
	253,
	173,
	6,
	0
};

uint16 sc67_try_stick[] = {
	131*64,
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
	6,
	1,
	1,
	7,
	1,
	1,
	8,
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

uint16 sc67_plast_hit[] = {
	134*64,
	322,
	220,
	0,
	322,
	220,
	1,
	322,
	220,
	2,
	322,
	220,
	3,
	322,
	220,
	4,
	322,
	220,
	5,
	322,
	220,
	6,
	322,
	220,
	6,
	322,
	220,
	7,
	322,
	220,
	8,
	322,
	220,
	9,
	322,
	220,
	10,
	322,
	220,
	11,
	322,
	220,
	12,
	322,
	220,
	13,
	322,
	220,
	14,
	0
};

Compact sc67_door = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	378,	// xcood
	213,	// ycood
	30*64,	// frame
	181,	// cursorText
	SC67_DOOR_MOUSE_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65531,	// mouseRel_x
	(int16) 65530,	// mouseRel_y
	25,	// mouseSize_x
	86,	// mouseSize_y
	SC67_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC67_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc67_mend = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	328,	// xcood
	243,	// ycood
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
	SC67_MEND_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc67_mending[] = {
	151*64,
	328,
	243,
	0,
	328,
	243,
	1,
	328,
	243,
	2,
	328,
	243,
	3,
	328,
	243,
	4,
	328,
	243,
	5,
	328,
	243,
	6,
	328,
	243,
	7,
	328,
	243,
	8,
	328,
	243,
	9,
	328,
	243,
	10,
	328,
	243,
	11,
	328,
	243,
	12,
	0
};

Compact sc67_plaster = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	319,	// xcood
	209,	// ycood
	56*64,	// frame
	20506,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	4,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	SC67_PLASTER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC67_PLASTER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc67_door_open[] = {
	30*64,
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

uint16 sc67_dustoff[] = {
	47*64,
	241,
	177,
	0,
	241,
	177,
	1,
	241,
	177,
	2,
	241,
	177,
	3,
	241,
	177,
	4,
	241,
	177,
	5,
	241,
	177,
	2,
	241,
	177,
	6,
	241,
	177,
	7,
	241,
	177,
	8,
	241,
	177,
	9,
	241,
	177,
	10,
	241,
	177,
	11,
	241,
	177,
	12,
	0
};

Compact sc67_rock = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	67,	// screen
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
	SC67_ROCK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc67_crowbar = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	325,	// xcood
	240,	// ycood
	152*64,	// frame
	5,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	12,	// mouseSize_x
	2,	// mouseSize_y
	SC67_CROWBAR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC67_CROWBAR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc67_puss_leak[] = {
	149*64,
	325,
	247,
	0,
	325,
	247,
	1,
	325,
	247,
	2,
	325,
	247,
	3,
	325,
	247,
	4,
	325,
	247,
	5,
	325,
	247,
	6,
	325,
	247,
	7,
	325,
	247,
	8,
	325,
	247,
	9,
	325,
	247,
	10,
	325,
	247,
	11,
	0
};

uint16 sc67_pulse3_anim[] = {
	28*64,
	346,
	228,
	0,
	346,
	228,
	1,
	346,
	228,
	1,
	346,
	228,
	0,
	346,
	228,
	0,
	346,
	228,
	0,
	346,
	228,
	0,
	346,
	228,
	1,
	346,
	228,
	1,
	346,
	228,
	1,
	346,
	228,
	1,
	346,
	228,
	1,
	346,
	228,
	0,
	346,
	228,
	2,
	346,
	228,
	2,
	346,
	228,
	2,
	346,
	228,
	2,
	0
};

uint16 sc67_plast_fall[] = {
	56*64,
	319,
	209,
	0,
	319,
	208,
	1,
	318,
	210,
	2,
	318,
	212,
	3,
	318,
	216,
	4,
	318,
	227,
	5,
	318,
	242,
	6,
	318,
	258,
	7,
	318,
	275,
	8,
	316,
	271,
	9,
	313,
	275,
	10,
	312,
	275,
	10,
	0
};

Compact sc67_pulse4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	334,	// xcood
	242,	// ycood
	29*64,	// frame
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
	SC67_PULSE4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc67_brick_fall[] = {
	55*64,
	327,
	209,
	0,
	328,
	209,
	1,
	327,
	209,
	0,
	327,
	209,
	0,
	327,
	209,
	0,
	328,
	209,
	1,
	328,
	209,
	2,
	326,
	209,
	3,
	325,
	209,
	4,
	324,
	213,
	5,
	325,
	217,
	6,
	320,
	217,
	7,
	317,
	224,
	8,
	317,
	232,
	9,
	317,
	242,
	10,
	311,
	256,
	11,
	311,
	266,
	11,
	293,
	272,
	11,
	291,
	272,
	12,
	290,
	272,
	13,
	289,
	272,
	14,
	0
};

uint16 sc67_bar_fall[] = {
	152*64,
	325,
	240,
	0,
	324,
	246,
	1,
	325,
	249,
	2,
	326,
	255,
	3,
	326,
	261,
	4,
	327,
	262,
	5,
	330,
	266,
	6,
	331,
	272,
	7,
	0
};

uint16 sc67_brick_hit[] = {
	133*64,
	322,
	220,
	0,
	322,
	220,
	1,
	322,
	220,
	2,
	322,
	220,
	3,
	322,
	220,
	4,
	322,
	220,
	5,
	322,
	220,
	6,
	322,
	220,
	6,
	322,
	220,
	7,
	322,
	220,
	8,
	322,
	220,
	9,
	322,
	220,
	10,
	322,
	220,
	0,
	322,
	220,
	0,
	322,
	220,
	0,
	322,
	220,
	0,
	0
};

uint16 sc67_pick_plast[] = {
	130*64,
	316,
	224,
	0,
	316,
	224,
	1,
	316,
	224,
	2,
	316,
	224,
	3,
	316,
	224,
	3,
	316,
	224,
	3,
	316,
	224,
	3,
	316,
	224,
	3,
	316,
	224,
	4,
	316,
	224,
	5,
	0
};

uint16 sc67_crawl[] = {
	46*64,
	157,
	275,
	0,
	158,
	275,
	1,
	158,
	275,
	2,
	168,
	275,
	3,
	172,
	275,
	4,
	179,
	275,
	5,
	179,
	275,
	2,
	188,
	274,
	6,
	193,
	275,
	7,
	203,
	275,
	8,
	204,
	272,
	9,
	204,
	267,
	10,
	201,
	266,
	11,
	201,
	264,
	12,
	202,
	255,
	13,
	202,
	247,
	14,
	202,
	243,
	15,
	0
};

uint16 sc67_medifix[] = {
	150*64,
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

uint16 sc67_pulse1_anim[] = {
	26*64,
	352,
	205,
	0,
	352,
	205,
	1,
	352,
	205,
	1,
	352,
	205,
	1,
	352,
	205,
	1,
	352,
	205,
	0,
	352,
	205,
	0,
	352,
	205,
	0,
	352,
	205,
	0,
	352,
	205,
	0,
	352,
	205,
	1,
	352,
	205,
	2,
	352,
	205,
	2,
	352,
	205,
	2,
	352,
	205,
	2,
	352,
	205,
	1,
	352,
	205,
	0,
	0
};

uint16 sc67_pulse2_anim[] = {
	27*64,
	354,
	218,
	0,
	354,
	218,
	0,
	354,
	218,
	1,
	354,
	218,
	1,
	354,
	218,
	1,
	354,
	218,
	1,
	354,
	218,
	0,
	354,
	218,
	0,
	354,
	218,
	0,
	354,
	218,
	0,
	354,
	218,
	0,
	354,
	218,
	2,
	354,
	218,
	3,
	354,
	218,
	3,
	354,
	218,
	3,
	354,
	218,
	3,
	354,
	218,
	1,
	0
};

uint16 sc67_stick_in[] = {
	131*64,
	322,
	220,
	0,
	322,
	220,
	1,
	322,
	220,
	2,
	322,
	220,
	3,
	322,
	220,
	4,
	322,
	220,
	5,
	322,
	220,
	6,
	322,
	220,
	6,
	322,
	220,
	7,
	322,
	220,
	8,
	322,
	220,
	9,
	322,
	220,
	10,
	322,
	220,
	11,
	322,
	220,
	12,
	322,
	220,
	13,
	322,
	220,
	14,
	0
};

Compact sc67_pulse1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	352,	// xcood
	205,	// ycood
	26*64,	// frame
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
	SC67_PULSE1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc67_rpocket[] = {
	142*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

Compact sc67_brickwork = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	320,	// xcood
	208,	// ycood
	0,	// frame
	20506,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	18,	// mouseSize_x
	10,	// mouseSize_y
	SC67_BRICKWORK_ACTION,	// actionScript
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

uint16 sc67_getbrick[] = {
	48*64,
	312,
	208,
	0,
	312,
	208,
	1,
	312,
	208,
	2,
	312,
	208,
	3,
	312,
	208,
	4,
	312,
	208,
	4,
	312,
	208,
	5,
	312,
	208,
	5,
	312,
	208,
	4,
	312,
	208,
	4,
	312,
	208,
	5,
	312,
	208,
	5,
	312,
	208,
	4,
	312,
	208,
	4,
	312,
	208,
	3,
	312,
	208,
	6,
	312,
	208,
	7,
	312,
	208,
	8,
	0
};

uint16 sc67_lpocket[] = {
	141*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 sc67_pull_out[] = {
	132*64,
	322,
	220,
	0,
	322,
	220,
	1,
	322,
	220,
	2,
	322,
	220,
	3,
	322,
	220,
	4,
	322,
	220,
	5,
	322,
	220,
	6,
	322,
	220,
	7,
	322,
	220,
	8,
	322,
	220,
	9,
	322,
	220,
	10,
	322,
	220,
	11,
	322,
	220,
	12,
	322,
	220,
	13,
	322,
	220,
	14,
	322,
	220,
	14,
	0
};

uint16 sc67_palette[] = {
	0,
	512,
	1027,
	771,
	1026,
	770,
	772,
	772,
	1028,
	1029,
	771,
	1285,
	1031,
	1284,
	1541,
	1542,
	2052,
	1541,
	1035,
	2308,
	1029,
	1798,
	2054,
	1287,
	1799,
	2824,
	1541,
	1293,
	2309,
	1799,
	2311,
	2567,
	1288,
	1804,
	3333,
	1798,
	2312,
	2569,
	2312,
	2059,
	3847,
	1542,
	2313,
	2315,
	1802,
	2317,
	3845,
	1288,
	2061,
	2824,
	2058,
	2825,
	4361,
	1543,
	1555,
	3334,
	1802,
	2317,
	3850,
	2312,
	2571,
	3851,
	1801,
	2065,
	3336,
	2315,
	3083,
	4873,
	1800,
	3084,
	3595,
	3082,
	2575,
	3850,
	2059,
	2828,
	4365,
	2569,
	2577,
	5382,
	2056,
	2323,
	3593,
	2061,
	3596,
	4361,
	1804,
	2833,
	4873,
	1547,
	3087,
	5899,
	1800,
	3341,
	4621,
	2827,
	3597,
	4107,
	3340,
	2580,
	5643,
	1802,
	2836,
	3848,
	2574,
	3342,
	4623,
	2573,
	3600,
	5132,
	2572,
	3854,
	3853,
	2832,
	3346,
	4109,
	3854,
	3093,
	6156,
	2571,
	3859,
	4618,
	3087,
	4112,
	7181,
	2314,
	3602,
	5136,
	3086,
	3856,
	6673,
	2316,
	3859,
	5902,
	3085,
	2336,
	4615,
	4623,
	4369,
	4367,
	3346,
	4115,
	4880,
	3345,
	3101,
	4617,
	4881,
	2595,
	4616,
	4115,
	4372,
	7956,
	2829,
	5139,
	6671,
	3600,
	4124,
	5133,
	4117,
	4632,
	7952,
	3343,
	3617,
	6668,
	3858,
	4885,
	5398,
	4629,
	3621,
	9996,
	2829,
	5910,
	8977,
	3857,
	4894,
	5906,
	4888,
	3880,
	6413,
	5144,
	5153,
	9746,
	4114,
	6681,
	10259,
	4371,
	6683,
	9242,
	5143,
	7196,
	11035,
	5398,
	0,
	768,
	1285,
	1288,
	1542,
	1543,
	1799,
	2568,
	2312,
	2571,
	3083,
	2828,
	2574,
	3852,
	2828,
	3346,
	4621,
	4110,
	3856,
	4881,
	3599,
	4372,
	5396,
	5651,
	0,
	768,
	515,
	516,
	771,
	1285,
	1798,
	1798,
	2055,
	2058,
	2825,
	2826,
	3084,
	3851,
	2828,
	3346,
	4621,
	4110,
	3856,
	4881,
	3599,
	4372,
	5396,
	5651,
	0,
	4352,
	1543,
	2069,
	5896,
	1800,
	2834,
	7179,
	2314,
	3101,
	8969,
	2058,
	3359,
	7179,
	3344,
	3871,
	9485,
	3086,
	4387,
	9743,
	4114,
	4904,
	11025,
	5398,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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
	1045,
	1030,
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
	14347,
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

uint32 *grid67 = 0;

uint16 sc67_fast_list[] = {
	12+DISK_5,
	51+DISK_5,
	52+DISK_5,
	53+DISK_5,
	54+DISK_5,
	268+DISK_5,
	0
};

uint16 sc67_door_close[] = {
	30*64,
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

uint16 sc67_rock_anim[] = {
	31*64,
	151,
	259,
	0,
	151,
	259,
	1,
	151,
	259,
	2,
	151,
	259,
	3,
	151,
	259,
	4,
	151,
	259,
	5,
	0
};

Compact sc67_pulse2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	354,	// xcood
	218,	// ycood
	27*64,	// frame
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
	SC67_PULSE2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc67_brick = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	327,	// xcood
	209,	// ycood
	55*64,	// frame
	20509,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	3,	// mouseSize_y
	SC67_BRICK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC67_BRICK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc67_floor_table[] = {
	ID_SC67_FLOOR,
	RET_OK,
	ID_SC67_BRICKWORK,
	GT_SC67_BRICKWORK,
	ID_SC67_DOOR,
	GT_SC67_DOOR,
	ID_SC67_CLOT,
	GT_SC67_CLOT,
	ID_SC67_VEIN,
	GT_SC67_VEIN,
	ID_SC67_PLASTER,
	GT_SC67_PLASTER,
	ID_SC67_BRICK,
	GT_SC67_BRICK,
	ID_SC67_CROWBAR,
	GT_SC67_CROWBAR,
	0,
	SC67_DOOR_WALK_ON,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	65535
};

uint16 sc67_pick_brick[] = {
	129*64,
	269,
	224,
	0,
	269,
	224,
	1,
	269,
	224,
	2,
	269,
	224,
	3,
	269,
	224,
	3,
	269,
	224,
	3,
	269,
	224,
	3,
	269,
	224,
	3,
	269,
	224,
	4,
	269,
	224,
	5,
	0
};

uint16 reset_66_67[] = {
	C_SCREEN,
	67,
	C_PLACE,
	ID_SC67_FLOOR,
	65535
};

Compact sc67_pulse3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	346,	// xcood
	228,	// ycood
	28*64,	// frame
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
	SC67_PULSE3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc67_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	sc67_floor_table,	// getToTable
	190,	// xcood
	270,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	390-190,	// mouseSize_x
	305-270,	// mouseSize_y
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

uint16 sc67_pulse4_anim[] = {
	29*64,
	334,
	242,
	0,
	334,
	242,
	1,
	334,
	242,
	2,
	334,
	242,
	2,
	334,
	242,
	1,
	334,
	242,
	1,
	334,
	242,
	1,
	334,
	242,
	1,
	334,
	242,
	2,
	334,
	242,
	2,
	334,
	242,
	2,
	334,
	242,
	2,
	334,
	242,
	2,
	334,
	242,
	1,
	334,
	242,
	0,
	334,
	242,
	0,
	334,
	242,
	0,
	0
};

uint16 sc67_chip_list[] = {
	IT_SC67_LAYER_0+DISK_5,
	IT_SC67_LAYER_1+DISK_5,
	IT_SC67_GRID_1+DISK_5,
	IT_MEDI+DISK_5,
	IT_SC67_CRAWL+0X8000+DISK_5,
	IT_SC67_DUSTOFF+0X8000+DISK_5,
	IT_SC67_PULSE1+DISK_5,
	IT_SC67_PULSE2+DISK_5,
	IT_SC67_PULSE3+DISK_5,
	IT_SC67_PULSE4+DISK_5,
	IT_SC67_ROCK+DISK_5,
	IT_SC67_DOOR+DISK_5,
	IT_SC67_BRICK+DISK_5,
	IT_SC67_PLASTER+DISK_5,
	IT_SC67_CROWBAR+DISK_5,
	IT_SC67_PUSS+DISK_5,
	IT_SC67_MEDIFIX+DISK_5,
	IT_SC67_MENDING+DISK_5,
	IT_SC67_PICK_PLAST+DISK_5,
	IT_SC67_PICK_BRICK+DISK_5,
	IT_SC67_LPOCKET+DISK_5,
	IT_SC67_RPOCKET+DISK_5,
	IT_SC67_RUB_HEAD+DISK_5,
	IT_SC67_GETBRICK+0X8000+DISK_5,
	IT_SC67_STICK_IN+0X8000+DISK_5,
	IT_SC67_PULL_OUT+0X8000+DISK_5,
	IT_SC67_BRICK_HIT+0X8000+DISK_5,
	IT_SC67_PLAST_HIT+0X8000+DISK_5,
	106+DISK_5,
	0
};

uint16 reset_68_67[] = {
	C_SCREEN,
	67,
	C_PLACE,
	ID_SC67_FLOOR,
	C_XCOOD,
	408,
	C_YCOOD,
	288,
	65535
};

Compact sc67_clot = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	67,	// screen
	0,	// place
	0,	// getToTable
	325,	// xcood
	247,	// ycood
	149*64,	// frame
	20502,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	(int16) 65530,	// mouseRel_y
	332-323,	// mouseSize_x
	248-241,	// mouseSize_y
	SC67_CLOT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC67_CLOT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
