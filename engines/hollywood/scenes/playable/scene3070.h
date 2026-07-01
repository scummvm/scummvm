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
	const char *resourceArchiveName() const override;
	uint sceneInitialRequiredChunkCount() const override;
	uint sceneArenaFirstChunk() const override;
	uint sceneArenaLastChunk() const override;
	uint sceneStageIndex() const override;
	const char *sceneDebugName() const override;
	uint16 sceneViewportXOffset() const override;
	uint16 sceneViewportMinXOffset() const override;
	uint16 sceneViewportMaxXOffset() const override;
	byte inventoryOwnerIndex() const override;
	void initializeInventoryOwnerState() override;
	uint resource000ActorBankTableEntry() const override;
	uint resource000ActorPaletteTableEntry() const override;
	uint32 inventoryActionTableExtraOffset() const override;
	uint resource003InventoryRowsOffsetIndex() const override;
	uint32 speechCueDescriptorTableOffset() const override;
	const byte *actorPathStepDeltaTable() const override;
	uint actorPathStepDeltaTableSize() const override;
	byte walkablePaletteMaxRegion() const override;
	const char *musicArchiveName() const override;
	const char *soundBank0ArchiveName() const override;
	bool usesActorDepthTest() const override;
	bool isMainFlowStateInScene(uint16 stateId) const override;
	bool hasCustomPreviewState() const override;
	void initializeCustomPreviewState() override;
	bool hasCustomComposite() const override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool hasCustomEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void rebuildWalkableMask();
	void removeColorMapItem(byte itemId);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void advanceBackLayer(uint32 delta);
	void advanceSmallIdleLayer(uint32 delta);
	void runEntryFromSecretPassage();
	void runEntryFromOtherSide();
	void runLateCutsceneBranch();
	void runInterludeCutscene();
	void runDoorPatchOverlay(bool open);
	void runItemPatchPickup();
	void drawForegroundBlocks(int activeWorldY, byte actorDrawOrderMode);

	TimedAnimationChannel _backChannel;
	TimedAnimationChannel _smallIdleChannel;
	ResourceSpriteLayer _backLayer;
	ResourceSpriteLayer _smallIdleLayer;
	byte _smallIdleMode;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3070_H
