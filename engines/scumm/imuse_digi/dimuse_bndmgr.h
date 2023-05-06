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

#ifndef SCUMM_IMUSE_DIGI_BUNDLE_MGR_H
#define SCUMM_IMUSE_DIGI_BUNDLE_MGR_H

#include "common/scummsys.h"
#include "common/file.h"
#include "scumm/imuse_digi/dimuse_defs.h"

namespace Scumm {

class BaseScummFile;

class BundleDirCache {
public:
	struct AudioTable {
		char filename[24];
		int32 offset;
		int32 size;
	};

	struct IndexNode {
		char filename[24];
		int32 index;
	};

private:

	struct FileDirCache {
		char fileName[20];
		AudioTable *bundleTable;
		int32 numFiles;
		bool isCompressed;
		IndexNode *indexTable;
	} _bundleDirCache[4];

	const ScummEngine *_vm;
public:
	BundleDirCache(const ScummEngine *vm);
	~BundleDirCache();

	int matchFile(const char *filename);
	AudioTable *getTable(int slot);
	IndexNode *getIndexTable(int slot);
	int32 getNumFiles(int slot);
	bool isSndDataExtComp(int slot);
};

class BundleMgr {

private:
	const ScummEngine *_vm;

	struct CompTable {
		int32 offset;
		int32 size;
		int32 codec;
	};

	BundleDirCache *_cache;
	BundleDirCache::AudioTable *_bundleTable;
	BundleDirCache::IndexNode *_indexTable;
	CompTable *_compTable;

	int _numFiles;
	int _numCompItems;
	int _lastBlockDecompressedSize;
	int _curSampleId;
	int _curDecompressedFilePos;
	BaseScummFile *_file;
	bool _compTableLoaded;
	bool _isUncompressed;
	int _fileBundleId;
	byte _compOutputBuff[0x2000];
	byte *_compInputBuff;
	int _outputSize;
	int _lastBlock;
	bool loadCompTable(int32 index);

public:

	BundleMgr(const ScummEngine *vm, BundleDirCache *_cache);
	~BundleMgr();

	bool open(const char *filename, bool &isCompressed, bool errorFlag = false);
	void close();
	Common::SeekableReadStream *getFile(const char *filename, int32 &offset, int32 &size);
	int32 seekFile(int32 offset, int size);
	int32 readFile(const char *name, int32 size, byte **compFinal, bool headerOutside);
	bool isExtCompBun(byte gameId);
};

} // End of namespace Scumm

#endif
