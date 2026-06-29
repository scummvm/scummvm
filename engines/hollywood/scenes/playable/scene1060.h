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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE1060_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE1060_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene1060 : public PlayableScene {
public:
	Scene1060(HollywoodEngine *vm);

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
	bool shouldAnimatePrimarySpeechLine() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void advanceLargeBackground(uint32 delta);
	void advanceInvisibleMan(uint32 delta);
	void advanceFlyDoctor(uint32 delta);
	void advanceSmallLoop(uint32 delta);
	void advanceSmallTrigger(uint32 delta);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);
	void copyStageSmallRow(byte sourceRow, byte destinationRow);
	void runJuniorConversation();
	void runDrMoscaConversation();
	void runInvisibleManConversation();
	void handlePocketPaperPickup();
	void handlePocketPaperTakeAction();
	void handlePocketPaperLook();
	void runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame = -1, byte patchSelector = 0);

	TimedAnimationChannel _largeBackgroundChannel;
	TimedAnimationChannel _invisibleManChannel;
	TimedAnimationChannel _flyDoctorChannel;
	TimedAnimationChannel _smallLoopChannel;
	TimedAnimationChannel _smallTriggerChannel;
	ResourceSpriteLayer _largeBackgroundLayer;
	ResourceSpriteLayer _invisibleManLayer;
	ResourceSpriteLayer _flyDoctorLayer;
	ResourceSpriteLayer _smallLoopLayer;
	ResourceSpriteLayer _smallTriggerLayer;
	byte _largeBackgroundMode;
	uint16 _largeBackgroundIdleCounter;
	byte _smallTriggerMode;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE1060_H
