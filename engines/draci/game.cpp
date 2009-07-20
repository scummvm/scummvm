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

#include <cmath>

namespace Draci {

static double real_to_double(byte real[6]);

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

	// Close persons file
	file->close();
	
	// Read in dialog offsets
	
	file = initArchive.getFile(4);
	Common::MemoryReadStream dialogData(file->_data, file->_length);
	
	unsigned int numDialogs = file->_length / sizeof(uint16);
	_dialogOffsets = new uint[numDialogs];	
	
	unsigned int curOffset;
	for (i = 0, curOffset = 0; i < numDialogs; ++i) {
		_dialogOffsets[i] = curOffset;
		curOffset += dialogData.readUint16LE();
	}
	
	// Close dialogs file
	file->close();	

	// Read in game info
	
	file = initArchive.getFile(3);
	Common::MemoryReadStream gameData(file->_data, file->_length);
	
	_info._startRoom = gameData.readByte() - 1;
	_info._mapRoom = gameData.readByte() - 1;
	_info._numObjects = gameData.readUint16LE();
	_info._numIcons = gameData.readUint16LE();
	_info._numVariables = gameData.readByte();
	_info._numPersons = gameData.readByte();
	_info._numDialogs = gameData.readByte();
	_info._maxIconWidth = gameData.readUint16LE();
	_info._maxIconHeight = gameData.readUint16LE();
	_info._musicLength = gameData.readUint16LE();
	_info._crc[0] = gameData.readUint16LE();
	_info._crc[1] = gameData.readUint16LE();
	_info._crc[2] = gameData.readUint16LE();
	_info._crc[3] = gameData.readUint16LE();

	_info._numDialogBlocks = curOffset;

	// Close game info file
	file->close();

	// Read in variables
	
	file = initArchive.getFile(2);
	unsigned int numVariables = file->_length / sizeof (int16);

	_variables = new int[numVariables];
	Common::MemoryReadStream variableData(file->_data, file->_length);
	
	for (i = 0; i < numVariables; ++i) {
		_variables[i] = variableData.readUint16LE();
	}

	// Close variables file
	file->close();

	// Read in item icon status
	
	file = initArchive.getFile(1);
	_iconStatus = file->_data;
	uint numIcons = file->_length;
	
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
	
	// Close object status file
	file->close();

	assert(numDialogs == _info._numDialogs);
	assert(numPersons == _info._numPersons);
	assert(numVariables == _info._numVariables);
	assert(numObjects == _info._numObjects);
	assert(numIcons == _info._numIcons);	
}

void Game::init() {
	loadObject(kDragonObject);
	
	GameObject *dragon = getObject(kDragonObject);
	debugC(4, kDraciLogicDebugLevel, "Running init program for the dragon object...");
	_vm->_script->run(dragon->_program, dragon->_init);

	_currentRoom._roomNum = _info._startRoom;
	changeRoom(_info._startRoom);
}

void Game::loop() {
	
	if (_currentRoom._mouseOn) {
		int x = _vm->_mouse->getPosX();
		int y = _vm->_mouse->getPosY();

		if (_vm->_mouse->lButtonPressed() && _currentRoom._walkingMap.isWalkable(x, y)) {
			
			// Fetch dragon's animation ID
			// FIXME: Need to add proper walking (this only warps the dragon to position)
			int animID = getObject(kDragonObject)->_anims[0];

			Animation *anim = _vm->_anims->getAnimation(animID);
			Drawable *frame = anim->getFrame();

			// Calculate scaling factor
			double scaleX = _currentRoom._pers0 + _currentRoom._persStep * y;
			double scaleY = scaleX;

			// Calculate scaled height of sprite
			int height = frame->getScaledHeight(scaleY);

			// Set the Z coordinate for the dragon's animation
			anim->setZ(y+1);

			// We naturally want the dragon to position its feet to the location of the
			// click but sprites are drawn from their top-left corner so we subtract
			// the height of the dragon's sprite
			y -= height;
			anim->setRelative(x, y);

			// Set the scaling factor
			anim->setScaling(scaleX, scaleY);

			// Play the animation
			_vm->_anims->play(animID);

			debugC(4, kDraciLogicDebugLevel, "Walk to x: %d y: %d", x, y);
		}
	}
}

void Game::loadRoom(int roomNum) {

	BAFile *f;
	f = _vm->_roomsArchive->getFile(roomNum * 4);
	Common::MemoryReadStream roomReader(f->_data, f->_length);

	roomReader.readUint32LE(); // Pointer to room program, not used
	roomReader.readUint16LE(); // Program length, not used
	roomReader.readUint32LE(); // Pointer to room title, not used

	_currentRoom._music = roomReader.readByte();

	int mapIdx = roomReader.readByte() - 1;
	f = _vm->_walkingMapsArchive->getFile(mapIdx);
	_currentRoom._walkingMap.load(f->_data, f->_length);

	_currentRoom._palette = roomReader.readByte() - 1;
	_currentRoom._numOverlays = roomReader.readSint16LE();
	_currentRoom._init = roomReader.readSint16LE();
	_currentRoom._look = roomReader.readSint16LE();
	_currentRoom._use = roomReader.readSint16LE();
	_currentRoom._canUse = roomReader.readSint16LE();
	_currentRoom._imInit = roomReader.readByte();
	_currentRoom._imLook = roomReader.readByte();
	_currentRoom._imUse = roomReader.readByte();
	_currentRoom._mouseOn = roomReader.readByte();
	_currentRoom._heroOn = roomReader.readByte();

	// Read in pers0 and persStep (stored as 6-byte Pascal reals)
	byte real[6];

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
		debug(2, "%d", real[i]);
	}

	_currentRoom._pers0 = real_to_double(real);

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
	}

	_currentRoom._persStep = real_to_double(real);

	_currentRoom._escRoom = roomReader.readByte() - 1;
	_currentRoom._numGates = roomReader.readByte();

	debugC(4, kDraciLogicDebugLevel, "Music: %d", _currentRoom._music);
	debugC(4, kDraciLogicDebugLevel, "Map: %d", mapIdx);
	debugC(4, kDraciLogicDebugLevel, "Palette: %d", _currentRoom._palette);
	debugC(4, kDraciLogicDebugLevel, "Overlays: %d", _currentRoom._numOverlays);
	debugC(4, kDraciLogicDebugLevel, "Init: %d", _currentRoom._init);
	debugC(4, kDraciLogicDebugLevel, "Look: %d", _currentRoom._look);
	debugC(4, kDraciLogicDebugLevel, "Use: %d", _currentRoom._use);
	debugC(4, kDraciLogicDebugLevel, "CanUse: %d", _currentRoom._canUse);
	debugC(4, kDraciLogicDebugLevel, "ImInit: %d", _currentRoom._imInit);
	debugC(4, kDraciLogicDebugLevel, "ImLook: %d", _currentRoom._imLook);
	debugC(4, kDraciLogicDebugLevel, "ImUse: %d", _currentRoom._imUse);
	debugC(4, kDraciLogicDebugLevel, "MouseOn: %d", _currentRoom._mouseOn);
	debugC(4, kDraciLogicDebugLevel, "HeroOn: %d", _currentRoom._heroOn);
	debugC(4, kDraciLogicDebugLevel, "Pers0: %f", _currentRoom._pers0);
	debugC(4, kDraciLogicDebugLevel, "PersStep: %f", _currentRoom._persStep);
	debugC(4, kDraciLogicDebugLevel, "EscRoom: %d", _currentRoom._escRoom);
	debugC(4, kDraciLogicDebugLevel, "Gates: %d", _currentRoom._numGates);

	// Read in the gates' numbers
	Common::Array<int> gates;

	for (uint i = 0; i < _currentRoom._numGates; ++i) {
		gates.push_back(roomReader.readSint16LE());
	}

	// Load the room's objects
	for (uint i = 0; i < _info._numObjects; ++i) {
		debugC(7, kDraciLogicDebugLevel, 
			"Checking if object %d (%d) is at the current location (%d)", i,
			_objects[i]._location, roomNum);

		if (_objects[i]._location == roomNum) {
			debugC(6, kDraciLogicDebugLevel, "Loading object %d from room %d", i, roomNum);
			loadObject(i);
		}
	}
	
	// Run the init scripts	for room objects
	// We can't do this in the above loop because some objects' scripts reference
	// other objects that may not yet be loaded
	for (uint i = 0; i < _info._numObjects; ++i) {
		if (_objects[i]._location == roomNum) {
			debugC(6, kDraciLogicDebugLevel, 
				"Running init program for object %d (offset %d)", i, getObject(i)->_init);		
			_vm->_script->run(getObject(i)->_program, getObject(i)->_init);
		}
	}

	// Load the room's GPL program and run the init part
	f = _vm->_roomsArchive->getFile(roomNum * 4 + 3);
	_currentRoom._program._bytecode = f->_data;
	_currentRoom._program._length = f->_length;

	debugC(4, kDraciLogicDebugLevel, "Running room init program...");
	_vm->_script->run(_currentRoom._program, _currentRoom._init);

	// HACK: Gates' scripts shouldn't be run unconditionally
	// This is for testing
	for (uint i = 0; i < _currentRoom._numGates; ++i) {
		debugC(6, kDraciLogicDebugLevel, "Running program for gate %d", i);
		_vm->_script->run(_currentRoom._program, gates[i]);
	}

	// Set room palette
	f = _vm->_paletteArchive->getFile(_currentRoom._palette);
	_vm->_screen->setPalette(f->_data, 0, kNumColours);

	// Set cursor state
	// Need to do this after we set the palette since the cursors use it
	if (_currentRoom._mouseOn) {
		debugC(6, kDraciLogicDebugLevel, "Mouse: ON");
		_vm->_mouse->cursorOn();
	} else {
		debugC(6, kDraciLogicDebugLevel, "Mouse: OFF");
		_vm->_mouse->cursorOff();
	}

	_vm->_mouse->setCursorType(kNormalCursor);

	// HACK: Create a visible overlay from the walking map so we can test it
	byte *wlk = new byte[kScreenWidth * kScreenHeight];
	memset(wlk, 255, kScreenWidth * kScreenHeight);

	for (uint i = 0; i < kScreenWidth; ++i) {
		for (uint j = 0; j < kScreenHeight; ++j) {
			if (_currentRoom._walkingMap.isWalkable(i, j)) {
				wlk[j * kScreenWidth + i] = 2;
			}
		}
	}

	Sprite *ov = new Sprite(wlk, kScreenWidth, kScreenHeight, 0, 0, false);

	Animation *map = _vm->_anims->addAnimation(-2, 255, false);
	map->addFrame(ov);
}

int Game::loadAnimation(uint animNum, uint z) {

	BAFile *animFile = _vm->_animationsArchive->getFile(animNum);
	Common::MemoryReadStream animationReader(animFile->_data, animFile->_length);	

	uint numFrames = animationReader.readByte();

	// FIXME: handle these properly
	animationReader.readByte(); // Memory logic field, not used
	animationReader.readByte(); // Disable erasing field, not used
	
	bool cyclic = animationReader.readByte();

	animationReader.readByte(); // Relative field, not used

	Animation *anim = _vm->_anims->addAnimation(animNum, z, false);

	anim->setLooping(cyclic);

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

		BAFile *spriteFile = _vm->_spritesArchive->getFile(spriteNum);

		Sprite *sp = new Sprite(spriteFile->_data, spriteFile->_length, x, y, true);

		if (mirror) 
			sp->setMirrorOn();

		sp->setDelay(delay * 10);

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
	obj->_z = objReader.readByte();
	objReader.readUint16LE(); // idxSeq field, not used
	objReader.readUint16LE(); // numSeq field, not used
	obj->_lookX = objReader.readUint16LE();
	obj->_lookY = objReader.readUint16LE();
	obj->_useX = objReader.readUint16LE();
	obj->_useY = objReader.readUint16LE();
	obj->_lookDir = objReader.readByte();
	obj->_useDir = objReader.readByte();
	
	obj->_absNum = objNum;
	
	file = _vm->_objectsArchive->getFile(objNum * 3 + 1);
	obj->_title = file->_data;
	
	file = _vm->_objectsArchive->getFile(objNum * 3 + 2);
	obj->_program._bytecode = file->_data;
	obj->_program._length = file->_length;
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
 
 	for (int i = 0; i < _currentRoom._numOverlays; i++) {
 
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
	
	debugC(1, kDraciLogicDebugLevel, "Changing to room %d", roomNum);

	// Clear archives
	_vm->_roomsArchive->clearCache();
	_vm->_spritesArchive->clearCache();
	_vm->_paletteArchive->clearCache();
	_vm->_animationsArchive->clearCache();
	_vm->_walkingMapsArchive->clearCache();

	_vm->_screen->clearScreen();

	_vm->_anims->deleteOverlays();
	
	// Delete walking map testing overlay
	_vm->_anims->deleteAnimation(-2);

	int oldRoomNum = _currentRoom._roomNum;

	// TODO: Make objects capable of stopping their own animations
	GameObject *dragon = getObject(kDragonObject);
	for (uint i = 0; i < dragon->_anims.size(); ++i) {
		_vm->_anims->stop(dragon->_anims[i]);
	}

	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];
		
		if (i != 0 && (obj->_location == oldRoomNum)) {
			for (uint j = 0; j < obj->_anims.size(); ++j) {
					_vm->_anims->deleteAnimation(obj->_anims[j]);
			}
			obj->_anims.clear();
		}
	}

	_currentRoom._roomNum = roomNum;
	loadRoom(roomNum);
	loadOverlays();
}

int Game::getRoomNum() {
	return _currentRoom._roomNum;
}

int Game::getVariable(int numVar) {
	return _variables[numVar];
}

void Game::setVariable(int numVar, int value) {
	_variables[numVar] = value;
}

int Game::getIconStatus(int iconID) {
	return _iconStatus[iconID];
}

Game::~Game() {
	delete[] _persons;
	delete[] _variables;
	delete[] _dialogOffsets;
	delete[] _objects;
}

bool WalkingMap::isWalkable(int x, int y) {

	// Convert to map pixels
	x = x / _deltaX;
	y = y / _deltaY;

	int pixelIndex = _mapWidth * y + x;
	int byteIndex = pixelIndex / 8;
	int mapByte = _data[byteIndex];

	return mapByte & (1 << pixelIndex % 8);
}

static double real_to_double(byte real[6]) {

	// Extract sign bit
	int sign = real[0] & (1 << 7);
	
	// Extract exponent and adjust for bias
	int exp = real[5] - 129;

	double mantissa;
	double tmp = 0.0;

	if (real[5] == 0) {
		mantissa = 0.0;
	} else {
		
		// Process the first four least significant bytes
		for (int i = 4; i >= 1; --i) {		
			tmp += real[i];
			tmp /= 1 << 8;
		}

		// Process the most significant byte (remove the sign bit)
		tmp += real[0] & ((1 << 7) - 1);
		tmp /= 1 << 8;

		// Calculate mantissa
		mantissa = 1.0;
		mantissa += 2.0 * tmp;
	}

	// Flip sign if necessary
	if (sign) {
		mantissa = -mantissa;
	}

	// Calculate final value
	return ldexp(mantissa, exp);	
}

} 
