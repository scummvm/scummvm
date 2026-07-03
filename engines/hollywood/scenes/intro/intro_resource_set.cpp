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

#include "hollywood/scenes/intro/intro_resource_set.h"

#include "common/debug.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/stream.h"

namespace Hollywood {

IntroResourceSet::IntroResourceSet() :
		arenaCursor(0) {
	chunkTable.clear();
	clearChunkOffsets();
}

void IntroResourceSet::clearChunkOffsets() {
	memset(chunkOffsets, 0, sizeof(chunkOffsets));
}

bool IntroResourceSet::loadChunkTable(ResourceManager *resources, const char *archiveName) {
	if (!resources->readChunkTable(Common::Path(archiveName), chunkTable)) {
		warning("Failed to read %s header", archiveName);
		chunkTable.clear();
		return false;
	}

	return true;
}

bool IntroResourceSet::validateChunk(const char *archiveName, const char *debugName, uint index) const {
	if (!chunkTable.isValidChunk(index)) {
		warning("%s is missing %s chunk %u", archiveName, debugName, index);
		return false;
	}

	return true;
}

bool IntroResourceSet::validateChunkRange(const char *archiveName, const char *debugName,
		uint firstChunk, uint lastChunk) const {
	for (uint i = firstChunk; i <= lastChunk; ++i) {
		if (!validateChunk(archiveName, debugName, i))
			return false;
	}

	return true;
}

uint32 IntroResourceSet::totalChunkSize(uint firstChunk, uint lastChunk) const {
	uint32 byteCount = 0;
	for (uint i = firstChunk; i <= lastChunk; ++i)
		byteCount += chunkTable.sizes[i];
	return byteCount;
}

void IntroResourceSet::allocateArena(uint32 byteCount) {
	arena.resize(byteCount);
	memset(arena.data(), 0, arena.size());
	arenaCursor = 0;
	clearChunkOffsets();
}

bool IntroResourceSet::loadFixedChunk(ResourceManager *resources, const char *archiveName,
		const char *debugName, uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination", archiveName, index, debugName);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool IntroResourceSet::loadFixedChunk(ResourceManager *resources, const char *archiveName,
		const char *debugName, uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination", archiveName, index, debugName);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool IntroResourceSet::loadVariableChunk(ResourceManager *resources, const char *archiveName,
		uint index, Common::Array<byte> &destination) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	destination.resize(stream->size());
	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool IntroResourceSet::loadArenaChunk(ResourceManager *resources, const char *archiveName,
		const char *debugName, uint archiveIndex, uint localChunkIndex) {
	if (localChunkIndex >= kResourceChunkCount) {
		warning("%s chunk %u cannot be loaded into invalid %s local slot %u",
			archiveName, archiveIndex, debugName, localChunkIndex);
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), archiveIndex));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, archiveIndex);
		return false;
	}

	if (arenaCursor + stream->size() > arena.size()) {
		warning("%s chunk %u does not fit the %s resource arena", archiveName, archiveIndex, debugName);
		return false;
	}

	chunkOffsets[localChunkIndex] = arenaCursor;
	if (stream->read(arena.data() + arenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, archiveIndex);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u as local chunk %u: offset=%u size=%u",
		archiveName, archiveIndex, localChunkIndex, arenaCursor, (uint)stream->size());
	arenaCursor += stream->size();
	return true;
}

bool IntroResourceSet::loadArenaChunkAlias(ResourceManager *resources, const char *archiveName,
		const char *debugName, uint sourceIndex, uint aliasIndex, uint targetIndex) {
	if (aliasIndex >= kResourceChunkCount || targetIndex >= kResourceChunkCount) {
		warning("%s chunk %u cannot be aliased to invalid %s slot %u/%u",
			archiveName, sourceIndex, debugName, aliasIndex, targetIndex);
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), sourceIndex));
	if (!stream) {
		warning("Failed to open %s branch chunk %u", archiveName, sourceIndex);
		return false;
	}

	const uint32 destinationOffset = chunkOffsets[targetIndex];
	const uint32 requiredSize = destinationOffset + stream->size();
	if (requiredSize > arena.size()) {
		const uint oldSize = arena.size();
		arena.resize(requiredSize);
		memset(arena.data() + oldSize, 0, arena.size() - oldSize);
	}

	if (stream->read(arena.data() + destinationOffset, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s branch chunk %u", archiveName, sourceIndex);
		return false;
	}

	chunkOffsets[aliasIndex] = destinationOffset;
	debugC(1, kDebugResources, "Loaded %s branch chunk %u as arena chunk %u: offset=%u size=%u",
		archiveName, sourceIndex, aliasIndex, destinationOffset, (uint)stream->size());
	return true;
}

} // End of namespace Hollywood
