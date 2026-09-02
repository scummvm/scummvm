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

#include "hollywood/scenes/intro/scene9100.h"

#include "common/debug.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kI10ArchiveName = "RESOURCE.I10";
const char *const kStage003ArchiveName = "RESOURCE.003";
const char *const kScene9100SoundArchiveName = "RESOURCE.S09";
const uint16 kScene9100MusicCueId = 0x000f;
const byte kScene9100ClockSoundCue = 0x0b;
const byte kScene9100TransitionEndSoundCue = 0x0c;
const byte kScene9100TransitionMiddleSoundCue = 0x0d;
const byte kScene9100ForegroundBeatSoundCue = 0x0e;
const byte kScene9100DelayedSoundCue = 0x0f;
const byte kScene9100SueEntrySoundCue = 0x10;
const byte kScene9100AmbientSoundCue = 0x11;
const uint16 kScene9101CompletionState = 1000;
const uint kStage910Index = 910;
const byte kPrimarySpeechTextColor = 0xfb;
const byte kSecondarySpeechTextColor = 0xfd;
const uint kActorFacingCount = 6;
const uint kActorCelsPerFacing = 13;
const uint kActorDescriptorCount = kActorFacingCount * kActorCelsPerFacing;
const uint kActorSpriteDescriptorSize = 28;
const uint kActorFacingRunStride = 160000;
const uint kPresentationPaletteRemapTableSize = 256;
const uint kI10RonActorRunChunk = 23;
const uint kI10RonActorDescriptorChunk = 24;
const uint kI10SueActorRunChunk = 25;
const uint kI10SueActorDescriptorChunk = 26;
const byte kI10SceneActorFacing = 5;
const uint kI10SceneActorDescriptorBase = kI10SceneActorFacing * kActorCelsPerFacing;
const uint kActorEntryFrameDelayMillis = 90;
const uint32 kClockFrameIntervalMillis = 1000;
const uint32 kTalkingFrameIntervalMillis = 125;
const uint32 kEntryPathFrameIntervalMillis = 60;
const uint32 kRonEntryPathDurationMillis = 4200;
const uint32 kSueEntryPathDurationMillis = 3600;

const byte kI10ForegroundFrameRemap[] = {
	0, 31, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13,
	32, 33, 34, 35, 14, 15, 16, 16, 17, 18, 19, 29, 20, 21, 22, 23,
	23, 24, 25, 26, 30, 22, 21, 20, 16
};

const Scene9100::SpeechTextStyle kDeskPrimaryBlueSpeech = { 0x78, 0xb9, kPrimarySpeechTextColor, 0x00, 0x26, 0x3f, true };
const Scene9100::SpeechTextStyle kInsetBlueSpeech = { 0x140, 0x50, kPrimarySpeechTextColor, 0x00, 0x26, 0x3f, true };
const Scene9100::SpeechTextStyle kInsetWhiteSpeech = { 0x140, 0x50, kPrimarySpeechTextColor, 0x3f, 0x3f, 0x3f, true };
const Scene9100::SpeechTextStyle kInsetSueSpeech = { 0x140, 0x50, kPrimarySpeechTextColor, 0x3f, 0x28, 0x32, true };
const Scene9100::SpeechTextStyle kRonSecondarySpeech = { 0x276, 0x10c, kSecondarySpeechTextColor, 0x00, 0x00, 0x00, false };
const Scene9100::SpeechTextStyle kSueSecondarySpeech = { 0x276, 0xf7, kSecondarySpeechTextColor, 0x00, 0x00, 0x00, false };

Scene9100::Scene9100(HollywoodEngine *vm) :
		PresentationScene(vm, "intro scene 9100"),
		_music(),
		_speech(vm->getLanguage(), vm->hasSpeechData()),
		_text(),
		_effectSound(),
		_clockSound(),
		_ambientSound(),
		_random("hollywood_scene9100"),
		_clockFrameAccumulator(kClockFrameIntervalMillis),
		_talkingFrameAccumulator(0),
		_foregroundActorFrame(0),
		_foregroundTalkBaseFrame(15),
		_clockChunk7Frame(0x2c),
		_clockChunk8Frame(0x30),
		_clockChunk9Frame(0x20),
		_clockChunk7CarryGate(4),
		_talkingFrame(0),
		_lastTalkingFrameVariant(0xff),
		_deskPrimaryActorFrame(0),
		_deskSecondaryActorFrame(0),
		_clockVisible(false),
		_deskPrimaryActorVisible(false),
		_deskSecondaryActorVisible(false),
		_dialogueBranch(false) {
	_paletteDefault.resize(kPaletteSize);
	_frameDecodeBuffer.resize(kFrameDecodeBufferSize);
	_cleanOfficeBaseFramebuffer.resize(kFrameDecodeBufferSize);
	_presentationPaletteRemapTable.resize(kPresentationPaletteRemapTableSize);
	for (uint i = 0; i < _presentationPaletteRemapTable.size(); ++i)
		_presentationPaletteRemapTable[i] = 0;
	_secondaryScratchBuffer.resize(kSecondaryScratchBufferSize);
	_effectSound.setArchive(Common::Path(kScene9100SoundArchiveName));
	_clockSound.setArchive(Common::Path(kScene9100SoundArchiveName));
	_ambientSound.setArchive(Common::Path(kScene9100SoundArchiveName));
}

bool Scene9100::play() {
	if (!load(false))
		return false;

	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _frameDecodeBuffer.size());
	runEntryActorAnimations();
	_clockVisible = true;
	drawInitialForegroundFrame();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());

	presentFrame();

	_music.playMusicCue(kScene9100MusicCueId, 30);

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
		if (delay(50))
			break;
	}

	if (!_skipRequested && !Engine::shouldQuit()) {
		_ambientSound.playSample(kScene9100AmbientSoundCue, 25, true);
		expandFillRunsToSavedFramebuffer();
		drawResourceBlockListToSceneFramebuffer(_resources._chunkOffsets[16]);
		presentFrame();
		runOpeningPrelude();
	}

	if (!_skipRequested && !Engine::shouldQuit())
		runCinematicSequence();

	if (!_skipRequested && !Engine::shouldQuit()) {
		runEndingWipe();
	}

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	return true;
}

bool Scene9100::playDialogueBranch() {
	if (!load(true))
		return false;

	initializeDialogueBranchOfficeState();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());

	copyDefaultPalette();
	presentFrame();
	_music.playMusicCue(kScene9100MusicCueId, 30);

	for (int sweepOffset = 0xdc; sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit(); sweepOffset -= 0x14) {
		revealSavedFramebufferBand((uint)sweepOffset, 0x14);
		presentFrame();
		if (delay(50))
			break;
	}

	if (!_skipRequested && !Engine::shouldQuit())
		_ambientSound.playSample(kScene9100AmbientSoundCue, 25, true);

	if (!_skipRequested && !Engine::shouldQuit())
		runDialogueBranchSequence();

	if (!_skipRequested && !Engine::shouldQuit())
		runEndingWipe();

	stopAudio();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit()) {
		GameplayState &state = _vm->gameState();
		state.initializeRonItemResourcePages();
		state.initializeRonInventoryItems(_vm->isDemo());
		state.currentInventoryOwnerIndex = 0;
		state.activeAudioChapterIndex = 9;
		state.inventoryPanelDirty = true;
		state.mainFlowStateId = kScene9101CompletionState;
	}

	return true;
}

bool Scene9100::load(bool dialogueBranch) {
	_dialogueBranch = dialogueBranch;
	if (!_resources.loadChunkTable(kI10ArchiveName))
		return false;

	if (!_resources.validateChunkRange(kI10ArchiveName, _debugName, 0, 16) ||
			(dialogueBranch &&
			!_resources.validateChunk(kI10ArchiveName, _debugName, 17)) ||
			!_resources.validateChunkRange(kI10ArchiveName, _debugName, 18, 26))
		return false;

	if (!loadFixedChunk(0, _frameDecodeBuffer, kFrameDecodeBufferSize) ||
			!loadFixedChunk(1, _paletteDefault, kPaletteSize) ||
			!loadVariableChunk(2, _sceneFillRuns) ||
			!_text.loadStage(kStage003ArchiveName, _debugName, kStage910Index))
		return false;
	memcpy(_cleanOfficeBaseFramebuffer.data(), _frameDecodeBuffer.data(), _cleanOfficeBaseFramebuffer.size());
	memcpy(_paletteCurrent.data(), _paletteDefault.data(), _paletteCurrent.size());
	buildPresentationPaletteRemapTable(_paletteCurrent, _presentationPaletteRemapTable);

	_resources.allocateArena(_resources.totalChunkSize(5, 16));
	for (uint i = 5; i <= 16; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}
	if (dialogueBranch && !loadArenaChunkAlias(17, 17, 16))
		return false;

	const uint32 scratchSize = MAX<uint32>(
		kScratchPrimaryPayloadBase + _resources._chunkTable.sizes[22],
		MAX<uint32>(_resources._chunkTable.sizes[20],
			kScratchChunk21Base + _resources._chunkTable.sizes[21]));
	_resourceScratchArena.resize(scratchSize);
	memset(_resourceScratchArena.data(), 0, _resourceScratchArena.size());
	memset(_secondaryScratchBuffer.data(), 0, _secondaryScratchBuffer.size());

	if (!loadChunkTo(20, _resourceScratchArena, 0) ||
			!loadChunkTo(21, _resourceScratchArena, kScratchChunk21Base) ||
			!loadChunkTo(22, _resourceScratchArena, kScratchPrimaryPayloadBase) ||
			!loadChunkTo(18, _secondaryScratchBuffer, kDeskPrimaryStaticBase) ||
			!loadChunkTo(19, _secondaryScratchBuffer, kDeskSecondaryStaticBase))
		return false;

	return loadActorResources();
}

bool Scene9100::loadActorResources() {
	return loadI10ActorBank(kI10RonActorRunChunk, kI10RonActorDescriptorChunk, _actorBankI10Ron) &&
		loadI10ActorBank(kI10SueActorRunChunk, kI10SueActorDescriptorChunk, _actorBankI10Sue);
}

bool Scene9100::loadI10ActorBank(uint runStreamChunkIndex, uint descriptorChunkIndex, ActorSpriteBank &bank) {
	Common::ScopedPtr<Common::SeekableReadStream> runStream(
		createResourceChunkReadStream(Common::Path(kI10ArchiveName), runStreamChunkIndex));
	if (!runStream) {
		warning("Failed to open %s actor run chunk %u", kI10ArchiveName, runStreamChunkIndex);
		return false;
	}

	bank.runStreams.resize(kActorFacingCount * kActorFacingRunStride);
	memset(bank.runStreams.data(), 0, bank.runStreams.size());
	bank.descriptors.resize(kActorDescriptorCount);
	for (uint i = 0; i < bank.descriptors.size(); ++i) {
		bank.descriptors[i].runStreamOffset = 0;
		bank.descriptors[i].opaqueRunCount = 0;
		bank.descriptors[i].paletteRunCount = 0;
		bank.descriptors[i].anchorX = 0;
		bank.descriptors[i].anchorY = 0;
		bank.descriptors[i].width = 0;
		bank.descriptors[i].height = 0;
	}

	if (runStream->size() > kActorFacingRunStride) {
		warning("%s actor run chunk %u is too large", kI10ArchiveName, runStreamChunkIndex);
		return false;
	}

	const uint runBase = kI10SceneActorFacing * kActorFacingRunStride;
	if (runStream->read(bank.runStreams.data() + runBase, runStream->size()) != (uint32)runStream->size()) {
		warning("Failed to read %s actor run chunk %u", kI10ArchiveName, runStreamChunkIndex);
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> descriptorStream(
		createResourceChunkReadStream(Common::Path(kI10ArchiveName), descriptorChunkIndex));
	if (!descriptorStream) {
		warning("Failed to open %s actor descriptor chunk %u", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	if (descriptorStream->size() % kActorSpriteDescriptorSize != 0) {
		warning("%s actor descriptor chunk %u has an invalid size", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	const uint descriptorCount = descriptorStream->size() / kActorSpriteDescriptorSize;
	if (descriptorCount > kActorCelsPerFacing) {
		warning("%s actor descriptor chunk %u has too many descriptors", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	Common::Array<byte> descriptorData;
	descriptorData.resize(descriptorStream->size());
	if (descriptorStream->read(descriptorData.data(), descriptorData.size()) != descriptorData.size()) {
		warning("Failed to read %s actor descriptor chunk %u", kI10ArchiveName, descriptorChunkIndex);
		return false;
	}

	for (uint i = 0; i < descriptorCount; ++i) {
		const uint offset = i * kActorSpriteDescriptorSize;
		ActorSpriteDescriptor &descriptor = bank.descriptors[kI10SceneActorDescriptorBase + i];
		descriptor.runStreamOffset = readUint32(descriptorData, offset);
		descriptor.opaqueRunCount = readUint32(descriptorData, offset + 4);
		descriptor.paletteRunCount = readUint32(descriptorData, offset + 8);
		descriptor.anchorX = readSint16(descriptorData, offset + 12);
		descriptor.anchorY = readSint16(descriptorData, offset + 16);
		descriptor.width = readUint16(descriptorData, offset + 20);
		descriptor.height = readUint16(descriptorData, offset + 24);
	}

	debugC(1, kDebugResources, "Loaded %s actor chunks %u/%u", kI10ArchiveName, runStreamChunkIndex, descriptorChunkIndex);
	return true;
}

void Scene9100::applyActorHighlightColor(byte highlightRed, byte highlightGreen, byte highlightBlue) {
	_paletteCurrent[0x2f7] = highlightRed;
	_paletteCurrent[0x2f8] = highlightGreen;
	_paletteCurrent[0x2f9] = highlightBlue;
}

void Scene9100::runEntryActorAnimations() {
	IndexedSurfaceBuffer baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	applyActorHighlightColor(0x3f, 0x3f, 0x3f);
	playEntryActorAnimation(_actorBankI10Ron, 0x307, 0x1d4, baseFramebuffer);

	memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
}

void Scene9100::playEntryActorAnimation(const ActorSpriteBank &bank, int worldX, int worldY, IndexedSurfaceBuffer &baseFramebuffer) {
	const byte kFacingTurnToCamera = 5;
	const byte kTurnCel = 2;
	const byte kFinalCel = 0;
	const byte kFrames[][2] = {
		{ kFacingTurnToCamera, kTurnCel },
		{ kFacingTurnToCamera, kFinalCel }
	};

	if (bank.descriptors.size() < kActorDescriptorCount || bank.runStreams.size() < kActorFacingCount * kActorFacingRunStride)
		return;

	for (uint i = 0; i < ARRAYSIZE(kFrames) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawActorFrame(bank, kFrames[i][0], kFrames[i][1], worldX, worldY);
		presentFrame();
		if (delay(kActorEntryFrameDelayMillis))
			return;
	}

	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());
}

void Scene9100::runRonEntryConversation() {
	const SceneSpeechCue popup = _text.stageCue(0, 2);
	const uint segmentCount = MAX<uint>(1, popup.continuationCount);

	IndexedSurfaceBuffer baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	uint32 pathElapsed = 0;
	uint32 pathFrameAccumulator = 0;
	byte foregroundFrame = _foregroundTalkBaseFrame;
	bool pathPresented = false;
	_talkingFrameAccumulator = kTalkingFrameIntervalMillis;

	for (uint segmentIndex = 0; segmentIndex < segmentCount && !_skipRequested && !Engine::shouldQuit(); ++segmentIndex) {
		const uint16 sampleId = popup.voiceSampleId + segmentIndex;
		beginSubtitle(popup, segmentIndex, kDeskPrimaryBlueSpeech);
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
		uint32 elapsed = 0;

		if (!pathPresented) {
			memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
			drawForegroundActorFrame(foregroundFrame);
			drawRonEntryPathFrame(0, kRonEntryPathDurationMillis);
			presentFrame();
			pathPresented = true;
		}

		while (!_skipRequested && !Engine::shouldQuit()) {
			const bool speechActive = _speech.isPlaying();
			if (!speechActive && elapsed >= fallbackMillis)
				break;

			const uint32 slice = 10;
			if (delay(slice)) {
				if (_skipRequested || Engine::shouldQuit())
					return;
				break;
			}
			elapsed += slice;

			const bool clockDirty = advanceClockTimer(slice);
			bool dirty = clockDirty;
			if (advanceTalkingTimer(slice)) {
				foregroundFrame = (byte)(_foregroundTalkBaseFrame + nextTalkingFrameVariant());
				dirty = true;
			}
			if (pathElapsed < kRonEntryPathDurationMillis)
				pathFrameAccumulator += slice;
			if (pathFrameAccumulator >= kEntryPathFrameIntervalMillis) {
				pathElapsed = MIN<uint32>(kRonEntryPathDurationMillis,
					pathElapsed + pathFrameAccumulator);
				pathFrameAccumulator = 0;
				dirty = true;
			}

			if (dirty) {
				memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
				if (clockDirty)
					advanceClockFrame();
				drawForegroundActorFrame(foregroundFrame);
				drawRonEntryPathFrame(pathElapsed, kRonEntryPathDurationMillis);
				presentFrame();
			}
		}
		_speech.stop();

		if (segmentIndex + 1 < segmentCount && !_skipRequested && !Engine::shouldQuit())
			delayFrame(375, kTalkingOverlayNone, 0, true, true);
	}
	clearSubtitle();

	if (!_skipRequested && !Engine::shouldQuit()) {
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawForegroundActorFrame(_foregroundTalkBaseFrame);
		drawRonEntryPathFrame(kRonEntryPathDurationMillis, kRonEntryPathDurationMillis);
		presentFrame();
		_deskPrimaryActorVisible = true;
		_deskPrimaryActorFrame = 0;
	}
}

void Scene9100::drawRonEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis) {
	const int originalStartX = 0x307;
	const int originalStartY = 0x1d4;
	const int targetX = 0xc0;
	const int targetY = 0x191;
	const uint startDescriptorIndex = kI10SceneActorDescriptorBase + 1;
	if (startDescriptorIndex >= _actorBankI10Ron.descriptors.size())
		return;

	const ActorSpriteDescriptor &startDescriptor = _actorBankI10Ron.descriptors[startDescriptorIndex];
	const int startSpriteLeft = MAX<int>(0, HollywoodEngine::kScreenWidth - (int)startDescriptor.width - 8);
	const int fixedViewportStartX = startSpriteLeft + startDescriptor.anchorX;

	const int visibleStartY = originalStartY +
		((originalStartX - fixedViewportStartX) * (targetY - originalStartY)) / (originalStartX - targetX);
	const uint32 clampedElapsed = MIN<uint32>(pathElapsedMillis, pathDurationMillis);
	const int x = fixedViewportStartX + ((targetX - fixedViewportStartX) * (int)clampedElapsed) / (int)pathDurationMillis;
	const int y = visibleStartY + ((targetY - visibleStartY) * (int)clampedElapsed) / (int)pathDurationMillis;

	const bool finalFrame = clampedElapsed >= pathDurationMillis;
	const byte facing = kI10SceneActorFacing;
	const byte cel = finalFrame ? 0 :
		(byte)(1 + ((clampedElapsed / kEntryPathFrameIntervalMillis) % 12));
	drawActorFrame(_actorBankI10Ron, facing, cel, x, y);
}

void Scene9100::runSueEntrySequence() {
	if (_skipRequested || Engine::shouldQuit())
		return;

	restoreOfficeFrameAndPresent();
	applyActorHighlightColor(0x3f, 0x28, 0x32);

	animateForegroundFrames(27, 31);
	_foregroundTalkBaseFrame = 32;
	_effectSound.playSample(kScene9100SueEntrySoundCue, 100);
	runConversationStep(1, 6, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	animateForegroundFrames(36, 40);
	_foregroundTalkBaseFrame = 23;
	_deskPrimaryActorVisible = true;
	animateDeskPrimaryStaticFrames(0, 2);
	drawOfficeCompositeLayers();
	presentFrame();
	runConversationStep(1, 7, kTalkingOverlayNone, 0, false, true, kSueSecondarySpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	runSueEntryPath();
	_deskSecondaryActorVisible = true;
	animateDeskSecondaryStaticFrames(0, 5);
	drawOfficeCompositeLayers();
	presentFrame();
}

void Scene9100::runSueEntryPath() {
	IndexedSurfaceBuffer baseFramebuffer;
	baseFramebuffer.resize(_sceneFramebuffer.size());
	memcpy(baseFramebuffer.data(), _sceneFramebuffer.data(), baseFramebuffer.size());

	uint32 pathElapsed = 0;
	uint32 pathFrameAccumulator = 0;

	memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
	drawSueEntryPathFrame(0, kSueEntryPathDurationMillis);
	presentFrame();

	TimedPresentationLoop loop(*this, kSueEntryPathDurationMillis);
	while (loop.beginFrame()) {
		pathFrameAccumulator += loop.finishFrame();
		if (pathFrameAccumulator < kEntryPathFrameIntervalMillis)
			continue;

		pathElapsed = MIN<uint32>(kSueEntryPathDurationMillis,
			pathElapsed + pathFrameAccumulator);
		pathFrameAccumulator = 0;
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawSueEntryPathFrame(pathElapsed, kSueEntryPathDurationMillis);
		presentFrame();
	}

	if (!_skipRequested && !Engine::shouldQuit()) {
		memcpy(_sceneFramebuffer.data(), baseFramebuffer.data(), _sceneFramebuffer.size());
		drawSueEntryPathFrame(kSueEntryPathDurationMillis, kSueEntryPathDurationMillis);
		presentFrame();
	}
}

void Scene9100::drawSueEntryPathFrame(uint32 pathElapsedMillis, uint32 pathDurationMillis) {
	const int originalStartX = 0x308;
	const int originalStartY = 0x1b5;
	const int targetX = 0x11b;
	const int targetY = 0x16e;
	const uint startDescriptorIndex = kI10SceneActorDescriptorBase + 1;
	if (startDescriptorIndex >= _actorBankI10Sue.descriptors.size())
		return;

	const ActorSpriteDescriptor &startDescriptor = _actorBankI10Sue.descriptors[startDescriptorIndex];
	const int startSpriteLeft = MAX<int>(0, HollywoodEngine::kScreenWidth - (int)startDescriptor.width - 8);
	const int fixedViewportStartX = startSpriteLeft + startDescriptor.anchorX;

	const int visibleStartY = originalStartY +
		((originalStartX - fixedViewportStartX) * (targetY - originalStartY)) / (originalStartX - targetX);
	const uint32 clampedElapsed = MIN<uint32>(pathElapsedMillis, pathDurationMillis);
	const int x = fixedViewportStartX + ((targetX - fixedViewportStartX) * (int)clampedElapsed) / (int)pathDurationMillis;
	const int y = visibleStartY + ((targetY - visibleStartY) * (int)clampedElapsed) / (int)pathDurationMillis;

	const bool finalFrame = clampedElapsed >= pathDurationMillis;
	const byte facing = kI10SceneActorFacing;
	const byte cel = finalFrame ? 0 :
		(byte)(1 + ((clampedElapsed / kEntryPathFrameIntervalMillis) % 12));
	drawActorFrame(_actorBankI10Sue, facing, cel, x, y);
}

void Scene9100::drawActorFrame(const ActorSpriteBank &bank, byte facing, byte cel, int worldX, int worldY) {
	drawActorSpriteFrame(bank, facing, cel, worldX, worldY, -1,
		_sceneFramebuffer.surface(), _presentationPaletteRemapTable);
}

void Scene9100::runForegroundIdleBeat() {
	byte beatCounter = 0;
	while (beatCounter <= 20 && !_skipRequested && !Engine::shouldQuit()) {
		if (_foregroundActorFrame == 1) {
			drawForegroundActorFrame(0);
			presentFrame();
			if (delayFrame(100, kTalkingOverlayNone, 0, false, true))
				return;
			continue;
		}

		if (beatCounter == 20)
			break;
		if (_random.getRandomNumber(14) == 0) {
			drawForegroundActorFrame(1);
			presentFrame();
		}
		beatCounter++;
		if (delayFrame(100, kTalkingOverlayNone, 0, false, true)) {
			if (_skipRequested || Engine::shouldQuit())
				return;
			break;
		}
	}

	if (_skipRequested || Engine::shouldQuit())
		return;

	_effectSound.playSample(kScene9100ForegroundBeatSoundCue, 100);
	animateForegroundFrames(2, 10);
	if (_skipRequested || Engine::shouldQuit())
		return;

	drawForegroundActorFrame(0);
	presentFrame();
	delayFrame(100, kTalkingOverlayNone, 0, false, true);
}

void Scene9100::runOpeningPrelude() {
	runForegroundIdleBeat();
	if (_skipRequested || Engine::shouldQuit())
		return;
	runForegroundIdleBeat();
	if (_skipRequested || Engine::shouldQuit())
		return;

	animateForegroundFrames(11, 14);
	_effectSound.playSample(kScene9100TransitionEndSoundCue, 25);

	_foregroundTalkBaseFrame = 15;
	runConversationStep(0, 0, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);

	animateForegroundFrames(20, 20);
	_effectSound.playSample(kScene9100TransitionMiddleSoundCue, 25);
	animateForegroundFrames(21, 22);
	for (uint pulse = 0; pulse < 120 && !_skipRequested && !Engine::shouldQuit(); ++pulse) {
		if (_random.getRandomNumber(14) == 0) {
			drawForegroundActorFrame(27);
			presentFrame();
		}
		if (delayFrame(100, kTalkingOverlayNone, 0, false, true)) {
			if (_skipRequested || Engine::shouldQuit())
				return;
			break;
		}
		if (_foregroundActorFrame == 27) {
			drawForegroundActorFrame(23);
			presentFrame();
		}
		if (pulse == 99)
			_effectSound.playSample(kScene9100DelayedSoundCue, 100);
	}

	runConversationStep(0, 1, kTalkingOverlayNone, 0, false, true, kRonSecondarySpeech);
	_foregroundTalkBaseFrame = 23;
	runRonEntryConversation();
}

void Scene9100::runCinematicSequence() {
	const CinematicStep kSteps[] = {
		{ 2, 0, 3, kTalkingOverlayBase320000, 0, true, false, kInsetWhiteSpeech },
		{ 1, 1, 0, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech },
		{ 2, 1, 1, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech },
		{ 4, 1, 2, kTalkingOverlayNone, 0, false, true, kDeskPrimaryBlueSpeech },
		{ 2, 1, 3, kTalkingOverlayBase320000, 0, true, false, kInsetWhiteSpeech },
		{ 1, 1, 4, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech },
		{ 2, 1, 5, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech },
		{ 3, 1, 16, kTalkingOverlayBase640000, 0, true, false, kInsetSueSpeech },
		{ 2, 1, 8, kTalkingOverlayBase320000, 1, false, false, kInsetWhiteSpeech },
		{ 3, 1, 17, kTalkingOverlayBase640000, 1, false, false, kInsetSueSpeech },
		{ 0, 1, 9, kTalkingOverlayBase640000, 0, false, false, kInsetSueSpeech },
		{ 1, 1, 10, kTalkingOverlayBase0, 0, false, false, kInsetBlueSpeech },
		{ 2, 1, 11, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech },
		{ 3, 1, 12, kTalkingOverlayBase640000, 1, false, false, kInsetSueSpeech },
		{ 2, 1, 13, kTalkingOverlayBase320000, 1, false, false, kInsetWhiteSpeech },
		{ 4, 1, 14, kTalkingOverlayNone, 0, false, true, kDeskPrimaryBlueSpeech },
		{ 3, 1, 15, kTalkingOverlayBase640000, 0, true, false, kInsetSueSpeech }
	};

	for (uint i = 0; i < ARRAYSIZE(kSteps) && !_skipRequested && !Engine::shouldQuit(); ++i) {
		if (i == 7)
			runSueEntrySequence();
		if (_skipRequested || Engine::shouldQuit())
			return;

		applyBackgroundMode(kSteps[i]);
		const bool animateClock = kSteps[i].backgroundMode == 4;
		if (kSteps[i].textBankIndex == 1 && kSteps[i].descriptorIndex == 2)
			runForegroundPoseToDialogueState();
		else if (kSteps[i].textBankIndex == 1 && kSteps[i].descriptorIndex == 14)
			_foregroundTalkBaseFrame = 0x17;
		runConversationStep(kSteps[i].textBankIndex, kSteps[i].descriptorIndex,
			kSteps[i].talkingOverlayBase, kSteps[i].talkingOverlayVariant, kSteps[i].animateForegroundActor, animateClock,
			kSteps[i].speechTextStyle);
		if (kSteps[i].textBankIndex == 1 && kSteps[i].descriptorIndex == 2)
			runForegroundPoseBackToDeskIdle();
	}
}

void Scene9100::initializeDialogueBranchOfficeState() {
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	_foregroundActorFrame = 0x17;
	_foregroundTalkBaseFrame = 0x17;
	_deskPrimaryActorFrame = 0;
	_deskSecondaryActorFrame = 0;
	_deskPrimaryActorVisible = true;
	_deskSecondaryActorVisible = false;
	_clockChunk7Frame = 0x33;
	_clockChunk8Frame = 0x13;
	_clockChunk9Frame = 8;
	_clockChunk7CarryGate = 1;
	_clockVisible = true;
	copyDefaultPalette();
	drawOfficeCompositeLayers();
}

void Scene9100::runDialogueBranchSequence() {
	prepareDialogueBranchOfficePatch();

	runForegroundPoseToDialogueState();
	runConversationStep(3, 0, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	runForegroundPoseBackToDeskIdle();
	memcpy(_savedFramebuffer.data(), _frameDecodeBuffer.data(), _savedFramebuffer.size());

	prepareSceneFadeFrameWithScratchActor(0);
	runConversationStep(3, 1, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareChunkDFrameWithOptionalActorOverlay(true);
	runConversationStep(3, 2, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareSceneFadeFrameWithScratchActor(0);
	runConversationStep(3, 3, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareChunkDFrameWithOptionalActorOverlay(true);
	runConversationStep(3, 4, kTalkingOverlayBase0, 1, false, false, kInsetBlueSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	prepareSceneFadeFrameWithScratchActor(0);
	runConversationStep(3, 5, kTalkingOverlayBase320000, 0, false, false, kInsetWhiteSpeech);
	if (_skipRequested || Engine::shouldQuit())
		return;

	restoreOfficeFrameAndPresent();
	_foregroundTalkBaseFrame = 0x17;
	drawForegroundActorFrame(_foregroundTalkBaseFrame);
	presentFrame();
	runConversationStep(3, 6, kTalkingOverlayNone, 0, true, true, kDeskPrimaryBlueSpeech);
}

void Scene9100::prepareDialogueBranchOfficePatch() {
	expandFillRunsToSavedFramebuffer();
	drawResourceBlockListToSceneFramebuffer(_resources._chunkOffsets[17]);
	drawOfficeCompositeLayers();
	presentFrame();
}

void Scene9100::runForegroundPoseToDialogueState() {
	animateForegroundFrames(27, 31);
	_foregroundTalkBaseFrame = 32;
}

void Scene9100::runForegroundPoseBackToDeskIdle() {
	animateForegroundFrames(36, 40);
	_foregroundTalkBaseFrame = 0x17;
	drawForegroundActorFrame(_foregroundTalkBaseFrame);
	presentFrame();
}

void Scene9100::prepareSceneFadeFrameWithScratchActor(byte talkingOverlayVariant) {
	applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(9));
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	copyPaletteSegment(6);
	drawTalkingOverlay(kTalkingOverlayBase320000, 0, talkingOverlayVariant);
	presentFrame();
}

void Scene9100::prepareChunkDFrameWithOptionalActorOverlay(bool actorOverlayEnabled) {
	applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(8));
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	copyPaletteSegment(5);
	if (actorOverlayEnabled)
		drawTalkingOverlay(kTalkingOverlayBase0, 0, 1);
	presentFrame();
}

void Scene9100::runEndingWipe() {
	for (uint sweepOffset = 0; sweepOffset < 240 && !_skipRequested && !Engine::shouldQuit(); sweepOffset += 20) {
		clearSceneFramebufferBand(sweepOffset, 20);
		presentFrame();
		if (delay(50))
			return;
	}
}

void Scene9100::runConversationStep(uint16 textBankIndex, byte descriptorIndex, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, const SpeechTextStyle &speechTextStyle, bool animateInsetActor, byte insetTalkBaseFrame) {
	_talkingFrame = 0;
	_talkingFrameAccumulator = 0;
	const SceneSpeechCue popup = _text.stageCue(textBankIndex, descriptorIndex);
	const uint segmentCount = MAX<uint>(1, popup.continuationCount);
	for (uint segmentIndex = 0; segmentIndex < segmentCount && !_skipRequested && !Engine::shouldQuit(); ++segmentIndex) {
		const uint16 sampleId = popup.voiceSampleId + segmentIndex;
		beginSubtitle(popup, segmentIndex, speechTextStyle);
		presentFrame();
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 fallbackMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) : 1200;
		waitForSpeechOrDelay(fallbackMillis, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor, animateClock, animateInsetActor, insetTalkBaseFrame);
		_speech.stop();
		clearSubtitle();
		if (segmentIndex + 1 < segmentCount && !_skipRequested && !Engine::shouldQuit())
			delayFrame(375, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor, animateClock, animateInsetActor, insetTalkBaseFrame);
	}

	if (!_skipRequested && !Engine::shouldQuit()) {
		if (talkingOverlayBase != kTalkingOverlayNone) {
			drawTalkingOverlay(talkingOverlayBase, 0, talkingOverlayVariant);
			presentFrame();
		} else if (animateInsetActor) {
			drawPersistentDeskActors();
			presentFrame();
		} else if (animateForegroundActor) {
			drawForegroundActorFrame(_foregroundTalkBaseFrame);
			presentFrame();
		}
	}
}

void Scene9100::waitForSpeechOrDelay(uint32 fallbackMillis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor, byte insetTalkBaseFrame) {
	uint32 elapsed = 0;
	while (!_skipRequested && !Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= fallbackMillis)
			break;

		if (delayFrame(50, talkingOverlayBase, talkingOverlayVariant, animateForegroundActor, animateClock, animateInsetActor, insetTalkBaseFrame))
			return;
		elapsed += 50;
	}
}

void Scene9100::beginSubtitle(const SceneSpeechCue &popup, uint segmentIndex, const SpeechTextStyle &speechTextStyle) {
	clearSubtitle();

	const Common::String text = _text.largeTextRecord(popup.textRecordId + segmentIndex);
	if (text.empty()) {
		debugC(2, kDebugScene, "Skipping empty subtitle text record %u",
			popup.textRecordId + segmentIndex);
		return;
	}

	if (!showAnchoredSubtitle(text, speechTextStyle.colorIndex,
			speechTextStyle.centerX, speechTextStyle.topY))
		return;

	if (speechTextStyle.updatePalette) {
		_paletteCurrent[speechTextStyle.colorIndex * 3] = speechTextStyle.red;
		_paletteCurrent[speechTextStyle.colorIndex * 3 + 1] = speechTextStyle.green;
		_paletteCurrent[speechTextStyle.colorIndex * 3 + 2] = speechTextStyle.blue;
	}

}

void Scene9100::drawInitialForegroundFrame() {
	drawForegroundActorFrame(0);
}

void Scene9100::drawForegroundActorFrame(byte frameIndex) {
	if (frameIndex >= ARRAYSIZE(kI10ForegroundFrameRemap))
		return;

	_foregroundActorFrame = frameIndex;
	restoreForegroundActorLayer();
	drawOfficeCompositeLayers();
}

void Scene9100::restoreForegroundActorLayer() {
	if (_foregroundActorFrame >= ARRAYSIZE(kI10ForegroundFrameRemap))
		return;

	const uint16 descriptorIndex = kI10ForegroundFrameRemap[_foregroundActorFrame];
	restoreSpriteBackground(_resources._arena, _resources._chunkOffsets[5], 0, kI10ForegroundDescriptorCount, descriptorIndex);
}

void Scene9100::drawForegroundActorLayer() {
	if (_foregroundActorFrame >= ARRAYSIZE(kI10ForegroundFrameRemap))
		return;

	const uint16 descriptorIndex = kI10ForegroundFrameRemap[_foregroundActorFrame];
	drawStripSpriteFrame(_resources._arena, _resources._chunkOffsets[5], 0, kI10ForegroundDescriptorCount, descriptorIndex);
}

void Scene9100::drawDeskActorLayer(uint32 baseOffset, uint16 descriptorCount, byte frameIndex, bool restoreBackground) {
	if (frameIndex >= descriptorCount)
		return;

	if (restoreBackground)
		restoreSpriteBackground(_secondaryScratchBuffer, baseOffset, 0, descriptorCount, frameIndex);
	drawStripSpriteFrame(_secondaryScratchBuffer, baseOffset, 0, descriptorCount, frameIndex);
}

void Scene9100::drawDeskPrimaryStaticFrame(byte frameIndex, bool restoreBackground) {
	_deskPrimaryActorFrame = MIN<byte>(frameIndex, kI10DeskPrimaryStaticDescriptorCount - 1);
	if (restoreBackground)
		drawDeskPrimaryStaticLayer(true);
	drawOfficeCompositeLayers();
}

void Scene9100::drawDeskSecondaryStaticFrame(byte frameIndex, bool restoreBackground) {
	_deskSecondaryActorFrame = MIN<byte>(frameIndex, kI10DeskSecondaryStaticDescriptorCount - 1);
	if (restoreBackground)
		drawDeskSecondaryStaticLayer(true);
	drawOfficeCompositeLayers();
}

void Scene9100::drawDeskPrimaryStaticLayer(bool restoreBackground) {
	if (!_deskPrimaryActorVisible)
		return;

	drawDeskActorLayer(kDeskPrimaryStaticBase, kI10DeskPrimaryStaticDescriptorCount,
		_deskPrimaryActorFrame, restoreBackground);
}

void Scene9100::drawDeskSecondaryStaticLayer(bool restoreBackground) {
	if (!_deskSecondaryActorVisible)
		return;

	drawDeskActorLayer(kDeskSecondaryStaticBase, kI10DeskSecondaryStaticDescriptorCount,
		_deskSecondaryActorFrame, restoreBackground);
}

void Scene9100::drawPersistentDeskActors() {
	drawDeskSecondaryStaticLayer(false);
	drawDeskPrimaryStaticLayer(false);
}

void Scene9100::drawOfficeCompositeLayers() {
	drawForegroundActorLayer();
	drawPersistentDeskActors();
	drawClockLayers(false);
}

void Scene9100::syncOfficeRestoreBaseFromSaved() {
	memcpy(_frameDecodeBuffer.data(), _savedFramebuffer.data(), _frameDecodeBuffer.size());
}

void Scene9100::animateForegroundFrames(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawForegroundActorFrame(frame);
		presentFrame();
		if (delayFrame(100, kTalkingOverlayNone, 0, false, true))
			return;
	}
}

void Scene9100::animateDeskPrimaryStaticFrames(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawDeskPrimaryStaticFrame(frame);
		presentFrame();
		if (delayFrame(100, kTalkingOverlayNone, 0, false, true))
			return;
	}
}

void Scene9100::animateDeskSecondaryStaticFrames(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		drawDeskSecondaryStaticFrame(frame);
		presentFrame();
		if (delayFrame(50, kTalkingOverlayNone, 0, false, true))
			return;
	}
}

void Scene9100::advanceClockFrame() {
	_clockVisible = true;
	_clockChunk9Frame = (_clockChunk9Frame + 1) % kI10ClockDescriptorCount;
	if (_clockChunk9Frame == 0) {
		_clockChunk8Frame = (_clockChunk8Frame + 1) % kI10ClockDescriptorCount;
		if (_clockChunk7CarryGate == 5) {
			_clockChunk7Frame = (_clockChunk7Frame + 1) % kI10ClockDescriptorCount;
			_clockChunk7CarryGate = 0;
		}
	}
	restoreClockAreaBackground();
	drawOfficeCompositeLayers();
	_clockSound.playSample(kScene9100ClockSoundCue, 50);
}

void Scene9100::restoreClockAreaBackground() {
	for (byte frame = 0; frame < kI10ClockDescriptorCount; ++frame) {
		restoreSpriteBackground(_resources._arena, _resources._chunkOffsets[7], 0, kI10ClockDescriptorCount, frame);
		restoreSpriteBackground(_resources._arena, _resources._chunkOffsets[8], 0, kI10ClockDescriptorCount, frame);
		restoreSpriteBackground(_resources._arena, _resources._chunkOffsets[9], 0, kI10ClockDescriptorCount, frame);
	}
}

void Scene9100::drawClockLayers(bool restoreBackground) {
	if (!_clockVisible)
		return;

	if (restoreBackground)
		restoreClockAreaBackground();
	drawStripSpriteFrame(_resources._arena, _resources._chunkOffsets[7], 0, kI10ClockDescriptorCount, _clockChunk7Frame);
	drawStripSpriteFrame(_resources._arena, _resources._chunkOffsets[8], 0, kI10ClockDescriptorCount, _clockChunk8Frame);
	drawStripSpriteFrame(_resources._arena, _resources._chunkOffsets[9], 0, kI10ClockDescriptorCount, _clockChunk9Frame);
}

void Scene9100::drawTalkingOverlay(TalkingOverlayBase talkingOverlayBase, byte frameIndex, byte talkingOverlayVariant) {
	if (talkingOverlayBase == kTalkingOverlayNone)
		return;

	_talkingFrame = frameIndex % 5;
	_lastTalkingFrameVariant = _talkingFrame;
	const uint16 descriptorIndex = _talkingFrame + (5 * (talkingOverlayVariant != 0 ? 1 : 0));
	const uint32 baseOffset = (uint32)talkingOverlayBase;
	restoreSpriteBackground(_resourceScratchArena, baseOffset, 0, kI10TalkingOverlayDescriptorCount, descriptorIndex);
	drawStripSpriteFrame(_resourceScratchArena, baseOffset, 0, kI10TalkingOverlayDescriptorCount, descriptorIndex);
}

void Scene9100::drawStripSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex) {
	Hollywood::drawStripSpriteFrame(resource, baseOffset, descriptorTableOffset,
		descriptorCount, descriptorIndex, _sceneFramebuffer.surface());
}

void Scene9100::restoreSpriteBackground(const Common::Array<byte> &resource, uint32 baseOffset, uint32 descriptorTableOffset, uint16 descriptorCount, uint16 descriptorIndex) {
	Hollywood::restoreSpriteBackground(resource, baseOffset, descriptorTableOffset,
		descriptorCount, descriptorIndex, _frameDecodeBuffer.surface(), _sceneFramebuffer.surface());
}

void Scene9100::applyResourceSpanPatchToFrameDecodeBuffer(uint32 baseOffset) {
	Hollywood::drawResourceBlockList(_resources._arena, baseOffset, _frameDecodeBuffer.surface());
}

void Scene9100::drawResourceBlockListToSceneFramebuffer(uint32 baseOffset) {
	Hollywood::drawResourceBlockList(_resources._arena, baseOffset, _sceneFramebuffer.surface());
}

void Scene9100::restoreResourceBlockListFromCleanOfficeBase(uint32 baseOffset, IndexedSurfaceBuffer &destination) {
	if (baseOffset + 2 > _resources._arena.size())
		return;

	const uint16 blockCount = readUint16(_resources._arena, baseOffset);
	uint cursor = baseOffset + 2;
	for (uint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
		if (cursor + 6 > _resources._arena.size())
			return;

		const uint32 packedDestination = readUint32(_resources._arena, cursor);
		const uint16 size = readUint16(_resources._arena, cursor + 4);
		cursor += 6;

		const uint x = packedDestination & 0xffff;
		const uint y = (packedDestination >> 16) & 0xffff;
		const uint targetOffset = x + y * HollywoodEngine::kSceneBufferWidth;
		if (cursor + size > _resources._arena.size() ||
				targetOffset + size > destination.size() ||
				targetOffset + size > _cleanOfficeBaseFramebuffer.size())
			return;

		memcpy(destination.data() + targetOffset, _cleanOfficeBaseFramebuffer.data() + targetOffset, size);
		cursor += size;
	}
}

void Scene9100::removeInitialOfficeTitlePatch(IndexedSurfaceBuffer &destination) {
	if (_dialogueBranch)
		return;

	restoreResourceBlockListFromCleanOfficeBase(_resources._chunkOffsets[16], destination);
}

void Scene9100::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	while (destinationOffset < _savedFramebuffer.size() && sourceOffset + 3 <= _sceneFillRuns.size()) {
		const byte fill = _sceneFillRuns[sourceOffset];
		const uint16 runLength = readUint16(_sceneFillRuns, sourceOffset + 1);
		sourceOffset += 3;
		if (runLength == 0)
			break;

		const uint clampedRunLength = MIN<uint>(runLength, _savedFramebuffer.size() - destinationOffset);
		memset(_savedFramebuffer.data() + destinationOffset, fill, clampedRunLength);
		destinationOffset += clampedRunLength;
	}
}

void Scene9100::restoreOfficeFrameAndPresent() {
	syncOfficeRestoreBaseFromSaved();
	removeInitialOfficeTitlePatch(_frameDecodeBuffer);
	expandFillRunsToSavedFramebuffer();
	memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
	drawForegroundActorFrame(_foregroundActorFrame);
	copyDefaultPalette();
	presentFrame();
}

void Scene9100::applyBackgroundMode(const CinematicStep &step) {
	if (step.copyFrameToSavedBefore) {
		memcpy(_savedFramebuffer.data(), _frameDecodeBuffer.data(), _savedFramebuffer.size());
		removeInitialOfficeTitlePatch(_savedFramebuffer);
	}

	switch (step.backgroundMode) {
	case 1:
		applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(8));
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(5);
		break;
	case 2:
		applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(9));
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(6);
		break;
	case 3:
		applyResourceSpanPatchToFrameDecodeBuffer(getSegmentOffset(10));
		memcpy(_sceneFramebuffer.data(), _frameDecodeBuffer.data(), _sceneFramebuffer.size());
		copyPaletteSegment(7);
		break;
	case 4:
		restoreOfficeFrameAndPresent();
		break;
	default:
		break;
	}

	const bool drewInitialOverlay = step.talkingOverlayBase != kTalkingOverlayNone;
	if (drewInitialOverlay)
		drawTalkingOverlay(step.talkingOverlayBase, 0, step.talkingOverlayVariant);

	if ((step.backgroundMode != 0 && step.backgroundMode != 4) || drewInitialOverlay)
		presentFrame();
}

void Scene9100::copyPaletteSegment(byte segmentIndex) {
	const uint32 sourceOffset = getSegmentOffset(segmentIndex);
	if (sourceOffset + kPaletteSize > _resources._arena.size())
		return;

	memcpy(_paletteCurrent.data(), _resources._arena.data() + sourceOffset, kPaletteSize);
	buildPresentationPaletteRemapTable(_paletteCurrent, _presentationPaletteRemapTable);
}

void Scene9100::copyDefaultPalette() {
	memcpy(_paletteCurrent.data(), _paletteDefault.data(), _paletteCurrent.size());
	buildPresentationPaletteRemapTable(_paletteCurrent, _presentationPaletteRemapTable);
}

bool Scene9100::delayFrame(uint32 millis, TalkingOverlayBase talkingOverlayBase, byte talkingOverlayVariant, bool animateForegroundActor, bool animateClock, bool animateInsetActor, byte insetTalkBaseFrame) {
	TimedPresentationLoop loop(*this, millis);
	while (loop.beginFrame()) {
		const uint32 slice = loop.finishFrame();
		bool dirty = false;
		if (animateClock && advanceClockTimer(slice)) {
			advanceClockFrame();
			dirty = true;
		}
		const bool animateTalkingActor = talkingOverlayBase != kTalkingOverlayNone ||
			animateInsetActor || animateForegroundActor;
		const bool talkingFrameDue = animateTalkingActor && advanceTalkingTimer(slice);
		if (talkingOverlayBase != kTalkingOverlayNone && talkingFrameDue) {
			drawTalkingOverlay(talkingOverlayBase, nextTalkingFrameVariant(), talkingOverlayVariant);
			dirty = true;
		}
		if (animateInsetActor && talkingOverlayBase == kTalkingOverlayNone && talkingFrameDue) {
			drawPersistentDeskActors();
			dirty = true;
		} else if (animateForegroundActor && talkingOverlayBase == kTalkingOverlayNone && talkingFrameDue) {
			drawForegroundActorFrame((byte)(_foregroundTalkBaseFrame + nextTalkingFrameVariant()));
			dirty = true;
		}
		if (dirty)
			presentFrame();
	}

	return consumeStepAdvanceRequest() || _skipRequested || Engine::shouldQuit();
}

bool Scene9100::advanceClockTimer(uint32 millis) {
	_clockFrameAccumulator += millis;
	if (_clockFrameAccumulator < kClockFrameIntervalMillis)
		return false;

	_clockFrameAccumulator %= kClockFrameIntervalMillis;
	return true;
}

bool Scene9100::advanceTalkingTimer(uint32 millis) {
	_talkingFrameAccumulator += millis;
	if (_talkingFrameAccumulator < kTalkingFrameIntervalMillis)
		return false;

	_talkingFrameAccumulator %= kTalkingFrameIntervalMillis;
	return true;
}

void Scene9100::stopAudio() {
	clearSubtitle();
	_music.stop();
	_speech.stop();
	_effectSound.stop();
	_clockSound.stop();
	_ambientSound.stop();
}

byte Scene9100::nextTalkingFrameVariant() {
	byte nextFrame = 0;
	do {
		nextFrame = (byte)_random.getRandomNumber(4);
	} while (nextFrame == _lastTalkingFrameVariant);

	_lastTalkingFrameVariant = nextFrame;
	return nextFrame;
}

uint32 Scene9100::getSegmentOffset(byte segmentIndex) const {
	const uint chunkIndex = 5 + segmentIndex;
	if (chunkIndex >= SceneResources::kResourceChunkCount)
		return 0;

	return _resources._chunkOffsets[chunkIndex];
}

uint16 Scene9100::readUint16(const Common::Array<byte> &source, uint offset) const {
	if (offset + 2 > source.size())
		return 0;

	return source[offset] | (source[offset + 1] << 8);
}

int16 Scene9100::readSint16(const Common::Array<byte> &source, uint offset) const {
	return (int16)readUint16(source, offset);
}

uint32 Scene9100::readUint32(const Common::Array<byte> &source, uint offset) const {
	if (offset + 4 > source.size())
		return 0;

	return source[offset] |
		(source[offset + 1] << 8) |
		(source[offset + 2] << 16) |
		(source[offset + 3] << 24);
}

} // End of namespace Hollywood
