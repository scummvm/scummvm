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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7010_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7010_H

#include "hollywood/gameplay/dialogue_menu.h"
#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

/**
 * Uses separate transient-layer stacks behind and in front of the actors.
 *
 * Its custom tick advances primary speech, ambient audio, idle animation, and
 * the dialogue overlay. The base still advances secondary speech, viewport
 * scrolling, and actor-pose persistence.
 */
class Scene7010 : public PlayableScene {
public:
	Scene7010(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldDrawSecondaryActorInPlayableComposite() const override;
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
	bool shouldAnimatePrimarySpeechLine() const override;
	void setPrimaryLeftSpeechFrame(byte frameIndex) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	void advanceFullscreenAnimation(uint32 delta) override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void runSueEntryPath(int startX, int startY, int targetX, int targetY);
	void runJuniorSpeech();
	void advanceChunk8Cycle();
	void advanceChunk10IdleFrames();
	void advanceDoghouseSpeechFrame(uint32 delta);
	void advanceDialogueOverlay(uint32 delta);
	void handleActionSlot00TransitionToG03();
	void handleActionSlot01SecondarySpeech();
	void handleActionSlot02SecondarySpeech();
	void handleActionSlot03DialogueSequence();
	void handleActionSlot04Item06Speech();
	void handleActionSlot06DoghouseSequence();
	void handleActionSlot07DialogueAndReturn();
	void handleActionSlot08CommonSpeech();
	void runHannoverDialogueMenuRow98();
	void initializeHannoverDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setHannoverDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	bool applyHannoverDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const;
	void beginHannoverPrimarySpeechLine(byte frameIndex, byte poseVariant);
	void advanceHannoverPrimarySpeechFrame(uint32 delta);
	void runChunk8RevealSequence();
	void runChunk8HideSequence();
	bool runChunk11FrameRange(byte startFrame, byte endFrame);
	void runChunk11MidFrames();
	void runChunk11ReturnFrames();
	void runChunk11ExtendedFrames();
	void runChunk13Item09PickupOverlaySequence();
	void runChunk14FrameRange(byte startFrame, byte endFrame, bool restoreChunk11AtEnd);
	void runDoghouseDepartureSequence();
	void runDoghouseSpeechLine(byte frameIndex);
	void runDogCloseupSequence();
	void runDialogueOverlayFrames(byte startFrame, byte endFrame, byte finalMode);
	void updateG01AmbientAudioAndMusicCues(uint32 delta);
	void configureAnimationLayers();
	void setDialogueOverlayMode(byte mode, byte frameIndex);
	void setDialogueOverlayFrame(byte frameIndex);
	void setChunk8Frame(byte frameIndex);
	byte chunk8Frame() const;
	void setChunk9Frame(byte frameIndex);
	void updateChunk9Visibility();
	void setChunk11Visible(bool visible);
	void setChunk11Frame(byte frameIndex);
	void setChunk14Visible(bool visible);
	void setChunk14Frame(byte frameIndex);

	byte _dialogueOverlayMode;
	byte _chunk11RightSpeechPoseVariant;
	bool _chunk8SpecialSequenceActive;
	bool _chunk11RightSpeechActive;
	bool _doghouseSpeechActive;
	AlternatingRandomFramePair _chunk10IdlePairA;
	AlternatingRandomFramePair _chunk10IdlePairB;
	TimedAnimationChannel _chunk8Animation;
	TimedAnimationChannel _chunk10Animation;
	TimedAnimationChannel _dialogueOverlayAnimation;
	RandomFrameAnimation _hannoverSpeechAnimation;
	RandomFrameAnimation _doghouseSpeechAnimation;
	SceneAnimationLayers _animationLayers;
};

} // End of namespace Hollywood

#endif
