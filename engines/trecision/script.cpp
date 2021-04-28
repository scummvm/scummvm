/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "trecision/actor.h"
#include "trecision/dialog.h"
#include "trecision/logic.h"
#include "trecision/graphics.h"
#include "trecision/nl/define.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/message.h"
#include "trecision/trecision.h"

#include "common/scummsys.h"
#include "graphics/scaler.h"
#include "nl/extern.h"


namespace Trecision {

void SScriptFrame::sendFrame() {
	doEvent(_class, _event, MP_DEFAULT, _u16Param1, _u16Param2, _u8Param, _u32Param);
}

void TrecisionEngine::endScript() {
	_curStack--;
	if (_curStack == 0) {
		_flagscriptactive = false;
		showCursor();
		redrawString();
	}
}

void TrecisionEngine::playScript(uint16 id) {
	_curStack++;
	_flagscriptactive = true;
	hideCursor();
	_curScriptFrame[_curStack] = _script[id]._firstFrame;

	SScriptFrame *curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
	// If the event is empty, terminate the script
	if (curFrame->isEmptyEvent()) {
		endScript();
		return;
	}

	bool loop = true;
	while (loop) {
		loop = false;
		curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
		SScriptFrame *nextFrame = &_scriptFrame[_curScriptFrame[_curStack] + 1];
		curFrame->sendFrame();
		if (curFrame->_noWait && !nextFrame->isEmptyEvent()) {
			_curScriptFrame[_curStack]++;
			loop = true;
		}
	}
}

void TrecisionEngine::evalScript() {
	if (_characterQueue.testEmptyCharacterQueue4Script() && _gameQueue.testEmptyQueue(MC_DIALOG) && _flagScreenRefreshed) {
		_curScriptFrame[_curStack]++;
		hideCursor();

		SScriptFrame *curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
		if (curFrame->isEmptyEvent()) {
			endScript();
			return;
		}

		bool loop = true;
		while (loop) {
			loop = false;
			curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
			SScriptFrame *nextFrame = &_scriptFrame[_curScriptFrame[_curStack] + 1];
			curFrame->sendFrame();
			if (curFrame->_noWait && !nextFrame->isEmptyEvent()) {
				_curScriptFrame[_curStack]++;
				loop = true;
			}
		}
	}
}

bool TrecisionEngine::quitGame() {
	for (int a = 0; a < TOP; a++)
		memcpy(_zBuffer + a * MAXX, _screenBuffer + MAXX * a, MAXX * 2);

	for (int a = 0; a < TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	SDText SText;
	SText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageConfirmExit]
	);
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	freeKey();

	checkSystem();

	char ch = waitKey();

	bool exitFl = ((ch == 'y') || (ch == 'Y'));

	for (int a = 0; a < TOP; a++)
		memcpy(_screenBuffer + MAXX * a, _zBuffer + a * MAXX, MAXX * 2);

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	return exitFl;
}

void TrecisionEngine::demoOver() {
	for (int a = 0; a < TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	SDText SText;
	SText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageDemoOver]
	);
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	freeKey();
	waitKey();
	quitGame();
}

void TrecisionEngine::doAction() {
	if ((_curMessage->_event == ME_MOUSEOPERATE) || (_curMessage->_event == ME_MOUSEEXAMINE)) {
		// Action in the game area
		_curObj = _curMessage->_u32Param;
		if (_curObj == oLASTLEV5)
			CharacterSay(2003);

		if (!_curObj || !(_obj[_curObj]._mode & OBJMODE_OBJSTATUS))
			return;

		if (_obj[_curObj]._mode & OBJMODE_HIDDEN)
			_obj[_curObj]._mode &= ~OBJMODE_HIDDEN;

		if (_flagUseWithStarted) {
			if ((_obj[_curObj]._flag & (kObjFlagRoomOut | kObjFlagRoomIn)) && !(_obj[_curObj]._flag & kObjFlagExamine))
				return;
			_flagUseWithStarted = false;
			_flagInventoryLocked = false;
			_useWith[WITH] = _curObj;
			_useWithInv[WITH] = false;
			_lightIcon = 0xFF;

			if (!_useWithInv[USED] && (_curObj == _useWith[USED])) {
				_useWith[USED] = 0;
				_useWith[WITH] = 0;
				_useWithInv[USED] = false;
				_useWithInv[WITH] = false;
				_flagUseWithStarted = false;
				clearText();
			} else
				doEvent(MC_ACTION, ME_USEWITH, MP_SYSTEM, 0, 0, 0, 0);
			_curObj = 0;
			return;
		}

		if ((_curMessage->_event == ME_MOUSEOPERATE) && (_obj[_curObj]._flag & kObjFlagUseWith)) {
			_flagUseWithStarted = true;
			_flagInventoryLocked = true;
			_useWith[USED] = _curObj;
			_useWith[WITH] = 0;
			_useWithInv[USED] = false;
			_useWithInv[WITH] = false;
			ShowObjName(_curObj, true);
			return;
		}
	}

	switch (_curMessage->_event) {
	case ME_MOUSEOPERATE:
		if (_obj[_curObj]._flag & kObjFlagRoomIn)
			doRoomIn(_curObj);
		else if (_obj[_curObj]._flag & kObjFlagPerson)
			doMouseTalk(_curObj);
		else if (_obj[_curObj]._flag & kObjFlagRoomOut)
			doRoomOut(_curObj);
		else if (_obj[_curObj]._flag & kObjFlagTake)
			doMouseTake(_curObj);
		else
			doMouseOperate(_curObj);
		break;

	case ME_MOUSEEXAMINE:
		if (_obj[_curObj]._flag & kObjFlagExamine)
			doMouseExamine(_curObj);
		else if (_obj[_curObj]._flag & kObjFlagRoomIn)
			doRoomIn(_curObj);
		else if (_obj[_curObj]._flag & kObjFlagPerson)
			doMouseExamine(_curObj);
		else if (_obj[_curObj]._flag & kObjFlagRoomOut)
			doRoomOut(_curObj);
		else
			doMouseExamine(_curObj);
		break;

	case ME_INVOPERATE:
		doInvOperate();
		break;

	case ME_INVEXAMINE:
		doInvExamine();
		break;

	case ME_USEWITH:
		ShowObjName(0, false);
		doUseWith();
		break;

	default:
		break;
	}
}

void TrecisionEngine::doMouse() {
#define POSUP 0
#define POSGAME 1
#define POSINV 2

	switch (_curMessage->_event) {
	case ME_MMOVE:
		int8 curPos;
		if (GAMEAREA(_curMessage->_u16Param2))
			curPos = POSGAME;
		else if (isInventoryArea(_curMessage->_u16Param2))
			curPos = POSINV;
		else
			curPos = POSUP;

		if (curPos == POSGAME) {
			// Game area
			if (_flagSomeoneSpeaks || _flagDialogMenuActive || _flagDialogActive)
				break;

			checkMask(_curMessage->_u16Param1, _curMessage->_u16Param2);
			_logicMgr->doMouseGame();
		} else if (curPos == POSINV) {
			if (_logicMgr->doMouseInventory())
				break;
			if ((_flagSomeoneSpeaks && !_flagCharacterSpeak) || _flagDialogMenuActive || _flagDialogActive)
				break;
			if (_animMgr->_playingAnims[kSmackerAction])
				break;

			if (_inventoryStatus == INV_OFF)
				doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else if (_inventoryStatus == INV_INACTION)
				doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, _curMessage->_u16Param1, _curMessage->_u16Param2, 0, 0);
		} else {
			// Up area
			if (_curRoom == kRoomControlPanel)
				break;

			_curObj = 0;
			ShowObjName(_curObj, true);

			if (_flagDialogMenuActive)
				_dialogMgr->updateChoices(_curMessage->_u16Param1, _curMessage->_u16Param2);
		}
		break;

	case ME_MRIGHT:
	case ME_MLEFT:
		if (_flagSomeoneSpeaks) {
			_flagSkipTalk = _flagSkipEnable;
			break;
		}
		if (_actor->_curAction > hWALKIN)
			break;

		if (_flagDialogActive && _flagDialogMenuActive) {
			_dialogMgr->selectChoice(_curMessage->_u16Param1, _curMessage->_u16Param2);
			break;
		}

		_logicMgr->doMouseLeftRight();
		break;
	default:
		break;
	}
}

void TrecisionEngine::doCharacter() {
	switch (_curMessage->_event) {
	case ME_CHARACTERDOACTION:
	case ME_CHARACTERGOTOACTION:
	case ME_CHARACTERGOTOEXAMINE:
	case ME_CHARACTERGOTOEXIT:
	case ME_CHARACTERGOTO:

		if (nextStep()) {
			_characterInMovement = false;
			_characterGoToPosition = -1;
			_flagWaitRegen = true;
		} else
			_characterInMovement = true;

		if (_fastWalk) {
			if (nextStep()) {
				_characterInMovement = false;
				_characterGoToPosition = -1;
				_flagWaitRegen = true;
			} else
				_characterInMovement = true;
		}

		_flagPaintCharacter = true;

		if (_characterInMovement)
			REEVENT;
		else {
			showCursor();

			if (_curMessage->_event == ME_CHARACTERGOTOACTION)
				doEvent(MC_ACTION, ME_MOUSEOPERATE, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, 0, _curMessage->_u32Param);
			else if (_curMessage->_event == ME_CHARACTERGOTOEXAMINE)
				doEvent(MC_ACTION, ME_MOUSEEXAMINE, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, 0, _curMessage->_u32Param);
			else if (_curMessage->_event == ME_CHARACTERGOTOEXIT) {
				_flagShowCharacter = false;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, _curMessage->_u8Param, _curMessage->_u32Param);
			} else if (_curMessage->_event == ME_CHARACTERDOACTION) {
				_lastObj = 0;
				ShowObjName(_curObj, true);
				refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
			}
		}
		break;

	case ME_CHARACTERACTION:
		if (_flagWaitRegen)
			REEVENT;
		_characterQueue.initQueue();
		_inventoryRefreshStartLine = INVENTORY_HIDE;
		refreshInventory(_inventoryRefreshStartIcon, INVENTORY_HIDE);
		_inventoryStatus = INV_OFF;
		if (_curMessage->_u16Param1 > hLAST) {
			_animMgr->startSmkAnim(_curMessage->_u16Param1);
			InitAtFrameHandler(_curMessage->_u16Param1, _curMessage->_u32Param);
			hideCursor();
			doEvent(MC_CHARACTER, ME_CHARACTERCONTINUEACTION, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, _curMessage->_u8Param, _curMessage->_u32Param);
		} else
			actorDoAction(_curMessage->_u16Param1);

		clearText();
		break;

	case ME_CHARACTERCONTINUEACTION:
		_flagShowCharacter = false;
		AtFrameHandler(kAnimTypeCharacter);
		//	If the animation is over
		if (!_animMgr->_playingAnims[kSmackerAction]) {
			showCursor();
			_flagShowCharacter = true;
			_characterInMovement = false;
			_characterQueue.initQueue();
			AtFrameEnd(kAnimTypeCharacter);
			_flagWaitRegen = true;
			_lastObj = 0;
			ShowObjName(_curObj, true);
			//	If the room changes at the end
			if (_curMessage->_u16Param2) {
				_flagShowCharacter = false;
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _curMessage->_u16Param2, 0, _curMessage->_u8Param, _curMessage->_u32Param);
			} else if (_curMessage->_u8Param)
				setPosition(_curMessage->_u8Param);

			if ((_curMessage->_u16Param1 == _obj[oCANCELLATA1B]._anim) && !(_obj[oBOTTIGLIA1D]._mode & OBJMODE_OBJSTATUS) && !(_obj[oRETE17]._mode & OBJMODE_OBJSTATUS)) {
				_dialogMgr->playDialog(dF181);
				hideCursor();
				setPosition(1);
			}
		} else
			REEVENT;
		break;
	default:
		break;
	}
}

void TrecisionEngine::doSystem() {
	switch (_curMessage->_event) {
	case ME_START:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _curRoom, 0, 0, _curObj);
		break;

	case ME_REDRAWROOM:
		RedrawRoom();
		break;

	case ME_CHANGEROOM:
		if (_curRoom == 0)
			return;

		// if regen still has to occur
		if (_flagWaitRegen)
			REEVENT;

		_logicMgr->doSystemChangeRoom();

		setPosition(_curMessage->_u8Param);
		actorStop();

		if (_curMessage->_u16Param2)
			StartCharacterAction(_curMessage->_u16Param2, 0, 0, 0);

		_logicMgr->endChangeRoom();

		_room[_curRoom]._flag |= kObjFlagDone; // Visited
		drawCharacter(CALCPOINTS);                         // for right _actorPos entrance

		break;
	default:
		break;
	}
}

void TrecisionEngine::doIdle() {
	char c = getKey();
	switch (c) {
	// Quit
	case 'q':
	case 'Q':
		if (!_flagDialogActive && !_flagDialogMenuActive) {
			if (quitGame())
				doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		}
		break;

	// Skip
	case 0x1B:
		if (!_flagSomeoneSpeaks && !_flagscriptactive && !_flagDialogActive && !_flagDialogMenuActive && (_actor->_curAction < hWALKIN) && !_flagUseWithStarted && _flagShowCharacter && !_animMgr->_playingAnims[kSmackerAction]) {
			actorStop();
			nextStep();
			showCursor();
			_obj[o00EXIT]._goRoom = _curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, kRoomControlPanel, 0, 0, c);
			_flagShowCharacter = false;
			_flagCharacterExists = false;
			::createThumbnailFromScreen(&_thumbnail);
		}
		break;

	// Sys
	case 0x3B:
		if (!_flagSomeoneSpeaks && !_flagscriptactive && !_flagDialogActive && !_flagDialogMenuActive && (_actor->_curAction < hWALKIN) && !_flagUseWithStarted && _flagShowCharacter && !_animMgr->_playingAnims[kSmackerAction]) {
			actorStop();
			nextStep();
			showCursor();
			_obj[o00EXIT]._goRoom = _curRoom;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, kRoomControlPanel, 0, 0, c);
			_flagShowCharacter = false;
			_flagCharacterExists = false;
			::createThumbnailFromScreen(&_thumbnail);
		}
		break;

	// Save
	case 0x3C:
		if (!_flagSomeoneSpeaks && !_flagscriptactive && !_flagDialogActive && !_flagDialogMenuActive && (_actor->_curAction < hWALKIN) && !_flagUseWithStarted && _flagShowCharacter && !_animMgr->_playingAnims[kSmackerAction]) {
			::createThumbnailFromScreen(&_thumbnail);
			dataSave();
			showInventoryName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, _mouseX, _mouseY, 0, 0);
			refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
		}
		break;

	// Load
	case 0x3D:
		if (!_flagSomeoneSpeaks && !_flagscriptactive && !_flagDialogActive && !_flagDialogMenuActive && (_actor->_curAction < hWALKIN) && !_flagUseWithStarted && _flagShowCharacter && !_animMgr->_playingAnims[kSmackerAction]) {
			::createThumbnailFromScreen(&_thumbnail);
			if (!dataLoad()) {
				showInventoryName(NO_OBJECTS, false);
				doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, _mouseX, _mouseY, 0, 0);
				refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
			}
		}
		break;
	default:
		break;
	}

	if (GAMEAREA(_mouseY) && ((_inventoryStatus == INV_ON) || (_inventoryStatus == INV_INACTION)))
		doEvent(MC_INVENTORY, ME_CLOSE, MP_SYSTEM, 0, 0, 0, 0);

	if (_inventoryScrollTime > TheTime)
		_inventoryScrollTime = TheTime;

	if (isInventoryArea(_mouseY) && (TheTime > (INVSCROLLSP + _inventoryScrollTime))) {
		doScrollInventory(_mouseX);
		_inventoryScrollTime = TheTime;
	}

	if (g_engine->shouldQuit() && !_flagDialogActive && !_flagDialogMenuActive)
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
}

} // End of namespace Trecision
