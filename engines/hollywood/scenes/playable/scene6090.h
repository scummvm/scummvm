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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6090_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6090_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

// Runs the wax-pool confrontation, timed rope rescue, and transition to the ending.
class Scene6090 : public PlayableScene {
public:
	Scene6090(HollywoodEngine *vm);

private:
	enum CompositeMode {
		kIntroComposite,
		kRescueComposite,
		kSpecialComposite,
		kEscapeComposite
	};

	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldApplyGameplayPanelObjectPalette() const override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool shouldPlayGameplayClickPath() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void clearRonInventory();
	void restoreTiedSequencePalette();
	void resetSceneLayers();
	void drawActorFrames(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY);
	bool playLayerTransition(ResourceSpriteLayer &layer, byte firstFrame, byte lastFrame,
		uint32 frameMillis);
	bool runCurtainRevealFromBlack();
	void runOpeningConversation();

	void advanceTiedRonIdle(uint32 delta);
	void advanceAmbientLayers(uint32 delta);
	void advanceSueIdle(uint32 delta);
	void advanceMechanism(uint32 delta);
	void advanceHannoverPose();
	void runInterruptionClips();

	void startAsyncPrimarySpeechLine(uint16 rowIndex, byte frameIndex,
		uint16 centerX, uint16 topY, byte red, byte green, byte blue,
		byte animationGroup, byte volumePercent = 100);
	void startAsyncPrimarySpeechPart();
	void advanceAsyncPrimarySpeech(uint32 delta);
	void stopAsyncPrimarySpeech();
	void waitForAsyncPrimarySpeech();

	void applyPatchChunk(uint chunkIndex);
	void runRopeRescueSequence();
	void advanceEscapeAnimation(uint32 delta);
	void advanceEscapePalette(uint32 delta);
	void dimEscapePaletteStep();

	ResourceSpriteLayer _leftAmbientLayer;
	ResourceSpriteLayer _rightAmbientLayer;
	ResourceSpriteLayer _tiedRonLayer;
	ResourceSpriteLayer _hannoverLayer;
	ResourceSpriteLayer _karloffLayer;
	ResourceSpriteLayer _sueFaceLayer;
	ResourceSpriteLayer _apparatusLayer;
	ResourceSpriteLayer _rescueRonLayer;
	ResourceSpriteLayer _rescueHannoverLayer;
	ResourceSpriteLayer _rescueKarloffLayer;
	ResourceSpriteLayer _rescueApparatusLayer;
	ResourceSpriteLayer _rescueEffectLayer;
	ResourceSpriteLayer _escapeBackdropLayer;
	ResourceSpriteLayer _freedSueLayer;
	ResourceSpriteLayer _specialEffectLayer;
	TimedAnimationChannel _tiedRonChannel;
	TimedAnimationChannel _ambientLayerChannel;
	TimedAnimationChannel _sueIdleChannel;
	TimedAnimationChannel _mechanismChannel;
	TimedAnimationChannel _escapeChannel;
	TimedAnimationChannel _paletteFadeChannel;
	SoundBank0Player _secondaryEffectSound;
	Common::Array<byte> _escapePaletteSource;

	CompositeMode _compositeMode;
	byte _tiedRonIdleFrame;
	byte _sueIdleMode;
	byte _sueIdleRepeatCount;
	byte _mechanismState;
	byte _speakerMode;
	byte _hannoverPoseMode;
	byte _interruptionCycleCount;
	byte _paletteFadeThreshold;
	bool _delayedEventDone;
	bool _postRescue;
	bool _automaticEventRunning;
	bool _manualSequenceActive;
	bool _freedSueActive;
	bool _escapeAnimationActive;
	bool _escapePaletteActive;
	bool _paletteLockedDark;
	bool _muffledSpeechStarted;

	bool _asyncPrimaryActive;
	bool _asyncPrimaryAnimated;
	uint16 _asyncTextRecordId;
	uint16 _asyncVoiceSampleId;
	uint16 _asyncCenterX;
	uint16 _asyncTopY;
	byte _asyncPartCount;
	byte _asyncPartIndex;
	byte _asyncAnimationGroup;
	byte _asyncColorIndex;
	byte _asyncVolumePercent;
	uint32 _asyncPartRemainingMillis;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6090_H
