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

#ifndef SKY17COMP_H
#define SKY17COMP_H




namespace Sky {

namespace SkyCompact {

uint16 pulseb_seq[] = {
	132*64,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	1,
	256,
	203,
	2,
	256,
	203,
	3,
	256,
	203,
	4,
	256,
	203,
	5,
	256,
	203,
	6,
	256,
	203,
	7,
	256,
	203,
	8,
	256,
	203,
	9,
	256,
	203,
	10,
	256,
	203,
	11,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	256,
	203,
	0,
	0
};

uint16 s17_floor_table[] = {
	ID_S17_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S17_LEFT_ON,
	ID_CORE_EXIT,
	GT_CORE_EXIT,
	ID_ANITA_CARD,
	GT_ANITA_CARD,
	ID_PULSE,
	GT_RODS,
	65535
};

Compact s17_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	17,	// screen
	0,	// place
	s17_floor_table,	// getToTable
	200,	// xcood
	264,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	216,	// mouseSize_x
	1024,	// mouseSize_y
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

uint32 *grid17 = 0;

uint16 get_card_seq[] = {
	30*64,
	323,
	248,
	0,
	323,
	248,
	1,
	323,
	248,
	2,
	323,
	248,
	3,
	323,
	248,
	3,
	323,
	248,
	3,
	323,
	248,
	3,
	323,
	248,
	2,
	323,
	248,
	1,
	323,
	248,
	0,
	0
};

Compact core_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	17,	// screen
	0,	// place
	0,	// getToTable
	184,	// xcood
	208,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	12,	// mouseSize_x
	292-208,	// mouseSize_y
	CORE_EXIT_ACTION,	// actionScript
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

uint16 rs_foster_17_16[] = {
	C_XCOOD,
	296,
	C_YCOOD,
	256,
	C_SCREEN,
	16,
	C_PLACE,
	ID_S16_FLOOR,
	65535
};

uint16 s17_chip_list[] = {
	173,
	111,
	136+DISK_2,
	137+DISK_2,
	90+DISK_6,
	96+DISK_6,
	97+DISK_6,
	106+DISK_6,
	30+DISK_6,
	110+DISK_6,
	132+DISK_6,
	133+DISK_6,
	0
};

uint16 s17_pal[] = {
	0,
	3584,
	5650,
	4880,
	4375,
	6164,
	5138,
	4118,
	6421,
	5395,
	4376,
	6934,
	5652,
	5403,
	6422,
	6162,
	4892,
	6679,
	6164,
	5659,
	6934,
	5909,
	5148,
	7704,
	6166,
	6170,
	7192,
	6675,
	5661,
	7449,
	6932,
	5663,
	8218,
	6681,
	6173,
	7962,
	6935,
	5918,
	8476,
	7446,
	6431,
	8219,
	7194,
	6174,
	8222,
	7449,
	7458,
	8476,
	7451,
	6688,
	8478,
	8217,
	6689,
	9502,
	7708,
	6945,
	8735,
	7713,
	7968,
	8735,
	8224,
	7716,
	8993,
	8733,
	7205,
	10019,
	8991,
	7460,
	10277,
	9248,
	9256,
	9762,
	8995,
	7716,
	10535,
	9254,
	8742,
	9766,
	9765,
	8487,
	11048,
	9511,
	9256,
	10535,
	9769,
	10025,
	10791,
	10536,
	9257,
	11818,
	10534,
	10797,
	10792,
	11045,
	9516,
	11821,
	11050,
	11307,
	11818,
	10542,
	12333,
	11049,
	11307,
	11565,
	11307,
	11561,
	10030,
	12078,
	11820,
	10540,
	12335,
	11565,
	12594,
	12076,
	11567,
	10544,
	13104,
	11819,
	11315,
	11314,
	12079,
	10798,
	13362,
	12844,
	12592,
	12335,
	12589,
	12594,
	12849,
	12595,
	11568,
	13620,
	12339,
	12339,
	12597,
	13103,
	13366,
	12850,
	12343,
	11827,
	14133,
	13616,
	12597,
	14132,
	13365,
	12595,
	12600,
	13111,
	12084,
	14391,
	12857,
	13109,
	13622,
	14385,
	13113,
	14136,
	13370,
	13622,
	13879,
	14132,
	14137,
	13622,
	14900,
	13108,
	14905,
	13882,
	13368,
	15418,
	14395,
	13623,
	14907,
	14395,
	14138,
	14395,
	14141,
	14137,
	15162,
	15413,
	14653,
	14907,
	14653,
	14139,
	15165,
	15674,
	15931,
	15419,
	15930,
	15422,
	15679,
	15678,
	16190,
	16191,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	63,
	16128,
	0,
	0,
	15360,
	15420,
	13621,
	10551,
	11561,
	8995,
	6697,
	8218,
	2834,
	1811,
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
	15620,
	16189,
	14649,
	11321,
	12588,
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
	0,
	63,
	16128,
	0,
	63,
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

uint16 s17_mouse[] = {
	ID_JOEY,
	ID_CORE_EXIT,
	ID_ANITA_CARD,
	ID_PULSE,
	ID_S17_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact pulseb = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	17,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	132*64,	// frame
	8377,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	225-0XCF,	// mouseSize_y
	COAT_ACTION,	// actionScript
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

uint16 s17_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	129+DISK_6,
	130+DISK_6,
	131+DISK_6,
	0
};

uint16 pulse_seq[] = {
	110*64,
	256,
	247,
	0,
	256,
	247,
	1,
	256,
	247,
	2,
	256,
	247,
	3,
	256,
	247,
	4,
	256,
	247,
	5,
	256,
	247,
	6,
	256,
	247,
	7,
	256,
	247,
	8,
	256,
	247,
	9,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	256,
	247,
	0,
	0
};

Compact anita_card = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	17,	// screen
	0,	// place
	0,	// getToTable
	328,	// xcood
	296,	// ycood
	133*64,	// frame
	68,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	ANITA_CARD_ACTION,	// actionScript
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

uint16 rs_foster_16_17[] = {
	C_XCOOD,
	176,
	C_YCOOD,
	280,
	C_SCREEN,
	17,
	C_PLACE,
	ID_S17_FLOOR,
	65535
};

uint16 s17_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_PULSE,
	ID_PULSEB,
	ID_ANITA_CARD,
	0XFFFF,
	ID_MENU_LOGIC
};

Compact pulse = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_SORT+ST_MOUSE,	// status
	0,	// sync
	17,	// screen
	0,	// place
	0,	// getToTable
	20,	// xcood
	20,	// ycood
	110*64,	// frame
	9259,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	5,	// mouseRel_x
	(int16) 65510,	// mouseRel_y
	45,	// mouseSize_x
	30,	// mouseSize_y
	RODS_ACTION,	// actionScript
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

} // namespace SkyCompact

} // namespace Sky

#endif
