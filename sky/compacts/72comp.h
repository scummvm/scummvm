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

#ifndef SKY72COMP_H
#define SKY72COMP_H




namespace Sky {

namespace SkyCompact {

uint16 wit_st_right[] = {
	159*64,
	1,
	0,
	38,
	0
};

uint16 sc72_comp_flash[] = {
	169*64,
	169,
	225,
	0,
	169,
	225,
	0,
	169,
	225,
	0,
	169,
	225,
	0,
	169,
	225,
	0,
	169,
	225,
	0,
	169,
	225,
	1,
	169,
	225,
	2,
	169,
	225,
	3,
	169,
	225,
	4,
	169,
	225,
	5,
	169,
	225,
	0,
	169,
	225,
	0,
	169,
	225,
	0,
	0
};

Compact sc72_rot_light = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	145,	// xcood
	283,	// ycood
	177*64,	// frame
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
	SC72_ROT_LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc72_fast_list[] = {
	12+DISK_7,
	51+DISK_7,
	52+DISK_7,
	53+DISK_7,
	54+DISK_7,
	IT_SC72_TANK+DISK_7,
	IT_SC72_ROT_LIGHT+DISK_7,
	268+DISK_7,
	0
};

uint16 sc72_spill_anim[] = {
	31*64,
	1,
	1,
	0,
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
	1,
	1,
	9,
	1,
	1,
	10,
	0
};

uint16 wit_d_to_u[] = {
	39+159*64,
	38+159*64,
	37+159*64,
	0
};

Compact sc72_chamber2 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	284,	// xcood
	190,	// ycood
	175*64,	// frame
	20569,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	309-280,	// mouseSize_x
	242-189,	// mouseSize_y
	SC72_CHAMBERS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC72_CHAMBER2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 wit_l_to_r[] = {
	33+159*64,
	32+159*64,
	39+159*64,
	0
};

uint16 sc72_joey_tap[] = {
	29*64,
	248,
	240,
	0,
	248,
	240,
	1,
	248,
	240,
	2,
	248,
	240,
	0,
	248,
	240,
	1,
	248,
	240,
	2,
	248,
	240,
	0,
	248,
	240,
	1,
	248,
	240,
	2,
	0
};

uint16 sc72_comp2_flash[] = {
	170*64,
	178,
	222,
	0,
	178,
	222,
	0,
	178,
	222,
	0,
	178,
	222,
	0,
	178,
	222,
	0,
	178,
	222,
	0,
	178,
	222,
	1,
	178,
	222,
	2,
	178,
	222,
	3,
	178,
	222,
	4,
	178,
	222,
	5,
	178,
	222,
	0,
	178,
	222,
	0,
	178,
	222,
	0,
	0
};

uint16 wit_auto[32];

Compact sc72_tank = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_SORT,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	278,	// xcood
	255,	// ycood
	173*64,	// frame
	20568,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65535,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	331-277,	// mouseSize_x
	283-254,	// mouseSize_y
	SC72_TANK_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC72_TANK_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 wit_r_to_d[] = {
	39+159*64,
	0
};

uint16 sc72_floor_table[] = {
	ID_SC72_FLOOR,
	RET_OK,
	0,
	SC72_DOOR_WALK_ON,
	1,
	SC72_EXIT_WALK_ON,
	3,
	GT_SC72_WITNESS_TALK,
	4,
	GT_SC72_FOSTER_TALK,
	5,
	GT_SC72_WITNESS_KILL,
	ID_SC72_DOOR,
	GT_SC72_DOOR,
	ID_SC72_EXIT,
	GT_SC72_EXIT,
	ID_SC72_TANK,
	GT_SC72_TANK,
	ID_SC72_TAP,
	GT_SC72_TAP,
	ID_SC72_SPILL,
	GT_SC72_SPILL,
	ID_SC72_GRILL,
	GT_SC72_GRILL,
	ID_SC72_CHAMBER1,
	GT_SC72_CHAMBER1,
	ID_SC72_CHAMBER2,
	GT_SC72_CHAMBER2,
	ID_SC72_CHAMBER3,
	GT_SC72_CHAMBER3,
	ID_SC72_LIGHT1,
	GT_SC72_LIGHT1,
	ID_SC72_LIGHT2,
	GT_SC72_LIGHT2,
	ID_SC72_LIGHT3,
	GT_SC72_LIGHT3,
	ID_SC72_COMPUTER,
	GT_SC72_COMPUTER,
	ID_JOEY_PARK,
	GT_JOEY_PARK,
	65535
};

Compact sc72_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	72,	// screen
	0,	// place
	sc72_floor_table,	// getToTable
	140,	// xcood
	260,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	374-140,	// mouseSize_x
	327-260,	// mouseSize_y
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

uint16 sc72_drip_anim[] = {
	31*64,
	1,
	1,
	11,
	1,
	1,
	12,
	1,
	1,
	13,
	1,
	1,
	14,
	1,
	1,
	15,
	1,
	1,
	16,
	1,
	1,
	17,
	1,
	1,
	18,
	1,
	1,
	19,
	0
};

uint16 wit_u_to_d[] = {
	35+159*64,
	34+159*64,
	33+159*64,
	0
};

uint32 *wit = (uint32*)&witness;

Compact sc72_cham2_light = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	293,	// xcood
	244,	// ycood
	172*64,	// frame
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
	SC72_CHAM2_LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc72_computer2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	178,	// xcood
	224,	// ycood
	170*64,	// frame
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
	SC72_COMPUTER2_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc72_walter_die[] = {
	30*64,
	222,
	228,
	0,
	223,
	230,
	1,
	224,
	233,
	2,
	220,
	239,
	3,
	216,
	245,
	4,
	215,
	256,
	5,
	212,
	267,
	6,
	213,
	278,
	7,
	218,
	278,
	8,
	222,
	278,
	9,
	0
};

uint16 wit_r_to_l[] = {
	37+159*64,
	36+159*64,
	35+159*64,
	0
};

uint16 wit_up[] = {
	2,
	0+159*64,
	0,
	65534,
	2,
	1+159*64,
	0,
	65534,
	2,
	2+159*64,
	0,
	65534,
	2,
	3+159*64,
	0,
	65534,
	2,
	4+159*64,
	0,
	65534,
	2,
	5+159*64,
	0,
	65534,
	2,
	6+159*64,
	0,
	65534,
	2,
	7+159*64,
	0,
	65534,
	0
};

uint16 sc72_rotating[] = {
	177*64,
	145,
	283,
	0,
	145,
	283,
	1,
	145,
	283,
	2,
	145,
	283,
	3,
	145,
	283,
	4,
	145,
	283,
	5,
	145,
	283,
	6,
	145,
	283,
	7,
	145,
	283,
	8,
	145,
	283,
	8,
	145,
	283,
	8,
	145,
	283,
	9,
	145,
	283,
	10,
	145,
	283,
	11,
	0
};

uint16 sc72_walter_kill[] = {
	178*64,
	320,
	253,
	0,
	320,
	253,
	1,
	320,
	253,
	2,
	321,
	253,
	3,
	319,
	253,
	4,
	317,
	254,
	5,
	321,
	253,
	6,
	322,
	252,
	7,
	322,
	252,
	8,
	324,
	252,
	9,
	326,
	252,
	10,
	327,
	252,
	11,
	328,
	252,
	12,
	329,
	252,
	13,
	330,
	252,
	14,
	331,
	252,
	15,
	332,
	252,
	16,
	334,
	252,
	17,
	336,
	252,
	18,
	337,
	252,
	19,
	337,
	252,
	20,
	337,
	252,
	20,
	337,
	252,
	21,
	337,
	252,
	22,
	337,
	252,
	23,
	337,
	252,
	24,
	337,
	252,
	25,
	337,
	252,
	26,
	337,
	252,
	27,
	335,
	252,
	28,
	332,
	252,
	29,
	335,
	252,
	30,
	337,
	252,
	31,
	337,
	252,
	32,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	337,
	253,
	33,
	0
};

Compact sc72_exit = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	342,	// xcood
	180,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	355-342,	// mouseSize_x
	282-180,	// mouseSize_y
	SC72_EXIT_ACTION,	// actionScript
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

uint16 wit_l_to_d[] = {
	33+159*64,
	0
};

uint16 sc72_cham3_anim[] = {
	176*64,
	344,
	191,
	0,
	344,
	191,
	1,
	344,
	191,
	2,
	344,
	191,
	3,
	344,
	191,
	4,
	344,
	191,
	5,
	344,
	191,
	6,
	344,
	191,
	7,
	344,
	191,
	8,
	344,
	191,
	9,
	344,
	191,
	10,
	344,
	191,
	11,
	344,
	191,
	12,
	344,
	191,
	13,
	344,
	191,
	14,
	344,
	191,
	15,
	344,
	191,
	16,
	344,
	191,
	17,
	344,
	191,
	18,
	344,
	191,
	19,
	0
};

uint16 sc72_chip_list[] = {
	IT_SC72_LAYER_0+DISK_7,
	IT_SC72_LAYER_1+DISK_7,
	IT_SC72_LAYER_2+DISK_7,
	IT_SC72_GRID_1+DISK_7,
	IT_SC72_GRID_2+DISK_7,
	IT_MEDI+DISK_7,
	IT_MEDI_TALK+DISK_7,
	IT_WITNESS+DISK_7+0X8000,
	IT_WALTER_TALK_UP+DISK_7,
	IT_WALTER_TALK_DOWN+DISK_7,
	IT_WALTER_TALK_LEFT+DISK_7,
	IT_WALTER_CONVERSATION+DISK_7+0X8000,
	133+DISK_7,
	IT_SC72_WALTER_KILL+DISK_7,
	IT_SC72_CHAM1_LIGHT+DISK_7,
	IT_SC72_CHAM2_LIGHT+DISK_7,
	IT_SC72_COMPUTER+DISK_7,
	IT_SC72_COMPUTER2+DISK_7,
	IT_SC72_CHAMBER1+DISK_7,
	IT_SC72_CHAMBER2+DISK_7,
	IT_SC72_CHAMBER3+DISK_7,
	IT_SC72_GRILL+DISK_7,
	IT_SC72_JOEY_TAP+DISK_7,
	IT_SC72_SPILL+DISK_7,
	0
};

uint16 sc72_foster_die[] = {
	179*64,
	340,
	250,
	0,
	340,
	250,
	0,
	340,
	250,
	0,
	340,
	250,
	0,
	341,
	249,
	1,
	340,
	249,
	2,
	340,
	248,
	3,
	339,
	247,
	4,
	341,
	246,
	5,
	345,
	246,
	6,
	345,
	246,
	7,
	346,
	247,
	8,
	345,
	247,
	9,
	342,
	247,
	10,
	343,
	246,
	11,
	342,
	246,
	12,
	348,
	246,
	13,
	350,
	246,
	14,
	349,
	246,
	15,
	350,
	246,
	16,
	350,
	247,
	17,
	348,
	247,
	18,
	344,
	247,
	19,
	344,
	246,
	20,
	341,
	246,
	21,
	350,
	246,
	22,
	349,
	246,
	23,
	350,
	246,
	24,
	349,
	248,
	25,
	351,
	248,
	26,
	342,
	249,
	27,
	349,
	249,
	28,
	349,
	248,
	29,
	348,
	248,
	30,
	352,
	249,
	31,
	353,
	249,
	32,
	351,
	249,
	33,
	352,
	249,
	34,
	353,
	249,
	35,
	354,
	249,
	36,
	353,
	249,
	37,
	353,
	249,
	38,
	352,
	249,
	39,
	353,
	249,
	38,
	0
};

uint16 sc72_cham1_anim[] = {
	174*64,
	221,
	190,
	0,
	221,
	190,
	1,
	221,
	190,
	2,
	221,
	190,
	3,
	221,
	190,
	4,
	221,
	190,
	5,
	221,
	190,
	6,
	221,
	190,
	7,
	221,
	190,
	8,
	221,
	190,
	9,
	221,
	190,
	10,
	221,
	190,
	11,
	221,
	190,
	12,
	221,
	190,
	13,
	221,
	190,
	14,
	221,
	190,
	15,
	221,
	190,
	16,
	221,
	190,
	17,
	221,
	190,
	18,
	221,
	190,
	19,
	0
};

uint16 sc72_cham2_anim[] = {
	175*64,
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
	3,
	280,
	190,
	4,
	280,
	190,
	5,
	280,
	190,
	6,
	280,
	190,
	7,
	280,
	190,
	8,
	280,
	190,
	9,
	280,
	190,
	10,
	280,
	190,
	11,
	280,
	190,
	12,
	280,
	190,
	13,
	280,
	190,
	14,
	280,
	190,
	15,
	280,
	190,
	16,
	280,
	190,
	17,
	280,
	190,
	18,
	280,
	190,
	19,
	0
};

uint16 reset_71_72[] = {
	C_SCREEN,
	72,
	C_PLACE,
	ID_SC72_FLOOR,
	C_XCOOD,
	400,
	C_YCOOD,
	296,
	65535
};

uint16 wit_st_left[] = {
	159*64,
	1,
	0,
	34,
	0
};

Compact sc72_cham1_light = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	232,	// xcood
	244,	// ycood
	171*64,	// frame
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
	SC72_CHAM1_LIGHT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 sc72_palette[] = {
	0,
	1024,
	1028,
	1286,
	1285,
	1799,
	1543,
	2056,
	1800,
	2311,
	2569,
	2057,
	2569,
	3594,
	2056,
	2827,
	3337,
	3083,
	3339,
	4108,
	2830,
	3598,
	3086,
	3345,
	3347,
	3341,
	4625,
	3857,
	3856,
	3858,
	3861,
	4368,
	4881,
	4627,
	4111,
	4373,
	4375,
	5138,
	4628,
	5138,
	3349,
	4889,
	5650,
	6168,
	4629,
	4890,
	5140,
	5656,
	5401,
	4119,
	5915,
	5660,
	5651,
	6680,
	5408,
	6931,
	5656,
	6932,
	5916,
	5914,
	5918,
	4376,
	7710,
	8210,
	6682,
	5916,
	5921,
	5398,
	6686,
	6940,
	5914,
	7709,
	5924,
	4888,
	8225,
	8212,
	4644,
	7203,
	7961,
	8732,
	6171,
	8215,
	9248,
	6938,
	7709,
	9756,
	6170,
	9493,
	7965,
	6688,
	7458,
	5407,
	9763,
	8729,
	7715,
	7969,
	6953,
	10010,
	6686,
	10006,
	6432,
	10020,
	8733,
	8484,
	7202,
	9500,
	6432,
	8742,
	7721,
	6173,
	7721,
	7974,
	8991,
	7205,
	8236,
	7198,
	7211,
	8744,
	7204,
	11049,
	10783,
	9504,
	8486,
	11036,
	7971,
	9769,
	11548,
	11815,
	7972,
	9258,
	7719,
	10539,
	12317,
	10017,
	9768,
	9772,
	7716,
	7730,
	12066,
	8739,
	11309,
	11304,
	8232,
	11823,
	10032,
	9256,
	12077,
	12324,
	8743,
	8244,
	12583,
	9253,
	9012,
	12326,
	13101,
	11048,
	11309,
	9772,
	10802,
	12076,
	9768,
	9271,
	12839,
	10290,
	10038,
	13608,
	12080,
	11569,
	13865,
	10796,
	13108,
	13613,
	11313,
	11064,
	13362,
	11565,
	12089,
	14895,
	13612,
	13109,
	14129,
	12084,
	12857,
	14387,
	12592,
	13371,
	14899,
	13623,
	14653,
	15417,
	15161,
	15423,
	3082,
	4362,
	3344,
	63,
	0,
	47,
	16191,
	10520,
	3881,
	4883,
	8198,
	2848,
	9,
	2816,
	0,
	13,
	3840,
	0,
	20,
	6400,
	0,
	40,
	14080,
	0,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
	5662,
	7704,
	6166,
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
	5141,
	3854,
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
	7743,
	6166,
	5662,
	7704,
	6166,
	5662,
	13848,
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

uint16 sc72_joey_list[] = {
	152,
	343,
	272,
	319,
	1,
	288,
	343,
	296,
	303,
	0,
	184,
	231,
	272,
	295,
	0,
	0
};

uint16 sc72_cham2_flash[] = {
	172*64,
	293,
	244,
	0,
	293,
	244,
	1,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	293,
	244,
	0,
	0
};

uint16 wit_st_down[] = {
	159*64,
	1,
	0,
	32,
	0
};

uint32 *grid72 = 0;

uint16 sc72_mouse_list[] = {
	ID_MEDI,
	ID_SC72_TAP,
	ID_SC72_SPILL,
	ID_SC72_TANK,
	ID_SC72_GRILL,
	ID_WITNESS,
	ID_SC72_COMPUTER,
	ID_SC72_CHAMBER1,
	ID_SC72_CHAMBER2,
	ID_SC72_DOOR,
	ID_SC72_EXIT,
	ID_SC72_FLOOR,
	ID_SC72_FAKE_FLOOR,
	0XFFFF,
	ID_TEXT_MOUSE
};

Compact sc72_fake_floor = {
	0,	// logic
	0,	// status
	0,	// sync
	72,	// screen
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

uint16 wit_st_up[] = {
	159*64,
	1,
	0,
	36,
	0
};

Compact sc72_tap = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	269,	// xcood
	269,	// ycood
	0,	// frame
	20567,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	278-269,	// mouseSize_x
	278-269,	// mouseSize_y
	SC72_TAP_ACTION,	// actionScript
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

uint16 wit_l_to_t[] = {
	33+159*64,
	32+159*64,
	0
};

uint16 wit_st_talk[] = {
	129*64,
	1,
	0,
	0,
	0
};

uint16 wit_d_to_l[] = {
	33+159*64,
	0
};

Compact sc72_spill = {
	L_SCRIPT,	// logic
	ST_LOGIC,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	250,	// xcood
	279,	// ycood
	31*64,	// frame
	20577,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	2,	// mouseRel_y
	277-250,	// mouseSize_x
	295-281,	// mouseSize_y
	SC72_SPILL_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC72_SPILL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc72_computer = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	169,	// xcood
	227,	// ycood
	169*64,	// frame
	20695,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65532,	// mouseRel_x
	(int16) 65530,	// mouseRel_y
	183-165,	// mouseSize_x
	236-221,	// mouseSize_y
	SC72_COMPUTER_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC72_COMPUTER_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 wit_l_to_u[] = {
	35+159*64,
	0
};

Compact sc72_door = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	368,	// xcood
	227,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_RIGHT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	387-368,	// mouseSize_x
	306-227,	// mouseSize_y
	SC72_DOOR_ACTION,	// actionScript
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

uint16 wit_u_to_l[] = {
	35+159*64,
	0
};

uint16 sc72_cham1_flash[] = {
	171*64,
	232,
	244,
	0,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	232,
	244,
	1,
	0
};

uint16 wit_d_to_r[] = {
	39+159*64,
	0
};

uint16 wit_left[] = {
	4,
	16+159*64,
	65532,
	0,
	4,
	17+159*64,
	65532,
	0,
	4,
	18+159*64,
	65532,
	0,
	4,
	19+159*64,
	65532,
	0,
	4,
	20+159*64,
	65532,
	0,
	4,
	21+159*64,
	65532,
	0,
	4,
	22+159*64,
	65532,
	0,
	4,
	23+159*64,
	65532,
	0,
	0
};

uint16 sc72_tank_anim[] = {
	173*64,
	278,
	255,
	0,
	278,
	255,
	1,
	278,
	255,
	2,
	278,
	255,
	3,
	278,
	255,
	4,
	278,
	255,
	5,
	278,
	255,
	6,
	278,
	255,
	7,
	278,
	255,
	8,
	278,
	255,
	9,
	278,
	255,
	10,
	278,
	255,
	11,
	278,
	255,
	12,
	278,
	255,
	13,
	278,
	255,
	14,
	278,
	255,
	15,
	278,
	255,
	16,
	278,
	255,
	17,
	278,
	255,
	18,
	278,
	255,
	19,
	0
};

uint16 wit_right[] = {
	4,
	24+159*64,
	4,
	0,
	4,
	25+159*64,
	4,
	0,
	4,
	26+159*64,
	4,
	0,
	4,
	27+159*64,
	4,
	0,
	4,
	28+159*64,
	4,
	0,
	4,
	29+159*64,
	4,
	0,
	4,
	30+159*64,
	4,
	0,
	4,
	31+159*64,
	4,
	0,
	0
};

uint16 wit_t_to_u[] = {
	38+159*64,
	37+159*64,
	0
};

uint16 wit_u_to_t[] = {
	37+159*64,
	38+159*64,
	0
};

Compact sc72_chamber3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	344,	// xcood
	191,	// ycood
	176*64,	// frame
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
	SC72_CHAMBER3_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 wit_down[] = {
	2,
	8+159*64,
	0,
	2,
	2,
	9+159*64,
	0,
	2,
	2,
	10+159*64,
	0,
	2,
	2,
	11+159*64,
	0,
	2,
	2,
	12+159*64,
	0,
	2,
	2,
	13+159*64,
	0,
	2,
	2,
	14+159*64,
	0,
	2,
	2,
	15+159*64,
	0,
	2,
	0
};

uint16 wit_u_to_r[] = {
	37+159*64,
	0
};

uint16 wit_r_to_u[] = {
	37+159*64,
	0
};

uint16 wit_t_to_l[] = {
	32+159*64,
	33+159*64,
	0
};

TurnTable witness_turnTable0 = {
	{ // turnTableUp
		0,
		wit_u_to_d,
		wit_u_to_l,
		wit_u_to_r,
		wit_u_to_t
	},
	{ // turnTableDown
		wit_d_to_u,
		0,
		wit_d_to_l,
		wit_d_to_r,
		0
	},
	{ // turnTableLeft
		wit_l_to_u,
		wit_l_to_d,
		0,
		wit_l_to_r,
		wit_l_to_t
	},
	{ // turnTableRight
		wit_r_to_u,
		wit_r_to_d,
		wit_r_to_l,
		0,
		0
	},
	{ // turnTableTalk
		wit_t_to_u,
		0,
		wit_t_to_l,
		0,
		0
	}
};

MegaSet witness_megaSet0 = {
	3,	// gridWidth
	8,	// colOffset
	16,	// colWidth
	24,	// lastChr
	wit_up,	// animUp
	wit_down,	// animDown
	wit_left,	// animLeft
	wit_right,	// animRight
	wit_st_up,	// standUp
	wit_st_down,	// standDown
	wit_st_left,	// standLeft
	wit_st_right,	// standRight
	wit_st_talk,	// standTalk
	&witness_turnTable0
};

ExtCompact witness_ext = {
	0,	// actionSub
	0,	// actionSub_off
	0,	// getToSub
	0,	// getToSub_off
	0,	// extraSub
	0,	// extraSub_off
	DOWN,	// dir
	STD_MEGA_STOP,	// stopScript
	STD_MINI_BUMP,	// miniBump
	0,	// leaving
	0,	// atWatch
	0,	// atWas
	0,	// alt
	0,	// request
	96+32,	// spWidth_xx
	SP_COL_WITNESS,	// spColour
	0,	// spTextId
	0,	// spTime
	0,	// arAnimIndex
	0,	// turnProg
	0,	// waitingFor
	0,	// arTarget_x
	0,	// arTarget_y
	wit_auto,	// animScratch
	0,	// megaSet
	&witness_megaSet0,
	0,
	0,
	0
};

Compact witness = {
	L_SCRIPT,	// logic
	ST_SORT+ST_RECREATE+ST_LOGIC+ST_GRID_PLOT+ST_MOUSE+ST_COLLISION,	// status
	0,	// sync
	72,	// screen
	ID_SC72_FLOOR,	// place
	0,	// getToTable
	256,	// xcood
	280,	// ycood
	32+159*64,	// frame
	4,	// cursorText
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
	WITNESS_LOGIC,	// baseSub
	0,	// baseSub_off
	&witness_ext
};

uint16 sc72_logic_list[] = {
	ID_FOSTER,
	ID_MEDI,
	ID_WITNESS,
	ID_KEN,
	ID_SC67_DOOR,
	ID_SC68_DOOR,
	ID_SC72_CHAMBER1,
	ID_SC72_CHAM1_LIGHT,
	ID_SC72_CHAMBER2,
	ID_SC72_CHAM2_LIGHT,
	ID_SC72_CHAMBER3,
	ID_SC72_TANK,
	ID_SC72_ROT_LIGHT,
	ID_SC72_COMPUTER,
	ID_SC72_COMPUTER2,
	ID_SC72_SPILL,
	ID_SC72_GRILL,
	ID_SC75_TONGS,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 reset_73_72[] = {
	C_SCREEN,
	72,
	C_PLACE,
	ID_SC72_FLOOR,
	C_XCOOD,
	368,
	C_YCOOD,
	272,
	65535
};

uint16 sc720_chip_list[] = {
	IT_SC720_LAYER_0+DISK_7,
	IT_SC720_LAYER_1+DISK_7,
	IT_SC720_LAYER_2+DISK_7,
	IT_SC720_GRID_1+DISK_7,
	IT_SC720_GRID_2+DISK_7,
	IT_MEDI+DISK_7,
	IT_MEDI_TALK+DISK_7,
	IT_WITNESS+DISK_7+0X8000,
	IT_WALTER_TALK_UP+DISK_7,
	IT_WALTER_TALK_DOWN+DISK_7,
	IT_WALTER_TALK_LEFT+DISK_7,
	IT_SC72_CHAM1_LIGHT+DISK_7,
	IT_SC72_CHAM2_LIGHT+DISK_7,
	IT_SC72_COMPUTER+DISK_7,
	IT_SC72_COMPUTER2+DISK_7,
	IT_SC72_CHAMBER1+DISK_7,
	IT_SC72_CHAMBER2+DISK_7,
	IT_SC72_CHAMBER3+DISK_7,
	IT_SC72_GRILL+DISK_7,
	IT_SC72_JOEY_TAP+DISK_7,
	IT_SC72_SPILL+DISK_7,
	0
};

Compact sc72_grill = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	214,	// xcood
	282,	// ycood
	28*64,	// frame
	20511,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	1,	// mouseRel_x
	(int16) 65535,	// mouseRel_y
	242-215,	// mouseSize_x
	287-281,	// mouseSize_y
	SC72_GRILL_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC72_GRILL_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact sc72_chamber1 = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_BACKGROUND,	// status
	0,	// sync
	72,	// screen
	0,	// place
	0,	// getToTable
	223,	// xcood
	191,	// ycood
	174*64,	// frame
	20569,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	(int16) 65533,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	249-220,	// mouseSize_x
	242-189,	// mouseSize_y
	SC72_CHAMBERS_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	SC72_CHAMBER1_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
