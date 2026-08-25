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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6010_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6010_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene6010 : public PlayableScene {
public:
	Scene6010(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	bool shouldUseActorDepthTest(int actorWorldX, int actorWorldY) const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;

	void updateSceneDepthThresholds(byte actorDrawOrderMode, int activeWorldX, int activeWorldY);
	void rebuildStudioWalkableMask();
	void rememberOriginalColorMap();
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void copyStepDeltasFromSet87(uint firstOffset, uint lastOffset);
	void runEntryFromTravel();
	void runEntryAndPendingPickup();
	void runEntryFromRight();
	void runEntryCutsceneState();
	void runStudioClipSequence(bool exitAfterPlayback);
	void runPendingItem69PickupOverlay();
	void runDoorRevealOverlay();
	void runExitToScene6020Overlay();
	void runPickupItem59Overlay();
	void runPickupItem58Overlay();
	void runLayeredOverlay(uint primaryChunkIndex, uint primaryDescriptorCount,
		const byte *primaryFrameMap, uint primaryFrameMapSize,
		uint secondaryChunkIndex, uint secondaryDescriptorCount,
		const byte *secondaryFrameMap, uint secondaryFrameMapSize,
		uint32 frameMillis, int soundFrame = -1, byte soundId = 0);

	Common::Array<byte> _originalColorToItemMap;
	TransientLayerCompositor _temporaryOverlayLayers;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6010_H
