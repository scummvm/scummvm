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

namespace Draci {

Game::Game(DraciEngine *vm) : _vm(vm) {
	unsigned int i;
	Common::String path("INIT.DFW");
	
	BArchive initArchive(path);
	BAFile *file;
	
	// Read in persons
	
	file = initArchive[5];
	Common::MemoryReadStream personData(file->_data, file->_length);
	
	unsigned int numPersons = file->_length / personSize;
	_persons = new Person[numPersons];
	
	for (i = 0; i < numPersons; ++i) {
		_persons[i]._x = personData.readByte();
		_persons[i]._y = personData.readByte();
		_persons[i]._fontColour = personData.readUint16LE();
	}
	
	// Read in dialog offsets
	
	file = initArchive[4];
	Common::MemoryReadStream dialogData(file->_data, file->_length);
	
	unsigned int numDialogs = file->_length / sizeof(uint16);
	_dialogOffsets = new uint16[numDialogs];	
	
	unsigned int curOffset;
	for (i = 0, curOffset = 0; i < numDialogs; ++i) {
		_dialogOffsets[i] = curOffset;
		curOffset += dialogData.readUint16LE();
	}
	
	// Read in game info
	
	file = initArchive[3];
	Common::MemoryReadStream gameData(file->_data, file->_length);
	_info = new GameInfo();
	
	_info->_currentRoom = gameData.readByte();
	_info->_mapRoom = gameData.readByte();
	_info->_numObjects = gameData.readUint16LE();
	_info->_numIcons = gameData.readUint16LE();
	_info->_numVariables = gameData.readByte();
	_info->_numPersons = gameData.readByte();
	_info->_numDialogs = gameData.readByte();
	_info->_maxIconWidth = gameData.readUint16LE();
	_info->_maxIconHeight = gameData.readUint16LE();
	_info->_musicLength = gameData.readUint32LE();

// FIXME: Something is wrong here. The total file length is only 23 bytes
// but the whole struct should be 35 bytes.
	_info->_crc[0] = gameData.readUint32LE();
	_info->_crc[1] = gameData.readUint32LE();
	_info->_crc[2] = gameData.readUint32LE();
	_info->_crc[3] = gameData.readUint32LE();
	_info->_numDialogBlocks = gameData.readUint16LE();

	// Read in variables
	
	file = initArchive[2];
	unsigned int numVariables = file->_length / sizeof (int16);

	_variables = new int16[numVariables];
	Common::MemoryReadStream variableData(file->_data, file->_length);
	
	for (i = 0; i < numVariables; ++i) {
		_variables[i] = variableData.readUint16LE();
	}

	// Read in item status
	
	file = initArchive[1];
	_itemStatus = new byte[file->_length];
	memcpy(_itemStatus, file->_data, file->_length);
	
	// Read in object status
	
	file = initArchive[0];
	unsigned int numObjects = file->_length;
	
	_objectStatus = new byte[numObjects];
	memcpy(_objectStatus, file->_data, file->_length);
	
	assert(numDialogs == _info->_numDialogs);
	assert(numPersons == _info->_numPersons);
	assert(numVariables == _info->_numVariables);
	assert(numObjects == _info->_numObjects);	

// TODO: Why is this failing?
//	assert(curOffset == _info->_numDialogBlocks);

	loadObject(0, &_heroObj);
	_vm->_script->run(_heroObj._program, _heroObj._init);
}

void Game::loadObject(uint16 objNum, GameObject *obj) {
	Common::String path("OBJEKTY.DFW");
	
	BArchive objArchive(path);
	BAFile *file;
	
	file = objArchive[objNum * 3];
	Common::MemoryReadStream objReader(file->_data, file->_length);
	
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

	file = objArchive[objNum * 3 + 1];
	obj->_title = new byte[file->_length];
	memcpy(obj->_title, file->_data, file->_length);
	
	file = objArchive[objNum * 3 + 2];
	obj->_program._bytecode = new byte[file->_length];
	obj->_program._length = file->_length;
	memcpy(obj->_program._bytecode, file->_data, file->_length);
}
	
Game::~Game() {
	delete[] _persons;
	delete[] _variables;
	delete[] _dialogOffsets;
	delete[] _itemStatus;
	delete[] _objectStatus;
	delete _info;
}

GameObject::~GameObject() { 
	delete[] _seqTab; 
	delete[] _title;
	delete[] _program._bytecode;
}

} 
