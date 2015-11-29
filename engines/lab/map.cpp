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
#include "lab/text.h"
#include "lab/mouse.h"
#include "lab/parsefun.h"
#include "lab/parsetypes.h"
#include "lab/resource.h"
#include "lab/interface.h"
#include "lab/text.h"

namespace Lab {

static TextFont *BigMsgFont;
static TextFont bmf;


extern uint16 Direction;
extern bool IsHiRes;

extern CloseDataPtr CPtr;
extern uint16 RoomNum;

/*****************************************************************************/
/* Converts an Amiga palette (up to 16 colors) to a VGA palette, then sets   */
/* the VGA palette.                                                          */
/*****************************************************************************/
void setAmigaPal(uint16 *pal, uint16 numcolors) {
	byte vgapal[16 * 3];
	uint16 vgacount = 0;

	if (numcolors > 16)
		numcolors = 16;

	for (uint16 i = 0; i < numcolors; i++) {
		vgapal[vgacount++] = (byte)(((pal[i] & 0xf00) >> 8) << 2);
		vgapal[vgacount++] = (byte)(((pal[i] & 0x0f0) >> 4) << 2);
		vgapal[vgacount++] = (byte)(((pal[i] & 0x00f)) << 2);
	}

	g_lab->writeColorRegsSmooth(vgapal, 0, 16);
}

void decrypt(byte *text) {
	while (text && *text != '\0') {
		*text++ -= (byte)95;
	}
}

/*****************************************************************************/
/* Gets a chunk of text and puts it into the graphics memory.                */
/*****************************************************************************/
char *getText(const char *filename) {
	bool dodecrypt;
	byte **tfile;

	g_music->updateMusic();
	dodecrypt = (isBuffered(filename) == NULL);
	tfile = g_music->newOpen(filename);

	if (!tfile)
		return NULL;

	if (dodecrypt)
		decrypt(*tfile);

	return (char *)*tfile;
}



/*****************************************************************************/
/* Reads in an image from disk.                                              */
/*****************************************************************************/
void readImage(byte **buffer, Image **im) {
	uint32 size;

	(*im) = (Image *)(*buffer);

	(*im)->Width = READ_LE_UINT16(*buffer);
	(*im)->Height = READ_LE_UINT16(*buffer + 2);

	*buffer += 8; /* sizeof(struct Image); */

	size = (*im)->Width * (*im)->Height;

	if (1L & size)
		size++;

	(*im)->ImageData = (byte *)(*buffer);
	(*buffer) += size;
}


/*---------------------------------------------------------------------------*/
/*------------------------------ The Map stuff ------------------------------*/
/*---------------------------------------------------------------------------*/

extern RoomData *_rooms;

static Image *Map, *Room, *UpArrowRoom, *DownArrowRoom, *Bridge,
			 *HRoom, *VRoom, *Maze, *HugeMaze, *Path, *MapNorth,
			 *MapEast, *MapSouth, *MapWest, *XMark, *Back, *BackAlt,
			 *Down, *DownAlt, *Up, *UpAlt;

static uint16 MaxRooms;
static MapData *Maps;

extern char *LOWERFLOORS, *MIDDLEFLOORS, *UPPERFLOORS, *MEDMAZEFLOORS, *HEDGEMAZEFLOORS, *SURMAZEFLOORS, *CARNIVALFLOOR, *SURMAZEMSG;

extern TextFont *MsgFont;

uint16 *FadePalette;

static uint16 MapGadX[3] = {101, 55, 8}, MapGadY[3] = {105, 105, 105};

static Gadget downgadget = { 101, 105, 2, VKEY_DNARROW, 0L, NULL, NULL, NULL },
			  upgadget   = {  55, 105, 1, VKEY_UPARROW, 0L, NULL, NULL, &downgadget },
			  backgadget = {   8, 105, 0, 0, 0L, NULL, NULL, &upgadget };

static Gadget *MapGadgetList = &backgadget;

static uint16 AmigaMapPalette[] = {
	0x0BA8, 0x0C11, 0x0A74, 0x0076,
	0x0A96, 0x0DCB, 0x0CCA, 0x0222,
	0x0444, 0x0555, 0x0777, 0x0999,
	0x0AAA, 0x0ED0, 0x0EEE, 0x0694
};


#define LOWERFLOOR     1
#define MIDDLEFLOOR    2
#define UPPERFLOOR     3
#define MEDMAZEFLOOR   4
#define HEDGEMAZEFLOOR 5
#define SURMAZEFLOOR   6
#define CARNIVAL       7



static uint16 mapScaleX(uint16 x) {
	if (IsHiRes)
		return (x - 45);
	else
		return ((x - 45) >> 1);
}



static uint16 mapScaleY(uint16 y) {
	if (IsHiRes)
		return y;
	else
		return ((y - 35) >> 1) - (y >> 6);
}


/*****************************************************************************/
/* Loads in the map data.                                                    */
/*****************************************************************************/
static bool loadMapData() {
	byte **buffer;
	uint32 Size;
	Gadget *gptr;
	uint16 counter;

	BigMsgFont = &bmf;

	if (!(BigMsgFont = g_resource->getFont("P:Map.fon")))
		BigMsgFont = MsgFont;

	resetBuffer();  /* Make images load into start of buffer */
	buffer = g_music->newOpen("P:MapImage", Size);

	if (!buffer)
		return false;

	stealBufMem(Size); /* Now freeze that buffer from further use */

	readImage(buffer, &Map);

	readImage(buffer, &Room);
	readImage(buffer, &UpArrowRoom);
	readImage(buffer, &DownArrowRoom);
	readImage(buffer, &HRoom);
	readImage(buffer, &VRoom);
	readImage(buffer, &Maze);
	readImage(buffer, &HugeMaze);

	readImage(buffer, &MapNorth);
	readImage(buffer, &MapEast);
	readImage(buffer, &MapSouth);
	readImage(buffer, &MapWest);

	readImage(buffer, &Path);
	readImage(buffer, &Bridge);

	readImage(buffer, &Back);
	readImage(buffer, &BackAlt);
	readImage(buffer, &Up);
	readImage(buffer, &UpAlt);
	readImage(buffer, &Down);
	readImage(buffer, &DownAlt);

	backgadget.Im    = Back;
	backgadget.ImAlt = BackAlt;
	upgadget.Im      = Up;
	upgadget.ImAlt   = UpAlt;
	downgadget.Im    = Down;
	downgadget.ImAlt = DownAlt;

	counter = 0;
	gptr = MapGadgetList;

	while (gptr) {
		gptr->x = VGAScaleX(MapGadX[counter]);
		gptr->y = VGAScaleY(MapGadY[counter]);
		gptr = gptr->NextGadget;
		counter++;
	}

	Common::File *mapFile = g_resource->openDataFile("Lab:Maps", MKTAG('M', 'A', 'P', '0'));
	if (!mapFile)
		error("Corrupt map file");
	g_music->updateMusic();
	if (!g_music->_doNotFilestopSoundEffect)
		g_music->stopSoundEffect();

	MaxRooms = mapFile->readUint16LE();
	Maps = new MapData[MaxRooms];	// will be freed when the user exits the map
	for (int i = 0; i < MaxRooms; i++) {
		Maps[i].x = mapFile->readUint16LE();
		Maps[i].y = mapFile->readUint16LE();
		Maps[i].PageNumber = mapFile->readUint16LE();
		Maps[i].SpecialID = mapFile->readUint16LE();
		Maps[i].MapFlags = mapFile->readUint32LE();
	}

	delete mapFile;

	return true;
}


static void freeMapData() {
	delete[] Maps;
	Maps = NULL;
}


static uint16 fadeNumIn(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32)(15 - counter)) * ((int32)(num - res))) / 15));
}


static uint16 fadeNumOut(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32) counter) * ((int32)(num - res))) / 15));
}



/*****************************************************************************/
/* Does the fading of the Palette on the screen.                             */
/*****************************************************************************/
void fade(bool fadein, uint16 res) {
	uint16 newpal[16];

	for (uint16 i = 0; i < 16; i++) {
		for (uint16 palIdx = 0; palIdx < 16; palIdx++) {
			if (fadein)
				newpal[palIdx] = (0x00F & fadeNumIn(0x00F & FadePalette[palIdx], 0x00F & res, i)) +
				                 (0x0F0 & fadeNumIn(0x0F0 & FadePalette[palIdx], 0x0F0 & res, i)) +
				                 (0xF00 & fadeNumIn(0xF00 & FadePalette[palIdx], 0xF00 & res, i));
			else
				newpal[palIdx] = (0x00F & fadeNumOut(0x00F & FadePalette[palIdx], 0x00F & res, i)) +
				                 (0x0F0 & fadeNumOut(0x0F0 & FadePalette[palIdx], 0x0F0 & res, i)) +
				                 (0xF00 & fadeNumOut(0xF00 & FadePalette[palIdx], 0xF00 & res, i));
		}

		setAmigaPal(newpal, 16);
		g_lab->waitTOF();
		g_music->updateMusic();
	}
}

/*****************************************************************************/
/* Figures out what a room's coordinates should be.                          */
/*****************************************************************************/
static void roomCords(uint16 CurRoom, uint16 *x1, uint16 *y1, uint16 *x2, uint16 *y2) {
	*x1 = mapScaleX(Maps[CurRoom].x);
	*y1 = mapScaleY(Maps[CurRoom].y);
	*x2 = *x1;
	*y2 = *y1;

	switch (Maps[CurRoom].SpecialID) {
	case NORMAL:
	case UPARROWROOM:
	case DOWNARROWROOM:
		(*x2) += Room->Width;
		(*y2) += Room->Height;
		break;

	case BRIDGEROOM:
		(*x2) += Bridge->Width;
		(*y2) += Bridge->Height;
		break;

	case VCORRIDOR:
		(*x2) += VRoom->Width;
		(*y2) += VRoom->Height;
		break;

	case HCORRIDOR:
		(*x2) += HRoom->Width;
		(*y2) += HRoom->Height;
		break;
	}
}

/*****************************************************************************/
/* Draws a room to the bitmap.                                               */
/*****************************************************************************/
static void drawRoom(uint16 CurRoom, bool drawx) {
	uint16 x, y, xx, xy, offset;
	uint32 flags;

	x = mapScaleX(Maps[CurRoom].x);
	y = mapScaleY(Maps[CurRoom].y);
	flags = Maps[CurRoom].MapFlags;

	switch (Maps[CurRoom].SpecialID) {
	case NORMAL:
	case UPARROWROOM:
	case DOWNARROWROOM:
		if (Maps[CurRoom].SpecialID == NORMAL)
			g_lab->drawImage(Room, x, y);
		else if (Maps[CurRoom].SpecialID == DOWNARROWROOM)
			g_lab->drawImage(DownArrowRoom, x, y);
		else
			g_lab->drawImage(UpArrowRoom, x, y);

		offset = (Room->Width - Path->Width) / 2;

		if ((NORTHDOOR & flags) && (y >= Path->Height))
			g_lab->drawImage(Path, x + offset, y - Path->Height);

		if (SOUTHDOOR & flags)
			g_lab->drawImage(Path, x + offset, y + Room->Height);

		offset = (Room->Height - Path->Height) / 2;

		if (EASTDOOR & flags)
			g_lab->drawImage(Path, x + Room->Width, y + offset);

		if (WESTDOOR & flags)
			g_lab->drawImage(Path, x - Path->Width, y + offset);

		xx = x + (Room->Width - XMark->Width) / 2;
		xy = y + (Room->Height - XMark->Height) / 2;

		break;

	case BRIDGEROOM:
		g_lab->drawImage(Bridge, x, y);

		xx = x + (Bridge->Width - XMark->Width) / 2;
		xy = y + (Bridge->Height - XMark->Height) / 2;

		break;

	case VCORRIDOR:
		g_lab->drawImage(VRoom, x, y);

		offset = (VRoom->Width - Path->Width) / 2;

		if (NORTHDOOR & flags)
			g_lab->drawImage(Path, x + offset, y - Path->Height);

		if (SOUTHDOOR & flags)
			g_lab->drawImage(Path, x + offset, y + VRoom->Height);

		offset = (Room->Height - Path->Height) / 2;

		if (EASTDOOR & flags)
			g_lab->drawImage(Path, x + VRoom->Width, y + offset);

		if (WESTDOOR & flags)
			g_lab->drawImage(Path, x - Path->Width, y + offset);

		if (EASTBDOOR & flags)
			g_lab->drawImage(Path, x + VRoom->Width, y - offset - Path->Height + VRoom->Height);

		if (WESTBDOOR & flags)
			g_lab->drawImage(Path, x - Path->Width, y - offset - Path->Height + VRoom->Height);

		offset = (VRoom->Height - Path->Height) / 2;

		if (EASTMDOOR & flags)
			g_lab->drawImage(Path, x + VRoom->Width, y - offset - Path->Height + VRoom->Height);

		if (WESTMDOOR & flags)
			g_lab->drawImage(Path, x - Path->Width, y - offset - Path->Height + VRoom->Height);

		xx = x + (VRoom->Width - XMark->Width) / 2;
		xy = y + (VRoom->Height - XMark->Height) / 2;

		break;

	case HCORRIDOR:
		g_lab->drawImage(HRoom, x, y);

		offset = (Room->Width - Path->Width) / 2;

		if (NORTHDOOR & flags)
			g_lab->drawImage(Path, x + offset, y - Path->Height);

		if (SOUTHDOOR & flags)
			g_lab->drawImage(Path, x + offset, y + Room->Height);

		if (NORTHRDOOR & flags)
			g_lab->drawImage(Path, x - offset - Path->Width + HRoom->Width, y - Path->Height);

		if (SOUTHRDOOR & flags)
			g_lab->drawImage(Path, x - offset - Path->Width + HRoom->Width, y + Room->Height);

		offset = (HRoom->Width - Path->Width) / 2;

		if (NORTHMDOOR & flags)
			g_lab->drawImage(Path, x - offset - Path->Width + HRoom->Width, y - Path->Height);

		if (SOUTHMDOOR & flags)
			g_lab->drawImage(Path, x - offset - Path->Width + HRoom->Width, y + Room->Height);

		offset = (Room->Height - Path->Height) / 2;

		if (EASTDOOR & flags)
			g_lab->drawImage(Path, x + HRoom->Width, y + offset);

		if (WESTDOOR & flags)
			g_lab->drawImage(Path, x - Path->Width, y + offset);

		xx = x + (HRoom->Width - XMark->Width) / 2;
		xy = y + (HRoom->Height - XMark->Height) / 2;

		break;

	default:
		return;
	}

	if (drawx)
		g_lab->drawImage(XMark, xx, xy);
}

/*****************************************************************************/
/* Checks if a floor has been visitted.                                      */
/*****************************************************************************/
static bool onFloor(uint16 Floor) {
	for (uint16 i = 1; i <= MaxRooms; i++) {
		if ((Maps[i].PageNumber == Floor) && g_lab->_roomsFound->in(i) && Maps[i].x)
			return true;
	}

	return false;
}

/*****************************************************************************/
/* Figures out which floor, if any, should be gone to if the up arrow is hit */
/*****************************************************************************/
static void getUpFloor(uint16 *Floor, bool *isfloor) {
	do {
		*isfloor = true;

		if (*Floor < UPPERFLOOR)
			(*Floor)++;
		else {
			*Floor   = CARNIVAL + 1;
			*isfloor = false;
			return;
		}
	} while ((!onFloor(*Floor)) && (*Floor <= CARNIVAL));
}




/*****************************************************************************/
/* Figures out which floor, if any, should be gone to if the down arrow is   */
/* hit.                                                                      */
/*****************************************************************************/
static void getDownFloor(uint16 *Floor, bool *isfloor) {
	do {
		*isfloor = true;

		if ((*Floor == LOWERFLOOR) || (*Floor == 0)) {
			*Floor   = 0;
			*isfloor = false;
			return;
		} else if (*Floor > UPPERFLOOR) {
			// Labyrinth specific code
			if (*Floor == HEDGEMAZEFLOOR)
				*Floor = UPPERFLOOR;
			else if ((*Floor == CARNIVAL) || (*Floor == MEDMAZEFLOOR))
				*Floor = MIDDLEFLOOR;
			else if (*Floor == SURMAZEFLOOR)
				*Floor = LOWERFLOOR;
			else {
				*Floor = 0;
				*isfloor = false;
				return;
			}
		} else
			(*Floor)--;

	} while ((!onFloor(*Floor)) && *Floor);
}





/*****************************************************************************/
/* Draws the map                                                             */
/*****************************************************************************/
void LabEngine::drawMap(uint16 CurRoom, uint16 CurMsg, uint16 Floor, bool fadeout, bool fadein) {
	char *sptr;

	uint16 tempfloor;
	bool noghoast;

	_event->mouseHide();

	if (fadeout)
		fade(false, 0);

	setAPen(0);
	rectFill(0, 0, _screenWidth - 1, _screenHeight - 1);

	drawImage(Map, 0, 0);
	drawGadgetList(MapGadgetList);

	for (uint16 i = 1; i <= MaxRooms; i++) {
		if ((Maps[i].PageNumber == Floor) && _roomsFound->in(i) && Maps[i].x) {
			drawRoom(i, (bool)(i == CurRoom));
			g_music->updateMusic();
		}
	}

	if ((Maps[CurRoom].PageNumber == Floor)   /* Makes sure the X is drawn in corridors */
	        && _roomsFound->in(CurRoom) /* NOTE: this here on purpose just in case there's some weird condition, like the surreal maze where there are no rooms */
	        && Maps[CurRoom].x)
		drawRoom(CurRoom, true);

	tempfloor = Floor;
	getUpFloor(&tempfloor, &noghoast);

	if (noghoast)
		unGhoastGadget(&upgadget);
	else
		ghoastGadget(&upgadget, 12);

	tempfloor = Floor;
	getDownFloor(&tempfloor, &noghoast);

	if (noghoast)
		unGhoastGadget(&downgadget);
	else
		ghoastGadget(&downgadget, 12);

	// Labyrinth specific code
	if (Floor == LOWERFLOOR) {
		if (onFloor(SURMAZEFLOOR))
			drawImage(Maze, mapScaleX(538), mapScaleY(277));
	} else if (Floor == MIDDLEFLOOR) {
		if (onFloor(CARNIVAL))
			drawImage(Maze, mapScaleX(358), mapScaleY(72));

		if (onFloor(MEDMAZEFLOOR))
			drawImage(Maze, mapScaleX(557), mapScaleY(325));
	} else if (Floor == UPPERFLOOR) {
		if (onFloor(HEDGEMAZEFLOOR))
			drawImage(HugeMaze, mapScaleX(524), mapScaleY(97));
	} else if (Floor == SURMAZEFLOOR) {
		sptr = (char *)g_resource->getStaticText(kTextSurmazeMessage).c_str();
		flowText(MsgFont, 0, 7, 0, true, true, true, true, mapScaleX(360), 0, mapScaleX(660), mapScaleY(450), sptr);
	}

	switch (Floor) {
	case LOWERFLOOR:
		sptr = (char *)g_resource->getStaticText(kTextLowerFloor).c_str();
		break;
	case MIDDLEFLOOR:
		sptr = (char *)g_resource->getStaticText(kTextMiddleFloor).c_str();
		break;
	case UPPERFLOOR:
		sptr = (char *)g_resource->getStaticText(kTextUpperFloor).c_str();
		break;
	case MEDMAZEFLOOR:
		sptr = (char *)g_resource->getStaticText(kTextMedMazeFloor).c_str();
		break;
	case HEDGEMAZEFLOOR:
		sptr = (char *)g_resource->getStaticText(kTextHedgeMazeFloor).c_str();
		break;
	case SURMAZEFLOOR:
		sptr = (char *)g_resource->getStaticText(kTextSurMazeFloor).c_str();
		break;
	case CARNIVAL:
		sptr = (char *)g_resource->getStaticText(kTextCarnivalFloor).c_str();
		break;
	default:
		sptr = NULL;
		break;
	}

	if (sptr)
		flowText(MsgFont, 0, 5, 3, true, true, true, true, VGAScaleX(14), VGAScaleY(75), VGAScaleX(134), VGAScaleY(97), sptr);

	if ((sptr = _rooms[CurMsg]._roomMsg))
		flowText(MsgFont, 0, 5, 3, true, true, true, true, VGAScaleX(14), VGAScaleY(148), VGAScaleX(134), VGAScaleY(186), sptr);

	if (fadein)
		fade(true, 0);

	_event->mouseShow();
}

/*****************************************************************************/
/* Processes the map.                                                        */
/*****************************************************************************/
void LabEngine::processMap(uint16 CurRoom) {
	uint32 Class, place = 1;
	uint16 Code, Qualifier, MouseX, MouseY, GadgetID, CurFloor, OldFloor, OldMsg, CurMsg, x1, y1, x2, y2;
	char *sptr;
	byte newcolor[3];
	bool drawmap;
	IntuiMessage *Msg;

	CurMsg   = CurRoom;
	CurFloor = Maps[CurRoom].PageNumber;

	while (1) {
		g_music->updateMusic();  /* Make sure we check the music at least after every message */
		Msg = getMsg();

		if (Msg == NULL) {
			g_music->updateMusic();

			if (place <= 14) {
				newcolor[0] = 14 << 2;
				newcolor[1] = place << 2;
				newcolor[2] = newcolor[1];
			} else {
				newcolor[0] = 14 << 2;
				newcolor[1] = (28 - place) << 2;
				newcolor[2] = newcolor[1];
			}

			waitTOF();
			writeColorReg(newcolor, 1);
			_event->updateMouse();
			waitTOF();
			_event->updateMouse();
			waitTOF();
			_event->updateMouse();
			waitTOF();
			_event->updateMouse();

			place++;

			if (place >= 28)
				place = 1;

		} else {
			Class     = Msg->msgClass;
			Code      = Msg->code;
			GadgetID  = Msg->gadgetID;
			Qualifier = Msg->qualifier;
			MouseX    = Msg->mouseX;
			MouseY    = Msg->mouseY;

			if (((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) ||
			        ((Class == RAWKEY) && (Code == 27)))
				return;

			if (Class == GADGETUP) {
				if (GadgetID == 0) { /* Quit menu button */
					return;
				} else if (GadgetID == 1) { /* Up arrow */
					OldFloor = CurFloor;
					getUpFloor(&CurFloor, &drawmap);

					if (drawmap) {
						fade(false, 0);
						drawMap(CurRoom, CurMsg, CurFloor, false, false);
						fade(true, 0);
					} else
						CurFloor = OldFloor;
				} else if (GadgetID == 2) { /* Down arrow */
					OldFloor = CurFloor;
					getDownFloor(&CurFloor, &drawmap);

					if (drawmap) {
						fade(false, 0);
						drawMap(CurRoom, CurMsg, CurFloor, false, false);
						fade(true, 0);
					} else
						CurFloor = OldFloor;
				}
			}

			else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier)) {
				if ((CurFloor == LOWERFLOOR) && (MouseX >= mapScaleX(538)) && (MouseY >= mapScaleY(277))
				        && (MouseX <= mapScaleX(633)) && (MouseY <= mapScaleY(352))
				        && onFloor(SURMAZEFLOOR)) {
					CurFloor = SURMAZEFLOOR;

					fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					fade(true, 0);
				}

				else if ((CurFloor == MIDDLEFLOOR) && (MouseX >= mapScaleX(358)) && (MouseY >= mapScaleY(71))
				         && (MouseX <= mapScaleX(452)) && (MouseY <= mapScaleY(147))
				         && onFloor(CARNIVAL)) {
					CurFloor = CARNIVAL;

					fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					fade(true, 0);
				}

				else if ((CurFloor == MIDDLEFLOOR) && (MouseX >= mapScaleX(557)) && (MouseY >= mapScaleY(325))
				         && (MouseX <= mapScaleX(653)) && (MouseY <= mapScaleY(401))
				         && onFloor(MEDMAZEFLOOR)) {
					CurFloor = MEDMAZEFLOOR;

					fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					fade(true, 0);
				}

				else if ((CurFloor == UPPERFLOOR) && (MouseX >= mapScaleX(524)) && (MouseY >=  mapScaleY(97))
				         && (MouseX <= mapScaleX(645)) && (MouseY <= mapScaleY(207))
				         && onFloor(HEDGEMAZEFLOOR)) {
					CurFloor = HEDGEMAZEFLOOR;

					fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					fade(true, 0);
				}

				else if (MouseX > mapScaleX(314)) {
					OldMsg = CurMsg;

					for (uint16 i = 1; i <= MaxRooms; i++) {
						roomCords(i, &x1, &y1, &x2, &y2);

						if ((Maps[i].PageNumber == CurFloor)
						        && _roomsFound->in(i)
						        && (MouseX >= x1) && (MouseX <= x2)
						        && (MouseY >= y1) && (MouseY <= y2)) {
							CurMsg = i;
						}
					}

					if (OldMsg != CurMsg) {
						if (_rooms[CurMsg]._roomMsg == nullptr)
							g_resource->readViews(CurMsg);

						if ((sptr = _rooms[CurMsg]._roomMsg)) {
							_event->mouseHide();
							setAPen(3);
							rectFill(VGAScaleX(13), VGAScaleY(148), VGAScaleX(135), VGAScaleY(186));
							flowText(MsgFont, 0, 5, 3, true, true, true, true, VGAScaleX(14), VGAScaleY(148), VGAScaleX(134), VGAScaleY(186), sptr);

							if (Maps[OldMsg].PageNumber == CurFloor)
								drawRoom(OldMsg, (bool)(OldMsg == CurRoom));

							roomCords(CurMsg, &x1, &y1, &x2, &y2);
							x1 = (x1 + x2) / 2;
							y1 = (y1 + y2) / 2;

							if ((CurMsg != CurRoom) && (Maps[CurMsg].PageNumber == CurFloor)) {
								setAPen(1);
								rectFill(x1 - 1, y1, x1, y1);
							}

							_event->mouseShow();
						}
					}
				}
			}

			WSDL_UpdateScreen();
		}
	}
}


/*****************************************************************************/
/* Does the map processing.                                                  */
/*****************************************************************************/
void LabEngine::doMap(uint16 CurRoom) {
	FadePalette = AmigaMapPalette;

	g_music->updateMusic();
	loadMapData();
	blackAllScreen();

	if (Direction == NORTH)
		XMark = MapNorth;
	else if (Direction == SOUTH)
		XMark = MapSouth;
	else if (Direction == EAST)
		XMark = MapEast;
	else if (Direction == WEST)
		XMark = MapWest;

	drawMap(CurRoom, CurRoom, Maps[CurRoom].PageNumber, false, true);
	_event->mouseShow();
	_event->attachGadgetList(MapGadgetList);
	WSDL_UpdateScreen();
	processMap(CurRoom);
	_event->attachGadgetList(NULL);
	fade(false, 0);
	blackAllScreen();
	_event->mouseHide();
	setAPen(0);
	rectFill(0, 0, _screenWidth - 1, _screenHeight - 1);
	freeMapData();
	blackAllScreen();
	_event->mouseShow();
	WSDL_UpdateScreen();
}

} // End of namespace Lab
