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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5040_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5040_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5040 : public PlayableScene {
public:
	Scene5040(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void prepareCustomComposite(bool drawActors, byte activeFacing, int activeWorldX,
		int activeWorldY, byte actorDrawOrderMode) override;
	void drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void advancePrimarySpeechAnimation(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;
	byte ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	byte primarySpeechAnimationFrameCount(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;

	void resetAnimationLayers();
	void advanceKarlLayer(uint32 delta);
	void advanceKarlMiningSpeech(uint32 delta);
	bool playKarlFrames(byte firstFrame, byte lastFrame, uint32 frameMillis);
	bool playKarlTransition(byte firstFrame, byte lastFrame, byte finalFrame, uint32 frameMillis);
	void settleKarlForConversation();
	void runMineCartEntryClip();
	void runExitToMineSwitches();
	void runExitToMineHole();
	void runKarlConversation();
	void runMineBoxLook();
	void runPatchedSockPickup();
	void runMineKeyPickup();
	void runSpecialMineExitWithMagneticPillbox();
	void runDowsingRodSwap();
	void runRestoredDowsingRodOffer();
	void initializeKarlDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setKarlDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	bool applyKarlDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const;
	void beginKarlSpeechLine(byte frameIndex);
	void beginKarlRestoredDowsingRodReply();
	void beginKarlMiningSpeechLine(byte frameIndex);
	void copyStageSmallRow(byte destinationRow, byte sourceRow);
	void remapSceneColors(byte sourceColor, byte itemId);
	void clearSceneItemFromColorMap(byte itemId);

	TimedAnimationChannel _karlIdleChannel;
	byte _karlIdleFrame;
	byte _karlIdleMode;
	byte _karlStrikeRepeatCount;
	bool _suspendKarlIdle;
	bool _mineCartRumbleActive;
	byte _lastKarlMiningSpeechFrame;
	byte _previousKarlMiningSpeechFrame;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5040_H
