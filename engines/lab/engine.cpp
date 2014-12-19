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

#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/diff.h"
#include "lab/vga.h"
#include "lab/timing.h"
#include "lab/text.h"
#include "lab/storage.h"
#include "lab/parsefun.h"
#include "lab/interface.h"
#include "lab/mouse.h"

namespace Lab {

const char *CurFileName = " ";

bool LongWinInFront = false;

struct TextFont *MsgFont;

extern bool DoBlack, waiteffect, EffectPlaying, stopsound, DoNotDrawMessage, IsHiRes, nopalchange, DoMusic;

/* Global parser data */

extern RoomData *Rooms;
extern InventoryData *Inventory;
extern uint16 NumInv, RoomNum, ManyRooms, HighestCondition, Direction;
extern LargeSet Conditions, RoomsFound;
CloseDataPtr CPtr;

#if !defined(DOSCODE)
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
#endif

bool Alternate = false, ispal = false, noupdatediff = false, MainDisplay = true, QuitLab = false, DoNotReset = false;

extern char *NewFileName;  /* When ProcessRoom.c decides to change the filename
                              of the current picture. */

extern char *LAMPONMSG, *TURNLEFT, *TURNRIGHT;
extern char *GOFORWARDDIR, *NOPATH, *TAKEITEM, *USEONWHAT, *TAKEWHAT, *MOVEWHAT, *OPENWHAT, *CLOSEWHAT, *LOOKWHAT, *NOTHING, *USEMAP, *USEJOURNAL, *TURNLAMPON, *TURNLAMPOFF, *USEWHISKEY, *USEPITH, *USEHELMET;


#define BIGBUFFERSIZE 850000L
#define SMALLBUFFERSIZE 250000L

static uint32 BUFFERSIZE = BIGBUFFERSIZE;


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


#define CLOWNROOM           123
#define DIMROOM              80



static byte *MovePanelBuffer, *InvPanelBuffer;
static uint32 MovePanelBufferSize, InvPanelBufferSize;
static struct Image *MoveImages[20],
#if defined(DOSCODE)
		*InvImages[6];
#else
		*InvImages[10];
#endif
static struct Gadget *MoveGadgetList, *InvGadgetList;


static char initcolors[] = { '\x00', '\x00', '\x00', '\x30',
							 '\x30', '\x30', '\x10', '\x10',
							 '\x10', '\x14', '\x14', '\x14',
							 '\x20', '\x20', '\x20', '\x24',
							 '\x24', '\x24', '\x2c', '\x2c',
							 '\x2c', '\x08', '\x08', '\x08'};




/******************************************************************************/
/* Draws the control panel display.                                           */
/******************************************************************************/
void drawPanel(void) {
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
#if defined(DOSCODE)
		drawVLine(VGAScaleX(124), VGAScaleY(170) + 1, VGAScaleY(199)); /* Vertical Black lines */
		drawVLine(VGAScaleX(194), VGAScaleY(170) + 1, VGAScaleY(199));
#else
		drawVLine(VGAScaleX(90), VGAScaleY(170) + 1, VGAScaleY(199));  /* Vertical Black lines */
		drawVLine(VGAScaleX(160), VGAScaleY(170) + 1, VGAScaleY(199));
		drawVLine(VGAScaleX(230), VGAScaleY(170) + 1, VGAScaleY(199));
#endif

		setAPen(4);
		drawHLine(0, VGAScaleY(170) + 1, VGAScaleX(122));   /* The horizontal lines under the black one */
		drawHLine(VGAScaleX(126), VGAScaleY(170) + 1, VGAScaleX(192));
		drawHLine(VGAScaleX(196), VGAScaleY(170) + 1, VGAScaleX(319));

		drawVLine(VGAScaleX(1), VGAScaleY(170) + 2, VGAScaleY(198)); /* The vertical high light lines */
#if defined(DOSCODE)
		drawVLine(VGAScaleX(126), VGAScaleY(170) + 2, VGAScaleY(198));
		drawVLine(VGAScaleX(196), VGAScaleY(170) + 2, VGAScaleY(198));
#else
		drawVLine(VGAScaleX(92), VGAScaleY(170) + 2, VGAScaleY(198));
		drawVLine(VGAScaleX(162), VGAScaleY(170) + 2, VGAScaleY(198));
		drawVLine(VGAScaleX(232), VGAScaleY(170) + 2, VGAScaleY(198));
#endif

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
	char text[250], ManyText[8], *ManyPtr;

	if (LastTooLong) {
		LastTooLong = false;
		return;
	}

	if (Alternate) {
		if ((CurInv <= NumInv) && In(Conditions, CurInv) && Inventory[CurInv].BInvName) {
			if ((CurInv == LAMPNUM) && In(Conditions, LAMPON))  /* LAB: Labyrith specific */
				drawMessage(LAMPONMSG);
			else if (Inventory[CurInv].Many > 1) {
				ManyPtr = numtostr(ManyText, Inventory[CurInv].Many);
				strcpy(text, Inventory[CurInv].name);
				strcat(text, "  (");
				strcat(text, ManyPtr);
				strcat(text, ")");
				drawMessage(text);
			} else
				drawMessage(Inventory[CurInv].name);
		}
	} else
		drawDirection(cptr);

	LastTooLong = LastMessageLong;
}


static uint16 OldMode;

/******************************************************************************/
/* Sets up the Labyrinth screens, and opens up the initial windows.           */
/******************************************************************************/
bool setUpScreens(void) {
	uint16 counter;
	byte *bufferstorage, **buffer = &bufferstorage;
	struct Gadget *curgad;
	uint16 y;

	if (!createScreen(IsHiRes))
		return false;

	/* Loads in the graphics for the movement control panel */
	MovePanelBufferSize = sizeOfFile("P:Control");

	if (MovePanelBufferSize == 0L)
		return false;

	if (!allocate((void **) &MovePanelBuffer, MovePanelBufferSize))
		return false;

	Common::File *file = openPartial("P:Control");

	if (!file)
		return false;

	file->read(MovePanelBuffer, MovePanelBufferSize);
	file->close();

	*buffer = MovePanelBuffer;

	for (counter = 0; counter < 20; counter++)
		readImage(buffer, &(MoveImages[counter]));

	/* Creates the gadgets for the movement control panel */
	y = VGAScaleY(173) - SVGACord(2);
#if !defined(DOSCODE)
	MoveGadgetList = createButton(VGAScaleX(1), y, 0, 't', MoveImages[0], MoveImages[1]);
	curgad = MoveGadgetList;
	curgad->NextGadget = createButton(VGAScaleX(33), y, 1, 'm', MoveImages[2], MoveImages[3]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(65), y, 2, 'o', MoveImages[4], MoveImages[5]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(97), y, 3, 'c', MoveImages[6], MoveImages[7]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(129), y, 4, 'l', MoveImages[8], MoveImages[9]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(161), y, 5, 'i', MoveImages[12], MoveImages[13]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(193), y, 6, VKEY_LTARROW, MoveImages[14], MoveImages[15]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(225), y, 7, VKEY_UPARROW, MoveImages[16], MoveImages[17]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(257), y, 8, VKEY_RTARROW, MoveImages[18], MoveImages[19]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(289), y, 9, 'p', MoveImages[10], MoveImages[11]);
#else
	MoveGadgetList = createButton(VGAScaleX(1), y, 0, MoveImages[0], MoveImages[1]);
	curgad = MoveGadgetList;
	curgad->NextGadget = createButton(VGAScaleX(33), y, 1, MoveImages[2], MoveImages[3]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(65), y, 2, MoveImages[4], MoveImages[5]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(97), y, 3, MoveImages[6], MoveImages[7]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(129), y, 4, MoveImages[8], MoveImages[9]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(161), y, 5, MoveImages[12], MoveImages[13]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(193), y, 6, MoveImages[14], MoveImages[15]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(225), y, 7, MoveImages[16], MoveImages[17]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(257), y, 8, MoveImages[18], MoveImages[19]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(289), y, 9, MoveImages[10], MoveImages[11]);
#endif

	InvPanelBufferSize = sizeOfFile("P:Inv");

	if (InvPanelBufferSize == 0L)
		return false;

	if (!allocate((void **) &InvPanelBuffer, InvPanelBufferSize))
		return false;

	file = openPartial("P:Inv");

	if (!file)
		return false;

	file->read(InvPanelBuffer, InvPanelBufferSize);
	file->close();

	*buffer = InvPanelBuffer;
#if !defined(DOSCODE)

	for (counter = 0; counter < 10; counter++)
		readImage(buffer, &(InvImages[counter]));

	InvGadgetList = createButton(VGAScaleX(24), y, 0, 'm', InvImages[0], InvImages[1]);
	curgad = InvGadgetList;
	curgad->NextGadget = createButton(VGAScaleX(56), y, 1, 'g', InvImages[2], InvImages[3]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(94), y, 2, 'u', InvImages[4], InvImages[5]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(126), y, 3, 'l', MoveImages[8], MoveImages[9]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(164), y, 4, VKEY_LTARROW, MoveImages[14], MoveImages[15]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(196), y, 5, VKEY_RTARROW, MoveImages[18], MoveImages[19]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(234), y, 6, 'b', InvImages[6], InvImages[7]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(266), y, 7, 'f', InvImages[8], InvImages[9]);
	curgad = curgad->NextGadget;
#else

	for (counter = 0; counter < 6; counter++)
		readImage(buffer, &(InvImages[counter]));

	InvGadgetList = createButton(VGAScaleX(58), y, 0, InvImages[0], InvImages[1]);
	curgad = InvGadgetList;
	curgad->NextGadget = createButton(VGAScaleX(90), y, 1, InvImages[2], InvImages[3]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(128), y, 2, InvImages[4], InvImages[5]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(160), y, 3, MoveImages[8], MoveImages[9]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(198), y, 4, MoveImages[14], MoveImages[15]);
	curgad = curgad->NextGadget;
	curgad->NextGadget = createButton(VGAScaleX(230), y, 5, MoveImages[18], MoveImages[19]);
	curgad = curgad->NextGadget;
#endif

	return true;
}





uint16 curmousex = 0, curmousey = 0;



/******************************************************************************/
/* Permanently flips the imagry of a gadget.                                  */
/******************************************************************************/
static void perFlipGadget(uint16 GadID) {
	struct Image *Temp;
	struct Gadget *TopGad;

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
void eatMessages(void) {
	struct IntuiMessage *Msg;

	do {
		Msg = getMsg();

		if (Msg) {
			replyMsg((void *) Msg);
		}
	} while (Msg);

	return;
}





static uint16 lastmusicroom = 1;


/******************************************************************************/
/* Checks the music that should be playing in a particular room.              */
/******************************************************************************/
static void checkRoomMusic(void) {
	if ((lastmusicroom == RoomNum) || !g_music->_musicOn)
		return;

	if (RoomNum == CLOWNROOM)
		g_music->changeMusic("Music:Laugh");
	else if (RoomNum == DIMROOM)
		g_music->changeMusic("Music:Rm81");
	else if (!DoNotReset)
		g_music->resetMusic();

	lastmusicroom = RoomNum;
}



#if defined(DOSCODE)
#define MONLTMARGIN     0
#define MONRTMARGIN     319
#define MONTOPMARGIN    0
#define LUTERTMARGIN    124
#else
#define MONLTMARGIN     2
#define MONRTMARGIN     317
#define MONTOPMARGIN    2
#define LUTERTMARGIN    128
#endif

/******************************************************************************/
/* Checks whether the close up is one of the special case closeups.           */
/******************************************************************************/
static bool doCloseUp(CloseDataPtr cptr) {
	if (cptr == NULL)
		return false;

	if ((cptr->CloseUpType == MUSEUMMONITOR) || (cptr->CloseUpType == LIBRARYMONITOR) ||
	        (cptr->CloseUpType == WINDOWMONITOR))
		doMonitor(cptr->GraphicName, cptr->Message, false, MONLTMARGIN, MONTOPMARGIN, MONRTMARGIN, 165);
	else if (cptr->CloseUpType == GRAMAPHONEMONITOR)
		doMonitor(cptr->GraphicName, cptr->Message, false, MONLTMARGIN, MONTOPMARGIN, 171, 165);
	else if (cptr->CloseUpType == UNICYCLEMONITOR)
		doMonitor(cptr->GraphicName, cptr->Message, false, 100, MONTOPMARGIN, MONRTMARGIN, 165);
	else if (cptr->CloseUpType == STATUEMONITOR)
		doMonitor(cptr->GraphicName, cptr->Message, false, 117, MONTOPMARGIN, MONRTMARGIN, 165);
	else if (cptr->CloseUpType == TALISMANMONITOR)
		doMonitor(cptr->GraphicName, cptr->Message, false, MONLTMARGIN, MONTOPMARGIN, 184, 165);
	else if (cptr->CloseUpType == LUTEMONITOR)
		doMonitor(cptr->GraphicName, cptr->Message, false, MONLTMARGIN, MONTOPMARGIN, LUTERTMARGIN, 165);
	else if (cptr->CloseUpType == CLOCKMONITOR)
		doMonitor(cptr->GraphicName, cptr->Message, false, MONLTMARGIN, MONTOPMARGIN, 206, 165);
	else if (cptr->CloseUpType == TERMINALMONITOR)
		doMonitor(cptr->GraphicName, cptr->Message, true, MONLTMARGIN, MONTOPMARGIN, MONRTMARGIN, 165);
	else
		return false;

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

	if ((CurInv == LAMPNUM) && In(Conditions, LAMPON))
		return "P:Mines/120";

	else if ((CurInv == BELTNUM) && In(Conditions, BELTGLOW))
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
static void interfaceOff(void) {
	if (!interfaceisoff) {
		attachGadgetList(NULL);
		mouseHide();
		interfaceisoff = true;
	}
}




/******************************************************************************/
/* Turns the interface on.                                                    */
/******************************************************************************/
static void interfaceOn(void) {
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
		drawMessage(USEMAP);
		interfaceOff();
		stopDiff();
		CurFileName = " ";
		CPtr = NULL;
		doMap(RoomsFound, RoomNum);
		VGASetPal(initcolors, 8);
		drawMessage(NULL);
		drawPanel();
	}

	else if (CurInv == JOURNALNUM) {         /* LAB: Labyrinth specific */
		drawMessage(USEJOURNAL);
		interfaceOff();
		stopDiff();
		CurFileName = " ";
		CPtr = NULL;
		doJournal(Conditions);
		drawPanel();
		drawMessage(NULL);
	}

	else if (CurInv == LAMPNUM) {            /* LAB: Labyrinth specific */
		interfaceOff();

		if (In(Conditions, LAMPON)) {
			drawMessage(TURNLAMPOFF);
			exclElement(Conditions, LAMPON);
		} else {
			drawMessage(TURNLAMPON);
			inclElement(Conditions, LAMPON);
		}

		DoBlack = false;
		waiteffect = true;
		readPict("Music:Click", true);
		waiteffect = false;

		DoBlack = false;
		Test = getInvName(CurInv);
	}

	else if (CurInv == BELTNUM) {                    /* LAB: Labyrinth specific */
		if (!In(Conditions, BELTGLOW))
			inclElement(Conditions, BELTGLOW);

		DoBlack = false;
		Test = getInvName(CurInv);
	}

	else if (CurInv == WHISKEYNUM) {                 /* LAB: Labyrinth specific */
		inclElement(Conditions, USEDHELMET);
		drawMessage(USEWHISKEY);
	}

	else if (CurInv == PITHHELMETNUM) {              /* LAB: Labyrinth specific */
		inclElement(Conditions, USEDHELMET);
		drawMessage(USEPITH);
	}

	else if (CurInv == HELMETNUM) {                  /* LAB: Labyrinth specific */
		inclElement(Conditions, USEDHELMET);
		drawMessage(USEHELMET);
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
		if (In(Conditions, *CurInv) && Inventory[*CurInv].BInvName) {
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
			if (In(Conditions, *CurInv) && Inventory[*CurInv].BInvName) {
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


#if defined(DOSCODE)
static bool nosvgamem = false;
static bool didintro = false;
#else
int didintro = false; // change to int do labw.c can access
#endif
static bool novesa = false, noaudio = false;

/******************************************************************************/
/* Processes user input events.                                               */
/******************************************************************************/
static void process(void) {
	struct IntuiMessage *Msg;
	uint32 Class;

	uint16 Code, Qualifier, MouseX, MouseY, ActionMode = 4, CurInv = MAPNUM, /* Lab: Labyrinth specific initialization */
	                                        LastInv = MAPNUM,     /* Lab: Labyrinth specific initialization */
	                                        Old;

	bool ForceDraw   = false, doit, GotMessage  = true;

	uint16 OldRoomNum,
#if !defined(DOSCODE)
	       OldDirection,
#endif
	       GadID, NewDir;

	CloseDataPtr OldCPtr, TempCPtr, HCPtr    = NULL;
	ViewDataPtr VPtr;

	VGASetPal(initcolors, 8);

	CPtr    = NULL;
	RoomNum = 1;
	Direction = NORTH;

	readRoomData("LAB:Doors");
	readInventory("LAB:Inventor");

	if (!createSet(&Conditions, HighestCondition + 1))
		return;

	if (!createSet(&RoomsFound, ManyRooms + 1))
		return;

	readInitialConditions(Conditions, "LAB:Conditio");

	LongWinInFront = false;
	drawPanel();

	perFlipGadget(ActionMode);

	/* Set up initial picture. */

	while (1) {
		WSDL_ProcessInput(1);

		if (GotMessage) {
			if (QuitLab) {
				stopDiff();
				break;
			}

			g_music->restartBackMusic();

			/* Sees what kind of close up we're in and does the appropriate stuff, if any. */
			if (doCloseUp(CPtr)) {
				CPtr = NULL;
#if !defined(DOSCODE)
				mayShowCrumbIndicator();
				WSDL_UpdateScreen();
#endif
			}

			/* Sets the current picture properly on the screen */
			if (MainDisplay)
				Test = getPictName(&CPtr);

			if (noupdatediff) {
				inclElement(RoomsFound, RoomNum); /* Potentially entered another room */
				ForceDraw = (strcmp(Test, CurFileName) != 0) || ForceDraw;

				noupdatediff = false;
				CurFileName = Test;
			}

			else if (strcmp(Test, CurFileName) != 0) {
				interfaceOff();
				inclElement(RoomsFound, RoomNum); /* Potentially entered another room */
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

#if !defined(DOSCODE)
				mayShowCrumbIndicator();
				WSDL_UpdateScreen();

				if (!FollowingCrumbs)
					eatMessages();

#else
				eatMessages();
#endif
			}

			if (ForceDraw) {
				drawRoomMessage(CurInv, CPtr);
				ForceDraw = false;
#if !defined(DOSCODE)
				WSDL_UpdateScreen();
#endif
			}
		}

		g_music->checkMusic();  /* Make sure we check the music at least after every message */
		interfaceOn();
		Msg = getMsg();

		if (Msg == NULL) { /* Does music load and next animation frame when you've run out of messages */
			GotMessage = false;
			checkRoomMusic();
			g_music->newCheckMusic();
			diffNextFrame();
#if !defined(DOSCODE)

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
#endif
		} else {
			GotMessage = true;

			Class     = Msg->Class;
			Code      = Msg->Code;
			Qualifier = Msg->Qualifier;
			MouseX    = Msg->MouseX;
			MouseY    = Msg->MouseY;
			GadID     = Msg->GadgetID;

			replyMsg((void *) Msg);

#if !defined(DOSCODE)
			FollowingCrumbs = false;

from_crumbs:
#endif
			DoBlack = false;

			if ((Class == RAWKEY) && (!LongWinInFront)) {
				if (Code == 13) { /* The return key */
					Class     = MOUSEBUTTONS;
					Qualifier = IEQUALIFIER_LEFTBUTTON;
					mouseXY(&MouseX, &MouseY);
				}

#if !defined(DOSCODE)
				else if (Code == 'b' || Code == 'B') {  /* Start bread crumbs */
					BreadCrumbs[0].RoomNum = 0;
					NumCrumbs = 0;
					DroppingCrumbs = true;
					mayShowCrumbIndicator();
					WSDL_UpdateScreen();
				}

				else if (Code == 'f' || Code == 'F' ||
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
				}

#endif

				else if ((Code == 315) || (Code == 'x') || (Code == 'X')
				         || (Code == 'q') || (Code == 'Q')) {  /* Quit? */
					DoNotDrawMessage = false;
					drawMessage("Do you want to quit? (Y/N)");
					doit = false;
					eatMessages();
					interfaceOff();

					while (1) {
						g_music->checkMusic();  /* Make sure we check the music at least after every message */
						Msg = getMsg();

						if (Msg == NULL) { /* Does music load and next animation frame when you've run out of messages */
							g_music->newCheckMusic();
							diffNextFrame();
						} else {
							replyMsg((void *) Msg);    /* Can't do this in non-IBM versions */

							if (Msg->Class == RAWKEY) {
#if defined(DOSCODE)

								if ((Msg->Code == 'Y') || (Msg->Code == 'y'))
#else
								if ((Msg->Code == 'Y') || (Msg->Code == 'y') || (Msg->Code == 'Q') || (Msg->Code == 'q'))
#endif
								{
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
				}

#ifdef undef
				else if (Code == 324) {
					if (BUFFERSIZE >= BIGBUFFERSIZE) { /* F10 key to switch resolutions */
						blackAllScreen();
						setMode((char) OldMode);

						Alternate = false;
						MainDisplay = true;
						attachGadgetList(NULL);
						fillUpMusic(true);
						g_system->delayMillis(750);

						IsHiRes = !IsHiRes;

						deallocate(MovePanelBuffer, MovePanelBufferSize);
						deallocate(InvPanelBuffer, InvPanelBufferSize);
						freeButtonList(MoveGadgetList);
						freeButtonList(InvGadgetList);
						MoveGadgetList = NULL;
						InvGadgetList = NULL;

						if (!setUpScreens()) {
							IsHiRes = false;
							setUpScreens();
						}

						blackAllScreen();
						resetBuffer();
						CPtr = NULL;
						DoBlack = true;
						CurFileName = " ";
						closeFont(MsgFont);
						openFont("P:AvanteG.12", &MsgFont);
						/*
						            mouseReset();
						 */
						initMouse();
						drawPanel();
						perFlipGadget(ActionMode);
					} else
						drawMessage("Not enough memory to change resolutions.");
				}

#endif

				else if (Code == 9) { /* TAB key */
					Class = DELTAMOVE;
				}

				else if (Code == 27) { /* ESC key */
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
#if !defined(DOSCODE)
					WSDL_UpdateScreen();
#endif
				}
			}

			else if ((Class == GADGETUP) && !Alternate) {
				if (GadID <= 5) {
					if ((ActionMode == 4) && (GadID == 4) && (CPtr != NULL)) {
						doMainView(&CPtr);

						DoBlack = true;
						HCPtr = NULL;
						CPtr = NULL;
#if !defined(DOSCODE)
						mayShowCrumbIndicator();
						WSDL_UpdateScreen();
#endif
					}

					else if (GadID == 5) {
						eatMessages();

						Alternate = true;
						DoBlack = true;
						DoNotDrawMessage = false;
						interfaceOn(); /* Sets the correct gadget list */

						MainDisplay = false;

						if (LastInv && In(Conditions, LastInv)) {
							CurInv = LastInv;
							Test = getInvName(CurInv);
						} else
							decIncInv(&CurInv, false);

						drawPanel();
						drawRoomMessage(CurInv, CPtr);
#if !defined(DOSCODE)
						mayShowCrumbIndicator();
						WSDL_UpdateScreen();
#endif
					} else {
						Old        = ActionMode;
						ActionMode = GadID;

						if (Old < 5)
							perFlipGadget(Old);

						perFlipGadget(ActionMode);

						if (GadID == 0)
							drawMessage(TAKEWHAT);
						else if (GadID == 1)
							drawMessage(MOVEWHAT);
						else if (GadID == 2)
							drawMessage(OPENWHAT);
						else if (GadID == 3)
							drawMessage(CLOSEWHAT);
						else if (GadID == 4)
							drawMessage(LOOKWHAT);

#if !defined(DOSCODE)
						WSDL_UpdateScreen();
#endif
					}
				}

				else if (GadID == 9) {
					doUse(MAPNUM);
#if !defined(DOSCODE)
					mayShowCrumbIndicator();
					WSDL_UpdateScreen();
#endif
				}

				else if (GadID >= 6) { /* Arrow Gadgets */
					CPtr = NULL;
					HCPtr = NULL;

					if ((GadID == 6) || (GadID == 8)) {
						if (GadID == 6)
							drawMessage(TURNLEFT);
						else
							drawMessage(TURNRIGHT);

						CurFileName = " ";
#if !defined(DOSCODE)
						OldDirection = Direction;
#endif
						NewDir = Direction;
						processArrow(&NewDir, GadID - 6);
						doTurn(Direction, NewDir, &CPtr);
						DoBlack = true;
						Direction = NewDir;
						ForceDraw = true;
#if !defined(DOSCODE)
						mayShowCrumbIndicator();
						WSDL_UpdateScreen();
#endif
					} else if (GadID == 7) {
						OldRoomNum = RoomNum;

						if (doGoForward(&CPtr)) {
							if (OldRoomNum == RoomNum)
								DoBlack = true;
						} else {
							DoBlack = true;
							processArrow(&Direction, GadID - 6);

							if (OldRoomNum != RoomNum) {
								drawMessage(GOFORWARDDIR);
								inclElement(RoomsFound, RoomNum); /* Potentially entered a new room */
								CurFileName = " ";
								ForceDraw = true;
							} else {
								DoBlack = true;
								drawMessage(NOPATH);
							}
						}

#if defined(DEMODATA)
						{
							void writeDemoData();
							writeDemoData();
						}
#endif

#if !defined(DOSCODE)

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
#endif
					}
				}
			}

			else if ((Class == GADGETUP) && Alternate) {
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
#if !defined(DOSCODE)
					WSDL_UpdateScreen();
#endif
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
#if !defined(DOSCODE)
						WSDL_UpdateScreen();
#endif
						g_system->delayMillis(1000);
					}

#if !defined(DOSCODE)
					else {
						WSDL_UpdateScreen();
					}

#endif
				}

				else if (GadID == 1) {
					if (!doUse(CurInv)) {
						Old        = ActionMode;
						ActionMode = 5;  /* Use button */

						if (Old < 5)
							perFlipGadget(Old);

						drawMessage(USEONWHAT);
						MainDisplay = true;
#if !defined(DOSCODE)
						WSDL_UpdateScreen();
#endif
					}
				}

				else if (GadID == 2) {
					MainDisplay = !MainDisplay;

					if ((CurInv == 0) || (CurInv > NumInv)) {
						CurInv = 1;

						while ((CurInv <= NumInv) && (!In(Conditions, CurInv)))
							CurInv++;
					}

					if ((CurInv <= NumInv) && In(Conditions, CurInv) &&
					        Inventory[CurInv].BInvName)
						Test = getInvName(CurInv);

#if !defined(DOSCODE)
					WSDL_UpdateScreen();
#endif
				}

				else if (GadID == 3) { /* Left gadget */
					decIncInv(&CurInv, true);
					LastInv = CurInv;
					DoNotDrawMessage = false;
					drawRoomMessage(CurInv, CPtr);
#if !defined(DOSCODE)
					WSDL_UpdateScreen();
#endif
				}

				else if (GadID == 4) { /* Right gadget */
					decIncInv(&CurInv, false);
					LastInv = CurInv;
					DoNotDrawMessage = false;
					drawRoomMessage(CurInv, CPtr);
#if !defined(DOSCODE)
					WSDL_UpdateScreen();
#endif
				}

#if !defined(DOSCODE)
				else if (GadID == 5) { /* bread crumbs */
					BreadCrumbs[0].RoomNum = 0;
					NumCrumbs = 0;
					DroppingCrumbs = true;
					mayShowCrumbIndicator();
					WSDL_UpdateScreen();
				}

				else if (GadID == 6) { /* follow crumbs */
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
			}

#endif

			else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier) && MainDisplay) {
				interfaceOff();
				MainDisplay = true;

				doit = false;

				if (CPtr) {
					if ((CPtr->CloseUpType == SPECIALLOCK) && MainDisplay) /* LAB: Labrinth specific code */
						mouseCombination(Conditions, MouseX, MouseY);
					else if ((CPtr->CloseUpType == SPECIALBRICK) && MainDisplay)
						mouseTile(Conditions, MouseX, MouseY);
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
							drawMessage(TAKEITEM);
						else if (doActionRule(MouseX, MouseY, TAKEDEF - 1, RoomNum, &CPtr))
							CurFileName = NewFileName;
						else if (doActionRule(MouseX, MouseY, TAKE - 1, 0, &CPtr))
							CurFileName = NewFileName;
						else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
							drawMessage(NOTHING);
					}

					else if ((ActionMode == 1) /* Manipulate an object */  ||
					         (ActionMode == 2) /* Open up a "door" */      ||
					         (ActionMode == 3)) { /* Close a "door" */
						if (doActionRule(MouseX, MouseY, ActionMode, RoomNum, &CPtr))
							CurFileName = NewFileName;
						else if (!doActionRule(MouseX, MouseY, ActionMode, 0, &CPtr)) {
							if (MouseY < (VGAScaleY(149) + SVGACord(2)))
								drawMessage(NOTHING);
						}
					}

					else if (ActionMode == 4) { /* Look at closeups */
						TempCPtr = CPtr;
						setCurClose(MouseX, MouseY, &TempCPtr);

						if (CPtr == TempCPtr) {
							if (MouseY < (VGAScaleY(149) + SVGACord(2)))
								drawMessage(NOTHING);
						} else if (TempCPtr->GraphicName) {
							if (*(TempCPtr->GraphicName)) {
								DoBlack = true;
								CPtr = TempCPtr;
							} else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
								drawMessage(NOTHING);
						} else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
							drawMessage(NOTHING);
					}

					else if ((ActionMode == 5)  &&
					         In(Conditions, CurInv)) { /* Use an item on something else */
						if (doOperateRule(MouseX, MouseY, CurInv, &CPtr)) {
							CurFileName = NewFileName;

							if (!In(Conditions, CurInv))
								decIncInv(&CurInv, false);
						} else if (MouseY < (VGAScaleY(149) + SVGACord(2)))
							drawMessage(NOTHING);
					}
				}

#if !defined(DOSCODE)
				mayShowCrumbIndicator();
				WSDL_UpdateScreen();
#endif
			}

			else if (Class == DELTAMOVE) {
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
			}

			else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) {
				eatMessages();
				Alternate = !Alternate;
				DoBlack = true;
				DoNotDrawMessage = false;
				MainDisplay = true;
				interfaceOn(); /* Sets the correct gadget list */

				if (Alternate) {
					if (LastInv && In(Conditions, LastInv))
						CurInv = LastInv;
					else
						decIncInv(&CurInv, false);
				}

				drawPanel();
				drawRoomMessage(CurInv, CPtr);
#if !defined(DOSCODE)
				mayShowCrumbIndicator();
				WSDL_UpdateScreen();
#endif
			}
		}
	}

	if (Conditions)
		deleteSet(Conditions);

	if (RoomsFound)
		deleteSet(RoomsFound);

	if (Rooms)
		deallocate(Rooms, (ManyRooms + 1) * sizeof(RoomData));

	if (Inventory) {
		for (Code = 1; Code <= NumInv; Code++) {
			if (Inventory[Code].name)
				deallocate(Inventory[Code].name, strlen(Inventory[Code].name) + 1);

			if (Inventory[Code].BInvName)
				deallocate(Inventory[Code].BInvName, strlen(Inventory[Code].BInvName) + 1);
		}

		deallocate(Inventory, (NumInv + 1) * sizeof(InventoryData));
	}
}


/*
   void mySignalHandler(int sig)
   {
   signal(sig, mySignalHandler);
   }
 */



#if defined(DOSCODE)
void (__interrupt __far *oldctrlc)(), (__interrupt __far *oldctrlcb)();

uint16 iretdummy;
void __interrupt __far myiret() {
	iretdummy++;
}
#endif


void inner_main() {
	bool mem, dointro = false;
	uint16 counter;

	getMode(&OldMode);

	IsHiRes = true;
	DoMusic = true;
	g_music->_turnMusicOn = true;
	dointro = true;

#if 0
	for (counter = 1; counter < argc; counter++) {
#if defined(DOSCODE)

		if (((argv[counter])[0] == 'v') || ((argv[counter])[0] == 'V')) {
			IsHiRes = false;
		} else
#endif
			if (((argv[counter])[0] == 'q') || ((argv[counter])[0] == 'Q')) {
				DoMusic = false;
				g_music->_turnMusicOn = false;
			}

#ifdef _DEBUG
			else if (((argv[counter])[0] == 'i') || ((argv[counter])[0] == 'I')) {
				dointro = false;
			}

#endif
			else if (((argv[counter])[0] == '/') && ((argv[counter])[1] == '?')) {
				debug("\n\nPlayer Version 1.0.  Copyright (c) 1993 Terra Nova Development\n");
				debug("Player v q\n");
#ifdef _DEBUG
				debug("    i : Skip intro (debug only).\n");
#endif
#if defined(DOSCODE)
				debug("    v : Open up on a low res VGA display.\n");
#endif
				debug("    q : Start in quiet mode; no sound output.\n\n");
				return;
			}
	}
#endif

	if (initBuffer(BUFFERSIZE, true)) {
		mem = true;
	} else {
#if defined(DOSCODE)
		BUFFERSIZE = SMALLBUFFERSIZE;
		mem = initBuffer(BUFFERSIZE, true);
		IsHiRes = false;
		nosvgamem = true;
#else
		// unacceptable !!!!
		warning("initBuffer() failed");
		return;
#endif
	}

	if (!initAudio()) {
		noaudio = true;
		DoMusic = false;
		g_music->_turnMusicOn = false;
		debug("Could not open Audio.");
		g_system->delayMillis(500);
	}

	if (!setUpScreens()) {
		IsHiRes = false;
		novesa = true;
		mem = mem && setUpScreens();
	}

	if (!initMouse()) {
		setMode((char) OldMode);
#if defined(DOSCODE)
		_dos_setvect(0x23, oldctrlc);
		_dos_setvect(0x1B, oldctrlcb);
#endif
		debug("\n\nMouse or mouse driver not installed!\n\n");
		return;
	}

	mem = mem && initRoomBuffer() &&
	      initLabText();

	doDisks();

	if (!dointro)
		g_music->initMusic();

	openFont("P:AvanteG.12", &MsgFont);

	if (dointro && mem) {
#if defined(DOSCODE)
		didintro = true;
#endif
		introSequence();
#if !defined(DOSCODE)
		didintro = true;
#endif
	} else
		DoBlack = true;

	if (mem) {
		mouseShow();
		process();
	} else
		debug("\n\nNot enough memory to start game.\n\n");

	if (QuitLab) { /* Won the game */
		blackAllScreen();
		readPict("P:End/L2In.1", true);

		for (counter = 0; counter < 120; counter++) {
			g_music->newCheckMusic();
			waitTOF();
		}

		readPict("P:End/L2In.9", true);
		readPict("P:End/Lost", true);

#if defined(DOSCODE)
		counter = 600;

		while (counter) {
			newCheckMusic();
			diffNextFrame();
			waitTOF();
			counter--;
		}

#else

		warning("STUB: waitForPress");
		while (!1) { // 1 means ignore SDL_ProcessInput calls
			g_music->newCheckMusic();
			diffNextFrame();
			waitTOF();
		}

		// If we weren't exiting, a call to a mythical SDL_StopWaitForPress() would be in order.
#endif
	}

	closeFont(MsgFont);

	freeLabText();
	freeRoomBuffer();
	freeBuffer();

	g_music->freeMusic();
	freeAudio();

	mouseReset();
	setMode((char) OldMode);

#if defined(DOSCODE)
	_dos_setvect(0x23, oldctrlc);
	_dos_setvect(0x1B, oldctrlcb);
#endif
}

#if !defined(DOSCODE)
/*****************************************************************************/
/* New code to allow quick(er) return navigation in game.                    */
/*****************************************************************************/
int followCrumbs() {
	// NORTH, SOUTH, EAST, WEST
	static int movement[4][4] = {
		{ VKEY_UPARROW, VKEY_RTARROW, VKEY_RTARROW, VKEY_LTARROW }, { VKEY_RTARROW, VKEY_UPARROW, VKEY_LTARROW, VKEY_RTARROW }, { VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW, VKEY_RTARROW }, { VKEY_RTARROW, VKEY_LTARROW, VKEY_RTARROW, VKEY_UPARROW }
#if 0
		{ -1, EAST, EAST, WEST }, { WEST, -1, EAST, WEST }, { NORTH, SOUTH, -1, SOUTH }, { NORTH, SOUTH, NORTH, -1 }
#endif
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

struct Image DropCrumbsImage = { 24, 24, dropCrumbs };
struct Image DropCrumbsOffImage = { 24, 24, dropCrumbsOff };

void mayShowCrumbIndicator() {
	if (DroppingCrumbs && MainDisplay) {
		mouseHide();
		drawMaskImage(&DropCrumbsImage, 612, 4);
		mouseShow();
	}
}

void mayShowCrumbIndicatorOff() {
	if (MainDisplay) {
		mouseHide();
		drawMaskImage(&DropCrumbsOffImage, 612, 4);
		mouseShow();
	}
}
#endif

} // End of namespace Lab
