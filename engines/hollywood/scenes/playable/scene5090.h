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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5090_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5090_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5090 : public PlayableScene {
public:
	Scene5090(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	void prepareCustomActorPathRoute(int startX, int startY) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void applyScenePaletteDimming();
	void resetPaletteAnimations();
	void advancePaletteAnimations(uint32 delta);
	void rotateWaterfallPalette();
	void advanceLagoonPalette();
	void runEntryClip();
	bool waitEntryClipFrameMillis(uint32 millis);
	void runReturnToMineSwitches();
	void runFillWaterContainer();
	void rebuildWalkableMask();

	TimedAnimationChannel _waterfallPaletteChannel;
	TimedAnimationChannel _lagoonPaletteChannel;
	byte _lagoonPalettePhase;
	bool _lagoonPaletteReverse;
	bool _mineCartRumbleActive;
	byte _routeStartRegion;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5090_H
