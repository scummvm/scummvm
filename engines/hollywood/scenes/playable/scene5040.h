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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5040_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5040_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5040 : public PlayableScene {
public:
	Scene5040(HollywoodEngine *vm);

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
	AmbientAudioProfile ambientAudioProfile() const override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;

	void resetAnimationLayers();
	void advanceKarlLayer(uint32 delta);
	void runMineCartEntryClip();
	void runExitToMineSwitches();
	void runExitToMineHole();
	void runKarlConversation();
	void runMineBoxLook();
	void runPatchedSockPickup();
	void runMineKeyPickup();
	void runSpecialMineExitWithMagneticPillbox();
	void initializeKarlDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void setKarlDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const;
	bool applyKarlDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const;
	void beginKarlSpeechLine(byte frameIndex);
	void copyStageSmallRow(byte destinationRow, byte sourceRow);
	void remapSceneColors(byte sourceColor, byte itemId);
	void clearSceneItemFromColorMap(byte itemId);

	TimedAnimationChannel _karlIdleChannel;
	ResourceSpriteLayer _karlLayer;
	byte _karlIdleFrame;
	byte _karlIdleMode;
	byte _karlStrikeRepeatCount;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5040_H
