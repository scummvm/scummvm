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

#ifndef QUEENLOGIC_H
#define QUEENLOGIC_H

#include "common/util.h"
#include "queen/defs.h"
#include "queen/structs.h"

namespace Queen {

enum RoomDisplayMode {
	RDM_FADE_NOJOE  = 0, // fade in, no Joe
	RDM_FADE_JOE    = 1, // Joe is to be displayed
	RDM_NOFADE_JOE  = 2, // screen does not dissolve into view
	RDM_FADE_JOE_XY = 3  // display Joe at the current X, Y coords
};

enum {
	ZONE_ROOM  = 0,
	ZONE_PANEL = 1
};

enum JoeWalkMode {
	JWM_NORMAL  = 0,
	JWM_MOVE    = 1,
	JWM_EXECUTE = 2,
	JWM_SPEAK   = 3
};

struct ZoneSlot {
	bool valid;
	Box box;
};

class QueenEngine;

class Logic {

public:
	Logic(QueenEngine *vm);

	uint16 currentRoom() const { return _currentRoom; }
	void currentRoom(uint16 room) { 
		if (room >= 1 && room <= _numRooms)
			_currentRoom = room; 
		else
			error("Invalid room number: %i", room);
	}

	uint16 oldRoom() const { return _oldRoom; }
	void oldRoom(uint16 room) { 
		if (room <= _numRooms)
			_oldRoom = room; 
		else
			error("Invalid room number: %i", room);
	}

	uint16 newRoom() const { return _newRoom; }
	void newRoom(uint16 room) { 
		if (room <= _numRooms)
			_newRoom = room; 
		else
			error("Invalid room number: %i", room);
	}

	ObjectData *objectData(int index) const;
	uint16 roomData(int room) const { return _roomData[room]; }
	uint16 objMax(int room) const { return _objMax[room]; } 
	GraphicData *graphicData(int index) const { return &_graphicData[index]; }
	ItemData *itemData(int index) const { return &_itemData[index]; }
	uint16 itemDataCount() const { return _numItems; }

	uint16 findBob(uint16 obj);
	uint16 findFrame(uint16 obj);
	uint16 objectForPerson(uint16 bobnum) const;
	WalkOffData *walkOffPointForObject(uint16 obj) const;

	Area *area(int room, int num) const { return &_area[room][num]; }
	Area *currentRoomArea(int num) const;
	uint16 areaMax(int room) const { return _areaMax[room]; }
	uint16 currentRoomAreaMax() const { return _areaMax[_currentRoom]; }
	uint16 walkOffCount() const { return _numWalkOffs; }
	WalkOffData *walkOffData(int index) const { return &_walkOffData[index]; }
	uint16 currentRoomObjMax() const { return _objMax[_currentRoom]; }
	uint16 currentRoomData() const { return _roomData[_currentRoom]; }
	ObjectDescription *objectDescription(uint16 objNum) const { return &_objectDescription[objNum]; }
	uint16 objectDescriptionCount() const { return _numObjDesc; }
	uint16 currentRoomSfx() const { return _sfxName[_currentRoom]; }

	uint16 joeFacing() const { return _joe.facing; }
	uint16 joeX() const { return _joe.x; }
	uint16 joeY() const { return _joe.y; }
	JoeWalkMode joeWalk() const { return _joe.walk; }
	uint16 joeScale() const { return _joe.scale; }
	uint16 joeCutFacing() const { return _joe.cutFacing; }
	uint16 joePrevFacing() const { return _joe.prevFacing; }

	void joeFacing(uint16 dir) { _joe.facing = dir; }
	void joePos(uint16 x, uint16 y) { _joe.x = x; _joe.y = y; }
	void joeWalk(JoeWalkMode walking);
	void joeScale(uint16 scale) { _joe.scale = scale; }
	void joeCutFacing(uint16 dir) { _joe.cutFacing = dir; }
	void joePrevFacing(uint16 dir) { _joe.prevFacing = dir; }
	
	const char *joeResponse(int i) const { return _joeResponse[i]; }
	const char *verbName(Verb v) const { return _verbName[v]; }

	int16 gameState(int index);
	void gameState(int index, int16 newValue);

	TalkSelected *talkSelected(int index) { return _talkSelected + index; }

	const char *roomName(uint16 roomNum) const { 
		if (roomNum >= 1 && roomNum <= _numRooms)
			return _roomName[roomNum];
		else
			error("Invalid room number: %i", roomNum);
	}

	const char *objectName(uint16 objNum) const { return _objName[objNum]; }
	const char *objectTextualDescription(uint16 objNum) const { return _objDescription[objNum]; }

	void zoneSet(uint16 screen, uint16 zone, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void zoneSet(uint16 screen, uint16 zone, const Box& box);
	uint16 zoneIn(uint16 screen, uint16 x, uint16 y) const;
	uint16 zoneInArea(uint16 screen, uint16 x, uint16 y) const;
	void zoneClearAll(uint16 screen);
	void zoneSetup();
	void zoneSetupPanel();
	Box &zoneBox(uint16 screen, uint16 index) { return _zones[screen][index].box; } 

	void roomErase();
	void roomSetupFurniture();
	void roomSetupObjects();
	uint16 roomRefreshObject(uint16 obj);
	void roomSetup(const char *room, int comPanel, bool inCutaway);
	void roomDisplay(uint16 room, RoomDisplayMode mode, uint16 joeScale, int comPanel, bool inCutaway);

	uint16 findScale(uint16 x, uint16 y);

	int16 entryObj() const { return _entryObj; }
	void entryObj(int16 obj) { _entryObj = obj; }

	uint16 numFrames() const { return _numFrames; }

	void personSetData(int16 noun, const char *actorName, bool loadBank, Person *pp);
	uint16 personSetup(uint16 noun, uint16 curImage);
	uint16 personAllocate(uint16 noun, uint16 curImage);
	uint16 personFrames(uint16 bobNum) const { return _personFrames[bobNum]; }

	uint16 animCreate(uint16 curImage, const Person *person);
	void animErase(uint16 bobNum);
	void animReset(uint16 bobNum);
	void animSetup(const GraphicData *gd, uint16 firstImage, uint16 bobNum, bool visible);

	void joeSetupFromBanks(const char *animBank, const char *standBank);

	//! Load the various bobs needed to animate Joe
	void joeSetup();

	//! Setup Joe at the right place when entering a room
	ObjectData *joeSetupInRoom(bool autoPosition, uint16 scale);
	
	uint16 joeFace();
	void joeGrab(int16 grabState);

	void joeUseDress(bool showCut);
	void joeUseClothes(bool showCut);
	void joeUseUnderwear();

	void joeSpeak(uint16 descNum, bool objectType = false);

	void makePersonSpeak(const char *sentence, Person *person, const char *voiceFilePrefix);
	void dialogue(const char *dlgFile, int personInRoom, char *cutaway);
	void playCutaway(const char *cutFile, char *next = NULL);

	Verb findVerbUnderCursor(int16 cursorx, int16 cursory) const;
	uint16 findObjectUnderCursor(int16 cursorx, int16 cursory) const;
	uint16 findObjectNumber(uint16 zoneNum) const;

	void inventorySetup();
	uint16 findInventoryItem(int invSlot) const;
	void inventoryRefresh();
	int16 previousInventoryItem(int16 start) const;
	int16 nextInventoryItem(int16 start) const;
	void removeDuplicateItems();
	uint16 numItemsInventory() const;
	void inventoryInsertItem(uint16 itemNum, bool refresh = true);
	void inventoryDeleteItem(uint16 itemNum, bool refresh = true);
	void inventoryScroll(uint16 count, bool up);

	//! Copy data from dummy object to object
	void objectCopy(int dummyObjectIndex, int objectIndex);

	void checkPlayer();

	void customMoveJoe(int facing, uint16 areaNum, uint16 walkDataNum);

	void handlePinnacleRoom();

	void update();

	bool gameSave(uint16 slot, const char *desc);
	bool gameLoad(uint16 slot);

	//! Ugly hack from original code
	void sceneReset() { _scene = 0; }

	//! Make a scene
	void sceneStart();

	//! Stop making a scene
	void sceneStop();

	void changeRoom();

	void useJournal();

	int talkSpeed() const { return _talkSpeed; }
	void talkSpeed(int speed) { _talkSpeed = speed; }
	bool subtitles() const { return _subtitles; }
	void subtitles(bool enable) { _subtitles = enable; }

	void registerDefaultSettings();
	void checkOptionSettings();
	void readOptionSettings();
	void writeOptionSettings();

	bool preChangeRoom_Demo();
	bool preChangeRoom_Interview();
	bool preChangeRoom_Game();

	bool executeSpecialMove_Demo(uint16 sm);
	bool executeSpecialMove_Interview(uint16 sm);
	bool executeSpecialMove_Game(uint16 sm);
	void executeSpecialMove(uint16 sm);

	void asmMakeJoeUseDress();
	void asmMakeJoeUseNormalClothes();
	void asmMakeJoeUseUnderwear();
	void asmSwitchToDressPalette();
	void asmSwitchToNormalPalette();
	void asmStartCarAnimation();
	void asmStopCarAnimation();
	void asmStartFightAnimation();
	void asmWaitForFrankPosition();
	void asmMakeFrankGrowing();
	void asmMakeRobotGrowing();
	void asmShrinkRobot();
	void asmEndGame();
	void asmPutCameraOnDino();
	void asmPutCameraOnJoe();
	void asmAltIntroPanRight();
	void asmAltIntroPanLeft();
	void asmSetAzuraInLove();
	void asmPanRightFromJoe();
	void asmSetLightsOff();
	void asmSetLightsOn();
	void asmSetManequinAreaOn();
	void asmPanToJoe();
	void asmTurnGuardOn();
	void asmPanLeft320To144();
	void asmSmooch();
	void asmMakeLightningHitPlane();
	void asmScaleBlimp();
	void asmScaleEnding();
	void asmWaitForCarPosition();
	void asmShakeScreen();
	void asmAttemptPuzzle();
	void asmScaleTitle();
	void asmPanRightToHugh();
	void asmMakeWhiteFlash();
	void asmPanRightToJoeAndRita();
	void asmPanLeftToBomb();
	void asmEndDemo();
	void asmInterviewIntro();
	void asmEndInterview();

	typedef bool (Logic::*ExecuteSpecialMoveProc)(uint16);
	typedef bool (Logic::*PreChangeRoomProc)();

	enum {
		MAX_ZONES_NUMBER    = 32,
		MAX_AREAS_NUMBER    = 11,
		JOE_RESPONSE_MAX    = 40,
		DEFAULT_TALK_SPEED  = 7 * 3,
		GAME_STATE_COUNT    = 211,
		TALK_SELECTED_COUNT = 86
	};

protected:

	void initialise();

	uint16 _currentRoom;
	uint16 _oldRoom;
	uint16 _newRoom;

	//! Total number of room in game
	uint16 _numRooms;

	//! First object number in room
	uint16 *_roomData;

	//! Background music to play in room
	uint16 *_sfxName;

	//! Number of objects in room
	int16 *_objMax;

	//! Bounding box of object
	Box *_objectBox;

	//! Inventory items
	ItemData *_itemData;
	uint16 _numItems;

	GraphicData *_graphicData;
	uint16 _numGraphics;

	ObjectData *_objectData;
	uint16 _numObjects;

	ObjectDescription *_objectDescription;
	uint16 _numObjDesc;

	ActorData *_actorData;
	uint16 _numActors;

	//! Areas in room
	Area (*_area)[MAX_AREAS_NUMBER];

	//! Number of areas in room
	int16 *_areaMax;

	//! Walk off point for an object
	WalkOffData *_walkOffData;
	uint16 _numWalkOffs;

	FurnitureData *_furnitureData;
	uint16 _numFurniture;
	
	GraphicAnim *_graphicAnim;
	uint16 _numGraphicAnim;

	//! Current areas in room
	ZoneSlot _zones[2][MAX_ZONES_NUMBER];

	//! Actor position in room is _walkOffData[_entryObj]
	int16 _entryObj;

	//! Object description (Look At)
	char **_objDescription;
	uint16 _numDescriptions;

	char **_objName;
	uint16 _numNames;

	//! Room name, prefix for data files (PCX, LUM...)
	char **_roomName;

	char *_verbName[13];

	char *_joeResponse[JOE_RESPONSE_MAX + 1];

	//! Actor animation string
	char **_aAnim;
	uint16 _numAAnim;

	//! Actor name
	char **_aName;
	uint16 _numAName;

	//! Actor filename
	char **_aFile;
	uint16 _numAFile;

	struct {
		uint16 x, y;
		uint16 facing, cutFacing, prevFacing;
		JoeWalkMode walk;
		uint16 scale;
	} _joe;
	
	int16 _gameState[GAME_STATE_COUNT];

	TalkSelected _talkSelected[TALK_SELECTED_COUNT];
	
	//! Number of animated furniture in current room
	uint16 _numFurnitureAnimated;

	//! Number of static furniture in current room
	uint16 _numFurnitureStatic;

	//! Total number of frames for the animated furniture
	uint16 _numFurnitureAnimatedLen;

	//! Current number of frames unpacked
	uint16 _numFrames;

	//! Last frame number used for person animation
	uint16 _personFrames[4];

	//! Describe a string based animation (30 frames maximum, bob number must be < 17)
	AnimFrame _newAnim[17][30];

	//! Inventory items
	int16 _inventoryItem[4];

	//! scene counter
	int _scene;

	int _talkSpeed;

	bool _subtitles;

	ExecuteSpecialMoveProc _executeSpecialMove;
	PreChangeRoomProc _preChangeRoom;

	QueenEngine *_vm;
};


} // End of namespace Queen

#endif
