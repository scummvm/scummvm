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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE8020_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE8020_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene8020 : public PlayableScene {
public:
	Scene8020(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void runExitSideEffectsAfterLoop() override;
	void drawCustomBackgroundComposite(int activeWorldX, int activeWorldY) override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void advanceAmbientAudio(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;

	void resetForegroundLayer();
	void advanceForegroundLayer(uint32 delta);
	bool waitForForegroundAnimationIdle();
	bool isWalkableAt(int x, int y) const;
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void runExitToScene8010();
	void runPickupInventoryItem6cSequence();
	void runPickupInventoryItem5dSequence();
	void runRemoveInventoryItem6cSequence();
	void runForegroundTransformationSequence();

	TimedAnimationChannel _foregroundChannel;
	TimedAnimationChannel _secondaryAmbientChannel;
	Common::Array<byte> _originalColorToItemMap;
	byte _foregroundAnimationState;
	byte _foregroundRepeatCount;
	byte _previousPrimaryAmbientCue;
	byte _previousSecondaryAmbientCue;
	bool _foregroundSequenceLocked;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE8020_H
