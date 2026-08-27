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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2080_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2080_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

/**
 * Gives the foreground character a separate dialogue-time animation path.
 * While the dialogue menu is open, its custom tick advances that pose, the
 * ambient layer and audio, and viewport scrolling without running the normal
 * scene tick.
 */
class Scene2080 : public PlayableScene {
public:
	Scene2080(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	void advanceDialogueMenu(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void advanceAmbientLayer(uint32 delta);
	void advanceForegroundActorIdle(uint32 delta);
	void advanceForegroundActorDialoguePose(uint32 delta);
	void drawCumulativeDeltaClip();
	bool playCumulativeDeltaFrames(byte firstFrame, byte lastFrame);
	void normalizeLinkedPassageState();
	void runEntryFromScene2070();
	void runEntryFromScene2090();
	bool runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel, bool fadeIn);
	void openForegroundActorForSpeech();
	void closeForegroundActorAfterSpeech();
	void beginForegroundActorEntrySpeechLine();
	bool waitForegroundDialogueMillis(uint32 millis);
	void beginForegroundDialogueSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void runForegroundActorPrimarySpeechLine(uint16 rowIndex, byte frameIndex);
	void runForegroundDialogue();
	void initializeForegroundDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const;
	void runForegroundActorExitOverlay();
	void runPostForegroundDialogueEffect();
	void runDepartureShake();
	void runForwardExitToScene2090();
	void runCentralSarcophagusHairSearch();
	void copySmallRow(uint sourceOffset, uint destinationOffset);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void setHotspotInteractionY(byte itemId, int16 y);
	void setHotspotInteractionX(byte itemId, int16 x);

	TimedAnimationChannel _ambientChannel;
	TimedAnimationChannel _foregroundActorChannel;
	ResourceSpriteLayer _ambientLayer;
	ResourceSpriteLayer _foregroundActorLayer;
	ResourceSpriteLayer _forwardExitPoseLayer;
	Common::Array<byte> _deltaClipData;
	byte _foregroundActorIdleState;
	byte _foregroundActorIdleDelay;
	byte _deltaClipMode;
	byte _deltaClipFrame;
	bool _foregroundActorManualSequenceActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2080_H
