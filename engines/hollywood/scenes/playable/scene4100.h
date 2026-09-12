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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4100_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4100_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4100 : public PlayableScene {
public:
	Scene4100(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void runCustomEntrySequence() override;
	void runExitSideEffectsAfterLoop() override;
	void prepareCustomGameplayLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	bool revealEntryPose(int x, int y, byte facing);
	void applyD10PaletteDimming();
	void resetPaletteCycle();
	void advancePaletteCycle(uint32 delta);
	void rotatePaletteCycle();
	void resetAmbientSoundScheduler();
	void advanceAmbientSound(uint32 delta);
	void copySpecialStepDeltas(uint destinationOffset);
	void runFirstDoorAction();
	void runDoorTransition(uint chunkIndex, uint descriptorCount, uint16 targetState);

	TimedAnimationChannel _paletteCycleChannel;
	uint32 _ambientSoundTimerAccumulator;
	byte _previousAmbientSoundCue;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4100_H
