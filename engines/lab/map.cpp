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

#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"
#include "lab/interface.h"
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {

/*---------------------------------------------------------------------------*/
/*------------------------------ The Map stuff ------------------------------*/
/*---------------------------------------------------------------------------*/

static Image *Map, *Room, *UpArrowRoom, *DownArrowRoom, *Bridge,
			 *HRoom, *VRoom, *Maze, *HugeMaze, *Path, *MapNorth,
			 *MapEast, *MapSouth, *MapWest, *XMark;

static uint16 MaxRooms;
static MapData *Maps;

enum MapFloor {
	kFloorNone,
	kFloorLower,
	kFloorMiddle,
	kFloorUpper,
	kFloorMedMaze,
	kFloorHedgeMaze,
	kFloorSurMaze,
	kFloorCarnival
};

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
void LabEngine::loadMapData() {
	Utils *utils = g_lab->_utils;
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

	_mapGadgetList.push_back(createButton( 8,  utils->vgaScaleY(105), 0, VKEY_LTARROW, new Image(mapImages), new Image(mapImages)));	// back
	_mapGadgetList.push_back(createButton( 55, utils->vgaScaleY(105), 1, VKEY_UPARROW, new Image(mapImages), new Image(mapImages)));	// up
	_mapGadgetList.push_back(createButton(101, utils->vgaScaleY(105), 2, VKEY_DNARROW, new Image(mapImages), new Image(mapImages)));	// down

	delete mapImages;

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
}

void LabEngine::freeMapData() {
	freeButtonList(&_mapGadgetList);

	delete[] Maps;
	Maps = NULL;
}

/**
 * Figures out what a room's coordinates should be.
 */
static void roomCoords(uint16 curRoom, uint16 *x1, uint16 *y1, uint16 *x2, uint16 *y2) {
	Image *curRoomImg = NULL;

	switch (Maps[curRoom].SpecialID) {
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

	*x1 = mapScaleX(Maps[curRoom].x);
	*y1 = mapScaleY(Maps[curRoom].y);
	*x2 = *x1;
	*y2 = *y1;

	if (curRoomImg) {
		*x2 += curRoomImg->_width;
		*y2 += curRoomImg->_height;
	}
}

/**
 * Draws a room map.
 */
static void drawRoomMap(uint16 curRoom, bool drawx) {
	uint16 x, y, xx, xy, offset;
	uint32 flags;

	x = mapScaleX(Maps[curRoom].x);
	y = mapScaleY(Maps[curRoom].y);
	flags = Maps[curRoom].MapFlags;

	switch (Maps[curRoom].SpecialID) {
	case NORMAL:
	case UPARROWROOM:
	case DOWNARROWROOM:
		if (Maps[curRoom].SpecialID == NORMAL)
			Room->drawImage(x, y);
		else if (Maps[curRoom].SpecialID == DOWNARROWROOM)
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
static bool onFloor(uint16 flr) {
	for (uint16 i = 1; i <= MaxRooms; i++) {
		if ((Maps[i].PageNumber == flr) && g_lab->_roomsFound->in(i) && Maps[i].x)
			return true;
	}

	return false;
}

/**
 * Figures out which floor, if any, should be gone to if the up arrow is hit
 */
static bool getUpFloor(uint16 *flr) {
	do {
		if (*flr < kFloorUpper)
			(*flr)++;
		else {
			*flr   = kFloorCarnival + 1;
			return false;
		}
	} while ((!onFloor(*flr)) && (*flr <= kFloorCarnival));

	return true;
}

/**
 * Figures out which floor, if any, should be gone to if the down arrow is
 * hit.
 */
static bool getDownFloor(uint16 *flr) {
	do {
		if ((*flr == kFloorLower) || (*flr == 0)) {
			*flr   = 0;
			return false;
		} else if (*flr > kFloorUpper) {
			// Labyrinth specific code
			if (*flr == kFloorHedgeMaze)
				*flr = kFloorUpper;
			else if ((*flr == kFloorCarnival) || (*flr == kFloorMedMaze))
				*flr = kFloorMiddle;
			else if (*flr == kFloorSurMaze)
				*flr = kFloorLower;
			else {
				*flr = 0;
				return false;
			}
		} else
			(*flr)--;

	} while ((!onFloor(*flr)) && *flr);

	return true;
}

/**
 * Draws the map
 */
void LabEngine::drawMap(uint16 curRoom, uint16 curMsg, uint16 flr, bool fadeout, bool fadein) {
	_event->mouseHide();

	if (fadeout)
		_graphics->fade(false, 0);

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);

	Map->drawImage(0, 0);
	drawGadgetList(&_mapGadgetList);

	for (uint16 i = 1; i <= MaxRooms; i++) {
		if ((Maps[i].PageNumber == flr) && _roomsFound->in(i) && Maps[i].x) {
			drawRoomMap(i, (bool)(i == curRoom));
			_music->updateMusic();
		}
	}

	// Makes sure the X is drawn in corridors
	// NOTE: this here on purpose just in case there's some weird
	// condition, like the surreal maze where there are no rooms
	if ((Maps[curRoom].PageNumber == flr) && _roomsFound->in(curRoom) && Maps[curRoom].x)
		drawRoomMap(curRoom, true);

	uint16 tempfloor = flr;

	bool noOverlay = getUpFloor(&tempfloor);

	Gadget *upGadget = _event->getGadget(1);
	Gadget *downGadget = _event->getGadget(2);

	if (noOverlay)
		enableGadget(upGadget);
	else
		disableGadget(upGadget, 12);

	tempfloor = flr;
	noOverlay = getDownFloor(&tempfloor);

	if (noOverlay)
		enableGadget(downGadget);
	else
		disableGadget(downGadget, 12);

	char *sptr;

	// Labyrinth specific code
	if (flr == kFloorLower) {
		if (onFloor(kFloorSurMaze))
			Maze->drawImage(mapScaleX(538), mapScaleY(277));
	} else if (flr == kFloorMiddle) {
		if (onFloor(kFloorCarnival))
			Maze->drawImage(mapScaleX(358), mapScaleY(72));

		if (onFloor(kFloorMedMaze))
			Maze->drawImage(mapScaleX(557), mapScaleY(325));
	} else if (flr == kFloorUpper) {
		if (onFloor(kFloorHedgeMaze))
			HugeMaze->drawImage(mapScaleX(524), mapScaleY(97));
	} else if (flr == kFloorSurMaze) {
		sptr = (char *)_resource->getStaticText(kTextSurmazeMessage).c_str();
		_graphics->flowText(_msgFont, 0, 7, 0, true, true, true, true, mapScaleX(360), 0, mapScaleX(660), mapScaleY(450), sptr);
	}

	if (flr >= kFloorLower && flr <= kFloorCarnival) {
		sptr = (char *)_resource->getStaticText(flr - 1).c_str();
		_graphics->flowTextScaled(_msgFont, 0, 5, 3, true, true, true, true, 14, 75, 134, 97, sptr);
	}

	if ((sptr = _rooms[curMsg]._roomMsg))
		_graphics->flowTextScaled(_msgFont, 0, 5, 3, true, true, true, true, 14, 148, 134, 186, sptr);

	if (fadein)
		_graphics->fade(true, 0);

	_event->mouseShow();
}

/**
 * Processes the map.
 */
void LabEngine::processMap(uint16 curRoom) {
	uint32 place = 1;

	uint16 curMsg   = curRoom;
	uint16 curFloor = Maps[curRoom].PageNumber;

	while (1) {
		// Make sure we check the music at least after every message
		_music->updateMusic();
		IntuiMessage *msg = getMsg();

		if (msg == NULL) {
			_music->updateMusic();

			byte newcolor[3];

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
			uint32 msgClass  = msg->_msgClass;
			uint16 msgCode   = msg->_code;
			uint16 gadgetID  = msg->_gadgetID;
			uint16 qualifier = msg->_qualifier;
			uint16 mouseX    = msg->_mouseX;
			uint16 mouseY    = msg->_mouseY;

			if (((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & qualifier)) || ((msgClass == RAWKEY) && (msgCode == 27)))
				return;

			if (msgClass == GADGETUP) {
				if (gadgetID == 0) {
					// Quit menu button
					return;
				} else if (gadgetID == 1) {
					// Up arrow
					uint16 oldFloor = curFloor;
					bool drawmap = getUpFloor(&curFloor);

					if (drawmap) {
						_graphics->fade(false, 0);
						drawMap(curRoom, curMsg, curFloor, false, false);
						_graphics->fade(true, 0);
					} else
						curFloor = oldFloor;
				} else if (gadgetID == 2) {
					// Down arrow
					uint16 oldFloor = curFloor;
					bool drawmap = getDownFloor(&curFloor);

					if (drawmap) {
						_graphics->fade(false, 0);
						drawMap(curRoom, curMsg, curFloor, false, false);
						_graphics->fade(true, 0);
					} else
						curFloor = oldFloor;
				}
			} else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & qualifier)) {
				if ((curFloor == kFloorLower) && (mouseX >= mapScaleX(538)) && (mouseY >= mapScaleY(277))
					  && (mouseX <= mapScaleX(633)) && (mouseY <= mapScaleY(352))
					  && onFloor(kFloorSurMaze)) {
					curFloor = kFloorSurMaze;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((curFloor == kFloorMiddle) && (mouseX >= mapScaleX(358)) && (mouseY >= mapScaleY(71))
							  && (mouseX <= mapScaleX(452)) && (mouseY <= mapScaleY(147))
							  && onFloor(kFloorCarnival)) {
					curFloor = kFloorCarnival;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((curFloor == kFloorMiddle) && (mouseX >= mapScaleX(557)) && (mouseY >= mapScaleY(325))
						  && (mouseX <= mapScaleX(653)) && (mouseY <= mapScaleY(401))
						  && onFloor(kFloorMedMaze)) {
					curFloor = kFloorMedMaze;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((curFloor == kFloorUpper) && (mouseX >= mapScaleX(524)) && (mouseY >=  mapScaleY(97))
						  && (mouseX <= mapScaleX(645)) && (mouseY <= mapScaleY(207))
						  && onFloor(kFloorHedgeMaze)) {
					curFloor = kFloorHedgeMaze;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if (mouseX > mapScaleX(314)) {
					uint16 oldMsg = curMsg;
					uint16 x1, y1, x2, y2;

					for (uint16 i = 1; i <= MaxRooms; i++) {
						roomCoords(i, &x1, &y1, &x2, &y2);

						if ((Maps[i].PageNumber == curFloor)
							  && _roomsFound->in(i)
							  && (mouseX >= x1) && (mouseX <= x2)
							  && (mouseY >= y1) && (mouseY <= y2)) {
							curMsg = i;
						}
					}

					if (oldMsg != curMsg) {
						if (_rooms[curMsg]._roomMsg == nullptr)
							_resource->readViews(curMsg);

						char *sptr;
						if ((sptr = _rooms[curMsg]._roomMsg)) {
							_event->mouseHide();
							_graphics->setAPen(3);
							_graphics->rectFillScaled(13, 148, 135, 186);
							_graphics->flowTextScaled(_msgFont, 0, 5, 3, true, true, true, true, 14, 148, 134, 186, sptr);

							if (Maps[oldMsg].PageNumber == curFloor)
								drawRoomMap(oldMsg, (bool)(oldMsg == curRoom));

							roomCoords(curMsg, &x1, &y1, &x2, &y2);
							x1 = (x1 + x2) / 2;
							y1 = (y1 + y2) / 2;

							if ((curMsg != curRoom) && (Maps[curMsg].PageNumber == curFloor)) {
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
void LabEngine::doMap(uint16 curRoom) {
	static uint16 amigaMapPalette[] = {
		0x0BA8, 0x0C11, 0x0A74, 0x0076,
		0x0A96, 0x0DCB, 0x0CCA, 0x0222,
		0x0444, 0x0555, 0x0777, 0x0999,
		0x0AAA, 0x0ED0, 0x0EEE, 0x0694
	};

	_graphics->FadePalette = amigaMapPalette;

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

	_event->attachGadgetList(&_mapGadgetList);
	drawMap(curRoom, curRoom, Maps[curRoom].PageNumber, false, true);
	_event->mouseShow();
	_graphics->screenUpdate();
	processMap(curRoom);
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
