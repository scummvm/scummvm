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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2050_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2050_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2050 : public PlayableScene {
public:
	Scene2050(HollywoodEngine *vm);

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
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void resetAmbientLayer();
	void advanceAmbientLayer(uint32 delta);
	void runEntryFromSphinxInterior();
	void runEntryFromLabyrinthReturn();
	void runGoToLabyrinth();
	void runLongLabyrinthWalkClip();
	void runRevealFloorSeal();
	void runMuralSubscreenAction();
	void runGuidedMuralAction();
	void runMuralClipForward();
	void runMuralClipBackward();
	void runMuralPuzzleSubscreen();
	void runSealDiscoveryIfPending();
	void runSealDiscoverySequence();
	void drawSealDiscoveryDeltaLayer();
	void drawRawSceneChunk(uint chunkIndex);
	bool loadMuralHitMask();
	bool initializeMuralTilePermutation(uint chunkIndex);
	bool isSavedMuralTilePermutationValid() const;
	void randomizeMuralTilePermutation();
	void setMuralTilePermutationSolved();
	void drawMuralTileGrid(uint chunkIndex);
	void drawMuralSelectionHighlight(byte tileId);
	void clearMuralSelectionHighlight(byte tileId);
	void rotateMuralHighlightPalette();
	byte muralTileAtScreenPoint(int16 x, int16 y) const;
	void handleMuralTileClick(byte tileId, bool &done);
	bool isMuralPuzzleSolved() const;
	uint solvedMuralTileCount() const;
	void copyMuralTile(byte visibleTileId, byte sourceTileId, const byte *rawFrame, byte *destination);
	void copyStageSmallRow(byte destinationRow, byte sourceRow);

	TimedAnimationChannel _ambientChannel;
	ResourceSpriteLayer _ambientLayer;
	bool _muralPermutationInitialized;
	byte _muralPermutationChunkIndex;
	byte _muralSelectedTile;
	Common::Array<byte> _muralHitMask;
	uint32 _muralPaletteAccumulator;
	bool _sealDiscoveryActive;
	byte _sealDiscoveryFrame;
	uint _sealDiscoveryActorPathFrameIndex;
	bool _sealDiscoverySpeechStarted;
	uint32 _sealDiscoverySpeechStartMillis;
	uint32 _sealDiscoverySpeechDurationMillis;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2050_H
