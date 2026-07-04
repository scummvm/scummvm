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

#include "hollywood/scenes/playable/scene_resources.h"

#include "common/debug.h"
#include "common/path.h"
#include "graphics/managed_surface.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

SceneResources::SceneResources() :
		arenaCursor(0),
		_archive() {
	chunkTable.clear();
	clearChunkOffsets();
}

void SceneResources::clearChunkOffsets() {
	memset(chunkOffsets, 0, sizeof(chunkOffsets));
}

bool SceneResources::loadChunkTable(const char *archiveName) {
	if (!_archive.open(Common::Path(archiveName))) {
		chunkTable.clear();
		return false;
	}

	chunkTable = _archive.chunkTable();
	return true;
}

bool SceneResources::validateRequiredChunks(const char *archiveName, const char *sceneDebugName,
		uint requiredChunkCount) const {
	for (uint i = 0; i < requiredChunkCount; ++i) {
		if (!chunkTable.isValidChunk(i)) {
			warning("%s is missing required %s chunk %u", archiveName, sceneDebugName, i);
			return false;
		}
	}

	return true;
}

void SceneResources::allocateArena(uint32 byteCount) {
	arena.resize(byteCount);
	memset(arena.data(), 0, arena.size());
	arenaCursor = 0;
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

bool SceneResources::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	return _archive.readVariableChunk(index, destination);
}

bool SceneResources::loadArenaChunk(const char *sceneDebugName, uint index) {
	const Common::String archiveName = _archive.fileName().toString();
	if (!_archive.isValidChunk(index)) {
		warning("Failed to open %s chunk %u", archiveName.c_str(), index);
		return false;
	}

	const uint32 chunkSize = _archive.chunkSize(index);
	if (arenaCursor > arena.size() || chunkSize > arena.size() - arenaCursor) {
		warning("%s chunk %u does not fit the %s resource arena", archiveName.c_str(), index, sceneDebugName);
		return false;
	}

	chunkOffsets[index] = arenaCursor;
	if (!_archive.readChunkTo(index, arena, arenaCursor))
		return false;

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		archiveName.c_str(), index, arenaCursor, (uint)chunkSize);
	arenaCursor += chunkSize;
	return true;
}

} // End of namespace Hollywood
