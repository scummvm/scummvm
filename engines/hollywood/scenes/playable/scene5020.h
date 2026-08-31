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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5020_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5020_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5020 : public PlayableScene {
public:
	Scene5020(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	byte ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const override;

	void runMineCartArrival();
	void runExitToMineSwitches();
	void runPickupWoodenPlank();
	void drawForeground(byte actorDrawOrderMode, int actorWorldY);
	void copySlopeStepDeltasFromSet5A(uint firstOffset);
	void copyStageSmallRow(byte destinationRow, byte sourceRow);
	void clearSceneItemFromColorMap(byte itemId);

	ResourceSpriteLayer _mineCartLayer;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5020_H
