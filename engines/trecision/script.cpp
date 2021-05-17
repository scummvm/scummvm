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

#include "common/scummsys.h"
#include "graphics/scaler.h"

#include "trecision/anim.h"
#include "trecision/scheduler.h"
#include "trecision/3d.h"
#include "trecision/actor.h"
#include "trecision/dialog.h"
#include "trecision/logic.h"
#include "trecision/graphics.h"
#include "trecision/defines.h"
#include "trecision/trecision.h"
#include "trecision/sound.h"
#include "trecision/text.h"
#include "trecision/video.h"

namespace Trecision {

void SScriptFrame::sendFrame() {
	g_vm->_scheduler->doEvent(_class, _event, MP_DEFAULT, _u16Param1, _u16Param2, _u8Param, _u32Param);
}

void TrecisionEngine::endScript() {
	_curStack--;
	if (_curStack == 0) {
		_flagScriptActive = false;
		_graphicsMgr->showCursor();
		_textMgr->redrawString();
	}
}

void TrecisionEngine::playScript(uint16 id) {
	_curStack++;
	_flagScriptActive = true;
	_graphicsMgr->hideCursor();
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
	if (_characterQueue.testEmptyCharacterQueue4Script() && _gameQueue.testEmptyQueue(MC_DIALOG)) {
		_curScriptFrame[_curStack]++;
		_graphicsMgr->hideCursor();

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

bool TrecisionEngine::quitPrompt() {
	_graphicsMgr->clearScreenBufferTop();

	SDText drawText;
	drawText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageConfirmExit]
	);
	drawText.draw();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	freeKey();
	checkSystem();

	_graphicsMgr->clearScreenBufferTop();

	const Common::KeyCode ch = waitKey();
	return (ch == Common::KEYCODE_y || ch == Common::KEYCODE_j); // German confirmation is J, English and French use 'Y'
}

void TrecisionEngine::demoOver() {
	_graphicsMgr->clearScreenBufferTop();

	SDText drawText;
	drawText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageDemoOver]
	);
	drawText.draw();

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
			_textMgr->characterSay(2003);

		if (!_curObj || !isObjectVisible(_curObj))
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
				_textMgr->clearLastText();
			} else
				_scheduler->doEvent(MC_ACTION, ME_USEWITH, MP_SYSTEM, 0, 0, 0, 0);
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
			_textMgr->showObjName(_curObj, true);
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
		_textMgr->showObjName(0, false);
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
		if (isGameArea(Common::Point(_curMessage->_u16Param1, _curMessage->_u16Param2)))
			curPos = POSGAME;
		else if (isInventoryArea(Common::Point(_curMessage->_u16Param1, _curMessage->_u16Param2)))
			curPos = POSINV;
		else
			curPos = POSUP;

		if (curPos == POSGAME) {
			// Game area
			if (_flagSomeoneSpeaks || _flagDialogMenuActive || _flagDialogActive)
				break;

			checkMask(Common::Point(_curMessage->_u16Param1, _curMessage->_u16Param2));
			_logicMgr->doMouseGame();
		} else if (curPos == POSINV) {
			if (_logicMgr->doMouseInventory())
				break;
			if ((_flagSomeoneSpeaks && !_flagCharacterSpeak) || _flagDialogMenuActive || _flagDialogActive)
				break;
			if (_animMgr->_playingAnims[kSmackerAction])
				break;

			if (_inventoryStatus == INV_OFF)
				_scheduler->doEvent(MC_INVENTORY, ME_OPEN, MP_DEFAULT, 0, 0, 0, 0);
			else if (_inventoryStatus == INV_INACTION)
				_scheduler->doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, _curMessage->_u16Param1, _curMessage->_u16Param2, 0, 0);
		} else {
			// Up area
			if (_curRoom == kRoomControlPanel)
				break;

			_curObj = 0;
			_textMgr->showObjName(_curObj, true);

			if (_flagDialogMenuActive)
				_dialogMgr->updateChoices(_curMessage->_u16Param1, _curMessage->_u16Param2);
		}
		break;

	case ME_MRIGHT:
	case ME_MLEFT:
		if (_flagSomeoneSpeaks) {
			_flagSkipTalk = true;
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

		if (_pathFind->nextStep()) {
			_pathFind->_characterInMovement = false;
			_pathFind->_characterGoToPosition = -1;
			_flagWaitRegen = true;
		} else
			_pathFind->_characterInMovement = true;

		if (_fastWalk) {
			if (_pathFind->nextStep()) {
				_pathFind->_characterInMovement = false;
				_pathFind->_characterGoToPosition = -1;
				_flagWaitRegen = true;
			} else
				_pathFind->_characterInMovement = true;
		}

		_flagPaintCharacter = true;

		if (_pathFind->_characterInMovement)
			reEvent();
		else {
			_graphicsMgr->showCursor();

			if (_curMessage->_event == ME_CHARACTERGOTOACTION)
				_scheduler->doEvent(MC_ACTION, ME_MOUSEOPERATE, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, 0, _curMessage->_u32Param);
			else if (_curMessage->_event == ME_CHARACTERGOTOEXAMINE)
				_scheduler->doEvent(MC_ACTION, ME_MOUSEEXAMINE, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, 0, _curMessage->_u32Param);
			else if (_curMessage->_event == ME_CHARACTERGOTOEXIT) {
				_flagShowCharacter = false;
				_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, _curMessage->_u8Param, _curMessage->_u32Param);
			} else if (_curMessage->_event == ME_CHARACTERDOACTION) {
				_lastObj = 0;
				_textMgr->showObjName(_curObj, true);
				refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
			}
		}
		break;

	case ME_CHARACTERACTION:
		if (_flagWaitRegen)
			reEvent();
		_characterQueue.initQueue();
		_inventoryRefreshStartLine = INVENTORY_HIDE;
		refreshInventory(_inventoryRefreshStartIcon, INVENTORY_HIDE);
		_inventoryStatus = INV_OFF;
		if (_curMessage->_u16Param1 > hLAST) {
			_animMgr->startSmkAnim(_curMessage->_u16Param1);
			_animTypeMgr->init(_curMessage->_u16Param1, _curMessage->_u32Param);
			_graphicsMgr->hideCursor();
			_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERCONTINUEACTION, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, _curMessage->_u8Param, _curMessage->_u32Param);
		} else
			_actor->actorDoAction(_curMessage->_u16Param1);

		_textMgr->clearLastText();
		break;

	case ME_CHARACTERCONTINUEACTION:
		_flagShowCharacter = false;
		_animTypeMgr->handler(kAnimTypeCharacter);
		//	If the animation is over
		if (!_animMgr->_playingAnims[kSmackerAction]) {
			_graphicsMgr->showCursor();
			_flagShowCharacter = true;
			_pathFind->_characterInMovement = false;
			_characterQueue.initQueue();
			_animTypeMgr->end(kAnimTypeCharacter);
			_flagWaitRegen = true;
			_lastObj = 0;
			_textMgr->showObjName(_curObj, true);
			//	If the room changes at the end
			if (_curMessage->_u16Param2) {
				_flagShowCharacter = false;
				_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _curMessage->_u16Param2, 0, _curMessage->_u8Param, _curMessage->_u32Param);
			} else if (_curMessage->_u8Param)
				_pathFind->setPosition(_curMessage->_u8Param);

			if ((_curMessage->_u16Param1 == _obj[oCANCELLATA1B]._anim) && !isObjectVisible(oBOTTIGLIA1D) && !isObjectVisible(oRETE17)) {
				_dialogMgr->playDialog(dF181);
				_graphicsMgr->hideCursor();
				_pathFind->setPosition(1);
			}
		} else
			reEvent();
		break;
	default:
		break;
	}
}

void TrecisionEngine::doSystem() {
	switch (_curMessage->_event) {
	case ME_START:
		_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _curRoom, 0, 0, _curObj);
		break;

	case ME_REDRAWROOM:
		redrawRoom();
		break;

	case ME_CHANGEROOM:
		if (_curRoom == 0)
			return;

		// if regen still has to occur
		if (_flagWaitRegen)
			reEvent();

		_logicMgr->doSystemChangeRoom();

		_pathFind->setPosition(_curMessage->_u8Param);
		_actor->actorStop();

		if (_curMessage->_u16Param2)
			startCharacterAction(_curMessage->_u16Param2, 0, 0, 0);

		_logicMgr->endChangeRoom();

		_room[_curRoom]._flag |= kObjFlagDone; // Visited
		_renderer->drawCharacter(CALCPOINTS);                         // for right _actorPos entrance

		break;
	default:
		break;
	}
}

void TrecisionEngine::doIdle() {
	uint16 c = getKey();
	switch (c) {
	// Quit
	case 'q':
	case 'Q':
		if (!_flagDialogActive && !_flagDialogMenuActive) {
			if (quitPrompt())
				_scheduler->doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		}
		break;

	// Skip
	case 0x1B:
		if (canPlayerInteract()) {
			_actor->actorStop();
			_pathFind->nextStep();
			_graphicsMgr->showCursor();
			_obj[o00EXIT]._goRoom = _curRoom;
			_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, kRoomControlPanel, 0, 0, c);
			_flagShowCharacter = false;
			_flagCharacterExists = false;
			::createThumbnailFromScreen(&_thumbnail);
		}
		break;

	// Sys
	case 0x3B:
		if (canPlayerInteract()) {
			_actor->actorStop();
			_pathFind->nextStep();
			_graphicsMgr->showCursor();
			_obj[o00EXIT]._goRoom = _curRoom;
			_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, kRoomControlPanel, 0, 0, c);
			_flagShowCharacter = false;
			_flagCharacterExists = false;
			::createThumbnailFromScreen(&_thumbnail);
		}
		break;

	// Save
	case 0x3C:
		if (canPlayerInteract()) {
			::createThumbnailFromScreen(&_thumbnail);
			dataSave();
			showInventoryName(NO_OBJECTS, false);
			_scheduler->doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, _mousePos.x, _mousePos.y, 0, 0);
			refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
		}
		break;

	// Load
	case 0x3D:
		if (canPlayerInteract()) {
			::createThumbnailFromScreen(&_thumbnail);
			if (!dataLoad()) {
				showInventoryName(NO_OBJECTS, false);
				_scheduler->doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, _mousePos.x, _mousePos.y, 0, 0);
				refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
			}
		}
		break;
	default:
		break;
	}

	if (isGameArea(_mousePos) && ((_inventoryStatus == INV_ON) || (_inventoryStatus == INV_INACTION)))
		_scheduler->doEvent(MC_INVENTORY, ME_CLOSE, MP_SYSTEM, 0, 0, 0, 0);

	if (_inventoryScrollTime > _curTime)
		_inventoryScrollTime = _curTime;

	if (isInventoryArea(_mousePos) && (_curTime > (INVSCROLLSP + _inventoryScrollTime))) {
		doScrollInventory(_mousePos);
		_inventoryScrollTime = _curTime;
	}

	if (shouldQuit() && !_flagDialogActive && !_flagDialogMenuActive)
		_scheduler->doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
}

void TrecisionEngine::doRoomIn(uint16 curObj) {
	_graphicsMgr->hideCursor();

	uint16 curAction = _obj[curObj]._anim;
	uint16 curPos = _obj[curObj]._ninv;

	_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[curObj]._goRoom, curAction, curPos, curObj);

	_obj[curObj]._flag |= kObjFlagDone;
}

void TrecisionEngine::doRoomOut(uint16 curObj) {
	_graphicsMgr->hideCursor();

	uint16 curAction, curPos;
	_logicMgr->roomOut(curObj, &curAction, &curPos);

	if (curAction)
		_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, curAction, _obj[curObj]._goRoom, curPos, curObj);

	_obj[curObj]._flag |= kObjFlagDone;
}

void TrecisionEngine::doMouseExamine(uint16 curObj) {
	if (!curObj)
		warning("doMouseExamine - curObj not set");

	bool printSentence = _logicMgr->mouseExamine(curObj);

	if (printSentence && _obj[curObj]._examine)
		_textMgr->characterSay(_obj[curObj]._examine);
}

void TrecisionEngine::doMouseOperate(uint16 curObj) {
	if (!curObj)
		warning("doMouseOperate - curObj not set");

	bool printSentence = _logicMgr->mouseOperate(curObj);

	if (printSentence && _obj[curObj]._action)
		_textMgr->characterSay(_obj[curObj]._action);
}

void TrecisionEngine::doMouseTake(uint16 curObj) {
	if (!curObj)
		warning("doMouseTake - curObj not set");

	bool del = _logicMgr->mouseTake(curObj);
	uint16 curAction = _obj[curObj]._anim;

	if (curAction)
		_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, curAction, 0, 0, curObj);

	// Remove object being taken
	if (del) {
		if (curAction) {
			for (uint16 j = 0; j < MAXATFRAME; j++) {
				SAtFrame *frame = &_animMgr->_animTab[curAction]._atFrame[j];
				if (frame->_type == ATFCLR && frame->_index == curObj)
					break;

				if (frame->_type == 0) {
					frame->_child = 0;
					frame->_numFrame = 1;
					frame->_type = ATFCLR;
					frame->_index = curObj;
					break;
				}
			}
		} else {
			setObjectVisible(curObj, false);
		}
	}
	addIcon(_obj[_curObj]._ninv);
}

void TrecisionEngine::doMouseTalk(uint16 curObj) {
	if (!curObj)
		warning("doMouseTalk - curObj not set");

	bool printSentence = _logicMgr->mouseTalk(curObj);

	if (printSentence)
		_dialogMgr->playDialog(_obj[curObj]._goRoom);
}

void TrecisionEngine::doUseWith() {
	if (_useWithInv[USED]) {
		if (_useWithInv[WITH])
			doInventoryUseWithInventory();
		else
			doInventoryUseWithScreen();
	} else
		doScreenUseWithScreen();

	_useWith[USED] = 0;
	_useWith[WITH] = 0;
	_useWithInv[USED] = false;
	_useWithInv[WITH] = false;
	_flagUseWithStarted = false;
}

void TrecisionEngine::doScreenUseWithScreen() {
	if (!_useWith[USED] || !_useWith[WITH])
		warning("doScreenUseWithScreen - _useWith not set properly");

	if (_pathFind->_characterInMovement)
		return;

	bool printSentence = _logicMgr->useScreenWithScreen();

	if (printSentence)
		_textMgr->characterSay(_obj[_useWith[USED]]._action);
}

void TrecisionEngine::doInvExamine() {
	if (!_curInventory)
		warning("doInvExamine - _curInventory not set properly");

	if (_inventoryObj[_curInventory]._examine)
		_textMgr->characterSay(_inventoryObj[_curInventory]._examine);
}

void TrecisionEngine::doInvOperate() {
	if (!_curInventory)
		warning("doInvOperate - _curInventory not set properly");

	bool printSentence = _logicMgr->operateInventory();
	if (_inventoryObj[_curInventory]._action && printSentence)
		_textMgr->characterSay(_inventoryObj[_curInventory]._action);
}

void TrecisionEngine::doDoing() {
	switch (_curMessage->_event) {
	case ME_INITOPENCLOSE:
		if (_actor->_curAction == hSTAND)
			reEvent();
		else if (_actor->_curFrame == 4)
			_scheduler->doEvent(_curMessage->_class, ME_OPENCLOSE, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, _curMessage->_u8Param, _curMessage->_u32Param);
		else
			reEvent();

		break;
	case ME_OPENCLOSE: {
		uint16 curObj = _curMessage->_u16Param1;
		uint16 curAnim = _curMessage->_u16Param2;
		setObjectVisible(curObj, false);
		if (curAnim)
			_scheduler->doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, curAnim, 0, 0, 0);

		_curMessage->_event = ME_WAITOPENCLOSE;
	}
		// no break!
	case ME_WAITOPENCLOSE:
		if (_actor->_curAction == hSTAND)
			_graphicsMgr->showCursor();
		break;

	default:
		break;
	}
}

void TrecisionEngine::doScript() {
	Message *message = _curMessage;
	uint8 scope = message->_u8Param;
	uint16 index = message->_u16Param1;
	uint16 index2 = message->_u16Param2;
	uint32 value = message->_u32Param;
	SObject *obj = &_obj[index];

	switch (message->_event) {
	case ME_PAUSE:
		if (!_pauseStartTime) {
			_pauseStartTime = _curTime;
			_scheduler->doEvent(message->_class, message->_event, message->_priority, message->_u16Param1, message->_u16Param2, message->_u8Param, message->_u32Param);
		} else if (_curTime >= (_pauseStartTime + message->_u16Param1))
			_pauseStartTime = 0;
		else
			_scheduler->doEvent(message->_class, message->_event, message->_priority, message->_u16Param1, message->_u16Param2, message->_u8Param, message->_u32Param);

		break;

	case ME_SETOBJ:
		switch (scope) {
		case C_ONAME:
			obj->_name = (uint16)value;
			break;
		case C_OEXAMINE:
			obj->_examine = (uint16)value;
			break;
		case C_OACTION:
			obj->_action = (uint16)value;
			break;
		case C_OGOROOM:
			obj->_goRoom = (uint8)value;
			break;
		case C_OMODE:
			if (value)
				obj->_mode |= (uint8)index2;
			else
				obj->_mode &= ~(uint8)index2;
			break;
		case C_OFLAG:
			if (value)
				obj->_flag |= (uint8)index2;
			else
				obj->_flag &= ~(uint8)index2;
			break;
		default:
			break;
		}
		break;

	case ME_SETINVOBJ:
		switch (scope) {
		case C_INAME:
			_inventoryObj[index]._name = (uint16)value;
			break;
		case C_IEXAMINE:
			_inventoryObj[index]._examine = (uint16)value;
			break;
		case C_IACTION:
			_inventoryObj[index]._action = (uint16)value;
			break;
		case C_IFLAG:
			if (value)
				_inventoryObj[index]._flag |= (uint8)index2;
			else
				_inventoryObj[index]._flag &= ~(uint8)index2;
			break;
		default:
			break;
		}
		break;

	case ME_ADDICON:
		addIcon(index);
		break;

	case ME_KILLICON:
		removeIcon(index);
		break;

	case ME_PLAYDIALOG:
		_dialogMgr->playDialog(index);
		break;

	case ME_CHARACTERSAY:
		_textMgr->characterSay(message->_u32Param);
		break;

	case ME_PLAYSOUND:
		_soundMgr->play(index);
		break;

	case ME_STOPSOUND:
		_soundMgr->stop(index);
		break;

	case ME_REGENROOM:
		break;

	case ME_CHANGER:
		_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, index, index2, value, _curObj);
		break;

	default:
		break;
	}
}

void TrecisionEngine::processCurrentMessage() {
	switch (_curMessage->_class) {
	case MC_CHARACTER:
		doCharacter();
		break;

	case MC_IDLE:
		doIdle();
		break;

	case MC_MOUSE:
		doMouse();
		break;

	case MC_SYSTEM:
		doSystem();
		break;

	case MC_INVENTORY:
		doInventory();
		break;

	case MC_ACTION:
		doAction();
		break;

	case MC_STRING:
		_textMgr->doString();
		break;

	case MC_DOING:
		doDoing();
		break;

	case MC_DIALOG:
		_dialogMgr->doDialog();
		break;

	case MC_SCRIPT:
		doScript();
		break;

	default:
		break;
	}
}

} // End of namespace Trecision
