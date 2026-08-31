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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5010_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5010_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5010 : public PlayableScene {
public:
	Scene5010(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void initializeSwitchLayer();
	byte switchDescriptorIndex() const;
	void copySlopeStepDeltasFromSet5A(uint targetFirstOffset);
	void runFirstEntrySequence();
	void runReturnEntrySequence();
	void runEntryPathWithFinalFacing(int startX, int startY, byte startFacing, int targetX, int targetY, byte finalFacing);
	void runMineCartArrival();
	void runReturnShake();
	void prepareMineTransport(bool showBlinkPatch);
	void enterMineTransport();
	void ensureMineDestinationTable();
	uint16 mineDestinationForCurrentSwitch() const;
	void runSwitchPanel();
	void drawSwitchPanelFrame();
	void drawSwitchPanelOverlay();
	byte switchPanelMaskPixelAt(uint16 screenX, uint16 screenY) const;
	void handleSwitchPanelChoice(byte choice);
	Common::Array<byte> buildSwitchPanelAnimation(byte currentValue, byte targetValue,
		uint &hideStaticFrame, uint &showStaticFrame) const;
	void activateSwitchPanelAtCursor(bool &done);
	bool pollSwitchPanelEvent(bool &done);

	bool _blinkPatchVisible;
	bool _switchPanelActive;
	byte _switchPanelMovingSelector;
	bool _switchPanelMovingSelectorVisible;
	byte _switchPanelDisplayedRow;
	byte _switchPanelDisplayedColumn;
	byte _switchPanelTargetValue;
	uint _switchPanelHideStaticFrame;
	uint _switchPanelShowStaticFrame;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5010_H
