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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6080_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6080_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene6080 : public PlayableScene {
public:
	Scene6080(HollywoodEngine *vm);

private:
	enum LayerId {
		kSueNormalLayer,
		kSueAlternateLayer,
		kWaxBallLayer,
		kGuardNormalLayer,
		kGuardAlternateLayer
	};

	void initializeCustomPreviewState() override;
	bool shouldDrawSecondaryActorInPlayableComposite() const override;
	void runCustomEntrySequence() override;
	void prepareCustomGameplayLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetSceneLayers();
	void advanceSueIdle(uint32 delta);
	void finishSueIdleSequence();
	void runReturnConversation();
	void runWaxBallEscapeSequence();
	void startWaxBallAnimation();
	void advanceWaxBallAnimation(uint32 delta);
	void runFinalSueAnimation();
	void advanceFinalSueAnimation(uint32 delta);
	void dimEscapePalette();
	ResourceSpriteLayer &sueNormalLayer() { return _sceneLayers.layer(kSueNormalLayer); }
	ResourceSpriteLayer &sueAlternateLayer() { return _sceneLayers.layer(kSueAlternateLayer); }
	const ResourceSpriteLayer &sueAlternateLayer() const { return _sceneLayers.layer(kSueAlternateLayer); }
	ResourceSpriteLayer &waxBallLayer() { return _sceneLayers.layer(kWaxBallLayer); }
	ResourceSpriteLayer &guardNormalLayer() { return _sceneLayers.layer(kGuardNormalLayer); }
	ResourceSpriteLayer &guardAlternateLayer() { return _sceneLayers.layer(kGuardAlternateLayer); }

	TimedAnimationChannel _sueIdleChannel;
	uint _guardIdleTrack;
	TimedAnimationChannel _waxBallChannel;
	TimedAnimationChannel _escapeSueChannel;
	TimedAnimationChannel _escapeGuardChannel;
	TimedAnimationChannel _finalSueChannel;
	bool _sueLongIdleActive;
	bool _guardManualSequenceActive;
	bool _manualSequenceActive;
	bool _waxBallAnimationActive;
	bool _escapeLayersSwitched;
	bool _finalSueAnimationActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6080_H
