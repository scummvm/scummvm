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

#ifndef SKY22COMP_H
#define SKY22COMP_H




namespace Sky {

namespace SkyCompact {

uint16 fost_replace_pill[] = {
	197*64,
	320,
	226,
	3,
	320,
	226,
	3,
	320,
	226,
	2,
	320,
	226,
	2,
	320,
	226,
	1,
	320,
	226,
	1,
	320,
	226,
	0,
	320,
	226,
	0,
	0
};

uint16 rs_foster_20_22[] = {
	C_XCOOD,
	424,
	C_YCOOD,
	304,
	C_SCREEN,
	22,
	C_PLACE,
	ID_S22_FLOOR,
	65535
};

Compact reich_picture = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	259,	// xcood
	175,	// ycood
	0,	// frame
	12744,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	326-259,	// mouseSize_x
	210-175,	// mouseSize_y
	REICH_PICTURE_ACTION,	// actionScript
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

Compact inner_reich_door = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	393,	// xcood
	249,	// ycood
	85*64,	// frame
	181,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	403-393,	// mouseSize_x
	313-249,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	INNER_R_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_22_20[] = {
	C_XCOOD,
	216,
	C_YCOOD,
	280,
	C_SCREEN,
	20,
	C_PLACE,
	ID_S20_FLOOR,
	65535
};

uint16 fost_remove_pill[] = {
	197*64,
	320,
	226,
	0,
	320,
	226,
	0,
	320,
	226,
	0,
	320,
	226,
	0,
	320,
	226,
	0,
	320,
	226,
	0,
	320,
	226,
	1,
	320,
	226,
	1,
	320,
	226,
	2,
	320,
	226,
	2,
	320,
	226,
	3,
	320,
	226,
	3,
	320,
	226,
	4,
	320,
	226,
	4,
	0
};

uint16 inner_reich_open[] = {
	85*64,
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
	7,
	1,
	1,
	8,
	1,
	1,
	9,
	1,
	1,
	10,
	1,
	1,
	11,
	1,
	1,
	12,
	0
};

uint16 rs_magazine[] = {
	C_SCREEN,
	424,
	65535
};

uint32 *grid22 = 0;

uint16 remove_pillow[] = {
	196*64,
	1,
	1,
	0,
	1,
	1,
	0,
	1,
	1,
	0,
	1,
	1,
	0,
	1,
	1,
	0,
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
	2,
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
	4,
	1,
	1,
	4,
	0
};

uint16 s22_floor_table[] = {
	ID_S22_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S22_START_ON,
	ID_RIGHT_EXIT_22,
	GT_RIGHT_EXIT_22,
	ID_LAMB_BED,
	GT_LAMB_BED,
	ID_LAMB_TV,
	GT_LAMB_TV,
	ID_FISH_TANK,
	GT_FISH_TANK,
	ID_FISH_POSTER,
	GT_FISH_TANK,
	ID_PILLOW,
	GT_PILLOW,
	ID_MAGAZINE,
	GT_MAGAZINE,
	ID_REICH_CHAIR,
	GT_REICH_CHAIR,
	ID_CABINET,
	GT_CABINET,
	ID_CERT,
	GT_CERT,
	ID_REICH_PICTURE,
	GT_REICH_PICTURE,
	ID_FISH_FOOD,
	GT_FISH_FOOD,
	65535
};

Compact pillow = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_SORT+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	324,	// xcood
	244,	// ycood
	196*64,	// frame
	12734,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	7,	// mouseRel_x
	0,	// mouseRel_y
	26,	// mouseSize_x
	16,	// mouseSize_y
	PILLOW_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	PILLOW_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 inner_reich_close[] = {
	85*64,
	1,
	1,
	11,
	1,
	1,
	10,
	1,
	1,
	9,
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

Compact lamb_bed = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	283,	// xcood
	251,	// ycood
	0,	// frame
	12731,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	365-283,	// mouseSize_x
	275-251,	// mouseSize_y
	BED_ACTION,	// actionScript
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

uint16 replace_pillow[] = {
	196*64,
	1,
	1,
	4,
	1,
	1,
	4,
	1,
	1,
	4,
	1,
	1,
	4,
	1,
	1,
	4,
	1,
	1,
	2,
	1,
	1,
	2,
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

uint16 get_magazine_seq[] = {
	200*64,
	273,
	218,
	0,
	273,
	218,
	1,
	273,
	218,
	2,
	273,
	218,
	3,
	273,
	218,
	3,
	273,
	218,
	3,
	273,
	218,
	2,
	273,
	218,
	1,
	273,
	218,
	0,
	0
};

uint16 tank3_seq[] = {
	92*64,
	245,
	238,
	0,
	245,
	238,
	1,
	245,
	238,
	2,
	245,
	238,
	3,
	245,
	238,
	4,
	245,
	238,
	5,
	245,
	238,
	6,
	245,
	238,
	7,
	245,
	238,
	8,
	245,
	238,
	9,
	245,
	238,
	10,
	245,
	238,
	11,
	245,
	238,
	12,
	245,
	238,
	13,
	245,
	238,
	14,
	245,
	238,
	15,
	245,
	238,
	16,
	245,
	238,
	17,
	245,
	238,
	18,
	245,
	238,
	19,
	0
};

uint16 tank2_seq[] = {
	91*64,
	245,
	238,
	0,
	245,
	238,
	1,
	245,
	238,
	2,
	245,
	238,
	3,
	245,
	238,
	4,
	245,
	238,
	5,
	245,
	238,
	6,
	245,
	238,
	7,
	245,
	238,
	8,
	245,
	238,
	4,
	245,
	238,
	9,
	245,
	238,
	6,
	245,
	238,
	7,
	245,
	238,
	10,
	245,
	238,
	11,
	245,
	238,
	12,
	245,
	238,
	13,
	245,
	238,
	14,
	245,
	238,
	15,
	245,
	238,
	16,
	245,
	238,
	17,
	245,
	238,
	18,
	245,
	238,
	19,
	245,
	238,
	20,
	245,
	238,
	21,
	245,
	238,
	22,
	245,
	238,
	23,
	245,
	238,
	24,
	245,
	238,
	25,
	245,
	238,
	26,
	0
};

Compact s22_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	s22_floor_table,	// getToTable
	128,	// xcood
	240,	// ycood
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

Compact right_exit_22 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	393,	// xcood
	249,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	402-393,	// mouseSize_x
	313-249,	// mouseSize_y
	ER22_ACTION,	// actionScript
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

Compact reich_chair = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	221,	// xcood
	287,	// ycood
	0,	// frame
	12752,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	240-221,	// mouseSize_x
	314-287,	// mouseSize_y
	REICH_CHAIR_ACTION,	// actionScript
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

uint16 s22_mouse[] = {
	ID_JOEY,
	ID_INNER_R_DOOR,
	ID_RIGHT_EXIT_22,
	ID_MAGAZINE,
	ID_PILLOW,
	ID_LAMB_BED,
	ID_LAMB_TV,
	ID_FISH_TANK,
	ID_FISH_POSTER,
	ID_REICH_CHAIR,
	ID_CABINET,
	ID_CERT,
	ID_REICH_PICTURE,
	ID_FISH_FOOD,
	ID_S22_FLOOR,
	ID_FAKE_FLOOR_22,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact fish_tank = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	245,	// xcood
	238,	// ycood
	199*64,	// frame
	12738,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	(int16) 65533,	// mouseRel_y
	264-242,	// mouseSize_x
	258-235,	// mouseSize_y
	FISH_TANK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	FISH_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact fake_floor_22 = {
	0,	// logic
	0,	// status
	0,	// sync
	22,	// screen
	0,	// place
	s22_floor_table,	// getToTable
	128,	// xcood
	136,	// ycood
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

uint16 s22_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	187+DISK_9,
	188+DISK_9,
	189+DISK_9,
	190+DISK_9,
	191+DISK_9,
	0
};

uint16 s22_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_FISH_TANK,
	ID_MAGAZINE,
	ID_PILLOW,
	ID_INNER_R_DOOR,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_GALLAGER_BEL,
	ID_LIFT_29,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 s22_chip_list[] = {
	196+DISK_9,
	197+DISK_9,
	198+DISK_9,
	199+DISK_9,
	91+DISK_9,
	92+DISK_9,
	200+DISK_9,
	85+DISK_9,
	0
};

Compact magazine = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	340,	// xcood
	249,	// ycood
	198*64,	// frame
	163,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	26,	// mouseSize_x
	16,	// mouseSize_y
	MAGAZINE_ACTION,	// actionScript
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

uint16 fish_seq[] = {
	199*64,
	245,
	238,
	0,
	245,
	238,
	1,
	245,
	238,
	2,
	245,
	238,
	3,
	245,
	238,
	4,
	245,
	238,
	5,
	245,
	238,
	6,
	245,
	238,
	7,
	245,
	238,
	8,
	245,
	238,
	9,
	245,
	238,
	10,
	245,
	238,
	11,
	245,
	238,
	12,
	245,
	238,
	13,
	245,
	238,
	14,
	245,
	238,
	15,
	245,
	238,
	16,
	245,
	238,
	17,
	245,
	238,
	18,
	245,
	238,
	19,
	245,
	238,
	20,
	245,
	238,
	21,
	245,
	238,
	22,
	245,
	238,
	23,
	245,
	238,
	24,
	245,
	238,
	25,
	245,
	238,
	26,
	245,
	238,
	27,
	245,
	238,
	28,
	245,
	238,
	29,
	245,
	238,
	30,
	245,
	238,
	31,
	245,
	238,
	32,
	245,
	238,
	33,
	245,
	238,
	34,
	245,
	238,
	35,
	245,
	238,
	36,
	245,
	238,
	37,
	245,
	238,
	38,
	245,
	238,
	39,
	0
};

Compact cert = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	310,	// xcood
	215,	// ycood
	0,	// frame
	12746,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	325-310,	// mouseSize_x
	224-215,	// mouseSize_y
	CERT_ACTION,	// actionScript
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

uint16 s22_pal[] = {
	0,
	0,
	0,
	7,
	768,
	0,
	9,
	1024,
	259,
	266,
	2048,
	514,
	526,
	2560,
	4,
	1032,
	1540,
	1030,
	1037,
	4608,
	257,
	1292,
	4353,
	4,
	785,
	2307,
	1799,
	1806,
	5633,
	258,
	2316,
	5381,
	6,
	2061,
	4616,
	9,
	1050,
	4098,
	1034,
	2576,
	5130,
	522,
	2835,
	6149,
	9,
	3341,
	7435,
	1030,
	2073,
	5638,
	1292,
	3601,
	6921,
	1035,
	3601,
	3342,
	2067,
	4367,
	6412,
	526,
	3353,
	5385,
	3343,
	4121,
	7430,
	2061,
	4631,
	7434,
	1296,
	6414,
	8458,
	783,
	5393,
	6929,
	2833,
	5141,
	3854,
	3609,
	4129,
	7943,
	3088,
	5397,
	7698,
	2067,
	5147,
	6414,
	4628,
	5912,
	8717,
	2579,
	5155,
	7685,
	2838,
	6929,
	5909,
	5400,
	5915,
	4884,
	4637,
	6177,
	4873,
	3616,
	5669,
	7945,
	3864,
	7195,
	8975,
	3352,
	6684,
	10008,
	3096,
	8725,
	8467,
	3101,
	6691,
	6673,
	4385,
	8470,
	8473,
	5659,
	7710,
	7957,
	6684,
	7206,
	10767,
	2843,
	9240,
	9750,
	4892,
	7716,
	8471,
	7199,
	7721,
	9745,
	4896,
	10009,
	10009,
	2339,
	8235,
	8205,
	8226,
	9758,
	9496,
	6689,
	7980,
	10260,
	5922,
	10012,
	10783,
	4643,
	9005,
	11279,
	6177,
	9252,
	10017,
	7205,
	11038,
	10525,
	5415,
	9263,
	11032,
	7717,
	10023,
	12576,
	4902,
	11553,
	11552,
	6951,
	10026,
	11812,
	8486,
	10033,
	12314,
	7720,
	10797,
	10532,
	10028,
	10803,
	12572,
	8746,
	11571,
	11296,
	10798,
	11314,
	12070,
	11312,
	12339,
	12841,
	12081,
	13364,
	13356,
	12596,
	0,
	3840,
	3089,
	5393,
	5393,
	3604,
	5397,
	6162,
	3351,
	6929,
	5909,
	5400,
	7443,
	7186,
	6170,
	8470,
	7705,
	5406,
	10009,
	7193,
	7975,
	11038,
	8477,
	8237,
	0,
	4096,
	2570,
	3601,
	4361,
	3598,
	3861,
	5389,
	3604,
	5145,
	6930,
	5143,
	6175,
	7183,
	6170,
	6945,
	8470,
	7199,
	8485,
	10010,
	7205,
	9515,
	10014,
	8231,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	15360,
	13621,
	12856,
	13106,
	11309,
	9773,
	9253,
	7710,
	5916,
	5143,
	4112,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
	16191,
	16128,
	63,
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

Compact cabinet = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	312,	// xcood
	239,	// ycood
	0,	// frame
	12749,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	340-312,	// mouseSize_x
	248-239,	// mouseSize_y
	CABINET_ACTION,	// actionScript
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

Compact lamb_tv = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	203,	// xcood
	223,	// ycood
	0,	// frame
	57,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	225-203,	// mouseSize_x
	251-223,	// mouseSize_y
	LAMB_TV_ACTION,	// actionScript
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

Compact fish_food = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	179,	// xcood
	236,	// ycood
	0,	// frame
	12668,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	192-179,	// mouseSize_x
	289-236,	// mouseSize_y
	FISH_FOOD_ACTION,	// actionScript
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

Compact fish_poster = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	22,	// screen
	0,	// place
	0,	// getToTable
	232,	// xcood
	193,	// ycood
	0,	// frame
	12741,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	248-232,	// mouseSize_x
	232-193,	// mouseSize_y
	FISH_POSTER_ACTION,	// actionScript
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

} // namespace SkyCompact

} // namespace Sky

#endif
