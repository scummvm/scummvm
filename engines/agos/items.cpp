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

void AGOSEngine::setupCommonOpcodes(OpcodeProc *op) {
	// A sensible set of opcodes for Simon 1 and later.

	op[1] = &AGOSEngine::o_at;
	op[2] = &AGOSEngine::o_notAt;
	op[5] = &AGOSEngine::o_carried;
	op[6] = &AGOSEngine::o_notCarried;
	op[7] = &AGOSEngine::o_isAt;
	op[11] = &AGOSEngine::o_zero;
	op[12] = &AGOSEngine::o_notZero;
	op[13] = &AGOSEngine::o_eq;
	op[14] = &AGOSEngine::o_notEq;
	op[15] = &AGOSEngine::o_gt;
	op[16] = &AGOSEngine::o_lt;
	op[17] = &AGOSEngine::o_eqf;
	op[18] = &AGOSEngine::o_notEqf;
	op[19] = &AGOSEngine::o_ltf;
	op[20] = &AGOSEngine::o_gtf;
	op[23] = &AGOSEngine::o_chance;
	op[25] = &AGOSEngine::o_isRoom;
	op[26] = &AGOSEngine::o_isObject;
	op[27] = &AGOSEngine::o_state;
	op[28] = &AGOSEngine::o_oflag;
	op[31] = &AGOSEngine::o_destroy;
	op[33] = &AGOSEngine::o_place;
	op[36] = &AGOSEngine::o_copyff;
	op[41] = &AGOSEngine::o_clear;
	op[42] = &AGOSEngine::o_let;
	op[43] = &AGOSEngine::o_add;
	op[44] = &AGOSEngine::o_sub;
	op[45] = &AGOSEngine::o_addf;
	op[46] = &AGOSEngine::o_subf;
	op[47] = &AGOSEngine::o_mul;
	op[48] = &AGOSEngine::o_div;
	op[49] = &AGOSEngine::o_mulf;
	op[50] = &AGOSEngine::o_divf;
	op[51] = &AGOSEngine::o_mod;
	op[52] = &AGOSEngine::o_modf;
	op[53] = &AGOSEngine::o_random;
	op[55] = &AGOSEngine::o_goto;
	op[56] = &AGOSEngine::o_oset;
	op[57] = &AGOSEngine::o_oclear;
	op[58] = &AGOSEngine::o_putBy;
	op[59] = &AGOSEngine::o_inc;
	op[60] = &AGOSEngine::o_dec;
	op[61] = &AGOSEngine::o_setState;
	op[62] = &AGOSEngine::o_print;
	op[63] = &AGOSEngine::o_message;
	op[64] = &AGOSEngine::o_msg;
	op[65] = &AGOSEngine::o_addTextBox;
	op[66] = &AGOSEngine::o_setShortText;
	op[67] = &AGOSEngine::o_setLongText;
	op[68] = &AGOSEngine::o_end;
	op[69] = &AGOSEngine::o_done;
	op[71] = &AGOSEngine::o_process;
	op[76] = &AGOSEngine::o_when;
	op[77] = &AGOSEngine::o_if1;
	op[78] = &AGOSEngine::o_if2;
	op[79] = &AGOSEngine::o_isCalled;
	op[80] = &AGOSEngine::o_is;
	op[82] = &AGOSEngine::o_debug;
	op[87] = &AGOSEngine::o_comment;
	op[88] = &AGOSEngine::o_haltAnimation;
	op[89] = &AGOSEngine::o_restartAnimation;
	op[90] = &AGOSEngine::o_getParent;
	op[91] = &AGOSEngine::o_getNext;
	op[92] = &AGOSEngine::o_getChildren;
	op[96] = &AGOSEngine::o_picture;
	op[97] = &AGOSEngine::o_loadZone;
	op[100] = &AGOSEngine::o_killAnimate;
	op[101] = &AGOSEngine::o_defWindow;
	op[102] = &AGOSEngine::o_window;
	op[103] = &AGOSEngine::o_cls;
	op[104] = &AGOSEngine::o_closeWindow;
	op[107] = &AGOSEngine::o_addBox;
	op[108] = &AGOSEngine::o_delBox;
	op[109] = &AGOSEngine::o_enableBox;
	op[110] = &AGOSEngine::o_disableBox;
	op[111] = &AGOSEngine::o_moveBox;
	op[114] = &AGOSEngine::o_doIcons;
	op[115] = &AGOSEngine::o_isClass;
	op[116] = &AGOSEngine::o_setClass;
	op[117] = &AGOSEngine::o_unsetClass;
	op[119] = &AGOSEngine::o_waitSync;
	op[120] = &AGOSEngine::o_sync;
	op[121] = &AGOSEngine::o_defObj;
	op[125] = &AGOSEngine::o_here;
	op[126] = &AGOSEngine::o_doClassIcons;
	op[128] = &AGOSEngine::o_waitEndTune;
	op[129] = &AGOSEngine::o_ifEndTune;
	op[130] = &AGOSEngine::o_setAdjNoun;
	op[132] = &AGOSEngine::o_saveUserGame;
	op[133] = &AGOSEngine::o_loadUserGame;
	op[134] = &AGOSEngine::o_stopTune;
	op[135] = &AGOSEngine::o_pauseGame;
	op[136] = &AGOSEngine::o_copysf;
	op[137] = &AGOSEngine::o_restoreIcons;
	op[138] = &AGOSEngine::o_freezeZones;
	op[139] = &AGOSEngine::o_placeNoIcons;
	op[140] = &AGOSEngine::o_clearTimers;
	op[141] = &AGOSEngine::o_setDollar;
	op[142] = &AGOSEngine::o_isBox;
	op[143] = &AGOSEngine::o_doTable;
	op[151] = &AGOSEngine::o_storeItem;
	op[152] = &AGOSEngine::o_getItem;
	op[153] = &AGOSEngine::o_bSet;
	op[154] = &AGOSEngine::o_bClear;
	op[155] = &AGOSEngine::o_bZero;
	op[156] = &AGOSEngine::o_bNotZero;
	op[157] = &AGOSEngine::o_getOValue;
	op[158] = &AGOSEngine::o_setOValue;
	op[160] = &AGOSEngine::o_ink;
	op[161] = &AGOSEngine::o_screenTextBox;
	op[162] = &AGOSEngine::o_screenTextMsg;
	op[163] = &AGOSEngine::o_playEffect;
	op[164] = &AGOSEngine::o_getDollar2;
	op[165] = &AGOSEngine::o_isAdjNoun;
	op[166] = &AGOSEngine::o_b2Set;
	op[167] = &AGOSEngine::o_b2Clear;
	op[168] = &AGOSEngine::o_b2Zero;
	op[169] = &AGOSEngine::o_b2NotZero;
	op[175] = &AGOSEngine::o_lockZones;
	op[176] = &AGOSEngine::o_unlockZones;
	op[178] = &AGOSEngine::o_getPathPosn;
	op[179] = &AGOSEngine::o_scnTxtLongText;
	op[180] = &AGOSEngine::o_mouseOn;
	op[184] = &AGOSEngine::o_unloadZone;
	op[186] = &AGOSEngine::o_unfreezeZones;
}

void AGOSEngine::setupElvira1Opcodes(OpcodeProc *op) {
	op[0] = &AGOSEngine::o_at;
	op[1] = &AGOSEngine::o_notAt;
	op[2] = &AGOSEngine::oe1_present;
	op[3] = &AGOSEngine::oe1_notPresent;
	op[4] = &AGOSEngine::oe1_worn;
	op[5] = &AGOSEngine::oe1_notWorn;
	op[6] = &AGOSEngine::o_carried;
	op[7] = &AGOSEngine::o_notCarried;
	op[8] = &AGOSEngine::o_isAt;
	op[9] = &AGOSEngine::oe1_isNotAt;
	op[10] = &AGOSEngine::oe1_sibling;
	op[11] = &AGOSEngine::oe1_notSibling;
	op[12] = &AGOSEngine::o_zero;
	op[13] = &AGOSEngine::o_notZero;
	op[14] = &AGOSEngine::o_eq;
	op[15] = &AGOSEngine::o_notEq;
	op[16] = &AGOSEngine::o_gt;
	op[17] = &AGOSEngine::o_lt;
	op[18] = &AGOSEngine::o_eqf;
	op[19] = &AGOSEngine::o_notEqf;
	op[20] = &AGOSEngine::o_ltf;
	op[21] = &AGOSEngine::o_gtf;

	op[32] = &AGOSEngine::o_isRoom;
	op[33] = &AGOSEngine::o_isObject;

	op[34] = &AGOSEngine::o_state;
	op[36] = &AGOSEngine::o_oflag;

	op[48] = &AGOSEngine::o_destroy;

	op[51] = &AGOSEngine::o_place;

	op[54] = &AGOSEngine::o_copyof;
	op[55] = &AGOSEngine::o_copyfo;
	op[56] = &AGOSEngine::o_copyff;

	op[60] = &AGOSEngine::oe1_setFF;
	op[61] = &AGOSEngine::o_clear;

	op[64] = &AGOSEngine::o_let;
	op[65] = &AGOSEngine::o_add;
	op[66] = &AGOSEngine::o_sub;
	op[67] = &AGOSEngine::o_addf;
	op[68] = &AGOSEngine::o_subf;
	op[69] = &AGOSEngine::o_mul;
	op[70] = &AGOSEngine::o_div;
	op[71] = &AGOSEngine::o_mulf;
	op[72] = &AGOSEngine::o_divf;
	op[73] = &AGOSEngine::o_mod;
	op[74] = &AGOSEngine::o_modf;
	op[75] = &AGOSEngine::o_random;

	op[77] = &AGOSEngine::o_goto;

	op[80] = &AGOSEngine::o_oset;
	op[81] = &AGOSEngine::o_oclear;

	op[84] = &AGOSEngine::o_putBy;
	op[85] = &AGOSEngine::o_inc;
	op[86] = &AGOSEngine::o_dec;
	op[87] = &AGOSEngine::o_setState;

	op[91] = &AGOSEngine::o_message;

	op[97] = &AGOSEngine::o_end;
	op[98] = &AGOSEngine::o_done;

	op[105] = &AGOSEngine::o_process;

	op[119] = &AGOSEngine::o_when;

	op[128] = &AGOSEngine::o_if1;
	op[129] = &AGOSEngine::o_if2;

	op[135] = &AGOSEngine::o_isCalled;
	op[136] = &AGOSEngine::o_is;

	op[152] = &AGOSEngine::o_debug;

	op[176] = &AGOSEngine::oe1_opcode176;

	op[178] = &AGOSEngine::oe1_opcode178;

	op[180] = &AGOSEngine::oww_whereTo;

	op[198] = &AGOSEngine::o_comment;

	op[206] = &AGOSEngine::o_getParent;
	op[207] = &AGOSEngine::o_getNext;
	op[208] = &AGOSEngine::o_getChildren;

	op[224] = &AGOSEngine::o_picture;
	op[225] = &AGOSEngine::o_loadZone;
	op[226] = &AGOSEngine::o1_animate;
	op[227] = &AGOSEngine::o1_stopAnimate;
	op[228] = &AGOSEngine::o_killAnimate;
	op[229] = &AGOSEngine::o_defWindow;
	op[230] = &AGOSEngine::o_window;
	op[231] = &AGOSEngine::o_cls;
	op[232] = &AGOSEngine::o_closeWindow;

	op[235] = &AGOSEngine::o_addBox;
	op[236] = &AGOSEngine::o_delBox;
	op[237] = &AGOSEngine::o_enableBox;
	op[238] = &AGOSEngine::o_disableBox;
	op[239] = &AGOSEngine::o_moveBox;

	op[242] = &AGOSEngine::o_doIcons;
	op[243] = &AGOSEngine::o_isClass;
	op[249] = &AGOSEngine::o_setClass;
	op[250] = &AGOSEngine::o_unsetClass;

	op[255] = &AGOSEngine::o_waitSync;
	op[256] = &AGOSEngine::o_sync;
	op[257] = &AGOSEngine::o_defObj;

	op[261] = &AGOSEngine::o_here;
	op[262] = &AGOSEngine::o_doClassIcons;
	op[264] = &AGOSEngine::o_waitEndTune;
	op[263] = &AGOSEngine::o1_playTune;

	op[265] = &AGOSEngine::o_ifEndTune;
	op[266] = &AGOSEngine::o_setAdjNoun;
	op[267] = &AGOSEngine::oe1_zoneDisk;
	op[268] = &AGOSEngine::o_saveUserGame;
	op[269] = &AGOSEngine::o_loadUserGame;
	op[270] = &AGOSEngine::oe1_printStats;
	op[271] = &AGOSEngine::o_stopTune;

	op[274] = &AGOSEngine::o_pauseGame;
	op[275] = &AGOSEngine::o_copysf;
	op[276] = &AGOSEngine::o_restoreIcons;

	op[279] = &AGOSEngine::o_freezeZones;
	op[280] = &AGOSEngine::o_placeNoIcons;
	op[281] = &AGOSEngine::o_clearTimers;

	op[283] = &AGOSEngine::o_isBox;
}

void AGOSEngine::setupElvira2Opcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[34] = &AGOSEngine::o_copyof;
	op[35] = &AGOSEngine::o_copyfo;
	op[83] = &AGOSEngine::o1_rescan;
	op[98] = &AGOSEngine::o1_animate;
	op[99] = &AGOSEngine::o1_stopAnimate;
	op[127] = &AGOSEngine::o1_playTune;
	op[148] = &AGOSEngine::oww_ifDoorOpen;
	op[180] = &AGOSEngine::o_b2Set;
	op[181] = &AGOSEngine::o_b2Clear;
	op[182] = &AGOSEngine::o_b2Zero;
	op[183] = &AGOSEngine::o_b2NotZero;
}

void AGOSEngine::setupWaxworksOpcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	// Confirmed
	op[34] = &AGOSEngine::o_copyof;
	op[35] = &AGOSEngine::o_copyfo;
	op[54] = &AGOSEngine::oww_moveDirn;
	op[55] = &AGOSEngine::oww_goto;
	op[70] = &AGOSEngine::o1_printLongText;
	op[83] = &AGOSEngine::o1_rescan;
	op[98] = &AGOSEngine::o1_animate;
	op[99] = &AGOSEngine::o1_stopAnimate;
	op[85] = &AGOSEngine::oww_whereTo;
	op[105] = &AGOSEngine::oww_menu;
	op[106] = &AGOSEngine::oww_textMenu;
	op[127] = &AGOSEngine::o1_playTune;
	op[148] = &AGOSEngine::oww_ifDoorOpen;
	op[175] = &AGOSEngine::o_getDollar2;
	op[179] = &AGOSEngine::o_isAdjNoun;
	op[180] = &AGOSEngine::o_b2Set;
	op[181] = &AGOSEngine::o_b2Clear;
	op[182] = &AGOSEngine::o_b2Zero;
	op[183] = &AGOSEngine::o_b2NotZero;
	op[184] = &AGOSEngine::oww_opcode184;
	op[187] = &AGOSEngine::oww_opcode187;

	// Code difference, check if triggered
	op[161] = NULL;
	op[162] = NULL;
	op[163] = NULL;
	op[164] = NULL;
	op[165] = NULL;
	op[166] = NULL;
	op[167] = NULL;
	op[168] = NULL;
	op[169] = NULL;
	op[170] = NULL;
	op[171] = NULL;
	op[172] = NULL;
	op[173] = NULL;
	op[174] = NULL;
	op[176] = NULL;
	op[177] = NULL;
	op[178] = NULL;
	op[185] = NULL;
	op[186] = NULL;
	op[188] = NULL;
	op[189] = NULL;
	op[190] = NULL;
}

void AGOSEngine::setupSimon1Opcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[70] = &AGOSEngine::o1_printLongText;
	op[83] = &AGOSEngine::o1_rescan;
	op[98] = &AGOSEngine::o1_animate;
	op[99] = &AGOSEngine::o1_stopAnimate;
	op[127] = &AGOSEngine::o1_playTune;
	op[177] = &AGOSEngine::o1_screenTextPObj;
	op[181] = &AGOSEngine::o1_mouseOff;
	op[182] = &AGOSEngine::o1_loadBeard;
	op[183] = &AGOSEngine::o1_unloadBeard;
	op[185] = &AGOSEngine::o1_loadStrings;
	op[187] = &AGOSEngine::o1_specialFade;
}

void AGOSEngine::setupSimon2Opcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[70] = &AGOSEngine::o2_printLongText;
	op[83] = &AGOSEngine::o2_rescan;
	op[98] = &AGOSEngine::o2_animate;
	op[99] = &AGOSEngine::o2_stopAnimate;
	op[127] = &AGOSEngine::o2_playTune;
	op[177] = &AGOSEngine::o2_screenTextPObj;
	op[181] = &AGOSEngine::o2_mouseOff;
	op[188] = &AGOSEngine::o2_isShortText;
	op[189] = &AGOSEngine::o2_clearMarks;
	op[190] = &AGOSEngine::o2_waitMark;
}

void AGOSEngine::setupFeebleOpcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[23] = &AGOSEngine::o3_chance;
	op[37] = &AGOSEngine::o3_jumpOut;
	op[65] = &AGOSEngine::o3_addTextBox;
	op[70] = &AGOSEngine::o3_printLongText;
	op[83] = &AGOSEngine::o2_rescan;
	op[98] = &AGOSEngine::o2_animate;
	op[99] = &AGOSEngine::o2_stopAnimate;
	op[107] = &AGOSEngine::o3_addBox;
	op[122] = &AGOSEngine::o3_oracleTextDown;
	op[123] = &AGOSEngine::o3_oracleTextUp;
	op[124] = &AGOSEngine::o3_ifTime;
	op[127] = NULL;
	op[131] = &AGOSEngine::o3_setTime;
	op[132] = &AGOSEngine::o3_saveUserGame;
	op[133] = &AGOSEngine::o3_loadUserGame;
	op[134] = &AGOSEngine::o3_listSaveGames;
	op[135] = &AGOSEngine::o3_checkCD;
	op[161] = &AGOSEngine::o3_screenTextBox;
	op[163] = NULL;
	op[165] = &AGOSEngine::o3_isAdjNoun;
	op[171] = &AGOSEngine::o3_hyperLinkOn;
	op[172] = &AGOSEngine::o3_hyperLinkOff;
	op[173] = &AGOSEngine::o3_checkPaths;
	op[177] = &AGOSEngine::o3_screenTextPObj;
	op[181] = &AGOSEngine::o3_mouseOff;
	op[182] = &AGOSEngine::o3_loadVideo;
	op[183] = &AGOSEngine::o3_playVideo;
	op[187] = &AGOSEngine::o3_centreScroll;
	op[188] = &AGOSEngine::o2_isShortText;
	op[189] = &AGOSEngine::o2_clearMarks;
	op[190] = &AGOSEngine::o2_waitMark;
	op[191] = &AGOSEngine::o3_resetPVCount;
	op[192] = &AGOSEngine::o3_setPathValues;
	op[193] = &AGOSEngine::o3_stopClock;
	op[194] = &AGOSEngine::o3_restartClock;
	op[195] = &AGOSEngine::o3_setColour;
	op[196] = &AGOSEngine::o3_b3Set;
	op[197] = &AGOSEngine::o3_b3Clear;
	op[198] = &AGOSEngine::o3_b3Zero;
	op[199] = &AGOSEngine::o3_b3NotZero;
}

void AGOSEngine::setupPuzzleOpcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[23] = &AGOSEngine::o3_chance;
	op[30] = &AGOSEngine::o4_iconifyWindow;
	op[32] = &AGOSEngine::o4_restoreOopsPosition;
	op[38] = &AGOSEngine::o4_loadMouseImage;
	op[63] = &AGOSEngine::o4_message;
	op[65] = &AGOSEngine::o3_addTextBox;
	op[66] = &AGOSEngine::o4_setShortText;
	op[70] = &AGOSEngine::o3_printLongText;
	op[83] = &AGOSEngine::o2_rescan;
	op[98] = &AGOSEngine::o2_animate;
	op[99] = &AGOSEngine::o2_stopAnimate;
	op[105] = &AGOSEngine::o4_loadHiScores;
	op[106] = &AGOSEngine::o4_checkHiScores;
	op[107] = &AGOSEngine::o3_addBox;
	op[120] = &AGOSEngine::o4_sync;
	op[122] = &AGOSEngine::o3_oracleTextDown;
	op[123] = &AGOSEngine::o3_oracleTextUp;
	op[124] = &AGOSEngine::o3_ifTime;
	op[127] = NULL;
	op[131] = &AGOSEngine::o3_setTime;
	op[132] = &AGOSEngine::o4_saveUserGame;
	op[133] = &AGOSEngine::o4_loadUserGame;
	op[134] = &AGOSEngine::o3_listSaveGames;
	op[161] = &AGOSEngine::o3_screenTextBox;
	op[163] = NULL;
	op[165] = &AGOSEngine::o3_isAdjNoun;
	op[166] = NULL;
	op[167] = NULL;
	op[168] = NULL;
	op[169] = NULL;
	op[171] = &AGOSEngine::o3_hyperLinkOn;
	op[172] = &AGOSEngine::o3_hyperLinkOff;
	op[173] = &AGOSEngine::o4_saveOopsPosition;
	op[177] = &AGOSEngine::o3_screenTextPObj;
	op[181] = &AGOSEngine::o3_mouseOff;
	op[187] = &AGOSEngine::o4_resetGameTime;
	op[188] = &AGOSEngine::o2_isShortText;
	op[189] = &AGOSEngine::o2_clearMarks;
	op[190] = &AGOSEngine::o2_waitMark;
	op[191] = &AGOSEngine::o4_resetPVCount;
	op[192] = &AGOSEngine::o4_setPathValues;
	op[193] = &AGOSEngine::o3_stopClock;
	op[194] = &AGOSEngine::o4_restartClock;
	op[195] = &AGOSEngine::o3_setColour;
}

void AGOSEngine::setupOpcodes() {
	static OpcodeProc opcode_table[300];

	for (int i = 0; i < ARRAYSIZE(opcode_table); i++)
		opcode_table[i] = NULL;

	_opcode_table = opcode_table;
	_numOpcodes = ARRAYSIZE(opcode_table);

	switch (getGameType()) {
	case GType_ELVIRA:
		setupElvira1Opcodes(opcode_table);
		break;
	case GType_ELVIRA2:
		setupElvira2Opcodes(opcode_table);
		break;
	case GType_WW:
		setupWaxworksOpcodes(opcode_table);
		break;
	case GType_SIMON1:
		setupSimon1Opcodes(opcode_table);
		break;
	case GType_SIMON2:
		setupSimon2Opcodes(opcode_table);
		break;
	case GType_FF:
		setupFeebleOpcodes(opcode_table);
		break;
	case GType_PP:
		setupPuzzleOpcodes(opcode_table);
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

void AGOSEngine::o_copyof() {
	// 34:
	Item *item = getNextItemPtr();
	uint tmp = getVarOrByte();
	writeNextVarContents(getUserFlag(item, tmp));
}

void AGOSEngine::o_copyfo() {
	// 35:
	uint tmp = getNextVarContents();
	Item *item = getNextItemPtr();
	setUserFlag(item, getVarOrByte(), tmp);
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

	_picture8600 = (vga_res == 8600);

	if (mode == 4) {
		vc29_stopAllSounds();

		if (!_initMouse) {
			_initMouse = 1;
			vc33_setMouseOn();
		}
	}

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
		if (getGameType() == GType_FF || getGameType() == GType_PP)
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
	uint maxPath = (getGameType() == GType_FF || getGameType() == GType_PP) ? 100 : 20;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		x += _scrollX;
		y += _scrollY;
	} else if (getGameType() == GType_SIMON2) {
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

	if (getGameType() == GType_FF || getGameType() == GType_PP)
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

void AGOSEngine::oe1_present() {
	// 2: present (here or carried)
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent == getItem1ID() || item->parent == me()->parent);
}

void AGOSEngine::oe1_notPresent() {
	// 3: not present (neither here nor carried)
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent != getItem1ID() && item->parent != me()->parent);
}

void AGOSEngine::oe1_worn() {
	// 4: worn
	getNextItemPtr();
}

void AGOSEngine::oe1_notWorn() {
	// 5: not worn
	getNextItemPtr();
}

void AGOSEngine::oe1_isNotAt() {
	// 9: parent is not
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent != getNextItemID());
}

void AGOSEngine::oe1_sibling() {
	// 10: sibling
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(item1->parent == item2->parent);
}

void AGOSEngine::oe1_notSibling() {
	// 11: not sibling
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(item1->parent != item2->parent);
}

void AGOSEngine::oe1_setFF() {
	// 60
	writeNextVarContents(0xFF);
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

void AGOSEngine::oe1_zoneDisk() {
	// 267: zone disk 
	getVarOrWord();
	getVarOrWord();
}

void AGOSEngine::oe1_printStats() {
	// 270: print stats
}


// -----------------------------------------------------------------------
// Waxworks Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::oww_moveDirn() {
	// 54: move direction
	int16 d = getVarOrByte();
	moveDirn(me(), d);
}

void AGOSEngine::oww_goto() {
	// 55: set itemA parent
	uint item = getNextItemID();
	if (_itemArrayPtr[item] == NULL) {
		setItemParent(me(), NULL);
		loadRoomItems(item);
	}
	setItemParent(me(), _itemArrayPtr[item]);
}

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

void AGOSEngine::oww_opcode184() {
	printf("%s\n", getStringPtrByID(getNextStringID()));
}

void AGOSEngine::oww_opcode187() {
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

	if (!strcmp(_gameDescription->desc.extra, "4CD")) {
		_sound->switchVoiceFile(gss, disc);
	} else if (!strcmp(_gameDescription->desc.extra, "2CD")) {
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

void AGOSEngine::o4_iconifyWindow() {
	// 30
	getNextItemPtr();
	if (_clockStopped != 0)
		_gameTime += time(NULL) - _clockStopped;
	_clockStopped  = 0;
	_system->setFeatureState(OSystem::kFeatureIconifyWindow, true);
}

void AGOSEngine::o4_restoreOopsPosition() {
	// 32: restore oops position
	uint i;

	getNextItemPtr();

	if (_oopsValid) {
		for (i = 0; i < _numVars; i++) {
			_variableArray[i] = _variableArray2[i];
		}
		i = _variableArray[999] * 100 + 11;
		set_video_mode_internal(4,i);
		if (getBitFlag(110)) {
			_gameTime += 10;
		} else {
			// Swampy adventures
			_gameTime += 30;
		}
		_oopsValid = false;
	}
}

void AGOSEngine::o4_loadMouseImage() {
	// 38: load mouse image
	getNextItemPtr();
	getVarOrByte();
	loadMouseImage();
}

void AGOSEngine::o4_message() {
	// 63: show string nl

	if (getBitFlag(105)) {
		// Swampy adventures
		getStringPtrByID(getNextStringID());
//		printInfoText(getStringPtrByID(getNextStringID()));
	} else {
		showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
	}
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

void AGOSEngine::o4_loadHiScores() {
	// 105: load high scores
	getVarOrByte();
}

void AGOSEngine::o4_checkHiScores() {
	// 106: check high scores
	getVarOrByte();
	getVarOrByte();
}

void AGOSEngine::o4_sync() {
	// 120: sync
	uint a = getVarOrWord();
	if (a == 8001 || a == 8101 || a == 8201 || a == 8301 || a == 8401) {
		_marks &= ~(1 << 2);
	}
	sendSync(a);
}

void AGOSEngine::o4_saveUserGame() {
	// 132: save game
	if (_clockStopped != 0)
		_gameTime += time(NULL) - _clockStopped;
	_clockStopped = 0;

	if (getGameId() == GID_DIMP) {
		saveGame(1, NULL);
	} else if (!getBitFlag(110)) {
		// Swampy adventures
		saveGame(1, NULL);
	}

	//saveHiScores()
}

void AGOSEngine::o4_loadUserGame() {
	// 133: load usergame

	// NoPatience or Jumble
	if (getBitFlag(110)) {
		//getHiScoreName();
		return;
	}

	// XXX
	loadGame(1);
}

void AGOSEngine::o4_saveOopsPosition() {
	// 173: save oops position
	if (!isVgaQueueEmpty()) {
		_oopsValid = true;
		for (uint i = 0; i < _numVars; i++) {
			_variableArray2[i] = _variableArray[i];
		}
	} else {
		_oopsValid = false;
	}
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

		if (getGameType() == GType_ELVIRA) {
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
		if (getGameType() == GType_ELVIRA) {
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
				debug(1, "runScript: opcode %d", opcode);

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
	if ((getGameType() == GType_FF || getGameType() == GType_PP) && _mouseCursor != 5) {
		resetVerbs();
		_noRightClick = 0;
	} else if (getGameType() == GType_SIMON2 && getBitFlag(79)) {
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
			if (getGameType() == GType_PP) {
				if (_picture8600)
					break;
			} else {
				if (getBitFlag(9)) {
					endCutscene();
					break;
				}
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
