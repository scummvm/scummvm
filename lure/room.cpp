/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/room.h"
#include "lure/luredefs.h"
#include "lure/res.h"
#include "lure/screen.h"
#include "lure/events.h"
#include "lure/strings.h"
#include "lure/scripts.h"

namespace Lure {

static Room *int_room;

RoomLayer::RoomLayer(uint16 screenId, bool backgroundLayer): 
		Surface(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT) {
	loadScreen(screenId);	
	byte cellIndex = 0;
	byte *screenData = data().data();

	memset(_cells, 0xff, FULL_HORIZ_RECTS*FULL_VERT_RECTS);

	// Loop through each cell of the screen
	for (int cellY = 0; cellY < NUM_VERT_RECTS; ++cellY) {
		for (int cellX = 0; cellX < NUM_HORIZ_RECTS; ++cellX) {
			bool hasPixels = false;

			if (backgroundLayer) {
				hasPixels = true;
			} else {
				// Check the cell
				for (int yP = 0; yP < RECT_SIZE; ++yP) {
					if (hasPixels) break;
					byte *linePos = screenData + (cellY * RECT_SIZE + yP + 8) 
						* FULL_SCREEN_WIDTH + (cellX * RECT_SIZE);

					for (int xP = 0; xP < RECT_SIZE; ++xP) {
						hasPixels = *linePos++ != 0;
						if (hasPixels) break;
					}
				}
			}

			_cells[(cellY + NUM_EDGE_RECTS) * FULL_HORIZ_RECTS + NUM_EDGE_RECTS +
				cellX] = !hasPixels ? 0xff : cellIndex++;
		}
	}
}

Room::Room(): _screen(Screen::getReference()) {
	int_room = this;

	_roomData = NULL;
	_hotspotName[0] = '\0';
	for (int ctr = 0; ctr < MAX_NUM_LAYERS; ++ctr) _layers[ctr] = NULL;
	_numLayers = 0;
	_showInfo = false;
	_currentAction = NONE;
}

Room::~Room() {
	for (int layerNum = 0; layerNum < _numLayers; ++layerNum)
		if (_layers[layerNum])
			delete _layers[layerNum];

	int_room = NULL;
}

Room &Room::getReference() {
	return *int_room;
}

// leaveRoom
// Handles leaving the current room

void Room::leaveRoom() {
	Resources &r = Resources::getReference();

	// Deallocate graphical layers from the room
	for (int layerNum = 0; layerNum < _numLayers; ++layerNum)
		if (_layers[layerNum]) {
			delete _layers[layerNum];
			_layers[layerNum] = NULL;
		}

	// Scan through the hotspot list and remove any uneeded entries 
//r.activeHotspots().clear();
	HotspotList &list = r.activeHotspots();
	HotspotList::iterator i = list.begin();
	while (i != list.end()) {
		Hotspot *h = i.operator*();
		if (!h->persistant()) {
			i = list.erase(i);
		} else {
			++i;
		}
	}
}

void Room::loadRoomHotspots() {
	Resources &r = Resources::getReference();
	HotspotDataList &list = r.hotspotData();

	HotspotDataList::iterator i;
	for (i = list.begin(); i != list.end(); ++i) {
		HotspotData *rec = *i;

		if ((rec->hotspotId < 0x7530) && (rec->roomNumber == _roomNumber) && 
			(rec->layer != 0))
			r.activateHotspot(rec->hotspotId);
	}
}

void Room::checkRoomHotspots() {
	Mouse &m = Mouse::getReference();
	Resources &r = Resources::getReference();
	HotspotDataList &list = r.hotspotData();
	HotspotData *entry = NULL;
	int16 currentX = m.x();
	int16 currentY = m.y();

	HotspotDataList::iterator i;
	for (i = list.begin(); i != list.end(); ++i) {
		entry = *i;

		bool skipFlag = (entry->roomNumber != _roomNumber);
		if (!skipFlag) {
			skipFlag = (((entry->flags & 0x80) == 0) && 
				        ((entry->flags & 0x40) != 0)) ||
				       ((entry->flags & 0x20) != 0);
		}

		if ((!skipFlag) && (entry->hotspotId < 0x409))
			skipFlag = sub_112();
		
		if (!skipFlag && (entry->hotspotId >= 0x2710) && (entry->hotspotId <= 0x27ff)) {
			RoomExitJoinData *rec = r.getExitJoin(entry->hotspotId);
			if ((rec) && (!rec->blocked))
				// Hotspot is over a room exit, and it's not blocked, so don't 
				// register it as an active hotspot
				skipFlag = true;
		}

		if (!skipFlag) {
			// Check for a hotspot override
			HotspotOverrideData *hsEntry = r.getHotspotOverride(entry->hotspotId);

			if (hsEntry) {
				// Check whether cursor is in override hotspot area
				if ((currentX >= hsEntry->xs) && (currentX <= hsEntry->xe) &&
					(currentY >= hsEntry->ys) && (currentY <= hsEntry->ye))
					// Found to be in hotspot entry
					break;
			} else {
				// Check whether cursor is in default hospot area
				if ((currentX >= entry->startX) && (currentX < entry->startX + entry->width) && 
					(currentY >= entry->startY) && (currentY < entry->startY + entry->height)) 
					// Found hotspot entry
					break;
			}
		}
	}

	if (i == list.end()) {
		_hotspotId = 0;
		_hotspotNameId = 0;
		_hotspot = NULL;
	} else {
		_hotspotNameId = entry->nameId;
		_hotspot = entry;
		_hotspotId = entry->hotspotId;
	}
}

uint8 Room::checkRoomExits() {
	Mouse &m = Mouse::getReference();
	Resources &r = Resources::getReference();

	RoomExitHotspotList &exits = _roomData->exitHotspots;
	if (exits.isEmpty()) return CURSOR_ARROW;
	RoomExitJoinData *join;
	bool skipFlag;

	RoomExitHotspotList::iterator i;
	for (i = exits.begin(); i != exits.end(); ++i) {
		RoomExitHotspotData *rec = *i;
		skipFlag = false;

		if (rec->hotspotId != 0) {
			join = r.getExitJoin(rec->hotspotId);
			if ((join) && (join->blocked != 0))
				skipFlag = true;
		}

		if (!skipFlag && (m.x() >= rec->xs) && (m.x() <= rec->xe) &&
			(m.y() >= rec->ys) && (m.y() <= rec->ye)) {
			// Cursor is within exit area
			uint8 cursorNum = rec->cursorNum;

			// If it's a hotspotted exit, change arrow to the + arrow
			if (rec->hotspotId != 0) cursorNum += 7;

			return cursorNum;
		}
	}

	// No room exits found
	return CURSOR_ARROW;
}

void Room::flagCoveredCells(Hotspot &h) {
	int16 yStart = (h.y() - MENUBAR_Y_SIZE) / RECT_SIZE;
	int16 yEnd = (h.y() + h.height() - 1 - MENUBAR_Y_SIZE) / RECT_SIZE;
	int16 numY = yEnd - yStart + 1;
	int16 xStart = h.x() / RECT_SIZE;
	int16 xEnd = (h.x() + h.width() - 1) / RECT_SIZE;
	int16 numX = xEnd - xStart + 1;

	int index = yStart * NUM_HORIZ_RECTS + xStart;

	for (int16 yP = 0; yP < numY; ++yP) {
		for (int16 xP = 0; xP < numX; ++xP) {
			int indexPos = index + xP;
			if ((indexPos < 0) || (indexPos >= NUM_HORIZ_RECTS*NUM_VERT_RECTS)) 
				continue;
			_cells[index+xP] |= 0x81;
			_cells2[index+xP] |= 1;
		}
		index += NUM_HORIZ_RECTS;
	}
}

void Room::addAnimation(Hotspot &h) {
	Surface &s = _screen.screen();
	char buffer[10];
	h.copyTo(&s);

	if (_showInfo) {
		int16 x = h.x();
		int16 y = h.y();
		if ((x >= 0) && (x <= 319) && (y >= 0) && (y <= 200)) {
			sprintf(buffer, "%x", h.resource().hotspotId);
			strcat(buffer, "h");
			s.writeString(h.x(), h.y(), buffer, false);
		}
	}
}

void Room::addLayers(Hotspot &h) {
	int16 hsX = h.x() + (4 * RECT_SIZE);
	int16 hsY = h.y() + (4 * RECT_SIZE) - MENUBAR_Y_SIZE;

	int16 xStart = hsX / RECT_SIZE;
	int16 xEnd = (hsX + h.width()) / RECT_SIZE;
	int16 numX = xEnd - xStart + 1;
	int16 yStart = hsY / RECT_SIZE;
	int16 yEnd = (hsY + h.height() - 1) / RECT_SIZE;
	int16 numY = yEnd - yStart + 1;

	for (int16 xCtr = 0; xCtr < numX; ++xCtr, ++xStart) {
		int16 xs = xStart - 4;
		if (xs < 0) continue;

		// Check foreground layers for an occupied one
/* DEBUG
		int layerNum = 1;
		while ((layerNum < _numLayers) &&
					!_layers[layerNum]->isOccupied(xStart, yEnd)) 
			++layerNum;
		if (layerNum == _numLayers) continue;
*/
		int layerNum = _numLayers - 1;
		while ((layerNum > 0) &&
					!_layers[layerNum]->isOccupied(xStart, yEnd)) 
			--layerNum;
		if (layerNum == 0) continue;

		int16 ye = yEnd - 4;
		for (int16 yCtr = 0; yCtr < numY; ++yCtr, --ye) {
			if (ye < 0) break;
			addCell(xs, ye, layerNum);
		}
	}
}

void Room::addCell(int16 xp, int16 yp, int layerNum) {
	Surface &s = _screen.screen();

	while ((layerNum > 0) && !_layers[layerNum]->isOccupied(xp+4, yp+4))
		--layerNum;
	if (layerNum == 0) return;
/* DEBUG
	while ((layerNum < _numLayers) && !_layers[layerNum]->isOccupied(xp+4, yp+4))
		++layerNum;
	if (layerNum == _numLayers) return;
*/
	RoomLayer *layer = _layers[layerNum];

	int index = ((yp * RECT_SIZE + 8) * FULL_SCREEN_WIDTH) + (xp * RECT_SIZE);
	byte *srcPos = layer->data().data() + index;
	byte *destPos = s.data().data() + index;

	for (int yCtr = 0; yCtr < RECT_SIZE; ++yCtr) {
		for (int xCtr = 0; xCtr < RECT_SIZE; ++xCtr, ++destPos) {
			byte pixel = *srcPos++;
			if (pixel) *destPos = pixel;
		}

		// Move to start of next cell line
		srcPos += FULL_SCREEN_WIDTH - RECT_SIZE;
		destPos += FULL_SCREEN_WIDTH - RECT_SIZE;
	}

	// Note: old version of screen layers load compresses loaded layers down to
	// only a set of the non-empty rects. Since modern memory allows me to load
	// all the layers completely, I'm bypassing the need to use cell index values
}

void Room::update() {
	Surface &s = _screen.screen();
	Resources &r = Resources::getReference();
	HotspotList &hotspots = r.activeHotspots();
	HotspotList::iterator i;

	memset(_cells, 0x81, NUM_HORIZ_RECTS*NUM_VERT_RECTS);
	memset(_cells2, 0x81, NUM_HORIZ_RECTS*NUM_VERT_RECTS);

	_layers[0]->copyTo(&s);
	for (int ctr = 1; ctr < _numLayers; ++ctr)
		_layers[ctr]->transparentCopyTo(&s);

	// Handle first layer (layer 3)
	for (i = hotspots.begin(); i != hotspots.end(); ++i) {
		Hotspot &h = *i.operator*();
		if ((h.roomNumber() == _roomNumber) && h.isActiveAnimation() && (h.layer() == 3)) {
			flagCoveredCells(h);
			addAnimation(h);
			addLayers(h);
		}
	}

	// Handle second layer (layer 1) - do in order of Y axis
	List<Hotspot *> tempList;
	List<Hotspot *>::iterator iTemp;
	for (i = hotspots.begin(); i != hotspots.end(); ++i) {
		Hotspot *h = i.operator*();
		if ((h->roomNumber() != _roomNumber) || !h->isActiveAnimation() 
				|| (h->layer() != 1)) 
			continue;
		int16 endY = h->y() + h->height();

		for (iTemp = tempList.begin(); iTemp != tempList.end(); ++iTemp) {
			Hotspot *hTemp = iTemp.operator*();
			int16 tempY = hTemp->y() + hTemp->height();
			if (endY < tempY) {
				if (iTemp != tempList.begin()) --iTemp;
				break;
			}
		}
		tempList.insert(iTemp, h);
	}
	for (iTemp = tempList.begin(); iTemp != tempList.end(); ++iTemp) {
		Hotspot &h = *iTemp.operator*();
		flagCoveredCells(h);
		addAnimation(h);
		addLayers(h);
	}

	// Handle third layer (layer 2)
	for (i = hotspots.begin(); i != hotspots.end(); ++i) {
		Hotspot &h = *i.operator*();
		if ((h.roomNumber() == _roomNumber) && h.isActiveAnimation() && (h.layer() == 2)) {
			flagCoveredCells(h);
			addAnimation(h);
		}
	}

	// Handle showing name of highlighted hotspot
	if (_hotspotName[0] != '\0') {
		if (_currentAction == NONE) {
			s.writeString(0, 0, _hotspotName, false, DIALOG_TEXT_COLOUR);
		} else {
			char buffer[MAX_ACTION_NAME_SIZE + MAX_HOTSPOT_NAME_SIZE];
			strcpy(buffer, actionList[_currentAction]);
			strcat(buffer, " ");
			strcat(buffer, _hotspotName);
			s.writeString(0, 0, buffer, false, DIALOG_WHITE_COLOUR);
		}
	}

	// If show information is turned on, show room and position
	if (_showInfo) {
		char buffer[64];
		Mouse &m = Mouse::getReference();
		sprintf(buffer, "Room %d Pos (%d,%d)", _roomNumber, m.x(), m.y());
		s.writeString(FULL_SCREEN_WIDTH / 2, 0, buffer, false, DIALOG_TEXT_COLOUR);
	}

	_screen.update();
}

void Room::setRoomNumber(uint16 newRoomNumber, bool showOverlay) {
	Resources &r = Resources::getReference();
	_roomData = r.getRoom(newRoomNumber);
	if (!_roomData)
		error("Tried to change to non-existant room: %d", newRoomNumber);

	_roomNumber = _roomData->roomNumber;
	_descId = _roomData->descId;
	
	_screen.empty();
	_screen.resetPalette();

	if (_layers[0]) leaveRoom();

	_numLayers = _roomData->numLayers;
	if (showOverlay) ++_numLayers;

	uint16 paletteId = (_roomData->layers[0] & 0xffe0) - 1;

	for (uint8 layerNum = 0; layerNum < _numLayers; ++layerNum) 
		_layers[layerNum] = new RoomLayer(_roomData->layers[layerNum],
			layerNum == 0);

	// Load in the palette, add in the two replacements segments, and then
	// set to the system palette
	Palette p(228, NULL, RGB64);
	Palette tempPalette(paletteId);
	p.copyFrom(&tempPalette);
	r.insertPaletteSubset(p);
	_screen.setPalette(&p);

	if (_roomData->sequenceOffset != 0xffff)
		Script::execute(_roomData->sequenceOffset);
	loadRoomHotspots();
	cursorMoved();

	update();
}

// cursorMoved
// Called as the cursor moves to handle any changes that must occur

void Room::cursorMoved() {
	uint16 cursorNew = CURSOR_ARROW;
	uint16 oldHotspotId = _hotspotId;

	Mouse &m = Mouse::getReference();
	checkRoomHotspots();

	if (_hotspotId != 0) {
		cursorNew = CURSOR_CROSS;

		if (oldHotspotId != _hotspotId) 
			StringData::getReference().getString(_hotspotNameId, _hotspotName, NULL, NULL);
	} else {
		_hotspotName[0] = '\0';
		cursorNew = checkRoomExits();
	}

	if (m.getCursorNum() != cursorNew) 
		m.setCursorNum(cursorNew);
}

} // end of namespace Lure
