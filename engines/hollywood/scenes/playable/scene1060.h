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
	enum LayerId {
		kLargeBackgroundLayer,
		kInvisibleManLayer,
		kFlyDoctorLayer,
		kSmallLoopLayer,
		kSmallTriggerLayer
	};

	void initializeCustomPreviewState() override;
	void runCustomEntrySequence() override;
	void prepareCustomGameplayLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void advancePrimarySpeechAnimation(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldAnimatePrimarySpeechLine() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void advanceLargeBackground(uint32 delta);
	void advanceFlyDoctorAmbientAudio();
	void advanceFlyDoctorModeAndInvisibleMan(uint32 delta);
	void advanceA06PrimaryDialogueSpeechFrame(uint32 delta);
	void advanceFlyDoctor(uint32 delta);
	void advanceFlyDoctorIdle(uint32 delta);
	void advanceFlySlimeDrip(uint32 delta);
	void advanceTicketPickupFrame(uint32 delta);
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
	void handleCloakroomTicketPickup();
	void handlePocketPaperLook();
	void handleSkullcrackerExchange();
	void handleFlySlimeExchange();
	ResourceSpriteLayer &largeBackgroundLayer() { return _sceneLayers.layer(kLargeBackgroundLayer); }
	ResourceSpriteLayer &invisibleManLayer() { return _sceneLayers.layer(kInvisibleManLayer); }
	ResourceSpriteLayer &flyDoctorLayer() { return _sceneLayers.layer(kFlyDoctorLayer); }
	ResourceSpriteLayer &smallLoopLayer() { return _sceneLayers.layer(kSmallLoopLayer); }
	ResourceSpriteLayer &smallTriggerLayer() { return _sceneLayers.layer(kSmallTriggerLayer); }

	TimedAnimationChannel _largeBackgroundChannel;
	TimedAnimationChannel _invisibleManChannel;
	TimedAnimationChannel _invisibleManRandomChannel;
	TimedAnimationChannel _flyDoctorModeChannel;
	TimedAnimationChannel _flyDoctorIdleChannel;
	TimedAnimationChannel _flySlimeDripChannel;
	TimedAnimationChannel _smallTriggerChannel;
	uint _smallLoopTrack;
	byte _largeBackgroundMode;
	uint16 _largeBackgroundIdleCounter;
	byte _flyDoctorMode;
	byte _invisibleManMode;
	byte _lastInvisibleManRandomFrame;
	byte _lastFlyDoctorIdleFrame;
	byte _lastFlyDoctorIdleSound;
	byte _lastFlyDoctorDripSound;
	byte _smallTriggerMode;
	bool _ticketPickupSequenceActive;
	bool _pocketPaperPickupSequenceActive;
	bool _juniorPoseSequenceActive;
	bool _juniorConversationActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE1060_H
