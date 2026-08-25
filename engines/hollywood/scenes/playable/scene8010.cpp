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

#include "hollywood/scenes/playable/scene8010.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/cursor.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene8010FirstState = 0x1f4a;
const uint16 kScene8010ReturnEntryState = 0x1f4b;
const uint16 kScene8020State = 0x1f54;
const uint kScene8010ActorBankTableEntry = 0x0000;
const uint kScene8010ActorPaletteTableEntry = 0x00cc;
const uint kScene8010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene8010SpeechCueDescriptorTableOffset = 0x1135;
const int kScene8010EntryStartX = 0x3b0;
const int kScene8010EntryStartY = 0x17f;
const int kScene8010EntryTargetX = 0x2f0;
const int kScene8010EntryTargetY = 0x17d;
const byte kScene8010EntryFacing = 4;
const uint32 kScene8010FishermanFrameMillis = 75;
const uint32 kScene8010BoatFrameMillis = 60;
const uint32 kScene8010TransitionFrameMillis = 40;
const uint kScene8010FishermanDescriptorCount = 0x18;
const uint kScene8010BoatDescriptorCount = 8;
const uint kScene8010TransitionDescriptorCount = 0x87;
const byte kScene8010PrimarySpeechBaseFrame = 0x17;
const byte kScene8010PrimarySpeechRestFrame = 0x17;
const byte kScene8010PrimarySpeechIdleBlinkFrame = 0x1b;
const uint16 kScene8010FishermanSpeechCenterX = 0x00fa;
const uint16 kScene8010FishermanSpeechTopY = 0x0104;
const byte kScene8010FishermanSpeechRed = 0x3f;
const byte kScene8010FishermanSpeechGreen = 0x0d;
const byte kScene8010FishermanSpeechBlue = 0x0d;
const byte kScene8010InvalidPrimarySpeechGroup = 0xff;
const byte kScene8010DefaultPrimarySpeechFrame = 7;
const uint16 kScene8010FishermanPlayerSpeechRow = 0x62;
const uint16 kScene8010FishermanResponseSpeechRow = 0x63;
const byte kScene8010FishermanQuizMenuStage = 0xfe;
const uint32 kScene8010QuizMenuTickMillis = 10;
const uint32 kScene8010QuizMenuLineHeight = 0x15;
const uint16 kScene8010QuizMenuHoverTopBaseY = 0x1d3;
const uint16 kScene8010QuizMenuHoverBottomY = 0x1d5;
const uint32 kScene8010GeneratedSpeechMinMillis = 1200;
const uint32 kScene8010GeneratedSpeechLineMillis = 1100;
const uint32 kScene8010SelectedAnswerMinMillis = 900;
const uint32 kScene8010SelectedAnswerLineMillis = 900;

const byte kScene8010FishermanFrameMap[] = {
	0, 23, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 10, 9, 8,
	0, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22
};

const byte kScene8010BoatFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7
};

const byte kScene8010FishermanQuizLinePermutation[] = {
	0, 4, 7, 1, 3, 2, 5, 6,
	6, 1, 0, 7, 2, 4, 3, 5,
	3, 5, 2, 6, 0, 7, 1, 4,
	5, 7, 1, 3, 6, 0, 4, 2,
	2, 3, 0, 6, 4, 1, 7, 5,
	4, 2, 7, 0, 6, 5, 1, 3,
	5, 3, 4, 2, 7, 0, 6, 1,
	6, 5, 1, 4, 3, 2, 0, 7
};

static void appendFishermanPromptFragment(Common::String &text, const Common::String &fragment) {
	if (fragment.empty())
		return;
	if (!text.empty())
		text += " ";
	text += fragment;
}

PlayableSceneConfig scene8010Config() {
	PlayableSceneConfig config(8010,
		SceneResourceLayout(5, 5, 9),
		SceneViewport(0, 0, 0x00d0),
		SceneActorPose(kScene8010EntryTargetX, kScene8010EntryTargetY, kScene8010EntryFacing));
	config.setActorResources(kScene8010ActorBankTableEntry, kScene8010ActorPaletteTableEntry);
	config.setTextResources(kScene8010Resource003RowsOffsetIndex, kScene8010SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet00);
	config.loadActorDepthTables = false;
	return config;
}

Scene8010::Scene8010(HollywoodEngine *vm) :
		PlayableScene(vm, scene8010Config()),
		_fishermanLayer(),
		_boatLayer(),
		_fishermanChannel(),
		_boatChannel(),
		_fishermanState(0),
		_fishermanRepeatCount(0),
		_fishermanQuizAlternatePattern(_random.getRandomBit() != 0) {
}

void Scene8010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetSceneAnimations();
	setActiveActorPose(kScene8010EntryTargetX, kScene8010EntryTargetY, kScene8010EntryFacing);
}

void Scene8010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_fishermanLayer);
	drawResourceSpriteLayer(_boatLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	drawActionOverlayLayer();
}

void Scene8010::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId != kScene8010FirstState && stateId != kScene8010ReturnEntryState) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	if (stateId == kScene8010ReturnEntryState) {
		runReturnEntryTransition();
		return;
	}

	runFirstEntry();
}

bool Scene8010::prepareCustomGameplayLoop() {
	return true;
}

bool Scene8010::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceFishermanIdle(delta);
	advanceBoatLoop(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene8010::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Hablar con pescador/Graeme McDundee (talk to fisherman/Graeme): clan quiz conversation.
		runFishermanConversation();
		return true;
	case 302: // Mirar pescador (look at fisherman): stress/Graeme introduction response.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar barca (look at boat): small boat.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar barca (use boat): boat cannot hold Ron's weight.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar ruinas (look at ruins): old abbey.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 306: // Ir a alto del acantilado (go to cliff top): enter scene 8020.
		runExitToScene8020();
		return true;
	case 307: // Mirar alto del acantilado (look at cliff top): good lake viewpoint.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar lago (look at lake): monster is not visible.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Ir a ruinas (go to ruins): nothing interesting there.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 310: // Ir a lago (go to lake): Nessie danger refusal.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Ir a hidroavión (go to seaplane): return to destination selector.
		runTravelScreenAction();
		return true;
	default:
		return false;
	}
}

Common::String Scene8010::dialogueMenuText(byte stageId, byte textRowId) const {
	if (stageId == kScene8010FishermanQuizMenuStage && textRowId < ARRAYSIZE(_fishermanQuizChoiceText))
		return _fishermanQuizChoiceText[textRowId];

	return PlayableScene::dialogueMenuText(stageId, textRowId);
}

bool Scene8010::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	bool handled = false;

	if (selector == 0 || selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		handled = true;
	}

	if (selector == 3 || selector == 0xff) {
		restoreBaseFramebufferFromOriginal();
		handled = true;
	}

	if (selector == 1 || selector == 0xff) {
		applyFishermanNameTextPatch();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		handled = true;
	}

	return handled;
}

byte Scene8010::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return kScene8010PrimarySpeechBaseFrame;
}

void Scene8010::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_fishermanState = 0;
	_fishermanRepeatCount = 0;
	_fishermanLayer.setFrame(frameIndex);
}

void Scene8010::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	_fishermanState = 0;
	_fishermanRepeatCount = 0;
	_fishermanLayer.setFrame(baseFrame);
}

AmbientAudioProfile Scene8010::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x12, 6, 100, 1, 0x0c, 1, 100, 50);
}

void Scene8010::resetSceneAnimations() {
	_fishermanLayer.configure(5, kScene8010FishermanDescriptorCount,
		kScene8010FishermanFrameMap, ARRAYSIZE(kScene8010FishermanFrameMap));
	_boatLayer.configure(6, kScene8010BoatDescriptorCount,
		kScene8010BoatFrameMap, ARRAYSIZE(kScene8010BoatFrameMap));
	_fishermanLayer.visible = true;
	_boatLayer.visible = true;
	_fishermanLayer.reset(0);
	_boatLayer.reset(0);
	_fishermanChannel.reset(0, kScene8010FishermanFrameMillis);
	_boatChannel.reset(0, kScene8010BoatFrameMillis);
	_fishermanState = 0;
	_fishermanRepeatCount = 0;
}

void Scene8010::advanceFishermanIdle(uint32 delta) {
	const uint frameCount = _fishermanChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_fishermanLayer.frameIndex >= kScene8010PrimarySpeechRestFrame) {
			_fishermanState = 0;
			_fishermanRepeatCount = 0;
			if (_fishermanLayer.frameIndex == kScene8010PrimarySpeechIdleBlinkFrame) {
				_fishermanLayer.setFrame(kScene8010PrimarySpeechRestFrame);
			} else if (_random.getRandomNumber(14) == 0) {
				_fishermanLayer.setFrame(kScene8010PrimarySpeechIdleBlinkFrame);
			}
			continue;
		}

		if (_fishermanState == 0) {
			if (_fishermanLayer.frameIndex == 1) {
				_fishermanLayer.setFrame(0);
				continue;
			}
			if (_random.getRandomNumber(14) == 0) {
				_fishermanLayer.setFrame(1);
				continue;
			}
			if (_random.getRandomNumber(49) == 0) {
				_fishermanLayer.setFrame(9);
				_fishermanState = 3;
				continue;
			}
			if (_random.getRandomNumber(49) == 0) {
				_fishermanLayer.setFrame(2);
				_fishermanState = 1;
				_fishermanRepeatCount = (byte)_random.getRandomNumber(5);
				_soundBank0.playSample(0x0b, 100, true);
				continue;
			}
			if (_random.getRandomNumber(49) == 0) {
				_fishermanLayer.setFrame(8);
				_fishermanState = 2;
				_fishermanRepeatCount = (byte)_random.getRandomNumber(5);
				_soundBank0.playSample(0x0b, 100, true);
				continue;
			}
			continue;
		}

		if (_fishermanState == 1) {
			if (_fishermanLayer.frameIndex == 8) {
				if (_fishermanRepeatCount == 0) {
					_fishermanLayer.setFrame(0);
					_fishermanState = 0;
					_soundBank0.stop();
				} else {
					_fishermanLayer.setFrame(2);
					--_fishermanRepeatCount;
				}
			} else {
				_fishermanLayer.setFrame(_fishermanLayer.frameIndex + 1);
			}
			continue;
		}

		if (_fishermanState == 2) {
			if (_fishermanLayer.frameIndex == 2) {
				if (_fishermanRepeatCount == 0) {
					_fishermanLayer.setFrame(0);
					_fishermanState = 0;
					_soundBank0.stop();
				} else {
					_fishermanLayer.setFrame(8);
					--_fishermanRepeatCount;
				}
			} else {
				_fishermanLayer.setFrame(_fishermanLayer.frameIndex - 1);
			}
			continue;
		}

		if (_fishermanLayer.frameIndex == 0x0f) {
			_fishermanLayer.setFrame(0);
			_fishermanState = 0;
		} else {
			_fishermanLayer.setFrame(_fishermanLayer.frameIndex + 1);
		}
	}
}

void Scene8010::advanceBoatLoop(uint32 delta) {
	const uint frameCount = _boatChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte nextFrame = _boatLayer.frameIndex == 7 ? 0 : (byte)(_boatLayer.frameIndex + 1);
		_boatLayer.setFrame(nextFrame);
	}
}

void Scene8010::drawTransitionClip(uint chunkIndex) {
	drawPlayableComposite();
	presentFrame();

	for (byte frame = 0; frame < kScene8010TransitionDescriptorCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		drawClipFrameDelta(chunkIndex, kScene8010TransitionDescriptorCount, frame);
		presentFrame();
		if (waitDeltaClipFrameMillis(kScene8010TransitionFrameMillis))
			break;
	}
}

void Scene8010::runFirstEntry() {
	_vm->gameplayMusic()->playMusicCue(0x0b, 100);
	runEntryPath(kScene8010EntryStartX, kScene8010EntryStartY, kScene8010EntryFacing,
		kScene8010EntryTargetX, kScene8010EntryTargetY);

	if (!_vm->gameState().seenScene8010EntryLine) {
		_vm->gameState().seenScene8010EntryLine = true;
		beginSecondarySpeechLine(0, 0);
	}
}

void Scene8010::runReturnEntryTransition() {
	setActiveActorPose(kScene8010EntryTargetX, kScene8010EntryTargetY, kScene8010EntryFacing);
	drawTransitionClip(9);
}

void Scene8010::runFishermanConversation() {
	GameplayState &state = _vm->gameState();
	if (state.scene8010FishermanConversationState == 0) {
		state.scene8010FishermanConversationState = 1;
		applySceneStateToHotspotsAndPatches(1);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 0);
		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 0, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 2);
		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 2, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 3);
		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 3, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 4);
		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 4, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 5);
	} else {
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 1);
		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 1, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 7);
	}

	if (state.scene8010FishermanConversationState != 2) {
		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 5, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 6);
		runFishermanQuiz();
		return;
	}

	beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 10, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
	beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 8);
}

void Scene8010::runExitToScene8020() {
	drawTransitionClip(8);
	_vm->gameState().mainFlowStateId = kScene8020State;
	_vm->gameState().activeActorPoseValid = false;
}

void Scene8010::runTravelScreenAction() {
	_vm->gameState().requestTravelScreenSelection(6);
}

void Scene8010::applyFishermanNameTextPatch() {
	if (_vm->gameState().scene8010FishermanConversationState == 0)
		return;

	const uint rowSize = kStage003SmallRowSize;
	const uint sourceOffset = 7 * rowSize;
	const uint destinationOffset = rowSize;
	if (sourceOffset + rowSize > _stage003SmallRows.size() ||
			destinationOffset + rowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, rowSize);
}

void Scene8010::setActiveActorPose(int x, int y, byte facing) {
	_activeActorWorldX = x;
	_activeActorWorldY = y;
	_activeActorFacing = facing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene8010::initializeFishermanQuizData(byte targetLineIndex) {
	bool usedShortPrompt[6];
	bool usedPromptPrefix[12];
	bool usedPromptSuffix[12];
	bool usedFinalPrompt[6];
	bool usedFinalSuffix[12];
	bool usedMenuPrefix[16];
	bool usedMenuSuffix[16];
	memset(usedShortPrompt, 0, sizeof(usedShortPrompt));
	memset(usedPromptPrefix, 0, sizeof(usedPromptPrefix));
	memset(usedPromptSuffix, 0, sizeof(usedPromptSuffix));
	memset(usedFinalPrompt, 0, sizeof(usedFinalPrompt));
	memset(usedFinalSuffix, 0, sizeof(usedFinalSuffix));
	memset(usedMenuPrefix, 0, sizeof(usedMenuPrefix));
	memset(usedMenuSuffix, 0, sizeof(usedMenuSuffix));

	if (!_fishermanQuizAlternatePattern) {
		_fishermanQuizEntries[0].promptStage = 0x54;
		_fishermanQuizEntries[0].promptRow = randomUnusedRow(0, 6, usedShortPrompt,
			ARRAYSIZE(usedShortPrompt));
		_fishermanQuizEntries[0].promptSuffixStage = 0x50;
		_fishermanQuizEntries[0].promptSuffixRow = randomUnusedRow(0, 12, usedPromptSuffix,
			ARRAYSIZE(usedPromptSuffix));
		_fishermanQuizEntries[0].menuStage = 0x56;
		_fishermanQuizEntries[0].menuRow = randomUnusedRow(8, 8, usedMenuPrefix, ARRAYSIZE(usedMenuPrefix));
		_fishermanQuizEntries[0].menuSuffixStage = 0x57;
		_fishermanQuizEntries[0].menuSuffixRow = randomUnusedRow(0, 16, usedMenuSuffix, ARRAYSIZE(usedMenuSuffix));

		for (uint i = 1; i < ARRAYSIZE(_fishermanQuizEntries); ++i) {
			_fishermanQuizEntries[i].promptStage = 0x53;
			_fishermanQuizEntries[i].promptRow = randomUnusedRow(0, 12, usedPromptPrefix,
				ARRAYSIZE(usedPromptPrefix));
			_fishermanQuizEntries[i].promptSuffixStage = 0x50;
			_fishermanQuizEntries[i].promptSuffixRow = randomUnusedRow(0, 12, usedPromptSuffix,
				ARRAYSIZE(usedPromptSuffix));
			_fishermanQuizEntries[i].menuStage = 0x56;
			_fishermanQuizEntries[i].menuRow = randomUnusedRow(8, 8, usedMenuPrefix, ARRAYSIZE(usedMenuPrefix));
			_fishermanQuizEntries[i].menuSuffixStage = 0x57;
			_fishermanQuizEntries[i].menuSuffixRow = randomUnusedRow(0, 16, usedMenuSuffix, ARRAYSIZE(usedMenuSuffix));
		}

		_fishermanQuizFinalEntry.promptStage = 0x55;
		_fishermanQuizFinalEntry.promptRow = randomUnusedRow(0, 6, usedFinalPrompt, ARRAYSIZE(usedFinalPrompt));
		_fishermanQuizFinalEntry.promptSuffixStage = 0x52;
		_fishermanQuizFinalEntry.promptSuffixRow = randomUnusedRow(0, 12, usedFinalSuffix, ARRAYSIZE(usedFinalSuffix));
		_fishermanQuizFinalEntry.menuStage = 0x56;
		_fishermanQuizFinalEntry.menuRow = randomUnusedRow(0, 8, usedMenuPrefix, ARRAYSIZE(usedMenuPrefix));
		_fishermanQuizFinalEntry.menuSuffixStage = 0x57;
		_fishermanQuizFinalEntry.menuSuffixRow = randomUnusedRow(0, 16, usedMenuSuffix, ARRAYSIZE(usedMenuSuffix));
		_fishermanQuizAlternatePattern = true;
	} else {
		_fishermanQuizEntries[0].promptStage = 0x54;
		_fishermanQuizEntries[0].promptRow = randomUnusedRow(0, 6, usedShortPrompt,
			ARRAYSIZE(usedShortPrompt));
		_fishermanQuizEntries[0].promptSuffixStage = 0x52;
		_fishermanQuizEntries[0].promptSuffixRow = randomUnusedRow(0, 12, usedPromptSuffix,
			ARRAYSIZE(usedPromptSuffix));
		_fishermanQuizEntries[0].menuStage = 0x56;
		_fishermanQuizEntries[0].menuRow = randomUnusedRow(0, 8, usedMenuPrefix, ARRAYSIZE(usedMenuPrefix));
		_fishermanQuizEntries[0].menuSuffixStage = 0x57;
		_fishermanQuizEntries[0].menuSuffixRow = randomUnusedRow(0, 16, usedMenuSuffix, ARRAYSIZE(usedMenuSuffix));

		for (uint i = 1; i < ARRAYSIZE(_fishermanQuizEntries); ++i) {
			_fishermanQuizEntries[i].promptStage = 0x51;
			_fishermanQuizEntries[i].promptRow = randomUnusedRow(0, 12, usedPromptPrefix,
				ARRAYSIZE(usedPromptPrefix));
			_fishermanQuizEntries[i].promptSuffixStage = 0x52;
			_fishermanQuizEntries[i].promptSuffixRow = randomUnusedRow(0, 12, usedPromptSuffix,
				ARRAYSIZE(usedPromptSuffix));
			_fishermanQuizEntries[i].menuStage = 0x56;
			_fishermanQuizEntries[i].menuRow = randomUnusedRow(0, 8, usedMenuPrefix, ARRAYSIZE(usedMenuPrefix));
			_fishermanQuizEntries[i].menuSuffixStage = 0x57;
			_fishermanQuizEntries[i].menuSuffixRow = randomUnusedRow(0, 16, usedMenuSuffix, ARRAYSIZE(usedMenuSuffix));
		}

		_fishermanQuizFinalEntry.promptStage = 0x55;
		_fishermanQuizFinalEntry.promptRow = randomUnusedRow(0, 6, usedFinalPrompt, ARRAYSIZE(usedFinalPrompt));
		_fishermanQuizFinalEntry.promptSuffixStage = 0x50;
		_fishermanQuizFinalEntry.promptSuffixRow = randomUnusedRow(0, 12, usedFinalSuffix, ARRAYSIZE(usedFinalSuffix));
		_fishermanQuizFinalEntry.menuStage = 0x56;
		_fishermanQuizFinalEntry.menuRow = randomUnusedRow(8, 8, usedMenuPrefix, ARRAYSIZE(usedMenuPrefix));
		_fishermanQuizFinalEntry.menuSuffixStage = 0x57;
		_fishermanQuizFinalEntry.menuSuffixRow = randomUnusedRow(0, 16, usedMenuSuffix, ARRAYSIZE(usedMenuSuffix));
		_fishermanQuizAlternatePattern = false;
	}

	for (uint line = 0; line < ARRAYSIZE(_fishermanQuizChoiceText); ++line) {
		const uint permutationIndex = (uint)targetLineIndex * ARRAYSIZE(_fishermanQuizChoiceText) + line;
		const byte answerIndex = kScene8010FishermanQuizLinePermutation[permutationIndex];
		_fishermanQuizChoiceEntryIndex[line] = answerIndex;
		_fishermanQuizChoiceText[line] =
			composeFishermanQuizChoice(_fishermanQuizEntries[answerIndex].menuStage,
				_fishermanQuizEntries[answerIndex].menuRow,
				_fishermanQuizEntries[answerIndex].menuSuffixStage,
				_fishermanQuizEntries[answerIndex].menuSuffixRow);
	}
}

Common::String Scene8010::composeFishermanQuizChoice(byte firstStage, byte firstRow, byte secondStage,
		byte secondRow) const {
	Common::String text = PlayableScene::dialogueMenuText(firstStage, firstRow);
	if (!text.empty())
		text += " ";
	text += PlayableScene::dialogueMenuText(secondStage, secondRow);
	return text;
}

void Scene8010::composeFishermanGeneratedPromptLines(byte promptLineIndex,
		Common::Array<Common::String> &lines) const {
	lines.clear();
	const FishermanQuizEntry &entry = promptLineIndex < ARRAYSIZE(_fishermanQuizEntries) ?
		_fishermanQuizEntries[promptLineIndex] : _fishermanQuizFinalEntry;

	const Common::String prompt = PlayableScene::dialogueMenuText(entry.promptStage, entry.promptRow);
	const Common::String promptSuffix = PlayableScene::dialogueMenuText(entry.promptSuffixStage, entry.promptSuffixRow);
	const Common::String menuPrefix = PlayableScene::dialogueMenuText(entry.menuStage, entry.menuRow);
	const Common::String menuSuffix = PlayableScene::dialogueMenuText(entry.menuSuffixStage, entry.menuSuffixRow);

	Common::String firstLine = promptLineIndex >= 1 && promptLineIndex <= 7 ? "..." : "";
	firstLine += prompt;
	if (prompt.size() + promptSuffix.size() < 0x33) {
		appendFishermanPromptFragment(firstLine, promptSuffix);
		if (!firstLine.empty())
			lines.push_back(firstLine);
	} else {
		if (!firstLine.empty())
			lines.push_back(firstLine);
		if (!promptSuffix.empty())
			lines.push_back(promptSuffix);
	}

	Common::String finalLine = menuPrefix;
	appendFishermanPromptFragment(finalLine, menuSuffix);
	if (!finalLine.empty())
		finalLine += promptLineIndex < 7 ? "..." : ".";
	if (!finalLine.empty())
		lines.push_back(finalLine);
}

uint16 Scene8010::fishermanQuizFragmentVoiceSampleId(const FishermanQuizEntry &entry, byte fragmentIndex) const {
	byte stage = 0;
	byte row = 0;
	switch (fragmentIndex) {
	case 0:
		stage = entry.promptStage;
		row = entry.promptRow;
		break;
	case 1:
		stage = entry.promptSuffixStage;
		row = entry.promptSuffixRow;
		break;
	case 2:
		stage = entry.menuStage;
		row = entry.menuRow;
		break;
	case 3:
		stage = entry.menuSuffixStage;
		row = entry.menuSuffixRow;
		break;
	default:
		return 0;
	}

	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(stage, row, textRecordId, continuationCount, voiceSampleId))
		return 0;
	return voiceSampleId;
}

bool Scene8010::waitFishermanQuizFragmentVoices(const FishermanQuizEntry &entry, byte firstFragment,
		uint32 fallbackMillis) {
	bool playedAny = false;
	for (byte fragment = firstFragment; fragment < 4 && !Engine::shouldQuit(); ++fragment) {
		const uint16 voiceSampleId = fishermanQuizFragmentVoiceSampleId(entry, fragment);
		const bool started = voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
		if (!started)
			continue;

		playedAny = true;
		const uint32 duration = MAX<uint32>(_speech.lastSampleDurationMillis(), 250);
		if (waitForSpeechOrDelay(duration, false)) {
			_speech.stop();
			return true;
		}
	}

	if (!playedAny)
		return waitForSpeechOrDelay(fallbackMillis, false);

	return Engine::shouldQuit();
}

bool Scene8010::runFishermanGeneratedPrimarySpeechLine(byte promptLineIndex) {
	const FishermanQuizEntry &entry = promptLineIndex < ARRAYSIZE(_fishermanQuizEntries) ?
		_fishermanQuizEntries[promptLineIndex] : _fishermanQuizFinalEntry;
	Common::Array<Common::String> lines;
	composeFishermanGeneratedPromptLines(promptLineIndex, lines);
	if (lines.empty())
		return false;

	setPaletteEntry6Bit(0xfb, kScene8010FishermanSpeechRed, kScene8010FishermanSpeechGreen,
		kScene8010FishermanSpeechBlue);
	_primarySpeechOverlay.visible = true;
	_primarySpeechOverlay.colorIndex = 0xfb;
	_primarySpeechOverlay.lines = lines;
	calculateSpeechOverlayBounds(_primarySpeechOverlay, kScene8010FishermanSpeechCenterX,
		kScene8010FishermanSpeechTopY, true, _activeActorWorldY);

	const byte baseFrame = primarySpeechAnimationBaseFrame(0);
	_speechController.startPrimaryDialogueSpeech(0, baseFrame);
	primarySpeechAnimationStarted(0, baseFrame);
	setPrimarySpeechAnimationFrame(0, baseFrame);

	const uint32 duration = MAX<uint32>(kScene8010GeneratedSpeechMinMillis,
		_primarySpeechOverlay.lines.size() * kScene8010GeneratedSpeechLineMillis);
	const bool interrupted = waitFishermanQuizFragmentVoices(entry, 0, duration);

	_speech.stop();
	setPrimarySpeechAnimationFrame(0, baseFrame);
	_speechController.stopPrimaryDialogueSpeech(kScene8010InvalidPrimarySpeechGroup,
		kScene8010DefaultPrimarySpeechFrame);
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
	primarySpeechAnimationRestored(0, baseFrame);
	return interrupted;
}

void Scene8010::runFishermanGeneratedPromptSpeech() {
	for (byte promptLine = 0; promptLine <= ARRAYSIZE(_fishermanQuizEntries) && !Engine::shouldQuit();
			++promptLine) {
		if (runFishermanGeneratedPrimarySpeechLine(promptLine))
			break;
	}
}

void Scene8010::runFishermanSelectedAnswerSpeech(byte selectedLine) {
	if (selectedLine >= ARRAYSIZE(_fishermanQuizChoiceText))
		return;

	Common::String text = _fishermanQuizChoiceText[selectedLine];
	if (text.empty())
		return;
	text += ".";

	_speechOverlay.visible = true;
	_speechOverlay.colorIndex = 0xfd;
	wrapActorSpeechText(text, _activeActorWorldX, _speechOverlay.lines);
	calculateSecondarySpeechBounds(_activeActorWorldX, _activeActorWorldY);
	_speechController.prepareSecondaryActorSpeech();

	const uint32 duration = MAX<uint32>(kScene8010SelectedAnswerMinMillis,
		_speechOverlay.lines.size() * kScene8010SelectedAnswerLineMillis);
	const byte entryIndex = _fishermanQuizChoiceEntryIndex[selectedLine];
	if (entryIndex < ARRAYSIZE(_fishermanQuizEntries))
		waitFishermanQuizFragmentVoices(_fishermanQuizEntries[entryIndex], 2, duration);
	else
		waitForSpeechOrDelay(duration, false);

	_speech.stop();
	_speechOverlay.visible = false;
	_speechOverlay.lines.clear();
}

byte Scene8010::chooseFishermanQuizLine() {
	DialogueMenuState state;
	for (byte line = 0; line < ARRAYSIZE(_fishermanQuizChoiceText); ++line) {
		DialogueMenuLine menuLine;
		menuLine.text = _fishermanQuizChoiceText[line];
		menuLine.choiceIndex = line;
		menuLine.firstLineOfChoice = true;
		state.lines.push_back(menuLine);
	}
	state.lineCount = (byte)state.lines.size();

	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	updateFishermanQuizMenuHover(state);
	drawDialogueMenuFrame();
	presentDialogueMenuFrame(state);

	uint32 lastMillis = g_system->getMillis();
	while (!Engine::shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				Engine::quitGame();
				_vm->cursor()->leaveInteractiveMode();
				return 0xff;
			case Common::EVENT_MOUSEMOVE:
				_vm->cursor()->updatePosition(event.mouse);
				updateFishermanQuizMenuHover(state);
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (state.highlightedLineIndex != 0xff) {
					_vm->cursor()->leaveInteractiveMode();
					return state.choiceForLine(state.highlightedLineIndex);
				}
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_vm->cursor()->leaveInteractiveMode();
					return 0xff;
				}
				if ((event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_SPACE) &&
						state.highlightedLineIndex != 0xff) {
					_vm->cursor()->leaveInteractiveMode();
					return state.choiceForLine(state.highlightedLineIndex);
				}
				break;
			default:
				break;
			}
		}

		g_system->delayMillis(kScene8010QuizMenuTickMillis);
		const uint32 now = g_system->getMillis();
		uint32 delta = now - lastMillis;
		lastMillis = now;
		if (delta > 250)
			delta = 250;

		advanceDialogueMenu(delta);
		_vm->cursor()->advance(delta);
		updateFishermanQuizMenuHover(state);
		drawDialogueMenuFrame();
		presentDialogueMenuFrame(state);
	}

	_vm->cursor()->leaveInteractiveMode();
	return 0xff;
}

void Scene8010::updateFishermanQuizMenuHover(DialogueMenuState &state) {
	state.highlightedLineIndex = fishermanQuizMenuLineAt(_vm->cursor()->surfaceY(), state);
}

byte Scene8010::fishermanQuizMenuLineAt(uint16 cursorY, const DialogueMenuState &state) const {
	if (state.lineCount == 0)
		return 0xff;

	const int topY = (int)kScene8010QuizMenuHoverTopBaseY - (int)state.lineCount * kScene8010QuizMenuLineHeight;
	if ((int)cursorY < topY || cursorY >= kScene8010QuizMenuHoverBottomY)
		return 0xff;

	const byte lineIndex = (byte)(((int)cursorY - topY) / kScene8010QuizMenuLineHeight);
	return lineIndex < state.lineCount ? lineIndex : 0xff;
}

byte Scene8010::randomUnusedRow(byte firstRow, byte rowCount, bool *usedRows, uint usedRowCount) {
	for (uint attempt = 0; attempt < rowCount * 2; ++attempt) {
		const byte row = (byte)(firstRow + _random.getRandomNumber(rowCount - 1));
		if (row < usedRowCount && !usedRows[row]) {
			usedRows[row] = true;
			return row;
		}
	}

	for (byte row = firstRow; row < firstRow + rowCount && row < usedRowCount; ++row) {
		if (!usedRows[row]) {
			usedRows[row] = true;
			return row;
		}
	}

	return firstRow;
}

bool Scene8010::runFishermanQuiz() {
	bool usedTargetLines[8];
	memset(usedTargetLines, 0, sizeof(usedTargetLines));

	for (byte round = 0; round < 3 && !Engine::shouldQuit(); ++round) {
		const byte targetLine = randomUnusedRow(0, ARRAYSIZE(usedTargetLines), usedTargetLines,
			ARRAYSIZE(usedTargetLines));
		initializeFishermanQuizData(targetLine);
		runFishermanGeneratedPromptSpeech();
		const byte selectedLine = chooseFishermanQuizLine();
		if (selectedLine == 0xff) {
			beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 9);
			return false;
		}

		runFishermanSelectedAnswerSpeech(selectedLine);
		if (selectedLine != targetLine) {
			beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 6, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
			return false;
		}

		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, (byte)(7 + round),
			0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
	}

	if (!Engine::shouldQuit()) {
		_vm->gameState().scene8010FishermanConversationState = 2;
		beginPrimarySpeechLine(kScene8010FishermanResponseSpeechRow, 10, 0x00fa, 0x0104, 0x0d, 0x0d, 0x3f);
		beginSecondarySpeechLine(kScene8010FishermanPlayerSpeechRow, 8);
	}

	return _vm->gameState().scene8010FishermanConversationState == 2;
}

} // End of namespace Hollywood
