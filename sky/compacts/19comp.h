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

#ifndef SKY19COMP_H
#define SKY19COMP_H




namespace Sky {

namespace SkyCompact {

uint16 rs_foster_19_28[] = {
	C_XCOOD,
	120,
	C_YCOOD,
	272,
	C_SCREEN,
	28,
	C_PLACE,
	ID_S28_FLOOR,
	65535
};

uint16 s19_mouse[] = {
	12289,
	ID_JOEY,
	ID_LEFT_EXIT_19,
	ID_RIGHT_EXIT_19,
	ID_SMALL_19,
	ID_S19_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 s19_floor_table[] = {
	ID_S19_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S19_RIGHT_ON,
	1,
	S19_LEFT_ON,
	ID_RIGHT_EXIT_19,
	GT_RIGHT_EXIT_19,
	ID_LEFT_EXIT_19,
	GT_LEFT_EXIT_19,
	ID_SMALL_19,
	GT_TOP_RIGHT_19,
	12289,
	GET_TO_UCHAR,
	65535
};

Compact s19_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	19,	// screen
	0,	// place
	s19_floor_table,	// getToTable
	128,	// xcood
	216,	// ycood
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

uint16 rs_28_sml_19[] = {
	C_XCOOD,
	456+8,
	C_YCOOD,
	248,
	C_SCREEN,
	19,
	C_PLACE,
	ID_S19_FLOOR,
	C_FRAME,
	44+12*64,
	65535
};

uint16 s19_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_INNER_LAMB_DOOR,
	ID_GALLAGER_BEL,
	ID_LIFT_29,
	12289,
	0XFFFF,
	ID_MENU_LOGIC
};

Compact right_exit_19 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	19,	// screen
	0,	// place
	0,	// getToTable
	431,	// xcood
	266,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	30,	// mouseSize_x
	4242,	// mouseSize_y
	ER19_ACTION,	// actionScript
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

ExtCompact useless_char_ext = {
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
	SP_COL_UCHAR,	// spColour
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

Compact useless_char = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_RECREATE+ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	19,	// screen
	0,	// place
	0,	// getToTable
	236,	// xcood
	189,	// ycood
	16*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	32,	// mouseSize_x
	58,	// mouseSize_y
	UCHAR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	UCHAR_LOGIC,	// baseSub
	0,	// baseSub_off
	&useless_char_ext
};

Compact left_exit_19 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	19,	// screen
	0,	// place
	0,	// getToTable
	163,	// xcood
	211,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	277-211,	// mouseSize_y
	EL19_ACTION,	// actionScript
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

uint16 joey_list_s19[] = {
	208,
	360,
	240,
	304,
	1,
	0
};

uint16 rs_foster_28_19[] = {
	C_XCOOD,
	456,
	C_YCOOD,
	304,
	C_SCREEN,
	19,
	C_PLACE,
	ID_S19_FLOOR,
	65535
};

Compact top_right_19 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	19,	// screen
	0,	// place
	0,	// getToTable
	431,	// xcood
	137,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	30,	// mouseSize_x
	265-137,	// mouseSize_y
	TOP_R19_ACTION,	// actionScript
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

uint32 *grid19 = 0;

uint16 s19_pal[] = {
	0,
	1792,
	257,
	1285,
	2309,
	772,
	525,
	2051,
	1799,
	1549,
	4098,
	1284,
	2059,
	2823,
	2568,
	2317,
	5125,
	1029,
	1809,
	3074,
	1293,
	1560,
	4358,
	2825,
	2325,
	3587,
	2315,
	2329,
	5123,
	2315,
	3854,
	4360,
	2829,
	3089,
	7184,
	2056,
	3350,
	5638,
	3340,
	4369,
	6664,
	2828,
	3859,
	4110,
	3858,
	3104,
	5892,
	2575,
	2591,
	5898,
	4366,
	3613,
	6408,
	3600,
	5394,
	5387,
	4114,
	3613,
	5645,
	5137,
	4378,
	6674,
	3092,
	4638,
	5642,
	2839,
	3621,
	9229,
	2321,
	3879,
	7686,
	4626,
	5403,
	6928,
	5396,
	4897,
	6413,
	4633,
	4898,
	8209,
	5397,
	5159,
	7435,
	6167,
	4907,
	10506,
	3860,
	6688,
	9486,
	3862,
	5922,
	10002,
	4885,
	7197,
	9492,
	3098,
	5924,
	8214,
	6681,
	6438,
	10771,
	4376,
	7457,
	11541,
	3608,
	7207,
	8976,
	6171,
	8222,
	10007,
	6170,
	7972,
	11794,
	4633,
	7211,
	9233,
	7197,
	7466,
	10006,
	6430,
	6958,
	12566,
	4380,
	7467,
	9754,
	7712,
	7981,
	10003,
	5922,
	7727,
	12569,
	5151,
	8492,
	10779,
	7969,
	8499,
	11793,
	5668,
	9768,
	12316,
	6434,
	9010,
	12053,
	7715,
	9771,
	12831,
	6949,
	9780,
	12055,
	7464,
	9775,
	11297,
	9001,
	9779,
	11551,
	10025,
	10805,
	13848,
	7209,
	10547,
	12322,
	9515,
	11316,
	14366,
	6700,
	12081,
	13088,
	10284,
	12087,
	14109,
	9517,
	11832,
	12065,
	12079,
	12340,
	13607,
	9009,
	12339,
	14380,
	8242,
	12599,
	14122,
	9779,
	13364,
	14385,
	10805,
	13367,
	14638,
	11576,
	14135,
	14641,
	13625,
	771,
	6402,
	777,
	2836,
	5897,
	2575,
	5146,
	9228,
	2321,
	7719,
	11289,
	6945,
	10287,
	13085,
	7974,
	10547,
	12578,
	8239,
	11832,
	13601,
	9009,
	13111,
	14374,
	10805,
	771,
	6658,
	3092,
	7457,
	11541,
	4895,
	9768,
	12316,
	6434,
	9522,
	13851,
	7209,
	10547,
	13346,
	7724,
	11315,
	14376,
	8494,
	12599,
	14122,
	9779,
	13624,
	14634,
	11576,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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

uint16 s19_chip_list[] = {
	136+DISK_9,
	137+DISK_9,
	70+DISK_9,
	71+DISK_9,
	72+DISK_9,
	74+DISK_9,
	75+DISK_9,
	16+DISK_9,
	0
};

uint16 s19_fast_list[] = {
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
