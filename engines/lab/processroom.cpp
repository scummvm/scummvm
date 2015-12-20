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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "gui/message.h"

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {

#define NOFILE         "no file"

bool LabEngine::checkConditions(int16 *condition) {
	if (!condition)
		return true;

	if (condition[0] == 0)
		return true;

	int counter = 1;
	bool res = _conditions->in(condition[0]);

	while (condition[counter] && res) {
		res = _conditions->in(condition[counter]);
		counter++;
	}

	return res;
}

ViewData *LabEngine::getViewData(uint16 roomNum, uint16 direction) {
	if (_rooms[roomNum]._roomMsg == "")
		_resource->readViews(roomNum);

	ViewData *view = _rooms[roomNum]._view[direction];

	do {
		if (checkConditions(view->_condition))
			break;

		view = view->_nextCondition;
	} while (true);

	return view;
}

CloseData *LabEngine::getObject(Common::Point pos, CloseDataPtr closePtr) {
	CloseDataPtr wrkClosePtr;
	if (!closePtr)
		wrkClosePtr = getViewData(_roomNum, _direction)->_closeUps;
	else
		wrkClosePtr = closePtr->_subCloseUps;

	Common::Rect objRect;
	while (wrkClosePtr) {
		objRect = _utils->rectScale(wrkClosePtr->_x1, wrkClosePtr->_y1, wrkClosePtr->_x2, wrkClosePtr->_y2);
		if (objRect.contains(pos))
			return wrkClosePtr;

		wrkClosePtr = wrkClosePtr->_nextCloseUp;
	}

	return nullptr;
}

CloseDataPtr LabEngine::findClosePtrMatch(CloseDataPtr closePtr, CloseDataPtr closePtrList) {
	CloseDataPtr resClosePtr;

	while (closePtrList) {
		if ((closePtr->_x1 == closePtrList->_x1) && (closePtr->_x2 == closePtrList->_x2) &&
			  (closePtr->_y1 == closePtrList->_y1) && (closePtr->_y2 == closePtrList->_y2) &&
			  (closePtr->_depth == closePtrList->_depth))
			return closePtrList;

		resClosePtr = findClosePtrMatch(closePtr, closePtrList->_subCloseUps);

		if (resClosePtr)
			return resClosePtr;
		else
			closePtrList = closePtrList->_nextCloseUp;
	}

	return nullptr;
}

Common::String LabEngine::getPictName(CloseDataPtr *closePtrList) {
	ViewData *viewPtr = getViewData(_roomNum, _direction);

	if (*closePtrList) {
		*closePtrList = findClosePtrMatch(*closePtrList, viewPtr->_closeUps);

		if (*closePtrList)
			return (*closePtrList)->_graphicName;
	}

	return viewPtr->_graphicName;
}

void LabEngine::drawDirection(CloseDataPtr closePtr) {
	if (closePtr && closePtr->_message != "") {
		_graphics->drawMessage(closePtr->_message.c_str());
		return;
	}

	Common::String message;

	if (_rooms[_roomNum]._roomMsg != "") {
		message = Common::String(_rooms[_roomNum]._roomMsg).c_str();
		message += ", ";
	}

	if (_direction == NORTH)
		message += _resource->getStaticText(kTextFacingNorth);
	else if (_direction == EAST)
		message += _resource->getStaticText(kTextFacingEast);
	else if (_direction == SOUTH)
		message += _resource->getStaticText(kTextFacingSouth);
	else if (_direction == WEST)
		message += _resource->getStaticText(kTextFacingWest);

	_graphics->drawMessage(message.c_str());
}

uint16 LabEngine::processArrow(uint16 curDirection, uint16 arrow) {
	if (arrow == 1) { // Forward
		uint16 room = _rooms[_roomNum]._doors[curDirection];
		if (room != 0)
			_roomNum = room;

		return curDirection;
	} else if (arrow == 0) { // Left
		if (curDirection == NORTH)
			return WEST;
		else if (curDirection == WEST)
			return SOUTH;
		else if (curDirection == SOUTH)
			return EAST;
		else
			return NORTH;
	} else if (arrow == 2) { // Right
		if (curDirection == NORTH)
			return EAST;
		else if (curDirection == EAST)
			return SOUTH;
		else if (curDirection == SOUTH)
			return WEST;
		else
			return NORTH;
	}

	// Should never reach here!
	return curDirection;
}

void LabEngine::setCurrentClose(Common::Point pos, CloseDataPtr *closePtrList, bool useAbsoluteCoords) {
	CloseDataPtr closePtr;

	if (!*closePtrList)
		closePtr = getViewData(_roomNum, _direction)->_closeUps;
	else
		closePtr = (*closePtrList)->_subCloseUps;

	Common::Rect target;
	while (closePtr) {
		if (!useAbsoluteCoords)
			target = Common::Rect(closePtr->_x1, closePtr->_y1, closePtr->_x2, closePtr->_y2);
		else
			target = Common::Rect(_utils->scaleX(closePtr->_x1), _utils->scaleY(closePtr->_y1), _utils->scaleX(closePtr->_x2), _utils->scaleY(closePtr->_y2));

		if (target.contains(pos) && closePtr->_graphicName != "") {
			*closePtrList = closePtr;
			return;
		}

		closePtr = closePtr->_nextCloseUp;
	}
}

bool LabEngine::takeItem(Common::Point pos, CloseDataPtr *closePtrList) {
	CloseDataPtr closePtr;

	if (!*closePtrList) {
		closePtr = getViewData(_roomNum, _direction)->_closeUps;
	} else if ((*closePtrList)->_closeUpType < 0) {
		_conditions->inclElement(abs((*closePtrList)->_closeUpType));
		return true;
	} else
		closePtr = (*closePtrList)->_subCloseUps;

	Common::Rect objRect;
	while (closePtr) {
		objRect = Common::Rect(_utils->scaleX(closePtr->_x1), _utils->scaleX(closePtr->_y1), _utils->scaleX(closePtr->_x2), _utils->scaleX(closePtr->_y2));
		if (objRect.contains(pos) && (closePtr->_closeUpType < 0)) {
			_conditions->inclElement(abs(closePtr->_closeUpType));
			return true;
		}

		closePtr = closePtr->_nextCloseUp;
	}

	return false;
}

void LabEngine::doActions(Action *actionList, CloseDataPtr *closePtrList) {
	while (actionList) {
		_music->updateMusic();

		switch (actionList->_actionType) {
		case PLAYSOUND:
			_music->_loopSoundEffect = false;
			_music->_waitTillFinished = true;
			_music->readMusic(actionList->_messages[0], true);
			_music->_waitTillFinished = false;
			break;

		case PLAYSOUNDB:
			_music->_loopSoundEffect = false;
			_music->_waitTillFinished = false;
			_music->readMusic(actionList->_messages[0], false);
			break;

		case PLAYSOUNDCONT:
			_music->_loopSoundEffect = true;
			_music->readMusic(actionList->_messages[0], _music->_waitTillFinished);
			break;

		case SHOWDIFF:
			_graphics->readPict(actionList->_messages[0].c_str(), true);
			break;

		case SHOWDIFFCONT:
			_graphics->readPict(actionList->_messages[0].c_str(), false);
			break;

		case LOADDIFF:
			if (actionList->_messages[0].size())
				// Puts a file into memory
				_graphics->loadPict(actionList->_messages[0].c_str());

			break;

		case TRANSITION:
			_graphics->doTransition((TransitionType)actionList->_param1, closePtrList, actionList->_messages[0].c_str());
			break;

		case NOUPDATE:
			_noUpdateDiff = true;
			_anim->_doBlack = false;
			break;

		case FORCEUPDATE:
			_curFileName = " ";
			break;

		case SHOWCURPICT: {
			Common::String test = getPictName(closePtrList);

			if (test != _curFileName) {
				_curFileName = test;
				_graphics->readPict(_curFileName.c_str(), true);
			}
			}
			break;

		case SETELEMENT:
			_conditions->inclElement(actionList->_param1);
			break;

		case UNSETELEMENT:
			_conditions->exclElement(actionList->_param1);
			break;

		case SHOWMESSAGE: {
			_graphics->_doNotDrawMessage = false;

			Common::String text = actionList->_messages[0];
			if (_graphics->_longWinInFront)
				_graphics->longDrawMessage(text);
			else
				_graphics->drawMessage(text);

			_graphics->_doNotDrawMessage = true;
			}
			break;

		case CSHOWMESSAGE:
			if (!*closePtrList) {
				Common::String text = actionList->_messages[0];
				_graphics->_doNotDrawMessage = false;
				_graphics->drawMessage(text);
				_graphics->_doNotDrawMessage = true;
			}

			break;

		case SHOWMESSAGES: {
			Common::String *str = actionList->_messages;
			_graphics->_doNotDrawMessage = false;
			_graphics->drawMessage(str[_utils->getRandom(actionList->_param1)]);
			_graphics->_doNotDrawMessage = true;
			}
			break;

		case SETPOSITION:
			if (actionList->_param1 & 0x8000) {
				// This is a Wyrmkeep Windows trial version, thus stop at this
				// point, since we can't check for game payment status
				_graphics->readPict(getPictName(closePtrList), true);
				actionList = nullptr;
				GUI::MessageDialog trialMessage("This is the end of the trial version. You can play the full game using the original interpreter from Wyrmkeep");
				trialMessage.runModal();
				continue;
			}

			_roomNum   = actionList->_param1;
			_direction = actionList->_param2 - 1;
			*closePtrList = nullptr;
			_anim->_doBlack = true;
			break;

		case SETCLOSEUP: {
			Common::Point curPos = Common::Point(_utils->scaleX(actionList->_param1), _utils->scaleY(actionList->_param2));
				CloseDataPtr tmpClosePtr = getObject(curPos, *closePtrList);

				if (tmpClosePtr)
					*closePtrList = tmpClosePtr;
			}
			break;

		case MAINVIEW:
			*closePtrList = nullptr;
			break;

		case SUBINV:
			if (_inventory[actionList->_param1]._quantity)
				(_inventory[actionList->_param1]._quantity)--;

			if (_inventory[actionList->_param1]._quantity == 0)
				_conditions->exclElement(actionList->_param1);

			break;

		case ADDINV:
			(_inventory[actionList->_param1]._quantity) += actionList->_param2;
			_conditions->inclElement(actionList->_param1);
			break;

		case SHOWDIR:
			_graphics->_doNotDrawMessage = false;
			break;

		case WAITSECS: {
				uint32 targetMillis = g_system->getMillis() + actionList->_param1 * 1000;

				_graphics->screenUpdate();

				while (g_system->getMillis() < targetMillis) {
					_music->updateMusic();
					_anim->diffNextFrame();
				}
			}
			break;

		case STOPMUSIC:
			_music->setMusic(false);
			break;

		case STARTMUSIC:
			_music->setMusic(true);
			break;

		case CHANGEMUSIC:
			_music->changeMusic(actionList->_messages[0]);
			_music->setMusicReset(false);
			break;

		case RESETMUSIC:
			_music->resetMusic();
			_music->setMusicReset(true);
			break;

		case FILLMUSIC:
			_music->updateMusic();
			break;

		case WAITSOUND:
			while (_music->isSoundEffectActive()) {
				_music->updateMusic();
				_anim->diffNextFrame();
				waitTOF();
			}

			break;

		case CLEARSOUND:
			if (_music->_loopSoundEffect) {
				_music->_loopSoundEffect = false;
				_music->stopSoundEffect();
			} else if (_music->isSoundEffectActive())
				_music->stopSoundEffect();

			break;

		case WINMUSIC:
			_music->freeMusic();
			_music->initMusic("Music:WinGame");
			break;

		case WINGAME:
			_quitLab = true;
			showLab2Teaser();
			break;

		case LOSTGAME:
			// This seems to be unused?
			error("Unused opcode LOSTGAME has been called");
			break;

		case RESETBUFFER:
			_graphics->freePict();
			break;

		case SPECIALCMD:
			if (actionList->_param1 == 0)
				_anim->_doBlack = true;
			else if (actionList->_param1 == 1)
				_anim->_doBlack = (_closeDataPtr == nullptr);
			else if (actionList->_param1 == 2)
				_anim->_doBlack = (_closeDataPtr != nullptr);
			else if (actionList->_param1 == 5) {
				// inverse the palette
				for (uint16 idx = (8 * 3); idx < (255 * 3); idx++)
					_anim->_diffPalette[idx] = 255 - _anim->_diffPalette[idx];

				waitTOF();
				_graphics->setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (actionList->_param1 == 4) {
				// white the palette
				_graphics->whiteScreen();
				waitTOF();
				waitTOF();
			} else if (actionList->_param1 == 6) {
				// Restore the palette
				waitTOF();
				_graphics->setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (actionList->_param1 == 7) {
				// Quick pause
				waitTOF();
				waitTOF();
				waitTOF();
			}

			break;
		}

		actionList = actionList->_nextAction;
	}

	if (_music->_loopSoundEffect) {
		_music->_loopSoundEffect = false;
		_music->stopSoundEffect();
	} else {
		while (_music->isSoundEffectActive()) {
			_music->updateMusic();
			_anim->diffNextFrame();
			waitTOF();
		}
	}
}

bool LabEngine::doActionRuleSub(int16 action, int16 roomNum, CloseDataPtr closePtr, CloseDataPtr *setCloseList, bool allowDefaults) {
	action++;

	if (closePtr) {
		RuleList *rules = _rooms[_roomNum]._rules;

		if (!rules && (roomNum == 0)) {
			_resource->readViews(roomNum);
			rules = _rooms[roomNum]._rules;
		}

		for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
			if (((*rule)->_ruleType == ACTION) &&
				(((*rule)->_param1 == action) || (((*rule)->_param1 == 0) && allowDefaults))) {
				if ((((*rule)->_param2 == closePtr->_closeUpType) ||
					  (((*rule)->_param2 == 0) && allowDefaults)) ||
					  ((action == 1) && ((*rule)->_param2 == (-closePtr->_closeUpType)))) {
					if (checkConditions((*rule)->_condition)) {
						doActions((*rule)->_actionList, setCloseList);
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool LabEngine::doActionRule(Common::Point pos, int16 action, int16 roomNum, CloseDataPtr *closePtrList) {
	if (roomNum)
		_newFileName = NOFILE;
	else
		_newFileName = _curFileName;

	CloseDataPtr curClosePtr = getObject(pos, *closePtrList);

	if (doActionRuleSub(action, roomNum, curClosePtr, closePtrList, false))
		return true;
	else if (doActionRuleSub(action, roomNum, *closePtrList, closePtrList, false))
		return true;
	else if (doActionRuleSub(action, roomNum, curClosePtr, closePtrList, true))
		return true;
	else if (doActionRuleSub(action, roomNum, *closePtrList, closePtrList, true))
		return true;

	return false;
}

bool LabEngine::doOperateRuleSub(int16 itemNum, int16 roomNum, CloseDataPtr closePtr, CloseDataPtr *setCloseList, bool allowDefaults) {
	if (closePtr)
		if (closePtr->_closeUpType > 0) {
			RuleList *rules = _rooms[roomNum]._rules;

			if (!rules && (roomNum == 0)) {
				_resource->readViews(roomNum);
				rules = _rooms[roomNum]._rules;
			}

			for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
				if (((*rule)->_ruleType == OPERATE) &&
					  (((*rule)->_param1 == itemNum) || (((*rule)->_param1 == 0) && allowDefaults)) &&
						(((*rule)->_param2 == closePtr->_closeUpType) || (((*rule)->_param2 == 0) && allowDefaults))) {
					if (checkConditions((*rule)->_condition)) {
						doActions((*rule)->_actionList, setCloseList);
						return true;
					}
				}
			}
		}

	return false;
}

bool LabEngine::doOperateRule(Common::Point pos, int16 ItemNum, CloseDataPtr *closePtrList) {
	_newFileName = NOFILE;
	CloseDataPtr closePtr = getObject(pos, *closePtrList);

	if (doOperateRuleSub(ItemNum, _roomNum, closePtr, closePtrList, false))
		return true;
	else if (doOperateRuleSub(ItemNum, _roomNum, *closePtrList, closePtrList, false))
		return true;
	else if (doOperateRuleSub(ItemNum, _roomNum, closePtr, closePtrList, true))
		return true;
	else if (doOperateRuleSub(ItemNum, _roomNum, *closePtrList, closePtrList, true))
		return true;
	else {
		_newFileName = _curFileName;

		if (doOperateRuleSub(ItemNum, 0, closePtr, closePtrList, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, *closePtrList, closePtrList, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, closePtr, closePtrList, true))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, *closePtrList, closePtrList, true))
			return true;
	}

	return false;
}

bool LabEngine::doGoForward(CloseDataPtr *closePtrList) {
	RuleList *rules = _rooms[_roomNum]._rules;

	for (RuleList::iterator ruleIter = rules->begin(); ruleIter != rules->end(); ++ruleIter) {
		Rule *rule = *ruleIter;
		if ((rule->_ruleType == GOFORWARD) && (rule->_param1 == (_direction + 1))) {
			if (checkConditions(rule->_condition)) {
				doActions(rule->_actionList, closePtrList);
				return true;
			}
		}
	}

	return false;
}

bool LabEngine::doTurn(uint16 from, uint16 to, CloseDataPtr *closePtrList) {
	from++;
	to++;

	RuleList *rules = _rooms[_roomNum]._rules;

	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if (((*rule)->_ruleType == TURN) ||
			  (((*rule)->_ruleType == TURNFROMTO) &&
			  ((*rule)->_param1 == from) && ((*rule)->_param2 == to))) {
			if (checkConditions((*rule)->_condition)) {
				doActions((*rule)->_actionList, closePtrList);
				return true;
			}
		}
	}

	return false;
}

bool LabEngine::doMainView(CloseDataPtr *closePtrList) {
	RuleList *rules = _rooms[_roomNum]._rules;
	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if ((*rule)->_ruleType == GOMAINVIEW) {
			if (checkConditions((*rule)->_condition)) {
				doActions((*rule)->_actionList, closePtrList);
				return true;
			}
		}
	}

	return false;
}

} // End of namespace Lab
