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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2040_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2040_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2040 : public PlayableScene {
public:
	Scene2040(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	void prepareCustomActorPathRoute(int startX, int startY) override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void rebuildWalkableMask();
	void updateSceneDepthThresholds(byte actorDrawOrderMode);
	void resetForegroundLayer();
	void advanceForegroundLayer(uint32 delta);
	void runEntryFromMarket();
	void runEntryFromInterior();
	void runExitToInterior();
	void runFlowerPickup();
	void runSphinxNoseSequence();
	void runSeedPlantingSequence();
	void runEyeExchangeSequence();
	void runBaseOpeningSequence();
	void runBaseOpeningDeltaSequence();
	void installEyeEffectPalette();
	void restoreEyeEffectPalette();
	void rotateEyeEffectPalette();
	void copyStepDeltasFromB4(uint targetOffset, uint sourceOffset, uint count);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);
	void restoreOriginalColorMapItem(byte itemId);
	byte originalColorMapItemAt(uint paletteIndex) const;

	TimedAnimationChannel _foregroundChannel;
	ResourceSpriteLayer _behindActorLayer;
	ResourceSpriteLayer _foregroundLayer;
	int _routeStartX;
	int _routeStartY;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2040_H
