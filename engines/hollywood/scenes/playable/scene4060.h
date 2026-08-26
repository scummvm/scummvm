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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE4060_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE4060_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene4060 : public PlayableScene {
public:
	Scene4060(HollywoodEngine *vm);

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
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	void handleAnimationFrameHook(byte hookId, uint frame) override;

	void resetForegroundLayer();
	void configureForegroundLayerForState();
	void setForegroundScrollStep(byte step);
	void advanceForegroundLayer(uint32 delta);
	void drawForegroundTableLayer();
	void drawSceneForegroundBlocks(int activeWorldY);
	void runFirstEntrySequence();
	void runReturnEntrySequence();
	void runExitToNextRoom();
	void runFirstCardStage();
	void runSecondCardStage();
	void runInstallMirrorStage();
	void runSherilynCardDialogue();
	void initializeSherilynCardDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void runSherilynDialogueProgressReplay();
	void runSherilynDialogueTransition();
	void runSherilynPokerTransitionAnimation(bool finalRewardBranch);
	bool presentPokerTransitionFrame(byte tableFrame, uint overlayChunk, uint overlayDescriptorCount, byte overlayFrame);
	void clearPokerTransitionLayers();
	void beginSherilynSpeechLine(uint16 rowIndex, byte frameIndex, bool allowAlternatePose = true);
	void beginTeddyBearSpeechLine(uint16 rowIndex, byte frameIndex);
	void openSherilynSpeechPose(bool allowAlternatePose);
	void closeSherilynSpeechPose();
	byte sherilynSpeechBaseStep() const;
	void applyCardPatchStateColorMap(byte cardState);
	void replaceColorMapItem(byte sourceItem, byte destinationItem);
	void copySmallTextRow(byte destinationRow, byte sourceRow);

	ResourceSpriteLayer _foregroundLayer;
	TransientLayerCompositor _pokerTransitionLayers;
	TimedAnimationChannel _foregroundChannel;
	byte _foregroundScrollStep;
	bool _foregroundLongAnimationActive;
	byte _sherilynSpeechPoseMode;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE4060_H
