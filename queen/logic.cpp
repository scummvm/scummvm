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

namespace Queen {

Logic::Logic(Resource *resource) 
	: _maxAnimatedFrame(0), _maxStaticFrame(0), _resource(resource) {
	_jas = _resource->loadFile("QUEEN.JAS", 20);
	
	initialise();
}

Logic::~Logic() {
	delete[] _jas;
	//free(_graphicData);
}

void Logic::initialise() {
	int16 i, j, k;
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
	for (i = 1; i <= _numObjects; i++) {
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
	
	//Room data
	_roomData = new uint16[_numRooms + 2];
	for (i = 1; i <= (_numRooms + 1); i++) {
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

		for (i = 1; i <= _numRooms; i++) {
			_sfxName[i] = READ_BE_UINT16(ptr);
			ptr += 2;
		}	
	}

	//Item information
	_numItems = READ_BE_UINT16(ptr);
	ptr += 2;

	_itemData = new ItemData[_numItems + 1];	

	for (i = 1; i <= _numItems; i++) {
		_itemData[i].name = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_itemData[i].description = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_itemData[i].state = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_itemData[i].bobFrame = READ_BE_UINT16(ptr);
		ptr += 2;
		_itemData[i].sfxDescription = READ_BE_UINT16(ptr);
		ptr += 2;
	}
		
	//Graphic Image Data

	_numGraphics = READ_BE_UINT16(ptr);
	ptr += 2;

	_graphicData = new GraphicData[_numGraphics + 1];

	for (i = 1; i <= _numGraphics; i++) {
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
	
	_objMax   = new int16[_numRooms + 1];
	_areaMax  = new int16[_numRooms + 1];
	_area     = new int16[_numRooms + 1][11][8];

	for (i = 1; i <= _numRooms; i++) {
		_objMax[i] = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_areaMax[i] = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		
		for (j = 1; j <= _areaMax[i]; j++)
			for (k = 0; k < 8; k++) {
				assert(j < 11);
				_area[i][j][k] = READ_BE_UINT16(ptr);
				ptr += 2;
			}
	}

	_objectBox = new Box[_numObjects + 1];
	for (i = 1; i <= _numObjects; i++) {
		_objectBox[i].x1 = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectBox[i].y1 = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectBox[i].x2 = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectBox[i].y2 = READ_BE_UINT16(ptr);
		ptr += 2;
	}

	//Walk OFF Data

	_numWalkOffs = READ_BE_UINT16(ptr);
	ptr += 2;

	_walkOffData = new WalkOffData[_numWalkOffs + 1];
	for (i = 1; i <= _numWalkOffs; i++) {
		_walkOffData[i].entryObj = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_walkOffData[i].x = READ_BE_UINT16(ptr);
		ptr += 2;
		_walkOffData[i].y = READ_BE_UINT16(ptr);
		ptr += 2;
	}

	//Special Object Descriptions

	_numObjDesc = READ_BE_UINT16(ptr);
	ptr += 2;

	_objectDescription = new ObjectDescription[_numObjDesc + 1];
	for (i = 1; i <= _numObjDesc; i++) {
		_objectDescription[i].field1 = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectDescription[i].field2 = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectDescription[i].field3 = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectDescription[i].field4 = READ_BE_UINT16(ptr);
		ptr += 2;
	}

	//Command List Data

}

uint16 Logic::currentRoom() {
	return _currentRoom;
}

void Logic::currentRoom(uint16 room) {
	_currentRoom = room;
}

void Logic::oldRoom(uint16 room) {
	_oldRoom = room;
}

ObjectData* Logic::objectData(int index) {
  return &_objectData[index];
}

uint16 Logic::roomData(int room) {
	return _roomData[room];
}

uint16 Logic::objMax(int room) {
	return _objMax[room];
}

int16 *Logic::area(int index, int subIndex) {
	return _area[index][subIndex];
}

uint16 Logic::walkOffCount() {
	return _numWalkOffs;
}

WalkOffData *Logic::walkOffData(int index) {
	return &_walkOffData[index];
}

GraphicData *Logic::findGraphic(int index) {
	return &_graphicData[index];
}

uint16 Logic::findBob(uint16 obj) {

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
				else if(img == -1) {
					++idxStatic;
				}
				else if(img == -2) {
					++idxAnimated;
				}
			}
			// FIXME: _max*Frame variables should be initialized in SETUP_FURNITURE and DISP_ROOM
			if(bobtype == 0) {
				// static bob
				if(idxStatic > 0) {
					bobnum = 19 + _maxStaticFrame + idxStatic;
				}
			}
			else {
				// animated bob
				if(idxAnimated > 0) {
					bobnum = 4 + _maxAnimatedFrame + idxAnimated;
				}
			}
		}
	}
	return bobnum;
}


uint16 Logic::findFrame(uint16 obj) {

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
				idx += ABS(pgd->lastFrame) - pgd->firstFrame + 1;
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

void Logic::joeFacing(uint16 dir) {
	_joe.facing = dir;
}

void Logic::joeX(uint16 x) {
	_joe.x = x;
}

void Logic::joeY(uint16 y) {
	_joe.y = y;
}

int16 Logic::gameState(int index) {
	if (index >= 0 && index < GAME_STATE_COUNT)
		return _gameState[index];
	else
		error("[QueenLogic::gameState] invalid index: %i", index);
}

void Logic::gameState(int index, int16 newValue) {
	if (index >= 0 && index < GAME_STATE_COUNT)
		 _gameState[index] = newValue;
	else
		error("[QueenLogic::gameState] invalid index: %i", index);
}


} // End of namespace Queen

