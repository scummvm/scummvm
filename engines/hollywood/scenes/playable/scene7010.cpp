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

#include "hollywood/scenes/playable/scene7010.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene7010ArchiveName = "RESOURCE.G01";
const uint kScene7010InitialRequiredChunkCount = 19;
const uint kScene7010ArenaFirstChunk = 5;
const uint kScene7010ArenaLastChunk = 18;
const uint kScene7010StageIndex = 701;
const uint16 kScene7010ViewportXOffset = 0;
const uint16 kScene7010ViewportMaxXOffset = 0xd0;
const uint16 kScene7010FirstState = 0x1b62;
const uint16 kScene7010LastState = 0x1b6b;
const uint16 kScene7010ReturnState = 0x1b63;
const uint16 kScene7010ExitState7020 = 0x1b6c;
const uint16 kScene7010ExitState7030 = 0x1b76;
const uint16 kScene7010Chunk8DescriptorCount = 0x16;
const uint16 kScene7010Chunk9DescriptorCount = 2;
const uint16 kScene7010Chunk10DescriptorCount = 0x10;
const uint16 kScene7010Chunk11DescriptorCount = 0x25;
const uint16 kScene7010Chunk14DescriptorCount = 0x20;
const uint16 kScene7010Chunk15DescriptorCount = 0x17;
const uint16 kScene7010DialogueOverlayMode1DescriptorCount = 3;
const uint16 kScene7010DialogueOverlayMode2DescriptorCount = 0x1b;
const uint kScene7010DialogueOverlayLayer = 0;
const uint kScene7010Chunk14Layer = 1;
const uint kScene7010Chunk11Layer = 2;
const uint kScene7010Chunk15Layer = 0;
const byte kScene7010AmbientMusicCueWithoutChunk9 = 0x0f;
const byte kScene7010SueEntryFacing = 1;
const byte kScene7010SueEntryFinalCel = 0;
const int kScene7010SueEntryStartX = 0x16b;
const int kScene7010SueEntryStartY = 0x1df;
const int kScene7010SueEntryTargetX = 0x184;
const int kScene7010SueEntryTargetY = 0x1c6;
const int kScene7010SueReturnStartX = 0x78;
const int kScene7010SueReturnStartY = 0x14b;
const int kScene7010SueReturnTargetX = 0x186;
const int kScene7010SueReturnTargetY = 0x1c5;
const uint32 kScene7010ActorPathFrameMillis = 60;
const uint32 kScene7010SecondaryActorFrameMillis = 150;
const uint32 kScene7010Chunk8FrameMillis = 75;
const uint32 kScene7010Chunk10FrameMillis = 125;
const uint32 kScene7010DialogueOverlayFrameMillis = 60;
const byte kScene7010Chunk8FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 5, 7, 8, 9, 10, 11, 12, 13, 21,
	9, 8, 7, 0, 14, 15
};
const byte kScene7010Chunk11FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 20, 19, 18, 13, 12,
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 35, 13, 26, 27,
	28, 29, 30, 31, 32, 33, 34, 13, 36
};
const byte kScene7010Chunk14FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 30, 31
};
const byte kScene7010Chunk15FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 19, 20, 21, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 22
};
const byte kScene7010DialogueOverlayMode1FrameMap[] = { 0, 1, 2, 1 };
const byte kScene7010DialogueOverlayMode2FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
};
const byte kScene7010Route3To2StepDeltas[] = {
	4, 8, 10, 6, 3, 5, 5, 5, 5, 6, 2, 3
};

static PlayableSceneConfig scene7010Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene7010ArchiveName;
	config.initialRequiredChunkCount = kScene7010InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene7010ArenaFirstChunk;
	config.arenaLastChunk = kScene7010ArenaLastChunk;
	config.stageIndex = kScene7010StageIndex;
	config.debugName = "Scene 7010";
	config.viewportXOffset = kScene7010ViewportXOffset;
	config.viewportMaxXOffset = kScene7010ViewportMaxXOffset;
	config.loadInventoryActionTables = false;
	config.loadActorDepthTables = false;
	config.mainFlowFirstState = kScene7010FirstState;
	config.mainFlowLastState = kScene7010LastState;
	return config;
}

Scene7010::Scene7010(HollywoodEngine *vm) :
		PlayableScene(vm, scene7010Config(), "scene7010", 0x184, 0x1c6, 1, 0xfd, 0xfb),
		_chunk8FrameIndex(0),
		_chunk9AmbientOverlayFrameIndex(0),
		_chunk9AmbientDecisionCounter(0),
		_chunk10IdleFrameA(0),
		_chunk10IdleFrameB(8),
		_chunk10IdleFrameC(4),
		_chunk10IdleFrameD(0x0c),
		_dialogueOverlayFrameIndex(0),
		_dialogueOverlayMode(0),
		_chunk8SpecialSequenceActive(false),
		_chunk10IdlePairAAltPhase(false),
		_chunk10IdlePairBAltPhase(false),
		_chunk10IdlePairATicksRemaining(10),
		_chunk10IdlePairBTicksRemaining(16),
		_chunk8TimerAccumulator(0),
		_chunk10TimerAccumulator(0),
		_dialogueOverlayTimerAccumulator(0),
		_backTransientLayers(),
		_frontTransientLayers() {
}

bool Scene7010::hasCustomPreviewState() const {
	return true;
}

void Scene7010::initializeCustomPreviewState() {
	_chunk8FrameIndex = _vm->gameState().currentAmbientMusicCueId == kScene7010AmbientMusicCueWithoutChunk9 ? 0x14 : 0;
	_chunk9AmbientOverlayFrameIndex = 0;
	_chunk10IdleFrameA = 0;
	_chunk10IdleFrameB = 8;
	_chunk10IdleFrameC = 4;
	_chunk10IdleFrameD = 0x0c;
	_dialogueOverlayFrameIndex = 0;
	_dialogueOverlayMode = 0;
	_primaryLeftSpeechLastFrame = 0;
	resetTransientOverlayLayers();
	_chunk8SpecialSequenceActive = false;
	_chunk10IdlePairAAltPhase = _random.getRandomNumber(1) != 0;
	_chunk10IdlePairBAltPhase = _random.getRandomNumber(1) != 0;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_chunk10IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk10IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk9AmbientDecisionCounter = 0;
	_chunk8TimerAccumulator = 0;
	_chunk10TimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_dialogueOverlayTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_activeActorWorldX = kScene7010SueEntryTargetX;
	_activeActorWorldY = kScene7010SueEntryTargetY;
	_activeActorFacing = kScene7010SueEntryFacing;
	_activeActorCel = kScene7010SueEntryFinalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	_actionOverlayVisible = false;
	_hideActiveActor = false;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	_sceneStateFlags[1] = _vm->gameState().frankensteinNoteOverlayMode;
	_sceneStateFlags[4] = 1;
	_sceneStateFlags[5] = _vm->gameState().hannoverCourtyardDialogueState;
	_sceneStateFlags[6] = _vm->gameState().hannoverCourtyardFollowUpSeen ? 1 : 0;
	setDialogueOverlayMode(_sceneStateFlags[1], 0);
	applySceneStateToHotspotsAndPatches(0xff);
}

bool Scene7010::hasCustomComposite() const {
	return true;
}

void Scene7010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kScene7010Chunk10DescriptorCount, _chunk10IdleFrameA, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kScene7010Chunk10DescriptorCount, _chunk10IdleFrameC, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kScene7010Chunk10DescriptorCount, _chunk10IdleFrameB, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kScene7010Chunk10DescriptorCount, _chunk10IdleFrameD, _sceneFramebuffer);

	drawTransientLayers(_backTransientLayers);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	if (actorDrawOrderMode == 1) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	} else {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	}

	const byte chunk8DescriptorIndex = _chunk8FrameIndex < ARRAYSIZE(kScene7010Chunk8FrameMap) ?
		kScene7010Chunk8FrameMap[_chunk8FrameIndex] : 0;
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[8], 0,
		kScene7010Chunk8DescriptorCount, chunk8DescriptorIndex, _sceneFramebuffer);

	if (!_chunk8SpecialSequenceActive &&
			_vm->gameState().currentAmbientMusicCueId != kScene7010AmbientMusicCueWithoutChunk9) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[9], 0,
			kScene7010Chunk9DescriptorCount, _chunk9AmbientOverlayFrameIndex, _sceneFramebuffer);
	}

	drawTransientLayers(_frontTransientLayers);
}

bool Scene7010::shouldDrawSecondaryActorInPlayableComposite() const {
	return _speechOverlay.visible && !_actionOverlayVisible;
}

bool Scene7010::hasCustomEntrySequence() const {
	return true;
}

void Scene7010::runCustomEntrySequence() {
	const bool returnEntry = _vm->gameState().mainFlowStateId == kScene7010ReturnState;
	if (returnEntry) {
		runSueEntryPath(kScene7010SueReturnStartX, kScene7010SueReturnStartY,
			kScene7010SueReturnTargetX, kScene7010SueReturnTargetY);
		_sceneStateFlags[0] = 1;
		return;
	}

	runSueEntryPath(kScene7010SueEntryStartX, kScene7010SueEntryStartY,
		kScene7010SueEntryTargetX, kScene7010SueEntryTargetY);
	if (_skipRequested || Engine::shouldQuit())
		return;

	runJuniorSpeech();
	_sceneStateFlags[0] = 1;
}

void Scene7010::runSueEntryPath(int startX, int startY, int targetX, int targetY) {
	const byte cels[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 1, 2, 3, 4, kScene7010SueEntryFinalCel };
	uint32 chunk8Accumulator = 0;
	uint32 chunk10Accumulator = 0;
	uint32 lastMillis = g_system->getMillis();

	for (uint frame = 0; frame < ARRAYSIZE(cels) && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		if (pollEvents(true))
			return;

		const int x = startX + ((targetX - startX) * (int)frame) / (int)(ARRAYSIZE(cels) - 1);
		const int y = startY + ((targetY - startY) * (int)frame) / (int)(ARRAYSIZE(cels) - 1);
		_activeActorWorldX = x;
		_activeActorWorldY = y;
		_activeActorFacing = kScene7010SueEntryFacing;
		_activeActorCel = cels[frame];
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
		drawCutsceneComposite(true, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
			false, 0, 0, 0, 0, _activeActorDrawOrderMode);
		presentFrame();

		uint32 waited = 0;
		while (waited < kScene7010ActorPathFrameMillis && !_skipRequested && !Engine::shouldQuit()) {
			if (pollEvents(true))
				return;
			const uint32 slice = MIN<uint32>(10, kScene7010ActorPathFrameMillis - waited);
			g_system->delayMillis(slice);
			waited += slice;
		}

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastMillis;
		lastMillis = now;
		chunk8Accumulator += elapsed;
		chunk10Accumulator += elapsed;
		if (chunk8Accumulator >= kScene7010Chunk8FrameMillis) {
			_chunk8FrameIndex = (_chunk8FrameIndex == 7) ? 0 : _chunk8FrameIndex + 1;
			if ((_chunk8FrameIndex & 1) != 0)
				_chunk9AmbientOverlayFrameIndex ^= 1;
			chunk8Accumulator = 0;
		}
		if (chunk10Accumulator >= kScene7010Chunk10FrameMillis) {
			advanceChunk10IdleFrames();
			chunk10Accumulator = 0;
		}
	}

	_activeActorWorldX = targetX;
	_activeActorWorldY = targetY;
	_activeActorFacing = kScene7010SueEntryFacing;
	_activeActorCel = kScene7010SueEntryFinalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene7010::runJuniorSpeech() {
	clearSpeechOverlay();
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(0, 0, textRecordId, continuationCount, voiceSampleId))
		return;
	(void)continuationCount;

	if (voiceSampleId != 0)
		_speech.playSample(voiceSampleId, 100);

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return;

	wrapActorSpeechText(text, kScene7010SueEntryTargetX, _speechOverlay.lines);
	if (_speechOverlay.lines.empty())
		return;

	_speechOverlay.visible = true;
	_speechOverlay.colorIndex = 0xfd;
	calculateSecondarySpeechBounds(kScene7010SueEntryTargetX, kScene7010SueEntryTargetY);

	const uint32 speechMillis = _speech.isPlaying() ?
		MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(2800, _speechOverlay.lines.size() * 1500);
	uint32 elapsed = 0;
	uint32 frameAccumulator = kScene7010SecondaryActorFrameMillis;
	uint32 chunk8Accumulator = 0;
	uint32 chunk10Accumulator = 0;
	uint32 lastMillis = g_system->getMillis();

	while ((_speech.isPlaying() || elapsed < speechMillis) && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents(true))
			break;

		_activeActorWorldX = kScene7010SueEntryTargetX;
		_activeActorWorldY = kScene7010SueEntryTargetY;
		_activeActorFacing = kScene7010SueEntryFacing;
		_activeActorCel = kScene7010SueEntryFinalCel;
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
		drawPlayableComposite();
		presentFrame();

		g_system->delayMillis(10);
		elapsed += 10;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		chunk8Accumulator += delta;
		chunk10Accumulator += delta;
		if (frameAccumulator >= kScene7010SecondaryActorFrameMillis) {
			advanceSecondaryActorSpeechFrame();
			frameAccumulator = 0;
		}
		if (chunk8Accumulator >= kScene7010Chunk8FrameMillis) {
			_chunk8FrameIndex = (_chunk8FrameIndex == 7) ? 0 : _chunk8FrameIndex + 1;
			if ((_chunk8FrameIndex & 1) != 0)
				_chunk9AmbientOverlayFrameIndex ^= 1;
			chunk8Accumulator = 0;
		}
		if (chunk10Accumulator >= kScene7010Chunk10FrameMillis) {
			advanceChunk10IdleFrames();
			chunk10Accumulator = 0;
		}
	}

	clearSpeechOverlay();
	_speech.stop();
	_secondaryActorFrame = 0;
	drawPlayableComposite();
	presentFrame();
}

bool Scene7010::prepareCustomGameplayLoop() {
	if (_vm->gameState().mainFlowStateId == kScene7010ReturnState) {
		_activeActorWorldX = kScene7010SueReturnTargetX;
		_activeActorWorldY = kScene7010SueReturnTargetY;
	} else {
		_activeActorWorldX = kScene7010SueEntryTargetX;
		_activeActorWorldY = kScene7010SueEntryTargetY;
	}
	_activeActorFacing = kScene7010SueEntryFacing;
	_activeActorCel = kScene7010SueEntryFinalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	resetTransientOverlayLayers();
	_chunk8SpecialSequenceActive = false;
	setDialogueOverlayMode(0, 0);
	_chunk8TimerAccumulator = 0;
	_chunk10TimerAccumulator = 0;
	_dialogueOverlayTimerAccumulator = 0;
	return true;
}

bool Scene7010::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryLeftSpeechActive && _primarySpeechOverlay.visible) {
		_primaryLeftSpeechTimerAccumulator += delta;
		while (_primaryLeftSpeechTimerAccumulator >= kScene7010Chunk10FrameMillis) {
			advancePrimaryLeftSpeechFrame();
			_primaryLeftSpeechTimerAccumulator -= kScene7010Chunk10FrameMillis;
		}
	} else {
		_primaryLeftSpeechTimerAccumulator = 0;
		_chunk8TimerAccumulator += delta;
		while (_chunk8TimerAccumulator >= kScene7010Chunk8FrameMillis) {
			advanceChunk8Cycle();
			_chunk8TimerAccumulator -= kScene7010Chunk8FrameMillis;
		}
	}

	_chunk10TimerAccumulator += delta;
	while (_chunk10TimerAccumulator >= kScene7010Chunk10FrameMillis) {
		advanceChunk10IdleFrames();
		_chunk10TimerAccumulator -= kScene7010Chunk10FrameMillis;
	}

	advanceDialogueOverlay(delta);
	return true;
}

bool Scene7010::dispatchCustomSceneAction(uint16 handlerId) {
	if (handlerId <= 34 || handlerId == 36 || handlerId == 38 || handlerId == 39 || handlerId == 40)
		return dispatchGenericSceneAction(handlerId);

	switch (handlerId) {
	case 301: // Ir a escalera (go to stairs)
		handleActionSlot00TransitionToG03();
		break;
	case 302: // Mirar escalera (look at stairs)
		handleActionSlot01SecondarySpeech();
		break;
	case 303: // Mirar caseta de perro (look at doghouse)
		handleActionSlot02SecondarySpeech();
		break;
	case 304: // Ir a edificio (go to building)
		handleActionSlot03DialogueSequence();
		break;
	case 305: // Mirar edificio (look at building)
		handleActionSlot04Item06Speech();
		break;
	case 307: // Usar hueso con caseta de perro (use bone with doghouse)
		handleActionSlot06FrankensteinNoteSequence();
		break;
	case 308: // Hablar con Junior (talk to Junior)
		handleActionSlot07DialogueAndReturn();
		break;
	case 309: // Mirar Junior (look at Junior)
		handleActionSlot08CommonSpeech();
		break;
	default:
		warning("Unhandled %s action handler %u", sceneDebugName(), handlerId);
		break;
	}
	return true;
}

bool Scene7010::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x16b, 0x268);

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

bool Scene7010::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	if (currentRegion == 3 && nextRegion == 2) {
		memcpy(_actorPathStepDeltas.data() + 12, kScene7010Route3To2StepDeltas,
			ARRAYSIZE(kScene7010Route3To2StepDeltas));
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 2 && nextRegion == 1 && state.x < boundary.x && boundary.y <= state.y) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7010::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)restoredStepDeltas;
	if (currentRegion == 3 && targetRegion == 3) {
		requestedFacing = 4;
		return true;
	}
	if (currentRegion == 1 && targetRegion == 1 && state.x < targetX && targetY <= state.y) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7010::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if ((selector == 2 || selector == 0xff) && _sceneStateFlags[2] == 1) {
		const uint sourceOffset = 5 * kStage003SmallRowSize;
		const uint destinationOffset = 3 * kStage003SmallRowSize;
		if (sourceOffset + kStage003SmallRowSize <= _stage003SmallRows.size() &&
				destinationOffset + kStage003SmallRowSize <= _stage003SmallRows.size()) {
			memcpy(_stage003SmallRows.data() + destinationOffset,
				_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
			_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		}
	}

	if (selector == 4 || selector == 0xff) {
		// The original initializer sets G01 flag 4 to 1, and slot 03 only
		// promotes it to 2. The pre-walk branch for value 0 is not reachable
		// in normal gameplay, so bodegas/edificio dispatches in place.
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x19, 0);
	}
	return true;
}

bool Scene7010::shouldAnimatePrimarySpeechLine() const {
	return false;
}

void Scene7010::setPrimaryLeftSpeechFrame(byte frameIndex) {
	_chunk8FrameIndex = 0x0b + frameIndex;
}

void Scene7010::advanceChunk8Cycle() {
	if (_vm->gameState().currentAmbientMusicCueId == kScene7010AmbientMusicCueWithoutChunk9) {
		_chunk8FrameIndex = _chunk8FrameIndex == 0x1a ? 0x14 : _chunk8FrameIndex + 1;
		return;
	}

	_chunk8FrameIndex = _chunk8FrameIndex == 7 ? 0 : _chunk8FrameIndex + 1;
	if ((_chunk8FrameIndex & 1) == 0)
		return;

	if (_chunk9AmbientOverlayFrameIndex == 1) {
		_chunk9AmbientOverlayFrameIndex = 0;
	} else {
		++_chunk9AmbientDecisionCounter;
		if ((_chunk9AmbientDecisionCounter % 5) == 0)
			_chunk9AmbientOverlayFrameIndex = 1;
	}
}

void Scene7010::advanceChunk10IdleFrames() {
	if (!_chunk10IdlePairAAltPhase) {
		if (_chunk10IdlePairATicksRemaining == 0) {
			_chunk10IdlePairAAltPhase = true;
			_chunk10IdleFrameA = 0;
			_chunk10IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
		} else {
			--_chunk10IdlePairATicksRemaining;
			_chunk10IdleFrameB = 8 + (byte)_random.getRandomNumber(3);
		}
	} else if (_chunk10IdlePairATicksRemaining == 0) {
		_chunk10IdlePairAAltPhase = false;
		_chunk10IdleFrameB = 8;
		_chunk10IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	} else {
		--_chunk10IdlePairATicksRemaining;
		_chunk10IdleFrameA = (byte)_random.getRandomNumber(3);
	}

	if (!_chunk10IdlePairBAltPhase) {
		if (_chunk10IdlePairBTicksRemaining == 0) {
			_chunk10IdlePairBAltPhase = true;
			_chunk10IdleFrameC = 4;
			_chunk10IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
		} else {
			--_chunk10IdlePairBTicksRemaining;
			_chunk10IdleFrameD = 0x0c + (byte)_random.getRandomNumber(3);
		}
	} else if (_chunk10IdlePairBTicksRemaining == 0) {
		_chunk10IdlePairBAltPhase = false;
		_chunk10IdleFrameD = 0x0c;
		_chunk10IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	} else {
		--_chunk10IdlePairBTicksRemaining;
		_chunk10IdleFrameC = 4 + (byte)_random.getRandomNumber(3);
	}
}

void Scene7010::advanceDialogueOverlay(uint32 delta) {
	if (_dialogueOverlayMode == 0)
		return;

	_dialogueOverlayTimerAccumulator += delta;
	while (_dialogueOverlayTimerAccumulator >= kScene7010DialogueOverlayFrameMillis) {
		_dialogueOverlayTimerAccumulator -= kScene7010DialogueOverlayFrameMillis;
		if (_dialogueOverlayMode == 1) {
			setDialogueOverlayFrame(_dialogueOverlayFrameIndex == 3 ? 0 : _dialogueOverlayFrameIndex + 1);
		} else {
			setDialogueOverlayFrame(_dialogueOverlayFrameIndex == 0x1b ? 5 : _dialogueOverlayFrameIndex + 1);
		}
	}
}

void Scene7010::handleActionSlot00TransitionToG03() {
	_vm->gameState().mainFlowStateId = kScene7010ExitState7030;
}

void Scene7010::handleActionSlot01SecondarySpeech() {
	beginSecondarySpeechLine(1, 0);
}

void Scene7010::handleActionSlot02SecondarySpeech() {
	// The office statue sequence primes the Frankenstein-note condition;
	// this look/description slot switches to the post-note line when set.
	beginSecondarySpeechLine(2, _sceneStateFlags[1] == 0 ? 0 : 1);
}

void Scene7010::handleActionSlot03DialogueSequence() {
	GameplayState &state = _vm->gameState();
	_sceneStateFlags[5] = state.hannoverCourtyardDialogueState;
	_sceneStateFlags[6] = state.hannoverCourtyardFollowUpSeen ? 1 : 0;

	if (!hasInventoryItem(6)) {
		beginSecondarySpeechLine(3, 0);
		return;
	}

	if (_sceneStateFlags[2] == 0)
		handleActionSlot04Item06Speech();

	beginSecondarySpeechLine(_sceneStateFlags[4] == 1 ? 3 : 4, _sceneStateFlags[4] == 1 ? 1 : 2);
	walkActiveActorTo(0x298, 0x1af, 1, 0);
	setChunk11Visible(true);
	runChunk11FrameRange(0, 0x0e);
	beginPrimarySpeechLine(99, _sceneStateFlags[4] == 1 ? 0 : 2, 0x302, 0xe3, 0x28, 0x16, 0x0b);
	runChunk11FrameRange(0x12, 0x16);
	beginPrimarySpeechLine(99, _sceneStateFlags[4] == 1 ? 1 : 4, 0x2ee, 0xe8, 0x28, 0x16, 0x0b);
	runChunk11FrameRange(0x1a, 0x1e);
	if (_sceneStateFlags[5] != 0) {
		beginPrimarySpeechLine(99, 10, 0x302, 0xe3, 0x28, 0x16, 0x0b);
		runChunk14FrameRange(0, 0x18);
		if (_sceneStateFlags[6] == 0) {
			beginSecondarySpeechLine(0x62, 0x0b);
			beginPrimarySpeechLine(99, 0x0b, 0x302, 0xe3, 0x28, 0x16, 0x0b);
			_sceneStateFlags[6] = 1;
			state.hannoverCourtyardFollowUpSeen = true;
		}
		if (_sceneStateFlags[5] == 2) {
			beginPrimarySpeechLine(99, 0x0c, 0x302, 0xe3, 0x28, 0x16, 0x0b);
			runChunk14FrameRange(0x19, 0x26);
			walkActiveActorTo(0x298, 0x1af, 4, 0);
			beginSecondarySpeechLine(0x62, 0x0c);
			walkActiveActorTo(0x3b0, 0x1a9, 0xff, 0);
			_vm->gameplayMusic()->stop();
			state.mainFlowStateId = kScene7010ExitState7020;
			return;
		}
	}
	walkActiveActorTo(0x17b, 0x1b2, 0xff, 0);
	setChunk11Visible(false);
	if (_sceneStateFlags[4] == 1)
		_sceneStateFlags[4] = 2;
	else
		beginSecondarySpeechLine(5, 0);
}

void Scene7010::handleActionSlot04Item06Speech() {
	if (_sceneStateFlags[2] == 0) {
		beginSecondarySpeechLine(4, 0);
		_sceneStateFlags[2] = 1;
		applySceneStateToHotspotsAndPatches(2);
		return;
	}

	beginSecondarySpeechLine(4, hasInventoryItem(6) ? 2 : 1);
}

void Scene7010::handleActionSlot06FrankensteinNoteSequence() {
	// Payoff for the Frankenstein-note condition. Without the primed overlay,
	// the original only plays row 6 frame 0 and does not consume Húmero's bone.
	if (_sceneStateFlags[1] == 0) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	runChunk15ItemSequence();
	removeInventoryItem(0x0b);
	runDialogueOverlayFrames(5, 0x1b, 0);
	beginSecondarySpeechLine(6, 4);
	_vm->gameState().reviewedFrankensteinNote = true;
}

void Scene7010::handleActionSlot07DialogueAndReturn() {
	beginSecondarySpeechLine(7, 0);
	runChunk8RevealSequence();
	byte primaryFollowUpFrame = 2;
	if (_sceneStateFlags[3] == 0) {
		const byte firstVisitVariant = (byte)_random.getRandomNumber(2);
		if (firstVisitVariant == 0) {
			beginPrimaryLeftSpeechLine(8, 0);
			primaryFollowUpFrame = 3;
		} else if (firstVisitVariant == 1) {
			beginPrimaryLeftSpeechLine(8, 0);
			primaryFollowUpFrame = 4;
		} else {
			beginPrimaryLeftSpeechLine(8, 3);
			primaryFollowUpFrame = 4;
		}
		_sceneStateFlags[3] = 1;
	} else {
		beginPrimaryLeftSpeechLine(8, 1);
	}
	beginPrimaryLeftSpeechLine(8, primaryFollowUpFrame);
	beginSecondarySpeechLine(7, 1);
	runChunk8HideSequence();
	walkActiveActorTo(0x16b, 0x1df, 3, 0);
	beginSecondarySpeechLine(7, 2);
}

void Scene7010::handleActionSlot08CommonSpeech() {
	beginSecondarySpeechLine(9, 0);
}

void Scene7010::runChunk8RevealSequence() {
	_chunk8SpecialSequenceActive = true;
	_chunk8FrameIndex = 7;
	for (byte frame = 7; frame <= 0x0b && !Engine::shouldQuit(); ++frame) {
		_chunk8FrameIndex = frame;
		waitSceneMillis(kScene7010Chunk8FrameMillis);
	}
}

void Scene7010::runChunk8HideSequence() {
	for (byte frame = 0x0f; frame <= 0x13 && !Engine::shouldQuit(); ++frame) {
		_chunk8FrameIndex = frame;
		waitSceneMillis(kScene7010Chunk8FrameMillis);
	}
	_chunk8FrameIndex = _vm->gameState().currentAmbientMusicCueId == kScene7010AmbientMusicCueWithoutChunk9 ? 0x14 : 0;
	_chunk8SpecialSequenceActive = false;
}

void Scene7010::runChunk11FrameRange(byte startFrame, byte endFrame) {
	setChunk11Visible(true);
	for (byte frame = startFrame; frame <= endFrame && !Engine::shouldQuit(); ++frame) {
		setChunk11Frame(frame);
		waitSceneMillis(kScene7010Chunk8FrameMillis);
	}
}

void Scene7010::runChunk14FrameRange(byte startFrame, byte endFrame) {
	setChunk14Visible(true);
	setChunk11Visible(false);
	for (byte frame = startFrame; frame <= endFrame && !Engine::shouldQuit(); ++frame) {
		setChunk14Frame(frame);
		waitSceneMillis(kScene7010Chunk8FrameMillis);
	}
	setChunk14Visible(false);
}

void Scene7010::runChunk15ItemSequence() {
	setChunk15Visible(true);
	for (byte frame = 0; frame <= 0x17 && !Engine::shouldQuit(); ++frame) {
		setChunk15Frame(frame);
		if (frame == 10) {
			beginPrimarySpeechLine(6, 1, 0x20e, 0x109, 0x3f, 0x28, 0x32);
			beginPrimarySpeechLine(6, 2, 0x20e, 0x109, 0x3f, 0x28, 0x32);
			runDialogueOverlayFrames(0, 5, 2);
			beginPrimarySpeechLine(6, 3, 0x20e, 0x109, 0x3f, 0x28, 0x32);
		}
		waitSceneMillis(kScene7010Chunk8FrameMillis);
	}
	setChunk15Visible(false);
}

void Scene7010::runDialogueOverlayFrames(byte startFrame, byte endFrame, byte finalMode) {
	// Mode 1 is the persistent Frankenstein-note overlay from G04.
	// Mode 2 is the active note transition; mode 0 clears it after the bone is used.
	_sceneStateFlags[1] = 2;
	_vm->gameState().frankensteinNoteOverlayMode = 2;
	setDialogueOverlayMode(2, startFrame);
	for (byte frame = startFrame; frame <= endFrame && !Engine::shouldQuit(); ++frame) {
		setDialogueOverlayFrame(frame);
		waitSceneMillis(kScene7010DialogueOverlayFrameMillis);
	}
	_sceneStateFlags[1] = finalMode;
	_vm->gameState().frankensteinNoteOverlayMode = finalMode;
	setDialogueOverlayMode(finalMode, finalMode == 0 ? 0 : endFrame);
}

void Scene7010::resetTransientOverlayLayers() {
	_backTransientLayers.clear();
	_backTransientLayers.configureLayer(kScene7010DialogueOverlayLayer, 12,
		kScene7010DialogueOverlayMode1DescriptorCount, kScene7010DialogueOverlayMode1FrameMap,
		ARRAYSIZE(kScene7010DialogueOverlayMode1FrameMap), false);
	_backTransientLayers.configureLayer(kScene7010Chunk14Layer, 14, kScene7010Chunk14DescriptorCount,
		kScene7010Chunk14FrameMap, ARRAYSIZE(kScene7010Chunk14FrameMap), false);
	_backTransientLayers.configureLayer(kScene7010Chunk11Layer, 11, kScene7010Chunk11DescriptorCount,
		kScene7010Chunk11FrameMap, ARRAYSIZE(kScene7010Chunk11FrameMap), false);
	_frontTransientLayers.clear();
	_frontTransientLayers.configureLayer(kScene7010Chunk15Layer, 15, kScene7010Chunk15DescriptorCount,
		kScene7010Chunk15FrameMap, ARRAYSIZE(kScene7010Chunk15FrameMap), false);
}

void Scene7010::setDialogueOverlayMode(byte mode, byte frameIndex) {
	_dialogueOverlayMode = mode;
	_dialogueOverlayFrameIndex = frameIndex;

	if (mode == 1) {
		_backTransientLayers.configureLayer(kScene7010DialogueOverlayLayer, 12,
			kScene7010DialogueOverlayMode1DescriptorCount, kScene7010DialogueOverlayMode1FrameMap,
			ARRAYSIZE(kScene7010DialogueOverlayMode1FrameMap));
		_backTransientLayers.setLayerFrame(kScene7010DialogueOverlayLayer, frameIndex);
		return;
	}

	if (mode == 2) {
		_backTransientLayers.configureLayer(kScene7010DialogueOverlayLayer, 16,
			kScene7010DialogueOverlayMode2DescriptorCount, kScene7010DialogueOverlayMode2FrameMap,
			ARRAYSIZE(kScene7010DialogueOverlayMode2FrameMap));
		_backTransientLayers.setLayerFrame(kScene7010DialogueOverlayLayer, frameIndex);
		return;
	}

	_backTransientLayers.setLayerVisible(kScene7010DialogueOverlayLayer, false);
}

void Scene7010::setDialogueOverlayFrame(byte frameIndex) {
	_dialogueOverlayFrameIndex = frameIndex;
	_backTransientLayers.setLayerFrame(kScene7010DialogueOverlayLayer, frameIndex);
}

void Scene7010::setChunk11Visible(bool visible) {
	_backTransientLayers.setLayerVisible(kScene7010Chunk11Layer, visible);
}

void Scene7010::setChunk11Frame(byte frameIndex) {
	_backTransientLayers.setLayerFrame(kScene7010Chunk11Layer, frameIndex);
}

void Scene7010::setChunk14Visible(bool visible) {
	_backTransientLayers.setLayerVisible(kScene7010Chunk14Layer, visible);
}

void Scene7010::setChunk14Frame(byte frameIndex) {
	_backTransientLayers.setLayerFrame(kScene7010Chunk14Layer, frameIndex);
}

void Scene7010::setChunk15Visible(bool visible) {
	_frontTransientLayers.setLayerVisible(kScene7010Chunk15Layer, visible);
}

void Scene7010::setChunk15Frame(byte frameIndex) {
	_frontTransientLayers.setLayerFrame(kScene7010Chunk15Layer, frameIndex);
}

} // End of namespace Hollywood
