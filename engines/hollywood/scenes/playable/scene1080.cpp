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

#include "hollywood/scenes/playable/scene1080.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene1080FirstState = 0x0438;
const uint16 kScene1080ExitStateBallroom = 0x042f;
const uint16 kScene1080ExitStatePantry = 0x0442;
const uint16 kScene1080ViewportXOffset = 0x00a0;
const uint16 kScene1080ViewportMinXOffset = 0x0068;
const uint16 kScene1080ViewportMaxXOffset = 0x00a8;
const uint kScene1080ActorBankTableEntry = 0x0000;
const uint kScene1080ActorPaletteTableEntry = 0x00cc;
const uint kScene1080Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1080SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1080FrameMillis = 75;
const uint32 kScene1080FrancoisIdleFrameMillis = 75;
const uint32 kScene1080FrancoisWorkFrameMillis = 60;
const uint kScene1080ForegroundDescriptorCount = 5;
const uint kScene1080FrancoisDescriptorCount = 0x13;
const uint kScene1080FrancoisActionDescriptorCount = 0x1c;
const uint kScene1080BalloonDescriptorCount = 8;
const byte kScene1080FrancoisSpeechGroup = 1;
const byte kScene1080DialogueNoResponseFrame = 0xff;
const uint kScene1080DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene1080PrimarySpeechTextColor = 0xfb;
const byte kScene1080FrancoisWorkSoundFirstCue = 0x34;
const byte kScene1080FrancoisWorkSoundCueCount = 3;
const byte kScene1080BalloonSoundCue = 0x33;
const byte kScene1080BalloonSoundHook = 1;
const byte kScene1080FirstAmbientMusicCue = 0x0b;
const byte kScene1080AmbientMusicCueCount = 5;

const byte kScene1080ForegroundFrameMap[] = { 0, 1, 2, 3, 4, 3, 2, 1, 0 };

const byte kScene1080FrancoisFrameMap[] = {
	0, 1, 2, 3, 16, 4, 17, 5, 4, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 17, 18
};

const byte kScene1080BalloonFrameMap[] = {
	0, 0, 7, 6, 5, 4, 3, 2,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 4, 5, 6, 7, 0
};

static PlayableSceneConfig scene1080Config() {
	PlayableSceneConfig config(1080,
		SceneResourceLayout(12, 5, 11),
		SceneViewport(kScene1080ViewportXOffset, kScene1080ViewportMinXOffset, kScene1080ViewportMaxXOffset),
		SceneActorPose(0x1d3, 0x15c, 2));
	config.setActorResources(kScene1080ActorBankTableEntry, kScene1080ActorPaletteTableEntry);
	config.setTextResources(kScene1080Resource003RowsOffsetIndex, kScene1080SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 6;
	return config;
}

Scene1080::Scene1080(HollywoodEngine *vm) :
		PlayableScene(vm, scene1080Config()),
		_foregroundChannel(),
		_francoisIdleChannel(),
		_francoisWorkChannel(),
		_foregroundLayer(),
		_francoisLayer(),
		_francoisActionLayer(),
		_foregroundMode(0),
		_francoisMode(0),
		_francoisActionActive(false) {
	_foregroundLayer.configure(11, kScene1080ForegroundDescriptorCount,
		kScene1080ForegroundFrameMap, ARRAYSIZE(kScene1080ForegroundFrameMap));
	_francoisLayer.configure(8, kScene1080FrancoisDescriptorCount,
		kScene1080FrancoisFrameMap, ARRAYSIZE(kScene1080FrancoisFrameMap));
	_francoisActionLayer.configure(9, kScene1080FrancoisActionDescriptorCount, nullptr, 0);
}

void Scene1080::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	if (_vm->gameState().mainFlowStateId == kScene1080FirstState) {
		_activeActorWorldX = 0x1d3;
		_activeActorWorldY = 0x15c;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x1fd;
		_activeActorWorldY = 0x14d;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene1080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_vm->gameState().scene1080FrancoisProgressState < 2 && !_francoisActionActive)
		drawResourceSpriteLayer(_francoisLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
	drawResourceSpriteLayer(_foregroundLayer);
	if (_francoisActionActive)
		drawResourceSpriteLayer(_francoisActionLayer);
}

void Scene1080::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene1080FirstState) {
		runEntryPath(0x157, 0x0b4, 2, 0x1d3, 0x15c);
		if (!state.scene1080EntryLineSeen) {
			beginSecondarySpeechLine(0, 0);
			state.scene1080EntryLineSeen = true;
		}
	} else {
		runEntryPath(0x1fd, 0x14d, 4, 0x1fd, 0x14d);
	}
	drawPlayableComposite();
	presentFrame();
}

bool Scene1080::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene1080::advanceCustomGameplayLoop(uint32 delta) {
	advanceForegroundLayer(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else if (_vm->gameState().scene1080FrancoisProgressState < 2 && !_francoisActionActive)
		advanceFrancoisLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1080::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 178: // Usar globo con gas con Francois (use the gas-filled balloon with Francois).
		handleFrancoisDistraction();
		return true;
	case 301: // Ir a escalera (go to stairs).
		_vm->gameState().mainFlowStateId = kScene1080ExitStateBallroom;
		return true;
	case 302: // Mirar escalera (look at stairs).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Ir a despensa (go to pantry).
		if (_vm->gameState().scene1080FrancoisProgressState >= 2)
			_vm->gameState().mainFlowStateId = kScene1080ExitStatePantry;
		else
			beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar despensa (look at pantry).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Hablar con Francois (talk to Francois).
		runFrancoisConversation();
		return true;
	case 306: // Mirar Francois (look at Francois).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Coger/abrir cajas (take/open boxes).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Mirar cajas (look at boxes).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Coger lata derramada (take spilled can).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar lata derramada (look at spilled can).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Coger latas (take cans).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Mirar latas (look at cans).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Mirar platos sucios (look at dirty plates).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Continuar la distraccion de Francois (run the balloon distraction sequence).
		handleFrancoisDistraction();
		return true;
	default:
		return false;
	}
}

bool Scene1080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	applyKitchenItemMap();
	rebuildKitchenWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (_vm->gameState().scene1080FrancoisProgressState >= 2) {
		ScenePoint point;
		point.x = 0x02b9;
		point.y = 0x0123;
		_hotspots.setActionTarget(2, point, point);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x11, 1);
	}
	return true;
}

bool Scene1080::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene1080::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene1080FrancoisSpeechGroup)
		return 0;
	return 0;
}

void Scene1080::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene1080FrancoisSpeechGroup)
		_francoisLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene1080::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = kScene1080FirstAmbientMusicCue;
	profile.musicCueCount = kScene1080AmbientMusicCueCount;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene1080::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId != kScene1080BalloonSoundHook)
		return;

	if (frame == 8)
		_soundBank0.playSample(kScene1080BalloonSoundCue, 30, true);
	else if (frame == 47)
		_soundBank0.stop();
}

void Scene1080::resetAnimationLayers() {
	_foregroundChannel.reset(0, kScene1080FrameMillis);
	_francoisIdleChannel.reset(0, kScene1080FrancoisIdleFrameMillis);
	_francoisWorkChannel.reset(0, kScene1080FrancoisWorkFrameMillis);
	_foregroundLayer.reset(0);
	_francoisLayer.reset(0);
	_francoisActionLayer.reset(0);
	_foregroundLayer.visible = true;
	_francoisLayer.visible = _vm->gameState().scene1080FrancoisProgressState < 2;
	_francoisActionLayer.visible = false;
	_foregroundMode = 0;
	_francoisMode = 0;
	_francoisActionActive = false;
}

void Scene1080::advanceForegroundLayer(uint32 delta) {
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_foregroundMode == 0) {
			if (_random.getRandomNumber(29) == 0) {
				_foregroundMode = 1;
				_foregroundLayer.setFrame(0);
			}
		} else if (_foregroundLayer.frameIndex + 1 < ARRAYSIZE(kScene1080ForegroundFrameMap)) {
			_foregroundLayer.setFrame(_foregroundLayer.frameIndex + 1);
		} else {
			_foregroundLayer.setFrame(0);
			_foregroundMode = 0;
		}
	}
}

void Scene1080::advanceFrancoisLayer(uint32 delta) {
	uint frameCount = _francoisIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		switch (_francoisMode) {
		case 0:
			if (_francoisLayer.frameIndex != 0x13) {
				_francoisLayer.setFrame(0x13);
			} else if (_random.getRandomNumber(14) == 0) {
				_francoisLayer.setFrame(0x14);
			} else if (_random.getRandomNumber(49) == 0) {
				_francoisLayer.setFrame(8);
				_francoisMode = 2;
			}
			break;
		case 1:
			if (_francoisLayer.frameIndex < 6)
				_francoisLayer.setFrame(_francoisLayer.frameIndex + 1);
			else {
				_francoisLayer.setFrame(0x13);
				_francoisMode = 0;
			}
			break;
		case 2:
			if (_francoisLayer.frameIndex < 8)
				_francoisLayer.setFrame(_francoisLayer.frameIndex + 1);
			else
				_francoisMode = 3;
			break;
		case 5:
			if (_francoisLayer.frameIndex == 0) {
				if (_random.getRandomNumber(14) == 0)
					_francoisLayer.setFrame(4);
			} else {
				_francoisLayer.setFrame(0);
			}
			break;
		default:
			break;
		}
	}

	frameCount = _francoisWorkChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_francoisMode != 3)
			continue;

		if (_francoisLayer.frameIndex < 0x12) {
			if (_francoisLayer.frameIndex == 9) {
				const byte cue = kScene1080FrancoisWorkSoundFirstCue +
					(byte)_random.getRandomNumber(kScene1080FrancoisWorkSoundCueCount - 1);
				_additionalAmbientSoundBank0Slots[0].playSample(cue, 15);
			}
			_francoisLayer.setFrame(_francoisLayer.frameIndex + 1);
		} else {
			_additionalAmbientSoundBank0Slots[0].stop();
			if (_random.getRandomNumber(9) == 0) {
				_francoisLayer.setFrame(6);
				_francoisMode = 1;
			} else {
				_francoisLayer.setFrame(9);
			}
		}
	}
}

void Scene1080::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x134 && activeWorldX < 0x1c7 && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (activeWorldY < 0x133 && activeWorldX > 0x23d && _sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene1080::rebuildKitchenWalkableMask() {
	const bool francoisGone = _vm->gameState().scene1080FrancoisProgressState >= 2;
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		const byte originalRegion = _paletteMaskOriginal[i];
		byte region = originalRegion;
		if (francoisGone && originalRegion == 6)
			region = 1;
		_fullPaletteRegionMask[i] = region;

		byte walkableRegion = region;
		if (walkableRegion > walkablePaletteMaxRegion() || walkableRegion == 2 || walkableRegion == 4 ||
				(!francoisGone && originalRegion == 6))
			walkableRegion = 0;
		_walkablePaletteMask[i] = walkableRegion;
	}
}

void Scene1080::applyKitchenItemMap() {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	const bool francoisGone = _vm->gameState().scene1080FrancoisProgressState >= 2;
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
		byte item = originalItem;
		if (francoisGone) {
			if (originalItem == 8)
				item = 0;
			else if (originalItem == 3 || originalItem == 2)
				item = 2;
		} else {
			if (originalItem == 8 || originalItem == 3)
				item = 3;
			else if (originalItem == 2)
				item = 0;
		}
		_paletteMask[kSceneColorToItemMap + i] = item;
	}
}

void Scene1080::runFrancoisConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeFrancoisDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	const byte frame = state.scene1080FrancoisProgressState == 0 ? 0 : 1;
	_additionalAmbientSoundBank0Slots[0].stop();
	_francoisMode = 5;
	_francoisLayer.setFrame(0);
	beginSecondarySpeechLine(0x62, frame);
	beginPrimarySpeechLineWithAnimationGroup(99, frame, 0x022e, 0x0084,
		0x0d, 0x32, 0x3a, kScene1080FrancoisSpeechGroup);
	if (state.scene1080FrancoisProgressState == 0)
		state.scene1080FrancoisProgressState = 1;

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(0x62, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(0x62, 6);
			beginPrimarySpeechLineWithAnimationGroup(99, 6, 0x022e, 0x0084,
				0x0d, 0x32, 0x3a, kScene1080FrancoisSpeechGroup);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(0x62, record.playerTextRowId);
		if (record.responseFrameIndex != kScene1080DialogueNoResponseFrame) {
			beginPrimarySpeechLineWithAnimationGroup(99, record.responseFrameIndex,
				0x022e, 0x0084, 0x0d, 0x32, 0x3a, kScene1080FrancoisSpeechGroup);
		}
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

void Scene1080::handleFrancoisDistraction() {
	GameplayState &state = _vm->gameState();
	if (state.scene1080FrancoisProgressState >= 2) {
		beginStaticSecondarySpeechLine(0x13, (byte)_random.getRandomNumber(1));
		return;
	}

	if (!walkActiveActorTo(0x0317, 0x01b3, 1, 0, false))
		return;
	runActorReplacement(ActionOverlaySpec(10, kScene1080BalloonDescriptorCount,
		kScene1080BalloonFrameMap, ARRAYSIZE(kScene1080BalloonFrameMap),
		kScene1080FrameMillis).hookEveryFrame(kScene1080BalloonSoundHook));
	_soundBank0.stop();

	if (hasInventoryItem(0x4d))
		removeInventoryItem(0x4d);
	if (!hasInventoryItem(0x1c))
		addInventoryItem(0x1c);
	_soundBank0.playSample(1, 100);
	setActiveActorPose(_activeActorWorldX, _activeActorWorldY, 5);

	_additionalAmbientSoundBank0Slots[0].stop();
	_francoisMode = 5;
	_francoisLayer.setFrame(0);
	runFrancoisActionSpeechLine(0, 0, 0x0f, 0x022e, 0x0084);
	runFrancoisActionSpeechLine(2, 0x10, 0x1b, 0x01bd, 0x0066);
	beginPrimarySpeechLine(12, 3, 0x0154, 0x0048, 0x0d, 0x32, 0x3a);
	_francoisActionActive = false;
	_francoisActionLayer.visible = false;

	state.scene1080FrancoisProgressState = 2;
	_francoisLayer.visible = false;
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(12, 1);
}

void Scene1080::runFrancoisActionSpeechLine(byte frameIndex, byte firstDescriptor,
		byte lastDescriptor, uint16 centerX, uint16 topY) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(12, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	setPaletteEntry6Bit(kScene1080PrimarySpeechTextColor, 0x0d, 0x32, 0x3a);
	_francoisActionActive = true;
	_francoisActionLayer.visible = true;
	_francoisActionLayer.setFrame(firstDescriptor);
	byte descriptor = firstDescriptor;
	uint32 frameMillis = 0;
	const byte partCount = MAX<byte>(1, continuationCount);
	bool interrupted = false;

	for (byte part = 0; part < partCount && !interrupted && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++part) {
		const Common::String text = getResource003LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		_primarySpeechOverlay.visible = true;
		_primarySpeechOverlay.colorIndex = kScene1080PrimarySpeechTextColor;
		wrapActorSpeechText(text, centerX, _primarySpeechOverlay.lines);
		calculateSpeechOverlayBounds(_primarySpeechOverlay, centerX, topY, true,
			_activeActorWorldY);

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _primarySpeechOverlay.lines.size() * 1100);
		uint32 elapsed = 0;
		while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
			const bool speechFinished = !_speech.isPlaying() && elapsed >= duration;
			if (speechFinished && (part + 1 < partCount || descriptor >= lastDescriptor))
				break;

			const uint32 slice = 10;
			if (waitSceneMillis(slice)) {
				interrupted = true;
				break;
			}
			elapsed += slice;
			frameMillis += slice;
			while (frameMillis >= kScene1080FrancoisWorkFrameMillis && descriptor < lastDescriptor) {
				frameMillis -= kScene1080FrancoisWorkFrameMillis;
				_francoisActionLayer.setFrame(++descriptor);
			}
		}

		_speech.stop();
		_primarySpeechOverlay.visible = false;
		_primarySpeechOverlay.lines.clear();
	}
}

void Scene1080::initializeFrancoisDialogueRecords(
		Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene1080DialogueChoiceRecordCount);

	setDialogueRecord(records, 0, 1, 0, 3, 2, 2, 1);
	setDialogueRecord(records, 1, 1, 0, 1, 3, 3, 1);
	setDialogueRecord(records, 2, 1, 0, 3, 4, 4, 1);
	setDialogueRecord(records, 3, 1, 1, 1, 5, 5, 1);
	setDialogueRecord(records, 4, 1, 0, 0, 6, 6, 1);

	setDialogueRecord(records, 70, 1, 0, 3, 7, 7, 1);
	setDialogueRecord(records, 71, 0, 0, 3, 8, 8, 1);
	setDialogueRecord(records, 72, 1, 0, 2, 9, 9, 0);

	setDialogueRecord(records, 77, 1, 1, 3, 10, 10, 1);
	setDialogueRecord(records, 78, 1, 1, 3, 11, 11, 1);
	setDialogueRecord(records, 79, 1, 1, 3, 12, 12, 1);
	setDialogueRecord(records, 80, 1, 1, 3, 13, 13, 1);
	setDialogueRecord(records, 81, 1, 0, 2, 9, 9, 1);
}

void Scene1080::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = enabled;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = 0xff;
}

} // End of namespace Hollywood
