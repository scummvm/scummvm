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
#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/diff.h"
#include "lab/vga.h"
#include "lab/timing.h"
#include "lab/text.h"
#include "lab/parsefun.h"
#include "lab/interface.h"
#include "lab/mouse.h"
#include "lab/resource.h"

namespace Lab {

const char *CurFileName = " ";

bool LongWinInFront = false;

TextFont *MsgFont;

extern bool DoBlack, waitForEffect, stopsound, DoNotDrawMessage, IsHiRes, nopalchange;

/* Global parser data */

extern RoomData *Rooms;
extern InventoryData *Inventory;
extern uint16 NumInv, RoomNum, ManyRooms, HighestCondition, Direction;
CloseDataPtr CPtr;

CrumbData BreadCrumbs[MAX_CRUMBS];
uint16 NumCrumbs;
bool DroppingCrumbs;
bool FollowingCrumbs;
bool FollowCrumbsFast;
bool IsCrumbTurning;
uint32 CrumbSecs, CrumbMicros;
bool IsCrumbWaiting;

int     followCrumbs();
void    mayShowCrumbIndicator();
void    mayShowCrumbIndicatorOff();

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
void drawPanel() {
	mouseHide();

	setAPen(3);                 /* Clear Area */
	rectFill(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319), VGAScaleY(199));

	setAPen(0);                 /* First Line */
	drawHLine(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319));
	setAPen(5);                 /* Second Line */
	drawHLine(0, VGAScaleY(149) + 1 + SVGACord(2), VGAScaleX(319));

	/* Gadget Seperators */
	setAPen(0);
	drawHLine(0, VGAScaleY(170), VGAScaleX(319));     /* First black line to seperate buttons */

	if (!Alternate) {
		setAPen(4);
		drawHLine(0, VGAScaleY(170) + 1, VGAScaleX(319)); /* The horizontal lines under the black one */
		drawGadgetList(MoveGadgetList);
	} else {
		if (g_lab->getPlatform() != Common::kPlatformWindows) {
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
		if (g_lab->getPlatform() != Common::kPlatformWindows) {
			drawVLine(VGAScaleX(126), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(196), VGAScaleY(170) + 2, VGAScaleY(198));
		} else {
			drawVLine(VGAScaleX(92), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(162), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(232), VGAScaleY(170) + 2, VGAScaleY(198));
		}

		drawGadgetList(InvGadgetList);
	}

	mouseShow();
}


extern bool LastMessageLong;

static bool LastTooLong = false;




/******************************************************************************/
/* Draws the message for the room.                                            */
/******************************************************************************/
static void drawRoomMessage(uint16 CurInv, CloseDataPtr cptr) {
	if (LastTooLong) {
		LastTooLong = false;
		return;
	}

	if (Alternate) {
		if ((CurInv <= NumInv) && g_lab->_conditions->in(CurInv) && Inventory[CurInv].BInvName) {
			if ((CurInv == LAMPNUM) && g_lab->_conditions->in(LAMPON))  /* LAB: Labyrith specific */
				drawStaticMessage(kTextLampOn);
			else if (Inventory[CurInv].Many > 1) {
				Common::String roomMessage = Common::String(Inventory[CurInv].name) + "  (" + Common::String::format("%d", Inventory[CurInv].Many) + ")";
				drawMessage(roomMessage.c_str());
			} else
				drawMessage(Inventory[CurInv].name);
		}
	} else
		drawDirection(cptr);

	LastTooLong = LastMessageLong;
}


/******************************************************************************/
/* Sets up the Labyrinth screens, and opens up the initial windows.           */
/******************************************************************************/
bool setUpScreens() {
	uint16 counter;
	byte *buffer;
	byte *MovePanelBuffer, *InvPanelBuffer;
	Gadget *curgad;
	uint16 y;

	if (!createScreen(IsHiRes))
		return false;

	/* Loads in the graphics for the movement control panel */
	Common::File file;
	file.open(translateFileName("P:Control"));
	if (file.err() || file.size() == 0)
		return false;

	if (!(MovePanelBuffer = (byte *)calloc(file.size(), 1)))
		return false;

	file.read(MovePanelBuffer, file.size());
	file.close();

	buffer = MovePanelBuffer;

	for (counter = 0; counter < 20; counter++)
		readImage(buffer, &(MoveImages[counter]));

	/* Creates the gadgets for the movement control panel */
	y = VGAScaleY(173) - SVGACord(2);

	if (g_lab->getPlatform() == Common::kPlatformWindows) {
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
	if (file.err() || file.size() == 0)
		return false;

	if (!(InvPanelBuffer = (byte *)calloc(file.size(), 1)))
		return false;

	file.read(InvPanelBuffer, file.size());
	file.close();

	buffer = InvPanelBuffer;

	if (g_lab->getPlatform() == Common::kPlatformWindows) {
		for (counter = 0; counter < 10; counter++)
			readImage(buffer, &(InvImages[counter]));

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
		for (counter = 0; counter < 6; counter++)
			readImage(buffer, &(InvImages[counter]));

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
static void perFlipGadget(uint16 GadID) {
	Image *Temp;
	Gadget *TopGad;

	TopGad = MoveGadgetList;

	while (TopGad) {
		if (TopGad->GadgetID == GadID) {
			Temp = TopGad->Im;
			TopGad->Im = TopGad->ImAlt;
			TopGad->ImAlt = Temp;

			if (!Alternate) {
				mouseHide();
				drawImage(TopGad->Im, TopGad->x, TopGad->y);
				mouseShow();
			}

			return;
		} else
			TopGad = TopGad->NextGadget;
	}
}



/******************************************************************************/
/* Eats all the available messages.                                           */
/******************************************************************************/
void eatMessages() {
	IntuiMessage *Msg;

	do {
		Msg = getMsg();
	} while (Msg);

	return;
}

/******************************************************************************/
/* Checks whether the close up is one of the special case closeups.           */
/******************************************************************************/
static bool doCloseUp(CloseDataPtr cptr) {
	if (cptr == NULL)
		return false;

	int monltmargin, monrtmargin, montopmargin, lutertmargin;

	if (g_lab->getPlatform() != Common::kPlatformWindows) {
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
		nopalchange = true;
		readPict(CurFileName, false);
		nopalchange = false;
		doWestPaper();
	}

	else if (CurInv == NOTESNUM) {
		CurFileName = Inventory[CurInv].BInvName;
		nopalchange = true;
		readPict(CurFileName, false);
		nopalchange = false;
		doNotes();
	}

	return Inventory[CurInv].BInvName;
}




static bool interfaceisoff = false;


/******************************************************************************/
/* Turns the interface off.                                                   */
/******************************************************************************/
static void interfaceOff() {
	if (!interfaceisoff) {
		attachGadgetList(NULL);
		mouseHide();
		interfaceisoff = true;
	}
}




/******************************************************************************/
/* Turns the interface on.                                                    */
/******************************************************************************/
static void interfaceOn() {
	if (interfaceisoff) {
		interfaceisoff = false;

		mouseShow();
	}

	if (LongWinInFront)
		attachGadgetList(NULL);
	else if (Alternate)
		attachGadgetList(InvGadgetList);
	else
		attachGadgetList(MoveGadgetList);
}



static const char *Test;


/******************************************************************************/
/* If the user hits the "Use" gadget; things that can get used on themselves. */
/******************************************************************************/
static bool doUse(uint16 CurInv) {

	if (CurInv == MAPNUM) {                  /* LAB: Labyrinth specific */
		drawStaticMessage(kTextUseMap);
		interfaceOff();
		stopDiff();
		CurFileName = " ";
		CPtr = NULL;
		doMap(RoomNum);
		VGASetPal(initcolors, 8);
		drawMessage(NULL);
		drawPanel();
	}

	else if (CurInv == JOURNALNUM) {         /* LAB: Labyrinth specific */
		drawStaticMessage(kTextUseJournal);
		interfaceOff();
		stopDiff();
		CurFileName = " ";
		CPtr = NULL;
		doJournal();
		drawPanel();
		drawMessage(NULL);
	}

	else if (CurInv == LAMPNUM) {            /* LAB: Labyrinth specific */
		interfaceOff();

		if (g_lab->_conditions->in(LAMPON)) {
			drawStaticMessage(kTextTurnLampOff);
			g_lab->_conditions->exclElement(LAMPON);
		} else {
			drawStaticMessage(kTextTurnLampOn);
			g_lab->_conditions->inclElement(LAMPON);
		}

		DoBlack = false;
		waitForEffect = true;
		readPict("Music:Click", true);
		waitForEffect = false;

		DoBlack = false;
		Test = getInvName(CurInv);
	}

	else if (CurInv == BELTNUM) {                    /* LAB: Labyrinth specific */
		if (!g_lab->_conditions->in(BELTGLOW))
			g_lab->_conditions->inclElement(BELTGLOW);

		DoBlack = false;
		Test = getInvName(CurInv);
	}

	else if (CurInv == WHISKEYNUM) {                 /* LAB: Labyrinth specific */
		g_lab->_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUseWhiskey);
	}

	else if (CurInv == PITHHELMETNUM) {              /* LAB: Labyrinth specific */
		g_lab->_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUsePith);
	}

	else if (CurInv == HELMETNUM) {                  /* LAB: Labyrinth specific */
		g_lab->_conditions->inclElement(USEDHELMET);
		drawStaticMessage(kTextUseHelmet);
	}

	else
		return false;

	return true;
}




/******************************************************************************/
/* Decrements the current inventory number.                                   */
/******************************************************************************/
static void decIncInv(uint16 *CurInv, bool dec) {
	interfaceOff();

	if (dec)
		(*CurInv)--;
	else
		(*CurInv)++;

	while (*CurInv && (*CurInv <= NumInv)) {
		if (g_lab->_conditions->in(*CurInv) && Inventory[*CurInv].BInvName) {
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
			if (g_lab->_conditions->in(*CurInv) && Inventory[*CurInv].BInvName) {
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
static void mainGameLoop() {
	IntuiMessage *Msg;
	uint32 Class;

	uint16 Code = 0, Qualifier, MouseX, MouseY, ActionMode = 4;
	uint16 CurInv = MAPNUM, LastInv = MAPNUM, Old;

	bool ForceDraw = false, doit, GotMessage = true;

	uint16 OldRoomNum, OldDirection = 0, GadID = 0, NewDir;

	CloseDataPtr OldCPtr, TempCPtr, HCPtr = NULL;
	ViewData *VPtr;

	VGASetPal(initcolors, 8);

	CPtr    = NULL;
	RoomNum = 1;
	Direction = NORTH;

	g_resource->readRoomData("LAB:Doors");
	g_resource->readInventory("LAB:Inventor");

	if (!(g_lab->_conditions = new LargeSet(HighestCondition + 1)))
		return;

	if (!(g_lab->_roomsFound = new LargeSet(ManyRooms + 1)))
		return;

	g_lab->_conditions->readInitialConditions("LAB:Conditio");

	LongWinInFront = false;
	drawPanel();

	perFlipGadget(ActionMode);

	/* Set up initial picture. */

	while (1) {
		WSDL_ProcessInput(1);

		if (GotMessage) {
			if (QuitLab || g_engine->shouldQuit()) {
				stopDiff();
				break;
			}

			g_music->resumeBackMusic();

			/* Sees what kind of close up we're in and does the appropriate stuff, if any. */
			if (doCloseUp(CPtr)) {
				CPtr = NULL;

				mayShowCrumbIndicator();
				WSDL_UpdateScreen();
			}

			/* Sets the current picture properly on the screen */
			if (MainDisplay)
				Test = getPictName(&CPtr);

			if (noupdatediff) {
				g_lab->_roomsFound->inclElement(RoomNum); /* Potentially entered another room */
				ForceDraw = (strcmp(Test, CurFileName) != 0) || ForceDraw;

				noupdatediff = false;
				CurFileName = Test;
			}

			else if (strcmp(Test, CurFileName) != 0) {
				interfaceOff();
				g_lab->_roomsFound->inclElement(RoomNum); /* Potentially entered another room */
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

				drawRoomMessage(CurInv, CPtr);
				ForceDraw = false;

				mayShowCrumbIndicator();
				WSDL_UpdateScreen();

				if (!FollowingCrumbs)
					eatMessages();
			}

			if (ForceDraw) {
				drawRoomMessage(CurInv, CPtr);
				ForceDraw = false;
				WSDL_UpdateScreen();
			}
		}

		g_music->updateMusic();  /* Make sure we check the music at least after every message */
		interfaceOn();
		Msg = getMsg();

		if (Msg == NULL) { /* Does music load and next animation frame when you've run out of messages */
			GotMessage = false;
			g_music->checkRoomMusic();
			g_music->updateMusic();
			diffNextFrame();

			if (FollowingCrumbs) {
				int result = followCrumbs();

				if (result != 0) {
					int x = 0, y = 0;

					WSDL_GetMousePos(&x, &y);

					Class     = GADGETUP;
					Qualifier = 0;
					MouseX    = x;
					MouseY    = y;

					if (result == VKEY_UPARROW) {
						Code = GadID = 7;
					} else if (result == VKEY_LTARROW) {
						Code = GadID = 6;
					} else if (result == VKEY_RTARROW) {
						Code = GadID = 8;
					}

					GotMessage = true;
					mayShowCrumbIndicator();
					WSDL_UpdateScreen();
					goto from_crumbs;
				}
			}

			mayShowCrumbIndicator();
			WSDL_UpdateScreen();
		} else {
			GotMessage = true;

			Class     = Msg->Class;
			Code      = Msg->Code;
			Qualifier = Msg->Qualifier;
			MouseX    = Msg->MouseX;
			MouseY    = Msg->MouseY;
			GadID     = Msg->GadgetID;

			FollowingCrumbs = false;

from_crumbs:
			DoBlack = false;

			if ((Class == RAWKEY) && (!LongWinInFront)) {
				if (Code == 13) { /* The return key */
					Class     = MOUSEBUTTONS;
					Qualifier = IEQUALIFIER_LEFTBUTTON;
					mouseXY(&MouseX, &MouseY);
				} else if (g_lab->getPlatform() == Common::kPlatformWindows && 
						(Code == 'b' || Code == 'B')) {  /* Start bread crumbs */
					BreadCrumbs[0].RoomNum = 0;
					NumCrumbs = 0;
					DroppingCrumbs = true;
					mayShowCrumbIndicator();
					WSDL_UpdateScreen();
				} else if (Code == 'f' || Code == 'F' ||
				         Code == 'r' || Code == 'R') {  /* Follow bread crumbs */
					if (DroppingCrumbs) {
						if (NumCrumbs > 0) {
							FollowingCrumbs = true;
							FollowCrumbsFast = (Code == 'r' || Code == 'R');
							IsCrumbTurning = false;
							IsCrumbWaiting = false;
							getTime(&CrumbSecs, &CrumbMicros);

							if (Alternate) {
								eatMessages();
								Alternate = false;
								DoBlack = true;
								DoNotDrawMessage = false;

								MainDisplay = true;
								interfaceOn(); /* Sets the correct gadget list */
								drawPanel();
								drawRoomMessage(CurInv, CPtr);
								WSDL_UpdateScreen();
							}
						} else {
							BreadCrumbs[0].RoomNum = 0;
							DroppingCrumbs = false;

							// Need to hide indicator!!!!
							mayShowCrumbIndicatorOff();
							WSDL_UpdateScreen();
						}
					}
				} else if ((Code == 315) || (Code == 'x') || (Code == 'X')
				         || (Code == 'q') || (Code == 'Q')) {  /* Quit? */
					DoNotDrawMessage = false;
					drawMessage("Do you want to quit? (Y/N)");
					doit = false;
					eatMessages();
					interfaceOff();

					while (1) {
						g_music->updateMusic();  /* Make sure we check the music at least after every message */
						Msg = getMsg();

						if (Msg == NULL) { /* Does music load and next animation frame when you've run out of messages */
							g_music->updateMusic();
							diffNextFrame();
						} else {
							if (Msg->Class == RAWKEY) {
								if ((Msg->Code == 'Y') || (Msg->Code == 'y') || (Msg->Code == 'Q') || (Msg->Code == 'q')) {
									doit = true;
									break;
								} else if (Msg->Code < 128) {
									break;
								}
							} else if (Msg->Class == MOUSEBUTTONS) {
								break;
							}
						}
					}

					if (doit) {
						stopDiff();
						break;
					} else {
						ForceDraw = true;
						interfaceOn();
					}
				} else if (Code == 9) { /* TAB key */
					Class = DELTAMOVE;
				} else if (Code == 27) { /* ESC key */
					CPtr = NULL;
				}

				eatMessages();
			}

			if (LongWinInFront) {
				if ((Class == RAWKEY) ||
				        ((Class == MOUSEBUTTONS) &&
				         ((IEQUALIFIER_LEFTBUTTON & Qualifier) ||
				          (IEQUALIFIER_RBUTTON & Qualifier)))) {
					LongWinInFront = false;
					DoNotDrawMessage = false;
					drawPanel();
					drawRoomMessage(CurInv, CPtr);
					WSDL_UpdateScreen();
				}
			} else if ((Class == GADGETUP) && !Alternate) {
				if (GadID <= 5) {
					if ((ActionMode == 4) && (GadID == 4) && (CPtr != NULL)) {
						doMainView(&CPtr);

						DoBlack = true;
						HCPtr = NULL;
						CPtr = NULL;
						mayShowCrumbIndicator();
						WSDL_UpdateScreen();
					} else if (GadID == 5) {
						eatMessages();

						Alternate = true;
						DoBlack = true;
						DoNotDrawMessage = false;
						interfaceOn(); /* Sets the correct gadget list */

						MainDisplay = false;

						if (LastInv && g_lab->_conditions->in(LastInv)) {
							CurInv = LastInv;
							Test = getInvName(CurInv);
						} else
							decIncInv(&CurInv, false);

						drawPanel();
						drawRoomMessage(CurInv, CPtr);

						mayShowCrumbIndicator();
						WSDL_UpdateScreen();
					} else {
						Old        = ActionMode;
						ActionMode = GadID;

						if (Old < 5)
							perFlipGadget(Old);

						perFlipGadget(ActionMode);

						if (GadID == 0)
							drawStaticMessage(kTextTakeWhat);
						else if (GadID == 1)
							drawStaticMessage(kTextMoveWhat);
						else if (GadID == 2)
							drawStaticMessage(kTextOpenWhat);
						else if (GadID == 3)
							drawStaticMessage(kTextCloseWhat);
						else if (GadID == 4)
							drawStaticMessage(kTextLookWhat);

						WSDL_UpdateScreen();
					}
				} else if (GadID == 9) {
					doUse(MAPNUM);

					mayShowCrumbIndicator();
					WSDL_UpdateScreen();
				} else if (GadID >= 6) { /* Arrow Gadgets */
					CPtr = NULL;
					HCPtr = NULL;

					if ((GadID == 6) || (GadID == 8)) {
						if (GadID == 6)
							drawStaticMessage(kTextTurnLeft);
						else
							drawStaticMessage(kTextTurnRight);

						CurFileName = " ";

						OldDirection = Direction;

						NewDir = Direction;
						processArrow(&NewDir, GadID - 6);
						doTurn(Direction, NewDir, &CPtr);
						DoBlack = true;
						Direction = NewDir;
						ForceDraw = true;

						mayShowCrumbIndicator();
						WSDL_UpdateScreen();
					} else if (GadID == 7) {
						OldRoomNum = RoomNum;

						if (doGoForward(&CPtr)) {
							if (OldRoomNum == RoomNum)
								DoBlack = true;
						} else {
							DoBlack = true;
							processArrow(&Direction, GadID - 6);

							if (OldRoomNum != RoomNum) {
								drawStaticMessage(kTextGoForward);
								g_lab->_roomsFound->inclElement(RoomNum); /* Potentially entered a new room */
								CurFileName = " ";
								ForceDraw = true;
							} else {
								DoBlack = true;
								drawStaticMessage(kTextNoPath);
							}
						}

						if (FollowingCrumbs) {
							if (IsCrumbTurning) {
								if (Direction == OldDirection) {
									FollowingCrumbs = false;
								}
							} else {
								if (RoomNum == OldRoomNum) { // didn't get there?
									FollowingCrumbs = false;
								}
							}
						} else if (DroppingCrumbs && OldRoomNum != RoomNum) {
							// If in surreal maze, turn off DroppingCrumbs.
							// Note: These numbers were generated by parsing the
							// "Maps" file, which is why they are hard-coded. Bleh!
							if (RoomNum >= 245 && RoomNum <= 280) {
								FollowingCrumbs = false;
								DroppingCrumbs = false;
								NumCrumbs = 0;
								BreadCrumbs[0].RoomNum = 0;
							} else {
								bool intersect = false;
								int i;

								for (i = 0; i < NumCrumbs; i++) {
									if (BreadCrumbs[i].RoomNum == RoomNum) {
										NumCrumbs = i + 1;
										BreadCrumbs[NumCrumbs].RoomNum = 0;
										intersect = true;
									}
								}

								if (!intersect) {
									if (NumCrumbs == MAX_CRUMBS) {
										NumCrumbs = MAX_CRUMBS - 1;
										memcpy(&BreadCrumbs[0], &BreadCrumbs[1], NumCrumbs * sizeof BreadCrumbs[0]);
									}

									BreadCrumbs[NumCrumbs].RoomNum = RoomNum;
									BreadCrumbs[NumCrumbs++].Direction = Direction;
								}
							}
						}

						mayShowCrumbIndicator();
						WSDL_UpdateScreen();
					}
				}
			} else if ((Class == GADGETUP) && Alternate) {
				DoBlack = true;

				if (GadID == 0) {
					eatMessages();
					Alternate = false;
					DoBlack = true;
					DoNotDrawMessage = false;

					MainDisplay = true;
					interfaceOn(); /* Sets the correct gadget list */
					drawPanel();
					drawRoomMessage(CurInv, CPtr);

					WSDL_UpdateScreen();
				}

				GadID--;

				if (GadID == 0) {
					interfaceOff();
					stopDiff();
					CurFileName = " ";

					doit = !saveRestoreGame();
					CPtr = NULL;

					MainDisplay = true;

					CurInv = MAPNUM;
					LastInv = MAPNUM;

					Test = getInvName(CurInv);

					drawPanel();

					if (doit) {
						drawMessage("Disk operation failed.");
						VGASetPal(initcolors, 8);

						WSDL_UpdateScreen();

						g_system->delayMillis(1000);
					} else {
						WSDL_UpdateScreen();
					}
				} else if (GadID == 1) {
					if (!doUse(CurInv)) {
						Old        = ActionMode;
						ActionMode = 5;  /* Use button */

						if (Old < 5)
							perFlipGadget(Old);

						drawStaticMessage(kTextUseOnWhat);
						MainDisplay = true;

						WSDL_UpdateScreen();
					}
				} else if (GadID == 2) {
					MainDisplay = !MainDisplay;

					if ((CurInv == 0) || (CurInv > NumInv)) {
						CurInv = 1;

						while ((CurInv <= NumInv) && (!g_lab->_conditions->in(CurInv)))
							CurInv++;
					}

					if ((CurInv <= NumInv) && g_lab->_conditions->in(CurInv) &&
					        Inventory[CurInv].BInvName)
						Test = getInvName(CurInv);

					WSDL_UpdateScreen();
				} else if (GadID == 3) { /* Left gadget */
					decIncInv(&CurInv, true);
					LastInv = CurInv;
					DoNotDrawMessage = false;
					drawRoomMessage(CurInv, CPtr);

					WSDL_UpdateScreen();
				} else if (GadID == 4) { /* Right gadget */
					decIncInv(&CurInv, false);
					LastInv = CurInv;
					DoNotDrawMessage = false;
					drawRoomMessage(CurInv, CPtr);

					WSDL_UpdateScreen();
				} else if (GadID == 5) { /* bread crumbs */
					BreadCrumbs[0].RoomNum = 0;
					NumCrumbs = 0;
					DroppingCrumbs = true;
					mayShowCrumbIndicator();
					WSDL_UpdateScreen();
				} else if (GadID == 6) { /* follow crumbs */
					if (DroppingCrumbs) {
						if (NumCrumbs > 0) {
							FollowingCrumbs = true;
							FollowCrumbsFast = false;
							IsCrumbTurning = false;
							IsCrumbWaiting = false;
							getTime(&CrumbSecs, &CrumbMicros);

							eatMessages();
							Alternate = false;
							DoBlack = true;
							DoNotDrawMessage = false;

							MainDisplay = true;
							interfaceOn(); /* Sets the correct gadget list */
							drawPanel();
							drawRoomMessage(CurInv, CPtr);
							WSDL_UpdateScreen();
						} else {
							BreadCrumbs[0].RoomNum = 0;
							DroppingCrumbs = false;

							// Need to hide indicator!!!!
							mayShowCrumbIndicatorOff();
							WSDL_UpdateScreen();
						}
					}
				}
			} else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier) && MainDisplay) {
				interfaceOff();
				MainDisplay = true;

				doit = false;

				if (CPtr) {
					if ((CPtr->CloseUpType == SPECIALLOCK) && MainDisplay) /* LAB: Labrinth specific code */
						mouseCombination(MouseX, MouseY);
					else if ((CPtr->CloseUpType == SPECIALBRICK) && MainDisplay)
						mouseTile(MouseX, MouseY);
					else
						doit = true;
				} else
					doit = true;


				if (doit) {
					HCPtr = NULL;
					eatMessages();

					if (ActionMode == 0) { /* Take something. */
						if (doActionRule(MouseX, MouseY, ActionMode, RoomNum, &CPtr))
							CurFileName = NewFileName;
						else if (takeItem(MouseX, MouseY, &CPtr))
							drawStaticMessage(kTextTakeItem);
						else if (doActionRule(MouseX, MouseY, TAKEDEF - 1, RoomNum, &CPtr))
							CurFileName = NewFileName;
						else if (doActionRule(MouseX, MouseY, TAKE - 1, 0, &CPtr))
							CurFileName = NewFileName;
						else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
							drawStaticMessage(kTextNothing);
					} else if ((ActionMode == 1) /* Manipulate an object */  ||
					         (ActionMode == 2) /* Open up a "door" */      ||
					         (ActionMode == 3)) { /* Close a "door" */
						if (doActionRule(MouseX, MouseY, ActionMode, RoomNum, &CPtr))
							CurFileName = NewFileName;
						else if (!doActionRule(MouseX, MouseY, ActionMode, 0, &CPtr)) {
							if (MouseY < (VGAScaleY(149) + SVGACord(2)))
								drawStaticMessage(kTextNothing);
						}
					} else if (ActionMode == 4) { /* Look at closeups */
						TempCPtr = CPtr;
						setCurClose(MouseX, MouseY, &TempCPtr);

						if (CPtr == TempCPtr) {
							if (MouseY < (VGAScaleY(149) + SVGACord(2)))
								drawStaticMessage(kTextNothing);
						} else if (TempCPtr->GraphicName) {
							if (*(TempCPtr->GraphicName)) {
								DoBlack = true;
								CPtr = TempCPtr;
							} else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
								drawStaticMessage(kTextNothing);
						} else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
							drawStaticMessage(kTextNothing);
					} else if ((ActionMode == 5)  &&
					         g_lab->_conditions->in(CurInv)) { /* Use an item on something else */
						if (doOperateRule(MouseX, MouseY, CurInv, &CPtr)) {
							CurFileName = NewFileName;

							if (!g_lab->_conditions->in(CurInv))
								decIncInv(&CurInv, false);
						} else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
							drawStaticMessage(kTextNothing);
					}
				}

				mayShowCrumbIndicator();
				WSDL_UpdateScreen();
			} else if (Class == DELTAMOVE) {
				VPtr = getViewData(RoomNum, Direction);
				OldCPtr = VPtr->closeUps;

				if (HCPtr == NULL) {
					TempCPtr = CPtr;
					setCurClose(MouseX, MouseY, &TempCPtr);

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
					mouseMove(scaleX((HCPtr->x1 + HCPtr->x2) / 2), scaleY((HCPtr->y1 + HCPtr->y2) / 2));
			} else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) {
				eatMessages();
				Alternate = !Alternate;
				DoBlack = true;
				DoNotDrawMessage = false;
				MainDisplay = true;
				interfaceOn(); /* Sets the correct gadget list */

				if (Alternate) {
					if (LastInv && g_lab->_conditions->in(LastInv))
						CurInv = LastInv;
					else
						decIncInv(&CurInv, false);
				}

				drawPanel();
				drawRoomMessage(CurInv, CPtr);

				mayShowCrumbIndicator();
				WSDL_UpdateScreen();
			}
		}
	}

	delete g_lab->_conditions;
	delete g_lab->_roomsFound;

	if (Rooms)
		free(Rooms);

	if (Inventory) {
		for (Code = 1; Code <= NumInv; Code++) {
			if (Inventory[Code].name)
				free(Inventory[Code].name);

			if (Inventory[Code].BInvName)
				free(Inventory[Code].BInvName);
		}

		free(Inventory);
	}
}


void LabEngine::go() {
	bool mem, dointro = false;
	uint16 counter;

	dointro = true;

	IsHiRes = ((getFeatures() & GF_LOWRES) == 0);

#if 0
	if (IsHiRes)
		warning("Running in HiRes mode");
	else
		warning("Running in LowRes mode");
#endif
	if (initBuffer(BUFFERSIZE, true)) {
		mem = true;
	} else {
		warning("initBuffer() failed");
		return;
	}

	if (!setUpScreens()) {
		IsHiRes = false;
		mem = mem && setUpScreens();
	}

	initMouse();

	mem = mem && initRoomBuffer();

	if (!dointro)
		g_music->initMusic();

	MsgFont = g_resource->getFont("P:AvanteG.12");

	mouseHide();

	if (dointro && mem) {
		introSequence();
	} else
		DoBlack = true;

	if (mem) {
		mouseShow();
		mainGameLoop();
	} else
		debug("\n\nNot enough memory to start game.\n\n");

	if (QuitLab) { /* Won the game */
		blackAllScreen();
		readPict("P:End/L2In.1", true);

		for (counter = 0; counter < 120; counter++) {
			g_music->updateMusic();
			waitTOF();
		}

		readPict("P:End/L2In.9", true);
		readPict("P:End/Lost", true);

		warning("STUB: waitForPress");
		while (!1) { // 1 means ignore SDL_ProcessInput calls
			g_music->updateMusic();
			diffNextFrame();
			waitTOF();
		}
	}

	closeFont(MsgFont);

	freeRoomBuffer();
	freeBuffer();

	g_music->freeMusic();
}

/*****************************************************************************/
/* New code to allow quick(er) return navigation in game.                    */
/*****************************************************************************/
int followCrumbs() {
	// NORTH, SOUTH, EAST, WEST
	static int movement[4][4] = {
		{ VKEY_UPARROW, VKEY_RTARROW, VKEY_RTARROW, VKEY_LTARROW },
		{ VKEY_RTARROW, VKEY_UPARROW, VKEY_LTARROW, VKEY_RTARROW },
		{ VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW, VKEY_RTARROW },
		{ VKEY_RTARROW, VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW }
	};

	int ExitDir;
	int MoveDir;

	if (IsCrumbWaiting) {
		uint32 Secs;
		uint32 Micros;

		timeDiff(CrumbSecs, CrumbMicros, &Secs, &Micros);

		if (Secs != 0 || Micros != 0)
			return 0;

		IsCrumbWaiting = false;
	}

	if (!IsCrumbTurning)
		BreadCrumbs[NumCrumbs--].RoomNum = 0;

	// Is the current crumb this room? If not, logic error.
	if (RoomNum != BreadCrumbs[NumCrumbs].RoomNum) {
		NumCrumbs = 0;
		BreadCrumbs[0].RoomNum = 0;
		DroppingCrumbs = false;
		FollowingCrumbs = false;
		return 0;
	}

	// which direction is last crumb
	if (BreadCrumbs[NumCrumbs].Direction == EAST)
		ExitDir = WEST;
	else if (BreadCrumbs[NumCrumbs].Direction == WEST)
		ExitDir = EAST;
	else if (BreadCrumbs[NumCrumbs].Direction == NORTH)
		ExitDir = SOUTH;
	else
		ExitDir = NORTH;

	MoveDir = movement[Direction][ExitDir];

	if (NumCrumbs == 0) {
		IsCrumbTurning = false;
		BreadCrumbs[0].RoomNum = 0;
		DroppingCrumbs = false;
		FollowingCrumbs = false;
	} else {
		int theDelay = (FollowCrumbsFast ? ONESECOND / 4 : ONESECOND);

		IsCrumbTurning = (MoveDir != VKEY_UPARROW);
		IsCrumbWaiting = true;

		addCurTime(theDelay / ONESECOND, theDelay % ONESECOND, &CrumbSecs, &CrumbMicros);
	}

	return MoveDir;
}

byte dropCrumbs[] = { 0x00 };
byte dropCrumbsOff[] = { 0x00 };

Image DropCrumbsImage = { 24, 24, dropCrumbs };
Image DropCrumbsOffImage = { 24, 24, dropCrumbsOff };

void mayShowCrumbIndicator() {
	if (g_lab->getPlatform() != Common::kPlatformWindows)
		return;

	if (DroppingCrumbs && MainDisplay) {
		mouseHide();
		drawMaskImage(&DropCrumbsImage, 612, 4);
		mouseShow();
	}
}

void mayShowCrumbIndicatorOff() {
	if (g_lab->getPlatform() != Common::kPlatformWindows)
		return;

	if (MainDisplay) {
		mouseHide();
		drawMaskImage(&DropCrumbsOffImage, 612, 4);
		mouseShow();
	}
}

} // End of namespace Lab
