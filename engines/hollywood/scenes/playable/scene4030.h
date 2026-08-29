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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4030_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4030_H

#include "hollywood/scenes/playable/animation_channels.h"
#include "hollywood/scenes/playable/playable_scene.h"

namespace Graphics {
class ManagedSurface;
}

namespace Hollywood {

class HollywoodEngine;

class Scene4030 : public PlayableScene {
public:
	Scene4030(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void initializeSpriteLayers();
	void advanceBackgroundAnimations(uint32 delta);
	void advanceLeftPropLayer(uint frameCount);
	void advanceRightPropLayer(uint frameCount);
	void advanceMarkerPixels(uint32 delta);
	void drawMarkerPixels();
	void runTowerTransitionToScene4040();
	bool runTowerTransitionClip(bool returningFromTower);
	void drawTowerTransitionFrame(const Graphics::ManagedSurface &transitionBackground);
	bool drawClipFrameDeltaToSurface(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::ManagedSurface &destination);
	void takeRope();
	void talkToSkeleton();
	void playBoneRevealAnimation();
	void takeBone();
	void installImprovisedLever();
	void updateIronMaidenMechanism();
	void playIronMaidenMechanismAnimation(byte secondaryFrameGroup);
	void copyStageItemName(byte destinationItem, byte sourceItem);
	void removeColorMapItem(byte itemId);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);

	ResourceSpriteLayer _leftPropLayer;
	ResourceSpriteLayer _rightPropLayer;
	ResourceSpriteLayer _secondaryActionLayer;
	ResourceSpriteLayer _primaryActionLayer;
	TimedAnimationChannel _leftPropChannel;
	TimedAnimationChannel _rightPropChannel;
	TimedAnimationChannel _markerChannel;
	Common::Array<byte> _originalStageSmallRows;
	bool _markerDark[7];
	byte _rightPropState;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4030_H
