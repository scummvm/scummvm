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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2060_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2060_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2060 : public PlayableScene {
public:
	Scene2060(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldApplyGameplayPanelObjectPalette() const override;
	bool isInventoryPanelAvailable() const override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	byte paletteRegionAt(int x, int y) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void installSceneActorBank();
	void copyPassageTextRows();
	void remapWallPresentationPalette();
	byte remapScenePaletteColor(byte color, uint steps) const;
	void updateSceneDepthThresholds(byte actorDrawOrderMode, int actorWorldX, int actorWorldY);
	void restoreActorLightBackgroundRect(int actorWorldX, int actorWorldY);
	void drawActorLightLayer(int actorWorldX, int actorWorldY);
	bool prepareGuideEffectForCurrentMazePosition();
	void restoreGuideBackgroundRect();
	void drawGuideLayer();
	bool guideCenterForFrame(byte direction, byte frameIndex, int &centerX, int &centerY) const;
	void rebuildWalkableMask();
	void runEntryPathAndGuide(int startX, int startY, byte startFacing, byte initialDrawOrder,
		int targetX, int targetY);
	void moveThroughPassage(int delta, uint16 nextState);
	void transitionToCurrentMazeState();

	bool _sceneActorBankInstalled;
	bool _guideEffectPrepared;
	bool _guideEffectActive;
	byte _guideDirection;
	byte _guideFrameIndex;
	byte _guideFrameCount;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2060_H
