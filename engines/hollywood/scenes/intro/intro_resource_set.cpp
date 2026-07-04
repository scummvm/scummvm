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

namespace Hollywood {

IntroResourceSet::IntroResourceSet() :
		arenaCursor(0),
		_archive() {
	chunkTable.clear();
	clearChunkOffsets();
}

void IntroResourceSet::clearChunkOffsets() {
	memset(chunkOffsets, 0, sizeof(chunkOffsets));
}

bool IntroResourceSet::loadChunkTable(const char *archiveName) {
	if (!_archive.open(Common::Path(archiveName))) {
		warning("Failed to read %s header", archiveName);
		chunkTable.clear();
		return false;
	}

	chunkTable = _archive.chunkTable();
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
	return _archive.totalChunkSize(firstChunk, lastChunk);
}

void IntroResourceSet::allocateArena(uint32 byteCount) {
	arena.resize(byteCount);
	memset(arena.data(), 0, arena.size());
	arenaCursor = 0;
	clearChunkOffsets();
}

bool IntroResourceSet::loadFixedChunk(const char *debugName,
		uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _archive.readFixedChunk(index, destination, fixedSize, debugName);
}

bool IntroResourceSet::loadFixedChunk(const char *debugName,
		uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _archive.readFixedChunk(index, destination, fixedSize, debugName);
}

bool IntroResourceSet::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	return _archive.readVariableChunk(index, destination);
}

bool IntroResourceSet::loadArenaChunk(const char *debugName, uint archiveIndex, uint localChunkIndex) {
	const Common::String archiveName = _archive.fileName().toString();
	if (localChunkIndex >= kResourceChunkCount) {
		warning("%s chunk %u cannot be loaded into invalid %s local slot %u",
			archiveName.c_str(), archiveIndex, debugName, localChunkIndex);
		return false;
	}

	if (!_archive.isValidChunk(archiveIndex)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), archiveIndex);
		return false;
	}

	const uint32 chunkSize = _archive.chunkSize(archiveIndex);
	if (arenaCursor > arena.size() || chunkSize > arena.size() - arenaCursor) {
		warning("%s chunk %u does not fit the %s resource arena", archiveName.c_str(), archiveIndex, debugName);
		return false;
	}

	chunkOffsets[localChunkIndex] = arenaCursor;
	if (!_archive.readChunkTo(archiveIndex, arena, arenaCursor))
		return false;

	debugC(1, kDebugResources, "Loaded %s arena chunk %u as local chunk %u: offset=%u size=%u",
		archiveName.c_str(), archiveIndex, localChunkIndex, arenaCursor, (uint)chunkSize);
	arenaCursor += chunkSize;
	return true;
}

bool IntroResourceSet::loadArenaChunkAlias(const char *debugName, uint sourceIndex, uint aliasIndex, uint targetIndex) {
	const Common::String archiveName = _archive.fileName().toString();
	if (aliasIndex >= kResourceChunkCount || targetIndex >= kResourceChunkCount) {
		warning("%s chunk %u cannot be aliased to invalid %s slot %u/%u",
			archiveName.c_str(), sourceIndex, debugName, aliasIndex, targetIndex);
		return false;
	}

	if (!_archive.isValidChunk(sourceIndex)) {
		warning("Failed to open %s branch chunk %u", archiveName.c_str(), sourceIndex);
		return false;
	}

	const uint32 destinationOffset = chunkOffsets[targetIndex];
	const uint32 chunkSize = _archive.chunkSize(sourceIndex);
	const uint32 requiredSize = destinationOffset + chunkSize;
	if (requiredSize > arena.size()) {
		const uint oldSize = arena.size();
		arena.resize(requiredSize);
		memset(arena.data() + oldSize, 0, arena.size() - oldSize);
	}

	if (!_archive.readChunkTo(sourceIndex, arena, destinationOffset))
		return false;

	chunkOffsets[aliasIndex] = destinationOffset;
	debugC(1, kDebugResources, "Loaded %s branch chunk %u as arena chunk %u: offset=%u size=%u",
		archiveName.c_str(), sourceIndex, aliasIndex, destinationOffset, (uint)chunkSize);
	return true;
}

} // End of namespace Hollywood
