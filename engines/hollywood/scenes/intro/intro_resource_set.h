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

#ifndef HOLLYWOOD_SCENES_INTRO_INTRO_RESOURCE_SET_H
#define HOLLYWOOD_SCENES_INTRO_INTRO_RESOURCE_SET_H

#include "common/array.h"

#include "hollywood/graphics.h"
#include "hollywood/resource.h"

namespace Hollywood {

// Owns the currently loaded intro archive table and decode arena.
class IntroResourceSet {
public:
	IntroResourceSet();

	void clearChunkOffsets();
	bool loadChunkTable(const char *archiveName);
	bool validateChunk(const char *archiveName, const char *debugName, uint index) const;
	bool validateChunkRange(const char *archiveName, const char *debugName, uint firstChunk, uint lastChunk) const;
	uint32 totalChunkSize(uint firstChunk, uint lastChunk) const;
	void allocateArena(uint32 byteCount);

	bool loadFixedChunk(const char *debugName,
		uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadFixedChunk(const char *debugName,
		uint index, IndexedSurfaceBuffer &destination, uint fixedSize);
	bool loadVariableChunk(uint index, Common::Array<byte> &destination);
	bool loadArenaChunk(const char *debugName, uint archiveIndex, uint localChunkIndex);
	bool loadArenaChunkAlias(const char *debugName,
		uint sourceIndex, uint aliasIndex, uint targetIndex);

	enum {
		kResourceChunkCount = 40
	};

	ResourceChunkTable chunkTable;
	uint32 chunkOffsets[kResourceChunkCount];
	Common::Array<byte> arena;
	uint32 arenaCursor;

private:
	ChunkArchive _archive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_INTRO_RESOURCE_SET_H
