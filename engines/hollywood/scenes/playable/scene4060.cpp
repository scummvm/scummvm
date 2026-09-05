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

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/scenes/playable/scene4060.h"

namespace Hollywood {

const uint16 kScene4060ReturnState = 0x0fdd;
const uint16 kScene4100EntryFromScene4060State = 0x1005;
const int kScene4060EntryRonWorldX = 0x0064;
const int kScene4060EntryRonWorldY = 0x0145;
const byte kScene4060EntryRonFacing = 2;
const int kScene4060ReturnRonWorldX = 0x0292;
const int kScene4060ReturnRonWorldY = 0x010c;
const byte kScene4060ReturnRonFacing = 4;
const uint kScene4060ActorBankTableEntry = 0x0000;
const uint kScene4060ActorPaletteTableEntry = 0x00cc;
const uint kScene4060Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4060SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4060FrameMillis = 75;
const uint kScene4060ForegroundState0Chunk = 11;
const uint kScene4060ForegroundState1Chunk = 12;
const uint kScene4060ForegroundState0DescriptorCount = 0x1b;
const uint kScene4060ForegroundState1DescriptorCount = 0x19;
const uint kScene4060FirstCardPatchChunk = 8;
const uint kScene4060MirrorInstalledPatchChunk = 9;
const uint kScene4060SecondCardPatchChunk = 5;
const uint kScene4060ForegroundMidPatchChunk = 20;
const uint kScene4060PokerTableTransitionChunk = 13;
const uint kScene4060PokerTableTransitionDescriptorCount = 0x1b;
const uint kScene4060PokerTableTransitionLayer = 0;
const uint kScene4060PokerOverlayChunk = 14;
const uint kScene4060PokerOverlayDescriptorCount = 0x0b;
const uint kScene4060PokerRewardOverlayChunk = 17;
const uint kScene4060PokerRewardOverlayDescriptorCount = 0x0c;
const uint kScene4060PokerOverlayTransitionLayer = 1;
const uint kScene4060ForegroundLayer = 2;
const uint kScene4060EntryOverlayChunk = 19;
const uint kScene4060EntryOverlayDescriptorCount = 4;
const uint kScene4060ExitOverlayChunk = 15;
const uint kScene4060ExitOverlayDescriptorCount = 4;
const uint kScene4060FirstCardOverlayChunk = 18;
const uint kScene4060FirstCardOverlayDescriptorCount = 12;
const uint kScene4060SecondCardOverlayChunk = 16;
const uint kScene4060SecondCardOverlayDescriptorCount = 12;
const byte kScene4060FirstWonCardItem = 0x40;
const byte kScene4060SecondWonCardItem = 0x41;
const byte kScene4060DialogueRewardItem = 0x3f;
const byte kScene4060MirrorItem = 0x55;
const byte kScene4060DialogueStageId = 0x62;
const byte kScene4060SherilynResponseRow = 99;
const byte kScene4060PrimarySpeechTextColor = 0xfb;
const uint16 kScene4060SherilynSpeechCenterX = 0x01e3;
const uint16 kScene4060SherilynSpeechTopY = 0x004b;
const byte kScene4060SherilynSpeechRed = 0x3f;
const byte kScene4060SherilynSpeechGreen = 0x32;
const byte kScene4060SherilynSpeechBlue = 0x00;
const byte kScene4060SherilynAnimationGroup = 0;
const byte kScene4060TeddyBearAnimationGroup = 1;
const uint16 kScene4060TeddyBearSpeechCenterX = 0x0143;
const uint16 kScene4060TeddyBearSpeechTopY = 0x0077;
const byte kScene4060TeddyBearSpeechRed = 0x07;
const byte kScene4060TeddyBearSpeechGreen = 0x3f;
const byte kScene4060TeddyBearSpeechBlue = 0x3f;
const byte kScene4060TeddyBearSpeechVolumePercent = 25;
const uint32 kScene4060AmbientCheckMillis = 250;
const byte kScene4060FirstRandomAmbientCue = 0x0f;
const byte kScene4060RandomAmbientCueCount = 5;
const byte kScene4060SpecialAmbientCue = 0x0e;
const byte kScene4060PokerRestOverlayFrame = 7;
const byte kScene4060PokerRestTableFrame = 0x14;
const uint kScene4060PictureVerbRecordIndex = 0x1b;
const int kScene4060MinWalkX = 0x51;
const int kScene4060MaxWalkX = 0x296;
const int kScene4060MaxWalkY = 0x1df;
const byte kScene4060ForegroundIdleStep = 0;
const byte kScene4060SherilynSpeechPoseNone = 0;
const byte kScene4060SherilynSpeechPoseDirect = 2;
const byte kScene4060SherilynSpeechPoseOpened = 3;
const byte kScene4060SherilynDirectSpeechFirstStep = 2;
const byte kScene4060SherilynDirectSpeechLastStep = 6;
const byte kScene4060SherilynOpenedSpeechFirstStep = 0x14;
const byte kScene4060SherilynOpenedSpeechLastStep = 0x18;
const byte kScene4060SherilynOpenFirstStep = 7;
const byte kScene4060SherilynOpenLastStep = 0x0c;
const byte kScene4060SherilynCloseFirstStep = 0x0d;
const byte kScene4060SherilynCloseLastStep = 0x13;
const byte kScene4060ForegroundLongStartStep = 0x19;
const byte kScene4060ForegroundState0LongEndStep = 0x2a;
const byte kScene4060ForegroundState1LongEndStep = 0x2c;
const byte kScene4060MaxDialogueProgressCounter = 0x3c;
const byte kScene4060CardStateBase = 0;
const byte kScene4060CardStateFirstWon = 1;
const byte kScene4060CardStateMirrorInstalled = 2;
const byte kScene4060SecondCardStatePrompted = 1;
const byte kScene4060SecondCardStateWon = 2;

const byte kScene4060DemoEntryFrameMap[] = {
	0, 0, 1, 2, 3
};

const byte kScene4060PokerOpenTableFrames[] = {
	0x10, 0x11, 0x12, 0x13, 0x14, 0x14, 0x14
};

const byte kScene4060PokerHandOverlayFrames[] = {
	8, 9, 10, 9, 8, 7
};

const byte kScene4060PokerHandTableFrames[] = {
	0x15, 0x16, 0x17, 0x16, 0x15, 0x14
};

const byte kScene4060PokerShortCloseOverlayFrames[] = {
	6, 5, 4, 3, 2, 1, 0, 0, 0, 0
};

const byte kScene4060PokerShortCloseTableFrames[] = {
	0x14, 0x13, 0x12, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
};

const byte kScene4060PokerRewardOverlayFrames[] = {
	6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene4060PokerRewardTableFrames[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	13, 13, 13, 13, 14, 15, 0x19, 0x1a, 0x18, 0x18, 0x18, 0x18
};

const byte kScene4060ForegroundState0FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 11, 10, 9, 8, 7, 2, 0,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25, 26, 25, 24, 23, 22, 21,
	19, 18, 0, 0, 0
};

const byte kScene4060ForegroundState1FrameMap[] = {
	0, 1, 0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 9, 8, 7, 6, 5, 4, 0,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 23, 22, 21, 20, 19,
	18, 17, 16, 15, 0
};

struct Scene4060DialogueSeedRecord {
	uint16 index;
	byte enabled;
	byte nextNodeIndex;
	byte transitionMode;
	byte playerTextRowId;
	byte responseFrameIndex;
	byte disableAfterUse;
};

const Scene4060DialogueSeedRecord kScene4060DialogueSeedRecords[] = {
	{ 0, 0, 0, 1, 2, 2, 1 },
	{ 1, 0, 1, 1, 3, 3, 1 },
	{ 2, 0, 3, 1, 4, 4, 5 },
	{ 3, 1, 2, 1, 5, 5, 1 },
	{ 4, 0, 0, 3, 6, 6, 1 },
	{ 5, 1, 0, 0, 7, 7, 0 },
	{ 70, 0, 0, 1, 8, 8, 1 },
	{ 71, 1, 0, 3, 9, 9, 1 },
	{ 72, 1, 0, 3, 10, 10, 1 },
	{ 73, 1, 0, 2, 11, 11, 0 },
	{ 77, 1, 1, 1, 12, 12, 1 },
	{ 78, 1, 1, 3, 13, 13, 1 },
	{ 79, 1, 1, 3, 14, 14, 1 },
	{ 80, 1, 0, 2, 11, 11, 0 },
	{ 84, 1, 2, 3, 15, 15, 1 },
	{ 85, 1, 2, 3, 16, 16, 1 },
	{ 86, 1, 0, 2, 11, 11, 1 },
	{ 91, 1, 0, 0, 25, 0xff, 10 },
	{ 140, 1, 0, 3, 17, 17, 1 },
	{ 141, 1, 0, 1, 18, 18, 1 },
	{ 142, 1, 0, 4, 19, 19, 0 },
	{ 147, 1, 1, 3, 17, 20, 1 },
	{ 148, 1, 0, 0, 20, 0, 1 },
	{ 149, 1, 0, 4, 21, 19, 0 },
	{ 210, 0, 0, 0, 22, 21, 10 },
	{ 211, 1, 0, 5, 23, 22, 0 },
	{ 212, 1, 0, 5, 24, 23, 0 }
};

PlayableSceneConfig scene4060Config() {
	PlayableSceneConfig config(4060,
		SceneResourceLayout(5, 5, 20),
		SceneViewport(0, 0, 0x0088),
		SceneActorPose(kScene4060EntryRonWorldX, kScene4060EntryRonWorldY, kScene4060EntryRonFacing));
	config.setActorResources(kScene4060ActorBankTableEntry, kScene4060ActorPaletteTableEntry);
	config.setTextResources(kScene4060Resource003RowsOffsetIndex, kScene4060SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

void setDialogueRecord(Common::Array<DialogueChoiceRecord> &records,
		const Scene4060DialogueSeedRecord &seed) {
	if (seed.index >= records.size())
		return;

	DialogueChoiceRecord &record = records[seed.index];
	record.enabled = seed.enabled;
	record.nextNodeIndex = seed.nextNodeIndex;
	record.transitionMode = seed.transitionMode;
	record.playerTextRowId = seed.playerTextRowId;
	record.responseFrameIndex = seed.responseFrameIndex;
	record.disableAfterUse = seed.disableAfterUse;
}

Scene4060::Scene4060(HollywoodEngine *vm) :
		PlayableScene(vm, scene4060Config()),
		_foregroundChannel(),
		_foregroundScrollStep(0),
		_foregroundLongAnimationActive(false),
		_sherilynSpeechPoseMode(kScene4060SherilynSpeechPoseNone),
		_ambientEffectTimerAccumulator(0),
		_previousRandomAmbientCue(0),
		_pokerMidPatchVisible(false),
		_sherilynDialogueActive(false),
		_exitFrameVisible(false) {
	_sceneLayers.configureLayer(kScene4060ForegroundLayer,
		kSceneAnimationScenePlaced, kScene4060ForegroundState0Chunk,
		kScene4060ForegroundState0DescriptorCount,
		kScene4060ForegroundState0FrameMap,
		ARRAYSIZE(kScene4060ForegroundState0FrameMap));
}

void Scene4060::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetForegroundLayer();
	_ambientEffectTimerAccumulator = 0;
	_previousRandomAmbientCue = 0;
	_exitFrameVisible = false;

	if (_vm->gameState().mainFlowStateId == kScene4060ReturnState) {
		_activeActorWorldX = kScene4060ReturnRonWorldX;
		_activeActorWorldY = kScene4060ReturnRonWorldY;
		_activeActorFacing = kScene4060ReturnRonFacing;
	} else {
		_activeActorWorldX = kScene4060EntryRonWorldX;
		_activeActorWorldY = kScene4060EntryRonWorldY;
		_activeActorFacing = kScene4060EntryRonFacing;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene4060::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	configureForegroundLayerForState();
	if (_exitFrameVisible) {
		drawForegroundTableLayer();
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[kScene4060ExitOverlayChunk], 0,
			kScene4060ExitOverlayDescriptorCount, kScene4060ExitOverlayDescriptorCount - 1,
			_sceneFramebuffer);
		return;
	}

	if (_sceneLayers.layerVisible(kScene4060PokerTableTransitionLayer) ||
			_sceneLayers.layerVisible(kScene4060PokerOverlayTransitionLayer)) {
		if (_pokerMidPatchVisible && _sceneChunkTable.isValidChunk(kScene4060ForegroundMidPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4060ForegroundMidPatchChunk],
				_sceneFramebuffer);
		drawSceneLayer(kScene4060PokerTableTransitionLayer);
		drawSceneLayer(kScene4060PokerOverlayTransitionLayer);
		return;
	}

	if (_actionOverlayPlayer.replacesActor()) {
		drawForegroundTableLayer();
		drawActionOverlayLayer();
		return;
	}

	if (activeWorldY < 0x00f9) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		if (_sceneChunkTable.isValidChunk(10))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _sceneFramebuffer);
		drawForegroundTableLayer();
	} else {
		drawForegroundTableLayer();
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	drawSceneForegroundBlocks(activeWorldY);
}

void Scene4060::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene4060ReturnState) {
		runReturnEntrySequence();
		return;
	}

	runFirstEntrySequence();
}

void Scene4060::runExitSideEffectsAfterLoop() {
	if (!didLeaveSceneAfterLoop())
		return;

	fadePaletteToBlack();
	stopAmbientSoundCues();
}

void Scene4060::prepareCustomGameplayLoop() {
	resetForegroundLayer();
}

void Scene4060::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientSounds(delta);
	advanceForegroundLayer(delta);
}

bool Scene4060::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Mirar puerta (look at door): row 1, closed-door response.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar/abrir puerta (use/open door): door transition to the next room.
		runExitToNextRoom();
		return true;
	case 303: // Mirar ventana (look at window): row 2, Ron's window-entry stunt line.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar ventana (use window): row 3, too dangerous to try again.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Coger cuadro (take picture): first card trick overlay, adds item 0x40.
		runFirstCardStage();
		return true;
	case 306: // Mirar cuadro (look at picture): row 5, state-aware picture/card-trick response.
		if (state.scene4060PictureCardStage == kScene4060CardStateMirrorInstalled)
			beginSecondarySpeechLine(5, state.scene4060SherilynSheetWon == 0 ? 1 : 2);
		else
			beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar alcayata (look at hook): row 6, bent nail.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar cama (look at bed): row 7, four-poster bed line.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Usar cama (use bed): row 8, no time to rest.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Coger frasco (take bottle): second card trick overlay, adds item 0x41.
		runSecondCardStage();
		return true;
	case 311: // Mirar frasco (look at bottle): row 9, first call marks perfume/card prompt seen.
		if (state.scene4060PerfumeBottleCardStage == 0) {
			beginSecondarySpeechLine(9, 0);
			state.scene4060PerfumeBottleCardStage = kScene4060SecondCardStatePrompted;
			applySceneStateToHotspotsAndPatches(1);
		} else {
			beginSecondarySpeechLine(9, 1);
		}
		return true;
	case 312: // Mirar cuadro (look at second picture): row 10, Luis Colas self-portrait.
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Hablar con osito (talk to teddy bear): setup, teddy plea, then Ron turns and reacts.
		beginSharedInventorySpeechLine(0x0f, 0);
		beginTeddyBearSpeechLine(11, 0);
		walkActiveActorTo(_activeActorWorldX, _activeActorWorldY, 3, 0);
		beginSecondarySpeechLine(11, 1);
		return true;
	case 314: // Coger/usar osito (take/use teddy bear): row 12, unsafe with Sherilyn present.
		beginSecondarySpeechLine(12, 0);
		return true;
	case 315: // Mirar osito (look at teddy bear): row 13, soft/delicate response and refusal.
		beginSecondarySpeechLine(13, 0);
		return true;
	case 316: // Hablar con vampiresa/Sherilyn (talk to vampiress/Sherilyn): opens the Sherilyn card dialogue.
		runSherilynCardDialogue();
		return true;
	case 317: // Mirar vampiresa/Sherilyn (look at vampiress/Sherilyn): row 14, state-aware clothing response.
		beginSecondarySpeechLine(14, state.scene4060SherilynSheetWon == 0 ? 0 : 1);
		return true;
	case 318: // Usar cuadro/item 0x40 con alcayata (use picture with hook): row 16, refuses to rehang it.
		beginSecondarySpeechLine(16, 0);
		return true;
	case 319: // Usar espejo/item 0x55 con alcayata (use/install mirror on hook): restore the picture and remove the mirror.
		runInstallMirrorStage();
		return true;
	case 320: // Dar baraja de cartas/item 0x48 a Sherilyn (give deck of cards to Sherilyn): row 18 exchange.
		beginSecondarySpeechLine(18, 0);
		beginSherilynSpeechLine(18, 1);
		return true;
	default:
		return false;
	}
}

bool Scene4060::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, kScene4060MinWalkX, kScene4060MaxWalkX);
	targetY = CLIP<int>(targetY, 0, kScene4060MaxWalkY);

	if (targetY < kScene4060MaxWalkY)
		++targetY;
	while (targetY < kScene4060MaxWalkY && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;
	return true;
}

bool Scene4060::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	if (selector == 2) {
		configureForegroundLayerForState();
		return true;
	}

	if (selector == 4) {
		if (_vm->gameState().scene4060SherilynDialogueIntroSeen) {
			copySmallTextRow(9, 12);
			_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
			_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene4060PictureVerbRecordIndex,
				_vm->gameState().scene4060PictureCardStage == kScene4060CardStateBase ? 1 : 0);
		}
		return true;
	}

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	const byte cardState = MIN<byte>(state.scene4060PictureCardStage, kScene4060CardStateMirrorInstalled);
	if (cardState == kScene4060CardStateFirstWon && _sceneChunkTable.isValidChunk(kScene4060FirstCardPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4060FirstCardPatchChunk], _baseFramebuffer);
	else if (cardState == kScene4060CardStateMirrorInstalled && _sceneChunkTable.isValidChunk(kScene4060MirrorInstalledPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4060MirrorInstalledPatchChunk], _baseFramebuffer);
	applyCardPatchStateColorMap(cardState);

	if (cardState == kScene4060CardStateMirrorInstalled)
		copySmallTextRow(3, 10);
	if (state.scene4060PerfumeBottleCardStage == kScene4060SecondCardStatePrompted)
		copySmallTextRow(6, 11);
	if (state.scene4060PerfumeBottleCardStage == kScene4060SecondCardStateWon) {
		if (_sceneChunkTable.isValidChunk(kScene4060SecondCardPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4060SecondCardPatchChunk], _baseFramebuffer);
		replaceColorMapItem(6, 0);
	}
	if (state.scene4060SherilynDialogueIntroSeen)
		copySmallTextRow(9, 12);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene4060PictureVerbRecordIndex,
		cardState == kScene4060CardStateBase ? 1 : 0);
	configureForegroundLayerForState();
	return true;
}

AmbientAudioProfile Scene4060::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4060AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

byte Scene4060::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene4060TeddyBearAnimationGroup)
		return 0;
	return sherilynSpeechBaseStep();
}

byte Scene4060::primarySpeechAnimationFrameCount(byte animationGroup) const {
	return animationGroup == kScene4060TeddyBearAnimationGroup ? 1 : 5;
}

byte Scene4060::primarySpeechVolumePercent(byte animationGroup) const {
	return animationGroup == kScene4060TeddyBearAnimationGroup ?
		kScene4060TeddyBearSpeechVolumePercent : 100;
}

void Scene4060::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene4060TeddyBearAnimationGroup)
		return;
	if (_sherilynSpeechPoseMode == kScene4060SherilynSpeechPoseOpened) {
		if (frameIndex < kScene4060SherilynOpenedSpeechFirstStep)
			frameIndex = kScene4060SherilynOpenedSpeechFirstStep;
		else if (frameIndex > kScene4060SherilynOpenedSpeechLastStep)
			frameIndex = kScene4060SherilynOpenedSpeechLastStep;
	} else {
		if (frameIndex < kScene4060SherilynDirectSpeechFirstStep)
			frameIndex = kScene4060SherilynDirectSpeechFirstStep;
		else if (frameIndex > kScene4060SherilynDirectSpeechLastStep)
			frameIndex = kScene4060SherilynDirectSpeechLastStep;
	}

	setForegroundScrollStep(frameIndex);
}

void Scene4060::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)baseFrame;
	if (animationGroup == kScene4060TeddyBearAnimationGroup)
		return;
	closeSherilynSpeechPose();
}

void Scene4060::resetForegroundLayer() {
	_foregroundChannel.reset(0, kScene4060FrameMillis);
	_foregroundScrollStep = 0;
	_foregroundLongAnimationActive = false;
	clearPokerTransitionLayers();
	_sherilynSpeechPoseMode = kScene4060SherilynSpeechPoseNone;
	_sherilynDialogueActive = false;
	configureForegroundLayerForState();
}

void Scene4060::configureForegroundLayerForState() {
	const bool foregroundState = _vm->gameState().scene4060SherilynSheetWon != 0;
	_sceneLayers.setLayerResource(kScene4060ForegroundLayer,
		foregroundState ? kScene4060ForegroundState1Chunk : kScene4060ForegroundState0Chunk,
		foregroundState ? kScene4060ForegroundState1DescriptorCount : kScene4060ForegroundState0DescriptorCount,
		foregroundState ? kScene4060ForegroundState1FrameMap : kScene4060ForegroundState0FrameMap,
		foregroundState ? ARRAYSIZE(kScene4060ForegroundState1FrameMap) : ARRAYSIZE(kScene4060ForegroundState0FrameMap));
	_sceneLayers.setLayerVisible(kScene4060ForegroundLayer, true);
	setForegroundScrollStep(_foregroundScrollStep);
}

void Scene4060::setForegroundScrollStep(byte step) {
	const uint frameMapSize = _vm->gameState().scene4060SherilynSheetWon != 0 ?
		ARRAYSIZE(kScene4060ForegroundState1FrameMap) : ARRAYSIZE(kScene4060ForegroundState0FrameMap);
	if (step >= frameMapSize)
		step = 0;
	_foregroundScrollStep = step;
	_foregroundChannel.frameIndex = step;
	_sceneLayers.setLayerFrame(kScene4060ForegroundLayer, step);
}

void Scene4060::advanceForegroundLayer(uint32 delta) {
	if (_sceneLayers.layerVisible(kScene4060PokerTableTransitionLayer) ||
			_sceneLayers.layerVisible(kScene4060PokerOverlayTransitionLayer))
		return;

	if (_sherilynSpeechPoseMode != kScene4060SherilynSpeechPoseNone ||
			(_primaryDialogueSpeechActive && _primaryDialogueSpeechGroup != kScene4060TeddyBearAnimationGroup))
		return;

	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (!_foregroundLongAnimationActive) {
			if (_foregroundScrollStep != kScene4060ForegroundIdleStep) {
				setForegroundScrollStep(kScene4060ForegroundIdleStep);
			} else if (_random.getRandomNumber(14) == 0) {
				setForegroundScrollStep(1);
			} else if (!_sherilynDialogueActive && _random.getRandomNumber(49) == 0) {
				_foregroundLongAnimationActive = true;
				setForegroundScrollStep(kScene4060ForegroundLongStartStep);
			}
			continue;
		}

		const byte longEndStep = _vm->gameState().scene4060SherilynSheetWon == 0 ?
			kScene4060ForegroundState0LongEndStep : kScene4060ForegroundState1LongEndStep;
		if (_foregroundScrollStep < longEndStep) {
			setForegroundScrollStep((byte)(_foregroundScrollStep + 1));
		} else {
			_foregroundLongAnimationActive = false;
			setForegroundScrollStep(kScene4060ForegroundIdleStep);
		}
	}
}

void Scene4060::drawForegroundTableLayer() {
	drawSceneLayer(kScene4060ForegroundLayer);
}

void Scene4060::drawSceneForegroundBlocks(int activeWorldY) {
	(void)activeWorldY;
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
}

void Scene4060::runFirstEntrySequence() {
	const bool spanishDemo = _vm->isDemo() && _vm->getLanguage() == Common::ES_ESP;
	setActiveActorPose(kScene4060EntryRonWorldX, kScene4060EntryRonWorldY, kScene4060EntryRonFacing);
	const bool previousHideActiveActor = _actionOverlayPlayer.beginActorReplacement(
		kScene4060EntryOverlayChunk, kScene4060EntryOverlayDescriptorCount,
		spanishDemo ? kScene4060DemoEntryFrameMap : nullptr,
		spanishDemo ? ARRAYSIZE(kScene4060DemoEntryFrameMap) : 0);
	_actionOverlayPlayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();

	BlockingSequence sequence(*this);
	sequence.fadeFromBlack();
	if (spanishDemo) {
		sequence.presentedLayerFrames(_actionOverlayPlayer._layer,
			AnimationFrameRange(ARRAYSIZE(kScene4060DemoEntryFrameMap),
				kScene4060FrameMillis).soundAt(1, 0x2f).noFinalFrameDelay());
	} else {
		sequence.sound(0x31)
			.presentedLayerFrames(_actionOverlayPlayer._layer,
				AnimationFrameRange(kScene4060EntryOverlayDescriptorCount,
					kScene4060FrameMillis).noFinalFrameDelay());
	}
	_actionOverlayPlayer.finish(previousHideActiveActor);
	if (!sequence.completed())
		return;

	if (!_vm->gameState().scene4060EntryLineSeen) {
		if (spanishDemo) {
			sequence.actorPath(SceneActorPose(0x00c2, 0x0133, 1));
			if (!sequence.completed())
				return;
			beginSherilynSpeechLine(kScene4060SherilynResponseRow, 1);
			sequence.actorPath(SceneActorPose(0x00c2, 0x0133, 3));
		}
		sequence.secondarySpeech(0, 0)
			.commit(_vm->gameState().scene4060EntryLineSeen, true);
	}
}

void Scene4060::runReturnEntrySequence() {
	setActiveActorPose(kScene4060ReturnRonWorldX, kScene4060ReturnRonWorldY, kScene4060ReturnRonFacing);
	_soundBank0.playSample(5, 100);
	drawPlayableComposite();
	presentFrame();
	fadePaletteFromBlack();
}

void Scene4060::runExitToNextRoom() {
	drawPlayableComposite();
	presentFrame();
	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(kScene4060ExitOverlayChunk,
			kScene4060ExitOverlayDescriptorCount, kScene4060FrameMillis)
			.holdFrame(1).startAt(1)
			.noFinalFrameDelay());
	_exitFrameVisible = true;
	sequence.sound(3)
		.commit(_vm->gameState().mainFlowStateId, kScene4100EntryFromScene4060State);
}

void Scene4060::runFirstCardStage() {
	GameplayState &state = _vm->gameState();
	if (state.scene4060PictureCardStage == kScene4060CardStateMirrorInstalled) {
		beginSecondarySpeechLine(4, state.scene4060SherilynSheetWon == 0 ? 0 : 1);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(kScene4060FirstCardOverlayChunk,
			kScene4060FirstCardOverlayDescriptorCount, kScene4060FrameMillis)
			.bookendWithLastFrame()
			.resourcePatchAt(6, kScene4060FirstCardPatchChunk)
			.noFinalFrameDelay())
		.commit(state.scene4060PictureCardStage, kScene4060CardStateFirstWon)
		.framebufferPatch(0);
	addInventoryItem(kScene4060FirstWonCardItem);
	sequence.sound(1);
	if (sequence.completed())
		beginSharedInventorySpeechLine(0x14, randomSharedInventorySpeechFrame(4));
}

void Scene4060::runSecondCardStage() {
	GameplayState &state = _vm->gameState();
	if (state.scene4060PerfumeBottleCardStage == 0)
		beginSecondarySpeechLine(9, 0);

	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(kScene4060SecondCardOverlayChunk,
			kScene4060SecondCardOverlayDescriptorCount, kScene4060FrameMillis)
			.holdFirstFrame()
			.resourcePatchAt(5, kScene4060SecondCardPatchChunk)
			.noFinalFrameDelay())
		.commit(state.scene4060PerfumeBottleCardStage, kScene4060SecondCardStateWon)
		.framebufferPatch(1);
	addInventoryItem(kScene4060SecondWonCardItem);
	sequence.sound(1);
	if (sequence.completed())
		beginSharedInventorySpeechLine(0x14, randomSharedInventorySpeechFrame(4));
}

void Scene4060::runInstallMirrorStage() {
	GameplayState &state = _vm->gameState();
	if (state.scene4060SherilynPokerProgressCounter == 0) {
		beginSecondarySpeechLine(0x11, 0);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.secondarySpeech(0x11, 1)
		.actorReplacement(ActionOverlaySpec(kScene4060FirstCardOverlayChunk,
			kScene4060FirstCardOverlayDescriptorCount, kScene4060FrameMillis)
			.bookendWithLastFrame().reverse()
			.resourcePatchAt(7, kScene4060MirrorInstalledPatchChunk)
			.noFinalFrameDelay())
		.commit(state.scene4060PictureCardStage, kScene4060CardStateMirrorInstalled)
		.framebufferPatch(0);
	removeInventoryItem(kScene4060MirrorItem);
	sequence.sound(1)
		.secondarySpeech(0x11, 2);
}

void Scene4060::runSherilynCardDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeSherilynCardDialogueRecords(records);
	_sherilynDialogueActive = true;

	GameplayState &state = _vm->gameState();
	if (!state.scene4060SherilynDialogueIntroSeen) {
		beginSecondarySpeechLine(kScene4060DialogueStageId, 0);
		beginSherilynSpeechLine(kScene4060SherilynResponseRow, 0);
		state.scene4060SherilynDialogueIntroSeen = true;
		applySceneStateToHotspotsAndPatches(4);
	} else {
		beginSecondarySpeechLine(kScene4060DialogueStageId, 1);
		beginSherilynSpeechLine(kScene4060SherilynResponseRow, 1);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene4060DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene4060DialogueStageId, 7);
			beginSherilynSpeechLine(kScene4060SherilynResponseRow, 7);
			_sherilynDialogueActive = false;
			return;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			_sherilynDialogueActive = false;
			return;
		}

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene4060DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			beginSherilynSpeechLine(kScene4060SherilynResponseRow, record.responseFrameIndex);

		if (record.disableAfterUse != 0)
			record.enabled = 0;
		if (record.disableAfterUse == 5)
			runSherilynDialogueProgressReplay();
		if (record.disableAfterUse == 10)
			runSherilynDialogueTransition();

		switch (record.transitionMode) {
		case 0:
			_sherilynDialogueActive = false;
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
		case 3:
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 1 ? (byte)(depthIndex - 2) : 0;
			break;
		case 5:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 2 ? (byte)(depthIndex - 3) : 0;
			break;
		default:
			_sherilynDialogueActive = false;
			return;
		}
	}
	_sherilynDialogueActive = false;
}

void Scene4060::initializeSherilynCardDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.resize(10 * 10 * 7);
	for (uint i = 0; i < ARRAYSIZE(kScene4060DialogueSeedRecords); ++i)
		setDialogueRecord(records, kScene4060DialogueSeedRecords[i]);

	const GameplayState &state = _vm->gameState();
	if (state.scene4060SherilynSheetWon == 0) {
		records[0].enabled = 1;
		if (hasInventoryItem(0x48))
			records[210].enabled = 1;
		if (state.scene4060SherilynPokerProgressCounter == 0)
			records[70].enabled = 1;
		else
			records[2].enabled = 1;
	} else {
		records[1].enabled = 1;
	}

	if (state.scene4080GwendolynStateTransition != 0)
		records[4].enabled = 1;
}

byte Scene4060::redirectDialogueMenuChoice(byte depthIndex, byte nodeIndex, byte choiceIndex) const {
	if (_vm->gameState().scene4060SherilynSheetWon != 0 && depthIndex == 2 && nodeIndex == 1 && choiceIndex == 1)
		return 2;

	return choiceIndex;
}

void Scene4060::runSherilynDialogueProgressReplay() {
	const byte progress = MIN<byte>(_vm->gameState().scene4060SherilynPokerProgressCounter,
		kScene4060MaxDialogueProgressCounter);
	for (byte frame = 0; frame < progress && !Engine::shouldQuit(); ++frame)
		beginSherilynSpeechLine(0x61, frame, false);
}

void Scene4060::runSherilynDialogueTransition() {
	GameplayState &state = _vm->gameState();
	if (state.scene4060SherilynPokerProgressCounter < kScene4060MaxDialogueProgressCounter)
		++state.scene4060SherilynPokerProgressCounter;

	const bool finalRewardBranch = state.scene4060PictureCardStage > kScene4060CardStateFirstWon;
	runSherilynPokerTransitionAnimation(finalRewardBranch);

	if (finalRewardBranch) {
		addInventoryItem(kScene4060DialogueRewardItem);
		_soundBank0.playSample(1, 100);
		state.scene4060SherilynSheetWon = 1;
		applySceneStateToHotspotsAndPatches(2);
		beginSherilynSpeechLine(0x0f, 3);
		beginSecondarySpeechLine(0x0f, 4);
		return;
	}

	beginSherilynSpeechLine(0x0f, 0);
	beginSecondarySpeechLine(0x0f, 1);
	beginSherilynSpeechLine(0x0f, 2);
}

void Scene4060::runSherilynPokerTransitionAnimation(bool finalRewardBranch) {
	_pokerMidPatchVisible = false;
	bool interrupted = false;
	for (uint i = 0; i < ARRAYSIZE(kScene4060PokerOpenTableFrames) && !Engine::shouldQuit() && !interrupted; ++i) {
		if (presentPokerTransitionFrame(kScene4060PokerOpenTableFrames[i],
				kScene4060PokerOverlayChunk, kScene4060PokerOverlayDescriptorCount,
				(byte)i))
			interrupted = true;
	}

	bool animateOverlay = true;
	for (byte remainingCycles = 10; remainingCycles != 0 && !Engine::shouldQuit() && !interrupted;
			--remainingCycles) {
		for (uint i = 0; i < ARRAYSIZE(kScene4060PokerHandOverlayFrames) && !Engine::shouldQuit() && !interrupted;
				++i) {
			const byte tableFrame = animateOverlay ?
				kScene4060PokerRestTableFrame : kScene4060PokerHandTableFrames[i];
			const byte overlayFrame = animateOverlay ?
				kScene4060PokerHandOverlayFrames[i] : kScene4060PokerRestOverlayFrame;
			if (presentPokerTransitionFrame(tableFrame,
					kScene4060PokerOverlayChunk, kScene4060PokerOverlayDescriptorCount,
					overlayFrame))
				interrupted = true;
		}

		if (remainingCycles == 4)
			_pokerMidPatchVisible = true;
		if (!interrupted && presentPokerTransitionFrame(kScene4060PokerRestTableFrame,
				kScene4060PokerOverlayChunk, kScene4060PokerOverlayDescriptorCount,
				kScene4060PokerRestOverlayFrame))
			interrupted = true;
		animateOverlay = !animateOverlay;
	}
	_pokerMidPatchVisible = false;

	if (finalRewardBranch && !interrupted) {
		for (uint i = 0; i < ARRAYSIZE(kScene4060PokerRewardOverlayFrames) && !Engine::shouldQuit() && !interrupted;
				++i) {
			const bool firstOverlayChunk = i < 13;
			if (presentPokerTransitionFrame(kScene4060PokerRewardTableFrames[i],
					firstOverlayChunk ? kScene4060PokerOverlayChunk : kScene4060PokerRewardOverlayChunk,
					firstOverlayChunk ? kScene4060PokerOverlayDescriptorCount :
						kScene4060PokerRewardOverlayDescriptorCount,
					kScene4060PokerRewardOverlayFrames[i],
					i + 1 != ARRAYSIZE(kScene4060PokerRewardOverlayFrames)))
				interrupted = true;
		}
	} else if (!interrupted) {
		for (uint i = 0; i < ARRAYSIZE(kScene4060PokerShortCloseOverlayFrames) && !Engine::shouldQuit() && !interrupted;
				++i) {
			if (presentPokerTransitionFrame(kScene4060PokerShortCloseTableFrames[i],
					kScene4060PokerOverlayChunk, kScene4060PokerOverlayDescriptorCount,
					kScene4060PokerShortCloseOverlayFrames[i],
					i + 1 != ARRAYSIZE(kScene4060PokerShortCloseOverlayFrames)))
				interrupted = true;
		}
	}

	clearPokerTransitionLayers();
}

bool Scene4060::presentPokerTransitionFrame(byte tableFrame, uint overlayChunk, uint overlayDescriptorCount,
		byte overlayFrame, bool waitAfterFrame) {
	_sceneLayers.configureLayer(kScene4060PokerTableTransitionLayer,
		kSceneAnimationScenePlaced,
		kScene4060PokerTableTransitionChunk, kScene4060PokerTableTransitionDescriptorCount, nullptr, 0);
	_sceneLayers.setVisibleLayerFrame(kScene4060PokerTableTransitionLayer, tableFrame);
	_sceneLayers.configureLayer(kScene4060PokerOverlayTransitionLayer,
		kSceneAnimationScenePlaced, overlayChunk,
		(uint16)overlayDescriptorCount, nullptr, 0);
	_sceneLayers.setVisibleLayerFrame(kScene4060PokerOverlayTransitionLayer, overlayFrame);

	drawPlayableComposite();
	presentFrame();
	return waitAfterFrame ? waitSceneMillis(kScene4060FrameMillis) : animationPlaybackShouldStop();
}

void Scene4060::clearPokerTransitionLayers() {
	_pokerMidPatchVisible = false;
	clearSceneLayer(kScene4060PokerTableTransitionLayer);
	clearSceneLayer(kScene4060PokerOverlayTransitionLayer);
}

void Scene4060::beginSherilynSpeechLine(uint16 rowIndex, byte frameIndex, bool allowAlternatePose) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;
	if (getResource003LargeTextRecord(textRecordId).empty())
		return;

	openSherilynSpeechPose(allowAlternatePose);
	setPaletteEntry6Bit(kScene4060PrimarySpeechTextColor, kScene4060SherilynSpeechRed,
		kScene4060SherilynSpeechGreen, kScene4060SherilynSpeechBlue);
	runSpeechCue(_primarySpeechOverlay, textRecordId, continuationCount, voiceSampleId,
		kScene4060SherilynSpeechCenterX,
		kScene4060SherilynSpeechTopY, kScene4060PrimarySpeechTextColor,
		true, false, true, kScene4060SherilynAnimationGroup);
}

void Scene4060::beginTeddyBearSpeechLine(uint16 rowIndex, byte frameIndex) {
	setPaletteEntry6Bit(kScene4060PrimarySpeechTextColor, kScene4060TeddyBearSpeechRed,
		kScene4060TeddyBearSpeechGreen, kScene4060TeddyBearSpeechBlue);
	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, kScene4060TeddyBearSpeechCenterX,
		kScene4060TeddyBearSpeechTopY, kScene4060PrimarySpeechTextColor,
		true, false, true, kScene4060TeddyBearAnimationGroup);
}

void Scene4060::openSherilynSpeechPose(bool allowAlternatePose) {
	_foregroundLongAnimationActive = false;
	_sherilynSpeechPoseMode = allowAlternatePose && _random.getRandomBit() != 0 ?
		kScene4060SherilynSpeechPoseOpened : kScene4060SherilynSpeechPoseDirect;

	if (_sherilynSpeechPoseMode == kScene4060SherilynSpeechPoseDirect) {
		setForegroundScrollStep(kScene4060SherilynDirectSpeechFirstStep);
		drawPlayableComposite();
		presentFrame();
		return;
	}

	for (byte step = kScene4060SherilynOpenFirstStep;
			step <= kScene4060SherilynOpenLastStep && !Engine::shouldQuit() && !_vm->isSceneRestartRequested();
			++step) {
		setForegroundScrollStep(step);
		drawPlayableComposite();
		presentFrame();
		if (step < kScene4060SherilynOpenLastStep && waitSceneMillis(kScene4060FrameMillis))
			break;
	}
}

void Scene4060::closeSherilynSpeechPose() {
	if (_sherilynSpeechPoseMode == kScene4060SherilynSpeechPoseDirect) {
		setForegroundScrollStep(kScene4060ForegroundIdleStep);
		_sherilynSpeechPoseMode = kScene4060SherilynSpeechPoseNone;
		drawPlayableComposite();
		presentFrame();
		return;
	}

	if (_sherilynSpeechPoseMode == kScene4060SherilynSpeechPoseOpened) {
		for (byte step = kScene4060SherilynCloseFirstStep;
				step <= kScene4060SherilynCloseLastStep && !Engine::shouldQuit() && !_vm->isSceneRestartRequested();
				++step) {
			setForegroundScrollStep(step);
			drawPlayableComposite();
			presentFrame();
			if (step < kScene4060SherilynCloseLastStep && waitSceneMillis(kScene4060FrameMillis))
				break;
		}
	}

	_sherilynSpeechPoseMode = kScene4060SherilynSpeechPoseNone;
}

byte Scene4060::sherilynSpeechBaseStep() const {
	if (_sherilynSpeechPoseMode == kScene4060SherilynSpeechPoseOpened)
		return kScene4060SherilynOpenedSpeechFirstStep;

	return kScene4060SherilynDirectSpeechFirstStep;
}

void Scene4060::updateAmbientSounds(uint32 delta) {
	_ambientEffectTimerAccumulator += delta;
	while (_ambientEffectTimerAccumulator >= kScene4060AmbientCheckMillis) {
		_ambientEffectTimerAccumulator -= kScene4060AmbientCheckMillis;
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[1];
		if (player.isPlaying() || _random.getRandomNumber(24) != 0)
			continue;

		if (_random.getRandomNumber(9) == 0) {
			player.playSample(kScene4060SpecialAmbientCue, 50);
			continue;
		}

		byte cueId = 0;
		do {
			cueId = (byte)(kScene4060FirstRandomAmbientCue +
				_random.getRandomNumber(kScene4060RandomAmbientCueCount - 1));
		} while (cueId == _previousRandomAmbientCue);
		_previousRandomAmbientCue = cueId;
		player.playSample(cueId, 8);
	}
}

void Scene4060::applyCardPatchStateColorMap(byte cardState) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
		byte mappedItem = originalItem;
		if (cardState == kScene4060CardStateBase) {
			if (originalItem == 10)
				mappedItem = 0;
			else if (originalItem == 4)
				mappedItem = 3;
		} else if (cardState == kScene4060CardStateFirstWon) {
			if (originalItem == 3 || originalItem == 10)
				mappedItem = 0;
			else if (originalItem == 4)
				mappedItem = 4;
		} else if (cardState == kScene4060CardStateMirrorInstalled) {
			if (originalItem == 4 || originalItem == 10 || originalItem == 3)
				mappedItem = 3;
		}
		_paletteMask[kSceneColorToItemMap + i] = mappedItem;
	}
}

void Scene4060::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene4060::copySmallTextRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

} // End of namespace Hollywood
