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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6080_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6080_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene6080 : public PlayableScene {
public:
	Scene6080(HollywoodEngine *vm);

private:
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
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetSceneLayers();
	void advanceSueIdle(uint32 delta);
	void advanceGuardIdle(uint32 delta);
	void finishSueIdleSequence();
	void runReturnConversation();
	void runWaxBallEscapeSequence();
	void startWaxBallAnimation();
	void advanceWaxBallAnimation(uint32 delta);
	void startManualActorPath(int targetX, int targetY, byte finalFacing);
	void advanceManualActorPath(uint32 delta);
	void runFinalSueAnimation();
	void advanceFinalSueAnimation(uint32 delta);
	void dimEscapePalette();

	ResourceSpriteLayer _sueNormalLayer;
	ResourceSpriteLayer _sueAlternateLayer;
	ResourceSpriteLayer _guardNormalLayer;
	ResourceSpriteLayer _guardAlternateLayer;
	ResourceSpriteLayer _waxBallLayer;
	TimedAnimationChannel _sueIdleChannel;
	TimedAnimationChannel _guardIdleChannel;
	TimedAnimationChannel _waxBallChannel;
	TimedAnimationChannel _escapeSueChannel;
	TimedAnimationChannel _escapeGuardChannel;
	TimedAnimationChannel _manualActorPathChannel;
	TimedAnimationChannel _finalSueChannel;
	uint _manualActorPathFrameIndex;
	bool _sueLongIdleActive;
	bool _guardManualSequenceActive;
	bool _manualSequenceActive;
	bool _waxBallAnimationActive;
	bool _escapeLayersSwitched;
	bool _manualActorPathActive;
	bool _finalSueAnimationActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6080_H
