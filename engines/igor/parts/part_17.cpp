/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "igor/igor.h"

namespace Igor {

static int VAR_CURRENT_TALKING_ACTOR;

void IgorEngine::part17ExecuteAction(int action) {
	debugC(9, kDebugGame, "part17ExecuteAction %d", action);
	switch (action) {
	case 101: // walk to door
		part17WalkIn();
		break;
	case 102: // look at door
		igorSay(225, 1, 648);
		break;
	case 103: // talk to philip and jimmy
		part17TalkToPhilipAndJimmy();
		break;
	case 104: // look at file
		igorSay(220, 3, 646);
		break;
	case 105:
		part17LeaveThroughPath();
		break;
	case 106: // exchange files
		part17SwapFolders();
		break;
	default:
		error("part17ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part17WalkIn(bool lateGame) {
	const IgorMoveTick tick = lateGame ? 0 : &IgorEngine::part17UpdateBackground;
	_roomObjectAreasTable[_screenLayer2[22034]].area = 8;
	_roomObjectAreasTable[_screenLayer2[23923]].area = 8;
	--_walkDataLastIndex;
	buildWalkPath(243, 77, 243, 74);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(tick);
	// Added this line compared to the 2009 version to correct the walk path origin.
	--_walkDataLastIndex;

	buildWalkPath(243, 74, 274, 68);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(tick);
	_currentPart = 302;
}

void IgorEngine::part17TalkToPhilipAndJimmy() {
	part17PhilipToJimmyAnimFrame(0);

	if (_gameState.counter[4] == 1) {
		cutsceneSayWithCallback(156, 32, 35, 38, 47, 201, 2, 633, &IgorEngine::part17UpdateJimmyDialogue);
	} else if (_gameState.counter[4] == 2) {
		cutsceneSayWithCallback(156, 32, 35, 38, 47, 206, 2, 636, &IgorEngine::part17UpdateJimmyDialogue);
	} else if (_gameState.counter[4] == 3) {
		cutsceneSayWithCallback(156, 32, 35, 38, 47, 210, 2, 638, &IgorEngine::part17UpdateJimmyDialogue);
	}

	VAR_CURRENT_TALKING_ACTOR = 0;
	if (_gameState.counter[4] == 1) {
		debug("part17TalkToPhilipAndJimmy: counter[4] == 1");
		cutsceneSayWithCallback(135, 33, 63, 63, 0, { { 203, 1, 634 }, { 204, 2, 635 } }, &IgorEngine::part17UpdatePhilipDialogue);
	} else if (_gameState.counter[4] == 2) {
		debug("part17TalkToPhilipAndJimmy: counter[4] == 2");
		cutsceneSayWithCallback(135, 33, 63, 63, 0, 208, 2, 637, &IgorEngine::part17UpdatePhilipDialogue);
	} else if (_gameState.counter[4] == 3) {
		debug("part17TalkToPhilipAndJimmy: counter[4] == 3");
		cutsceneSayWithCallback(135, 33, 63, 63, 0, { { 212, 2, 639 }, { 214, 1, 640 } }, &IgorEngine::part17UpdatePhilipDialogue);
	}

	part17PhilipToIgor(0);

	waitForTimer(255);
	const int randomDialogue = getRandomNumber(3);
	VAR_CURRENT_TALKING_ACTOR = 1;
	cutsceneSayWithCallback(135, 33, 63, 63, 0, 215 + randomDialogue, 1, 641 + randomDialogue, &IgorEngine::part17UpdatePhilipDialogue);

	part17HandlePhilipDialogue();
	part17ApplyObjectState(255);
	part17PhilipToJimmyAnimFrame(0);

	--_walkDataLastIndex;
	buildWalkPath(104, 87, 143, 123);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::part17UpdateBackground);
	igorSayAndWait(219, 1, 645);
	if (_gameState.counter[4] == 3) {
		_gameState.counter[4] = 1;
	} else {
		++_gameState.counter[4];
	}
}

void IgorEngine::part17LeaveThroughPath(bool lateGame) {
	const IgorMoveTick tick = lateGame ? 0 : &IgorEngine::part17UpdateBackground;
	bool escaped = false;
	_roomObjectAreasTable[5].area = 1;
	_roomObjectAreasTable[10].area = 2;
	_roomObjectAreasTable[11].area = 2;
	_roomObjectAreasTable[13].area = 2;
	_roomObjectAreasTable[16].area = 3;
	_roomObjectAreasTable[17].area = 4;
	_roomObjectAreasTable[26].area = 4;
	_roomObjectAreasTable[28].area = 4;
	--_walkDataLastIndex;
	buildWalkPath(45, 95, 41, 86);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = lateGame ? 0 : 1;
	_gameState.igorMoving = true;
	escaped |= waitForIgorMove(tick, true, escaped);
	part17ChangePathZIndex(143);
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[44836]].area = 4;
	buildWalkPath(41, 86, 36, 143);
	_roomObjectAreasTable[_screenLayer2[44836]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	if (!lateGame) {
		for (int i = _walkDataCurrentIndex; i <= _walkDataLastIndex; ++i) {
			_walkData[i].posNum = kFacingPositionBack;
		}
	}
	escaped |= waitForIgorMove(tick, true, escaped, lateGame ? kFacingPositionBack : -1);
	part17SetYScaleRamp();
	_walkData[0].setPos(65, 115, 1, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 14;
	_walkData[0].scaleWidth = 23;
	_walkData[0].scaleHeight = 23;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[36865]].area = 4;
	buildWalkPath(65, 115, 21, 17);
	_roomObjectAreasTable[_screenLayer2[36865]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	escaped |= waitForIgorMove(tick, true, escaped);
	_currentPart = 40;
}

void IgorEngine::part17SwapFolders() {
	// folder already exchanged
	if (_objectsState[kCollegeEntranceFoldersSwappedStateIndex] == 1) {
		igorSay(226, 1, 649);
		return;
	}
	playSound(24, 1);

	const int offset = 0x311F;

	for (int frame = 0; frame <= 8; ++frame) {
		drawAnimRect(offset, 0x1E6E + frame * 1120, 40, 28);
		waitForTimer(30);
	}

	drawAnimRect(offset, 0x1E6E, 40, 28);

	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	g_engine->_screen->updateScreen();


	_objectsState[kCollegeEntranceFoldersSwappedStateIndex] = 1;
	_objectsState[kFolderSwappedStateIndex] = 1;
	part17ApplyObjectState(255);
	partMainUpdateObjectState(4);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	--_walkDataLastIndex;
	debugC(9, kDebugWalk, "PART_17_ACTION_106 walk: _walkDataLastIndex %d _walkData[%d] %d,%d",
	       _walkDataLastIndex, _walkDataLastIndex, _walkData[_walkDataLastIndex].x, _walkData[_walkDataLastIndex].y);
	buildWalkPath(104, 87, 143, 123);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::part17UpdateBackground);
	// Creo q no ha notado nada
	ADD_DIALOGUE_TEXT(223, 2, 647);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::part17HandlePhilipDialogue() {
	loadDialogueData(DLG_OutsideCollege);
	_updateDialogue = &IgorEngine::part17UpdatePhilipDialogue;
	handleDialogue(135, 33, 63, 63, 0);
	_updateDialogue = 0;
}

void IgorEngine::part17UpdateJimmyDialogue(int action) {
	debug("part17UpdateJimmyDialogue action: %d", action);
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part17JimmyTalkingAnimFrame(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part17JimmyTalkingAnimFrame(getRandomNumber(6));
		break;
	}
}

void IgorEngine::part17UpdatePhilipDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		if (VAR_CURRENT_TALKING_ACTOR == 0) {
			part17PhilipToJimmyAnimFrame(0);
		} else {
			part17PhilipToIgor(0);
		}
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		if (VAR_CURRENT_TALKING_ACTOR == 0) {
			debug("part17PhilipToJimmyAnimFrame");
			part17PhilipToJimmyAnimFrame(getRandomNumber(6));
		} else {
			debug("part17PhilipToIgor");
			part17PhilipToIgor(getRandomNumber(5));
		}
		break;
	}
}

void IgorEngine::part17UpdateBackground() {
	if (!_gameState.dialogueTextRunning && compareGameTick(3, 32) && _gameState.unkF) {
		// Counter (unk10) values 0 through 15 animate one character while holding the other on its resting frame.
		// Counter (unk10) values 16 through 31 animate the other character.
		switch (_gameState.unk10 / 16) {
		case 0: // animate jimmy
			part17PhilipToJimmyAnimFrame(0);
			part17JimmyTalkingAnimFrame(getRandomNumber(6));
			break;
		case 1: // animate philip
			part17PhilipToJimmyAnimFrame(getRandomNumber(6));
			part17JimmyTalkingAnimFrame(0);
		}
		if (_gameState.unk10 == 31) {
			_gameState.unk10 = 0;
		} else {
			++_gameState.unk10;
		}
	}
}

void IgorEngine::part17ApplyObjectState(int num) {
	// Enable or disable phillip and jimmy animation
	if (num == 1 || num == 255) {
		if (_objectsState[kPhillipAndJimmyPresentStateIndex] == 0) {
			_gameState.unkF = false;
		} else {
			part17DisableArea(0);
			_gameState.unkF = true;
		}
	}

	if (num == 2 || num == 255) {
		if (_objectsState[kCollegeEntranceFoldersSwappedStateIndex] == 1) {
			_roomActionsTable[2698] = 0;
		}
	}
	if (num == 3 || num == 255) {
		if (_objectsState[kPhillipAndJimmyHaveLeftStateIndex] == 1) {
			_gameState.unkF = false;
			part17DisableArea(6);
		}
	}
}

void IgorEngine::part17WalkFromMap(bool lateGame) {
	const IgorMoveTick tick = lateGame ? 0 : &IgorEngine::part17UpdateBackground;
	bool escaped = false;
	part17SetYScaleRamp();
	WalkData *wd = &_walkData[0];
	wd->setPos(21, 17, 3, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 4;
	wd->scaleWidth = 6;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 6;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[36865]].area = 4;
	buildWalkPath(21, 17, 65, 115);
	_roomObjectAreasTable[_screenLayer2[36865]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	escaped |= waitForIgorMove(tick, true, escaped);
	for (int i = 17; i <= 135; ++i) {
		_walkYScaleRoom[i] = 50;
	}
	_walkData[0].setPos(36, 140, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[44836]].area = 4;
	buildWalkPath(36, 143, 41, 86);
	_roomObjectAreasTable[_screenLayer2[44836]].area = 0;
	if (!lateGame) {
		_walkData[_walkDataLastIndex].frameNum = 0;
	}
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	if (!lateGame) {
		for (int i = _walkDataCurrentIndex; i <= _walkDataLastIndex; ++i) {
			_walkData[i].posNum = kFacingPositionFront;
		}
	}
	escaped |= waitForIgorMove(tick, true, escaped, lateGame ? kFacingPositionFront : -1);
	part17ChangePathZIndex(0);

	--_walkDataLastIndex;
	buildWalkPath(41, 86, 45, 95);
	_walkDataCurrentIndex = 2;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	escaped |= waitForIgorMove(tick, true, escaped);
	part17ClearEntryAreas();
}

void IgorEngine::part17SetYScaleRamp() {
	for (int i = 17; i <= 85; ++i) {
		_walkYScaleRoom[i] = ((i - 13) / 4) + 5;
	}
	for (int i = 86; i <= 135; ++i) {
		_walkYScaleRoom[i] = 23;
	}
}

void IgorEngine::part17ClearEntryAreas() {
	_roomObjectAreasTable[5].area = 0;
	_roomObjectAreasTable[10].area = 0;
	_roomObjectAreasTable[11].area = 0;
	_roomObjectAreasTable[13].area = 0;
	_roomObjectAreasTable[16].area = 0;
	_roomObjectAreasTable[17].area = 0;
	_roomObjectAreasTable[26].area = 0;
	_roomObjectAreasTable[28].area = 0;
}

void IgorEngine::part17DisableArea(int lum) {
	_roomObjectAreasTable[18].area = lum;
	_roomObjectAreasTable[19].area = lum;
	_roomObjectAreasTable[20].area = lum;
}

void IgorEngine::part17PaintFirstFrameOfPhilipAndJimmy() {
	int offset = 11642;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer, 44, 44, 49);
	drawAnimRect(offset, 0, 44, 49);
}

void IgorEngine::part17ChangePathZIndex(int lum) {
	_roomObjectAreasTable[23].y1Lum = lum;
	_roomObjectAreasTable[26].y1Lum = lum;
	_roomObjectAreasTable[27].y1Lum = lum;
	_roomObjectAreasTable[32].y1Lum = lum;
}

void IgorEngine::part17WalkFromCollege() {
	part17ApplyObjectState(255);
	fadeIn(768);
	_walkData[0].setPos(274, 68, 4, 1);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkCurrentFrame = 1;
	_walkDataCurrentIndex = 0;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[22034]].area = 8;
	_roomObjectAreasTable[_screenLayer2[23923]].area = 8;
	buildWalkPath(274, 68, 243, 74);
	_roomObjectAreasTable[_screenLayer2[22034]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::part17UpdateBackground);
	--_walkDataLastIndex;
	buildWalkPath(243, 74, 205, 124);
	_roomObjectAreasTable[_screenLayer2[23923]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::part17UpdateBackground);
	part17ChangePathZIndex(0);
	part17ClearEntryAreas();
}

void IgorEngine::part17PhilipToJimmyAnimFrame(int num) {
	for (int i = 0; i <= 8; ++i) {
		for (int j = 0; j <= 10; ++j) {
			int offset = (36 + i) * 320 + 130 + j;
			uint8 color = _screenVGA[offset];
			if (color >= 0xF0 && color <= 0xF1) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[num * 99 + i * 11 + j + 0x1967];
			}
		}
	}
	int offset = 11650;
	copyArea(_screenVGA, offset, 320, _screenTempLayer, 100, 11, 9);
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + num * 99 + 0x1967, 11, 11, 9);
}

void IgorEngine::part17JimmyTalkingAnimFrame(int num) {
	drawAnimRect(11663, num * 621 + 0x86C, 23, 27, true, kBlendBehindIgorAndText);
}

void IgorEngine::part17PhilipToIgor(int frame) {
	for (int i = 0; i <= 8; ++i) {
		for (int j = 0; j <= 10; ++j) {
			int offset = (i + 36) * 320 + j + 130;
			uint8 color = _screenVGA[offset];
			if (color != 0 && color != 240) {
				color = _animFramesBuffer[0x1C1C + frame * 99 + i * 11 + j];
			}
			_screenTempLayer[100 * i + j] = color;
		}
	}
	for (int i = 0; i <= 8; ++i) {
		const int offset = i * 320 + 11650;
		memcpy(_screenVGA + offset, _screenTempLayer + i * 100, 11);
		memcpy(_screenLayer1 + offset, _animFramesBuffer + 0x1C1C + frame * 99 + i * 11, 11);
	}
}

void IgorEngine::part17() {
	playMusic(1);
	_gameState.enableLight = 1;
	loadRoomData(PAL_OutsideCollege, IMG_OutsideCollege, BOX_OutsideCollege, MSK_OutsideCollege, TXT_OutsideCollege);
	static const int anm[] = { FRM_OutsideCollege1, FRM_OutsideCollege2, FRM_OutsideCollege3, FRM_OutsideCollege4, FRM_OutsideCollege5, 0 };
	loadAnimData(anm);
	loadActionData(DAT_OutsideCollege);

	_roomDataOffsets = PART_17_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 286, 143);

	partMainSetExecuteAction(1, &IgorEngine::part17ExecuteAction);
	_updateRoomBackground = &IgorEngine::part17UpdateBackground;

	part17ApplyObjectState(255);

	memcpy(_screenVGA, _screenLayer1, 46080);


	if (_objectsState[kPhillipAndJimmyHaveLeftStateIndex] == 1) {
		_gameState.unkF = false;
		part17DisableArea(6);
		_roomObjectAreasTable[14].object = 0;
		_roomObjectAreasTable[15].object = 0;
		_roomObjectAreasTable[19].object = 0;
	} else if (_gameState.unkF) {
		part17PaintFirstFrameOfPhilipAndJimmy();
		_gameState.counter[4] = 1;
	} else {
		_roomObjectAreasTable[14].object = 0;
		_roomObjectAreasTable[15].object = 0;
		_roomObjectAreasTable[19].object = 0;
	}

	if (_gameStateLoaded) {
		// Restore the path depth before drawing Igor at his saved position.
		part17ChangePathZIndex(0);
	}
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 170) {
			// from map
			fadeIn(768);
			part17WalkFromMap();
		} else if (_currentPart == 171) {
			// from inside college
			part17WalkFromCollege();
		}
	}
	enterPartLoop();
	while (_currentPart >= 170 && _currentPart <= 171 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded) {
		if (_objectsState[kCollegeEntranceFoldersSwappedStateIndex] == 1) {
			_objectsState[kPhillipAndJimmyHaveLeftStateIndex] = 1;
		}
		if (_objectsState[kPhillipAndJimmyPresentStateIndex] == 0) {
			_objectsState[kPhillipAndJimmyPresentStateIndex] = 1;
		}
		fadeOut(624);
	}
}

} // namespace Igor
