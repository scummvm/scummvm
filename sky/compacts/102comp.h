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

#ifndef SKY102COMP_H
#define SKY102COMP_H




namespace Sky {

namespace SkyCompact {

uint16 restore_seq[] = {
	182*64,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

Compact restore_butt = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	102,	// screen
	0,	// place
	0,	// getToTable
	RESTORE_BUTT_X,	// xcood
	RESTORE_BUTT_Y,	// ycood
	182*64,	// frame
	80+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	DEATH_CLICK,	// mouseClick
	0,	// mouseRel_x
	2,	// mouseRel_y
	80,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	RESTORE_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s102_logic[] = {
	ID_FOSTER,
	162,
	164,
	175,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 exit_seq[] = {
	183*64,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 restart_seq[] = {
	181*64,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 s102_mouse[] = {
	162,
	164,
	175,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact exit_butt = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	102,	// screen
	0,	// place
	0,	// getToTable
	EXIT_BUTT_X,	// xcood
	EXIT_BUTT_Y,	// ycood
	183*64,	// frame
	93+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	DEATH_CLICK,	// mouseClick
	0,	// mouseRel_x
	2,	// mouseRel_y
	80,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	EXIT_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 s102_pal[] = {
	0,
	256,
	257,
	514,
	514,
	1026,
	516,
	1026,
	1026,
	518,
	1028,
	1028,
	1028,
	1030,
	2052,
	1030,
	2054,
	1030,
	1544,
	2566,
	1030,
	1544,
	2056,
	2566,
	2056,
	3082,
	1544,
	2058,
	2570,
	3080,
	2062,
	6,
	16128,
	2062,
	2568,
	2570,
	2572,
	2060,
	3596,
	2576,
	4102,
	2058,
	3084,
	3598,
	3084,
	3090,
	3080,
	4622,
	3092,
	5128,
	2572,
	3600,
	4622,
	3598,
	3606,
	4616,
	4110,
	4110,
	6162,
	2062,
	4114,
	5646,
	3088,
	4116,
	4112,
	5138,
	3614,
	6662,
	2064,
	4628,
	7184,
	2064,
	4626,
	7702,
	2064,
	4634,
	4618,
	6676,
	5146,
	8462,
	2066,
	5652,
	8470,
	2578,
	4643,
	6664,
	4118,
	5654,
	8472,
	2580,
	5155,
	5642,
	7192,
	6676,
	8988,
	2582,
	6166,
	7713,
	4632,
	5669,
	9482,
	3094,
	6682,
	5656,
	8474,
	6680,
	8990,
	4120,
	6181,
	6156,
	8474,
	6682,
	7198,
	6682,
	6183,
	6668,
	7196,
	6684,
	10526,
	2584,
	7192,
	6689,
	7708,
	7192,
	10019,
	3610,
	7194,
	7201,
	7708,
	6693,
	6676,
	8988,
	7198,
	11036,
	2586,
	7706,
	10531,
	3612,
	7201,
	6686,
	10014,
	7209,
	7184,
	8990,
	7710,
	8483,
	8988,
	63,
	7231,
	8993,
	7723,
	7184,
	9505,
	8476,
	7719,
	8993,
	7725,
	7696,
	9505,
	8481,
	7201,
	9507,
	8988,
	8487,
	9505,
	8483,
	7201,
	11043,
	8990,
	9001,
	9505,
	8993,
	8485,
	10019,
	8997,
	8485,
	10533,
	9507,
	8487,
	11045,
	9507,
	12585,
	3621,
	9507,
	9003,
	11047,
	10533,
	9515,
	11561,
	16128,
	16191,
	16191,
	514,
	514,
	514,
	771,
	1027,
	1028,
	1285,
	1541,
	1542,
	1542,
	1798,
	1799,
	2056,
	2312,
	2313,
	2570,
	2826,
	2827,
	3084,
	3340,
	3341,
	3598,
	3854,
	3855,
	4112,
	4112,
	4369,
	4625,
	4626,
	4882,
	4883,
	5140,
	5396,
	5397,
	5654,
	5910,
	5911,
	5912,
	6424,
	6424,
	6426,
	6938,
	6938,
	6940,
	7451,
	7196,
	7454,
	7965,
	7709,
	7711,
	8223,
	8223,
	8225,
	8737,
	8737,
	8739,
	9251,
	8994,
	8997,
	9764,
	9508,
	9511,
	10278,
	10277,
	9769,
	11050,
	11303,
	10028,
	11310,
	12072,
	10539,
	11057,
	12841,
	10794,
	10547,
	13613,
	12330,
	11062,
	14388,
	14123,
	11065,
	14907,
	44,
	16191,
	14596,
	2363,
	14389,
	12557,
	4149,
	12846,
	10771,
	5679,
	11303,
	9496,
	7209,
	9506,
	7961,
	5922,
	7964,
	6677,
	4892,
	6679,
	5393,
	3863,
	5138,
	4109,
	16146,
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
	15679,
	15677,
	15163,
	14651,
	14649,
	14135,
	13623,
	13621,
	13107,
	12595,
	12593,
	12079,
	11567,
	11565,
	11051,
	10539,
	10537,
	10023,
	9511,
	9509,
	8995,
	8483,
	8481,
	3610,
	6442,
	9999,
	4121,
	6180,
	8465,
	4375,
	5662,
	6929,
	4116,
	4888,
	5391,
	3857,
	4115,
	4366,
	3342,
	3344,
	3597,
	3084,
	2829,
	2827,
	2570,
	8458,
	8481,
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

uint16 s102_chip_list[] = {
	180,
	181,
	182,
	183,
	0
};

Compact restart_butt = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	102,	// screen
	0,	// place
	0,	// getToTable
	RESTART_BUTT_X,	// xcood
	RESTART_BUTT_Y,	// ycood
	181*64,	// frame
	94+T7,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	DEATH_CLICK,	// mouseClick
	0,	// mouseRel_x
	2,	// mouseRel_y
	80,	// mouseSize_x
	20,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	RESTART_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
