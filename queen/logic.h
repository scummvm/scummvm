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
#include "queen/defs.h"
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

struct Command_ {
	Verb action, action2;
	uint16 noun, noun2;
};

struct GameSettings {
	int musicVolume;
	bool musicToggle;
	bool sfxToggle;
	bool textToggle;
	bool speechToggle;
	int talkSpeed;
};

struct State {

	//! FIND_STATE(state, "DIR");
	static Direction findDirection(uint16 state);

	//! FIND_STATE(state, "TALK");
	static StateTalk findTalk(uint16 state);

	//! FIND_STATE(state, "GRAB");
	static StateGrab findGrab(uint16 state);

	//! FIND_STATE(state, "ON");
	static StateOn findOn(uint16 state);

	//! FIND_STATE(state, "DEF");
	static Verb findDefaultVerb(uint16 state);

	static StateUse findUse(uint16 state);

	//! ALTER_STATE(state, "ON");
	static void alterOn(uint16 *objState, StateOn state);

	//! ALTER_STATE(state, verb);
	static void alterDefaultVerb(uint16 *objState, Verb v);
};


class Graphics;
class Resource;
class Display;
class Input;
class Sound;
class Walk;

class Logic {

public:
	Logic(Resource *resource, Graphics *graphics, Display *display, Input *input, Sound *sound);
	~Logic();

	uint16 currentRoom() const { return _currentRoom; }
	void currentRoom(uint16 room) { _currentRoom = room; }
	
	uint16 oldRoom() const { return _oldRoom; }
	void oldRoom(uint16 room) { _oldRoom = room; }
	
	uint16 newRoom() const { return _newRoom; }
	void newRoom(uint16 room) { _newRoom = room; }

	ObjectData *objectData(int index);
	uint16 roomData(int room);
	uint16 objMax(int room);
	GraphicData *graphicData(int index);

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
	uint16 currentRoomObjMax() const { return _objMax[_currentRoom]; }

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

	int talkSpeed() const { return _settings.talkSpeed; }
	Language language()	{ return _resource->getLanguage(); } 

	void zoneSet(uint16 screen, uint16 zone, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void zoneSet(uint16 screen, uint16 zone, const Box& box);
	uint16 zoneIn(uint16 screen, uint16 x, uint16 y) const;
	uint16 zoneInArea(uint16 screen, uint16 x, uint16 y) const;
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
	void animSetup(const GraphicData *gd, uint16 firstImage, uint16 bobNum, bool visible); // FIND_GRAPHIC_ANIMS

	void joeSetupFromBanks(const char *animBank, const char *standBank);

	//! SETUP_JOE(), loads the various bobs needed to animate Joe
	void joeSetup();

	//! SETUP_HERO(), places Joe at the right place when entering a room
	ObjectData *joeSetupInRoom(bool autoPosition, uint16 scale);
	
	//! FACE_JOE()
	uint16 joeFace();

	//! WALK()
	int16 joeWalkTo(int16 x, int16 y, const Command_ *cmd, bool mustWalk);

	//! GRAB_JOE()
	void joeGrab(uint16 state, uint16 speed);

	//! GRAB_DIR
	void joeGrabDirection(StateGrab grab, uint16 speed);

	//! USE_DRESS
	void joeUseDress(bool showCut);

	//! USE_CLOTHES
	void joeUseClothes(bool showCut);

	//! USE_UNDERWEAR
	void joeUseUnderwear();

	void playCutaway(const char* cutFile);

	const char* objectOrItemName(int16 obj) const;

	//! return selected verb in panel
	Verb findVerb(int16 cursorx, int16 cursory) const;

	Walk *walk() { return _walk; }
	Display *display() { return _display; }

	uint16 findObjectFromZone(uint16 zoneNum);

	const char *verbName(Verb v) const;

	void update();

protected:

	GameSettings _settings;

	void initialise();

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
	
	//! Number of animated furniture in current room (FMAXA)
	uint16 _numFurnitureAnimated;

	//! Number of static furniture in current room (FMAX)
	uint16 _numFurnitureStatic;

	//! Total number of frames for the animated furniture (FMAXLEN)
	uint16 _numFurnitureAnimatedLen;

	//! Current number of frames unpacked (FRAMES)
	uint16 _numFrames;

	//! Last frame number used for person animation
	uint16 _personFrames[4];

	//! Describe a string based animation (30 frames maximum, bob number must be < 17)
	AnimFrame _newAnim[17][30];

	Resource *_resource;
	Graphics *_graphics;
	Display *_display;
	Input *_input;
	Sound *_sound;
	Walk *_walk;

	//! Verbs (in order) available in panel
	static const Verb PANEL_VERBS[];

	friend class Command; // TEMP
};

} // End of namespace Queen

#endif
