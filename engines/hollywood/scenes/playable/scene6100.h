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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE6100_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE6100_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

// Runs the museum's upper floor, Charlie dialogue, and Billy Ford letter exchange.
class Scene6100 : public PlayableScene {
public:
	Scene6100(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion,
		const ActorPathBuildState &state, const ScenePoint &boundary,
		int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion,
		const ActorPathBuildState &state, int targetX, int targetY,
		int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetSceneLayers();
	void advanceCharlieIdle(uint32 delta);
	void advanceCharlieConversationIdle(uint32 delta);
	void advanceLetterReadingSpeech(uint32 delta);
	void enterCharlieDialoguePose();
	void switchCharlieToAlternatePose();
	void returnCharlieToDialoguePose();
	void finishCharlieDialoguePose();
	void runCharlieDialogue();
	void initializeCharlieDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setCharlieDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	void beginCharlieSpeechLine(byte frameIndex, byte animationGroup);
	void takeCharlieBriefcase();
	void giveBillyFordEnvelopeToCharlie();

	ResourceSpriteLayer _charlieLayer;
	ResourceSpriteLayer _letterLayer;
	ResourceSpriteLayer _departureLayer;
	TimedAnimationChannel _charlieIdleChannel;
	TimedAnimationChannel _charlieConversationChannel;
	byte _charlieIdleState;
	byte _charliePose;
	bool _charlieManualSequenceActive;
	bool _charlieConversationActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE6100_H
