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

#ifndef SKY94COMP_H
#define SKY94COMP_H




namespace Sky {

namespace SkyCompact {

uint16 door_l94_anim[] = {
	58*64,
	202,
	136,
	0,
	202,
	136,
	1,
	202,
	136,
	2,
	202,
	136,
	3,
	202,
	136,
	4,
	202,
	136,
	5,
	202,
	136,
	6,
	202,
	136,
	7,
	202,
	136,
	8,
	202,
	136,
	9,
	202,
	136,
	10,
	202,
	136,
	11,
	202,
	136,
	12,
	202,
	136,
	13,
	202,
	136,
	14,
	202,
	136,
	15,
	202,
	136,
	16,
	202,
	136,
	17,
	202,
	136,
	18,
	202,
	136,
	19,
	202,
	136,
	20,
	202,
	136,
	21,
	202,
	136,
	22,
	202,
	136,
	23,
	0
};

uint16 holo1_a_anim[] = {
	92*64,
	231,
	176,
	0,
	231,
	176,
	1,
	231,
	176,
	2,
	231,
	176,
	3,
	231,
	176,
	4,
	231,
	176,
	5,
	231,
	176,
	6,
	231,
	176,
	7,
	0
};

uint16 sc94_floor_table[] = {
	ID_SC94_FLOOR,
	RET_OK,
	ID_DOOR_L94,
	GET_TO_DOOR_L94,
	ID_DOOR_R94,
	GET_TO_DOOR_R94,
	ID_HOLOGRAM_PAD,
	GET_TO_HOLOGRAM_PAD,
	65535
};

Compact sc94_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	94,	// screen
	0,	// place
	sc94_floor_table,	// getToTable
	153,	// xcood
	256,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	270,	// mouseSize_x
	52,	// mouseSize_y
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

uint16 fast_list_sc94[] = {
	IT_BLUE_FOSTER+DISK_4,
	IT_LOGOFF+0X8000+DISK_4,
	IT_WINDOW+0X8000+DISK_4,
	IT_INFO_BUTTON+DISK_4,
	IT_LINK_ARROWS+DISK_4,
	IT_LINK_OBJECTS+DISK_4,
	IT_ENTER_TOP+DISK_4+0X8000,
	IT_EXIT_TOP+DISK_4+0X8000,
	IT_CROUCH_RIGHT+DISK_4,
	IT_SC94_LAYER_0+DISK_4,
	IT_HOLO1_A+0X8000+DISK_4,
	IT_HOLO1_B+DISK_4,
	0
};

uint16 holo3_anim[] = {
	97*64,
	241,
	191,
	0,
	241,
	191,
	1,
	241,
	191,
	2,
	241,
	191,
	3,
	241,
	191,
	4,
	0
};

Compact door_r94 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	94,	// screen
	0,	// place
	0,	// getToTable
	345,	// xcood
	136,	// ycood
	60*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	26,	// mouseRel_y
	28,	// mouseSize_x
	98,	// mouseSize_y
	DOOR_R94_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_R94_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc94_logic[] = {
	ID_BLUE_FOSTER,
	ID_DOOR_L94,
	ID_DOOR_L94R,
	ID_DOOR_R94,
	ID_DOOR_R94R,
	ID_HOLOGRAM_A,
	ID_HOLOGRAM_B,
	ID_EYEBALL_90,
	0XFFFF,
	ID_LINC_MENU_LOGIC
};

uint16 door_r94r_anim[] = {
	61*64,
	372,
	136,
	0,
	372,
	136,
	1,
	372,
	136,
	2,
	372,
	136,
	3,
	372,
	136,
	4,
	372,
	136,
	5,
	372,
	136,
	6,
	372,
	136,
	7,
	372,
	136,
	8,
	372,
	136,
	9,
	372,
	136,
	10,
	372,
	136,
	11,
	372,
	136,
	12,
	372,
	136,
	13,
	372,
	136,
	14,
	372,
	136,
	15,
	372,
	136,
	16,
	372,
	136,
	17,
	372,
	136,
	18,
	372,
	136,
	19,
	372,
	136,
	20,
	372,
	136,
	21,
	372,
	136,
	22,
	372,
	136,
	23,
	0
};

Compact door_r94r = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	94,	// screen
	0,	// place
	0,	// getToTable
	372,	// xcood
	136,	// ycood
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
	DOOR_R94R_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_94_90[] = {
	C_SCREEN,
	90,
	C_PLACE,
	ID_SC90_SMFLOOR,
	C_XCOOD,
	200,
	C_YCOOD,
	256,
	C_FRAME,
	46+182*64,
	C_DIR,
	RIGHT,
	65535
};

Compact door_l94r = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	94,	// screen
	0,	// place
	0,	// getToTable
	228,	// xcood
	136,	// ycood
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
	DOOR_L94R_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_94_95[] = {
	C_SCREEN,
	95,
	C_PLACE,
	ID_SC95_FLOOR,
	C_XCOOD,
	288,
	C_YCOOD,
	256,
	C_DIR,
	DOWN,
	65535
};

uint16 door_r94_anim[] = {
	60*64,
	345,
	136,
	0,
	345,
	136,
	1,
	345,
	136,
	2,
	345,
	136,
	3,
	345,
	136,
	4,
	345,
	136,
	5,
	345,
	136,
	6,
	345,
	136,
	7,
	345,
	136,
	8,
	345,
	136,
	9,
	345,
	136,
	10,
	345,
	136,
	11,
	345,
	136,
	12,
	345,
	136,
	13,
	345,
	136,
	14,
	345,
	136,
	15,
	345,
	136,
	16,
	345,
	136,
	17,
	345,
	136,
	18,
	345,
	136,
	19,
	345,
	136,
	20,
	345,
	136,
	21,
	345,
	136,
	22,
	345,
	136,
	23,
	0
};

uint16 chip_list_sc94[] = {
	IT_DOOR_L94+DISK_4,
	IT_DOOR_L94R+DISK_4,
	IT_DOOR_R94+DISK_4,
	IT_DOOR_R94R+DISK_4,
	IT_HOLO2_A+DISK_4,
	IT_HOLO2_B+DISK_4+0X8000,
	IT_HOLO3+0X8000+DISK_4,
	0
};

ExtCompact hologram_b_ext = {
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
	160,	// spWidth_xx
	5,	// spColour
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

Compact hologram_b = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	94,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	93*64,	// frame
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
	HOLOGRAM_B_LOGIC,	// baseSub
	0,	// baseSub_off
	&hologram_b_ext
};

uint16 holo1_b_anim[] = {
	93*64,
	231,
	234,
	0,
	231,
	234,
	1,
	231,
	234,
	2,
	231,
	234,
	3,
	231,
	234,
	4,
	231,
	234,
	5,
	231,
	234,
	6,
	231,
	234,
	7,
	0
};

Compact door_l94 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	94,	// screen
	0,	// place
	0,	// getToTable
	202,	// xcood
	136,	// ycood
	58*64,	// frame
	1+T7,	// cursorText
	STD_EXIT_UP_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	26,	// mouseRel_y
	27,	// mouseSize_x
	98,	// mouseSize_y
	DOOR_L94_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	DOOR_L94_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 door_l94r_anim[] = {
	59*64,
	228,
	136,
	0,
	228,
	136,
	1,
	228,
	136,
	2,
	228,
	136,
	3,
	228,
	136,
	4,
	228,
	136,
	5,
	228,
	136,
	6,
	228,
	136,
	7,
	228,
	136,
	8,
	228,
	136,
	9,
	228,
	136,
	10,
	228,
	136,
	11,
	228,
	136,
	12,
	228,
	136,
	13,
	228,
	136,
	14,
	228,
	136,
	15,
	228,
	136,
	16,
	228,
	136,
	17,
	228,
	136,
	18,
	228,
	136,
	19,
	228,
	136,
	20,
	228,
	136,
	21,
	228,
	136,
	22,
	228,
	136,
	23,
	0
};

uint16 sc94_mouse[] = {
	ID_HOLOGRAM_PAD,
	ID_DOOR_L94,
	ID_DOOR_R94,
	ID_SC94_FLOOR,
	0XFFFF,
	ID_LINC_MENU_MOUSE
};

uint16 anita_holo_talk[] = {
	96*64,
	96*64,
	280,
	225,
	0,
	280,
	225,
	1,
	280,
	225,
	2,
	280,
	225,
	3,
	280,
	225,
	4,
	280,
	225,
	1,
	280,
	225,
	2,
	280,
	225,
	3,
	280,
	225,
	5,
	280,
	225,
	0,
	280,
	225,
	1,
	280,
	225,
	5,
	280,
	225,
	6,
	280,
	225,
	1,
	280,
	225,
	2,
	280,
	225,
	2,
	280,
	225,
	3,
	280,
	225,
	4,
	280,
	225,
	6,
	280,
	225,
	5,
	280,
	225,
	0,
	280,
	225,
	1,
	280,
	225,
	2,
	280,
	225,
	3,
	280,
	225,
	4,
	280,
	225,
	5,
	280,
	225,
	0,
	280,
	225,
	1,
	280,
	225,
	2,
	280,
	225,
	3,
	280,
	225,
	4,
	280,
	225,
	1,
	280,
	225,
	2,
	280,
	225,
	3,
	280,
	225,
	5,
	280,
	225,
	2,
	280,
	225,
	6,
	280,
	225,
	5,
	280,
	225,
	0,
	280,
	225,
	1,
	280,
	225,
	6,
	280,
	225,
	2,
	280,
	225,
	3,
	280,
	225,
	4,
	280,
	225,
	6,
	280,
	225,
	5,
	280,
	225,
	0,
	280,
	225,
	1,
	280,
	225,
	3,
	280,
	225,
	4,
	280,
	225,
	6,
	280,
	225,
	5,
	280,
	225,
	0,
	280,
	225,
	1,
	280,
	225,
	2,
	280,
	225,
	5,
	280,
	225,
	0,
	280,
	225,
	3,
	280,
	225,
	4,
	280,
	225,
	6,
	280,
	225,
	5,
	280,
	225,
	6,
	280,
	225,
	0,
	280,
	225,
	6,
	0
};

Compact hologram_pad = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	94,	// screen
	0,	// place
	0,	// getToTable
	266,	// xcood
	271,	// ycood
	0,	// frame
	24607,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	41,	// mouseSize_x
	7,	// mouseSize_y
	HOLOGRAM_PAD_ACTION,	// actionScript
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

uint16 pal94[] = {
	0,
	3584,
	8960,
	1044,
	7211,
	12812,
	5413,
	11066,
	15390,
	0,
	8,
	3072,
	0,
	16,
	5120,
	1036,
	5120,
	8,
	3104,
	9216,
	20,
	7208,
	15360,
	36,
	0,
	24,
	0,
	15159,
	9787,
	12077,
	8987,
	4390,
	7448,
	3850,
	1301,
	3335,
	7176,
	12,
	1044,
	4096,
	9216,
	0,
	28,
	5120,
	0,
	6180,
	24,
	7168,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
	4415,
	16191,
	16145,
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
	16145,
	4415,
	16191,
	16145,
	4415,
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

Compact hologram_a = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	94,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	92*64,	// frame
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
	HOLOGRAM_A_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
