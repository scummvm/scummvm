/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef BUNDLE_MGR_H
#define BUNDLE_MGR_H

#include "common/scummsys.h"
#include "common/file.h"
#include "scumm/util.h"

namespace Scumm {

class BundleDirCache {
public:
	struct AudioTable {
		char filename[24];
		int32 offset;
		int32 size;
	};
private:

	struct FileDirCache {
		char fileName[20];
		AudioTable *bundleTable;
		int32 numFiles;
		bool compressedBun;
	} _budleDirCache[4];
	
public:
	BundleDirCache();
	~BundleDirCache();

	int matchFile(const char *filename);
	AudioTable *getTable(const char *filename, int slot);
	int32 getNumFiles(const char *filename, int slot);
	bool isCompressed(int slot);
};

class BundleMgr {
private:

	struct CompTable {
		int32 offset;
		int32 size;
		int32 codec;
	};

	BundleDirCache *_cache;
	BundleDirCache::AudioTable *_bundleTable;
	CompTable *_compTable;
	int _numFiles;
	int _numCompItems;
	int _curSample;
	ScummFile _file;
	bool _compTableLoaded;
	int _fileBundleId;
	byte _compOutput[0x2000];
	byte *_compInput;
	int _outputSize;
	int _lastBlock;
	
	bool loadCompTable(int32 index);

public:

	BundleMgr(BundleDirCache *_cache);
	~BundleMgr();

	bool open(const char *filename, bool &compressed, bool errorFlag=true);
	void close();
	Common::File *getFile(const char *filename, int32 &offset, int32 &size);
	int32 decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final, bool header_outside);
	int32 decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside);
	int32 decompressSampleByCurIndex(int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside);
};

namespace BundleCodecs {

uint32 decode12BitsSample(const byte *src, byte **dst, uint32 size);
void initializeImcTables();
#ifdef __PALM_OS__
void releaseImcTables();
#endif
int32 decompressCodec(int32 codec, byte *comp_input, byte *comp_output, int32 input_size);

} // End of namespace BundleCodecs

} // End of namespace Scumm

#endif
