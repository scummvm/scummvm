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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3060_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3060_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene3060 : public PlayableScene {
public:
	Scene3060(HollywoodEngine *vm);

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
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;

	void resetAnimationLayers();
	void rebuildWalkableMask();
	void copySmallRow(byte sourceRow, byte destinationRow);
	void updateTitleCaptionRows(byte selector);
	void promoteSecretDoorHotspots();
	void updateGlobeButtonDefaultStrips();
	void patchSecretDoorMovementModes();
	void updateSceneDepthThresholds(byte actorDrawOrderMode);
	void advanceFrontLayer(uint32 delta);
	void runEntryFromScene3050();
	void runEntryFromSecretPassage();
	void rotateGlobe(int delta);
	void markGlobeButtonsDiscovered();
	void recordGlobeButton(byte button);
	void resetGlobePuzzleHistory();
	bool matchesGlobePuzzle() const;
	void runGlobeButtonSequence(byte button, const byte *frameMap, uint frameMapSize, int globeDelta);
	void runRedButtonSequence();
	void runSecretDoorReveal();

	TimedAnimationChannel _frontChannel;
	ResourceSpriteLayer _frontLayer;
	ResourceSpriteLayer _globeLayer;
	ResourceSpriteLayer _buttonLayer;
	byte _frontLayerMode;
	bool _secretDoorRevealActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3060_H
