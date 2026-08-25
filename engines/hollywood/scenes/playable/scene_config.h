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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE_CONFIG_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE_CONFIG_H

#include "common/str.h"
#include "common/types.h"

namespace Hollywood {

enum {
	kSceneConfigNoAudioChapter = 0xff
};

struct SceneResourceLayout {
	SceneResourceLayout(uint requiredChunks, uint firstArenaChunk, uint lastArenaChunk) :
			initialRequiredChunkCount(requiredChunks),
			arenaFirstChunk(firstArenaChunk),
			arenaLastChunk(lastArenaChunk) {
	}

	uint initialRequiredChunkCount;
	uint arenaFirstChunk;
	uint arenaLastChunk;
};

struct SceneViewport {
	explicit SceneViewport(uint16 initialXOffset) :
			xOffset(initialXOffset),
			minXOffset(initialXOffset),
			maxXOffset(initialXOffset) {
	}

	SceneViewport(uint16 initialXOffset, uint16 minimumXOffset, uint16 maximumXOffset) :
			xOffset(initialXOffset),
			minXOffset(minimumXOffset),
			maxXOffset(maximumXOffset) {
	}

	uint16 xOffset;
	uint16 minXOffset;
	uint16 maxXOffset;
};

struct SceneActorPose {
	SceneActorPose(int actorX, int actorY, byte actorFacing) :
			x(actorX),
			y(actorY),
			facing(actorFacing) {
	}

	int x;
	int y;
	byte facing;
};

/*
 * Static resource and presentation metadata for a playable scene. The scene
 * number supplies conventional archive names, stage index, and chapter-wide
 * defaults; scene factories only describe their layout and exceptions.
 */
struct PlayableSceneConfig {
	PlayableSceneConfig(uint16 sceneNumber, const SceneResourceLayout &resourceLayout,
		const SceneViewport &sceneViewport, const SceneActorPose &actorPose);

	void setActorResources(uint bankTableEntry, uint paletteTableEntry);
	void setTextResources(uint rowsOffsetIndex, uint32 cueDescriptorTableOffset);

	template<uint size>
	void setActorPathStepDeltas(const byte (&table)[size]) {
		actorPathStepDeltaTable = table;
		actorPathStepDeltaTableSize = size;
	}

	uint16 sceneId;
	Common::String resourceArchiveName;
	uint initialRequiredChunkCount;
	uint arenaFirstChunk;
	uint arenaLastChunk;
	uint stageIndex;
	Common::String debugName;
	uint16 viewportXOffset;
	uint16 viewportMinXOffset;
	uint16 viewportMaxXOffset;
	SceneActorPose defaultActorPose;
	byte inventoryOwnerIndex;
	byte activeAudioChapterIndex;
	uint actorBankTableEntry;
	uint actorBankSegmentCount;
	uint actorPaletteTableEntry;
	uint32 inventoryActionTableExtraOffset;
	uint inventoryRowsOffsetIndex;
	uint32 speechCueDescriptorTableOffset;
	const byte *actorPathStepDeltaTable;
	uint actorPathStepDeltaTableSize;
	byte walkablePaletteMaxRegion;
	Common::String musicArchiveName;
	Common::String soundBank0ArchiveName;
	bool loadInventoryActionTables;
	bool loadActorDepthTables;
	bool useActorDepthTest;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE_CONFIG_H
