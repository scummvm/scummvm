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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE1070_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE1070_H

#include "hollywood/gameplay/dialogue_menu.h"
#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene1070 : public PlayableScene {
public:
	Scene1070(HollywoodEngine *vm);

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

	void resetAnimationLayers();
	void advanceBackLayer(uint32 delta);
	void advanceCharacterAmbientAudio();
	void advanceSpencerAmbientTrigger(uint32 delta);
	void advanceGhostLayer(uint32 delta);
	void advanceSpencerLayer(uint32 delta);
	void updateSpiritBlockingHotspot(bool blocked);
	void settleCharacterAnimations();
	bool hasActiveCharacterAnimation() const;
	void waitForGhostMode(byte mode);
	void waitForSpencerMode(byte mode);
	void beginSpencerPrimarySpeechLine(byte frameIndex, byte openFrame);
	void beginQuasimodoPrimarySpeechLine(byte frameIndex);
	void drawForegroundBlocks(int activeWorldX, int activeWorldY);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);
	void copyStageSmallRow(byte sourceRow, byte destinationRow);
	void runSpencerConversation();
	void runQuasimodoConversation();
	void finishSpencerConversation();
	void finishQuasimodoConversation();
	void initializeSpencerDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void initializeQuasimodoDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	void handleSpencerDialogueEffect(byte effect, Common::Array<DialogueChoiceRecord> &records,
		uint recordIndex);
	void runSpencerTravelUnlockEffect();
	void handleMicrophonePickup();
	void handleMicStandPickup();
	void handleBallChainPickup();
	void runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame = -1, byte patchSelector = 0);

	TimedAnimationChannel _backLayerChannel;
	TimedAnimationChannel _ghostIdleChannel;
	TimedAnimationChannel _ghostSequenceChannel;
	TimedAnimationChannel _ghostSpecialChannel;
	TimedAnimationChannel _spencerAmbientChannel;
	TimedAnimationChannel _spencerIdleChannel;
	TimedAnimationChannel _spencerLongChannel;
	TimedAnimationChannel _spencerTransitionChannel;
	ResourceSpriteLayer _backLayer;
	ResourceSpriteLayer _ghostLayer;
	ResourceSpriteLayer _spencerLayer;
	byte _backLayerMode;
	byte _ghostMode;
	byte _spencerMode;
	byte _spencerAmbientState;
	byte _lastGhostAmbientSound;
	byte _lastSpencerAmbientSound;
	bool _suppressRandomLayerStarts;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE1070_H
