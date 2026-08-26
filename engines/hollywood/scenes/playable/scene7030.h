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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7030_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7030_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene7030 : public PlayableScene {
public:
	Scene7030(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldDrawSecondaryActorInPlayableComposite() const override;
	void runCustomEntrySequence() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	void setPrimaryLeftSpeechFrame(byte frameIndex) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	void updateAmbientAudioAndMusicCues(uint32 delta);
	void advanceChunk5AmbientOverlay();
	void advanceChunk6IdleFrames();
	void configureAnimationLayers();
	void setChunk5Frame(byte frameIndex);
	byte chunk5Frame() const;
	void runPunchBowlPatchOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int statePatchFrame = -1);
	void handleActionSlot00TransitionToG04();
	void handleActionSlot01SecondarySpeech();
	void handleActionSlot02TransitionToG01Alt();
	void handleActionSlot03SecondarySpeech();
	void handleActionSlot04SecondarySpeech();
	void handleActionSlot05ToggleSceneState0Speech();
	void handleActionSlot08CommonSpeech();
	void handleActionSlot09CommonSpeech();
	void handleActionSlot10CommonSpeech();
	void handleActionHandler313ExchangeItem0CFor0D();
	void handleActionHandler314PickupBone();
	void handleActionHandler315PickupItem0C();
	void handleActionHandler316SecondarySpeech();

	byte _chunk9AmbientDecisionCounter;
	int _chunk5FrameDirection;
	TimedAnimationChannel _chunk5Animation;
	TimedAnimationChannel _chunk6Animation;
	AlternatingRandomFramePair _chunk6IdlePairA;
	AlternatingRandomFramePair _chunk6IdlePairB;
	SceneAnimationLayers _animationLayers;
};

} // End of namespace Hollywood

#endif
