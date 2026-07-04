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

#include "hollywood/resource.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"
#include "graphics/managed_surface.h"

#include "hollywood/graphics.h"

namespace Hollywood {

void ResourceChunkTable::clear() {
	for (int i = 0; i < HollywoodEngine::kResourceChunkCount; ++i) {
		offsets[i] = 0;
		sizes[i] = 0;
	}
}

bool ResourceChunkTable::load(Common::SeekableReadStream &stream) {
	clear();

	if (stream.size() < HollywoodEngine::kResourceChunkCount * 8)
		return false;

	if (!stream.seek(0))
		return false;
	for (int i = 0; i < HollywoodEngine::kResourceChunkCount; ++i)
		offsets[i] = stream.readUint32LE();

	for (int i = 0; i < HollywoodEngine::kResourceChunkCount; ++i)
		sizes[i] = stream.readUint32LE();

	return !stream.err();
}

bool ResourceChunkTable::isValidChunk(uint index) const {
	return index < HollywoodEngine::kResourceChunkCount && sizes[index] != 0;
}

bool ResourceManager::readChunkTable(const Common::Path &fileName, ResourceChunkTable &table) const {
	Common::File file;
	if (!file.open(fileName)) {
		table.clear();
		return false;
	}

	return table.load(file);
}

Common::SeekableReadStream *ResourceManager::createChunkReadStream(const Common::Path &fileName, uint index) const {
	if (index >= HollywoodEngine::kResourceChunkCount)
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

bool ChunkArchive::open(const Common::Path &fileName) {
	close();

	_fileName = fileName;
	if (!_file.open(fileName)) {
		_chunkTable.clear();
		return false;
	}

	if (!_chunkTable.load(_file)) {
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
	for (uint i = firstChunk; i <= lastChunk && i < HollywoodEngine::kResourceChunkCount; ++i)
		byteCount += _chunkTable.sizes[i];
	return byteCount;
}

bool ChunkArchive::readFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize,
		const char *debugName) {
	uint32 chunkOffset = 0;
	uint32 byteCount = 0;
	const Common::String archiveName = _fileName.toString();
	if (!chunkBounds(index, chunkOffset, byteCount)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	if (byteCount > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination",
			archiveName.c_str(), index, debugName);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (!readChunkBytes(index, destination.data(), destination.size())) {
		warning("Failed to read %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: offset=%u size=%u",
		archiveName.c_str(), index, chunkOffset, (uint)byteCount);
	return true;
}

bool ChunkArchive::readFixedChunk(uint index, Graphics::ManagedSurface &destination, uint fixedSize,
		const char *debugName) {
	uint32 chunkOffset = 0;
	uint32 byteCount = 0;
	const Common::String archiveName = _fileName.toString();
	if (!chunkBounds(index, chunkOffset, byteCount)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	const uint destinationSize = destination.pitch * destination.h;
	if (byteCount > fixedSize || destinationSize < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination",
			archiveName.c_str(), index, debugName);
		return false;
	}

	memset(destination.getPixels(), 0, destinationSize);
	if (!readChunkBytes(index, (byte *)destination.getPixels(), destinationSize)) {
		warning("Failed to read %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: offset=%u size=%u",
		archiveName.c_str(), index, chunkOffset, (uint)byteCount);
	return true;
}

bool ChunkArchive::readFixedChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize,
		const char *debugName) {
	uint32 chunkOffset = 0;
	uint32 byteCount = 0;
	const Common::String archiveName = _fileName.toString();
	if (!chunkBounds(index, chunkOffset, byteCount)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	if (byteCount > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination",
			archiveName.c_str(), index, debugName);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (!readChunkBytes(index, destination.data(), destination.size())) {
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
