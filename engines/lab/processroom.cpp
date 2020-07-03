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

#include "common/translation.h"
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

bool LabEngine::checkConditions(const Common::Array<int16> &condition) {
	for (unsigned int i = 0; i < condition.size(); ++i)
		if (!_conditions->in(condition[i]))
			return false;

	return true;
}

ViewData *LabEngine::getViewData(uint16 roomNum, uint16 direction) {
	if (_rooms[roomNum]._roomMsg.empty())
		_resource->readViews(roomNum);

	ViewDataList &views = _rooms[roomNum]._view[direction];
	ViewDataList::iterator view;

	for (view = views.begin(); view != views.end(); ++view) {
		if (checkConditions(view->_condition))
			return &(*view);
	}

	error("No view with matching condition found");
}

const CloseData *LabEngine::getObject(Common::Point pos, const CloseData *closePtr) {
	const CloseDataList *list;
	if (!closePtr)
		list = &(getViewData(_roomNum, _direction)->_closeUps);
	else
		list = &(closePtr->_subCloseUps);

	CloseDataList::const_iterator wrkClosePtr;

	for (wrkClosePtr = list->begin(); wrkClosePtr != list->end(); ++wrkClosePtr) {
		Common::Rect objRect;
		objRect = _utils->rectScale(wrkClosePtr->_x1, wrkClosePtr->_y1, wrkClosePtr->_x2, wrkClosePtr->_y2);
		if (objRect.contains(pos))
			return &(*wrkClosePtr);
	}

	return nullptr;
}

const CloseData *LabEngine::findClosePtrMatch(const CloseData *closePtr, const CloseDataList &list) {
	CloseDataList::const_iterator i;

	for (i = list.begin(); i != list.end(); ++i) {
		if ((closePtr->_x1 == i->_x1) && (closePtr->_x2 == i->_x2) &&
			  (closePtr->_y1 == i->_y1) && (closePtr->_y2 == i->_y2) &&
			  (closePtr->_depth == i->_depth))
			return &(*i);

		const CloseData *resClosePtr = findClosePtrMatch(closePtr, i->_subCloseUps);

		if (resClosePtr)
			return resClosePtr;
	}

	return nullptr;
}

Common::String LabEngine::getPictName(bool useClose) {
	ViewData *viewPtr = getViewData(_roomNum, _direction);

	if (useClose && _closeDataPtr) {
		_closeDataPtr = findClosePtrMatch(_closeDataPtr, viewPtr->_closeUps);

		if (_closeDataPtr)
			return _closeDataPtr->_graphicName;
	}

	return viewPtr->_graphicName;
}

void LabEngine::drawDirection(const CloseData *closePtr) {
	if (closePtr && !closePtr->_message.empty()) {
		_graphics->drawMessage(closePtr->_message, false);
		return;
	}

	Common::String message;

	if (!_rooms[_roomNum]._roomMsg.empty())
		message = _rooms[_roomNum]._roomMsg + ", ";

	if (_direction == kDirectionNorth)
		message += _resource->getStaticText(kTextFacingNorth);
	else if (_direction == kDirectionEast)
		message += _resource->getStaticText(kTextFacingEast);
	else if (_direction == kDirectionSouth)
		message += _resource->getStaticText(kTextFacingSouth);
	else if (_direction == kDirectionWest)
		message += _resource->getStaticText(kTextFacingWest);

	_graphics->drawMessage(message, false);
}

uint16 LabEngine::processArrow(uint16 curDirection, uint16 arrow) {
	if (arrow == 1) { // Forward
		uint16 room = _rooms[_roomNum]._doors[curDirection];
		if (room != 0) {
			_music->checkRoomMusic(_roomNum, room);
			_roomNum = room;
		}

		return curDirection;
	} else if (arrow == 0) { // Left
		if (curDirection == kDirectionNorth)
			return kDirectionWest;
		else if (curDirection == kDirectionWest)
			return kDirectionSouth;
		else if (curDirection == kDirectionSouth)
			return kDirectionEast;
		else
			return kDirectionNorth;
	} else if (arrow == 2) { // Right
		if (curDirection == kDirectionNorth)
			return kDirectionEast;
		else if (curDirection == kDirectionEast)
			return kDirectionSouth;
		else if (curDirection == kDirectionSouth)
			return kDirectionWest;
		else
			return kDirectionNorth;
	}

	// Should never reach here!
	return curDirection;
}

void LabEngine::setCurrentClose(Common::Point pos, const CloseData **closePtrList, bool useAbsoluteCoords, bool next) {
	const CloseDataList *list;

	if (!*closePtrList)
		list = &(getViewData(_roomNum, _direction)->_closeUps);
	else
		list = &((*closePtrList)->_subCloseUps);

	CloseDataList::const_iterator closePtr;
	for (closePtr = list->begin(); closePtr != list->end(); ++closePtr) {
		Common::Rect target;
		if (!useAbsoluteCoords)
			target = Common::Rect(closePtr->_x1, closePtr->_y1, closePtr->_x2, closePtr->_y2);
		else
			target = _utils->rectScale(closePtr->_x1, closePtr->_y1, closePtr->_x2, closePtr->_y2);

		if (target.contains(pos) && (next || !closePtr->_graphicName.empty())) {

			if (next) {
				// cycle to the next one
				++closePtr;
				if (closePtr == list->end())
					closePtr = list->begin();
			}
			*closePtrList = &(*closePtr);

			return;
		}
	}

	// If we got here, no match was found. If we want the "next" close-up,
	// return the first one in the list, if any.
	if (next) {
		if (!list->empty())
			*closePtrList = &(*list->begin());
	}
}

bool LabEngine::takeItem(Common::Point pos) {
	const CloseDataList *list;
	if (!_closeDataPtr) {
		list = &(getViewData(_roomNum, _direction)->_closeUps);
	} else if (_closeDataPtr->_closeUpType < 0) {
		_conditions->inclElement(abs(_closeDataPtr->_closeUpType));
		return true;
	} else
		list = &(_closeDataPtr->_subCloseUps);

	CloseDataList::const_iterator closePtr;
	for (closePtr = list->begin(); closePtr != list->end(); ++closePtr) {
		Common::Rect objRect;
		objRect = _utils->rectScale(closePtr->_x1, closePtr->_y1, closePtr->_x2, closePtr->_y2);
		if (objRect.contains(pos) && (closePtr->_closeUpType < 0)) {
			_conditions->inclElement(abs(closePtr->_closeUpType));
			return true;
		}
	}

	return false;
}

void LabEngine::doActions(const ActionList &actionList) {
	ActionList::const_iterator action;
	for (action = actionList.begin(); action != actionList.end(); ++action) {
		updateEvents();
		if (_quitLab || shouldQuit())
			return;

		switch (action->_actionType) {
		case kActionPlaySound:
			_music->loadSoundEffect(action->_messages[0], false, true);
			break;

		case kActionPlaySoundNoWait:	// only used in scene 7 (street, when teleporting to the surreal maze)
			_music->loadSoundEffect(action->_messages[0], false, false);
			break;

		case kActionPlaySoundLooping:
			_music->loadSoundEffect(action->_messages[0], true, false);
			break;

		case kActionShowDiff:
			_graphics->readPict(action->_messages[0], true);
			break;

		case kActionShowDiffLooping:	// used in scene 44 (heart of the labyrinth, minotaur)
			_graphics->readPict(action->_messages[0], false);
			break;

		case kActionLoadDiff:
			if (!action->_messages[0].empty())
				// Puts a file into memory
				_graphics->loadPict(action->_messages[0]);
			break;

		case kActionLoadBitmap:
			error("Unused opcode kActionLoadBitmap has been called");

		case kActionShowBitmap:
			error("Unused opcode kActionShowBitmap has been called");

		case kActionTransition:
			_graphics->doTransition((TransitionType)action->_param1, action->_messages[0].c_str());
			break;

		case kActionNoUpdate:
			_noUpdateDiff = true;
			_anim->_doBlack = false;
			break;

		case kActionForceUpdate:
			_curFileName = " ";
			break;

		case kActionShowCurPict: {
			Common::String test = getPictName(true);

			if (test != _curFileName) {
				_curFileName = test;
				_graphics->readPict(_curFileName);
			}
			}
			break;

		case kActionSetElement:
			_conditions->inclElement(action->_param1);
			break;

		case kActionUnsetElement:
			_conditions->exclElement(action->_param1);
			break;

		case kActionShowMessage:
			if (_graphics->_longWinInFront)
				_graphics->longDrawMessage(action->_messages[0], true);
			else
				_graphics->drawMessage(action->_messages[0], true);
			break;

		case kActionCShowMessage:
			if (!_closeDataPtr)
				_graphics->drawMessage(action->_messages[0], true);
			break;

		case kActionShowMessages:
			_graphics->drawMessage(action->_messages[_utils->getRandom(action->_param1)], true);
			break;

		case kActionChangeRoom:
			if (action->_param1 & 0x8000) {
				// This is a Wyrmkeep Windows trial version, thus stop at this
				// point, since we can't check for game payment status
				_graphics->readPict(getPictName(true));
				GUI::MessageDialog trialMessage(_("This is the end of the trial version. You can play the full game using the original interpreter from Wyrmkeep"));
				trialMessage.runModal();
				break;
			}

			_music->checkRoomMusic(_roomNum, action->_param1);
			_roomNum   = action->_param1;
			_direction = action->_param2 - 1;
			_closeDataPtr = nullptr;
			_anim->_doBlack = true;
			break;

		case kActionSetCloseup: {
			Common::Point curPos = Common::Point(_utils->scaleX(action->_param1), _utils->scaleY(action->_param2));
				const CloseData *tmpClosePtr = getObject(curPos, _closeDataPtr);

				if (tmpClosePtr)
					_closeDataPtr = tmpClosePtr;
			}
			break;

		case kActionMainView:
			_closeDataPtr = nullptr;
			break;

		case kActionSubInv:
			if (_inventory[action->_param1]._quantity)
				(_inventory[action->_param1]._quantity)--;

			if (_inventory[action->_param1]._quantity == 0)
				_conditions->exclElement(action->_param1);

			break;

		case kActionAddInv:
			(_inventory[action->_param1]._quantity) += action->_param2;
			_conditions->inclElement(action->_param1);
			break;

		case kActionShowDir:
			_graphics->setActionMessage(false);
			break;

		case kActionWaitSecs: {
				uint32 targetMillis = _system->getMillis() + action->_param1 * 1000;

				_graphics->screenUpdate();

				while (_system->getMillis() < targetMillis) {
					updateEvents();
					if (_quitLab || shouldQuit())
						return;
					_anim->diffNextFrame();
				}
			}
			break;

		case kActionStopMusic:	// used in scene 44 (heart of the labyrinth, minotaur)
			_music->freeMusic();
			break;

		case kActionStartMusic:	// unused
			error("Unused opcode kActionStartMusic has been called");
			break;

		case kActionChangeMusic:	// used in scene 46 (museum exhibit, for the alarm)
			_music->changeMusic(action->_messages[0], true, false);
			break;

		case kActionResetMusic:	// used in scene 45 (sheriff's office, after museum)
			_music->resetMusic(true);
			break;

		case kActionFillMusic:
			error("Unused opcode kActionFillMusic has been called");
			break;

		case kActionWaitSound:	// used in scene 44 (heart of the labyrinth / ending)
			while (_music->isSoundEffectActive()) {
				updateEvents();
				if (_quitLab || shouldQuit())
					return;
				_anim->diffNextFrame();
				waitTOF();
			}
			break;

		case kActionClearSound:
			_music->stopSoundEffect();
			break;

		case kActionWinMusic:	// used in scene 44 (heart of the labyrinth / ending)
			_music->freeMusic();
			_music->changeMusic("Music:WinGame", false, false);
			break;

		case kActionWinGame:	// used in scene 44 (heart of the labyrinth / ending)
			_quitLab = true;
			showLab2Teaser();
			break;

		case kActionLostGame:
			error("Unused opcode kActionLostGame has been called");

		case kActionResetBuffer:
			_graphics->freePict();
			break;

		case kActionSpecialCmd:
			if (action->_param1 == 0)
				_anim->_doBlack = true;
			else if (action->_param1 == 1)
				_anim->_doBlack = (_closeDataPtr == nullptr);
			else if (action->_param1 == 2)
				_anim->_doBlack = (_closeDataPtr != nullptr);
			else if (action->_param1 == 5) {
				// inverse the palette
				for (int idx = (8 * 3); idx < (255 * 3); idx++)
					_anim->_diffPalette[idx] = 255 - _anim->_diffPalette[idx];

				waitTOF();
				_graphics->setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (action->_param1 == 4) {
				// white the palette
				_graphics->whiteScreen();
				waitTOF();
				waitTOF();
			} else if (action->_param1 == 6) {
				// Restore the palette
				waitTOF();
				_graphics->setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (action->_param1 == 7) {
				// Quick pause
				waitTOF();
				waitTOF();
				waitTOF();
			}

			break;

		default:
			break;
		}
	}

	_music->stopSoundEffect();
}

bool LabEngine::doActionRuleSub(int16 action, int16 roomNum, const CloseData *closePtr, bool allowDefaults) {
	action++;

	if (closePtr) {
		RuleList *rules = &(_rooms[_roomNum]._rules);

		if (rules->empty() && (roomNum == 0)) {
			_resource->readViews(roomNum);
			rules = &(_rooms[roomNum]._rules);
		}

		for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
			if ((rule->_ruleType == kRuleTypeAction) &&
				((rule->_param1 == action) || ((rule->_param1 == 0) && allowDefaults))) {
				if (((rule->_param2 == closePtr->_closeUpType) ||
					  ((rule->_param2 == 0) && allowDefaults)) ||
					  ((action == 1) && (rule->_param2 == -closePtr->_closeUpType))) {
					if (checkConditions(rule->_condition)) {
						doActions(rule->_actionList);
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool LabEngine::doActionRule(Common::Point pos, int16 action, int16 roomNum) {
	if (roomNum)
		_newFileName = NOFILE;
	else
		_newFileName = _curFileName;

	const CloseData *curClosePtr = getObject(pos, _closeDataPtr);

	if (doActionRuleSub(action, roomNum, curClosePtr, false))
		return true;
	else if (doActionRuleSub(action, roomNum, _closeDataPtr, false))
		return true;
	else if (doActionRuleSub(action, roomNum, curClosePtr, true))
		return true;
	else if (doActionRuleSub(action, roomNum, _closeDataPtr, true))
		return true;

	return false;
}

bool LabEngine::doOperateRuleSub(int16 itemNum, int16 roomNum, const CloseData *closePtr, bool allowDefaults) {
	if (closePtr)
		if (closePtr->_closeUpType > 0) {
			RuleList *rules = &(_rooms[roomNum]._rules);

			if (rules->empty() && (roomNum == 0)) {
				_resource->readViews(roomNum);
				rules = &(_rooms[roomNum]._rules);
			}

			for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
				if ((rule->_ruleType == kRuleTypeOperate) &&
					  ((rule->_param1 == itemNum) || ((rule->_param1 == 0) && allowDefaults)) &&
						((rule->_param2 == closePtr->_closeUpType) || ((rule->_param2 == 0) && allowDefaults))) {
					if (checkConditions(rule->_condition)) {
						doActions(rule->_actionList);
						return true;
					}
				}
			}
		}

	return false;
}

bool LabEngine::doOperateRule(Common::Point pos, int16 ItemNum) {
	_newFileName = NOFILE;
	const CloseData *closePtr = getObject(pos, _closeDataPtr);

	if (doOperateRuleSub(ItemNum, _roomNum, closePtr, false))
		return true;
	else if (doOperateRuleSub(ItemNum, _roomNum, _closeDataPtr, false))
		return true;
	else if (doOperateRuleSub(ItemNum, _roomNum, closePtr, true))
		return true;
	else if (doOperateRuleSub(ItemNum, _roomNum, _closeDataPtr, true))
		return true;
	else {
		_newFileName = _curFileName;

		if (doOperateRuleSub(ItemNum, 0, closePtr, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, _closeDataPtr, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, closePtr, true))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, _closeDataPtr, true))
			return true;
	}

	return false;
}

bool LabEngine::doGoForward() {
	RuleList &rules = _rooms[_roomNum]._rules;

	for (RuleList::iterator rule = rules.begin(); rule != rules.end(); ++rule) {
		if ((rule->_ruleType == kRuleTypeGoForward) && (rule->_param1 == (_direction + 1))) {
			if (checkConditions(rule->_condition)) {
				doActions(rule->_actionList);
				return true;
			}
		}
	}

	return false;
}

bool LabEngine::doTurn(uint16 from, uint16 to) {
	from++;
	to++;

	RuleList &rules = _rooms[_roomNum]._rules;

	for (RuleList::iterator rule = rules.begin(); rule != rules.end(); ++rule) {
		if ((rule->_ruleType == kRuleTypeTurn) ||
			  ((rule->_ruleType == kRuleTypeTurnFromTo) &&
			  (rule->_param1 == from) && (rule->_param2 == to))) {
			if (checkConditions(rule->_condition)) {
				doActions(rule->_actionList);
				return true;
			}
		}
	}

	return false;
}

bool LabEngine::doMainView() {
	RuleList &rules = _rooms[_roomNum]._rules;
	for (RuleList::iterator rule = rules.begin(); rule != rules.end(); ++rule) {
		if (rule->_ruleType == kRuleTypeGoMainView) {
			if (checkConditions(rule->_condition)) {
				doActions(rule->_actionList);
				return true;
			}
		}
	}

	return false;
}

} // End of namespace Lab
