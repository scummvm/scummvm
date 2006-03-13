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
	byte *screenData = data().data();
	int cellY;

	// Reset all the cells to false
	for (cellY = 0; cellY < FULL_VERT_RECTS; ++cellY) 
		for (int cellX = 0; cellX < FULL_HORIZ_RECTS; ++cellX) 
			_cells[cellY][cellX] = false;

	// Loop through each cell of the screen
	for (cellY = 0; cellY < NUM_VERT_RECTS; ++cellY) {
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

			_cells[cellY + NUM_EDGE_RECTS][cellX + NUM_EDGE_RECTS] = hasPixels;
		}
	}
}

/*--------------------------------------------------------------------------*/

Room::Room(): _screen(Screen::getReference()) {
	int_room = this;

	_roomData = NULL;
	_talkDialog = NULL;
	_hotspotId = 0;
	_hotspotName[0] = '\0';
	for (int ctr = 0; ctr < MAX_NUM_LAYERS; ++ctr) _layers[ctr] = NULL;
	_numLayers = 0;
	_showInfo = false;
	_isExit = false;
}

Room::~Room() {
	for (int layerNum = 0; layerNum < _numLayers; ++layerNum)
		if (_layers[layerNum])
			delete _layers[layerNum];

	if (_talkDialog) delete _talkDialog;
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
	Resources &res = Resources::getReference();
	HotspotDataList &list = res.hotspotData();
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
			// For character hotspots, validate they're in clipping range
			skipFlag = !res.checkHotspotExtent(entry);
		
		if (!skipFlag && (entry->hotspotId >= 0x2710) && (entry->hotspotId <= 0x27ff)) {
			RoomExitJoinData *rec = res.getExitJoin(entry->hotspotId);
			if ((rec) && (!rec->blocked))
				// Hotspot is over a room exit, and it's not blocked, so don't 
				// register it as an active hotspot
				skipFlag = true;
		}

		if (!skipFlag) {
			// Check for a hotspot override
			HotspotOverrideData *hsEntry = res.getHotspotOverride(entry->hotspotId);

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
		_isExit = false;
	}
}

uint8 Room::checkRoomExits() {
	Mouse &m = Mouse::getReference();
	Resources &res = Resources::getReference();

	RoomExitHotspotList &exits = _roomData->exitHotspots;
	if (exits.isEmpty()) return CURSOR_ARROW;
	RoomExitJoinData *join;
	bool skipFlag;

	RoomExitHotspotList::iterator i;
	for (i = exits.begin(); i != exits.end(); ++i) {
		RoomExitHotspotData *rec = *i;
		skipFlag = false;

		if (rec->hotspotId != 0) {
			join = res.getExitJoin(rec->hotspotId);
			if ((join) && (join->blocked != 0))
				skipFlag = true;
		}

		if (!skipFlag && (m.x() >= rec->xs) && (m.x() <= rec->xe) &&
			(m.y() >= rec->ys) && (m.y() <= rec->ye)) {
			// Cursor is within exit area
			uint8 cursorNum = rec->cursorNum;

			// If it's a hotspotted exit, change arrow to the + arrow
			if (rec->hotspotId != 0) {
				_hotspotId = rec->hotspotId;
				_hotspot = res.getHotspot(_hotspotId);
				_hotspotNameId = _hotspot->nameId;
				_isExit = true;
				cursorNum += 7;
			}

			return cursorNum;
		}
	}

	// No room exits found
	return CURSOR_ARROW;
}

void Room::flagCoveredCells(Hotspot &h) {
	int16 yStart = (h.y() - MENUBAR_Y_SIZE) / RECT_SIZE;
	int16 yEnd = (h.y() + h.heightCopy() - 1 - MENUBAR_Y_SIZE) / RECT_SIZE;
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
			_cells[index+xP] = true;
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
			sprintf(buffer, "%x", h.hotspotId());
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
	int16 yEnd = (hsY + h.heightCopy() - 1) / RECT_SIZE;
	int16 numY = yEnd - yStart + 1;

	for (int16 xCtr = 0; xCtr < numX; ++xCtr, ++xStart) {
		int16 xs = xStart - 4;
		if (xs < 0) continue;

		// Check foreground layers for an occupied one

		int layerNum = _numLayers - 1;
		while ((layerNum > 0) && !_layers[layerNum]->isOccupied(xStart, yEnd)) 
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
}

void Room::update() {
	Surface &s = _screen.screen();
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	HotspotList &hotspots = res.activeHotspots();
	HotspotList::iterator i;

	memset(_cells, false, NUM_HORIZ_RECTS*NUM_VERT_RECTS);

	// Copy the background to the temporary srceen surface
	_layers[0]->copyTo(&s);

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
		int16 endY = h->y() + h->heightCopy();

		for (iTemp = tempList.begin(); iTemp != tempList.end(); ++iTemp) {
			Hotspot *hTemp = iTemp.operator*();
			int16 tempY = hTemp->y() + hTemp->heightCopy();
			if (endY < tempY) break; 
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

	// Loop to add in any remaining cells
	for (int yp = 0; yp < NUM_VERT_RECTS; ++yp) {
		for (int xp = 0; xp < NUM_HORIZ_RECTS; ++xp) {
			if (_cells[yp*NUM_HORIZ_RECTS+xp]) continue;

			int layerNum = _numLayers - 1;
			while ((layerNum > 0) && !_layers[layerNum]->isOccupied(xp+4, yp+4))
				--layerNum;
			if (layerNum != 0) 
				// Add in the cell
				addCell(xp, yp, layerNum);
		}
	}

	// Show any active talk dialog 
	if (_talkDialog)  {
		_talkDialog->surface().copyTo(&s, _talkDialogX, _talkDialogY);
	}

	// Handle showing name of highlighted hotspot
	if (_hotspotId != 0) {
		Action action = res.getCurrentAction();
		uint16 usedId = res.fieldList().getField(USE_HOTSPOT_ID);

		if (action == NONE) {
			s.writeString(0, 0, _hotspotName, false, DIALOG_TEXT_COLOUR);
		} else {
			char buffer[MAX_DESC_SIZE];
			strcpy(buffer, res.getCurrentActionStr());
			strcat(buffer, " ");

			if (usedId != 0xffff) {
				uint16 nameId = res.getHotspot(usedId)->nameId;
				strings.getString(nameId, buffer + strlen(buffer), NULL, NULL);
				if (action == GIVE) strcat(buffer, " to ");
				else strcat(buffer, " on ");
			}

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
	checkCursor();

	update();
}

// checkCursor
// Called repeatedly to check if any changes to the cursor are required

void Room::checkCursor() {
	Mouse &mouse = Mouse::getReference();
	Resources &res = Resources::getReference();
	uint16 oldHotspotId = _hotspotId;
	uint16 currentCursor = mouse.getCursorNum();
	uint16 newCursor = currentCursor;

	if ((currentCursor >= CURSOR_TIME_START) && (currentCursor <= CURSOR_TIME_END)) {
		++newCursor;
		if (newCursor == CURSOR_CROSS) newCursor = CURSOR_TIME_START;
	} else if (checkInTalkDialog()) {
		newCursor = CURSOR_TALK;
	} else if (res.getTalkData()) {
		newCursor = CURSOR_ARROW;
	} else if (mouse.y() < MENUBAR_Y_SIZE) {
		// If viewing a room remotely, then don't change to the menu cursor
		uint16 oldRoomNumber = res.fieldList().getField(OLD_ROOM_NUMBER);
		if (oldRoomNumber != 0) return;
		
		newCursor = CURSOR_MENUBAR;
	} else {
		// Check for a highlighted hotspot
		checkRoomHotspots();
		
		if (_hotspotId != 0) {
			newCursor = CURSOR_CROSS;
		} else {
			newCursor = checkRoomExits();
		}

		if (oldHotspotId != _hotspotId) 
			StringData::getReference().getString(_hotspotNameId, _hotspotName, NULL, NULL);
	}

	if (mouse.getCursorNum() != newCursor) 
		mouse.setCursorNum(newCursor);
}

void Room::setTalkDialog(uint16 characterId, uint16 stringId) {
	if (_talkDialog) {
		delete _talkDialog;
		_talkDialog = NULL;
	}

	Resources &res = Resources::getReference();
	res.setTalkingCharacter(characterId);

	if (characterId == 0) {
		if (res.getTalkState() == TALK_RESPONSE_WAIT)
			res.setTalkState(TALK_RESPOND_2);
		return;
	}

	HotspotData *character = res.getHotspot(characterId);
	if (character->roomNumber != _roomNumber)
		return;

	_talkDialog = new TalkDialog(characterId, stringId);
	_talkDialogX = character->startX + (character->width / 2) - (TALK_DIALOG_WIDTH / 2);

	if (_talkDialogX < 0) _talkDialogX = 0;
	if (_talkDialogX + TALK_DIALOG_WIDTH >= FULL_SCREEN_WIDTH - 10)
		_talkDialogX = FULL_SCREEN_WIDTH - 10 - TALK_DIALOG_WIDTH; 

	_talkDialogY = TALK_DIALOG_Y;
}

// Checks to see if a talk dialog is active, and if so if the mouse is in
// within it

bool Room::checkInTalkDialog() {
	// Make sure there is a talk dialog active
	if (!_talkDialog) return false;

	// Check boundaries
	Mouse &mouse = Mouse::getReference();
	return ((mouse.x() >= _talkDialogX) && (mouse.y() >= _talkDialogY) &&
		(mouse.x() < _talkDialogX + _talkDialog->surface().width()) &&
		(mouse.y() < _talkDialogY + _talkDialog->surface().height()));
}

} // end of namespace Lure
