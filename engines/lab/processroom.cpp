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
#include "lab/labfun.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/anim.h"
#include "lab/interface.h"

namespace Lab {

/* Global parser data */
#define NOFILE         "no file"

/*****************************************************************************/
/* Generates a random number.                                                */
/*****************************************************************************/
uint16 getRandom(uint16 max) {
	uint32 secs, micros;

	g_lab->getTime(&secs, &micros);
	return ((micros + secs) % max);
}

/*****************************************************************************/
/* Checks whether all the conditions in a condition list are met.            */
/*****************************************************************************/
static bool checkConditions(int16 *condition) {
	if (condition == NULL)
		return true;

	if (condition[0] == 0)
		return true;

	int counter = 1;
	bool res = g_lab->_conditions->in(condition[0]);

	while (condition[counter] && res) {
		res = g_lab->_conditions->in(condition[counter]);
		counter++;
	}

	return res;
}

/*****************************************************************************/
/* Gets the current ViewDataPointer.                                         */
/*****************************************************************************/
ViewData *getViewData(uint16 roomNum, uint16 direction) {
	if (!g_lab->_rooms[roomNum]._roomMsg)
		g_lab->_resource->readViews(roomNum);

	ViewData *view = g_lab->_rooms[roomNum]._view[direction];

	do {
		if (checkConditions(view->_condition))
			break;

		view = view->_nextCondition;
	} while (true);

	return view;
}

/*****************************************************************************/
/* Gets an object, if any, from the user's click on the screen.              */
/*****************************************************************************/
static CloseData *getObject(uint16 x, uint16 y, CloseDataPtr lcptr) {
	if (lcptr == NULL) {
		lcptr = getViewData(g_lab->_roomNum, g_lab->_direction)->_closeUps;
	} else {
		lcptr = lcptr->_subCloseUps;
	}

	while (lcptr != NULL) {
		if ((x >= g_lab->_utils->scaleX(lcptr->x1)) && (y >= g_lab->_utils->scaleY(lcptr->y1)) &&
			  (x <= g_lab->_utils->scaleX(lcptr->x2)) && (y <= g_lab->_utils->scaleY(lcptr->y2)))
			return lcptr;

		lcptr = lcptr->_nextCloseUp;
	}

	return NULL;
}

/*****************************************************************************/
/* Goes through the list of closeups to find a match.                        */
/* NYI: Known bug here.  If there are two objects that have closeups, and    */
/*      some of the closeups have the same hit boxes, then this returns the  */
/*      first occurence of the object with the same hit box.                 */
/*****************************************************************************/
static CloseDataPtr findCPtrMatch(CloseDataPtr cpmain, CloseDataPtr list) {
	CloseDataPtr cptr;

	while (list) {
		if ((cpmain->x1 == list->x1) && (cpmain->x2 == list->x2) &&
			  (cpmain->y1 == list->y1) && (cpmain->y2 == list->y2) &&
			  (cpmain->_depth == list->_depth))
			return list;

		cptr = findCPtrMatch(cpmain, list->_subCloseUps);

		if (cptr)
			return cptr;
		else
			list = list->_nextCloseUp;
	}

	return NULL;
}

/*****************************************************************************/
/* Returns the current picture name.                                         */
/*****************************************************************************/
char *LabEngine::getPictName(CloseDataPtr *lcptr) {
	ViewData *viewPtr = getViewData(g_lab->_roomNum, g_lab->_direction);

	if (*lcptr != NULL) {
		*lcptr = findCPtrMatch(*lcptr, viewPtr->_closeUps);

		if (*lcptr)
			return (*lcptr)->_graphicName;
	}

	return viewPtr->_graphicName;
}

/*****************************************************************************/
/* Draws the current direction to the screen.                                */
/*****************************************************************************/
void LabEngine::drawDirection(CloseDataPtr lcptr) {
	if (lcptr != NULL && lcptr->_message) {
		_graphics->drawMessage(lcptr->_message);
		return;
	}

	Common::String message;

	if (_rooms[_roomNum]._roomMsg) {
		message += _rooms[_roomNum]._roomMsg;
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

/*****************************************************************************/
/* process a arrow gadget movement.                                          */
/*****************************************************************************/
uint16 processArrow(uint16 curDirection, uint16 arrow) {
	if (arrow == 1) { // Forward
		uint16 room = 1;

		if (curDirection == NORTH)
			room = g_lab->_rooms[g_lab->_roomNum]._northDoor;
		else if (curDirection == SOUTH)
			room = g_lab->_rooms[g_lab->_roomNum]._southDoor;
		else if (curDirection == EAST)
			room = g_lab->_rooms[g_lab->_roomNum]._eastDoor;
		else if (curDirection == WEST)
			room = g_lab->_rooms[g_lab->_roomNum]._westDoor;

		if (room != 0)
			g_lab->_roomNum = room;

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

/*****************************************************************************/
/* Sets the current close up data.                                           */
/*****************************************************************************/
void setCurClose(Common::Point pos, CloseDataPtr *cptr, bool useAbsoluteCoords) {
	CloseDataPtr lcptr;
	uint16 x1, y1, x2, y2;

	if (*cptr == NULL) {
		lcptr = getViewData(g_lab->_roomNum, g_lab->_direction)->_closeUps;
	} else
		lcptr = (*cptr)->_subCloseUps;

	while (lcptr != NULL) {
		if (!useAbsoluteCoords) {
			x1 = lcptr->x1;
			y1 = lcptr->y1;
			x2 = lcptr->x2;
			y2 = lcptr->y2;
		} else {
			x1 = g_lab->_utils->scaleX(lcptr->x1);
			y1 = g_lab->_utils->scaleY(lcptr->y1);
			x2 = g_lab->_utils->scaleX(lcptr->x2);
			y2 = g_lab->_utils->scaleY(lcptr->y2);
		}

		if (pos.x >= x1 && pos.y >= y1 && pos.x <= x2 && pos.y <= y2 && lcptr->_graphicName) {
			*cptr = lcptr;
			return;
		}

		lcptr = lcptr->_nextCloseUp;
	}
}

/*****************************************************************************/
/* Takes the currently selected item.                                        */
/*****************************************************************************/
bool takeItem(uint16 x, uint16 y, CloseDataPtr *cptr) {
	CloseDataPtr lcptr;

	if (*cptr == NULL) {
		lcptr = getViewData(g_lab->_roomNum, g_lab->_direction)->_closeUps;
	} else if ((*cptr)->_closeUpType < 0) {
		g_lab->_conditions->inclElement(abs((*cptr)->_closeUpType));
		return true;
	} else
		lcptr = (*cptr)->_subCloseUps;


	while (lcptr != NULL) {
		if ((x >= g_lab->_utils->scaleX(lcptr->x1)) && (y >= g_lab->_utils->scaleY(lcptr->y1)) &&
			  (x <= g_lab->_utils->scaleX(lcptr->x2)) && (y <= g_lab->_utils->scaleY(lcptr->y2)) &&
			  (lcptr->_closeUpType < 0)) {
			g_lab->_conditions->inclElement(abs(lcptr->_closeUpType));
			return true;
		}

		lcptr = lcptr->_nextCloseUp;
	}

	return false;
}

/*****************************************************************************/
/* Processes the action list.                                                */
/*****************************************************************************/
void LabEngine::doActions(Action *aptr, CloseDataPtr *lcptr) {
	while (aptr) {
		_music->updateMusic();

		switch (aptr->_actionType) {
		case PLAYSOUND:
			_music->_loopSoundEffect = false;
			_music->_waitTillFinished = true;
			_music->readMusic((char *)aptr->_data, true);
			_music->_waitTillFinished = false;
			break;

		case PLAYSOUNDB:
			_music->_loopSoundEffect = false;
			_music->_waitTillFinished = false;
			_music->readMusic((char *)aptr->_data, false);
			break;

		case PLAYSOUNDCONT:
			_music->_doNotFilestopSoundEffect = true;
			_music->_loopSoundEffect = true;
			_music->readMusic((char *)aptr->_data, _music->_waitTillFinished);
			break;

		case SHOWDIFF:
			_graphics->readPict((char *)aptr->_data, true);
			break;

		case SHOWDIFFCONT:
			_graphics->readPict((char *)aptr->_data, false);
			break;

		case LOADDIFF:
			if (aptr->_data)
				_graphics->loadPict((char *)aptr->_data);          /* Puts a file into memory */

			break;

		case TRANSITION:
			_graphics->doTransition((TransitionType)aptr->_param1, lcptr, (char *)aptr->_data);
			break;

		case NOUPDATE:
			_noUpdateDiff = true;
			_anim->_doBlack = false;
			break;

		case FORCEUPDATE:
			_curFileName = " ";
			break;

		case SHOWCURPICT: {
				char *test = getPictName(lcptr);

				if (strcmp(test, _curFileName) != 0) {
					_curFileName = test;
					_graphics->readPict(_curFileName, true);
				}
			}
			break;

		case SETELEMENT:
			_conditions->inclElement(aptr->_param1);
			break;

		case UNSETELEMENT:
			_conditions->exclElement(aptr->_param1);
			break;

		case SHOWMESSAGE:
			_graphics->_doNotDrawMessage = false;

			if (_graphics->_longWinInFront)
				_graphics->longDrawMessage((char *)aptr->_data);
			else
				_graphics->drawMessage((char *)aptr->_data);

			_graphics->_doNotDrawMessage = true;
			break;

		case CSHOWMESSAGE:
			if (*lcptr == NULL) {
				_graphics->_doNotDrawMessage = false;
				_graphics->drawMessage((char *)aptr->_data);
				_graphics->_doNotDrawMessage = true;
			}

			break;

		case SHOWMESSAGES: {
				char **str = (char **)aptr->_data;
				_graphics->_doNotDrawMessage = false;
				_graphics->drawMessage(str[getRandom(aptr->_param1)]);
				_graphics->_doNotDrawMessage = true;
			}
			break;

		case SETPOSITION:
			if (aptr->_param1 & 0x8000) {
				// This is a Wyrmkeep Windows trial version, thus stop at this
				// point, since we can't check for game payment status
				_graphics->readPict(getPictName(lcptr), true);
				aptr = NULL;
				GUI::MessageDialog trialMessage("This is the end of the trial version. You can play the full game using the original interpreter from Wyrmkeep");
				trialMessage.runModal();
				continue;
			}

			_roomNum   = aptr->_param1;
			_direction = aptr->_param2 - 1;
			*lcptr      = NULL;
			_anim->_doBlack = true;
			break;

		case SETCLOSEUP: {
				CloseDataPtr tlcptr = getObject(g_lab->_utils->scaleX(aptr->_param1), g_lab->_utils->scaleY(aptr->_param2), *lcptr);

				if (tlcptr)
					*lcptr = tlcptr;
			}
			break;

		case MAINVIEW:
			*lcptr = NULL;
			break;

		case SUBINV:
			if (_inventory[aptr->_param1]._many)
				(_inventory[aptr->_param1]._many)--;

			if (_inventory[aptr->_param1]._many == 0)
				_conditions->exclElement(aptr->_param1);

			break;

		case ADDINV:
			(_inventory[aptr->_param1]._many) += aptr->_param2;
			_conditions->inclElement(aptr->_param1);
			break;

		case SHOWDIR:
			_graphics->_doNotDrawMessage = false;
			break;

		case WAITSECS: {
				uint32 startSecs, startMicros, curSecs, curMicros;

				addCurTime(aptr->_param1, 0, &startSecs, &startMicros);

				_graphics->screenUpdate();

				while (1) {
					_music->updateMusic();
					_anim->diffNextFrame();
					getTime(&curSecs, &curMicros);

					if ((curSecs > startSecs) || ((curSecs == startSecs) && (curMicros >= startMicros)))
						break;
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
			_music->changeMusic((const char *)aptr->_data);
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
			_music->_winmusic = true;
			_music->freeMusic();
			_music->initMusic();
			break;

		case WINGAME:
			_quitLab = true;
			break;

		case LOSTGAME:
			_quitLab = true;
			break;

		case RESETBUFFER:
			g_lab->_graphics->freePict();
			break;

		case SPECIALCMD:
			if (aptr->_param1 == 0)
				_anim->_doBlack = true;
			else if (aptr->_param1 == 1)
				_anim->_doBlack = (_cptr == NULL);
			else if (aptr->_param1 == 2)
				_anim->_doBlack = (_cptr != NULL);
			else if (aptr->_param1 == 5) { /* inverse the palette */
				for (uint16 idx = (8 * 3); idx < (255 * 3); idx++)
					_anim->_diffPalette[idx] = 255 - _anim->_diffPalette[idx];

				waitTOF();
				_graphics->setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (aptr->_param1 == 4) { /* white the palette */
				_graphics->whiteScreen();
				waitTOF();
				waitTOF();
			} else if (aptr->_param1 == 6) { /* Restore the palette */
				waitTOF();
				_graphics->setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (aptr->_param1 == 7) { /* Quick pause */
				waitTOF();
				waitTOF();
				waitTOF();
			}

			break;
		}

		aptr = aptr->_nextAction;
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

	_music->_doNotFilestopSoundEffect = false;
}

/*****************************************************************************/
/* Does the work for doActionRule.                                           */
/*****************************************************************************/
static bool doActionRuleSub(int16 action, int16 roomNum, CloseDataPtr lcptr, CloseDataPtr *set, bool allowDefaults) {
	action++;

	if (lcptr) {
		RuleList *rules = g_lab->_rooms[g_lab->_roomNum]._rules;

		if ((rules == NULL) && (roomNum == 0)) {
			g_lab->_resource->readViews(roomNum);
			rules = g_lab->_rooms[roomNum]._rules;
		}

		for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
			if (((*rule)->_ruleType == ACTION) &&
				(((*rule)->_param1 == action) || (((*rule)->_param1 == 0) && allowDefaults))) {
				if ((((*rule)->_param2 == lcptr->_closeUpType) ||
					  (((*rule)->_param2 == 0) && allowDefaults)) ||
					  ((action == 1) && ((*rule)->_param2 == (-lcptr->_closeUpType)))) {
					if (checkConditions((*rule)->_condition)) {
						g_lab->doActions((*rule)->_actionList, set);
						return true;
					}
				}
			}
		}
	}

	return false;
}

/*****************************************************************************/
/* Goes through the rules if an action is taken.                             */
/*****************************************************************************/
bool doActionRule(Common::Point pos, int16 action, int16 roomNum, CloseDataPtr *lcptr) {
	if (roomNum)
		g_lab->_newFileName = NOFILE;
	else
		g_lab->_newFileName = g_lab->_curFileName;

	CloseDataPtr tlcptr = getObject(pos.x, pos.y, *lcptr);

	if (doActionRuleSub(action, roomNum, tlcptr, lcptr, false))
		return true;
	else if (doActionRuleSub(action, roomNum, *lcptr, lcptr, false))
		return true;
	else if (doActionRuleSub(action, roomNum, tlcptr, lcptr, true))
		return true;
	else if (doActionRuleSub(action, roomNum, *lcptr, lcptr, true))
		return true;

	return false;
}

/*****************************************************************************/
/* Does the work for doActionRule.                                           */
/*****************************************************************************/
static bool doOperateRuleSub(int16 itemNum, int16 roomNum, CloseDataPtr lcptr, CloseDataPtr *set, bool allowDefaults) {
	if (lcptr)
		if (lcptr->_closeUpType > 0) {
			RuleList *rules = g_lab->_rooms[roomNum]._rules;

			if ((rules == NULL) && (roomNum == 0)) {
				g_lab->_resource->readViews(roomNum);
				rules = g_lab->_rooms[roomNum]._rules;
			}

			for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
				if (((*rule)->_ruleType == OPERATE) &&
					  (((*rule)->_param1 == itemNum) || (((*rule)->_param1 == 0) && allowDefaults)) &&
						(((*rule)->_param2 == lcptr->_closeUpType) || (((*rule)->_param2 == 0) && allowDefaults))) {
					if (checkConditions((*rule)->_condition)) {
						g_lab->doActions((*rule)->_actionList, set);
						return true;
					}
				}
			}
		}

	return false;
}

/*****************************************************************************/
/* Goes through the rules if the user tries to operate an item on an object. */
/*****************************************************************************/
bool doOperateRule(int16 x, int16 y, int16 ItemNum, CloseDataPtr *lcptr) {
	CloseDataPtr tlcptr;

	g_lab->_newFileName = NOFILE;

	tlcptr = getObject(x, y, *lcptr);

	if (doOperateRuleSub(ItemNum, g_lab->_roomNum, tlcptr, lcptr, false))
		return true;
	else if (doOperateRuleSub(ItemNum, g_lab->_roomNum, *lcptr, lcptr, false))
		return true;
	else if (doOperateRuleSub(ItemNum, g_lab->_roomNum, tlcptr, lcptr, true))
		return true;
	else if (doOperateRuleSub(ItemNum, g_lab->_roomNum, *lcptr, lcptr, true))
		return true;
	else {
		g_lab->_newFileName = g_lab->_curFileName;

		if (doOperateRuleSub(ItemNum, 0, tlcptr, lcptr, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, *lcptr, lcptr, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, tlcptr, lcptr, true))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, *lcptr, lcptr, true))
			return true;
	}

	return false;
}

/*****************************************************************************/
/* Goes through the rules if the user tries to go forward.                   */
/*****************************************************************************/
bool doGoForward(CloseDataPtr *lcptr) {
	RuleList *rules = g_lab->_rooms[g_lab->_roomNum]._rules;

	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if (((*rule)->_ruleType == GOFORWARD) && ((*rule)->_param1 == (g_lab->_direction + 1))) {
			if (checkConditions((*rule)->_condition)) {
				g_lab->doActions((*rule)->_actionList, lcptr);
				return true;
			}
		}
	}

	return false;
}

/*****************************************************************************/
/* Goes through the rules if the user tries to turn.                         */
/*****************************************************************************/
bool doTurn(uint16 from, uint16 to, CloseDataPtr *lcptr) {
	from++;
	to++;

	RuleList *rules = g_lab->_rooms[g_lab->_roomNum]._rules;

	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if (((*rule)->_ruleType == TURN) ||
			  (((*rule)->_ruleType == TURNFROMTO) &&
			  ((*rule)->_param1 == from) && ((*rule)->_param2 == to))) {
			if (checkConditions((*rule)->_condition)) {
				g_lab->doActions((*rule)->_actionList, lcptr);
				return true;
			}
		}
	}

	return false;
}

/*****************************************************************************/
/* Goes through the rules if the user tries to go to the main view           */
/*****************************************************************************/
bool doMainView(CloseDataPtr *lcptr) {
	RuleList *rules = g_lab->_rooms[g_lab->_roomNum]._rules;
	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if ((*rule)->_ruleType == GOMAINVIEW) {
			if (checkConditions((*rule)->_condition)) {
				g_lab->doActions((*rule)->_actionList, lcptr);
				return true;
			}
		}
	}

	return false;
}

} // End of namespace Lab
