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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5110_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5110_H

#include "hollywood/gameplay/dialogue_menu.h"
#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5110 : public PlayableScene {
public:
	Scene5110(HollywoodEngine *vm);

private:
	int replacementFillRunsResourceChunkIndex() const override;
	int replacementPaletteMaskResourceChunkIndex() const override;
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	void runCustomEntrySequence() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldConvertSavedFramebufferFF() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	AmbientAudioProfile ambientAudioProfile() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	byte primarySpeechAnimationFrameCount(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	void handleLeftClick(const GameplayLoopCursorState &state) override;

	bool runEntryElevatorSequence(bool alternateEntry);
	bool playElevatorSequence(const byte *frameMap, uint frameMapSize, bool drawActor);
	bool holdElevatorFrame(bool drawActor);
	void runButtonExitToState(uint16 nextState);
	void runButtonReturnSequence();
	void runUnderwearPickup();
	void runBottlePickup();
	void runWerewolfDialogue();
	void runManicureGirlTalk();
	void runJacuzziGirlTalk();
	void runMirrorApproach();
	void runMirrorPickup();
	void runJacuzziInspection();
	void runHairTreatmentTalk();
	void beginSalonPrimarySpeechLine(uint16 rowIndex, byte frameIndex);
	void settleWerewolfForSpeech();
	void initializeWerewolfDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setWerewolfDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	bool applyWerewolfDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex);
	void initializeSalonLayers();
	void advanceSalonAnimations(uint32 delta);
	void advanceSalonAnimationTick();
	void advanceUpperRightSalonTick();
	void advanceRightSalonDetailTick();
	void advanceLowerSalonDetailTick();
	void advanceCenterSalonTick();
	void advanceLeftSalonTick();
	void advanceWerewolfTick(bool allowLongGesture);
	void advanceWerewolfDialogueTick();
	void advanceCenterSalonDetailTick();
	void drawNormalRoomLayers(int activeWorldY);
	void drawSalonSpriteLayers();
	void drawElevatorComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY);
	void updateElevatorButtonActionTargets(bool useStrip);
	void initializeSceneItemLabels();
	void copyStageSmallRow(byte sourceRow, byte destinationRow);
	void clearSceneItemFromColorMap(byte itemId);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void rebuildWalkableMask();

	TimedAnimationChannel _salonAnimationChannel;
	byte _upperRightSalonState;
	byte _rightSalonDetailDirection;
	byte _rightSalonDetailDwell;
	byte _lowerSalonDetailState;
	byte _leftSalonRepeatCount;
	byte _werewolfState;
	byte _centerSalonDetailRepeatCount;
	bool _centerSalonToggleA;
	bool _centerSalonToggleB;
	bool _centerSalonDetailSequenceActive;
	bool _werewolfDialogueActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5110_H
