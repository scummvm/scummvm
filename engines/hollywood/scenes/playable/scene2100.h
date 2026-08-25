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
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void setForegroundFrame(byte frameIndex);
	void runEntryFromScene2010();
	void runEntryFromScene2110();
	void runEntryFromLeftPassage();
	void runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel);
	void runMummyDialogue();
	void initializeMummyDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const;
	void runMummyPrimarySpeechLine(byte frameIndex);
	void beginMummyDialogueSecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void runMummySpecialTransitionToScene2110();
	void runStoneDoorToTreasureRoom();
	void runRaStaffPickup();
	void runMappedLayerAnimation(ResourceSpriteLayer &layer, byte targetFrame, uint32 frameMillis,
		int speechTriggerFrame = -1, uint16 speechRow = 0, byte speechFrame = 0,
		int soundStartFrame = -1, byte soundStartId = 0, int soundStopFrame = -1);
	void removeColorMapItemFromOriginal(byte itemId);

	ResourceSpriteLayer _foregroundLayer;
	ResourceSpriteLayer _transitionLayer;
	bool _foregroundAlternateFrameSet;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE2100_H
