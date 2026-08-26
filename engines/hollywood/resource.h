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

#ifndef HOLLYWOOD_RESOURCE_H
#define HOLLYWOOD_RESOURCE_H

#include "common/array.h"
#include "common/file.h"
#include "common/path.h"
#include "common/stream.h"
#include "common/types.h"

#include "hollywood/hollywood.h"

namespace Graphics {
class ManagedSurface;
}

namespace Hollywood {

class IndexedSurfaceBuffer;

struct ResourceChunkTable {
	uint32 offsets[HollywoodEngine::kResourceChunkCount];
	uint32 sizes[HollywoodEngine::kResourceChunkCount];

	void clear();
	bool load(Common::SeekableReadStream &stream);
	bool isValidChunk(uint index) const;
};

struct ScenePoint {
	int16 x;
	int16 y;
};

struct SceneItemMetadata {
	byte defaultStrip;
	byte facing;
	ScenePoint interactionPoint;
	ScenePoint approachPoint;
};

enum SceneMetadataOffset {
	kActorDepthThresholds = 0x0000,
	kPaletteDeltaTable = 0x002a,
	kPaletteAdjustTable = 0x003f,
	kRouteBoundaryPoints = 0x007d,
	kRouteBoundarySteps = 0x1529,
	kSceneItemDefaultStrip = 0x35e4,
	kSceneItemInteractionPoints = 0x35f9,
	kSceneItemApproachPoints = 0x364d,
	kSceneItemFacing = 0x36a1,
	kSceneVerbActionRecords = 0x36b6,
	kSceneRelationRecords = 0x3956,
	kSceneMode2RelationOverlay = 0x610a
};

enum ScenePaletteMapOffset {
	kScenePaletteRegionMap = 0x000,
	kSceneColorToItemMap = 0x100,
	kSceneColorToActorDepthClassMap = 0x200,
	kSceneColorToActorPaletteDeltaClassMap = 0x300,
	kSceneColorToActorPaletteAdjustmentClassMap = 0x400,
	kSceneColorToFootstepSoundMap = 0x500,
	kScenePresentationPaletteRemapMap = 0x600,
	kScenePaletteMapPageSize = 0x100,
	kScenePaletteMapByteCount = 0x700
};

class ResourceManager {
public:
	bool readChunkTable(const Common::Path &fileName, ResourceChunkTable &table) const;
	Common::SeekableReadStream *createChunkReadStream(const Common::Path &fileName, uint index) const;
};

// Keeps a RESOURCE.* archive open and reads chunks through its parsed table.
class ChunkArchive {
public:
	ChunkArchive();

	bool open(const Common::Path &fileName);
	void close();
	bool isOpen() const { return _open; }

	const Common::Path &fileName() const { return _fileName; }
	const ResourceChunkTable &chunkTable() const { return _chunkTable; }
	bool isValidChunk(uint index) const { return _chunkTable.isValidChunk(index); }
	uint32 chunkSize(uint index) const;
	uint32 totalChunkSize(uint firstChunk, uint lastChunk) const;

	bool readFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize,
		const char *debugName);
	bool readFixedChunk(uint index, Graphics::ManagedSurface &destination, uint fixedSize,
		const char *debugName);
	bool readFixedChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize,
		const char *debugName);
	bool readVariableChunk(uint index, Common::Array<byte> &destination);
	bool readChunkTo(uint index, Common::Array<byte> &destination, uint32 destinationOffset);

private:
	bool chunkBounds(uint index, uint32 &offset, uint32 &size) const;
	bool readChunkBytes(uint index, byte *destination, uint32 destinationSize);

	Common::File _file;
	Common::Path _fileName;
	ResourceChunkTable _chunkTable;
	bool _open;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RESOURCE_H
