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

#include "common/system.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/savefile.h"

#include "made/database.h"

namespace Made {

/*
	Class types:
	  0x7FFF  byte array
	  0x7FFE  word array
	< 0x7FFE  object
*/

Object::Object() : _objData(NULL), _freeData(false) {
}

Object::~Object() {
	if (_freeData && _objData)
		delete[] _objData;
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
	} else if (getClass() <= 0x7FFE) {
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

int ObjectV2::load(Common::SeekableReadStream &source) {

	if (_freeData && _objData)
		delete[] _objData;

	_freeData = true;
	
	byte header[4];
	source.read(header, 4);
	
	uint16 type = READ_LE_UINT16(header);
	if (type == 0x7FFF) {
		_objSize = READ_LE_UINT16(header + 2);
	} else if (type == 0x7FFE) {
		_objSize = READ_LE_UINT16(header + 2) * 2;
	} else if (type < 0x7FFE) {
		byte count1 = header[2];
		byte count2 = header[3];
		_objSize = (count1 + count2) * 2;
	}
	_objSize += 4;
	_objData = new byte[_objSize];
	memcpy(_objData, header, 4);
	source.read(_objData + 4, _objSize - 4);

	return _objSize;

}

int ObjectV2::load(byte *source) {
	// Not implemented/used for version 2 objects
	return 0;
}

int ObjectV2::save(Common::WriteStream &dest) {
	dest.write(_objData, _objSize);
	return 0;
}

uint16 ObjectV2::getFlags() {
	return 1;
}

uint16 ObjectV2::getClass() {
	return READ_LE_UINT16(_objData);
}

uint16 ObjectV2::getSize() {
	return READ_LE_UINT16(_objData + 2);
}

byte ObjectV2::getCount1() {
	return _objData[2];
}

byte ObjectV2::getCount2() {
	return _objData[3];
}

byte *ObjectV2::getData() {
	return _objData + 4;
}

int ObjectV3::load(Common::SeekableReadStream &source) {

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
	return _objSize;

}

int ObjectV3::load(byte *source) {
	_objData = source;
	_freeData = false;
	if (getClass() < 0x7FFE) {
		_objSize = (getCount1() + getCount2()) * 2;
	} else {
		_objSize = getSize();
	}
	_objSize += 6;
	return _objSize;
}

int ObjectV3::save(Common::WriteStream &dest) {
	// Not implemented/used for version 3 objects
	return 0;
}

uint16 ObjectV3::getFlags() {
	return READ_LE_UINT16(_objData);
}

uint16 ObjectV3::getClass() {
	return READ_LE_UINT16(_objData + 2);
}

uint16 ObjectV3::getSize() {
	return READ_LE_UINT16(_objData + 4);
}

byte ObjectV3::getCount1() {
	return _objData[4];
}

byte ObjectV3::getCount2() {
	return _objData[5];
}

byte *ObjectV3::getData() {
	return _objData + 6;
}



GameDatabase::GameDatabase(MadeEngine *vm) : _vm(vm) {
}

GameDatabase::~GameDatabase() {
	if (_gameState)
		delete[] _gameState;
}

void GameDatabase::open(const char *filename) {
	debug(1, "GameDatabase::open() Loading from %s", filename);
	_isRedSource = false;
	_filename = filename;
	_redFilename = "";
	Common::File fd;
	if (!fd.open(filename))
		error("GameDatabase::open() Could not open %s", filename);
	load(fd);
	fd.close();
}

void GameDatabase::openFromRed(const char *redFilename, const char *filename) {
	debug(1, "GameDatabase::openFromRed() Loading from %s->%s", redFilename, filename);
	_isRedSource = true;
	_filename = filename;
	_redFilename = redFilename;
	Common::MemoryReadStream *fileS = RedReader::loadFromRed(redFilename, filename);
	if (!fileS)
		error("GameDatabase::openFromRed() Could not load %s from %s", filename, redFilename);
	load(*fileS);
	delete fileS;
}

void GameDatabase::reload() {
	if (!_isRedSource) {
		Common::File fd;
		if (!fd.open(_filename.c_str()))
			error("GameDatabase::reload() Could not open %s", _filename.c_str());
		reloadFromStream(fd);
	} else {
		Common::MemoryReadStream *fileS = RedReader::loadFromRed(_redFilename.c_str(), _filename.c_str());
		if (!fileS)
			error("GameDatabase::openFromRed() Could not load %s from %s", _filename.c_str(), _redFilename.c_str());
		reloadFromStream(*fileS);
		delete fileS;
	}
}

int16 GameDatabase::getVar(int16 index) {
	return (int16)READ_LE_UINT16(_gameState + index * 2);
}

void GameDatabase::setVar(int16 index, int16 value) {
	WRITE_LE_UINT16(_gameState + index * 2, value);
}

const char *GameDatabase::getObjectString(int16 index) {
	Object *obj = getObject(index);
	if (obj)
		return obj->getString();
	else
		return "";
}

void GameDatabase::setObjectString(int16 index, const char *str) {
	Object *obj = getObject(index);
	if (obj)
		obj->setString(str);
}

int16 GameDatabase::getObjectProperty(int16 objectIndex, int16 propertyId) {

	if (objectIndex == 0)
		return 0;

	int16 propertyFlag;
	int16 *property = findObjectProperty(objectIndex, propertyId, propertyFlag);

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
	int16 *property = findObjectProperty(objectIndex, propertyId, propertyFlag);

	if (property) {
		if (propertyFlag == 1) {
			WRITE_LE_UINT16(property, value);
		} else {
			warning("GameDatabase::setObjectProperty(%04X, %04X, %04X) Trying to set constant property\n",
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


/* GameDatabaseV2 */

GameDatabaseV2::GameDatabaseV2(MadeEngine *vm) : GameDatabase(vm), _gameText(NULL) {
}

GameDatabaseV2::~GameDatabaseV2() {
	if (_gameText)
		delete[] _gameText;
}

void GameDatabaseV2::load(Common::SeekableReadStream &sourceS) {
	
	// TODO: Read/verifiy header
	
	sourceS.seek(0x1C);
	
	uint32 textOffs = sourceS.readUint16LE() * 512;
	uint16 objectCount = sourceS.readUint16LE();
	uint16 varObjectCount = sourceS.readUint16LE();
	_gameStateSize = sourceS.readUint16LE() * 2;
	sourceS.readUint16LE(); // unknown
	uint32 objectsOffs = sourceS.readUint16LE() * 512;
	sourceS.readUint16LE(); // unknown
	_mainCodeObjectIndex = sourceS.readUint16LE();
	sourceS.readUint16LE(); // unknown
	uint32 objectsSize = sourceS.readUint32LE() * 2;
	uint32 textSize = objectsOffs - textOffs;

	debug(2, "textOffs = %08X; textSize = %08X; objectCount = %d; varObjectCount = %d; gameStateSize = %d; objectsOffs = %08X; objectsSize = %d\n", textOffs, textSize, objectCount, varObjectCount, _gameStateSize, objectsOffs, objectsSize);

	_gameState = new byte[_gameStateSize + 2];
	memset(_gameState, 0, _gameStateSize + 2);
	setVar(1, objectCount);

	sourceS.seek(textOffs);
	_gameText = new char[textSize];
	sourceS.read(_gameText, textSize);
	// "Decrypt" the text data
	for (uint32 i = 0; i < textSize; i++)
		_gameText[i] += 0x1E;

	sourceS.seek(objectsOffs);

	for (uint32 i = 0; i < objectCount; i++) {
		Object *obj = new ObjectV2();
		int objSize = obj->load(sourceS);
		// objects are aligned on 2-byte-boundaries, skip unused bytes
		sourceS.skip(objSize % 2);
		_objects.push_back(obj);
	}
	
}

void GameDatabaseV2::reloadFromStream(Common::SeekableReadStream &sourceS) {
	// Not used in version 2 games
}

bool GameDatabaseV2::getSavegameDescription(const char *filename, Common::String &description) {
	// Not used in version 2 games
	return false;
}

int16 GameDatabaseV2::savegame(const char *filename, const char *description, int16 version) {
	Common::OutSaveFile *out;
	int16 result = 0;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return 6;
	}
	// Variable 0 is not saved
	out->write(_gameState + 2, _gameStateSize - 2);
	for (uint i = 0; i < _objects.size(); i++)
		_objects[i]->save(*out);
	delete out;
	return result;
}

int16 GameDatabaseV2::loadgame(const char *filename, int16 version) {
	Common::InSaveFile *in;
	int16 result = 0;
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return 1;
	}
	// Variable 0 is not loaded
	in->read(_gameState + 2, _gameStateSize - 2);
	for (uint i = 0; i < _objects.size(); i++) {
		_objects[i]->load(*in);
	}
	delete in;
	return result;
}

int16 *GameDatabaseV2::findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag) {
	Object *obj = getObject(objectIndex);

	int16 *prop = (int16*)obj->getData();
	byte count1 = obj->getCount1();
	byte count2 = obj->getCount2();

	int16 *propPtr1 = prop + count1;
	int16 *propPtr2 = prop + count2;

	// First see if the property exists in the given object
	while (count2--) {
		if ((READ_LE_UINT16(prop) & 0x7FFF) == propertyId) {
			propertyFlag = obj->getFlags() & 1;
			return propPtr1;
		}
		prop++;
		propPtr1++;
	}

	// Now check in the object hierarchy of the given object
	int16 parentObjectIndex = obj->getClass();
	if (parentObjectIndex == 0) {
		return NULL;
	}

	while (parentObjectIndex != 0) {

		obj = getObject(parentObjectIndex);

		prop = (int16*)obj->getData();
		count1 = obj->getCount1();
		count2 = obj->getCount2();

		propPtr1 = propPtr2 + count1 - count2;
		int16 *propertyPtr = prop + count1;

		while (count2--) {
			if ((READ_LE_UINT16(prop) & 0x8000) == 0) {
				if ((READ_LE_UINT16(prop) & 0x7FFF) == propertyId) {
					propertyFlag = obj->getFlags() & 1;
					return propPtr1;
				} else {
					propPtr1++;
				}
			} else {
				if ((READ_LE_UINT16(prop) & 0x7FFF) == propertyId) {
					propertyFlag = obj->getFlags() & 1;
					return propertyPtr;
				}
			}
			prop++;
			propertyPtr++;
		}

		parentObjectIndex = obj->getClass();

	}

	debug(1, "findObjectProperty(%04X, %04X) Property not found", objectIndex, propertyId);
	return NULL;

}

const char *GameDatabaseV2::getString(uint16 offset) {
	return (const char*)&_gameText[offset * 4];
}


/* GameDatabaseV3 */

GameDatabaseV3::GameDatabaseV3(MadeEngine *vm) : GameDatabase(vm) {
}

void GameDatabaseV3::load(Common::SeekableReadStream &sourceS) {

	// TODO: Read/verifiy header

	sourceS.seek(0x1E);

	uint32 objectIndexOffs = sourceS.readUint32LE();
	uint16 objectCount = sourceS.readUint16LE();
	_gameStateOffs = sourceS.readUint32LE();
	_gameStateSize = sourceS.readUint32LE();
	uint32 objectsOffs = sourceS.readUint32LE();
	uint32 objectsSize = sourceS.readUint32LE();
	_mainCodeObjectIndex = sourceS.readUint16LE();

	debug(2, "objectIndexOffs = %08X; objectCount = %d; gameStateOffs = %08X; gameStateSize = %d; objectsOffs = %08X; objectsSize = %d\n", objectIndexOffs, objectCount, _gameStateOffs, _gameStateSize, objectsOffs, objectsSize);

	_gameState = new byte[_gameStateSize];
	sourceS.seek(_gameStateOffs);
	sourceS.read(_gameState, _gameStateSize);

	Common::Array<uint32> objectOffsets;
	sourceS.seek(objectIndexOffs);
	for (uint32 i = 0; i < objectCount; i++)
		objectOffsets.push_back(sourceS.readUint32LE());

	for (uint32 i = 0; i < objectCount; i++) {
		Object *obj = new ObjectV3();

		// The LSB indicates if it's a constant or variable object.
		// Constant objects are loaded from disk, while variable objects exist
		// in the _gameState buffer.

		if (objectOffsets[i] & 1) {
			sourceS.seek(objectsOffs + objectOffsets[i] - 1);
			obj->load(sourceS);
		} else {
			obj->load(_gameState + objectOffsets[i]);
		}
		_objects.push_back(obj);
	}

}

void GameDatabaseV3::reloadFromStream(Common::SeekableReadStream &sourceS) {
	sourceS.seek(_gameStateOffs);
	sourceS.read(_gameState, _gameStateSize);
}

bool GameDatabaseV3::getSavegameDescription(const char *filename, Common::String &description) {
	Common::InSaveFile *in;
	char desc[64];
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		return false;
	}
	in->skip(4); // TODO: Verify marker 'SGAM'
	in->skip(4); // TODO: Verify size
	in->skip(2); // TODO: Verify version
	in->read(desc, 64);
	description = desc;
	delete in;
	return true;
}

int16 GameDatabaseV3::savegame(const char *filename, const char *description, int16 version) {
	Common::OutSaveFile *out;
	char desc[64];
	int16 result = 0;
	uint32 size = 4 + 4 + 2 + _gameStateSize;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return 6;
	}
	strncpy(desc, description, 64);
	out->writeUint32BE(MKID_BE('SGAM'));
	out->writeUint32LE(size);
	out->writeUint16LE(version);
	out->write(desc, 64);
	out->write(_gameState, _gameStateSize);
	delete out;
	return result;
}

int16 GameDatabaseV3::loadgame(const char *filename, int16 version) {
	Common::InSaveFile *in;
	int16 result = 0;
	//uint32 expectedSize = 4 + 4 + 2 + _gameStateSize;
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return 1;
	}
	in->skip(4); // TODO: Verify marker 'SGAM'
	in->skip(4); // TODO: Verify size
	in->skip(2); // TODO: Verify version
	in->skip(64); // skip savegame description
	in->read(_gameState, _gameStateSize);
	delete in;
	return result;
}

int16 *GameDatabaseV3::findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag) {
	Object *obj = getObject(objectIndex);

	int16 *prop = (int16*)obj->getData();
	byte count1 = obj->getCount1();
	byte count2 = obj->getCount2();

	int16 *propPtr1 = prop + count1;
	int16 *propPtr2 = prop + count2;

	// First see if the property exists in the given object
	while (count2-- > 0) {
		if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
			if (READ_LE_UINT16(prop) & 0x4000) {
				propertyFlag = 1;
				return (int16*)_gameState + READ_LE_UINT16(propPtr1);
			} else {
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
		return NULL;
	}

	while (parentObjectIndex != 0) {

		obj = getObject(parentObjectIndex);

		prop = (int16*)obj->getData();
		count1 = obj->getCount1();
		count2 = obj->getCount2();

		propPtr1 = propPtr2 + count1 - count2;
		int16 *propertyPtr = prop + count1;

		while (count2-- > 0) {
			if (!(READ_LE_UINT16(prop) & 0x8000)) {
				if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
					if (READ_LE_UINT16(prop) & 0x4000) {
						propertyFlag = 1;
						return (int16*)_gameState + READ_LE_UINT16(propPtr1);
					} else {
						propertyFlag = obj->getFlags() & 1;
						return propPtr1;
					}
				} else {
					propPtr1++;
				}
			} else {
				if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
					if (READ_LE_UINT16(prop) & 0x4000) {
						propertyFlag = 1;
						return (int16*)_gameState + READ_LE_UINT16(propertyPtr);
					} else {
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

	return NULL;
	
}

const char *GameDatabaseV3::getString(uint16 offset) {
	// Not used in version 3 games
	return NULL;
}

} // End of namespace Made
