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

#ifndef SKY24COMP_H
#define SKY24COMP_H




namespace Sky {

namespace SkyCompact {

uint16 give_mag_seq[] = {
	70*64,
	131,
	226,
	0,
	131,
	226,
	0,
	131,
	226,
	0,
	131,
	226,
	1,
	131,
	226,
	2,
	131,
	226,
	2,
	131,
	226,
	2,
	131,
	226,
	3,
	131,
	226,
	4,
	0
};

uint16 trev_read_mag[] = {
	64*64,
	351,
	239,
	0,
	351,
	239,
	0,
	351,
	239,
	0,
	351,
	239,
	0,
	351,
	239,
	1,
	351,
	239,
	1,
	351,
	239,
	1,
	351,
	239,
	1,
	351,
	239,
	0,
	0
};

uint16 s24_floor_table[] = {
	ID_S24_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S24_LEFT_ON,
	ID_LEFT_EXIT_24,
	GT_LEFT_EXIT_24,
	ID_LONDON_POSTER,
	GT_LONDON_POSTER,
	ID_NEW_YORK,
	GT_NEW_YORK,
	ID_MURAL,
	GT_MURAL,
	ID_PIDGEONS,
	GT_PIDGEONS,
	ID_TREVOR,
	GT_TREVOR,
	ID_TICKET,
	GT_TICKET,
	ID_GLOBE,
	GT_GLOBE,
	65535
};

Compact s24_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	s24_floor_table,	// getToTable
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
	4242,	// mouseSize_y
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

uint16 mag_away[] = {
	66*64,
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
	0
};

uint32 *grid24 = 0;

Compact left_exit_24 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	175,	// xcood
	247,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	196-175,	// mouseSize_x
	308-247,	// mouseSize_y
	EL24_ACTION,	// actionScript
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

Compact mural = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	244,	// xcood
	159,	// ycood
	0,	// frame
	12767,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	383-244,	// mouseSize_x
	238-159,	// mouseSize_y
	MURAL_ACTION,	// actionScript
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

uint16 trevor_relax[] = {
	61*64,
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
	0
};

uint16 rs_foster_23_24[] = {
	C_XCOOD,
	160,
	C_YCOOD,
	296,
	C_SCREEN,
	24,
	C_PLACE,
	ID_S24_FLOOR,
	65535
};

uint16 s24_mouse[] = {
	ID_JOEY,
	ID_LEFT_EXIT_24,
	ID_LONDON_POSTER,
	ID_NEW_YORK,
	ID_PIDGEONS,
	ID_MURAL,
	ID_TREVOR,
	ID_TICKET,
	ID_GLOBE,
	ID_S24_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 trevor_unrelax[] = {
	61*64,
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

Compact pidgeons = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	246,	// xcood
	232,	// ycood
	0,	// frame
	12772,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	291-246,	// mouseSize_x
	275-232,	// mouseSize_y
	PIDGEONS_ACTION,	// actionScript
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

uint16 trev_get_mag[] = {
	63*64,
	351,
	239,
	0,
	351,
	239,
	0,
	351,
	239,
	1,
	351,
	239,
	2,
	351,
	239,
	3,
	351,
	239,
	3,
	351,
	239,
	3,
	351,
	239,
	4,
	351,
	239,
	5,
	351,
	239,
	6,
	0
};

uint16 s24_chip_list[] = {
	136+DISK_8,
	137+DISK_8,
	61+DISK_8,
	62+DISK_8,
	63+DISK_8,
	64+DISK_8,
	65+DISK_8,
	66+DISK_8,
	67+DISK_8,
	68+DISK_8,
	69+DISK_8,
	70+DISK_8,
	72+DISK_8,
	74+DISK_8,
	71+DISK_8,
	75+DISK_8,
	29+DISK_8,
	30+DISK_8,
	31+DISK_8,
	0
};

uint16 rs_foster_24_23[] = {
	C_XCOOD,
	328,
	C_YCOOD,
	240,
	C_SCREEN,
	23,
	C_PLACE,
	ID_S23_FLOOR,
	65535
};

uint16 mag_back_seq[] = {
	69*64,
	253,
	226,
	0,
	253,
	226,
	1,
	253,
	226,
	2,
	253,
	226,
	3,
	253,
	226,
	3,
	253,
	226,
	3,
	253,
	226,
	3,
	253,
	226,
	4,
	253,
	226,
	5,
	253,
	226,
	6,
	253,
	226,
	6,
	253,
	226,
	7,
	0
};

uint16 push_globe[] = {
	75*64,
	265,
	193,
	0,
	265,
	193,
	1,
	265,
	193,
	2,
	265,
	193,
	2,
	265,
	193,
	1,
	265,
	193,
	0,
	0
};

uint16 s24_pal[] = {
	0,
	0,
	0,
	5,
	512,
	2,
	773,
	2560,
	1,
	512,
	1040,
	518,
	525,
	2049,
	1029,
	769,
	2069,
	1798,
	1291,
	4099,
	5,
	1540,
	2838,
	1288,
	2054,
	4882,
	772,
	2825,
	1796,
	2826,
	2314,
	1807,
	5897,
	2577,
	4098,
	2057,
	1306,
	5121,
	1288,
	3594,
	5638,
	264,
	3601,
	2821,
	3088,
	1566,
	6659,
	266,
	3346,
	3850,
	3342,
	2840,
	3590,
	2322,
	2082,
	5378,
	1807,
	3352,
	3338,
	4114,
	4108,
	7962,
	1290,
	3356,
	3844,
	5138,
	3616,
	6145,
	3089,
	5137,
	9232,
	1291,
	4371,
	7702,
	1808,
	5398,
	8713,
	1296,
	4880,
	4382,
	5909,
	4633,
	4625,
	4887,
	3622,
	6920,
	3348,
	4895,
	5386,
	4121,
	5654,
	8984,
	2067,
	5651,
	9759,
	2834,
	5911,
	10525,
	533,
	5919,
	5134,
	8474,
	5666,
	6161,
	6427,
	6429,
	9752,
	2327,
	5415,
	9230,
	3352,
	7191,
	8222,
	5147,
	6445,
	10498,
	3097,
	7448,
	6947,
	8220,
	6691,
	11031,
	4120,
	7454,
	11035,
	2075,
	7460,
	10258,
	3868,
	7713,
	6936,
	8736,
	6446,
	8979,
	7454,
	7211,
	12053,
	544,
	7976,
	11028,
	4127,
	8736,
	11810,
	3105,
	8233,
	12313,
	1570,
	7982,
	11028,
	5410,
	8999,
	8476,
	10020,
	8494,
	8984,
	8997,
	8753,
	11029,
	7716,
	9521,
	12559,
	6946,
	10275,
	10023,
	9512,
	9777,
	11801,
	7207,
	10030,
	9761,
	10794,
	9779,
	13085,
	5674,
	10801,
	12829,
	8745,
	11052,
	10536,
	11053,
	11572,
	12831,
	9261,
	12332,
	13100,
	10032,
	12592,
	12586,
	11827,
	12853,
	13355,
	12853,
	0,
	0,
	4098,
	1540,
	1538,
	4616,
	2825,
	1796,
	2826,
	2314,
	4367,
	1294,
	3599,
	6157,
	3089,
	4371,
	5654,
	2325,
	4880,
	7966,
	3607,
	7454,
	9499,
	9765,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
	15163,
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
	15167,
	15163,
	15163,
	15163,
	15163,
	15163,
	13883,
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

uint16 joey_list_s24[] = {
	224,
	344,
	280,
	320,
	1,
	293,
	361,
	264,
	294,
	0,
	0
};

uint16 mag_out[] = {
	66*64,
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

uint16 trev_turn_page[] = {
	65*64,
	351,
	239,
	0,
	351,
	239,
	1,
	351,
	239,
	2,
	351,
	239,
	2,
	351,
	239,
	2,
	351,
	239,
	1,
	351,
	239,
	0,
	0
};

ExtCompact trevor_ext = {
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
	SP_COL_TREVOR,	// spColour
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

Compact trevor = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_MOUSE+ST_BACKGROUND,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	351,	// xcood
	239,	// ycood
	62*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	16,	// mouseRel_x
	0,	// mouseRel_y
	297-271,	// mouseSize_x
	256-233,	// mouseSize_y
	TREVOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	TREVOR_LOGIC,	// baseSub
	0,	// baseSub_off
	&trevor_ext
};

Compact ticket = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	324,	// xcood
	244,	// ycood
	0,	// frame
	189,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	6,	// mouseSize_x
	5,	// mouseSize_y
	TICKET_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	TICKET_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact new_york_poster = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	162,	// xcood
	166,	// ycood
	0,	// frame
	12741,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	200-162,	// mouseSize_x
	245-166,	// mouseSize_y
	NEW_YORK_ACTION,	// actionScript
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

Compact london_poster = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	207,	// xcood
	166,	// ycood
	0,	// frame
	12741,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	237-207,	// mouseSize_x
	226-166,	// mouseSize_y
	LONDON_ACTION,	// actionScript
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

uint16 get_ticket[] = {
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
	1,
	1,
	4,
	1,
	1,
	4,
	1,
	1,
	4,
	SEND_SYNC,
	ID_TICKET,
	1,
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

uint16 s24_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_TREVOR,
	ID_TICKET,
	ID_GLOBE,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_LIFT_29,
	ID_GALLAGER_BEL,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 globe_spin[] = {
	71*64,
	274,
	275,
	0,
	274,
	275,
	1,
	274,
	275,
	2,
	274,
	275,
	3,
	274,
	275,
	4,
	274,
	275,
	5,
	274,
	275,
	6,
	274,
	275,
	7,
	274,
	275,
	8,
	274,
	275,
	9,
	274,
	275,
	10,
	274,
	275,
	11,
	274,
	275,
	12,
	274,
	275,
	13,
	274,
	275,
	14,
	274,
	275,
	15,
	274,
	275,
	16,
	274,
	275,
	17,
	274,
	275,
	0,
	274,
	275,
	0,
	0
};

uint16 ticket_seq[] = {
	74*64,
	352,
	254,
	0,
	352,
	254,
	1,
	352,
	254,
	2,
	352,
	254,
	3,
	352,
	254,
	4,
	352,
	254,
	5,
	352,
	254,
	6,
	0
};

Compact globe = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	24,	// screen
	0,	// place
	0,	// getToTable
	274,	// xcood
	275,	// ycood
	0,	// frame
	12769,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	311-274,	// mouseSize_x
	312-275,	// mouseSize_y
	GLOBE_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	GLOBE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 trevor_scratch[] = {
	62*64,
	351,
	239,
	0,
	351,
	239,
	1,
	351,
	239,
	2,
	351,
	239,
	3,
	351,
	239,
	4,
	351,
	239,
	3,
	351,
	239,
	4,
	351,
	239,
	3,
	351,
	239,
	4,
	351,
	239,
	3,
	351,
	239,
	4,
	351,
	239,
	2,
	351,
	239,
	1,
	351,
	239,
	0,
	0
};

uint16 s24_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
