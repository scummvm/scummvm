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

#include "hollywood/scenes/playable/scene3030.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene3030ArchiveName = "RESOURCE.C03";
const char *const kScene3030MusicArchiveName = "RESOURCE.M03";
const char *const kScene3030SoundArchiveName = "RESOURCE.S03";
const uint kScene3030InitialRequiredChunkCount = 11;
const uint kScene3030ArenaFirstChunk = 5;
const uint kScene3030ArenaLastChunk = 10;
const uint kScene3030StageIndex = 303;
const uint16 kScene3030FirstState = 0x0bd6;
const uint16 kScene3030LastState = 0x0bdf;
const uint16 kScene3020EntryFromScene3030State = 0x0bcd;
const uint16 kScene3030ViewportXOffset = 0x0060;
const uint kScene3030ActorBankTableEntry = 0x0000;
const uint kScene3030ActorPaletteTableEntry = 0x00cc;
const uint kScene3030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3030LoopFrameMillis = 75;
const uint32 kScene3030MachineFrameMillis = 75;
const uint32 kScene3030TransitionFrameMillis = 75;
const uint kScene3030LoopDescriptorCount = 0x0c;
const uint kScene3030MachineEffectDescriptorCount = 0x19;
const uint kScene3030MachineActionDescriptorCount = 0x0c;
const uint kScene3030EntryTransitionChunk = 12;
const uint kScene3030EntryTransitionTableEntryCount = 0x20;
const byte kScene3030EntryTransitionFinalFrame = 0x1f;
const uint kScene3030ReturnTransitionChunk = 11;
const uint kScene3030ReturnTransitionTableEntryCount = 0x15;
const byte kScene3030ReturnTransitionFinalFrame = 0x14;
const byte kScene3030RequiredInventoryItem = 0x1f;
const byte kScene3030ResultInventoryItem = 0x41;

const byte kScene3030LoopFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene3030MachineEffectFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24
};

const byte kScene3030MachineActionFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 2,
	1, 0, 0, 0, 0, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11
};

PlayableSceneConfig scene3030Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene3030ArchiveName;
	config.initialRequiredChunkCount = kScene3030InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene3030ArenaFirstChunk;
	config.arenaLastChunk = kScene3030ArenaLastChunk;
	config.stageIndex = kScene3030StageIndex;
	config.debugName = "Scene 3030";
	config.viewportXOffset = kScene3030ViewportXOffset;
	config.viewportMinXOffset = kScene3030ViewportXOffset;
	config.viewportMaxXOffset = kScene3030ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 3;
	config.actorBankTableEntry = kScene3030ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene3030ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene3030Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene3030SpeechCueDescriptorTableOffset;
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene3030MusicArchiveName;
	config.soundBank0ArchiveName = kScene3030SoundArchiveName;
	config.mainFlowFirstState = kScene3030FirstState;
	config.mainFlowLastState = kScene3030LastState;
	return config;
}

Scene3030::Scene3030(HollywoodEngine *vm) :
		PlayableScene(vm, scene3030Config(), "scene3030", 0x110, 0x18a, 2, 0xfd, 0xfb),
		_loopChannel(),
		_loopLayer(),
		_machineEffectLayer(),
		_machineActionLayer(),
		_machineSequenceActive(false) {
	_loopLayer.configure(6, kScene3030LoopDescriptorCount,
		kScene3030LoopFrameMap, ARRAYSIZE(kScene3030LoopFrameMap));
	_machineEffectLayer.configure(9, kScene3030MachineEffectDescriptorCount,
		kScene3030MachineEffectFrameMap, ARRAYSIZE(kScene3030MachineEffectFrameMap));
	_machineActionLayer.configure(10, kScene3030MachineActionDescriptorCount,
		kScene3030MachineActionFrameMap, ARRAYSIZE(kScene3030MachineActionFrameMap));
}

bool Scene3030::hasCustomPreviewState() const {
	return true;
}

void Scene3030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	_activeActorWorldX = 0x110;
	_activeActorWorldY = 0x18a;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene3030::hasCustomComposite() const {
	return true;
}

void Scene3030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	(void)activeWorldY;

	copyBaseFramebufferToSceneFramebuffer();
	if (_vm->gameState().windmillBladesMoving)
		drawResourceSpriteLayer(_loopLayer);
	if (_machineSequenceActive) {
		drawResourceSpriteLayer(_machineEffectLayer);
		drawResourceSpriteLayer(_machineActionLayer);
		drawForegroundBlocks();
		return;
	}

	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks();
}

bool Scene3030::hasCustomEntrySequence() const {
	return _vm->gameState().mainFlowStateId == kScene3030FirstState;
}

void Scene3030::runCustomEntrySequence() {
	runEntryFromScene3020();
}

bool Scene3030::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene3030::advanceCustomGameplayLoop(uint32 delta) {
	if (_vm->gameState().windmillBladesMoving)
		advanceLoopingLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a camino anterior (go to previous forest area): return to scene 3020.
		runExitToScene3020();
		return true;
	case 302: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar/usar maquinaria (look/use machine), state-aware.
		beginSecondarySpeechLine(4, _vm->gameState().windmillBladesMoving ? 1 : 0);
		return true;
	case 307: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 312: // Usar objeto 0x1f con maquinaria: activates the windmill machinery.
		runMachineActivationSequence();
		return true;
	default:
		return false;
	}
}

bool Scene3030::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)targetX;
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 4 && state.drawOrderMode < 4) {
		requestedFacing = 1;
		return true;
	}

	return false;
}

bool Scene3030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 0) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		promoteMachineHotspots();
		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3030::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	if (_vm->gameState().windmillBladesMoving) {
		profile.soundMode = kAmbientSoundLoop;
		profile.soundCueId = 0x0e;
		profile.soundVolumePercent = 30;
	}
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3030::resetAnimationLayers() {
	_loopChannel.reset(0, kScene3030LoopFrameMillis);
	_loopLayer.visible = true;
	_loopLayer.reset(0);
	_machineEffectLayer.visible = false;
	_machineActionLayer.visible = false;
	_machineEffectLayer.reset(0);
	_machineActionLayer.reset(0);
	_machineSequenceActive = false;
}

void Scene3030::advanceLoopingLayer(uint32 delta) {
	const uint frameCount = _loopChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_loopChannel.frameIndex = _loopChannel.frameIndex + 1 < ARRAYSIZE(kScene3030LoopFrameMap) ?
			_loopChannel.frameIndex + 1 : 0;
		_loopLayer.setFrame(_loopChannel.frameIndex);
	}
}

void Scene3030::drawForegroundBlocks() {
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene3030::promoteMachineHotspots() {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	if (!_vm->gameState().scene3030MachineActivated) {
		for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
			if (_paletteMask[kSceneColorToItemMap + i] == 7)
				_paletteMask[kSceneColorToItemMap + i] = 2;
		}
		if (_sceneChunkTable.isValidChunk(7))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		return;
	}

	if (_sceneChunkTable.isValidChunk(8))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == 2)
			_paletteMask[kSceneColorToItemMap + i] = 0;
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == 7)
			_paletteMask[kSceneColorToItemMap + i] = 3;
	}
}

void Scene3030::runEntryFromScene3020() {
	runEntryPath(0x110, 0x18a, 2, 0x110, 0x18a);
	runDeltaTransitionClip(kScene3030EntryTransitionChunk,
		kScene3030EntryTransitionTableEntryCount, kScene3030EntryTransitionFinalFrame);
}

void Scene3030::runExitToScene3020() {
	drawPlayableComposite();
	presentFrame();
	runDeltaTransitionClip(kScene3030ReturnTransitionChunk,
		kScene3030ReturnTransitionTableEntryCount, kScene3030ReturnTransitionFinalFrame);
	_vm->gameState().mainFlowStateId = kScene3020EntryFromScene3030State;
}

void Scene3030::runDeltaTransitionClip(uint chunkIndex, uint tableEntryCount, byte finalFrameIndex) {
	Common::Array<byte> clipData;
	if (!loadVariableChunk(chunkIndex, clipData))
		return;

	Graphics::ManagedSurface transitionBackground;
	transitionBackground.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(true, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
		false, 0, 0, 0, 0, -1);
	drawForegroundBlocks();
	transitionBackground.copyRectToSurface(_sceneFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	uint32 frameAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	byte frameIndex = 0;

	drawDeltaTransitionFrame(clipData, tableEntryCount, frameIndex, *transitionBackground.surfacePtr());
	presentFrame();

	while (frameIndex < finalFrameIndex && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		if (_vm->gameState().windmillBladesMoving)
			advanceLoopingLayer(delta);
		updateAmbientAudioAndMusicCues(delta);

		bool frameDirty = false;
		while (frameAccumulator >= kScene3030TransitionFrameMillis && frameIndex < finalFrameIndex) {
			frameAccumulator -= kScene3030TransitionFrameMillis;
			++frameIndex;
			frameDirty = true;
		}

		if (frameDirty) {
			drawDeltaTransitionFrame(clipData, tableEntryCount, frameIndex, *transitionBackground.surfacePtr());
			presentFrame();
		}

		g_system->delayMillis(10);
	}
}

void Scene3030::drawDeltaTransitionFrame(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::Surface &transitionBackground) {
	_sceneFramebuffer.copyRectToSurface(transitionBackground, 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	if (_vm->gameState().windmillBladesMoving)
		drawResourceSpriteLayer(_loopLayer);
	drawClipFrameDeltaToSurface(clipData, tableEntryCount, frameIndex, *_sceneFramebuffer.surfacePtr());
	drawClipFrameDeltaToSurface(clipData, tableEntryCount, frameIndex, transitionBackground);
}

void Scene3030::drawClipFrameDeltaToSurface(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::Surface &destination) {
	const uint32 tableEntryOffset = (uint32)frameIndex * 4;
	if (tableEntryOffset + 4 > clipData.size())
		return;

	const uint32 frameOffset = ((uint32)tableEntryCount * 4) + readUint32LE(clipData, tableEntryOffset);
	if (frameOffset + 4 > clipData.size())
		return;

	const uint16 firstRow = readUint16LE(clipData, frameOffset);
	const uint16 lastRow = readUint16LE(clipData, frameOffset + 2);
	uint cursor = frameOffset + 4;
	byte *pixels = (byte *)destination.getPixels();
	const uint size = destination.pitch * destination.h;

	for (uint row = firstRow; row <= lastRow && row < (uint)destination.h; ++row) {
		if (cursor >= clipData.size())
			return;

		byte runCount = clipData[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > clipData.size())
				return;

			const uint x = readUint16LE(clipData, cursor);
			const byte literalLength = clipData[cursor + 2];
			const uint destinationOffset = row * destination.pitch + x;
			if (destinationOffset >= size)
				return;

			if (literalLength == 0) {
				if (cursor + 5 > clipData.size())
					return;

				const byte fillValue = clipData[cursor + 3];
				const uint fillLength = clipData[cursor + 4];
				cursor += 5;
				if (destinationOffset + fillLength <= size)
					memset(pixels + destinationOffset, fillValue, fillLength);
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > clipData.size())
					return;

				if (destinationOffset + literalLength <= size)
					memcpy(pixels + destinationOffset, clipData.data() + literalOffset, literalLength);
				cursor = literalOffset + literalLength;
			}
		}
	}
}

void Scene3030::runMachineActivationSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene3030MachineActivated) {
		beginSecondarySpeechLine(4, 1);
		return;
	}
	if (!hasInventoryItem(kScene3030RequiredInventoryItem)) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	beginSecondarySpeechLine(11, 0);
	_machineSequenceActive = true;
	_machineEffectLayer.visible = false;
	_machineActionLayer.visible = true;

	uint machineEffectFrame = 0;
	for (uint frame = 0; frame < ARRAYSIZE(kScene3030MachineActionFrameMap) && !Engine::shouldQuit(); ++frame) {
		_machineActionLayer.setFrame(frame);
		if (frame == 15) {
			_soundBank0.playSample(0x1d, 100);
			state.scene3030MachineActivated = true;
			applySceneStateToHotspotsAndPatches(0);
		}
		if (frame >= 15 && machineEffectFrame < ARRAYSIZE(kScene3030MachineEffectFrameMap)) {
			_machineEffectLayer.visible = true;
			_machineEffectLayer.setFrame(machineEffectFrame);
			if (machineEffectFrame == 5)
				state.windmillBladesMoving = true;
			++machineEffectFrame;
		}
		drawMachineSequenceFrame();
		if (waitSceneMillis(kScene3030MachineFrameMillis))
			break;
	}

	while (machineEffectFrame < ARRAYSIZE(kScene3030MachineEffectFrameMap) && !Engine::shouldQuit()) {
		_machineEffectLayer.visible = true;
		_machineEffectLayer.setFrame(machineEffectFrame++);
		drawMachineSequenceFrame();
		if (waitSceneMillis(kScene3030MachineFrameMillis))
			break;
	}

	_machineSequenceActive = false;
	_machineEffectLayer.visible = false;
	_machineActionLayer.visible = false;
	removeInventoryItem(kScene3030RequiredInventoryItem);
	addInventoryItem(kScene3030ResultInventoryItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(11, 1);
	drawPlayableComposite();
	presentFrame();
}

void Scene3030::drawMachineSequenceFrame() {
	if (_vm->gameState().windmillBladesMoving)
		advanceLoopingLayer(kScene3030MachineFrameMillis);
	drawPlayableComposite();
	presentFrame();
}

} // End of namespace Hollywood
