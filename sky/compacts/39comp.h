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

#ifndef SKY39COMP_H
#define SKY39COMP_H




namespace Sky {

namespace SkyCompact {

Compact sc39_exit_31 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	39,	// screen
	0,	// place
	0,	// getToTable
	164,	// xcood
	225,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	190-164,	// mouseSize_x
	305-225,	// mouseSize_y
	SC39_EXIT_31_ACTION,	// actionScript
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

uint16 sc39_floor_table[] = {
	ID_SC39_FLOOR,
	RET_OK,
	ID_SC39_EXIT_31,
	GT_SC39_EXIT_31,
	0,
	SC39_EXIT_31_WALK_ON,
	ID_SC39_EXIT_40,
	GT_SC39_EXIT_40,
	ID_SC39_EXIT_41,
	GT_SC39_EXIT_41,
	1,
	SC39_EXIT_41_WALK_ON,
	ID_SC39_WALTERS,
	GT_SC39_WALTERS,
	65535
};

Compact sc39_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	39,	// screen
	0,	// place
	sc39_floor_table,	// getToTable
	176,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-176,	// mouseSize_x
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

Compact sc39_walters = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	39,	// screen
	0,	// place
	0,	// getToTable
	376,	// xcood
	228,	// ycood
	0,	// frame
	16723,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	445-376,	// mouseSize_x
	320-228,	// mouseSize_y
	SC39_WALTERS_ACTION,	// actionScript
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

uint16 reset_41_39[] = {
	C_SCREEN,
	39,
	C_PLACE,
	ID_SC39_FLOOR,
	C_XCOOD,
	424,
	C_YCOOD,
	256,
	C_MEGA_SET,
	NEXT_MEGA_SET,
	C_FRAME,
	42+12*64,
	C_DIR,
	LEFT,
	65535
};

Compact sc39_exit_40 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	39,	// screen
	0,	// place
	0,	// getToTable
	289,	// xcood
	204,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	316-289,	// mouseSize_x
	252-204,	// mouseSize_y
	SC39_EXIT_40_ACTION,	// actionScript
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

uint16 sc39_ascend[] = {
	24*64,
	289,
	207,
	0,
	289,
	207,
	0,
	289,
	205,
	1,
	289,
	205,
	1,
	289,
	202,
	2,
	289,
	202,
	2,
	289,
	201,
	3,
	289,
	201,
	3,
	289,
	200,
	4,
	289,
	200,
	4,
	289,
	200,
	5,
	289,
	200,
	5,
	289,
	200,
	6,
	289,
	200,
	6,
	290,
	201,
	7,
	290,
	201,
	7,
	291,
	202,
	8,
	291,
	202,
	8,
	293,
	202,
	9,
	0
};

uint16 sc39_logic_list[] = {
	ID_FOSTER,
	ID_DANIELLE,
	ID_SPUNKY,
	ID_DOG_BARK_THING,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc39_chip_list[] = {
	IT_SC39_LAYER_0+DISK_14,
	IT_SC39_LAYER_1+DISK_14,
	IT_SC39_LAYER_2+DISK_14,
	IT_SC39_GRID_1+DISK_14,
	IT_SC39_GRID_2+DISK_14,
	23+DISK_14,
	24+DISK_14,
	0
};

uint16 reset_31_39[] = {
	C_SCREEN,
	39,
	C_PLACE,
	ID_SC39_FLOOR,
	C_XCOOD,
	144,
	C_YCOOD,
	296,
	65535
};

uint16 sc39_mouse_list[] = {
	ID_SC39_WALTERS,
	ID_SC39_EXIT_31,
	ID_SC39_EXIT_40,
	ID_SC39_EXIT_41,
	ID_SC39_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc39_exit_41 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	39,	// screen
	0,	// place
	0,	// getToTable
	384,	// xcood
	200,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	400-384,	// mouseSize_x
	227-200,	// mouseSize_y
	SC39_EXIT_41_ACTION,	// actionScript
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

uint32 *sc39_walk_grid = (uint32*)sc39_fast_list;

uint16 sc39_descend[] = {
	23*64,
	293,
	202,
	0,
	291,
	201,
	1,
	290,
	200,
	2,
	289,
	199,
	3,
	289,
	199,
	4,
	289,
	200,
	5,
	289,
	203,
	6,
	289,
	205,
	7,
	289,
	207,
	8,
	289,
	210,
	9,
	0
};

uint16 sc39_fast_list[] = {
	12+DISK_14,
	52+DISK_14,
	135+DISK_14,
	0
};

uint16 reset_40_39[] = {
	C_SCREEN,
	39,
	C_PLACE,
	ID_SC39_FLOOR,
	C_XCOOD,
	289,
	C_YCOOD,
	207,
	C_FRAME,
	24*64,
	C_DIR,
	DOWN,
	65535
};

uint16 sc39_palette[] = {
	0,
	512,
	512,
	2,
	1028,
	1026,
	522,
	1540,
	1540,
	1030,
	2570,
	1542,
	1546,
	3594,
	1542,
	2058,
	3082,
	2056,
	1552,
	4614,
	1542,
	2064,
	4104,
	2568,
	2572,
	3598,
	3594,
	2578,
	5130,
	2058,
	3086,
	5134,
	2570,
	3090,
	7182,
	1544,
	3094,
	3592,
	5134,
	3094,
	4620,
	4110,
	4110,
	5654,
	3598,
	3100,
	5642,
	4110,
	4116,
	5136,
	5136,
	4118,
	7184,
	3086,
	4122,
	4622,
	6674,
	4124,
	5646,
	5138,
	4126,
	6668,
	4626,
	4634,
	8468,
	3600,
	4636,
	7184,
	4626,
	5144,
	5654,
	6676,
	5144,
	8472,
	4114,
	4643,
	7694,
	4628,
	5658,
	8470,
	4628,
	5656,
	9502,
	3604,
	5660,
	9498,
	4628,
	5665,
	7700,
	6680,
	6177,
	9494,
	5142,
	5671,
	7698,
	7192,
	6686,
	10014,
	5144,
	6691,
	9496,
	5658,
	6691,
	11034,
	4632,
	6695,
	9494,
	6682,
	6693,
	8988,
	7196,
	6699,
	8468,
	8478,
	7209,
	10008,
	7196,
	7213,
	11028,
	6172,
	7719,
	11038,
	6174,
	7215,
	8982,
	8993,
	7721,
	10526,
	8478,
	8485,
	11555,
	6686,
	8489,
	12062,
	7198,
	7731,
	11544,
	7713,
	9001,
	12067,
	7713,
	9003,
	14627,
	6686,
	9009,
	13086,
	7715,
	9519,
	11553,
	10021,
	10025,
	12073,
	10021,
	10029,
	13095,
	8997,
	9019,
	13084,
	8999,
	10541,
	12589,
	11049,
	10043,
	13601,
	10025,
	11565,
	11055,
	13101,
	11059,
	15151,
	10025,
	11573,
	14635,
	11051,
	12083,
	15665,
	11053,
	12595,
	15671,
	11567,
	12599,
	15667,
	12081,
	13117,
	14643,
	14133,
	13629,
	14135,
	15671,
	14143,
	16183,
	15161,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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

} // namespace SkyCompact

} // namespace Sky

#endif
