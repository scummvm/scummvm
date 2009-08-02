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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef DRACI_GAME_H
#define DRACI_GAME_H

#include "common/str.h"
#include "draci/script.h"
#include "draci/animation.h"
#include "draci/sprite.h"

namespace Draci {

class DraciEngine;

enum {
	kDragonObject = 0
};

enum StructSizes {
	personSize = sizeof(uint16) * 2 + sizeof(byte)
};

enum {
	kNotFound = -1
};

enum SpeechConstants {
	kBaseSpeechDuration = 200,
	kSpeechTimeUnit = 400
};

class WalkingMap {

public:	
	WalkingMap() {
		_realWidth = 0;
		_realHeight = 0;
		_mapWidth = 0;
		_mapHeight = 0;
		_byteWidth = 0;
		_data = NULL;
	}	

	void load(byte *data, uint length) {
		Common::MemoryReadStream mapReader(data, length);

		_realWidth = mapReader.readUint16LE();
		_realHeight = mapReader.readUint16LE();
		_deltaX = mapReader.readUint16LE();
		_deltaY = mapReader.readUint16LE();
		_mapWidth = mapReader.readUint16LE();
		_mapHeight = mapReader.readUint16LE();
		_byteWidth = mapReader.readUint16LE();

		// Set the data pointer to raw map data
		_data = data + mapReader.pos();
	}

	bool isWalkable(int x, int y);

private:
	int _realWidth, _realHeight;
	int _deltaX, _deltaY;
	int _mapWidth, _mapHeight;
	int _byteWidth;
	byte *_data;
};

struct GameObject {
	
	uint _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	byte _walkDir;
	byte _z;
	uint16 _lookX, _lookY, _useX, _useY;
	byte _lookDir, _useDir;
	uint16 _absNum;
	Common::Array<int> _anims;
	GPL2Program _program;
	Common::String _title;
	int _location;
	bool _visible;
};

struct GameInfo {
	byte _startRoom;
	byte _mapRoom;
	uint16 _numObjects;
	uint16 _numIcons;
	byte _numVariables;
	byte _numPersons;
	byte _numDialogs;
	uint16 _maxIconWidth, _maxIconHeight;
	uint16 _musicLength;
	uint16 _crc[4];
	uint16 _numDialogBlocks;
};

struct Person {
	uint16 _x, _y;
	byte _fontColour;
};

struct Room {
	byte _roomNum;	
	byte _music;
	WalkingMap _walkingMap;
	byte _palette;
	int _numOverlays;
	int _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	bool _mouseOn, _heroOn;
	double _pers0, _persStep;
	byte _escRoom;
	byte _numGates;
	Common::Array<int> _gates;
	GPL2Program _program;
};

enum LoopStatus { 
	kStatusGate, kStatusOrdinary, kStatusInventory, 
	kStatusDialogue, kStatusTalk, kStatusStrange, 
	kStatusFade
};

/**
  * Enumerates the animations for the dragon's movement.
  */

enum Movement {
	kMoveUndefined, kMoveDown, kMoveUp, kMoveRight, kMoveLeft,
	kMoveRightDown, kMoveRightUp, kMoveLeftDown, kMoveLeftUp,
	kMoveDownRight, kMoveUpRight, kMoveDownLeft, kMoveUpLeft,
	kMoveLeftRight, kMoveRightLeft, kMoveUpStopLeft, kMoveUpStopRight,
	kSpeakRight, kSpeakLeft, kStopRight, kStopLeft
};

class Game {

public:

	Game(DraciEngine *vm);
	~Game();

	void init();
	void start();
	void loop();

	void changeRoom(uint roomNum);

	// HACK: this is only for testing
	int nextRoomNum() { 
		int n = _currentRoom._roomNum;
		n = n < 37 ? n+1 : n;

		// disable former distributor logo
		if (n == 30)
			++n;

		return n;
	}

	// HACK: same as above
	int prevRoomNum() { 
		int n = _currentRoom._roomNum;
		n = n > 0 ? n-1 : n;

		// disable former distributor logo
		if (n == 30)
			--n;

		return n;
	}

	void walkHero(int x, int y);

	void loadRoom(int roomNum);
	int loadAnimation(uint animNum, uint z);
	void loadOverlays();
	void loadObject(uint numObj);

	uint getNumObjects();
	GameObject *getObject(uint objNum);
	int getObjectWithAnimation(int animID);

	int getVariable(int varNum);
	void setVariable(int varNum, int value);	

	Person *getPerson(int personID);

	int getRoomNum();
	void setRoomNum(int room);

	int getGateNum();
	void setGateNum(int gate);

	int getIconStatus(int iconID);

	int getMarkedAnimationIndex();
	void setMarkedAnimationIndex(int index);

	void setLoopStatus(LoopStatus status);
	void setLoopSubstatus(LoopStatus status);
	LoopStatus getLoopStatus();
	LoopStatus getLoopSubstatus();

	bool shouldQuit() { return _shouldQuit; }
	void setQuit(bool quit) { _shouldQuit = quit; }

	bool shouldExitLoop() { return _shouldExitLoop; }
	void setExitLoop(bool exit) { _shouldExitLoop = exit; }

	void runGateProgram(int gate);

	void setSpeechTick(uint tick);

	bool _roomChange;

private:
	DraciEngine *_vm;

	GameInfo _info;
	uint *_dialogOffsets;

	int *_variables;
	byte *_iconStatus;
	Person *_persons;
	GameObject *_objects;

	Room _currentRoom;
	int _currentGate;	
	int _newRoom;
	int _newGate;

	LoopStatus _loopStatus;
	LoopStatus _loopSubstatus;

	bool _shouldQuit;
	bool _shouldExitLoop;

	uint _speechTick;

	int _objUnderCursor;
	int _markedAnimationIndex; //!< Used by the Mark GPL command
};

} // End of namespace Draci

#endif // DRACI_GAME_H
