/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "queen/logic.h"

QueenLogic::QueenLogic(QueenResource *resource) 
	: _maxAnimatedFrame(0), _maxStaticFrame(0), _resource(resource) {
	_jas = _resource->loadFile("QUEEN.JAS", 20);
	
	initialise();
}

QueenLogic::~QueenLogic() {
	free (_jas);
	//free(_graphicData);
}

void QueenLogic::initialise() {
	uint16 i;
	uint8 *ptr = _jas;

	//_display->loadFont();
	
	_numRooms = READ_BE_UINT16(ptr);
	ptr += 2;
	_numNames = READ_BE_UINT16(ptr);
	ptr += 2;
	_numObjects = READ_BE_UINT16(ptr);
	ptr += 2;
	_numDescriptions = READ_BE_UINT16(ptr);
	ptr += 2;
	

	//Object data
#ifdef USE_STRUCTS_JAS
	_objectData = new ObjectData[_numObjects + 1];
	//clear first object
	_objectData[0].name = 0;
	_objectData[0].x = 0;
	_objectData[0].y = 0;
	_objectData[0].description = 0;
	_objectData[0].entryObj = 0;
	_objectData[0].room = 0;
	_objectData[0].state = 0;
	_objectData[0].image = 0;	
	for (i = 1; i < (_numObjects + 1); i++) {
		_objectData[i].name = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_objectData[i].x = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectData[i].y = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectData[i].description = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectData[i].entryObj = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_objectData[i].room = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectData[i].state = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_objectData[i].image = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
	}
#else
	_objectData = new int16[_numObjects + 1][8];
		
	//clear first object
	for (uint16 j = 0; j < 8; j++)
		_objectData[0][j] = 0;
	
	for (i = 1; i < (_numObjects + 1); i++)
		for (uint16 j = 0; j < 8; j++) {
			_objectData[i][j] = (int16)READ_BE_UINT16(ptr);
			ptr += 2;
		}
#endif
	
	//Room data
	_roomData = new uint16[_numRooms + 2];
	for (i = 1; i < (_numRooms + 2); i++) {
		_roomData[i] = READ_BE_UINT16(ptr);
		ptr += 2;
	}

	_roomData[_numRooms + 1] = _numObjects;

	//SFX Name
	// the following table isn't available in demo version
	if (_resource->isDemo()) {
		_sfxName = NULL;
	}
	else {
		_sfxName = new uint16[_numRooms + 1];

		for (i = 0; i < (_numRooms + 1); i++) {
			_sfxName[i] = READ_BE_UINT16(ptr);
			ptr += 2;
		}	
	}

	//Item information
	_numItems = READ_BE_UINT16(ptr);
	ptr += 2;

	_itemData = new uint16[_numItems + 1][5];
	
	for (i = 1; i < (_numItems + 1); i++) {
		_itemData[i][0] = READ_BE_UINT16(ptr);
		ptr += 2;
		for (uint16 j = 1; j < 5; j++) {
		       _itemData[i][j] = READ_BE_UINT16(ptr);
		       ptr += 2;
		}
	}
		
	//Graphic Image Data

	_numGraphics = READ_BE_UINT16(ptr);
	ptr += 2;

	_graphicData = new GraphicData[_numGraphics + 1];

	for (i = 1; i < (_numGraphics + 1); i++) {
		_graphicData[i].x = READ_BE_UINT16(ptr);
		ptr += 2;
		_graphicData[i].y = READ_BE_UINT16(ptr);
		ptr += 2;
		_graphicData[i].firstFrame = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_graphicData[i].lastFrame = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_graphicData[i].speed = READ_BE_UINT16(ptr);
		ptr += 2;
	}
	
	_objMax = new uint16[_numRooms + 1];
	_areaMax = new uint16[_numRooms + 1];
	_area = new int16[_numRooms + 1][11][8];
/*
	for (uint16 i = 1; i < (_numRooms + 1); i++) {
		_objMax[i] = READ_BE_UINT16(ptr);
		ptr += 2;
		_areaMax[i] = READ_BE_UINT16(ptr);
		ptr += 2;
		
		for (uint16 j = 1; j < (_areaMax[i] + 1); j++)
			for (uint16 k = 0; k < 8; k++) {
				_area[i][j][k] = READ_BE_UINT16(ptr);
				ptr += 2;
			}
				
	}	
	
	_objectBox = new uint16[_numObjects + 1][4];
	for (uint16 i = 1; i < (_numObjects + 1); i++)
		for (uint16 j = 0; j < 4; j++) {
			_objectBox[i][j] = READ_BE_UINT16(ptr);
			ptr += 2;
		}
		

*/	

	_numWalkOffs = 0;
}

uint16 QueenLogic::currentRoom() {
	return _currentRoom;
}

void QueenLogic::currentRoom(uint16 room) {
	_currentRoom = room;
}

void QueenLogic::oldRoom(uint16 room) {
	_oldRoom = room;
}

#ifdef USE_STRUCTS_JAS
ObjectData* QueenLogic::objectData(int index) {
  return &_objectData[index];
}
#else
int16* QueenLogic::objectData(int index) {
	return _objectData[index];
}
#endif

uint16 QueenLogic::roomData(int room) {
	return _roomData[room];
}

uint16 QueenLogic::objMax(int room) {
	return _objMax[room];
}

int16 *QueenLogic::area(int index, int subIndex) {
	return _area[index][subIndex];
}

uint16 QueenLogic::walkOffCount() {
	return _numWalkOffs;
}

uint16 *QueenLogic::walkOffData(int index) {
	return _walkOffData[index];
}

GraphicData* QueenLogic::findGraphic(int index) {
	return &_graphicData[index];
}

#ifdef USE_STRUCTS_JAS
uint16 QueenLogic::findBob(uint16 obj) {

	uint16 i;
	uint16 bobnum = 0;
	uint16 bobtype = 0; // 1 for animated, 0 for static

	if (obj > _numObjects)
		error("Object index (%i) > _numObjects (%i)", obj, _numObjects);

	uint16 room = _objectData[obj].room;

	if (room >= _numRooms) {
		warning("room (%i) > _numRooms (%i)", room, _numRooms);
	}
	
	int16 img = _objectData[obj].image;
	if(img != 0) {
		if(img == -3 || img == -4) {
			// a person object
			for(i = _roomData[room] + 1; i <= obj; ++i) {
				img = _objectData[i].image;
				if(img == -3 || img == -4) {
					++bobnum;
				}
			}
		}
		else {
			if(img <= -10) {
				// object has been turned off, but the image order hasn't been updated
				if(_graphicData[-(img + 10)].lastFrame != 0) {
					bobtype = 1;
				}
			}
			else if(img == -2) {
				// -1 static, -2 animated
				bobtype = 1;
			}
			else if(img > 0) {
				if(_graphicData[img].lastFrame != 0) {
					bobtype = 1;
				}
			}

			uint16 idxAnimated = 0;
			uint16 idxStatic = 0;
			for(i = _roomData[room] + 1; i <= obj; ++i) {
				img = _objectData[i].image;
				if(img <= -10) {
					if(_graphicData[-(img + 10)].lastFrame != 0) {
						++idxAnimated;
					}
					else {
						++idxStatic;
					}
				}
				else if(img > 0) {
					if(img > 5000) {
						img -= 5000;
					}

					if (img >= _numGraphics)
						warning("img (%i) >= _numGraphics (%i)", img, _numGraphics);
					
					if(_graphicData[img].lastFrame != 0) {
						++idxAnimated;
					}
					else {
						++idxStatic;
					}
				}
			}
			// FIXME: _max*Frame variables should initialized in SETUP_FURNITURE and DISP_ROOM
			if(bobtype == 0) {
				// static bob
				bobnum = 19 + _maxStaticFrame + idxStatic;
			}
			else {
				// animated bob
				bobnum = 4 + _maxAnimatedFrame + idxAnimated;

			}
		}
	}
	return bobnum;
}


uint16 QueenLogic::findFrame(uint16 obj) {

	uint16 i;
	uint16 framenum = 0;

	uint16 room = _objectData[obj].room;
	int16 img = _objectData[obj].image;
	if(img == -3 || img == -4) {
		uint16 bobnum = 0;
		for(i = _roomData[room] + 1; i <= obj; ++i) {
			img = _objectData[i].image;
			if(img == -3 || img == -4) {
				++bobnum;
			}
		}
		if(bobnum <= 3) {
			framenum = 29 + FRAME_XTRA;
		}
	}
	else {
		uint16 idx = 0;
		for(i = _roomData[room] + 1; i < obj; ++i) {
			img = _objectData[i].image;
			if(img <= -10) {
				GraphicData* pgd = &_graphicData[-(img + 10)];
				if(pgd->lastFrame != 0) {
					// skip all the frames of the animation
					idx += ABS(pgd->lastFrame) - pgd->firstFrame + 1;
				}
				else {
					++idx;
				}
			}
			else if(img == -1) {
				++idx;
			}
			else if(img > 0) {
				if(img > 5000) {
					img -= 5000;
				}
				GraphicData* pgd = &_graphicData[img];
				uint16 lastFrame = ABS(pgd->lastFrame);
				if(pgd->firstFrame < 0) {
					idx += lastFrame;
				}
				else if(lastFrame != 0) {
					idx += (lastFrame - pgd->firstFrame) + 1;
				}
				else {
					++idx;
				}
			}
		}

		img = _objectData[obj].image;
		if(img <= -10) {
			GraphicData* pgd = &_graphicData[-(img + 10)];
			if(pgd->lastFrame != 0) {
				idx += ABS(pgd->lastFrame - pgd->firstFrame) + 1;
			}
			else {
				++idx;
			}
		}
		else if(img == -1 || img > 0) {
			++idx;
		}

		// calculate only if there are person frames
		if(idx > 0) {
			framenum = 36 + _maxStaticFrame + _maxAnimatedFrameLen + idx + FRAME_XTRA;
		}
	}
	return framenum;
}
#endif
