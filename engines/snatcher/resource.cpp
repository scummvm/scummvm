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

#include "snatcher/resource.h"
#include "snatcher/snatcher.h"
#include "snatcher/scene.h"

#include "common/stream.h"

namespace Snatcher {

class EndianWrapper : public Common::SeekableReadStreamEndian {
public:
	EndianWrapper(Common::SeekableReadStream *stream, bool bigEndian, bool disposeAfterUse = true) : Common::SeekableReadStreamEndian(bigEndian), Common::ReadStreamEndian(bigEndian), _stream(stream), _dispose(disposeAfterUse) {}
	~EndianWrapper() override { if (_dispose) delete _stream; }

	// Common::Stream interface
	bool err() const override { return _stream->err(); }

	// Common::ReadStream interface
	bool eos() const override { return _stream->eos(); }
	uint32 read(void *dataPtr, uint32 dataSize) override { return _stream->read(dataPtr, dataSize); }

	// Common::SeekableReadStream interface
	int64 pos() const override { return _stream->pos(); }
	int64 size() const override { return _stream->size(); }
	bool seek(int64 offset, int whence = SEEK_SET) override { return _stream->seek(offset, whence); }

private:
	Common::SeekableReadStream *_stream;
	bool _dispose;
};

FIO::FIO(SnatcherEngine *vm, bool isBigEndian) : _vm(vm), _bigEndianTarget(isBigEndian) {
	_files.add("global_search", &SearchMan, 0, false);
}

FIO::~FIO() {
}

SceneModule *FIO::loadModule(int index) {
	return (index >= 0 && index < _resFileListSize) ? new SceneModule(_vm, this, _resFileList[index], index) : 0;
}

uint8 *FIO::fileData(int index, uint32 *fileSize) {
	return (index >= 0 && index < _resFileListSize) ? fileData(_resFileList[index], fileSize) : nullptr;
}

uint8 *FIO::fileData(const Common::Path &file, uint32 *fileSize) {
	debug("%s(): Loading file %s", __FUNCTION__, file.baseName().c_str());
	Common::SeekableReadStream *s = readStream(file);

	uint32 size = s ? s->size() : 0;
	if (fileSize)
		*fileSize = size;

	if (!s) {
		warning("%s(): Unable to open file %s", __FUNCTION__, file.baseName().c_str());
		return nullptr;
	}

	uint8 *data = new uint8[size];
	s->read(data, size);
	delete s;

	return data;
}

Common::SeekableReadStream *FIO::readStream(int index) {
	return (index >= 0 && index < _resFileListSize) ? readStream(_resFileList[index]) : nullptr;
}

Common::SeekableReadStream *FIO::readStream(const Common::Path &file) {
	debug("%s(): Loading file %s", __FUNCTION__, file.baseName().c_str());
	return _files.createReadStreamForMember(file);
}

Common::SeekableReadStreamEndian *FIO::readStreamEndian(int index, EndianMode em) {
	return (index >= 0 && index < _resFileListSize) ? readStreamEndian(_resFileList[index], em) : nullptr;
}

Common::SeekableReadStreamEndian *FIO::readStreamEndian(const Common::Path &file, EndianMode em) {
	Common::SeekableReadStream *stream = readStream(file);
	return stream ? new EndianWrapper(stream, (em == kForceBE) ? true : (em == kForceLE ? false : _bigEndianTarget)) : nullptr;
}

const char *FIO::_resFileList[97] = {
	"DATA_A0.BIN",	"DATA_B0.BIN",	"DATA_D0.BIN",	"DATA_D1.BIN",	"DATA_D2.BIN",	"DATA_F0.BIN",	"DATA_F4.BIN",	"DATA_G0.BIN",
	"DATA_H00.BIN",	"DATA_H01.BIN",	"DATA_H02.BIN",	"DATA_H03.BIN",	"DATA_H04.BIN",	"DATA_H05.BIN",	"DATA_H06.BIN",	"DATA_H07.BIN",
	"DATA_H08.BIN",	"DATA_H09.BIN",	"DATA_H11.BIN",	"DATA_H12.BIN",	"DATA_H13.BIN",	"DATA_H14.BIN",	"DATA_H15.BIN",	"DATA_I0.BIN",
	"DATA_J0.BIN",	"DATA_K1.BIN",	"DATA_M0.BIN",	"DATA_O0.BIN",	"DATA_P0.BIN",	"DATA_Q3.BIN",	"DATA_Q5.BIN",	"DATA_Q8.BIN",
	"DATA_S0.BIN",	"DATA_S1.BIN",	"DATA_S2.BIN",	"DATA_T0.BIN",	"DATA_U0.BIN",	"DATA_Y00.BIN",	"DATA_Y01.BIN",	"DATA_Y03.BIN",
	"DATA_Y04.BIN",	"DATA_Y05.BIN",	"DATA_Y06.BIN",	"DATA_Y07.BIN",	"DATA_Y08.BIN",	"DATA_Y09.BIN",	"DATA_Y10.BIN",	"DATA_Y11.BIN",
	"DATA_Y12.BIN",	"DATA_Y13.BIN",	"DATA_Y14.BIN",	"DATA_Y15.BIN",	"DATA_Y16.BIN",	"FMWR_1.BIN",	"PCMDRMDT.BIN",	"PCMLD_01.BIN",
	"PCMLT_01.BIN",	"SP00.BIN",		"SP01.BIN",		"SP02.BIN",		"SP03.BIN",		"SP04.BIN",		"SP05.BIN",		"SP06.BIN",
	"SP07.BIN",		"SP08.BIN",		"SP09.BIN",		"SP10.BIN",		"SP11.BIN",		"SP12.BIN",		"SP13.BIN",		"SP14.BIN",
	"SP15.BIN",		"SP16.BIN",		"SP17.BIN",		"SP18.BIN",		"SP19.BIN",		"SP20.BIN",		"SP21.BIN",		"SP22.BIN",
	"SP23.BIN",		"SP24.BIN",		"SP25.BIN",		"SP26.BIN",		"SP27.BIN",		"SP28.BIN",		"SP29.BIN",		"SP30.BIN",
	"SP31.BIN",		"SP32.BIN",		"SP33.BIN",		"SP34.BIN",		"SP35.BIN",		"SP36.BIN",		"SP37.BIN",		"SP38.BIN",
	"SUBCODE.BIN"
};

const int FIO::_resFileListSize = ARRAYSIZE(_resFileList);

ResourcePointer::ResourcePointer() : _dataStart(nullptr), _writeableDataStart(nullptr), _offset(0), _moduleLocation(0x28000), _ownBuffer(false) {}

ResourcePointer::ResourcePointer(const uint8 *data, uint32 offs, uint32 loc, bool assignBufferOwnership) : _dataStart(data), _writeableDataStart(nullptr), _offset(offs), _moduleLocation(loc), _ownBuffer(assignBufferOwnership) {
	if (_offset >= _moduleLocation)
		_offset -= _moduleLocation;
}

ResourcePointer::ResourcePointer(uint8 *data, uint32 offs, uint32 loc, bool assignBufferOwnership) : _dataStart(data), _writeableDataStart(data), _offset(offs), _moduleLocation(loc), _ownBuffer(assignBufferOwnership) {
	if (_offset >= _moduleLocation)
		_offset -= _moduleLocation;
}

ResourcePointer::ResourcePointer(const ResourcePointer &ptr) : _dataStart(ptr._dataStart), _offset(ptr._offset), _moduleLocation(ptr._moduleLocation), _writeableDataStart(ptr._writeableDataStart), _ownBuffer(false) {
	assert(ptr._ownBuffer == false);
}

ResourcePointer::ResourcePointer(ResourcePointer &&ptr) noexcept : _dataStart(ptr._dataStart), _offset(ptr._offset), _moduleLocation(ptr._moduleLocation), _writeableDataStart(ptr._writeableDataStart), _ownBuffer(ptr._ownBuffer) {
	ptr._ownBuffer = false;
}

ResourcePointer::~ResourcePointer() {
	if (_ownBuffer && _dataStart)
		delete[] _dataStart;
}

const uint8 *ResourcePointer::operator()() const {
	return _dataStart + _offset;
}

const uint8 *ResourcePointer::operator++(int) {
	return _dataStart + (_offset++);
}

const uint8 *ResourcePointer::ResourcePointer::operator++() {
	return _dataStart + (++_offset);
}

uint16 ResourcePointer::readUINT16() const {
	return READ_BE_UINT16(_dataStart + _offset);
}

uint16 ResourcePointer::readSINT16() const {
	return READ_BE_INT16(_dataStart + _offset);
}

uint32 ResourcePointer::readUINT32() const {
	return READ_BE_UINT32(_dataStart + _offset);
}

uint32 ResourcePointer::readSINT32() const {
	return READ_BE_INT32(_dataStart + _offset);
}

uint16 ResourcePointer::readIncrUINT16() {
	uint16 r = READ_BE_UINT16(_dataStart + _offset);
	_offset += 2;
	return r;
}

uint16 ResourcePointer::readIncrSINT16() {
	int16 r = READ_BE_INT16(_dataStart + _offset);
	_offset += 2;
	return r;
}

uint32 ResourcePointer::readIncrUINT32() {
	uint32 r = READ_BE_UINT32(_dataStart + _offset);
	_offset += 4;
	return r;
}

uint32 ResourcePointer::readIncrSINT32() {
	int32 r = READ_BE_INT32(_dataStart + _offset);
	_offset += 4;
	return r;
}

void ResourcePointer::writeUINT32(uint32 value) {
	assert(_writeableDataStart);
	WRITE_BE_UINT32(_writeableDataStart + _offset, value);
}

ResourcePointer ResourcePointer::operator+(int inc) const {
	return _writeableDataStart ?
		ResourcePointer(_writeableDataStart, _offset + _moduleLocation + inc, _moduleLocation) :
			ResourcePointer(_dataStart, _offset + _moduleLocation + inc, _moduleLocation);
}

bool ResourcePointer::operator<(const ResourcePointer &ptr) const {
	assert(_dataStart == ptr._dataStart && _moduleLocation == ptr._moduleLocation);
	return _offset < ptr._offset;
}

bool ResourcePointer::operator>(const ResourcePointer &ptr) const {
	assert(_dataStart == ptr._dataStart && _moduleLocation == ptr._moduleLocation);
	return _offset > ptr._offset;
}

bool ResourcePointer::operator==(const ResourcePointer &ptr) const {
	return _dataStart == ptr._dataStart && _offset == ptr._offset && _moduleLocation == ptr._moduleLocation;
}

void ResourcePointer::operator+=(int inc) {
	_offset += inc;
}

void ResourcePointer::operator=(const uint8 *ptr) {
	_offset = ptr - _dataStart;
}

void ResourcePointer::operator=(const ResourcePointer &ptr) {
	assert(ptr._ownBuffer == false);
	_dataStart = ptr._dataStart;
	_writeableDataStart = ptr._writeableDataStart;
	_offset = ptr._offset;
	_moduleLocation = ptr._moduleLocation;
}

void ResourcePointer::operator=(ResourcePointer &&ptr) noexcept {
	_dataStart = ptr._dataStart;
	_writeableDataStart = ptr._writeableDataStart;
	_offset = ptr._offset;
	_moduleLocation = ptr._moduleLocation;
	_ownBuffer = ptr._ownBuffer;
	ptr._ownBuffer = false;
}

uint8 ResourcePointer::operator[](int index) const {
	return _dataStart[_offset + index];
}

ResourcePointer ResourcePointer::getDataFromTable(int tableEntry) const {
	return _writeableDataStart ?
		ResourcePointer(_writeableDataStart, _offset + _moduleLocation + READ_BE_INT16(_dataStart + _offset + tableEntry * 2), _moduleLocation) :
			ResourcePointer(_dataStart, _offset + _moduleLocation + READ_BE_INT16(_dataStart + _offset + tableEntry * 2), _moduleLocation);
}

ResourcePointer ResourcePointer::makePtr(uint32 offs) const {
	return _writeableDataStart ?
		ResourcePointer(_writeableDataStart, offs, _moduleLocation) :
			ResourcePointer(_dataStart, offs, _moduleLocation);
}

SceneModule::SceneModule(SnatcherEngine *vm, FIO *fio, const char *resFile, int index) : _vm(vm), _fio(fio), _resFile(resFile), _resIndex(index), _data(0), _dataSize(0), _updateHandler(0) {
	assert(!_resFile.empty());
	_data = _fio->fileData(_resFile, &_dataSize);
	_updateHandler = _shList[_resIndex] ? (_shList[_resIndex])(_vm, this, _fio) : 0;
}

SceneModule::~SceneModule() {
	delete[] _data;
	delete _updateHandler;
}

ResourcePointer SceneModule::getPtr(int offset) const {
	return ResourcePointer(_data, offset);
}

ResourcePointer SceneModule::getGfxData() const {
	return getPtr(READ_BE_UINT32(_data + 4));
}

void SceneModule::run(GameState &state) {
	if (_updateHandler)
		(*_updateHandler)(state);
}

#define S(id) &createSceneHandler_##id
#define INVALD 0

SceneModule::SHFactory *const SceneModule::_shList[97] = {
	INVALD, INVALD, S(D0), S(D1), S(D2), INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD, INVALD,
	INVALD, INVALD, INVALD, INVALD
};

#undef INVALD
#undef S

} // End of namespace Snatcher
