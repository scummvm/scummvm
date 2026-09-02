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

#include "hollywood/scenes/scene_resources.h"

#include "common/debug.h"
#include "common/path.h"
#include "graphics/managed_surface.h"

namespace Hollywood {

SceneResources::SceneResources() :
		_arenaCursor(0),
		_archive() {
	_chunkTable.clear();
	clearChunkOffsets();
}

void SceneResources::clearChunkOffsets() {
	memset(_chunkOffsets, 0, sizeof(_chunkOffsets));
}

bool SceneResources::loadChunkTable(const char *archiveName) {
	if (!_archive.open(Common::Path(archiveName))) {
		warning("Failed to read %s header", archiveName);
		_chunkTable.clear();
		return false;
	}

	_chunkTable = _archive.chunkTable();
	return true;
}

bool SceneResources::validateChunk(const char *archiveName, const char *sceneDebugName,
		uint index) const {
	if (!_chunkTable.isValidChunk(index)) {
		warning("%s is missing %s chunk %u", archiveName, sceneDebugName, index);
		return false;
	}

	return true;
}

bool SceneResources::validateChunkRange(const char *archiveName, const char *sceneDebugName,
		uint firstChunk, uint lastChunk) const {
	for (uint index = firstChunk; index <= lastChunk; ++index) {
		if (!validateChunk(archiveName, sceneDebugName, index))
			return false;
	}

	return true;
}

bool SceneResources::validateRequiredChunks(const char *archiveName, const char *sceneDebugName,
		uint requiredChunkCount, uint framebufferChunkIndex) const {
	for (uint i = 0; i < requiredChunkCount; ++i) {
		const uint chunkIndex = i == 0 ? framebufferChunkIndex : i;
		if (!_chunkTable.isValidChunk(chunkIndex)) {
			warning("%s is missing required %s chunk %u", archiveName, sceneDebugName, chunkIndex);
			return false;
		}
	}

	return true;
}

uint32 SceneResources::totalChunkSize(uint firstChunk, uint lastChunk) const {
	return _archive.totalChunkSize(firstChunk, lastChunk);
}

void SceneResources::allocateArena(uint32 byteCount) {
	_arena.resize(byteCount);
	memset(_arena.data(), 0, _arena.size());
	_arenaCursor = 0;
	clearChunkOffsets();
}

bool SceneResources::loadFixedChunk(const char *sceneDebugName,
		uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _archive.readFixedChunk(index, destination, fixedSize, sceneDebugName);
}

bool SceneResources::loadFixedChunk(const char *sceneDebugName,
		uint index, Graphics::ManagedSurface &destination, uint fixedSize) {
	return _archive.readFixedChunk(index, destination, fixedSize, sceneDebugName);
}

bool SceneResources::loadFixedChunk(const char *sceneDebugName,
		uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _archive.readFixedChunk(index, destination, fixedSize, sceneDebugName);
}

bool SceneResources::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	return _archive.readVariableChunk(index, destination);
}

bool SceneResources::loadChunkTo(const char *sceneDebugName, uint index,
		Common::Array<byte> &destination, uint32 destinationOffset) {
	if (!_archive.readChunkTo(index, destination, destinationOffset))
		return false;

	const Common::String archiveName = _archive.fileName().toString();
	debugC(1, kDebugResources, "Loaded %s chunk %u at %s buffer offset %u: size=%u",
		archiveName.c_str(), index, sceneDebugName,
		destinationOffset, (uint)_archive.chunkSize(index));
	return true;
}

bool SceneResources::loadArenaChunk(const char *sceneDebugName, uint index) {
	return loadArenaChunk(sceneDebugName, index, index);
}

bool SceneResources::loadArenaChunk(const char *sceneDebugName, uint archiveIndex,
		uint localChunkIndex) {
	const Common::String archiveName = _archive.fileName().toString();
	if (localChunkIndex >= kResourceChunkCount) {
		warning("%s chunk %u cannot be loaded into invalid %s local slot %u",
			archiveName.c_str(), archiveIndex, sceneDebugName, localChunkIndex);
		return false;
	}

	if (!_archive.isValidChunk(archiveIndex)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), archiveIndex);
		return false;
	}

	const uint32 chunkSize = _archive.chunkSize(archiveIndex);
	if (_arenaCursor > _arena.size() || chunkSize > _arena.size() - _arenaCursor) {
		warning("%s chunk %u does not fit the %s resource arena", archiveName.c_str(), archiveIndex, sceneDebugName);
		return false;
	}

	_chunkOffsets[localChunkIndex] = _arenaCursor;
	if (!_archive.readChunkTo(archiveIndex, _arena, _arenaCursor))
		return false;

	debugC(1, kDebugResources, "Loaded %s arena chunk %u as local chunk %u: offset=%u size=%u",
		archiveName.c_str(), archiveIndex, localChunkIndex, _arenaCursor, (uint)chunkSize);
	_arenaCursor += chunkSize;
	return true;
}

bool SceneResources::loadArenaChunkAlias(const char *sceneDebugName, uint sourceIndex,
		uint aliasIndex, uint targetIndex) {
	const Common::String archiveName = _archive.fileName().toString();
	if (aliasIndex >= kResourceChunkCount || targetIndex >= kResourceChunkCount) {
		warning("%s chunk %u cannot be aliased to invalid %s slot %u/%u",
			archiveName.c_str(), sourceIndex, sceneDebugName, aliasIndex, targetIndex);
		return false;
	}

	if (!_archive.isValidChunk(sourceIndex)) {
		warning("Failed to open %s branch chunk %u", archiveName.c_str(), sourceIndex);
		return false;
	}

	const uint32 destinationOffset = _chunkOffsets[targetIndex];
	const uint32 chunkSize = _archive.chunkSize(sourceIndex);
	const uint32 requiredSize = destinationOffset + chunkSize;
	if (requiredSize > _arena.size()) {
		const uint oldSize = _arena.size();
		_arena.resize(requiredSize);
		memset(_arena.data() + oldSize, 0, _arena.size() - oldSize);
	}

	if (!_archive.readChunkTo(sourceIndex, _arena, destinationOffset))
		return false;

	_chunkOffsets[aliasIndex] = destinationOffset;
	debugC(1, kDebugResources, "Loaded %s branch chunk %u as arena chunk %u: offset=%u size=%u",
		archiveName.c_str(), sourceIndex, aliasIndex, destinationOffset, (uint)chunkSize);
	return true;
}

} // End of namespace Hollywood
