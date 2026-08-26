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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE1020_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE1020_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene1020 : public PlayableScene {
public:
	Scene1020(HollywoodEngine *vm);

private:
	bool shouldLoadArenaChunk(uint index) const override;
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;

	void applyResourceBlockBackground();
	void applySceneColorMapRules(byte selector);
	void copyStageSmallRow(byte sourceRow, byte destinationRow);
	void runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame = -1);
	void runOverlaySequence(const ActionOverlaySpec &spec);
	void runQuasimodoGrateCutscene();
	void runGrateLiftShake();
	void runQuasimodoLayerTransition(byte fromFrame, byte toFrame);
	void runQuasimodoSpeechLine(byte frameIndex, bool bendToRope);
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void handleSceneEventFlag0();
	void handleSceneEventFlag0Overlay();
	void handleResourceBlockChoiceSpeech();
	void handleSceneVerb7Or8DescriptorAction();
	void handleSceneEventFlag1Speech();
	void handleSpeech19AfterEventFlag1();
	void handleResourceOverlayChunk18StateChange();
	void handleResourceOverlayChunk19EventFlag();
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	bool isFirstEntryState() const;
	bool isSpecialOverlayEntryState() const;

	ResourceSpriteLayer _quasimodoLayer;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE1020_H
