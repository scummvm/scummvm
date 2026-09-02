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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4050_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4050_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4050 : public PlayableScene {
public:
	Scene4050(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runExitSideEffectsAfterLoop() override;
	void prepareCustomGameplayLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldDrawSecondaryActorInPlayableComposite() const override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void restoreResourceLayerPalette();
	void advanceFlagPalette(uint32 delta);
	void rotateFlagPalette();
	void advanceRonLayer(uint32 delta);
	void updateAmbientSounds(uint32 delta);
	void setRonResourceFrame(byte frameIndex);
	void beginRonResourceSpeechLine(uint16 rowIndex, byte frameIndex);
	void runD09ReturnTransitionSequence();
	bool runCurtainRevealFromBlack();
	void applyCurtainBand(const Graphics::Surface *source, uint sweepOffset, byte bandWidth);
	void runCurtainClearToBlack();
	void useLongRopeOnLedge();
	void useSceneRope();
	void applyPatchStateColorMap(byte patchState);
	bool resourceDescriptorBounds(uint chunkIndex, uint16 descriptorCount, uint16 descriptorIndex,
		int &left, int &top, int &width, int &bottom) const;

	TimedAnimationChannel _flagPaletteChannel;
	TimedAnimationChannel _ronSpeechChannel;
	TimedAnimationChannel _ronIdleChannel;
	uint _backgroundTrack;
	uint32 _ambientEffectTimerAccumulator;
	byte _previousContinuousAmbientCue;
	byte _previousRandomAmbientCue;
	bool _ronManualSequenceActive;
	bool _transitionClearedToBlack;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4050_H
