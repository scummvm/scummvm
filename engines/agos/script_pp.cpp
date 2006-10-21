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

#include "common/stdafx.h"

#include "agos/agos.h"

namespace AGOS {

void AGOSEngine::setupPuzzleOpcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[23] = &AGOSEngine::o3_chance;
	op[30] = &AGOSEngine::o4_iconifyWindow;
	op[32] = &AGOSEngine::o4_restoreOopsPosition;
	op[38] = &AGOSEngine::o4_loadMouseImage;
	op[63] = &AGOSEngine::o4_message;
	op[65] = &AGOSEngine::o3_addTextBox;
	op[66] = &AGOSEngine::o4_setShortText;
	op[67] = &AGOSEngine::o_setLongText;
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
	op[131] = &AGOSEngine::o3_setTime;
	op[132] = &AGOSEngine::o4_saveUserGame;
	op[133] = &AGOSEngine::o4_loadUserGame;
	op[134] = &AGOSEngine::o3_listSaveGames;
	op[161] = &AGOSEngine::o3_screenTextBox;
	op[162] = &AGOSEngine::o_screenTextMsg;
	op[164] = &AGOSEngine::o_getDollar2;
	op[165] = &AGOSEngine::o3_isAdjNoun;
	op[171] = &AGOSEngine::o3_hyperLinkOn;
	op[172] = &AGOSEngine::o3_hyperLinkOff;
	op[173] = &AGOSEngine::o4_saveOopsPosition;
	op[175] = &AGOSEngine::o_lockZones;
	op[176] = &AGOSEngine::o_unlockZones;
	op[177] = &AGOSEngine::o3_screenTextPObj;
	op[178] = &AGOSEngine::o_getPathPosn;
	op[179] = &AGOSEngine::o_scnTxtLongText;
	op[180] = &AGOSEngine::o_mouseOn;
	op[181] = &AGOSEngine::o3_mouseOff;
	op[184] = &AGOSEngine::o_unloadZone;
	op[186] = &AGOSEngine::o_unfreezeZones;
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
		setWindowImage(4,i);
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

} // End of namespace AGOS
