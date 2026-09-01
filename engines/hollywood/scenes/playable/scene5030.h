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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5030_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5030_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5030 : public PlayableScene {
public:
	Scene5030(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	byte ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	byte primarySpeechAnimationFrameCount(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;

	void resetAnimationLayers();
	void advanceLayer(TimedAnimationChannel &channel, uint layerIndex, uint frameCount, uint32 delta);
	void advanceDialogueIdleLayer(TimedAnimationChannel &channel, uint layerIndex,
		byte baseFrame, byte accentFrame, uint32 delta);
	void advanceRonDialogueIdle(uint32 delta);
	void advanceConcurrentPrimarySpeech(uint32 delta);
	void advanceScoutTransitions(uint32 delta);
	void startScoutStopTransition();
	void startScoutResumeTransition();
	void waitForScoutTransition();
	void finishScoutStopTransition();
	void finishScoutResumeTransition();
	void showRonConversationLayer(uint chunkIndex, byte baseFrame);
	void clearActorReplacementLayers();
	void finishScoutConversation();
	void runRonPoseTransition(bool faceGladys);
	void runDeckRefusalSequence();
	void runDeckPickupSequence();
	void runUnderpantsPresentationAnimation();
	void runUnderpantsHandoffAnimation();
	void beginConversationMusicSuppression();
	void runMineCartEntryAnimation();
	void runExitToMineSwitches();
	void runDeckOfCardsAction(bool fromUnderpantsExchange = false);
	void runVanessaConversation();
	void runGladysConversation();
	void runSpecialInventorySequence();
	void grantDeckOfCards();
	void initializeVanessaDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void initializeGladysDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte otherScoutFrameIndex) const;
	bool applyDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const;
	void beginRonDialogueLine(uint16 rowIndex, byte frameIndex);
	void beginVanessaSpeechLine(uint16 rowIndex, byte frameIndex);
	void beginGladysSpeechLine(uint16 rowIndex, byte frameIndex);
	bool startConcurrentPrimarySpeechCue(uint16 textRecordId, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte red, byte green, byte blue, byte animationGroup);
	bool startConcurrentPrimarySpeechLine(uint16 rowIndex, byte frameIndex,
		uint16 centerX, uint16 topY, byte red, byte green, byte blue, byte animationGroup);
	bool waitForConcurrentPrimarySpeech();
	void finishConcurrentPrimarySpeech();
	void runScoutSpeechLineDuringRonTurn(bool gladys, uint16 rowIndex, byte frameIndex);
	void copyStageSmallRowLabel(byte destinationRow, byte sourceRow);
	void clearSceneItemFromColorMap(byte itemId);

	uint _chunk8Track;
	TimedAnimationChannel _chunk9Channel;
	TimedAnimationChannel _chunk10Channel;
	TimedAnimationChannel _ronDialogueIdleChannel;
	bool _scoutStopTransitionActive;
	bool _scoutResumeTransitionActive;
	bool _scoutTransitionCompletionPending;
	bool _scoutsInDialoguePose;
	bool _musicSuppressed;
	bool _concurrentPrimarySpeechActive;
	uint32 _concurrentPrimarySpeechElapsed;
	uint32 _concurrentPrimarySpeechDuration;
	byte _ronSpeechBaseFrame;
	uint _ronConversationChunk;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5030_H
