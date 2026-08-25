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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE1050_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE1050_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene1050 : public PlayableScene {
public:
	Scene1050(HollywoodEngine *vm);

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
	bool shouldAnimatePrimarySpeechLine() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void runDoorBackToGorillaRoomAction();
	void runCloakroomAttendantConversation();
	void initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void beginCloakroomAttendantSpeechLine(byte frameIndex, bool alternatePose);
	void handleDialogueEffect(byte effectId);
	void runDialogueEffectTen();
	void runTravelUnlockEffect(byte travelSlotId);
	void handleJacketExchange();
	void handleJackTalkLine();
	void handleJackLookLine();
	void handleSuitcasePickup();
	void runLargeOverlayPoseTransition(byte mode, byte startFrame);
	void finishLargeOverlayIdleSequence();
	void runSynchronizedOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *actionFrameMap,
		const byte *largeOverlayFrameMap, uint frameMapSize, uint32 frameMillis);
	void runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame = -1);
	void advanceSmallOverlay(uint32 delta);
	void advanceLargeOverlay(uint32 delta);
	void advanceLargeOverlay(uint32 delta, bool forceFinish);
	void copyStageSmallRow(byte sourceRow, byte destinationRow);

	TimedAnimationChannel _smallOverlayChannel;
	TimedAnimationChannel _largeOverlayChannel;
	ResourceSpriteLayer _smallOverlayLayer;
	ResourceSpriteLayer _largeOverlayLayer;
	byte _largeOverlayMode;
	bool _largeOverlayActionLocked;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE1050_H
