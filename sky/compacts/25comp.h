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

#ifndef SKY25COMP_H
#define SKY25COMP_H




namespace Sky {

namespace SkyCompact {

Compact sales_chart = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	25,	// screen
	0,	// place
	0,	// getToTable
	273,	// xcood
	185,	// ycood
	0,	// frame
	12778,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	346-273,	// mouseSize_x
	220-185,	// mouseSize_y
	SALES_CHART_ACTION,	// actionScript
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

uint16 hook_seq[] = {
	48*64,
	359,
	237,
	0,
	359,
	244,
	0,
	359,
	253,
	0,
	359,
	263,
	0,
	359,
	274,
	0,
	359,
	282,
	1,
	359,
	294,
	2,
	0
};

uint16 s25_pal[] = {
	0,
	1792,
	1543,
	2318,
	4103,
	1289,
	2828,
	4619,
	1546,
	2832,
	3593,
	3341,
	2838,
	4871,
	2061,
	3855,
	5390,
	2829,
	4114,
	5902,
	2063,
	4118,
	6666,
	2062,
	4370,
	6930,
	2320,
	4378,
	6157,
	3602,
	4884,
	6932,
	2834,
	4639,
	5643,
	5142,
	5654,
	8214,
	3604,
	5661,
	6927,
	4886,
	5403,
	6939,
	7700,
	5667,
	8461,
	4374,
	6681,
	7449,
	8469,
	6180,
	7952,
	5657,
	6683,
	10012,
	3608,
	7196,
	7961,
	8983,
	6439,
	9234,
	5147,
	7709,
	7452,
	9499,
	6698,
	10000,
	5149,
	7967,
	10271,
	6427,
	7716,
	9496,
	9753,
	7459,
	10782,
	4637,
	7712,
	8742,
	10268,
	7976,
	9495,
	6944,
	8481,
	11041,
	5407,
	7461,
	9002,
	7971,
	8236,
	10522,
	10781,
	8494,
	10005,
	8993,
	9508,
	8738,
	12578,
	8749,
	10781,
	7460,
	9509,
	10790,
	11296,
	9008,
	11031,
	10273,
	9766,
	9510,
	11301,
	9261,
	11296,
	11810,
	10023,
	11048,
	9255,
	10032,
	10522,
	9769,
	10021,
	11061,
	11558,
	10035,
	12316,
	10533,
	10280,
	11824,
	12324,
	10289,
	12320,
	9767,
	10794,
	10026,
	14121,
	10804,
	12318,
	12838,
	11057,
	11298,
	10541,
	10290,
	13357,
	10026,
	11567,
	11307,
	13356,
	11318,
	14368,
	9517,
	12335,
	13613,
	9775,
	12078,
	13110,
	11312,
	11829,
	12592,
	12338,
	12600,
	12070,
	14641,
	13108,
	12846,
	14130,
	13113,
	13353,
	12852,
	12600,
	12596,
	15155,
	13623,
	14895,
	11317,
	13370,
	15413,
	12085,
	13624,
	14648,
	12599,
	14133,
	15419,
	14134,
	14393,
	15931,
	12345,
	14397,
	15419,
	15675,
	15422,
	16186,
	15933,
	0,
	1792,
	1543,
	2318,
	4103,
	1289,
	2828,
	4619,
	1546,
	2832,
	3593,
	3341,
	2838,
	4871,
	2061,
	3855,
	5390,
	2829,
	4114,
	5902,
	2063,
	4118,
	6666,
	2062,
	0,
	3072,
	2827,
	3855,
	4622,
	4625,
	4632,
	5134,
	5139,
	5654,
	7444,
	3862,
	6681,
	7961,
	5657,
	7709,
	8476,
	8481,
	9508,
	11298,
	10541,
	12849,
	14384,
	14389,
	63,
	16128,
	5140,
	10559,
	16169,
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
	15676,
	11583,
	16180,
	11806,
	4159,
	16170,
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

Compact statue_25 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	25,	// screen
	0,	// place
	0,	// getToTable
	363,	// xcood
	238,	// ycood
	0,	// frame
	12775,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	424-363,	// mouseSize_x
	313-238,	// mouseSize_y
	STATUE_25_ACTION,	// actionScript
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

Compact anchor_exit_25 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	25,	// screen
	0,	// place
	0,	// getToTable
	185,	// xcood
	244,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	203-185,	// mouseSize_x
	309-244,	// mouseSize_y
	ANCHOR25_ACTION,	// actionScript
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

uint16 spark_25_seq[] = {
	56*64,
	348,
	239,
	0,
	348,
	239,
	1,
	348,
	239,
	2,
	348,
	239,
	3,
	0
};

uint16 rs_foster_23_25[] = {
	C_XCOOD,
	160,
	C_YCOOD,
	304,
	C_SCREEN,
	25,
	C_PLACE,
	ID_S25_FLOOR,
	65535
};

ExtCompact anchor_ext = {
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
	SP_COL_ANCHOR,	// spColour
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

Compact anchor = {
	L_SCRIPT,	// logic
	ST_RECREATE+ST_LOGIC+ST_MOUSE+ST_BACKGROUND,	// status
	0,	// sync
	25,	// screen
	0,	// place
	0,	// getToTable
	258,	// xcood
	233,	// ycood
	43*64,	// frame
	4,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	16,	// mouseRel_x
	0,	// mouseRel_y
	297-271,	// mouseSize_x
	256-233,	// mouseSize_y
	ANCHOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	ANCHOR_LOGIC,	// baseSub
	0,	// baseSub_off
	&anchor_ext
};

uint16 s25_chip_list[] = {
	136+DISK_8,
	137+DISK_8,
	43+DISK_8,
	44+DISK_8,
	45+DISK_8,
	46+DISK_8,
	47+DISK_8,
	48+DISK_8,
	55+DISK_8,
	56+DISK_8,
	57+DISK_8,
	58+DISK_8,
	59+DISK_8,
	60+DISK_8,
	26+DISK_8,
	27+DISK_8,
	28+DISK_8,
	0
};

uint16 lazer_25_seq[] = {
	55*64,
	344,
	244,
	0,
	344,
	244,
	1,
	0
};

uint16 s25_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	0
};

uint32 *grid25 = 0;

uint16 anchor_leave_seq[] = {
	45*64,
	280,
	233,
	0,
	279,
	226,
	1,
	280,
	226,
	2,
	278,
	227,
	3,
	277,
	226,
	4,
	273,
	226,
	5,
	270,
	226,
	6,
	264,
	227,
	7,
	263,
	226,
	6,
	259,
	226,
	5,
	255,
	226,
	8,
	252,
	226,
	9,
	253,
	225,
	10,
	253,
	224,
	11,
	253,
	223,
	12,
	253,
	223,
	13,
	253,
	223,
	14,
	252,
	223,
	15,
	252,
	223,
	15,
	252,
	223,
	15,
	252,
	223,
	15,
	252,
	223,
	15,
	0
};

uint16 joey_list_s25[] = {
	156,
	360,
	296,
	304,
	1,
	0
};

uint16 rs_foster_25_23[] = {
	C_XCOOD,
	432,
	C_YCOOD,
	288,
	C_SCREEN,
	23,
	C_PLACE,
	ID_S23_FLOOR,
	65535
};

uint16 s25_mouse[] = {
	ID_JOEY,
	ID_ANCHOR_EXIT_25,
	ID_ANCHOR,
	ID_HOOK,
	ID_STATUE_25,
	ID_SALES_CHART,
	ID_S25_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 s25_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_ANCHOR,
	ID_HOOK,
	ID_LAZER_25,
	ID_SPARK_25,
	ID_LAMB,
	ID_LAMB_DOOR_20,
	ID_GALLAGER_BEL,
	ID_INNER_LAMB_DOOR,
	ID_LIFT_29,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 anchor_seq[] = {
	43*64,
	258,
	233,
	0,
	258,
	233,
	0,
	258,
	233,
	0,
	258,
	233,
	0,
	258,
	233,
	1,
	258,
	233,
	1,
	258,
	233,
	2,
	258,
	233,
	2,
	258,
	233,
	3,
	258,
	233,
	3,
	258,
	233,
	4,
	258,
	233,
	4,
	258,
	233,
	4,
	258,
	233,
	4,
	258,
	233,
	4,
	258,
	233,
	4,
	258,
	233,
	5,
	258,
	233,
	5,
	258,
	233,
	6,
	258,
	233,
	6,
	258,
	233,
	7,
	258,
	233,
	7,
	258,
	233,
	8,
	258,
	233,
	8,
	258,
	233,
	0,
	0
};

uint16 anchor_return_seq[] = {
	46*64,
	252,
	223,
	0,
	252,
	223,
	1,
	252,
	223,
	2,
	252,
	224,
	3,
	252,
	226,
	4,
	254,
	226,
	5,
	257,
	227,
	6,
	264,
	226,
	7,
	269,
	226,
	8,
	272,
	226,
	9,
	272,
	227,
	10,
	279,
	226,
	9,
	283,
	226,
	8,
	278,
	226,
	5,
	279,
	226,
	11,
	280,
	233,
	12,
	280,
	233,
	12,
	280,
	233,
	12,
	280,
	233,
	12,
	0
};

Compact hook = {
	L_SCRIPT,	// logic
	ST_FOREGROUND+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	25,	// screen
	0,	// place
	0,	// getToTable
	359,	// xcood
	237,	// ycood
	48*64,	// frame
	191,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	10,	// mouseSize_x
	10,	// mouseSize_y
	HOOK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	HOOK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact spark_25 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	25,	// screen
	0,	// place
	0,	// getToTable
	348,	// xcood
	239,	// ycood
	56*64,	// frame
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
	0,	// mode
	SPARK_25_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact lazer_25 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	25,	// screen
	0,	// place
	0,	// getToTable
	344,	// xcood
	244,	// ycood
	55*64,	// frame
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
	0,	// mode
	LAZER_25_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s25_floor_table[] = {
	ID_S25_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S25_LEFT_ON,
	ID_ANCHOR_EXIT_25,
	GT_ANCHOR_EXIT_25,
	ID_ANCHOR,
	GT_ANCHOR,
	ID_ANCHOR_PC,
	GT_ANCHOR_PC,
	ID_STATUE_25,
	GT_STATUE_25,
	ID_HOOK,
	GT_HOOK,
	ID_SALES_CHART,
	GT_SALES_CHART,
	65535
};

uint16 get_anchor_seq[] = {
	58*64,
	341,
	264,
	0,
	341,
	264,
	1,
	341,
	264,
	2,
	341,
	264,
	3,
	341,
	264,
	3,
	341,
	264,
	3,
	341,
	264,
	3,
	341,
	264,
	4,
	341,
	264,
	5,
	0
};

Compact s25_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	25,	// screen
	0,	// place
	s25_floor_table,	// getToTable
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

} // namespace SkyCompact

} // namespace Sky

#endif
