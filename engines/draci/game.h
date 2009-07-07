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
	kDragonObject
};

enum StructSizes {
	personSize = sizeof(uint16) * 2 + sizeof(byte)
};

struct GameObject {
	
	GameObject() : _title(NULL) {}
	~GameObject();
		
	uint16 _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	byte _walkDir;
	byte _priority;
	uint16 _lookX, _lookY, _useX, _useY;
	byte _lookDir, _useDir;
	uint16 _absNum;
	Common::Array<int> _anims;
	GPL2Program _program;
	byte *_title;
	byte _location;
	bool _visible;
};

struct GameInfo {
	byte _currentRoom;
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
	byte _map;
	byte _palette;
	uint16 _numMasks;
	uint16 _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	bool _mouseOn, _heroOn;
	double _pers0, _persStep;
	byte _escRoom;
	byte _numGates;
	GPL2Program _program;
};

class Game {

public:
	Game(DraciEngine *vm);
	~Game();

	void init();

	void changeRoom(uint roomNum);

	int getRoomNum();

	// HACK: this is only for testing
	void incRoomNum() { 
		int n = _currentRoom._roomNum;
		n = n < 25 ? n+1 : n;
		_currentRoom._roomNum = n;
	}

	// HACK: same as above
	void decRoomNum() { 
		int n = _currentRoom._roomNum;
		n = n > 0 ? n-1 : n;
		_currentRoom._roomNum = n;
	}

	void loadRoom(uint roomNum);
	int loadAnimation(uint animNum, uint z);
	void loadOverlays();
	void loadObject(uint numObj);

	GameObject *getObject(uint objNum);

	int getVariable(int varNum);
	void setVariable(int varNum, int value);	

private:
	DraciEngine *_vm;
	int *_variables;
	GameInfo *_info;
	Person *_persons;
	uint16 *_dialogOffsets;
	byte *_itemStatus;
	GameObject *_objects;
	Room _currentRoom;	
};

} // End of namespace Draci

#endif // DRACI_GAME_H
