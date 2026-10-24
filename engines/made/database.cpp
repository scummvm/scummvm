/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "made/database.h"

#include "common/compression/kd_red.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Made {

Object::Object() : _objData(nullptr), _objSize(0), _freeData(false) {}

Object::~Object() {
	if (_freeData)
		delete[] _objData;
}

// Default impls for when a subclass does not override
uint16 Object::getFlags() const {
	return 1; // V1 / V2 objects do not have flags
}

int Object::save(Common::WriteStream &dest) const {
	error("Object::save Not implemented/used for this version"); // V3 objects do not Save
}

/*
	Class types:
	  0x7FFF  byte array
	  0x7FFE  word array
	< 0x7FFE  object
*/

bool Object::isByteArray() const {
	return getClass() == 0x7FFF;
}

bool Object::isWordArray() const {
	return getClass() == 0x7FFE;
}

bool Object::isObject() const {
	return getClass() < 0x7FFE;
}

bool Object::isWritable() const {
	return getFlags() & 1;
}

const char *Object::getString() const {
	if (isByteArray())
		return (const char *)getData();
	else {
		warning("getString() called on non-byte-array object (%04X)", getClass());
		return nullptr;
	}
}

void Object::setString(const char *str) {
	if (isByteArray()) {
		char *objStr = (char *)getData();
		if (str)
			strncpy(objStr, str, getCount());
		else
			objStr[0] = '\0';
	} else {
		warning("setString(%s) called on non-byte-array object (%04X)", str, getClass());
	}
}

int16 Object::getVectorItem(int16 index) const {
	if (isByteArray()) {
		const byte *vector = getData();
		return vector[index];
	} else if (isWordArray()) {
		const int16 *vector = (const int16 *)getData();
		return READ_LE_INT16(&vector[index]);
	} else {
		warning("getVectorItem(%d) called on non-vector object (%04X)", index, getClass());
		return 0;
	}
}

void Object::setVectorItem(int16 index, int16 value) {
	if (isByteArray()) {
		byte *vector = (byte *)getData();
		vector[index] = value;
	} else if (isWordArray()) {
		int16 *vector = (int16 *)getData();
		WRITE_LE_INT16(&vector[index], value);
	} else {
		warning("setVectorItem(%d, %d) called on non-vector object (%04X)", index, value, getClass());
	}
}

void Object::dump(const Common::String &filename) const {
	/*
	FILE *o = fopen(filename, "wb");
	fwrite(_objData, _objSize, 1, o);
	fclose(o);
	*/
}

// Version-specific object definitions follow
//
uint16 ObjectV1::calcSize(const byte *source) {
	uint16 type = READ_LE_UINT16(source);
	if (type == 0x7FFF) {
		// array of byte
		return 4 + READ_LE_UINT16(source + 2);
	} else if (type == 0x7FFE) {
		// array of word
		return 4 + READ_LE_UINT16(source + 2) * 2;
	} else {
		// object: sum of keys and values, each word-sized
		byte count1 = source[2];
		byte count2 = source[3];
		return 4 + (count1 + count2) * 2;
	}
}

int ObjectV1::load(Common::SeekableReadStream &source) {
	byte header[4];
	source.read(header, 4);
	uint16 objSize = calcSize(header);

	if (_objData) {
		// loading over existing object - verify size matches first!
		if (objSize != _objSize)
			error("ObjectV1::load: New object size does not match current!");
	} else {
		_objSize = objSize;
		_objData = new byte[_objSize];
		_freeData = true;
	}

	memcpy(_objData, header, 4);
	source.read(_objData + 4, _objSize - 4);

	return _objSize;
}

int ObjectV1::save(Common::WriteStream &dest) const {
	dest.write(_objData, _objSize);
	return _objSize;
}

uint16 ObjectV1::getClass() const {
	return READ_LE_UINT16(_objData);
}

uint16 ObjectV1::getCount() const {
	return READ_LE_UINT16(_objData + 2);
}

// v1 objects have the two property counts reversed from v2 / v3
byte ObjectV1::getKeyCount() const {
	return _objData[2];
}

byte ObjectV1::getValueCount() const {
	return _objData[3];
}

const byte *ObjectV1::getData() const {
	return _objData + 4;
}

//
byte ObjectV2::getKeyCount() const {
	return _objData[3];
}

byte ObjectV2::getValueCount() const {
	return _objData[2];
}

//
uint16 ObjectV3::calcSize(const byte *source) {
	// uint16 flags = READ_LE_UINT16(header);
	uint16 type = READ_LE_UINT16(source + 2);
	if (type == 0x7FFF) {
		// array of byte
		return 6 + READ_LE_UINT16(source + 4);
	} else if (type == 0x7FFE) {
		// array of word
		return 6 + READ_LE_UINT16(source + 4) * 2;
	} else {
		// object: sum of keys and values, each word-sized
		byte count1 = source[4];
		byte count2 = source[5];
		return 6 + (count1 + count2) * 2;
	}
}

int ObjectV3::load(Common::SeekableReadStream &source) {
	byte header[6];
	source.read(header, 6);
	uint16 objSize = calcSize(header);

	if (_objData) {
		// loading over existing object - verify size matches first!
		if (objSize != _objSize)
			error("ObjectV3::load: New object size does not match current!");
	} else {
		_objSize = objSize;
		_objData = new byte[_objSize];
		_freeData = true;
	}

	memcpy(_objData, header, 6);
	source.read(_objData + 6, _objSize - 6);

	return _objSize;
}

int ObjectV3::load(byte *source) {
	if (_freeData)
		delete[] _objData;

	_objSize = calcSize(source);
	_objData = source;
	_freeData = false;

	return _objSize;
}

uint16 ObjectV3::getFlags() const {
	return READ_LE_UINT16(_objData);
}

uint16 ObjectV3::getClass() const {
	return READ_LE_UINT16(_objData + 2);
}

uint16 ObjectV3::getCount() const {
	return READ_LE_UINT16(_objData + 4);
}

byte ObjectV3::getKeyCount() const {
	return _objData[5];
}

byte ObjectV3::getValueCount() const {
	return _objData[4];
}

const byte *ObjectV3::getData() const {
	return _objData + 6;
}

//
uint16 ObjectV3_1::calcSize(const byte *source) {
	// uint16 flags = READ_LE_UINT16(header);
	uint16 type = READ_LE_UINT16(source + 2);
	if (type == 0x7FFF) {
		// array of Bytes
		return 6 + READ_LE_UINT16(source + 4);
	} else if (type == 0x7FFE) {
		// array of Words
		return 6 + READ_LE_UINT16(source + 4) * 2;
	} else {
		// Object (array of Word -> Word)
		return 6 + READ_LE_UINT16(source + 4) * 4;
	}
}

/* GameDatabase */

GameDatabase::GameDatabase() : _mainCodeObjectIndex(0) {}

void GameDatabase::open(const char *filename) {
	debug(1, "GameDatabase::open() Loading from %s", filename);
	_filename = filename;
	_redFilename.clear();
	Common::File fd;
	if (!fd.open(filename))
		error("GameDatabase::open() Could not open %s", filename);
	load(fd);
	fd.close();
}

void GameDatabase::openFromRed(const char *redFilename, const char *filename) {
	debug(1, "GameDatabase::openFromRed() Loading from %s->%s", redFilename, filename);
	_filename = filename;
	_redFilename = redFilename;
	Common::SeekableReadStream *fileS = Common::RedReader::loadFromRed(redFilename, filename);
	if (!fileS)
		error("GameDatabase::openFromRed() Could not load %s from %s", filename, redFilename);
	load(*fileS);
	delete fileS;
}

void GameDatabase::reload() {
	if (_redFilename.empty()) {
		Common::File fd;
		if (!fd.open(_filename.c_str()))
			error("GameDatabase::reload() Could not open %s", _filename.c_str());
		reloadFromStream(fd);
	} else {
		Common::SeekableReadStream *fileS = Common::RedReader::loadFromRed(_redFilename.c_str(), _filename.c_str());
		if (!fileS)
			error("GameDatabase::openFromRed() Could not load %s from %s", _filename.c_str(), _redFilename.c_str());
		reloadFromStream(*fileS);
		delete fileS;
	}
}

// Variable and Object lookups are technically 1-based, but
//  we create an array with a dummy entry at idx = 0 (see GameDatabase::load())
int16 GameDatabase::getVar(int16 index) const {
	if (index < 1) {
		warning("Request to get variable %d", index);
		return 0;
	}

	return READ_LE_INT16((const int16 *)_gameState.data() + index);
}

void GameDatabase::setVar(int16 index, int16 value) {
	if (index < 1) {
		warning("Request to set variable %d", index);
		return;
	}

	WRITE_LE_INT16((int16 *)_gameState.data() + index, value);
}

Object *GameDatabase::getObject(int16 index) const {
	if (index < 1) {
		warning("Request to get object %d", index);
		return nullptr;
	}

	return _objects[index - 1];
}

const char *GameDatabase::getObjectString(int16 index) const {
	Object *obj = getObject(index);
	if (obj)
		return obj->getString();
	else
		return nullptr;
}

void GameDatabase::setObjectString(int16 index, const char *str) {
	Object *obj = getObject(index);
	if (obj)
		obj->setString(str);
}

int16 *GameDatabase::findObjectPropertyCached(int16 objectIndex, int16 propertyId, int16 &writableFlag) {
	uint32 id = (objectIndex << 16) | propertyId;
	ObjectPropertyCacheMap::iterator iter = _objectPropertyCache.find(id);
	int16 *propertyPtr = nullptr;
	if (iter != _objectPropertyCache.end()) {
		propertyPtr = iter->_value;
	} else {
		propertyPtr = findObjectProperty(objectIndex, propertyId, writableFlag);
		_objectPropertyCache[id] = propertyPtr;
	}
	writableFlag = 1;
	return propertyPtr;
}

int16 GameDatabase::getObjectProperty(int16 objectIndex, int16 propertyId) {
	if (objectIndex < 1)
		return 0;

	int16 propertyFlag;
	// int16 *property = findObjectProperty(objectIndex, propertyId, writableFlag);
	int16 *property = findObjectPropertyCached(objectIndex, propertyId, propertyFlag);

	if (property) {
		return READ_LE_INT16(property);
	} else {
		return 0;
	}

}

int16 GameDatabase::setObjectProperty(int16 objectIndex, int16 propertyId, int16 value) {
	if (objectIndex < 1)
		return 0;

	int16 writableFlag;
	// int16 *property = findObjectProperty(objectIndex, propertyId, writableFlag);
	int16 *property = findObjectPropertyCached(objectIndex, propertyId, writableFlag);

	if (property) {
		if (writableFlag == 1) {
			WRITE_LE_INT16(property, value);
		} else {
			warning("GameDatabase::setObjectProperty(%04X, %04X, %04X) Trying to set constant property",
				objectIndex, propertyId, value);
		}
		return value;
	} else {
		return 0;
	}

}

void GameDatabase::dumpObject(int16 index) const {
	Object *obj = getObject(index);
	if (obj)
		obj->dump(Common::String::format("obj%04X.0", index));
}

/* GameDatabaseV1 */

void GameDatabaseV1::load(Common::SeekableReadStream &sourceS) {
	int16 version = sourceS.readUint16LE();

	// The earlier EGA version of Manhole is version 40
	if (version != 40)
		warning("Unknown database version, known version is 40");

	char header[6];
	sourceS.read(header, 6);
	if (strncmp(header, "ADVSYS", 6))
		warning("Unexpected database header, expected ADVSYS");

	uint16 subVersion = sourceS.readUint16LE();
	char dbName[19] = "";
	sourceS.read(dbName, 18);
	debug(2, "databaseVersion = %d, databaseSubVersion = %d, databaseName = %s", version, subVersion, dbName);

	sourceS.readUint16LE(); // skip unused (always 1)
	uint16 objectCount = sourceS.readUint16LE();
	uint16 gameStateSize = sourceS.readUint16LE() * 2;
	uint32 objectsOffs = sourceS.readUint16LE() * 512;
	uint32 textOffs = sourceS.readUint16LE() * 512;
	_mainCodeObjectIndex = sourceS.readUint16LE();

	uint32 textSize = objectsOffs - textOffs;
	debug(0, "textOffs = %08X; textSize = %08X; objectCount = %d; gameStateSize = %d; objectsOffs = %08X; _mainCodeObjectIndex = %04X\n", textOffs, textSize, objectCount, gameStateSize, objectsOffs, _mainCodeObjectIndex);

	// V1 gamestate is initialized to all 0, and with a dummy element at idx = 0
	_gameState.resize(gameStateSize + 2, 0);
	setVar(1, objectCount);

	sourceS.seek(textOffs);
	_gameText.reserve(textSize);
	// Read and "decrypt" the text data
	for (uint32 i = 0; i < textSize; i++)
		_gameText.push_back(sourceS.readSByte() + 0x1E);

	sourceS.seek(objectsOffs);
	// Initialize the object array
	_objects.resize(objectCount, nullptr);
	// Read two "sections" of objects
	// It seems the first section is data while the second one is code.
	// The interpreter however doesn't care which section the objects come from.
	for (int section = 0; section < 2; section++) {
		while (!sourceS.eos()) {
			int16 objIndex = sourceS.readUint16LE();
			debug(1, "objIndex = %04X; section = %d", objIndex, section);
			if (objIndex == 0)
				break;
			ObjectV1 *obj = new ObjectV1();
			obj->load(sourceS);
			_objects[objIndex - 1] = obj;
		}
	}
}

void GameDatabaseV1::reloadFromStream(Common::SeekableReadStream &sourceS) {
	// Not used in version 1 / 2 games
}

bool GameDatabaseV1::getSavegameDescription(const char *filename, Common::String &description, int16 version) {
	// Not used in version 1 / 2 games
	return false;
}

int16 GameDatabaseV1::savegame(const char *filename, const char *description, int16 version) {
	Common::OutSaveFile *out;
	int16 result = 0;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return 6;
	}

	// Variable 0 is not saved
	out->write(_gameState.data() + 2, _gameState.size() - 2);
	for (uint i = 0; i < _objects.size(); i++) {
		int objSize = _objects[i]->save(*out);
		// objects in the original savefiles are word-aligned
		if (objSize % 2)
			out->writeByte(0);
	}
	out->finalize();
	delete out;
	return result;
}

int16 GameDatabaseV1::loadgame(const char *filename, int16 version) {
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);
	if (in == nullptr) {
		warning("Can't open file '%s', game not loaded", filename);
		return 1;
	}

	// Variable 0 is not loaded
	in->read(_gameState.data() + 2, _gameState.size() - 2);
	for (uint i = 0; i < _objects.size(); i++) {
		int objSize = _objects[i]->load(*in);
		// objects in the original savefiles are word-aligned
		if (objSize % 2)
			in->skip(1);
	}
	delete in;

	_objectPropertyCache.clear();	// make sure to clear cache
	return 0;
}

int16 *GameDatabaseV1::findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag) {

	Object *obj = getObject(objectIndex);
	if (!obj->isObject()) {
		error("GameDatabaseV2::findObjectProperty(%04X, %04X) Not an object (type=%04x)", objectIndex, propertyId, obj->getClass());
	}

	int16 *prop = (int16 *)obj->getData();
	byte count1 = obj->getValueCount();
	byte count2 = obj->getKeyCount();

	int16 *propPtr1 = prop + count1;
	int16 *propPtr2 = prop + count2;

	// First see if the property exists in the given object
	while (count2--) {
		if ((READ_LE_UINT16(prop) & 0x7FFF) == propertyId) {
			propertyFlag = 1;
			return propPtr1;
		}
		prop++;
		propPtr1++;
	}

	// Now check in the object hierarchy of the given object
	int16 parentObjectIndex = obj->getClass();
	while (parentObjectIndex != 0) {

		obj = getObject(parentObjectIndex);

		prop = (int16 *)obj->getData();
		count1 = obj->getValueCount();
		count2 = obj->getKeyCount();

		propPtr1 = propPtr2 + count1 - count2;
		int16 *propertyPtr = prop + count1;

		while (count2--) {
			if ((READ_LE_UINT16(prop) & 0x8000) == 0) {
				if ((READ_LE_UINT16(prop) & 0x7FFF) == propertyId) {
					propertyFlag = 1;
					return propPtr1;
				} else {
					propPtr1++;
				}
			} else {
				if ((READ_LE_UINT16(prop) & 0x7FFF) == propertyId) {
					propertyFlag = 1;
					return propertyPtr;
				}
			}
			prop++;
			propertyPtr++;
		}

		parentObjectIndex = obj->getClass();

	}

	debug(1, "findObjectProperty(%04X, %04X) Property not found", objectIndex, propertyId);
	return nullptr;

}

const char *GameDatabaseV1::getString(uint16 offset) {
	return (const char*)&_gameText[offset * 4];
}

/* GameDatabaseV2 */

void GameDatabaseV2::load(Common::SeekableReadStream &sourceS) {
	int16 version = sourceS.readUint16LE();

	// Manhole:NE, Rodney's Funscreen and LGOP2 are version 54
	if (version != 54)
		warning("Unknown database version, known version is 54");

	char header[6];
	sourceS.read(header, 6);
	if (strncmp(header, "ADVSYS", 6))
		warning("Unexpected database header, expected ADVSYS");

	uint32 textOffs = 0, objectsOffs = 0, objectsSize = 0, textSize;
	uint16 objectCount = 0, varObjectCount = 0;

	uint16 subVersion = sourceS.readUint16LE();
	char dbName[19] = "";
	sourceS.read(dbName, 18);
	debug(2, "databaseVersion = %d, databaseSubVersion = %d, databaseName = %s", version, subVersion, dbName);

	textOffs = sourceS.readUint16LE() * 512;
	objectCount = sourceS.readUint16LE();
	varObjectCount = sourceS.readUint16LE();
	uint16 _gameStateSize = sourceS.readUint16LE() * 2;
	sourceS.readUint16LE(); // unknown
	objectsOffs = sourceS.readUint16LE() * 512;
	sourceS.readUint16LE(); // unknown
	_mainCodeObjectIndex = sourceS.readUint16LE();
	sourceS.readUint16LE(); // unknown
	objectsSize = sourceS.readUint32LE() * 2;

	textSize = objectsOffs - textOffs;

	debug(0, "textOffs = %08X; textSize = %08X; objectCount = %d; varObjectCount = %d; gameStateSize = %d; objectsOffs = %08X; objectsSize = %d; _mainCodeObjectIndex = %04X\n", textOffs, textSize, objectCount, varObjectCount, _gameStateSize, objectsOffs, objectsSize, _mainCodeObjectIndex);

	// V2 gamestate is initialized to all 0, and with a dummy element at idx = 0
	_gameState.resize(_gameStateSize + 2, 0);
	setVar(1, objectCount);

	sourceS.seek(textOffs);
	_gameText.reserve(textSize);
	// Read and "decrypt" the text data
	for (uint32 i = 0; i < textSize; i++)
		_gameText.push_back(sourceS.readSByte() + 0x1E);

	sourceS.seek(objectsOffs);
	for (uint32 i = 0; i < objectCount; i++) {
		ObjectV2 *obj = new ObjectV2();
		int objSize = obj->load(sourceS);
		// Objects are aligned on 2-byte-boundaries, skip unused bytes
		if (objSize % 2)
			sourceS.skip(1);
		_objects.push_back(obj);
	}
}

/* GameDatabaseV3 */

GameDatabaseV3::GameDatabaseV3() : GameDatabase(), _gameStateOffs(0) {}

void GameDatabaseV3::load(Common::SeekableReadStream &sourceS) {
	char header[6];
	sourceS.read(header, 6);
	if (strncmp(header, "ADVSYS", 6))
		warning("Unexpected database header, expected ADVSYS");

	uint16 version = sourceS.readUint16LE();
	uint16 subVersion = sourceS.readUint16LE();

	// V3.00 databases are 209, V3.10 is 210
	if (version != 209 && version != 210)
		warning("Unknown database version, known versions are 209 and 210");

	char dbName[19] = "";
	sourceS.read(dbName, 18);
	debug(2, "databaseVersion = %d, databaseSubVersion = %d, databaseName = %s", version, subVersion, dbName);

	sourceS.readUint16LE(); // unknown, always 1?

	uint32 objectIndexOffs = sourceS.readUint32LE();
	uint16 objectCount = sourceS.readUint16LE();
	_gameStateOffs = sourceS.readUint32LE();
	uint32 _gameStateSize = sourceS.readUint32LE();
	uint32 objectsOffs = sourceS.readUint32LE();
	uint32 objectsSize = sourceS.readUint32LE();
	_mainCodeObjectIndex = sourceS.readUint16LE();

	debug(2, "objectIndexOffs = %08X; objectCount = %d; gameStateOffs = %08X; gameStateSize = %d; objectsOffs = %08X; objectsSize = %d; _mainCodeObjectIndex = %04X\n", objectIndexOffs, objectCount, _gameStateOffs, _gameStateSize, objectsOffs, objectsSize, _mainCodeObjectIndex);

	_gameState.resize(_gameStateSize, 0);
	reloadFromStream(sourceS);

	Common::Array<uint32> objectOffsets;
	sourceS.seek(objectIndexOffs);
	for (uint32 i = 0; i < objectCount; i++)
		objectOffsets.push_back(sourceS.readUint32LE());

	for (uint32 i = 0; i < objectCount; i++) {
		ObjectV3 *obj;
		if (version == 210)
			obj = new ObjectV3_1();
		else
			obj = new ObjectV3();
		// The LSB indicates if it's a constant or variable object.
		// Constant objects are loaded from disk, while variable objects exist
		// in the _gameState buffer.
		if (objectOffsets[i] & 1) {
			sourceS.seek(objectsOffs + (objectOffsets[i] ^ 1));
			obj->load(sourceS);
		} else {
			obj->load(_gameState.data() + objectOffsets[i]);
		}
		_objects.push_back(obj);
	}
}

void GameDatabaseV3::reloadFromStream(Common::SeekableReadStream &sourceS) {
	sourceS.seek(_gameStateOffs);
	sourceS.read(_gameState.data(), _gameState.size());

	_objectPropertyCache.clear();	// make sure to clear cache
}

bool GameDatabaseV3::getSavegameDescription(const char *filename, Common::String &description, int16 version) {
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);

	if (in == nullptr) {
		return false;
	}

	uint32 header = in->readUint32BE();
	if (header != MKTAG('S','G','A','M')) {
		warning("Save game header missing");
		delete in;
		return false;
	}

	int32 size = in->readUint32LE();
	int16 saveVersion = in->readUint16LE();

	if (saveVersion != version) {
		warning("Save game %s was saved with a different version of the game. Game version is %d, save version is %d", filename, version, saveVersion);
		delete in;
		return false;
	}

	if (size != in->size() - 64) {
		warning("Unexpected save game size. Expected %d, size is %d (file size - 64)", size, (int)in->size() - 64);
		delete in;
		return false;
	}

	char desc[64];
	in->read(desc, 64);
	description = desc;

	delete in;
	return true;
}

int16 GameDatabaseV3::savegame(const char *filename, const char *description, int16 version) {
	Common::OutSaveFile *out;
	char desc[64];
	int16 result = 0;
	uint32 size = 4 + 4 + 2 + _gameState.size();
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return 6;
	}
	Common::strlcpy(desc, description, 64);
	out->writeUint32BE(MKTAG('S','G','A','M'));
	out->writeUint32LE(size);
	out->writeUint16LE(version);
	out->write(desc, 64);
	out->write(_gameState.data(), _gameState.size());
	out->finalize();
	delete out;
	return result;
}

int16 GameDatabaseV3::loadgame(const char *filename, int16 version) {
	Common::InSaveFile *in;
	uint32 expectedSize = 4 + 4 + 2 + _gameState.size();

	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return 1;
	}

	uint32 header = in->readUint32BE();
	if (header != MKTAG('S','G','A','M')) {
		warning("Save game header missing");
		delete in;
		return 1;
	}

	uint32 size = in->readUint32LE();
	int16 saveVersion = in->readUint16LE();

	if (saveVersion != version) {
		warning("Save game %s was saved with a different version of the game. Game version is %d, save version is %d", filename, version, saveVersion);
		delete in;
		return 1;
	}

	if (size != expectedSize) {
		warning("Unexpected save game size. Expected %d, size is %d", expectedSize, size);
		delete in;
		return 1;
	}

	in->skip(64); // skip savegame description
	in->read(_gameState.data(), _gameState.size());
	delete in;

	_objectPropertyCache.clear();	// make sure to clear cache

	return 0;
}

int16 *GameDatabaseV3::findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag) {
	Object *obj = getObject(objectIndex);
	if (obj->getClass() >= 0x7FFE) {
		error("GameDatabaseV3::findObjectProperty(%04X, %04X) Not an object (type=%04X)", objectIndex, propertyId, obj->getClass());
	}

	int16 *prop = (int16 *)obj->getData();
	byte count1 = obj->getValueCount();
	byte count2 = obj->getKeyCount();

	int16 *propPtr1 = prop + count1;
	int16 *propPtr2 = prop + count2;

	// First see if the property exists in the given object
	while (count2-- > 0) {
		if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
			if (READ_LE_UINT16(prop) & 0x4000) {
				propertyFlag = 1;
				return (int16 *)_gameState.data() + READ_LE_UINT16(propPtr1);
			} else {
				propertyFlag = obj->isWritable();
				return propPtr1;
			}
		}
		prop++;
		propPtr1++;
	}

	// Now check in the object hierarchy of the given object
	int16 parentObjectIndex = obj->getClass();
	while (parentObjectIndex != 0) {

		obj = getObject(parentObjectIndex);

		prop = (int16 *)obj->getData();
		count1 = obj->getValueCount();
		count2 = obj->getKeyCount();

		propPtr1 = propPtr2 + count1 - count2;
		int16 *propertyPtr = prop + count1;

		while (count2-- > 0) {
			if (!(READ_LE_UINT16(prop) & 0x8000)) {
				if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
					if (READ_LE_UINT16(prop) & 0x4000) {
						propertyFlag = 1;
						return (int16 *)_gameState.data() + READ_LE_UINT16(propPtr1);
					} else {
						propertyFlag = obj->isWritable();
						return propPtr1;
					}
				} else {
					propPtr1++;
				}
			} else {
				if ((READ_LE_UINT16(prop) & 0x3FFF) == propertyId) {
					if (READ_LE_UINT16(prop) & 0x4000) {
						propertyFlag = 1;
						return (int16 *)_gameState.data() + READ_LE_UINT16(propertyPtr);
					} else {
						propertyFlag = obj->isWritable();
						return propertyPtr;
					}
				}
			}
			prop++;
			propertyPtr++;
		}

		parentObjectIndex = obj->getClass();

	}

	debug(1, "findObjectProperty(%04X, %04X) Property not found", objectIndex, propertyId);

	return nullptr;

}

const char *GameDatabaseV3::getString(uint16 offset) {
	// Not used in version 3 games
	return nullptr;
}

int16 *GameDatabaseV3_1::findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag) {

	debug(4, "findObjectProperty(%04X, %04X): Beginning search...", objectIndex, propertyId);

	do {
		Object *obj = getObject(objectIndex);

		if (obj->getClass() >= 0x7FFE) {
			error("GameDatabaseV3::findObjectProperty(%04X, %04X) Not an object (type=%04X)", objectIndex, propertyId, obj->getClass());
		}

		// 3.1 objects are a set of int16 (key) -> int16 (value) entries
		uint16 count = obj->getCount();

		// Check each "key" on the object for a match.
		while (count-- > 0) {

			int16 *prop = (int16 *)obj->getData() + (2 * count);
			uint16 readProp = READ_LE_UINT16(prop);

			if ((readProp & 0x3FFF) == propertyId) {
				if (readProp & 0x4000) {
					// A match w/ high-bit set indicates you go look in the Gamestate.
					//  "Value" is an index into the gamestate... return a ptr into gamestate.
					propertyFlag = 1;
					return (int16 *)_gameState.data() + READ_LE_UINT16(prop + 1);
				} else {
					// A match without high-bit indicates the value is right here (literal).
					//  Return a ptr to the value.
					propertyFlag = obj->isWritable();
					return (prop + 1);
				}
			}
			prop++;
		}

		// Objects are a hierarchy: if no match is found on the current object,
		//  check its "class" (parent).
		objectIndex = obj->getClass();
	} while (objectIndex != 0);

	debug(1, "findObjectProperty(%04X, %04X) Property not found", objectIndex, propertyId);

	return nullptr;
}

} // End of namespace Made
