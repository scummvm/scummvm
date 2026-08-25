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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4040_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4040_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4040 : public PlayableScene {
public:
	Scene4040(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void applyScenePaletteOverride();
	void resetBackgroundLayers();
	void drawBackgroundLayers();
	void advanceBackgroundLayers(uint32 delta);
	void advanceRandomBackgroundTick();
	bool isRandomBackgroundHoldFrame(byte frameIndex) const;
	void runReturnFromUpperExitEntry();
	void runStairReturnToDungeon();
	void takeCandil();
	void drawForegroundBlocks(int activeWorldY);
	void removeColorMapItem(byte itemId);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);

	TimedAnimationChannel _cyclicBackgroundChannel;
	TimedAnimationChannel _randomBackgroundChannel;
	ResourceSpriteLayer _cyclicBackgroundLayer;
	ResourceSpriteLayer _randomBackgroundLayer;
	byte _randomBackgroundState;
	byte _randomBackgroundRepeatCount;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4040_H
