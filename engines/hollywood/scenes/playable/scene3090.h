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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3090_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3090_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene3090 : public PlayableScene {
public:
	Scene3090(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void rebuildWalkableMask();
	void removeColorMapItem(byte itemId);
	void replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem);
	void replaceActorPaletteClassFromOriginal(byte sourceClass, byte destinationClass);
	void copySmallTextRow(byte sourceRow, byte destinationRow);
	void advanceFrontLayer(uint32 delta);
	void advanceBlindManLayer(uint32 delta);
	void beginBlindManSpeechAnimation();
	void advanceBlindManSpeechAnimation(uint32 delta);
	void endBlindManSpeechAnimation();
	byte blindManSpeechBaseFrame() const;
	void setBlindManPostConversationFrame();
	void advancePuzzleLayer(uint32 delta);
	void triggerPuzzleLayer();
	void runExitToScene3080();
	void runBlindManConversation();
	void initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	void beginBlindManResponse(byte frameIndex);
	void runSaltShakerPickup();
	void runDowsingRodPickup();
	void runUseStrawInFireplace();
	void runSaxophoneHandoff();
	void drawForegroundBlocks(int activeWorldY);

	TimedAnimationChannel _frontChannel;
	TimedAnimationChannel _blindManChannel;
	TimedAnimationChannel _puzzleChannel;
	ResourceSpriteLayer _frontLayer;
	ResourceSpriteLayer _blindManLayer;
	ResourceSpriteLayer _puzzleLayer;
	bool _puzzleLayerTriggered;
	bool _dialogueMenuActive;
	bool _blindManConversationActive;
	bool _blindManSpeechActive;
	byte _blindManSpeechLastRandomFrame;
	uint32 _blindManSpeechTimerAccumulator;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3090_H
