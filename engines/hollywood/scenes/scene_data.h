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

#ifndef HOLLYWOOD_SCENES_SCENE_DATA_H
#define HOLLYWOOD_SCENES_SCENE_DATA_H

#include "common/types.h"

namespace Hollywood {

struct ScenePoint {
	int16 x;
	int16 y;
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

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_SCENE_DATA_H
