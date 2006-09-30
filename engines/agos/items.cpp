/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Item script opcodes for Simon1/Simon2

#include "common/stdafx.h"

#include "common/system.h"

#include "agos/animation.h"
#include "agos/agos.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

namespace AGOS {

// Opcode table
void AGOSEngine::setupOpcodes() {
	// This opcode table is for Simon 1. Changes for Simon 2 and FF are
	// made below.

	static OpcodeProc opcode_table[300] = {
		// 0 - 4
		NULL,
		&AGOSEngine::o_at,
		&AGOSEngine::o_notAt,
		NULL,
		NULL,
		// 5 - 9
		&AGOSEngine::o_carried,
		&AGOSEngine::o_notCarried,
		&AGOSEngine::o_isAt,
		NULL,
		NULL,
		// 10 - 14
		NULL,
		&AGOSEngine::o_zero,
		&AGOSEngine::o_notZero,
		&AGOSEngine::o_eq,
		&AGOSEngine::o_notEq,
		// 15 - 19
		&AGOSEngine::o_gt,
		&AGOSEngine::o_lt,
		&AGOSEngine::o_eqf,
		&AGOSEngine::o_notEqf,
		&AGOSEngine::o_ltf,
		// 20 - 24
		&AGOSEngine::o_gtf,
		NULL,
		NULL,
		&AGOSEngine::o_chance,
		NULL,
		// 25 - 29
		&AGOSEngine::o_isRoom,
		&AGOSEngine::o_isObject,
		&AGOSEngine::o_state,
		&AGOSEngine::o_oflag,
		NULL,
		// 30 - 34
		NULL,
		&AGOSEngine::o_destroy,
		NULL,
		&AGOSEngine::o_place,
		NULL,
		// 35 - 39
		NULL,
		&AGOSEngine::o_copyff,
		NULL,
		NULL,
		NULL,
		// 40 - 44
		NULL,
		&AGOSEngine::o_clear,
		&AGOSEngine::o_let,
		&AGOSEngine::o_add,
		&AGOSEngine::o_sub,
		// 45 - 49
		&AGOSEngine::o_addf,
		&AGOSEngine::o_subf,
		&AGOSEngine::o_mul,
		&AGOSEngine::o_div,
		&AGOSEngine::o_mulf,
		// 50 - 54
		&AGOSEngine::o_divf,
		&AGOSEngine::o_mod,
		&AGOSEngine::o_modf,
		&AGOSEngine::o_random,
		NULL,
		// 55 - 59
		&AGOSEngine::o_goto,
		&AGOSEngine::o_oset,
		&AGOSEngine::o_oclear,
		&AGOSEngine::o_putBy,
		&AGOSEngine::o_inc,
		// 60 - 64
		&AGOSEngine::o_dec,
		&AGOSEngine::o_setState,
		&AGOSEngine::o_print,
		&AGOSEngine::o_message,
		&AGOSEngine::o_msg,
		// 65 - 69
		&AGOSEngine::o_addTextBox,
		&AGOSEngine::o_setShortText,
		&AGOSEngine::o_setLongText,
		&AGOSEngine::o_end,
		&AGOSEngine::o_done,
		// 70 - 74
		NULL,
		&AGOSEngine::o_process,
		NULL,
		NULL,
		NULL,
		// 75 - 79
		NULL,
		&AGOSEngine::o_when,
		&AGOSEngine::o_if1,
		&AGOSEngine::o_if2,
		&AGOSEngine::o_isCalled,
		// 80 - 84
		&AGOSEngine::o_is,
		NULL,
		&AGOSEngine::o_debug,
		NULL,
		NULL,
		// 85 - 89
		NULL,
		NULL,
		&AGOSEngine::o_comment,
		&AGOSEngine::o_haltAnimation,
		&AGOSEngine::o_restartAnimation,
		// 90 - 94
		&AGOSEngine::o_getParent,
		&AGOSEngine::o_getNext,
		&AGOSEngine::o_getChildren,
		NULL,
		NULL,
		// 95 - 99
		NULL,
		&AGOSEngine::o_picture,
		&AGOSEngine::o_loadZone,
		NULL,
		NULL,
		// 100 - 104
		&AGOSEngine::o_killAnimate,
		&AGOSEngine::o_defWindow,
		&AGOSEngine::o_window,
		&AGOSEngine::o_cls,
		&AGOSEngine::o_closeWindow,
		// 105 - 109
		NULL,
		NULL,
		&AGOSEngine::o_addBox,
		&AGOSEngine::o_delBox,
		&AGOSEngine::o_enableBox,
		// 110 - 114
		&AGOSEngine::o_disableBox,
		&AGOSEngine::o_moveBox,
		NULL,
		NULL,
		&AGOSEngine::o_doIcons,
		// 115 - 119
		&AGOSEngine::o_isClass,
		&AGOSEngine::o_setClass,
		&AGOSEngine::o_unsetClass,
		NULL,
		&AGOSEngine::o_waitSync,
		// 120 - 124
		&AGOSEngine::o_sync,
		&AGOSEngine::o_defObj,
		NULL,
		NULL,
		NULL,
		// 125 - 129
		&AGOSEngine::o_here,
		&AGOSEngine::o_doClassIcons,
		NULL,
		&AGOSEngine::o_waitEndTune,
		&AGOSEngine::o_ifEndTune,
		// 130 - 134
		&AGOSEngine::o_setAdjNoun,
		NULL,
		&AGOSEngine::o_saveUserGame,
		&AGOSEngine::o_loadUserGame,
		&AGOSEngine::o_stopTune,
		// 135 - 139
		&AGOSEngine::o_pauseGame,
		&AGOSEngine::o_copysf,
		&AGOSEngine::o_restoreIcons,
		&AGOSEngine::o_freezeZones,
		&AGOSEngine::o_placeNoIcons,
		// 140 - 144
		&AGOSEngine::o_clearTimers,
		&AGOSEngine::o_setDollar,
		&AGOSEngine::o_isBox,
		&AGOSEngine::o_doTable,
		NULL,
		// 145 - 149
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		// 150 - 154
		NULL,
		&AGOSEngine::o_storeItem,
		&AGOSEngine::o_getItem,
		&AGOSEngine::o_bSet,
		&AGOSEngine::o_bClear,
		// 155 - 159
		&AGOSEngine::o_bZero,
		&AGOSEngine::o_bNotZero,
		&AGOSEngine::o_getOValue,
		&AGOSEngine::o_setOValue,
		NULL,
		// 160 - 164
		&AGOSEngine::o_ink,
		&AGOSEngine::o_screenTextBox,
		&AGOSEngine::o_screenTextMsg,
		&AGOSEngine::o_playEffect,
		&AGOSEngine::o_getDollar2,
		// 165 - 169
		&AGOSEngine::o_isAdjNoun,
		&AGOSEngine::o_b2Set,
		&AGOSEngine::o_b2Clear,
		&AGOSEngine::o_b2Zero,
		&AGOSEngine::o_b2NotZero,
		// 170 - 174
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		// 175 - 179
		&AGOSEngine::o_lockZones,
		&AGOSEngine::o_unlockZones,
		NULL,
		&AGOSEngine::o_getPathPosn,
		&AGOSEngine::o_scnTxtLongText,
		// 180 - 184
		&AGOSEngine::o_mouseOn,
		NULL,
		NULL,
		NULL,
		&AGOSEngine::o_unloadZone,
		// 185 - 189
		NULL,
		&AGOSEngine::o_unfreezeZones,
		NULL,
		NULL,
		NULL,
		// 190 - 194
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		// 195 - 199
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};

	_opcode_table = opcode_table;
	_numOpcodes = ARRAYSIZE(opcode_table);

	switch (getGameType()) {
	case GType_ELVIRA:
		opcode_table[0] = &AGOSEngine::o_at;
		opcode_table[1] = &AGOSEngine::o_notAt;
		opcode_table[2] = NULL;

		opcode_table[6] = &AGOSEngine::o_carried;
		opcode_table[7] = &AGOSEngine::o_notCarried;
		opcode_table[8] = &AGOSEngine::o_isAt;

		opcode_table[12] = &AGOSEngine::o_zero;
		opcode_table[13] = &AGOSEngine::o_notZero;
		opcode_table[14] = &AGOSEngine::o_eq;
		opcode_table[15] = &AGOSEngine::o_notEq;
		opcode_table[16] = &AGOSEngine::o_gt;
		opcode_table[17] = &AGOSEngine::o_lt;
		opcode_table[18] = &AGOSEngine::o_eqf;
		opcode_table[19] = &AGOSEngine::o_notEqf;
		opcode_table[20] = &AGOSEngine::o_ltf;
		opcode_table[21] = &AGOSEngine::o_gtf;

		opcode_table[32] = &AGOSEngine::o_isRoom;
		opcode_table[33] = &AGOSEngine::o_isObject;

		opcode_table[34] = &AGOSEngine::o_state;
		opcode_table[36] = &AGOSEngine::o_oflag;

		opcode_table[48] = &AGOSEngine::o_destroy;
		opcode_table[49] = NULL;

		opcode_table[51] = &AGOSEngine::o_place;

		opcode_table[56] = &AGOSEngine::o_copyff;

		opcode_table[60] = &AGOSEngine::oe1_setFF;
		opcode_table[61] = &AGOSEngine::o_clear;

		opcode_table[64] = &AGOSEngine::o_let;
		opcode_table[65] = &AGOSEngine::o_add;
		opcode_table[66] = &AGOSEngine::o_sub;
		opcode_table[67] = &AGOSEngine::o_addf;
		opcode_table[68] = &AGOSEngine::o_subf;
		opcode_table[69] = &AGOSEngine::o_mul;
		opcode_table[70] = &AGOSEngine::o_div;
		opcode_table[71] = &AGOSEngine::o_mulf;
		opcode_table[72] = &AGOSEngine::o_divf;
		opcode_table[73] = &AGOSEngine::o_mod;
		opcode_table[74] = &AGOSEngine::o_modf;
		opcode_table[75] = &AGOSEngine::o_random;

		opcode_table[77] = &AGOSEngine::o_goto;

		opcode_table[80] = &AGOSEngine::o_oset;
		opcode_table[81] = &AGOSEngine::o_oclear;

		opcode_table[84] = &AGOSEngine::o_putBy;
		opcode_table[85] = &AGOSEngine::o_inc;
		opcode_table[86] = &AGOSEngine::o_dec;
		opcode_table[87] = &AGOSEngine::o_setState;
		opcode_table[88] = NULL;

		opcode_table[91] = &AGOSEngine::o_message;

		opcode_table[97] = &AGOSEngine::o_end;
		opcode_table[98] = &AGOSEngine::o_done;

		opcode_table[101] = NULL;
		opcode_table[102] = NULL;
		opcode_table[103] = NULL;
		opcode_table[104] = NULL;
		opcode_table[105] = &AGOSEngine::o_process;

		opcode_table[108] = NULL;
		opcode_table[109] = NULL;
		opcode_table[110] = NULL;
		opcode_table[111] = NULL;

		opcode_table[116] = NULL;
		opcode_table[117] = NULL;
		opcode_table[118] = NULL;
		opcode_table[119] = &AGOSEngine::o_when;
		opcode_table[120] = NULL;
		opcode_table[121] = NULL;
		opcode_table[122] = NULL;

		opcode_table[124] = NULL;
		opcode_table[125] = NULL;
		opcode_table[126] = NULL;
		opcode_table[127] = NULL;
		opcode_table[128] = &AGOSEngine::o_if1;
		opcode_table[129] = &AGOSEngine::o_if2;
		opcode_table[130] = NULL;
		opcode_table[131] = NULL;
		opcode_table[132] = NULL;

		opcode_table[134] = NULL;
		opcode_table[135] = &AGOSEngine::o_isCalled;
		opcode_table[136] = &AGOSEngine::o_is;

		opcode_table[138] = NULL;
		opcode_table[139] = NULL;

		opcode_table[141] = NULL;
		opcode_table[142] = NULL;
		opcode_table[143] = NULL;
		opcode_table[144] = NULL;
		opcode_table[145] = NULL;
		opcode_table[146] = NULL;
		opcode_table[147] = NULL;
		opcode_table[148] = NULL;
		opcode_table[149] = NULL;
		opcode_table[150] = NULL;
		opcode_table[151] = NULL;
		opcode_table[152] = &AGOSEngine::o_debug;
		opcode_table[153] = NULL;
		opcode_table[154] = NULL;
		opcode_table[155] = NULL;
		opcode_table[156] = NULL;
		opcode_table[157] = NULL;
		opcode_table[158] = NULL;
		opcode_table[159] = NULL;

		opcode_table[166] = NULL;
		opcode_table[167] = NULL;
		opcode_table[168] = NULL;
		opcode_table[169] = NULL;

		opcode_table[171] = NULL;
		opcode_table[172] = NULL;
		opcode_table[173] = NULL;
		opcode_table[174] = NULL;

		opcode_table[176] = &AGOSEngine::oe1_opcode176;

		opcode_table[178] = &AGOSEngine::oe1_opcode178;
		opcode_table[179] = NULL;

		opcode_table[182] = NULL;
		opcode_table[183] = NULL;
		opcode_table[184] = NULL;
		opcode_table[185] = NULL;
		opcode_table[186] = NULL;
		opcode_table[187] = NULL;
		opcode_table[188] = NULL;
		opcode_table[189] = NULL;
		opcode_table[190] = NULL;
		opcode_table[191] = NULL;
		opcode_table[192] = NULL;
		opcode_table[193] = NULL;

		opcode_table[195] = NULL;
		opcode_table[196] = NULL;
		opcode_table[197] = NULL;
		opcode_table[198] = &AGOSEngine::o_comment;
		opcode_table[199] = NULL;
		opcode_table[200] = NULL;

		opcode_table[203] = NULL;

		opcode_table[205] = NULL;
		opcode_table[206] = &AGOSEngine::o_getParent;
		opcode_table[207] = &AGOSEngine::o_getNext;
		opcode_table[208] = &AGOSEngine::o_getChildren;

		opcode_table[210] = NULL;
		opcode_table[211] = NULL;
		opcode_table[212] = NULL;
		opcode_table[213] = NULL;
		opcode_table[214] = NULL;
		opcode_table[215] = NULL;
		opcode_table[216] = NULL;
		opcode_table[217] = NULL;
		opcode_table[218] = NULL;

		opcode_table[221] = NULL;
		opcode_table[222] = NULL;
		opcode_table[223] = NULL;
		opcode_table[224] = &AGOSEngine::o_picture;
		opcode_table[225] = &AGOSEngine::o_loadZone;
		opcode_table[226] = &AGOSEngine::o1_animate;
		opcode_table[227] = &AGOSEngine::o1_stopAnimate;
		opcode_table[228] = &AGOSEngine::o_killAnimate;
		opcode_table[229] = &AGOSEngine::o_defWindow;
		opcode_table[230] = &AGOSEngine::o_window;
		opcode_table[231] = &AGOSEngine::o_cls;
		opcode_table[232] = &AGOSEngine::o_closeWindow;

		opcode_table[235] = &AGOSEngine::o_addBox;
		opcode_table[236] = &AGOSEngine::o_delBox;
		opcode_table[237] = &AGOSEngine::o_enableBox;
		opcode_table[238] = &AGOSEngine::o_disableBox;
		opcode_table[239] = &AGOSEngine::o_moveBox;

		opcode_table[242] = &AGOSEngine::o_doIcons;
		opcode_table[243] = &AGOSEngine::o_isClass;
		opcode_table[244] = NULL;
		opcode_table[245] = NULL;
		opcode_table[246] = NULL;
		opcode_table[247] = NULL;
		opcode_table[248] = NULL;
		opcode_table[249] = &AGOSEngine::o_setClass;
		opcode_table[250] = &AGOSEngine::o_unsetClass;

		opcode_table[255] = &AGOSEngine::o_waitSync;
		opcode_table[256] = &AGOSEngine::o_sync;
		opcode_table[257] = &AGOSEngine::o_defObj;

		opcode_table[261] = &AGOSEngine::o_here;
		opcode_table[262] = &AGOSEngine::o_doClassIcons;
		opcode_table[264] = &AGOSEngine::o_waitEndTune;
		opcode_table[263] = &AGOSEngine::o1_playTune;

		opcode_table[265] = &AGOSEngine::o_ifEndTune;
		opcode_table[266] = &AGOSEngine::o_setAdjNoun;
		opcode_table[267] = &AGOSEngine::oe1_zoneDisk;
		opcode_table[268] = &AGOSEngine::o_saveUserGame;
		opcode_table[269] = &AGOSEngine::o_loadUserGame;
		opcode_table[271] = &AGOSEngine::o_stopTune;

		opcode_table[274] = &AGOSEngine::o_pauseGame;
		opcode_table[275] = &AGOSEngine::o_copysf;
		opcode_table[276] = &AGOSEngine::o_restoreIcons;

		opcode_table[279] = &AGOSEngine::o_freezeZones;
		opcode_table[280] = &AGOSEngine::o_placeNoIcons;
		opcode_table[281] = &AGOSEngine::o_clearTimers;

		opcode_table[283] = &AGOSEngine::o_isBox;
		break;
	case GType_ELVIRA2:
		break;
	case GType_WW:
		// Confirmed
		opcode_table[70] = &AGOSEngine::o1_printLongText;
		opcode_table[83] = &AGOSEngine::o1_rescan;
		opcode_table[98] = &AGOSEngine::o1_animate;
		opcode_table[99] = &AGOSEngine::o1_stopAnimate;
		opcode_table[85] = &AGOSEngine::oww_whereTo;
		opcode_table[105] = &AGOSEngine::oww_menu;
		opcode_table[106] = &AGOSEngine::oww_textMenu;
		opcode_table[127] = &AGOSEngine::o1_playTune;
		opcode_table[148] = &AGOSEngine::oww_ifDoorOpen;
		opcode_table[179] = &AGOSEngine::o_isAdjNoun;
		opcode_table[180] = &AGOSEngine::o_b2Set;
		opcode_table[181] = &AGOSEngine::o_b2Clear;
		opcode_table[182] = &AGOSEngine::o_b2Zero;
		opcode_table[183] = &AGOSEngine::o_b2NotZero;

		// Code difference, check if triggered
		opcode_table[161] = NULL;
		opcode_table[162] = NULL;
		opcode_table[163] = NULL;
		opcode_table[164] = NULL;
		opcode_table[165] = NULL;
		opcode_table[166] = NULL;
		opcode_table[167] = NULL;
		opcode_table[168] = NULL;
		opcode_table[169] = NULL;
		opcode_table[170] = NULL;
		opcode_table[171] = NULL;
		opcode_table[172] = NULL;
		opcode_table[173] = NULL;
		opcode_table[174] = NULL;
		opcode_table[175] = NULL;
		opcode_table[176] = NULL;
		opcode_table[177] = NULL;
		opcode_table[178] = NULL;
		opcode_table[184] = NULL;
		opcode_table[185] = NULL;
		opcode_table[186] = NULL;
		opcode_table[187] = NULL;
		opcode_table[188] = NULL;
		opcode_table[189] = NULL;
		opcode_table[190] = NULL;
		break;
	case GType_SIMON1:
		opcode_table[70] = &AGOSEngine::o1_printLongText;
		opcode_table[83] = &AGOSEngine::o1_rescan;
		opcode_table[98] = &AGOSEngine::o1_animate;
		opcode_table[99] = &AGOSEngine::o1_stopAnimate;
		opcode_table[127] = &AGOSEngine::o1_playTune;
		opcode_table[177] = &AGOSEngine::o1_screenTextPObj;
		opcode_table[181] = &AGOSEngine::o1_mouseOff;
		opcode_table[182] = &AGOSEngine::o1_loadBeard;
		opcode_table[183] = &AGOSEngine::o1_unloadBeard;
		opcode_table[185] = &AGOSEngine::o1_loadStrings;
		opcode_table[187] = &AGOSEngine::o1_specialFade;
		break;
	case GType_SIMON2:
		opcode_table[70] = &AGOSEngine::o2_printLongText;
		opcode_table[83] = &AGOSEngine::o2_rescan;
		opcode_table[98] = &AGOSEngine::o2_animate;
		opcode_table[99] = &AGOSEngine::o2_stopAnimate;
		opcode_table[127] = &AGOSEngine::o2_playTune;
		opcode_table[177] = &AGOSEngine::o2_screenTextPObj;
		opcode_table[181] = &AGOSEngine::o2_mouseOff;
		opcode_table[188] = &AGOSEngine::o2_isShortText;
		opcode_table[189] = &AGOSEngine::o2_clearMarks;
		opcode_table[190] = &AGOSEngine::o2_waitMark;
		break;
	case GType_FF:
		opcode_table[23] = &AGOSEngine::o3_chance;
		opcode_table[37] = &AGOSEngine::o3_jumpOut;
		opcode_table[65] = &AGOSEngine::o3_addTextBox;
		opcode_table[70] = &AGOSEngine::o3_printLongText;
		opcode_table[83] = &AGOSEngine::o2_rescan;
		opcode_table[98] = &AGOSEngine::o2_animate;
		opcode_table[99] = &AGOSEngine::o2_stopAnimate;
		opcode_table[107] = &AGOSEngine::o3_addBox;
		opcode_table[122] = &AGOSEngine::o3_oracleTextDown;
		opcode_table[123] = &AGOSEngine::o3_oracleTextUp;
		opcode_table[124] = &AGOSEngine::o3_ifTime;
		opcode_table[127] = &AGOSEngine::o3_playTune;
		opcode_table[131] = &AGOSEngine::o3_setTime;
		opcode_table[132] = &AGOSEngine::o3_saveUserGame;
		opcode_table[133] = &AGOSEngine::o3_loadUserGame;
		opcode_table[134] = &AGOSEngine::o3_listSaveGames;
		opcode_table[135] = &AGOSEngine::o3_checkCD;
		opcode_table[161] = &AGOSEngine::o3_screenTextBox;
		opcode_table[165] = &AGOSEngine::o3_isAdjNoun;
		opcode_table[171] = &AGOSEngine::o3_hyperLinkOn;
		opcode_table[172] = &AGOSEngine::o3_hyperLinkOff;
		opcode_table[173] = &AGOSEngine::o3_checkPaths;
		opcode_table[177] = &AGOSEngine::o3_screenTextPObj;
		opcode_table[181] = &AGOSEngine::o3_mouseOff;
		opcode_table[182] = &AGOSEngine::o3_loadVideo;
		opcode_table[183] = &AGOSEngine::o3_playVideo;
		opcode_table[187] = &AGOSEngine::o3_centreScroll;
		opcode_table[188] = &AGOSEngine::o2_isShortText;
		opcode_table[189] = &AGOSEngine::o2_clearMarks;
		opcode_table[190] = &AGOSEngine::o2_waitMark;
		opcode_table[191] = &AGOSEngine::o3_resetPVCount;
		opcode_table[192] = &AGOSEngine::o3_setPathValues;
		opcode_table[193] = &AGOSEngine::o3_stopClock;
		opcode_table[194] = &AGOSEngine::o3_restartClock;
		opcode_table[195] = &AGOSEngine::o3_setColour;
		opcode_table[196] = &AGOSEngine::o3_b3Set;
		opcode_table[197] = &AGOSEngine::o3_b3Clear;
		opcode_table[198] = &AGOSEngine::o3_b3Zero;
		opcode_table[199] = &AGOSEngine::o3_b3NotZero;
		break;
	case GType_PP:
		opcode_table[23] = &AGOSEngine::o3_chance;
		opcode_table[30] = &AGOSEngine::o4_opcode30;
		opcode_table[32] = &AGOSEngine::o4_restoreOopsPosition;
		opcode_table[37] = &AGOSEngine::o4_checkTiles;
		opcode_table[38] = &AGOSEngine::o4_loadMouseImage;
		opcode_table[65] = &AGOSEngine::o3_addTextBox;
		opcode_table[66] = &AGOSEngine::o4_setShortText;
		opcode_table[70] = &AGOSEngine::o3_printLongText;
		opcode_table[83] = &AGOSEngine::o2_rescan;
		opcode_table[98] = &AGOSEngine::o4_animate;
		opcode_table[99] = &AGOSEngine::o2_stopAnimate;
		opcode_table[105] = &AGOSEngine::o4_loadHiScores;
		opcode_table[106] = &AGOSEngine::o4_checkHiScores;
		opcode_table[107] = &AGOSEngine::o3_addBox;
		opcode_table[122] = &AGOSEngine::o3_oracleTextDown;
		opcode_table[123] = &AGOSEngine::o3_oracleTextUp;
		opcode_table[124] = &AGOSEngine::o3_ifTime;
		opcode_table[127] = &AGOSEngine::o3_playTune;
		opcode_table[131] = &AGOSEngine::o3_setTime;
		opcode_table[132] = &AGOSEngine::o3_saveUserGame;
		opcode_table[133] = &AGOSEngine::o4_loadUserGame;
		opcode_table[134] = &AGOSEngine::o3_listSaveGames;
		opcode_table[161] = &AGOSEngine::o3_screenTextBox;
		opcode_table[165] = &AGOSEngine::o3_isAdjNoun;
		opcode_table[166] = NULL;
		opcode_table[167] = NULL;
		opcode_table[168] = NULL;
		opcode_table[169] = NULL;
		opcode_table[171] = &AGOSEngine::o3_hyperLinkOn;
		opcode_table[172] = &AGOSEngine::o3_hyperLinkOff;
		opcode_table[173] = &AGOSEngine::o4_saveOopsPosition;
		opcode_table[177] = &AGOSEngine::o3_screenTextPObj;
		opcode_table[181] = &AGOSEngine::o3_mouseOff;
		opcode_table[187] = &AGOSEngine::o4_resetGameTime;
		opcode_table[188] = &AGOSEngine::o2_isShortText;
		opcode_table[189] = &AGOSEngine::o2_clearMarks;
		opcode_table[190] = &AGOSEngine::o2_waitMark;
		opcode_table[191] = &AGOSEngine::o4_resetPVCount;
		opcode_table[192] = &AGOSEngine::o4_setPathValues;
		opcode_table[193] = &AGOSEngine::o3_stopClock;
		opcode_table[194] = &AGOSEngine::o4_restartClock;
		opcode_table[195] = &AGOSEngine::o3_setColour;
		break;
	default:
		error("setupOpcodes: Unknown game");
	}
}

void AGOSEngine::setScriptCondition(bool cond) {
	_runScriptCondition[_recursionDepth] = cond;
}

bool AGOSEngine::getScriptCondition() {
	return _runScriptCondition[_recursionDepth];
}

void AGOSEngine::setScriptReturn(int ret) {
	_runScriptReturn[_recursionDepth] = ret;
}

int AGOSEngine::getScriptReturn() {
	return _runScriptReturn[_recursionDepth];
}

// -----------------------------------------------------------------------
// Common Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o_at() {
	// 1: ptrA parent is
	setScriptCondition(me()->parent == getNextItemID());
}

void AGOSEngine::o_notAt() {
	// 2: ptrA parent is not
	setScriptCondition(me()->parent != getNextItemID());
}

void AGOSEngine::o_carried() {
	// 5: parent is 1
	setScriptCondition(getNextItemPtr()->parent == getItem1ID());
}

void AGOSEngine::o_notCarried() {
	// 6: parent isnot 1
	setScriptCondition(getNextItemPtr()->parent != getItem1ID());
}

void AGOSEngine::o_isAt() {
	// 7: parent is
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent == getNextItemID());
}

void AGOSEngine::o_zero() {
	// 11: is zero
	setScriptCondition(getNextVarContents() == 0);
}

void AGOSEngine::o_notZero() {
	// 12: isnot zero
	setScriptCondition(getNextVarContents() != 0);
}

void AGOSEngine::o_eq() {
	// 13: equal
	uint tmp = getNextVarContents();
	setScriptCondition(tmp == getVarOrWord());
}

void AGOSEngine::o_notEq() {
	// 14: not equal
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getVarOrWord());
}

void AGOSEngine::o_gt() {
	// 15: is greater
	uint tmp = getNextVarContents();
	setScriptCondition(tmp > getVarOrWord());
}

void AGOSEngine::o_lt() {
	// 16: is less
	uint tmp = getNextVarContents();
	setScriptCondition(tmp < getVarOrWord());
}

void AGOSEngine::o_eqf() {
	// 17: is eq f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp == getNextVarContents());
}

void AGOSEngine::o_notEqf() {
	// 18: is not equal f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getNextVarContents());
}

void AGOSEngine::o_ltf() {
	// 19: is greater f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp < getNextVarContents());
}

void AGOSEngine::o_gtf() {
	// 20: is less f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp > getNextVarContents());
}

void AGOSEngine::o_chance() {
	// 23
	uint a = getVarOrWord();

	if (a == 0) {
		setScriptCondition(false);
		return;
	}

	if (a == 100) {
		setScriptCondition(true);
		return;
	}

	a += _chanceModifier;

	if (a <= 0) {
		_chanceModifier = 0;
		setScriptCondition(false);
	} else if ((uint)_rnd.getRandomNumber(99) < a) {
		if (_chanceModifier <= 0)
			_chanceModifier -= 5;
		else
			_chanceModifier = 0;
		setScriptCondition(true);
	} else {
		if (_chanceModifier >= 0)
			_chanceModifier += 5;
		else
			_chanceModifier = 0;
		setScriptCondition(false);
	}
}

void AGOSEngine::o_isRoom() {
	// 25: is room
	setScriptCondition(isRoom(getNextItemPtr()));
}

void AGOSEngine::o_isObject() {
	// 26: is object
	setScriptCondition(isObject(getNextItemPtr()));
}

void AGOSEngine::o_state() {
	// 27: item state is
	Item *item = getNextItemPtr();
	setScriptCondition((uint) item->state == getVarOrWord());
}

void AGOSEngine::o_oflag() {
	// 28: item has prop
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	uint num = getVarOrByte();
	setScriptCondition(subObject != NULL && (subObject->objectFlags & (1 << num)) != 0);
}

void AGOSEngine::o_destroy() {
	// 31: set no parent
	setItemParent(getNextItemPtr(), NULL);
}

void AGOSEngine::o_place() {
	// 33: set item parent
	Item *item = getNextItemPtr();
	setItemParent(item, getNextItemPtr());
}

void AGOSEngine::o_copyff() {
	// 36: copy var
	uint value = getNextVarContents();
	writeNextVarContents(value);
}

void AGOSEngine::o_clear() {
	// 41: zero var
	writeNextVarContents(0);
}

void AGOSEngine::o_let() {
	// 42: set var
	uint var = getVarWrapper();
	writeVariable(var, getVarOrWord());
}

void AGOSEngine::o_add() {
	// 43: add
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) + getVarOrWord());
}

void AGOSEngine::o_sub() {
	// 44: sub
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) - getVarOrWord());
}

void AGOSEngine::o_addf() {
	// 45: add f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) + getNextVarContents());
}

void AGOSEngine::o_subf() {
	// 46: sub f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) - getNextVarContents());
}

void AGOSEngine::o_mul() {
	// 47: mul
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) * getVarOrWord());
}

void AGOSEngine::o_div() {
	// 48: div
	uint var = getVarWrapper();
	int value = getVarOrWord();
	if (value == 0)
		error("o_div: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void AGOSEngine::o_mulf() {
	// 49: mul f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) * getNextVarContents());
}

void AGOSEngine::o_divf() {
	// 50: div f
	uint var = getVarWrapper();
	int value = getNextVarContents();
	if (value == 0)
		error("o_divf: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void AGOSEngine::o_mod() {
	// 51: mod
	uint var = getVarWrapper();
	int value = getVarOrWord();
	if (value == 0)
		error("o_mod: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void AGOSEngine::o_modf() {
	// 52: mod f
	uint var = getVarWrapper();
	int value = getNextVarContents();
	if (value == 0)
		error("o_modf: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void AGOSEngine::o_random() {
	// 53: random
	uint var = getVarWrapper();
	uint value = (uint16)getVarOrWord();
	writeVariable(var, _rnd.getRandomNumber(value - 1));
}

void AGOSEngine::o_goto() {
	// 55: set itemA parent
	uint item = getNextItemID();
	if (_itemArrayPtr[item] == NULL) {
		setItemParent(me(), NULL);
		loadRoomItems(item);
	}
	setItemParent(me(), _itemArrayPtr[item]);
}

void AGOSEngine::o_oset() {
	// 56: set child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 0x10)
		subObject->objectFlags |= (1 << value);
}

void AGOSEngine::o_oclear() {
	// 57: clear child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 0x10)
		subObject->objectFlags &= ~(1 << value);
}

void AGOSEngine::o_putBy() {
	// 58: make siblings
	Item *item = getNextItemPtr();
	setItemParent(item, derefItem(getNextItemPtr()->parent));
}

void AGOSEngine::o_inc() {
	// 59: item inc state
	Item *item = getNextItemPtr();
	if (item->state <= 30000)
		setItemState(item, item->state + 1);
}

void AGOSEngine::o_dec() {
	// 60: item dec state
	Item *item = getNextItemPtr();
	if (item->state >= 0)
		setItemState(item, item->state - 1);
}

void AGOSEngine::o_setState() {
	// 61: item set state
	Item *item = getNextItemPtr();
	int value = getVarOrWord();
	if (value < 0)
		value = 0;
	if (value > 30000)
		value = 30000;
	setItemState(item, value);
}

void AGOSEngine::o_print() {
	// 62: show int
	showMessageFormat("%d", getNextVarContents());
}

void AGOSEngine::o_message() {
	// 63: show string nl
	showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
}

void AGOSEngine::o_msg() {
	// 64: show string
	showMessageFormat("%s", getStringPtrByID(getNextStringID()));
}

void AGOSEngine::o_addTextBox() {
	// 65: add hit area
	uint id = getVarOrWord();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint w = getVarOrWord();
	uint h = getVarOrWord();
	uint number = getVarOrByte();
	if (number < _numTextBoxes)
		defineBox(id, x, y, w, h, (number << 8) + 129, 208, _dummyItem2);
}

void AGOSEngine::o_setShortText() {
	// 66: set item name
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (var < _numTextBoxes) {
		_shortText[var] = stringId;
	}
}

void AGOSEngine::o_setLongText() {
	// 67: set item description
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (getFeatures() & GF_TALKIE) {
		uint speechId = getNextWord();
		if (var < _numTextBoxes) {
			_longText[var] = stringId;
			_longSound[var] = speechId;
		}
	} else {
		if (var < _numTextBoxes) {
			_longText[var] = stringId;
		}
	}
}

void AGOSEngine::o_end() {
	// 68: exit interpreter
	shutdown();
}

void AGOSEngine::o_done() {
	// 69: return 1
	setScriptReturn(1);
}

void AGOSEngine::o_process() {
	// 71: start subroutine
	Subroutine *sub = getSubroutineByID(getVarOrWord());
	if (sub != NULL)
		startSubroutine(sub);
}

void AGOSEngine::o_when() {
	// 76: add timeout
	uint timeout = getVarOrWord();
	addTimeEvent(timeout, getVarOrWord());
}

void AGOSEngine::o_if1() {
	// 77: has item minus 1
	setScriptCondition(_subjectItem != NULL);
}

void AGOSEngine::o_if2() {
	// 78: has item minus 3
	setScriptCondition(_objectItem != NULL);
}

void AGOSEngine::o_isCalled() {
	// 79: childstruct fr2 is
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	uint stringId = getNextStringID();
	setScriptCondition((subObject != NULL) && subObject->objectName == stringId);
}

void AGOSEngine::o_is() {
	// 80: item equal
	setScriptCondition(getNextItemPtr() == getNextItemPtr());
}

void AGOSEngine::o_debug() {
	// 82: debug opcode
	getVarOrByte();
}

void AGOSEngine::o_comment() {
	// 87: comment
	getNextStringID();
}

void AGOSEngine::o_haltAnimation() {
	// 88: stop animation
	_lockWord |= 0x10;
}

void AGOSEngine::o_restartAnimation() {
	// 89: restart animation
	_lockWord &= ~0x10;
}

void AGOSEngine::o_getParent() {
	// 90: set minusitem to parent
	Item *item = derefItem(getNextItemPtr()->parent);
	switch (getVarOrByte()) {
	case 0:
		_objectItem = item;
		break;
	case 1:
		_subjectItem = item;
		break;
	default:
		error("o_getParent: invalid subcode");
	}
}

void AGOSEngine::o_getNext() {
	// 91: set minusitem to sibling
	Item *item = derefItem(getNextItemPtr()->sibling);
	switch (getVarOrByte()) {
	case 0:
		_objectItem = item;
		break;
	case 1:
		_subjectItem = item;
		break;
	default:
		error("o_getNext: invalid subcode");
	}
}

void AGOSEngine::o_getChildren() {
	// 92: set minusitem to child
	Item *item = derefItem(getNextItemPtr()->child);
	switch (getVarOrByte()) {
	case 0:
		_objectItem = item;
		break;
	case 1:
		_subjectItem = item;
		break;
	default:
		error("o_getChildren: invalid subcode");
	}
}

void AGOSEngine::o_picture() {
	// 96
	uint vga_res = getVarOrWord();
	uint mode = getVarOrByte();

	if (mode == 4)
		vc29_stopAllSounds();

	if (_lockWord & 0x10)
		error("o_picture: _lockWord & 0x10");

	set_video_mode_internal(mode, vga_res);
}

void AGOSEngine::o_loadZone() {
	// 97: load vga
	uint vga_res = getVarOrWord();

	_lockWord |= 0x80;
	loadZone(vga_res);
	_lockWord &= ~0x80;
}

void AGOSEngine::o_killAnimate() {
	// 100: vga reset
	_lockWord |= 0x8000;
	vc27_resetSprite();
	_lockWord &= ~0x8000;
}

void AGOSEngine::o_defWindow() {
	// 101
	uint num = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint w = getVarOrWord();
	uint h = getVarOrWord();
	uint flags = getVarOrWord();
	uint fill_color = getVarOrWord();
	uint text_color = 0;

	num &= 7;

	if (_windowArray[num])
		closeWindow(num);

	_windowArray[num] = openWindow(x, y, w, h, flags, fill_color, text_color);

	if (num == _curWindow) {
		_textWindow = _windowArray[num];
		if (getGameType() == GType_FF)
			showmessage_helper_3(_textWindow->textColumn, _textWindow->width);
		else
			showmessage_helper_3(_textWindow->textLength, _textWindow->textMaxLength);
	}
}

void AGOSEngine::o_window() {
	// 102
	changeWindow(getVarOrByte() & 7);
}

void AGOSEngine::o_cls() {
	// 103
	mouseOff();
	removeIconArray(_curWindow);
	showMessageFormat("\x0C");
	_oracleMaxScrollY = 0;
	_noOracleScroll = 0;
	mouseOn();
}

void AGOSEngine::o_closeWindow() {
	// 104
	closeWindow(getVarOrByte() & 7);
}

void AGOSEngine::o_addBox() {
	// 107: add item hitarea
	uint flags = 0;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, verb;
	Item *item;

	id = id % 1000;

	if (params & 1)
		flags |= kBFInvertTouch;
	if (params & 2)
		flags |= kBFNoTouchName;
	if (params & 4)
		flags |= kBFBoxItem;
	if (params & 8)
		flags |= kBFTextBox;
	if (params & 16)
		flags |= 0x10;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	item = getNextItemPtrStrange();
	verb = getVarOrWord();
	if (x >= 1000) {
		verb += 0x4000;
		x -= 1000;
	}
	defineBox(id, x, y, w, h, flags, verb, item);
}

void AGOSEngine::o_delBox() {
	// 108: delete hitarea
	undefineBox(getVarOrWord());
}

void AGOSEngine::o_enableBox() {
	// 109: clear hitarea bit 0x40
	enableBox(getVarOrWord());
}

void AGOSEngine::o_disableBox() {
	// 110: set hitarea bit 0x40
	disableBox(getVarOrWord());
}

void AGOSEngine::o_moveBox() {
	// 111: set hitarea xy
	uint hitarea_id = getVarOrWord();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	moveBox(hitarea_id, x, y);
}

void AGOSEngine::o_doIcons() {
	// 114
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	mouseOff();
	drawIconArray(num, item, 0, 0);
	mouseOn();
}

void AGOSEngine::o_isClass() {
	// 115: item has flag
	Item *item = getNextItemPtr();
	setScriptCondition((item->classFlags & (1 << getVarOrByte())) != 0);
}

void AGOSEngine::o_setClass() {
	// 116: item set flag
	Item *item = getNextItemPtr();
	item->classFlags |= (1 << getVarOrByte());
}

void AGOSEngine::o_unsetClass() {
	// 117: item clear flag
	Item *item = getNextItemPtr();
	item->classFlags &= ~(1 << getVarOrByte());
}

void AGOSEngine::o_waitSync() {
	// 119: wait vga
	uint var = getVarOrWord();
	_scriptVar2 = (var == 200);

	if (var != 200 || !_skipVgaWait)
		waitForSync(var);
	_skipVgaWait = false;
}

void AGOSEngine::o_sync() {
	// 120: sync
	sendSync(getVarOrWord());
}

void AGOSEngine::o_defObj() {
	// 121: set vga item
	uint slot = getVarOrByte();
	_objectArray[slot] = getNextItemPtr();
}

void AGOSEngine::o_here() {
	// 125: item is sibling with item 1
	Item *item = getNextItemPtr();
	setScriptCondition(me()->parent == item->parent);
}

void AGOSEngine::o_doClassIcons() {
	// 126
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	uint a = 1 << getVarOrByte();
	mouseOff();
	drawIconArray(num, item, 1, a);
	mouseOn();
}

void AGOSEngine::o_waitEndTune() {
	// 128: dummy instruction
	getVarOrWord();
}

void AGOSEngine::o_ifEndTune() {
	// 129: dummy instruction
	getVarOrWord();
	setScriptCondition(true);
}

void AGOSEngine::o_setAdjNoun() {
	// 130: set adj noun
	uint var = getVarOrByte();
	if (var == 1) {
		_scriptAdj1 = getNextWord();
		_scriptNoun1 = getNextWord();
	} else {
		_scriptAdj2 = getNextWord();
		_scriptNoun2 = getNextWord();
	}
}

void AGOSEngine::o_saveUserGame() {
	// 132: save game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	userGame(false);
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::o_loadUserGame() {
	// 133: load game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	userGame(true);
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::o_stopTune() {
	// 134: dummy opcode?
	midi.stop();
	_lastMusicPlayed = -1;
}

void AGOSEngine::o_pauseGame() {
	// 135: quit if user presses y
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	// If all else fails, use English as fallback.
	byte keyYes = 'y';
	byte keyNo = 'n';

	switch (_language) {
	case Common::RU_RUS:
		break;
	case Common::PL_POL:
		keyYes = 't';
		break;
	case Common::HB_ISR:
		keyYes = 'f';
		break;
	case Common::ES_ESP:
		keyYes = 's';
		break;
	case Common::IT_ITA:
		keyYes = 's';
		break;
	case Common::FR_FRA:
		keyYes = 'o';
		break;
	case Common::DE_DEU:
		keyYes = 'j';
		break;
	default:
		break;
	}

	for (;;) {
		delay(1);
#ifdef _WIN32_WCE
		if (isSmartphone()) {
			if (_keyPressed) {
				if (_keyPressed == 13)
					shutdown();
				else
					break;
			}
		}
#endif
		if (_keyPressed == keyYes || _keyPressed == (keyYes - 32))
			shutdown();
		else if (_keyPressed == keyNo || _keyPressed == (keyNo - 32))
			break;
	}

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::o_copysf() {
	// 136: set var to item unk3
	Item *item = getNextItemPtr();
	writeNextVarContents(item->state);
}

void AGOSEngine::o_restoreIcons() {
	// 137
	uint num = getVarOrByte();
	WindowBlock *window = _windowArray[num & 7];
	if (window->iconPtr)
		drawIconArray(num, window->iconPtr->itemRef, window->iconPtr->line, window->iconPtr->classMask);
}

void AGOSEngine::o_freezeZones() {
	// 138: vga pointer op 4
	freezeBottom();
}

void AGOSEngine::o_placeNoIcons() {
	// 139: set parent special
	Item *item = getNextItemPtr();
	_noParentNotify = true;
	setItemParent(item, getNextItemPtr());
	_noParentNotify = false;
}

void AGOSEngine::o_clearTimers() {
	// 140: del te and add one
	killAllTimers();
	addTimeEvent(3, 0xA0);
}

void AGOSEngine::o_setDollar() {
	// 141: set m1 to m3
	uint which = getVarOrByte();
	Item *item = getNextItemPtr();
	if (which == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void AGOSEngine::o_isBox() {
	// 142: is hitarea 0x40 clear
	setScriptCondition(isBoxDead(getVarOrWord()));
}

void AGOSEngine::o_doTable() {
	// 143: start item sub
	SubRoom *subRoom = (SubRoom *)findChildOfType(getNextItemPtr(), 1);
	if (subRoom != NULL) {
		Subroutine *sub = getSubroutineByID(subRoom->subroutine_id);
		if (sub)
			startSubroutine(sub);
	}
}

void AGOSEngine::o_storeItem() {
	// 151: set array6 to item
	uint var = getVarOrByte();
	Item *item = getNextItemPtr();
	_itemStore[var] = item;
}

void AGOSEngine::o_getItem() {
	// 152: set m1 to m3 to array 6
	Item *item = _itemStore[getVarOrByte()];
	uint var = getVarOrByte();
	if (var == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void AGOSEngine::o_bSet() {
	// 153: set bit
	setBitFlag(getVarWrapper(), true);
}

void AGOSEngine::o_bClear() {
	// 154: clear bit
	setBitFlag(getVarWrapper(), false);
}

void AGOSEngine::o_bZero() {
	// 155: is bit clear
	setScriptCondition(!getBitFlag(getVarWrapper()));
}

void AGOSEngine::o_bNotZero() {
	// 156: is bit set
	uint bit = getVarWrapper();

	// WORKAROUND: Fix for glitch in some versions
	if (getGameType() == GType_SIMON1 && _subroutine == 2962 && bit == 63) {
		bit = 50;
	}

	setScriptCondition(getBitFlag(bit));
}

void AGOSEngine::o_getOValue() {
	// 157: get item int prop
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, 2);
	uint prop = getVarOrByte();

	if (subObject != NULL && subObject->objectFlags & (1 << prop) && prop < 16) {
		uint offs = getOffsetOfChild2Param(subObject, 1 << prop);
		writeNextVarContents(subObject->objectFlagValue[offs]);
	} else {
		writeNextVarContents(0);
	}
}

void AGOSEngine::o_setOValue() {
	// 158: set item prop
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, 2);
	uint prop = getVarOrByte();
	int value = getVarOrWord();

	if (subObject != NULL && subObject->objectFlags & (1 << prop) && prop < 16) {
		uint offs = getOffsetOfChild2Param(subObject, 1 << prop);
		subObject->objectFlagValue[offs] = value;
	}
}

void AGOSEngine::o_ink() {
	// 160
	setTextColor(getVarOrByte());
}

void AGOSEngine::o_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrByte();
	tl->width = getVarOrWord();
}
	
void AGOSEngine::o_screenTextMsg() {
	// 162: print string
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint stringId = getNextStringID();
	const byte *string_ptr = NULL;
	uint speechId = 0;
	TextLocation *tl;

	if (stringId != 0xFFFF)
		string_ptr = getStringPtrByID(stringId);

	if (getFeatures() & GF_TALKIE) {
		if (getGameType() == GType_FF || getGameType() == GType_PP)
			speechId = (uint16)getVarOrWord();
		else
			speechId = (uint16)getNextWord();
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		vgaSpriteId = 1;

	tl = getTextLocation(vgaSpriteId);
	if (_speech && speechId != 0)
		playSpeech(speechId, vgaSpriteId);
	if (((getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE)) || getGameType() == GType_FF) &&
		speechId == 0) {
		stopAnimateSimon2(2, vgaSpriteId + 2);
	}

	if (string_ptr != NULL && (speechId == 0 || _subtitles))
		printScreenText(vgaSpriteId, color, (const char *)string_ptr, tl->x, tl->y, tl->width);

}

void AGOSEngine::o_playEffect() {
	// 163: play sound
	uint soundId = getVarOrWord();

	if (getGameType() == GType_FF)
		error("o_playEffect: triggered");

	if (getGameId() == GID_SIMON1DOS)
		playSting(soundId);
	else
		_sound->playEffects(soundId);
}

void AGOSEngine::o_getDollar2() {
	// 164
	_showPreposition = true;

	setup_cond_c_helper();

	_objectItem = _hitAreaObjectItem;

	if (_objectItem == _dummyItem2)
		_objectItem = me();

	if (_objectItem == _dummyItem3)
		_objectItem = derefItem(me()->parent);

	if (_objectItem != NULL) {
		_scriptNoun2 = _objectItem->noun;
		_scriptAdj2 = _objectItem->adjective;
	} else {
		_scriptNoun2 = -1;
		_scriptAdj2 = -1;
	}

	_showPreposition = false;
}

void AGOSEngine::o_isAdjNoun() {
	// 165: item unk1 unk2 is
	Item *item = getNextItemPtr();
	int16 a = getNextWord(), b = getNextWord();
	setScriptCondition(item->adjective == a && item->noun == b);
}

void AGOSEngine::o_b2Set() {
	// 166: set bit2
	uint bit = getVarOrByte();
	_bitArrayTwo[bit / 16] |= (1 << (bit & 15));
}

void AGOSEngine::o_b2Clear() {
	// 167: clear bit2
	uint bit = getVarOrByte();
	_bitArrayTwo[bit / 16] &= ~(1 << (bit & 15));
}

void AGOSEngine::o_b2Zero() {
	// 168: is bit2 clear
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayTwo[bit / 16] & (1 << (bit & 15))) == 0);
}

void AGOSEngine::o_b2NotZero() {
	// 169: is bit2 set
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayTwo[bit / 16] & (1 << (bit & 15))) != 0);
}

void AGOSEngine::o_lockZones() {
	// 175: vga pointer op 1
	_vgaMemBase = _vgaMemPtr;
}

void AGOSEngine::o_unlockZones() {
	// 176: vga pointer op 2
	_vgaMemPtr = _vgaFrozenBase;
	_vgaMemBase = _vgaFrozenBase;
}

void AGOSEngine::o_getPathPosn() {
	// 178: path find
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint var_1 = getVarOrByte();
	uint var_2 = getVarOrByte();

	const uint16 *p;
	uint i, j;
	uint prev_i;
	uint x_diff, y_diff;
	uint best_i = 0, best_j = 0, best_dist = 0xFFFFFFFF;
	uint maxPath = (getGameType() == GType_FF) ? 100 : 20;

	if (getGameType() == GType_FF) {
		x += _scrollX;
		y += _scrollY;
	}
	if (getGameType() == GType_SIMON2) {
		x += _scrollX * 8;
	}

	int end = (getGameType() == GType_FF) ? 9999 : 999;
	prev_i = maxPath + 1 - readVariable(12);
	for (i = maxPath; i != 0; --i) {
		p = (const uint16 *)_pathFindArray[maxPath - i];
		if (!p)
			continue;
		for (j = 0; readUint16Wrapper(&p[0]) != end; j++, p += 2) {
			x_diff = ABS((int16)(readUint16Wrapper(&p[0]) - x));
			y_diff = ABS((int16)(readUint16Wrapper(&p[1]) - 12 - y));

			if (x_diff < y_diff) {
				x_diff /= 4;
				y_diff *= 4;
			}
			x_diff += y_diff /= 4;

			if (x_diff < best_dist || x_diff == best_dist && prev_i == i) {
				best_dist = x_diff;
				best_i = maxPath + 1 - i;
				best_j = j;
			}
		}
	}

	writeVariable(var_1, best_i);
	writeVariable(var_2, best_j);
}

void AGOSEngine::o_scnTxtLongText() {
	// 179: conversation responses and room descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint stringId = getVarOrByte();
	uint speechId = 0;
	TextLocation *tl;

	const char *string_ptr = (const char *)getStringPtrByID(_longText[stringId]);
	if (getFeatures() & GF_TALKIE)
		speechId = _longSound[stringId];

	if (getGameType() == GType_FF)
		vgaSpriteId = 1;
	tl = getTextLocation(vgaSpriteId);

	if (_speech && speechId != 0)
		playSpeech(speechId, vgaSpriteId);
	if (string_ptr != NULL && _subtitles)
		printScreenText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
}

void AGOSEngine::o_mouseOn() {
	// 180: force mouseOn
	scriptMouseOn();
}

void AGOSEngine::o_unloadZone() {
	// 184: clear vgapointer entry
	uint a = getVarOrWord();
	VgaPointersEntry *vpe = &_vgaBufferPointers[a];

	vpe->sfxFile = NULL;
	vpe->vgaFile1 = NULL;
	vpe->vgaFile2 = NULL;
}

void AGOSEngine::o_unfreezeZones() {
	// 186: vga pointer op 3
	unfreezeBottom();
}

// -----------------------------------------------------------------------
// Elvira 1 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::oe1_setFF() {
	writeNextVarContents(0xFF);
}

void AGOSEngine::oe1_zoneDisk() {
	getVarOrWord();
	getVarOrWord();
}

void AGOSEngine::oe1_opcode176() {
	getNextItemPtr();
	getVarOrWord();
	getNextItemPtr();
}


void AGOSEngine::oe1_opcode178() {
	getNextItemPtr();
	getVarOrWord();
}


// -----------------------------------------------------------------------
// Waxworks Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::oww_whereTo() {
	// 85: where to
	Item *i = getNextItemPtr();
	int16 d = getVarOrByte();
	int16 f = getVarOrByte();

	if (f == 1)
		_subjectItem = _itemArrayPtr[getExitOf(i, d)];
	else
		_objectItem = _itemArrayPtr[getExitOf(i, d)];
}

void AGOSEngine::oww_menu() {
	// 105: menu
	getVarOrByte();
}

void AGOSEngine::oww_textMenu() {
	// 106: text menu

	/* byte tmp = getVarOrByte();
	TextMenu[tmp] = getVarOrByte(); */

	getVarOrByte();
	getVarOrByte();
}

void AGOSEngine::oww_ifDoorOpen() {
	// 148: if door open
	Item *item = getNextItemPtr();
	uint16 d = getVarOrByte();
	setScriptCondition(getDoorState(item, d) != 0);
}

// -----------------------------------------------------------------------
// Simon 1 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o1_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_longText[getVarOrByte()]);
	showMessageFormat("%s\n", str);
}

void AGOSEngine::o1_rescan() {
	// 83: restart subroutine
	setScriptReturn(-10);
}

void AGOSEngine::o1_animate() {
	// 98: start vga
	uint vga_res, vgaSpriteId, windowNum, x, y, palette;
	vgaSpriteId = getVarOrWord();
	vga_res = vgaSpriteId / 100;
	windowNum = getVarOrByte();
	x = getVarOrWord();
	y = getVarOrWord();
	palette = getVarOrWord();
	loadSprite(windowNum, vga_res, vgaSpriteId, x, y, palette);
}

void AGOSEngine::o1_stopAnimate() {
	// 99: kill sprite
	stopAnimateSimon1(getVarOrWord());
}

void AGOSEngine::o1_playTune() {
	// 127: deals with music
	int music = getVarOrWord();
	int track = getVarOrWord();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).

	if (music != _lastMusicPlayed) {
		_lastMusicPlayed = music;
		loadMusic(music);
		midi.startTrack(track);
	}
}

void AGOSEngine::o1_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
			playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
		} else if (subObject != NULL && subObject->objectFlags & kOFNumber) {
			uint offs = getOffsetOfChild2Param(subObject, kOFNumber);
			playSpeech(subObject->objectFlagValue[offs] + 3550, vgaSpriteId);
		}
	}

	if (subObject != NULL && subObject->objectFlags & kOFText && _subtitles) {
		const char *stringPtr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		TextLocation *tl = getTextLocation(vgaSpriteId);
		char buf[256];
		int j, k;

		if (subObject->objectFlags & kOFNumber) {
			if (_language == Common::HB_ISR) {
				j = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];
				k = (j % 10) * 10;
				k += j / 10;
				if (!(j % 10))
					sprintf(buf,"0%d%s", k, stringPtr);
				else
					sprintf(buf,"%d%s", k, stringPtr);
			} else {
				sprintf(buf,"%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], stringPtr);
			}
			stringPtr = buf;
		}
		if (stringPtr != NULL)
			printScreenText(vgaSpriteId, color, stringPtr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine::o1_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
}

void AGOSEngine::o1_loadBeard() {
	// 182: load beard
	if (_beardLoaded == false) {
		_beardLoaded = true;
		_lockWord |= 0x8000;
		loadSimonVGAFile(328);
		_lockWord &= ~0x8000;
	}
}

void AGOSEngine::o1_unloadBeard() {
	// 183: unload beard
	if (_beardLoaded == true) {
		_beardLoaded = false;
		_lockWord |= 0x8000;
		loadSimonVGAFile(23);
		_lockWord &= ~0x8000;
	}
}

void AGOSEngine::o1_loadStrings() {
	// 185: load sound files
	_soundFileId = getVarOrWord();
	if (getPlatform() == Common::kPlatformAmiga && getFeatures() & GF_TALKIE) {
		char buf[10];
		sprintf(buf, "%d%s", _soundFileId, "Effects");
		_sound->readSfxFile(buf);
		sprintf(buf, "%d%s", _soundFileId, "simon");
		_sound->readVoiceFile(buf);
	}
}

void AGOSEngine::o1_specialFade() {
	// 187: fade to black
	uint i;

	memcpy(_videoBuf1, _currentPalette, 4 * 256);

	for (i = 32; i != 0; --i) {
		paletteFadeOut(_videoBuf1, 32, 8);
		paletteFadeOut(_videoBuf1 + 4 * 48, 144, 8);
		paletteFadeOut(_videoBuf1 + 4 * 208, 48, 8);
		_system->setPalette(_videoBuf1, 0, 256);
		delay(5);
	}

	memcpy(_currentPalette, _videoBuf1, 1024);
	memcpy(_displayPalette, _videoBuf1, 1024);
}

// -----------------------------------------------------------------------
// Simon 2 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o2_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_longText[getVarOrByte()]);
	writeVariable(51, strlen(str) / 53 * 8 + 8);
	showMessageFormat("%s\n", str);
}

void AGOSEngine::o2_rescan() {
	// 83: restart subroutine
	if (_exitCutscene) {
		if (getBitFlag(9)) {
			endCutscene();
		}
	} else {
		processSpecialKeys();
	}

	setScriptReturn(-10);
}

void AGOSEngine::o2_animate() {
	// 98: start vga
	uint vga_res = getVarOrWord();
	uint vgaSpriteId = getVarOrWord();
	uint windowNum = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint palette = getVarOrWord();
	loadSprite(windowNum, vga_res, vgaSpriteId, x, y, palette);
}

void AGOSEngine::o2_stopAnimate() {
	// 99: kill sprite
	uint a = getVarOrWord();
	uint b = getVarOrWord();
	stopAnimateSimon2(a, b);
}

void AGOSEngine::o2_playTune() {
	// 127: deals with music
	int music = getVarOrWord();
	int track = getVarOrWord();
	int loop = getVarOrByte();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).

	midi.setLoop(loop != 0);
	if (_lastMusicPlayed != music)
		_nextMusicToPlay = music;
	else
		midi.startTrack(track);
}

void AGOSEngine::o2_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint speechId = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFVoice)];

			if (subObject->objectFlags & kOFNumber) {
				uint speechIdOffs = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];

				if (speechId == 116)
					speechId = speechIdOffs + 115;
				if (speechId == 92)
					speechId = speechIdOffs + 98;
				if (speechId == 99)
					speechId = 9;
				if (speechId == 97) {
					switch (speechIdOffs) {
					case 12:
						speechId = 109;
						break;
					case 14:
						speechId = 108;
						break;
					case 18:
						speechId = 107;
						break;
					case 20:
						speechId = 106;
						break;
					case 22:
						speechId = 105;
						break;
					case 28:
						speechId = 104;
						break;
					case 90:
						speechId = 103;
						break;
					case 92:
						speechId = 102;
						break;
					case 100:
						speechId = 51;
						break;
					default:
						error("o2_screenTextPObj: invalid case %d", speechIdOffs);
					}
				}
			}

			if (_speech)
				playSpeech(speechId, vgaSpriteId);
		}

	}

	if (subObject != NULL && subObject->objectFlags & kOFText && _subtitles) {
		const char *stringPtr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		TextLocation *tl = getTextLocation(vgaSpriteId);
		char buf[256];
		int j, k;

		if (subObject->objectFlags & kOFNumber) {
			if (_language == Common::HB_ISR) {
				j = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];
				k = (j % 10) * 10;
				k += j / 10;
				if (!(j % 10))
					sprintf(buf,"0%d%s", k, stringPtr);
				else
					sprintf(buf,"%d%s", k, stringPtr);
			} else {
				sprintf(buf,"%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], stringPtr);
			}
			stringPtr = buf;
		}
		if (stringPtr != NULL)
			printScreenText(vgaSpriteId, color, stringPtr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine::o2_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
	changeWindow(1);
	showMessageFormat("\xC");
}

void AGOSEngine::o2_isShortText() {
	// 188: string2 is
	uint i = getVarOrByte();
	uint str = getNextStringID();
	setScriptCondition(str < _numTextBoxes && _shortText[i] == str);
}

void AGOSEngine::o2_clearMarks() {
	// 189: clear_op189_flag
	_marks = 0;
}

void AGOSEngine::o2_waitMark() {
	// 190
	uint i = getVarOrByte();
	if (!(_marks & (1 << i)))
		waitForMark(i);
}

// -----------------------------------------------------------------------
// Feeble Files Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o3_chance() {
	// 23
	uint a = getVarOrWord();

	if (a == 0) {
		setScriptCondition(false);
		return;
	}

	if (a == 100) {
		setScriptCondition(true);
		return;
	}

	if ((uint)_rnd.getRandomNumber(99) < a)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine::o3_jumpOut() {
	// 37
	getVarOrByte();
	setScriptReturn(1);
}

void AGOSEngine::o3_addTextBox() {
	// 65: add hit area
	uint flags = kBFTextBox | kBFBoxItem;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, num;

	id %= 1000;

	if (params & 1)
		flags |= kBFInvertTouch;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	num = getVarOrByte();
	if (num < _numTextBoxes)
		defineBox(id, x, y, w, h, flags + (num << 8), 208, _dummyItem2);
}

void AGOSEngine::o3_printLongText() {
	// 70: show string from array
	int num = getVarOrByte();
	const char *str = (const char *)getStringPtrByID(_longText[num]);
	sendInteractText(num, "%d. %s\n", num, str);
}

void AGOSEngine::o3_addBox() {
	// 107: add item hitarea
	uint flags = 0;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, verb;
	Item *item;

	id = id % 1000;

	if (params & 1)
		flags |= kBFInvertTouch;
	if (params & 2)
		flags |= kBFNoTouchName;
	if (params & 4)
		flags |= kBFBoxItem;
	if (params & 8)
		flags |= kBFTextBox;
	if (params & 16)
		flags |= 0x10;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	item = getNextItemPtrStrange();
	verb = getVarOrWord();
	defineBox(id, x, y, w, h, flags, verb, item);
}

void AGOSEngine::o3_oracleTextDown() {
	// 122: oracle text down
	oracleTextDown();
}

void AGOSEngine::o3_oracleTextUp() {
	// 123: oracle text up
	oracleTextUp();
}

void AGOSEngine::o3_ifTime() {
	// 124: if time
	time_t t;

	uint a = getVarOrWord();
	time(&t);
	t -= _gameStoppedClock;
	t -= a;
	if (t >= _timeStore)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine::o3_playTune() {
	// 127: usually deals with music, but is a no-op in FF.
	getVarOrWord();
	getVarOrWord();
	getVarOrByte();
}

void AGOSEngine::o3_setTime() {
	// 131
	time(&_timeStore);
	_timeStore -= _gameStoppedClock;
}

void AGOSEngine::o3_saveUserGame() {
	// 132: save game
	_noOracleScroll = 0;
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	saveUserGame(countSaveGames() + 1 - readVariable(55));
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::o3_loadUserGame() {
	// 133: load game
	loadGame(readVariable(55));
}

void AGOSEngine::o3_listSaveGames() {
	// 134: dummy opcode?
	listSaveGames(1);
}

void AGOSEngine::o3_checkCD() {
	// 135: switch CD
	uint disc = readVariable(97);

	if (!strcmp(_gameDescription->extra, "4CD")) {
		_sound->switchVoiceFile(gss, disc);
	} else if (!strcmp(_gameDescription->extra, "2CD")) {
		if (disc == 1 || disc == 2)
			_sound->switchVoiceFile(gss, 1);
		else if (disc == 3 || disc == 4)
			_sound->switchVoiceFile(gss, 2);
	}

	debug(0, "Switch to CD number %d", disc);
}

void AGOSEngine::o3_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrWord();
	tl->width = getVarOrWord();
}

void AGOSEngine::o3_isAdjNoun() {
	// 165: item unk1 unk2 is
	Item *item = getNextItemPtr();
	int16 a = getNextWord(), b = getNextWord();
	if (item->adjective == a && item->noun == b)
		setScriptCondition(true);
	else if (a == -1 && item->noun == b)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine::o3_hyperLinkOn() {
	// 171: oracle hyperlink on
	hyperLinkOn(getVarOrWord());
}

void AGOSEngine::o3_hyperLinkOff() {
	// 172: oracle hyperlink off
	hyperLinkOff();
}

void AGOSEngine::o3_checkPaths() {
	// 173 check paths
	int i, count;
	const uint8 *pathVal1 = _pathValues1;
	bool result = false;

	count = _variableArray2[38];
	for (i = 0; i < count; i++) {
		uint8 val = pathVal1[2];
		if (val == _variableArray2[50] ||
			val == _variableArray2[51] ||
			val == _variableArray2[201] ||
			val == _variableArray2[203] ||
			val == _variableArray2[205] ||
			val == _variableArray2[207] ||
			val == _variableArray2[209] ||
			val == _variableArray2[211] ||
			val == _variableArray2[213] ||
			val == _variableArray2[215] ||
			val == _variableArray2[219] ||
			val == _variableArray2[220] ||
			val == _variableArray2[221] ||
			val == _variableArray2[222] ||
			val == _variableArray2[223] ||
			val == _variableArray2[224] ||
			val == _variableArray2[225] ||
			val == _variableArray2[226]) {
				result = true;
				break;
		}
		pathVal1 += 4;
	}

	_variableArray2[52] = result;
}

void AGOSEngine::o3_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	const char *string_ptr = NULL;
	TextLocation *tl = NULL;
	char buf[256];

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (subObject != NULL && subObject->objectFlags & kOFText) {
		string_ptr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		tl = getTextLocation(vgaSpriteId);
	}

	if (subObject != NULL && subObject->objectFlags & kOFVoice) {
		uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
		playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
	}

	if (subObject != NULL && (subObject->objectFlags & kOFText) && _subtitles) {
		if (subObject->objectFlags & kOFNumber) {
			sprintf(buf, "%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], string_ptr);
			string_ptr = buf;
		}
		if (string_ptr != NULL)
			printScreenText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine::o3_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
	clearName();
}

void AGOSEngine::o3_loadVideo() {
	// 182: load video file
	const byte *filename = getStringPtrByID(getNextStringID());
	_moviePlay->load((const char *)filename);
}

void AGOSEngine::o3_playVideo() {
	// 183: play video
	_moviePlay->play();
}

void AGOSEngine::o3_centreScroll() {
	// 187
	centreScroll();
}

void AGOSEngine::o3_resetPVCount() {
	// 191
	if (getBitFlag(83)) {
		_PVCount1 = 0;
		_GPVCount1 = 0;
	} else {
		_PVCount = 0;
		_GPVCount = 0;
	}
}

void AGOSEngine::o3_setPathValues() {
	// 192
	uint8 a = getVarOrByte();
	uint8 b = getVarOrByte();
	uint8 c = getVarOrByte();
	uint8 d = getVarOrByte();
	if (getBitFlag(83)) {
		_pathValues1[_PVCount1++] = a;
		_pathValues1[_PVCount1++] = b;
		_pathValues1[_PVCount1++] = c;
		_pathValues1[_PVCount1++] = d;
	} else {
		_pathValues[_PVCount++] = a;
		_pathValues[_PVCount++] = b;
		_pathValues[_PVCount++] = c;
		_pathValues[_PVCount++] = d;
	}
}

void AGOSEngine::o3_stopClock() {
	// 193: pause clock
	_clockStopped = time(NULL);
}

void AGOSEngine::o3_restartClock() {
	// 194: resume clock
	if (_clockStopped != 0)
		_gameStoppedClock += time(NULL) - _clockStopped;
	_clockStopped = 0;
}

void AGOSEngine::o3_setColour() {
	// 195: set palette colour
	uint c = getVarOrByte() * 4;
	uint r = getVarOrByte();
	uint g = getVarOrByte();
	uint b = getVarOrByte();

	_displayPalette[c + 0] = r;
	_displayPalette[c + 1] = g;
	_displayPalette[c + 2] = b;

	_paletteFlag = 2;
}

void AGOSEngine::o3_b3Set() {
	// 196: set bit3
	uint bit = getVarOrByte();
	_bitArrayThree[bit / 16] |= (1 << (bit & 15));
}

void AGOSEngine::o3_b3Clear() {
	// 197: clear bit3
	uint bit = getVarOrByte();
	_bitArrayThree[bit / 16] &= ~(1 << (bit & 15));
}

void AGOSEngine::o3_b3Zero() {
	// 198: is bit3 clear
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayThree[bit / 16] & (1 << (bit & 15))) == 0);
}

void AGOSEngine::o3_b3NotZero() {
	// 199: is bit3 set
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayThree[bit / 16] & (1 << (bit & 15))) != 0);
}

// -----------------------------------------------------------------------
// Puzzle Pack Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o4_opcode30() {
	// 30
	getNextItemPtr();
}

void AGOSEngine::o4_restoreOopsPosition() {
	// 32: restore oops position
	getNextItemPtr();
	if (_oopsValid) {
		for (uint i = 0; i < _numVars; i++) {
			_variableArray[i] = _variableArray2[i];
		}
//		i=FlagData[999]*100+11;
//		C_PICTURE(4,i);
		if (getBitFlag(110)) {
			_gameTime += 10;
		} else {
			// Swampy adventures
			_gameTime += 30;
		}
		_oopsValid = false;
	}
}

void AGOSEngine::o4_checkTiles() {
	// 37: for MahJongg game
	getVarOrByte();
}

void AGOSEngine::o4_loadMouseImage() {
	// 38: load mouse image
	getVarOrByte();
	getNextItemPtr();
	loadMouseImage();
}

void AGOSEngine::o4_setShortText() {
	// 66: set item name
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (var < _numTextBoxes) {
		_shortText[var] = stringId;
		_shortTextX[var] = getVarOrWord();
		_shortTextY[var] = getVarOrWord();
	}
}

void AGOSEngine::o4_animate() {
	// 98: start vga
	uint vga_res = getVarOrWord();
	uint vgaSpriteId = getVarOrWord();
	uint windowNum = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint palette = getVarOrWord();

	if (getBitFlag(96)) {
		printf("Start Block\n");
	} else {
		loadSprite(windowNum, vga_res, vgaSpriteId, x, y, palette);
	}
}

void AGOSEngine::o4_loadHiScores() {
	// 105: load high scores
	getVarOrByte();
}

void AGOSEngine::o4_checkHiScores() {
	// 106: check high scores
	getVarOrByte();
	getVarOrByte();
}

void AGOSEngine::o4_loadUserGame() {
	// 133: load usergame
}

void AGOSEngine::o4_saveOopsPosition() {
	// 173: save oops position
//	if (IsVQueueEmpty() == 0) {
		_oopsValid = true;
		for (uint i = 0; i < _numVars; i++) {
			_variableArray2[i] = _variableArray[i];
		}
//	} else
//		_oopsValid = false;
}

void AGOSEngine::o4_resetGameTime() {
	// 187: reset game time
	_gameTime = 0;
}

void AGOSEngine::o4_resetPVCount() {
	// 191
	_PVCount = 0;
	_GPVCount = 0;
}

void AGOSEngine::o4_setPathValues() {
	// 192
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
}

void AGOSEngine::o4_restartClock() {
	// 194: resume clock
	if (_clockStopped != 0)
		_gameTime += time(NULL) - _clockStopped;
	_clockStopped = 0;
}

// -----------------------------------------------------------------------

int AGOSEngine::runScript() {
	int opcode;
	bool flag;

	do {
		if (_continousMainScript)
			dumpOpcode(_codePtr);

		if (getGameType() == GType_ELVIRA || getGameType() == GType_ELVIRA2) {
			opcode = getVarOrWord();
			if (opcode == 10000)
				return 0;
		} else {
			opcode = getByte();
			if (opcode == 0xFF)
				return 0;
		}
		debug(1, "runScript: opcode %d", opcode);

		if (_runScriptReturn1)
			return 1;

		/* Invert condition? */
		flag = false;
		if (getGameType() == GType_ELVIRA || getGameType() == GType_ELVIRA2) {
			if (opcode == 203) {
				flag = true;
				opcode = getVarOrWord();
				if (opcode == 10000)
					return 0;
			}
		} else {
			if (opcode == 0) {
				flag = true;
				opcode = getByte();
				if (opcode == 0xFF)
					return 0;
			}
		}

		setScriptCondition(true);
		setScriptReturn(0);

		if (opcode > _numOpcodes || !_opcode_table[opcode])
			error("Invalid opcode '%d' encountered", opcode);

		(this->*_opcode_table[opcode]) ();
	} while (getScriptCondition() != flag && !getScriptReturn());

	return getScriptReturn();
}

void AGOSEngine::scriptMouseOn() {
	if (getGameType() == GType_FF && _mouseCursor != 5) {
		resetVerbs();
		_noRightClick = 0;
	}
	if (getGameType() == GType_SIMON2 && getBitFlag(79)) {
		_mouseCursor = 0;
	}
	_mouseHideCount = 0;
}

void AGOSEngine::scriptMouseOff() {
	_lockWord |= 0x8000;
	vc34_setMouseOff();
	_lockWord &= ~0x8000;
}

void AGOSEngine::waitForMark(uint i) {
	_exitCutscene = false;
	while (!(_marks & (1 << i))) {
		if (_exitCutscene) {
			if (getBitFlag(9)) {
				endCutscene();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	}
}

void AGOSEngine::freezeBottom() {
	_vgaMemBase = _vgaMemPtr;
	_vgaFrozenBase = _vgaMemPtr;
}

void AGOSEngine::unfreezeBottom() {
	_vgaMemPtr = _vgaRealBase;
	_vgaMemBase = _vgaRealBase;
	_vgaFrozenBase = _vgaRealBase;
}

void AGOSEngine::sendSync(uint a) {
	uint16 id = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&id;
	vc15_sync();
	_lockWord &= ~0x8000;
}

void AGOSEngine::setTextColor(uint color) {
	WindowBlock *window;

	window = _windowArray[_curWindow];
	window->text_color = color;
}

void AGOSEngine::stopAnimateSimon1(uint a) {
	uint16 b = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&b;
	vc60_killSprite();
	_lockWord &= ~0x8000;
}

void AGOSEngine::stopAnimateSimon2(uint a, uint b) {
	uint16 items[2];

	items[0] = to16Wrapper(a);
	items[1] = to16Wrapper(b);

	_lockWord |= 0x8000;
	_vcPtr = (byte *)&items;
	vc60_killSprite();
	_lockWord &= ~0x8000;
}

} // End of namespace AGOS
