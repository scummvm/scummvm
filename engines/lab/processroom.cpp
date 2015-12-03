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
#include "lab/parsetypes.h"
#include "lab/parsefun.h"
#include "lab/resource.h"
#include "lab/anim.h"
#include "lab/interface.h"

namespace Lab {

/* Global parser data */

#define NOFILE         "no file"

RoomData *_rooms;
InventoryData *Inventory;
uint16 NumInv, ManyRooms, HighestCondition, Direction;

extern bool DoNotDrawMessage, noupdatediff, QuitLab, MusicOn;

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
static bool checkConditions(int16 *Condition) {
	int16 Counter;
	bool res;

	if (Condition == NULL)
		return true;

	if (Condition[0] == 0)
		return true;

	Counter = 1;
	res     = g_lab->_conditions->in(Condition[0]);

	while (Condition[Counter] && res) {
		res = g_lab->_conditions->in(Condition[Counter]);
		Counter++;
	}

	return res;
}

/*****************************************************************************/
/* Gets the current ViewDataPointer.                                         */
/*****************************************************************************/
ViewData *getViewData(uint16 roomNum, uint16 direction) {
	ViewData *view = NULL;

	if (!_rooms[roomNum]._roomMsg)
		g_lab->_resource->readViews(roomNum);

	if (direction == NORTH)
		view = _rooms[roomNum]._northView;
	else if (direction == SOUTH)
		view = _rooms[roomNum]._southView;
	else if (direction == EAST)
		view = _rooms[roomNum]._eastView;
	else if (direction == WEST)
		view = _rooms[roomNum]._westView;

	do {
		if (checkConditions(view->Condition))
			break;

		view = view->NextCondition;
	} while (true);

	return view;
}

/*****************************************************************************/
/* Gets an object, if any, from the user's click on the screen.              */
/*****************************************************************************/
static CloseData *getObject(uint16 x, uint16 y, CloseDataPtr lcptr) {
	ViewData *VPtr;

	if (lcptr == NULL) {
		VPtr = getViewData(g_lab->_roomNum, Direction);
		lcptr = VPtr->closeUps;
	}

	else
		lcptr = lcptr->SubCloseUps;


	while (lcptr != NULL) {
		if ((x >= scaleX(lcptr->x1)) && (y >= scaleY(lcptr->y1)) &&
		        (x <= scaleX(lcptr->x2)) && (y <= scaleY(lcptr->y2)))
			return lcptr;

		lcptr = lcptr->NextCloseUp;
	}

	return NULL;
}

/*****************************************************************************/
/* Goes through the list of closeups to find a match.                        */
/* NYI: Known bug here.  If there are two objects that have closeups, and    */
/*      some of the closeups have the same hit boxes, then this returns the  */
/*      first occurence of the object with the same hit box.                 */
/*****************************************************************************/
static CloseDataPtr findCPtrMatch(CloseDataPtr Main, CloseDataPtr List) {
	CloseDataPtr cptr;

	while (List) {
		if ((Main->x1 == List->x1) && (Main->x2 == List->x2) &&
		        (Main->y1 == List->y1) && (Main->y2 == List->y2) &&
		        (Main->depth == List->depth))
			return List;

		cptr = findCPtrMatch(Main, List->SubCloseUps);

		if (cptr)
			return cptr;
		else
			List = List->NextCloseUp;
	}

	return NULL;
}

/*****************************************************************************/
/* Returns the current picture name.                                         */
/*****************************************************************************/
char *getPictName(CloseDataPtr *lcptr) {
	ViewData *ViewPtr = getViewData(g_lab->_roomNum, Direction);

	if (*lcptr != NULL) {
		*lcptr = findCPtrMatch(*lcptr, ViewPtr->closeUps);

		if (*lcptr)
			return (*lcptr)->GraphicName;
	}

	return ViewPtr->GraphicName;
}

/*****************************************************************************/
/* Draws the current direction to the screen.                                */
/*****************************************************************************/
void LabEngine::drawDirection(CloseDataPtr lcptr) {
	if (lcptr != NULL && lcptr->Message) {
		drawMessage(lcptr->Message);
		return;
	}

	Common::String message;

	if (_rooms[_roomNum]._roomMsg) {
		message += _rooms[_roomNum]._roomMsg;
		message += ", ";
	}

	if (Direction == NORTH)
		message += _resource->getStaticText(kTextFacingNorth);
	else if (Direction == EAST)
		message += _resource->getStaticText(kTextFacingEast);
	else if (Direction == SOUTH)
		message += _resource->getStaticText(kTextFacingSouth);
	else if (Direction == WEST)
		message += _resource->getStaticText(kTextFacingWest);

	drawMessage(message.c_str());
}

/*****************************************************************************/
/* process a arrow gadget movement.                                          */
/*****************************************************************************/
bool processArrow(uint16 *direction, uint16 Arrow) {
	uint16 room = 1;

	if (Arrow == 1) { /* Forward */
		if (*direction == NORTH)
			room = _rooms[g_lab->_roomNum]._northDoor;
		else if (*direction == SOUTH)
			room = _rooms[g_lab->_roomNum]._southDoor;
		else if (*direction == EAST)
			room = _rooms[g_lab->_roomNum]._eastDoor;
		else if (*direction == WEST)
			room = _rooms[g_lab->_roomNum]._westDoor;

		if (room == 0)
			return false;
		else
			g_lab->_roomNum = room;
	} else if (Arrow == 0) { /* Left */
		if (*direction == NORTH)
			*direction = WEST;
		else if (*direction == WEST)
			*direction = SOUTH;
		else if (*direction == SOUTH)
			*direction = EAST;
		else
			*direction = NORTH;
	} else if (Arrow == 2) { /* Right */
		if (*direction == NORTH)
			*direction = EAST;
		else if (*direction == EAST)
			*direction = SOUTH;
		else if (*direction == SOUTH)
			*direction = WEST;
		else
			*direction = NORTH;
	}

	return true;
}

/*****************************************************************************/
/* Sets the current close up data.                                           */
/*****************************************************************************/
void setCurClose(Common::Point pos, CloseDataPtr *cptr, bool useAbsoluteCoords) {
	ViewData *VPtr;
	CloseDataPtr lcptr;
	uint16 x1, y1, x2, y2;

	if (*cptr == NULL) {
		VPtr = getViewData(g_lab->_roomNum, Direction);
		lcptr = VPtr->closeUps;
	} else
		lcptr = (*cptr)->SubCloseUps;

	while (lcptr != NULL) {
		if (!useAbsoluteCoords) {
			x1 = lcptr->x1;
			y1 = lcptr->y1;
			x2 = lcptr->x2;
			y2 = lcptr->y2;
		} else {
			x1 = scaleX(lcptr->x1);
			y1 = scaleY(lcptr->y1);
			x2 = scaleX(lcptr->x2);
			y2 = scaleY(lcptr->y2);
		}

		if (pos.x >= x1 && pos.y >= y1 && pos.x <= x2 && pos.y <= y2 && lcptr->GraphicName) {
			*cptr = lcptr;
			return;
		}

		lcptr = lcptr->NextCloseUp;
	}
}

/*****************************************************************************/
/* Takes the currently selected item.                                        */
/*****************************************************************************/
bool takeItem(uint16 x, uint16 y, CloseDataPtr *cptr) {
	ViewData *VPtr;
	CloseDataPtr lcptr;

	if (*cptr == NULL) {
		VPtr = getViewData(g_lab->_roomNum, Direction);
		lcptr = VPtr->closeUps;
	} else if ((*cptr)->CloseUpType < 0) {
		g_lab->_conditions->inclElement(abs((*cptr)->CloseUpType));
		return true;
	} else
		lcptr = (*cptr)->SubCloseUps;


	while (lcptr != NULL) {
		if ((x >= scaleX(lcptr->x1)) && (y >= scaleY(lcptr->y1)) &&
		        (x <= scaleX(lcptr->x2)) && (y <= scaleY(lcptr->y2)) &&
		        (lcptr->CloseUpType < 0)) {
			g_lab->_conditions->inclElement(abs(lcptr->CloseUpType));
			return true;
		}

		lcptr = lcptr->NextCloseUp;
	}

	return false;
}

/*****************************************************************************/
/* Processes the action list.                                                */
/*****************************************************************************/
void LabEngine::doActions(Action *aptr, CloseDataPtr *lcptr) {
	CloseDataPtr tlcptr;
	bool firstLoaded = true;
	char **str;
	uint32 startSecs, startMicros, curSecs, curMicros;

	while (aptr) {
		_music->updateMusic();

		switch (aptr->ActionType) {
		case PLAYSOUND:
			_music->_loopSoundEffect = false;
			_music->_waitTillFinished = true;
			readMusic((char *)aptr->Data, true);
			_music->_waitTillFinished = false;
			break;

		case PLAYSOUNDB:
			_music->_loopSoundEffect = false;
			_music->_waitTillFinished = false;
			readMusic((char *)aptr->Data, false);
			break;

		case PLAYSOUNDCONT:
			_music->_doNotFilestopSoundEffect = true;
			_music->_loopSoundEffect = true;
			readMusic((char *)aptr->Data, _music->_waitTillFinished);
			break;

		case SHOWDIFF:
			readPict((char *)aptr->Data, true);
			break;

		case SHOWDIFFCONT:
			readPict((char *)aptr->Data, false);
			break;

		case LOADDIFF:
			if (firstLoaded) {
				resetBuffer();
				firstLoaded = false;
			}

			if (aptr->Data)
				_music->newOpen((char *)aptr->Data);          /* Puts a file into memory */

			break;

		case WIPECMD:
			doWipe(aptr->Param1, lcptr, (char *)aptr->Data);
			break;

		case NOUPDATE:
			noupdatediff = true;
			_anim->_doBlack = false;
			break;

		case FORCEUPDATE:
			_curFileName = " ";
			break;

		case SHOWCURPICT: {
				char *test = getPictName(lcptr);

				if (strcmp(test, _curFileName) != 0) {
					_curFileName = test;
					readPict(_curFileName, true);
				}
			}
			break;

		case SETELEMENT:
			_conditions->inclElement(aptr->Param1);
			break;

		case UNSETELEMENT:
			_conditions->exclElement(aptr->Param1);
			break;

		case SHOWMESSAGE:
			DoNotDrawMessage = false;

			if (_longWinInFront)
				longDrawMessage((char *)aptr->Data);
			else
				drawMessage((char *)aptr->Data);

			DoNotDrawMessage = true;
			break;

		case CSHOWMESSAGE:
			if (*lcptr == NULL) {
				DoNotDrawMessage = false;
				drawMessage((char *)aptr->Data);
				DoNotDrawMessage = true;
			}

			break;

		case SHOWMESSAGES:
			str = (char **)aptr->Data;
			DoNotDrawMessage = false;
			drawMessage(str[getRandom(aptr->Param1)]);
			DoNotDrawMessage = true;
			break;

		case SETPOSITION:
			if (aptr->Param1 & 0x8000) {
				// This is a Wyrmkeep Windows trial version, thus stop at this
				// point, since we can't check for game payment status
				readPict(getPictName(lcptr), true);
				aptr = NULL;
				GUI::MessageDialog trialMessage("This is the end of the trial version. You can play the full game using the original interpreter from Wyrmkeep");
				trialMessage.runModal();
				continue;
			}

			_roomNum   = aptr->Param1;
			Direction = aptr->Param2 - 1;
			*lcptr      = NULL;
			_anim->_doBlack = true;
			break;

		case SETCLOSEUP:
			tlcptr = getObject(scaleX(aptr->Param1), scaleY(aptr->Param2), *lcptr);

			if (tlcptr)
				*lcptr = tlcptr;

			break;

		case MAINVIEW:
			*lcptr = NULL;
			break;

		case SUBINV:
			if (Inventory[aptr->Param1].Many)
				(Inventory[aptr->Param1].Many)--;

			if (Inventory[aptr->Param1].Many == 0)
				_conditions->exclElement(aptr->Param1);

			break;

		case ADDINV:
			(Inventory[aptr->Param1].Many) += aptr->Param2;
			_conditions->inclElement(aptr->Param1);
			break;

		case SHOWDIR:
			DoNotDrawMessage = false;
			break;

		case WAITSECS:
			addCurTime(aptr->Param1, 0, &startSecs, &startMicros);

			screenUpdate();

			while (1) {
				_music->updateMusic();
				_anim->diffNextFrame();
				getTime(&curSecs, &curMicros);

				if ((curSecs > startSecs) || ((curSecs == startSecs) &&
				                              (curMicros >= startMicros)))
					break;
			}

			break;

		case STOPMUSIC:
			_music->setMusic(false);
			break;

		case STARTMUSIC:
			_music->setMusic(true);
			break;

		case CHANGEMUSIC:
			_music->changeMusic((const char *)aptr->Data);
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
			QuitLab = true;
			break;

		case LOSTGAME:
			QuitLab = true;
			break;

		case RESETBUFFER:
			resetBuffer();
			break;

		case SPECIALCMD:
			if (aptr->Param1 == 0)
				_anim->_doBlack = true;
			else if (aptr->Param1 == 1)
				_anim->_doBlack = (_cptr == NULL);
			else if (aptr->Param1 == 2)
				_anim->_doBlack = (_cptr != NULL);
			else if (aptr->Param1 == 5) { /* inverse the palette */
				for (uint16 idx = (8 * 3); idx < (255 * 3); idx++)
					_anim->_diffPalette[idx] = 255 - _anim->_diffPalette[idx];

				waitTOF();
				setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (aptr->Param1 == 4) { /* white the palette */
				whiteScreen();
				waitTOF();
				waitTOF();
			} else if (aptr->Param1 == 6) { /* Restore the palette */
				waitTOF();
				setPalette(_anim->_diffPalette, 256);
				waitTOF();
				waitTOF();
			} else if (aptr->Param1 == 7) { /* Quick pause */
				waitTOF();
				waitTOF();
				waitTOF();
			}

			break;
		}

		aptr = aptr->NextAction;
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
static bool doActionRuleSub(int16 action, int16 roomNum, CloseDataPtr lcptr, CloseDataPtr *Set, bool AllowDefaults) {
	action++;

	if (lcptr) {
		RuleList *rules = _rooms[g_lab->_roomNum]._rules;

		if ((rules == NULL) && (roomNum == 0)) {
			g_lab->_resource->readViews(roomNum);
			rules = _rooms[roomNum]._rules;
		}

		for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
			if (((*rule)->RuleType == ACTION) &&
				(((*rule)->Param1 == action) || (((*rule)->Param1 == 0) && AllowDefaults))) {
				if ((((*rule)->Param2 == lcptr->CloseUpType) ||
					(((*rule)->Param2 == 0) && AllowDefaults))
				        ||
						((action == 1) && ((*rule)->Param2 == (-lcptr->CloseUpType)))) {
					if (checkConditions((*rule)->Condition)) {
						g_lab->doActions((*rule)->ActionList, Set);
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
	CloseDataPtr tlcptr;

	if (roomNum)
		g_lab->_newFileName = NOFILE;
	else
		g_lab->_newFileName = g_lab->_curFileName;

	tlcptr = getObject(pos.x, pos.y, *lcptr);

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
static bool doOperateRuleSub(int16 ItemNum, int16 roomNum, CloseDataPtr lcptr, CloseDataPtr *Set, bool AllowDefaults) {
	if (lcptr)
		if (lcptr->CloseUpType > 0) {
			RuleList *rules = _rooms[roomNum]._rules;

			if ((rules == NULL) && (roomNum == 0)) {
				g_lab->_resource->readViews(roomNum);
				rules = _rooms[roomNum]._rules;
			}

			for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
				if (((*rule)->RuleType == OPERATE) &&
				        (((*rule)->Param1 == ItemNum) || (((*rule)->Param1 == 0) && AllowDefaults)) &&
						(((*rule)->Param2 == lcptr->CloseUpType) || (((*rule)->Param2 == 0) && AllowDefaults))) {
					if (checkConditions((*rule)->Condition)) {
						g_lab->doActions((*rule)->ActionList, Set);
						return true;
					}
				}
			}
		}

	return false;
}

/*****************************************************************************/
/* Goes thru the rules if the user tries to operate an item on an object.    */
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
/* Goes thru the rules if the user tries to go forward.                      */
/*****************************************************************************/
bool doGoForward(CloseDataPtr *lcptr) {
	RuleList *rules = _rooms[g_lab->_roomNum]._rules;

	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if (((*rule)->RuleType == GOFORWARD) && ((*rule)->Param1 == (Direction + 1))) {
			if (checkConditions((*rule)->Condition)) {
				g_lab->doActions((*rule)->ActionList, lcptr);
				return true;
			}
		}
	}

	return false;
}

/*****************************************************************************/
/* Goes thru the rules if the user tries to turn.                            */
/*****************************************************************************/
bool doTurn(uint16 from, uint16 to, CloseDataPtr *lcptr) {
	from++;
	to++;

	RuleList *rules = _rooms[g_lab->_roomNum]._rules;

	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if (((*rule)->RuleType == TURN) ||
		        (((*rule)->RuleType == TURNFROMTO) &&
		         ((*rule)->Param1   == from) && ((*rule)->Param2 == to))) {
			if (checkConditions((*rule)->Condition)) {
				g_lab->doActions((*rule)->ActionList, lcptr);
				return true;
			}
		}
	}

	return false;
}

/*****************************************************************************/
/* Goes thru the rules if the user tries to go to the main view              */
/*****************************************************************************/
bool doMainView(CloseDataPtr *lcptr) {
	RuleList *rules = _rooms[g_lab->_roomNum]._rules;
	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if ((*rule)->RuleType == GOMAINVIEW) {
			if (checkConditions((*rule)->Condition)) {
				g_lab->doActions((*rule)->ActionList, lcptr);
				return true;
			}
		}
	}

	return false;
}

} // End of namespace Lab
