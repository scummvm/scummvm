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

#define MAX_ZONES_NUMBER 32


enum Language {
	ENGLISH  = 'E',
	FRENCH   = 'F',
	GERMAN   = 'G',
	ITALIAN  = 'I'
};

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
class Walk;

class Logic {

public:
	Logic(Resource *resource, Graphics *graphics);
	~Logic();

	uint16 currentRoom();
	void currentRoom(uint16 room);
	void oldRoom(uint16 room);
	ObjectData* objectData(int index);
	uint16 roomData(int room);
	uint16 objMax(int room);
	GraphicData* graphicData(int index);

	uint16 findBob(uint16 obj);
	uint16 findFrame(uint16 obj);
	uint16 objectForPerson(uint16 bobnum);
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

	void joeFacing(uint16 dir);
	void joeX(uint16 x);
	void joeY(uint16 y);
	void joeWalk(uint16 walk);
	void joeScale(uint16 scale);
	
	int16 gameState(int index);
	void gameState(int index, int16 newValue);

	Language language() { return ENGLISH; } // FIXME: get from queen.jas

	void zoneSet(uint16 screen, uint16 zone, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void zoneSet(uint16 screen, uint16 zone, const Box& box);
	uint16 zoneIn(uint16 screen, uint16 x, uint16 y);
	uint16 zoneInArea(uint16 screen, uint16 x, uint16 y);
	void zoneClearAll(uint16 screen);
	void zoneSetup();

	void roomErase();
	void roomSetupFurniture(); // SETUP_FURNITURE()
	void roomSetupObjects(); // DISP_OBJECTS
	void roomSetup(const char* room, int comPanel, bool inCutaway);
	void roomDisplay(const char* room, RoomDisplayMode mode, uint16 joeScale, int comPanel, bool inCutaway); // DISP_ROOM

	uint16 findScale(uint16 x, uint16 y);

	int16 entryObj() const { return _entryObj; }
	void entryObj(int16 obj) { _entryObj = obj; }


protected:
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

	uint16 _numObjectBoxes;
	uint16 _numWalkOffs;
	uint16 _numObjDesc;

	uint16 *_roomData;
	uint16 *_sfxName;
	int16 *_objMax;
	int16 *_areaMax;
	Box *_objectBox;
	ItemData *_itemData;
	GraphicData *_graphicData;
	ObjectData *_objectData;
	ObjectDescription *_objectDescription;
	uint16 (*_actorData)[12]; // FIXME: ActorData *_actorData;
	Area (*_area)[11];
	WalkOffData *_walkOffData;
	ZoneSlot _zones[2][MAX_ZONES_NUMBER];
	uint16 _entryObj;

	enum {
		GAME_STATE_COUNT = 211
	};

	struct {
		uint16	x, y;
		uint16	facing;
		uint16  walk;
		uint16  scale;
	} _joe;
	
	int16 _gameState[GAME_STATE_COUNT];
	
	uint16 _numFurnitureAnimated; // FMAXA
	uint16 _numFurnitureStatic; // FMAX
	uint16 _numFurnitureAnimatedLen; // FMAXLEN
	uint16 _numFrames; // FRAMES

	Resource *_resource;
	Graphics *_graphics;
	Walk *_walk;

	void initialise();
};

} // End of namespace Queen

#endif
