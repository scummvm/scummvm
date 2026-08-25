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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4110_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4110_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4110 : public PlayableScene {
public:
	Scene4110(HollywoodEngine *vm);

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

	bool bridgeOpened() const;
	void resetBackgroundLayer();
	void advanceBackgroundLayer(uint32 delta);
	void advanceBackgroundTick();
	void updateAmbientLoopSound();
	void beginConditionalSpeechLine(uint16 falseRow, byte falseFrame, uint16 trueRow, byte trueFrame);
	void runExitToScene4010();
	void takeLetter();
	void runAlternateStateSequence();
	void runBridgeOpeningOverlay();
	void patchActionMovementModes();

	TransientLayerCompositor _backgroundLayers;
	TransientLayerCompositor _bridgeBackLayers;
	TransientLayerCompositor _bridgeFrontLayers;
	TimedAnimationChannel _backgroundChannel;
	byte _backgroundSequence;
	byte _backgroundFrameInSequence;
	byte _backgroundRepeatCounter;
	bool _bridgeSequenceActive;
	SoundBank0Player _ambientLoopSound;
	byte _lastAmbientLoopCue;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4110_H
