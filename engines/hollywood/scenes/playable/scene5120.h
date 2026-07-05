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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5120_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5120_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

// Saved GameplayState fields read:
// mainFlowStateId, scene1070SpencerCocktailRecipeLearned,
// scene5110SalonTransformState, scene5110JacuzziInspectionState,
// scene5110WerewolfDialogueChoiceUnlocked, ronPendingMabusePillsInMagnetPillbox,
// scene5120TongsTaken, scene5120CocktailState.
// Saved GameplayState fields written:
// mainFlowStateId, scene5110SalonTransformState,
// scene5110WerewolfDialogueChoiceUnlocked, scene5120TongsTaken,
// scene5120CocktailState.
class Scene5120 : public PlayableScene {
public:
	Scene5120(HollywoodEngine *vm);

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
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldConvertSavedFramebufferFF() const override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleActionOverlayFrameHook(byte hookId, uint frame) override;

	void runFirstEntrySequence();
	void runAlternateEntrySequence();
	void runElevatorDoorClip(bool opening);
	void runElevatorReturnSequence();
	void runElevatorExitToState(uint16 nextState);
	void runTongsPickup();
	void runCocktailFillPillbox();
	void runFilmProjectorSequence();
	void runUseShaker();
	void initializeTransformedRoomLayers();
	void resetTransformedRoomLayers();
	void advanceTransformedRoomLayers(uint32 delta);
	void drawTransformedRoomLayers();
	void drawStaticForegroundLayers(byte actorDrawOrderMode);
	void clearSceneItemFromColorMap(byte itemId);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void rebuildWalkableMask();

	ResourceSpriteLayer _movingWallLayer;
	ResourceSpriteLayer _mainProjectionLayer;
	ResourceSpriteLayer _sideLoopLayer;
	ResourceSpriteLayer _toggleLayer;
	ResourceSpriteLayer _randomDetailLayer;
	TimedAnimationChannel _movingWallChannel;
	TimedAnimationChannel _mainProjectionChannel;
	TimedAnimationChannel _sideLoopChannel;
	TimedAnimationChannel _toggleChannel;
	TimedAnimationChannel _randomDetailChannel;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5120_H
