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

#include "lab/lab.h"
#include "lab/labfun.h"
#include "lab/anim.h"
#include "lab/image.h"
#include "lab/text.h"
#include "lab/intro.h"
#include "lab/parsefun.h"
#include "lab/interface.h"
#include "lab/resource.h"

namespace Lab {

extern bool stopsound, DoNotDrawMessage;

/* Global parser data */

extern RoomData *_rooms;
extern InventoryData *Inventory;
extern uint16 NumInv, ManyRooms, HighestCondition, Direction;

bool ispal = false, noupdatediff = false, MainDisplay = true, QuitLab = false;

#define BUFFERSIZE 850000L

/* LAB: Labyrinth specific code for the special puzzles */
#define SPECIALLOCK         100
#define SPECIALBRICK        101
#define SPECIALBRICKNOMOUSE 102

#define MAPNUM               28
#define JOURNALNUM            9
#define WESTPAPERNUM         18
#define NOTESNUM             12
#define WHISKEYNUM           25
#define PITHHELMETNUM         7
#define HELMETNUM             1

#define LAMPNUM              27
#define LAMPON              151

#define BELTNUM               3
#define BELTGLOW             70

#define USEDHELMET          184

#define QUARTERNUM           30


#define MUSEUMMONITOR        71
#define GRAMAPHONEMONITOR    72
#define UNICYCLEMONITOR      73
#define STATUEMONITOR        74
#define TALISMANMONITOR      75
#define LUTEMONITOR          76
#define CLOCKMONITOR         77
#define WINDOWMONITOR        78
#define BELTMONITOR          79
#define LIBRARYMONITOR       80
#define TERMINALMONITOR      81
#define LEVERSMONITOR        82

static char initcolors[] = { '\x00', '\x00', '\x00', '\x30',
							 '\x30', '\x30', '\x10', '\x10',
							 '\x10', '\x14', '\x14', '\x14',
							 '\x20', '\x20', '\x20', '\x24',
							 '\x24', '\x24', '\x2c', '\x2c',
							 '\x2c', '\x08', '\x08', '\x08'};

/******************************************************************************/
/* Draws the control panel display.                                           */
/******************************************************************************/
void LabEngine::drawPanel() {
	_event->mouseHide();

	setAPen(3);                 /* Clear Area */
	rectFill(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319), VGAScaleY(199));

	setAPen(0);                 /* First Line */
	drawHLine(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319));
	setAPen(5);                 /* Second Line */
	drawHLine(0, VGAScaleY(149) + 1 + SVGACord(2), VGAScaleX(319));

	/* Gadget Separators */
	setAPen(0);
	drawHLine(0, VGAScaleY(170), VGAScaleX(319));     /* First black line to separate buttons */

	if (!_alternate) {
		setAPen(4);
		drawHLine(0, VGAScaleY(170) + 1, VGAScaleX(319)); /* The horizontal lines under the black one */
		drawGadgetList(_moveGadgetList);
	} else {
		if (getPlatform() != Common::kPlatformWindows) {
			drawVLine(VGAScaleX(124), VGAScaleY(170) + 1, VGAScaleY(199)); /* Vertical Black lines */
			drawVLine(VGAScaleX(194), VGAScaleY(170) + 1, VGAScaleY(199));
		} else {
			drawVLine(VGAScaleX(90), VGAScaleY(170) + 1, VGAScaleY(199));  /* Vertical Black lines */
			drawVLine(VGAScaleX(160), VGAScaleY(170) + 1, VGAScaleY(199));
			drawVLine(VGAScaleX(230), VGAScaleY(170) + 1, VGAScaleY(199));
		}

		setAPen(4);
		drawHLine(0, VGAScaleY(170) + 1, VGAScaleX(122));   /* The horizontal lines under the black one */
		drawHLine(VGAScaleX(126), VGAScaleY(170) + 1, VGAScaleX(192));
		drawHLine(VGAScaleX(196), VGAScaleY(170) + 1, VGAScaleX(319));

		drawVLine(VGAScaleX(1), VGAScaleY(170) + 2, VGAScaleY(198)); /* The vertical high light lines */
		if (getPlatform() != Common::kPlatformWindows) {
			drawVLine(VGAScaleX(126), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(196), VGAScaleY(170) + 2, VGAScaleY(198));
		} else {
			drawVLine(VGAScaleX(92), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(162), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(232), VGAScaleY(170) + 2, VGAScaleY(198));
		}

		drawGadgetList(_invGadgetList);
	}

	_event->mouseShow();
}

/******************************************************************************/
/* Draws the message for the room.                                            */
/******************************************************************************/
void LabEngine::drawRoomMessage(uint16 curInv, CloseDataPtr closePtr) {
	if (_lastTooLong) {
		_lastTooLong = false;
		return;
	}

	if (_alternate) {
		if ((curInv <= NumInv) && _conditions->in(curInv) && Inventory[curInv].BInvName) {
			if ((curInv == LAMPNUM) && _conditions->in(LAMPON))  /* LAB: Labyrinth specific */
				drawStaticMessage(kTextLampOn);
			else if (Inventory[curInv].Many > 1) {
				Common::String roomMessage = Common::String(Inventory[curInv].name) + "  (" + Common::String::format("%d", Inventory[curInv].Many) + ")";
				drawMessage(roomMessage.c_str());
			} else
				drawMessage(Inventory[curInv].name);
		}
	} else
		drawDirection(closePtr);

	_lastTooLong = _lastMessageLong;
}

/******************************************************************************/
/* Sets up the Labyrinth screens, and opens up the initial windows.           */
/******************************************************************************/
bool LabEngine::setUpScreens() {
	if (!createScreen(_isHiRes))
		return false;

	Common::File *controlFile = g_lab->_resource->openDataFile("P:Control");
	for (uint16 i = 0; i < 20; i++)
		_moveImages[i] = new Image(controlFile);
	delete controlFile;

	/* Creates the gadgets for the movement control panel */
	uint16 y = VGAScaleY(173) - SVGACord(2);

	if (getPlatform() == Common::kPlatformWindows) {
		_moveGadgetList = createButton(1, y, 0, 't', _moveImages[0], _moveImages[1]);
		Gadget *curGadget = _moveGadgetList;
		curGadget->NextGadget = createButton(33, y, 1, 'm', _moveImages[2], _moveImages[3]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(65, y, 2, 'o', _moveImages[4], _moveImages[5]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(97, y, 3, 'c', _moveImages[6], _moveImages[7]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(129, y, 4, 'l', _moveImages[8], _moveImages[9]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(161, y, 5, 'i', _moveImages[12], _moveImages[13]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(193, y, 6, VKEY_LTARROW, _moveImages[14], _moveImages[15]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(225, y, 7, VKEY_UPARROW, _moveImages[16], _moveImages[17]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(257, y, 8, VKEY_RTARROW, _moveImages[18], _moveImages[19]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(289, y, 9, 'p', _moveImages[10], _moveImages[11]);
	} else {
		_moveGadgetList = createButton(1, y, 0, 0, _moveImages[0], _moveImages[1]);
		Gadget *curGadget = _moveGadgetList;
		curGadget->NextGadget = createButton(33, y, 1, 0, _moveImages[2], _moveImages[3]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(65, y, 2, 0, _moveImages[4], _moveImages[5]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(97, y, 3, 0, _moveImages[6], _moveImages[7]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(129, y, 4, 0, _moveImages[8], _moveImages[9]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(161, y, 5, 0, _moveImages[12], _moveImages[13]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(193, y, 6, 0, _moveImages[14], _moveImages[15]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(225, y, 7, 0, _moveImages[16], _moveImages[17]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(257, y, 8, 0, _moveImages[18], _moveImages[19]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(289, y, 9, 0, _moveImages[10], _moveImages[11]);
	}

	Common::File *invFile = g_lab->_resource->openDataFile("P:Inv");

	if (getPlatform() == Common::kPlatformWindows) {
		for (uint16 imgIdx = 0; imgIdx < 10; imgIdx++)
			_invImages[imgIdx] = new Image(invFile);

		_invGadgetList = createButton(24, y, 0, 'm', _invImages[0], _invImages[1]);
		Gadget *curGadget = _invGadgetList;
		curGadget->NextGadget = createButton(56, y, 1, 'g', _invImages[2], _invImages[3]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(94, y, 2, 'u', _invImages[4], _invImages[5]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(126, y, 3, 'l', _moveImages[8], _moveImages[9]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(164, y, 4, VKEY_LTARROW, _moveImages[14], _moveImages[15]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(196, y, 5, VKEY_RTARROW, _moveImages[18], _moveImages[19]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(234, y, 6, 'b', _invImages[6], _invImages[7]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(266, y, 7, 'f', _invImages[8], _invImages[9]);
		curGadget = curGadget->NextGadget;
	} else {
		for (uint16 imgIdx = 0; imgIdx < 6; imgIdx++)
			_invImages[imgIdx] = new Image(invFile);

		_invGadgetList = createButton(58, y, 0, 0, _invImages[0], _invImages[1]);
		Gadget *curGadget = _invGadgetList;
		curGadget->NextGadget = createButton(90, y, 1, 0, _invImages[2], _invImages[3]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(128, y, 2, 0, _invImages[4], _invImages[5]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(160, y, 3, 0, _moveImages[8], _moveImages[9]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(198, y, 4, 0, _moveImages[14], _moveImages[15]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(230, y, 5, 0, _moveImages[18], _moveImages[19]);
		curGadget = curGadget->NextGadget;
	}

	delete invFile;

	return true;
}

void LabEngine::freeScreens() {
	for (uint16 i = 0; i < 20; i++)
		delete _moveImages[i];

	if (getPlatform() == Common::kPlatformWindows) {
		for (uint16 imgIdx = 0; imgIdx < 10; imgIdx++)
			delete _invImages[imgIdx];
	} else {
		for (uint16 imgIdx = 0; imgIdx < 6; imgIdx++)
			delete _invImages[imgIdx];
	}
}


/******************************************************************************/
/* Permanently flips the imagery of a gadget.                                 */
/******************************************************************************/
void LabEngine::perFlipGadget(uint16 gadgetId) {
	Gadget *topGadget = _moveGadgetList;

	while (topGadget) {
		if (topGadget->GadgetID == gadgetId) {
			Image *tmpImage = topGadget->_image;
			topGadget->_image = topGadget->_altImage;
			topGadget->_altImage = tmpImage;

			if (!_alternate) {
				_event->mouseHide();
				topGadget->_image->drawImage(topGadget->x, topGadget->y);
				_event->mouseShow();
			}

			return;
		} else
			topGadget = topGadget->NextGadget;
	}
}

/******************************************************************************/
/* Eats all the available messages.                                           */
/******************************************************************************/
void LabEngine::eatMessages() {
	IntuiMessage *msg;

	do {
		msg = getMsg();
	} while (msg);

	return;
}

/******************************************************************************/
/* Checks whether the close up is one of the special case closeups.           */
/******************************************************************************/
bool LabEngine::doCloseUp(CloseDataPtr closePtr) {
	if (closePtr == NULL)
		return false;

	int monltmargin, monrtmargin, montopmargin, lutertmargin;

	if (getPlatform() != Common::kPlatformWindows) {
		monltmargin = 0;
		monrtmargin = 319;
		montopmargin = 0;
		lutertmargin = 124;
	} else {
		monltmargin = 2;
		monrtmargin = 317;
		montopmargin = 2;
		lutertmargin = 128;
	}

	switch (closePtr->CloseUpType) {
	case MUSEUMMONITOR:
	case LIBRARYMONITOR:
	case WINDOWMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, false, monltmargin, montopmargin, monrtmargin, 165);
		break;
	case GRAMAPHONEMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, false, monltmargin, montopmargin, 171, 165);
		break;
	case UNICYCLEMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, false, 100, montopmargin, monrtmargin, 165);
		break;
	case STATUEMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, false, 117, montopmargin, monrtmargin, 165);
		break;
	case TALISMANMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, false, monltmargin, montopmargin, 184, 165);
		break;
	case LUTEMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, false, monltmargin, montopmargin, lutertmargin, 165);
		break;
	case CLOCKMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, false, monltmargin, montopmargin, 206, 165);
		break;
	case TERMINALMONITOR:
		doMonitor(closePtr->GraphicName, closePtr->Message, true, monltmargin, montopmargin, monrtmargin, 165);
		break;
	default:
		return false;
	}

	_curFileName = " ";
	drawPanel();

	return true;
}

/******************************************************************************/
/* Gets the current inventory name.                                           */
/******************************************************************************/
const char *LabEngine::getInvName(uint16 CurInv) {
	if (MainDisplay)
		return Inventory[CurInv].BInvName;

	if ((CurInv == LAMPNUM) && _conditions->in(LAMPON))
		return "P:Mines/120";

	if ((CurInv == BELTNUM) && g_lab->_conditions->in(BELTGLOW))
		return "P:Future/BeltGlow";

	if (CurInv == WESTPAPERNUM) {
		g_lab->_curFileName = Inventory[CurInv].BInvName;
		g_lab->_anim->_noPalChange = true;
		readPict(g_lab->_curFileName, false);
		g_lab->_anim->_noPalChange = false;
		doWestPaper();
	} else if (CurInv == NOTESNUM) {
		g_lab->_curFileName = Inventory[CurInv].BInvName;
		g_lab->_anim->_noPalChange = true;
		readPict(g_lab->_curFileName, false);
		g_lab->_anim->_noPalChange = false;
		doNotes();
	}

	return Inventory[CurInv].BInvName;
}

/******************************************************************************/
/* Turns the interface off.                                                   */
/******************************************************************************/
void LabEngine::interfaceOff() {
	if (!_interfaceOff) {
		_event->attachGadgetList(NULL);
		_event->mouseHide();
		_interfaceOff = true;
	}
}

/******************************************************************************/
/* Turns the interface on.                                                    */
/******************************************************************************/
void LabEngine::interfaceOn() {
	if (_interfaceOff) {
		_interfaceOff = false;
		_event->mouseShow();
	}

	if (_longWinInFront)
		_event->attachGadgetList(NULL);
	else if (_alternate)
		_event->attachGadgetList(_invGadgetList);
	else
		_event->attachGadgetList(_moveGadgetList);
}

/******************************************************************************/
/* If the user hits the "Use" gadget; things that can get used on themselves. */
/******************************************************************************/
bool LabEngine::doUse(uint16 CurInv) {
	if (CurInv == MAPNUM) {                  /* LAB: Labyrinth specific */
		drawStaticMessage(kTextUseMap);
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";
		_cptr = NULL;
		doMap(_roomNum);
		setPalette(initcolors, 8);
		drawMessage(NULL);
		drawPanel();
	} else if (CurInv == JOURNALNUM) {         /* LAB: Labyrinth specific */
		drawStaticMessage(kTextUseJournal);
		interfaceOff();
		_anim->stopDiff();
		_curFileName = " ";
		_cptr = NULL;
		doJournal();
		drawPanel();
		drawMessage(NULL);
	} else if (CurInv == LAMPNUM) {            /* LAB: Labyrinth specific */
		interfaceOff();

		if (_conditions->in(LAMPON)) {
			drawStaticMessage(kTextTurnLampOff);
			_conditions->exclElement(LAMPON);
		} else {
			drawStaticMessage(kTextTurnLampOn);
			_conditions->inclElement(LAMPON);
		}

		_anim->_doBlack = false;
		_anim->_waitForEffect = true;
		readPict("Music:Click", true);
		_anim->_waitForEffect = false;

		_anim->_doBlack = false;
		_nextFileName = getInvName(CurInv);
	} else if (CurInv == BELTNUM) {                    /* LAB: Labyrinth specific */
		if (!_conditions->in(BELTGLOW))
			_conditions->inclElement(BELTGLOW);

		_anim->_doBlack = false;
		_nextFileName = getInvName(CurInv);
	} else if (CurInv == WHISKEYNUM) {                 /* LAB: Labyrinth specific */
		_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUseWhiskey);
	} else if (CurInv == PITHHELMETNUM) {              /* LAB: Labyrinth specific */
		_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUsePith);
	} else if (CurInv == HELMETNUM) {                  /* LAB: Labyrinth specific */
		_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUseHelmet);
	} else
		return false;

	return true;
}

/******************************************************************************/
/* Decrements the current inventory number.                                   */
/******************************************************************************/
void LabEngine::decIncInv(uint16 *CurInv, bool dec) {
	interfaceOff();

	if (dec)
		(*CurInv)--;
	else
		(*CurInv)++;

	while (*CurInv && (*CurInv <= NumInv)) {
		if (_conditions->in(*CurInv) && Inventory[*CurInv].BInvName) {
			_nextFileName = getInvName(*CurInv);
			break;
		}

		if (dec)
			(*CurInv)--;
		else
			(*CurInv)++;
	}

	if ((*CurInv == 0) || (*CurInv > NumInv)) {
		if (dec)
			*CurInv = NumInv;
		else
			*CurInv = 1;

		while (*CurInv && (*CurInv <= NumInv)) {
			if (_conditions->in(*CurInv) && Inventory[*CurInv].BInvName) {
				_nextFileName = getInvName(*CurInv);
				break;
			}

			if (dec)
				(*CurInv)--;
			else
				(*CurInv)++;
		}
	}
}


/******************************************************************************/
/* The main game loop                                                         */
/******************************************************************************/
void LabEngine::mainGameLoop() {
	uint16 actionMode = 4;
	uint16 curInv = MAPNUM;

	bool forceDraw = false, GotMessage = true;

	setPalette(initcolors, 8);

	_cptr    = NULL;
	_roomNum = 1;
	Direction = NORTH;

	_resource->readRoomData("LAB:Doors");
	_resource->readInventory("LAB:Inventor");

	if (!(_conditions = new LargeSet(HighestCondition + 1, this)))
		return;

	if (!(_roomsFound = new LargeSet(ManyRooms + 1, this)))
		return;

	_conditions->readInitialConditions("LAB:Conditio");

	_longWinInFront = false;
	drawPanel();

	perFlipGadget(actionMode);

	/* Set up initial picture. */

	while (1) {
		_event->processInput(true);

		if (GotMessage) {
			if (QuitLab || g_engine->shouldQuit()) {
				_anim->stopDiff();
				break;
			}

			_music->resumeBackMusic();

			/* Sees what kind of close up we're in and does the appropriate stuff, if any. */
			if (doCloseUp(_cptr)) {
				_cptr = NULL;

				mayShowCrumbIndicator();
				screenUpdate();
			}

			/* Sets the current picture properly on the screen */
			if (MainDisplay)
				_nextFileName = getPictName(&_cptr);

			if (noupdatediff) {
				_roomsFound->inclElement(_roomNum); /* Potentially entered another room */
				forceDraw |= (strcmp(_nextFileName, _curFileName) != 0);

				noupdatediff = false;
				_curFileName = _nextFileName;
			} else if (strcmp(_nextFileName, _curFileName) != 0) {
				interfaceOff();
				_roomsFound->inclElement(_roomNum); /* Potentially entered another room */
				_curFileName = _nextFileName;

				if (_cptr) {
					if ((_cptr->CloseUpType == SPECIALLOCK) && MainDisplay)  /* LAB: Labyrinth specific code */
						showCombination(_curFileName);
					else if (((_cptr->CloseUpType == SPECIALBRICK)  ||
					          (_cptr->CloseUpType == SPECIALBRICKNOMOUSE)) &&
					         MainDisplay) /* LAB: Labyrinth specific code */
						showTile(_curFileName, (bool)(_cptr->CloseUpType == SPECIALBRICKNOMOUSE));
					else
						readPict(_curFileName, false);
				} else
					readPict(_curFileName, false);

				drawRoomMessage(curInv, _cptr);
				forceDraw = false;

				mayShowCrumbIndicator();
				screenUpdate();

				if (!_followingCrumbs)
					eatMessages();
			}

			if (forceDraw) {
				drawRoomMessage(curInv, _cptr);
				forceDraw = false;
				screenUpdate();
			}
		}

		_music->updateMusic();  /* Make sure we check the music at least after every message */
		interfaceOn();
		IntuiMessage *curMsg = getMsg();

		if (curMsg == NULL) { /* Does music load and next animation frame when you've run out of messages */
			GotMessage = false;
			_music->checkRoomMusic();
			_music->updateMusic();
			_anim->diffNextFrame();

			if (_followingCrumbs) {
				int result = followCrumbs();

				if (result != 0) {
					uint16 code = 0;
					if (result == VKEY_UPARROW)
						code = 7;
					else if (result == VKEY_LTARROW)
						code = 6;
					else if (result == VKEY_RTARROW)
						code = 8;

					GotMessage = true;
					mayShowCrumbIndicator();
					screenUpdate();
					if (!from_crumbs(GADGETUP, code, 0, _event->updateAndGetMousePos(), curInv, curMsg, forceDraw, code, actionMode))
						break;
				}
			}

			mayShowCrumbIndicator();
			screenUpdate();
		} else {
			GotMessage = true;

			Common::Point curPos;
			curPos.x  = curMsg->mouseX;
			curPos.y  = curMsg->mouseY;

			_followingCrumbs = false;
			if (!from_crumbs(curMsg->msgClass, curMsg->code, curMsg->qualifier, curPos, curInv, curMsg, forceDraw, curMsg->gadgetID, actionMode))
				break;
		}
	}

	delete _conditions;
	delete _roomsFound;

	if (_rooms) {
		free(_rooms);
		_rooms = nullptr;
	}

	if (Inventory) {
		for (int i = 1; i <= NumInv; i++) {
			if (Inventory[i].name)
				free(Inventory[i].name);

			if (Inventory[i].BInvName)
				free(Inventory[i].BInvName);
		}

		free(Inventory);
	}
}

bool LabEngine::from_crumbs(uint32 tmpClass, uint16 code, uint16 Qualifier, Common::Point tmpPos, uint16 &curInv, IntuiMessage *curMsg, bool &forceDraw, uint16 gadgetId, uint16 &actionMode) {
	uint32 msgClass = tmpClass;
	Common::Point curPos = tmpPos;

	uint16 OldRoomNum, OldDirection = 0;
	uint16 LastInv = MAPNUM, Old;
	CloseDataPtr oldcptr, tempcptr, hcptr = NULL;
	ViewData *VPtr;
	bool doit;
	uint16 NewDir;


	_anim->_doBlack = false;

	if ((msgClass == RAWKEY) && (!_longWinInFront)) {
		if (code == 13) { /* The return key */
			msgClass     = MOUSEBUTTONS;
			Qualifier = IEQUALIFIER_LEFTBUTTON;
			curPos = _event->getMousePos();
		} else if (getPlatform() == Common::kPlatformWindows &&
				(code == 'b' || code == 'B')) {  /* Start bread crumbs */
			_breadCrumbs[0]._roomNum = 0;
			_numCrumbs = 0;
			_droppingCrumbs = true;
			mayShowCrumbIndicator();
			screenUpdate();
		} else if (code == 'f' || code == 'F' ||
		         code == 'r' || code == 'R') {  /* Follow bread crumbs */
			if (_droppingCrumbs) {
				if (_numCrumbs > 0) {
					_followingCrumbs = true;
					_followCrumbsFast = (code == 'r' || code == 'R');
					_isCrumbTurning = false;
					_isCrumbWaiting = false;
					getTime(&_crumbSecs, &_crumbMicros);

					if (_alternate) {
						eatMessages();
						_alternate = false;
						_anim->_doBlack = true;
						DoNotDrawMessage = false;

						MainDisplay = true;
						interfaceOn(); /* Sets the correct gadget list */
						drawPanel();
						drawRoomMessage(curInv, _cptr);
						screenUpdate();
					}
				} else {
					_breadCrumbs[0]._roomNum = 0;
					_droppingCrumbs = false;

					// Need to hide indicator!!!!
					mayShowCrumbIndicatorOff();
					screenUpdate();
				}
			}
		} else if ((code == 315) || (code == 'x') || (code == 'X')
		         || (code == 'q') || (code == 'Q')) {  /* Quit? */
			DoNotDrawMessage = false;
			drawMessage("Do you want to quit? (Y/N)");
			doit = false;
			eatMessages();
			interfaceOff();

			while (1) {
				_music->updateMusic();  /* Make sure we check the music at least after every message */
				curMsg = getMsg();

				if (curMsg == NULL) { /* Does music load and next animation frame when you've run out of messages */
					_music->updateMusic();
					_anim->diffNextFrame();
				} else {
					if (curMsg->msgClass == RAWKEY) {
						if ((curMsg->code == 'Y') || (curMsg->code == 'y') || (curMsg->code == 'Q') || (curMsg->code == 'q')) {
							doit = true;
							break;
						} else if (curMsg->code < 128) {
							break;
						}
					} else if (curMsg->msgClass == MOUSEBUTTONS) {
						break;
					}
				}
			}

			if (doit) {
				_anim->stopDiff();
				return false;
			} else {
				forceDraw = true;
				interfaceOn();
			}
		} else if (code == 9) { /* TAB key */
			msgClass = DELTAMOVE;
		} else if (code == 27) { /* ESC key */
			_cptr = NULL;
		}

		eatMessages();
	}

	if (_longWinInFront) {
		if ((msgClass == RAWKEY) ||
		        ((msgClass == MOUSEBUTTONS) &&
		         ((IEQUALIFIER_LEFTBUTTON & Qualifier) ||
		          (IEQUALIFIER_RBUTTON & Qualifier)))) {
			_longWinInFront = false;
			DoNotDrawMessage = false;
			drawPanel();
			drawRoomMessage(curInv, _cptr);
			screenUpdate();
		}
	} else if ((msgClass == GADGETUP) && !_alternate) {
		if (gadgetId <= 5) {
			if ((actionMode == 4) && (gadgetId == 4) && (_cptr != NULL)) {
				doMainView(&_cptr);

				_anim->_doBlack = true;
				hcptr = NULL;
				_cptr = NULL;
				mayShowCrumbIndicator();
				screenUpdate();
			} else if (gadgetId == 5) {
				eatMessages();

				_alternate = true;
				_anim->_doBlack = true;
				DoNotDrawMessage = false;
				interfaceOn(); /* Sets the correct gadget list */

				MainDisplay = false;

				if (LastInv && _conditions->in(LastInv)) {
					curInv = LastInv;
					_nextFileName = getInvName(curInv);
				} else
					decIncInv(&curInv, false);

				drawPanel();
				drawRoomMessage(curInv, _cptr);

				mayShowCrumbIndicator();
				screenUpdate();
			} else {
				Old        = actionMode;
				actionMode = gadgetId;

				if (Old < 5)
					perFlipGadget(Old);

				perFlipGadget(actionMode);

				if (gadgetId == 0)
					drawStaticMessage(kTextTakeWhat);
				else if (gadgetId == 1)
					drawStaticMessage(kTextMoveWhat);
				else if (gadgetId == 2)
					drawStaticMessage(kTextOpenWhat);
				else if (gadgetId == 3)
					drawStaticMessage(kTextCloseWhat);
				else if (gadgetId == 4)
					drawStaticMessage(kTextLookWhat);

				screenUpdate();
			}
		} else if (gadgetId == 9) {
			doUse(MAPNUM);

			mayShowCrumbIndicator();
			screenUpdate();
		} else if (gadgetId >= 6) { /* Arrow Gadgets */
			_cptr = NULL;
			hcptr = NULL;

			if ((gadgetId == 6) || (gadgetId == 8)) {
				if (gadgetId == 6)
					drawStaticMessage(kTextTurnLeft);
				else
					drawStaticMessage(kTextTurnRight);

				_curFileName = " ";

				OldDirection = Direction;

				NewDir = Direction;
				processArrow(&NewDir, gadgetId - 6);
				doTurn(Direction, NewDir, &_cptr);
				_anim->_doBlack = true;
				Direction = NewDir;
				forceDraw = true;

				mayShowCrumbIndicator();
				screenUpdate();
			} else if (gadgetId == 7) {
				OldRoomNum = _roomNum;

				if (doGoForward(&_cptr)) {
					if (OldRoomNum == _roomNum)
						_anim->_doBlack = true;
				} else {
					_anim->_doBlack = true;
					processArrow(&Direction, gadgetId - 6);

					if (OldRoomNum != _roomNum) {
						drawStaticMessage(kTextGoForward);
						_roomsFound->inclElement(_roomNum); /* Potentially entered a new room */
						_curFileName = " ";
						forceDraw = true;
					} else {
						_anim->_doBlack = true;
						drawStaticMessage(kTextNoPath);
					}
				}

				if (_followingCrumbs) {
					if (_isCrumbTurning) {
						if (Direction == OldDirection) {
							_followingCrumbs = false;
						}
					} else {
						if (_roomNum == OldRoomNum) { // didn't get there?
							_followingCrumbs = false;
						}
					}
				} else if (_droppingCrumbs && OldRoomNum != _roomNum) {
					// If in surreal maze, turn off DroppingCrumbs.
					// Note: These numbers were generated by parsing the
					// "Maps" file, which is why they are hard-coded. Bleh!
					if (_roomNum >= 245 && _roomNum <= 280) {
						_followingCrumbs = false;
						_droppingCrumbs = false;
						_numCrumbs = 0;
						_breadCrumbs[0]._roomNum = 0;
					} else {
						bool intersect = false;
						for (int idx = 0; idx < _numCrumbs; idx++) {
							if (_breadCrumbs[idx]._roomNum == _roomNum) {
								_numCrumbs = idx + 1;
								_breadCrumbs[_numCrumbs]._roomNum = 0;
								intersect = true;
							}
						}

						if (!intersect) {
							if (_numCrumbs == MAX_CRUMBS) {
								_numCrumbs = MAX_CRUMBS - 1;
								memcpy(&_breadCrumbs[0], &_breadCrumbs[1], _numCrumbs * sizeof _breadCrumbs[0]);
							}

							_breadCrumbs[_numCrumbs]._roomNum = _roomNum;
							_breadCrumbs[_numCrumbs++]._direction = Direction;
						}
					}
				}

				mayShowCrumbIndicator();
				screenUpdate();
			}
		}
	} else if ((msgClass == GADGETUP) && _alternate) {
		_anim->_doBlack = true;

		if (gadgetId == 0) {
			eatMessages();
			_alternate = false;
			_anim->_doBlack = true;
			DoNotDrawMessage = false;

			MainDisplay = true;
			interfaceOn(); /* Sets the correct gadget list */
			drawPanel();
			drawRoomMessage(curInv, _cptr);

			screenUpdate();
		}

		gadgetId--;

		if (gadgetId == 0) {
			interfaceOff();
			_anim->stopDiff();
			_curFileName = " ";

			doit = !saveRestoreGame();
			_cptr = NULL;

			MainDisplay = true;

			curInv = MAPNUM;
			LastInv = MAPNUM;

			_nextFileName = getInvName(curInv);

			drawPanel();

			if (doit) {
				drawMessage("Disk operation failed.");
				setPalette(initcolors, 8);

				screenUpdate();

				g_system->delayMillis(1000);
			} else {
				screenUpdate();
			}
		} else if (gadgetId == 1) {
			if (!doUse(curInv)) {
				Old        = actionMode;
				actionMode = 5;  /* Use button */

				if (Old < 5)
					perFlipGadget(Old);

				drawStaticMessage(kTextUseOnWhat);
				MainDisplay = true;

				screenUpdate();
			}
		} else if (gadgetId == 2) {
			MainDisplay = !MainDisplay;

			if ((curInv == 0) || (curInv > NumInv)) {
				curInv = 1;

				while ((curInv <= NumInv) && (!_conditions->in(curInv)))
					curInv++;
			}

			if ((curInv <= NumInv) && _conditions->in(curInv) &&
			        Inventory[curInv].BInvName)
				_nextFileName = getInvName(curInv);

			screenUpdate();
		} else if (gadgetId == 3) { /* Left gadget */
			decIncInv(&curInv, true);
			LastInv = curInv;
			DoNotDrawMessage = false;
			drawRoomMessage(curInv, _cptr);

			screenUpdate();
		} else if (gadgetId == 4) { /* Right gadget */
			decIncInv(&curInv, false);
			LastInv = curInv;
			DoNotDrawMessage = false;
			drawRoomMessage(curInv, _cptr);

			screenUpdate();
		} else if (gadgetId == 5) { /* bread crumbs */
			_breadCrumbs[0]._roomNum = 0;
			_numCrumbs = 0;
			_droppingCrumbs = true;
			mayShowCrumbIndicator();
			screenUpdate();
		} else if (gadgetId == 6) { /* follow crumbs */
			if (_droppingCrumbs) {
				if (_numCrumbs > 0) {
					_followingCrumbs = true;
					_followCrumbsFast = false;
					_isCrumbTurning = false;
					_isCrumbWaiting = false;
					getTime(&_crumbSecs, &_crumbMicros);

					eatMessages();
					_alternate = false;
					_anim->_doBlack = true;
					DoNotDrawMessage = false;

					MainDisplay = true;
					interfaceOn(); /* Sets the correct gadget list */
					drawPanel();
					drawRoomMessage(curInv, _cptr);
					screenUpdate();
				} else {
					_breadCrumbs[0]._roomNum = 0;
					_droppingCrumbs = false;

					// Need to hide indicator!!!!
					mayShowCrumbIndicatorOff();
					screenUpdate();
				}
			}
		}
	} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier) && MainDisplay) {
		interfaceOff();
		MainDisplay = true;

		doit = false;

		if (_cptr) {
			if ((_cptr->CloseUpType == SPECIALLOCK) && MainDisplay) /* LAB: Labyrinth specific code */
				mouseCombination(curPos);
			else if ((_cptr->CloseUpType == SPECIALBRICK) && MainDisplay)
				mouseTile(curPos);
			else
				doit = true;
		} else
			doit = true;


		if (doit) {
			hcptr = NULL;
			eatMessages();

			if (actionMode == 0) { /* Take something. */
				if (doActionRule(Common::Point(curPos.x, curPos.y), actionMode, _roomNum, &_cptr))
					_curFileName = _newFileName;
				else if (takeItem(curPos.x, curPos.y, &_cptr))
					drawStaticMessage(kTextTakeItem);
				else if (doActionRule(curPos, TAKEDEF - 1, _roomNum, &_cptr))
					_curFileName = _newFileName;
				else if (doActionRule(curPos, TAKE - 1, 0, &_cptr))
					_curFileName = _newFileName;
				else if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 1) /* Manipulate an object */  ||
			         (actionMode == 2) /* Open up a "door" */      ||
			         (actionMode == 3)) { /* Close a "door" */
				if (doActionRule(curPos, actionMode, _roomNum, &_cptr))
					_curFileName = _newFileName;
				else if (!doActionRule(curPos, actionMode, 0, &_cptr)) {
					if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
						drawStaticMessage(kTextNothing);
				}
			} else if (actionMode == 4) { /* Look at closeups */
				tempcptr = _cptr;
				setCurClose(curPos, &tempcptr);

				if (_cptr == tempcptr) {
					if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
						drawStaticMessage(kTextNothing);
				} else if (tempcptr->GraphicName) {
					if (*(tempcptr->GraphicName)) {
						_anim->_doBlack = true;
						_cptr = tempcptr;
					} else if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
						drawStaticMessage(kTextNothing);
				} else if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 5)  &&
			         _conditions->in(curInv)) { /* Use an item on something else */
				if (doOperateRule(curPos.x, curPos.y, curInv, &_cptr)) {
					_curFileName = _newFileName;

					if (!_conditions->in(curInv))
						decIncInv(&curInv, false);
				} else if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
					drawStaticMessage(kTextNothing);
			}
		}

		mayShowCrumbIndicator();
		screenUpdate();
	} else if (msgClass == DELTAMOVE) {
		VPtr = getViewData(_roomNum, Direction);
		oldcptr = VPtr->closeUps;

		if (hcptr == NULL) {
			tempcptr = _cptr;
			setCurClose(curPos, &tempcptr);

			if ((tempcptr == NULL) || (tempcptr == _cptr)) {
				if (_cptr == NULL)
					hcptr = oldcptr;
				else
					hcptr = _cptr->SubCloseUps;
			} else
				hcptr = tempcptr->NextCloseUp;
		} else
			hcptr = hcptr->NextCloseUp;


		if (hcptr == NULL) {
			if (_cptr == NULL)
				hcptr = oldcptr;
			else
				hcptr = _cptr->SubCloseUps;
		}

		if (hcptr)
			_event->setMousePos(Common::Point(scaleX((hcptr->x1 + hcptr->x2) / 2), scaleY((hcptr->y1 + hcptr->y2) / 2)));
	} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) {
		eatMessages();
		_alternate = !_alternate;
		_anim->_doBlack = true;
		DoNotDrawMessage = false;
		MainDisplay = true;
		interfaceOn(); /* Sets the correct gadget list */

		if (_alternate) {
			if (LastInv && _conditions->in(LastInv))
				curInv = LastInv;
			else
				decIncInv(&curInv, false);
		}

		drawPanel();
		drawRoomMessage(curInv, _cptr);

		mayShowCrumbIndicator();
		screenUpdate();
	}
	return true;
}

void LabEngine::go() {
	bool doIntro = true;

	_isHiRes = ((getFeatures() & GF_LOWRES) == 0);

	bool mem = false;
	if (initBuffer(BUFFERSIZE, true)) {
		mem = true;
	} else {
		warning("initBuffer() failed");
		return;
	}

	if (!setUpScreens()) {
		_isHiRes = false;
		mem = mem && setUpScreens();
	}

	_event->initMouse();

	mem = mem && initRoomBuffer();

	if (!doIntro)
		_music->initMusic();

	_msgFont = _resource->getFont("P:AvanteG.12");

	_event->mouseHide();

	if (doIntro && mem) {
		Intro *intro = new Intro(this);
		intro->introSequence();
		delete intro;
	} else
		_anim->_doBlack = true;

	if (mem) {
		_event->mouseShow();
		mainGameLoop();
	} else
		debug("\n\nNot enough memory to start game.\n\n");

	if (QuitLab) { /* Won the game */
		blackAllScreen();
		readPict("P:End/L2In.1", true);

		for (uint16 i = 0; i < 120; i++) {
			_music->updateMusic();
			waitTOF();
		}

		readPict("P:End/L2In.9", true);
		readPict("P:End/Lost", true);

		warning("STUB: waitForPress");
		while (!1) { // 1 means ignore SDL_ProcessInput calls
			_music->updateMusic();
			_anim->diffNextFrame();
			waitTOF();
		}
	}

	closeFont(_msgFont);

	freeRoomBuffer();
	freeBuffer();

	freeScreens();

	_music->freeMusic();
}

/*****************************************************************************/
/* New code to allow quick(er) return navigation in game.                    */
/*****************************************************************************/
int LabEngine::followCrumbs() {
	// NORTH, SOUTH, EAST, WEST
	static int movement[4][4] = {
		{ VKEY_UPARROW, VKEY_RTARROW, VKEY_RTARROW, VKEY_LTARROW },
		{ VKEY_RTARROW, VKEY_UPARROW, VKEY_LTARROW, VKEY_RTARROW },
		{ VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW, VKEY_RTARROW },
		{ VKEY_RTARROW, VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW }
	};

	if (_isCrumbWaiting) {
		uint32 Secs;
		uint32 Micros;

		g_lab->timeDiff(_crumbSecs, _crumbMicros, &Secs, &Micros);

		if (Secs != 0 || Micros != 0)
			return 0;

		_isCrumbWaiting = false;
	}

	if (!_isCrumbTurning)
		_breadCrumbs[_numCrumbs--]._roomNum = 0;

	// Is the current crumb this room? If not, logic error.
	if (g_lab->_roomNum != _breadCrumbs[_numCrumbs]._roomNum) {
		_numCrumbs = 0;
		_breadCrumbs[0]._roomNum = 0;
		_droppingCrumbs = false;
		_followingCrumbs = false;
		return 0;
	}

	int exitDir;

	// which direction is last crumb
	if (_breadCrumbs[_numCrumbs]._direction == EAST)
		exitDir = WEST;
	else if (_breadCrumbs[_numCrumbs]._direction == WEST)
		exitDir = EAST;
	else if (_breadCrumbs[_numCrumbs]._direction == NORTH)
		exitDir = SOUTH;
	else
		exitDir = NORTH;

	int moveDir = movement[Direction][exitDir];

	if (_numCrumbs == 0) {
		_isCrumbTurning = false;
		_breadCrumbs[0]._roomNum = 0;
		_droppingCrumbs = false;
		_followingCrumbs = false;
	} else {
		int theDelay = (_followCrumbsFast ? ONESECOND / 4 : ONESECOND);

		_isCrumbTurning = (moveDir != VKEY_UPARROW);
		_isCrumbWaiting = true;

		g_lab->addCurTime(theDelay / ONESECOND, theDelay % ONESECOND, &_crumbSecs, &_crumbMicros);
	}

	return moveDir;
}

byte dropCrumbs[] = { 0x00 };
Image dropCrumbsImage(24, 24, dropCrumbs);

void LabEngine::mayShowCrumbIndicator() {
	if (getPlatform() != Common::kPlatformWindows)
		return;

	if (_droppingCrumbs && MainDisplay) {
		_event->mouseHide();
		dropCrumbsImage.drawMaskImage(612, 4);
		_event->mouseShow();
	}
}

byte dropCrumbsOff[] = { 0x00 };
Image dropCrumbsOffImage(24, 24, dropCrumbsOff);

void LabEngine::mayShowCrumbIndicatorOff() {
	if (getPlatform() != Common::kPlatformWindows)
		return;

	if (MainDisplay) {
		_event->mouseHide();
		dropCrumbsOffImage.drawMaskImage(612, 4);
		_event->mouseShow();
	}
}

/* Have to make sure that ROOMBUFFERSIZE is bigger than the biggest piece of memory
   that we need */
#define ROOMBUFFERSIZE (2 * 20480L)

static void *_roomBuffer = nullptr;
static uint16 _curMarker  = 0;
static void *_memPlace = nullptr;

/*****************************************************************************/
/* Allocates the memory for the room buffers.                                */
/*****************************************************************************/
bool initRoomBuffer() {
	_curMarker = 0;

	if ((_roomBuffer = calloc(ROOMBUFFERSIZE, 1))) {
		_memPlace = _roomBuffer;

		return true;
	} else
		return false;
}

/*****************************************************************************/
/* Frees the memory for the room buffers.                                    */
/*****************************************************************************/
void freeRoomBuffer() {
	if (_roomBuffer) {
		free(_roomBuffer);
		_roomBuffer = nullptr;
	}
}

} // End of namespace Lab
