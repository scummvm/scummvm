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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5080_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5080_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5080 : public PlayableScene {
public:
	Scene5080(HollywoodEngine *vm);

private:
	uint resource000ActorBankTableEntry() const override;
	const byte *actorPathStepDeltaTable() const override;
	uint actorPathStepDeltaTableSize() const override;
	void initializeCustomPreviewState() override;
	void prepareCustomComposite(bool drawActors, byte activeFacing, int activeWorldX,
		int activeWorldY, byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;
	byte ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const override;

	void runMineCartEntryClip();
	void runExitToMineSwitches();
	void runBookPickup();
	void runWardrobeAttempt();
	void runPassageSideSwitch();
	void handleStairDoorBarrier();
	void handleStairDoorConstruction();
	void handleStairDoorUseOrOpen();
	void handleStairDoorClose();
	void handlePassageUnlock();
	void switchActiveActorBankForCurrentSide();
	void rebuildWalkableMaskForCurrentSide();
	void updateSceneActorDepthAndPalette(byte facing, int worldX, int worldY);
	void clearSceneItemFromColorMap(byte itemId);
	void remapSceneItemInColorMap(byte fromItemId, byte toItemId);
	void remapActorDepthClass(byte fromClass, byte toClass);
	void copyRouteStepDeltas(uint destinationFirst, const byte *source, uint sourceSize, uint sourceFirst);
	void copyStepDeltasForCurrentSide();

	bool _mineCartRumbleActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5080_H
