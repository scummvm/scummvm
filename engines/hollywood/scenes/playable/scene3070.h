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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3070_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3070_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene3070 : public PlayableScene {
public:
	Scene3070(HollywoodEngine *vm);

private:
	bool shouldLoadArenaChunk(uint index) const override;
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	bool shouldAnimatePrimarySpeechLine() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetCutsceneLayers();
	void rebuildWalkableMask();
	void removeColorMapItem(byte itemId);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void fixControlPanelInteractionPoint();
	void copySpecialStepDeltas(uint firstOffset, uint lastOffset);
	void runEntryFromSecretPassage();
	void runEntryFromOtherSide();
	void runLateCutsceneBranch();
	void runInterludeCutscene();
	void runDoorPatchOverlay(bool open);
	void runItemPatchPickup();
	void runFrankensteinRevival();
	void runBrainInstallation();
	void runBodyAssembly();
	void addSerumIngredient(byte itemId, uint16 speechRow, bool speakBefore, bool useSyringeAnimation);
	void applyActionPatchChunk(uint chunkIndex);
	bool runCurtainRevealFromBlack();
	void runCurtainClearToBlack();
	void drawForegroundBlocks(int activeWorldY, byte actorDrawOrderMode, bool drawNearForeground);

	ResourceSpriteLayer _interludeLeftLayer;
	ResourceSpriteLayer _interludeRightLayer;
	ResourceSpriteLayer _lateCutsceneLayer;
	bool _interludeActive;
	bool _interludeAlternatePose;
	bool _lateCutsceneActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3070_H
