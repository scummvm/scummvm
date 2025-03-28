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
	ResourcePointer() : dataStart(0), offset(0) {}
	ResourcePointer(const uint8 *data, uint32 offs) : dataStart(data), offset(offs >= 0x28000 ? offs - 0x28000 : offs) {}
	const uint8 *dataStart;
	uint32 offset;
	const uint8 *operator()() const { return dataStart + offset; }
	const uint8 *operator++(int) { return dataStart + (offset++); }
	const uint8 *operator++() { return dataStart + (++offset); }
	const uint8 *operator+(int inc) const { return dataStart + offset + inc; }
	void operator+=(int inc) { offset += inc; }
	void operator=(const uint8 *ptr) { offset = ptr - dataStart; }
	uint8 operator[](int index) const { return dataStart[offset + index]; }
	ResourcePointer getDataFromTable(int tableEntry) const { return ResourcePointer(dataStart, offset + READ_BE_UINT16(dataStart + offset + (tableEntry << 1))); }
	ResourcePointer makeAbsPtr(uint32 offs) const { return ResourcePointer(dataStart, offs); }
};

class SceneModule {
	friend class FIO;
public:
	~SceneModule();

	const uint8 *getData(int offset) const;
	ResourcePointer getPtr(int offset) const;

	void run(GameState &state);

private:
	SceneModule(SnatcherEngine *vm, FIO *fio, const char *resFile, int index);

	const uint8 *_data;
	uint32 _dataSize;
	//const uint8 **_table;
	//uint16 _tableSize;

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
