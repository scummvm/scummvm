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
#include "queen/defs.h"
#include "queen/graphics.h"
#include "queen/walk.h"
#include "common/str.h"

namespace Queen {

Logic::Logic(Resource *resource, Graphics *graphics) 
	: _resource(resource), _graphics(graphics) {
	_jas = _resource->loadFile("QUEEN.JAS", 20);
	_joe.x = _joe.y = 0;
	_walk = new Walk(this, _graphics);
	initialise();
}

Logic::~Logic() {
	delete[] _jas;
	delete _walk;
	//free(_graphicData);
}

void Logic::initialise() {
	int16 i, j;
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
		_itemData[i].item = (int16)READ_BE_UINT16(ptr);
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
	_area     = new Area[_numRooms + 1][11];

	for (i = 1; i <= _numRooms; i++) {
		_objMax[i] = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		_areaMax[i] = (int16)READ_BE_UINT16(ptr);
		ptr += 2;
		
		for (j = 1; j <= _areaMax[i]; j++) {
			assert(j < 11);
			_area[i][j].mapNeighbours = READ_BE_UINT16(ptr); ptr += 2;
			_area[i][j].box.x1 = READ_BE_UINT16(ptr); ptr += 2;
			_area[i][j].box.y1 = READ_BE_UINT16(ptr); ptr += 2;
			_area[i][j].box.x2 = READ_BE_UINT16(ptr); ptr += 2;
			_area[i][j].box.y2 = READ_BE_UINT16(ptr); ptr += 2;
			_area[i][j].bottomScaleFactor = READ_BE_UINT16(ptr); ptr += 2;
			_area[i][j].topScaleFactor = READ_BE_UINT16(ptr); ptr += 2;
			_area[i][j].object = READ_BE_UINT16(ptr); ptr += 2;
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
		_objectDescription[i].object = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectDescription[i].type = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectDescription[i].lastDescription = READ_BE_UINT16(ptr);
		ptr += 2;
		_objectDescription[i].seenCount = READ_BE_UINT16(ptr);
		ptr += 2;
	}

	//Command List Data


	memset(_zones, 0, sizeof(_zones));
	_oldRoom = 0;
	_entryObj = 0;
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

Area *Logic::area(int room, int num) {
	return &_area[room][num];
}

Area *Logic::currentRoomArea(int num) {
	return &_area[_currentRoom][num];
}

uint16 Logic::areaMax(int room) {
	return _areaMax[room];
}

uint16 Logic::currentRoomAreaMax() {
	return _areaMax[_currentRoom];
}

uint16 Logic::walkOffCount() {
	return _numWalkOffs;
}

WalkOffData *Logic::walkOffData(int index) {
	return &_walkOffData[index];
}

GraphicData *Logic::graphicData(int index) {
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
			if(bobtype == 0) {
				// static bob
				if(idxStatic > 0) {
					bobnum = 19 + _numFurnitureStatic + idxStatic;
				}
			}
			else {
				// animated bob
				if(idxAnimated > 0) {
					bobnum = 4 + _numFurnitureAnimated + idxAnimated;
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
			framenum = 29 + FRAMES_JOE_XTRA + bobnum;
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
					// static bob, skip one frame
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
			framenum = 36 + FRAMES_JOE_XTRA + _numFurnitureStatic + _numFurnitureAnimatedLen + idx;
		}
	}
	return framenum;
}


uint16 Logic::objectForPerson(uint16 bobNum) {

	uint16 bobcur = 0;
	// first object number in the room
	uint16 cur = _roomData[_currentRoom] + 1;
	// last object number in the room
	uint16 last = _roomData[_currentRoom + 1];
	while (cur <= last) {
		int16 image = _objectData[cur].image;
		if (image == -3 || image == -4) {
			// the object is a bob
			++bobcur;
		}
		if (bobcur == bobNum) {
			return cur;
		}
		++cur;
	}
	return 0;
}


WalkOffData *Logic::walkOffPointForObject(uint16 obj) {
	
	uint16 i;
	for (i = 1; i <= _numWalkOffs; ++i) {
		if (_walkOffData[i].entryObj == obj) {
			return &_walkOffData[i];
		}
	}
	return NULL;
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

void Logic::joeWalk(uint16 walk) {
	_joe.walk = walk;
}

void Logic::joeScale(uint16 scale) {
	_joe.scale = scale;
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


void Logic::zoneSet(uint16 screen, uint16 zone, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {

	ZoneSlot *pzs = &_zones[screen][zone];
	pzs->valid = true;
	pzs->box.x1 = x1;
	pzs->box.y1 = y1;
	pzs->box.x2 = x2;
	pzs->box.y2 = y2;
}


void Logic::zoneSet(uint16 screen, uint16 zone, const Box& box) {
	
	debug(9, "Logic::zoneSet(%d, %d, (%d,%d), (%d,%d))", screen, zone, box.x1, box.y1, box.x2, box.y2);

	ZoneSlot *pzs = &_zones[screen][zone];
	pzs->valid = true;
	pzs->box = box;
}


uint16 Logic::zoneIn(uint16 screen, uint16 x, uint16 y) {

	int i;
	if (screen == ZONE_PANEL) {
		y -= ROOM_ZONE_HEIGHT;
	}
	for(i = 1; i < MAX_ZONES_NUMBER; ++i) {
		ZoneSlot *pzs = &_zones[screen][i];
		if (pzs->valid && pzs->box.contains(x, y)) {
			return i;
		}
	}
	return 0;
}


uint16 Logic::zoneInArea(uint16 screen, uint16 x, uint16 y) {
	uint16 zone = zoneIn(screen, x, y);
	if (zone <= _objMax[_currentRoom]) {
		zone = 0;
	}
	else {
		zone -= _objMax[_currentRoom];
	}
	return zone;
}


void Logic::zoneClearAll(uint16 screen) {

	int i;
	for(i = 1; i < MAX_ZONES_NUMBER; ++i) {
		_zones[screen][i].valid = false;
	}
}


void Logic::zoneSetup() {

	zoneClearAll(ZONE_ROOM);

	int i;
	int zoneNum;

	// setup objects zones
	uint16 maxObjRoom = _objMax[_currentRoom];
	uint16 objRoomNum = _roomData[_currentRoom];
	zoneNum = 1;
	for (i = objRoomNum + 1; i <= objRoomNum + maxObjRoom; ++i) {
		if (_objectData[i].name != 0) {
			zoneSet(ZONE_ROOM, zoneNum, _objectBox[i]);
		}
		++zoneNum;
	}

	// setup room zones (areas)
	uint16 maxAreaRoom = _areaMax[_currentRoom];
	for (zoneNum = 1; zoneNum <= maxAreaRoom; ++zoneNum) {
		zoneSet(ZONE_ROOM, maxObjRoom + zoneNum, _area[_currentRoom][zoneNum].box);
		_graphics->boxDraw(_area[_currentRoom][zoneNum].box, 18);
	}
}


void Logic::roomErase() {

	_graphics->frameEraseAll(false);
	_graphics->bankErase(15);
	_graphics->bankErase(11);
	_graphics->bankErase(10);
	_graphics->bankErase(12);

	// TODO: TALKHEAD=0;
	// TODO: _display->fadeOut();
	// TODO: credits system
	// TODO: person animations

	uint16 cur = _roomData[_oldRoom] + 1;
	uint16 last = _roomData[_oldRoom + 1];
	while (cur <= last) {
		ObjectData *pod = &_objectData[cur];
		if (pod->name == 0) {
			// object has been deleted, invalidate image
			pod->image = 0;
		}
		else if (pod->image > -4000 && pod->image <= -10) {
			if (_graphicData[ABS(pod->image + 10)].lastFrame == 0) {
				// static Bob
				pod->image = -1;
			}
			else {
				// animated Bob
				pod->image = -2;
			}
		}
		++cur;
	}
}


void Logic::roomSetupFurniture() {
	_numFurnitureStatic = 0;
	_numFurnitureAnimated = 0;
	_numFurnitureAnimatedLen = 0;
	uint16 curImage = 36 + FRAMES_JOE_XTRA;

	// count the furniture and update gameState
	uint16 furnitureTotal = 0;
	uint16 i;
	// FIXME: uncomment when Array< FurnitureData > available in Logic
//	for (i = 1; i <= _numFurnitureData; ++i) {
//		if (_furnitureData[i].room == _currentRoom) {
//			++furnitureTotal;
//			_gameState[furnitureTotal] = _furnitureData[i].gameStateValue;
//		}
//	}
	if (furnitureTotal == 0) {
		return;
	}

	// unpack the furniture from the bank 15
	// there are 3 kinds :
	// - static (bobs), gamestate range = ]0;5000]
	// - animated (bobs), gamestate range = ]0;5000]
	// - static (paste downs), gamestate range = [5000; [

	// unpack the static bobs
	for	(i = 1; i <= furnitureTotal; ++i) {
		int16 obj = _gameState[i];
		if (obj > 0 && obj <= 5000) {
			GraphicData *pgd = &_graphicData[obj];
			if (pgd->lastFrame == 0) {
				++_numFurnitureStatic;
				++curImage;
				_graphics->bankUnpack(pgd->firstFrame, curImage, 15);
				++_numFrames;
				BobSlot *pbs = _graphics->bob(19 + _numFurnitureStatic);
				pbs->active = true;
				pbs->x = pgd->x;
				pbs->y = pgd->y;
				pbs->frameNum = curImage;
			}
		}
	}

	// unpack the animated bobs
	uint16 curBob = 0;
	for  (i = 1; i <= furnitureTotal; ++i) {
		int16 obj = _gameState[i];
		if (obj > 0 && obj <= 5000) {
			GraphicData *pgd = &_graphicData[obj];

			bool rebound = false;
			int16 lastFrame = pgd->lastFrame;
			if (lastFrame < 0) {
				rebound = true;
				lastFrame = -lastFrame;
			}

			if (lastFrame > 0) {
				_numFurnitureAnimatedLen += lastFrame - pgd->firstFrame + 1;
				++_numFurnitureAnimated;
				uint16 image = curImage + 1;
				int k;
				for (k = pgd->firstFrame; k <= pgd->lastFrame; ++k) {
					++curImage;
					_graphics->bankUnpack(k, curImage, 15);
					++_numFrames;
				}
				_graphics->bobAnimNormal(5 + curBob, image, curImage, pgd->speed / 4, rebound, false);
				BobSlot *pbs = _graphics->bob(5 + curBob);
				pbs->x = pgd->x;
				pbs->y = pgd->y;
				++curBob;
			}
		}
	}

	// unpack the paste downs
	++curImage;
	for  (i = 1; i <= furnitureTotal; ++i) {
		int16 obj = _gameState[i];
		if (obj > 5000) {
			obj -= 5000;
			GraphicData *pgd = &_graphicData[obj];
			_graphics->bankUnpack(pgd->firstFrame, curImage, 15);
			_graphics->bobPaste(curImage, pgd->x, pgd->y);
			// no need to increment curImage here, as bobPaste() destroys the 
			// unpacked frame after blitting it
		}
	}
}


void Logic::roomSetupObjects() {
	warning("Logic::roomSetupObjects() not fully implemented");

	uint16 i;
	uint16 curImage = 36 + FRAMES_JOE_XTRA + _numFurnitureStatic + _numFurnitureAnimatedLen;
	uint16 firstRoomObj = _roomData[_currentRoom] + 1;
	uint16 lastRoomObj = _roomData[_currentRoom + 1];

	for (i = 1; i <= 3; ++i) {
		_graphics->bob(i)->active = false;
	}

	// TODO: bobs static/animated
	// TODO: bobs persons

	// paste downs list
	++curImage;
	_numFrames = curImage;
	for (i = firstRoomObj; i <= lastRoomObj; ++i) {
		int16 obj = _objectData[i].image;
		if (obj > 5000) {
			obj -= 5000;
			GraphicData *pgd = &_graphicData[obj];
			_graphics->bankUnpack(pgd->firstFrame, curImage, 15);
			_graphics->bobPaste(curImage, pgd->x, pgd->y);
		}
	}
}


void Logic::roomSetup(const char* room, int comPanel, bool inCutaway) {

	// loads background image
	Common::String bdFile(room);
	bdFile += ".PCX";
	_graphics->backdropLoad(bdFile.c_str(), _currentRoom);

	// setup graphics to enter fullscreen/panel mode
	_graphics->setScreenMode(comPanel, inCutaway);

	// reset sprites table (bounding box...)
	_graphics->bobClearAll();

	// setup any hard-coded palette effect
	// TODO: graphics->check_colors(_currentRoom);

	// load/setup objects associated to this room
	Common::String bkFile(room);
	bkFile += ".BBK";
	_graphics->bankLoad(bkFile.c_str(), 15);
	_numFrames = 37 + FRAMES_JOE_XTRA;
	roomSetupFurniture();
	roomSetupObjects();
}


void Logic::roomDisplay(const char* room, RoomDisplayMode mode, uint16 scale, int comPanel, bool inCutaway) {

	debug(9, "Logic::roomDisplay(%s, %d, %d, %d, %d)", room, mode, scale, comPanel, inCutaway);

	roomErase();
	// TODO: _sound->loadSFX(SFXNAME[_currentRoom]);
	roomSetup(room, comPanel, inCutaway);
	zoneSetup();
	ObjectData *pod = NULL;
	if (mode != RDM_FADE_NOJOE) {
		pod = _walk->joeSetupInRoom(mode != RDM_FADE_JOE_XY, scale);
	}
	if (mode != RDM_NOFADE_JOE) {
		_graphics->update();
		// TODO: _display->fadeIn();
	}
	if (pod != NULL) {
		_walk->joeMove(0, pod->x, pod->y, inCutaway);
	}
}


uint16 Logic::findScale(uint16 x, uint16 y) {
	uint16 scale = 100;
	uint16 areaNum = zoneInArea(ZONE_ROOM, x, y);
	if(areaNum != 0) {
		scale = _area[_currentRoom][areaNum].calcScale(y);
	}
	return scale;
}


} // End of namespace Queen

