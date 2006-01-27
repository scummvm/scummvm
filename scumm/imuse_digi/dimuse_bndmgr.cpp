/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

BundleDirCache::BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		_budleDirCache[fileId].bundleTable = NULL;
		_budleDirCache[fileId].fileName[0] = 0;
		_budleDirCache[fileId].numFiles = 0;
		_budleDirCache[fileId].compressedBun = false;
		_budleDirCache[fileId].indexTable = NULL;
	}
}

BundleDirCache::~BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		free(_budleDirCache[fileId].bundleTable);
		free(_budleDirCache[fileId].indexTable);
	}
}

BundleDirCache::AudioTable *BundleDirCache::getTable(int slot) {
	return _budleDirCache[slot].bundleTable;
}

int32 BundleDirCache::getNumFiles(int slot) {
	return _budleDirCache[slot].numFiles;
}

BundleDirCache::IndexNode *BundleDirCache::getIndexTable(int slot) {
	return _budleDirCache[slot].indexTable;
}

bool BundleDirCache::isCompressed(int slot) {
	return _budleDirCache[slot].compressedBun;
}

int BundleDirCache::matchFile(const char *filename) {
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
		ScummFile file;

		if (g_scumm->openFile(file, filename) == false) {
			error("BundleDirCache::matchFile() Can't open bundle file: %s", filename);
			return false;
		}

		if (freeSlot == -1)
			error("BundleDirCache::matchFileFile() Can't find free slot for file bundle dir cache");

		tag = file.readUint32BE();
		if (tag == 'LB23')
			_budleDirCache[freeSlot].compressedBun = true;
		offset = file.readUint32BE();

		strcpy(_budleDirCache[freeSlot].fileName, filename);
		_budleDirCache[freeSlot].numFiles = file.readUint32BE();
		_budleDirCache[freeSlot].bundleTable = (AudioTable *) malloc(_budleDirCache[freeSlot].numFiles * sizeof(AudioTable));

		file.seek(offset, SEEK_SET);

		_budleDirCache[freeSlot].indexTable =
				(IndexNode *)calloc(_budleDirCache[freeSlot].numFiles, sizeof(IndexNode));

		for (int32 i = 0; i < _budleDirCache[freeSlot].numFiles; i++) {
			char name[24], c;
			int32 z = 0;
			int32 z2;

			if (tag == 'LB23') {
				file.read(_budleDirCache[freeSlot].bundleTable[i].filename, 24);
			} else {
				for (z2 = 0; z2 < 8; z2++)
					if ((c = file.readByte()) != 0)
						name[z++] = c;
				name[z++] = '.';
				for (z2 = 0; z2 < 4; z2++)
					if ((c = file.readByte()) != 0)
						name[z++] = c;

				name[z] = '\0';
				strcpy(_budleDirCache[freeSlot].bundleTable[i].filename, name);
			}
			_budleDirCache[freeSlot].bundleTable[i].offset = file.readUint32BE();
			_budleDirCache[freeSlot].bundleTable[i].size = file.readUint32BE();
			strcpy(_budleDirCache[freeSlot].indexTable[i].filename,
					_budleDirCache[freeSlot].bundleTable[i].filename);
			_budleDirCache[freeSlot].indexTable[i].index = i;
		}
		qsort(_budleDirCache[freeSlot].indexTable, _budleDirCache[freeSlot].numFiles,
				sizeof(IndexNode), (int (*)(const void*, const void*))scumm_stricmp);
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
	_numCompItems = 0;
	_curSample = -1;
	_fileBundleId = -1;
	_compInput = NULL;
}

BundleMgr::~BundleMgr() {
	close();
}

Common::File *BundleMgr::getFile(const char *filename, int32 &offset, int32 &size) {
	BundleDirCache::IndexNode target;
	strcpy(target.filename, filename);
	BundleDirCache::IndexNode *found = (BundleDirCache::IndexNode *)bsearch(&target, _indexTable, _numFiles,
			sizeof(BundleDirCache::IndexNode), (int (*)(const void*, const void*))scumm_stricmp);
	if (found) {
		_file.seek(_bundleTable[found->index].offset, SEEK_SET);
		offset = _bundleTable[found->index].offset;
		size = _bundleTable[found->index].size;
		return &_file;
	}

	return NULL;
}

bool BundleMgr::open(const char *filename, bool &compressed, bool errorFlag) {
	if (_file.isOpen())
		return true;

	if (g_scumm->openFile(_file, filename) == false) {
		if (errorFlag) {
			error("BundleMgr::open() Can't open bundle file: %s", filename);
		} else {
			warning("BundleMgr::open() Can't open bundle file: %s", filename);
		}
		return false;
	}

	int slot = _cache->matchFile(filename);
	assert(slot != -1);
	compressed = _cache->isCompressed(slot);
	_numFiles = _cache->getNumFiles(slot);
	assert(_numFiles);
	_bundleTable = _cache->getTable(slot);
	_indexTable = _cache->getIndexTable(slot);
	assert(_bundleTable);
	_compTableLoaded = false;
	_outputSize = 0;
	_lastBlock = -1;

	return true;
}

void BundleMgr::close() {
	if (_file.isOpen()) {
		_file.close();
		_bundleTable = NULL;
		_numFiles = 0;
		_numCompItems = 0;
		_compTableLoaded = false;
		_lastBlock = -1;
		_outputSize = 0;
		_curSample = -1;
		free(_compTable);
		_compTable = NULL;
		free(_compInput);
		_compInput = NULL;
	}
}

bool BundleMgr::loadCompTable(int32 index) {
	_file.seek(_bundleTable[index].offset, SEEK_SET);
	uint32 tag = _file.readUint32BE();
	_numCompItems = _file.readUint32BE();
	assert(_numCompItems > 0);
	_file.seek(8, SEEK_CUR);

	if (tag != MKID_BE('COMP')) {
		error("BundleMgr::loadCompTable() Compressed sound %d invalid (%s)", index, tag2str(tag));
		return false;
	}

	_compTable = (CompTable *)malloc(sizeof(CompTable) * _numCompItems);
	int32 maxSize = 0;
	for (int i = 0; i < _numCompItems; i++) {
		_compTable[i].offset = _file.readUint32BE();
		_compTable[i].size = _file.readUint32BE();
		_compTable[i].codec = _file.readUint32BE();
		_file.seek(4, SEEK_CUR);
		if (_compTable[i].size > maxSize)
			maxSize = _compTable[i].size;
	}
	// CMI hack: one more byte at the end of input buffer
	_compInput = (byte *)malloc(maxSize + 1);

	return true;
}

int32 BundleMgr::decompressSampleByCurIndex(int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside) {
	return decompressSampleByIndex(_curSample, offset, size, comp_final, header_size, header_outside);
}

int32 BundleMgr::decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside) {
	int32 i, final_size, output_size;
	int skip, first_block, last_block;

	assert(0 <= index && index < _numFiles);

	if (_file.isOpen() == false) {
		error("BundleMgr::decompressSampleByIndex() File is not open!");
		return 0;
	}

	if (_curSample == -1)
		_curSample = index;

	assert(_curSample == index);

	if (!_compTableLoaded) {
		_compTableLoaded = loadCompTable(index);
		if (!_compTableLoaded)
			return 0;
	}

	first_block = (offset + header_size) / 0x2000;
	last_block = (offset + header_size + size - 1) / 0x2000;

	// Clip last_block by the total number of blocks (= "comp items")
	if ((last_block >= _numCompItems) && (_numCompItems > 0))
		last_block = _numCompItems - 1;

	int32 blocks_final_size = 0x2000 * (1 + last_block - first_block);
	*comp_final = (byte *)malloc(blocks_final_size);
	final_size = 0;

	skip = (offset + header_size) % 0x2000;

	for (i = first_block; i <= last_block; i++) {
		if (_lastBlock != i) {
			// CMI hack: one more zero byte at the end of input buffer
			_compInput[_compTable[i].size] = 0;
			_file.seek(_bundleTable[index].offset + _compTable[i].offset, SEEK_SET);
			_file.read(_compInput, _compTable[i].size);
			_outputSize = BundleCodecs::decompressCodec(_compTable[i].codec, _compInput, _compOutput, _compTable[i].size);
			if (_outputSize > 0x2000) {
				error("_outputSize: %d", _outputSize);
			}
			_lastBlock = i;
		}

		output_size = _outputSize;

		if (header_outside) {
			output_size -= skip;
		} else {
			if ((header_size != 0) && (skip >= header_size))
				output_size -= skip;
		}

		if ((output_size + skip) > 0x2000) // workaround
			output_size -= (output_size + skip) - 0x2000;

		if (output_size > size)
			output_size = size;

		assert(final_size + output_size <= blocks_final_size);

		memcpy(*comp_final + final_size, _compOutput + skip, output_size);
		final_size += output_size;

		size -= output_size;
		assert(size >= 0);
		if (size == 0)
			break;

		skip = 0;
	}

	return final_size;
}

int32 BundleMgr::decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final, bool header_outside) {
	int32 final_size = 0;

	if (!_file.isOpen()) {
		error("BundleMgr::decompressSampleByName() File is not open!");
		return 0;
	}

	BundleDirCache::IndexNode target;
	strcpy(target.filename, name);
	BundleDirCache::IndexNode *found = (BundleDirCache::IndexNode *)bsearch(&target, _indexTable, _numFiles,
			sizeof(BundleDirCache::IndexNode), (int (*)(const void*, const void*))scumm_stricmp);
	if (found) {
		final_size = decompressSampleByIndex(found->index, offset, size, comp_final, 0, header_outside);
		return final_size;
	}

	debug(2, "BundleMgr::decompressSampleByName() Failed finding voice %s", name);
	return final_size;
}

} // End of namespace Scumm
