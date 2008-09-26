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
#include "common/savefile.h"

#include "gob/gob.h"
#include "gob/saveload.h"
#include "gob/draw.h"

namespace Gob {

TempSprite::TempSprite() {
	_sprite = 0;
	_width = _height = 0;
	_size = -1;
	memset(_palette, 0, 768);
}

TempSprite::~TempSprite() {
	delete[] _sprite;
}

int TempSprite::getSpriteIndex(int32 size) const {
	if (size < -1000)
		size += 1000;

	return -size - 1;
}

bool TempSprite::getSpritePalette(int32 size) const {
	return size < -1000;
}

bool TempSprite::getProperties(int16 dataVar, int32 size, int32 offset,
		int &index, bool &palette) const {

	if (size >= 0) {
		warning("Invalid index (%d)", size);
		return false;
	}

	index = getSpriteIndex(size);
	palette = getSpritePalette(size);

	if ((index < 0) || (index >= SPRITES_COUNT)) {
		warning("Index out of range (%d)", index);
		return false;
	}

	return true;
}

int32 TempSprite::getSize() const {
	return _size;
}

bool TempSprite::saveSprite(const SurfaceDesc &surfDesc) {
	delete[] _sprite;

	_width = surfDesc.getWidth();
	_height = surfDesc.getHeight();
	_size = _width * _height;
	_sprite = new byte[_size];

	memcpy(_sprite, surfDesc.getVidMem(), _size);

	return true;
}

bool TempSprite::savePalette(const Video::Color *palette) {
	memcpy((byte *) _palette, (byte *) palette, 768);
	return true;
}

bool TempSprite::loadSprite(SurfaceDesc &surfDesc) {
	if (!_sprite) {
		warning("No sprite saved");
		return false;
	}

	if (_size != (surfDesc.getWidth() * surfDesc.getHeight())) {
		warning("Dimensions don't match (%dx%d - %dx%d",
				_width, _height, surfDesc.getWidth(), surfDesc.getHeight());
		return false;
	}

	memcpy(surfDesc.getVidMem(), _sprite, _size);

	return true;
}

bool TempSprite::loadPalette(Video::Color *palette) {
	memcpy((byte *) palette, (byte *) _palette, 768);
	return true;
}

bool TempSprite::toBuffer(byte *buffer, int32 size, bool palette) const {

	int32 haveSize = _size + (palette ? 768 : 0);
	if (size != haveSize) {
		warning("Sizes don't match (%d != %d)", size, haveSize);
		return false;
	}

	if (palette) {
		memcpy(buffer, (byte *) _palette, 768);
		buffer += 768;
	}

	memcpy(buffer, _sprite, _size);

	return true;
}

bool TempSprite::fromBuffer(const byte *buffer, int32 size, bool palette) {
	if (palette) {
		memcpy((byte *) _palette, buffer, 768);
		buffer += 768;
		size -= 768;
	}

	_size = size;

	delete[] _sprite;
	_sprite = new byte[_size];

	memcpy(_sprite, buffer, _size);

	return true;
}


PlainSave::PlainSave(Endianness endianness) : _endianness(endianness) {
}

PlainSave::~PlainSave() {
}

bool PlainSave::save(int16 dataVar, int32 size, int32 offset, const char *name,
		const Variables *variables) {

	if ((size <= 0) || (offset != 0)) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	byte *vars = new byte[size];
	byte *varSizes = new byte[size];

	if (!variables->copyTo(dataVar, vars, varSizes, size)) {
		delete[] vars;
		delete[] varSizes;
		warning("dataVar (%d) or size (%d) out of range", dataVar, size);
		return false;
	}

	bool result = save(0, size, offset, name, vars, varSizes);

	delete[] vars;
	delete[] varSizes;

	return result;
}

bool PlainSave::load(int16 dataVar, int32 size, int32 offset, const char *name,
		Variables *variables) {

	if ((size <= 0) || (offset != 0)) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	byte *vars = new byte[size];
	byte *varSizes = new byte[size];

	bool result = load(0, size, offset, name, vars, varSizes);

	if (result && variables) {
		if (!variables->copyFrom(dataVar, vars, varSizes, size)) {
			delete[] vars;
			delete[] varSizes;
			warning("dataVar (%d) or size (%d) out of range", dataVar, size);
			return false;
		}
	}

	delete[] vars;
	delete[] varSizes;

	return result;
}

bool PlainSave::save(int16 dataVar, int32 size, int32 offset, const char *name,
		const byte *variables, const byte *variableSizes) const {

	if ((size <= 0) || (offset != 0)) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *out = saveMan->openForSaving(name);

	if (!out) {
		warning("Can't open file \"%s\" for writing", name);
		return false;
	}

	bool retVal;
	retVal = SaveLoad::saveDataEndian(*out, dataVar, size,
			variables, variableSizes, _endianness);

	out->finalize();
	if (out->ioFailed()) {
		warning("Can't write to file \"%s\"", name);
		retVal = false;
	}

	delete out;
	return retVal;
}

bool PlainSave::load(int16 dataVar, int32 size, int32 offset, const char *name,
		byte *variables, byte *variableSizes) const {

	if ((size <= 0) || (offset != 0)) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in = saveMan->openForLoading(name);

	if (!in) {
		warning("Can't open file \"%s\" for reading", name);
		return false;
	}

	bool retVal = SaveLoad::loadDataEndian(*in, dataVar, size,
			variables, variableSizes, _endianness);
	delete in;
	return retVal;
}


StagedSave::StagedSave(Endianness endianness) : _endianness(endianness) {
	_mode = kModeNone;
	_name = 0;
	_loaded = false;
}

StagedSave::~StagedSave() {
	clear();
}

void StagedSave::addStage(int32 size, bool endianed) {
	int32 offset = 0;

	if (!_stages.empty())
		offset = _stages[_stages.size() - 1].offset +
		         _stages[_stages.size() - 1].size;

	Stage stage(size, offset, endianed);
	_stages.push_back(stage);
}

int StagedSave::findStage(int16 dataVar, int32 size, int32 offset) const {
	for (uint i = 0; i < _stages.size(); i++)
		if ((_stages[i].size == size) &&
		    (_stages[i].offset == offset))
			return i;

	return -1;
}

bool StagedSave::allSaved() const {
	for (uint i = 0; i < _stages.size(); i++)
		if (!_stages[i].bufVar)
			return false;

	return true;
}

uint32 StagedSave::getSize() const {
	uint32 size = 0;

	for (uint i = 0; i < _stages.size(); i++) {
		if (_stages[i].endianed)
			size += 2 * _stages[i].size;
		else
			size += _stages[i].size;
	}
	
	return size;
}

void StagedSave::clear() {
	for (uint i = 0; i < _stages.size(); i++) {
		delete[] _stages[i].bufVar;
		delete[] _stages[i].bufVarSizes;
		_stages[i].bufVar = 0;
		_stages[i].bufVarSizes = 0;
	}

	delete[] _name;
	_name = 0;

	_mode = kModeNone;
	_loaded = false;
}

void StagedSave::assertMode(Mode mode, const char *name) {
	if ((_mode != mode) || ((name[0] != '\0') && strcmp(_name, name))) {
		clear();
		_mode = mode;
		_name = new char[strlen(name) + 1];
		strcpy(_name, name);
	}
}

bool StagedSave::save(int16 dataVar, int32 size, int32 offset, const char *name,
		const Variables *variables) {

	if ((dataVar < 0) || (size <= 0) || (offset < 0)) {
		warning("Invalid dataVar (%d), size (%d) or offset (%d)", dataVar, size, offset);
		return false;
	}

	byte *vars = 0, *varSizes = 0;

	if (variables) {
		vars = new byte[size];
		varSizes = new byte[size];

		if (!variables->copyTo(dataVar, vars, varSizes, size)) {
			delete[] vars;
			delete[] varSizes;
			warning("dataVar (%d) or size (%d) out of range", dataVar, size);
			return false;
		}
	}

	bool result = save(0, size, offset, name, vars, varSizes);

	delete[] vars;
	delete[] varSizes;

	return result;
}

bool StagedSave::load(int16 dataVar, int32 size, int32 offset, const char *name,
		Variables *variables) {

	if ((dataVar < 0) || (size <= 0) || (offset < 0)) {
		warning("Invalid dataVar (%d), size (%d) or offset (%d)", dataVar, size, offset);
		return false;
	}

	byte *vars = new byte[size];
	byte *varSizes = new byte[size];

	bool result = load(0, size, offset, name, vars, varSizes);

	if (result && variables) {
		if (!variables->copyFrom(dataVar, vars, varSizes, size)) {
			delete[] vars;
			delete[] varSizes;
			warning("dataVar (%d) or size (%d) out of range", dataVar, size);
			return false;
		}
	}

	delete[] vars;
	delete[] varSizes;

	return result;
}

bool StagedSave::save(int16 dataVar, int32 size, int32 offset, const char *name,
		const byte *variables, const byte *variableSizes) {

	if ((dataVar < 0) || (size <= 0) || (offset < 0)) {
		warning("Invalid dataVar (%d), size (%d) or offset (%d)", dataVar, size, offset);
		return false;
	}

	int stage = findStage(dataVar, size, offset);
	if (stage == -1) {
		warning("Invalid saving procedure");
		return false;
	}

	if (!variables || (_stages[stage].endianed && !variableSizes)) {
		warning("Missing data");
		return false;
	}

	assertMode(kModeSave, name);

	_stages[stage].bufVar = new byte[size];
	memcpy(_stages[stage].bufVar, variables + dataVar, size);

	if (_stages[stage].endianed) {
		_stages[stage].bufVarSizes = new byte[size];
		memcpy(_stages[stage].bufVarSizes, variableSizes + dataVar, size);
	}

	if (allSaved()) {
		bool result = write();
		clear();
		return result;
	}

	return true;
}

bool StagedSave::load(int16 dataVar, int32 size, int32 offset, const char *name,
		byte *variables, byte *variableSizes) {

	if ((dataVar < 0) || (size <= 0) || (offset < 0)) {
		warning("Invalid dataVar (%d), size (%d) or offset (%d)", dataVar, size, offset);
		return false;
	}

	int stage = findStage(dataVar, size, offset);
	if (stage == -1) {
		warning("Invalid loading procedure");
		return false;
	}

	assertMode(kModeLoad, name);

	if (!_loaded) {
		if (!read()) {
			clear();
			return false;
		}
	}

	if (variables)
		memcpy(variables + dataVar, _stages[stage].bufVar, size);
	if (_stages[stage].endianed && variableSizes)
		memcpy(variableSizes + dataVar, _stages[stage].bufVarSizes, size);

	return true;
}

bool StagedSave::write() const {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *out = saveMan->openForSaving(_name);

	if (!out) {
		warning("Can't open file \"%s\" for writing", _name);
		return false;
	}

	bool result = true;
	for (uint i = 0; (i < _stages.size()) && result; i++) {
		if (!_stages[i].endianed) {

			uint32 written = out->write(_stages[i].bufVar, _stages[i].size);

			result = (written == ((uint32) _stages[i].size));
			if (!result)
				warning("Can't write data: requested %d, wrote %d", _stages[i].size, written);

		} else
			result = SaveLoad::saveDataEndian(*out, 0, _stages[i].size,
					_stages[i].bufVar, _stages[i].bufVarSizes, _endianness);
	}

	if (result) {
		out->finalize();
		if (out->ioFailed()) {
			warning("Can't write to file \"%s\"", _name);
			result = false;
		}
	}

	delete out;
	return result;
}

bool StagedSave::read() {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in = saveMan->openForLoading(_name);

	if (!in) {
		warning("Can't open file \"%s\" for reading", _name);
		return false;
	}

	int32 saveSize = getSize();
	if (in->size() != saveSize) {
		warning("Wrong size (%d != %d)", in->size(), saveSize);
		return false;
	}

	bool result = true;
	for (uint i = 0; ((i < _stages.size()) && result); i++) {
		_stages[i].bufVar = new byte[_stages[i].size];

		if (!_stages[i].endianed) {

			uint32 nRead = in->read(_stages[i].bufVar, _stages[i].size);

			result = (nRead == ((uint32) _stages[i].size));
			if (!result)
				warning("Can't read data: requested %d, got %d", _stages[i].size, nRead);

		} else {
			_stages[i].bufVarSizes = new byte[_stages[i].size];

			result = SaveLoad::loadDataEndian(*in, 0, _stages[i].size,
					_stages[i].bufVar, _stages[i].bufVarSizes, _endianness);
		}
	}

	if (result)
		_loaded = true;

	delete in;
	return result;
}


PagedBuffer::PagedBuffer(uint32 pageSize) {

	_size = 0;
	_pageSize = pageSize;
}

PagedBuffer::~PagedBuffer() {
	clear();
}

bool PagedBuffer::empty() const {
	return _pages.empty();
}

uint32 PagedBuffer::getSize() const {
	return _size;
}

void PagedBuffer::clear() {
	for (uint i = 0; i < _pages.size(); i++)
		delete[] _pages[i];
	_pages.clear();
	_size = 0;
}

bool PagedBuffer::write(const byte *buffer, uint32 size, uint32 offset) {
	grow(size, offset);

	uint page = offset / _pageSize;
	while (size > 0) {
		if (!_pages[page])
			_pages[page] = new byte[_pageSize];

		uint32 pStart = offset % _pageSize;
		uint32 n = MIN(size, _pageSize - pStart);

		memcpy(_pages[page] + pStart, buffer, n);

		buffer += n;
		offset += n;
		size -= n;
		page++;
	}

	return true;
}

bool PagedBuffer::read(byte *buffer, uint32 size, uint32 offset) const {
	uint page = offset / _pageSize;

	while (size > 0) {
		if (offset >= _size) {
			memset(buffer, 0, size);
			break;
		}

		uint32 pStart = offset % _pageSize;
		uint32 n = MIN(MIN(size, _pageSize - pStart), _size - offset);

		if (_pages[page])
			memcpy(buffer, _pages[page] + pStart, n);
		else
			memset(buffer, 0, n);

		buffer += n;
		offset += n;
		size -= n;
		page++;
	}

	return true;
}

uint32 PagedBuffer::writeToStream(Common::WriteStream &out) const {
	for (uint i = 0; i < _pages.size(); i++) {
		if (!_pages[i]) {
			for (uint32 j = 0; j < _pageSize; j++)
				out.writeByte(0);
		} else
			out.write(_pages[i], _pageSize);
	}

	return _size;
}

uint32 PagedBuffer::readFromStream(Common::ReadStream &in) {
	clear();

	while (!in.eos()) {
		byte *buffer = new byte[_pageSize];

		_size += in.read(buffer, _pageSize);

		_pages.push_back(buffer);
	}

	return _size;
}

void PagedBuffer::grow(uint32 size, uint32 offset) {
	uint32 eSize = offset + size;

	while (_size < eSize) {
		_pages.push_back(0);
		_size += MIN(_pageSize, eSize - _size);
	}
}


SaveLoad::SaveLoad(GobEngine *vm, const char *targetName) : _vm(vm) {

	_targetName = new char[strlen(targetName) + 1];
	strcpy(_targetName, targetName);
}

SaveLoad::~SaveLoad() {
	delete[] _targetName;
}

int32 SaveLoad::getSize(const char *fileName) {
	int type;

	type = getSaveType(stripPath(fileName));
	if (type == -1)
		return -1;

	debugC(3, kDebugSaveLoad, "Requested size of save file \"%s\" (type %d)",
			fileName, type);

	return getSizeVersioned(type);
}

bool SaveLoad::load(const char *fileName, int16 dataVar, int32 size, int32 offset) {
	int type;

	type = getSaveType(stripPath(fileName));
	if (type == -1)
		return false;

	debugC(3, kDebugSaveLoad, "Requested loading of save file \"%s\" (type %d) - %d, %d, %d",
			fileName, type, dataVar, size, offset);

	return loadVersioned(type, dataVar, size, offset);
}

bool SaveLoad::save(const char *fileName, int16 dataVar, int32 size, int32 offset) {
	int type;

	type = getSaveType(stripPath(fileName));
	if (type == -1)
		return false;

	debugC(3, kDebugSaveLoad, "Requested saving of save file \"%s\" (type %d) - %d, %d, %d",
			fileName, type, dataVar, size, offset);

	return saveVersioned(type, dataVar, size, offset);
}

const char *SaveLoad::stripPath(const char *fileName) {
	const char *backSlash;
	if ((backSlash = strrchr(fileName, '\\')))
		return backSlash + 1;

	return fileName;
}

char *SaveLoad::setCurrentSlot(char *destName, int slot) {
	char *slotBase = destName + strlen(destName) - 2;

	snprintf(slotBase, 3, "%02d", slot);

	return destName;
}

void SaveLoad::buildIndex(byte *buffer, char *name, int n, int32 size, int32 offset) {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	for (int i = 0; i < n; i++, buffer += size) {
		in = saveMan->openForLoading(setCurrentSlot(name, i));
		if (in) {
			in->seek(offset);
			in->read(buffer, size);
			delete in;
		} else
			memset(buffer, 0, size);
	}
}

bool SaveLoad::fromEndian(byte *buf, const byte *sizes, uint32 count, Endianness endianness) {
	bool LE = (endianness == kEndiannessLE);

	while (count-- > 0) {
		if (*sizes == 3)
			*((uint32 *) buf) = LE ? READ_LE_UINT32(buf) : READ_BE_UINT32(buf);
		else if (*sizes == 1)
			*((uint16 *) buf) = LE ? READ_LE_UINT16(buf) : READ_BE_UINT16(buf);
		else if (*sizes != 0) {
			warning("SaveLoad::fromEndian(): Corrupted variables sizes");
			return false;
		}

		count -= *sizes;
		buf += *sizes + 1;
		sizes += *sizes + 1;
	}

	return true;
}

bool SaveLoad::toEndian(byte *buf, const byte *sizes, uint32 count, Endianness endianness) {
	while (count-- > 0) {
		if (*sizes == 3) {
			if (endianness == kEndiannessLE)
				WRITE_LE_UINT32(buf, *((uint32 *) buf));
			else
				WRITE_BE_UINT32(buf, *((uint32 *) buf));
		} else if (*sizes == 1) {
			if (endianness == kEndiannessLE)
				WRITE_LE_UINT16(buf, *((uint16 *) buf));
			else
				WRITE_BE_UINT16(buf, *((uint16 *) buf));
		}
		else if (*sizes != 0) {
			warning("SaveLoad::toEndian(): Corrupted variables sizes");
			return false;
		}

		count -= *sizes;
		buf += *sizes + 1;
		sizes += *sizes + 1;
	}

	return true;
}

uint32 SaveLoad::read(Common::ReadStream &in,
		byte *buf, byte *sizes, uint32 count) {
	uint32 nRead;

	nRead = in.read(buf, count);
	if (nRead != count) {
		warning("Can't read data: requested %d, got %d", count, nRead);
		return 0;
	}

	nRead = in.read(sizes, count);
	if (nRead != count) {
		warning("Can't read data sizes: requested %d, got %d", count, nRead);
		return 0;
	}

	return count;
}

uint32 SaveLoad::write(Common::WriteStream &out,
		const byte *buf, const byte *sizes, uint32 count) {
	uint32 written;

	written = out.write(buf, count);
	if (written != count) {
		warning("Can't write data: requested %d, wrote %d", count, written);
		return 0;
	}

	written = out.write(sizes, count);
	if (written != count) {
		warning("Can't write data: requested %d, wrote %d", count, written);
		return 0;
	}

	return count;
}

bool SaveLoad::loadDataEndian(Common::ReadStream &in,
		int16 dataVar, uint32 size,
		byte *variables, byte *variableSizes, Endianness endianness) {

	bool retVal = false;

	byte *varBuf = new byte[size];
	byte *sizeBuf = new byte[size];

	assert(varBuf && sizeBuf);

	if (read(in, varBuf, sizeBuf, size) == size) {
		if (fromEndian(varBuf, sizeBuf, size, endianness)) {
			memcpy(variables + dataVar, varBuf, size);
			memcpy(variableSizes + dataVar, sizeBuf, size);
			retVal = true;
		}
	}

	delete[] varBuf;
	delete[] sizeBuf;

	return retVal;
}

bool SaveLoad::saveDataEndian(Common::WriteStream &out,
		int16 dataVar, uint32 size,
		const byte *variables, const byte *variableSizes, Endianness endianness) {

	bool retVal = false;

	byte *varBuf = new byte[size];
	byte *sizeBuf = new byte[size];

	assert(varBuf && sizeBuf);

	memcpy(varBuf, variables + dataVar, size);
	memcpy(sizeBuf, variableSizes + dataVar, size);

	if (toEndian(varBuf, sizeBuf, size, endianness))
		if (write(out, varBuf, sizeBuf, size) == size)
			retVal = true;

	delete[] varBuf;
	delete[] sizeBuf;

	return retVal;
}

} // End of namespace Gob
