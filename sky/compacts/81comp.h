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

#ifndef SKY81COMP_H
#define SKY81COMP_H




namespace Sky {

namespace SkyCompact {

Compact sc81_big_tent3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	257,	// xcood
	239,	// ycood
	189*64,	// frame
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
	SC81_BIG_TENT3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc81_fos_squirm[] = {
	202*64,
	255,
	212,
	0,
	255,
	212,
	1,
	255,
	212,
	2,
	255,
	212,
	3,
	255,
	212,
	4,
	255,
	212,
	5,
	255,
	212,
	5,
	255,
	212,
	3,
	255,
	212,
	3,
	255,
	212,
	6,
	255,
	212,
	7,
	255,
	212,
	6,
	255,
	212,
	3,
	255,
	212,
	6,
	255,
	212,
	7,
	255,
	212,
	6,
	255,
	212,
	3,
	255,
	212,
	2,
	255,
	212,
	4,
	255,
	212,
	5,
	255,
	212,
	4,
	255,
	212,
	3,
	255,
	212,
	6,
	255,
	212,
	3,
	255,
	212,
	4,
	255,
	212,
	5,
	255,
	212,
	3,
	255,
	212,
	6,
	255,
	212,
	7,
	255,
	212,
	6,
	255,
	212,
	3,
	255,
	212,
	3,
	255,
	212,
	4,
	255,
	212,
	3,
	255,
	212,
	4,
	255,
	212,
	5,
	255,
	212,
	4,
	255,
	212,
	5,
	255,
	212,
	4,
	255,
	212,
	6,
	255,
	212,
	7,
	255,
	212,
	6,
	255,
	212,
	7,
	255,
	212,
	6,
	255,
	212,
	3,
	255,
	212,
	1,
	0
};

uint16 sc81_tentwig1[] = {
	181*64,
	231,
	235,
	0,
	231,
	235,
	0,
	231,
	235,
	0,
	231,
	235,
	1,
	231,
	235,
	0,
	231,
	235,
	0,
	231,
	235,
	0,
	231,
	235,
	0,
	231,
	235,
	2,
	231,
	235,
	2,
	0
};

uint16 sc81_ken_helm_dn[] = {
	197*64,
	264,
	212,
	0,
	264,
	212,
	1,
	264,
	212,
	2,
	264,
	212,
	3,
	264,
	212,
	4,
	264,
	212,
	5,
	264,
	212,
	6,
	0
};

uint16 sc81_tentwig2[] = {
	182*64,
	225,
	258,
	0,
	225,
	258,
	0,
	225,
	258,
	1,
	225,
	258,
	1,
	225,
	258,
	2,
	225,
	258,
	3,
	225,
	258,
	4,
	225,
	258,
	5,
	225,
	258,
	6,
	225,
	258,
	7,
	0
};

uint16 sc81_fast_list[] = {
	12+DISK_10,
	51+DISK_10,
	168+DISK_10,
	169+DISK_10,
	171+DISK_10,
	170+DISK_10,
	203+DISK_10,
	268+DISK_10,
	0
};

Compact sc81_tent5 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	296,	// xcood
	247,	// ycood
	185*64,	// frame
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
	SC81_TENT5_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc81_helmet = {
	0,	// logic
	ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	264,	// xcood
	212,	// ycood
	174*64,	// frame
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

uint16 sc81_ken_sit[] = {
	195*64,
	263,
	244,
	0,
	263,
	243,
	1,
	263,
	242,
	2,
	263,
	241,
	3,
	263,
	240,
	4,
	263,
	238,
	5,
	263,
	235,
	6,
	263,
	231,
	7,
	263,
	229,
	8,
	263,
	228,
	9,
	263,
	226,
	10,
	263,
	224,
	11,
	263,
	221,
	12,
	264,
	219,
	13,
	267,
	218,
	14,
	262,
	218,
	15,
	261,
	218,
	16,
	260,
	220,
	17,
	261,
	224,
	18,
	261,
	227,
	19,
	261,
	230,
	20,
	261,
	229,
	21,
	261,
	226,
	22,
	261,
	226,
	23,
	0
};

uint16 sc81_palette[] = {
	0,
	1536,
	0,
	514,
	514,
	1026,
	518,
	2050,
	1024,
	1028,
	3078,
	512,
	524,
	3076,
	1538,
	18,
	1538,
	2054,
	2054,
	3082,
	1542,
	532,
	14084,
	14135,
	2058,
	7434,
	512,
	2564,
	5140,
	2052,
	2570,
	4620,
	2566,
	539,
	7432,
	1026,
	3084,
	4620,
	3080,
	3082,
	9488,
	512,
	1053,
	1542,
	5646,
	3088,
	1548,
	6926,
	3596,
	7440,
	2566,
	549,
	11014,
	1024,
	3600,
	9488,
	2052,
	4104,
	7451,
	3080,
	4110,
	6420,
	3084,
	1573,
	11530,
	1538,
	4614,
	5667,
	3600,
	1579,
	4616,
	5138,
	4624,
	5147,
	5138,
	3107,
	10508,
	3080,
	5132,
	5155,
	5652,
	5138,
	2589,
	10006,
	5145,
	13072,
	2054,
	2097,
	3084,
	10009,
	4131,
	5650,
	6422,
	2609,
	5132,
	7449,
	5659,
	8982,
	3092,
	3625,
	12050,
	4108,
	6420,
	8993,
	6418,
	5665,
	6416,
	7449,
	6926,
	6955,
	7451,
	6943,
	3094,
	13597,
	7444,
	10535,
	6420,
	4655,
	6420,
	8477,
	6437,
	7446,
	7965,
	6951,
	8978,
	6941,
	8464,
	7475,
	10015,
	6953,
	12059,
	5657,
	7971,
	7967,
	8993,
	8982,
	10543,
	7453,
	8978,
	8503,
	9505,
	8999,
	13083,
	6939,
	8489,
	7969,
	11045,
	9507,
	12071,
	7457,
	10021,
	9513,
	11047,
	10029,
	12061,
	8485,
	10021,
	13615,
	8993,
	10535,
	11055,
	11049,
	10037,
	12063,
	10025,
	10037,
	10529,
	12587,
	12075,
	11043,
	12589,
	11061,
	11559,
	10033,
	11575,
	11041,
	13615,
	12077,
	14131,
	11563,
	13615,
	13601,
	11565,
	12085,
	14121,
	9009,
	12591,
	14135,
	12081,
	13619,
	5175,
	1538,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	2103,
	1024,
	13,
	4869,
	1537,
	537,
	7686,
	1538,
	803,
	10247,
	2052,
	1325,
	13064,
	2054,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	7479,
	45,
	10261,
	3584,
	35,
	7689,
	1024,
	25,
	5121,
	0,
	16,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
	55,
	14135,
	14080,
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
	13886,
	14893,
	9263,
	10294,
	13084,
	5409,
	6703,
	11278,
	2324,
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
	14143,
	14080,
	55,
	14135,
	14080,
	55,
	13879,
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

uint16 sc81_dad_helm_up[] = {
	175*64,
	264,
	212,
	0,
	264,
	212,
	1,
	264,
	212,
	2,
	264,
	212,
	3,
	264,
	212,
	4,
	264,
	212,
	5,
	0
};

Compact sc81_tent2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	225,	// xcood
	259,	// ycood
	182*64,	// frame
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
	SC81_TENT2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc83_draw_list[] = {
	ID_SC81_FATHER,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc81_tentwig5[] = {
	185*64,
	295,
	248,
	0,
	295,
	248,
	1,
	295,
	248,
	2,
	295,
	248,
	3,
	295,
	248,
	4,
	295,
	248,
	5,
	295,
	248,
	6,
	295,
	248,
	7,
	295,
	248,
	8,
	295,
	248,
	9,
	0
};

Compact sc81_chair = {
	L_SCRIPT,	// logic
	ST_MOUSE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	251,	// xcood
	222,	// ycood
	0,	// frame
	20938,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	282-256,	// mouseSize_x
	268-221,	// mouseSize_y
	SC81_CHAIR_ACTION,	// actionScript
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

uint16 sc81_chip_list[] = {
	167+DISK_10,
	172+DISK_10,
	173+DISK_10,
	174+DISK_10,
	175+DISK_10,
	176+DISK_10,
	177+DISK_10,
	178+DISK_10,
	179+DISK_10+0X8000,
	180+DISK_10+0X8000,
	181+DISK_10,
	182+DISK_10,
	183+DISK_10,
	184+DISK_10,
	185+DISK_10,
	186+DISK_10,
	187+DISK_10,
	188+DISK_10+0X8000,
	189+DISK_10,
	190+DISK_10+0X8000,
	191+DISK_10+0X8000,
	192+DISK_10+0X8000,
	193+DISK_10,
	194+DISK_10+0X8000,
	195+DISK_10+0X8000,
	196+DISK_10,
	197+DISK_10,
	198+DISK_10,
	199+DISK_10+0X8000,
	200+DISK_10,
	201+DISK_10,
	202+DISK_10+0X8000,
	204+DISK_10,
	205+DISK_10,
	0
};

uint32 *grid81 = 0;

uint16 sc81_tentwig6[] = {
	186*64,
	308,
	283,
	0,
	308,
	283,
	1,
	308,
	283,
	2,
	308,
	283,
	2,
	308,
	283,
	3,
	308,
	283,
	4,
	308,
	283,
	5,
	308,
	283,
	6,
	308,
	283,
	7,
	308,
	283,
	8,
	0
};

Compact sc81_ken_sat = {
	0,	// logic
	ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	261,	// xcood
	226,	// ycood
	196*64,	// frame
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

uint16 sc81_tentdrop2[] = {
	188*64,
	218,
	171,
	0,
	218,
	171,
	0,
	218,
	171,
	0,
	218,
	171,
	0,
	218,
	171,
	0,
	218,
	171,
	0,
	218,
	171,
	1,
	218,
	171,
	2,
	218,
	171,
	3,
	218,
	171,
	4,
	218,
	171,
	5,
	218,
	171,
	6,
	218,
	171,
	7,
	218,
	171,
	8,
	218,
	171,
	9,
	218,
	171,
	10,
	0
};

uint16 sc81_ken_walk2[] = {
	194*64,
	232,
	244,
	0,
	232,
	244,
	1,
	232,
	245,
	2,
	233,
	245,
	3,
	238,
	245,
	4,
	247,
	244,
	5,
	252,
	244,
	6,
	252,
	245,
	7,
	253,
	245,
	8,
	259,
	245,
	9,
	267,
	244,
	10,
	263,
	245,
	11,
	263,
	244,
	12,
	0
};

Compact sc81_foster_sat = {
	0,	// logic
	ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	256,	// xcood
	225,	// ycood
	200*64,	// frame
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

uint16 sc81_tentdrop1[] = {
	187*64,
	218,
	137,
	0,
	218,
	137,
	0,
	218,
	137,
	1,
	218,
	137,
	2,
	218,
	137,
	3,
	218,
	137,
	4,
	218,
	137,
	5,
	218,
	137,
	6,
	218,
	137,
	7,
	218,
	137,
	8,
	218,
	137,
	9,
	218,
	137,
	10,
	218,
	137,
	11,
	218,
	137,
	12,
	218,
	137,
	13,
	218,
	137,
	14,
	0
};

Compact sc81_father_sat = {
	0,	// logic
	ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	256,	// xcood
	225,	// ycood
	172*64,	// frame
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

uint16 sc81_tentwig4[] = {
	184*64,
	305,
	212,
	0,
	305,
	212,
	0,
	305,
	212,
	1,
	305,
	212,
	0,
	305,
	212,
	0,
	305,
	212,
	0,
	305,
	212,
	2,
	305,
	212,
	0,
	305,
	212,
	0,
	305,
	212,
	0,
	0
};

uint16 sc81_ken_walk1[] = {
	192*64,
	228,
	211,
	0,
	228,
	214,
	1,
	228,
	216,
	2,
	228,
	218,
	3,
	228,
	219,
	4,
	228,
	221,
	5,
	228,
	224,
	6,
	228,
	226,
	7,
	228,
	228,
	8,
	228,
	229,
	9,
	228,
	231,
	0,
	228,
	234,
	1,
	228,
	236,
	2,
	228,
	238,
	3,
	228,
	239,
	4,
	228,
	241,
	5,
	228,
	244,
	6,
	228,
	244,
	10,
	232,
	244,
	11,
	0
};

uint16 reset_dad_spec[] = {
	C_XCOOD,
	247,
	C_YCOOD,
	182,
	C_FRAME,
	171*64,
	65535
};

uint16 sc81_mouse_list[] = {
	ID_KEN,
	ID_SC81_FATHER,
	ID_SC81_CHAIR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 sc81_dad_breath1[] = {
	176*64,
	270,
	256,
	0,
	270,
	256,
	1,
	270,
	256,
	1,
	270,
	256,
	2,
	270,
	256,
	2,
	270,
	256,
	3,
	270,
	256,
	3,
	270,
	256,
	3,
	270,
	256,
	3,
	270,
	256,
	2,
	270,
	256,
	2,
	270,
	256,
	1,
	270,
	256,
	1,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	0
};

uint16 sc81_fos_helm_dn[] = {
	201*64,
	264,
	212,
	0,
	264,
	212,
	1,
	264,
	212,
	2,
	264,
	212,
	3,
	264,
	212,
	4,
	264,
	212,
	5,
	0
};

uint16 sc81_dad_fall2[] = {
	180*64,
	289,
	275,
	0,
	289,
	275,
	0,
	287,
	275,
	1,
	287,
	275,
	1,
	286,
	273,
	2,
	283,
	266,
	3,
	282,
	260,
	4,
	279,
	258,
	5,
	278,
	257,
	6,
	277,
	257,
	7,
	278,
	258,
	8,
	278,
	258,
	9,
	278,
	258,
	9,
	0
};

Compact sc81_door = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	81,	// screen
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
	SC81_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc81_big_tent1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	257,	// xcood
	137,	// ycood
	187*64,	// frame
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
	SC81_BIG_TENT1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc81_tent1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	232,	// xcood
	236,	// ycood
	181*64,	// frame
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
	SC81_TENT1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc81_tent4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	305,	// xcood
	212,	// ycood
	184*64,	// frame
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
	SC81_TENT4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

ExtCompact sc81_father_ext = {
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
	96+32,	// spWidth_xx
	SP_COL_FATHER81,	// spColour
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

Compact sc81_father = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	264,	// xcood
	212,	// ycood
	173*64,	// frame
	20939,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	MEGA_CLICK,	// mouseClick
	0,	// mouseRel_x
	2,	// mouseRel_y
	299-278,	// mouseSize_x
	293-258,	// mouseSize_y
	SC81_FATHER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC81_FATHER_CHAIR_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc81_father_ext
};

uint16 sc81_door_close[] = {
	204*64,
	166,
	200,
	0,
	166,
	200,
	1,
	166,
	200,
	2,
	166,
	200,
	3,
	166,
	200,
	4,
	166,
	200,
	5,
	166,
	200,
	6,
	0
};

uint16 sc81_dad_breath2[] = {
	177*64,
	270,
	256,
	0,
	270,
	256,
	1,
	270,
	256,
	1,
	270,
	256,
	2,
	270,
	256,
	2,
	270,
	256,
	3,
	270,
	256,
	3,
	270,
	256,
	4,
	270,
	256,
	4,
	270,
	256,
	5,
	270,
	256,
	5,
	270,
	256,
	6,
	270,
	256,
	6,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	270,
	256,
	0,
	0
};

uint16 sc81_tentwig3[] = {
	183*64,
	223,
	277,
	0,
	223,
	277,
	1,
	223,
	277,
	2,
	223,
	277,
	3,
	223,
	277,
	4,
	223,
	277,
	5,
	223,
	277,
	6,
	223,
	277,
	7,
	223,
	277,
	8,
	223,
	277,
	9,
	0
};

uint16 sc81_father_die[] = {
	203*64,
	278,
	256,
	0,
	278,
	256,
	0,
	278,
	256,
	0,
	278,
	256,
	1,
	278,
	256,
	2,
	278,
	256,
	3,
	278,
	256,
	3,
	278,
	256,
	3,
	278,
	256,
	3,
	278,
	256,
	3,
	278,
	256,
	4,
	278,
	256,
	5,
	278,
	256,
	6,
	278,
	256,
	7,
	278,
	256,
	8,
	0
};

uint16 reset_ken_81[] = {
	C_STATUS,
	ST_MOUSE+ST_LOGIC+ST_RECREATE,
	C_SCREEN,
	81,
	C_MOUSE_CLICK,
	MEGA_CLICK,
	C_MOUSE_REL_X,
	10,
	C_MOUSE_REL_Y,
	3,
	C_MOUSE_SIZE_X,
	240-231,
	C_MOUSE_SIZE_Y,
	293-243,
	C_ACTION_SCRIPT,
	SC81_KEN_ACTION,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC81_KEN_LOGIC,
	C_BASE_SUB+2,
	0,
	C_SP_COLOUR,
	SP_COL_KEN81,
	65535
};

uint16 reset_80_81[] = {
	C_SCREEN,
	81,
	C_XCOOD,
	152,
	C_YCOOD,
	256,
	C_FRAME,
	46+12*64,
	C_DIR,
	RIGHT,
	65535
};

uint16 sc81_dad_fall1[] = {
	179*64,
	256,
	227,
	0,
	256,
	227,
	1,
	255,
	229,
	2,
	256,
	237,
	3,
	262,
	245,
	4,
	271,
	254,
	5,
	276,
	267,
	6,
	287,
	272,
	7,
	284,
	275,
	8,
	282,
	278,
	9,
	282,
	277,
	10,
	282,
	278,
	11,
	282,
	278,
	11,
	282,
	278,
	11,
	282,
	278,
	12,
	282,
	278,
	12,
	283,
	278,
	13,
	283,
	278,
	13,
	284,
	278,
	14,
	284,
	278,
	14,
	284,
	278,
	15,
	284,
	278,
	15,
	284,
	278,
	16,
	284,
	278,
	16,
	283,
	278,
	17,
	283,
	278,
	17,
	283,
	278,
	18,
	283,
	278,
	18,
	283,
	278,
	19,
	283,
	278,
	19,
	283,
	278,
	20,
	283,
	278,
	20,
	283,
	278,
	21,
	283,
	278,
	21,
	283,
	278,
	22,
	283,
	278,
	22,
	283,
	277,
	23,
	283,
	277,
	23,
	284,
	277,
	24,
	284,
	277,
	24,
	287,
	277,
	25,
	287,
	277,
	25,
	289,
	276,
	26,
	289,
	276,
	26,
	0
};

uint16 sc81_ken_jump_in[] = {
	190*64,
	166,
	202,
	0,
	166,
	209,
	1,
	170,
	215,
	2,
	187,
	220,
	3,
	194,
	224,
	4,
	202,
	222,
	5,
	209,
	216,
	6,
	211,
	212,
	7,
	218,
	210,
	8,
	226,
	210,
	9,
	232,
	210,
	10,
	233,
	209,
	11,
	233,
	209,
	12,
	228,
	209,
	13,
	227,
	209,
	14,
	0
};

uint16 sc81_logic_list[] = {
	ID_SC81_TENT1,
	ID_SC81_TENT2,
	ID_SC81_TENT3,
	ID_SC81_TENT4,
	ID_SC81_TENT5,
	ID_SC81_TENT6,
	ID_SC81_FOSTER_SAT,
	ID_FOSTER,
	ID_SC81_KEN_SAT,
	ID_KEN,
	ID_SC81_FATHER_SAT,
	ID_SC81_FATHER,
	ID_SC81_HELMET,
	ID_SC81_DOOR,
	ID_SC81_BIG_TENT1,
	ID_SC81_BIG_TENT2,
	ID_SC81_BIG_TENT3,
	ID_SC67_PULSE1,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

Compact sc81_big_tent2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	257,	// xcood
	171,	// ycood
	188*64,	// frame
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
	SC81_BIG_TENT2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc81_tent3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	223,	// xcood
	278,	// ycood
	183*64,	// frame
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
	SC81_TENT3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc81_tentdrop3[] = {
	189*64,
	218,
	239,
	0,
	218,
	239,
	0,
	218,
	239,
	0,
	218,
	239,
	0,
	218,
	239,
	0,
	218,
	239,
	0,
	218,
	239,
	0,
	218,
	239,
	0,
	218,
	239,
	1,
	218,
	239,
	2,
	218,
	239,
	3,
	218,
	239,
	4,
	218,
	239,
	5,
	218,
	239,
	6,
	218,
	239,
	7,
	218,
	239,
	8,
	0
};

Compact sc81_tent6 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	81,	// screen
	0,	// place
	0,	// getToTable
	307,	// xcood
	283,	// ycood
	186*64,	// frame
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
	SC81_TENT6_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc83_palette[] = {
	0,
	15616,
	14649,
	14655,
	15159,
	13621,
	13625,
	13621,
	13619,
	10535,
	15157,
	13107,
	12595,
	9523,
	13097,
	13631,
	14129,
	12593,
	12083,
	12593,
	12591,
	10021,
	9009,
	12581,
	14655,
	15151,
	12081,
	12597,
	14127,
	12079,
	12083,
	11567,
	12075,
	9507,
	13615,
	11565,
	11057,
	10541,
	11561,
	9507,
	9005,
	11555,
	8993,
	16173,
	11065,
	13631,
	14635,
	11053,
	11059,
	16171,
	4133,
	10029,
	9003,
	11045,
	8993,
	7723,
	11041,
	12607,
	12073,
	10535,
	9517,
	9513,
	10533,
	9507,
	7209,
	10526,
	14143,
	16167,
	10035,
	12607,
	16167,
	10029,
	11061,
	12583,
	10023,
	9517,
	8487,
	10017,
	7708,
	6695,
	10014,
	13119,
	16165,
	9521,
	12093,
	14629,
	9517,
	11071,
	11557,
	9509,
	9515,
	10021,
	9509,
	9005,
	11045,
	9507,
	9001,
	6693,
	9500,
	14143,
	16163,
	9013,
	12095,
	16163,
	9005,
	11581,
	16163,
	8999,
	9515,
	11043,
	8995,
	8487,
	7715,
	8990,
	13631,
	16161,
	8495,
	12093,
	15649,
	8491,
	10553,
	15649,
	8483,
	7719,
	9505,
	8478,
	7196,
	5665,
	8472,
	13119,
	16158,
	7729,
	11581,
	16158,
	7723,
	11069,
	14622,
	7723,
	11063,
	16158,
	7721,
	10041,
	13086,
	7719,
	9023,
	9502,
	7708,
	5656,
	5150,
	7702,
	12095,
	15132,
	7213,
	11071,
	15132,
	7211,
	10555,
	14108,
	7203,
	9009,
	16156,
	7201,
	8509,
	11036,
	7198,
	6689,
	7708,
	7192,
	5656,
	5148,
	7190,
	5140,
	15132,
	6701,
	11071,
	14618,
	6695,
	9525,
	13082,
	6693,
	7231,
	15642,
	6684,
	7223,
	8474,
	6680,
	5662,
	5658,
	6676,
	10559,
	15128,
	6185,
	9017,
	13592,
	6179,
	8499,
	12056,
	6177,
	8491,
	12056,
	6174,
	7215,
	10520,
	6172,
	6717,
	10008,
	6170,
	6193,
	8472,
	6168,
	5662,
	14104,
	5683,
	12081,
	12566,
	5677,
	11053,
	11030,
	5673,
	10047,
	15126,
	5669,
	6693,
	7190,
	5652,
	5146,
	13078,
	5167,
	11567,
	12052,
	5163,
	10541,
	10516,
	5159,
	9535,
	15124,
	5157,
	9511,
	15124,
	5155,
	9015,
	14100,
	5150,
	7731,
	12052,
	5150,
	7721,
	9492,
	5150,
	7715,
	12564,
	5146,
	6699,
	10516,
	5142,
	5181,
	14100,
	5140,
	5169,
	11540,
	5140,
	4663,
	10516,
	5138,
	4632,
	7444,
	45,
	10541,
	10514,
	4647,
	9531,
	10002,
	4645,
	7225,
	14098,
	4636,
	7219,
	11538,
	4636,
	6203,
	12050,
	4632,
	5175,
	15122,
	4626,
	4657,
	5650,
	4624,
	4116,
	4114,
	4624,
	11569,
	15661,
	4131,
	8507,
	5136,
	5138,
	8483,
	15120,
	4126,
	7735,
	13584,
	4126,
	6711,
	11536,
	4122,
	6686,
	11024,
	4120,
	6183,
	10000,
	4116,
	5153,
	6160,
	4116,
	4663,
	11024,
	4112,
	4135,
	8464,
	4112,
	4116,
	5136,
	4110,
	3602,
	9488,
	3621,
	8485,
	15118,
	3614,
	7737,
	14094,
	3612,
	7219,
	12046,
	3612,
	6701,
	8974,
	3610,
	6684,
	12046,
	3608,
	6187,
	9998,
	3608,
	5658,
	9486,
	3604,
	5150,
	11022,
	3602,
	4145,
	11022,
	3600,
	4133,
	8974,
	3600,
	4120,
	7182,
	3598,
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

uint16 reset_foster_spec[] = {
	C_XCOOD,
	300,
	C_YCOOD,
	248,
	C_FRAME,
	42+12*64,
	65535
};

uint16 sc81_foster_sit[] = {
	199*64,
	291,
	234,
	0,
	283,
	235,
	1,
	279,
	236,
	2,
	277,
	237,
	3,
	277,
	235,
	4,
	274,
	232,
	5,
	270,
	227,
	6,
	272,
	225,
	7,
	269,
	224,
	8,
	267,
	224,
	9,
	259,
	226,
	10,
	255,
	226,
	11,
	255,
	226,
	12,
	255,
	224,
	13,
	255,
	224,
	14,
	255,
	225,
	15,
	255,
	225,
	16,
	257,
	225,
	17,
	259,
	225,
	18,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
