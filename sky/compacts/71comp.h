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

#ifndef SKY71COMP_H
#define SKY71COMP_H




namespace Sky {

namespace SkyCompact {

uint16 medi_d_to_u[] = {
	35+32*64,
	24+32*64,
	34+32*64,
	0
};

uint16 sc71_use_board[] = {
	148*64,
	243,
	191,
	0,
	243,
	191,
	1,
	243,
	191,
	2,
	243,
	191,
	3,
	243,
	191,
	4,
	243,
	191,
	5,
	243,
	191,
	6,
	243,
	191,
	7,
	243,
	191,
	8,
	243,
	191,
	9,
	243,
	191,
	9,
	243,
	191,
	9,
	243,
	191,
	9,
	243,
	191,
	8,
	243,
	191,
	7,
	243,
	191,
	6,
	243,
	191,
	3,
	243,
	191,
	2,
	243,
	191,
	1,
	243,
	191,
	0,
	0
};

Compact sc71_door72 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	194,	// xcood
	219,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	208-194,	// mouseSize_x
	280-219,	// mouseSize_y
	SC71_DOOR72_ACTION,	// actionScript
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

uint16 sc71_medi_charge[] = {
	166*64,
	279,
	196,
	0,
	279,
	196,
	1,
	279,
	196,
	2,
	279,
	196,
	3,
	279,
	196,
	4,
	279,
	196,
	5,
	279,
	196,
	6,
	279,
	196,
	7,
	279,
	196,
	8,
	279,
	196,
	9,
	0
};

uint16 medi_auto[32];

uint16 medi_up[] = {
	1,
	0+32*64,
	0,
	65535,
	1,
	1+32*64,
	0,
	65535,
	1,
	2+32*64,
	0,
	65535,
	1,
	3+32*64,
	0,
	65535,
	1,
	4+32*64,
	0,
	65535,
	1,
	5+32*64,
	0,
	65535,
	1,
	6+32*64,
	0,
	65535,
	1,
	7+32*64,
	0,
	65535,
	0
};

uint16 medi_down[] = {
	1,
	8+32*64,
	0,
	1,
	1,
	9+32*64,
	0,
	1,
	1,
	10+32*64,
	0,
	1,
	1,
	11+32*64,
	0,
	1,
	1,
	12+32*64,
	0,
	1,
	1,
	13+32*64,
	0,
	1,
	1,
	14+32*64,
	0,
	1,
	1,
	15+32*64,
	0,
	1,
	0
};

uint16 medi_left[] = {
	2,
	16+32*64,
	65534,
	0,
	2,
	17+32*64,
	65534,
	0,
	2,
	18+32*64,
	65534,
	0,
	2,
	19+32*64,
	65534,
	0,
	2,
	20+32*64,
	65534,
	0,
	2,
	21+32*64,
	65534,
	0,
	2,
	22+32*64,
	65534,
	0,
	2,
	23+32*64,
	65534,
	0,
	0
};

uint16 medi_right[] = {
	2,
	24+32*64,
	2,
	0,
	2,
	25+32*64,
	2,
	0,
	2,
	26+32*64,
	2,
	0,
	2,
	27+32*64,
	2,
	0,
	2,
	28+32*64,
	2,
	0,
	2,
	29+32*64,
	2,
	0,
	2,
	30+32*64,
	2,
	0,
	2,
	31+32*64,
	2,
	0,
	0
};

uint16 medi_st_up[] = {
	32*64,
	1,
	1,
	0,
	0
};

uint16 medi_st_down[] = {
	32*64,
	1,
	1,
	8,
	0
};

uint16 medi_st_left[] = {
	32*64,
	1,
	1,
	16,
	0
};

uint16 medi_st_right[] = {
	32*64,
	1,
	1,
	24,
	0
};

uint16 medi_u_to_d[] = {
	33+32*64,
	16+32*64,
	32+32*64,
	0
};

uint16 medi_u_to_l[] = {
	33+32*64,
	0
};

uint16 medi_u_to_r[] = {
	34+32*64,
	0
};

uint16 medi_d_to_l[] = {
	32+32*64,
	0
};

uint16 medi_d_to_r[] = {
	35+32*64,
	0
};

uint16 medi_l_to_u[] = {
	33+32*64,
	0
};

uint16 medi_l_to_d[] = {
	32+32*64,
	0
};

uint16 medi_l_to_r[] = {
	32+32*64,
	8+32*64,
	35+32*64,
	0
};

uint16 medi_r_to_u[] = {
	34+32*64,
	0
};

uint16 medi_r_to_d[] = {
	35+32*64,
	0
};

uint16 medi_r_to_l[] = {
	34+32*64,
	0+32*64,
	33+32*64,
	0
};

TurnTable medi_turnTable0 = {
	{ // turnTableUp
		0,
		medi_u_to_d,
		medi_u_to_l,
		medi_u_to_r,
		0
	},
	{ // turnTableDown
		medi_d_to_u,
		0,
		medi_d_to_l,
		medi_d_to_r,
		0
	},
	{ // turnTableLeft
		medi_l_to_u,
		medi_l_to_d,
		0,
		medi_l_to_r,
		0
	},
	{ // turnTableRight
		medi_r_to_u,
		medi_r_to_d,
		medi_r_to_l,
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

MegaSet medi_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	medi_up,	// animUp
	medi_down,	// animDown
	medi_left,	// animLeft
	medi_right,	// animRight
	medi_st_up,	// standUp
	medi_st_down,	// standDown
	medi_st_left,	// standLeft
	medi_st_right,	// standRight
	0,	// standTalk
	&medi_turnTable0
};

ExtCompact medi_ext = {
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
	SP_COL_MEDI,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	medi_auto,	// animScratch
	0,	// megaSet
	&medi_megaSet0,
	0,
	0,
	0
};

Compact medi = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_COLLISION+ST_MOUSE,	// status
	0,	// sync
	69,	// screen
	ID_SC69_FLOOR,	// place
	0,	// getToTable
	128,	// xcood
	280,	// ycood
	16+32*64,	// frame
	20527,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	(int16) 65529,	// mouseRel_x
	(int16) 65504,	// mouseRel_y
	12,	// mouseSize_x
	29,	// mouseSize_y
	MEDI_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	MEDI_LOGIC,	// baseSub
	0,	// baseSub_off
	&medi_ext
};

uint16 sc71_mouse_list[] = {
	ID_SC71_MEDI_SLOT,
	ID_MEDI,
	ID_SC71_RECHARGER,
	ID_SC71_MONITOR,
	ID_SC71_CONTROLS,
	ID_SC71_DOOR69,
	ID_SC71_DOOR72,
	ID_SC71_LOCKED_DOOR,
	ID_SC71_FLOOR,
	ID_SC71_FAKE_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc71_light1_anim[] = {
	162*64,
	249,
	197,
	0,
	249,
	197,
	1,
	249,
	197,
	1,
	249,
	197,
	2,
	249,
	197,
	2,
	249,
	197,
	3,
	249,
	197,
	4,
	249,
	197,
	4,
	249,
	197,
	5,
	249,
	197,
	5,
	0
};

uint16 sc71_logic_list[] = {
	ID_FOSTER,
	ID_MEDI,
	ID_WITNESS,
	ID_SC67_DOOR,
	ID_SC68_DOOR,
	ID_SC71_LIGHT1,
	ID_SC71_CONTROLS,
	ID_SC71_CHLITE,
	ID_SC71_MONITOR,
	ID_SC71_RECHARGER,
	ID_SC71_PANEL2,
	ID_SC72_SPILL,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 reset_69_71[] = {
	C_SCREEN,
	71,
	C_PLACE,
	ID_SC71_FLOOR,
	C_XCOOD,
	152,
	C_YCOOD,
	288,
	65535
};

uint16 sc71_chip_list[] = {
	IT_SC71_LAYER_0+DISK_5,
	IT_SC71_LAYER_1+DISK_5,
	IT_SC71_LAYER_2+DISK_5,
	IT_SC71_GRID_1+DISK_5,
	IT_SC71_GRID_2+DISK_5,
	IT_MEDI+DISK_5,
	IT_MEDI_TALK+DISK_5,
	IT_SC71_LIGHT1+DISK_5,
	IT_SC71_LIGHT2+DISK_5,
	IT_SC71_PANEL+DISK_5,
	IT_SC71_PANEL2+DISK_5,
	IT_SC71_CHARGE_LIGHT+DISK_5,
	IT_SC71_SCREEN+DISK_5,
	IT_SC71_MEDI_CHARGE+DISK_5,
	IT_SC71_USE_BOARD+DISK_5,
	0
};

uint16 sc710_chip_list[] = {
	IT_SC710_LAYER_0+DISK_5,
	IT_SC710_LAYER_1+DISK_5,
	IT_SC710_LAYER_2+DISK_5,
	IT_SC710_GRID_1+DISK_5,
	IT_SC710_GRID_2+DISK_5,
	IT_MEDI+DISK_5,
	IT_MEDI_TALK+DISK_5,
	IT_SC71_LIGHT1+DISK_5,
	IT_SC71_LIGHT2+DISK_5,
	IT_SC71_PANEL+DISK_5,
	IT_SC71_PANEL2+DISK_5,
	IT_SC71_CHARGE_LIGHT+DISK_5,
	IT_SC71_SCREEN+DISK_5,
	IT_SC71_MEDI_CHARGE+DISK_5,
	0
};

uint16 reset_72_71[] = {
	C_SCREEN,
	71,
	C_PLACE,
	ID_SC71_FLOOR,
	C_XCOOD,
	176,
	C_YCOOD,
	264,
	65535
};

uint16 sc71_medi_get_up[] = {
	166*64,
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

Compact sc71_chlite = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	268,	// xcood
	197,	// ycood
	165*64,	// frame
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
	SC71_CHLITE_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc71_controls = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	372,	// xcood
	270,	// ycood
	163*64,	// frame
	20551,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	(int16) 65533,	// mouseRel_y
	380-370,	// mouseSize_x
	283-267,	// mouseSize_y
	SC71_CONTROLS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC71_CONTROLS_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc71_panel_anim[] = {
	167*64,
	272,
	226,
	0,
	272,
	226,
	1,
	272,
	226,
	2,
	272,
	226,
	3,
	272,
	226,
	4,
	272,
	226,
	5,
	272,
	226,
	6,
	272,
	226,
	7,
	272,
	226,
	8,
	272,
	226,
	9,
	272,
	226,
	10,
	272,
	226,
	11,
	0
};

Compact sc71_panel2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	287,	// xcood
	226,	// ycood
	168*64,	// frame
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
	SC71_PANEL2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc71_panel2_anim[] = {
	168*64,
	287,
	226,
	0,
	287,
	226,
	1,
	287,
	226,
	2,
	287,
	226,
	3,
	287,
	226,
	4,
	287,
	226,
	5,
	287,
	226,
	6,
	287,
	226,
	7,
	287,
	226,
	8,
	287,
	226,
	9,
	287,
	226,
	10,
	287,
	226,
	11,
	0
};

uint16 sc71_joey_list[] = {
	184,
	399,
	272,
	327,
	1,
	184,
	215,
	272,
	279,
	0,
	328,
	447,
	280,
	287,
	0,
	336,
	447,
	304,
	311,
	0,
	0
};

Compact sc71_light1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	249,	// xcood
	197,	// ycood
	162*64,	// frame
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
	SC71_LIGHT1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc71_palette[] = {
	0,
	512,
	514,
	1285,
	2053,
	2056,
	2315,
	2570,
	3082,
	3083,
	3342,
	3084,
	3085,
	3854,
	3596,
	3596,
	3342,
	4109,
	3599,
	3856,
	4623,
	3858,
	4113,
	4113,
	4113,
	3859,
	4882,
	4369,
	4629,
	4626,
	5137,
	7700,
	3085,
	5396,
	5142,
	5141,
	4132,
	10766,
	4371,
	5396,
	5656,
	5654,
	5910,
	6936,
	5398,
	6166,
	6170,
	6168,
	6168,
	5659,
	6426,
	6682,
	6162,
	6939,
	7197,
	6928,
	6682,
	6682,
	7197,
	5916,
	7448,
	6686,
	7197,
	7965,
	6929,
	7964,
	7712,
	7440,
	5919,
	7453,
	8221,
	4640,
	7708,
	7969,
	7965,
	8221,
	7967,
	7455,
	8482,
	7444,
	8737,
	7968,
	8225,
	6435,
	8741,
	8211,
	8993,
	8479,
	7718,
	9251,
	8996,
	8730,
	8482,
	9255,
	8465,
	9763,
	8739,
	7972,
	10021,
	9505,
	10793,
	4391,
	9507,
	9764,
	9252,
	9507,
	10023,
	6185,
	9766,
	9254,
	10790,
	10276,
	11047,
	4906,
	10282,
	9757,
	8489,
	10278,
	9257,
	10538,
	10029,
	10274,
	11048,
	11056,
	9746,
	11050,
	11566,
	9750,
	11819,
	11566,
	10522,
	11562,
	10795,
	13099,
	4653,
	11053,
	10792,
	9518,
	11560,
	11054,
	11564,
	12085,
	10770,
	12333,
	12340,
	12823,
	9517,
	12076,
	12334,
	11053,
	12335,
	11814,
	12334,
	12331,
	13617,
	5683,
	12080,
	12078,
	12848,
	12845,
	13106,
	10290,
	12848,
	12848,
	5142,
	12847,
	12852,
	12849,
	14135,
	13853,
	9014,
	13106,
	12340,
	13876,
	13364,
	12854,
	13878,
	14135,
	13356,
	14390,
	13878,
	13878,
	14392,
	14647,
	14651,
	14649,
	15161,
	15163,
	15675,
	15677,
	16190,
	16191,
	0,
	16128,
	16191,
	14647,
	12347,
	13876,
	11046,
	8494,
	10533,
	3358,
	9228,
	3600,
	4906,
	12817,
	5142,
	6682,
	7442,
	4383,
	10535,
	13592,
	4655,
	14135,
	14109,
	11319,
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
	15107,
	9007,
	10554,
	13593,
	4641,
	5936,
	10508,
	1807,
	2082,
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
	5141,
	6165,
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

uint16 sc71_fast_list[] = {
	12+DISK_5,
	51+DISK_5,
	52+DISK_5,
	53+DISK_5,
	54+DISK_5,
	268+DISK_5,
	0
};

uint16 sc71_mon_anim[] = {
	164*64,
	372,
	248,
	0,
	372,
	248,
	1,
	372,
	248,
	2,
	372,
	248,
	3,
	372,
	248,
	4,
	372,
	248,
	5,
	0
};

uint16 sc71_floor_table[] = {
	ID_SC71_FLOOR,
	RET_OK,
	0,
	SC71_DOOR69_WALK_ON,
	3,
	SC71_DOOR72_WALK_ON,
	1,
	GET_INTO_RECHARGING_UNIT,
	ID_SC71_DOOR69,
	GT_SC71_DOOR69,
	ID_SC71_DOOR72,
	GT_SC71_DOOR72,
	ID_SC71_LOCKED_DOOR,
	GT_SC71_LOCKED_DOOR,
	ID_SC71_RECHARGER,
	GT_SC71_RECHARGER,
	ID_SC71_MONITOR,
	GT_SC71_MONITOR,
	ID_SC71_CONTROLS,
	GT_SC71_CONTROLS,
	ID_MEDI,
	GT_RECHARGING_MEDI,
	ID_SC71_MEDI_SLOT,
	GT_SC71_MEDI_SLOT,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	65535
};

Compact sc71_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	sc71_floor_table,	// getToTable
	194,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	395-194,	// mouseSize_x
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

Compact sc71_door69 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	163,	// xcood
	230,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	178-163,	// mouseSize_x
	296-230,	// mouseSize_y
	SC71_DOOR69_ACTION,	// actionScript
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

uint32 *grid71 = 0;

Compact sc71_fake_floor = {
	0,	// logic
	0,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	136,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-128,	// mouseSize_x
	327-136,	// mouseSize_y
	0,	// actionScript
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

Compact sc71_medi_slot = {
	0,	// logic
	0,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	275,	// xcood
	242,	// ycood
	0,	// frame
	50,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	280-275,	// mouseSize_x
	247-242,	// mouseSize_y
	SC71_MEDI_SLOT_ACTION,	// actionScript
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

Compact sc71_monitor = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	372,	// xcood
	248,	// ycood
	164*64,	// frame
	57,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	380-370,	// mouseSize_x
	266-248,	// mouseSize_y
	SC71_MONITOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC71_MONITOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc71_chlite_anim[] = {
	165*64,
	268,
	197,
	0,
	268,
	197,
	0,
	268,
	197,
	1,
	268,
	197,
	1,
	268,
	197,
	2,
	268,
	197,
	2,
	268,
	197,
	3,
	268,
	197,
	3,
	268,
	197,
	4,
	268,
	197,
	4,
	268,
	197,
	5,
	268,
	197,
	5,
	0
};

Compact sc71_locked_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	353,	// xcood
	222,	// ycood
	0,	// frame
	181,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	362-353,	// mouseSize_x
	280-222,	// mouseSize_y
	SC71_LOCKED_DOOR_ACTION,	// actionScript
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

Compact sc71_recharger = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	71,	// screen
	0,	// place
	0,	// getToTable
	272,	// xcood
	226,	// ycood
	167*64,	// frame
	20549,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	300-268,	// mouseSize_x
	231-224,	// mouseSize_y
	SC71_RECHARGER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC71_RECHARGER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc71_light2_anim[] = {
	163*64,
	372,
	270,
	0,
	372,
	270,
	1,
	372,
	270,
	2,
	372,
	270,
	3,
	372,
	270,
	4,
	372,
	270,
	5,
	372,
	270,
	6,
	372,
	270,
	7,
	372,
	270,
	8,
	372,
	270,
	9,
	372,
	270,
	10,
	372,
	270,
	11,
	372,
	270,
	12,
	372,
	270,
	13,
	372,
	270,
	14,
	372,
	270,
	15,
	372,
	270,
	16,
	372,
	270,
	17,
	372,
	270,
	18,
	372,
	270,
	19,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
