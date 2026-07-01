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

#include "common/types.h"

namespace Hollywood {

enum {
	kSceneConfigUseViewportOffset = 0xffff,
	kSceneConfigNoAudioChapter = 0xff,
	kSceneConfigNoMainFlowRangeStart = 0xffff,
	kSceneConfigNoMainFlowRangeEnd = 0
};

/*
 * Static scene metadata used by the default PlayableScene accessors. Scenes
 * can fill this once in their constructor and override only unusual behavior.
 */
struct PlayableSceneConfig {
	PlayableSceneConfig();

	const char *resourceArchiveName;
	uint initialRequiredChunkCount;
	uint arenaFirstChunk;
	uint arenaLastChunk;
	uint stageIndex;
	const char *debugName;
	uint16 viewportXOffset;
	uint16 viewportMinXOffset;
	uint16 viewportMaxXOffset;
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
	const char *musicArchiveName;
	const char *soundBank0ArchiveName;
	bool loadActorDepthTables;
	bool useActorDepthTest;
	uint16 mainFlowFirstState;
	uint16 mainFlowLastState;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE_CONFIG_H
