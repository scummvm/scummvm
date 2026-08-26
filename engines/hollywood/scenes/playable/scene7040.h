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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7040_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7040_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

/**
 * Supports mutually exclusive resource modes based on whether the Frankenstein
 * note has been reviewed. That state selects an alternate palette and animation
 * set; exit cleanup clears it only after the office note pickup is complete and
 * the actor leaves for scene 7050.
 */
class Scene7040 : public PlayableScene {
public:
	Scene7040(HollywoodEngine *vm);

private:
	int alternatePaletteResourceChunkIndex() const override;
	bool isAlternatePaletteResourceActive() const override;
	bool shouldConvertSavedFramebufferFF() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void runJosephGuestListGreeting();
	void waitPreItemIdleSequence();
	bool shouldStopJosephGuestListGreeting();
	void advanceChunk11PreItemIdleAnimation(uint32 delta);
	void advanceChunk16PostItemAnimation(uint32 delta);
	void advancePrimaryDialogueSpeechFrame(uint32 delta);
	byte pickPrimarySpeechFrameExcluding(byte frameCount, byte previousFrame);
	void handleActionSlot00ReturnToG03();
	void handleActionSlot01ProgressSpeech();
	void handleActionSlot02MajorHotspotAction();
	void handleActionSlot03TransitionToState7060();
	void handleActionSlot05ExitProgressSpeech();
	void handleActionSlot06TransitionToG05();
	void handleActionSlot09PickupItem0FThenExit();
	void handleActionSlot10CommonSpeech();
	void handleActionHandler312ProgressSpeech();
	void handleActionHandler313ConversationGate();
	void handleActionHandler314FrankensteinNoteSpeech();
	void handleActionHandler315PickupItem0C();
	void runDialogueMenuRow98();
	void initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void runMajorHotspotFrankensteinBranch();
	void runMajorHotspotReturnPath(byte finalFacing);
	void runChunk11Range(byte firstFrame, byte endFrame);
	void runChunk14ActionRange(byte firstFrame, byte endFrame);
	void runChunk14AltRange(uint chunkIndex, byte firstFrame, byte endFrame);
	void applyChunk14ActionSideEffects(byte frameIndex);
	void applyChunk14AltSideEffects(byte frameIndex);
	void configureAnimationLayers();
	void resetTransientAnimationLayers();
	void syncAnimationLayerFrames();
	void setChunk12OverlayVisible(bool visible);
	void setChunk12OverlayFrame(byte frameIndex);
	void setChunk14ActionVisible(bool visible);
	void setChunk14ActionFrame(byte frameIndex);
	void configureChunk14AltLayer(uint chunkIndex, bool visible);
	void setChunk14AltVisible(bool visible);
	void setChunk14AltFrame(byte frameIndex);
	uint chunk14AltChunkIndex() const;

	byte _postItemIdleState;
	byte _primarySpeechLeadInTicks;
	byte _primarySpeechLastMouthFrameOffset;
	SceneAnimationLayers _animationLayers;
	RandomIdleAnimation _preItemIdleAnimation;
	TimedAnimationChannel _postItemAnimation;
	TimedAnimationChannel _chunk17Animation;
};

} // End of namespace Hollywood

#endif
