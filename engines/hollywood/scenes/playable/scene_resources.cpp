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
#include "common/ptr.h"
#include "common/stream.h"
#include "graphics/managed_surface.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

SceneResources::SceneResources() : arenaCursor(0) {
	chunkTable.clear();
	clearChunkOffsets();
}

void SceneResources::clearChunkOffsets() {
	memset(chunkOffsets, 0, sizeof(chunkOffsets));
}

bool SceneResources::loadChunkTable(ResourceManager *resources, const char *archiveName) {
	if (!resources->readChunkTable(Common::Path(archiveName), chunkTable)) {
		chunkTable.clear();
		return false;
	}

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

bool SceneResources::loadFixedChunk(ResourceManager *resources, const char *archiveName,
		const char *sceneDebugName, uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination", archiveName, index, sceneDebugName);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool SceneResources::loadFixedChunk(ResourceManager *resources, const char *archiveName,
		const char *sceneDebugName, uint index, Graphics::ManagedSurface &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	const uint destinationSize = destination.pitch * destination.h;
	if (stream->size() > fixedSize || destinationSize < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination", archiveName, index, sceneDebugName);
		return false;
	}

	memset(destination.getPixels(), 0, destinationSize);
	if (stream->read(destination.getPixels(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool SceneResources::loadVariableChunk(ResourceManager *resources, const char *archiveName,
		uint index, Common::Array<byte> &destination) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	destination.resize(stream->size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool SceneResources::loadArenaChunk(ResourceManager *resources, const char *archiveName,
		const char *sceneDebugName, uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	if (arenaCursor + stream->size() > arena.size()) {
		warning("%s chunk %u does not fit the %s resource arena", archiveName, index, sceneDebugName);
		return false;
	}

	chunkOffsets[index] = arenaCursor;
	if (stream->read(arena.data() + arenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		archiveName, index, arenaCursor, (uint)stream->size());
	arenaCursor += stream->size();
	return true;
}

} // End of namespace Hollywood
