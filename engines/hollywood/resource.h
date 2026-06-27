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

#include "common/path.h"
#include "common/stream.h"
#include "common/types.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

struct ResourceChunkTable {
	uint32 offsets[HollywoodEngine::kResourceChunkCount];
	uint32 sizes[HollywoodEngine::kResourceChunkCount];

	void clear();
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

namespace SceneMetadataOffsets {
static const uint16 kActorDepthThresholds = 0x0000;
static const uint16 kPaletteDeltaTable = 0x002a;
static const uint16 kPaletteAdjustTable = 0x003f;
static const uint16 kRouteBoundaryPoints = 0x007d;
static const uint16 kRouteBoundarySteps = 0x1529;
static const uint16 kSceneItemDefaultStrip = 0x35e4;
static const uint16 kSceneItemInteractionPoints = 0x35f9;
static const uint16 kSceneItemApproachPoints = 0x364d;
static const uint16 kSceneItemFacing = 0x36a1;
static const uint16 kSceneVerbActionRecords = 0x36b6;
static const uint16 kSceneRelationRecords = 0x3956;
static const uint16 kSceneMode2RelationOverlay = 0x610a;
} // End of namespace SceneMetadataOffsets

class ResourceManager {
public:
	bool readChunkTable(const Common::Path &fileName, ResourceChunkTable &table) const;
	Common::SeekableReadStream *createChunkReadStream(const Common::Path &fileName, uint index) const;

private:
	bool readChunkTable(Common::SeekableReadStream &stream, ResourceChunkTable &table) const;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RESOURCE_H
