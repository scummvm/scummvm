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

#ifndef SKY69COMP_H
#define SKY69COMP_H




namespace Sky {

namespace SkyCompact {

uint32 *grid69 = 0;

uint16 sc69_logic_list[] = {
	ID_FOSTER,
	ID_MEDI,
	ID_SC67_DOOR,
	ID_SC68_DOOR,
	ID_SC69_PULSE1,
	ID_SC69_PULSE2,
	ID_SC69_PULSE3,
	ID_SC69_PULSE4,
	ID_SC69_PULSE5,
	ID_SC69_PULSE6,
	ID_SC71_LIGHT1,
	ID_SC71_CONTROLS,
	ID_SC71_CHLITE,
	ID_SC71_MONITOR,
	ID_SC71_RECHARGER,
	ID_SC71_PANEL2,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc69_pulse5_anim[] = {
	113*64,
	212,
	181,
	0,
	212,
	181,
	1,
	212,
	181,
	2,
	212,
	181,
	2,
	212,
	181,
	1,
	212,
	181,
	1,
	212,
	181,
	1,
	212,
	181,
	1,
	212,
	181,
	2,
	212,
	181,
	2,
	212,
	181,
	2,
	212,
	181,
	2,
	212,
	181,
	2,
	212,
	181,
	1,
	212,
	181,
	0,
	212,
	181,
	0,
	212,
	181,
	0,
	0
};

uint16 sc69_chip_list[] = {
	IT_SC69_LAYER_0+DISK_5,
	IT_SC69_LAYER_1+DISK_5,
	IT_SC69_LAYER_2+DISK_5,
	IT_SC69_GRID_1+DISK_5,
	IT_SC69_GRID_2+DISK_5,
	IT_MEDI+DISK_5,
	IT_MEDI_TALK+DISK_5,
	IT_SC69_PULSE1+DISK_5,
	IT_SC69_PULSE2+DISK_5,
	IT_SC69_PULSE3+DISK_5,
	IT_SC69_PULSE4+DISK_5,
	IT_SC69_PULSE5+DISK_5,
	IT_SC69_PULSE6+DISK_5,
	0
};

uint16 sc69_palette[] = {
	0,
	1024,
	514,
	1028,
	1796,
	771,
	1286,
	1284,
	1542,
	1288,
	2821,
	1029,
	1799,
	2312,
	1544,
	1802,
	2057,
	2057,
	1804,
	3846,
	1286,
	2312,
	3594,
	1288,
	2570,
	3848,
	1800,
	2570,
	3084,
	2569,
	2317,
	4364,
	1546,
	3084,
	5642,
	1543,
	2829,
	4621,
	2313,
	2830,
	4363,
	2059,
	2831,
	3342,
	3085,
	1817,
	4870,
	1804,
	3343,
	7438,
	1543,
	3347,
	6921,
	2056,
	2585,
	6151,
	2570,
	3601,
	5389,
	2062,
	4112,
	4364,
	4110,
	2080,
	5638,
	2829,
	2589,
	6919,
	2315,
	3604,
	4624,
	3600,
	2083,
	5638,
	2576,
	2846,
	9738,
	1544,
	3355,
	5132,
	4113,
	4121,
	6153,
	3089,
	4627,
	8467,
	2572,
	4374,
	6930,
	2832,
	3613,
	9998,
	2058,
	2853,
	8970,
	3085,
	4633,
	6418,
	3603,
	4637,
	5642,
	5139,
	2602,
	8200,
	2321,
	4127,
	8719,
	3599,
	3621,
	6410,
	5140,
	5148,
	9999,
	3086,
	3115,
	7690,
	3092,
	5655,
	6676,
	3862,
	4898,
	7690,
	3606,
	5659,
	9236,
	4114,
	5409,
	10509,
	3088,
	3628,
	9228,
	2837,
	6169,
	8472,
	4118,
	4139,
	6926,
	5656,
	4648,
	9743,
	4627,
	6175,
	7696,
	5656,
	5669,
	9744,
	3351,
	4906,
	7441,
	6425,
	6434,
	9489,
	4632,
	6688,
	10006,
	4120,
	5419,
	11795,
	4628,
	5929,
	8982,
	5915,
	7455,
	8731,
	5405,
	6952,
	9746,
	6172,
	6444,
	8727,
	7198,
	7466,
	9747,
	6175,
	7972,
	10526,
	5918,
	8232,
	10522,
	5921,
	8747,
	10265,
	7971,
	9003,
	11548,
	6436,
	9513,
	10274,
	10023,
	11053,
	13867,
	10031,
	514,
	5634,
	1543,
	2080,
	6918,
	2315,
	2083,
	7686,
	2571,
	3105,
	8714,
	3599,
	3623,
	11020,
	2572,
	4139,
	10254,
	3858,
	5419,
	11283,
	5913,
	7721,
	11031,
	6434,
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
	3093,
	2569,
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

uint16 reset_71_69[] = {
	C_SCREEN,
	69,
	C_PLACE,
	ID_SC69_FLOOR,
	C_XCOOD,
	264,
	C_YCOOD,
	272,
	65535
};

uint16 sc69_mouse_list[] = {
	ID_MEDI,
	ID_SC69_GRILL,
	ID_SC69_EXIT,
	ID_SC69_DOOR,
	ID_SC69_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc69_pulse2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	315,	// xcood
	185,	// ycood
	110*64,	// frame
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
	SC69_PULSE2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc69_floor_table[] = {
	ID_SC69_FLOOR,
	RET_OK,
	0,
	SC69_EXIT_WALK_ON,
	1,
	SC69_DOOR_WALK_ON,
	ID_SC69_EXIT,
	GT_SC69_EXIT,
	ID_SC69_DOOR,
	GT_SC69_DOOR,
	ID_SC69_GRILL,
	GT_SC69_GRILL,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	65535
};

Compact sc69_pulse6 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	136,	// xcood
	190,	// ycood
	114*64,	// frame
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
	SC69_PULSE6_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc69_joey_list[] = {
	136,
	439,
	264,
	327,
	1,
	200,
	447,
	264,
	279,
	0,
	296,
	447,
	288,
	295,
	0,
	0
};

Compact sc69_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	220,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	27,	// mouseSize_x
	107,	// mouseSize_y
	SC69_EXIT_ACTION,	// actionScript
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

uint16 sc69_fast_list[] = {
	12+DISK_5,
	51+DISK_5,
	52+DISK_5,
	53+DISK_5,
	54+DISK_5,
	268+DISK_5,
	0
};

Compact sc69_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	240,	// xcood
	220,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	32,	// mouseSize_x
	61,	// mouseSize_y
	SC69_DOOR_ACTION,	// actionScript
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

Compact sc69_pulse3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	298,	// xcood
	199,	// ycood
	111*64,	// frame
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
	SC69_PULSE3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc69_pulse5 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	212,	// xcood
	181,	// ycood
	113*64,	// frame
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
	SC69_PULSE5_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc69_pulse6_anim[] = {
	114*64,
	135,
	190,
	0,
	135,
	190,
	0,
	135,
	190,
	1,
	135,
	190,
	2,
	135,
	190,
	2,
	135,
	190,
	1,
	135,
	190,
	1,
	135,
	190,
	1,
	135,
	190,
	1,
	135,
	190,
	2,
	135,
	190,
	2,
	135,
	190,
	2,
	135,
	190,
	2,
	135,
	190,
	2,
	135,
	190,
	1,
	135,
	190,
	0,
	135,
	190,
	0,
	0
};

Compact sc69_pulse4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	255,	// xcood
	189,	// ycood
	112*64,	// frame
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
	SC69_PULSE4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc69_pulse1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	370,	// xcood
	210,	// ycood
	109*64,	// frame
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
	SC69_PULSE1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc69_pulse4_anim[] = {
	112*64,
	255,
	189,
	0,
	255,
	189,
	1,
	255,
	189,
	1,
	255,
	189,
	0,
	255,
	189,
	0,
	255,
	189,
	0,
	255,
	189,
	0,
	255,
	189,
	1,
	255,
	189,
	1,
	255,
	189,
	1,
	255,
	189,
	1,
	255,
	189,
	1,
	255,
	189,
	0,
	255,
	189,
	2,
	255,
	189,
	2,
	255,
	189,
	2,
	255,
	189,
	2,
	0
};

Compact sc69_grill = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	69,	// screen
	0,	// place
	0,	// getToTable
	306,	// xcood
	232,	// ycood
	0,	// frame
	20511,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	331-306,	// mouseSize_x
	251-232,	// mouseSize_y
	SC69_GRILL_ACTION,	// actionScript
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

uint16 sc69_pulse1_anim[] = {
	109*64,
	370,
	210,
	0,
	370,
	210,
	1,
	370,
	210,
	1,
	370,
	210,
	1,
	370,
	210,
	1,
	370,
	210,
	0,
	370,
	210,
	0,
	370,
	210,
	0,
	370,
	210,
	0,
	370,
	210,
	0,
	370,
	210,
	1,
	370,
	210,
	2,
	370,
	210,
	2,
	370,
	210,
	2,
	370,
	210,
	2,
	370,
	210,
	1,
	370,
	210,
	0,
	0
};

uint16 sc69_pulse3_anim[] = {
	111*64,
	298,
	199,
	0,
	298,
	199,
	0,
	298,
	199,
	1,
	298,
	199,
	1,
	298,
	199,
	1,
	298,
	199,
	1,
	298,
	199,
	0,
	298,
	199,
	0,
	298,
	199,
	0,
	298,
	199,
	0,
	298,
	199,
	0,
	298,
	199,
	1,
	298,
	199,
	2,
	298,
	199,
	2,
	298,
	199,
	2,
	298,
	199,
	2,
	298,
	199,
	1,
	0
};

uint16 reset_68_69[] = {
	C_SCREEN,
	69,
	C_PLACE,
	ID_SC69_FLOOR,
	C_XCOOD,
	OFF_LEFT,
	C_YCOOD,
	280,
	65535
};

uint16 sc69_pulse2_anim[] = {
	110*64,
	315,
	185,
	0,
	315,
	185,
	0,
	315,
	185,
	1,
	315,
	185,
	1,
	315,
	185,
	1,
	315,
	185,
	1,
	315,
	185,
	0,
	315,
	185,
	0,
	315,
	185,
	0,
	315,
	185,
	0,
	315,
	185,
	0,
	315,
	185,
	1,
	315,
	185,
	2,
	315,
	185,
	2,
	315,
	185,
	2,
	315,
	185,
	2,
	315,
	185,
	1,
	0
};

Compact sc69_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	69,	// screen
	0,	// place
	sc69_floor_table,	// getToTable
	128,	// xcood
	268,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	431-128,	// mouseSize_x
	327-268,	// mouseSize_y
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
