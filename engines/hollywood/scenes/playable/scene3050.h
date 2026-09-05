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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3050_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3050_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene3050 : public PlayableScene {
public:
	Scene3050(HollywoodEngine *vm);

private:
	void initializeCustomPreviewState() override;
	void drawCustomForegroundComposite(int activeWorldX, int activeWorldY) override;
	void runCustomEntrySequence() override;
	void prepareCustomGameplayLoop() override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void advancePrimarySpeechAnimation(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetAnimationLayers();
	void rebuildWalkableMask();
	void copyCaptionRow(byte sourceRow, byte destinationRow);
	void setForegroundActorFrame(byte frameIndex);
	void advanceForegroundActorLayer(uint32 delta);
	void advanceDialogueActorLayer(uint32 delta);
	bool canStartForegroundActorIdleSpeech() const;
	void startForegroundActorIdleSpeech(byte frameIndex);
	void finishForegroundActorIdleSpeech();
	void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte playerTextRowId, byte responseFrameIndex, byte transitionMode, byte disableAfterUse) const;
	void drawForegroundBlocks();
	void runEntryFromScene3010();
	void runEntryFromScene3060();
	void runExitToScene3010();
	void runDialogueAction(bool playOpeningLine);
	void runForegroundActorDialoguePose();
	void runDialogueMenuRow98();
	void initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const;
	void beginDialogueResponse(byte frameIndex);

	TimedAnimationChannel _foregroundActorChannel;
	TimedAnimationChannel _dialogueActorChannel;
	uint _backgroundTrack;
	byte _foregroundActorMode;
	byte _foregroundActorIdleCounter;
	byte _foregroundActorIdleSpeechFrame;
	bool _foregroundActorManualSequenceActive;
	bool _dialogueMenuActive;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3050_H
