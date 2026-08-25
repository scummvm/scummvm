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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE8010_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE8010_H

#include "hollywood/scenes/playable/playable_scene.h"

#include "common/str.h"

namespace Hollywood {

class HollywoodEngine;

class Scene8010 : public PlayableScene {
public:
	Scene8010(HollywoodEngine *vm);

private:
	struct FishermanQuizEntry {
		byte promptStage;
		byte promptRow;
		byte promptSuffixStage;
		byte promptSuffixRow;
		byte menuStage;
		byte menuRow;
		byte menuSuffixStage;
		byte menuSuffixRow;
	};

	void initializeCustomPreviewState() override;
	void drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) override;
	void runCustomEntrySequence() override;
	bool prepareCustomGameplayLoop() override;
	bool advanceCustomGameplayLoop(uint32 delta) override;
	bool dispatchCustomSceneAction(uint16 handlerId) override;
	Common::String dialogueMenuText(byte stageId, byte textRowId) const override;
	bool applyCustomSceneStateToHotspotsAndPatches(byte selector) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;
	void primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) override;
	AmbientAudioProfile ambientAudioProfile() const override;

	void resetSceneAnimations();
	void advanceFishermanIdle(uint32 delta);
	void advanceBoatLoop(uint32 delta);
	void drawTransitionClip(uint chunkIndex);
	void runFirstEntry();
	void runReturnEntryTransition();
	void runFishermanConversation();
	void runExitToScene8020();
	void runTravelScreenAction();
	void applyFishermanNameTextPatch();
	void setActiveActorPose(int x, int y, byte facing);
	void initializeFishermanQuizData(byte targetLineIndex);
	Common::String composeFishermanQuizChoice(byte firstStage, byte firstRow, byte secondStage, byte secondRow) const;
	void composeFishermanGeneratedPromptLines(byte promptLineIndex, Common::Array<Common::String> &lines) const;
	uint16 fishermanQuizFragmentVoiceSampleId(const FishermanQuizEntry &entry, byte fragmentIndex) const;
	bool waitFishermanQuizFragmentVoices(const FishermanQuizEntry &entry, byte firstFragment, uint32 fallbackMillis);
	bool runFishermanGeneratedPrimarySpeechLine(byte promptLineIndex);
	void runFishermanGeneratedPromptSpeech();
	void runFishermanSelectedAnswerSpeech(byte selectedLine);
	byte chooseFishermanQuizLine();
	void updateFishermanQuizMenuHover(DialogueMenuState &state);
	byte fishermanQuizMenuLineAt(uint16 cursorY, const DialogueMenuState &state) const;
	byte randomUnusedRow(byte firstRow, byte rowCount, bool *usedRows, uint usedRowCount);
	bool runFishermanQuiz();

	ResourceSpriteLayer _fishermanLayer;
	ResourceSpriteLayer _boatLayer;
	TimedAnimationChannel _fishermanChannel;
	TimedAnimationChannel _boatChannel;
	FishermanQuizEntry _fishermanQuizEntries[8];
	FishermanQuizEntry _fishermanQuizFinalEntry;
	Common::String _fishermanQuizChoiceText[8];
	byte _fishermanQuizChoiceEntryIndex[8];
	byte _fishermanState;
	byte _fishermanRepeatCount;
	bool _fishermanQuizAlternatePattern;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE8010_H
