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
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/interface.h"

namespace Lab {

/*---------------------------------------------------------------------------*/
/*------------------------------ The Map stuff ------------------------------*/
/*---------------------------------------------------------------------------*/

static Image *Map, *Room, *UpArrowRoom, *DownArrowRoom, *Bridge,
			 *HRoom, *VRoom, *Maze, *HugeMaze, *Path, *MapNorth,
			 *MapEast, *MapSouth, *MapWest, *XMark;

static uint16 MaxRooms;
static MapData *Maps;

static uint16 MapGadX[3] = {101, 55, 8}, MapGadY[3] = {105, 105, 105};

static Gadget
	backgadget = { 8, 105, 0, 0, 0L, NULL, NULL },
	upgadget = { 55, 105, 1, VKEY_UPARROW, 0L, NULL, NULL },
	downgadget = { 101, 105, 2, VKEY_DNARROW, 0L, NULL, NULL };

static GadgetList *MapGadgetList;

#define LOWERFLOOR     1
#define MIDDLEFLOOR    2
#define UPPERFLOOR     3
#define MEDMAZEFLOOR   4
#define HEDGEMAZEFLOOR 5
#define SURMAZEFLOOR   6
#define CARNIVAL       7

static uint16 mapScaleX(uint16 x) {
	if (g_lab->_isHiRes)
		return (x - 45);
	else
		return ((x - 45) >> 1);
}

static uint16 mapScaleY(uint16 y) {
	if (g_lab->_isHiRes)
		return y;
	else
		return ((y - 35) >> 1) - (y >> 6);
}

/**
 * Loads in the map data.
 */
static bool loadMapData() {
	uint16 counter;

	MapGadgetList = new GadgetList();
	MapGadgetList->push_back(&backgadget);
	MapGadgetList->push_back(&upgadget);
	MapGadgetList->push_back(&downgadget);

	Common::File *mapImages = g_lab->_resource->openDataFile("P:MapImage");

	Map = new Image(mapImages);

	Room = new Image(mapImages);
	UpArrowRoom = new Image(mapImages);
	DownArrowRoom = new Image(mapImages);
	HRoom = new Image(mapImages);
	VRoom = new Image(mapImages);
	Maze = new Image(mapImages);
	HugeMaze = new Image(mapImages);

	MapNorth = new Image(mapImages);
	MapEast = new Image(mapImages);
	MapSouth = new Image(mapImages);
	MapWest = new Image(mapImages);

	Path = new Image(mapImages);
	Bridge = new Image(mapImages);

	backgadget._image = new Image(mapImages);
	backgadget._altImage = new Image(mapImages);
	upgadget._image = new Image(mapImages);
	upgadget._altImage = new Image(mapImages);
	downgadget._image = new Image(mapImages);
	downgadget._altImage = new Image(mapImages);

	delete mapImages;

	counter = 0;

	for (GadgetList::iterator gadget = MapGadgetList->begin(); gadget != MapGadgetList->end(); ++gadget) {
		(*gadget)->x = g_lab->_utils->vgaScaleX(MapGadX[counter]);
		(*gadget)->y = g_lab->_utils->vgaScaleY(MapGadY[counter]);
		counter++;
	}

	Common::File *mapFile = g_lab->_resource->openDataFile("Lab:Maps", MKTAG('M', 'A', 'P', '0'));
	g_lab->_music->updateMusic();
	if (!g_lab->_music->_doNotFilestopSoundEffect)
		g_lab->_music->stopSoundEffect();

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
	MapGadgetList->clear();
	delete MapGadgetList;

	delete[] Maps;
	Maps = NULL;
}

/**
 * Figures out what a room's coordinates should be.
 */
static void roomCoords(uint16 CurRoom, uint16 *x1, uint16 *y1, uint16 *x2, uint16 *y2) {
	Image *curRoomImg = NULL;

	switch (Maps[CurRoom].SpecialID) {
	case NORMAL:
	case UPARROWROOM:
	case DOWNARROWROOM:
		curRoomImg = Room;
		break;
	case BRIDGEROOM:
		curRoomImg = Bridge;
		break;
	case VCORRIDOR:
		curRoomImg = VRoom;
		break;
	case HCORRIDOR:
		curRoomImg = HRoom;
		break;
	default:
		// Some rooms (like the map) do not have an image
		break;
	}

	*x1 = mapScaleX(Maps[CurRoom].x);
	*y1 = mapScaleY(Maps[CurRoom].y);
	*x2 = *x1;
	*y2 = *y1;

	if (curRoomImg) {
		*x2 += curRoomImg->_width;
		*y2 += curRoomImg->_height;
	}
}

/**
 * Draws a room to the bitmap.
 */
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
			Room->drawImage(x, y);
		else if (Maps[CurRoom].SpecialID == DOWNARROWROOM)
			DownArrowRoom->drawImage(x, y);
		else
			UpArrowRoom->drawImage(x, y);

		offset = (Room->_width - Path->_width) / 2;

		if ((NORTHDOOR & flags) && (y >= Path->_height))
			Path->drawImage(x + offset, y - Path->_height);

		if (SOUTHDOOR & flags)
			Path->drawImage(x + offset, y + Room->_height);

		offset = (Room->_height - Path->_height) / 2;

		if (EASTDOOR & flags)
			Path->drawImage(x + Room->_width, y + offset);

		if (WESTDOOR & flags)
			Path->drawImage(x - Path->_width, y + offset);

		xx = x + (Room->_width - XMark->_width) / 2;
		xy = y + (Room->_height - XMark->_height) / 2;

		break;

	case BRIDGEROOM:
		Bridge->drawImage(x, y);

		xx = x + (Bridge->_width - XMark->_width) / 2;
		xy = y + (Bridge->_height - XMark->_height) / 2;

		break;

	case VCORRIDOR:
		VRoom->drawImage(x, y);

		offset = (VRoom->_width - Path->_width) / 2;

		if (NORTHDOOR & flags)
			Path->drawImage(x + offset, y - Path->_height);

		if (SOUTHDOOR & flags)
			Path->drawImage(x + offset, y + VRoom->_height);

		offset = (Room->_height - Path->_height) / 2;

		if (EASTDOOR & flags)
			Path->drawImage(x + VRoom->_width, y + offset);

		if (WESTDOOR & flags)
			Path->drawImage(x - Path->_width, y + offset);

		if (EASTBDOOR & flags)
			Path->drawImage(x + VRoom->_width, y - offset - Path->_height + VRoom->_height);

		if (WESTBDOOR & flags)
			Path->drawImage(x - Path->_width, y - offset - Path->_height + VRoom->_height);

		offset = (VRoom->_height - Path->_height) / 2;

		if (EASTMDOOR & flags)
			Path->drawImage(x + VRoom->_width, y - offset - Path->_height + VRoom->_height);

		if (WESTMDOOR & flags)
			Path->drawImage(x - Path->_width, y - offset - Path->_height + VRoom->_height);

		xx = x + (VRoom->_width - XMark->_width) / 2;
		xy = y + (VRoom->_height - XMark->_height) / 2;

		break;

	case HCORRIDOR:
		HRoom->drawImage(x, y);

		offset = (Room->_width - Path->_width) / 2;

		if (NORTHDOOR & flags)
			Path->drawImage(x + offset, y - Path->_height);

		if (SOUTHDOOR & flags)
			Path->drawImage(x + offset, y + Room->_height);

		if (NORTHRDOOR & flags)
			Path->drawImage(x - offset - Path->_width + HRoom->_width, y - Path->_height);

		if (SOUTHRDOOR & flags)
			Path->drawImage(x - offset - Path->_width + HRoom->_width, y + Room->_height);

		offset = (HRoom->_width - Path->_width) / 2;

		if (NORTHMDOOR & flags)
			Path->drawImage(x - offset - Path->_width + HRoom->_width, y - Path->_height);

		if (SOUTHMDOOR & flags)
			Path->drawImage(x - offset - Path->_width + HRoom->_width, y + Room->_height);

		offset = (Room->_height - Path->_height) / 2;

		if (EASTDOOR & flags)
			Path->drawImage(x + HRoom->_width, y + offset);

		if (WESTDOOR & flags)
			Path->drawImage(x - Path->_width, y + offset);

		xx = x + (HRoom->_width - XMark->_width) / 2;
		xy = y + (HRoom->_height - XMark->_height) / 2;

		break;

	default:
		return;
	}

	if (drawx)
		XMark->drawImage(xx, xy);
}

/**
 * Checks if a floor has been visitted.
 */
static bool onFloor(uint16 Floor) {
	for (uint16 i = 1; i <= MaxRooms; i++) {
		if ((Maps[i].PageNumber == Floor) && g_lab->_roomsFound->in(i) && Maps[i].x)
			return true;
	}

	return false;
}

/**
 * Figures out which floor, if any, should be gone to if the up arrow is hit
 */
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

/**
 * Figures out which floor, if any, should be gone to if the down arrow is
 * hit.
 */
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

/**
 * Draws the map
 */
void LabEngine::drawMap(uint16 CurRoom, uint16 CurMsg, uint16 Floor, bool fadeout, bool fadein) {
	char *sptr;

	uint16 tempfloor;
	bool noOverlay;

	_event->mouseHide();

	if (fadeout)
		_graphics->fade(false, 0);

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);

	Map->drawImage(0, 0);
	drawGadgetList(MapGadgetList);

	for (uint16 i = 1; i <= MaxRooms; i++) {
		if ((Maps[i].PageNumber == Floor) && _roomsFound->in(i) && Maps[i].x) {
			drawRoom(i, (bool)(i == CurRoom));
			_music->updateMusic();
		}
	}

	/* Makes sure the X is drawn in corridors */
	/* NOTE: this here on purpose just in case there's some weird condition, like the surreal maze where there are no rooms */
	if ((Maps[CurRoom].PageNumber == Floor) && _roomsFound->in(CurRoom) && Maps[CurRoom].x)
		drawRoom(CurRoom, true);

	tempfloor = Floor;
	getUpFloor(&tempfloor, &noOverlay);

	if (noOverlay)
		enableGadget(&upgadget);
	else
		disableGadget(&upgadget, 12);

	tempfloor = Floor;
	getDownFloor(&tempfloor, &noOverlay);

	if (noOverlay)
		enableGadget(&downgadget);
	else
		disableGadget(&downgadget, 12);

	// Labyrinth specific code
	if (Floor == LOWERFLOOR) {
		if (onFloor(SURMAZEFLOOR))
			Maze->drawImage(mapScaleX(538), mapScaleY(277));
	} else if (Floor == MIDDLEFLOOR) {
		if (onFloor(CARNIVAL))
			Maze->drawImage(mapScaleX(358), mapScaleY(72));

		if (onFloor(MEDMAZEFLOOR))
			Maze->drawImage(mapScaleX(557), mapScaleY(325));
	} else if (Floor == UPPERFLOOR) {
		if (onFloor(HEDGEMAZEFLOOR))
			HugeMaze->drawImage(mapScaleX(524), mapScaleY(97));
	} else if (Floor == SURMAZEFLOOR) {
		sptr = (char *)_resource->getStaticText(kTextSurmazeMessage).c_str();
		_graphics->flowText(_msgFont, 0, 7, 0, true, true, true, true, mapScaleX(360), 0, mapScaleX(660), mapScaleY(450), sptr);
	}

	if (Floor >= LOWERFLOOR && Floor <= CARNIVAL) {
		sptr = (char *)_resource->getStaticText(Floor - 1).c_str();
		_graphics->flowTextScaled(_msgFont, 0, 5, 3, true, true, true, true, 14, 75, 134, 97, sptr);
	}

	if ((sptr = _rooms[CurMsg]._roomMsg))
		_graphics->flowTextScaled(_msgFont, 0, 5, 3, true, true, true, true, 14, 148, 134, 186, sptr);

	if (fadein)
		_graphics->fade(true, 0);

	_event->mouseShow();
}

/**
 * Processes the map.
 */
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
		_music->updateMusic();  /* Make sure we check the music at least after every message */
		Msg = getMsg();

		if (Msg == NULL) {
			_music->updateMusic();

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
			_graphics->writeColorRegs(newcolor, 1, 1);
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
			Class     = Msg->_msgClass;
			Code      = Msg->_code;
			GadgetID  = Msg->_gadgetID;
			Qualifier = Msg->_qualifier;
			MouseX    = Msg->_mouseX;
			MouseY    = Msg->_mouseY;

			if (((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) || ((Class == RAWKEY) && (Code == 27)))
				return;

			if (Class == GADGETUP) {
				if (GadgetID == 0) { /* Quit menu button */
					return;
				} else if (GadgetID == 1) { /* Up arrow */
					OldFloor = CurFloor;
					getUpFloor(&CurFloor, &drawmap);

					if (drawmap) {
						_graphics->fade(false, 0);
						drawMap(CurRoom, CurMsg, CurFloor, false, false);
						_graphics->fade(true, 0);
					} else
						CurFloor = OldFloor;
				} else if (GadgetID == 2) { /* Down arrow */
					OldFloor = CurFloor;
					getDownFloor(&CurFloor, &drawmap);

					if (drawmap) {
						_graphics->fade(false, 0);
						drawMap(CurRoom, CurMsg, CurFloor, false, false);
						_graphics->fade(true, 0);
					} else
						CurFloor = OldFloor;
				}
			} else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier)) {
				if ((CurFloor == LOWERFLOOR) && (MouseX >= mapScaleX(538)) && (MouseY >= mapScaleY(277))
					  && (MouseX <= mapScaleX(633)) && (MouseY <= mapScaleY(352))
					  && onFloor(SURMAZEFLOOR)) {
					CurFloor = SURMAZEFLOOR;

					_graphics->fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((CurFloor == MIDDLEFLOOR) && (MouseX >= mapScaleX(358)) && (MouseY >= mapScaleY(71))
							  && (MouseX <= mapScaleX(452)) && (MouseY <= mapScaleY(147))
							  && onFloor(CARNIVAL)) {
					CurFloor = CARNIVAL;

					_graphics->fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((CurFloor == MIDDLEFLOOR) && (MouseX >= mapScaleX(557)) && (MouseY >= mapScaleY(325))
						  && (MouseX <= mapScaleX(653)) && (MouseY <= mapScaleY(401))
						  && onFloor(MEDMAZEFLOOR)) {
					CurFloor = MEDMAZEFLOOR;

					_graphics->fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((CurFloor == UPPERFLOOR) && (MouseX >= mapScaleX(524)) && (MouseY >=  mapScaleY(97))
						  && (MouseX <= mapScaleX(645)) && (MouseY <= mapScaleY(207))
						  && onFloor(HEDGEMAZEFLOOR)) {
					CurFloor = HEDGEMAZEFLOOR;

					_graphics->fade(false, 0);
					drawMap(CurRoom, CurMsg, CurFloor, false, false);
					_graphics->fade(true, 0);
				} else if (MouseX > mapScaleX(314)) {
					OldMsg = CurMsg;

					for (uint16 i = 1; i <= MaxRooms; i++) {
						roomCoords(i, &x1, &y1, &x2, &y2);

						if ((Maps[i].PageNumber == CurFloor)
							  && _roomsFound->in(i)
							  && (MouseX >= x1) && (MouseX <= x2)
							  && (MouseY >= y1) && (MouseY <= y2)) {
							CurMsg = i;
						}
					}

					if (OldMsg != CurMsg) {
						if (_rooms[CurMsg]._roomMsg == nullptr)
							_resource->readViews(CurMsg);

						if ((sptr = _rooms[CurMsg]._roomMsg)) {
							_event->mouseHide();
							_graphics->setAPen(3);
							_graphics->rectFillScaled(13, 148, 135, 186);
							_graphics->flowTextScaled(_msgFont, 0, 5, 3, true, true, true, true, 14, 148, 134, 186, sptr);

							if (Maps[OldMsg].PageNumber == CurFloor)
								drawRoom(OldMsg, (bool)(OldMsg == CurRoom));

							roomCoords(CurMsg, &x1, &y1, &x2, &y2);
							x1 = (x1 + x2) / 2;
							y1 = (y1 + y2) / 2;

							if ((CurMsg != CurRoom) && (Maps[CurMsg].PageNumber == CurFloor)) {
								_graphics->setAPen(1);
								_graphics->rectFill(x1 - 1, y1, x1, y1);
							}

							_event->mouseShow();
						}
					}
				}
			}

			_graphics->screenUpdate();
		}
	}
}

/**
 * Does the map processing.
 */
void LabEngine::doMap(uint16 CurRoom) {
	static uint16 AmigaMapPalette[] = {
		0x0BA8, 0x0C11, 0x0A74, 0x0076,
		0x0A96, 0x0DCB, 0x0CCA, 0x0222,
		0x0444, 0x0555, 0x0777, 0x0999,
		0x0AAA, 0x0ED0, 0x0EEE, 0x0694
	};

	_graphics->FadePalette = AmigaMapPalette;

	_music->updateMusic();
	loadMapData();
	_graphics->blackAllScreen();

	if (_direction == NORTH)
		XMark = MapNorth;
	else if (_direction == SOUTH)
		XMark = MapSouth;
	else if (_direction == EAST)
		XMark = MapEast;
	else if (_direction == WEST)
		XMark = MapWest;

	drawMap(CurRoom, CurRoom, Maps[CurRoom].PageNumber, false, true);
	_event->mouseShow();
	_event->attachGadgetList(MapGadgetList);
	_graphics->screenUpdate();
	processMap(CurRoom);
	_event->attachGadgetList(NULL);
	_graphics->fade(false, 0);
	_graphics->blackAllScreen();
	_event->mouseHide();
	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);
	freeMapData();
	_graphics->blackAllScreen();
	_event->mouseShow();
	_graphics->screenUpdate();
}

} // End of namespace Lab
