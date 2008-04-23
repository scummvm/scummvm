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

#include "common/endian.h"
#include "common/util.h"

#include "made/database.h"

namespace Made {

/*
	Class types:
	  0x7FFF  byte array
	  0x7FFE  word array
	< 0x7FFE  object
*/

Object::Object() {
}

Object::~Object() {
	if (_freeData && _objData)
		delete[] _objData;
}

void Object::load(Common::SeekableReadStream &source) {

	_freeData = true;

	source.readUint16LE(); // skip flags
	uint16 type = source.readUint16LE();

	if (type == 0x7FFF) {
		_objSize = source.readUint16LE();
	} else if (type == 0x7FFE) {
		_objSize = source.readUint16LE() * 2;
	} else if (type < 0x7FFE) {
		byte count1 = source.readByte();
		byte count2 = source.readByte();
		_objSize = (count1 + count2) * 2;
	}

	source.seek(-6, SEEK_CUR);

	_objSize += 6;

	_objData = new byte[_objSize];
	source.read(_objData, _objSize);

}

void Object::load(byte *source) {

	_objData = source;
	_freeData = false;

	if (getClass() < 0x7FFE) {
		_objSize = (getCount1() + getCount2()) * 2;
	} else {
		_objSize = getSize();
	}
	
	_objSize += 6;

}

uint16 Object::getFlags() const {
	return READ_LE_UINT16(_objData);
}

uint16 Object::getClass() const {
	return READ_LE_UINT16(_objData + 2);
}

uint16 Object::getSize() const {
	return READ_LE_UINT16(_objData + 4);
}

byte Object::getCount1() const {
	return _objData[4];
}

byte Object::getCount2() const {
	return _objData[5];
}

byte *Object::getData() {
	return _objData + 6;
}

const char *Object::getString() {
	if (getClass() == 0x7FFF)
		return (const char*)getData();
	else
		return NULL;
}

void Object::setString(const char *str) {
	if (getClass() == 0x7FFF) {
		char *objStr = (char*)getData();
		if (str)
			strncpy(objStr, str, getSize());
		else
			objStr[0] = '\0';
	}
}

bool Object::isObject() {
	return getClass() < 0x7FFE;
}

bool Object::isVector() {
	return getClass() == 0x7FFF;
}

int16 Object::getVectorSize() {
	if (getClass() == 0x7FFF || getClass() == 0x7FFE) {
		return getSize();
	} else if (getClass() < 0x7FFE) {
		return getCount1() + getCount2();
	} else {
		// should never reach here
		error("Unknown object class");
		return 0;
	}
}

int16 Object::getVectorItem(int16 index) {
	if (getClass() == 0x7FFF) {
		byte *vector = (byte*)getData();
		return vector[index];
	} else if (getClass() == 0x7FFE) {
		int16 *vector = (int16*)getData();
		return READ_LE_UINT16(&vector[index]);
	} else if (getClass() < 0x7FFE) {
		int16 *vector = (int16*)getData();
		return READ_LE_UINT16(&vector[index]);
	} else {
		// should never reach here
		error("Unknown object class");
		return 0;
	}
}

void Object::setVectorItem(int16 index, int16 value) {
	if (getClass() == 0x7FFF) {
		byte *vector = (byte*)getData();
		vector[index] = value;
	} else if (getClass() <= 0x7FFE) {
		int16 *vector = (int16*)getData();
		WRITE_LE_UINT16(&vector[index], value);
	}
}

void Object::dump(const char *filename) {
	/*
	FILE *o = fopen(filename, "wb");
	fwrite(_objData, _objSize, 1, o);
	fclose(o);
	*/
}

GameDatabase::GameDatabase() {
}

GameDatabase::~GameDatabase() {
	if (_gameState)
		delete[] _gameState;
}

void GameDatabase::open(const char *filename) {
	debug(1, "GameDatabase::open() Loading from %s", filename);
	Common::File fd;
	if (!fd.open(filename))
		error("GameDatabase::open() Could not open %s", filename);
	load(fd);
	fd.close();
}

void GameDatabase::openFromRed(const char *redFilename, const char *filename) {
	debug(1, "GameDatabase::openFromRed() Loading from %s->%s", redFilename, filename);
	Common::MemoryReadStream *fileS = RedReader::loadFromRed(redFilename, filename);
	if (!fileS)
		error("GameDatabase::openFromRed() Could not load %s from %s", filename, redFilename);
	load(*fileS);
	delete fileS;
}

void GameDatabase::load(Common::SeekableReadStream &sourceS) {
	
	// TODO: Read/verifiy header
	
	sourceS.seek(0x1E);
	
	uint32 objectIndexOffs = sourceS.readUint32LE();
	uint16 objectCount = sourceS.readUint16LE();
	uint32 gameStateOffs = sourceS.readUint32LE();
	_gameStateSize = sourceS.readUint32LE();
	uint32 objectsOffs = sourceS.readUint32LE();
	uint32 objectsSize = sourceS.readUint32LE();
	_mainCodeObjectIndex = sourceS.readUint16LE();
	
	debug(2, "objectIndexOffs = %08X; objectCount = %d; gameStateOffs = %08X; gameStateSize = %d; objectsOffs = %08X; objectsSize = %d\n",
		objectIndexOffs, objectCount, gameStateOffs, _gameStateSize, objectsOffs, objectsSize);

	_gameState = new byte[_gameStateSize];
	sourceS.seek(gameStateOffs);
	sourceS.read(_gameState, _gameStateSize);

	Common::Array<uint32> objectOffsets;
	sourceS.seek(objectIndexOffs);
	for (uint32 i = 0; i < objectCount; i++)
		objectOffsets.push_back(sourceS.readUint32LE());

	for (uint32 i = 0; i < objectCount; i++) {
		Object *obj = new Object();

		// The LSB indicates if it's a constant or variable object.
		// Constant objects are loaded from disk, while variable objects exist
		// in the _gameState buffer.
		
		debug(2, "obj(%04X) ofs = %08X\n", i, objectOffsets[i]);
		
		if (objectOffsets[i] & 1) {
			debug(2, "-> const %08X\n", objectsOffs + objectOffsets[i] - 1);
			sourceS.seek(objectsOffs + objectOffsets[i] - 1);
			obj->load(sourceS);
		} else {
			debug(2, "-> var\n");
			obj->load(_gameState + objectOffsets[i]);
		}
		_objects.push_back(obj);
	}

}

int16 GameDatabase::getVar(int16 index) {
	return (int16)READ_LE_UINT16(_gameState + index * 2);
}

void GameDatabase::setVar(int16 index, int16 value) {
	WRITE_LE_UINT16(_gameState + index * 2, value);
}

int16 *GameDatabase::getObjectPropertyPtr(int16 objectIndex, int16 propertyId, int16 &propertyFlag) {
	Object *obj = getObject(objectIndex);

	int16 *prop = (int16*)obj->getData();
	byte count1 = obj->getCount1();
	byte count2 = obj->getCount2();
	
	int16 *propPtr1 = prop + count1;
	int16 *propPtr2 = prop + count2;

	debug(2, "# propertyId = %04X\n", propertyId);

	// First see if the property exists in the given object
	while (count2-- > 0) {
		if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
			if (READ_LE_UINT16(prop) & 0x4000) {
				//debug(2, "! L1.1\n");
				propertyFlag = 1;
				return (int16*)_gameState + READ_LE_UINT16(propPtr1);
			} else {
				//debug(2, "! L1.2\n");
				propertyFlag = obj->getFlags() & 1;
				return propPtr1;
			}
		}
		prop++;
		propPtr1++;
	}
	
	// Now check in the object hierarchy of the given object
	int16 parentObjectIndex = obj->getClass();
	if (parentObjectIndex == 0) {
		//debug(2, "! NULL(np)\n");
		return NULL;
	}
		
	while (parentObjectIndex != 0) {
	
		//debug(2, "parentObjectIndex = %04X\n", parentObjectIndex);

		obj = getObject(parentObjectIndex);
		
		prop = (int16*)obj->getData();
		count1 = obj->getCount1();
		count2 = obj->getCount2();

		propPtr1 = propPtr2 + count1 - count2;
		int16 *propertyPtr = prop + count1;
		
		while (count2-- > 0) {
			if (!(READ_LE_UINT16(prop) & 0x8000)) {
				if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
					if (*prop & 0x4000) {
						//debug(2, "! L2.1\n");
						propertyFlag = 1;
						return (int16*)_gameState + READ_LE_UINT16(propPtr1);
					} else {
						//debug(2, "! L2.2\n");
						propertyFlag = obj->getFlags() & 1;
						return propPtr1;
					}
				} else {
					propPtr1++;
				}
			} else {
				if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
					if (*prop & 0x4000) {
						//debug(2, "! L3.1\n");
						propertyFlag = 1;
						return (int16*)_gameState + READ_LE_UINT16(propertyPtr);
					} else {
						//debug(2, "! L3.2\n");
						propertyFlag = obj->getFlags() & 1;
						return propertyPtr;
					}
				}
			}
			prop++;
			propertyPtr++;
		}
	
		parentObjectIndex = obj->getClass();
		
	}

	//debug(2, "! NULL(nf)\n");
	return NULL;
	
}

int16 GameDatabase::getObjectProperty(int16 objectIndex, int16 propertyId) {

	if (objectIndex == 0)
		return 0;

	int16 propertyFlag;
	int16 *property = getObjectPropertyPtr(objectIndex, propertyId, propertyFlag);

	if (property) {
		return (int16)READ_LE_UINT16(property);
	} else {
		return 0;
	}

}

int16 GameDatabase::setObjectProperty(int16 objectIndex, int16 propertyId, int16 value) {

	if (objectIndex == 0)
		return 0;

	int16 propertyFlag;
	int16 *property = getObjectPropertyPtr(objectIndex, propertyId, propertyFlag);
	
	if (property) {
		if (propertyFlag == 1) {
			WRITE_LE_UINT16(property, value);
		} else {
			debug(2, "GameDatabase::setObjectProperty(%04X, %04X, %04X) Trying to set constant property\n",
				objectIndex, propertyId, value);
		}
		return value;
	} else {
		return 0;
	}
	
}

void GameDatabase::dumpObject(int16 index) {
	Object *obj = getObject(index);
	char fn[512];
	sprintf(fn, "obj%04X.0", index);
	obj->dump(fn);
}

} // End of namespace Made
