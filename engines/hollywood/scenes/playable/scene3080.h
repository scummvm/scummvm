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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3080_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3080_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

// Saved GameplayState fields read:
// mainFlowStateId, scene3080EntryLineSeen, scene3080CabinDoorVisited,
// scene3080FrankensteinDiaryRevealed, scene3080FrankensteinDiaryTaken,
// scene3080WindowOpened, scene3080BranchTaken,
// scene3080ChimneySmokeAnimationChanged.
// Saved GameplayState fields written:
// mainFlowStateId, scene3080EntryLineSeen, scene3080CabinDoorVisited,
// scene3080FrankensteinDiaryRevealed, scene3080FrankensteinDiaryTaken,
// scene3080BranchTaken.
class Scene3080 : public PlayableScene {
public:
	Scene3080(HollywoodEngine *vm);

private:
	bool hasCustomPreviewState() const override;
	void initializeCustomPreviewState() override;
	bool hasCustomComposite() const override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool hasCustomEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void rebuildWalkableMask();
	void removeColorMapItem(byte itemId);
	void restoreOrRemoveDiaryHotspot();
	void advanceLargeLayer(uint32 delta);
	void advanceSmallIdleLayer(uint32 delta);
	void runEntryFromForest();
	void runEntryFromCabin();
	void runEntryFromBrook();
	void runDiaryPickup();
	void runStickPickup();
	void runBranchExchangeOverlay();
	void drawForegroundBlocks(int activeWorldY);

	TimedAnimationChannel _largeChannel;
	TimedAnimationChannel _smallIdleChannel;
	ResourceSpriteLayer _largeLayer;
	ResourceSpriteLayer _smallIdleLayer;
	byte _smallIdleMode;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3080_H
