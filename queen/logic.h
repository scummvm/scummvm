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

#include "queen/queen.h"
#include "queen/structs.h"

namespace Queen {

#define MAX_ZONES_NUMBER	32
#define MAX_AREAS_NUMBER    11
#define JOE_RESPONSE_MAX	40

enum RoomDisplayMode {
	RDM_FADE_NOJOE  = 0, // fade in, no Joe
	RDM_FADE_JOE    = 1, // Joe is to be displayed
	RDM_NOFADE_JOE  = 2, // screen does not dissolve into view
	RDM_FADE_JOE_XY = 3  // display Joe at the current X, Y coords
};

struct ZoneSlot {
	bool valid;
	Box box;
};


class Graphics;
class Resource;
class Display;
class Walk;

class Logic {

public:
	Logic(Resource *resource, Graphics *graphics, Display *display);
	~Logic();

	uint16 currentRoom();
	void currentRoom(uint16 room);
	
	uint16 oldRoom()              { return _oldRoom; }
	void oldRoom(uint16 room);

	uint16 newRoom()              { return _newRoom; }
	void newRoom(uint16 room)     { _newRoom = room; }

	ObjectData* objectData(int index);
	uint16 roomData(int room);
	uint16 objMax(int room);
	GraphicData* graphicData(int index);

	uint16 findBob(uint16 obj);
	uint16 findFrame(uint16 obj);
	uint16 objectForPerson(uint16 bobnum); // OBJ_PERSON
	WalkOffData *walkOffPointForObject(uint16 obj);

	Area *area(int room, int num);
	Area *currentRoomArea(int num);
	uint16 areaMax(int room);
	uint16 currentRoomAreaMax();
	uint16 walkOffCount();
	WalkOffData *walkOffData(int index);

	uint16 joeFacing()	{ return _joe.facing; }
	uint16 joeX()		{ return _joe.x; }
	uint16 joeY()		{ return _joe.y; }
	uint16 joeWalk()	{ return _joe.walk; }
	uint16 joeScale()	{ return _joe.scale; }
	uint16 joePrevFacing() { return _joe.prevFacing; }

	void joeFacing(uint16 dir);
	void joeX(uint16 x);
	void joeY(uint16 y);
	void joeWalk(uint16 walking);
	void joeScale(uint16 scale);
	void joePrevFacing(uint16 dir);
	
	int16 gameState(int index);
	void gameState(int index, int16 newValue);

	const char *roomName(uint16 roomNum)	{ return _roomName[roomNum] ; }
	const char *objectName(uint16 objNum)	{ return _objName[objNum]; }

	uint16 numFrames() { return _numFrames; }

	Language language()	{ return _resource->getLanguage(); } 

	void zoneSet(uint16 screen, uint16 zone, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void zoneSet(uint16 screen, uint16 zone, const Box& box);
	uint16 zoneIn(uint16 screen, uint16 x, uint16 y);
	uint16 zoneInArea(uint16 screen, uint16 x, uint16 y);
	void zoneClearAll(uint16 screen);
	void zoneSetup();
	void zoneSetupPanel();

	void roomErase();
	void roomSetupFurniture(); // SETUP_FURNITURE()
	void roomSetupObjects(); // DISP_OBJECTS
	uint16 roomRefreshObject(uint16 obj); // REDISP_OBJECT
	void roomSetup(const char* room, int comPanel, bool inCutaway);
	void roomDisplay(const char* room, RoomDisplayMode mode, uint16 joeScale, int comPanel, bool inCutaway); // DISP_ROOM

	uint16 findScale(uint16 x, uint16 y);

	int16 entryObj() const { return _entryObj; }
	void entryObj(int16 obj) { _entryObj = obj; }

	void personSetData(int16 noun, const char *actorName, bool loadBank, Person *pp); // SET_PERSON_DATA
	uint16 personSetup(uint16 noun, uint16 curImage); // SETUP_PERSON
	uint16 personAllocate(uint16 noun, uint16 curImage); // ALLOCATE_PERSON

	uint16 animCreate(uint16 curImage, const Person *person); // CREATE_ANIM
	void animErase(uint16 bobNum);
	int16 animFindAll(const GraphicData *gd, uint16 firstImage, AnimFrame *paf); // FIND_GRAPHIC_ANIMS

	StateDirection findStateDirection(uint16 state); // == FIND_STATE(state, "DIR");
	StateTalk      findStateTalk     (uint16 state); // == FIND_STATE(state, "TALK");

	Walk *walk()	{ return _walk; }

	int talkSpeed() { return _talkSpeed; }

	//! SETUP_JOE(), loads the various bobs needed to animate Joe
	void joeSetup();

	//! SETUP_HERO(), places Joe at the right place when entering a room
	ObjectData *joeSetupInRoom(bool autoPosition, uint16 scale);
	
	//! FACE_JOE()
	uint16 joeFace();

protected:
	bool _textToggle;
	bool _speechToggle;
	
	uint8 *_jas;
	uint16 _numRooms;
	uint16 _currentRoom;
	uint16 _oldRoom;	
	uint16 _newRoom;	
	uint16 _numNames;
	uint16 _numObjects;
	uint16 _numDescriptions;
	uint16 _numItems;
	uint16 _numGraphics;

	uint16 _numWalkOffs;
	uint16 _numObjDesc;
	uint16 _numCmdList;	//COM_LIST_MAX
	uint16 _numCmdArea;	//COM_A_MAX
	uint16 _numCmdObject;	//COM_O_MAX
	uint16 _numCmdInventory;	//COM_I_MAX
	uint16 _numCmdGameState;	//COM_G_MAX
	uint16 _numFurniture;	//FURN_DATA_MAX
	uint16 _numActors;	//ACTOR_DATA_MAX
	uint16 _numAAnim;	//A_ANIM_MAX
	uint16 _numAName;	//A_NAME_MAX
	uint16 _numAFile;	//A_FILE_MAX
	uint16 _numGraphicAnim;	//GRAPHIC_ANIM_MAX

	uint16 *_roomData;
	uint16 *_sfxName;
	int16 *_objMax;
	int16 *_areaMax;
	Box *_objectBox;
	ItemData *_itemData;
	GraphicData *_graphicData;
	ObjectData *_objectData;
	ObjectDescription *_objectDescription;
	ActorData *_actorData;
	Area (*_area)[MAX_AREAS_NUMBER];
	WalkOffData *_walkOffData;
	CmdListData *_cmdList;
	CmdArea *_cmdArea;
	CmdObject *_cmdObject;
	CmdInventory *_cmdInventory;	
	CmdGameState *_cmdGameState;
	FurnitureData *_furnitureData;
	GraphicAnim *_graphicAnim;
	ZoneSlot _zones[2][MAX_ZONES_NUMBER];
	uint16 _entryObj;

	char **_objDescription;	//OBJECT_DESCRstr
	char **_objName;	//OBJECT_NAMEstr
	char **_roomName;	//ROOM_NAMEstr	
	char *_verbName[13];	//VERB_NAMEstr
	char *_joeResponse[JOE_RESPONSE_MAX + 1];	//JOE_RESPstr
	char **_aAnim;	//A_ANIMstr
	char **_aName;	//A_NAMEstr
	char **_aFile;	//A_FILEstr

	enum {
		DEFAULT_TALK_SPEED = 7,
		GAME_STATE_COUNT = 211
	};

	struct {
		uint16	x, y;
		uint16	facing, prevFacing;
		uint16  walk;
		uint16  scale;
	} _joe;
	
	int16 _gameState[GAME_STATE_COUNT];
	
	uint16 _numFurnitureAnimated; // FMAXA
	uint16 _numFurnitureStatic; // FMAX
	uint16 _numFurnitureAnimatedLen; // FMAXLEN
	uint16 _numFrames; // FRAMES
	uint16 _personFrames[4];

	//! contains the animation frames (max 30) to use for a bob (whose number must be < 17)
	AnimFrame _newAnim[17][30];

	Resource *_resource;
	Graphics *_graphics;
	Display *_display;
	Walk *_walk;

	int _talkSpeed;	// TALKSPD

	void initialise();
};

} // End of namespace Queen

#endif
