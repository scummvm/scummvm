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
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {

/*---------------------------------------------------------------------------*/
/*------------------------------ The Map stuff ------------------------------*/
/*---------------------------------------------------------------------------*/

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

void LabEngine::loadMapData() {
	Common::File *mapImages = _resource->openDataFile("P:MapImage");

	_imgMap = new Image(mapImages, this);
	_imgRoom = new Image(mapImages, this);
	_imgUpArrowRoom = new Image(mapImages, this);
	_imgDownArrowRoom = new Image(mapImages, this);
	_imgHRoom = new Image(mapImages, this);
	_imgVRoom = new Image(mapImages, this);
	_imgMaze = new Image(mapImages, this);
	_imgHugeMaze = new Image(mapImages, this);

	_imgMapX[NORTH] = new Image(mapImages, this);
	_imgMapX[EAST] = new Image(mapImages, this);
	_imgMapX[SOUTH] = new Image(mapImages, this);
	_imgMapX[WEST] = new Image(mapImages, this);
	_imgPath = new Image(mapImages, this);
	_imgBridge = new Image(mapImages, this);

	_mapButtonList.push_back(_event->createButton( 8,  _utils->vgaScaleY(105), 0, VKEY_LTARROW, new Image(mapImages, this), new Image(mapImages, this)));	// back
	_mapButtonList.push_back(_event->createButton( 55, _utils->vgaScaleY(105), 1, VKEY_UPARROW, new Image(mapImages, this), new Image(mapImages, this)));	// up
	_mapButtonList.push_back(_event->createButton(101, _utils->vgaScaleY(105), 2, VKEY_DNARROW, new Image(mapImages, this), new Image(mapImages, this)));	// down

	delete mapImages;

	Common::File *mapFile = _resource->openDataFile("Lab:Maps", MKTAG('M', 'A', 'P', '0'));
	_music->updateMusic();
	if (!_music->_loopSoundEffect)
		_music->stopSoundEffect();

	_maxRooms = mapFile->readUint16LE();
	_maps = new MapData[_maxRooms];	// will be freed when the user exits the map
	for (int i = 0; i < _maxRooms; i++) {
		_maps[i]._x = mapFile->readUint16LE();
		_maps[i]._y = mapFile->readUint16LE();
		_maps[i]._pageNumber = mapFile->readUint16LE();
		_maps[i]._specialID = mapFile->readUint16LE();
		_maps[i]._mapFlags = mapFile->readUint32LE();
	}

	delete mapFile;
}

void LabEngine::freeMapData() {
	_event->freeButtonList(&_mapButtonList);

	delete _imgMap;
	delete _imgRoom;
	delete _imgUpArrowRoom;
	delete _imgDownArrowRoom;
	delete _imgBridge;
	delete _imgHRoom;
	delete _imgVRoom;
	delete _imgMaze;
	delete _imgHugeMaze;
	delete _imgPath;
	for (int i = 0; i < 4; i++)
		delete _imgMapX[i];
	delete[] _maps;

	_imgMap = nullptr;
	_imgRoom = nullptr;
	_imgUpArrowRoom = nullptr;
	_imgDownArrowRoom = nullptr;
	_imgBridge = nullptr;
	_imgHRoom = nullptr;
	_imgVRoom = nullptr;
	_imgMaze = nullptr;
	_imgHugeMaze = nullptr;
	_imgPath = nullptr;
	for (int i = 0; i < 4; i++)
		_imgMapX[i] = nullptr;
	_maps = nullptr;
}

Common::Rect LabEngine::roomCoords(uint16 curRoom) {
	Image *curRoomImg = nullptr;

	switch (_maps[curRoom]._specialID) {
	case NORMAL:
	case UPARROWROOM:
	case DOWNARROWROOM:
		curRoomImg = _imgRoom;
		break;
	case BRIDGEROOM:
		curRoomImg = _imgBridge;
		break;
	case VCORRIDOR:
		curRoomImg = _imgVRoom;
		break;
	case HCORRIDOR:
		curRoomImg = _imgHRoom;
		break;
	default:
		// Some rooms (like the map) do not have an image
		break;
	}

	int x1 = _utils->mapScaleX(_maps[curRoom]._x);
	int y1 = _utils->mapScaleY(_maps[curRoom]._y);
	int x2 = x1;
	int y2 = y1;

	if (curRoomImg) {
		x2 += curRoomImg->_width;
		y2 += curRoomImg->_height;
	}

	return Common::Rect(x1, y1, x2, y2);
}

void LabEngine::drawRoomMap(uint16 curRoom, bool drawMarkFl) {
	uint16 drawX, drawY, offset;

	uint16 x = _utils->mapScaleX(_maps[curRoom]._x);
	uint16 y = _utils->mapScaleY(_maps[curRoom]._y);
	uint32 flags = _maps[curRoom]._mapFlags;

	switch (_maps[curRoom]._specialID) {
	case NORMAL:
	case UPARROWROOM:
	case DOWNARROWROOM:
		if (_maps[curRoom]._specialID == NORMAL)
			_imgRoom->drawImage(x, y);
		else if (_maps[curRoom]._specialID == DOWNARROWROOM)
			_imgDownArrowRoom->drawImage(x, y);
		else
			_imgUpArrowRoom->drawImage(x, y);

		offset = (_imgRoom->_width - _imgPath->_width) / 2;

		if ((NORTHDOOR & flags) && (y >= _imgPath->_height))
			_imgPath->drawImage(x + offset, y - _imgPath->_height);

		if (SOUTHDOOR & flags)
			_imgPath->drawImage(x + offset, y + _imgRoom->_height);

		offset = (_imgRoom->_height - _imgPath->_height) / 2;

		if (EASTDOOR & flags)
			_imgPath->drawImage(x + _imgRoom->_width, y + offset);

		if (WESTDOOR & flags)
			_imgPath->drawImage(x - _imgPath->_width, y + offset);

		drawX = x + (_imgRoom->_width - _imgMapX[_direction]->_width) / 2;
		drawY = y + (_imgRoom->_height - _imgMapX[_direction]->_height) / 2;

		break;

	case BRIDGEROOM:
		_imgBridge->drawImage(x, y);

		drawX = x + (_imgBridge->_width - _imgMapX[_direction]->_width) / 2;
		drawY = y + (_imgBridge->_height - _imgMapX[_direction]->_height) / 2;

		break;

	case VCORRIDOR:
		_imgVRoom->drawImage(x, y);

		offset = (_imgVRoom->_width - _imgPath->_width) / 2;

		if (NORTHDOOR & flags)
			_imgPath->drawImage(x + offset, y - _imgPath->_height);

		if (SOUTHDOOR & flags)
			_imgPath->drawImage(x + offset, y + _imgVRoom->_height);

		offset = (_imgRoom->_height - _imgPath->_height) / 2;

		if (EASTDOOR & flags)
			_imgPath->drawImage(x + _imgVRoom->_width, y + offset);

		if (WESTDOOR & flags)
			_imgPath->drawImage(x - _imgPath->_width, y + offset);

		if (EASTBDOOR & flags)
			_imgPath->drawImage(x + _imgVRoom->_width, y - offset - _imgPath->_height + _imgVRoom->_height);

		if (WESTBDOOR & flags)
			_imgPath->drawImage(x - _imgPath->_width, y - offset - _imgPath->_height + _imgVRoom->_height);

		offset = (_imgVRoom->_height - _imgPath->_height) / 2;

		if (EASTMDOOR & flags)
			_imgPath->drawImage(x + _imgVRoom->_width, y - offset - _imgPath->_height + _imgVRoom->_height);

		if (WESTMDOOR & flags)
			_imgPath->drawImage(x - _imgPath->_width, y - offset - _imgPath->_height + _imgVRoom->_height);

		drawX = x + (_imgVRoom->_width - _imgMapX[_direction]->_width) / 2;
		drawY = y + (_imgVRoom->_height - _imgMapX[_direction]->_height) / 2;

		break;

	case HCORRIDOR:
		_imgHRoom->drawImage(x, y);

		offset = (_imgRoom->_width - _imgPath->_width) / 2;

		if (NORTHDOOR & flags)
			_imgPath->drawImage(x + offset, y - _imgPath->_height);

		if (SOUTHDOOR & flags)
			_imgPath->drawImage(x + offset, y + _imgRoom->_height);

		if (NORTHRDOOR & flags)
			_imgPath->drawImage(x - offset - _imgPath->_width + _imgHRoom->_width, y - _imgPath->_height);

		if (SOUTHRDOOR & flags)
			_imgPath->drawImage(x - offset - _imgPath->_width + _imgHRoom->_width, y + _imgRoom->_height);

		offset = (_imgHRoom->_width - _imgPath->_width) / 2;

		if (NORTHMDOOR & flags)
			_imgPath->drawImage(x - offset - _imgPath->_width + _imgHRoom->_width, y - _imgPath->_height);

		if (SOUTHMDOOR & flags)
			_imgPath->drawImage(x - offset - _imgPath->_width + _imgHRoom->_width, y + _imgRoom->_height);

		offset = (_imgRoom->_height - _imgPath->_height) / 2;

		if (EASTDOOR & flags)
			_imgPath->drawImage(x + _imgHRoom->_width, y + offset);

		if (WESTDOOR & flags)
			_imgPath->drawImage(x - _imgPath->_width, y + offset);

		drawX = x + (_imgHRoom->_width - _imgMapX[_direction]->_width) / 2;
		drawY = y + (_imgHRoom->_height - _imgMapX[_direction]->_height) / 2;

		break;

	default:
		return;
	}

	if (drawMarkFl)
		_imgMapX[_direction]->drawImage(drawX, drawY);
}

bool LabEngine::floorVisited(uint16 floorNum) {
	for (uint16 i = 0; i < _maxRooms; i++) {
		if ((_maps[i]._pageNumber == floorNum) && _roomsFound->in(i) && _maps[i]._x)
			return true;
	}

	return false;
}

uint16 LabEngine::getUpperFloor(uint16 floorNum) {
	if ((floorNum == kFloorCarnival) || (floorNum == kFloorNone))
		return kFloorNone;

	for (uint16 i = floorNum; i < kFloorCarnival; i++)
		if (floorVisited(i + 1))
			return i + 1;

	return kFloorNone;
}

uint16 LabEngine::getLowerFloor(uint16 floorNum) {
	if ((floorNum == kFloorLower) || (floorNum == kFloorNone))
		return kFloorNone;

	for (uint16 i = floorNum; i > kFloorLower; i--)
		if (floorVisited(i - 1))
			return i - 1;

	return kFloorNone;
}

void LabEngine::drawMap(uint16 curRoom, uint16 curMsg, uint16 floorNum, bool fadeOut, bool fadeIn) {
	_event->mouseHide();

	if (fadeOut)
		_graphics->fade(false, 0);

	_graphics->setPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);

	_imgMap->drawImage(0, 0);
	_event->drawButtonList(&_mapButtonList);

	for (uint16 i = 1; i <= _maxRooms; i++) {
		if ((_maps[i]._pageNumber == floorNum) && _roomsFound->in(i) && _maps[i]._x) {
			drawRoomMap(i, (bool)(i == curRoom));
			_music->updateMusic();
		}
	}

	// Makes sure the X is drawn in corridors
	// NOTE: this here on purpose just in case there's some weird
	// condition, like the surreal maze where there are no rooms
	if ((_maps[curRoom]._pageNumber == floorNum) && _roomsFound->in(curRoom) && _maps[curRoom]._x)
		drawRoomMap(curRoom, true);

	_event->toggleButton(_event->getButton(1), 12, (getUpperFloor(floorNum) != kFloorNone));	// up button
	_event->toggleButton(_event->getButton(2), 12, (getLowerFloor(floorNum) != kFloorNone));	// down button

	// Labyrinth specific code
	if (floorNum == kFloorLower) {
		if (floorVisited(kFloorSurMaze))
			_imgMaze->drawImage(_utils->mapScaleX(538), _utils->mapScaleY(277));
	} else if (floorNum == kFloorMiddle) {
		if (floorVisited(kFloorCarnival))
			_imgMaze->drawImage(_utils->mapScaleX(358), _utils->mapScaleY(72));

		if (floorVisited(kFloorMedMaze))
			_imgMaze->drawImage(_utils->mapScaleX(557), _utils->mapScaleY(325));
	} else if (floorNum == kFloorUpper) {
		if (floorVisited(kFloorHedgeMaze))
			_imgHugeMaze->drawImage(_utils->mapScaleX(524), _utils->mapScaleY(97));
	} else if (floorNum == kFloorSurMaze) {
		Common::Rect textRect = Common::Rect(_utils->mapScaleX(360), 0, _utils->mapScaleX(660), _utils->mapScaleY(450));
		const char *textPtr = _resource->getStaticText(kTextSurmazeMessage).c_str();
		_graphics->flowText(_msgFont, 0, 7, 0, true, true, true, true, textRect, textPtr);
	}

	if ((floorNum >= kFloorLower) && (floorNum <= kFloorCarnival)) {
		const char *textPrt = _resource->getStaticText(floorNum - 1).c_str();
		_graphics->flowText(_msgFont, 0, 5, 3, true, true, true, true, _utils->vgaRectScale(14, 75, 134, 97), textPrt);
	}

	if (_rooms[curMsg]._roomMsg != "")
		_graphics->flowText(_msgFont, 0, 5, 3, true, true, true, true, _utils->vgaRectScale(14, 148, 134, 186), _rooms[curMsg]._roomMsg.c_str());

	if (fadeIn)
		_graphics->fade(true, 0);

	_event->mouseShow();
}

void LabEngine::processMap(uint16 curRoom) {
	byte place = 1;
	uint16 curMsg = curRoom;
	uint16 curFloor = _maps[curRoom]._pageNumber;

	while (1) {
		// Make sure we check the music at least after every message
		_music->updateMusic();
		IntuiMessage *msg = _event->getMsg();
		if (g_engine->shouldQuit()) {
			_quitLab = true;
			return;
		}

		if (!msg) {
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
			uint16 mouseX    = msg->_mouse.x;
			uint16 mouseY    = msg->_mouse.y;

			if ((msgClass == kMessageRightClick) || ((msgClass == kMessageRawKey) && (msgCode == Common::KEYCODE_ESCAPE)))
				return;

			if (msgClass == kMessageButtonUp) {
				if (msgCode == 0) {
					// Quit menu button
					return;
				} else if (msgCode == 1) {
					// Up arrow
					uint16 upperFloor = getUpperFloor(curFloor);
					if (upperFloor != kFloorNone) {
						curFloor = upperFloor;
						_graphics->fade(false, 0);
						drawMap(curRoom, curMsg, curFloor, false, false);
						_graphics->fade(true, 0);
					}
				} else if (msgCode == 2) {
					// Down arrow
					uint16 lowerFloor = getLowerFloor(curFloor);
					if (lowerFloor != kFloorNone) {
						curFloor = lowerFloor;
						_graphics->fade(false, 0);
						drawMap(curRoom, curMsg, curFloor, false, false);
						_graphics->fade(true, 0);
					}
				}
			} else if (msgClass == kMessageLeftClick) {
				if ((curFloor == kFloorLower) && _utils->mapRectScale(538, 277, 633, 352).contains(mouseX, mouseY)
					  && floorVisited(kFloorSurMaze)) {
					curFloor = kFloorSurMaze;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((curFloor == kFloorMiddle) && _utils->mapRectScale(358, 71, 452, 147).contains(mouseX, mouseY)
							&& floorVisited(kFloorCarnival)) {
					curFloor = kFloorCarnival;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((curFloor == kFloorMiddle) && _utils->mapRectScale(557, 325, 653, 401).contains(mouseX, mouseY)
							&& floorVisited(kFloorMedMaze)) {
					curFloor = kFloorMedMaze;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if ((curFloor == kFloorUpper) && _utils->mapRectScale(524, 97, 645, 207).contains(mouseX, mouseY)
							&& floorVisited(kFloorHedgeMaze)) {
					curFloor = kFloorHedgeMaze;

					_graphics->fade(false, 0);
					drawMap(curRoom, curMsg, curFloor, false, false);
					_graphics->fade(true, 0);
				} else if (mouseX > _utils->mapScaleX(314)) {
					uint16 oldMsg = curMsg;
					Common::Rect curCoords;

					for (uint16 i = 1; i <= _maxRooms; i++) {
						curCoords = roomCoords(i);

						if ((_maps[i]._pageNumber == curFloor)
							  && _roomsFound->in(i) && curCoords.contains(Common::Point(mouseX, mouseY))) {
							curMsg = i;
						}
					}

					if (oldMsg != curMsg) {
						if (_rooms[curMsg]._roomMsg != "")
							_resource->readViews(curMsg);

						const char *sptr;
						if ((sptr = _rooms[curMsg]._roomMsg.c_str())) {
							_event->mouseHide();
							_graphics->setPen(3);
							_graphics->rectFillScaled(13, 148, 135, 186);
							_graphics->flowText(_msgFont, 0, 5, 3, true, true, true, true, _utils->vgaRectScale(14, 148, 134, 186), sptr);

							if (_maps[oldMsg]._pageNumber == curFloor)
								drawRoomMap(oldMsg, (bool)(oldMsg == curRoom));

							curCoords = roomCoords(curMsg);
							int right = (curCoords.left + curCoords.right) / 2;
							int left = right - 1;
							int top, bottom;
							top = bottom = (curCoords.top + curCoords.bottom) / 2;

							if ((curMsg != curRoom) && (_maps[curMsg]._pageNumber == curFloor)) {
								_graphics->setPen(1);
								_graphics->rectFill(left, top, right, bottom);
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

void LabEngine::doMap(uint16 curRoom) {
	static uint16 amigaMapPalette[] = {
		0x0BA8, 0x0C11, 0x0A74, 0x0076,
		0x0A96, 0x0DCB, 0x0CCA, 0x0222,
		0x0444, 0x0555, 0x0777, 0x0999,
		0x0AAA, 0x0ED0, 0x0EEE, 0x0694
	};

	_graphics->_fadePalette = amigaMapPalette;

	_music->updateMusic();
	loadMapData();
	_graphics->blackAllScreen();
	_event->attachButtonList(&_mapButtonList);
	drawMap(curRoom, curRoom, _maps[curRoom]._pageNumber, false, true);
	_event->mouseShow();
	_graphics->screenUpdate();
	processMap(curRoom);
	_event->attachButtonList(nullptr);
	_graphics->fade(false, 0);
	_graphics->blackAllScreen();
	_event->mouseHide();
	_graphics->setPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);
	freeMapData();
	_graphics->blackAllScreen();
	_event->mouseShow();
	_graphics->screenUpdate();
}

} // End of namespace Lab
