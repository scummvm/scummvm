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

#ifndef SKY82COMP_H
#define SKY82COMP_H




namespace Sky {

namespace SkyCompact {

uint16 sc82_push_button[] = {
	209*64,
	312,
	200,
	0,
	312,
	200,
	1,
	312,
	200,
	2,
	312,
	200,
	3,
	312,
	200,
	2,
	312,
	200,
	4,
	312,
	200,
	5,
	312,
	200,
	4,
	312,
	200,
	5,
	312,
	200,
	3,
	312,
	200,
	2,
	312,
	200,
	4,
	312,
	200,
	1,
	312,
	200,
	0,
	0
};

ExtCompact sc82_jobsworth_ext = {
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
	SP_COL_JOBS82,	// spColour
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

Compact sc82_jobsworth = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_SORT+ST_RECREATE,	// status
	0,	// sync
	82,	// screen
	0,	// place
	0,	// getToTable
	312,	// xcood
	200,	// ycood
	209*64,	// frame
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
	SC82_JOBSWORTH_LOGIC,	// baseSub
	0,	// baseSub_off
	&sc82_jobsworth_ext
};

uint16 reset_foster_82[] = {
	C_SCREEN,
	82,
	C_XCOOD,
	408,
	C_YCOOD,
	248,
	C_FRAME,
	42+12*64,
	C_DIR,
	LEFT,
	65535
};

uint16 sc82_jobs_turn[] = {
	210*64,
	312,
	200,
	0,
	312,
	200,
	1,
	312,
	200,
	2,
	312,
	200,
	3,
	0
};

uint16 sc82_jobs_take[] = {
	213*64,
	312,
	200,
	0,
	312,
	200,
	1,
	312,
	200,
	2,
	312,
	200,
	3,
	312,
	200,
	4,
	312,
	200,
	4,
	312,
	200,
	4,
	312,
	200,
	4,
	312,
	200,
	4,
	312,
	200,
	4,
	312,
	200,
	5,
	312,
	200,
	2,
	312,
	200,
	0,
	312,
	200,
	0,
	0
};

uint16 sc82_logic_list[] = {
	ID_FOSTER,
	ID_KEN,
	ID_SC82_JOBSWORTH,
	0XFFFF,
	ID_STD_MENU_LOGIC
};

uint16 sc82_fast_list[] = {
	12+DISK_7,
	51+DISK_7,
	0
};

uint32 *grid82 = 0;

uint16 sc82_mouse_list[] = {
	0
};

uint16 reset_ken_82[] = {
	C_STATUS,
	ST_LOGIC+ST_FOREGROUND+ST_RECREATE,
	C_SCREEN,
	82,
	C_XCOOD,
	262,
	C_YCOOD,
	209,
	C_FRAME,
	215*64,
	C_MODE,
	C_BASE_MODE,
	C_BASE_SUB,
	SC82_KEN_LOGIC,
	C_BASE_SUB+2,
	0,
	65535
};

uint16 sc82_chip_list[] = {
	206+DISK_7,
	207+DISK_7,
	208+DISK_7,
	209+DISK_7,
	210+DISK_7,
	211+DISK_7,
	212+DISK_7,
	213+DISK_7,
	214+DISK_7,
	215+DISK_7,
	0
};

uint16 sc82_palette[] = {
	0,
	1536,
	2054,
	2054,
	1544,
	3080,
	2056,
	3594,
	2058,
	3082,
	3086,
	3596,
	3594,
	3090,
	4622,
	3094,
	3592,
	5136,
	4108,
	6170,
	2574,
	4116,
	4112,
	5138,
	4622,
	5660,
	4626,
	4126,
	4618,
	5652,
	5646,
	4643,
	7190,
	5654,
	8470,
	3092,
	6158,
	5157,
	7704,
	6168,
	7702,
	4630,
	6668,
	5163,
	8986,
	6678,
	2590,
	12572,
	6674,
	7209,
	6682,
	7184,
	6191,
	8476,
	6183,
	6162,
	9500,
	7196,
	8990,
	6682,
	8462,
	6195,
	10526,
	7706,
	7205,
	9502,
	8972,
	4667,
	12579,
	8478,
	6691,
	11041,
	8474,
	7725,
	9505,
	8485,
	12060,
	4638,
	10000,
	7737,
	10531,
	8990,
	9003,
	10019,
	8990,
	9519,
	8997,
	9505,
	13611,
	4641,
	9505,
	11055,
	7717,
	10520,
	7735,
	13095,
	9513,
	8997,
	11047,
	10021,
	13097,
	6181,
	10023,
	9003,
	12585,
	10029,
	9507,
	10539,
	10535,
	8493,
	13611,
	10539,
	9511,
	12587,
	10041,
	10518,
	11563,
	11055,
	11557,
	10539,
	12062,
	10045,
	13613,
	11561,
	15153,
	6697,
	11563,
	12591,
	9517,
	11567,
	10541,
	13103,
	12069,
	12603,
	10543,
	11575,
	11555,
	13103,
	13611,
	13609,
	10031,
	12585,
	16185,
	7213,
	12089,
	12579,
	11569,
	12589,
	14135,
	10033,
	12597,
	15659,
	9007,
	12095,
	12062,
	13619,
	13609,
	14141,
	11571,
	13613,
	15675,
	10033,
	14639,
	12591,
	10043,
	13119,
	14115,
	11573,
	13619,
	12597,
	14645,
	13631,
	14627,
	12597,
	14129,
	16189,
	10549,
	13629,
	12589,
	14137,
	14137,
	13107,
	15161,
	14143,
	13615,
	15163,
	14655,
	14129,
	15675,
	15167,
	14131,
	16189,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
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
	9773,
	8995,
	6945,
	7194,
	4883,
	2836,
	1804,
	770,
	519,
	15875,
	11574,
	11577,
	13347,
	6693,
	7727,
	10771,
	3094,
	4133,
	11271,
	2056,
	10002,
	7441,
	6960,
	11030,
	4119,
	3875,
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

uint16 sc82_foster_give[] = {
	214*64,
	342,
	200,
	0,
	342,
	200,
	1,
	342,
	200,
	2,
	342,
	200,
	3,
	342,
	200,
	4,
	342,
	200,
	4,
	342,
	200,
	4,
	342,
	200,
	4,
	342,
	200,
	2,
	342,
	200,
	5,
	342,
	200,
	0,
	0
};

uint16 sc82_jobs_return[] = {
	210*64,
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

} // namespace SkyCompact

} // namespace Sky

#endif
