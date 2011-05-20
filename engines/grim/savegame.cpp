/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/endian.h"
#include "common/system.h"

#include "graphics/vector3d.h"

#include "engines/grim/savegame.h"
#include "engines/grim/color.h"

namespace Grim {

#define SAVEGAME_HEADERTAG	'RSAV'
#define SAVEGAME_FOOTERTAG	'ESAV'

int SaveGame::SAVEGAME_VERSION = 14;

// Constructor. Should create/open a saved game
SaveGame::SaveGame(const char *filename, bool saving) :
		_saving(saving), _currentSection(0) {
	if (_saving) {
		_outSaveFile = g_system->getSavefileManager()->openForSaving(filename);
		if (!_outSaveFile) {
			warning("SaveGame::SaveGame() Error creating savegame file");
			return;
		}
		_outSaveFile->writeUint32BE(SAVEGAME_HEADERTAG);
		_outSaveFile->writeUint32BE(SAVEGAME_VERSION);

		_version = SAVEGAME_VERSION;
	} else {
		uint32 tag;

		_inSaveFile = g_system->getSavefileManager()->openForLoading(filename);
		if (!_inSaveFile) {
			warning("SaveGame::SaveGame() Error opening savegame file");
			return;
		}
		tag = _inSaveFile->readUint32BE();
		assert(tag == SAVEGAME_HEADERTAG);
		_version = _inSaveFile->readUint32BE();
	}
}

SaveGame::~SaveGame() {
	if (_saving) {
		_outSaveFile->writeUint32BE(SAVEGAME_FOOTERTAG);
		_outSaveFile->finalize();
		if (_outSaveFile->err())
			warning("SaveGame::~SaveGame() Can't write file. (Disk full?)");
		delete _outSaveFile;
	} else {
		delete _inSaveFile;
	}
}

int SaveGame::saveVersion() const {
	return _version;
}

uint32 SaveGame::beginSection(uint32 sectionTag) {
	assert(_version == SAVEGAME_VERSION);

	if (_currentSection != 0)
		error("Tried to begin a new save game section with ending old section");
	_currentSection = sectionTag;
	_sectionSize = 0;
	_sectionAlloc = _allocAmmount;
	if (!_saving) {
		uint32 tag = 0;

		while (tag != sectionTag) {
			tag = _inSaveFile->readUint32BE();
			if (tag == SAVEGAME_FOOTERTAG)
				error("Unable to find requested section of savegame");
			_sectionSize = _inSaveFile->readUint32BE();
			_inSaveFile->seek(_sectionSize, SEEK_CUR);
		}
		_sectionBuffer = (byte *)malloc(_sectionSize);
		_inSaveFile->seek(-(int32)_sectionSize, SEEK_CUR);
		_inSaveFile->read(_sectionBuffer, _sectionSize);

	} else {
		_sectionBuffer = (byte *)malloc(_sectionAlloc);
	}
	_sectionPtr = 0;
	return _sectionSize;
}

void SaveGame::endSection() {
	if (_currentSection == 0)
		error("Tried to end a save game section without starting a section");
	if (_saving) {
		_outSaveFile->writeUint32BE(_currentSection);
		_outSaveFile->writeUint32BE(_sectionSize);
		_outSaveFile->write(_sectionBuffer, _sectionSize);
	}
	free(_sectionBuffer);
	_sectionBuffer = NULL;
	_currentSection = 0;
}

uint32 SaveGame::getBufferPos() {
	if (_saving)
		return _sectionSize;
	else
		return _sectionPtr;
}

void SaveGame::read(void *data, int size) {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame");
	if (_currentSection == 0)
		error("Tried to read a block without starting a section");
	memcpy(data, &_sectionBuffer[_sectionPtr], size);
	_sectionPtr += size;
}

uint32 SaveGame::readLEUint32() {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame");
	if (_currentSection == 0)
		error("Tried to read a block without starting a section");
	uint32 data = READ_LE_UINT32(&_sectionBuffer[_sectionPtr]);
	_sectionPtr += 4;
	return data;
}

uint16 SaveGame::readLEUint16() {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame");
	if (_currentSection == 0)
		error("Tried to read a block without starting a section");
	uint16 data = READ_LE_UINT16(&_sectionBuffer[_sectionPtr]);
	_sectionPtr += 2;
	return data;
}

int32 SaveGame::readLESint32() {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame");
	if (_currentSection == 0)
		error("Tried to read a block without starting a section");
	int32 data = (int32)READ_LE_UINT32(&_sectionBuffer[_sectionPtr]);
	_sectionPtr += 4;
	return data;
}

byte SaveGame::readByte() {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame");
	if (_currentSection == 0)
		error("Tried to read a block without starting a section");
	byte data = _sectionBuffer[_sectionPtr];
	_sectionPtr++;
	return data;
}

bool SaveGame::readLEBool() {
	if (_saving)
		error("SaveGame::readBlock called when storing a savegame");
	if (_currentSection == 0)
		error("Tried to read a block without starting a section");
	uint32 data = READ_LE_UINT32(&_sectionBuffer[_sectionPtr]);
	_sectionPtr += 4;
	return data != 0;
}

void SaveGame::checkAlloc(int size) {
	if (_sectionSize + size > _sectionAlloc) {
		while (_sectionSize + size > _sectionAlloc)
			_sectionAlloc += _allocAmmount;
		_sectionBuffer = (byte *)realloc(_sectionBuffer, _sectionAlloc);
		if (!_sectionBuffer)
			error("Failed to allocate space for buffer");
	}
}

void SaveGame::write(const void *data, int size) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame");
	if (_currentSection == 0)
		error("Tried to write a block without starting a section");

	checkAlloc(size);

	memcpy(&_sectionBuffer[_sectionSize], data, size);
	_sectionSize += size;
}

void SaveGame::writeLEUint32(uint32 data) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame");
	if (_currentSection == 0)
		error("Tried to write a block without starting a section");

	checkAlloc(4);

	WRITE_LE_UINT32(&_sectionBuffer[_sectionSize], data);
	_sectionSize += 4;
}

void SaveGame::writeLEUint16(uint16 data) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame");
	if (_currentSection == 0)
		error("Tried to write a block without starting a section");

	checkAlloc(2);

	WRITE_LE_UINT16(&_sectionBuffer[_sectionSize], data);
	_sectionSize += 2;
}

void SaveGame::writeLESint32(int32 data) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame");
	if (_currentSection == 0)
		error("Tried to write a block without starting a section");

	checkAlloc(4);

	WRITE_LE_UINT32(&_sectionBuffer[_sectionSize], (uint32)data);
	_sectionSize += 4;
}

void SaveGame::writeLEBool(bool data) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame");
	if (_currentSection == 0)
		error("Tried to write a block without starting a section");

	checkAlloc(4);

	WRITE_LE_UINT32(&_sectionBuffer[_sectionSize], (uint32)data);
	_sectionSize += 4;
}

void SaveGame::writeByte(byte data) {
	if (!_saving)
		error("SaveGame::writeBlock called when restoring a savegame");
	if (_currentSection == 0)
		error("Tried to write a block without starting a section");

	checkAlloc(1);

	_sectionBuffer[_sectionSize] = data;
	_sectionSize++;
}

void SaveGame::writeVector3d(const Graphics::Vector3d &vec) {
	writeFloat(vec.x());
	writeFloat(vec.y());
	writeFloat(vec.z());
}

void SaveGame::writeColor(const Grim::Color &color) {
	writeByte(color.getRed());
	writeByte(color.getGreen());
	writeByte(color.getBlue());
}

void SaveGame::writeFloat(float data) {
	byte *udata = (byte *)(&data);
	uint32 v;
#if defined(SCUMM_LITTLE_ENDIAN)
	byte b[4];
	b[0] = udata[3];
	b[1] = udata[2];
	b[2] = udata[1];
	b[3] = udata[0];
	v = *(uint32 *)b;
#else
	memcpy(&v, udata, 4);
#endif
	writeLEUint32(v);
}

void SaveGame::writeCharString(const char *string) {
	int32 len = strlen(string);
	writeLESint32(len);
	write(string, len);
}

void SaveGame::writeString(const Common::String &string) {
	writeCharString(string.c_str());
}

Graphics::Vector3d SaveGame::readVector3d() {
	float x = readFloat();
	float y = readFloat();
	float z = readFloat();
	return Graphics::Vector3d(x, y, z);
}

Grim::Color SaveGame::readColor() {
	Color color;
	color.getRed() = readByte();
	color.getGreen() = readByte();
	color.getBlue() = readByte();

	return color;
}

float SaveGame::readFloat() {
	float f;
	byte *udata = (byte *)(&f);
	uint32 v = readLEUint32();
#if defined(SCUMM_LITTLE_ENDIAN)
	byte b[4];
	*(uint32 *)&b = v;
	udata[0] = b[3];
	udata[1] = b[2];
	udata[2] = b[1];
	udata[3] = b[0];
#else
	memcpy(udata, &v, 4);
#endif

	return f;
}

const char *SaveGame::readCharString() {
	int32 len = readLESint32();
	char *str = new char[len + 1];
	read(str, len);
	str[len] = '\0';

	return str;
}

Common::String SaveGame::readString() {
	const char *str = readCharString();
	Common::String s = str;
	delete[] str;
	return s;
}

} // end of namespace Grim
