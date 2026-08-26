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

#include "hollywood/scenes/playable/scene7060.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene7060ReturnState = 0x1b95;
const uint16 kScene7060ReturnToG04State = 0x1b82;
const uint16 kScene7060ExitToG07State = 0x1b9e;
const uint16 kScene7060ShortExitToG07State = 0x1b9f;
const uint16 kScene7060ViewportXOffset = 0x50;
const uint16 kScene7060ViewportMaxXOffset = 0x100;
const int kScene7060EntryFromG04StartX = 0x3b0;
const int kScene7060EntryFromG04StartY = 0x18d;
const byte kScene7060EntryFromG04Facing = 4;
const int kScene7060EntryFromG04TargetX = 0x306;
const int kScene7060EntryFromG04TargetY = 0x17a;
const int kScene7060ReturnEntryX = 0x1cb;
const int kScene7060ReturnEntryY = 0x172;
const byte kScene7060ReturnEntryFacing = 2;
const uint16 kScene7060Chunk6DescriptorCount = 0x36;
const uint16 kScene7060Chunk7DescriptorCount = 0x0e;
const uint16 kScene7060Chunk8DescriptorCount = 4;
const uint16 kScene7060Chunk9And10DescriptorCount = 10;
const uint32 kScene7060Chunk6FrameMillis = 100;
const uint32 kScene7060OverlayFrameMillis = 100;
const uint32 kScene7060KeyPickupWindowMultiplier = 2;
const byte kScene7060DialogueStageId = 0x62;
const byte kScene7060DialoguePrimaryRow = 99;
const uint16 kScene7060DialoguePrimaryCenterX = 0x0d2;
const uint16 kScene7060DialoguePrimaryTopY = 0x0a4;
const byte kScene7060DialoguePrimaryRed = 0x2c;
const byte kScene7060DialoguePrimaryGreen = 0x3f;
const byte kScene7060DialoguePrimaryBlue = 0x2c;
const byte kScene7060InvalidPrimarySpeechAnimationGroup = 0xff;
const uint kScene7060DialogueRecordCount = 10 * 10 * 7;
const byte kScene7060PickupItem11Hook = 1;
const byte kScene7060UseItem0DHook = 2;
const byte kScene7060Chunk6FrameMap[] = {
	0, 0, 51, 1, 2, 3, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 0, 18, 19, 20, 21, 52, 22, 23, 24, 25,
	26, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 38, 28, 26,
	25, 24, 25, 26, 28, 26, 25, 24, 23, 22, 28, 53, 41, 42, 43, 44,
	45, 46, 47, 48, 49, 50, 49, 0, 18, 21, 21, 21, 21, 18, 0, 0
};
const byte kScene7060Chunk7PickupItem11FrameMap[] = {
	0, 9, 10, 11, 12, 2, 3, 4, 5, 6, 7, 8, 0
};
const byte kScene7060Chunk7UseItem0DFrameMap[] = {
	0, 8, 7, 6, 5, 4, 3, 2, 1, 1, 8, 0
};
const byte kScene7060Chunk9Or10MachineFrameMap[] = {
	0, 9, 8, 7, 8, 9, 8, 7, 8, 9, 8, 7, 8, 9, 8, 7, 8, 9, 0
};
const byte kScene7060Chunk9ExitFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 9, 8, 7, 9, 8, 7
};
const byte kScene7060Chunk10SpeechFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 9, 8, 7, 9, 8, 7, 8, 9, 8,
	7, 8, 9, 8, 7, 8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};
const byte kScene7060ShortExitFrameMap[] = {
	0, 9, 8, 7
};

static PlayableSceneConfig scene7060Config() {
	PlayableSceneConfig config(7060,
		SceneResourceLayout(11, 5, 10),
		SceneViewport(kScene7060ViewportXOffset, kScene7060ViewportXOffset, kScene7060ViewportMaxXOffset),
		SceneActorPose(kScene7060EntryFromG04TargetX, kScene7060EntryFromG04TargetY, kScene7060EntryFromG04Facing));
	return config;
}

Scene7060::Scene7060(HollywoodEngine *vm) :
		PlayableScene(vm, scene7060Config()),
		_chunk8FrameIndex(3),
		_chunk6RandomIdlePaused(false),
		_colorMapItem8Promoted(false) {
	_chunk6Animation.configure(kScene7060Chunk6FrameMillis, 1, 2, 3, 0x16, 0x0e, 0x31);
	initializeChunk6FrameMap();
}

void Scene7060::initializeCustomPreviewState() {
	initializeChunk6FrameMap();
	applyChunk6FrameMapForInventoryState();
	_chunk6Animation.channel.frameMillis = kScene7060Chunk6FrameMillis;
	_chunk6Animation.reset();
	_chunk8FrameIndex = 3;
	_chunk6RandomIdlePaused = false;
	_colorMapItem8Promoted = false;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = kScene7060InvalidPrimarySpeechAnimationGroup;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_actionOverlayPlayer.reset();

	if (_vm->gameState().mainFlowStateId == kScene7060ReturnState) {
		_activeActorWorldX = kScene7060ReturnEntryX;
		_activeActorWorldY = kScene7060ReturnEntryY;
		_activeActorFacing = kScene7060ReturnEntryFacing;
	} else {
		_activeActorWorldX = kScene7060EntryFromG04TargetX;
		_activeActorWorldY = kScene7060EntryFromG04TargetY;
		_activeActorFacing = kScene7060EntryFromG04Facing;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7060::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();

	const byte chunk6Frame = _chunk6Animation.channel.frameIndex < _chunk6FrameMap.size() ?
		_chunk6FrameMap[_chunk6Animation.channel.frameIndex] : 0;
	if (_actionOverlayPlayer.replacesActor()) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[6], 0,
			kScene7060Chunk6DescriptorCount, chunk6Frame, _sceneFramebuffer);
		drawActionOverlayLayer();
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		return;
	}

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[8], 0,
		kScene7060Chunk8DescriptorCount, _chunk8FrameIndex, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[6], 0,
		kScene7060Chunk6DescriptorCount, chunk6Frame, _sceneFramebuffer);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene7060::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene7060ReturnState) {
		_soundBank0.playSample(4, 100);
		runEntryPath(kScene7060ReturnEntryX, kScene7060ReturnEntryY, kScene7060ReturnEntryFacing,
			kScene7060ReturnEntryX, kScene7060ReturnEntryY);
	} else {
		runEntryPath(kScene7060EntryFromG04StartX, kScene7060EntryFromG04StartY,
			kScene7060EntryFromG04Facing, kScene7060EntryFromG04TargetX, kScene7060EntryFromG04TargetY);
	}

	_chunk6Animation.channel.frameMillis = kScene7060Chunk6FrameMillis;
	_chunk6Animation.reset();
	_chunk8FrameIndex = 3;
	_chunk6RandomIdlePaused = false;
	drawPlayableComposite();
	presentFrame();
}

bool Scene7060::prepareCustomGameplayLoop() {
	_chunk6Animation.channel.resetTimer();
	_chunk6Animation.channel.frameMillis = kScene7060Chunk6FrameMillis;
	if (_chunk6FrameMap.empty()) {
		initializeChunk6FrameMap();
		applyChunk6FrameMapForInventoryState();
	}
	setColorMapItem8Promoted(false);
	return true;
}

bool Scene7060::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceChunk6IdleAndMachineFrame(delta);

	updateAmbientAudioAndMusicCues(delta);
	if (_chunk6Animation.state == 4 && !_soundBank0.isPlaying())
		_soundBank0.playSample(0x18, 50);
	return true;
}

bool Scene7060::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a escalera (go to stairs)
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar escalera (look at stairs)
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Hablar con gorila (talk to gorilla)
		runDialogueMenuRow98();
		return true;
	case 304: // Mirar gorila (look at gorilla)
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar puerta (look at door)
		handleSpeechRow04Variant();
		return true;
	case 306: // Mirar puerta (look at door)
		handleSpeechRow04Or06();
		return true;
	case 307: // Usar/Abrir puerta (use/open door)
		handleChunk9Or10MachineAction();
		return true;
	case 308: // Ir a escalera (go to stairs)
		_vm->gameState().mainFlowStateId = kScene7060ReturnToG04State;
		return true;
	case 309: // Mirar escalera (look at stairs)
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar adornos (look at decorations)
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Mirar cuadro (look at painting)
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Coger llave (take key)
		handleChunk7PickupItem11();
		return true;
	case 313: // Mirar llave (look at key)
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Usar llave con puerta (use key with door)
		handleChunk9ExitToG07();
		return true;
	case 315: // Usar llave con puerta (use key with door)
		handleChunk10SpeechAction();
		return true;
	case 316: // Dar vaso a gorila (give glass to gorilla)
		handleUseItem0DOnMachine();
		return true;
	default:
		return false;
	}
}

bool Scene7060::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX > 0x374)
		targetX = 0x374;

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		--targetY;
	}

	return true;
}

bool Scene7060::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	(void)restoredStepDeltas;

	if (currentRegion == 3 && nextRegion == 1) {
		requestedFacing = 4;
		return true;
	}
	if (currentRegion == 1 && nextRegion == 3) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7060::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 3 && targetRegion == 3) {
		requestedFacing = 1;
		return true;
	}
	if (currentRegion == 1 && targetRegion == 1 && state.x < targetX) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7060::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		rebuildWalkableMask();

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
				_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
			for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
				if (_paletteMaskOriginal[kSceneColorToItemMap + i] == 8)
					_paletteMask[kSceneColorToItemMap + i] = _colorMapItem8Promoted ? 8 : 2;
			}
		}

		if (_vm->gameState().spokenToBruno &&
				_stage003SmallRows.size() >= 10 * kStage003SmallRowSize) {
			memcpy(_stage003SmallRows.data() + 2 * kStage003SmallRowSize,
				_stage003SmallRows.data() + 9 * kStage003SmallRowSize, kStage003SmallRowSize);
		}

		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

byte Scene7060::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0x17;
}

void Scene7060::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_chunk6Animation.setFrame(frameIndex);
}

void Scene7060::initializeChunk6FrameMap() {
	_chunk6FrameMap.resize(ARRAYSIZE(kScene7060Chunk6FrameMap));
	memcpy(_chunk6FrameMap.data(), kScene7060Chunk6FrameMap, _chunk6FrameMap.size());
}

void Scene7060::applyChunk6FrameMapForInventoryState() {
	if (hasInventoryItem(0x11))
		applyChunk6KeyTakenFrameMap();
}

void Scene7060::applyChunk6KeyTakenFrameMap() {
	if (_chunk6FrameMap.size() <= 70)
		return;

	// Ghidra: HandleG06Chunk7PickupItem11 mutates these global frame-map bytes.
	// Reapply it on scene reload when item 0x11 is already in Sue's inventory.
	_chunk6FrameMap[33] = _chunk6FrameMap[58];
	_chunk6FrameMap[37] = _chunk6FrameMap[62];
	_chunk6FrameMap[41] = _chunk6FrameMap[66];
	_chunk6FrameMap[45] = _chunk6FrameMap[70];
}

void Scene7060::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 3)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene7060::setColorMapItem8Promoted(bool promoted) {
	if (_colorMapItem8Promoted == promoted)
		return;

	_colorMapItem8Promoted = promoted;
	applyCustomSceneStateToHotspotsAndPatches(0);
}

AmbientAudioProfile Scene7060::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(50);
}

void Scene7060::advanceChunk6IdleAndMachineFrame(uint32 delta) {
	TimedAnimationChannel &channel = _chunk6Animation.channel;
	channel.addDelta(delta);
	while (channel.consumeFrame()) {
		switch (_chunk6Animation.state) {
		case 0:
			if (!_chunk6RandomIdlePaused && _random.getRandomNumber(0x31) == 0) {
				_chunk6Animation.setStateAndFrame(2, 3);
			} else if (!_chunk6RandomIdlePaused && _random.getRandomNumber(0x0e) == 0) {
				_chunk6Animation.setStateAndFrame(1, 2);
			}
			break;
		case 1:
			_chunk6Animation.reset();
			break;
		case 2:
			if (channel.frameIndex == 0x16)
				_chunk6Animation.reset();
			else
				++channel.frameIndex;
			break;
		case 3:
			if (channel.frameIndex == 0x20) {
				_chunk6Animation.setStateAndFrame(4, 0x21);
				// Intentional usability deviation: the original key pickup window is short.
				channel.frameMillis = kScene7060OverlayFrameMillis * kScene7060KeyPickupWindowMultiplier;
				setColorMapItem8Promoted(true);
			} else {
				++channel.frameIndex;
			}
			break;
		case 4:
			if (channel.frameIndex == 0x23)
				channel.frameMillis = (_vm->gameState().labMachineSpeed * 2 + 100) *
					kScene7060KeyPickupWindowMultiplier;
			if (channel.frameIndex == 0x2c) {
				_chunk6Animation.setStateAndFrame(5, 0x2d);
				channel.frameMillis = kScene7060OverlayFrameMillis;
				setColorMapItem8Promoted(false);
			} else {
				++channel.frameIndex;
			}
			break;
		case 5:
			if (channel.frameIndex == 0x31) {
				channel.frameMillis = kScene7060OverlayFrameMillis;
			}
			if (channel.frameIndex >= 0x35 && channel.frameIndex <= 0x38)
				_chunk8FrameIndex = channel.frameIndex - 0x35;
			if (channel.frameIndex == 0x39) {
				_chunk6Animation.setStateAndFrame(6, 0x47);
			} else {
				++channel.frameIndex;
			}
			break;
		case 6:
			if (channel.frameIndex == 0x48)
				_ambientSoundBank0.playSample(0x0e, 100);
			if (channel.frameIndex == 0x49)
				_soundBank0.playSample(0x0f, 100);
			if (channel.frameIndex == 0x4d) {
				_chunk6Animation.reset();
				channel.frameMillis = kScene7060OverlayFrameMillis;
			} else {
				++channel.frameIndex;
			}
			break;
		default:
			_chunk6Animation.reset();
			break;
		}

		if (channel.frameIndex >= _chunk6FrameMap.size())
			channel.frameIndex = 1;
	}
}

void Scene7060::waitForMachineIdleBeforeDialogue() {
	while (_chunk6Animation.state > 1 && !Engine::shouldQuit()) {
		if (waitSceneMillis(kScene7060Chunk6FrameMillis))
			break;
	}
}

void Scene7060::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene7060DialogueRecordCount);

	records[0].enabled = 1;
	records[0].transitionMode = 3;
	records[0].playerTextRowId = 2;
	records[0].responseFrameIndex = 2;
	records[0].disableAfterUse = 1;
	records[0].reserved = 0xff;

	records[1].enabled = 1;
	records[1].transitionMode = 3;
	records[1].playerTextRowId = 3;
	records[1].responseFrameIndex = 3;
	records[1].disableAfterUse = 1;
	records[1].reserved = 0xff;

	records[2].enabled = 1;
	records[2].transitionMode = 3;
	records[2].playerTextRowId = 4;
	records[2].responseFrameIndex = 4;
	records[2].disableAfterUse = 1;
	records[2].reserved = 0xff;

	records[3].enabled = 1;
	records[3].transitionMode = 3;
	records[3].playerTextRowId = 5;
	records[3].responseFrameIndex = 5;
	records[3].disableAfterUse = 1;
	records[3].reserved = 0xff;

	records[4].enabled = 1;
	records[4].transitionMode = 0;
	records[4].playerTextRowId = 6;
	records[4].responseFrameIndex = 6;
	records[4].reserved = 0xff;
}

void Scene7060::runDialogueMenuRow98() {
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	GameplayState &state = _vm->gameState();
	if (!state.spokenToBruno) {
		beginSecondarySpeechLine(kScene7060DialogueStageId, 0);
		waitForMachineIdleBeforeDialogue();
		beginPrimaryDialogueSpeech(0);
		state.spokenToBruno = true;
		applySceneStateToHotspotsAndPatches(0);
	} else {
		beginSecondarySpeechLine(kScene7060DialogueStageId, 1);
		waitForMachineIdleBeforeDialogue();
		beginPrimaryDialogueSpeech(1);
	}

	while (!finished && !Engine::shouldQuit()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene7060DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene7060DialogueStageId, 6);
			beginPrimaryDialogueSpeech(6);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene7060DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff)
			beginPrimaryDialogueSpeech(record.responseFrameIndex);

		if (record.disableAfterUse == 1)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 1;
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 2;
			break;
		default:
			break;
		}
	}
}

void Scene7060::beginPrimaryDialogueSpeech(byte frameIndex) {
	beginPrimaryBrunoSpeechLine(kScene7060DialoguePrimaryRow, frameIndex);
}

void Scene7060::beginPrimaryBrunoSpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLine(rowIndex, frameIndex,
		kScene7060DialoguePrimaryCenterX, kScene7060DialoguePrimaryTopY,
		kScene7060DialoguePrimaryRed, kScene7060DialoguePrimaryGreen,
		kScene7060DialoguePrimaryBlue);
}

void Scene7060::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int soundFrame, byte soundId) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		frameMap, frameMapSize, frameMillis)
		.soundAt(soundFrame, soundId));
}

void Scene7060::handleSpeechRow04Variant() {
	beginSecondarySpeechLine(4, _vm->gameState().activatedLabExitMachine ? 1 : 0);
}

void Scene7060::handleSpeechRow04Or06() {
	if (_vm->gameState().activatedLabExitMachine)
		beginSecondarySpeechLine(6, 0);
	else
		beginSecondarySpeechLine(4, 0);
}

void Scene7060::handleChunk9Or10MachineAction() {
	if (_activeActorWorldX < 600 && _vm->gameState().activatedLabExitMachine) {
		handleShortExitToState7071();
		return;
	}

	const uint chunkIndex = _activeActorWorldX < 600 ? 9 : 10;
	runOverlaySequence(chunkIndex, kScene7060Chunk9And10DescriptorCount,
		kScene7060Chunk9Or10MachineFrameMap, ARRAYSIZE(kScene7060Chunk9Or10MachineFrameMap),
		kScene7060OverlayFrameMillis, 3, 0x10);
	beginSecondarySpeechLine(5, 0);
}

void Scene7060::handleShortExitToState7071() {
	runOverlaySequence(9, kScene7060Chunk9And10DescriptorCount,
		kScene7060ShortExitFrameMap, ARRAYSIZE(kScene7060ShortExitFrameMap),
		kScene7060OverlayFrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene7060ShortExitToG07State;
}

void Scene7060::handleChunk7PickupItem11() {
	runActorReplacement(ActionOverlaySpec(7, kScene7060Chunk7DescriptorCount,
		kScene7060Chunk7PickupItem11FrameMap, ARRAYSIZE(kScene7060Chunk7PickupItem11FrameMap), kScene7060OverlayFrameMillis)
		.hookAt(4, kScene7060PickupItem11Hook));
	addInventoryItem(0x11);
	applyChunk6KeyTakenFrameMap();
	_soundBank0.playSample(1, 100);
}

void Scene7060::handleChunk9ExitToG07() {
	if (_vm->gameState().activatedLabExitMachine) {
		beginSecondarySpeechLine(8, 0);
		return;
	}

	runOverlaySequence(9, kScene7060Chunk9And10DescriptorCount,
		kScene7060Chunk9ExitFrameMap, ARRAYSIZE(kScene7060Chunk9ExitFrameMap),
		kScene7060OverlayFrameMillis, 8, 0x11);
	_vm->gameState().activatedLabExitMachine = true;
	_vm->gameState().mainFlowStateId = kScene7060ExitToG07State;
}

void Scene7060::handleChunk10SpeechAction() {
	runOverlaySequence(10, kScene7060Chunk9And10DescriptorCount,
		kScene7060Chunk10SpeechFrameMap, ARRAYSIZE(kScene7060Chunk10SpeechFrameMap),
		kScene7060OverlayFrameMillis, 8, 0x12);
	_soundBank0.stop();
	beginSecondarySpeechLine(11, 0);
}

void Scene7060::handleUseItem0DOnMachine() {
	GameplayState &state = _vm->gameState();
	_chunk6RandomIdlePaused = true;
	if (!hasInventoryItem(0x11) && state.labMachineSpeed < 0x5c)
		state.labMachineSpeed += 8;

	beginSecondarySpeechLine(12, state.labMachineSpeed != 0x0c ? 1 : 0);
	waitForMachineIdleBeforeDialogue();
	beginPrimaryBrunoSpeechLine(0x0c, 2);
	beginSecondarySpeechLine(12, 3);

	runActorReplacement(ActionOverlaySpec(7, kScene7060Chunk7DescriptorCount,
		kScene7060Chunk7UseItem0DFrameMap, ARRAYSIZE(kScene7060Chunk7UseItem0DFrameMap), kScene7060OverlayFrameMillis)
		.hookAt(4, kScene7060UseItem0DHook));

	removeInventoryItem(0x0d);
	_soundBank0.playSample(1, 100);
	_chunk6RandomIdlePaused = false;
}

void Scene7060::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;

	if (hookId == kScene7060PickupItem11Hook) {
		applyChunk6KeyTakenFrameMap();
		setColorMapItem8Promoted(false);
	} else if (hookId == kScene7060UseItem0DHook) {
		_chunk6Animation.setStateAndFrame(3, 0x1c);
	}
}

} // End of namespace Hollywood
