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

#include "hollywood/scenes/playable/scene2080.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2080ArchiveName = "RESOURCE.B08";
const char *const kScene2080MusicArchiveName = "RESOURCE.M02";
const char *const kScene2080SoundArchiveName = "RESOURCE.S02";
const uint kScene2080InitialRequiredChunkCount = 13;
const uint kScene2080ArenaFirstChunk = 5;
const uint kScene2080ArenaLastChunk = 12;
const uint kScene2080StageIndex = 208;
const uint16 kScene2080FirstState = 0x0820;
const uint16 kScene2080LastState = 0x0821;
const uint16 kScene2070ReturnState = 0x0817;
const uint16 kScene2090FirstState = 0x082a;
const uint16 kScene2080ViewportXOffset = 0x00d0;
const uint16 kScene2080ViewportMaxXOffset = 0x0138;
const uint kScene2080ActorBankTableEntry = 0x0000;
const uint kScene2080ActorPaletteTableEntry = 0x00cc;
const uint kScene2080Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2080SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2080FrameMillis = 75;
const uint32 kScene2080ClipFrameMillis = 75;
const byte kScene2080InvalidFacing = 0xff;
const byte kScene2080ForegroundDialogueStageId = 0x62;
const byte kScene2080ForegroundEntryPrimaryRow = 0;
const byte kScene2080ForegroundDialoguePrimaryRow = 99;
const uint kScene2080ForegroundDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene2080NoPrimaryResponseFrame = 0xff;
const byte kScene2080InvalidPrimarySpeechGroup = 0xff;
const byte kScene2080DefaultPrimarySpeechFrame = 7;
const byte kScene2080PrimarySpeechTextColor = 0xfb;
const byte kScene2080ForegroundActorChunk = 5;
const byte kScene2080ForegroundActorDescriptorCount = 0x10;
const byte kScene2080ForegroundActorOpenFrame = 0x0b;
const byte kScene2080ForegroundActorRestFrame = 0x08;
const byte kScene2080ForegroundExitChunk = 6;
const byte kScene2080ForegroundExitDescriptorCount = 0x21;
const byte kScene2080ForwardExitOverlayChunk = 7;
const byte kScene2080ForwardExitOverlayDescriptorCount = 6;
const byte kScene2080PrincessHairSearchFirstChunk = 8;
const byte kScene2080PrincessHairSearchFirstDescriptorCount = 0x0c;
const byte kScene2080PrincessHairSearchSecondChunk = 9;
const byte kScene2080PrincessHairSearchSecondDescriptorCount = 0x0d;
const byte kScene2080AmbientChunk = 10;
const byte kScene2080AmbientDescriptorCount = 0x1a;
const byte kScene2080ForegroundLeftChunk = 11;
const byte kScene2080ForwardExitClipChunk = 12;
const byte kScene2080ForwardExitClipDescriptorCount = 0x14;
const byte kScene2080ReturnEntryClipChunk = 14;
const byte kScene2080ReturnEntryClipDescriptorCount = 0x15;
const byte kScene2080PrincessHairInventoryItem = 0x2e;
const uint kScene2080ExitBackRecordIndex = 9;

enum Scene2080OverlayHook {
	kScene2080ForegroundExitSoundHook = 1
};

const byte kScene2080ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene2080ForegroundActorFrameMap[] = {
	1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

const byte kScene2080AmbientFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25
};

const byte kScene2080ForwardExitOverlayFrameMap[] = {
	0, 1, 2, 3, 4, 5, 4, 5, 4, 3, 2, 1, 0
};

const byte kScene2080ForegroundExitFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 32
};

const byte kScene2080PrincessHairSearchFirstFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kScene2080PrincessHairSearchSecondFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

static_assert(ARRAYSIZE(kScene2080ActorPathStepDeltaTable) == 72, "Scene 2080 actor path table size changed");
static_assert(ARRAYSIZE(kScene2080ForegroundActorFrameMap) == 0x10, "Scene 2080 foreground actor frame map size changed");
static_assert(ARRAYSIZE(kScene2080AmbientFrameMap) == 0x1a, "Scene 2080 ambient frame map size changed");
static_assert(ARRAYSIZE(kScene2080ForwardExitOverlayFrameMap) == 13, "Scene 2080 forward exit overlay frame map size changed");
static_assert(ARRAYSIZE(kScene2080ForegroundExitFrameMap) == 0x21, "Scene 2080 foreground exit frame map size changed");
static_assert(ARRAYSIZE(kScene2080PrincessHairSearchFirstFrameMap) == 0x2c, "Scene 2080 princess hair search first frame map size changed");
static_assert(ARRAYSIZE(kScene2080PrincessHairSearchSecondFrameMap) == 0x0c, "Scene 2080 princess hair search second frame map size changed");

static PlayableSceneConfig scene2080Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2080ArchiveName;
	config.initialRequiredChunkCount = kScene2080InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2080ArenaFirstChunk;
	config.arenaLastChunk = kScene2080ArenaLastChunk;
	config.stageIndex = kScene2080StageIndex;
	config.debugName = "Scene 2080";
	config.viewportXOffset = kScene2080ViewportXOffset;
	config.viewportMinXOffset = kScene2080ViewportXOffset;
	config.viewportMaxXOffset = kScene2080ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorBankTableEntry = kScene2080ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene2080ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2080Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2080SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2080ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2080ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 1;
	config.musicArchiveName = kScene2080MusicArchiveName;
	config.soundBank0ArchiveName = kScene2080SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = false;
	config.mainFlowFirstState = kScene2080FirstState;
	config.mainFlowLastState = kScene2080LastState;
	return config;
}

Scene2080::Scene2080(HollywoodEngine *vm) :
		PlayableScene(vm, scene2080Config(), "scene2080", 0x168, 0x143, 1, 0xfd, 0xfb),
		_ambientChannel(),
		_foregroundActorChannel(),
		_ambientLayer(),
		_foregroundActorLayer(),
		_foregroundActorIdleState(0),
		_foregroundActorIdleDelay(0),
		_foregroundActorManualSequenceActive(false) {
	_ambientLayer.configure(kScene2080AmbientChunk, kScene2080AmbientDescriptorCount,
		kScene2080AmbientFrameMap, ARRAYSIZE(kScene2080AmbientFrameMap));
	_foregroundActorLayer.configure(kScene2080ForegroundActorChunk, kScene2080ForegroundActorDescriptorCount,
		kScene2080ForegroundActorFrameMap, ARRAYSIZE(kScene2080ForegroundActorFrameMap));
}

bool Scene2080::hasCustomPreviewState() const {
	return true;
}

void Scene2080::initializeCustomPreviewState() {
	normalizeLinkedPassageState();
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	if (_vm->gameState().mainFlowStateId == kScene2080LastState) {
		_activeActorWorldX = 0x17f;
		_activeActorWorldY = 299;
		_activeActorFacing = 5;
	} else {
		_activeActorWorldX = 0x064;
		_activeActorWorldY = 0x130;
		_activeActorFacing = 2;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2080::hasCustomComposite() const {
	return true;
}

void Scene2080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_foregroundActorLayer);
	drawResourceSpriteLayer(_ambientLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	const byte foregroundChunk = activeWorldX < 0x24d ?
		kScene2080ForegroundLeftChunk : kScene2080ForwardExitClipChunk;
	if (_sceneChunkTable.isValidChunk(foregroundChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[foregroundChunk], _sceneFramebuffer);
}

bool Scene2080::hasCustomEntrySequence() const {
	return true;
}

void Scene2080::runCustomEntrySequence() {
	normalizeLinkedPassageState();
	if (_vm->gameState().mainFlowStateId == kScene2080LastState)
		runEntryFromScene2090();
	else
		runEntryFromScene2070();
}

bool Scene2080::prepareCustomGameplayLoop() {
	normalizeLinkedPassageState();
	return true;
}

bool Scene2080::advanceCustomGameplayLoop(uint32 delta) {
	advanceAmbientLayer(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else if (!_foregroundActorManualSequenceActive)
		advanceForegroundActorIdle(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

void Scene2080::advanceDialogueMenu(uint32 delta) {
	advanceSecondaryActorSpeechAnimation(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceForegroundActorDialoguePose(delta);
	advanceAmbientLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	advanceViewportScroll(delta);
}

bool Scene2080::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir a puerta (go to door): Hecker blocks return until the curse conversation is resolved.
		if (state.scene2080ForegroundState != 0) {
			beginSecondarySpeechLine(1, 0);
			return true;
		}
		if (!state.scene2080ExitBackSequenceSeen) {
			beginSecondarySpeechLine(1, 1);
			walkActiveActorTo(0x064, 0x130, kScene2080InvalidFacing, 0, false);
			state.scene2080ExitBackSequenceSeen = true;
			applySceneStateToHotspotsAndPatches(2);
		}
		state.mainFlowStateId = kScene2070ReturnState;
		return true;
	case 302: // Mirar puerta (look at door): first-man-through warning, then Hecker has crossed it.
		beginSecondarySpeechLine(2, state.scene2080ForegroundState != 0 ? 0 : 1);
		return true;
	case 303: // Hablar con arqueólogo/Gunther Hecker (talk to archaeologist): curse dialogue tree.
		runForegroundDialogue();
		return true;
	case 304: // Mirar arqueólogo/Gunther Hecker (look at archaeologist): before/after he identifies himself.
		beginSecondarySpeechLine(3, state.scene2080ForegroundState < 2 ? 0 : 1);
		return true;
	case 305: // Coger/usar bastón hotspot (staff): animated transition to the altar scene after Hecker leaves.
		runForwardExitToScene2090();
		return true;
	case 306: // Mirar bastón (look at staff): Hecker is holding it tightly.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar sarcófago de Amesis-Huni (look at sarcophagus): empty/princess variant.
		beginSecondarySpeechLine(5, state.scene2080PrincessHairSearchState == 0 ? 0 : 1);
		return true;
	case 308: // Usar sarcófago (use sarcophagus): Ron refuses to get inside.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar sarcófago (look at side sarcophagus): preserved for centuries.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar jeroglífico (look at hieroglyphic): Ron cannot read it.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Mirar abertura (look at opening): first look wonders what is above; repeated look names Karnak.
		beginSecondarySpeechLine(9, state.scene2080FirstRow09LookSeen ? 1 : 0);
		state.scene2080FirstRow09LookSeen = true;
		return true;
	case 312: // Usar abertura (use opening): Ron cannot reach it.
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Usar lupa con sarcófago central (use magnifying glass): finds Amesis-Huni's hair.
		runCentralSarcophagusHairSearch();
		return true;
	case 314: // Usar lupa con sarcófago lateral (use magnifying glass): no special interest response.
		beginSecondarySpeechLine(11, 0);
		return true;
	default:
		return false;
	}
}

bool Scene2080::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x0d8, 0x2f7);
	if (targetY < 0x1df)
		++targetY;

	while (targetY < 0x1df) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		++targetY;
	}

	if (walkableMaskAt(targetX, targetY) != 0)
		return true;

	while (targetY > 0) {
		--targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene2080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if (selector == 1 || selector == 0xff) {
		if (state.scene2080ForegroundState == 0) {
			replaceColorMapItemFromOriginal(2, 0);
			replaceColorMapItemFromOriginal(3, 3);
			replaceColorMapItemFromOriginal(9, 6);
		} else {
			replaceColorMapItemFromOriginal(2, 2);
			replaceColorMapItemFromOriginal(3, 6);
			replaceColorMapItemFromOriginal(9, 2);
			if (state.scene2080ForegroundState > 1)
				copySmallRow(0x171, 0x52);
		}
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (selector == 1 || selector == 0xff) {
		const int16 y = state.scene2080ForegroundState == 0 ? 299 : 0x131;
		setHotspotInteractionY(3, y);
		setHotspotInteractionY(6, y);
	}

	if ((selector == 2 || selector == 0xff) && state.scene2080ExitBackSequenceSeen) {
		setHotspotInteractionX(1, 100);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene2080ExitBackRecordIndex, 1);
	}

	return true;
}

byte Scene2080::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return kScene2080ForegroundActorOpenFrame;
}

void Scene2080::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	if (_vm->gameState().scene2080ForegroundState != 0) {
		_foregroundActorLayer.visible = true;
		_foregroundActorLayer.setFrame(frameIndex);
	}
}

void Scene2080::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	if (_vm->gameState().scene2080ForegroundState != 0) {
		_foregroundActorLayer.visible = true;
		_foregroundActorLayer.setFrame(baseFrame);
	}
}

void Scene2080::handleActionOverlayFrameHook(byte hookId, uint frame) {
	if (hookId == kScene2080ForegroundExitSoundHook &&
			(frame == 6 || frame == 12 || frame == 19 || frame == 24 || frame == 30))
		_soundBank0.playSample(10, 100);
}

AmbientAudioProfile Scene2080::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2080::resetAnimationLayers() {
	_ambientChannel.reset(0, kScene2080FrameMillis);
	_foregroundActorChannel.reset(0, kScene2080FrameMillis);
	_ambientLayer.visible = true;
	_ambientLayer.reset(0);
	_foregroundActorLayer.visible = _vm->gameState().scene2080ForegroundState != 0;
	_foregroundActorLayer.reset(1);
	_foregroundActorIdleState = 0;
	_foregroundActorIdleDelay = 0;
	_foregroundActorManualSequenceActive = false;
}

void Scene2080::advanceAmbientLayer(uint32 delta) {
	const uint frameCount = _ambientChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		byte nextFrame = (byte)(_ambientLayer.frameIndex + 1);
		if (nextFrame >= ARRAYSIZE(kScene2080AmbientFrameMap))
			nextFrame = 0;
		_ambientLayer.setFrame(nextFrame);
	}
}

void Scene2080::advanceForegroundActorIdle(uint32 delta) {
	if (_vm->gameState().scene2080ForegroundState == 0)
		return;

	const uint frameCount = _foregroundActorChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		switch (_foregroundActorIdleState) {
		case 0:
			if (_random.getRandomNumber(0x1d) == 0) {
				_foregroundActorLayer.setFrame(2);
				_foregroundActorIdleState = 2;
				_foregroundActorIdleDelay = (byte)_random.getRandomNumber(0x13);
			} else if (_random.getRandomNumber(0x0e) == 0) {
				_foregroundActorLayer.setFrame(0);
				_foregroundActorIdleState = 1;
			}
			break;
		case 1:
			_foregroundActorLayer.setFrame(1);
			_foregroundActorIdleState = 0;
			break;
		case 2:
			if (_foregroundActorLayer.frameIndex != 8) {
				_foregroundActorLayer.setFrame(_foregroundActorLayer.frameIndex + 1);
			} else if (_foregroundActorIdleDelay == 0) {
				_foregroundActorIdleState = 3;
			} else {
				--_foregroundActorIdleDelay;
			}
			break;
		case 3:
			if (_foregroundActorLayer.frameIndex == 1) {
				_foregroundActorIdleState = 0;
			} else {
				_foregroundActorLayer.setFrame(_foregroundActorLayer.frameIndex - 1);
			}
			break;
		default:
			_foregroundActorIdleState = 0;
			_foregroundActorLayer.setFrame(1);
			break;
		}
	}
}

void Scene2080::advanceForegroundActorDialoguePose(uint32 delta) {
	if (_vm->gameState().scene2080ForegroundState == 0)
		return;

	const uint frameCount = _foregroundActorChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_foregroundActorLayer.frameIndex == 0x0f) {
			_foregroundActorLayer.setFrame(kScene2080ForegroundActorOpenFrame);
		} else if (_random.getRandomNumber(14) == 0) {
			_foregroundActorLayer.setFrame(0x0f);
		}
	}
}

void Scene2080::normalizeLinkedPassageState() {
	GameplayState &state = _vm->gameState();

	// The original foreground actor exit opens the return passage in B07.
	// Do not infer the reverse: Hecker must remain talkable while his scene state says he is present.
	if (state.scene2080ForegroundState == 0)
		state.scene2070InnerPassagePatchState = 1;
}

void Scene2080::runEntryFromScene2070() {
	resetAnimationLayers();
	runEntryPathWithFinalFacing(0x064, 0x130, 2, 0x168, 0x143,
		_vm->gameState().scene2080EntryLineSeen ? kScene2080InvalidFacing : 1, 0);

	GameplayState &state = _vm->gameState();
	if (!state.scene2080EntryLineSeen) {
		beginForegroundActorEntrySpeechLine();
		state.scene2080EntryLineSeen = true;
	}
}

void Scene2080::runEntryFromScene2090() {
	resetAnimationLayers();
	if (_sceneChunkTable.isValidChunk(kScene2080ReturnEntryClipChunk)) {
		Common::Array<byte> clipData;
		if (loadVariableChunk(kScene2080ReturnEntryClipChunk, clipData)) {
			copyBaseFramebufferToSceneFramebuffer();
			drawResourceSpriteLayer(_ambientLayer);
			drawClipFrameDeltaFromResource(clipData, 0, clipData.size(),
				kScene2080ReturnEntryClipDescriptorCount, 0);
			presentFrame();
			playDeltaClipFromResource(clipData, 0, clipData.size(),
				kScene2080ReturnEntryClipDescriptorCount,
				kScene2080ReturnEntryClipDescriptorCount, kScene2080ClipFrameMillis, 1);
		}
	}

	runEntryPathWithFinalFacing(0x17f, 299, 5, 0x1f4, 0x154, kScene2080InvalidFacing, 0);
}

void Scene2080::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);

	drawPlayableComposite();
	presentFrame();

	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	if (finalFacing != kScene2080InvalidFacing)
		_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene2080::openForegroundActorForSpeech() {
	_foregroundActorManualSequenceActive = true;
	_foregroundActorLayer.visible = true;
	while (_foregroundActorLayer.frameIndex < kScene2080ForegroundActorOpenFrame &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		_foregroundActorLayer.setFrame(_foregroundActorLayer.frameIndex + 1);
		if (waitSceneMillis(kScene2080FrameMillis))
			break;
	}
	_foregroundActorManualSequenceActive = false;
}

void Scene2080::closeForegroundActorAfterSpeech() {
	_foregroundActorManualSequenceActive = true;
	while (_foregroundActorLayer.frameIndex > kScene2080ForegroundActorRestFrame &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		_foregroundActorLayer.setFrame(_foregroundActorLayer.frameIndex - 1);
		if (waitSceneMillis(kScene2080FrameMillis))
			break;
	}
	_foregroundActorManualSequenceActive = false;
	_foregroundActorIdleState = 2;
	_foregroundActorIdleDelay = (byte)_random.getRandomNumber(0x13);
}

void Scene2080::beginForegroundActorEntrySpeechLine() {
	openForegroundActorForSpeech();
	runForegroundActorPrimarySpeechLine(kScene2080ForegroundEntryPrimaryRow, 0);
	closeForegroundActorAfterSpeech();
}

bool Scene2080::waitForegroundDialogueMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		advanceDialogueMenu(slice);
		drawPlayableComposite();
		presentFrame();
		remaining -= slice;
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene2080::beginForegroundDialogueSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	const bool started = startSecondarySpeechLine(rowIndex, frameIndex);
	if (!started && _speechOverlay.lines.empty())
		return;

	const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(1200, (uint32)_speechOverlay.lines.size() * 1100);
	uint32 elapsed = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= duration)
			break;

		const uint32 slice = speechActive ? 50 : MIN<uint32>(50, duration - elapsed);
		if (waitForegroundDialogueMillis(slice))
			break;
		elapsed += slice;
	}

	_speech.stop();
	clearSpeechOverlay();
}

void Scene2080::runForegroundActorPrimarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	setPaletteEntry6Bit(kScene2080PrimarySpeechTextColor, 0x20, 0x30, 0x3f);

	const byte baseFrame = primarySpeechAnimationBaseFrame(0);
	_speechController.startPrimaryDialogueSpeech(0, baseFrame);
	primarySpeechAnimationStarted(0, baseFrame);
	setPrimarySpeechAnimationFrame(0, baseFrame);

	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++part) {
		const Common::String text = getResource003LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		_primarySpeechOverlay.visible = true;
		_primarySpeechOverlay.colorIndex = kScene2080PrimarySpeechTextColor;
		wrapActorSpeechText(text, 0x1a9, _primarySpeechOverlay.lines);
		calculateSpeechOverlayBounds(_primarySpeechOverlay, 0x1a9, 0x69, true,
			_activeActorWorldY);

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _primarySpeechOverlay.lines.size() * 1100);
		const bool interrupted = waitForSpeechOrDelay(duration, false);

		_speech.stop();
		_primarySpeechOverlay.visible = false;
		_primarySpeechOverlay.lines.clear();
		if (interrupted)
			break;
	}

	setPrimarySpeechAnimationFrame(0, baseFrame);
	_speechController.stopPrimaryDialogueSpeech(kScene2080InvalidPrimarySpeechGroup,
		kScene2080DefaultPrimarySpeechFrame);
	primarySpeechAnimationRestored(0, baseFrame);
}

void Scene2080::runForegroundDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeForegroundDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	if (state.scene2080ForegroundState < 2) {
		beginSecondarySpeechLine(kScene2080ForegroundDialogueStageId, 0);
		openForegroundActorForSpeech();
		runForegroundActorPrimarySpeechLine(kScene2080ForegroundDialoguePrimaryRow, 0);
		state.scene2080ForegroundState = 2;
		applySceneStateToHotspotsAndPatches(1);
	} else {
		beginSecondarySpeechLine(kScene2080ForegroundDialogueStageId, 1);
		openForegroundActorForSpeech();
		runForegroundActorPrimarySpeechLine(kScene2080ForegroundDialoguePrimaryRow, 1);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene2080ForegroundDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginForegroundDialogueSecondarySpeechLine(kScene2080ForegroundDialogueStageId, 6);
			runForegroundActorPrimarySpeechLine(kScene2080ForegroundDialoguePrimaryRow, 6);
			closeForegroundActorAfterSpeech();
			return;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			closeForegroundActorAfterSpeech();
			return;
		}

		DialogueChoiceRecord &record = records[recordIndex];
		beginForegroundDialogueSecondarySpeechLine(kScene2080ForegroundDialogueStageId,
			record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != kScene2080NoPrimaryResponseFrame)
			runForegroundActorPrimarySpeechLine(kScene2080ForegroundDialoguePrimaryRow,
				record.responseFrameIndex);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (record.disableAfterUse == 2) {
			if (!records.empty()) {
				records[0].enabled = 1;
				records[0].selectable = 1;
			}
			state.scene2080PrincessHairSearchState = 1;
		} else if (record.disableAfterUse == 3) {
			if (records.size() > 2) {
				records[2].enabled = 1;
				records[2].selectable = 1;
			}
			state.scene2080DialogueBranchBUnlocked = true;
		} else if (record.disableAfterUse == 4) {
			beginForegroundDialogueSecondarySpeechLine(kScene2080ForegroundDialogueStageId, 0x12);
			runForegroundActorPrimarySpeechLine(kScene2080ForegroundDialoguePrimaryRow, 0x0d);
			runForegroundActorExitOverlay();
			return;
		}

		switch (record.transitionMode) {
		case 0:
			closeForegroundActorAfterSpeech();
			return;
		case 1:
			nodeIndex = record.nextNodeIndex;
			++depthIndex;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			if (depthIndex != 0)
				--depthIndex;
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 1 ? (byte)(depthIndex - 2) : 0;
			break;
		default:
			break;
		}
	}
}

void Scene2080::initializeForegroundDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	const GameplayState &state = _vm->gameState();
	records.resize(kScene2080ForegroundDialogueChoiceRecordCount);

	setDialogueRecord(records, 0, 0, 0, 3, 2, 2, 3, 0xff);
	setDialogueRecord(records, 1, 1, 0, 1, 3, 3, 2, 0xff);
	setDialogueRecord(records, 2, 0, 2, 1, 4, 4, 1, 0xff);
	setDialogueRecord(records, 3, 1, 1, 1, 5, 5, 1, 0xff);
	setDialogueRecord(records, 4, 1, 0, 0, 6, 6, 1, 0xff);
	setDialogueRecord(records, 70, 1, 0, 2, 7, 7, 1, 0xff);
	setDialogueRecord(records, 77, 1, 0, 2, 8, 8, 1, 0xff);
	setDialogueRecord(records, 84, 1, 0, 0, 9, 10, 1, 0xff);
	setDialogueRecord(records, 85, 1, 0, 0, 10, 10, 1, 0xff);
	setDialogueRecord(records, 86, 1, 0, 0, 11, 10, 1, 0xff);
	setDialogueRecord(records, 87, 1, 0, 0, 12, 10, 1, 0xff);
	setDialogueRecord(records, 88, 1, 0, 0, 6, 6, 1, 0xff);
	setDialogueRecord(records, 140, 1, 0, 0, 13, 10, 1, 0xff);
	setDialogueRecord(records, 141, 1, 0, 0, 14, 10, 1, 0xff);
	setDialogueRecord(records, 142, 1, 0, 0, 15, 10, 1, 0xff);
	setDialogueRecord(records, 143, 1, 0, 0, 16, 10, 1, 0xff);
	setDialogueRecord(records, 144, 1, 0, 0, 6, 6, 1, 0xff);
	setDialogueRecord(records, 210, 1, 0, 0, 17, 10, 1, 0xff);
	setDialogueRecord(records, 211, 1, 0, 0, 19, 10, 1, 0xff);
	setDialogueRecord(records, 212, 1, 0, 0, 20, 10, 1, 0xff);
	setDialogueRecord(records, 213, 1, 0, 0, 21, 10, 1, 0xff);
	setDialogueRecord(records, 214, 1, 0, 0, 6, 6, 1, 0xff);

	if (state.scene2080PrincessHairSearchState != 0) {
		records[0].enabled = 1;
		records[0].selectable = 1;
		records[1].disableAfterUse = 1;
	}
	if (state.scene2080DialogueBranchBUnlocked) {
		records[2].enabled = 1;
		records[2].selectable = 1;
		records[0].disableAfterUse = 1;
	}

	const uint branchARecord = 84 + state.scene2080DialogueBranchAIndex;
	if (branchARecord < records.size()) {
		records[branchARecord].transitionMode = 1;
		records[branchARecord].responseFrameIndex = 9;
	}
	const uint branchBRecord = 140 + state.scene2080DialogueBranchBIndex;
	if (branchBRecord < records.size()) {
		records[branchBRecord].transitionMode = 1;
		records[branchBRecord].responseFrameIndex = 0x0b;
	}
	const uint terminalRecord = 210 + state.scene2080DialogueTerminalIndex;
	if (terminalRecord < records.size()) {
		records[terminalRecord].responseFrameIndex = 0x0c;
		records[terminalRecord].disableAfterUse = 4;
	}
}

void Scene2080::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = enabled;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = reserved;
	record.selectable = enabled != 0;
}

void Scene2080::runForegroundActorExitOverlay() {
	_foregroundActorLayer.visible = false;
	ActionOverlayOptions options;
	options.actorVisibility = kActionOverlayKeepActiveActorVisibility;
	options.hookId = kScene2080ForegroundExitSoundHook;
	runActionOverlay(kScene2080ForegroundExitChunk, kScene2080ForegroundExitDescriptorCount,
		kScene2080ForegroundExitFrameMap, ARRAYSIZE(kScene2080ForegroundExitFrameMap),
		kScene2080FrameMillis, options);

	GameplayState &state = _vm->gameState();
	state.scene2080ForegroundState = 0;
	state.scene2070InnerPassagePatchState = 1;
	state.scene2070HiddenItemPatchState = 1;
	_foregroundActorLayer.visible = false;
	applySceneStateToHotspotsAndPatches(0xff);
	runPostForegroundDialogueEffect();
}

void Scene2080::runPostForegroundDialogueEffect() {
	walkActiveActorTo(0x168, 0x143, 5, 0, false);
	for (uint i = 0; i < 0x4b && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++i) {
		if (waitSceneMillis(10))
			break;
	}
	beginSecondarySpeechLine(12, 0);
}

void Scene2080::runForwardExitToScene2090() {
	if (_vm->gameState().scene2080ForegroundState != 0) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	runConfiguredActionOverlay(kScene2080ForwardExitOverlayChunk,
		kScene2080ForwardExitOverlayDescriptorCount,
		kScene2080ForwardExitOverlayFrameMap, ARRAYSIZE(kScene2080ForwardExitOverlayFrameMap),
		kScene2080FrameMillis, kActionOverlayHideActiveActor, -1, 0, 6, 0x11,
		100, -1, 0, true, 1);

	_soundBank0.playSample(0x12, 100);
	playDeltaClip(kScene2080ForwardExitClipChunk, kScene2080ForwardExitClipDescriptorCount,
		kScene2080ForwardExitClipDescriptorCount, kScene2080ClipFrameMillis);
	_vm->gameState().mainFlowStateId = kScene2090FirstState;
}

void Scene2080::runCentralSarcophagusHairSearch() {
	GameplayState &state = _vm->gameState();
	if (state.scene2080PrincessHairSearchState == 0) {
		beginSecondarySpeechLine(11, 0);
		return;
	}
	if (state.scene2080PrincessHairSearchState == 2) {
		beginSecondarySpeechLine(11, 4);
		return;
	}

	beginSecondarySpeechLine(11, 1);
	runConfiguredActionOverlay(kScene2080PrincessHairSearchFirstChunk,
		kScene2080PrincessHairSearchFirstDescriptorCount,
		kScene2080PrincessHairSearchFirstFrameMap, ARRAYSIZE(kScene2080PrincessHairSearchFirstFrameMap),
		kScene2080FrameMillis, kActionOverlayHideActiveActor);
	beginSecondarySpeechLine(11, 2);
	runConfiguredActionOverlay(kScene2080PrincessHairSearchSecondChunk,
		kScene2080PrincessHairSearchSecondDescriptorCount,
		kScene2080PrincessHairSearchSecondFrameMap, ARRAYSIZE(kScene2080PrincessHairSearchSecondFrameMap),
		kScene2080FrameMillis, kActionOverlayHideActiveActor);

	if (!hasInventoryItem(kScene2080PrincessHairInventoryItem))
		addInventoryItem(kScene2080PrincessHairInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene2080PrincessHairSearchState = 2;
	beginSecondarySpeechLine(11, 3);
}

void Scene2080::drawClipFrameDeltaFromResource(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, byte frameIndex) {
	if (frameIndex >= tableEntryCount)
		return;

	const uint32 chunkEnd = frameTableOffset + chunkSize;
	if (frameTableOffset > resource.size() || chunkEnd > resource.size() ||
			frameTableOffset + (uint32)tableEntryCount * 4 > chunkEnd)
		return;

	const uint32 tableEntryOffset = frameTableOffset + ((uint32)frameIndex * 4);
	if (tableEntryOffset + 4 > chunkEnd)
		return;

	const uint32 frameOffset = frameTableOffset + ((uint32)tableEntryCount * 4) +
		readUint32LE(resource, tableEntryOffset);
	if (frameOffset + 4 > chunkEnd)
		return;

	const uint16 firstRow = readUint16LE(resource, frameOffset);
	const uint16 lastRow = readUint16LE(resource, frameOffset + 2);
	if (firstRow > lastRow)
		return;

	uint cursor = frameOffset + 4;
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	const uint size = framebufferByteCount();
	if (!pixels)
		return;

	for (uint row = firstRow; row <= lastRow && row < HollywoodEngine::kSceneBufferHeight; ++row) {
		if (cursor >= chunkEnd)
			return;

		byte runCount = resource[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > chunkEnd)
				return;

			const uint x = readUint16LE(resource, cursor);
			const byte literalLength = resource[cursor + 2];
			const uint destinationOffset = row * HollywoodEngine::kSceneBufferWidth + x;

			if (literalLength == 0) {
				if (cursor + 5 > chunkEnd)
					return;

				const byte fillValue = resource[cursor + 3];
				const uint fillLength = resource[cursor + 4];
				cursor += 5;
				if (x < HollywoodEngine::kSceneBufferWidth && destinationOffset < size) {
					const uint copyLength = MIN<uint>(fillLength,
						HollywoodEngine::kSceneBufferWidth - x);
					if (destinationOffset + copyLength <= size)
						memset(pixels + destinationOffset, fillValue, copyLength);
				}
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > chunkEnd)
					return;

				if (x < HollywoodEngine::kSceneBufferWidth && destinationOffset < size) {
					const uint copyLength = MIN<uint>((uint)literalLength,
						HollywoodEngine::kSceneBufferWidth - x);
					if (destinationOffset + copyLength <= size)
						memcpy(pixels + destinationOffset, resource.data() + literalOffset, copyLength);
				}
				cursor = literalOffset + literalLength;
			}
		}
	}
}

void Scene2080::drawClipFrameDelta(byte chunkIndex, uint tableEntryCount, byte frameIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;

	drawClipFrameDeltaFromResource(_resourceArena, _resourceChunkOffsets[chunkIndex],
		_sceneChunkTable.sizes[chunkIndex], tableEntryCount, frameIndex);
}

void Scene2080::playDeltaClipFromResource(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, uint frameCount, uint32 frameMillis, uint firstFrame) {
	for (uint frame = firstFrame; frame < frameCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		drawClipFrameDeltaFromResource(resource, frameTableOffset, chunkSize, tableEntryCount, (byte)frame);
		presentFrame();
		if (waitTransitionFrameMillis(frameMillis))
			break;
	}
}

void Scene2080::playDeltaClip(byte chunkIndex, uint tableEntryCount, uint frameCount, uint32 frameMillis,
		uint firstFrame) {
	for (uint frame = firstFrame; frame < frameCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		drawClipFrameDelta(chunkIndex, tableEntryCount, (byte)frame);
		presentFrame();
		if (waitTransitionFrameMillis(frameMillis))
			break;
	}
}

bool Scene2080::waitTransitionFrameMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene2080::copySmallRow(uint sourceOffset, uint destinationOffset) {
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene2080::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == sourceItem)
			_paletteMask[kSceneColorToItemMap + color] = destinationItem;
	}
}

void Scene2080::setHotspotInteractionY(byte itemId, int16 y) {
	SceneActionTarget target = _hotspots.actionTarget(itemId);
	target.interactionPoint.y = y;
	_hotspots.setActionTarget(itemId, target.interactionPoint, target.approachPoint);
}

void Scene2080::setHotspotInteractionX(byte itemId, int16 x) {
	SceneActionTarget target = _hotspots.actionTarget(itemId);
	target.interactionPoint.x = x;
	_hotspots.setActionTarget(itemId, target.interactionPoint, target.approachPoint);
}

} // End of namespace Hollywood
