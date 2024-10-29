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

#include "dgds/resource.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/platform.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/substream.h"
#include "dgds/decompress.h"
#include "dgds/includes.h"

namespace Dgds {

static const int FILENAME_LENGTH = 12;

ResourceManager::ResourceManager() {
	const char *indexFiles[] = {
		"volume.vga", // Dragon VGA versions
		"volume.ega", // Dragon EGA versions
		"volume.rmf", // Beamish / HoC
		"resource.map"  // Beamish CD
	};

	Common::File indexFile;
	for (int i = 0; i < ARRAYSIZE(indexFiles); i++) {
		if (Common::File::exists(indexFiles[i])) {
			indexFile.open(indexFiles[i]);
			break;
		}
	}

	if (!indexFile.isOpen()) {
		warning("No DGDS volume index file found to open.");
		return;
	}

	indexFile.readUint32LE(); // salt for file hash, TODO
	int nvolumes = indexFile.readUint16LE();

	char fnbuf[FILENAME_LENGTH + 1];
	fnbuf[FILENAME_LENGTH] = '\0';

	for (int i = 0; i < nvolumes; i++) {
		indexFile.read(fnbuf, FILENAME_LENGTH);
		Common::Path volumeName(fnbuf);
		assert(!volumeName.empty());

		_volumes[i].open(volumeName);
		if (!_volumes[i].isOpen())
			error("Couldn't open volume data %s", volumeName.toString().c_str());

		indexFile.readSByte(); // unknown byte
		int entries = indexFile.readUint16LE();

		for (int j = 0; j < entries; j++) {
			Resource res;
			res.volume = i;
			res.checksum = indexFile.readUint32LE();
			res.pos = indexFile.readUint32LE();

			_volumes[i].seek(res.pos, SEEK_SET);
			res.pos += FILENAME_LENGTH + 1 + 4;

			_volumes[i].read(fnbuf, FILENAME_LENGTH);
			Common::String fileName(fnbuf);
			fileName.toLowercase();

			_volumes[i].readSByte(); // unknown byte
			res.size = _volumes[i].readUint32LE();

			if (fileName.empty() || res.size == 0)
				continue;

			// Some sounds in Beamish have size -1 but are needed for the game.. where are they?
			if (res.size > (uint32)1 << 31) {
				warning("Res %s : %s at pos %d has negative size??", volumeName.toString().c_str(), fileName.c_str(), res.pos);
			}

			_resources[fileName] = res;
		}
	}

	indexFile.close();
}

ResourceManager::~ResourceManager() {
	for (int i = 0; i < MAX_VOLUMES; i++)
		_volumes[i].close();
}

Common::SeekableReadStream *ResourceManager::getResource(Common::String name, bool ignorePatches) {
	name.toLowercase();

	// Load external patches
	if (!ignorePatches && Common::File::exists(Common::Path(name))) {
		Common::File *patch = new Common::File();
		patch->open(Common::Path(name));
		return patch;
	}

	if (!_resources.contains(name))
		return nullptr;

	Resource res = _resources[name];

	if (res.size == 0xffffffff) // In willy beamish??
		return nullptr;

	return new Common::SeekableSubReadStream(&_volumes[res.volume], res.pos, res.pos + res.size);
}

Resource ResourceManager::getResourceInfo(Common::String name) {
	name.toLowercase();

	if (!_resources.contains(name))
		return Resource();

	return _resources[name];
}

bool ResourceManager::hasResource(Common::String name) const {
	name.toLowercase();
	return _resources.contains(name);
}

DgdsChunkReader::DgdsChunkReader(Common::SeekableReadStream *stream)
: _sourceStream(stream), _contentStream(nullptr), _size(0), _container(false),
 _startPos(0), _id(0), _ex(0) {
	ARRAYCLEAR(_idStr);
}

DgdsChunkReader::~DgdsChunkReader() {
	if (_contentStream)
		delete _contentStream;
}

bool DgdsChunkReader::isSection(const Common::String &section) const {
	return section.equals(_idStr);
}

bool DgdsChunkReader::isSection(DGDS_ID section) const {
	return (section == _id);
}

bool DgdsChunkReader::isPacked() const {
	bool packed = false;

	switch (_ex) {
	case EX_ADS:
	case EX_ADL:
	case EX_ADH:
		packed = (_id == ID_SCR);
		break;
	case EX_BMP:
		packed = (_id == ID_BIN || _id == ID_VGA);
		break;
	case EX_GDS:
	case EX_SDS:
		packed = (_id == ID_SDS);
		break;
	case EX_SCR:
		packed = (_id == ID_BIN || _id == ID_VGA || _id == ID_MA8);
		break;
	case EX_SNG:
		packed = (_id == ID_SNG);
		break;
	case EX_TTM:
		packed = (_id == ID_TT3);
		break;
	case EX_TDS:
		packed = (_id == ID_THD);
		break;
	case EX_DDS:
		packed = (_id == ID_DDS);
		break;
	default:
		break;
	}

	switch (_ex) {
	case EX_DDS:
		packed = !strcmp(_idStr, "DDS:");
		break;
	case EX_OVL:
		if (0) {
		} else if (strcmp(_idStr, "ADL:") == 0)
			packed = true;
		else if (strcmp(_idStr, "ADS:") == 0)
			packed = true;
		else if (strcmp(_idStr, "APA:") == 0)
			packed = true;
		else if (strcmp(_idStr, "ASB:") == 0)
			packed = true;
		else if (strcmp(_idStr, "GMD:") == 0)
			packed = true;
		else if (strcmp(_idStr, "M32:") == 0)
			packed = true;
		else if (strcmp(_idStr, "NLD:") == 0)
			packed = true;
		else if (strcmp(_idStr, "PRO:") == 0)
			packed = true;
		else if (strcmp(_idStr, "PS1:") == 0)
			packed = true;
		else if (strcmp(_idStr, "SBL:") == 0)
			packed = true;
		else if (strcmp(_idStr, "SBP:") == 0)
			packed = true;
		else if (strcmp(_idStr, "STD:") == 0)
			packed = true;
		else if (strcmp(_idStr, "TAN:") == 0)
			packed = true;
		else if (strcmp(_idStr, "T3V:") == 0)
			packed = true;
		else if (strcmp(_idStr, "001:") == 0)
			packed = true;
		else if (strcmp(_idStr, "003:") == 0)
			packed = true;
		else if (strcmp(_idStr, "004:") == 0)
			packed = true;
		else if (strcmp(_idStr, "005:") == 0)
			packed = true;
		else if (strcmp(_idStr, "007:") == 0)
			packed = true;
		else if (strcmp(_idStr, "009:") == 0)
			packed = true;
		else if (strcmp(_idStr, "101:") == 0)
			packed = true;
		else if (strcmp(_idStr, "VGA:") == 0)
			packed = true;
		break;
	case EX_TDS:
		if (0) {
		} else if (strcmp(_idStr, "TDS:") == 0)
			packed = true; /* ? */
		break;
	default:
		break;
	}
	return packed;
}

bool DgdsChunkReader::readNextHeader(DGDS_EX ex, const Common::String &filename) {
	if (_contentStream) {
		_sourceStream->seek(_startPos + _size);
		delete _contentStream;
		_contentStream = nullptr;
	}
	_size = 0;

	memset(_idStr, 0, sizeof(_idStr));
	_id = 0;
	_ex = ex;

	if (_sourceStream->pos() >= _sourceStream->size()) {
		return false;
	}

	_sourceStream->read(_idStr, DGDS_TYPENAME_MAX);

	if (_idStr[DGDS_TYPENAME_MAX - 1] != ':') {
		error("bad header reading chunk from %s at %d", filename.c_str(), (int)_sourceStream->pos() - 4);
		return false;
	}
	_idStr[DGDS_TYPENAME_MAX] = '\0';
	_id = MKTAG24(uint32(_idStr[0]), uint32(_idStr[1]), uint32(_idStr[2]));

	_size = _sourceStream->readUint32LE();
	_startPos = _sourceStream->pos();
	//ctx._file.skip(2);
	if (_size & 0x80000000) {
		_size &= ~0x80000000;
		_container = true;
	} else {
		_container = false;
	}
	return true;
}


bool DgdsChunkReader::readContent(Decompressor* decompressor) {
	assert(_sourceStream && !_contentStream);
	_contentStream = isPacked() ? decodeStream(decompressor) : readStream();
	return _contentStream != nullptr;
}

void DgdsChunkReader::skipContent() {
	assert(_sourceStream && !_contentStream);
	_sourceStream->seek(_startPos + _size);
}

Common::SeekableReadStream *DgdsChunkReader::decodeStream(Decompressor *decompressor) {
	Common::SeekableReadStream *output = nullptr;

	if (!_container) {
		uint32 uncompressedSize;
		byte *data = decompressor->decompress(_sourceStream, _size - (1 + 4), uncompressedSize);
		output = new Common::MemoryReadStream(data, uncompressedSize, DisposeAfterUse::YES);
	}

	return output;
}

Common::SeekableReadStream *DgdsChunkReader::readStream() {
	Common::SeekableReadStream *output = nullptr;

	if (!_container) {
		output = new Common::SeekableSubReadStream(_sourceStream, _startPos, _startPos + _size, DisposeAfterUse::NO);
	}

	//debug("    %s %u%c", _idStr, _size, (_container ? '+' : ' '));
	return output;
}

Common::SeekableReadStream *DgdsChunkReader::makeMemoryStream() {
	assert(_contentStream);
	assert(_contentStream->pos() == 0);

	int64 startPos = _contentStream->pos();
	int16 dataSize = _contentStream->size() - startPos;
	byte *data = (byte *)malloc(dataSize);
	_contentStream->read(data, dataSize);
	Common::MemoryReadStream *output = new Common::MemoryReadStream(data, dataSize, DisposeAfterUse::YES);
	_contentStream->seek(startPos);
	return output;
}

} // End of namespace Dgds
