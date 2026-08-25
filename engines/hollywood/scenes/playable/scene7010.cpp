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

#include "hollywood/gameplay/dialogue_menu.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const uint16 kScene7010ViewportXOffset = 0;
const uint16 kScene7010ViewportMaxXOffset = 0xd0;
const uint16 kScene7010ReturnState = 0x1b63;
const uint16 kScene7010ExitState7020 = 0x1b6c;
const uint16 kScene7010ExitState7030 = 0x1b76;
const uint16 kScene7010Chunk8DescriptorCount = 0x16;
const uint16 kScene7010Chunk9DescriptorCount = 2;
const uint16 kScene7010Chunk10DescriptorCount = 0x10;
const uint16 kScene7010Chunk11DescriptorCount = 0x25;
const uint16 kScene7010Chunk13DescriptorCount = 0x0d;
const uint16 kScene7010Chunk14DescriptorCount = 0x20;
const uint16 kScene7010Chunk15DescriptorCount = 0x17;
const uint kScene7010HannoverDialogueChoiceRecordCount = 10 * 10 * 7;
const uint16 kScene7010EmbeddedClipBaseChunk = 19;
const uint16 kScene7010EmbeddedClipPaletteChunk = 20;
const uint16 kScene7010EmbeddedClipFramesChunk = 21;
const uint16 kScene7010EmbeddedClipFrameCount = 0x0c;
const uint16 kScene7010DialogueOverlayMode1DescriptorCount = 3;
const uint16 kScene7010DialogueOverlayMode2DescriptorCount = 0x1b;
const uint kScene7010CourtyardPatchState1Chunk = 17;
const uint kScene7010CourtyardPatchState2Chunk = 18;
const uint kScene7010DialogueOverlayLayer = 0;
const uint kScene7010Chunk14Layer = 1;
const uint kScene7010Chunk11Layer = 2;
const uint kScene7010Chunk15Layer = 0;
const byte kScene7010AmbientMusicCueWithoutChunk9 = 0x0f;
const byte kScene7010SueInventoryOwner = 1;
const byte kScene7010HannoverBusinessCardItem = 9;
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
const uint32 kScene7010Chunk11SpeechFrameMillis = 125;
const uint32 kScene7010Chunk14WindowFrameMillis = 60;
const uint32 kScene7010EmbeddedClipFrameMillis = 60;
const uint32 kScene7010DialogueOverlayFrameMillis = 60;
const uint32 kScene7010DoghouseSpeechFrameMillis = 150;
const byte kScene7010DoghouseSpeechBaseFrame = 10;
const byte kScene7010DoghouseSpeechFrameCount = 5;
const byte kScene7010Chunk8FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 5, 7, 8, 9, 10, 11, 12, 13, 21,
	9, 8, 7, 0, 14, 15, 16, 17, 18, 19, 20
};
const byte kScene7010Chunk11FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 20, 19, 18, 13, 12,
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 35, 13, 26, 27,
	28, 29, 30, 31, 32, 33, 34, 13, 36
};
const byte kScene7010Chunk13FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
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
const byte kScene7010DialogueTransitionEnd = 0;
const byte kScene7010DialogueTransitionDown = 1;
const byte kScene7010DialogueTransitionUp = 2;
const byte kScene7010DialogueTransitionStay = 3;
const byte kScene7010DialogueTransitionUpTwo = 4;

static PlayableSceneConfig scene7010Config() {
	PlayableSceneConfig config(7010,
		SceneResourceLayout(19, 5, 18),
		SceneViewport(kScene7010ViewportXOffset, kScene7010ViewportXOffset, kScene7010ViewportMaxXOffset),
		SceneActorPose(0x184, 0x1c6, 1));
	config.loadInventoryActionTables = false;
	config.loadActorDepthTables = false;
	return config;
}

Scene7010::Scene7010(HollywoodEngine *vm) :
		PlayableScene(vm, scene7010Config()),
		_chunk8FrameIndex(0),
		_chunk9AmbientOverlayFrameIndex(0),
		_chunk10IdleFrameA(0),
		_chunk10IdleFrameB(8),
		_chunk10IdleFrameC(4),
		_chunk10IdleFrameD(0x0c),
		_dialogueOverlayFrameIndex(0),
		_dialogueOverlayMode(0),
		_chunk11RightSpeechPoseVariant(0),
		_chunk11RightSpeechLastRandomFrame(0),
		_doghouseSpeechLastRandomFrame(0),
		_chunk8SpecialSequenceActive(false),
		_chunk11RightSpeechActive(false),
		_doghouseSpeechActive(false),
		_chunk10IdlePairAAltPhase(false),
		_chunk10IdlePairBAltPhase(false),
		_chunk10IdlePairATicksRemaining(10),
		_chunk10IdlePairBTicksRemaining(16),
		_chunk8TimerAccumulator(0),
		_chunk10TimerAccumulator(0),
		_chunk11RightSpeechTimerAccumulator(0),
		_doghouseSpeechTimerAccumulator(0),
		_dialogueOverlayTimerAccumulator(0),
		_backTransientLayers(),
		_frontTransientLayers() {
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
	_chunk11RightSpeechPoseVariant = 0;
	_chunk11RightSpeechLastRandomFrame = 0;
	_doghouseSpeechLastRandomFrame = 0;
	_primaryLeftSpeechLastFrame = 0;
	resetTransientOverlayLayers();
	_chunk8SpecialSequenceActive = false;
	_chunk11RightSpeechActive = false;
	_doghouseSpeechActive = false;
	_chunk10IdlePairAAltPhase = _random.getRandomNumber(1) != 0;
	_chunk10IdlePairBAltPhase = _random.getRandomNumber(1) != 0;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_chunk10IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk10IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk8TimerAccumulator = 0;
	_chunk10TimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_chunk11RightSpeechTimerAccumulator = 0;
	_doghouseSpeechTimerAccumulator = 0;
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
	GameplayState &state = _vm->gameState();
	_sceneStateFlags[1] = _vm->gameState().frankensteinNoteOverlayMode;
	// Original G01 flag 4 gates the one-time Hannover business-card handoff.
	_sceneStateFlags[4] = state.hasInventoryItem(kScene7010SueInventoryOwner,
		kScene7010HannoverBusinessCardItem) ? 2 : 1;
	_sceneStateFlags[5] = state.hannoverCourtyardDialogueState;
	_sceneStateFlags[6] = state.hannoverCourtyardFollowUpSeen ? 1 : 0;
	setDialogueOverlayMode(_sceneStateFlags[1], 0);
	applySceneStateToHotspotsAndPatches(0xff);
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

	if (_actionOverlayVisible) {
		const byte chunk8DescriptorIndex = _chunk8FrameIndex < ARRAYSIZE(kScene7010Chunk8FrameMap) ?
			kScene7010Chunk8FrameMap[_chunk8FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[8], 0,
			kScene7010Chunk8DescriptorCount, chunk8DescriptorIndex, _sceneFramebuffer);

		if (!_chunk8SpecialSequenceActive &&
				_vm->gameState().currentAmbientMusicCueId != kScene7010AmbientMusicCueWithoutChunk9) {
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[9], 0,
				kScene7010Chunk9DescriptorCount, _chunk9AmbientOverlayFrameIndex, _sceneFramebuffer);
		}

		// G01 restores chunk-15's dirty rect before drawing the next action frame.
		restoreResourceSpriteLayerBackground(_actionOverlayLayer, _baseFramebuffer);
		drawActionOverlayLayer();
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
		return;
	}

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
	_chunk11RightSpeechActive = false;
	_doghouseSpeechActive = false;
	setDialogueOverlayMode(0, 0);
	_chunk8TimerAccumulator = 0;
	_chunk10TimerAccumulator = 0;
	_chunk11RightSpeechTimerAccumulator = 0;
	_doghouseSpeechTimerAccumulator = 0;
	_dialogueOverlayTimerAccumulator = 0;
	return true;
}

bool Scene7010::advanceCustomGameplayLoop(uint32 delta) {
	updateG01AmbientAudioAndMusicCues(delta);
	advanceDoghouseSpeechFrame(delta);

	if (_primaryLeftSpeechActive && _primarySpeechOverlay.visible) {
		_primaryLeftSpeechTimerAccumulator += delta;
		while (_primaryLeftSpeechTimerAccumulator >= kScene7010Chunk10FrameMillis) {
			advancePrimaryLeftSpeechFrame();
			_primaryLeftSpeechTimerAccumulator -= kScene7010Chunk10FrameMillis;
		}
	} else if (_chunk11RightSpeechActive && _primarySpeechOverlay.visible) {
		_primaryLeftSpeechTimerAccumulator = 0;
		advanceHannoverPrimarySpeechFrame(delta);
		_chunk8TimerAccumulator += delta;
		while (_chunk8TimerAccumulator >= kScene7010Chunk8FrameMillis) {
			advanceChunk8Cycle();
			_chunk8TimerAccumulator -= kScene7010Chunk8FrameMillis;
		}
	} else if (!_chunk8SpecialSequenceActive) {
		_primaryLeftSpeechTimerAccumulator = 0;
		_chunk11RightSpeechTimerAccumulator = 0;
		_chunk8TimerAccumulator += delta;
		while (_chunk8TimerAccumulator >= kScene7010Chunk8FrameMillis) {
			advanceChunk8Cycle();
			_chunk8TimerAccumulator -= kScene7010Chunk8FrameMillis;
		}
	} else {
		_primaryLeftSpeechTimerAccumulator = 0;
		_chunk11RightSpeechTimerAccumulator = 0;
		_chunk8TimerAccumulator = 0;
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
		handleActionSlot06DoghouseSequence();
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

	if (selector == 5 || selector == 0xff) {
		// Original G01 flag 5 patches the mansion window after the Hannover bedroom state changes.
		restoreBaseFramebufferFromOriginal();
		if (_sceneStateFlags[5] == 1) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene7010CourtyardPatchState1Chunk],
				_baseFramebuffer);
		} else if (_sceneStateFlags[5] == 2) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene7010CourtyardPatchState2Chunk],
				_baseFramebuffer);
		}
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
		if (_random.getRandomNumber(4) == 0)
			_chunk9AmbientOverlayFrameIndex = 1;
	}
}

void Scene7010::updateG01AmbientAudioAndMusicCues(uint32 delta) {
	const byte previousCue = _vm->gameState().currentAmbientMusicCueId;
	updateAmbientAudioAndMusicCues(delta);
	const byte currentCue = _vm->gameState().currentAmbientMusicCueId;
	if (currentCue == previousCue)
		return;

	_chunk8FrameIndex = currentCue == kScene7010AmbientMusicCueWithoutChunk9 ? 0x14 : 0;
	_chunk9AmbientOverlayFrameIndex = 0;
	_chunk8TimerAccumulator = 0;
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

void Scene7010::advanceDoghouseSpeechFrame(uint32 delta) {
	if (!_doghouseSpeechActive) {
		_doghouseSpeechTimerAccumulator = 0;
		return;
	}

	_doghouseSpeechTimerAccumulator += delta;
	while (_doghouseSpeechTimerAccumulator >= kScene7010DoghouseSpeechFrameMillis) {
		_doghouseSpeechTimerAccumulator -= kScene7010DoghouseSpeechFrameMillis;

		byte nextFrame = _doghouseSpeechLastRandomFrame;
		for (uint attempt = 0; attempt < 8 && nextFrame == _doghouseSpeechLastRandomFrame; ++attempt)
			nextFrame = (byte)_random.getRandomNumber(kScene7010DoghouseSpeechFrameCount - 1);
		if (nextFrame == _doghouseSpeechLastRandomFrame)
			nextFrame = (byte)((_doghouseSpeechLastRandomFrame + 1) % kScene7010DoghouseSpeechFrameCount);

		_doghouseSpeechLastRandomFrame = nextFrame;
		_actionOverlayPlayer.setFrame(kScene7010DoghouseSpeechBaseFrame + nextFrame);
	}
}

void Scene7010::advanceDialogueOverlay(uint32 delta) {
	if (_dialogueOverlayMode != 1 || _doghouseSpeechActive)
		return;

	_dialogueOverlayTimerAccumulator += delta;
	while (_dialogueOverlayTimerAccumulator >= kScene7010DialogueOverlayFrameMillis) {
		_dialogueOverlayTimerAccumulator -= kScene7010DialogueOverlayFrameMillis;
		setDialogueOverlayFrame(_dialogueOverlayFrameIndex == 3 ? 0 : _dialogueOverlayFrameIndex + 1);
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

	if (_sceneStateFlags[4] == 1) {
		beginHannoverPrimarySpeechLine(0, 0);
		beginSecondarySpeechLine(0x62, 0);
		runChunk11MidFrames();
		beginHannoverPrimarySpeechLine(1, 1);
		runChunk11ReturnFrames();
		runHannoverDialogueMenuRow98();
		beginHannoverPrimarySpeechLine(7, 0);
		runChunk11ExtendedFrames();
		beginSecondarySpeechLine(0x62, 8);
		runChunk11MidFrames();
		beginHannoverPrimarySpeechLine(3, 1);
		runChunk11ReturnFrames();
		beginSecondarySpeechLine(0x62, 9);
		runChunk11MidFrames();
		beginHannoverPrimarySpeechLine(8, 1);
	} else {
		beginHannoverPrimarySpeechLine(2, 0);
		beginSecondarySpeechLine(0x62, 1);
		beginHannoverPrimarySpeechLine(3, 0);

		if (_sceneStateFlags[5] != 0) {
			beginHannoverPrimarySpeechLine(10, 0);
			runChunk14FrameRange(0, 0x18, true);
			if (_sceneStateFlags[6] == 0) {
				beginSecondarySpeechLine(0x62, 0x0b);
				beginHannoverPrimarySpeechLine(0x0b, 0);
				_sceneStateFlags[6] = 1;
				state.hannoverCourtyardFollowUpSeen = true;
			}
			if (_sceneStateFlags[5] == 2) {
				beginHannoverPrimarySpeechLine(0x0c, 0);
				runChunk14FrameRange(0x19, 0x38, false);
				walkActiveActorTo(0x298, 0x1af, 4, 0);
				beginSecondarySpeechLine(0x62, 0x0c);
				walkActiveActorTo(0x3b0, 0x1a9, 0xff, 0);
				_vm->gameplayMusic()->stop();
				state.mainFlowStateId = kScene7010ExitState7020;
				return;
			}
		}

		runChunk11MidFrames();
		beginHannoverPrimarySpeechLine(4, 1);
		runChunk11ReturnFrames();
		runHannoverDialogueMenuRow98();
		beginHannoverPrimarySpeechLine(6, 0);
		beginSecondarySpeechLine(0x62, 10);
		runChunk11MidFrames();
		beginHannoverPrimarySpeechLine(9, 1);
	}

	runChunk11ReturnFrames();
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

void Scene7010::handleActionSlot06DoghouseSequence() {
	// Payoff for the Frankenstein-note condition. Without the primed overlay,
	// the original only plays row 6 frame 0 and does not consume Húmero's bone.
	if (_sceneStateFlags[1] == 0) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	runDoghouseDepartureSequence();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	removeInventoryItem(0x0b);
	_soundBank0.playSample(1, 100);
	runDialogueOverlayFrames(5, 0x1b, 0);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
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

void Scene7010::runHannoverDialogueMenuRow98() {
	Common::Array<DialogueChoiceRecord> records;
	initializeHannoverDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	while (!finished && !Engine::shouldQuit()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(0x62, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(0x62, 5);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(0x62, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			runChunk11MidFrames();
			beginHannoverPrimarySpeechLine(record.responseFrameIndex, 1);
			runChunk11ReturnFrames();
		}

		if (record.disableAfterUse == 1)
			record.enabled = 0;
		finished = applyHannoverDialogueTransition(record, depthIndex, nodeIndex);
	}
}

void Scene7010::initializeHannoverDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene7010HannoverDialogueChoiceRecordCount);

	// DAT_00512088: Sue's row-98 Hannover courtyard dialogue menu.
	setHannoverDialogueRecord(records, 0, 1, 0, kScene7010DialogueTransitionDown, 2, 5, 1);
	setHannoverDialogueRecord(records, 1, 1, 0, kScene7010DialogueTransitionEnd, 3, 0, 1);
	setHannoverDialogueRecord(records, 2, 1, 0, kScene7010DialogueTransitionEnd, 4, 0, 1);
	setHannoverDialogueRecord(records, 3, 1, 0, kScene7010DialogueTransitionEnd, 5, 0, 1);

	// Depth 1, node 0: follow-up choices after the first root topic.
	setHannoverDialogueRecord(records, 70, 1, 0, kScene7010DialogueTransitionEnd, 6, 0, 1);
	setHannoverDialogueRecord(records, 71, 1, 0, kScene7010DialogueTransitionEnd, 7, 0, 1);
}

void Scene7010::setHannoverDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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
	record.selectable = 1;
}

bool Scene7010::applyHannoverDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const {
	const byte previousDepth = depthIndex;
	switch (record.transitionMode) {
	case kScene7010DialogueTransitionEnd:
		return true;
	case kScene7010DialogueTransitionDown:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth + 1;
		break;
	case kScene7010DialogueTransitionUp:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth == 0 ? 0 : (byte)(previousDepth - 1);
		break;
	case kScene7010DialogueTransitionUpTwo:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
		break;
	case kScene7010DialogueTransitionStay:
	default:
		break;
	}

	return false;
}

void Scene7010::beginHannoverPrimarySpeechLine(byte frameIndex, byte poseVariant) {
	_chunk11RightSpeechPoseVariant = poseVariant;
	_chunk11RightSpeechLastRandomFrame = 0xff;
	_chunk11RightSpeechTimerAccumulator = 0;
	_chunk11RightSpeechActive = true;
	setChunk11Frame(poseVariant == 0 ? 0x0e : 0x16);

	beginPrimarySpeechLine(99, frameIndex, poseVariant == 0 ? 0x302 : 0x2ee,
		poseVariant == 0 ? 0xe3 : 0xe8, 0x28, 0x16, 0x0b);

	_chunk11RightSpeechActive = false;
	_chunk11RightSpeechTimerAccumulator = 0;
	setChunk11Frame(poseVariant == 0 ? 0x0e : 0x16);
}

void Scene7010::advanceHannoverPrimarySpeechFrame(uint32 delta) {
	_chunk11RightSpeechTimerAccumulator += delta;
	while (_chunk11RightSpeechTimerAccumulator >= kScene7010Chunk11SpeechFrameMillis) {
		_chunk11RightSpeechTimerAccumulator -= kScene7010Chunk11SpeechFrameMillis;

		byte nextFrame = _chunk11RightSpeechLastRandomFrame;
		for (uint attempt = 0; attempt < 8 && nextFrame == _chunk11RightSpeechLastRandomFrame; ++attempt)
			nextFrame = (byte)_random.getRandomNumber(4);
		if (nextFrame == _chunk11RightSpeechLastRandomFrame)
			nextFrame = (byte)((_chunk11RightSpeechLastRandomFrame + 1) % 5);

		_chunk11RightSpeechLastRandomFrame = nextFrame;
		setChunk11Frame((_chunk11RightSpeechPoseVariant == 0 ? 0x0e : 0x16) + nextFrame);
	}
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

void Scene7010::runChunk11MidFrames() {
	runChunk11FrameRange(0x12, 0x16);
	_chunk11RightSpeechPoseVariant = 1;
}

void Scene7010::runChunk11ReturnFrames() {
	runChunk11FrameRange(0x1a, 0x1e);
	setChunk11Frame(0x0e);
	_chunk11RightSpeechPoseVariant = 0;
}

void Scene7010::runChunk11ExtendedFrames() {
	for (byte frame = 0x2d; frame <= 0x37 && !Engine::shouldQuit(); ++frame) {
		if (frame == 0x33) {
			runChunk13Item09PickupOverlaySequence();
			setChunk11Frame(0x33);
		}
		setChunk11Frame(frame);
		waitSceneMillis(kScene7010Chunk8FrameMillis);
	}
	setChunk11Frame(0x0e);
}

void Scene7010::runChunk13Item09PickupOverlaySequence() {
	const bool previousHideActiveActor = _actionOverlayPlayer.applyActorVisibility(kActionOverlayHideActiveActor);
	_actionOverlayPlayer.begin(13, kScene7010Chunk13DescriptorCount,
		kScene7010Chunk13FrameMap, ARRAYSIZE(kScene7010Chunk13FrameMap));

	for (byte frame = 1; frame < ARRAYSIZE(kScene7010Chunk13FrameMap) && !Engine::shouldQuit(); ++frame) {
		if (frame == 5)
			setChunk11Frame(0x38);
		_actionOverlayPlayer.setFrame(frame);
		if (waitSceneMillis(kScene7010Chunk8FrameMillis))
			break;
	}

	_actionOverlayPlayer.finish(previousHideActiveActor);
	if (!hasInventoryItem(kScene7010HannoverBusinessCardItem))
		addInventoryItem(kScene7010HannoverBusinessCardItem);
	_soundBank0.playSample(1, 100);
}

void Scene7010::runChunk14FrameRange(byte startFrame, byte endFrame, bool restoreChunk11AtEnd) {
	setChunk14Visible(true);
	setChunk11Visible(false);
	for (byte frame = startFrame; frame <= endFrame && !Engine::shouldQuit(); ++frame) {
		setChunk14Frame(frame);
		waitSceneMillis(kScene7010Chunk14WindowFrameMillis);
	}
	setChunk14Visible(false);
	if (restoreChunk11AtEnd)
		setChunk11Visible(true);
}

void Scene7010::runDoghouseDepartureSequence() {
	const bool previousHideActiveActor = _actionOverlayPlayer.applyActorVisibility(kActionOverlayHideActiveActor);
	_actionOverlayPlayer.begin(15, kScene7010Chunk15DescriptorCount,
		kScene7010Chunk15FrameMap, ARRAYSIZE(kScene7010Chunk15FrameMap));
	_doghouseSpeechLastRandomFrame = 0xff;

	for (byte frame = 1; frame <= 0x17 && !Engine::shouldQuit(); ++frame) {
		_actionOverlayPlayer.setFrame(frame);
		if (waitSceneMillis(kScene7010Chunk8FrameMillis, false))
			break;
		if (frame == 10) {
			runDoghouseSpeechLine(1);
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				break;
			runDoghouseSpeechLine(2);
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				break;
			runDialogueOverlayFrames(0, 5, 2);
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				break;
			runDoghouseSpeechLine(3);
		}
	}

	if (Engine::shouldQuit() || _vm->isSceneRestartRequested()) {
		_actionOverlayPlayer.finish(previousHideActiveActor);
		return;
	}

	_actionOverlayPlayer.setFrame(0);
	runDogCloseupSequence();
	_actionOverlayPlayer.finish(previousHideActiveActor);
}

void Scene7010::runDoghouseSpeechLine(byte frameIndex) {
	_doghouseSpeechActive = true;
	_doghouseSpeechTimerAccumulator = 0;
	_actionOverlayPlayer.setFrame(kScene7010DoghouseSpeechBaseFrame);
	beginPrimarySpeechLine(6, frameIndex, 0x20e, 0x109, 0x3f, 0x28, 0x32);
	_doghouseSpeechActive = false;
	_doghouseSpeechTimerAccumulator = 0;
	_skipRequested = false;
	_actionOverlayPlayer.setFrame(kScene7010DoghouseSpeechBaseFrame);
}

void Scene7010::runDogCloseupSequence() {
	Common::Array<byte> base;
	Common::Array<byte> palette;
	Common::Array<byte> frames;
	if (!loadVariableChunk(kScene7010EmbeddedClipBaseChunk, base) ||
			!loadVariableChunk(kScene7010EmbeddedClipPaletteChunk, palette) ||
			!loadVariableChunk(kScene7010EmbeddedClipFramesChunk, frames)) {
		warning("Scene 7010 failed to load the dog close-up clip");
		return;
	}
	if (palette.size() != kPaletteSize ||
			frames.size() < (uint32)kScene7010EmbeddedClipFrameCount * 4) {
		warning("Scene 7010 dog close-up clip has invalid resources");
		return;
	}

	Graphics::ManagedSurface savedScene;
	savedScene.copyFrom(_sceneFramebuffer);
	const Common::Array<byte> savedPalette = _paletteCurrent;
	const uint16 savedViewportX = _viewportXOffset;
	Common::Array<byte> blackPalette;
	blackPalette.resize(kPaletteSize);
	memset(blackPalette.data(), 0, blackPalette.size());

	_paletteCurrent = blackPalette;
	_displayPalette.markAllDirty();
	presentFrame();

	_viewportXOffset = 0;
	memset(framebufferPixels(_sceneFramebuffer), 0, framebufferByteCount());
	drawResourceBlockList(base, 0, _sceneFramebuffer);
	presentFrame();

	_paletteCurrent = palette;
	_displayPalette.markAllDirty();
	presentFrame();

	for (byte frame = 0; frame < kScene7010EmbeddedClipFrameCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		if (!ResourceDeltaClipPlayer::drawFrame(frames, 0, frames.size(),
				kScene7010EmbeddedClipFrameCount, frame,
				framebufferPixels(_sceneFramebuffer), framebufferByteCount())) {
			warning("Scene 7010 failed to decode dog close-up frame %u", frame);
			break;
		}
		presentFrame();

		uint32 remaining = kScene7010EmbeddedClipFrameMillis;
		while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
			if (pollEvents(false)) {
				remaining = 0;
				break;
			}
			const uint32 slice = MIN<uint32>(remaining, 10);
			g_system->delayMillis(slice);
			updateG01AmbientAudioAndMusicCues(slice);
			remaining -= slice;
		}
	}

	_paletteCurrent = blackPalette;
	_displayPalette.markAllDirty();
	presentFrame();

	_sceneFramebuffer.copyRectToSurface(savedScene.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	_viewportXOffset = savedViewportX;
	presentFrame();
	_actionOverlayPlayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();

	_paletteCurrent = savedPalette;
	_displayPalette.markAllDirty();
	presentFrame();
}

void Scene7010::runDialogueOverlayFrames(byte startFrame, byte endFrame, byte finalMode) {
	// Mode 1 is the persistent Frankenstein-note overlay from G04.
	// Mode 2 is the active note transition; mode 0 clears it after the bone is used.
	_sceneStateFlags[1] = 2;
	_vm->gameState().frankensteinNoteOverlayMode = 2;
	setDialogueOverlayMode(2, startFrame);
	for (uint frame = startFrame + 1; frame <= endFrame && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		setDialogueOverlayFrame((byte)frame);
		if (frame == 0x0e || frame == 0x15 || frame == 0x1b)
			_soundBank0.playSample(0x13, 80);
		if (waitSceneMillis(kScene7010DialogueOverlayFrameMillis, false))
			break;
	}
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
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
