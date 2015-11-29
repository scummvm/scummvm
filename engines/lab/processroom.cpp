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
#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/parsetypes.h"
#include "lab/parsefun.h"
#include "lab/resource.h"
#include "lab/diff.h"
#include "lab/interface.h"

namespace Lab {

/* Global parser data */

#define NOFILE         "no file"

RoomData *_rooms;
InventoryData *Inventory;
uint16 NumInv, RoomNum, ManyRooms, HighestCondition, Direction;
const char *NewFileName;

extern bool DoNotDrawMessage, IsBM, noupdatediff, QuitLab, MusicOn, DoBlack, LongWinInFront;
extern char diffcmap[256 * 3];
extern const char *CurFileName;
extern CloseDataPtr CPtr;

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
		g_resource->readViews(roomNum);

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
static CloseData *getObject(uint16 x, uint16 y, CloseDataPtr LCPtr) {
	ViewData *VPtr;

	if (LCPtr == NULL) {
		VPtr = getViewData(RoomNum, Direction);
		LCPtr = VPtr->closeUps;
	}

	else
		LCPtr = LCPtr->SubCloseUps;


	while (LCPtr != NULL) {
		if ((x >= scaleX(LCPtr->x1)) && (y >= scaleY(LCPtr->y1)) &&
		        (x <= scaleX(LCPtr->x2)) && (y <= scaleY(LCPtr->y2)))
			return LCPtr;

		LCPtr = LCPtr->NextCloseUp;
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
char *getPictName(CloseDataPtr *LCPtr) {
	ViewData *ViewPtr = getViewData(RoomNum, Direction);

	if (*LCPtr != NULL) {
		*LCPtr = findCPtrMatch(*LCPtr, ViewPtr->closeUps);

		if (*LCPtr)
			return (*LCPtr)->GraphicName;
	}

	return ViewPtr->GraphicName;
}

/*****************************************************************************/
/* Draws the current direction to the screen.                                */
/*****************************************************************************/
void LabEngine::drawDirection(CloseDataPtr LCPtr) {
	if (LCPtr != NULL && LCPtr->Message) {
		drawMessage(LCPtr->Message);
		return;
	}

	Common::String message;

	if (_rooms[RoomNum]._roomMsg) {
		message += _rooms[RoomNum]._roomMsg;
		message += ", ";
	}

	if (Direction == NORTH)
		message += g_resource->getStaticText(kTextFacingNorth);
	else if (Direction == EAST)
		message += g_resource->getStaticText(kTextFacingEast);
	else if (Direction == SOUTH)
		message += g_resource->getStaticText(kTextFacingSouth);
	else if (Direction == WEST)
		message += g_resource->getStaticText(kTextFacingWest);

	drawMessage(message.c_str());
}

/*****************************************************************************/
/* process a arrow gadget movement.                                          */
/*****************************************************************************/
bool processArrow(uint16 *direction, uint16 Arrow) {
	uint16 room = 1;

	if (Arrow == 1) { /* Forward */
		if (*direction == NORTH)
			room = _rooms[RoomNum]._northDoor;
		else if (*direction == SOUTH)
			room = _rooms[RoomNum]._southDoor;
		else if (*direction == EAST)
			room = _rooms[RoomNum]._eastDoor;
		else if (*direction == WEST)
			room = _rooms[RoomNum]._westDoor;

		if (room == 0)
			return false;
		else
			RoomNum = room;
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
	CloseDataPtr LCPtr;
	uint16 x1, y1, x2, y2;

	if (*cptr == NULL) {
		VPtr = getViewData(RoomNum, Direction);
		LCPtr = VPtr->closeUps;
	} else
		LCPtr = (*cptr)->SubCloseUps;

	while (LCPtr != NULL) {
		if (!useAbsoluteCoords) {
			x1 = LCPtr->x1;
			y1 = LCPtr->y1;
			x2 = LCPtr->x2;
			y2 = LCPtr->y2;
		} else {
			x1 = scaleX(LCPtr->x1);
			y1 = scaleY(LCPtr->y1);
			x2 = scaleX(LCPtr->x2);
			y2 = scaleY(LCPtr->y2);
		}

		if (pos.x >= x1 && pos.y >= y1 && pos.x <= x2 && pos.y <= y2 && LCPtr->GraphicName) {
			*cptr = LCPtr;
			return;
		}

		LCPtr = LCPtr->NextCloseUp;
	}
}

/*****************************************************************************/
/* Takes the currently selected item.                                        */
/*****************************************************************************/
bool takeItem(uint16 x, uint16 y, CloseDataPtr *cptr) {
	ViewData *VPtr;
	CloseDataPtr LCPtr;

	if (*cptr == NULL) {
		VPtr = getViewData(RoomNum, Direction);
		LCPtr = VPtr->closeUps;
	} else if ((*cptr)->CloseUpType < 0) {
		g_lab->_conditions->inclElement(abs((*cptr)->CloseUpType));
		return true;
	} else
		LCPtr = (*cptr)->SubCloseUps;


	while (LCPtr != NULL) {
		if ((x >= scaleX(LCPtr->x1)) && (y >= scaleY(LCPtr->y1)) &&
		        (x <= scaleX(LCPtr->x2)) && (y <= scaleY(LCPtr->y2)) &&
		        (LCPtr->CloseUpType < 0)) {
			g_lab->_conditions->inclElement(abs(LCPtr->CloseUpType));
			return true;
		}

		LCPtr = LCPtr->NextCloseUp;
	}

	return false;
}

/*****************************************************************************/
/* Processes the action list.                                                */
/*****************************************************************************/
static void doActions(Action * APtr, CloseDataPtr *LCPtr) {
	CloseDataPtr TLCPtr;
	bool FirstLoaded = true;
	char **str, *Test;
	uint32 StartSecs, StartMicros, CurSecs, CurMicros;

	while (APtr) {
		g_music->updateMusic();

		switch (APtr->ActionType) {
		case PLAYSOUND:
			g_music->_loopSoundEffect = false;
			g_music->_waitTillFinished = true;
			readMusic((char *)APtr->Data, true);
			g_music->_waitTillFinished = false;
			break;

		case PLAYSOUNDB:
			g_music->_loopSoundEffect = false;
			g_music->_waitTillFinished = false;
			readMusic((char *)APtr->Data, false);
			break;

		case PLAYSOUNDCONT:
			g_music->_doNotFilestopSoundEffect = true;
			g_music->_loopSoundEffect = true;
			readMusic((char *)APtr->Data, g_music->_waitTillFinished);
			break;

		case SHOWDIFF:
			readPict((char *)APtr->Data, true);
			break;

		case SHOWDIFFCONT:
			readPict((char *)APtr->Data, false);
			break;

		case LOADDIFF:
			if (FirstLoaded) {
				resetBuffer();
				FirstLoaded = false;
			}

			if (APtr->Data)
				g_music->newOpen((char *)APtr->Data);          /* Puts a file into memory */

			break;

		case WIPECMD:
			g_lab->doWipe(APtr->Param1, LCPtr, (char *)APtr->Data);
			break;

		case NOUPDATE:
			noupdatediff = true;
			DoBlack = false;
			break;

		case FORCEUPDATE:
			CurFileName = " ";
			break;

		case SHOWCURPICT:
			Test = getPictName(LCPtr);

			if (strcmp(Test, CurFileName) != 0) {
				CurFileName = Test;
				readPict(CurFileName, true);
			}

			break;

		case SETELEMENT:
			g_lab->_conditions->inclElement(APtr->Param1);
			break;

		case UNSETELEMENT:
			g_lab->_conditions->exclElement(APtr->Param1);
			break;

		case SHOWMESSAGE:
			DoNotDrawMessage = false;

			if (LongWinInFront)
				g_lab->longDrawMessage((char *)APtr->Data);
			else
				g_lab->drawMessage((char *)APtr->Data);

			DoNotDrawMessage = true;
			break;

		case CSHOWMESSAGE:
			if (*LCPtr == NULL) {
				DoNotDrawMessage = false;
				g_lab->drawMessage((char *)APtr->Data);
				DoNotDrawMessage = true;
			}

			break;

		case SHOWMESSAGES:
			str = (char **)APtr->Data;
			DoNotDrawMessage = false;
			g_lab->drawMessage(str[getRandom(APtr->Param1)]);
			DoNotDrawMessage = true;
			break;

		case SETPOSITION:
			if (APtr->Param1 & 0x8000) {
				// This is a Wyrmkeep Windows trial version, thus stop at this
				// point, since we can't check for game payment status
				readPict(getPictName(LCPtr), true);
				APtr = NULL;
				GUI::MessageDialog trialMessage("This is the end of the trial version. You can play the full game using the original interpreter from Wyrmkeep");
				trialMessage.runModal();
				continue;
			}

			RoomNum   = APtr->Param1;
			Direction = APtr->Param2 - 1;
			*LCPtr      = NULL;
			DoBlack    = true;
			break;

		case SETCLOSEUP:
			TLCPtr = getObject(scaleX(APtr->Param1), scaleY(APtr->Param2), *LCPtr);

			if (TLCPtr)
				*LCPtr = TLCPtr;

			break;

		case MAINVIEW:
			*LCPtr = NULL;
			break;

		case SUBINV:
			if (Inventory[APtr->Param1].Many)
				(Inventory[APtr->Param1].Many)--;

			if (Inventory[APtr->Param1].Many == 0)
				g_lab->_conditions->exclElement(APtr->Param1);

			break;

		case ADDINV:
			(Inventory[APtr->Param1].Many) += APtr->Param2;
			g_lab->_conditions->inclElement(APtr->Param1);
			break;

		case SHOWDIR:
			DoNotDrawMessage = false;
			break;

		case WAITSECS:
			g_lab->addCurTime(APtr->Param1, 0, &StartSecs, &StartMicros);

			g_lab->WSDL_UpdateScreen();

			while (1) {
				g_music->updateMusic();
				g_lab->diffNextFrame();
				g_lab->getTime(&CurSecs, &CurMicros);

				if ((CurSecs > StartSecs) || ((CurSecs == StartSecs) &&
				                              (CurMicros >= StartMicros)))
					break;
			}

			break;

		case STOPMUSIC:
			g_music->setMusic(false);
			break;

		case STARTMUSIC:
			g_music->setMusic(true);
			break;

		case CHANGEMUSIC:
			g_music->changeMusic((const char *)APtr->Data);
			g_music->setMusicReset(false);
			break;

		case RESETMUSIC:
			g_music->resetMusic();
			g_music->setMusicReset(true);
			break;

		case FILLMUSIC:
			g_music->updateMusic();
			break;

		case WAITSOUND:
			while (g_music->isSoundEffectActive()) {
				g_music->updateMusic();
				g_lab->diffNextFrame();
				g_lab->waitTOF();
			}

			break;

		case CLEARSOUND:
			if (g_music->_loopSoundEffect) {
				g_music->_loopSoundEffect = false;
				g_music->stopSoundEffect();
			} else if (g_music->isSoundEffectActive())
				g_music->stopSoundEffect();

			break;

		case WINMUSIC:
			g_music->_winmusic = true;
			g_music->freeMusic();
			g_music->initMusic();
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
			if (APtr->Param1 == 0)
				DoBlack = true;
			else if (APtr->Param1 == 1)
				DoBlack = (CPtr == NULL);
			else if (APtr->Param1 == 2)
				DoBlack = (CPtr != NULL);
			else if (APtr->Param1 == 5) { /* inverse the palette */
				for (uint16 idx = (8 * 3); idx < (255 * 3); idx++)
					diffcmap[idx] = 255 - diffcmap[idx];

				g_lab->waitTOF();
				g_lab->VGASetPal(diffcmap, 256);
				g_lab->waitTOF();
				g_lab->waitTOF();
			} else if (APtr->Param1 == 4) { /* white the palette */
				whiteScreen();
				g_lab->waitTOF();
				g_lab->waitTOF();
			} else if (APtr->Param1 == 6) { /* Restore the palette */
				g_lab->waitTOF();
				g_lab->VGASetPal(diffcmap, 256);
				g_lab->waitTOF();
				g_lab->waitTOF();
			} else if (APtr->Param1 == 7) { /* Quick pause */
				g_lab->waitTOF();
				g_lab->waitTOF();
				g_lab->waitTOF();
			}

			break;
		}

		APtr = APtr->NextAction;
	}

	if (g_music->_loopSoundEffect) {
		g_music->_loopSoundEffect = false;
		g_music->stopSoundEffect();
	} else {
		while (g_music->isSoundEffectActive()) {
			g_music->updateMusic();
			g_lab->diffNextFrame();
			g_lab->waitTOF();
		}
	}

	g_music->_doNotFilestopSoundEffect = false;
}

/*****************************************************************************/
/* Does the work for doActionRule.                                           */
/*****************************************************************************/
static bool doActionRuleSub(int16 action, int16 roomNum, CloseDataPtr LCPtr, CloseDataPtr *Set, bool AllowDefaults) {
	action++;

	if (LCPtr) {
		RuleList *rules = _rooms[RoomNum]._rules;

		if ((rules == NULL) && (roomNum == 0)) {
			g_resource->readViews(roomNum);
			rules = _rooms[roomNum]._rules;
		}

		for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
			if (((*rule)->RuleType == ACTION) &&
				(((*rule)->Param1 == action) || (((*rule)->Param1 == 0) && AllowDefaults))) {
				if ((((*rule)->Param2 == LCPtr->CloseUpType) ||
					(((*rule)->Param2 == 0) && AllowDefaults))
				        ||
						((action == 1) && ((*rule)->Param2 == (-LCPtr->CloseUpType)))) {
					if (checkConditions((*rule)->Condition)) {
						doActions((*rule)->ActionList, Set);
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
bool doActionRule(Common::Point pos, int16 action, int16 roomNum, CloseDataPtr *LCPtr) {
	CloseDataPtr TLCPtr;

	if (roomNum)
		NewFileName = NOFILE;
	else
		NewFileName = CurFileName;

	TLCPtr = getObject(pos.x, pos.y, *LCPtr);

	if (doActionRuleSub(action, roomNum, TLCPtr, LCPtr, false))
		return true;
	else if (doActionRuleSub(action, roomNum, *LCPtr, LCPtr, false))
		return true;
	else if (doActionRuleSub(action, roomNum, TLCPtr, LCPtr, true))
		return true;
	else if (doActionRuleSub(action, roomNum, *LCPtr, LCPtr, true))
		return true;

	return false;
}

/*****************************************************************************/
/* Does the work for doActionRule.                                           */
/*****************************************************************************/
static bool doOperateRuleSub(int16 ItemNum, int16 roomNum, CloseDataPtr LCPtr, CloseDataPtr *Set, bool AllowDefaults) {
	if (LCPtr)
		if (LCPtr->CloseUpType > 0) {
			RuleList *rules = _rooms[roomNum]._rules;

			if ((rules == NULL) && (roomNum == 0)) {
				g_resource->readViews(roomNum);
				rules = _rooms[roomNum]._rules;
			}

			for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
				if (((*rule)->RuleType == OPERATE) &&
				        (((*rule)->Param1 == ItemNum) || (((*rule)->Param1 == 0) && AllowDefaults)) &&
						(((*rule)->Param2 == LCPtr->CloseUpType) || (((*rule)->Param2 == 0) && AllowDefaults))) {
					if (checkConditions((*rule)->Condition)) {
						doActions((*rule)->ActionList, Set);
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
bool doOperateRule(int16 x, int16 y, int16 ItemNum, CloseDataPtr *LCPtr) {
	CloseDataPtr TLCPtr;

	NewFileName = NOFILE;

	TLCPtr = getObject(x, y, *LCPtr);

	if (doOperateRuleSub(ItemNum, RoomNum, TLCPtr, LCPtr, false))
		return true;
	else if (doOperateRuleSub(ItemNum, RoomNum, *LCPtr, LCPtr, false))
		return true;
	else if (doOperateRuleSub(ItemNum, RoomNum, TLCPtr, LCPtr, true))
		return true;
	else if (doOperateRuleSub(ItemNum, RoomNum, *LCPtr, LCPtr, true))
		return true;
	else {
		NewFileName = CurFileName;

		if (doOperateRuleSub(ItemNum, 0, TLCPtr, LCPtr, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, *LCPtr, LCPtr, false))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, TLCPtr, LCPtr, true))
			return true;
		else if (doOperateRuleSub(ItemNum, 0, *LCPtr, LCPtr, true))
			return true;
	}

	return false;
}

/*****************************************************************************/
/* Goes thru the rules if the user tries to go forward.                      */
/*****************************************************************************/
bool doGoForward(CloseDataPtr *LCPtr) {
	RuleList *rules = _rooms[RoomNum]._rules;

	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if (((*rule)->RuleType == GOFORWARD) && ((*rule)->Param1 == (Direction + 1))) {
			if (checkConditions((*rule)->Condition)) {
				doActions((*rule)->ActionList, LCPtr);
				return true;
			}
		}
	}

	return false;
}

/*****************************************************************************/
/* Goes thru the rules if the user tries to turn.                            */
/*****************************************************************************/
bool doTurn(uint16 from, uint16 to, CloseDataPtr *LCPtr) {
	from++;
	to++;

	RuleList *rules = _rooms[RoomNum]._rules;

	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if (((*rule)->RuleType == TURN) ||
		        (((*rule)->RuleType == TURNFROMTO) &&
		         ((*rule)->Param1   == from) && ((*rule)->Param2 == to))) {
			if (checkConditions((*rule)->Condition)) {
				doActions((*rule)->ActionList, LCPtr);
				return true;
			}
		}
	}

	return false;
}

/*****************************************************************************/
/* Goes thru the rules if the user tries to go to the main view              */
/*****************************************************************************/
bool doMainView(CloseDataPtr *LCPtr) {
	RuleList *rules = _rooms[RoomNum]._rules;
	for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule) {
		if ((*rule)->RuleType == GOMAINVIEW) {
			if (checkConditions((*rule)->Condition)) {
				doActions((*rule)->ActionList, LCPtr);
				return true;
			}
		}
	}

	return false;
}

} // End of namespace Lab
