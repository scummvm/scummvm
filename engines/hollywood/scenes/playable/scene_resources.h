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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE_RESOURCES_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE_RESOURCES_H

#include "common/array.h"
#include "common/types.h"

#include "hollywood/resource.h"

namespace Graphics {
class ManagedSurface;
}

namespace Hollywood {

// Owns scene archive chunks and generic chunk loading helpers.
class SceneResources {
public:
	SceneResources();

	void clearChunkOffsets();
	bool loadChunkTable(const char *archiveName);
	bool validateRequiredChunks(const char *archiveName, const char *sceneDebugName, uint requiredChunkCount) const;
	void allocateArena(uint32 byteCount);

	bool loadFixedChunk(const char *sceneDebugName,
		uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadFixedChunk(const char *sceneDebugName,
		uint index, Graphics::ManagedSurface &destination, uint fixedSize);
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadArenaChunk(const char *sceneDebugName, uint index);

	ResourceChunkTable chunkTable;
	uint32 chunkOffsets[HollywoodEngine::kResourceChunkCount];
	uint32 arenaCursor;
	Common::Array<byte> arena;
	Common::Array<byte> metadata;

private:
	ChunkArchive _archive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE_RESOURCES_H
