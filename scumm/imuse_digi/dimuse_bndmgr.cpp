/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

#include "stdafx.h"
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

BundleDirCache::BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		_budleDirCache[fileId].bundleTable = NULL;
	}
}

BundleDirCache::~BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		if (_budleDirCache[fileId].bundleTable != NULL)
			free (_budleDirCache[fileId].bundleTable);
	}
}

BundleDirCache::AudioTable *BundleDirCache::getTable(const char *filename, const char *directory) {
	int slot = matchFile(filename, directory);
	assert(slot != -1);
	return _budleDirCache[slot].bundleTable;
}

int32 BundleDirCache::getNumFiles(const char *filename, const char *directory) {
	int slot = matchFile(filename, directory);
	assert(slot != -1);
	return _budleDirCache[slot].numFiles;
}

int BundleDirCache::matchFile(const char *filename, const char *directory) {
	int32 tag, offset;
	bool found = false;
	int freeSlot = -1;
	int fileId;

	for (fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		if ((_budleDirCache[fileId].bundleTable == NULL) && (freeSlot == -1)) {
			freeSlot = fileId;
		}
		if (scumm_stricmp(filename, _budleDirCache[fileId].fileName) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		File file;

		if (file.open(filename, directory) == false) {
			warning("BundleDirCache::matchFile() Can't open bundle file: %s", filename);
			return false;
		}

		if (freeSlot == -1)
			error("BundleDirCache::matchFileFile() Can't find free slot for file bundle dir cache");

		tag = file.readUint32BE();
		offset = file.readUint32BE();
		
		strcpy(_budleDirCache[freeSlot].fileName, filename);
		_budleDirCache[freeSlot].numFiles = file.readUint32BE();
		_budleDirCache[freeSlot].bundleTable = (AudioTable *) malloc(_budleDirCache[freeSlot].numFiles * sizeof(AudioTable));

		file.seek(offset, SEEK_SET);

		for (int32 i = 0; i < _budleDirCache[freeSlot].numFiles; i++) {
			char name[13], c;
			int32 z = 0;
			int32 z2;

			for (z2 = 0; z2 < 8; z2++)
				if ((c = file.readByte()) != 0)
					name[z++] = c;
			name[z++] = '.';
			for (z2 = 0; z2 < 4; z2++)
				if ((c = file.readByte()) != 0)
					name[z++] = c;

			name[z] = '\0';
			strcpy(_budleDirCache[freeSlot].bundleTable[i].filename, name);
			_budleDirCache[freeSlot].bundleTable[i].offset = file.readUint32BE();
			_budleDirCache[freeSlot].bundleTable[i].size = file.readUint32BE();
		}
		return freeSlot;
	} else {
		return fileId;
	}
}

BundleMgr::BundleMgr(BundleDirCache *cache) {
	_cache = cache;
	_bundleTable = NULL;
	_compTable = NULL;
	_numFiles = 0;
	_curSample = -1;
	_fileBundleId = -1;
}

BundleMgr::~BundleMgr() {
	closeFile();
}

bool BundleMgr::openFile(const char *filename, const char *directory) {
	if (_file.isOpen())
		return true;

	if (_file.open(filename, directory) == false) {
		warning("BundleMgr::openFile() Can't open bundle file: %s", filename);
		return false;
	}

	_numFiles = _cache->getNumFiles(filename, directory);
	assert(_numFiles);
	_bundleTable = _cache->getTable(filename, directory);
	assert(_bundleTable);
	_compTableLoaded = false;
	_lastCacheOutputSize = 0;
	_lastBlock = -1;

	return true;
}

void BundleMgr::closeFile() {
	if (_file.isOpen()) {
		_file.close();
		_bundleTable = NULL;
		_numFiles = 0;
		_compTableLoaded = false;
		_lastBlock = -1;
		_lastCacheOutputSize = 0;
		_curSample = -1;
		free(_compTable);
		_compTable = NULL;
	}
}

int32 BundleMgr::decompressSampleByCurIndex(int32 offset, int32 size, byte **comp_final, int header_size) {
	return decompressSampleByIndex(_curSample, offset, size, comp_final, header_size);
}

int32 BundleMgr::decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **comp_final, int header_size) {
	int32 i, tag, num, final_size, output_size;
	byte *comp_input, *comp_output;
	
	if (index != -1)
		_curSample = index;

	if (_file.isOpen() == false) {
		warning("BundleMgr::decompressSampleByIndex() File is not open!");
		return 0;
	}

	if (!_compTableLoaded) {
		_file.seek(_bundleTable[index].offset, SEEK_SET);
		tag = _file.readUint32BE();
		num = _file.readUint32BE();
		_file.readUint32BE();
		_file.readUint32BE();

		if (tag != MKID_BE('COMP')) {
			warning("BundleMgr::decompressSampleByIndex() Compressed sound %d invalid (%s)", index, tag2str(tag));
			return 0;
		}

		_compTable = (CompTable *)malloc(sizeof(CompTable) * num);
		for (i = 0; i < num; i++) {
			_compTable[i].offset = _file.readUint32BE();
			_compTable[i].size = _file.readUint32BE();
			_compTable[i].codec = _file.readUint32BE();
			_file.readUint32BE();
		}
		_compTableLoaded = true;
	}

	int first_block = (offset + header_size) / 0x2000;
	int last_block = (offset + size + header_size - 1) / 0x2000;

	comp_output = (byte *)malloc(0x2000);
	*comp_final = (byte *)malloc(0x2000 * (1 + last_block - first_block));
	final_size = 0;

	int skip = offset - (first_block * 0x2000) + header_size;

	// CMI hack: one more zero byte at the end of input buffer
	comp_input = (byte *)malloc(0x2000);
	comp_input[0x2000-1] = 0;

	for (i = first_block; i <= last_block; i++) {
		assert(size);
		assert(0x2000 >= _compTable[i].size + 1);

		byte *curBuf;
		if (_lastBlock != i) {
			_file.seek(_bundleTable[index].offset + _compTable[i].offset, SEEK_SET);
			_file.read(comp_input, _compTable[i].size);

			output_size = BundleCodecs::decompressCodec(_compTable[i].codec, comp_input, comp_output, _compTable[i].size);
			assert(output_size <= 0x2000);
			_lastBlock = i;
			_lastCacheOutputSize = output_size;
			memcpy(_blockChache, comp_output, output_size);
			curBuf = comp_output;
		} else {
			output_size = _lastCacheOutputSize;
			curBuf = _blockChache;
		}

		if ((header_size != 0) && (skip > header_size))
			output_size -= skip;
		if (output_size > size)
			output_size = size;

		memcpy(*comp_final + final_size, curBuf + skip, output_size);

		final_size += output_size;
		size -= output_size;
		skip = 0;
	}
	free(comp_input);
	free(comp_output);

	return final_size;
}

int32 BundleMgr::decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final) {
	int32 final_size = 0, i;

	if (!_file.isOpen()) {
		warning("BundleMgr::decompressSampleByName() File is not open!");
		return 0;
	}

	for (i = 0; i < _numFiles; i++) {
		if (!scumm_stricmp(name, _bundleTable[i].filename)) {
			final_size = decompressSampleByIndex(i, offset, size, comp_final, 0);
			return final_size;
		}
	}
	debug(2, "BundleMgr::decompressSampleByName() Failed finding voice %s", name);
	return final_size;
}

} // End of namespace Scumm
