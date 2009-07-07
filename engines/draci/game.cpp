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

#include "common/stream.h"

#include "draci/draci.h"
#include "draci/game.h"
#include "draci/barchive.h"
#include "draci/script.h"
#include "draci/animation.h"

namespace Draci {

Game::Game(DraciEngine *vm) : _vm(vm) {
	unsigned int i;
	Common::String path("INIT.DFW");
	
	BArchive initArchive(path);
	BAFile *file;
	
	// Read in persons
	
	file = initArchive.getFile(5);
	Common::MemoryReadStream personData(file->_data, file->_length);
	
	unsigned int numPersons = file->_length / personSize;
	_persons = new Person[numPersons];
	
	for (i = 0; i < numPersons; ++i) {
		_persons[i]._x = personData.readByte();
		_persons[i]._y = personData.readByte();
		_persons[i]._fontColour = personData.readUint16LE();
	}
	
	// Read in dialog offsets
	
	file = initArchive.getFile(4);
	Common::MemoryReadStream dialogData(file->_data, file->_length);
	
	unsigned int numDialogs = file->_length / sizeof(uint16);
	_dialogOffsets = new uint16[numDialogs];	
	
	unsigned int curOffset;
	for (i = 0, curOffset = 0; i < numDialogs; ++i) {
		_dialogOffsets[i] = curOffset;
		curOffset += dialogData.readUint16LE();
	}
	
	// Read in game info
	
	file = initArchive.getFile(3);
	Common::MemoryReadStream gameData(file->_data, file->_length);
	_info = new GameInfo();
	
	_info->_currentRoom = gameData.readByte() - 1;
	_info->_mapRoom = gameData.readByte() - 1;
	_info->_numObjects = gameData.readUint16LE();
	_info->_numIcons = gameData.readUint16LE();
	_info->_numVariables = gameData.readByte();
	_info->_numPersons = gameData.readByte();
	_info->_numDialogs = gameData.readByte();
	_info->_maxIconWidth = gameData.readUint16LE();
	_info->_maxIconHeight = gameData.readUint16LE();
	_info->_musicLength = gameData.readUint16LE();
	_info->_crc[0] = gameData.readUint16LE();
	_info->_crc[1] = gameData.readUint16LE();
	_info->_crc[2] = gameData.readUint16LE();
	_info->_crc[3] = gameData.readUint16LE();

	_info->_numDialogBlocks = curOffset;

	// Read in variables
	
	file = initArchive.getFile(2);
	unsigned int numVariables = file->_length / sizeof (int16);

	_variables = new int16[numVariables];
	Common::MemoryReadStream variableData(file->_data, file->_length);
	
	for (i = 0; i < numVariables; ++i) {
		_variables[i] = variableData.readUint16LE();
	}

	// Read in item status
	
	file = initArchive.getFile(1);
	_itemStatus = new byte[file->_length];
	memcpy(_itemStatus, file->_data, file->_length);
	
	// Read in object status
	
	file = initArchive.getFile(0);
	unsigned int numObjects = file->_length;
	
	_objects = new GameObject[numObjects];
	Common::MemoryReadStream objStatus(file->_data, file->_length); 
	
	for (i = 0; i < numObjects; ++i) {
		byte tmp = objStatus.readByte();

		// Set object visibility
		_objects[i]._visible = tmp & (1 << 7);

		// Set object location
		_objects[i]._location = (~(1 << 7) & tmp) - 1;
 	}

	assert(numDialogs == _info->_numDialogs);
	assert(numPersons == _info->_numPersons);
	assert(numVariables == _info->_numVariables);
	assert(numObjects == _info->_numObjects);	
}

void Game::init() {
	loadObject(0);
	
	_vm->_script->run(getObject(0)->_program, getObject(0)->_init);

	// HACK: this is only for testing
	_vm->_anims->play(getObject(0)->_seqTab[9]);

	changeRoom(0);
}

void Game::loadRoom(uint roomNum) {

	BAFile *f;
	f = _vm->_roomsArchive->getFile(roomNum * 4);
	Common::MemoryReadStream roomReader(f->_data, f->_length);

	roomReader.readUint32LE(); // Pointer to room program, not used
	roomReader.readUint16LE(); // Program length, not used
	roomReader.readUint32LE(); // Pointer to room title, not used

	_currentRoom._music = roomReader.readByte();
	_currentRoom._map = roomReader.readByte();
	_currentRoom._palette = roomReader.readByte() - 1;
	_currentRoom._numMasks = roomReader.readUint16LE();
	_currentRoom._init = roomReader.readUint16LE();
	_currentRoom._look = roomReader.readUint16LE();
	_currentRoom._use = roomReader.readUint16LE();
	_currentRoom._canUse = roomReader.readUint16LE();
	_currentRoom._imInit = roomReader.readByte();
	_currentRoom._imLook = roomReader.readByte();
	_currentRoom._imUse = roomReader.readByte();
	_currentRoom._mouseOn = roomReader.readByte();
	_currentRoom._heroOn = roomReader.readByte();
	roomReader.read(&_currentRoom._pers0, 12);
	roomReader.read(&_currentRoom._persStep, 12);
	_currentRoom._escRoom = roomReader.readByte() - 1;
	_currentRoom._numGates = roomReader.readByte();

	for (uint i = 0; i < _info->_numObjects; ++i) {
		debugC(1, kDraciLogicDebugLevel, 
			"Checking if object %d (%d) is at the current location (%d)", i,
			_objects[i]._location, roomNum);

		if (_objects[i]._location == roomNum) {
			debugC(1, kDraciLogicDebugLevel, "Loading object %d from room %d", i, roomNum);
			loadObject(i);
		}
	}
	
	// Run the init scripts	for room objects
	// We can't do this in the above loop because some objects' scripts reference
	// other objects that may not yet be loaded
	for (uint i = 0; i < _info->_numObjects; ++i) {
		if (_objects[i]._location == roomNum) {
			_vm->_script->run(getObject(i)->_program, getObject(i)->_init);
		}
	}

	f = _vm->_roomsArchive->getFile(roomNum * 4 + 3);
	_currentRoom._program._bytecode = new byte[f->_length];
	_currentRoom._program._length = f->_length;
	memcpy(_currentRoom._program._bytecode, f->_data, f->_length);

	_vm->_script->run(_currentRoom._program, _currentRoom._init);

	f = _vm->_paletteArchive->getFile(_currentRoom._palette);
	_vm->_screen->setPalette(f->_data, 0, kNumColours);
}

int Game::loadAnimation(uint animNum) {

	BAFile *animFile = _vm->_animationsArchive->getFile(animNum);
	Common::MemoryReadStream animationReader(animFile->_data, animFile->_length);	

	int numFrames = animationReader.readByte();

	// FIXME: handle these properly
	animationReader.readByte(); // Memory logic field, not used
	animationReader.readByte(); // Disable erasing field, not used
	animationReader.readByte(); // Cyclic field, not used
	animationReader.readByte(); // Relative field, not used

	Animation *anim = _vm->_anims->addAnimation(animNum, 254, false);
	
	for (uint i = 0; i < numFrames; ++i) {
		uint spriteNum = animationReader.readUint16LE() - 1;
		int x = animationReader.readSint16LE();
		int y = animationReader.readSint16LE();
		uint zoomX = animationReader.readUint16LE();
		uint zoomY = animationReader.readUint16LE();
		byte mirror = animationReader.readByte();
		uint sample = animationReader.readUint16LE();
		uint freq = animationReader.readUint16LE();
		uint delay = animationReader.readUint16LE();

		anim->setDelay(delay * 10);

		BAFile *spriteFile = _vm->_spritesArchive->getFile(spriteNum);

		Sprite *sp = new Sprite(spriteFile->_data, spriteFile->_length, x, y, true);

		if (mirror) 
			sp->setMirrorOn();

		// HACK: This is only for testing
		anim->setLooping(true);

		anim->addFrame(sp);
	}

	return animNum;
}

void Game::loadObject(uint objNum) {
	BAFile *file;
	
	file = _vm->_objectsArchive->getFile(objNum * 3);
	Common::MemoryReadStream objReader(file->_data, file->_length);
	
	GameObject *obj = _objects + objNum;

	obj->_init = objReader.readUint16LE();
	obj->_look = objReader.readUint16LE();
	obj->_use = objReader.readUint16LE();
	obj->_canUse = objReader.readUint16LE();
	obj->_imInit = objReader.readByte();
	obj->_imLook = objReader.readByte();
	obj->_imUse = objReader.readByte();
	obj->_walkDir = objReader.readByte();
	obj->_priority = objReader.readByte();
	obj->_idxSeq = objReader.readUint16LE();
	obj->_numSeq = objReader.readUint16LE();
	obj->_lookX = objReader.readUint16LE();
	obj->_lookY = objReader.readUint16LE();
	obj->_useX = objReader.readUint16LE();
	obj->_useY = objReader.readUint16LE();
	obj->_lookDir = objReader.readByte();
	obj->_useDir = objReader.readByte();
	
	obj->_absNum = objNum;
	obj->_animObj = 0;
	
	obj->_seqTab = new uint16[obj->_numSeq];

	file = _vm->_objectsArchive->getFile(objNum * 3 + 1);
	obj->_title = new byte[file->_length];
	memcpy(obj->_title, file->_data, file->_length);
	
	file = _vm->_objectsArchive->getFile(objNum * 3 + 2);
	obj->_program._bytecode = new byte[file->_length];
	obj->_program._length = file->_length;
	memcpy(obj->_program._bytecode, file->_data, file->_length);
}

GameObject *Game::getObject(uint objNum) {
	return _objects + objNum;
}

void Game::loadOverlays() {
 	uint x, y, z, num;
 
	BAFile *overlayHeader;

	overlayHeader = _vm->_roomsArchive->getFile(_currentRoom._roomNum * 4 + 2);
	Common::MemoryReadStream overlayReader(overlayHeader->_data, overlayHeader->_length);
 	BAFile *overlayFile;
 
 	for (uint i = 0; i < _currentRoom._numMasks; i++) {
 
		num = overlayReader.readUint16LE() - 1;
 		x = overlayReader.readUint16LE();
 		y = overlayReader.readUint16LE();
 		z = overlayReader.readByte();
 
		overlayFile = _vm->_overlaysArchive->getFile(num);
 		Sprite *sp = new Sprite(overlayFile->_data, overlayFile->_length, x, y, true);
 
 		_vm->_anims->addOverlay(sp, z);		
	}

	_vm->_overlaysArchive->clearCache();

 	_vm->_screen->getSurface()->markDirty();
}

void Game::changeRoom(uint roomNum) {
	_vm->_roomsArchive->clearCache();	
	_vm->_anims->deleteOverlays();

	int oldRoomNum = _currentRoom._roomNum;

	for (uint i = 0; i < _info->_numObjects; ++i) {
		GameObject *obj = &_objects[i];
	
		if (i != 0 && obj->_location == oldRoomNum) {
			for (uint j = 0; j < obj->_numSeq; ++j) {
					_vm->_anims->deleteAnimation(obj->_seqTab[j]);
			}
		}
	}

	_currentRoom._roomNum = roomNum;
	loadRoom(roomNum);
	loadOverlays();
}

int Game::getRoomNum() {
	return _currentRoom._roomNum;
}

Game::~Game() {
	delete[] _persons;
	delete[] _variables;
	delete[] _dialogOffsets;
	delete[] _itemStatus;
	delete[] _objects;
	delete _info;
}

GameObject::~GameObject() { 
	delete[] _seqTab; 
	delete[] _title;
	delete[] _program._bytecode;
}

} 
