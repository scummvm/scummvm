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

#ifndef SKY38COMP_H
#define SKY38COMP_H




namespace Sky {

namespace SkyCompact {

uint16 spu_left[] = {
	2,
	16+71*64,
	65534,
	0,
	2,
	17+71*64,
	65534,
	0,
	2,
	18+71*64,
	65534,
	0,
	2,
	19+71*64,
	65534,
	0,
	2,
	20+71*64,
	65534,
	0,
	2,
	21+71*64,
	65534,
	0,
	2,
	22+71*64,
	65534,
	0,
	2,
	23+71*64,
	65534,
	0,
	0
};

uint16 sc38_mouse_list[] = {
	ID_SPUNKY,
	ID_SC32_BUZZER,
	ID_DANIELLE,
	ID_SC38_SOFA,
	ID_SC38_STATUE,
	ID_SC38_VIDEO,
	ID_SC38_MONITOR,
	ID_SC38_BISCUITS,
	ID_SC38_DOG_TRAY,
	ID_SC38_LIFT,
	ID_SC38_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sniff_left[] = {
	72*64,
	208,
	219,
	0,
	208,
	219,
	1,
	208,
	219,
	2,
	208,
	219,
	3,
	208,
	219,
	4,
	208,
	219,
	5,
	208,
	219,
	4,
	208,
	219,
	5,
	208,
	219,
	5,
	208,
	219,
	6,
	208,
	219,
	6,
	208,
	219,
	5,
	208,
	219,
	5,
	208,
	219,
	6,
	208,
	219,
	6,
	208,
	219,
	7,
	208,
	219,
	7,
	208,
	219,
	6,
	208,
	219,
	5,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	5,
	208,
	219,
	5,
	208,
	219,
	3,
	208,
	219,
	4,
	208,
	219,
	5,
	208,
	219,
	6,
	208,
	219,
	7,
	208,
	219,
	6,
	208,
	219,
	5,
	208,
	219,
	3,
	208,
	219,
	2,
	208,
	219,
	1,
	208,
	219,
	0,
	0
};

uint16 sc38_palette[] = {
	0,
	1280,
	1798,
	1797,
	2051,
	1033,
	1804,
	5127,
	1284,
	2571,
	2313,
	2061,
	1809,
	3593,
	1292,
	2325,
	6150,
	2824,
	3090,
	3339,
	2831,
	1566,
	5382,
	3850,
	4113,
	3591,
	4114,
	2336,
	8714,
	1290,
	2843,
	4622,
	3090,
	3606,
	5907,
	3600,
	2342,
	4105,
	5396,
	3614,
	5131,
	4371,
	5398,
	6922,
	4623,
	5393,
	12058,
	1544,
	2858,
	9222,
	1808,
	3624,
	4874,
	8213,
	5911,
	11535,
	2827,
	4644,
	7693,
	5396,
	3626,
	5135,
	7193,
	3377,
	10759,
	1810,
	4390,
	6418,
	6679,
	5410,
	5905,
	8983,
	3887,
	11022,
	3603,
	6676,
	13354,
	2831,
	7192,
	7198,
	8217,
	5671,
	4627,
	12059,
	5677,
	10762,
	6421,
	5925,
	6171,
	10269,
	6689,
	13597,
	4114,
	7706,
	5667,
	11550,
	5425,
	5141,
	12831,
	7455,
	11042,
	7449,
	6446,
	11800,
	3867,
	5431,
	6676,
	12320,
	8475,
	7978,
	9761,
	6707,
	10261,
	8480,
	8984,
	8244,
	11809,
	6455,
	12825,
	6939,
	9501,
	8752,
	10532,
	7989,
	13332,
	6688,
	9756,
	13622,
	7966,
	9762,
	8494,
	13349,
	9015,
	6928,
	15144,
	9767,
	14378,
	5922,
	9521,
	13340,
	5926,
	9773,
	8230,
	14377,
	9271,
	8988,
	13098,
	10296,
	10772,
	10282,
	11039,
	13885,
	8232,
	10552,
	13338,
	9513,
	11555,
	12090,
	10794,
	11303,
	11318,
	11821,
	11323,
	13847,
	8749,
	11322,
	9502,
	16175,
	11828,
	12583,
	11311,
	12073,
	13883,
	12333,
	12337,
	14386,
	9521,
	12603,
	13600,
	11826,
	12851,
	14647,
	10803,
	12855,
	14899,
	12084,
	13622,
	14650,
	13623,
	14388,
	14654,
	15673,
	14906,
	15165,
	15931,
	15677,
	16190,
	16191,
	3592,
	4121,
	4112,
	6939,
	10011,
	10023,
	13107,
	16179,
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
	1287,
	2822,
	2568,
	2830,
	4622,
	4622,
	4372,
	5910,
	6677,
	6169,
	7198,
	8733,
	8480,
	9254,
	10791,
	11049,
	11822,
	12848,
	13363,
	14646,
	14905,
	16191,
	63,
	0,
	0,
	14336,
	13364,
	11569,
	10541,
	9766,
	7716,
	7453,
	5140,
	3093,
	1805,
	770,
	519,
	14851,
	8751,
	10295,
	13596,
	5921,
	5931,
	9488,
	2576,
	3104,
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
	12607,
	14885,
	7209,
	8501,
	11797,
	3352,
	4135,
	8199,
	778,
	9791,
	14893,
	9759,
	5938,
	10783,
	6417,
	2332,
	3857,
	2308,
	2865,
	11531,
	1542,
	287,
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

uint16 spu_u_to_l[] = {
	35+71*64,
	0
};

uint16 dan_d_to_r[] = {
	39+55*64,
	0
};

uint16 sex_t_to_r[] = {
	32+106*64,
	39+106*64,
	0
};

uint16 spu_u_to_d[] = {
	35+71*64,
	34+71*64,
	33+71*64,
	0
};

Compact sc38_dog_tray = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	220,	// xcood
	251,	// ycood
	0,	// frame
	17178,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	241-220,	// mouseSize_x
	267-251,	// mouseSize_y
	SC38_DOG_TRAY_ACTION,	// actionScript
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

uint16 sex_up[] = {
	2,
	0+106*64,
	0,
	65534,
	2,
	1+106*64,
	0,
	65534,
	2,
	2+106*64,
	0,
	65534,
	2,
	3+106*64,
	0,
	65534,
	2,
	4+106*64,
	0,
	65534,
	2,
	5+106*64,
	0,
	65534,
	2,
	6+106*64,
	0,
	65534,
	2,
	7+106*64,
	0,
	65534,
	0
};

uint16 dan_u_to_r[] = {
	37+55*64,
	0
};

uint16 sex_l_to_r[] = {
	33+106*64,
	32+106*64,
	39+106*64,
	0
};

uint16 spu_t_to_l[] = {
	32+71*64,
	33+71*64,
	0
};

uint16 dan_u_to_t[] = {
	37+55*64,
	38+55*64,
	0
};

uint16 sc38_screen_2[] = {
	136*64,
	379,
	209,
	0,
	379,
	209,
	1,
	379,
	209,
	2,
	379,
	209,
	3,
	379,
	209,
	4,
	379,
	209,
	5,
	379,
	209,
	6,
	379,
	209,
	7,
	379,
	209,
	8,
	379,
	209,
	9,
	379,
	209,
	10,
	379,
	209,
	11,
	379,
	209,
	12,
	379,
	209,
	13,
	379,
	209,
	14,
	379,
	209,
	15,
	379,
	209,
	16,
	379,
	209,
	17,
	379,
	209,
	18,
	379,
	209,
	19,
	379,
	209,
	20,
	379,
	209,
	21,
	379,
	209,
	22,
	0
};

uint16 dan_l_to_u[] = {
	35+55*64,
	0
};

uint16 sc38_screen_3[] = {
	137*64,
	379,
	209,
	0,
	379,
	209,
	1,
	379,
	209,
	2,
	379,
	209,
	3,
	379,
	209,
	4,
	379,
	209,
	5,
	379,
	209,
	6,
	379,
	209,
	7,
	379,
	209,
	8,
	379,
	209,
	9,
	379,
	209,
	10,
	379,
	209,
	11,
	379,
	209,
	12,
	379,
	209,
	13,
	379,
	209,
	14,
	379,
	209,
	15,
	379,
	209,
	16,
	379,
	209,
	17,
	379,
	209,
	18,
	379,
	209,
	19,
	379,
	209,
	20,
	379,
	209,
	21,
	379,
	209,
	22,
	379,
	209,
	23,
	379,
	209,
	24,
	379,
	209,
	25,
	379,
	209,
	26,
	379,
	209,
	27,
	379,
	209,
	28,
	0
};

uint16 dan_r_to_t[] = {
	39+55*64,
	32+55*64,
	0
};

uint16 sex_u_to_t[] = {
	37+106*64,
	38+106*64,
	0
};

uint16 sc38_dani_get_up[] = {
	109*64,
	294,
	244,
	0,
	295,
	244,
	1,
	298,
	245,
	2,
	301,
	241,
	3,
	305,
	238,
	4,
	307,
	238,
	5,
	308,
	238,
	6,
	311,
	238,
	7,
	319,
	239,
	8,
	319,
	238,
	9,
	317,
	242,
	10,
	317,
	243,
	11,
	317,
	243,
	12,
	0
};

uint16 sex_r_to_d[] = {
	39+106*64,
	0
};

uint16 dan_r_to_l[] = {
	37+55*64,
	36+55*64,
	35+55*64,
	0
};

uint16 dan_auto[32];

uint16 sc38_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_DOG_BARK_THING,
	ID_SC38_MONITOR,
	ID_SC38_VIDEO,
	ID_SC38_HAND_SET,
	ID_SC38_LIFT,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 piss_left[] = {
	75*64,
	208,
	219,
	0,
	208,
	219,
	1,
	208,
	219,
	2,
	208,
	219,
	2,
	208,
	219,
	2,
	208,
	219,
	3,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	4,
	208,
	219,
	3,
	208,
	219,
	2,
	208,
	219,
	1,
	208,
	219,
	0,
	0
};

uint16 sex_d_to_u[] = {
	39+106*64,
	38+106*64,
	37+106*64,
	0
};

uint16 dan_l_to_d[] = {
	33+55*64,
	0
};

uint16 sex_u_to_r[] = {
	37+106*64,
	0
};

Compact sc38_hand_set = {
	0,	// logic
	ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	156,	// xcood
	267,	// ycood
	110*64,	// frame
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

uint16 spu_u_to_r[] = {
	37+71*64,
	0
};

uint16 dan_st_left[] = {
	55*64,
	1,
	0,
	34,
	0
};

uint16 piss_right[] = {
	76*64,
	257,
	219,
	0,
	257,
	219,
	1,
	257,
	219,
	2,
	257,
	219,
	2,
	257,
	219,
	2,
	257,
	219,
	3,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	3,
	257,
	219,
	2,
	257,
	219,
	1,
	257,
	219,
	0,
	0
};

uint16 sex_r_to_u[] = {
	37+106*64,
	0
};

uint16 sex_left[] = {
	4,
	16+106*64,
	65532,
	0,
	4,
	17+106*64,
	65532,
	0,
	4,
	18+106*64,
	65532,
	0,
	4,
	19+106*64,
	65532,
	0,
	4,
	20+106*64,
	65532,
	0,
	4,
	21+106*64,
	65532,
	0,
	4,
	22+106*64,
	65532,
	0,
	4,
	23+106*64,
	65532,
	0,
	0
};

uint16 sex_l_to_u[] = {
	35+106*64,
	0
};

uint16 spu_l_to_t[] = {
	33+71*64,
	32+71*64,
	0
};

uint16 sex_d_to_r[] = {
	39+106*64,
	0
};

uint32 *spu = (uint32*)&spunky;

uint16 sc38_floor_table[] = {
	ID_SC38_FLOOR,
	RET_OK,
	ID_SC38_LIFT,
	GT_SC38_LIFT,
	ID_SC38_SOFA,
	GT_SC38_SOFA,
	ID_SC38_STATUE,
	GT_SC38_STATUE,
	ID_SC38_VIDEO,
	GT_SC38_VIDEO,
	ID_SC38_MONITOR,
	GT_SC38_MONITOR,
	ID_SC38_BISCUITS,
	GT_SC38_BISCUITS,
	ID_SC38_DOG_TRAY,
	GT_SC38_DOG_TRAY,
	ID_SC38_HAND_SET,
	GT_SC38_HAND_SET,
	ID_DANIELLE,
	GT_SC38_DANIELLE,
	ID_STD_LEFT_TALK,
	GT_SC38_LEFT_TALK,
	ID_STD_RIGHT_TALK,
	GT_SC38_RIGHT_TALK,
	65535
};

uint16 sc38_get_food[] = {
	139*64,
	235,
	216,
	0,
	235,
	216,
	1,
	235,
	216,
	2,
	235,
	216,
	3,
	235,
	216,
	4,
	235,
	216,
	5,
	235,
	216,
	6,
	235,
	216,
	6,
	235,
	216,
	6,
	235,
	216,
	5,
	235,
	216,
	4,
	235,
	216,
	3,
	235,
	216,
	2,
	235,
	216,
	1,
	235,
	216,
	0,
	0
};

uint16 sex_right[] = {
	4,
	24+106*64,
	4,
	0,
	4,
	25+106*64,
	4,
	0,
	4,
	26+106*64,
	4,
	0,
	4,
	27+106*64,
	4,
	0,
	4,
	28+106*64,
	4,
	0,
	4,
	29+106*64,
	4,
	0,
	4,
	30+106*64,
	4,
	0,
	4,
	31+106*64,
	4,
	0,
	0
};

uint16 dan_st_talk[] = {
	85*64,
	1,
	0,
	0,
	0
};

uint16 reset_32_38[] = {
	C_SCREEN,
	38,
	C_PLACE,
	ID_SC38_FLOOR,
	C_XCOOD,
	192,
	C_YCOOD,
	288,
	C_FRAME,
	46+12*64,
	C_DIR,
	RIGHT,
	65535
};

Compact sc38_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	sc38_floor_table,	// getToTable
	200,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-200,	// mouseSize_x
	327-256,	// mouseSize_y
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

uint16 sex_d_to_l[] = {
	33+106*64,
	0
};

uint16 spu_r_to_u[] = {
	37+71*64,
	0
};

uint16 reset_dani_sit[] = {
	C_STATUS,
	ST_LOGIC+ST_SORT+ST_RECREATE,
	C_SCREEN,
	38,
	C_PLACE,
	ID_SC38_FLOOR,
	C_XCOOD,
	294,
	C_YCOOD,
	244,
	C_FRAME,
	107*64,
	65535
};

uint16 spu_u_to_t[] = {
	37+71*64,
	38+71*64,
	0
};

uint16 spu_l_to_u[] = {
	35+71*64,
	0
};

uint16 spu_d_to_u[] = {
	39+71*64,
	38+71*64,
	37+71*64,
	0
};

uint16 sex_u_to_d[] = {
	35+106*64,
	34+106*64,
	33+106*64,
	0
};

uint16 sc38_dani_anim_2[] = {
	107*64,
	294,
	244,
	0,
	294,
	245,
	1,
	294,
	245,
	2,
	294,
	245,
	3,
	294,
	245,
	4,
	294,
	244,
	5,
	294,
	244,
	6,
	294,
	244,
	7,
	294,
	244,
	8,
	294,
	244,
	9,
	294,
	244,
	10,
	294,
	244,
	11,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	13,
	294,
	244,
	13,
	294,
	244,
	15,
	294,
	244,
	8,
	294,
	244,
	16,
	294,
	244,
	16,
	294,
	244,
	17,
	294,
	244,
	17,
	294,
	244,
	18,
	294,
	244,
	17,
	294,
	244,
	19,
	294,
	244,
	17,
	294,
	244,
	18,
	294,
	244,
	17,
	294,
	244,
	19,
	294,
	244,
	19,
	294,
	244,
	17,
	294,
	244,
	17,
	294,
	244,
	18,
	294,
	244,
	18,
	294,
	244,
	17,
	294,
	244,
	17,
	294,
	244,
	16,
	294,
	244,
	16,
	294,
	244,
	8,
	294,
	244,
	8,
	294,
	244,
	7,
	294,
	244,
	7,
	294,
	244,
	6,
	294,
	244,
	5,
	294,
	245,
	4,
	294,
	245,
	3,
	294,
	245,
	2,
	294,
	245,
	1,
	0
};

uint16 dan_t_to_r[] = {
	32+55*64,
	39+55*64,
	0
};

uint16 dan_st_up[] = {
	55*64,
	1,
	0,
	36,
	0
};

uint16 bark[] = {
	65*64,
	243,
	225,
	0,
	243,
	225,
	1,
	243,
	225,
	2,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	1,
	243,
	225,
	2,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	1,
	243,
	225,
	2,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	1,
	243,
	225,
	2,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	1,
	243,
	225,
	2,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	1,
	243,
	225,
	2,
	243,
	225,
	0,
	243,
	225,
	1,
	243,
	225,
	2,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	0,
	243,
	225,
	0,
	0
};

uint16 dan_left[] = {
	4,
	16+55*64,
	65532,
	0,
	4,
	17+55*64,
	65532,
	0,
	4,
	18+55*64,
	65532,
	0,
	4,
	19+55*64,
	65532,
	0,
	4,
	20+55*64,
	65532,
	0,
	4,
	21+55*64,
	65532,
	0,
	4,
	22+55*64,
	65532,
	0,
	4,
	23+55*64,
	65532,
	0,
	0
};

uint16 spu_r_to_d[] = {
	39+71*64,
	0
};

uint16 spu_l_to_r[] = {
	33+71*64,
	32+71*64,
	39+71*64,
	0
};

uint16 sex_r_to_t[] = {
	39+106*64,
	32+106*64,
	0
};

uint16 sex_st_down[] = {
	106*64,
	1,
	0,
	32,
	0
};

uint16 sc38_scr_bark[] = {
	140*64,
	202,
	253,
	0,
	202,
	253,
	1,
	202,
	253,
	2,
	202,
	253,
	0,
	202,
	253,
	3,
	202,
	253,
	1,
	202,
	253,
	2,
	202,
	253,
	0,
	202,
	253,
	0,
	202,
	253,
	3,
	202,
	253,
	1,
	202,
	253,
	2,
	202,
	253,
	0,
	202,
	253,
	0,
	202,
	253,
	1,
	202,
	253,
	2,
	202,
	253,
	0,
	202,
	253,
	0,
	202,
	253,
	0,
	202,
	253,
	1,
	202,
	253,
	2,
	202,
	253,
	0,
	202,
	253,
	3,
	202,
	253,
	1,
	202,
	253,
	2,
	202,
	253,
	0,
	202,
	253,
	1,
	202,
	253,
	2,
	202,
	253,
	0,
	202,
	253,
	0,
	202,
	253,
	0,
	202,
	253,
	0,
	0
};

uint16 dan_st_right[] = {
	55*64,
	1,
	0,
	38,
	0
};

uint16 spu_st_left[] = {
	71*64,
	1,
	0,
	34,
	0
};

Compact sc38_sofa = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	272,	// xcood
	242,	// ycood
	0,	// frame
	16679,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	308-272,	// mouseSize_x
	301-242,	// mouseSize_y
	SC38_SOFA_ACTION,	// actionScript
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

uint16 dan_d_to_l[] = {
	33+55*64,
	0
};

uint16 spu_r_to_l[] = {
	37+71*64,
	36+71*64,
	35+71*64,
	0
};

uint16 sc38_chip_list[] = {
	IT_SC38_LAYER_0+DISK_14,
	IT_SC38_SEXY_DANI+DISK_14+0X8000,
	IT_SPUNKY+DISK_12,
	IT_SNIFF_LEFT+DISK_12,
	IT_BARK+DISK_12,
	IT_SC38_FOSTER_LIFT+DISK_14,
	IT_SC38_DANI_ANIMS+DISK_14+0X8000,
	IT_SC38_DANI_SATTLK+DISK_14,
	IT_SC38_DANI_GET_UP+DISK_14+0X8000,
	IT_SC38_HAND_SET+DISK_14,
	IT_SC38_GET_PHONE+DISK_14+0X8000,
	IT_SC38_PHONE_TALK+DISK_14,
	IT_SC38_GET_FOOD+DISK_14+0X8000,
	IT_SC38_USE_VIDEO+DISK_14+0X8000,
	IT_SC38_VIDEO_ANIM+DISK_14,
	IT_SC38_SCREEN_1+0X8000+DISK_14,
	IT_SC38_SCREEN_2+0X8000+DISK_14,
	IT_SC38_SCREEN_3+0X8000+DISK_14,
	IT_SC38_SCREEN_4+DISK_14,
	140+DISK_14,
	0
};

uint16 sc38_fast_list[] = {
	12+DISK_14,
	51+DISK_14,
	52+DISK_14,
	53+DISK_14,
	54+DISK_14,
	IT_SC38_LAYER_1+DISK_14,
	IT_SC38_LAYER_2+DISK_14,
	IT_SC38_GRID_1+DISK_14,
	IT_SC38_GRID_2+DISK_14,
	0
};

Compact sc38_video = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_RECREATE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	376,	// xcood
	252,	// ycood
	133*64,	// frame
	16678,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	(int16) 65532,	// mouseRel_y
	401-376,	// mouseSize_x
	264-248,	// mouseSize_y
	SC38_VIDEO_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC38_VIDEO_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact dog_bark_thing = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	38,	// screen
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
	DOG_BARK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc38_screen_4[] = {
	138*64,
	379,
	209,
	0,
	379,
	209,
	1,
	379,
	209,
	2,
	379,
	209,
	3,
	379,
	209,
	4,
	379,
	209,
	5,
	379,
	209,
	6,
	379,
	209,
	7,
	379,
	209,
	8,
	379,
	209,
	9,
	379,
	209,
	10,
	379,
	209,
	11,
	379,
	209,
	12,
	379,
	209,
	13,
	379,
	209,
	14,
	379,
	209,
	15,
	379,
	209,
	16,
	379,
	209,
	17,
	379,
	209,
	18,
	379,
	209,
	19,
	0
};

uint16 dan_l_to_r[] = {
	33+55*64,
	32+55*64,
	39+55*64,
	0
};

uint16 sex_st_talk[] = {
	106*64,
	1,
	0,
	0,
	0
};

uint16 sex_st_right[] = {
	106*64,
	1,
	0,
	38,
	0
};

uint16 sc38_use_video[] = {
	132*64,
	354,
	210,
	0,
	354,
	210,
	1,
	354,
	210,
	2,
	354,
	210,
	3,
	354,
	210,
	4,
	354,
	210,
	5,
	354,
	210,
	6,
	354,
	210,
	7,
	0
};

Compact sc38_monitor = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	379,	// xcood
	209,	// ycood
	134*64,	// frame
	57,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	0,	// mouseRel_y
	406-378,	// mouseSize_x
	242-209,	// mouseSize_y
	SC38_MONITOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC38_MONITOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 spu_st_talk[] = {
	71*64,
	1,
	0,
	0,
	0
};

uint16 sex_t_to_u[] = {
	38+106*64,
	37+106*64,
	0
};

uint16 sex_r_to_l[] = {
	37+106*64,
	36+106*64,
	35+106*64,
	0
};

uint16 sc38_dani_anim_1[] = {
	107*64,
	294,
	244,
	0,
	294,
	245,
	1,
	294,
	245,
	2,
	294,
	245,
	3,
	294,
	245,
	4,
	294,
	244,
	5,
	294,
	244,
	6,
	294,
	244,
	7,
	294,
	244,
	8,
	294,
	244,
	9,
	294,
	244,
	10,
	294,
	244,
	11,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	13,
	294,
	244,
	14,
	294,
	244,
	14,
	294,
	244,
	14,
	294,
	244,
	14,
	294,
	244,
	13,
	294,
	244,
	15,
	294,
	244,
	8,
	294,
	244,
	16,
	294,
	244,
	16,
	294,
	244,
	17,
	294,
	244,
	17,
	294,
	244,
	18,
	294,
	244,
	17,
	294,
	244,
	19,
	294,
	244,
	16,
	294,
	244,
	16,
	294,
	244,
	8,
	294,
	244,
	15,
	294,
	244,
	13,
	294,
	244,
	9,
	294,
	244,
	8,
	294,
	244,
	7,
	294,
	244,
	6,
	294,
	244,
	5,
	294,
	245,
	4,
	294,
	245,
	3,
	294,
	245,
	2,
	294,
	245,
	1,
	0
};

uint16 sc38_get_phone[] = {
	130*64,
	156,
	259,
	0,
	156,
	259,
	1,
	156,
	259,
	2,
	156,
	259,
	3,
	157,
	259,
	4,
	157,
	259,
	4,
	158,
	259,
	5,
	164,
	259,
	6,
	165,
	259,
	7,
	0
};

Compact sc38_lift = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	182,	// xcood
	242,	// ycood
	7+101*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	8,	// mouseRel_x
	(int16) 65528,	// mouseRel_y
	201-190,	// mouseSize_x
	300-234,	// mouseSize_y
	SC38_LIFT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC38_LIFT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc38_biscuits = {
	0,	// logic
	0,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	228,	// xcood
	258,	// ycood
	0,	// frame
	198,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	237-228,	// mouseSize_x
	261-258,	// mouseSize_y
	SC38_BISCUITS_ACTION,	// actionScript
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

uint16 dan_r_to_u[] = {
	37+55*64,
	0
};

uint16 spu_up[] = {
	1,
	0+71*64,
	0,
	65535,
	1,
	1+71*64,
	0,
	65535,
	1,
	2+71*64,
	0,
	65535,
	1,
	3+71*64,
	0,
	65535,
	1,
	4+71*64,
	0,
	65535,
	1,
	5+71*64,
	0,
	65535,
	1,
	6+71*64,
	0,
	65535,
	1,
	7+71*64,
	0,
	65535,
	0
};

uint16 sc38_dani_anim_3[] = {
	107*64,
	294,
	244,
	0,
	294,
	245,
	1,
	294,
	245,
	2,
	294,
	245,
	3,
	294,
	245,
	4,
	294,
	244,
	5,
	294,
	244,
	6,
	294,
	244,
	7,
	294,
	244,
	8,
	294,
	244,
	9,
	294,
	244,
	10,
	294,
	244,
	11,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	13,
	294,
	244,
	14,
	294,
	244,
	14,
	294,
	244,
	14,
	294,
	244,
	14,
	294,
	244,
	14,
	294,
	244,
	13,
	294,
	244,
	10,
	294,
	244,
	11,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	13,
	294,
	244,
	13,
	294,
	244,
	10,
	294,
	244,
	11,
	294,
	244,
	12,
	294,
	244,
	12,
	294,
	244,
	11,
	294,
	244,
	10,
	294,
	244,
	9,
	294,
	244,
	8,
	294,
	244,
	7,
	294,
	244,
	6,
	294,
	244,
	5,
	294,
	245,
	4,
	294,
	245,
	3,
	294,
	245,
	2,
	294,
	245,
	1,
	294,
	244,
	0,
	0
};

uint16 sc38_lift_up[] = {
	101*64,
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
	SEND_SYNC,
	ID_FOSTER,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 spu_auto[32];

uint16 sniff_right[] = {
	74*64,
	257,
	219,
	0,
	257,
	219,
	1,
	257,
	219,
	2,
	257,
	219,
	3,
	257,
	219,
	4,
	257,
	219,
	5,
	257,
	219,
	4,
	257,
	219,
	5,
	257,
	219,
	5,
	257,
	219,
	6,
	257,
	219,
	6,
	257,
	219,
	5,
	257,
	219,
	5,
	257,
	219,
	6,
	257,
	219,
	6,
	257,
	219,
	7,
	257,
	219,
	7,
	257,
	219,
	6,
	257,
	219,
	5,
	257,
	219,
	4,
	257,
	219,
	4,
	257,
	219,
	5,
	257,
	219,
	5,
	257,
	219,
	3,
	257,
	219,
	4,
	257,
	219,
	5,
	257,
	219,
	6,
	257,
	219,
	7,
	257,
	219,
	6,
	257,
	219,
	5,
	257,
	219,
	3,
	257,
	219,
	2,
	257,
	219,
	1,
	257,
	219,
	0,
	0
};

uint16 dan_st_down[] = {
	55*64,
	1,
	0,
	32,
	0
};

uint16 sc38_ringer_anim[] = {
	129*64,
	161,
	261,
	0,
	161,
	261,
	1,
	161,
	261,
	2,
	161,
	261,
	3,
	161,
	261,
	4,
	161,
	261,
	5,
	161,
	261,
	4,
	161,
	261,
	3,
	161,
	261,
	2,
	161,
	261,
	6,
	161,
	261,
	0,
	161,
	261,
	0,
	161,
	261,
	0,
	161,
	261,
	0,
	161,
	261,
	0,
	161,
	261,
	0,
	0
};

uint32 *sc38_walk_grid = (uint32*)sc38_fast_list;

uint32 *dna = (uint32*)&danielle;

uint16 spu_st_right[] = {
	71*64,
	1,
	0,
	38,
	0
};

uint16 spu_l_to_d[] = {
	33+71*64,
	0
};

uint16 sc38_reach_food[] = {
	139*64,
	235,
	216,
	0,
	235,
	216,
	1,
	235,
	216,
	2,
	235,
	216,
	3,
	235,
	216,
	4,
	235,
	216,
	5,
	235,
	216,
	6,
	235,
	216,
	7,
	235,
	216,
	8,
	235,
	216,
	0,
	0
};

Compact sc38_ringer = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	161,	// xcood
	261,	// ycood
	129*64,	// frame
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
	SC38_RINGER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 spu_t_to_u[] = {
	38+71*64,
	37+71*64,
	0
};

Compact sc38_statue = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	38,	// screen
	0,	// place
	0,	// getToTable
	354,	// xcood
	199,	// ycood
	0,	// frame
	16620,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	366-354,	// mouseSize_x
	254-199,	// mouseSize_y
	SC38_STATUE_ACTION,	// actionScript
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

uint16 dan_d_to_u[] = {
	39+55*64,
	38+55*64,
	37+55*64,
	0
};

uint16 spu_d_to_l[] = {
	33+71*64,
	0
};

uint16 dan_u_to_l[] = {
	35+55*64,
	0
};

uint16 reset_spunky_38[] = {
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_COLLISION,
	C_SCREEN,
	38,
	C_PLACE,
	ID_SC38_FLOOR,
	C_XCOOD,
	248,
	C_YCOOD,
	256,
	C_FRAME,
	34+71*64,
	C_DIR,
	LEFT,
	C_MOUSE_CLICK,
	ADVISOR_188,
	65535
};

uint16 spu_right[] = {
	2,
	24+71*64,
	2,
	0,
	2,
	25+71*64,
	2,
	0,
	2,
	26+71*64,
	2,
	0,
	2,
	27+71*64,
	2,
	0,
	2,
	28+71*64,
	2,
	0,
	2,
	29+71*64,
	2,
	0,
	2,
	30+71*64,
	2,
	0,
	2,
	31+71*64,
	2,
	0,
	0
};

uint16 sex_l_to_d[] = {
	33+106*64,
	0
};

uint16 reset_dani_stand[] = {
	C_STATUS,
	ST_LOGIC+ST_SORT+ST_RECREATE+ST_GRID_PLOT+ST_COLLISION,
	C_XCOOD,
	328,
	C_YCOOD,
	288,
	C_FRAME,
	32+106*64,
	C_DIR,
	DOWN,
	C_MOUSE_CLICK,
	ADVISOR_188,
	C_MOUSE_REL_X,
	4,
	C_MOUSE_REL_Y,
	2,
	C_MOUSE_SIZE_X,
	20,
	C_MOUSE_SIZE_Y,
	54,
	C_ACTION_SCRIPT,
	SC38_DANIELLE_ACTION,
	C_MEGA_SET,
	NEXT_MEGA_SET,
	65535
};

uint16 dan_r_to_d[] = {
	39+55*64,
	0
};

uint16 spu_st_down[] = {
	71*64,
	1,
	0,
	32,
	0
};

uint16 dan_up[] = {
	2,
	0+55*64,
	0,
	65534,
	2,
	1+55*64,
	0,
	65534,
	2,
	2+55*64,
	0,
	65534,
	2,
	3+55*64,
	0,
	65534,
	2,
	4+55*64,
	0,
	65534,
	2,
	5+55*64,
	0,
	65534,
	2,
	6+55*64,
	0,
	65534,
	2,
	7+55*64,
	0,
	65534,
	0
};

uint16 dan_u_to_d[] = {
	35+55*64,
	34+55*64,
	33+55*64,
	0
};

uint16 dan_down[] = {
	2,
	8+55*64,
	0,
	2,
	2,
	9+55*64,
	0,
	2,
	2,
	10+55*64,
	0,
	2,
	2,
	11+55*64,
	0,
	2,
	2,
	12+55*64,
	0,
	2,
	2,
	13+55*64,
	0,
	2,
	2,
	14+55*64,
	0,
	2,
	2,
	15+55*64,
	0,
	2,
	0
};

uint16 dan_right[] = {
	4,
	24+55*64,
	4,
	0,
	4,
	25+55*64,
	4,
	0,
	4,
	26+55*64,
	4,
	0,
	4,
	27+55*64,
	4,
	0,
	4,
	28+55*64,
	4,
	0,
	4,
	29+55*64,
	4,
	0,
	4,
	30+55*64,
	4,
	0,
	4,
	31+55*64,
	4,
	0,
	0
};

uint16 dan_t_to_u[] = {
	38+55*64,
	37+55*64,
	0
};

uint16 sex_down[] = {
	2,
	8+106*64,
	0,
	2,
	2,
	9+106*64,
	0,
	2,
	2,
	10+106*64,
	0,
	2,
	2,
	11+106*64,
	0,
	2,
	2,
	12+106*64,
	0,
	2,
	2,
	13+106*64,
	0,
	2,
	2,
	14+106*64,
	0,
	2,
	2,
	15+106*64,
	0,
	2,
	0
};

uint16 sex_st_up[] = {
	106*64,
	1,
	0,
	36,
	0
};

uint16 sex_st_left[] = {
	106*64,
	1,
	0,
	34,
	0
};

uint16 sex_u_to_l[] = {
	35+106*64,
	0
};

TurnTable danielle_turnTable0 = {
	{ // turnTableUp
		0,
		dan_u_to_d,
		dan_u_to_l,
		dan_u_to_r,
		dan_u_to_t
	},
	{ // turnTableDown
		dan_d_to_u,
		0,
		dan_d_to_l,
		dan_d_to_r,
		0
	},
	{ // turnTableLeft
		dan_l_to_u,
		dan_l_to_d,
		0,
		dan_l_to_r,
		0
	},
	{ // turnTableRight
		dan_r_to_u,
		dan_r_to_d,
		dan_r_to_l,
		0,
		dan_r_to_t
	},
	{ // turnTableTalk
		dan_t_to_u,
		0,
		0,
		dan_t_to_r,
		0
	},
};

TurnTable danielle_turnTable1 = {
	{ // turnTableUp
		0,
		sex_u_to_d,
		sex_u_to_l,
		sex_u_to_r,
		sex_u_to_t
	},
	{ // turnTableDown
		sex_d_to_u,
		0,
		sex_d_to_l,
		sex_d_to_r,
		0
	},
	{ // turnTableLeft
		sex_l_to_u,
		sex_l_to_d,
		0,
		sex_l_to_r,
		0
	},
	{ // turnTableRight
		sex_r_to_u,
		sex_r_to_d,
		sex_r_to_l,
		0,
		sex_r_to_t
	},
	{ // turnTableTalk
		sex_t_to_u,
		0,
		0,
		sex_t_to_r,
		0
	}
};

MegaSet danielle_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	dan_up,	// animUp
	dan_down,	// animDown
	dan_left,	// animLeft
	dan_right,	// animRight
	dan_st_up,	// standUp
	dan_st_down,	// standDown
	dan_st_left,	// standLeft
	dan_st_right,	// standRight
	dan_st_talk,	// standTalk
	&danielle_turnTable0,
};

MegaSet danielle_megaSet1 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	sex_up,	// animUp
	sex_down,	// animDown
	sex_left,	// animLeft
	sex_right,	// animRight
	sex_st_up,	// standUp
	sex_st_down,	// standDown
	sex_st_left,	// standLeft
	sex_st_right,	// standRight
	sex_st_talk,	// standTalk
	&danielle_turnTable1
};

ExtCompact danielle_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	RIGHT,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_DAN_COL,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	dan_auto,	// animScratch
	0,	// megaSet
	&danielle_megaSet0,
	&danielle_megaSet1,
	0,
	0
};

Compact danielle = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_SORT+ST_RECREATE+ST_GRID_PLOT+ST_COLLISION,	// status
	0,	// sync
	31,	// screen
	ID_SC31_FLOOR,	// place
	0,	// getToTable
	352,	// xcood
	248,	// ycood
	38+55*64,	// frame
	54,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65528,	// mouseRel_x
	(int16) 65493,	// mouseRel_y
	14,	// mouseSize_x
	50,	// mouseSize_y
	MEGA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DANIELLE_LOGIC,	// baseSub
	0,	// baseSub_off
	&danielle_ext
};

uint16 spu_d_to_r[] = {
	39+71*64,
	0
};

uint16 spu_down[] = {
	1,
	8+71*64,
	0,
	1,
	1,
	9+71*64,
	0,
	1,
	1,
	10+71*64,
	0,
	1,
	1,
	11+71*64,
	0,
	1,
	1,
	12+71*64,
	0,
	1,
	1,
	13+71*64,
	0,
	1,
	1,
	14+71*64,
	0,
	1,
	1,
	15+71*64,
	0,
	1,
	0
};

uint16 spu_st_up[] = {
	71*64,
	1,
	0,
	36,
	0
};

TurnTable spunky_turnTable0 = {
	{ // turnTableUp
		0,
		spu_u_to_d,
		spu_u_to_l,
		spu_u_to_r,
		spu_u_to_t
	},
	{ // turnTableDown
		spu_d_to_u,
		0,
		spu_d_to_l,
		spu_d_to_r,
		0
	},
	{ // turnTableLeft
		spu_l_to_u,
		spu_l_to_d,
		0,
		spu_l_to_r,
		spu_l_to_t
	},
	{ // turnTableRight
		spu_r_to_u,
		spu_r_to_d,
		spu_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		spu_t_to_u,
		0,
		spu_t_to_l,
		0,
		0
	}
};

MegaSet spunky_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	spu_up,	// animUp
	spu_down,	// animDown
	spu_left,	// animLeft
	spu_right,	// animRight
	spu_st_up,	// standUp
	spu_st_down,	// standDown
	spu_st_left,	// standLeft
	spu_st_right,	// standRight
	spu_st_talk,	// standTalk
	&spunky_turnTable0
};

ExtCompact spunky_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	RIGHT,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	7,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	spu_auto,	// animScratch
	0,	// megaSet
	&spunky_megaSet0,
	0,
	0,
	0
};

Compact spunky = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_SORT+ST_RECREATE+ST_GRID_PLOT+ST_COLLISION,	// status
	0,	// sync
	31,	// screen
	ID_SC31_FLOOR,	// place
	0,	// getToTable
	312,	// xcood
	240,	// ycood
	38+71*64,	// frame
	16614,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65528,	// mouseRel_x
	(int16) 65493,	// mouseRel_y
	14,	// mouseSize_x
	50,	// mouseSize_y
	MEGA_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SPUNKY_LOGIC,	// baseSub
	0,	// baseSub_off
	&spunky_ext
};

uint16 sc38_video_anim[] = {
	133*64,
	376,
	252,
	0,
	376,
	252,
	1,
	376,
	252,
	2,
	376,
	252,
	3,
	376,
	252,
	4,
	376,
	252,
	5,
	376,
	252,
	6,
	0
};

uint16 sc38_screen_1[] = {
	134*64,
	379,
	209,
	0,
	379,
	209,
	1,
	379,
	209,
	2,
	379,
	209,
	3,
	379,
	209,
	4,
	379,
	209,
	5,
	379,
	209,
	6,
	379,
	209,
	7,
	379,
	209,
	8,
	379,
	209,
	9,
	379,
	209,
	10,
	379,
	209,
	11,
	379,
	209,
	12,
	379,
	209,
	13,
	379,
	209,
	14,
	379,
	209,
	15,
	379,
	209,
	16,
	379,
	209,
	17,
	379,
	209,
	18,
	379,
	209,
	19,
	379,
	209,
	20,
	379,
	209,
	21,
	379,
	209,
	22,
	379,
	209,
	23,
	379,
	209,
	24,
	379,
	209,
	25,
	379,
	209,
	26,
	379,
	209,
	27,
	379,
	209,
	28,
	379,
	209,
	29,
	379,
	209,
	30,
	379,
	209,
	31,
	379,
	209,
	32,
	379,
	209,
	33,
	379,
	209,
	34,
	379,
	209,
	35,
	379,
	209,
	36,
	379,
	209,
	37,
	379,
	209,
	38,
	379,
	209,
	28,
	379,
	209,
	29,
	379,
	209,
	30,
	379,
	209,
	31,
	379,
	209,
	32,
	379,
	209,
	33,
	379,
	209,
	34,
	379,
	209,
	35,
	379,
	209,
	36,
	379,
	209,
	37,
	379,
	209,
	38,
	379,
	209,
	39,
	379,
	209,
	40,
	379,
	209,
	41,
	379,
	209,
	42,
	379,
	209,
	43,
	379,
	209,
	44,
	379,
	209,
	45,
	0
};

uint16 sc38_lift_down[] = {
	101*64,
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
	SEND_SYNC,
	ID_FOSTER,
	1,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
