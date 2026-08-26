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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2100_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2100_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2100 : public PlayableScene {
public:
	Scene2100(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool shouldPresentPreviewBeforeEntrySequence() const override;
	bool shouldRunExitSideEffectsAfterLoop() const override;
	void runExitSideEffectsAfterLoop() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void advanceReturnLayers(uint32 delta);
	void advanceMummyIdle(uint32 delta);
	void advanceDoorCeremony(uint32 delta);
	void advanceSpecialSpeechAnimation(uint32 delta);
	void startManualActorPath(int targetX, int targetY, byte finalFacing, byte finalCel);
	void advanceManualActorPath(uint32 delta);
	void finishManualActorPath();
	bool isRaStaffAvailable() const;
	void runEntryFromScene2010();
	void runEntryFromScene2110();
	void runEntryFromLeftPassage();
	void runPrincessArrivalSequence();
	void runLateReturnSequence();
	void runExitToScene2010(int targetX, int targetY);
	void runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		byte startCel, int targetX, int targetY, byte finalFacing, byte finalCel);
	bool runMummyDialogue(bool playGreeting);
	void initializeMummyDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const;
	void runMummyPrimarySpeechLine(byte frameIndex);
	void runPrincessArrivalPrimarySpeechLine(byte frameIndex);
	void runMummyFrameSetTransition(ResourceSpriteLayer &layer, bool opening, byte closingFinalFrame);
	void beginMummyDialogueSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void runTreasureIntroductionSequence();
	void runSpecialTransitionSpeech();
	void runStoneDoorToTreasureRoom();
	void runRaStaffPickup();
	void removeColorMapItemFromOriginal(byte itemId);

	TimedAnimationChannel _foregroundChannel;
	TimedAnimationChannel _auxChannel;
	TimedAnimationChannel _manualActorPathChannel;
	ResourceSpriteLayer _mummyLayer;
	ResourceSpriteLayer _frontLayer;
	ResourceSpriteLayer _auxLayer;
	bool _foregroundAlternateFrameSet;
	bool _mummySpeechUsesFrontLayer;
	bool _suppressMummySpeechAnimation;
	bool _auxLayerInFront;
	bool _returnLayerAnimationActive;
	bool _mummyIdleEnabled;
	bool _doorCeremonyAnimationActive;
	bool _doorCeremonyFinishing;
	byte _doorCeremonyState;
	bool _specialSpeechAnimationActive;
	bool _specialSpeechFinishing;
	byte _specialSpeechVariant;
	bool _manualActorPathActive;
	uint _manualActorPathFrameIndex;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2100_H
