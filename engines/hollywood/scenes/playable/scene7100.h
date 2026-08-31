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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7100_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7100_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene7100 : public PlayableScene {
public:
	Scene7100(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void prepareCustomGameplayLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void suspendAudioForOptionsMenu() override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	byte primarySpeechVolumePercent(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void rebuildWalkableMask();
	void advancePrimaryIdleFrame(uint32 delta);
	void advanceAutonomousBanter(uint32 delta);
	void advanceEnvironmentFrame(uint32 delta);
	void drawPrimaryNpc();
	void drawEnvironmentOverlayBeforeActor();
	void drawEnvironmentOverlayAfterForeground();
	void startAutonomousBanter();
	void startScriptedRonSpeech(uint16 rowIndex, byte frameIndex, uint16 centerX,
		byte red, byte green, byte blue, byte animationGroup);
	void startAsyncPrimarySpeechPart();
	void advanceAsyncSpeech(uint32 delta);
	void finishAsyncPrimarySpeech();
	void waitForScriptedRonSpeech();
	void startAutonomousSueReply();
	void cancelAsyncSpeech();
	void faceSueTowardRon();
	void runRonDialogue();
	void initializeRonDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void runRescueEntrySequence();
	bool runRescueDialogue();
	void initializeRescueDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	bool runCurtainRevealFromBlack();
	void runCurtainClearToBlack();
	void handlePickupItem15();
	void handleActionHandler315();
	void handlePlateOnMousetrap();
	void handleCaptureRat();
	void handleRemovePlate();
	void handleInventoryTransferAction();

	SpeechPlayer _asyncSpeechPlayer;
	uint32 _primaryTimerAccumulator;
	uint32 _banterTimerAccumulator;
	uint32 _asyncSpeechElapsed;
	uint32 _asyncSpeechDuration;
	uint32 _environmentTimerAccumulator;
	uint16 _asyncTextRecordId;
	uint16 _asyncVoiceSampleId;
	uint16 _asyncSpeechCenterX;
	byte _primaryMode;
	byte _primaryFrame;
	byte _primaryAltFrame;
	byte _environmentState;
	byte _environmentFrame;
	byte _lastBanterFrame;
	byte _banterRemarkCount;
	byte _asyncContinuationPart;
	byte _asyncContinuationCount;
	byte _asyncSpeechPhase;
	byte _asyncPrimaryAnimationGroup;
	bool _manualPrimaryAnimationActive;
	bool _dialogueMenuActive;
	bool _specialBanterUsed;
};

} // End of namespace Hollywood

#endif
