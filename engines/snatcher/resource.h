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
#include "common/str.h"

namespace Common {
class SeekableReadStream;
class SeekableReadStreamEndian;
} // End of namespace Common

namespace Snatcher {

class SnatcherEngine;
class SceneResource;
class FIO;

class SceneHandler {
public:
	SceneHandler(SnatcherEngine *vm, SceneResource *scn, FIO *fio) : _vm(vm), _scene(scn), _fio(fio) {}
	virtual ~SceneHandler() {}
	virtual void operator()() = 0;
protected:
	SnatcherEngine *_vm;
	SceneResource *_scene;
	FIO *_fio;
};

class SceneResource {
	friend class FIO;
public:
	~SceneResource();

	const uint8 *getData(int offset) const;
	const uint8 *getDataFromTable(int offset, int tableEntry) const;
	const uint8 *getDataFromMainTable(int tableEntry) const;

	void startScene();

private:
	SceneResource(SnatcherEngine *vm, FIO *fio, const char *resFile, int index);

	const uint8 *_data;
	uint32 _dataSize;
	//const uint8 **_table;
	//uint16 _tableSize;

	SnatcherEngine *_vm;
	FIO *_fio;
	const int _resIndex;
	Common::Path _resFile;
	SceneHandler *_handler;

	typedef SceneHandler*(SHFactory)(SnatcherEngine*, SceneResource*, FIO*);
	static SHFactory *const _shList[97];
};

class FIO {
public:
	FIO(SnatcherEngine *vm, bool isBigEndian);
	~FIO();

	SceneResource *createSceneResource(int index);

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
