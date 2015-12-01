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
#include "lab/text.h"
#include "lab/intro.h"
#include "lab/parsefun.h"
#include "lab/interface.h"
#include "lab/mouse.h"
#include "lab/resource.h"

namespace Lab {

const char *CurFileName = " ";

bool LongWinInFront = false;

TextFont *MsgFont;

extern bool stopsound, DoNotDrawMessage;

/* Global parser data */

extern RoomData *_rooms;
extern InventoryData *Inventory;
extern uint16 NumInv, ManyRooms, HighestCondition, Direction;
CloseDataPtr CPtr;

bool Alternate = false, ispal = false, noupdatediff = false, MainDisplay = true, QuitLab = false;

extern const char *NewFileName;  /* When ProcessRoom.c decides to change the filename
                                    of the current picture. */

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


static Image *MoveImages[20], *InvImages[10];
static Gadget *MoveGadgetList, *InvGadgetList;


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

	if (!Alternate) {
		setAPen(4);
		drawHLine(0, VGAScaleY(170) + 1, VGAScaleX(319)); /* The horizontal lines under the black one */
		drawGadgetList(MoveGadgetList);
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

		drawGadgetList(InvGadgetList);
	}

	_event->mouseShow();
}

/******************************************************************************/
/* Draws the message for the room.                                            */
/******************************************************************************/
void LabEngine::drawRoomMessage(uint16 CurInv, CloseDataPtr cptr) {
	if (_lastTooLong) {
		_lastTooLong = false;
		return;
	}

	if (Alternate) {
		if ((CurInv <= NumInv) && _conditions->in(CurInv) && Inventory[CurInv].BInvName) {
			if ((CurInv == LAMPNUM) && _conditions->in(LAMPON))  /* LAB: Labyrith specific */
				drawStaticMessage(kTextLampOn);
			else if (Inventory[CurInv].Many > 1) {
				Common::String roomMessage = Common::String(Inventory[CurInv].name) + "  (" + Common::String::format("%d", Inventory[CurInv].Many) + ")";
				drawMessage(roomMessage.c_str());
			} else
				drawMessage(Inventory[CurInv].name);
		}
	} else
		drawDirection(cptr);

	_lastTooLong = _lastMessageLong;
}


/******************************************************************************/
/* Sets up the Labyrinth screens, and opens up the initial windows.           */
/******************************************************************************/
bool LabEngine::setUpScreens() {
	byte *buffer;
	byte *MovePanelBuffer, *InvPanelBuffer;
	Gadget *curgad;
	uint16 y;

	if (!createScreen(_isHiRes))
		return false;

	/* Loads in the graphics for the movement control panel */
	Common::File file;
	file.open(translateFileName("P:Control"));
	if (!file.isOpen())
		warning("setUpScreens couldn't open %s", translateFileName("P:Control"));

	if (file.err() || file.size() == 0)
		return false;

	if (!(MovePanelBuffer = (byte *)calloc(file.size(), 1)))
		return false;

	file.read(MovePanelBuffer, file.size());
	file.close();

	buffer = MovePanelBuffer;

	for (uint16 i = 0; i < 20; i++)
		readImage(&buffer, &(MoveImages[i]));

	/* Creates the gadgets for the movement control panel */
	y = VGAScaleY(173) - SVGACord(2);

	if (getPlatform() == Common::kPlatformWindows) {
		MoveGadgetList = createButton(1, y, 0, 't', MoveImages[0], MoveImages[1]);
		curgad = MoveGadgetList;
		curgad->NextGadget = createButton(33, y, 1, 'm', MoveImages[2], MoveImages[3]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(65, y, 2, 'o', MoveImages[4], MoveImages[5]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(97, y, 3, 'c', MoveImages[6], MoveImages[7]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(129, y, 4, 'l', MoveImages[8], MoveImages[9]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(161, y, 5, 'i', MoveImages[12], MoveImages[13]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(193, y, 6, VKEY_LTARROW, MoveImages[14], MoveImages[15]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(225, y, 7, VKEY_UPARROW, MoveImages[16], MoveImages[17]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(257, y, 8, VKEY_RTARROW, MoveImages[18], MoveImages[19]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(289, y, 9, 'p', MoveImages[10], MoveImages[11]);
	} else {
		MoveGadgetList = createButton(1, y, 0, 0, MoveImages[0], MoveImages[1]);
		curgad = MoveGadgetList;
		curgad->NextGadget = createButton(33, y, 1, 0, MoveImages[2], MoveImages[3]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(65, y, 2, 0, MoveImages[4], MoveImages[5]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(97, y, 3, 0, MoveImages[6], MoveImages[7]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(129, y, 4, 0, MoveImages[8], MoveImages[9]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(161, y, 5, 0, MoveImages[12], MoveImages[13]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(193, y, 6, 0, MoveImages[14], MoveImages[15]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(225, y, 7, 0, MoveImages[16], MoveImages[17]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(257, y, 8, 0, MoveImages[18], MoveImages[19]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(289, y, 9, 0, MoveImages[10], MoveImages[11]);
	}

	file.open(translateFileName("P:Inv"));
	if (!file.isOpen())
		warning("setUpScreens couldn't open %s", translateFileName("P:Inv"));

	if (file.err() || file.size() == 0)
		return false;

	if (!(InvPanelBuffer = (byte *)calloc(file.size(), 1)))
		return false;

	file.read(InvPanelBuffer, file.size());
	file.close();

	buffer = InvPanelBuffer;

	if (getPlatform() == Common::kPlatformWindows) {
		for (uint16 imgIdx = 0; imgIdx < 10; imgIdx++)
			readImage(&buffer, &(InvImages[imgIdx]));

		InvGadgetList = createButton(24, y, 0, 'm', InvImages[0], InvImages[1]);
		curgad = InvGadgetList;
		curgad->NextGadget = createButton(56, y, 1, 'g', InvImages[2], InvImages[3]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(94, y, 2, 'u', InvImages[4], InvImages[5]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(126, y, 3, 'l', MoveImages[8], MoveImages[9]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(164, y, 4, VKEY_LTARROW, MoveImages[14], MoveImages[15]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(196, y, 5, VKEY_RTARROW, MoveImages[18], MoveImages[19]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(234, y, 6, 'b', InvImages[6], InvImages[7]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(266, y, 7, 'f', InvImages[8], InvImages[9]);
		curgad = curgad->NextGadget;
	} else {
		for (uint16 imgIdx = 0; imgIdx < 6; imgIdx++)
			readImage(&buffer, &(InvImages[imgIdx]));

		InvGadgetList = createButton(58, y, 0, 0, InvImages[0], InvImages[1]);
		curgad = InvGadgetList;
		curgad->NextGadget = createButton(90, y, 1, 0, InvImages[2], InvImages[3]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(128, y, 2, 0, InvImages[4], InvImages[5]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(160, y, 3, 0, MoveImages[8], MoveImages[9]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(198, y, 4, 0, MoveImages[14], MoveImages[15]);
		curgad = curgad->NextGadget;
		curgad->NextGadget = createButton(230, y, 5, 0, MoveImages[18], MoveImages[19]);
		curgad = curgad->NextGadget;
	}

	return true;
}



/******************************************************************************/
/* Permanently flips the imagry of a gadget.                                  */
/******************************************************************************/
void LabEngine::perFlipGadget(uint16 GadID) {
	Image *Temp;
	Gadget *TopGad;

	TopGad = MoveGadgetList;

	while (TopGad) {
		if (TopGad->GadgetID == GadID) {
			Temp = TopGad->Im;
			TopGad->Im = TopGad->ImAlt;
			TopGad->ImAlt = Temp;

			if (!Alternate) {
				_event->mouseHide();
				drawImage(TopGad->Im, TopGad->x, TopGad->y);
				_event->mouseShow();
			}

			return;
		} else
			TopGad = TopGad->NextGadget;
	}
}



/******************************************************************************/
/* Eats all the available messages.                                           */
/******************************************************************************/
void LabEngine::eatMessages() {
	IntuiMessage *Msg;

	do {
		Msg = getMsg();
	} while (Msg);

	return;
}

/******************************************************************************/
/* Checks whether the close up is one of the special case closeups.           */
/******************************************************************************/
bool LabEngine::doCloseUp(CloseDataPtr cptr) {
	if (cptr == NULL)
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

	switch (cptr->CloseUpType) {
	case MUSEUMMONITOR:
	case LIBRARYMONITOR:
	case WINDOWMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, false, monltmargin, montopmargin, monrtmargin, 165);
		break;
	case GRAMAPHONEMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, false, monltmargin, montopmargin, 171, 165);
		break;
	case UNICYCLEMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, false, 100, montopmargin, monrtmargin, 165);
		break;
	case STATUEMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, false, 117, montopmargin, monrtmargin, 165);
		break;
	case TALISMANMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, false, monltmargin, montopmargin, 184, 165);
		break;
	case LUTEMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, false, monltmargin, montopmargin, lutertmargin, 165);
		break;
	case CLOCKMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, false, monltmargin, montopmargin, 206, 165);
		break;
	case TERMINALMONITOR:
		doMonitor(cptr->GraphicName, cptr->Message, true, monltmargin, montopmargin, monrtmargin, 165);
		break;
	default:
		return false;
	}

	CurFileName = " ";
	drawPanel();

	return true;
}




/******************************************************************************/
/* Gets the current inventory name.                                           */
/******************************************************************************/
static const char *getInvName(uint16 CurInv) {
	if (MainDisplay)
		return Inventory[CurInv].BInvName;

	if ((CurInv == LAMPNUM) && g_lab->_conditions->in(LAMPON))
		return "P:Mines/120";

	else if ((CurInv == BELTNUM) && g_lab->_conditions->in(BELTGLOW))
		return "P:Future/BeltGlow";

	else if (CurInv == WESTPAPERNUM) {
		CurFileName = Inventory[CurInv].BInvName;
		g_lab->_anim->nopalchange = true;
		readPict(CurFileName, false);
		g_lab->_anim->nopalchange = false;
		doWestPaper();
	}

	else if (CurInv == NOTESNUM) {
		CurFileName = Inventory[CurInv].BInvName;
		g_lab->_anim->nopalchange = true;
		readPict(CurFileName, false);
		g_lab->_anim->nopalchange = false;
		doNotes();
	}

	return Inventory[CurInv].BInvName;
}

static bool interfaceisoff = false;

/******************************************************************************/
/* Turns the interface off.                                                   */
/******************************************************************************/
void LabEngine::interfaceOff() {
	if (!interfaceisoff) {
		_event->attachGadgetList(NULL);
		_event->mouseHide();
		interfaceisoff = true;
	}
}

/******************************************************************************/
/* Turns the interface on.                                                    */
/******************************************************************************/
void LabEngine::interfaceOn() {
	if (interfaceisoff) {
		interfaceisoff = false;

		_event->mouseShow();
	}

	if (LongWinInFront)
		_event->attachGadgetList(NULL);
	else if (Alternate)
		_event->attachGadgetList(InvGadgetList);
	else
		_event->attachGadgetList(MoveGadgetList);
}

static const char *Test;

/******************************************************************************/
/* If the user hits the "Use" gadget; things that can get used on themselves. */
/******************************************************************************/
bool LabEngine::doUse(uint16 CurInv) {
	if (CurInv == MAPNUM) {                  /* LAB: Labyrinth specific */
		drawStaticMessage(kTextUseMap);
		interfaceOff();
		_anim->stopDiff();
		CurFileName = " ";
		CPtr = NULL;
		doMap(_roomNum);
		setPalette(initcolors, 8);
		drawMessage(NULL);
		drawPanel();
	} else if (CurInv == JOURNALNUM) {         /* LAB: Labyrinth specific */
		drawStaticMessage(kTextUseJournal);
		interfaceOff();
		_anim->stopDiff();
		CurFileName = " ";
		CPtr = NULL;
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

		_anim->DoBlack = false;
		_anim->waitForEffect = true;
		readPict("Music:Click", true);
		_anim->waitForEffect = false;

		_anim->DoBlack = false;
		Test = getInvName(CurInv);
	} else if (CurInv == BELTNUM) {                    /* LAB: Labyrinth specific */
		if (!_conditions->in(BELTGLOW))
			_conditions->inclElement(BELTGLOW);

		_anim->DoBlack = false;
		Test = getInvName(CurInv);
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
			Test = getInvName(*CurInv);
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
				Test = getInvName(*CurInv);
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

	CPtr    = NULL;
	_roomNum = 1;
	Direction = NORTH;

	_resource->readRoomData("LAB:Doors");
	_resource->readInventory("LAB:Inventor");

	if (!(_conditions = new LargeSet(HighestCondition + 1, this)))
		return;

	if (!(_roomsFound = new LargeSet(ManyRooms + 1, this)))
		return;

	_conditions->readInitialConditions("LAB:Conditio");

	LongWinInFront = false;
	drawPanel();

	perFlipGadget(actionMode);

	/* Set up initial picture. */

	while (1) {
		processInput(true);

		if (GotMessage) {
			if (QuitLab || g_engine->shouldQuit()) {
				_anim->stopDiff();
				break;
			}

			_music->resumeBackMusic();

			/* Sees what kind of close up we're in and does the appropriate stuff, if any. */
			if (doCloseUp(CPtr)) {
				CPtr = NULL;

				mayShowCrumbIndicator();
				screenUpdate();
			}

			/* Sets the current picture properly on the screen */
			if (MainDisplay)
				Test = getPictName(&CPtr);

			if (noupdatediff) {
				_roomsFound->inclElement(_roomNum); /* Potentially entered another room */
				forceDraw = (strcmp(Test, CurFileName) != 0) || forceDraw;

				noupdatediff = false;
				CurFileName = Test;
			}

			else if (strcmp(Test, CurFileName) != 0) {
				interfaceOff();
				_roomsFound->inclElement(_roomNum); /* Potentially entered another room */
				CurFileName = Test;

				if (CPtr) {
					if ((CPtr->CloseUpType == SPECIALLOCK) && MainDisplay)  /* LAB: Labyrinth specific code */
						showCombination(CurFileName);
					else if (((CPtr->CloseUpType == SPECIALBRICK)  ||
					          (CPtr->CloseUpType == SPECIALBRICKNOMOUSE)) &&
					         MainDisplay) /* LAB: Labyrinth specific code */
						showTile(CurFileName, (bool)(CPtr->CloseUpType == SPECIALBRICKNOMOUSE));
					else
						readPict(CurFileName, false);
				} else
					readPict(CurFileName, false);

				drawRoomMessage(curInv, CPtr);
				forceDraw = false;

				mayShowCrumbIndicator();
				screenUpdate();

				if (!_followingCrumbs)
					eatMessages();
			}

			if (forceDraw) {
				drawRoomMessage(curInv, CPtr);
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
					if (!from_crumbs(GADGETUP, code, 0, getMousePos(), curInv, curMsg, forceDraw, code, actionMode))
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
	CloseDataPtr OldCPtr, TempCPtr, HCPtr = NULL;
	ViewData *VPtr;
	bool doit;
	uint16 NewDir;


	_anim->DoBlack = false;

	if ((msgClass == RAWKEY) && (!LongWinInFront)) {
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

					if (Alternate) {
						eatMessages();
						Alternate = false;
						_anim->DoBlack = true;
						DoNotDrawMessage = false;

						MainDisplay = true;
						interfaceOn(); /* Sets the correct gadget list */
						drawPanel();
						drawRoomMessage(curInv, CPtr);
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
			CPtr = NULL;
		}

		eatMessages();
	}

	if (LongWinInFront) {
		if ((msgClass == RAWKEY) ||
		        ((msgClass == MOUSEBUTTONS) &&
		         ((IEQUALIFIER_LEFTBUTTON & Qualifier) ||
		          (IEQUALIFIER_RBUTTON & Qualifier)))) {
			LongWinInFront = false;
			DoNotDrawMessage = false;
			drawPanel();
			drawRoomMessage(curInv, CPtr);
			screenUpdate();
		}
	} else if ((msgClass == GADGETUP) && !Alternate) {
		if (gadgetId <= 5) {
			if ((actionMode == 4) && (gadgetId == 4) && (CPtr != NULL)) {
				doMainView(&CPtr);

				_anim->DoBlack = true;
				HCPtr = NULL;
				CPtr = NULL;
				mayShowCrumbIndicator();
				screenUpdate();
			} else if (gadgetId == 5) {
				eatMessages();

				Alternate = true;
				_anim->DoBlack = true;
				DoNotDrawMessage = false;
				interfaceOn(); /* Sets the correct gadget list */

				MainDisplay = false;

				if (LastInv && _conditions->in(LastInv)) {
					curInv = LastInv;
					Test = getInvName(curInv);
				} else
					decIncInv(&curInv, false);

				drawPanel();
				drawRoomMessage(curInv, CPtr);

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
			CPtr = NULL;
			HCPtr = NULL;

			if ((gadgetId == 6) || (gadgetId == 8)) {
				if (gadgetId == 6)
					drawStaticMessage(kTextTurnLeft);
				else
					drawStaticMessage(kTextTurnRight);

				CurFileName = " ";

				OldDirection = Direction;

				NewDir = Direction;
				processArrow(&NewDir, gadgetId - 6);
				doTurn(Direction, NewDir, &CPtr);
				_anim->DoBlack = true;
				Direction = NewDir;
				forceDraw = true;

				mayShowCrumbIndicator();
				screenUpdate();
			} else if (gadgetId == 7) {
				OldRoomNum = _roomNum;

				if (doGoForward(&CPtr)) {
					if (OldRoomNum == _roomNum)
						_anim->DoBlack = true;
				} else {
					_anim->DoBlack = true;
					processArrow(&Direction, gadgetId - 6);

					if (OldRoomNum != _roomNum) {
						drawStaticMessage(kTextGoForward);
						_roomsFound->inclElement(_roomNum); /* Potentially entered a new room */
						CurFileName = " ";
						forceDraw = true;
					} else {
						_anim->DoBlack = true;
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
	} else if ((msgClass == GADGETUP) && Alternate) {
		_anim->DoBlack = true;

		if (gadgetId == 0) {
			eatMessages();
			Alternate = false;
			_anim->DoBlack = true;
			DoNotDrawMessage = false;

			MainDisplay = true;
			interfaceOn(); /* Sets the correct gadget list */
			drawPanel();
			drawRoomMessage(curInv, CPtr);

			screenUpdate();
		}

		gadgetId--;

		if (gadgetId == 0) {
			interfaceOff();
			_anim->stopDiff();
			CurFileName = " ";

			doit = !saveRestoreGame();
			CPtr = NULL;

			MainDisplay = true;

			curInv = MAPNUM;
			LastInv = MAPNUM;

			Test = getInvName(curInv);

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
				Test = getInvName(curInv);

			screenUpdate();
		} else if (gadgetId == 3) { /* Left gadget */
			decIncInv(&curInv, true);
			LastInv = curInv;
			DoNotDrawMessage = false;
			drawRoomMessage(curInv, CPtr);

			screenUpdate();
		} else if (gadgetId == 4) { /* Right gadget */
			decIncInv(&curInv, false);
			LastInv = curInv;
			DoNotDrawMessage = false;
			drawRoomMessage(curInv, CPtr);

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
					Alternate = false;
					_anim->DoBlack = true;
					DoNotDrawMessage = false;

					MainDisplay = true;
					interfaceOn(); /* Sets the correct gadget list */
					drawPanel();
					drawRoomMessage(curInv, CPtr);
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

		if (CPtr) {
			if ((CPtr->CloseUpType == SPECIALLOCK) && MainDisplay) /* LAB: Labyrinth specific code */
				mouseCombination(curPos);
			else if ((CPtr->CloseUpType == SPECIALBRICK) && MainDisplay)
				mouseTile(curPos);
			else
				doit = true;
		} else
			doit = true;


		if (doit) {
			HCPtr = NULL;
			eatMessages();

			if (actionMode == 0) { /* Take something. */
				if (doActionRule(Common::Point(curPos.x, curPos.y), actionMode, _roomNum, &CPtr))
					CurFileName = NewFileName;
				else if (takeItem(curPos.x, curPos.y, &CPtr))
					drawStaticMessage(kTextTakeItem);
				else if (doActionRule(curPos, TAKEDEF - 1, _roomNum, &CPtr))
					CurFileName = NewFileName;
				else if (doActionRule(curPos, TAKE - 1, 0, &CPtr))
					CurFileName = NewFileName;
				else if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 1) /* Manipulate an object */  ||
			         (actionMode == 2) /* Open up a "door" */      ||
			         (actionMode == 3)) { /* Close a "door" */
				if (doActionRule(curPos, actionMode, _roomNum, &CPtr))
					CurFileName = NewFileName;
				else if (!doActionRule(curPos, actionMode, 0, &CPtr)) {
					if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
						drawStaticMessage(kTextNothing);
				}
			} else if (actionMode == 4) { /* Look at closeups */
				TempCPtr = CPtr;
				setCurClose(curPos, &TempCPtr);

				if (CPtr == TempCPtr) {
					if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
						drawStaticMessage(kTextNothing);
				} else if (TempCPtr->GraphicName) {
					if (*(TempCPtr->GraphicName)) {
						_anim->DoBlack = true;
						CPtr = TempCPtr;
					} else if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
						drawStaticMessage(kTextNothing);
				} else if (curPos.y < (VGAScaleY(149) + SVGACord(2)))
					drawStaticMessage(kTextNothing);
			} else if ((actionMode == 5)  &&
			         _conditions->in(curInv)) { /* Use an item on something else */
				if (doOperateRule(curPos.x, curPos.y, curInv, &CPtr)) {
					CurFileName = NewFileName;

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
		OldCPtr = VPtr->closeUps;

		if (HCPtr == NULL) {
			TempCPtr = CPtr;
			setCurClose(curPos, &TempCPtr);

			if ((TempCPtr == NULL) || (TempCPtr == CPtr)) {
				if (CPtr == NULL)
					HCPtr = OldCPtr;
				else
					HCPtr = CPtr->SubCloseUps;
			} else
				HCPtr = TempCPtr->NextCloseUp;
		} else
			HCPtr = HCPtr->NextCloseUp;


		if (HCPtr == NULL) {
			if (CPtr == NULL)
				HCPtr = OldCPtr;
			else
				HCPtr = CPtr->SubCloseUps;
		}

		if (HCPtr)
			_event->setMousePos(Common::Point(scaleX((HCPtr->x1 + HCPtr->x2) / 2), scaleY((HCPtr->y1 + HCPtr->y2) / 2)));
	} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) {
		eatMessages();
		Alternate = !Alternate;
		_anim->DoBlack = true;
		DoNotDrawMessage = false;
		MainDisplay = true;
		interfaceOn(); /* Sets the correct gadget list */

		if (Alternate) {
			if (LastInv && _conditions->in(LastInv))
				curInv = LastInv;
			else
				decIncInv(&curInv, false);
		}

		drawPanel();
		drawRoomMessage(curInv, CPtr);

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

	MsgFont = _resource->getFont("P:AvanteG.12");

	_event->mouseHide();

	if (doIntro && mem) {
		Intro intro;
		intro.introSequence();
	} else
		_anim->DoBlack = true;

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

	closeFont(MsgFont);

	freeRoomBuffer();
	freeBuffer();

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

void LabEngine::mayShowCrumbIndicator() {
	if (getPlatform() != Common::kPlatformWindows)
		return;

	if (_droppingCrumbs && MainDisplay) {
		static byte dropCrumbs[] = { 0x00 };
		static Image dropCrumbsImage = { 24, 24, dropCrumbs };

		_event->mouseHide();
		drawMaskImage(&dropCrumbsImage, 612, 4);
		_event->mouseShow();
	}
}

void LabEngine::mayShowCrumbIndicatorOff() {
	if (getPlatform() != Common::kPlatformWindows)
		return;

	if (MainDisplay) {
		static byte dropCrumbsOff[] = { 0x00 };
		static Image dropCrumbsOffImage = { 24, 24, dropCrumbsOff };

		_event->mouseHide();
		drawMaskImage(&dropCrumbsOffImage, 612, 4);
		_event->mouseShow();
	}
}

} // End of namespace Lab
