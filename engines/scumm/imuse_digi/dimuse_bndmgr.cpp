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


#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_codecs.h"

namespace Scumm {

BundleDirCache::BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		_budleDirCache[fileId].bundleTable = nullptr;
		_budleDirCache[fileId].fileName[0] = 0;
		_budleDirCache[fileId].numFiles = 0;
		_budleDirCache[fileId].isCompressed = false;
		_budleDirCache[fileId].indexTable = nullptr;
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

bool BundleDirCache::isSndDataExtComp(int slot) {
	return _budleDirCache[slot].isCompressed;
}

int BundleDirCache::matchFile(const char *filename) {
	int32 tag, offset;
	bool found = false;
	int freeSlot = -1;
	int fileId;

	for (fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		if ((_budleDirCache[fileId].bundleTable == nullptr) && (freeSlot == -1)) {
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
		if (tag == MKTAG('L','B','2','3'))
			_budleDirCache[freeSlot].isCompressed = true;
		offset = file.readUint32BE();

		Common::strlcpy(_budleDirCache[freeSlot].fileName, filename, sizeof(_budleDirCache[freeSlot].fileName));
		_budleDirCache[freeSlot].numFiles = file.readUint32BE();
		_budleDirCache[freeSlot].bundleTable = (AudioTable *)malloc(_budleDirCache[freeSlot].numFiles * sizeof(AudioTable));
		assert(_budleDirCache[freeSlot].bundleTable);

		file.seek(offset, SEEK_SET);

		_budleDirCache[freeSlot].indexTable =
				(IndexNode *)calloc(_budleDirCache[freeSlot].numFiles, sizeof(IndexNode));
		assert(_budleDirCache[freeSlot].indexTable);

		for (int32 i = 0; i < _budleDirCache[freeSlot].numFiles; i++) {
			char name[24], c;
			int32 z = 0;
			int32 z2;

			if (tag == MKTAG('L','B','2','3')) {
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
				Common::strlcpy(_budleDirCache[freeSlot].bundleTable[i].filename, name, sizeof(_budleDirCache[freeSlot].bundleTable[i].filename));
			}
			_budleDirCache[freeSlot].bundleTable[i].offset = file.readUint32BE();
			_budleDirCache[freeSlot].bundleTable[i].size = file.readUint32BE();
			Common::strlcpy(_budleDirCache[freeSlot].indexTable[i].filename, _budleDirCache[freeSlot].bundleTable[i].filename, sizeof(_budleDirCache[freeSlot].indexTable[i].filename));
			_budleDirCache[freeSlot].indexTable[i].index = i;
		}
		qsort(_budleDirCache[freeSlot].indexTable, _budleDirCache[freeSlot].numFiles,
				sizeof(IndexNode), (int (*)(const void *, const void *))scumm_stricmp);
		return freeSlot;
	} else {
		return fileId;
	}
}

BundleMgr::BundleMgr(BundleDirCache *cache) {
	_cache = cache;
	_bundleTable = nullptr;
	_compTable = nullptr;
	_numFiles = 0;
	_numCompItems = 0;
	_lastBlockDecompressedSize = 0;
	_curSampleId = -1;
	_fileBundleId = -1;
	_file = new ScummFile();
	_compInputBuff = nullptr;
}

BundleMgr::~BundleMgr() {
	close();
	delete _file;
}

Common::SeekableReadStream *BundleMgr::getFile(const char *filename, int32 &offset, int32 &size) {
	BundleDirCache::IndexNode target;
	Common::strlcpy(target.filename, filename, sizeof(target.filename));
	BundleDirCache::IndexNode *found = (BundleDirCache::IndexNode *)bsearch(&target, _indexTable, _numFiles,
			sizeof(BundleDirCache::IndexNode), (int (*)(const void *, const void *))scumm_stricmp);
	if (found) {
		_file->seek(_bundleTable[found->index].offset, SEEK_SET);
		offset = _bundleTable[found->index].offset;
		size = _bundleTable[found->index].size;
		return _file;
	}

	return nullptr;
}

bool BundleMgr::open(const char *filename, bool &isCompressed, bool errorFlag) {
	if (_file->isOpen())
		return true;

	if (g_scumm->openFile(*_file, filename) == false) {
		if (errorFlag) {
			error("BundleMgr::open() Can't open bundle file: %s", filename);
		} else {
			warning("BundleMgr::open() Can't open bundle file: %s", filename);
		}
		return false;
	}

	int slot = _cache->matchFile(filename);
	assert(slot != -1);
	isCompressed = _cache->isSndDataExtComp(slot);
	_numFiles = _cache->getNumFiles(slot);
	assert(_numFiles);
	_bundleTable = _cache->getTable(slot);
	_indexTable = _cache->getIndexTable(slot);
	assert(_bundleTable);
	_compTableLoaded = false;
	_isUncompressed = false;
	_outputSize = 0;
	_lastBlockDecompressedSize = 0;
	_curDecompressedFilePos = 0;
	_lastBlock = -1;

	return true;
}

void BundleMgr::close() {
	if (_file->isOpen()) {
		_file->close();
		_bundleTable = nullptr;
		_numFiles = 0;
		_numCompItems = 0;
		_lastBlockDecompressedSize = 0;
		_curDecompressedFilePos = 0;
		_compTableLoaded = false;
		_isUncompressed = false;
		_lastBlock = -1;
		_outputSize = 0;
		_curSampleId = -1;
		free(_compTable);
		_compTable = nullptr;
		free(_compInputBuff);
		_compInputBuff = nullptr;
	}
}

bool BundleMgr::loadCompTable(int32 index) {
	_file->seek(_bundleTable[index].offset, SEEK_SET);
	uint32 tag = _file->readUint32BE();

	if (tag == MKTAG('i','M','U','S')) {
		_isUncompressed = true;
		return true;
	}

	_numCompItems = _file->readUint32BE();
	assert(_numCompItems > 0);
	_file->seek(4, SEEK_CUR);
	_lastBlockDecompressedSize = _file->readUint32BE();
	if (tag != MKTAG('C','O','M','P')) {
		debug("BundleMgr::loadCompTable() Compressed sound %d (%s:%d) invalid (%s)", index, _file->getName(), _bundleTable[index].offset, tag2str(tag));
		return false;
	}

	_compTable = (CompTable *)malloc(sizeof(CompTable) * _numCompItems);
	assert(_compTable);
	int32 maxSize = 0;
	for (int i = 0; i < _numCompItems; i++) {
		_compTable[i].offset = _file->readUint32BE();
		_compTable[i].size = _file->readUint32BE();
		_compTable[i].codec = _file->readUint32BE();
		_file->seek(4, SEEK_CUR);
		if (_compTable[i].size > maxSize)
			maxSize = _compTable[i].size;
	}
	// CMI hack: one more byte at the end of input buffer
	_compInputBuff = (byte *)malloc(maxSize + 1);
	assert(_compInputBuff);

	return true;
}

int32 BundleMgr::seekFile(int32 offset, int mode) {
	// We don't actually seek the file, but instead try to find that the specified offset exists
	// within the decompressed blocks, and save that offset in _curDecompressedFilePos
	int result = 0;
	switch (mode) {
	case SEEK_END:
		if (_isUncompressed) {
			result = offset + _bundleTable[_curSampleId].size;
		} else {
			result = offset + ((_numCompItems - 1) * DIMUSE_BUN_CHUNK_SIZE) + _lastBlockDecompressedSize;
		}
		_curDecompressedFilePos = result;
		break;
	case SEEK_SET:
	default:
		if (_isUncompressed) {
			result = offset;
			_curDecompressedFilePos = result;
		} else {
			int destBlock = offset / DIMUSE_BUN_CHUNK_SIZE + (offset % DIMUSE_BUN_CHUNK_SIZE != 0);
			if (destBlock <= _numCompItems) {
				result = offset;
				_curDecompressedFilePos = result;
			}
		}
		break;
	}
	return result;
}

int32 BundleMgr::readFile(const char *name, int32 size, byte **comp_final, bool header_outside) {
	int32 final_size = 0;

	if (!_file->isOpen()) {
		error("BundleMgr::readFile() File is not open");
		return 0;
	}

	// Find the sound in the bundle
	BundleDirCache::IndexNode target;
	strncpy(target.filename, name, sizeof(target.filename));
	target.filename[sizeof(target.filename) - 1] = '\0';
	BundleDirCache::IndexNode *found = (BundleDirCache::IndexNode *)bsearch(&target, _indexTable, _numFiles,
		sizeof(BundleDirCache::IndexNode), (int(*)(const void *, const void *))scumm_stricmp);

	if (found) {
		int32 i, finalSize, outputSize;
		int skip, firstBlock, lastBlock;
		int headerSize = 0;

		assert(0 <= found->index && found->index < _numFiles);

		if (_file->isOpen() == false) {
			error("BundleMgr::readFile() File is not open");
			return 0;
		}

		if (_curSampleId == -1)
			_curSampleId = found->index;

		assert(_curSampleId == found->index);

		if (!_compTableLoaded) {
			_compTableLoaded = loadCompTable(found->index);
			if (!_compTableLoaded)
				return 0;
		}

		if (_isUncompressed) {
			_file->seek(_bundleTable[found->index].offset + _curDecompressedFilePos + headerSize, SEEK_SET);
			*comp_final = (byte *)malloc(size);
			assert(*comp_final);
			_file->read(*comp_final, size);
			_curDecompressedFilePos += size;
			return size;
		}

		firstBlock = (_curDecompressedFilePos + headerSize) / DIMUSE_BUN_CHUNK_SIZE;
		lastBlock = (_curDecompressedFilePos + headerSize + size - 1) / DIMUSE_BUN_CHUNK_SIZE;

		// Clip last_block by the total number of blocks (= "comp items")
		if ((lastBlock >= _numCompItems) && (_numCompItems > 0))
			lastBlock = _numCompItems - 1;

		int32 blocksFinalSize = DIMUSE_BUN_CHUNK_SIZE * (1 + lastBlock - firstBlock);
		*comp_final = (byte *)malloc(blocksFinalSize);
		assert(*comp_final);
		finalSize = 0;

		skip = (_curDecompressedFilePos + headerSize) % DIMUSE_BUN_CHUNK_SIZE; // Excess length after the last block

		for (i = firstBlock; i <= lastBlock; i++) {
			if (_lastBlock != i) {
				// CMI hack: one more zero byte at the end of input buffer
				_compInputBuff[_compTable[i].size] = 0;
				_file->seek(_bundleTable[found->index].offset + _compTable[i].offset, SEEK_SET);
				_file->read(_compInputBuff, _compTable[i].size);
				_outputSize = BundleCodecs::decompressCodec(_compTable[i].codec, _compInputBuff, _compOutputBuff, _compTable[i].size);

				if (_outputSize > DIMUSE_BUN_CHUNK_SIZE) {
					error("_outputSize: %d", _outputSize);
				}
				_lastBlock = i;
			}

			outputSize = _outputSize;

			if (header_outside) {
				outputSize -= skip;
			} else {
				if ((headerSize != 0) && (skip >= headerSize))
					outputSize -= skip;
			}

			if ((outputSize + skip) > DIMUSE_BUN_CHUNK_SIZE) // workaround
				outputSize -= (outputSize + skip) - DIMUSE_BUN_CHUNK_SIZE;

			if (outputSize > size)
				outputSize = size;

			assert(finalSize + outputSize <= blocksFinalSize);

			memcpy(*comp_final + finalSize, _compOutputBuff + skip, outputSize);
			finalSize += outputSize;

			size -= outputSize;
			assert(size >= 0);
			if (size == 0)
				break;

			skip = 0;
		}
		_curDecompressedFilePos += finalSize;

		return finalSize;
	}

	debug(2, "BundleMgr::readFile() Failed finding sound %s", name);
	return final_size;
}

bool BundleMgr::isExtCompBun(byte gameId) {
	bool isExtComp = false;
	if (gameId == GID_CMI) {
		bool isExtComp1 = false, isExtComp2 = false, isExtComp3 = false, isExtComp4 = false;
		this->open("voxdisk1.bun", isExtComp1); this->close();
		this->open("voxdisk2.bun", isExtComp2); this->close();
		this->open("musdisk1.bun", isExtComp3); this->close();
		this->open("musdisk2.bun", isExtComp4); this->close();

		isExtComp = isExtComp1 | isExtComp2 | isExtComp3 | isExtComp4;
	} else {
		bool isExtComp1 = false, isExtComp2 = false;
		this->open("digvoice.bun", isExtComp1); this->close();
		this->open("digmusic.bun", isExtComp2); this->close();
		isExtComp = isExtComp1 | isExtComp2;
	}

	return isExtComp;
}

} // End of namespace Scumm
