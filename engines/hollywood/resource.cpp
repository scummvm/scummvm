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

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"
#include "graphics/managed_surface.h"

#include "hollywood/debug.h"
#include "hollywood/resource.h"

namespace Hollywood {

uint16 readUint16LE(const Common::Array<byte> &source, uint offset) {
	if (offset + 2 > source.size())
		return 0;

	return source[offset] | (source[offset + 1] << 8);
}

int16 readSint16LE(const Common::Array<byte> &source, uint offset) {
	return (int16)readUint16LE(source, offset);
}

uint32 readUint32LE(const Common::Array<byte> &source, uint offset) {
	if (offset + 4 > source.size())
		return 0;

	return source[offset] |
		(source[offset + 1] << 8) |
		(source[offset + 2] << 16) |
		(source[offset + 3] << 24);
}

void ResourceChunkTable::clear() {
	count = 0;
	for (int i = 0; i < kMaxResourceChunkCount; ++i) {
		offsets[i] = 0;
		sizes[i] = 0;
	}
}

bool ResourceChunkTable::load(Common::SeekableReadStream &stream, uint chunkCount) {
	clear();

	if (chunkCount == 0 || chunkCount > kMaxResourceChunkCount || stream.size() < chunkCount * 8)
		return false;

	if (!stream.seek(0))
		return false;
	for (uint i = 0; i < chunkCount; ++i)
		offsets[i] = stream.readUint32LE();

	for (uint i = 0; i < chunkCount; ++i)
		sizes[i] = stream.readUint32LE();

	if (stream.err() || stream.eos())
		return false;
	count = chunkCount;
	return true;
}

bool ResourceChunkTable::isValidChunk(uint index) const {
	return index < count && sizes[index] != 0;
}

Common::SeekableReadStream *createResourceChunkReadStream(const Common::Path &fileName, uint index) {
	if (index >= kResourceChunkCount)
		return nullptr;

	Common::File *file = new Common::File();
	if (!file->open(fileName)) {
		delete file;
		return nullptr;
	}

	ResourceChunkTable table;
	if (!table.load(*file) || !table.isValidChunk(index)) {
		delete file;
		return nullptr;
	}

	const uint32 begin = table.offsets[index];
	const uint32 fileSize = file->size();
	if (begin > fileSize || table.sizes[index] > fileSize - begin) {
		delete file;
		return nullptr;
	}

	const uint32 end = begin + table.sizes[index];
	return new Common::SeekableSubReadStream(file, begin, end, DisposeAfterUse::YES);
}

ChunkArchive::ChunkArchive() :
		_file(),
		_fileName(),
		_chunkTable(),
		_open(false) {
	_chunkTable.clear();
}

bool ChunkArchive::open(const Common::Path &fileName, uint chunkCount) {
	close();

	_fileName = fileName;
	if (!_file.open(fileName)) {
		_chunkTable.clear();
		return false;
	}

	if (!_chunkTable.load(_file, chunkCount)) {
		close();
		return false;
	}

	_open = true;
	return true;
}

void ChunkArchive::close() {
	_file.close();
	_fileName = Common::Path();
	_chunkTable.clear();
	_open = false;
}

uint32 ChunkArchive::chunkSize(uint index) const {
	if (!_chunkTable.isValidChunk(index))
		return 0;

	return _chunkTable.sizes[index];
}

uint32 ChunkArchive::totalChunkSize(uint firstChunk, uint lastChunk) const {
	uint32 byteCount = 0;
	for (uint i = firstChunk; i <= lastChunk && i < _chunkTable.count; ++i)
		byteCount += _chunkTable.sizes[i];
	return byteCount;
}

bool ChunkArchive::readFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize,
		const char *debugName) {
	return readFixedChunk(index, destination.data(), destination.size(), fixedSize, debugName);
}

bool ChunkArchive::readFixedChunk(uint index, Graphics::ManagedSurface &destination, uint fixedSize,
		const char *debugName) {
	return readFixedChunk(index, (byte *)destination.getPixels(), destination.pitch * destination.h,
		fixedSize, debugName);
}

bool ChunkArchive::readFixedChunk(uint index, byte *destination, uint destinationSize,
		uint fixedSize, const char *debugName) {
	uint32 chunkOffset = 0;
	uint32 byteCount = 0;
	const Common::String archiveName = _fileName.toString();
	if (!chunkBounds(index, chunkOffset, byteCount)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	if (byteCount > fixedSize || destinationSize < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination",
			archiveName.c_str(), index, debugName);
		return false;
	}

	memset(destination, 0, destinationSize);
	if (!readChunkBytes(index, destination, destinationSize)) {
		warning("Failed to read %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: offset=%u size=%u",
		archiveName.c_str(), index, chunkOffset, (uint)byteCount);
	return true;
}

bool ChunkArchive::readVariableChunk(uint index, Common::Array<byte> &destination) {
	uint32 chunkOffset = 0;
	uint32 byteCount = 0;
	const Common::String archiveName = _fileName.toString();
	if (!chunkBounds(index, chunkOffset, byteCount)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	destination.resize(byteCount);
	if (!readChunkBytes(index, destination.data(), destination.size())) {
		warning("Failed to read %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: offset=%u size=%u",
		archiveName.c_str(), index, chunkOffset, (uint)byteCount);
	return true;
}

bool ChunkArchive::readChunkTo(uint index, Common::Array<byte> &destination, uint32 destinationOffset) {
	uint32 chunkOffset = 0;
	uint32 byteCount = 0;
	const Common::String archiveName = _fileName.toString();
	if (!chunkBounds(index, chunkOffset, byteCount)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), index);
		return false;
	}
	(void)chunkOffset;

	if (destinationOffset > destination.size() || byteCount > destination.size() - destinationOffset) {
		warning("%s chunk %u does not fit destination buffer", archiveName.c_str(), index);
		return false;
	}

	if (!readChunkBytes(index, destination.data() + destinationOffset, destination.size() - destinationOffset)) {
		warning("Failed to read %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	return true;
}

bool ChunkArchive::chunkBounds(uint index, uint32 &offset, uint32 &size) const {
	offset = 0;
	size = 0;

	if (!_open || !_chunkTable.isValidChunk(index))
		return false;

	const uint32 fileSize = (uint32)_file.size();
	offset = _chunkTable.offsets[index];
	size = _chunkTable.sizes[index];
	return offset <= fileSize && size <= fileSize - offset;
}

bool ChunkArchive::readChunkBytes(uint index, byte *destination, uint32 destinationSize) {
	uint32 offset = 0;
	uint32 byteCount = 0;
	if (!chunkBounds(index, offset, byteCount) || byteCount > destinationSize)
		return false;

	if (!_file.seek(offset))
		return false;

	return _file.read(destination, byteCount) == byteCount;
}

} // End of namespace Hollywood
