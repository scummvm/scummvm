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

#ifndef SKY68COMP_H
#define SKY68COMP_H




namespace Sky {

namespace SkyCompact {

Compact sc68_pulse5 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	280,	// xcood
	190,	// ycood
	62*64,	// frame
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
	SC68_PULSE5_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc68_chip_list[] = {
	IT_SC68_LAYER_0+DISK_5,
	IT_SC68_LAYER_1+DISK_5,
	IT_SC68_GRID_1+DISK_5,
	IT_MEDI+DISK_5,
	IT_MEDI_TALK+DISK_5,
	IT_SC68_PULSE1+DISK_5,
	IT_SC68_PULSE2+DISK_5,
	IT_SC68_PULSE3+DISK_5,
	IT_SC68_PULSE4+DISK_5,
	IT_SC68_PULSE5+DISK_5,
	IT_SC68_PULSE6+DISK_5,
	IT_SC68_SENSOR+DISK_5,
	IT_SC68_DESCEND+DISK_5,
	IT_SC68_ASCEND+DISK_5,
	IT_SC68_DOOR+DISK_5,
	0
};

uint16 reset_69_68[] = {
	C_SCREEN,
	68,
	C_PLACE,
	ID_SC68_FLOOR,
	C_XCOOD,
	OFF_RIGHT,
	C_YCOOD,
	288,
	65535
};

Compact sc68_pulse1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	425,	// xcood
	191,	// ycood
	58*64,	// frame
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
	SC68_PULSE1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc68_door = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	184,	// xcood
	213,	// ycood
	57*64,	// frame
	181,	// cursorText
	SC68_DOOR_MOUSE_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	4,	// mouseRel_y
	21,	// mouseSize_x
	67,	// mouseSize_y
	SC68_DOOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC68_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc68_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	420,	// xcood
	220,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	27,	// mouseSize_x
	107,	// mouseSize_y
	SC68_EXIT_ACTION,	// actionScript
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

uint16 sc68_logic_list[] = {
	ID_FOSTER,
	ID_MEDI,
	ID_WITNESS,
	ID_SC67_DOOR,
	ID_SC68_DOOR,
	ID_SC67_CLOT,
	ID_SC67_CROWBAR,
	ID_SC67_MEND,
	ID_SC68_PULSE1,
	ID_SC68_PULSE2,
	ID_SC68_PULSE3,
	ID_SC68_PULSE4,
	ID_SC68_PULSE5,
	ID_SC68_PULSE6,
	ID_SC68_SENSOR,
	ID_SC70_IRIS,
	ID_SC72_SPILL,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc68_palette[] = {
	0,
	1024,
	772,
	1285,
	1798,
	1029,
	1799,
	2310,
	1287,
	1800,
	3081,
	1287,
	2058,
	2057,
	2314,
	2314,
	2827,
	2058,
	2573,
	4102,
	1545,
	2826,
	3598,
	2060,
	2068,
	3592,
	3083,
	3340,
	4621,
	1803,
	3088,
	5386,
	1803,
	3087,
	4367,
	3340,
	3853,
	6159,
	2570,
	2330,
	4360,
	2831,
	3345,
	5649,
	2317,
	3855,
	4369,
	3855,
	2843,
	5130,
	2576,
	3606,
	4876,
	4367,
	3864,
	7689,
	2572,
	4372,
	5647,
	3089,
	2849,
	5641,
	4624,
	4881,
	6675,
	3855,
	4633,
	7947,
	3086,
	2853,
	8713,
	2829,
	4885,
	4883,
	4885,
	4887,
	6673,
	3603,
	3365,
	7947,
	3600,
	4888,
	7188,
	3091,
	5400,
	8718,
	3343,
	5404,
	10254,
	3086,
	5402,
	6164,
	5399,
	5660,
	5649,
	5912,
	3627,
	10252,
	3600,
	4644,
	7697,
	3863,
	4646,
	8974,
	3860,
	6171,
	8469,
	4118,
	6425,
	7703,
	4632,
	4140,
	8718,
	4887,
	6434,
	7441,
	5914,
	5161,
	8210,
	4890,
	4654,
	9999,
	5141,
	6431,
	11289,
	4371,
	7197,
	8985,
	5146,
	5929,
	9490,
	4635,
	7200,
	10266,
	5912,
	5424,
	10003,
	5147,
	7713,
	11800,
	4887,
	7461,
	7700,
	7200,
	7460,
	12315,
	5399,
	7464,
	10774,
	5149,
	7974,
	8214,
	7712,
	6701,
	12312,
	5913,
	8227,
	10269,
	5921,
	8233,
	9499,
	7969,
	8492,
	10774,
	6434,
	7219,
	11800,
	7198,
	8999,
	13344,
	6941,
	9260,
	10265,
	8741,
	8243,
	12063,
	6693,
	9774,
	10781,
	8743,
	10287,
	10783,
	9513,
	10541,
	12581,
	8488,
	10801,
	12067,
	10027,
	11060,
	13603,
	9517,
	0,
	6144,
	2570,
	3102,
	8714,
	3343,
	4136,
	9230,
	4370,
	5161,
	11794,
	3858,
	6184,
	12311,
	5399,
	6701,
	13080,
	6172,
	7726,
	13084,
	7968,
	10289,
	13345,
	9003,
	0,
	3840,
	3852,
	3857,
	4879,
	4367,
	4885,
	5907,
	4371,
	5402,
	6164,
	5399,
	6685,
	7959,
	6425,
	7200,
	8474,
	6174,
	8227,
	9501,
	7969,
	8999,
	10272,
	8741,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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
	3349,
	2826,
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
	14347,
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

Compact sc68_pulse3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	387,	// xcood
	186,	// ycood
	60*64,	// frame
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
	SC68_PULSE3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc68_pulse5_anim[] = {
	62*64,
	280,
	190,
	0,
	280,
	190,
	1,
	280,
	190,
	2,
	280,
	190,
	2,
	280,
	190,
	1,
	280,
	190,
	1,
	280,
	190,
	1,
	280,
	190,
	1,
	280,
	190,
	2,
	280,
	190,
	2,
	280,
	190,
	2,
	280,
	190,
	2,
	280,
	190,
	2,
	280,
	190,
	1,
	280,
	190,
	0,
	280,
	190,
	0,
	280,
	190,
	0,
	0
};

uint16 sc68_ascend[] = {
	154*64,
	256,
	219,
	0,
	256,
	217,
	1,
	256,
	216,
	2,
	256,
	214,
	3,
	256,
	215,
	4,
	256,
	216,
	5,
	256,
	216,
	6,
	257,
	217,
	7,
	259,
	218,
	8,
	261,
	218,
	9,
	0
};

uint16 sc68_pulse3_anim[] = {
	60*64,
	387,
	186,
	0,
	387,
	186,
	0,
	387,
	186,
	1,
	387,
	186,
	1,
	387,
	186,
	1,
	387,
	186,
	1,
	387,
	186,
	0,
	387,
	186,
	0,
	387,
	186,
	0,
	387,
	186,
	0,
	387,
	186,
	0,
	387,
	186,
	1,
	387,
	186,
	2,
	387,
	186,
	2,
	387,
	186,
	2,
	387,
	186,
	2,
	387,
	186,
	1,
	0
};

uint16 sc68_pulse2_anim[] = {
	59*64,
	396,
	228,
	0,
	396,
	228,
	0,
	396,
	228,
	1,
	396,
	228,
	1,
	396,
	228,
	1,
	396,
	228,
	1,
	396,
	228,
	0,
	396,
	228,
	0,
	396,
	228,
	0,
	396,
	228,
	0,
	396,
	228,
	0,
	396,
	228,
	1,
	396,
	228,
	2,
	396,
	228,
	2,
	396,
	228,
	2,
	396,
	228,
	2,
	396,
	228,
	1,
	0
};

Compact sc68_pulse4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	339,	// xcood
	191,	// ycood
	61*64,	// frame
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
	SC68_PULSE4_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc68_pulse6_anim[] = {
	63*64,
	245,
	197,
	0,
	245,
	197,
	0,
	245,
	197,
	1,
	245,
	197,
	2,
	245,
	197,
	2,
	245,
	197,
	1,
	245,
	197,
	1,
	245,
	197,
	1,
	245,
	197,
	1,
	245,
	197,
	2,
	245,
	197,
	2,
	245,
	197,
	2,
	245,
	197,
	2,
	245,
	197,
	2,
	245,
	197,
	1,
	245,
	197,
	0,
	245,
	197,
	0,
	0
};

Compact sc68_sensor = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	171,	// xcood
	253,	// ycood
	137*64,	// frame
	20512,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65534,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	6,	// mouseSize_x
	11,	// mouseSize_y
	SC68_SENSOR_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC68_SENSOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint32 *grid68 = 0;

uint16 sc68_pulse1_anim[] = {
	58*64,
	425,
	191,
	0,
	425,
	191,
	1,
	425,
	191,
	1,
	425,
	191,
	1,
	425,
	191,
	1,
	425,
	191,
	0,
	425,
	191,
	0,
	425,
	191,
	0,
	425,
	191,
	0,
	425,
	191,
	0,
	425,
	191,
	1,
	425,
	191,
	2,
	425,
	191,
	2,
	425,
	191,
	2,
	425,
	191,
	2,
	425,
	191,
	1,
	425,
	191,
	0,
	0
};

uint16 sc68_pulse4_anim[] = {
	61*64,
	339,
	191,
	0,
	339,
	191,
	1,
	339,
	191,
	1,
	339,
	191,
	0,
	339,
	191,
	0,
	339,
	191,
	0,
	339,
	191,
	0,
	339,
	191,
	1,
	339,
	191,
	1,
	339,
	191,
	1,
	339,
	191,
	1,
	339,
	191,
	1,
	339,
	191,
	0,
	339,
	191,
	2,
	339,
	191,
	2,
	339,
	191,
	2,
	339,
	191,
	2,
	0
};

Compact sc68_pulse2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	396,	// xcood
	228,	// ycood
	59*64,	// frame
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
	SC68_PULSE2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc68_descend[] = {
	153*64,
	261,
	218,
	0,
	259,
	217,
	1,
	257,
	216,
	2,
	256,
	215,
	3,
	256,
	215,
	4,
	256,
	214,
	5,
	256,
	214,
	6,
	256,
	216,
	7,
	256,
	218,
	8,
	256,
	220,
	9,
	0
};

uint16 reset_70_68[] = {
	C_SCREEN,
	68,
	C_PLACE,
	ID_SC68_FLOOR,
	C_XCOOD,
	256,
	C_YCOOD,
	219,
	C_FRAME,
	154*64,
	C_DIR,
	DOWN,
	65535
};

uint16 sc68_sensor_anim[] = {
	137*64,
	171,
	253,
	0,
	171,
	253,
	0,
	171,
	253,
	0,
	171,
	253,
	0,
	171,
	253,
	0,
	171,
	253,
	1,
	171,
	253,
	2,
	171,
	253,
	2,
	171,
	253,
	2,
	171,
	253,
	2,
	0
};

uint16 sc68_door_close[] = {
	57*64,
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

uint16 sc68_floor_table[] = {
	ID_SC68_FLOOR,
	RET_OK,
	0,
	SC68_DOOR_WALK_ON,
	1,
	SC68_EXIT_WALK_ON,
	ID_SC68_DOOR,
	GT_SC68_DOOR,
	ID_SC68_SENSOR,
	GT_SC68_SENSOR,
	ID_SC68_GRILL,
	GT_SC68_GRILL,
	ID_SC68_STAIRS,
	GT_SC68_STAIRS,
	ID_SC68_EXIT,
	GT_SC68_EXIT,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	3,
	GT_SC68_JOEY_WAIT,
	65535
};

Compact sc68_pulse6 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	245,	// xcood
	197,	// ycood
	63*64,	// frame
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
	SC68_PULSE6_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc68_grill = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	351,	// xcood
	217,	// ycood
	0,	// frame
	20511,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	15,	// mouseSize_y
	SC68_GRILL_ACTION,	// actionScript
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

Compact sc68_stairs = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	68,	// screen
	0,	// place
	0,	// getToTable
	255,	// xcood
	210,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	30,	// mouseSize_x
	60,	// mouseSize_y
	SC68_STAIRS_ACTION,	// actionScript
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

uint16 sc68_fast_list[] = {
	12+DISK_5,
	51+DISK_5,
	52+DISK_5,
	53+DISK_5,
	54+DISK_5,
	268+DISK_5,
	0
};

uint16 reset_67_68[] = {
	C_SCREEN,
	68,
	C_PLACE,
	ID_SC68_FLOOR,
	C_XCOOD,
	160,
	C_YCOOD,
	280,
	65535
};

uint16 sc68_mouse_list[] = {
	ID_MEDI,
	ID_SC68_SENSOR,
	ID_SC68_GRILL,
	ID_SC68_DOOR,
	ID_SC68_STAIRS,
	ID_SC68_EXIT,
	ID_SC68_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc68_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	68,	// screen
	0,	// place
	sc68_floor_table,	// getToTable
	202,	// xcood
	265,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	447-202,	// mouseSize_x
	327-265,	// mouseSize_y
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

uint16 sc68_joey_list[] = {
	208,
	439,
	272,
	327,
	1,
	208,
	231,
	288,
	303,
	0,
	248,
	303,
	272,
	279,
	0,
	344,
	399,
	272,
	279,
	0,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
