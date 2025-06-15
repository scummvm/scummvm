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

#include "common/fs.h"
#include "common/scummsys.h"
#include "snatcher/state.h"

namespace Common {
class SeekableReadStream;
class SeekableReadStreamEndian;
} // End of namespace Common

namespace Snatcher {

class SnatcherEngine;
class SceneModule;
class FIO;

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

class ResourcePointer {
public:
	ResourcePointer();
	explicit ResourcePointer(const uint8 *data, uint32 offs, uint32 loc = 0x28000, bool assignBufferOwnership = false);
	explicit ResourcePointer(uint8 *data, uint32 offs, uint32 loc = 0x28000, bool assignBufferOwnership = false);
	ResourcePointer(const ResourcePointer &ptr);
	ResourcePointer(ResourcePointer &&ptr) noexcept;
	~ResourcePointer();

	const uint8 *operator()() const;
	const uint8 *operator++(int);
	const uint8 *operator++();
	uint16 readUINT16() const;
	uint16 readSINT16() const;
	uint32 readUINT32() const;
	uint32 readSINT32() const;
	uint16 readIncrUINT16();
	uint16 readIncrSINT16();
	uint32 readIncrUINT32();
	uint32 readIncrSINT32();
	void writeUINT16(uint16 value);
	void writeUINT32(uint32 value);
	ResourcePointer operator+(int inc) const;
	bool operator<(const ResourcePointer &ptr) const;
	bool operator>(const ResourcePointer &ptr) const;
	bool operator==(const ResourcePointer &ptr) const;
	void operator+=(int inc);
	void operator=(const uint8 *ptr);
	void operator=(const ResourcePointer &ptr);
	void operator=(ResourcePointer &&ptr) noexcept;
	uint8 operator[](int index) const;
	ResourcePointer getDataFromTable(int tableEntry) const;
	ResourcePointer makePtr(uint32 offs) const;
private:
	const uint8 *_dataStart;
	uint8 *_writeableDataStart;
	uint32 _offset;
	uint32 _moduleLocation;
	bool _ownBuffer;
};

class SceneModule {
	friend class FIO;
public:
	~SceneModule();

	ResourcePointer getPtr(int offset) const;
	ResourcePointer getGfxData() const;

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

	enum EndianMode {
		kPlatformEndianness = 0,
		kForceLE,
		kForceBE
	};

	SceneModule *loadModule(int index);
	uint8 *fileData(int index, uint32 *fileSize);
	uint8 *fileData(const Common::Path &file, uint32 *fileSize);
	Common::SeekableReadStream *readStream(int index);
	Common::SeekableReadStream *readStream(const Common::Path &file);
	Common::SeekableReadStreamEndian *readStreamEndian(int index, EndianMode em = kPlatformEndianness);
	Common::SeekableReadStreamEndian *readStreamEndian(const Common::Path &file, EndianMode em = kPlatformEndianness);


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
