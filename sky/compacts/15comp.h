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

#ifndef SKY15COMP_H
#define SKY15COMP_H




namespace Sky {

namespace SkyCompact {

uint16 rs_foster_13_15[] = {
	C_XCOOD,
	440,
	C_YCOOD,
	240,
	C_SCREEN,
	15,
	65535
};

Compact flap = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	0XA6,	// xcood
	0XC9,	// ycood
	74*64,	// frame
	8372,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	45,	// mouseRel_x
	230-202,	// mouseRel_y
	30,	// mouseSize_x
	266-230,	// mouseSize_y
	FLAP_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	FLAP_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 weld_seq[] = {
	72*64,
	369,
	148,
	0,
	369,
	148,
	1,
	369,
	148,
	2,
	369,
	148,
	3,
	369,
	148,
	4,
	369,
	148,
	5,
	369,
	148,
	6,
	369,
	148,
	7,
	0
};

uint16 rs_foster_15_13[] = {
	C_XCOOD,
	168,
	C_YCOOD,
	224,
	C_SCREEN,
	13,
	C_PLACE,
	ID_S13_FLOOR,
	C_STATUS,
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_AR_PRIORITY,
	65535
};

Compact whole_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	42,	// xcood
	42,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	STORE_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	2000,	// mouseSize_x
	2000,	// mouseSize_y
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

uint16 s15_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_FLOOR_PUTTY,
	ID_FLAP,
	ID_SKEY,
	ID_WD40,
	177,
	0XFFFF,
	ID_MENU_LOGIC
};

uint32 *grid15 = 0;

uint16 flap_seq[] = {
	74*64,
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
	2,
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
	0
};

uint16 s15_floor_table[] = {
	ID_S15_FLOOR,
	RET_OK,
	ID_NU_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S15_WALK_ON,
	ID_STORE_EXIT,
	GT_STORE_EXIT,
	ID_JUNCTION_BOX,
	GT_JUNCTION_BOX,
	ID_FLAP,
	GT_FLAP,
	ID_SKEY,
	GT_SKEY,
	ID_WD40,
	GT_WD40,
	ID_FLOOR_PUTTY,
	GT_PUTTY,
	65535
};

Compact s15_flooor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	s15_floor_table,	// getToTable
	208,	// xcood
	216,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	STORE_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	424-208,	// mouseSize_x
	63,	// mouseSize_y
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

uint16 flip_seq[] = {
	75*64,
	245,
	202,
	0,
	240,
	206,
	1,
	237,
	215,
	2,
	234,
	225,
	3,
	234,
	225,
	3,
	234,
	225,
	3,
	234,
	225,
	3,
	237,
	223,
	4,
	237,
	215,
	5,
	234,
	213,
	6,
	235,
	213,
	7,
	235,
	214,
	8,
	235,
	212,
	9,
	223,
	210,
	10,
	218,
	207,
	11,
	209,
	206,
	12,
	202,
	205,
	13,
	202,
	204,
	14,
	208,
	203,
	15,
	203,
	204,
	16,
	203,
	203,
	17,
	195,
	203,
	18,
	200,
	202,
	19,
	202,
	202,
	20,
	203,
	202,
	21,
	0
};

Compact skey = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	164+5,	// xcood
	221-2,	// ycood
	85*64,	// frame
	8380,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	SKEY_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SHELF_OBJECT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s15_pal[] = {
	0,
	1536,
	1028,
	1540,
	1542,
	1542,
	2056,
	2568,
	1544,
	2058,
	2568,
	2570,
	2570,
	3084,
	2570,
	2574,
	4104,
	1546,
	3084,
	3082,
	3084,
	2576,
	3598,
	2574,
	3092,
	3592,
	3598,
	3092,
	4106,
	3598,
	4110,
	4112,
	3600,
	4112,
	4624,
	3088,
	3098,
	5130,
	4622,
	4118,
	6156,
	3088,
	3105,
	6152,
	4622,
	4626,
	5138,
	4114,
	3610,
	7696,
	3086,
	4628,
	4626,
	5140,
	4634,
	8974,
	2574,
	4634,
	7184,
	5136,
	5142,
	6676,
	2582,
	4641,
	5642,
	5142,
	5654,
	6166,
	5142,
	5656,
	10006,
	3088,
	5150,
	11536,
	2574,
	5153,
	10000,
	3090,
	5662,
	6674,
	6168,
	6172,
	6166,
	6682,
	5669,
	12558,
	3088,
	5161,
	12048,
	3090,
	6179,
	7188,
	6682,
	5675,
	7692,
	6170,
	6183,
	7188,
	7196,
	6693,
	12054,
	4118,
	5171,
	12558,
	3606,
	7201,
	8988,
	6684,
	7205,
	10518,
	5146,
	7710,
	7708,
	7710,
	7719,
	12558,
	4120,
	7213,
	13580,
	4120,
	7211,
	10520,
	4641,
	8483,
	11550,
	5660,
	7721,
	12058,
	2593,
	8993,
	12579,
	5660,
	7725,
	8984,
	8483,
	9005,
	8462,
	9507,
	8489,
	11548,
	4131,
	8491,
	12060,
	6177,
	9509,
	10021,
	8997,
	9003,
	12577,
	2599,
	9519,
	10004,
	9509,
	9005,
	12574,
	6691,
	10035,
	9996,
	9511,
	10021,
	12585,
	4647,
	9011,
	12058,
	7717,
	11057,
	10510,
	10025,
	10031,
	13603,
	4137,
	10537,
	12073,
	9511,
	10539,
	13095,
	7719,
	11061,
	11026,
	10539,
	11575,
	14098,
	3631,
	11571,
	11548,
	11053,
	11567,
	12075,
	11567,
	12593,
	13103,
	12595,
	16149,
	1294,
	1285,
	1799,
	2055,
	2313,
	3597,
	4366,
	4626,
	5653,
	6422,
	6682,
	7709,
	1566,
	1542,
	2058,
	2568,
	2570,
	3084,
	4106,
	3600,
	4371,
	6415,
	4,
	13622,
	4405,
	3084,
	5398,
	8469,
	8995,
	10537,
	13353,
	13107,
	15935,
	3389,
	2056,
	2058,
	3590,
	2058,
	3092,
	4616,
	3088,
	4634,
	10768,
	1800,
	5,
	13887,
	13621,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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

Compact wd40 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	162+3,	// xcood
	230-6,	// ycood
	86*64,	// frame
	8373,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	WD40_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SHELF_OBJECT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact junction_box = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	397,	// xcood
	199,	// ycood
	0,	// frame
	8382,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	STORE_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	4,	// mouseSize_x
	16,	// mouseSize_y
	JUNCTION_ACTION,	// actionScript
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

uint16 rs_joey_15_13[] = {
	C_XCOOD,
	168,
	C_YCOOD,
	224,
	C_SCREEN,
	13,
	C_PLACE,
	ID_S13_FLOOR,
	65535
};

Compact store_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	407,	// xcood
	184,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	STORE_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	9,	// mouseSize_x
	249-184,	// mouseSize_y
	STORE_EXIT_ACTION,	// actionScript
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

uint16 s15_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	87+DISK_3,
	24+DISK_3,
	25+DISK_3,
	26+DISK_3,
	93+DISK_3,
	94+DISK_3,
	0
};

Compact nu_floor = {
	0,	// logic
	0,	// status
	0,	// sync
	15,	// screen
	0,	// place
	s15_floor_table,	// getToTable
	208-32,	// xcood
	216,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	STORE_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	216+32,	// mouseSize_x
	63,	// mouseSize_y
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

Compact floor_putty = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	15,	// screen
	0,	// place
	0,	// getToTable
	225,	// xcood
	251,	// ycood
	88*64,	// frame
	137,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	5,	// mouseSize_y
	FLOOR_PUTTY_ACTION,	// actionScript
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

uint16 s15_mouse[] = {
	ID_JOEY,
	ID_STORE_EXIT,
	ID_JUNCTION_BOX,
	ID_FLAP,
	ID_SKEY,
	ID_WD40,
	ID_FLOOR_PUTTY,
	ID_S15_FLOOR,
	ID_NU_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 joey_list_15[] = {
	256,
	376,
	216,
	272,
	1,
	0
};

uint16 get_putty_seq[] = {
	144*64,
	220,
	200,
	0,
	220,
	200,
	1,
	220,
	200,
	2,
	220,
	200,
	3,
	220,
	200,
	3,
	220,
	200,
	3,
	220,
	200,
	3,
	220,
	200,
	3,
	220,
	200,
	4,
	220,
	200,
	5,
	0
};

uint16 shelf_get_seq[] = {
	76*64,
	283,
	221,
	0,
	283,
	221,
	1,
	283,
	221,
	2,
	283,
	221,
	3,
	283,
	221,
	4,
	283,
	221,
	5,
	283,
	221,
	6,
	283,
	221,
	7,
	283,
	221,
	8,
	283,
	221,
	9,
	283,
	221,
	10,
	283,
	221,
	11,
	283,
	221,
	12,
	283,
	221,
	13,
	283,
	221,
	14,
	283,
	221,
	15,
	283,
	221,
	16,
	283,
	221,
	17,
	283,
	221,
	18,
	283,
	221,
	19,
	283,
	221,
	20,
	283,
	221,
	21,
	283,
	221,
	22,
	283,
	221,
	0,
	0
};

uint16 s15_fake_mouse[] = {
	ID_JOEY,
	ID_STORE_EXIT,
	ID_JUNCTION_BOX,
	ID_FLAP,
	ID_FAKE_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 s15_chip_list[] = {
	173,
	111,
	136+DISK_2,
	137+DISK_2,
	72+DISK_3,
	74+DISK_3,
	75+DISK_3,
	76+DISK_3,
	85+DISK_3,
	86+DISK_3,
	88+DISK_3,
	144+DISK_3,
	148+DISK_3,
	149+DISK_3,
	150+DISK_3,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
