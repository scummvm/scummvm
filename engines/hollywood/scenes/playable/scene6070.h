/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file for details.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6070_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6070_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene6070 : public PlayableScene {
public:
	Scene6070(HollywoodEngine *vm);

private:
	int alternatePaletteResourceChunkIndex() const override;
	bool isAlternatePaletteResourceActive() const override;
	bool shouldLoadArenaChunk(uint index) const override;
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
							 int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
							 byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
							 byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	void prepareCustomActorPathRoute(int startX, int startY) override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion,
							   const ActorPathBuildState &state, const ScenePoint &boundary,
							   int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion,
							 const ActorPathBuildState &state, int targetX, int targetY,
							 int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	byte primarySpeechVolumePercent(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetSceneLayers();
	void advanceSueIdle(uint32 delta);
	void advanceState609Prop(uint32 delta);
	void drawNormalForeground(int actorWorldX, int actorWorldY, byte actorDrawOrderMode);
	void runArrivalCutscene();
	void runState609Cutscene();
	void runSueDialogue();
	void initializeSueDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void runSueDiscovery();
	void handleGiveItemToSue();
	byte sueInventoryItemForRonItem(byte itemId) const;
	bool runCurtainRevealFromBlack();
	void runCurtainClearToBlack();
	void rememberOriginalColorMap();
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void rebuildSceneWalkableMask();

	Common::Array<byte> _originalColorToItemMap;
	ResourceSpriteLayer _sueLayer;
	ResourceSpriteLayer _arrivalLayer;
	ResourceSpriteLayer _state609SueLayer;
	ResourceSpriteLayer _state609PropLayer;
	ResourceSpriteLayer _state609NpcLayer;
	TimedAnimationChannel _sueIdleChannel;
	TimedAnimationChannel _state609PropChannel;
	uint32 _sueSpeechTimerAccumulator;
	byte _sueMode;
	byte _completedSueSpeechCount;
	byte _lastSueIdleSpeechFrame;
	bool _manualSequenceActive;
	bool _pendingRonRetort;
	bool _state609PropContinuous;
	bool _state609PropAlternatePose;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6070_H
