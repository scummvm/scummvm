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

#include "stdafx.h"
#include "queen/logic.h"

#include "common/config-manager.h"
#include "queen/command.h"
#include "queen/cutaway.h"
#include "queen/defs.h"
#include "queen/debug.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/input.h"
#include "queen/journal.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/state.h"
#include "queen/talk.h"
#include "queen/walk.h"


namespace Queen {

const Verb Logic::PANEL_VERBS[] = {
	VERB_NONE,
	VERB_OPEN,
	VERB_CLOSE,
	VERB_MOVE,
	VERB_GIVE,
	VERB_LOOK_AT,
	VERB_PICK_UP,
	VERB_TALK_TO,
	VERB_USE,
	VERB_SCROLL_UP,
	VERB_SCROLL_DOWN,
	VERB_DIGIT_1, // inventory item 1
	VERB_DIGIT_2, // inventory item 2
	VERB_DIGIT_3, // inventory item 3
	VERB_DIGIT_4, // inventory item 4
};


Logic::Logic(QueenEngine *vm)
	: _vm(vm) {
	_joe.x = _joe.y = 0;
	_joe.scale = 100;
	_dbg = new Debug(vm);
	memset(_gameState, 0, sizeof(_gameState));
	memset(_talkSelected, 0, sizeof(_talkSelected));
	initialise();
}

Logic::~Logic() {
	delete _dbg;
}

void Logic::initialise() {
	
	int16 i, j;


	// Step 1 : read queen.jas file and 'unserialize' some arrays

	uint8 *jas = _vm->resource()->loadFile("QUEEN.JAS", 20);
	uint8 *ptr = jas;

	_numRooms = READ_BE_UINT16(ptr); ptr += 2;
	_numNames = READ_BE_UINT16(ptr); ptr += 2;
	_numObjects = READ_BE_UINT16(ptr); ptr += 2;
	_numDescriptions = READ_BE_UINT16(ptr); ptr += 2;

	// Object data
	_objectData = new ObjectData[_numObjects + 1];
	memset(&_objectData[0], 0, sizeof(ObjectData));
	for (i = 1; i <= _numObjects; i++) {
		_objectData[i].readFrom(ptr);
	}

	// Room data
	_roomData = new uint16[_numRooms + 2];
	_roomData[0] = 0;
	for (i = 1; i <= (_numRooms + 1); i++) {
		_roomData[i] = READ_BE_UINT16(ptr);	ptr += 2;
	}
	_roomData[_numRooms + 1] = _numObjects;

	// SFX Name
	// the following table isn't available in demo version
	if (_vm->resource()->isDemo()) {
		_sfxName = NULL;
	}
	else {
		_sfxName = new uint16[_numRooms + 1];
		_sfxName[0] = 0;
		for (i = 1; i <= _numRooms; i++) {
			_sfxName[i] = READ_BE_UINT16(ptr); ptr += 2;
		}	
	}

	// Item information
	_numItems = READ_BE_UINT16(ptr); ptr += 2;

	_itemData = new ItemData[_numItems + 1];
	memset(&_itemData[0], 0, sizeof(ItemData));
	for (i = 1; i <= _numItems; i++) {
		_itemData[i].readFrom(ptr);
	}

	// Graphic Image Data
	_numGraphics = READ_BE_UINT16(ptr); ptr += 2;

	_graphicData = new GraphicData[_numGraphics + 1];
	memset(&_graphicData[0], 0, sizeof(GraphicData));
	for (i = 1; i <= _numGraphics; i++) {
		_graphicData[i].readFrom(ptr);
	}

	_objMax   = new int16[_numRooms + 1];
	_areaMax  = new int16[_numRooms + 1];
	_area     = new Area[_numRooms + 1][MAX_AREAS_NUMBER];

	_objMax[0] = 0;
	_areaMax[0] = 0;
	memset(&_area[0], 0, sizeof(Area) * MAX_AREAS_NUMBER);
	for (i = 1; i <= _numRooms; i++) {
		_objMax[i] = (int16)READ_BE_UINT16(ptr); ptr += 2;
		_areaMax[i] = (int16)READ_BE_UINT16(ptr); ptr += 2;
		memset(&_area[i][0], 0, sizeof(Area));
		for (j = 1; j <= _areaMax[i]; j++) {
			assert(j < MAX_AREAS_NUMBER);
			_area[i][j].readFrom(ptr);
		}
	}

	_objectBox = new Box[_numObjects + 1];
	memset(&_objectBox[0], 0, sizeof(Box));
	for (i = 1; i <= _numObjects; i++) {
		_objectBox[i].readFrom(ptr);
	}

	// Walk OFF Data
	_numWalkOffs = READ_BE_UINT16(ptr);	ptr += 2;

	_walkOffData = new WalkOffData[_numWalkOffs + 1];
	memset(&_walkOffData[0], 0, sizeof(WalkOffData));
	for (i = 1; i <= _numWalkOffs; i++) {
		_walkOffData[i].readFrom(ptr);
	}

	// Special Object Descriptions
	_numObjDesc = READ_BE_UINT16(ptr); ptr += 2;

	_objectDescription = new ObjectDescription[_numObjDesc + 1];
	memset(&_objectDescription[0], 0, sizeof(ObjectDescription));
	for (i = 1; i <= _numObjDesc; i++) {
		_objectDescription[i].readFrom(ptr);
	}

	_vm->command()->readCommandsFrom(ptr);

	_entryObj = READ_BE_UINT16(ptr); ptr += 2;

	// Furniture DATA
	_numFurniture = READ_BE_UINT16(ptr); ptr += 2;

	_furnitureData = new FurnitureData[_numFurniture + 1];
	memset(&_furnitureData[0], 0, sizeof(_furnitureData));
	for (i = 1; i <= _numFurniture; i++) {
		_furnitureData[i].readFrom(ptr);
	}

	// Actors
	_numActors = READ_BE_UINT16(ptr); ptr += 2;
	_numAAnim = READ_BE_UINT16(ptr); ptr += 2;
	_numAName = READ_BE_UINT16(ptr); ptr += 2;
	_numAFile = READ_BE_UINT16(ptr); ptr += 2;

	_actorData = new ActorData[_numActors + 1];
	memset(&_actorData[0], 0, sizeof(ActorData));
	for (i = 1; i <= _numActors; i++) {
		_actorData[i].readFrom(ptr);
	}

	_numGraphicAnim = READ_BE_UINT16(ptr); ptr += 2;
	
	_graphicAnim = new GraphicAnim[_numGraphicAnim + 1];
	memset(&_graphicAnim[0], 0, sizeof(GraphicAnim));
	for (i = 1; i <= _numGraphicAnim; i++) {
		_graphicAnim[i].readFrom(ptr);
	}

	_currentRoom = _objectData[_entryObj].room;
	_entryObj = 0;

	if(memcmp(ptr, _vm->resource()->JASVersion(), 5) != 0) {
		warning("Unexpected queen.jas file format");
	}

	delete[] jas;


	// Step 2 : read queen2.jas and grab all description texts
	
	_objDescription = new char*[_numDescriptions + 1];
	_objDescription[0] = 0;
	for (i = 1; i <= _numDescriptions; i++)
		_objDescription[i] = _vm->resource()->getJAS2Line();

	//Patch for German text bug
	if (_vm->resource()->getLanguage() == GERMAN) {
		char *txt = new char[48];
		strcpy(txt, "Es bringt nicht viel, das festzubinden.");
		_objDescription[296] = txt;
	}
	
	_objName = new char*[_numNames + 1];
	_objName[0] = 0;
	for (i = 1; i <= _numNames; i++)
		_objName[i] = _vm->resource()->getJAS2Line();

	_roomName = new char*[_numRooms + 1];
	_roomName[0] = 0;
	for (i = 1; i <= _numRooms; i++)
		_roomName[i] = _vm->resource()->getJAS2Line();

	_verbName[0] = 0;
	for (i = 1; i <= 12; i++)
		_verbName[i] = _vm->resource()->getJAS2Line();

	_joeResponse[0] = 0;
	for (i = 1; i <= JOE_RESPONSE_MAX; i++)
		_joeResponse[i] = _vm->resource()->getJAS2Line();

	_aAnim = new char*[_numAAnim + 1];
	_aAnim[0] = 0;
	for (i = 1; i <= _numAAnim; i++)
		_aAnim[i] = _vm->resource()->getJAS2Line();

	_aName = new char*[_numAName + 1];
	_aName[0] = 0;
	for (i = 1; i <= _numAName; i++)
		_aName[i] = _vm->resource()->getJAS2Line();
	
	_aFile = new char*[_numAFile + 1];
	_aFile[0] = 0;
	for (i = 1; i <= _numAFile; i++)
		_aFile[i] = _vm->resource()->getJAS2Line();


	// Step 3 : initialise game state / variables

	_vm->command()->clear(false);
	_scene = 0;
	memset(_gameState, 0, sizeof(_gameState));
	_vm->graphics()->loadPanel();
	_vm->graphics()->bobSetupControl();
	joeSetup();
	zoneSetupPanel();

	_oldRoom = 0;
}


ObjectData* Logic::objectData(int index) const {

	if (index < 0) {
		warning("Logic::objectData() called with negative object index: %i", index);
	}
	index = ABS(index); // cyx: is that really necessary ?
	if (index <= _numObjects)
		return &_objectData[index];
	else
		error("[Logic::objectData] Invalid object data index: %i", index);
}

uint16 Logic::roomData(int room) const {
	return _roomData[room];
}

uint16 Logic::objMax(int room) const {
	return _objMax[room];
}

GraphicData *Logic::graphicData(int index) const {
	return &_graphicData[index];
}

Area *Logic::area(int room, int num) const {
	return &_area[room][num];
}

Area *Logic::currentRoomArea(int num) const {
	if (num == 0 || num > _areaMax[_currentRoom]) {
		error("Logic::currentRoomArea() - Bad area number = %d (max = %d), currentRoom = %d", num, _areaMax[_currentRoom], _currentRoom);
	}

	return &_area[_currentRoom][num];
}

uint16 Logic::areaMax(int room) const {
	return _areaMax[room];
}

uint16 Logic::currentRoomAreaMax() const {
	return _areaMax[_currentRoom];
}

uint16 Logic::walkOffCount() const {
	return _numWalkOffs;
}

WalkOffData *Logic::walkOffData(int index) const {
	return &_walkOffData[index];
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


uint16 Logic::objectForPerson(uint16 bobNum) const {

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


WalkOffData *Logic::walkOffPointForObject(uint16 obj) const {
	
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

void Logic::joeWalk(JoeWalkMode walking) {
	_joe.walk = walking;

	// Do this so that Input doesn't need to know the walk value
	_vm->input()->dialogueRunning(JWM_SPEAK == walking);
}

void Logic::joeScale(uint16 scale) {
	_joe.scale = scale;
}

void Logic::joeCutFacing(uint16 dir) {

	_joe.cutFacing = dir;
}

void Logic::joePrevFacing(uint16 dir) {
	_joe.prevFacing = dir;
}

int16 Logic::gameState(int index) {
	if (index >= 0 && index < GAME_STATE_COUNT)
		return _gameState[index];
	else
		error("[QueenLogic::gameState] invalid index: %i", index);
}

void Logic::gameState(int index, int16 newValue) {
	if (index >= 0 && index < GAME_STATE_COUNT) {
//		debug(0, "Logic::gameState() - GAMESTATE[%d] = %d", index, newValue);
		_gameState[index] = newValue;
	}
	else
		error("[QueenLogic::gameState] invalid index: %i", index);
}


void Logic::zoneSet(uint16 screen, uint16 zone, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {

	debug(9, "Logic::zoneSet(%d, %d, (%d,%d), (%d,%d))", screen, zone, x1, y1, x2, y2);
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


uint16 Logic::zoneIn(uint16 screen, uint16 x, uint16 y) const {

	debug(9, "Logic::zoneIn(%d, (%d,%d))", screen, x, y);
	int i;
	if (screen == ZONE_PANEL) {
		y -= ROOM_ZONE_HEIGHT;
	}
	for(i = 1; i < MAX_ZONES_NUMBER; ++i) {
		const ZoneSlot *pzs = &_zones[screen][i];
		if (pzs->valid && pzs->box.contains(x, y)) {
			return i;
		}
	}
	return 0;
}


uint16 Logic::zoneInArea(uint16 screen, uint16 x, uint16 y) const {

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

	debug(9, "Logic::zoneClearAll(%d)", screen);
	int i;
	for(i = 1; i < MAX_ZONES_NUMBER; ++i) {
		_zones[screen][i].valid = false;
	}
}


void Logic::zoneSetup() {

	debug(9, "Logic::zoneSetup()");
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
	}
}


void Logic::zoneSetupPanel() {

	// verbs 
	int i;
	for (i = 0; i <= 7; ++i) {
		int x = i * 20;
		zoneSet(ZONE_PANEL, i + 1, x, 10, x + 19, 49);
	}

	// inventory scrolls
	zoneSet(ZONE_PANEL,  9, 160, 10, 179, 29);
	zoneSet(ZONE_PANEL, 10, 160, 30, 179, 49);

	// inventory items
	zoneSet(ZONE_PANEL, 11, 180, 10, 213, 49);
	zoneSet(ZONE_PANEL, 12, 214, 10, 249, 49);
	zoneSet(ZONE_PANEL, 13, 250, 10, 284, 49);
	zoneSet(ZONE_PANEL, 14, 285, 10, 320, 49);
}


void Logic::roomErase() {

	_vm->graphics()->frameEraseAll(false);
	_vm->graphics()->bankErase(15);
	_vm->graphics()->bankErase(11);
	_vm->graphics()->bankErase(10);
	_vm->graphics()->bankErase(12);

	if (_currentRoom >= 114) {
		_vm->display()->palFadeOut(0, 255, _currentRoom);
	}
	else {
		_vm->display()->palFadeOut(0, 223, _currentRoom);
	}
	
	// TODO: credits system

	// invalidates all persons animations
	uint16 i;
	for (i = 0; i <= 3; ++i) {
		_personFrames[i] = 0;
	}
	for (i = 1; i <= 16; ++i) {
		_newAnim[i][0].frame = 0;
	}

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

	int16 gstate[9];
	_numFurnitureStatic = 0;
	_numFurnitureAnimated = 0;
	_numFurnitureAnimatedLen = 0;
	uint16 curImage = 36 + FRAMES_JOE_XTRA;

	// count the furniture and update gameState
	uint16 furnitureTotal = 0;
	uint16 i;
	for (i = 1; i <= _numFurniture; ++i) {
		if (_furnitureData[i].room == _currentRoom) {
			++furnitureTotal;
			gstate[furnitureTotal] = _furnitureData[i].gameStateValue;
		}
	}
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
		int16 obj = gstate[i];
		if (obj > 0 && obj <= 5000) {
			GraphicData *pgd = &_graphicData[obj];
			if (pgd->lastFrame == 0) {
				++_numFurnitureStatic;
				++curImage;
				_vm->graphics()->bankUnpack(pgd->firstFrame, curImage, 15);
				++_numFrames;
				BobSlot *pbs = _vm->graphics()->bob(19 + _numFurnitureStatic);
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
		int16 obj = gstate[i];
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
				for (k = pgd->firstFrame; k <= lastFrame; ++k) {
					++curImage;
					_vm->graphics()->bankUnpack(k, curImage, 15);
					++_numFrames;
				}
				BobSlot *pbs = _vm->graphics()->bob(5 + curBob);
				pbs->animNormal(image, curImage, pgd->speed / 4, rebound, false);
				pbs->x = pgd->x;
				pbs->y = pgd->y;
				++curBob;
			}
		}
	}

	// unpack the paste downs
	++curImage;
	for  (i = 1; i <= furnitureTotal; ++i) {
		int16 obj = gstate[i];
		if (obj > 5000) {
			obj -= 5000;
			GraphicData *pgd = &_graphicData[obj];
			_vm->graphics()->bankUnpack(pgd->firstFrame, curImage, 15);
			_vm->graphics()->bobPaste(curImage, pgd->x, pgd->y);
			// no need to increment curImage here, as bobPaste() destroys the 
			// unpacked frame after blitting it
		}
	}
}


void Logic::roomSetupObjects() {

	uint16 i;
	// furniture frames are reserved in ::roomSetupFurniture(), we append objects 
	// frames after the furniture ones.
	uint16 curImage = 36 + FRAMES_JOE_XTRA + _numFurnitureStatic + _numFurnitureAnimatedLen;
	uint16 firstRoomObj = _roomData[_currentRoom] + 1;
	uint16 lastRoomObj = _roomData[_currentRoom + 1];
	uint16 numObjectStatic = 0;
	uint16 numObjectAnimated = 0;
	uint16 curBob;

	// invalidates all Bobs for persons (except Joe's one)
	for (i = 1; i <= 3; ++i) {
		_vm->graphics()->bob(i)->active = false;
	}

	// static/animated Bobs
	for (i = firstRoomObj; i <= lastRoomObj; ++i) {
		ObjectData *pod = &_objectData[i];
		// setup blanks bobs for turned off objects (in case 
		// you turn them on again)
		if (pod->image == -1) {
			// static OFF Bob
			curBob = 20 + _numFurnitureStatic + numObjectStatic;
			++numObjectStatic;
			// create a blank frame for the for the OFF object
			++_numFrames;
			++curImage;
		}
		else if(pod->image == -2) {
			// animated OFF Bob
			curBob = 5 + _numFurnitureAnimated + numObjectAnimated;
			++numObjectAnimated;
		}
		else if(pod->image > 0 && pod->image < 5000) {
			GraphicData *pgd = &_graphicData[pod->image];
			int16 lastFrame = pgd->lastFrame;
			bool rebound = false;
			if (lastFrame < 0) {
				lastFrame = -lastFrame;
				rebound = true;
			}
			if (pgd->firstFrame < 0) {
				// FIXME: if(TEMPA[1]<0) bobs[CURRBOB].xflip=1;
				curBob = 5 + _numFurnitureAnimated;
				animSetup(pgd, curImage + 1, curBob + numObjectAnimated, pod->name > 0);
				curImage += pgd->lastFrame;
				++numObjectAnimated;
			}
			else if (lastFrame != 0) {
				// animated objects
				uint16 j;
				uint16 firstFrame = curImage + 1;
				for (j = pgd->firstFrame; j <= lastFrame; ++j) {
					++curImage;
					_vm->graphics()->bankUnpack(j, curImage, 15);
					++_numFrames;
				}
				curBob = 5 + _numFurnitureAnimated + numObjectAnimated;
				if (pod->name > 0) {
					BobSlot *pbs = _vm->graphics()->bob(curBob);
					pbs->active = true;
					pbs->x = pgd->x;
					pbs->y = pgd->y;
					pbs->frameNum = firstFrame;
					if (pgd->speed > 0) {
						pbs->animNormal(firstFrame, curImage, pgd->speed / 4, rebound, false);
					}
				}
				++numObjectAnimated;
			}
			else {
				// static objects
				curBob = 20 + _numFurnitureStatic + numObjectStatic;
				++curImage;
				_vm->graphics()->bobClear(curBob);

				// FIXME: if((COMPANEL==2) && (FULLSCREEN==1)) bobs[CURRBOB].y2=199;

				_vm->graphics()->bankUnpack(pgd->firstFrame, curImage, 15);
				++_numFrames;
				if (pod->name > 0) {
					BobSlot *pbs = _vm->graphics()->bob(curBob);
					pbs->active = true;
					pbs->x = pgd->x;
					pbs->y = pgd->y;
					pbs->frameNum = curImage;
				}
				++numObjectStatic;
			}
		}
	}

	// persons Bobs
	for (i = firstRoomObj; i <= lastRoomObj; ++i) {
		ObjectData *pod = &_objectData[i];
		if (pod->image == -3 || pod->image == -4) {
			debug(0, "Logic::roomSetupObjects() - Setting up person %X, name=%X", i, pod->name);
			uint16 noun = i - _roomData[_currentRoom];
			if (pod->name > 0) {
				curImage = personSetup(noun, curImage);
			}
			else {
				curImage = personAllocate(noun, curImage);
			}
		}
	}

	// paste downs list
	++curImage;
	_numFrames = curImage;
	for (i = firstRoomObj; i <= lastRoomObj; ++i) {
		ObjectData *pod = &_objectData[i];
		if (pod->name > 0) {
			int16 obj = pod->image;
			if (obj > 5000) {
				obj -= 5000;
				GraphicData *pgd = &_graphicData[obj];
				_vm->graphics()->bankUnpack(pgd->firstFrame, curImage, 15);
				_vm->graphics()->bobPaste(curImage, pgd->x, pgd->y);
			}
		}
	}
}


uint16 Logic::roomRefreshObject(uint16 obj) {

	uint16 curImage = _numFrames;

	if (obj == 0 || obj > _numObjects) {
		warning("Invalid object number %d", obj);
		return curImage;
	}

	ObjectData *pod = &_objectData[obj];
	if (pod->image == 0) {
		return curImage;
	}

	debug(0, "Logic::roomRefreshObject(%X, %s)", obj, _objName[ABS(pod->name)]);

	// check the object is in the current room
	if (pod->room != _currentRoom) {
		debug(0, "Trying to display an object (%i=%s) that is not in room (object room=%i, current room=%i)", obj, _objName[ABS(pod->name)], pod->room, _currentRoom);
		return curImage;
	}

	// find bob for the object
	uint16 curBob = findBob(obj);
	BobSlot *pbs = _vm->graphics()->bob(curBob);

	if (pod->image == -3 || pod->image == -4) {
		// a person object
		if (pod->name <= 0) {
			_vm->graphics()->bobClear(curBob);
		}
		else {
			// find person number
			uint16 pNum = 1;
			uint16 i = _roomData[_currentRoom] + 1;
			while (i < obj) {
				if (_objectData[i].image == -3 || _objectData[i].image == -4) {
					++pNum;
				}
				++i;
			}
			curImage = _personFrames[pNum] - 1;
			if (_personFrames[pNum] == 0) {
				curImage = _numFrames;
				_personFrames[pNum] = curImage;
			}
			curImage = personSetup(obj - _roomData[_currentRoom], curImage);
		}
		return curImage;
	}

	// find frame used for object
	curImage = findFrame(obj);

	if (pod->name < 0 || pod->image < 0) {
		// object is hidden or disabled
		_vm->graphics()->bobClear(curBob);
		return curImage;
	}

	int image = pod->image;
	if (image > 5000) {
		image -= 5000;
	}
	
	GraphicData *pgd = &_graphicData[image];
	bool rebound = false;
	int16 lastFrame = pgd->lastFrame;
	if (lastFrame < 0) {
		lastFrame = -lastFrame;
		rebound = true;
	}
	if (pgd->firstFrame < 0) {
		animSetup(pgd, curImage, curBob, pod->name != 0);
		curImage += pgd->lastFrame - 1;
	}
	else if (lastFrame != 0) {
		// turn on an animated bob
		_vm->graphics()->bankUnpack(pgd->firstFrame, 2, 15);
		pbs->animating = false;
		uint16 firstImage = curImage;
		--curImage;
		uint16 j;
		for (j = pgd->firstFrame; j <= lastFrame; ++j) {
			++curImage;
			_vm->graphics()->bankUnpack(j, curImage, 15);
		}
		pbs->active = true;
		pbs->x = pgd->x;
		pbs->y = pgd->y;
		pbs->frameNum = firstImage;
		if (pgd->speed > 0) {
			pbs->animNormal(firstImage, curImage, pgd->speed / 4, rebound, false);
		}
	}
	else {
		// frame 2 is used as a buffer frame to prevent BOB flickering
		_vm->graphics()->bankUnpack(pgd->firstFrame, 2, 15);
		_vm->graphics()->bankUnpack(pgd->firstFrame, curImage, 15);
		pbs->active = true;
		pbs->x = pgd->x;
		pbs->y = pgd->y;
		pbs->frameNum = curImage;
	}

	return curImage;
}


void Logic::roomSetup(const char *room, int comPanel, bool inCutaway) {

	char filename[20];

	// loads background image
	sprintf(filename, "%s.PCX", room);
	_vm->graphics()->loadBackdrop(filename, _currentRoom);

	// custom colors
	_vm->display()->palCustomColors(_currentRoom);

	// setup graphics to enter fullscreen/panel mode
	_vm->display()->screenMode(comPanel, inCutaway);

	// reset sprites table (bounding box...)
	_vm->graphics()->bobClearAll();

	// load/setup objects associated to this room
	sprintf(filename, "%s.BBK", room);
	_vm->graphics()->bankLoad(filename, 15);

	zoneSetup();
	_numFrames = 37 + FRAMES_JOE_XTRA;
	roomSetupFurniture();
	roomSetupObjects();
}


void Logic::roomDisplay(uint16 room, RoomDisplayMode mode, uint16 scale, int comPanel, bool inCutaway) {

	debug(0, "Logic::roomDisplay(%d, %d, %d, %d, %d)", room, mode, scale, comPanel, inCutaway);

	roomErase();

	// XXX _vm->sound()->loadSFX(SFXNAME[_currentRoom]);

	roomSetup(roomName(room), comPanel, inCutaway);
	ObjectData *pod = NULL;
	if (mode != RDM_FADE_NOJOE) {
		pod = joeSetupInRoom(mode != RDM_FADE_JOE_XY, scale);
	}
	if (mode != RDM_NOFADE_JOE) {
		update();
		BobSlot *joe = _vm->graphics()->bob(0);
		if (IS_CD_INTRO_ROOM(_currentRoom)) {
			_vm->display()->palFadeIn(0, 255, _currentRoom, joe->active, joe->x, joe->y);
		}
		else {
			_vm->display()->palFadeIn(0, 223, _currentRoom, joe->active, joe->x, joe->y);
		}
	}
	if (pod != NULL) {
		_vm->walk()->moveJoe(0, pod->x, pod->y, inCutaway);
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


void Logic::personSetData(int16 noun, const char *actorName, bool loadBank, Person *pp) {

	if (noun <= 0) {
		warning("Logic::personSetData() - Invalid object number: %i", noun);
	}

	uint16 i;
	uint16 obj = _roomData[_currentRoom] + noun;
	int16 img = _objectData[obj].image;
	if (img != -3 && img != -4) {
		warning("Logic::personSetData() - Object %d is not a person", obj);
		return;
	}

	// search Bob number for the person
	uint16 bobNum = 0;
	for (i = _roomData[_currentRoom] + 1; i <= obj; ++i) {
		img = _objectData[i].image;
		if (img == -3 || img == -4) {
			++bobNum;
		}
	}

	// search for a matching actor
	uint16 actor = 0;
	for (i = 1; i <= _numActors; ++i) {
		ActorData *pad = &_actorData[i];
		if (pad->room == _currentRoom) {
			if (_gameState[pad->gameStateSlot] == pad->gameStateValue) {
				if ((bobNum > 0 && bobNum == pad->bobNum) || strcmp(_aName[pad->name], actorName) == 0) {
					actor = i;
					break;
				}
			}
		}
	}

	if (actor != 0) {

		pp->actor = &_actorData[actor];
		pp->name = _aName[pp->actor->name];
		if (pp->actor->anim != 0) {
			pp->anim = _aAnim[pp->actor->anim];
		}
		else {
			pp->anim = NULL;
		}

		debug(0, "Logic::personSetData() - name=%s n=%d", pp->name, actor);

		if (loadBank) {
			const char *actorFile = _aFile[pp->actor->actorFile];
			if (actorFile) {
				_vm->graphics()->bankLoad(actorFile, pp->actor->bankNum);
			}
			// if actorFile is null, the person data is already loaded as
			// it is contained in objects room bank (.bbk)
		}

		pp->bobFrame = 29 + FRAMES_JOE_XTRA + pp->actor->bobNum;
	}
}


uint16 Logic::personSetup(uint16 noun, uint16 curImage) {

	Person p;
	personSetData(noun, "", true, &p);

	const ActorData *pad = p.actor;
	uint16 scale = 100;
	uint16 a = zoneInArea(ZONE_ROOM, pad->x, pad->y);
	if (a > 0) {
		// person is not standing in the area box, scale it accordingly
		scale = currentRoomArea(a)->calcScale(pad->y);
	}

	if (noun == 0) {
		warning("Trying to setup person 0");
		return curImage;
	}

	_vm->graphics()->bankUnpack(pad->bobFrameStanding, p.bobFrame, p.actor->bankNum);
	bool xflip = false;
	uint16 person = _roomData[_currentRoom] + noun;
	if (_objectData[person].image == -3) {
		// person is facing left
		xflip = true;
	}
	BobSlot *pbs = _vm->graphics()->bob(pad->bobNum);
	pbs->active = true;
	pbs->scale = scale;
	pbs->x = pad->x;
	pbs->y = pad->y;
	pbs->frameNum = p.bobFrame;
	pbs->xflip = xflip;

	debug(0, "Logic::personSetup(%d, %d) - bob = %d name = %s", noun, curImage, pad->bobNum, p.name);

	if (p.anim != NULL) {
		_personFrames[pad->bobNum] = curImage + 1;
		curImage = animCreate(curImage, &p);
	}
	else {
		animErase(pad->bobNum);
	}
	return curImage;
}


uint16 Logic::personAllocate(uint16 noun, uint16 curImage) {

	uint16 i;
	uint16 person = _roomData[_currentRoom] + noun;

	// search Bob number for the person
	uint16 bobNum = 0;
	for (i = _roomData[_currentRoom] + 1; i <= person; ++i) {
		int16 img = _objectData[i].image;
		if (img == -3 || img == -4) {
			++bobNum;
		}
	}

	// search for a matching actor
	uint16 actor = 0;
	for (i = 1; i <= _numActors; ++i) {
		ActorData *pad = &_actorData[i];
		if (pad->room == _currentRoom) {
			if (_gameState[pad->gameStateSlot] == pad->gameStateValue) {
				if (bobNum > 0 && bobNum == pad->bobNum) {
					actor = i;
					break;
				}
			}
		}
	}

	if (actor > 0) {
		const char *animStr = _aAnim[_actorData[actor].anim];
		if (animStr) {
			bool allocatedFrames[256];
			memset(allocatedFrames, 0, sizeof(allocatedFrames));
			uint16 f1, f2;
			do {
				sscanf(animStr, "%3hu,%3hu", &f1, &f2);
				animStr += 8;
				allocatedFrames[f1] = true;
			} while(f1 != 0);
			for (i = 1; i <= 255; ++i) {
				if (allocatedFrames[i]) {
					++curImage;
				}
			}
			// FIXME: shouldn't this line be executed BEFORE curImage is incremented ?
			_personFrames[bobNum] = curImage + 1;
		}
	}
	return curImage;
}


uint16 Logic::animCreate(uint16 curImage, const Person *person) {

	AnimFrame *animFrames = _newAnim[person->actor->bobNum];

	uint16 allocatedFrames[256];
	memset(allocatedFrames, 0, sizeof(allocatedFrames));
	const char *p = person->anim;
	int frame = 0;
	uint16 f1, f2;
	do {
		sscanf(p, "%3hu,%3hu", &f1, &f2);
		animFrames[frame].frame = f1;
		animFrames[frame].speed = f2;

		if (f1 > 500) {
			// SFX
			allocatedFrames[f1 - 500] = 1;
		}
		else {
			allocatedFrames[f1] = 1;
		}
		
		p += 8;
		++frame;
	} while(f1 != 0);
	
	// ajust frame numbers
	uint16 n = 1;
	uint16 i;
	for (i = 1; i <= 255; ++i) {
		if (allocatedFrames[i] != 0) {
			allocatedFrames[i] = n;
			++n;
		}
	}
	for (i = 0; animFrames[i].frame != 0; ++i) {
		uint16 frameNum = animFrames[i].frame;
		if (frameNum > 500) {
			animFrames[i].frame = curImage + allocatedFrames[frameNum - 500] + 500;
		}
		else {
			animFrames[i].frame = curImage + allocatedFrames[frameNum];
		}
	}

	// unpack necessary frames
	for (i = 1; i <= 255; ++i) {
		if (allocatedFrames[i] != 0) {
			++curImage;
			_vm->graphics()->bankUnpack(i, curImage, person->actor->bankNum);
		}
	}

	// start animation
	_vm->graphics()->bob(person->actor->bobNum)->animString(animFrames);

	return curImage;
}


void Logic::animErase(uint16 bobNum) {

	_newAnim[bobNum][0].frame = 0;
	BobSlot *pbs = _vm->graphics()->bob(bobNum);
	pbs->animating = false;
	pbs->anim.string.buffer = NULL;
}


void Logic::animReset(uint16 bobNum) {

	if (_newAnim[bobNum][0].frame != 0) {
		_vm->graphics()->bob(bobNum)->animString(_newAnim[bobNum]);
	}
}


void Logic::animSetup(const GraphicData *gd, uint16 firstImage, uint16 bobNum, bool visible) {
	
	int16 tempFrames[20];
	memset(tempFrames, 0, sizeof(tempFrames));
	uint16 numTempFrames = 0;
	uint16 i, j;
	for (i = 1; i <= _numGraphicAnim; ++i) {
		const GraphicAnim *pga = &_graphicAnim[i];
		if (pga->keyFrame == gd->firstFrame) {
			int16 frame = pga->frame;
			if (frame > 500) { // SFX
				frame -= 500;
			}
			bool foundMatchingFrame = false;
			for (j = 0; j < numTempFrames; ++j) {
				if (tempFrames[j] == frame) {
					foundMatchingFrame = true;
					break;
				}
			}
			if (!foundMatchingFrame) {
				assert(numTempFrames < 20);
				tempFrames[numTempFrames] = frame;
				++numTempFrames;
			}
		}
	}

	// sort found frames ascending
	bool swap = true;
	while (swap) {
		swap = false;
		for (i = 0; i < numTempFrames - 1; ++i) {
			if (tempFrames[i] > tempFrames[i + 1]) {
				SWAP(tempFrames[i], tempFrames[i + 1]);
				swap = true;
			}
		}
	}

	// queen.c l.962-980 / l.1269-1294
	for (i = 0; i < gd->lastFrame; ++i) {
		_vm->graphics()->bankUnpack(ABS(tempFrames[i]), firstImage + i, 15);
	}
	BobSlot *pbs = _vm->graphics()->bob(bobNum);
	pbs->animating = false;
	if (visible) {
		pbs->x = gd->x;
		pbs->y = gd->y;
		if (tempFrames[0] < 0) {
			pbs->xflip = true;
		}
		AnimFrame *paf = _newAnim[bobNum];
		for (i = 1; i <= _numGraphicAnim; ++i) {
			const GraphicAnim *pga = &_graphicAnim[i];
			if (pga->keyFrame == gd->firstFrame) {
				uint16 frameNr = 0;
				for (j = 1; j <= gd->lastFrame; ++j) {
					if (pga->frame > 500) {
						if (pga->frame - 500 == tempFrames[j - 1]) {
							frameNr = j + firstImage - 1 + 500;
						}
					}
					else if (pga->frame == tempFrames[j - 1]) {
						frameNr = j + firstImage - 1;
					}
				}
				paf->frame = frameNr;
				paf->speed = pga->speed;
				++paf;
			}
		}
		paf->frame = 0;
		paf->speed = 0;
		pbs->animString(_newAnim[bobNum]);
	}
}


void Logic::joeSetupFromBanks(const char *animBank, const char *standBank) {

	int i;
	_vm->graphics()->bankLoad(animBank, 13);
	for (i = 11; i <= 28 + FRAMES_JOE_XTRA; ++i) {
		_vm->graphics()->bankUnpack(i - 10, i, 13);
	}
	_vm->graphics()->bankErase(13);

	_vm->graphics()->bankLoad(standBank, 7);
	_vm->graphics()->bankUnpack(1, 33 + FRAMES_JOE_XTRA, 7);
	_vm->graphics()->bankUnpack(3, 34 + FRAMES_JOE_XTRA, 7);
	_vm->graphics()->bankUnpack(5, 35 + FRAMES_JOE_XTRA, 7);
}


void Logic::joeSetup() {

	joeSetupFromBanks("joe_a.BBK", "joe_b.BBK");
	joeFacing(DIR_FRONT);
}


ObjectData *Logic::joeSetupInRoom(bool autoPosition, uint16 scale) {

	debug(0, "Logic::joeSetupInRoom(%d, %d) joe.x=%d joe.y=%d", autoPosition, scale, _joe.x, _joe.y);

	uint16 oldx;
	uint16 oldy;
	WalkOffData *pwo = NULL;
	ObjectData *pod = objectData(_entryObj);
	if (pod == NULL) {
		error("Logic::joeSetupInRoom() - No object data for obj %d", _entryObj);
	}

	if (!autoPosition || joeX() != 0 || joeY() != 0) {
		oldx = joeX();
		oldy = joeY();
	}
	else {
		// find the walk off point for the entry object and make 
		// Joe walking to that point
		pwo = walkOffPointForObject(_entryObj);
		if (pwo != NULL) {
			oldx = pwo->x;
			oldy = pwo->y;
		}
		else {
			// no walk off point, use object position
			oldx = pod->x;
			oldy = pod->y;
		}
	}

	debug(0, "Logic::joeSetupInRoom() - oldx=%d, oldy=%d scale=%d", oldx, oldy, scale);

	if (scale > 0 && scale < 100) {
		joeScale(scale);
	}
	else {
		uint16 a = zoneInArea(ZONE_ROOM, oldx, oldy);
		if (a > 0) {
			joeScale(currentRoomArea(a)->calcScale(oldy));
		}
		else {
			joeScale(100);
		}
	}

	if (joeCutFacing() > 0) {
		joeFacing(joeCutFacing());
		joeCutFacing(0);
	}
	else {
		// check to see which way Joe entered room
		switch (State::findDirection(pod->state)) {
		case DIR_BACK:
			joeFacing(DIR_FRONT);
			break;
		case DIR_FRONT:
			joeFacing(DIR_BACK);
			break;
		case DIR_LEFT:
			joeFacing(DIR_RIGHT);
			break;
		case DIR_RIGHT:
			joeFacing(DIR_LEFT);
			break;
		}
	}
	joePrevFacing(joeFacing());

	BobSlot *pbs = _vm->graphics()->bob(0);
	pbs->scale = joeScale();

	if (_currentRoom == 108) {
		_vm->graphics()->cameraBob(-1);
		_vm->graphics()->bankLoad("joe_e.act", 7);
		_vm->graphics()->bankUnpack(2, 29 + FRAMES_JOE_XTRA, 7);

		_vm->display()->horizontalScroll(320);

		joeFacing(DIR_RIGHT);
		joeCutFacing(DIR_RIGHT);
		joePrevFacing(DIR_RIGHT);
	}

	joeFace();
	pbs->active = true;
	pbs->x = oldx;
	pbs->y = oldy;
	pbs->frameNum = 29 + FRAMES_JOE_XTRA;
	joeX(0);
	joeY(0);

	if (pwo != NULL) {
		// entryObj has a walk off point, then walk from there to object x,y
		return pod;
	}
	return NULL;
}


uint16 Logic::joeFace() {

	debug(9, "Logic::joeFace() - curFace = %d, prevFace = %d", _joe.facing, _joe.prevFacing);
	BobSlot *pbs = _vm->graphics()->bob(0);
	uint16 frame;
	if (_currentRoom == 108) {
		frame = 1;
	}
	else {
		frame = 33;
		if (joeFacing() == DIR_FRONT) {
			if (joePrevFacing() == DIR_BACK) {
				pbs->frameNum = 33 + FRAMES_JOE_XTRA;
				update();
			}
			frame = 34;
		}
		else if (joeFacing() == DIR_BACK) {
			if (joePrevFacing() == DIR_FRONT) {
				pbs->frameNum = 33 + FRAMES_JOE_XTRA;
				update();
			}
			frame = 35;
		}
		else if ((joeFacing() == DIR_LEFT && joePrevFacing() == DIR_RIGHT) 
			|| 	(joeFacing() == DIR_RIGHT && joePrevFacing() == DIR_LEFT)) {
			pbs->frameNum = 34 + FRAMES_JOE_XTRA;
			update();
		}
		pbs->frameNum = frame + FRAMES_JOE_XTRA;
		pbs->scale = joeScale();
		pbs->xflip = (joeFacing() == DIR_LEFT);
		update();
		joePrevFacing(joeFacing());
		switch (frame) {
		case 33:
			frame = 1;
			break;
		case 34:
			frame = 3;
			break;
		case 35:
			frame = 5;
			break;
		}
	}
	pbs->frameNum = 29 + FRAMES_JOE_XTRA;
	_vm->graphics()->bankUnpack(frame, pbs->frameNum, 7);
	return frame;
}


void Logic::joeGrab(int16 grabState) {

	uint16 frame = 0;
	BobSlot *bobJoe = _vm->graphics()->bob(0);
	
	switch (grabState) {
	case STATE_GRAB_NONE:
		break;

	case STATE_GRAB_MID:
		if (joeFacing() == DIR_BACK) {
			frame = 6;
		}
		else if (joeFacing() == DIR_FRONT) {
			frame = 4;
		}
		else {
			frame = 2;
		}
		break;

	case STATE_GRAB_DOWN:
		if (joeFacing() == DIR_BACK) {
			frame = 9;
		}
		else {
			frame = 8;
		}
		break;

	case STATE_GRAB_UP:
		// turn back
		_vm->graphics()->bankUnpack(5, 29 + FRAMES_JOE_XTRA, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		update();
		// grab up
		_vm->graphics()->bankUnpack(7, 29 + FRAMES_JOE_XTRA, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		update();
		// turn back
		frame = 7;
		break;
	}

	if (frame != 0) {
		_vm->graphics()->bankUnpack(frame, 29 + FRAMES_JOE_XTRA, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		update();

		// extra delay for grab down
		if (grabState == STATE_GRAB_DOWN) {
			update();
			update();
		}
	}
}


void Logic::joeUseDress(bool showCut) {

	if (showCut) {
		joeFacing(DIR_FRONT);
		joeFace();
		if (gameState(VAR_DRESSING_MODE) == 0) {
			playCutaway("cdres.CUT");
			inventoryInsertItem(ITEM_CLOTHES);
		}
		else {
			playCutaway("cudrs.CUT");
		}
	}
	_vm->display()->palSetJoe(JP_DRESS);
	joeSetupFromBanks("JoeD_A.BBK", "JoeD_B.BBK");
	inventoryDeleteItem(ITEM_DRESS);
	gameState(VAR_DRESSING_MODE, 2);
}


void Logic::joeUseClothes(bool showCut) {

	if (showCut) {
		joeFacing(DIR_FRONT);
		joeFace();
		playCutaway("cdclo.CUT");
		inventoryInsertItem(ITEM_DRESS);
	}
	_vm->display()->palSetJoe(JP_CLOTHES);
	joeSetupFromBanks("Joe_A.BBK", "Joe_B.BBK");
	inventoryDeleteItem(ITEM_CLOTHES);
	gameState(VAR_DRESSING_MODE, 0);
}


void Logic::joeUseUnderwear() {

	_vm->display()->palSetJoe(JP_CLOTHES);
	joeSetupFromBanks("JoeU_A.BBK", "JoeU_B.BBK");
	gameState(VAR_DRESSING_MODE, 1);
}


void Logic::makePersonSpeak(const char *sentence, Person *person, const char *voiceFilePrefix) {

	_vm->command()->clear(false);
	Talk::speak(sentence, person, voiceFilePrefix, _vm);
}


void Logic::dialogue(const char *dlgFile, int personInRoom, char *cutaway) {

	char cutawayFile[20];
	if (cutaway == NULL) {
		cutaway = cutawayFile;
	}
	_vm->display()->fullscreen(true);
	Talk::talk(dlgFile, personInRoom, cutaway, _vm);
	if (!cutaway[0]) {
		_vm->display()->fullscreen(false);
	}
}


void Logic::playCutaway(const char *cutFile, char *next) {

	char nextFile[20];
	if (next == NULL) {
		next = nextFile;
	}
	_vm->graphics()->textClear(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);
	Cutaway::run(cutFile, next, _vm);
}


void Logic::joeSpeak(uint16 descNum, bool objectType) {

	// joeSpeak(k, false) == SPEAK(JOE_RESPstr[k],"JOE",find_cd_desc(k)) 
	// joeSpeak(k, true)  == SPEAK(OBJECT_DESCRstr[k],"JOE",find_cd_desc(JOERESPMAX+k))
	const char *text = objectType ? _objDescription[descNum] : _joeResponse[descNum];
	if (objectType) {
		descNum += JOE_RESPONSE_MAX;
	}
	char descFilePrefix[10];
	sprintf(descFilePrefix, "JOE%04i", descNum);
	makePersonSpeak(text, NULL, descFilePrefix);
}


Verb Logic::findVerbUnderCursor(int16 cursorx, int16 cursory) const {

	return Verb(PANEL_VERBS[zoneIn(ZONE_PANEL, cursorx, cursory)]);
}


uint16 Logic::findObjectUnderCursor(int16 cursorx, int16 cursory) const {

	uint16 roomObj = 0;
	if (cursory < ROOM_ZONE_HEIGHT) {
		int16 x = cursorx + _vm->display()->horizontalScroll();
		roomObj = zoneIn(ZONE_ROOM, x, cursory);
	}
	return roomObj;
}


uint16 Logic::findObjectRoomNumber(uint16 zoneNum) const {

	// l.316-327 select.c
	uint16 noun = zoneNum;
	uint16 objectMax = _objMax[_currentRoom];
	debug(0, "Logic::findObjectRoomNumber(%X, %X)", zoneNum, objectMax);
	if (zoneNum > objectMax) {
		// this is an area box, check for associated object
		uint16 obj = currentRoomArea(zoneNum - objectMax)->object;
		if (obj != 0 && objectData(obj)->name != 0) {
			// there is an object, get its number
			noun = obj - _roomData[_currentRoom];
		}
	}
	return noun;
}


uint16 Logic::findObjectGlobalNumber(uint16 zoneNum) const {

	return _roomData[_currentRoom] + findObjectRoomNumber(zoneNum);
}


uint16 Logic::findInventoryItem(int invSlot) const {
	// queen.c l.3894-3898
	if (invSlot >= 0 && invSlot < 4) {
		return _inventoryItem[invSlot];
	}
	return 0;
}


void Logic::inventorySetup() {

	_vm->graphics()->bankLoad("objects.BBK", 14);
	_inventoryItem[0] = ITEM_BAT;
	_inventoryItem[1] = ITEM_JOURNAL;
	_inventoryItem[2] = ITEM_NONE;
	_inventoryItem[3] = ITEM_NONE;
}

void Logic::inventoryRefresh() {

	int16 i;
	uint16 x = 182;
	for (i = 0; i < 4; ++i) {
		uint16 itemNum = _inventoryItem[i];
		if (itemNum != 0) {
			// 1st object in inventory uses frame 8, 
			// whereas 2nd, 3rd and 4th uses frame 9
			uint16 dstFrame = (itemNum != 0) ? 8 : 9;
			// unpack frame for object and draw it
			_vm->graphics()->bankUnpack(_itemData[itemNum].frame, dstFrame, 14);
			_vm->graphics()->bobDrawInventoryItem(dstFrame, x, 14);
		}
		else {
			// no object, clear the panel 
			_vm->graphics()->bobDrawInventoryItem(0, x, 14);
		}
		x += 35;
	}
	// XXX OLDVERB=VERB;
	update();
}

int16 Logic::previousInventoryItem(int16 start) const {
	int i;
	for (i = start - 1; i >= 1; i--)
		if (_itemData[i].name > 0)
			return i;
	for (i = _numItems; i > start; i--)
		if (_itemData[i].name > 0)
			return i;

	return 0;	//nothing found
}

int16 Logic::nextInventoryItem(int16 start) const {
	int i;
	for (i = start + 1; i < _numItems; i++)
		if (_itemData[i].name > 0)
			return i;
	for (i = 1; i < start; i++)
		if (_itemData[i].name > 0)
			return i;

	return 0;	//nothing found
}

void Logic::removeDuplicateItems() {
	for (int i = 0; i < 4; i++)
		for (int j = i + 1; j < 4; j++)
			if (_inventoryItem[i] == _inventoryItem[j])
				_inventoryItem[j] = ITEM_NONE;
}

uint16 Logic::numItemsInventory() const {
	uint16 count = 0;
	for (int i = 1; i < _numItems; i++)
		if (_itemData[i].name > 0)
			count++;

	return count;
}

void Logic::inventoryInsertItem(uint16 itemNum, bool refresh) {
	int16 item = _inventoryItem[0] = (int16)itemNum; 
	_itemData[itemNum].name = ABS(_itemData[itemNum].name);	//set visible
	for (int i = 1; i < 4; i++) {
		item = nextInventoryItem(item);
		_inventoryItem[i] = item;
		removeDuplicateItems();
	}

	if (refresh)
		inventoryRefresh();
}


void Logic::inventoryDeleteItem(uint16 itemNum, bool refresh) {
	int16 item = (int16)itemNum;
	_itemData[itemNum].name = -ABS(_itemData[itemNum].name);	//set invisible
	for (int i = 0; i < 4; i++) {
		item = nextInventoryItem(item);
		_inventoryItem[i] = item;
		removeDuplicateItems();
	}

	if (refresh)
		inventoryRefresh();
}


void Logic::inventoryScroll(uint16 count, bool up) {
	if (!(numItemsInventory() > 4))
		return;

	if (up) {
		for (int i = 3; i > 0; i--)
			_inventoryItem[i] = _inventoryItem[i - 1];
		_inventoryItem[0] = previousInventoryItem(_inventoryItem[0]);
	} else {
		for (int i = 0; i < 3; i++)
			_inventoryItem[i] = _inventoryItem[i + 1];
		_inventoryItem[3] = nextInventoryItem(_inventoryItem[3]);		
	}

	inventoryRefresh();
}


void Logic::objectCopy(int dummyObjectIndex, int realObjectIndex) {
	// P3_COPY_FROM function in cutaway.c
	/* Copy data from Dummy (D) object to object (K)
		 If COPY_FROM Object images are greater than COPY_TO Object
		 images then swap the objects around. */

	ObjectData *dummyObject = objectData(dummyObjectIndex);
	ObjectData *realObject  = objectData(realObjectIndex);
	
	int fromState = (dummyObject->name < 0) ? -1 : 0;

	int frameCountReal  = 1;
	int frameCountDummy = 1;

	int graphic = realObject->image;
	if (graphic > 0) {
		if (graphic > 5000)
			graphic -= 5000;

		GraphicData *data = graphicData(graphic);

		if (data->lastFrame > 0) 
			frameCountReal = data->lastFrame - data->firstFrame + 1;

		graphic = dummyObject->image;
		if (graphic > 0) {
			if (graphic > 5000)
				graphic -= 5000;

			data = graphicData(graphic);

			if (data->lastFrame > 0) 
				frameCountDummy = data->lastFrame - data->firstFrame + 1;
		}
	}

	ObjectData temp = *realObject;
	*realObject = *dummyObject;

	if (frameCountDummy > frameCountReal)
		*dummyObject = temp;

	realObject->name = ABS(realObject->name);

	if  (fromState == -1)
		dummyObject->name = -ABS(dummyObject->name);

	//  Make sure that WALK_OFF_DATA is copied too!

	for (int i = 1; i <= _numWalkOffs; i++) {
		WalkOffData *walkOff = &_walkOffData[i];
		if (walkOff->entryObj == (int16)dummyObjectIndex) {
			walkOff->entryObj = (int16)realObjectIndex;
			break;
		}
	}

}


void Logic::checkPlayer() {
	update();
	_vm->command()->updatePlayer();
}


void Logic::customMoveJoe(int facing, uint16 areaNum, uint16 walkDataNum) {

	// queen.c l.2838-2911
	debug(9, "customMoveJoe(%d, %d, %d)\n", facing, areaNum, walkDataNum);

	// Stop animating Joe
	_vm->graphics()->bob(0)->animating = false;

	// Make Joe face the right direction
	joeFacing(facing);
	joeFace();

	_newRoom = 0;
	_entryObj = 0;

	char nextCut[20];
	memset(nextCut, 0, sizeof(nextCut));

	switch (_currentRoom) {
	case ROOM_JUNGLE_BRIDGE:
		joeSpeak(16);
		break;
	case ROOM_JUNGLE_GORILLA_1:
		playCutaway("c6c.CUT", nextCut);
		break;
	case ROOM_JUNGLE_GORILLA_2:
		playCutaway("c14b.CUT", nextCut);
		break;
	case ROOM_AMAZON_ENTRANCE:
		if (areaNum == 3) {
			playCutaway("c16a.CUT", nextCut);
		}
		break;
	case ROOM_AMAZON_HIDEOUT:
		if (walkDataNum == 4) {
			playCutaway("c17a.CUT", nextCut);
		}
		else if (walkDataNum == 2) {
			playCutaway("c17b.CUT", nextCut);
		}
		break;
	case ROOM_FLODA_OUTSIDE:
		playCutaway("c22a.CUT", nextCut);
		break;
	case ROOM_FLODA_KITCHEN:
		playCutaway("c26b.CUT", nextCut);
		break;
	case ROOM_FLODA_KLUNK:
		playCutaway("c30a.CUT", nextCut);
		break;
	case ROOM_FLODA_HENRY:
		playCutaway("c32c.CUT", nextCut);
		break;
	case ROOM_TEMPLE_ZOMBIES:
		if (areaNum == 6) {
			if (_gameState[21] == 0) {
				playCutaway("c50d.CUT", nextCut);
				while (nextCut[0] != '\0') {
					playCutaway(nextCut, nextCut);
				}
				_gameState[21] = 1;
			} else {
				playCutaway("c50h.CUT", nextCut);
			}
		}
		break;
	case ROOM_TEMPLE_SNAKE:
		playCutaway("c53b.CUT", nextCut);
		break;
	case ROOM_TEMPLE_LIZARD_LASER:
		joeSpeak(19);
		break;
	case ROOM_HOTEL_DOWNSTAIRS:
		joeSpeak(21);
		break;
	case ROOM_HOTEL_LOBBY:
		if (_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] == 0) {
			playCutaway("c73a.CUT"); 
			_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] = 1;
			joeUseUnderwear();
			joeFace();
		}
		else if (_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] == 1) {
			playCutaway("c73b.CUT");
			_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] = 2;
		}
		else if (_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] == 2) {
			playCutaway("c73c.CUT");
		}
		break;
	case ROOM_TEMPLE_MAZE_5:
		if (areaNum == 7) {
			joeSpeak(17);
		}
		break;
	case ROOM_TEMPLE_MAZE_6:
		if (areaNum == 5 && _gameState[187] == 0) {
			playCutaway("c101b.CUT", nextCut);
		}
		break;
	case ROOM_FLODA_FRONTDESK:
		if (areaNum == 3) {
			if (_gameState[VAR_BYPASS_FLODA_RECEPTIONIST] == 1) {
				playCutaway("c103e.CUT", nextCut);
			}
			else if (_gameState[VAR_BYPASS_FLODA_RECEPTIONIST] == 0) {
				playCutaway("c103b.CUT", nextCut);
				_gameState[VAR_BYPASS_FLODA_RECEPTIONIST] = 1;
			}
		}
		break;
	}

	while (strlen(nextCut) > 4 && 
		scumm_stricmp(nextCut + strlen(nextCut) - 4, ".cut") == 0) {
		playCutaway(nextCut, nextCut);
	}
}


void Logic::handlePinnacleRoom() {

	// camera does not follow Joe anymore
	_vm->graphics()->cameraBob(-1);
	roomDisplay(ROOM_JUNGLE_PINNACLE, RDM_NOFADE_JOE, 100, 2, true);

	BobSlot *joe   = _vm->graphics()->bob(6);
	BobSlot *piton = _vm->graphics()->bob(7);

	// set scrolling value to mouse position to avoid glitch
	_vm->display()->horizontalScroll(_vm->input()->mousePosX());

	joe->x = piton->x = 3 * _vm->input()->mousePosX() / 4 + 200;

	joe->frameNum = _vm->input()->mousePosX() / 36 + 43 + FRAMES_JOE_XTRA;

	// adjust bounding box for fullscreen
	joe->box.y2 = piton->box.y2 = GAME_SCREEN_HEIGHT - 1;

	// bobs have been unpacked from animating objects, we don't need them
	// to animate anymore ; so turn animating off
	joe->animating = piton->animating = false;

	update();
	_vm->display()->palFadeIn(0, 223, ROOM_JUNGLE_PINNACLE, joe->active, joe->x, joe->y);

	_entryObj = 0;
	uint16 prevObj = 0;
	while (_vm->input()->mouseButton() == 0 || _entryObj == 0) {

		update();
		int mx = _vm->input()->mousePosX();
		int my = _vm->input()->mousePosY();

		// update screen scrolling
		_vm->display()->horizontalScroll(_vm->input()->mousePosX());

		// update bobs position / frame
		joe->x = piton->x = 3 * mx / 4 + 200;
		joe->frameNum = mx / 36 + 43 + FRAMES_JOE_XTRA;

		uint16 curObj = findObjectUnderCursor(mx, my);
		if (curObj != 0 && curObj != prevObj) {
			_entryObj = 0;
			curObj += _roomData[_currentRoom]; // global object number
			ObjectData *objData = &_objectData[curObj];
			if (objData->name > 0) {
				_entryObj = objData->entryObj;
				char textCmd[CmdText::MAX_COMMAND_LEN];
				sprintf(textCmd, "%s %s", verbName(VERB_WALK_TO), _objName[objData->name]);
				_vm->graphics()->textCurrentColor(INK_PINNACLE_ROOM);
				_vm->graphics()->textSetCentered(5, textCmd);
			}
			prevObj = curObj;
		}
	}
	_vm->input()->clearMouseButton();

	_newRoom = _objectData[_entryObj].room;

	// Only a few commands can be triggered from this room :
	// piton -> crash  : 0x216 (obj=0x2a, song=3)
	// piton -> floda  : 0x217 (obj=0x29, song=16)
	// piton -> bob    : 0x219 (obj=0x2f, song=6)
	// piton -> embark : 0x218 (obj=0x2c, song=7)
	// piton -> jungle : 0x20B (obj=0x2b, song=3)
	// piton -> amazon : 0x21A (obj=0x30, song=3)
	// 
	// Because none of these update objects/areas/gamestate, the EXECUTE_ACTION()
	// call, as the original does, is useless. All we have to do is the playsong 
	// call (all songs have the PLAY_BEFORE type). This way we could get rid of 
	// the hack described in execute.c l.334-339.
	//
	// XXX if (com->song > 0) { playsong(com->song); }

	joe->active = piton->active = false;
	_vm->graphics()->textClear(5, 5);

	// camera follows Joe again
	_vm->graphics()->cameraBob(0);

	_vm->display()->palFadeOut(0, 223, ROOM_JUNGLE_PINNACLE);
}


void Logic::update() {
	_vm->graphics()->update(_currentRoom);
	_vm->input()->delay();
	_vm->display()->palCustomScroll(_currentRoom);
	BobSlot *joe = _vm->graphics()->bob(0);
	_vm->display()->update(joe->active, joe->x, joe->y);
	_dbg->update(_vm->input()->checkKeys());

	if (_vm->input()->quickSave())
		if (!_vm->input()->cutawayRunning()) {
			_vm->input()->quickSaveReset();
			gameSave(0, "Quicksave");
		}
	if (_vm->input()->quickLoad())
		if (!_vm->input()->cutawayRunning()) {
			_vm->input()->quickLoadReset();
			gameLoad(0);
		}
}

bool Logic::gameSave(uint16 slot, const char *desc) {
	if (!desc)	//no description entered
		return false;

	debug(3, "Saving game to slot %d", slot);
	
	int i, j;
	char *buf = new char[32];
	memcpy(buf, desc, strlen(desc) < 32 ? strlen(desc) : 32);
	for (i = strlen(desc); i < 32; i++)
		buf[i] = '\0';
	byte *saveData = new byte[SAVEGAME_SIZE];
	byte *ptr = saveData;
	memcpy(ptr, buf, 32); ptr += 32;
	delete[] buf;
	
	WRITE_BE_UINT16(ptr, _talkSpeed); ptr += 2;
	WRITE_BE_UINT16(ptr, 0 /*_settings.musicVolume*/); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->sound()->sfxOn() ? 1 : 0); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->sound()->speechOn() ? 1 : 0); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->sound()->musicOn() ? 1 : 0); ptr += 2;
	WRITE_BE_UINT16(ptr, _subtitles ? 1 : 0); ptr += 2;
	
	for (i = 0; i < 4; i++) {
		WRITE_BE_UINT16(ptr, _inventoryItem[i]); ptr += 2;
	}
	
	WRITE_BE_UINT16(ptr, _vm->graphics()->bob(0)->x); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->graphics()->bob(0)->y); ptr += 2;
	WRITE_BE_UINT16(ptr, _currentRoom); ptr += 2;

	for (i = 1; i <= _numObjects; i++)
		_objectData[i].writeTo(ptr);
		
	for (i = 1; i <= _numItems; i++)
		_itemData[i].writeTo(ptr);
		
	for (i = 0; i < GAME_STATE_COUNT; i++) {
		WRITE_BE_UINT16(ptr, gameState(i)); ptr += 2;
	}
	
	for (i = 1; i <= _numRooms; i++)
		for (j = 1; j <= _areaMax[i]; j++)
			_area[i][j].writeTo(ptr);
			
	for (i = 0; i < TALK_SELECTED_COUNT; i++)
			_talkSelected[i].writeTo(ptr);
	
	for (i = 1; i <= _numWalkOffs; i++)
		_walkOffData[i].writeTo(ptr);

	WRITE_BE_UINT16(ptr, _joe.facing); ptr += 2;
	WRITE_BE_UINT16(ptr, 0); ptr += 2; //TODO: tmpbamflag
	WRITE_BE_UINT16(ptr, 0); ptr += 2; //TODO: lastoverride
	
	//TODO: lastmerge, lastalter, altmrgpri
	for (i = 0; i < 3; i++) {
		WRITE_BE_UINT16(ptr, 0); ptr += 2;
	}
	
	if ((ptr - saveData) != SAVEGAME_SIZE) {
		delete[] saveData;
		return false;
	}
	
	bool result = _vm->resource()->writeSave(slot, saveData, SAVEGAME_SIZE);
	delete[] saveData;
	
	return result;	
}

bool Logic::gameLoad(uint16 slot) {
	int i, j;
	byte *saveData = new byte[SAVEGAME_SIZE];
	byte *ptr = saveData;
	if (!_vm->resource()->readSave(slot, saveData)) {
		warning("Couldn't load savegame from slot %d", slot);
		delete[] saveData;
		return false;
	}
	
	debug(3, "Loading game from slot %d", slot);
	ptr += 32;	//skip description
	/*_talkSpeed = (int16)READ_BE_UINT16(ptr);*/ ptr += 2;
	/*_settings.musicVolume = (int16)READ_BE_UINT16(ptr);*/ ptr += 2;
	_vm->sound()->sfxToggle(READ_BE_UINT16(ptr) != 0); ptr += 2;
	_vm->sound()->speechToggle(READ_BE_UINT16(ptr) != 0); ptr += 2;
	_vm->sound()->musicToggle(READ_BE_UINT16(ptr) != 0); ptr += 2;
	_subtitles = READ_BE_UINT16(ptr) != 0; ptr += 2;

	for (i = 0; i < 4; i++) {
		_inventoryItem[i] = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}

	_joe.x = (int16)READ_BE_UINT16(ptr); ptr += 2;
	_joe.y = (int16)READ_BE_UINT16(ptr); ptr += 2;

	currentRoom(READ_BE_UINT16(ptr)); ptr += 2;
	
	for (i = 1; i <= _numObjects; i++)
		_objectData[i].readFrom(ptr);

	for (i = 1; i <= _numItems; i++)
		_itemData[i].readFrom(ptr);

	for (i = 0; i < GAME_STATE_COUNT; i++) {
		gameState(i, (int16)READ_BE_UINT16(ptr)); ptr += 2;
	}

	for (i = 1; i <= _numRooms; i++)
		for (j = 1; j <= _areaMax[i]; j++)
			_area[i][j].readFrom(ptr);
	
	for (i = 0; i < TALK_SELECTED_COUNT; i++)
		_talkSelected[i].readFrom(ptr);
		
	for (i = 1; i <= _numWalkOffs; i++)
		_walkOffData[i].readFrom(ptr);

	joeFacing(READ_BE_UINT16(ptr));  ptr += 2;
	READ_BE_UINT16(ptr); ptr += 2;	//TODO: tmpbamflag
	READ_BE_UINT16(ptr); ptr += 2; //TODO: lastoverride
	//_vm->sound()->playSound(_vm->sound()->lastOverride())
	
	//TODO: lastmerge, lastalter, altmrgpri
	for (i = 0; i < 3; i++) {
		READ_BE_UINT16(ptr); ptr += 2;
	}

	if ((ptr - saveData) != SAVEGAME_SIZE) {
		delete[] saveData;
		return false;
	}
	
	joeCutFacing(joeFacing());
	joeFace();
	
	//OLDX = _joe.x;
	//OLDY = _joe.y;
	_oldRoom = 0;
	newRoom(_currentRoom);
	_entryObj = 0;

	switch (gameState(VAR_DRESSING_MODE)) {
		case  0: 
			joeUseClothes(false);
			break;
		case  1:
			joeUseUnderwear();
			break;
		case  2:
			joeUseDress(false);
			break;
	}
	inventoryRefresh();
	//bamflag = ..
	
	delete[] saveData;
	return true;
}

void Logic::sceneStart() {
	debug(0, "[Logic::sceneStart] _scene = %i", _scene);
	_scene++;

	_vm->display()->showMouseCursor(false);

	if (1 == _scene) { // && _vm->input()->cutawayRunning()) { // sceneStart is always called when cutaway is running
		_vm->display()->palFadePanel();
	}

	update();
}

void Logic::sceneStop() {
	debug(0, "[Logic::sceneStop] _scene = %i", _scene);
	_scene--;

	if (_scene > 0)
		return;

	_vm->display()->palSetAllDirty();
	_vm->display()->showMouseCursor(true);
	zoneSetupPanel();
}


void Logic::changeRoom() {

	if (currentRoom() == ROOM_JUNGLE_PINNACLE) {
		handlePinnacleRoom();
	}
	else if (currentRoom() == FOTAQ_LOGO && gameState(VAR_INTRO_PLAYED) == 0) {
		// FIXME: this should be rewritten in a more elegant way
		bool pcGamesDemo = _vm->resource()->isDemo() && !_vm->resource()->exists("pclogo.cut");

		if (pcGamesDemo) {
			currentRoom(79);
		}
		roomDisplay(currentRoom(), RDM_FADE_NOJOE, 100, 2, true);

		if (_vm->resource()->isDemo()) {
			if (pcGamesDemo) {
				playCutaway("clogo.cut");
			}
			else {
				playCutaway("pclogo.cut");
			}
		}
		else {
			playCutaway("copy.cut");
			playCutaway("clogo.cut");

			// TODO enable talking for talkie version

			playCutaway("cdint.cut");

			// restore palette colors ranging from 144 to 256
			_vm->graphics()->loadPanel();
			
			playCutaway("cred.cut");
		}

		// Ugly fix from original code
		sceneReset();

		currentRoom(ROOM_HOTEL_LOBBY);
		entryObj(584);

		roomDisplay(currentRoom(), RDM_FADE_JOE, 100, 2, true);
		playCutaway("c70d.cut");

		gameState(VAR_INTRO_PLAYED, 1);

		inventorySetup();
		inventoryRefresh();
	}
	else {
		roomDisplay(currentRoom(), RDM_FADE_JOE, 100, 1, false);
	}
	_vm->display()->showMouseCursor(true); // _drawMouseFlag = 1;
}


void Logic::useJournal() {

	if (_vm->resource()->isDemo()) {
		makePersonSpeak("This is a demo, so I can't load or save games*14", NULL, "");
	}
	else {

		// XXX save some vars
		// 
		// XXX tmpbamflag=bamflag;
		// XXX bamflag=0;
		// XXX in_journal=1;

		_vm->command()->clear(false);

		Journal j(_vm);
		j.use();

		_vm->walk()->stopJoe();

		// XXX restore vars
		// 
		// XXX in_journal=0;
		// XXX bamflag=tmpbamflag;
		// XXX TALKQUIT=CUTQUIT=0; Make sure that we turn off cut stuff in case we use Journal during cutaways

	}
}


void Logic::registerDefaultSettings() {

	ConfMan.registerDefault("master_volume", 255);
	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("talkspeed", DEFAULT_TALK_SPEED);
	ConfMan.registerDefault("speech_mute", _vm->resource()->isFloppy());
	ConfMan.registerDefault("subtitles", true);
}


void Logic::checkOptionSettings() {

	// check talkspeed value
	if (_talkSpeed < 4) {
		_talkSpeed = 4;
	}
	else if (_talkSpeed > 95) {
		_talkSpeed = 100;
	}

	// XXX check master_volume value

	// only CD-ROM version has speech
	if (_vm->resource()->JASVersion()[0] != 'C' && _vm->sound()->speechOn()) {
		_vm->sound()->speechToggle(false);
	}

	// ensure text is always on when voice is off
	if (!_vm->sound()->speechOn()) {
		_subtitles = true;
	}
}


void Logic::readOptionSettings() {

	// XXX master_volume
	_vm->sound()->musicToggle(!ConfMan.getBool("music_mute"));
	_vm->sound()->sfxToggle(!ConfMan.getBool("sfx_mute"));
	_talkSpeed = ConfMan.getInt("talkspeed");
	_vm->sound()->speechToggle(!ConfMan.getBool("speech_mute"));
	_subtitles = ConfMan.getBool("subtitles");

	checkOptionSettings();
}


void Logic::writeOptionSettings() {

	// XXX master_volume
	ConfMan.set("music_mute", !_vm->sound()->musicOn());
	ConfMan.set("sfx_mute", !_vm->sound()->sfxOn());
	ConfMan.set("talkspeed", _talkSpeed);
	ConfMan.set("speech_mute", !_vm->sound()->speechOn());
	ConfMan.set("subtitles", _subtitles);

	ConfMan.flushToDisk();
}


void Logic::executeSpecialMove(uint16 sm) {
	// FIXME: for now, we initialise the various 'asm' procs here but,
	// in order to support the 'interview' mini-game', we will have to do
	// that in a proper setupAsmForGame() or setupAsmForInterview() function.
	static const SpecialMoveProc proc[40] = {
		/* 00 */
		NULL,
		NULL,
		&Logic::asmMakeJoeUseDress,
		&Logic::asmMakeJoeUseNormalClothes,
		/* 04 */
		&Logic::asmMakeJoeUseUnderwear,
		&Logic::asmSwitchToDressPalette,
		&Logic::asmSwitchToNormalPalette,
		&Logic::asmStartCarAnimation,
		/* 08 */
		&Logic::asmStopCarAnimation,
		&Logic::asmStartFightAnimation,
		&Logic::asmWaitForFrankPosition,
		&Logic::asmMakeFrankGrowing,
		/* 12 */
		&Logic::asmMakeRobotGrowing,
		&Logic::asmShrinkRobot,
		&Logic::asmEndGame,
		&Logic::asmPutCameraOnDino,
		/* 16 */
		&Logic::asmPutCameraOnJoe,
		NULL, // XXX alternative introduction
		NULL, // XXX alternative introduction
		&Logic::asmSetAzuraInLove,
		/* 20 */
		&Logic::asmPanRightFromJoe,
		&Logic::asmSetLightsOff,
		&Logic::asmSetLightsOn,
		&Logic::asmSetManequinAreaOn,
		/* 24 */
		&Logic::asmPanToJoe,
		&Logic::asmTurnGuardOn,
		&Logic::asmPanLeft320To144,
		&Logic::asmSmooch,
		/* 28 */
		&Logic::asmMakeLightningHitPlane,
		&Logic::asmScaleBlimp,
		&Logic::asmScaleEnding,
		&Logic::asmWaitForCarPosition,
		/* 32 */
		&Logic::asmShakeScreen,
		&Logic::asmAttemptPuzzle,
		&Logic::asmScaleTitle,
		NULL, // XXX PC Demo ?
		/* 36 */
		&Logic::asmPanRightToHugh,
		&Logic::asmMakeWhiteFlash,
		&Logic::asmPanRightToJoeAndRita,
		&Logic::asmPanLeftToBomb
	};

	if (sm >= ARRAYSIZE(proc) || proc[sm] == NULL) {
		warning("unhandled / invalid special move : %d", sm);
	}
	else {
		debug(0, "Special move: %d", sm);
		(this->*proc[sm])();
	}
}


void Logic::asmMakeJoeUseDress() {
	
	joeUseDress(false);
}


void Logic::asmMakeJoeUseNormalClothes() {
	
	joeUseClothes(false);
}


void Logic::asmMakeJoeUseUnderwear() {
	
	joeUseUnderwear();
}


void Logic::asmSwitchToDressPalette() {

	_vm->display()->palSetJoe(JP_DRESS);
}


void Logic::asmSwitchToNormalPalette() {

	_vm->display()->palSetJoe(JP_CLOTHES);
}


void Logic::asmStartCarAnimation() {

	// Carbam background animation - room 74
	_vm->graphics()->initCarBamScene();
}


void Logic::asmStopCarAnimation() {

	// CR 2 - Turn off big oil splat and gun shots!
	_vm->graphics()->cleanupCarBamScene(findBob(594)); // Oil object
}


void Logic::asmStartFightAnimation() {
	
	// Fight1 background animation - room 69
	_vm->graphics()->initFightBamScene();
	gameState(148, 1);
}


void Logic::asmWaitForFrankPosition() {

	// c69e.cut
	_vm->graphics()->bamData()->flag = 2;
	while (_vm->graphics()->bamData()->flag) {
		update();
	}
}


void Logic::asmMakeFrankGrowing() {

	// c69z.cut
	_vm->graphics()->bankUnpack(1, 38, 15);
	BobSlot *bobFrank = _vm->graphics()->bob(5);
	bobFrank->frameNum = 38;
	bobFrank->curPos(160, 200);
	bobFrank->box.y2 = GAME_SCREEN_HEIGHT - 1;

	int i;
	for (i = 10; i <= 100; i += 4) {
		bobFrank->scale = i;
		update();
	}
	for (i = 0; i <= 20; ++i) {
		update();
	}

	objectData(521)->name =  ABS(objectData(521)->name); // Dinoray
	objectData(526)->name =  ABS(objectData(526)->name); // Frank obj
	objectData(522)->name = -ABS(objectData(522)->name); // TMPD object off
	objectData(525)->name = -ABS(objectData(525)->name); // Floda guards off
	objectData(523)->name = -ABS(objectData(523)->name); // Sparky object off
	gameState(157, 1); // No more Ironstein
}


void Logic::asmMakeRobotGrowing() { 

	// c69z.cut
	_vm->graphics()->bankUnpack(1, 38, 15);
	BobSlot *bobRobot = _vm->graphics()->bob(5);
	bobRobot->frameNum = 38;
	bobRobot->curPos(160, 200);
	bobRobot->box.y2 = GAME_SCREEN_HEIGHT - 1;

	int i;
	for (i = 10; i <= 100; i += 4) {
		bobRobot->scale = i;
		update();
	}
	for (i = 0; i <= 20; ++i) {
		update();
	}
	
	objectData(524)->name = -ABS(objectData(524)->name); // Azura object off
	objectData(526)->name = -ABS(objectData(526)->name); // Frank object off
}


void Logic::asmShrinkRobot() {
	
	int i;
	for (i = 100; i >= 35; i -= 5) {
		_vm->graphics()->bob(6)->scale = i;
		update();
	}
}


void Logic::asmEndGame() {
	
	int i;
	for (i = 0; i < 40; ++i) {
		update();
	}
	OSystem::instance()->quit();
	debug(0, "Game completed");
}


void Logic::asmPutCameraOnDino() {

	_vm->graphics()->cameraBob(-1);
	while (_vm->display()->horizontalScroll() < 320) {
		_vm->display()->horizontalScroll(_vm->display()->horizontalScroll() + 16);
		if (_vm->display()->horizontalScroll() > 320) {
			_vm->display()->horizontalScroll(320);
		}
		update();
	}
	_vm->graphics()->cameraBob(1);
}


void Logic::asmPutCameraOnJoe() {

	_vm->graphics()->cameraBob(0);
}


void Logic::asmSetAzuraInLove() {

	gameState(VAR_AZURA_IN_LOVE, 1);
}


void Logic::asmPanRightFromJoe() {

	_vm->graphics()->cameraBob(-1);
	while (_vm->display()->horizontalScroll() < 320) {
		_vm->display()->horizontalScroll(_vm->display()->horizontalScroll() + 16);
		if (_vm->display()->horizontalScroll() > 320) {
			_vm->display()->horizontalScroll(320);
		}
		update();
	}
}


void Logic::asmSetLightsOff() {

	_vm->display()->palCustomLightsOff(currentRoom());
}


void Logic::asmSetLightsOn() {

	_vm->display()->palCustomLightsOn(currentRoom());
}


void Logic::asmSetManequinAreaOn() {

	area(ROOM_FLODA_FRONTDESK, 7)->mapNeighbours = ABS(area(ROOM_FLODA_FRONTDESK, 7)->mapNeighbours);
}


void Logic::asmPanToJoe() {

	int i = _vm->graphics()->bob(0)->x - 160;
	if (i < 0) {
		i = 0;
	}
	else if (i > 320) {
		i = 320;
	}
	_vm->graphics()->cameraBob(-1);
	if (i < _vm->display()->horizontalScroll()) {
		while (_vm->display()->horizontalScroll() > i) {
			_vm->display()->horizontalScroll(_vm->display()->horizontalScroll() - 16);
			if (_vm->display()->horizontalScroll() < i) {
				_vm->display()->horizontalScroll(i);
			}
			update();
		}
	}
	else {
		while (_vm->display()->horizontalScroll() < i) {
			_vm->display()->horizontalScroll(_vm->display()->horizontalScroll() + 16);
			if (_vm->display()->horizontalScroll() > i ) {
				_vm->display()->horizontalScroll(i);
			}
		}
		update();
	}
	_vm->graphics()->cameraBob(0);
}


void Logic::asmTurnGuardOn() {

	gameState(85, 1);
}


void Logic::asmPanLeft320To144() {
	
	_vm->graphics()->cameraBob(-1);
	while (_vm->display()->horizontalScroll() > 144) {
		_vm->display()->horizontalScroll(_vm->display()->horizontalScroll() - 8);
		if (_vm->display()->horizontalScroll() < 144) {
			_vm->display()->horizontalScroll(144);
		}
		update();
	}
}


void Logic::asmSmooch() {
			
	_vm->graphics()->cameraBob(-1);
	BobSlot *bobAzura = _vm->graphics()->bob(5);
	BobSlot *bobJoe = _vm->graphics()->bob(6);
	while (_vm->display()->horizontalScroll() < 320) {
		_vm->display()->horizontalScroll(_vm->display()->horizontalScroll() + 8);
		if (bobJoe->x - bobAzura->x > 128) {
			bobAzura->x += 10;
			bobJoe->x += 6;
		}
		else {
			bobAzura->x += 8;
			bobJoe->x += 8;
		}
		update();
	}
}


void Logic::asmMakeLightningHitPlane() {

	_vm->graphics()->cameraBob(-1);
	short iy = 0, x, ydir = -1, j, k;
				
	BobSlot *planeBob     = _vm->graphics()->bob(5);
	BobSlot *lightningBob = _vm->graphics()->bob(20);

	planeBob->box.y2 = lightningBob->box.y2 = 199;
	planeBob->y = 135;

	planeBob->scale = 20;

	for (x = 660; x > 163; x -= 6) {
		planeBob->x = x;
		planeBob->y = 135 + iy;

		iy -= ydir;
		if (iy < -9 || iy > 9)
			ydir = -ydir;

		planeBob->scale++;
		if (planeBob->scale > 100)
			planeBob->scale = 100;

		int scrollX = x - 163;
		if (scrollX > 320)
			scrollX = 320;
		_vm->display()->horizontalScroll(scrollX);
		update();
	}

	planeBob->scale = 100;
	_vm->display()->horizontalScroll(0);

	planeBob->x -= -8;
	planeBob->y += 6;

	lightningBob->x = 160;
	lightningBob->y = 0;

	// 23/2/95 - Play lightning SFX
	// XXX sfxplay(NULLstr);

	_vm->graphics()->bankUnpack(18, lightningBob->frameNum, 15);
	_vm->graphics()->bankUnpack(4,  planeBob    ->frameNum, 15);

	// Plane plunges into the jungle!
	BobSlot *fireBob = _vm->graphics()->bob(6);

	fireBob->animating = true;
	fireBob->x = planeBob->x;
	fireBob->y = planeBob->y + 10;
				
	_vm->graphics()->bankUnpack(19, fireBob->frameNum, 15);
	update();

	k = 20;
	j = 1;

	for (x = 163; x > -30; x -= 10) {
		planeBob->y += 4;
		fireBob->y += 4;
		planeBob->x = fireBob->x = x;

		if (k < 40) {
			_vm->graphics()->bankUnpack(j, planeBob->frameNum, 15);
			_vm->graphics()->bankUnpack(k, fireBob ->frameNum, 15);
			k++;
			j++;

			if (j == 4)
				j = 1;
		}
					
		update();
	}

	_vm->graphics()->cameraBob(0);
}


void Logic::asmScaleBlimp() {

	int16 z = 256;
	BobSlot *bob = _vm->graphics()->bob(7);
	int16 x = bob->x;
	int16 y = bob->y;
	while (bob->x > 150) {
		bob->x = x * 256 / z + 150;
		bob->x = y * 256 / z + 112;
		bob->scale = 100 * 256 / z;

		++z;
		if (z % 6 == 0) {
			--x;
		}

		update();
	}
}


void Logic::asmScaleEnding() {

	_vm->graphics()->bob(7)->active = false; // Turn off blimp
	BobSlot *b = _vm->graphics()->bob(20);
	b->x = 160;
	b->y = 100;
	int i;
	for (i = 5; i <= 100; i += 5) {
		b->scale = i;
		update();
	}
	for (i = 0; i < 50; ++i) {
		update();
	}
	_vm->display()->palFadeOut(0, 255, currentRoom());
}


void Logic::asmWaitForCarPosition() {

	// Wait for car to reach correct position before pouring oil
	while (_vm->graphics()->bamData()->index != 60) {
		update();
	}
}


void Logic::asmShakeScreen() {

	OSystem::instance()->set_shake_pos(3);
	update();
	OSystem::instance()->set_shake_pos(0);
	update();
}


void Logic::asmAttemptPuzzle() {
	
	static short n = 0;
	++n;
	if (n & 4) {
		joeSpeak(226, true);
	}
}


void Logic::asmScaleTitle() {

	BobSlot *bob = _vm->graphics()->bob(5);
	bob->animating = false;
	bob->x = 161;
	bob->y = 200;
	bob->scale = 100;

	int i;
	for (i = 5; i <= 100; i +=5) {
		bob->scale = i;
		bob->y -= 4;
		update();
	}
}


void Logic::asmPanRightToHugh() {
	
	BobSlot *bob_thugA1 = _vm->graphics()->bob(20);
	BobSlot *bob_thugA2 = _vm->graphics()->bob(21);
	BobSlot *bob_thugA3 = _vm->graphics()->bob(22);
	BobSlot *bob_hugh1  = _vm->graphics()->bob(1);
	BobSlot *bob_hugh2  = _vm->graphics()->bob(23);
	BobSlot *bob_hugh3  = _vm->graphics()->bob(24);
	BobSlot *bob_thugB1 = _vm->graphics()->bob(25);
	BobSlot *bob_thugB2 = _vm->graphics()->bob(26);

	_vm->graphics()->cameraBob(-1);
	_vm->input()->fastMode(true);
	update();
				
	int i = 4, k = 160;

	// Adjust thug1 gun so it matches rest of body
	bob_thugA1->x += (k / 2) * 2 - 45; 
	bob_thugA2->x += (k / 2) * 2; 
	bob_thugA3->x += (k / 2) * 2;

	bob_hugh1->x += (k / 2) * 3 + (k / 2);
	bob_hugh2->x += (k / 2) * 3 + (k / 2);
	bob_hugh3->x += (k / 2) * 3 + (k / 2);

	bob_thugB1->x += (k / 2) * 4 + k; 
	bob_thugB2->x += (k / 2) * 4 + k; 

	if (i == 3) {
		bob_thugB1->x += 10;
		bob_thugB2->x += 10;
	}

	i *= 2;

	int horizontalScroll = 0;
	while (horizontalScroll < k) {

		horizontalScroll = horizontalScroll + i;
		if (horizontalScroll > k)
			horizontalScroll = k;

		//debug(0, "horizontalScroll = %i", horizontalScroll);

		_vm->display()->horizontalScroll(horizontalScroll);

		bob_thugA1->x -= i * 2; 
		bob_thugA2->x -= i * 2; 
		bob_thugA3->x -= i * 2;

		bob_hugh1->x -= i * 3;
		bob_hugh2->x -= i * 3;
		bob_hugh3->x -= i * 3;

		bob_thugB1->x -= i * 4;
		bob_thugB2->x -= i * 4;

		update();

		if (_vm->input()->cutawayQuit())
			return;
	}

	_vm->input()->fastMode(false);
}


void Logic::asmMakeWhiteFlash() {

	_vm->display()->palCustomFlash();
}


void Logic::asmPanRightToJoeAndRita() { // cdint.cut

	BobSlot *bob_box   = _vm->graphics()->bob(20);
	BobSlot *bob_beam  = _vm->graphics()->bob(21);
	BobSlot *bob_crate = _vm->graphics()->bob(22);
	BobSlot *bob_clock = _vm->graphics()->bob(23);
	BobSlot *bob_hands = _vm->graphics()->bob(24);

	_vm->graphics()->cameraBob(-1);
	_vm->input()->fastMode(true);
					
	update();

	bob_box  ->x += 280 * 2;
	bob_beam ->x += 30;
	bob_crate->x += 180 * 3;

	int horizontalScroll = _vm->display()->horizontalScroll();

	int i = 1;
	while (horizontalScroll < 290) {

		horizontalScroll = horizontalScroll + i;
		if (horizontalScroll > 290)
			horizontalScroll = 290;

		//debug(0, "horizontalScroll = %i", horizontalScroll);

		_vm->display()->horizontalScroll(horizontalScroll);

		bob_box  ->x -= i * 2;
		bob_beam ->x -= i;
		bob_crate->x -= i * 3;
		bob_clock->x -= i * 2;
		bob_hands->x -= i * 2;

		update();

		if (_vm->input()->cutawayQuit())
			return;
	}
	_vm->input()->fastMode(false);
}


void Logic::asmPanLeftToBomb() { // cdint.cut
		
	BobSlot *bob21 = _vm->graphics()->bob(21);
	BobSlot *bob22 = _vm->graphics()->bob(22);

	_vm->graphics()->cameraBob(-1);
	_vm->input()->fastMode(true);
				
	int horizontalScroll = _vm->display()->horizontalScroll();

	int i = 5;
	while (horizontalScroll > 0 || bob21->x < 136) {

		horizontalScroll -= i;
		if (horizontalScroll < 0)
			horizontalScroll = 0;

		//debug(0, "horizontalScroll = %i", horizontalScroll);
		_vm->display()->horizontalScroll(horizontalScroll);

		if (horizontalScroll < 272 && bob21->x < 136)
			bob21->x += (i/2);

		bob22->x += i;

		update();

		if (_vm->input()->cutawayQuit())
			return;
	}

	_vm->input()->fastMode(false);
}



} // End of namespace Queen

