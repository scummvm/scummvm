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

#ifndef SNATCHER_RESOURCE_H
#define SNATCHER_RESOURCE_H

#include "common/endian.h"
#include "common/fs.h"
#include "common/func.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
class SeekableReadStreamEndian;
} // End of namespace Common

namespace Snatcher {

class SnatcherEngine;
class SceneModule;
class FIO;

struct GameState {
	int16 frameNo;
	int16 frameState;
	int16 finish;
	int16 progressMain;
	int16 progressSub;
	int16 counter;
	int16 modIndex;
	int16 initState;
};

class SceneHandler {
public:
	SceneHandler(SnatcherEngine *vm, SceneModule *scn, FIO *fio) : _vm(vm), _module(scn), _fio(fio) {}
	virtual ~SceneHandler() {}
	virtual void operator()(GameState &state) = 0;
protected:
	SnatcherEngine *_vm;
	SceneModule *_module;
	FIO *_fio;
};

struct ResourcePointer {
	ResourcePointer() : dataStart(0), offset(0), startAddr(0x28000) {}
	ResourcePointer(const uint8 *data, uint32 offs) : dataStart(data), offset(offs), startAddr(0x28000) { if (offset >= startAddr) offset -= startAddr; }
	const uint8 *dataStart;
	uint32 offset;
	uint32 startAddr;
	const uint8 *operator()() const { return dataStart + offset; }
	const uint8 *operator++(int) { return dataStart + (offset++); }
	const uint8 *operator++() { return dataStart + (++offset); }
	uint16 readUINT16() { return READ_BE_UINT16(dataStart + offset); }
	uint16 readSINT16() { return READ_BE_INT16(dataStart + offset); }
	uint32 readUINT32() { return READ_BE_UINT32(dataStart + offset); }
	uint32 readSINT32() { return READ_BE_INT32(dataStart + offset); }
	uint16 readIncrUINT16() { uint16 r = READ_BE_UINT16(dataStart + offset); offset += 2; return r; }
	uint16 readIncrSINT16() { int16 r = READ_BE_INT16(dataStart + offset); offset += 2; return r; }
	uint32 readIncrUINT32() { uint32 r = READ_BE_UINT32(dataStart + offset); offset += 4; return r; }
	uint32 readIncrSINT32() { int32 r = READ_BE_INT32(dataStart + offset); offset += 4; return r; }
	ResourcePointer operator+(int inc) const { return ResourcePointer(dataStart, offset + startAddr + inc); }
	bool operator<(const ResourcePointer &ptr) const { assert(dataStart == ptr.dataStart); return offset < ptr.offset; }
	bool operator>(const ResourcePointer &ptr) const { assert(dataStart == ptr.dataStart); return offset > ptr.offset; }
	bool operator==(const ResourcePointer &ptr) const { return dataStart == ptr.dataStart && offset == ptr.offset; }
	void operator+=(int inc) { offset += inc; }
	void operator=(const uint8 *ptr) { offset = ptr - dataStart; }
	uint8 operator[](int index) const { return dataStart[offset + index]; }
	ResourcePointer getDataFromTable(int tableEntry) const { return ResourcePointer(dataStart, offset + startAddr + READ_BE_UINT16(dataStart + offset + (tableEntry << 1))); }
	ResourcePointer makeAbsPtr(uint32 offs) const { return ResourcePointer(dataStart, offs); }
};

class SceneModule {
	friend class FIO;
public:
	~SceneModule();

	ResourcePointer getPtr(int offset) const;

	void run(GameState &state);

private:
	SceneModule(SnatcherEngine *vm, FIO *fio, const char *resFile, int index);

	const uint8 *_data;
	uint32 _dataSize;
	SnatcherEngine *_vm;
	FIO *_fio;
	const int _resIndex;
	Common::Path _resFile;
	SceneHandler *_updateHandler;

	typedef SceneHandler*(SHFactory)(SnatcherEngine*, SceneModule*, FIO*);
	static SHFactory *const _shList[97];
};

class FIO {
public:
	FIO(SnatcherEngine *vm, bool isBigEndian);
	~FIO();

	SceneModule *loadModule(int index);

	enum EndianMode {
		kPlatformEndianness = 0,
		kForceLE,
		kForceBE
	};

	Common::SeekableReadStream *readStream(const Common::Path &file);
	Common::SeekableReadStreamEndian *readStreamEndian(const Common::Path &file, EndianMode em = kPlatformEndianness);
	uint8 *fileData(const Common::Path &file, uint32 *fileSize);

private:
	Common::SearchSet _files;

	SnatcherEngine *_vm;
	bool _bigEndianTarget;

private:
	static const char *_resFileList[97];
	static const int _resFileListSize;
};

} // End of namespace Snatcher

#endif // SNATCHER_RESOURCE_H
