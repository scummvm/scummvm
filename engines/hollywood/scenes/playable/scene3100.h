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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3100_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3100_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

// Saved GameplayState fields read:
// mainFlowStateId, scene3100CabinVisited, scene3100GirlConversationState,
// scene3100DaisyVisible, scene3100GirlDialogueRepeatCounter,
// scene3100SapSyringeTaken, scene3100DaisyTaken.
// Saved GameplayState fields written:
// mainFlowStateId, scene3100CabinVisited, scene3100GirlConversationState,
// scene3100DaisyVisible, scene3100GirlDialogueRepeatCounter,
// scene3100SapSyringeTaken, scene3100DaisyTaken.
class Scene3100 : public PlayableScene {
public:
	Scene3100(HollywoodEngine *vm);

private:
	bool hasCustomPreviewState() const override;
	void initializeCustomPreviewState() override;
	bool hasCustomComposite() const override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	bool hasCustomEntrySequence() const override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void rebuildWalkableMask();
	void removeColorMapItem(byte itemId);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void advanceCabinLayers(uint32 delta);
	void runExitToScene3080();
	void runCabinConversation();
	void initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	void beginCabinPrimaryResponse(byte frameIndex);
	void runConversationResolutionSequence();
	void runObjectPickup();
	void runExchangePickup();

	TimedAnimationChannel _cabinChannel;
	TimedAnimationChannel _alternateChannel;
	ResourceSpriteLayer _cabinLayer;
	ResourceSpriteLayer _alternateLayer;
	bool _dialogueMenuActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3100_H
