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

#ifndef SKY23COMP_H
#define SKY23COMP_H




namespace Sky {

namespace SkyCompact {

uint32 *grid23 = 0;

Compact link_23 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	191,	// xcood
	236,	// ycood
	0,	// frame
	51,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	212-191,	// mouseSize_x
	276-236,	// mouseSize_y
	LINK_23_ACTION,	// actionScript
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

uint16 s23_chip_list[] = {
	136+DISK_8,
	137+DISK_8,
	21+DISK_8,
	22+DISK_8,
	24+DISK_8,
	23+DISK_8,
	25+DISK_8,
	0
};

Compact sculpture = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	274,	// xcood
	235,	// ycood
	0,	// frame
	12755,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	309-274,	// mouseSize_x
	278-235,	// mouseSize_y
	SCULPTURE_ACTION,	// actionScript
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

Compact wreck_23 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	322,	// xcood
	310,	// ycood
	0,	// frame
	12763,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	4242,	// mouseSize_x
	4242,	// mouseSize_y
	WRECK_23_ACTION,	// actionScript
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

Compact anchor_exit_23 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	393,	// xcood
	219,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	12,	// mouseSize_x
	299-219,	// mouseSize_y
	ANCHOR23_ACTION,	// actionScript
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

Compact bin_23 = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	127,	// xcood
	300,	// ycood
	0,	// frame
	12757,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	155-127,	// mouseSize_x
	4242,	// mouseSize_y
	BIN_23_ACTION,	// actionScript
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

uint16 s23_pal[] = {
	0,
	768,
	0,
	514,
	1538,
	1,
	517,
	1794,
	771,
	778,
	1281,
	2053,
	1288,
	3590,
	260,
	1291,
	2564,
	2055,
	1297,
	3587,
	1031,
	2313,
	2313,
	3081,
	1806,
	5383,
	518,
	2320,
	2566,
	3851,
	2829,
	5645,
	1033,
	2831,
	4363,
	2315,
	2835,
	5382,
	2061,
	3854,
	4879,
	2829,
	3598,
	6930,
	1035,
	3857,
	6417,
	2062,
	3607,
	9483,
	1033,
	3612,
	5894,
	3600,
	4372,
	8721,
	1291,
	4624,
	6423,
	2577,
	4123,
	5133,
	5140,
	4887,
	7952,
	2065,
	3874,
	7177,
	2323,
	5144,
	11284,
	1291,
	4639,
	10764,
	1805,
	5402,
	5648,
	5909,
	5651,
	7450,
	3859,
	5148,
	9491,
	2577,
	4898,
	8202,
	3605,
	5917,
	11536,
	2063,
	5156,
	9997,
	2067,
	5914,
	7189,
	6168,
	6425,
	9499,
	2583,
	6432,
	6160,
	7962,
	4655,
	8713,
	5144,
	5673,
	9738,
	3607,
	6181,
	10258,
	2074,
	6692,
	7695,
	6682,
	6944,
	10773,
	3352,
	5424,
	8715,
	6426,
	7451,
	9760,
	4380,
	7201,
	8988,
	5405,
	6701,
	11275,
	3866,
	7208,
	9748,
	6172,
	6194,
	12557,
	4633,
	7726,
	9223,
	7454,
	6960,
	11021,
	4637,
	7976,
	11794,
	3869,
	7467,
	9750,
	6944,
	8489,
	12821,
	2079,
	8239,
	11281,
	5921,
	8744,
	12573,
	3618,
	8242,
	9747,
	8228,
	9006,
	11796,
	6180,
	9523,
	11020,
	6949,
	9009,
	10518,
	8742,
	9266,
	13082,
	5158,
	10032,
	10775,
	9512,
	9781,
	13079,
	6440,
	10543,
	11552,
	9770,
	10804,
	11548,
	10540,
	11318,
	13086,
	8749,
	11824,
	12841,
	11056,
	12343,
	13350,
	11570,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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

uint16 s23_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_TREVOR,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_LIFT_29,
	ID_GALLAGER_BEL,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 s23_mouse[] = {
	ID_JOEY,
	ID_LEFT_EXIT_23,
	ID_SMALL_23,
	ID_ANCHOR_EXIT_23,
	ID_TRAVEL_EXIT_23,
	ID_BIN_23,
	ID_SCULPTURE,
	ID_LINK_23,
	ID_WRECK_23,
	ID_S23_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact small_exit_23 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	217,	// xcood
	190,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	227-217,	// mouseSize_x
	250-190,	// mouseSize_y
	SML_EXIT_S23_ACTION,	// actionScript
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

Compact travel_exit_23 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	315,	// xcood
	189,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	342-315,	// mouseSize_x
	246-189,	// mouseSize_y
	TRAVEL_23_ACTION,	// actionScript
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

uint16 s23_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	0
};

Compact left_exit_23 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	0,	// getToTable
	150,	// xcood
	226,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	288-226,	// mouseSize_y
	EL23_ACTION,	// actionScript
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

uint16 s23_floor_table[] = {
	ID_S23_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S23_LEFT_ON,
	1,
	S23_ANCHOR_ON,
	3,
	S23_TRAVEL_ON,
	ID_LEFT_EXIT_23,
	GT_LEFT_EXIT_23,
	ID_ANCHOR_EXIT_23,
	GT_ANCHOR_EXIT_23,
	ID_TRAVEL_EXIT_23,
	GT_TRAVEL_EXIT_23,
	ID_BIN_23,
	GT_BIN_23,
	ID_SCULPTURE,
	GT_SCULPTURE,
	ID_LINK_23,
	GT_LINK_23,
	ID_WRECK_23,
	GT_WRECK_23,
	ID_SMALL_23,
	GT_SMALL_23,
	65535
};

Compact s23_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	23,	// screen
	0,	// place
	s23_floor_table,	// getToTable
	128,	// xcood
	240,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	1000,	// mouseSize_x
	1000,	// mouseSize_y
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

uint16 rs_foster_29_23[] = {
	C_XCOOD,
	136,
	C_YCOOD,
	280,
	C_SCREEN,
	23,
	C_PLACE,
	ID_S23_FLOOR,
	65535
};

} // namespace SkyCompact

} // namespace Sky

#endif
