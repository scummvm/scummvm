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

#include "common/system.h"

#include "agos/agos.h"

namespace AGOS {

void AGOSEngine_PuzzlePack::setupOpcodes(OpcodeProc *op) {
	AGOSEngine::setupOpcodes(op);

	op[23] = &AGOSEngine::off_chance;
	op[30] = &AGOSEngine::opp_iconifyWindow;
	op[32] = &AGOSEngine::opp_restoreOopsPosition;
	op[38] = &AGOSEngine::opp_loadMouseImage;
	op[63] = &AGOSEngine::opp_message;
	op[65] = &AGOSEngine::off_addTextBox;
	op[66] = &AGOSEngine::opp_setShortText;
	op[67] = &AGOSEngine::oww_setLongText;
	op[70] = &AGOSEngine::off_printLongText;
	op[83] = &AGOSEngine::os2_rescan;
	op[98] = &AGOSEngine::os2_animate;
	op[99] = &AGOSEngine::os2_stopAnimate;
	op[105] = &AGOSEngine::opp_loadHiScores;
	op[106] = &AGOSEngine::opp_checkHiScores;
	op[107] = &AGOSEngine::off_addBox;
	op[120] = &AGOSEngine::opp_sync;
	op[122] = &AGOSEngine::off_oracleTextDown;
	op[123] = &AGOSEngine::off_oracleTextUp;
	op[124] = &AGOSEngine::off_ifTime;
	op[131] = &AGOSEngine::off_setTime;
	op[132] = &AGOSEngine::opp_saveUserGame;
	op[133] = &AGOSEngine::opp_loadUserGame;
	op[134] = &AGOSEngine::off_listSaveGames;
	op[161] = &AGOSEngine::off_screenTextBox;
	op[162] = &AGOSEngine::os1_screenTextMsg;
	op[164] = &AGOSEngine::oe2_getDollar2;
	op[165] = &AGOSEngine::off_isAdjNoun;
	op[171] = &AGOSEngine::off_hyperLinkOn;
	op[172] = &AGOSEngine::off_hyperLinkOff;
	op[173] = &AGOSEngine::opp_saveOopsPosition;
	op[175] = &AGOSEngine::oww_lockZones;
	op[176] = &AGOSEngine::oww_unlockZones;
	op[177] = &AGOSEngine::off_screenTextPObj;
	op[178] = &AGOSEngine::os1_getPathPosn;
	op[179] = &AGOSEngine::os1_scnTxtLongText;
	op[180] = &AGOSEngine::os1_mouseOn;
	op[181] = &AGOSEngine::off_mouseOff;
	op[184] = &AGOSEngine::os1_unloadZone;
	op[186] = &AGOSEngine::os1_unfreezeZones;
	op[187] = &AGOSEngine::opp_resetGameTime;
	op[188] = &AGOSEngine::os2_isShortText;
	op[189] = &AGOSEngine::os2_clearMarks;
	op[190] = &AGOSEngine::os2_waitMark;
	op[191] = &AGOSEngine::opp_resetPVCount;
	op[192] = &AGOSEngine::opp_setPathValues;
	op[193] = &AGOSEngine::off_stopClock;
	op[194] = &AGOSEngine::opp_restartClock;
	op[195] = &AGOSEngine::off_setColour;
}

// -----------------------------------------------------------------------
// Puzzle Pack Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::opp_iconifyWindow() {
	// 30
	getNextItemPtr();
	if (_clockStopped != 0)
		_gameTime += time(NULL) - _clockStopped;
	_clockStopped  = 0;
	_system->setFeatureState(OSystem::kFeatureIconifyWindow, true);
}

void AGOSEngine::opp_restoreOopsPosition() {
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

void AGOSEngine::opp_loadMouseImage() {
	// 38: load mouse image
	getNextItemPtr();
	getVarOrByte();
	loadMouseImage();
}

void AGOSEngine::opp_message() {
	// 63: show string nl

	if (getBitFlag(105)) {
		// Swampy adventures
		getStringPtrByID(getNextStringID());
//		printInfoText(getStringPtrByID(getNextStringID()));
	} else {
		showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
	}
}

void AGOSEngine::opp_setShortText() {
	// 66: set item name
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (var < _numTextBoxes) {
		_shortText[var] = stringId;
		_shortTextX[var] = getVarOrWord();
		_shortTextY[var] = getVarOrWord();
	}
}

void AGOSEngine::opp_loadHiScores() {
	// 105: load high scores
	getVarOrByte();
}

void AGOSEngine::opp_checkHiScores() {
	// 106: check high scores
	getVarOrByte();
	getVarOrByte();
}

void AGOSEngine::opp_sync() {
	// 120: sync
	uint a = getVarOrWord();
	if (a == 8001 || a == 8101 || a == 8201 || a == 8301 || a == 8401) {
		_marks &= ~(1 << 2);
	}
	sendSync(a);
}

void AGOSEngine::opp_saveUserGame() {
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

void AGOSEngine::opp_loadUserGame() {
	// 133: load usergame

	// NoPatience or Jumble
	if (getBitFlag(110)) {
		//getHiScoreName();
		return;
	}

	// XXX
	loadGame(genSaveName(1));
}

void AGOSEngine::opp_saveOopsPosition() {
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

void AGOSEngine::opp_resetGameTime() {
	// 187: reset game time
	_gameTime = 0;
}

void AGOSEngine::opp_resetPVCount() {
	// 191
	_PVCount = 0;
	_GPVCount = 0;
}

void AGOSEngine::opp_setPathValues() {
	// 192
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
	_pathValues[_PVCount++] = getVarOrByte();
}

void AGOSEngine::opp_restartClock() {
	// 194: resume clock
	if (_clockStopped != 0)
		_gameTime += time(NULL) - _clockStopped;
	_clockStopped = 0;
}

} // End of namespace AGOS
