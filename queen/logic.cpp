/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
#include "queen/bankman.h"
#include "queen/command.h"
#include "queen/credits.h"
#include "queen/cutaway.h"
#include "queen/debug.h"
#include "queen/defs.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/input.h"
#include "queen/journal.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/state.h"
#include "queen/talk.h"
#include "queen/walk.h"


namespace Queen {

Logic::Logic(QueenEngine *vm)
	:  _queen2jas(NULL), _credits(NULL), _vm(vm) {
	_joe.x = _joe.y = 0;
	_joe.scale = 100;
	memset(_gameState, 0, sizeof(_gameState));
	memset(_talkSelected, 0, sizeof(_talkSelected));
	_puzzleAttemptCount = 0;
	initialise();
	_journal = new Journal(vm);
}

Logic::~Logic() {
	delete _journal;
	delete _credits;
	delete _queen2jas;
}

void Logic::initialise() {	
	int16 i;

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
		_objectData[i].readFromBE(ptr);
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
	} else {
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
		_itemData[i].readFromBE(ptr);
	}

	// Graphic Image Data
	_numGraphics = READ_BE_UINT16(ptr); ptr += 2;

	_graphicData = new GraphicData[_numGraphics + 1];
	memset(&_graphicData[0], 0, sizeof(GraphicData));
	for (i = 1; i <= _numGraphics; i++) {
		_graphicData[i].readFromBE(ptr);
	}

	_vm->grid()->readDataFrom(_numObjects, _numRooms, ptr);

	// Walk OFF Data
	_numWalkOffs = READ_BE_UINT16(ptr);	ptr += 2;

	_walkOffData = new WalkOffData[_numWalkOffs + 1];
	memset(&_walkOffData[0], 0, sizeof(WalkOffData));
	for (i = 1; i <= _numWalkOffs; i++) {
		_walkOffData[i].readFromBE(ptr);
	}

	// Special Object Descriptions
	_numObjDesc = READ_BE_UINT16(ptr); ptr += 2;

	_objectDescription = new ObjectDescription[_numObjDesc + 1];
	memset(&_objectDescription[0], 0, sizeof(ObjectDescription));
	for (i = 1; i <= _numObjDesc; i++) {
		_objectDescription[i].readFromBE(ptr);
	}

	_vm->command()->readCommandsFrom(ptr);

	_entryObj = READ_BE_UINT16(ptr); ptr += 2;

	// Furniture DATA
	_numFurniture = READ_BE_UINT16(ptr); ptr += 2;

	_furnitureData = new FurnitureData[_numFurniture + 1];
	memset(&_furnitureData[0], 0, sizeof(_furnitureData));
	for (i = 1; i <= _numFurniture; i++) {
		_furnitureData[i].readFromBE(ptr);
	}

	// Actors
	_numActors = READ_BE_UINT16(ptr); ptr += 2;
	_numAAnim = READ_BE_UINT16(ptr); ptr += 2;
	_numAName = READ_BE_UINT16(ptr); ptr += 2;
	_numAFile = READ_BE_UINT16(ptr); ptr += 2;

	_actorData = new ActorData[_numActors + 1];
	memset(&_actorData[0], 0, sizeof(ActorData));
	for (i = 1; i <= _numActors; i++) {
		_actorData[i].readFromBE(ptr);
	}

	_numGraphicAnim = READ_BE_UINT16(ptr); ptr += 2;

	_graphicAnim = new GraphicAnim[_numGraphicAnim + 1];
	if (_numGraphicAnim == 0) {
		_graphicAnim[0].readFromBE(ptr);
	} else {
		memset(&_graphicAnim[0], 0, sizeof(GraphicAnim));
		for (i = 1; i <= _numGraphicAnim; i++) {
			_graphicAnim[i].readFromBE(ptr);
		}
	}

	_currentRoom = _objectData[_entryObj].room;
	_entryObj = 0;

	if(memcmp(ptr, _vm->resource()->JASVersion(), 5) != 0) {
		warning("Unexpected queen.jas file format");
	}

	delete[] jas;


	// Step 2 : read queen2.jas and grab all description texts

	_queen2jas = new LineReader((char*)_vm->resource()->loadFile("QUEEN2.JAS"));
	
	_objDescription = new char*[_numDescriptions + 1];
	_objDescription[0] = 0;
	for (i = 1; i <= _numDescriptions; i++)
		_objDescription[i] = _queen2jas->nextLine();

	//Patch for German text bug
	if (_vm->resource()->getLanguage() == GERMAN) {
		char *txt = new char[48];
		strcpy(txt, "Es bringt nicht viel, das festzubinden.");
		_objDescription[296] = txt;
	}
	
	_objName = new char*[_numNames + 1];
	_objName[0] = 0;
	for (i = 1; i <= _numNames; i++)
		_objName[i] = _queen2jas->nextLine();

	_roomName = new char*[_numRooms + 1];
	_roomName[0] = 0;
	for (i = 1; i <= _numRooms; i++)
		_roomName[i] = _queen2jas->nextLine();

	_verbName[0] = 0;
	for (i = 1; i <= 12; i++)
		_verbName[i] = _queen2jas->nextLine();

	_joeResponse[0] = 0;
	for (i = 1; i <= JOE_RESPONSE_MAX; i++)
		_joeResponse[i] = _queen2jas->nextLine();

	_aAnim = new char*[_numAAnim + 1];
	_aAnim[0] = 0;
	for (i = 1; i <= _numAAnim; i++)
		_aAnim[i] = _queen2jas->nextLine();

	_aName = new char*[_numAName + 1];
	_aName[0] = 0;
	for (i = 1; i <= _numAName; i++)
		_aName[i] = _queen2jas->nextLine();
	
	_aFile = new char*[_numAFile + 1];
	_aFile[0] = 0;
	for (i = 1; i <= _numAFile; i++)
		_aFile[i] = _queen2jas->nextLine();


	// Step 3 : initialise game state / variables

	_vm->command()->clear(false);
	_scene = 0;
	memset(_gameState, 0, sizeof(_gameState));
	_vm->display()->setupPanel();
	_vm->graphics()->bobSetupControl();
	setupJoe();
	_vm->grid()->setupPanel();

	_oldRoom = 0;
}


ObjectData* Logic::objectData(int index) const {
	if (index >= 0 && index <= _numObjects)
		return &_objectData[index];
	else
		error("[Logic::objectData] Invalid object data index: %i", index);
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
		} else {
			if(img <= -10) {
				// object has been turned off, but the image order hasn't been updated
				if(_graphicData[-(img + 10)].lastFrame != 0) {
					bobtype = 1;
				}
			} else if(img == -2) {
				// -1 static, -2 animated
				bobtype = 1;
			} else if(img > 0) {
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
					} else {
						++idxStatic;
					}
				} else if(img > 0) {
					if(img > 5000) {
						img -= 5000;
					}

					if (img > _numGraphics)
						warning("img (%i) > _numGraphics (%i)", img, _numGraphics);
					
					if(_graphicData[img].lastFrame != 0) {
						++idxAnimated;
					} else {
						++idxStatic;
					}
				} else if(img == -1) {
					++idxStatic;
				} else if(img == -2) {
					++idxAnimated;
				}
			}
			if(bobtype == 0) {
				// static bob
				if(idxStatic > 0) {
					bobnum = 19 + _numFurnitureStatic + idxStatic;
				}
			} else {
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
	} else {
		uint16 idx = 0;
		for(i = _roomData[room] + 1; i < obj; ++i) {
			img = _objectData[i].image;
			if(img <= -10) {
				GraphicData* pgd = &_graphicData[-(img + 10)];
				if(pgd->lastFrame != 0) {
					// skip all the frames of the animation
					idx += ABS(pgd->lastFrame) - pgd->firstFrame + 1;
				} else {
					// static bob, skip one frame
					++idx;
				}
			} else if(img == -1) {
				++idx;
			} else if(img > 0) {
				if(img > 5000) {
					img -= 5000;
				}
				GraphicData* pgd = &_graphicData[img];
				uint16 lastFrame = ABS(pgd->lastFrame);
				if(pgd->firstFrame < 0) {
					idx += lastFrame;
				} else if(lastFrame != 0) {
					idx += (lastFrame - pgd->firstFrame) + 1;
				} else {
					++idx;
				}
			}
		}

		img = _objectData[obj].image;
		if(img <= -10) {
			GraphicData* pgd = &_graphicData[-(img + 10)];
			if(pgd->lastFrame != 0) {
				idx += ABS(pgd->lastFrame) - pgd->firstFrame + 1;
			} else {
				++idx;
			}
		} else if(img == -1 || img > 0) {
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
	uint16 cur = currentRoomData() + 1;
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


void Logic::joeWalk(JoeWalkMode walking) {
	_joe.walk = walking;
	// Do this so that Input doesn't need to know the walk value
	_vm->input()->dialogueRunning(JWM_SPEAK == walking);
}


int16 Logic::gameState(int index) const {
	if (index >= 0 && index < GAME_STATE_COUNT)
		return _gameState[index];
	else
		error("[QueenLogic::gameState] invalid index: %i", index);
}

void Logic::gameState(int index, int16 newValue) {
	if (index >= 0 && index < GAME_STATE_COUNT) {
//		debug(6, "Logic::gameState() - GAMESTATE[%d] = %d", index, newValue);
		_gameState[index] = newValue;
	}
	else
		error("[QueenLogic::gameState] invalid index: %i", index);
}


void Logic::roomErase() {
	_vm->bankMan()->eraseAllFrames(false);
	_vm->bankMan()->close(15);
	_vm->bankMan()->close(11);
	_vm->bankMan()->close(10);
	_vm->bankMan()->close(12);

	if (_currentRoom >= 114) {
		_vm->display()->palFadeOut(0, 255, _currentRoom);
	} else {
		_vm->display()->palFadeOut(0, 223, _currentRoom);
	}

	// invalidates all persons animations
	uint16 i;
	for (i = 0; i <= 3; ++i) {
		_personFrames[i] = 0;
	}
	_vm->graphics()->eraseAllAnims();

	uint16 cur = _roomData[_oldRoom] + 1;
	uint16 last = _roomData[_oldRoom + 1];
	while (cur <= last) {
		ObjectData *pod = &_objectData[cur];
		if (pod->name == 0) {
			// object has been deleted, invalidate image
			pod->image = 0;
		} else if (pod->image > -4000 && pod->image <= -10) {
			if (_graphicData[ABS(pod->image + 10)].lastFrame == 0) {
				// static Bob
				pod->image = -1;
			} else {
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
				_vm->bankMan()->unpack(pgd->firstFrame, curImage, 15);
				++_numFrames;
				BobSlot *pbs = _vm->graphics()->bob(19 + _numFurnitureStatic);
				pbs->curPos(pgd->x, pgd->y);
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
					_vm->bankMan()->unpack(k, curImage, 15);
					++_numFrames;
				}
				BobSlot *pbs = _vm->graphics()->bob(5 + curBob);
				pbs->animNormal(image, curImage, pgd->speed / 4, rebound, false);
				pbs->curPos(pgd->x, pgd->y);
				++curBob;
			}
		}
	}

	// unpack the paste downs
	++curImage;
	for  (i = 1; i <= furnitureTotal; ++i) {
		if (gstate[i] > 5000) {;
			_vm->graphics()->bobPaste(gstate[i] - 5000, curImage);
		}
	}
}


void Logic::roomSetupObjects() {
	uint16 i;
	// furniture frames are reserved in ::roomSetupFurniture(), we append objects 
	// frames after the furniture ones.
	uint16 curImage = 36 + FRAMES_JOE_XTRA + _numFurnitureStatic + _numFurnitureAnimatedLen;
	uint16 firstRoomObj = currentRoomData() + 1;
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
		} else if(pod->image == -2) {
			// animated OFF Bob
			curBob = 5 + _numFurnitureAnimated + numObjectAnimated;
			++numObjectAnimated;
		} else if(pod->image > 0 && pod->image < 5000) {
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
				_vm->graphics()->setupObjectAnim(pgd, curImage + 1, curBob + numObjectAnimated, pod->name > 0);
				curImage += pgd->lastFrame;
				++numObjectAnimated;
			} else if (lastFrame != 0) {
				// animated objects
				uint16 j;
				uint16 firstFrame = curImage + 1;
				for (j = pgd->firstFrame; j <= lastFrame; ++j) {
					++curImage;
					_vm->bankMan()->unpack(j, curImage, 15);
					++_numFrames;
				}
				curBob = 5 + _numFurnitureAnimated + numObjectAnimated;
				if (pod->name > 0) {
					BobSlot *pbs = _vm->graphics()->bob(curBob);
					pbs->curPos(pgd->x, pgd->y);
					pbs->frameNum = firstFrame;
					if (pgd->speed > 0) {
						pbs->animNormal(firstFrame, curImage, pgd->speed / 4, rebound, false);
					}
				}
				++numObjectAnimated;
			} else {
				// static objects
				curBob = 20 + _numFurnitureStatic + numObjectStatic;
				++curImage;
				_vm->graphics()->bobClear(curBob);

				// FIXME: if((COMPANEL==2) && (FULLSCREEN==1)) bobs[CURRBOB].y2=199;

				_vm->bankMan()->unpack(pgd->firstFrame, curImage, 15);
				++_numFrames;
				if (pod->name > 0) {
					BobSlot *pbs = _vm->graphics()->bob(curBob);
					pbs->curPos(pgd->x, pgd->y);
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
			debug(6, "Logic::roomSetupObjects() - Setting up person %X, name=%X", i, pod->name);
			uint16 noun = i - currentRoomData();
			if (pod->name > 0) {
				curImage = setupPersonInRoom(noun, curImage);
			} else {
				curImage = countPersonFrames(noun, curImage);
			}
		}
	}

	// paste downs list
	++curImage;
	_numFrames = curImage;
	for (i = firstRoomObj; i <= lastRoomObj; ++i) {
		ObjectData *pod = &_objectData[i];
		if (pod->name > 0 && pod->image > 5000) {
			_vm->graphics()->bobPaste(pod->image - 5000, curImage);
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

	debug(6, "Logic::roomRefreshObject(%X, %s)", obj, _objName[ABS(pod->name)]);

	// check the object is in the current room
	if (pod->room != _currentRoom) {
		debug(6, "Refreshing an object (%i=%s) not in current room (object room=%i, current room=%i)", obj, _objName[ABS(pod->name)], pod->room, _currentRoom);
		return curImage;
	}

	// find bob for the object
	uint16 curBob = findBob(obj);
	BobSlot *pbs = _vm->graphics()->bob(curBob);

	if (pod->image == -3 || pod->image == -4) {
		// a person object
		if (pod->name <= 0) {
			_vm->graphics()->bobClear(curBob);
		} else {
			// find person number
			uint16 pNum = 1;
			uint16 i = currentRoomData() + 1;
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
			curImage = setupPersonInRoom(obj - currentRoomData(), curImage);
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
		_vm->graphics()->setupObjectAnim(pgd, curImage, curBob, pod->name != 0);
		curImage += pgd->lastFrame - 1;
	} else if (lastFrame != 0) {
		// turn on an animated bob
		_vm->bankMan()->unpack(pgd->firstFrame, 2, 15);
		pbs->animating = false;
		uint16 firstImage = curImage;
		--curImage;
		uint16 j;
		for (j = pgd->firstFrame; j <= lastFrame; ++j) {
			++curImage;
			_vm->bankMan()->unpack(j, curImage, 15);
		}
		pbs->curPos(pgd->x, pgd->y);
		pbs->frameNum = firstImage;
		if (pgd->speed > 0) {
			pbs->animNormal(firstImage, curImage, pgd->speed / 4, rebound, false);
		}
	} else {
		// frame 2 is used as a buffer frame to prevent BOB flickering
		_vm->bankMan()->unpack(pgd->firstFrame, 2, 15);
		_vm->bankMan()->unpack(pgd->firstFrame, curImage, 15);
		pbs->curPos(pgd->x, pgd->y);
		pbs->frameNum = curImage;
	}

	return curImage;
}


void Logic::roomSetup(const char *room, int comPanel, bool inCutaway) {
	// load backdrop image, init dynalum, setup colors
	_vm->display()->setupNewRoom(room, _currentRoom);

	// setup graphics to enter fullscreen/panel mode
	_vm->display()->screenMode(comPanel, inCutaway);

	// reset sprites table (bounding box...)
	_vm->graphics()->bobClearAll();

	// load/setup objects associated to this room
	char filename[20];	
	sprintf(filename, "%s.BBK", room);
	_vm->bankMan()->load(filename, 15);

	_vm->grid()->setupNewRoom(_currentRoom, _roomData[_currentRoom]);
	_numFrames = 37 + FRAMES_JOE_XTRA;
	roomSetupFurniture();
	roomSetupObjects();

	_vm->display()->forceFullRefresh();

	if (_currentRoom >= 90) {
		_vm->graphics()->putCameraOnBob(0);
	}
}


void Logic::roomDisplay(uint16 room, RoomDisplayMode mode, uint16 scale, int comPanel, bool inCutaway) {
	debug(6, "Logic::roomDisplay(%d, %d, %d, %d, %d)", room, mode, scale, comPanel, inCutaway);

	roomErase();

	if (_credits)
		_credits->nextRoom();

	roomSetup(roomName(room), comPanel, inCutaway);
	ObjectData *pod = NULL;
	if (mode != RDM_FADE_NOJOE) {
		pod = setupJoeInRoom(mode != RDM_FADE_JOE_XY, scale);
	}
	if (mode != RDM_NOFADE_JOE) {
		_vm->update();
		BobSlot *joe = _vm->graphics()->bob(0);
		if (IS_CD_INTRO_ROOM(_currentRoom)) {
			_vm->display()->palFadeIn(0, 255, _currentRoom, joe->active, joe->x, joe->y);
		} else {
			_vm->display()->palFadeIn(0, 223, _currentRoom, joe->active, joe->x, joe->y);
		}
	}
	if (pod != NULL) {
		_vm->walk()->moveJoe(0, pod->x, pod->y, inCutaway);
	}
}


ActorData *Logic::findActor(uint16 noun, const char *name) {
	uint16 obj = currentRoomData() + noun;
	int16 img = _objectData[obj].image;
	if (img != -3 && img != -4) {
		warning("Logic::findActor() - Object %d is not a person", obj);
		return NULL;
	}

	// search Bob number for the person
	uint16 i;
	uint16 bobNum = 0;
	for (i = currentRoomData() + 1; i <= obj; ++i) {
		img = _objectData[i].image;
		if (img == -3 || img == -4) {
			++bobNum;
		}
	}

	// search for a matching actor
	if (bobNum > 0) {
		for (i = 1; i <= _numActors; ++i) {
			ActorData *pad = &_actorData[i];
			if (pad->room == _currentRoom && gameState(pad->gsSlot) == pad->gsValue) {
				if (bobNum == pad->bobNum || (name && !strcmp(_aName[pad->name], name))) {
					return pad;
				}
			}
		}
	}
	return NULL;
}


void Logic::initPerson(int16 noun, const char *actorName, bool loadBank, Person *pp) {
	if (noun <= 0) {
		warning("Logic::initPerson() - Invalid object number: %i", noun);
	}	
	ActorData *pad = findActor(noun, actorName);
	if (pad != NULL) {
		pp->actor = pad;
		pp->name = _aName[pad->name];
		if (pad->anim != 0) {
			pp->anim = _aAnim[pad->anim];
		} else {
			pp->anim = NULL;
		}
		if (loadBank && pad->file != 0) {
			_vm->bankMan()->load(_aFile[pad->file], pad->bankNum);
			// if there is no valid actor file (ie pad->file is 0), the person 
			// data is already loaded as it is contained in objects room bank (.bbk)
		}
		pp->bobFrame = 29 + FRAMES_JOE_XTRA + pp->actor->bobNum;
	}
}


uint16 Logic::setupPersonInRoom(uint16 noun, uint16 curImage) {
	if (noun == 0) {
		warning("Trying to setup person 0");
		return curImage;
	}

	Person p;
	initPerson(noun, "", true, &p);

	const ActorData *pad = p.actor;
	uint16 scale = 100;
	uint16 a = _vm->grid()->findAreaForPos(GS_ROOM, pad->x, pad->y);
	if (a > 0) {
		// person is not standing in the area box, scale it accordingly
		scale = _vm->grid()->area(_currentRoom, a)->calcScale(pad->y);
	}

	_vm->bankMan()->unpack(pad->bobFrameStanding, p.bobFrame, p.actor->bankNum);
	uint16 obj = currentRoomData() + noun;
	BobSlot *pbs = _vm->graphics()->bob(pad->bobNum);
	pbs->curPos(pad->x, pad->y);
	pbs->scale = scale;
	pbs->frameNum = p.bobFrame;
	pbs->xflip = (_objectData[obj].image == -3); // person is facing left

	debug(6, "Logic::personSetup(%d, %d) - bob = %d name = %s", noun, curImage, pad->bobNum, p.name);

	if (p.anim != NULL) {
		_personFrames[pad->bobNum] = curImage + 1;
		curImage = _vm->graphics()->setupPersonAnim(pad, p.anim, curImage);
	} else {
		_vm->graphics()->erasePersonAnim(pad->bobNum);
	}
	return curImage;
}


uint16 Logic::countPersonFrames(uint16 noun, uint16 curImage) {
	ActorData *pad = findActor(noun);
	if (pad != NULL && pad->anim != 0) {
		curImage += _vm->graphics()->countAnimFrames(_aAnim[pad->anim]);
		_personFrames[pad->bobNum] = curImage + 1;
	}
	return curImage;
}


void Logic::loadJoeBanks(const char *animBank, const char *standBank) {
	int i;
	_vm->bankMan()->load(animBank, 13);
	for (i = 11; i <= 28 + FRAMES_JOE_XTRA; ++i) {
		_vm->bankMan()->unpack(i - 10, i, 13);
	}
	_vm->bankMan()->close(13);

	_vm->bankMan()->load(standBank, 7);
	_vm->bankMan()->unpack(1, 33 + FRAMES_JOE_XTRA, 7);
	_vm->bankMan()->unpack(3, 34 + FRAMES_JOE_XTRA, 7);
	_vm->bankMan()->unpack(5, 35 + FRAMES_JOE_XTRA, 7);
}


void Logic::setupJoe() {
	loadJoeBanks("joe_a.BBK", "joe_b.BBK");
	joePrevFacing(DIR_FRONT);
	joeFacing(DIR_FRONT);
}


ObjectData *Logic::setupJoeInRoom(bool autoPosition, uint16 scale) {
	debug(9, "Logic::setupJoeInRoom(%d, %d) joe.x=%d joe.y=%d", autoPosition, scale, _joe.x, _joe.y);

	uint16 oldx;
	uint16 oldy;
	WalkOffData *pwo = NULL;
	ObjectData *pod = objectData(_entryObj);
	if (pod == NULL) {
		error("Logic::setupJoeInRoom() - No object data for obj %d", _entryObj);
	}

	if (!autoPosition || joeX() != 0 || joeY() != 0) {
		oldx = joeX();
		oldy = joeY();
	} else {
		// find the walk off point for the entry object and make 
		// Joe walking to that point
		pwo = walkOffPointForObject(_entryObj);
		if (pwo != NULL) {
			oldx = pwo->x;
			oldy = pwo->y;
		} else {
			// no walk off point, use object position
			oldx = pod->x;
			oldy = pod->y;
		}
	}

	debug(6, "Logic::joeSetupInRoom() - oldx=%d, oldy=%d scale=%d", oldx, oldy, scale);

	if (scale > 0 && scale < 100) {
		joeScale(scale);
	} else {
		uint16 a = _vm->grid()->findAreaForPos(GS_ROOM, oldx, oldy);
		if (a > 0) {
			joeScale(_vm->grid()->area(_currentRoom, a)->calcScale(oldy));
		} else {
			joeScale(100);
		}
	}

	if (joeCutFacing() > 0) {
		joeFacing(joeCutFacing());
		joeCutFacing(0);
	} else {
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
		_vm->graphics()->putCameraOnBob(-1);
		_vm->bankMan()->load("joe_e.act", 7);
		_vm->bankMan()->unpack(2, 29 + FRAMES_JOE_XTRA, 7);

		_vm->display()->horizontalScroll(320);

		joeFacing(DIR_RIGHT);
		joeCutFacing(DIR_RIGHT);
		joePrevFacing(DIR_RIGHT);
	}

	joeFace();
	pbs->curPos(oldx, oldy);
	pbs->frameNum = 29 + FRAMES_JOE_XTRA;
	joePos(0, 0);

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
	} else {
		frame = 33;
		if (joeFacing() == DIR_FRONT) {
			if (joePrevFacing() == DIR_BACK) {
				pbs->frameNum = 33 + FRAMES_JOE_XTRA;
				_vm->update();
			}
			frame = 34;
		} else if (joeFacing() == DIR_BACK) {
			if (joePrevFacing() == DIR_FRONT) {
				pbs->frameNum = 33 + FRAMES_JOE_XTRA;
				_vm->update();
			}
			frame = 35;
		} else if ((joeFacing() == DIR_LEFT && joePrevFacing() == DIR_RIGHT) 
			|| 	(joeFacing() == DIR_RIGHT && joePrevFacing() == DIR_LEFT)) {
			pbs->frameNum = 34 + FRAMES_JOE_XTRA;
			_vm->update();
		}
		pbs->frameNum = frame + FRAMES_JOE_XTRA;
		pbs->scale = joeScale();
		pbs->xflip = (joeFacing() == DIR_LEFT);
		_vm->update();
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
	_vm->bankMan()->unpack(frame, pbs->frameNum, 7);
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
		} else if (joeFacing() == DIR_FRONT) {
			frame = 4;
		} else {
			frame = 2;
		}
		break;

	case STATE_GRAB_DOWN:
		if (joeFacing() == DIR_BACK) {
			frame = 9;
		} else {
			frame = 8;
		}
		break;

	case STATE_GRAB_UP:
		// turn back
		_vm->bankMan()->unpack(5, 29 + FRAMES_JOE_XTRA, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		_vm->update();
		// grab up
		_vm->bankMan()->unpack(7, 29 + FRAMES_JOE_XTRA, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		_vm->update();
		// turn back
		frame = 7;
		break;
	}

	if (frame != 0) {
		_vm->bankMan()->unpack(frame, 29 + FRAMES_JOE_XTRA, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		_vm->update();

		// extra delay for grab down
		if (grabState == STATE_GRAB_DOWN) {
			_vm->update();
			_vm->update();
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
		} else {
			playCutaway("cudrs.CUT");
		}
	}
	_vm->display()->palSetJoeDress();
	loadJoeBanks("JoeD_A.BBK", "JoeD_B.BBK");
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
	_vm->display()->palSetJoeNormal();
	loadJoeBanks("Joe_A.BBK", "Joe_B.BBK");
	inventoryDeleteItem(ITEM_CLOTHES);
	gameState(VAR_DRESSING_MODE, 0);
}


void Logic::joeUseUnderwear() {
	_vm->display()->palSetJoeNormal();
	loadJoeBanks("JoeU_A.BBK", "JoeU_B.BBK");
	gameState(VAR_DRESSING_MODE, 1);
}


void Logic::makePersonSpeak(const char *sentence, Person *person, const char *voiceFilePrefix) {
	_vm->command()->clear(false);
	Talk::speak(sentence, person, voiceFilePrefix, _vm);
}


void Logic::startDialogue(const char *dlgFile, int personInRoom, char *cutaway) {
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


void Logic::makeJoeSpeak(uint16 descNum, bool objectType) {
	// makeJoeSpeak(k, false) == SPEAK(JOE_RESPstr[k],"JOE",find_cd_desc(k)) 
	// makeJoeSpeak(k, true)  == SPEAK(OBJECT_DESCRstr[k],"JOE",find_cd_desc(JOERESPMAX+k))
	const char *text = objectType ? _objDescription[descNum] : _joeResponse[descNum];
	if (objectType) {
		descNum += JOE_RESPONSE_MAX;
	}
	char descFilePrefix[10];
	sprintf(descFilePrefix, "JOE%04i", descNum);
	makePersonSpeak(text, NULL, descFilePrefix);
}


uint16 Logic::findInventoryItem(int invSlot) const {
	// queen.c l.3894-3898
	if (invSlot >= 0 && invSlot < 4) {
		return _inventoryItem[invSlot];
	}
	return 0;
}


void Logic::inventorySetup() {
	_vm->bankMan()->load("objects.BBK", 14);
	if (_vm->resource()->isInterview()) {
		_inventoryItem[0] = 1;
		_inventoryItem[1] = 2;
		_inventoryItem[2] = 3;
		_inventoryItem[3] = 4;
	} else {
		_inventoryItem[0] = ITEM_BAT;
		_inventoryItem[1] = ITEM_JOURNAL;
		_inventoryItem[2] = ITEM_NONE;
		_inventoryItem[3] = ITEM_NONE;
	}
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
			_vm->bankMan()->unpack(_itemData[itemNum].frame, dstFrame, 14);
			_vm->graphics()->bobDrawInventoryItem(dstFrame, x, 14);
		} else {
			// no object, clear the panel 
			_vm->graphics()->bobDrawInventoryItem(0, x, 14);
		}
		x += 35;
	}
	// XXX OLDVERB=VERB;
	_vm->update();
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


void Logic::removeHotelItemsFromInventory() {
	if (currentRoom() == 1 && gameState(3) == 0) {
		inventoryDeleteItem(ITEM_CROWBAR, false);
		inventoryDeleteItem(ITEM_DRESS, false);
		inventoryDeleteItem(ITEM_CLOTHES, false);
		inventoryDeleteItem(ITEM_HAY, false);
		inventoryDeleteItem(ITEM_OIL, false);
		inventoryDeleteItem(ITEM_CHICKEN, false);
		gameState(3, 1);
		inventoryRefresh();
	}
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


void Logic::handleSpecialArea(int facing, uint16 areaNum, uint16 walkDataNum) {
	// queen.c l.2838-2911
	debug(9, "handleSpecialArea(%d, %d, %d)\n", facing, areaNum, walkDataNum);

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
		makeJoeSpeak(16);
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
		} else if (walkDataNum == 2) {
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
		makeJoeSpeak(19);
		break;
	case ROOM_HOTEL_DOWNSTAIRS:
		makeJoeSpeak(21);
		break;
	case ROOM_HOTEL_LOBBY:
		if (_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] == 0) {
			playCutaway("c73a.CUT"); 
			_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] = 1;
			joeUseUnderwear();
			joeFace();
		} else if (_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] == 1) {
			playCutaway("c73b.CUT");
			_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] = 2;
		} else if (_gameState[VAR_ESCAPE_FROM_HOTEL_COUNT] == 2) {
			playCutaway("c73c.CUT");
		}
		break;
	case ROOM_TEMPLE_MAZE_5:
		if (areaNum == 7) {
			makeJoeSpeak(17);
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
			} else if (_gameState[VAR_BYPASS_FLODA_RECEPTIONIST] == 0) {
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
	_vm->graphics()->putCameraOnBob(-1);
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

	_vm->update();
	_vm->display()->palFadeIn(0, 223, ROOM_JUNGLE_PINNACLE, joe->active, joe->x, joe->y);
	_vm->graphics()->textCurrentColor(INK_PINNACLE_ROOM);

	_entryObj = 0;
	uint16 prevObj = 0;
	while (_vm->input()->mouseButton() == 0 || _entryObj == 0) {

		_vm->update();
		int mx = _vm->input()->mousePosX();
		int my = _vm->input()->mousePosY();

		// update screen scrolling
		_vm->display()->horizontalScroll(mx);

		// update bobs position / frame
		joe->x = piton->x = 3 * mx / 4 + 200;
		joe->frameNum = mx / 36 + 43 + FRAMES_JOE_XTRA;

		_vm->graphics()->textClear(5, 5);

		uint16 curObj = _vm->grid()->findObjectUnderCursor(mx, my);
		if (curObj != 0 && curObj != prevObj) {
			_entryObj = 0;
			curObj += currentRoomData(); // global object number
			ObjectData *objData = objectData(curObj);
			if (objData->name > 0) {
				_entryObj = objData->entryObj;
				char textCmd[CmdText::MAX_COMMAND_LEN];
				sprintf(textCmd, "%s %s", verbName(VERB_WALK_TO), objectName(objData->name));
				_vm->graphics()->textSetCentered(5, textCmd);
			}
			prevObj = curObj;
		}
	}
	_vm->input()->clearMouseButton();

	_newRoom = objectData(_entryObj)->room;

	// FIXME - only a few commands can be triggered from this room :
	// piton -> crash  : 0x216 (obj1=0x2a, song=3)
	// piton -> floda  : 0x217 (obj1=0x29, song=16)
	// piton -> bob    : 0x219 (obj1=0x2f, song=6)
	// piton -> embark : 0x218 (obj1=0x2c, song=7)
	// piton -> jungle : 0x20B (obj1=0x2b, song=3)
	// piton -> amazon : 0x21A (obj1=0x30, song=3)
	// 
	// Because none of these update objects/areas/gamestate, the EXECUTE_ACTION()
	// call, as the original does, is useless. All we have to do is the playsong 
	// call (all songs have the PLAY_BEFORE type). This way we could get rid of 
	// the hack described in execute.c l.334-339.
	struct { 
		uint16 obj;
		int16 song;
	} songs[] = { 
		{ 0x2A,  3 },
		{ 0x29, 16 },
		{ 0x2F,  6 },
		{ 0x2C,  7 },
		{ 0x2B,  3 },
		{ 0x30,  3 }
	};
	for (int i = 0; i < ARRAYSIZE(songs); ++i) {
		if (songs[i].obj == prevObj) {
			_vm->sound()->playSong(songs[i].song);
		}
	}

	joe->active = piton->active = false;
	_vm->graphics()->textClear(5, 5);

	// camera follows Joe again
	_vm->graphics()->putCameraOnBob(0);

	_vm->display()->palFadeOut(0, 223, ROOM_JUNGLE_PINNACLE);
}


void Logic::update() {
	if (_credits)
		_credits->update();

	if (_vm->debugger()->_drawAreas) {
		_vm->grid()->drawZones();
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
	
	WRITE_BE_UINT16(ptr, _vm->talkSpeed()); ptr += 2;
	WRITE_BE_UINT16(ptr, 0 /*_settings.musicVolume*/); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->sound()->sfxOn() ? 1 : 0); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->sound()->speechOn() ? 1 : 0); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->sound()->musicOn() ? 1 : 0); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->subtitles() ? 1 : 0); ptr += 2;
	
	for (i = 0; i < 4; i++) {
		WRITE_BE_UINT16(ptr, _inventoryItem[i]); ptr += 2;
	}
	
	WRITE_BE_UINT16(ptr, _vm->graphics()->bob(0)->x); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->graphics()->bob(0)->y); ptr += 2;
	WRITE_BE_UINT16(ptr, _currentRoom); ptr += 2;

	for (i = 1; i <= _numObjects; i++)
		_objectData[i].writeToBE(ptr);
		
	for (i = 1; i <= _numItems; i++)
		_itemData[i].writeToBE(ptr);
		
	for (i = 0; i < GAME_STATE_COUNT; i++) {
		WRITE_BE_UINT16(ptr, gameState(i)); ptr += 2;
	}
	
	for (i = 1; i <= _numRooms; i++)
		for (j = 1; j <= _vm->grid()->areaMax(i); j++)
			_vm->grid()->area(i, j)->writeToBE(ptr);
			
	for (i = 0; i < TALK_SELECTED_COUNT; i++)
			_talkSelected[i].writeToBE(ptr);
	
	for (i = 1; i <= _numWalkOffs; i++)
		_walkOffData[i].writeToBE(ptr);

	WRITE_BE_UINT16(ptr, _joe.facing); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->bam()->_flag); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->sound()->lastOverride()); ptr += 2;
	
	//TODO: lastmerge, lastalter, altmrgpri
	for (i = 0; i < 3; i++) {
		WRITE_BE_UINT16(ptr, 0); ptr += 2;
	}
	// TOADD:
	//	Logic::_puzzleAttemptCount
	//	Logic::_objectDescription

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
	_vm->subtitles(READ_BE_UINT16(ptr) != 0); ptr += 2;

	for (i = 0; i < 4; i++) {
		_inventoryItem[i] = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}

	_joe.x = (int16)READ_BE_UINT16(ptr); ptr += 2;
	_joe.y = (int16)READ_BE_UINT16(ptr); ptr += 2;

	currentRoom(READ_BE_UINT16(ptr)); ptr += 2;
	
	for (i = 1; i <= _numObjects; i++)
		_objectData[i].readFromBE(ptr);

	for (i = 1; i <= _numItems; i++)
		_itemData[i].readFromBE(ptr);

	for (i = 0; i < GAME_STATE_COUNT; i++) {
		gameState(i, (int16)READ_BE_UINT16(ptr)); ptr += 2;
	}

	for (i = 1; i <= _numRooms; i++)
		for (j = 1; j <= _vm->grid()->areaMax(i); j++)
			_vm->grid()->area(i, j)->readFromBE(ptr);
	
	for (i = 0; i < TALK_SELECTED_COUNT; i++)
		_talkSelected[i].readFromBE(ptr);
		
	for (i = 1; i <= _numWalkOffs; i++)
		_walkOffData[i].readFromBE(ptr);

	joeFacing(READ_BE_UINT16(ptr));  ptr += 2;
	_vm->bam()->_flag = READ_BE_UINT16(ptr); ptr += 2;
	_vm->sound()->playSong((int16)READ_BE_UINT16(ptr)); ptr += 2;
	
	//TODO: lastmerge, lastalter, altmrgpri
	for (i = 0; i < 3; i++) {
		READ_BE_UINT16(ptr); ptr += 2;
	}

	if ((ptr - saveData) != SAVEGAME_SIZE) {
		delete[] saveData;
		return false;
	}
	
	if (_vm->bam()->_flag != BamScene::F_STOP) {
		_vm->bam()->prepareAnimation();
	}

	joeCutFacing(joeFacing());
	joeFace();
	
	//OLDX = _joe.x;
	//OLDY = _joe.y;
	_oldRoom = 0;
	newRoom(_currentRoom);
	_entryObj = 0;

	switch (gameState(VAR_DRESSING_MODE)) {
	case 0: 
		joeUseClothes(false);
		break;
	case 1:
		joeUseUnderwear();
		break;
	case 2:
		joeUseDress(false);
		break;
	}

	inventoryRefresh();

	delete[] saveData;
	return true;
}

void Logic::sceneStart() {
	debug(6, "[Logic::sceneStart] _scene = %i", _scene);
	_scene++;

	_vm->display()->showMouseCursor(false);

	if (1 == _scene) {
		_vm->display()->palFadePanel();
	}

	_vm->update();
}

void Logic::sceneStop() {
	debug(6, "[Logic::sceneStop] _scene = %i", _scene);
	_scene--;

	if (_scene > 0)
		return;

	_vm->display()->palSetAllDirty();
	_vm->display()->showMouseCursor(true);
	_vm->grid()->setupPanel();
}


void Logic::changeRoom() {
	if (!preChangeRoom()) 
		roomDisplay(currentRoom(), RDM_FADE_JOE, 100, 1, false);
	_vm->display()->showMouseCursor(true);
}


void Logic::useJournal() {
	if (_vm->resource()->isDemo()) {
		makePersonSpeak("This is a demo, so I can't load or save games*14", NULL, "");
	} else if (!_vm->resource()->isInterview()) {
		_vm->command()->clear(false);
		_journal->use();
		_vm->walk()->stopJoe();
		// XXX TALKQUIT=CUTQUIT=0; Make sure that we turn off cut stuff in case we use Journal during cutaways
	}
}


void Logic::executeSpecialMove(uint16 sm) {
	debug(6, "Special move: %d", sm);
	if (!handleSpecialMove(sm))
		warning("unhandled / invalid special move : %d", sm);
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
	_vm->display()->palSetJoeDress();
}


void Logic::asmSwitchToNormalPalette() {
	_vm->display()->palSetJoeNormal();
}


void Logic::asmStartCarAnimation() {
	_vm->bam()->_flag = BamScene::F_PLAY;
	_vm->bam()->prepareAnimation();
}


void Logic::asmStopCarAnimation() {
	_vm->bam()->_flag = BamScene::F_STOP;
	_vm->graphics()->bob(findBob(594))->active = false; // oil object
	_vm->graphics()->bob(7)->active = false; // gun shots
}


void Logic::asmStartFightAnimation() {
	_vm->bam()->_flag = BamScene::F_PLAY;
	_vm->bam()->prepareAnimation();
	gameState(148, 1);
}


void Logic::asmWaitForFrankPosition() {	
	_vm->bam()->_flag = BamScene::F_REQ_STOP;
	while (_vm->bam()->_flag != BamScene::F_STOP) {
		_vm->update();
	}
}


void Logic::asmMakeFrankGrowing() {
	_vm->bankMan()->unpack(1, 38, 15);
	BobSlot *bobFrank = _vm->graphics()->bob(5);
	bobFrank->frameNum = 38;
	bobFrank->curPos(160, 200);
	bobFrank->box.y2 = GAME_SCREEN_HEIGHT - 1;

	int i;
	for (i = 10; i <= 100; i += 4) {
		bobFrank->scale = i;
		_vm->update();
	}
	for (i = 0; i <= 20; ++i) {
		_vm->update();
	}

	objectData(521)->name =  ABS(objectData(521)->name); // Dinoray
	objectData(526)->name =  ABS(objectData(526)->name); // Frank obj
	objectData(522)->name = -ABS(objectData(522)->name); // TMPD object off
	objectData(525)->name = -ABS(objectData(525)->name); // Floda guards off
	objectData(523)->name = -ABS(objectData(523)->name); // Sparky object off
	gameState(157, 1); // No more Ironstein
}


void Logic::asmMakeRobotGrowing() { 	
	_vm->bankMan()->unpack(1, 38, 15);
	BobSlot *bobRobot = _vm->graphics()->bob(5);
	bobRobot->frameNum = 38;
	bobRobot->curPos(160, 200);
	bobRobot->box.y2 = GAME_SCREEN_HEIGHT - 1;

	int i;
	for (i = 10; i <= 100; i += 4) {
		bobRobot->scale = i;
		_vm->update();
	}
	for (i = 0; i <= 20; ++i) {
		_vm->update();
	}
	
	objectData(524)->name = -ABS(objectData(524)->name); // Azura object off
	objectData(526)->name = -ABS(objectData(526)->name); // Frank object off
}


void Logic::asmShrinkRobot() {
	int i;
	BobSlot *robot = _vm->graphics()->bob(6);
	for (i = 100; i >= 35; i -= 5) {
		robot->scale = i;
		_vm->update();
	}
}


void Logic::asmEndGame() {
	int i;
	for (i = 0; i < 40; ++i) {
		_vm->update();
	}
	debug(0, "Game completed.");
	OSystem::instance()->quit();
}


void Logic::asmPutCameraOnDino() {
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 320) {
		scrollx += 16;
		if (scrollx > 320) {
			scrollx = 320;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
	_vm->graphics()->putCameraOnBob(1);
}


void Logic::asmPutCameraOnJoe() {
	_vm->graphics()->putCameraOnBob(0);
}


void Logic::asmAltIntroPanRight() {
	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
	_vm->update();
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 285 && !_vm->input()->cutawayQuit()) {
		++scrollx;
		if (scrollx > 285) {
			scrollx = 285;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
	_vm->input()->fastMode(false);
}


void Logic::asmAltIntroPanLeft() {
	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx > 0 && !_vm->input()->cutawayQuit()) {
		scrollx -= 4;
		if (scrollx < 0) {
			scrollx = 0;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
	_vm->input()->fastMode(false);
}


void Logic::asmSetAzuraInLove() {
	gameState(VAR_AZURA_IN_LOVE, 1);
}


void Logic::asmPanRightFromJoe() {
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 320) {
		scrollx += 16;
		if (scrollx > 320) {
			scrollx = 320;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
}


void Logic::asmSetLightsOff() {
	_vm->display()->palCustomLightsOff(currentRoom());
}


void Logic::asmSetLightsOn() {
	_vm->display()->palCustomLightsOn(currentRoom());
}


void Logic::asmSetManequinAreaOn() {
	Area *a = _vm->grid()->area(ROOM_FLODA_FRONTDESK, 7);
	a->mapNeighbours = ABS(a->mapNeighbours);
}


void Logic::asmPanToJoe() {
	int i = _vm->graphics()->bob(0)->x - 160;
	if (i < 0) {
		i = 0;
	} else if (i > 320) {
		i = 320;
	}
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	if (i < scrollx) {
		while (scrollx > i) {
			scrollx -= 16;
			if (scrollx < i) {
				scrollx = i;
			}
			_vm->display()->horizontalScroll(scrollx);
			_vm->update();
		}
	} else {
		while (scrollx < i) {
			scrollx += 16;
			if (scrollx > i) {
				scrollx = i;
			}
			_vm->display()->horizontalScroll(scrollx);
			_vm->update();
		}
		_vm->update();
	}
	_vm->graphics()->putCameraOnBob(0);
}


void Logic::asmTurnGuardOn() {
	gameState(85, 1);
}


void Logic::asmPanLeft320To144() {
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx > 144) {
		scrollx -= 8;
		if (scrollx < 144) {
			scrollx = 144;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
}


void Logic::asmSmooch() {
	_vm->graphics()->putCameraOnBob(-1);
	BobSlot *bobAzura = _vm->graphics()->bob(5);
	BobSlot *bobJoe = _vm->graphics()->bob(6);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 320) {
		scrollx += 8;
		_vm->display()->horizontalScroll(scrollx);
		if (bobJoe->x - bobAzura->x > 128) {
			bobAzura->x += 10;
			bobJoe->x += 6;
		} else {
			bobAzura->x += 8;
			bobJoe->x += 8;
		}
		_vm->update();
	}
}


void Logic::asmMakeLightningHitPlane() {
	_vm->graphics()->putCameraOnBob(-1);
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
		_vm->update();
	}

	planeBob->scale = 100;
	_vm->display()->horizontalScroll(0);

	planeBob->x -= -8;
	planeBob->y += 6;

	lightningBob->x = 160;
	lightningBob->y = 0;

	// 23/2/95 - Play lightning SFX
	_vm->sound()->playSfx(_vm->logic()->currentRoomSfx());

	_vm->bankMan()->unpack(18, lightningBob->frameNum, 15);
	_vm->bankMan()->unpack(4,  planeBob    ->frameNum, 15);

	// Plane plunges into the jungle!
	BobSlot *fireBob = _vm->graphics()->bob(6);

	fireBob->animating = true;
	fireBob->x = planeBob->x;
	fireBob->y = planeBob->y + 10;
				
	_vm->bankMan()->unpack(19, fireBob->frameNum, 15);
	_vm->update();

	k = 20;
	j = 1;

	for (x = 163; x > -30; x -= 10) {
		planeBob->y += 4;
		fireBob->y += 4;
		planeBob->x = fireBob->x = x;

		if (k < 40) {
			_vm->bankMan()->unpack(j, planeBob->frameNum, 15);
			_vm->bankMan()->unpack(k, fireBob ->frameNum, 15);
			k++;
			j++;

			if (j == 4)
				j = 1;
		}
					
		_vm->update();
	}

	_vm->graphics()->putCameraOnBob(0);
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

		_vm->update();
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
		_vm->update();
	}
	for (i = 0; i < 50; ++i) {
		_vm->update();
	}
	_vm->display()->palFadeOut(0, 255, currentRoom());
}


void Logic::asmWaitForCarPosition() {
	// Wait for car to reach correct position before pouring oil
	while (_vm->bam()->_index != 60) {
		_vm->update();
	}
}


void Logic::asmShakeScreen() {
	OSystem::instance()->set_shake_pos(3);
	_vm->update();
	OSystem::instance()->set_shake_pos(0);
	_vm->update();
}


void Logic::asmAttemptPuzzle() {
	++_puzzleAttemptCount;
	if (_puzzleAttemptCount & 4) {
		makeJoeSpeak(226, true);
		_puzzleAttemptCount = 0;
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
		_vm->update();
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

	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
	_vm->update();
				
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
	while (horizontalScroll < k && !_vm->input()->cutawayQuit()) {

		horizontalScroll = horizontalScroll + i;
		if (horizontalScroll > k)
			horizontalScroll = k;

		_vm->display()->horizontalScroll(horizontalScroll);

		bob_thugA1->x -= i * 2; 
		bob_thugA2->x -= i * 2; 
		bob_thugA3->x -= i * 2;

		bob_hugh1->x -= i * 3;
		bob_hugh2->x -= i * 3;
		bob_hugh3->x -= i * 3;

		bob_thugB1->x -= i * 4;
		bob_thugB2->x -= i * 4;

		_vm->update();
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

	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
					
	_vm->update();

	bob_box  ->x += 280 * 2;
	bob_beam ->x += 30;
	bob_crate->x += 180 * 3;

	int horizontalScroll = _vm->display()->horizontalScroll();

	int i = 1;
	while (horizontalScroll < 290 && !_vm->input()->cutawayQuit()) {

		horizontalScroll = horizontalScroll + i;
		if (horizontalScroll > 290)
			horizontalScroll = 290;

		_vm->display()->horizontalScroll(horizontalScroll);

		bob_box  ->x -= i * 2;
		bob_beam ->x -= i;
		bob_crate->x -= i * 3;
		bob_clock->x -= i * 2;
		bob_hands->x -= i * 2;

		_vm->update();
	}
	_vm->input()->fastMode(false);
}


void Logic::asmPanLeftToBomb() {
	BobSlot *bob21 = _vm->graphics()->bob(21);
	BobSlot *bob22 = _vm->graphics()->bob(22);

	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
				
	int horizontalScroll = _vm->display()->horizontalScroll();

	int i = 5;
	while ((horizontalScroll > 0 || bob21->x < 136) && !_vm->input()->cutawayQuit()) {

		horizontalScroll -= i;
		if (horizontalScroll < 0)
			horizontalScroll = 0;

		_vm->display()->horizontalScroll(horizontalScroll);

		if (horizontalScroll < 272 && bob21->x < 136)
			bob21->x += (i/2);

		bob22->x += i;

		_vm->update();
	}

	_vm->input()->fastMode(false);
}


void Logic::asmEndDemo() {
	debug(0, "Flight of the Amazon Queen, released January 95.");
	OSystem::instance()->quit();
}


void Logic::asmInterviewIntro() {
	// put camera on airship
	_vm->graphics()->putCameraOnBob(5);
	BobSlot *bas = _vm->graphics()->bob(5);

	bas->curPos(-30, 40);

	bas->move(700, 10, 3);
	int scale = 450;
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		bas->scale = 256 * 100 / scale;
		--scale;
		if (scale < 256) {
			scale = 256;
		}
		_vm->update();
	}

	bas->scale = 90;
	bas->xflip = true;

	bas->move(560, 25, 4);
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		_vm->update();
	}

	bas->move(545, 65, 2);
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		_vm->update();
	}

	bas->move(540, 75, 2);
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		_vm->update();
	}

	// put camera on Joe
	_vm->graphics()->putCameraOnBob(0);
}


void Logic::asmEndInterview() {
	debug(0, "Interactive Interview copyright (c) 1995, IBI.");
	OSystem::instance()->quit();
}

void Logic::startCredits(const char *filename) {

	stopCredits();
	_credits = new Credits(_vm, filename);
}

void Logic::stopCredits() {
	if (_credits) {
		delete _credits;
		_credits = NULL;
	}
}


bool LogicDemo::preChangeRoom() {
	if (currentRoom() == FOTAQ_LOGO && gameState(VAR_INTRO_PLAYED) == 0) {
		currentRoom(79);
		roomDisplay(currentRoom(), RDM_FADE_NOJOE, 100, 2, true);
		playCutaway("clogo.cut");
		sceneReset();
		currentRoom(ROOM_HOTEL_LOBBY);
		entryObj(584);
		roomDisplay(currentRoom(), RDM_FADE_JOE, 100, 2, true);
		playCutaway("c70d.cut");
		gameState(VAR_INTRO_PLAYED, 1);
		inventorySetup();
		inventoryRefresh();
		return true;
	}
	return false;
}


bool LogicInterview::preChangeRoom() {
	if (currentRoom() == 2 && gameState(2) == 0) {
		currentRoom(6);
		roomDisplay(currentRoom(), RDM_FADE_NOJOE, 100, 2, true);
		playCutaway("start.cut");
		gameState(2, 1);
		inventorySetup();
		inventoryRefresh();
		return true;
	}
	return false;
}


bool LogicGame::preChangeRoom() {
	if (currentRoom() == ROOM_JUNGLE_PINNACLE) {
		handlePinnacleRoom();
		return true;
	} else if (currentRoom() == FOTAQ_LOGO && gameState(VAR_INTRO_PLAYED) == 0) {
		roomDisplay(currentRoom(), RDM_FADE_NOJOE, 100, 2, true);
		playCutaway("copy.cut");
		playCutaway("clogo.cut");

		// XXX enable talking for talkie version

		if (ConfMan.getBool("alt_intro")) {
			playCutaway("cintr.cut");
		} else {
			playCutaway("cdint.cut");
			_vm->display()->palSetPanel();
		}

		playCutaway("cred.cut");
		sceneReset();
		currentRoom(ROOM_HOTEL_LOBBY);
		entryObj(584);
		roomDisplay(currentRoom(), RDM_FADE_JOE, 100, 2, true);
		playCutaway("c70d.cut");
		gameState(VAR_INTRO_PLAYED, 1);
		inventorySetup();
		inventoryRefresh();
		return true;
	}
	return false;
}


bool LogicDemo::handleSpecialMove(uint16 sm) {
	switch (sm) {
	case 4:
		asmMakeJoeUseUnderwear();
		break;
	case 5:
		asmSwitchToDressPalette();
		break;
	case 14:
		asmEndDemo();
		break;
	default:
		return false;
	}
	return true;
}


bool LogicInterview::handleSpecialMove(uint16 sm) {
	switch (sm) {
	case 1:
		asmInterviewIntro();
		break;
	case 2:
		asmEndInterview();
		break;
	default:
		return false;
	}
	return true;
}


bool LogicGame::handleSpecialMove(uint16 sm) {
	typedef void (Logic::*SpecialMoveProc)();
	static const SpecialMoveProc asmTable[] = {
		/* 00 */
		0,
		0,
		&Logic::asmMakeJoeUseDress,
		&Logic::asmMakeJoeUseNormalClothes,
		/* 04 */
		&Logic::asmMakeJoeUseUnderwear,
		&Logic::asmSwitchToDressPalette,
		&Logic::asmSwitchToNormalPalette,
		&Logic::asmStartCarAnimation,       // room 74
		/* 08 */
		&Logic::asmStopCarAnimation,        // room 74
		&Logic::asmStartFightAnimation,     // room 69
		&Logic::asmWaitForFrankPosition,    // c69e.cut
		&Logic::asmMakeFrankGrowing,        // c69z.cut
		/* 12 */
		&Logic::asmMakeRobotGrowing,        // c69z.cut
		&Logic::asmShrinkRobot,
		&Logic::asmEndGame,
		&Logic::asmPutCameraOnDino,
		/* 16 */
		&Logic::asmPutCameraOnJoe,
		&Logic::asmAltIntroPanRight,        // cintr.cut
		&Logic::asmAltIntroPanLeft,         // cintr.cut
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
		0,
		/* 36 */
		&Logic::asmPanRightToHugh,
		&Logic::asmMakeWhiteFlash,
		&Logic::asmPanRightToJoeAndRita,
		&Logic::asmPanLeftToBomb            // cdint.cut
	};
	if (sm >= ARRAYSIZE(asmTable) || asmTable[sm] == 0)
		return false;
	(this->*asmTable[sm])();
	return true;
}


} // End of namespace Queen

