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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2070_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2070_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2070 : public PlayableScene {
public:
	Scene2070(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void resetForegroundLayer();
	void advanceForegroundLayer(uint32 delta);
	void runEntryFromLabyrinth();
	void runEntryFromRightPassage();
	void runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel);
	bool runSealMemoryEffect();
	void drawSealMemoryDeltaLayer();
	void runAnimatedInventoryStateChange();
	void rebuildWalkableMask();
	void setColorMapItem(byte sourceItem, byte destinationItem);

	TimedAnimationChannel _foregroundChannel;
	ResourceSpriteLayer _foregroundLayer;
	bool _sealMemoryActive;
	byte _sealMemoryFrame;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2070_H
