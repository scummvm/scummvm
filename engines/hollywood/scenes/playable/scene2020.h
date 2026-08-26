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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE2020_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE2020_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene2020 : public PlayableScene {
public:
	Scene2020(HollywoodEngine *vm);

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
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	byte primarySpeechAnimationFrameCount(byte animationGroup) const override;
	uint32 primarySpeechAnimationFrameMillis(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void advancePoolLayer(uint32 delta);
	void advanceTigerLayer(uint32 delta);
	void advanceTigerIdleFrame();
	void advanceTigerItemSequence(uint32 delta);
	void advancePrincessIdleLayer(uint32 delta, bool canStartLongSequence);
	bool advancePaletteCycle(uint32 delta);
	void rotatePoolPalette();
	void runEntryFromExterior();
	void runPrincessExitCutscene();
	bool runCurtainRevealFromBlack();
	void runCurtainClearToBlack();
	bool waitPrincessDepartureFrame(uint32 millis, byte clipFrame);
	void redrawPrincessDepartureFrame(byte clipFrame, bool poolChanged, bool tigerChanged);
	void runPrincessSpeechTransition(bool opening);
	void beginPrincessSpeechLine(byte frameIndex);
	void runPrincessDialogue();
	void initializePrincessDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const;
	void runHatPickup();
	void runSunglassesPickup();
	void runTigerToothPickup();
	void runSteakOnTigerSequence();
	void runLabItemOnTigerSequence();
	bool runTigerItemOverlaySequence(bool withEffect);
	void drawPickupPatch(byte hookId);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);
	void restoreOriginalColorMapItem(byte itemId);
	void remapOriginalColorMapItems(byte firstSourceItem, byte secondSourceItem, byte destinationItem);
	byte originalColorMapItemAt(uint paletteIndex) const;

	TimedAnimationChannel _poolChannel;
	TimedAnimationChannel _tigerChannel;
	TimedAnimationChannel _princessChannel;
	TimedAnimationChannel _paletteCycleChannel;
	TimedAnimationChannel _tigerItemActorChannel;
	TimedAnimationChannel _tigerItemEffectChannel;
	ResourceSpriteLayer _poolLayer;
	ResourceSpriteLayer _tigerLayer;
	ResourceSpriteLayer _princessLayer;
	ResourceSpriteLayer _tigerItemEffectLayer;
	byte _tigerAnimationState;
	byte _princessAnimationState;
	bool _princessSpeechTransitionActive;
	bool _princessLongIdleAllowed;
	bool _tigerItemSequenceActive;
	bool _tigerItemSequenceFinished;
	bool _tigerItemEffectEnabled;
	bool _tigerReactionStarted;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2020_H
