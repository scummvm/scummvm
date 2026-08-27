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
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldAnimatePrimarySpeechLine() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void resetAnimationLayers();
	void advanceLargeBackground(uint32 delta);
	void advanceFlyDoctorAmbientAudio();
	void advanceFlyDoctorModeAndInvisibleMan(uint32 delta);
	void advanceA06PrimaryDialogueSpeechFrame(uint32 delta);
	void advanceFlyDoctor(uint32 delta);
	void advanceFlyDoctorIdle(uint32 delta);
	void advanceFlySlimeDrip(uint32 delta);
	void advanceFlySlimePickupFrame(uint32 delta);
	void advanceSmallLoop(uint32 delta);
	void advanceSmallTrigger(uint32 delta);
	void restartSmallTriggerLayerFromFlyDoctorFrame(byte flyDoctorFrame);
	byte juniorIdleFrame() const;
	void runJuniorPoseTransition(bool opening);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);
	void copyStageSmallRow(byte sourceRow, byte destinationRow);
	byte pickRandomFrameExcluding(byte frameCount, byte previousFrame);
	void prepareDrMoscaConversation();
	void prepareInvisibleManConversation();
	void runInvisibleManTransition(bool entering);
	void runJuniorConversation();
	void runDrMoscaConversation();
	void runInvisibleManConversation();
	void initializeDrMoscaDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void initializeInvisibleManDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void beginDrMoscaPrimarySpeech(byte frameIndex);
	void beginInvisibleManPrimarySpeech(byte frameIndex, bool allowRandomTransition);
	void finishCharacterConversation();
	void handlePocketPaperPickup();
	void runPocketPaperPickupSequence();
	void handleFlySlimePickup();
	void handlePocketPaperLook();
	void runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame = -1, byte patchSelector = 0);

	TimedAnimationChannel _largeBackgroundChannel;
	TimedAnimationChannel _invisibleManChannel;
	TimedAnimationChannel _invisibleManRandomChannel;
	TimedAnimationChannel _flyDoctorModeChannel;
	TimedAnimationChannel _flyDoctorIdleChannel;
	TimedAnimationChannel _flySlimeDripChannel;
	TimedAnimationChannel _smallLoopChannel;
	TimedAnimationChannel _smallTriggerChannel;
	ResourceSpriteLayer _largeBackgroundLayer;
	ResourceSpriteLayer _invisibleManLayer;
	ResourceSpriteLayer _flyDoctorLayer;
	ResourceSpriteLayer _smallLoopLayer;
	ResourceSpriteLayer _smallTriggerLayer;
	byte _largeBackgroundMode;
	uint16 _largeBackgroundIdleCounter;
	byte _flyDoctorMode;
	byte _invisibleManMode;
	byte _lastInvisibleManRandomFrame;
	byte _lastFlyDoctorIdleFrame;
	byte _lastFlyDoctorIdleSound;
	byte _lastFlyDoctorDripSound;
	byte _smallTriggerMode;
	bool _flySlimePickupSequenceActive;
	bool _pocketPaperPickupSequenceActive;
	bool _juniorPoseSequenceActive;
	bool _juniorConversationActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE1060_H
