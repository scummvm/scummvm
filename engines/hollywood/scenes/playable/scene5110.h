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
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	bool shouldConvertSavedFramebufferFF() const override;
	AmbientAudioProfile ambientAudioProfile() const override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void runEntryElevatorSequence();
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
	void initializeWerewolfDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setWerewolfDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	bool applyWerewolfDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex);
	void drawStaticRoomLayers(int activeWorldY);
	void drawSpriteFrame(uint chunkIndex, uint descriptorCount, byte frameIndex);
	void clearSceneItemFromColorMap(byte itemId);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void rebuildWalkableMask();
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5110_H
