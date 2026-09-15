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
#include "hollywood/scenes/playable/scene3110.h"

namespace Hollywood {

const char *const kScene3110ArchiveName = "RESOURCE.C11";
const char *const kScene3110SoundArchiveName = "RESOURCE.S03";
const uint16 kScene3110LongBranchState = 0x0c26;
const uint16 kScene3110ReturnToScene3070OtherSideState = 0x0bff;
const uint16 kScene3110ReturnToScene3070LateBranchState = 0x0c00;
const uint16 kScene3110MachineRoomViewportXOffset = 0x0068;
const uint32 kScene3110DefaultFrameMillis = 75;
const uint32 kScene3110ExteriorCounterMillis = 40;
const uint32 kScene3110ExteriorBladeFrameMillis = 125;
const uint32 kScene3110ExteriorWingFrameMillis = 60;
const uint32 kScene3110ExteriorPaletteStepMillis = 150;
const uint32 kScene3110ExteriorLightningFlashMillis = 75;
const uint32 kScene3110MemoryPalettePulseMillis = 10;
const uint32 kScene3110MemoryPaletteBandMillis = 300;
const uint32 kScene3110MachineRoomPollMillis = 10;
const uint32 kScene3110MachineRoomInitialGateMillis = 100;
const uint32 kScene3110MachineRoomFinalCounterMillis = 40;
const uint32 kScene3110MachineRoomLeftMemoryActorMillis = 60;
const uint32 kScene3110MachineRoomRightMemoryActorMillis = 75;
const uint32 kScene3110MachineRoomLeftArcMillis = 75;
const uint32 kScene3110MachineRoomRightArcMillis = 60;
const uint32 kScene3110MachineRoomRevealMillis = 50;
const byte kScene3110MachineRoomCurtainBandHeight = 0x14;
const int kScene3110MachineRoomCurtainStartY = 0xdc;
const byte kScene3110MachineRoomInitialGateEndFrame = 10;
const uint kScene3110MachineRoomFinalCounterLimit = 100;
const byte kScene3110SoundCueLightning = 0x0d;
const byte kScene3110ExteriorBladeFrameCount = 8;
const byte kScene3110ExteriorWingStartCounter = 20;
const byte kScene3110ExteriorStormStartCounter = 30;
const byte kScene3110ExteriorStormTailCounter = 30;

const byte kScene3110ExteriorForegroundFrameMap[] = {
	4, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0,
	1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2,
	3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4
};

const byte kScene3110ShortFinalFrameMap[] = {
	4, 4, 4, 3, 2, 1, 0, 0, 0, 0, 1, 2, 3, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0
};

const byte kScene3110ShortInitialFrameMap[] = {
	0, 1, 2, 3, 4, 5, 4, 3, 2, 3, 4, 5, 6, 7
};

const byte kScene3110BlackFlashFrameMap[] = {
	0, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3,
	2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3,
	2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 0
};

const byte kScene3110CloseupFrameMap[] = {
	0, 1, 2, 1, 0
};

Scene3110::Scene3110(HollywoodEngine *vm) :
		PresentationScene(vm, "scene 3110"),
		_spriteSequenceTracks(nullptr),
		_spriteSequenceTrackCount(0),
		_spriteSequenceViewportX(0),
		_spriteSequenceTick(0),
		_memoryRandomState(0x3110),
		_memoryPulseLevel(0),
		_memoryPulseActive(false) {
	_memoryPulseSavedPalette.resize(kPaletteSize);
	_baseFramebuffer.resize(kSceneBufferByteCount);
	_sound0.setArchive(Common::Path(kScene3110SoundArchiveName));
	_sound1.setArchive(Common::Path(kScene3110SoundArchiveName));
	_sound2.setArchive(Common::Path(kScene3110SoundArchiveName));
}

Scene3110::~Scene3110() {
	stopSounds();
}

bool Scene3110::play() {
	if (!_resources.loadChunkTable(kScene3110ArchiveName))
		return false;

	GameplayState &state = _vm->gameState();
	state.activeAudioChapterIndex = 3;
	state.currentInventoryOwnerIndex = 0;

	const uint16 entryState = state.mainFlowStateId;
	if (entryState == kScene3110LongBranchState) {
		runLongBranch();
		if (!Engine::shouldQuit())
			state.mainFlowStateId = kScene3110ReturnToScene3070OtherSideState;
	} else {
		runShortBranch();
		if (!Engine::shouldQuit())
			state.mainFlowStateId = kScene3110ReturnToScene3070LateBranchState;
	}

	stopSounds();
	return true;
}

bool Scene3110::loadFramebufferAndPalette(uint framebufferChunk, uint paletteChunk, uint paletteReadSize) {
	if (!_resources.loadFixedChunk("scene 3110 framebuffer", framebufferChunk,
			_baseFramebuffer, kSceneBufferByteCount))
		return false;

	if (!_resources.loadFixedChunk("scene 3110 palette", paletteChunk,
			_paletteCurrent, paletteReadSize))
		return false;

	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	return true;
}

bool Scene3110::prepareScene(uint framebufferChunk, uint paletteChunk, uint paletteReadSize,
		const SpriteTrack *tracks, uint trackCount) {
	if (!loadFramebufferAndPalette(framebufferChunk, paletteChunk, paletteReadSize))
		return false;

	for (uint i = 0; i < trackCount; ++i) {
		const uint chunkIndex = tracks[i].chunkIndex;
		if (_chunks[chunkIndex].empty() && !_resources.loadVariableChunk(chunkIndex, _chunks[chunkIndex]))
			return false;
	}

	return true;
}

void Scene3110::runLongBranch() {
	playSound(0, 0x0f, 75, true);
	runExteriorLightningSequence();
	stopSound(0);

	playSound(0, 0x11, 75, true);
	playSound(1, 0x12, 75, true);
	runMachineRoomSequence();
	runMonsterTableElectricSequence();
	stopSound(0);
	stopSound(1);

	playSound(0, 0x0f, 75, true);
	runExteriorStormReturnSequence();
	stopSound(0);

	playSound(0, 0x11, 75, true);
	playSound(1, 0x12, 75, true);
	runBlackFlashSequence();
	runCloseupFlashSequence();
	runMonsterTableFinalSequence();
	stopSounds();
}

void Scene3110::runShortBranch() {
	const SpriteTrack initialTracks[] = {
		{24, 8, kScene3110ShortInitialFrameMap, ARRAYSIZE(kScene3110ShortInitialFrameMap), 30, true}
	};
	if (prepareScene(22, 23, kPaletteSize, initialTracks, ARRAYSIZE(initialTracks))) {
		runSpriteSequence(AnimationFrameRange(0, 79, kScene3110DefaultFrameMillis), 0,
			initialTracks, ARRAYSIZE(initialTracks));
	}

	const SpriteTrack finalTracks[] = {
		{21, 5, kScene3110ShortFinalFrameMap, ARRAYSIZE(kScene3110ShortFinalFrameMap), 30, true}
	};
	if (prepareScene(19, 20, 0x168, finalTracks, ARRAYSIZE(finalTracks))) {
		runSpriteSequence(AnimationFrameRange(0, 79, kScene3110DefaultFrameMillis), 0,
			finalTracks, ARRAYSIZE(finalTracks));
	}
}

void Scene3110::runExteriorLightningSequence() {
	if (!loadFramebufferAndPalette(0, 1, 0x2e8))
		return;
	for (uint chunkIndex = 2; chunkIndex <= 3; ++chunkIndex) {
		if (_chunks[chunkIndex].empty() && !_resources.loadVariableChunk(chunkIndex, _chunks[chunkIndex]))
			return;
	}

	const Common::Array<byte> clearPalette = _paletteCurrent;
	byte bladeFrame = 0;
	byte wingFrameIndex = 0;
	byte highPaletteStep = 0;
	byte lowPaletteStep = 0;
	uint counter = 0;
	uint tailCounter = 0;
	uint32 bladeAccumulator = 0;
	uint32 wingAccumulator = 0;
	uint32 paletteAccumulator = 0;
	bool wingActive = false;
	bool stormActive = false;
	bool stormFinished = false;

	drawExteriorFrame(wingFrameIndex, bladeFrame);
	presentFrame();
	while (!stormFinished || tailCounter != 0) {
		if (_skipRequested || Engine::shouldQuit() || pollEvents())
			return;

		++counter;
		bladeAccumulator += kScene3110ExteriorCounterMillis;
		while (bladeAccumulator >= kScene3110ExteriorBladeFrameMillis) {
			bladeAccumulator -= kScene3110ExteriorBladeFrameMillis;
			bladeFrame = (bladeFrame + 1) % kScene3110ExteriorBladeFrameCount;
		}

		if (counter == kScene3110ExteriorWingStartCounter) {
			wingActive = true;
			playSound(1, 0x0c, 25, true);
		}
		if (wingActive) {
			wingAccumulator += kScene3110ExteriorCounterMillis;
			while (wingAccumulator >= kScene3110ExteriorWingFrameMillis) {
				wingAccumulator -= kScene3110ExteriorWingFrameMillis;
				if (wingFrameIndex + 1 < ARRAYSIZE(kScene3110ExteriorForegroundFrameMap)) {
					++wingFrameIndex;
				} else {
					wingActive = false;
					stopSound(1);
					break;
				}
			}
		}

		if (counter == kScene3110ExteriorStormStartCounter)
			stormActive = true;
		bool stormFinishedThisTick = false;
		if (stormActive) {
			paletteAccumulator += kScene3110ExteriorCounterMillis;
			while (paletteAccumulator >= kScene3110ExteriorPaletteStepMillis) {
				paletteAccumulator -= kScene3110ExteriorPaletteStepMillis;
				if (highPaletteStep < 0x15) {
					darkenExteriorPaletteRange(clearPalette, 0xa0, 0xff,
						highPaletteStep++, true);
				}
				if (lowPaletteStep < 0x0b) {
					darkenExteriorPaletteRange(clearPalette, 1, 0x9f,
						lowPaletteStep++, false);
				} else {
					stormActive = false;
					stormFinished = true;
					stormFinishedThisTick = true;
					tailCounter = kScene3110ExteriorStormTailCounter;
					break;
				}
			}
		}
		if (stormFinished && !stormFinishedThisTick && tailCounter != 0)
			--tailCounter;

		drawExteriorFrame(wingFrameIndex, bladeFrame);
		presentFrame();
		if (delay(kScene3110ExteriorCounterMillis))
			return;
	}

	stopSound(1);
	_exteriorStormPalette = _paletteCurrent;
}

void Scene3110::runExteriorStormReturnSequence() {
	if (!loadFramebufferAndPalette(0, 1, 0x2e8))
		return;
	for (uint chunkIndex = 2; chunkIndex <= 3; ++chunkIndex) {
		if (_chunks[chunkIndex].empty() && !_resources.loadVariableChunk(chunkIndex, _chunks[chunkIndex]))
			return;
	}

	const Common::Array<byte> clearPalette = _paletteCurrent;
	if (_exteriorStormPalette.size() == kPaletteSize)
		_paletteCurrent = _exteriorStormPalette;
	const Common::Array<byte> stormReferencePalette = _paletteCurrent;
	Common::Array<byte> lightningDarkPalette = _paletteCurrent;
	byte bladeFrame = 0;
	byte highPaletteStep = 0;
	byte lowPaletteStep = 0;
	uint counter = 0;
	uint lightningFlashPhase = 0;
	uint32 bladeAccumulator = 0;
	uint32 paletteAccumulator = kScene3110ExteriorPaletteStepMillis;
	uint32 lightningFlashAccumulator = 0;
	bool stormActive = true;
	bool lightningFlashActive = false;

	drawExteriorFrame(0, bladeFrame);
	presentFrame();
	while (counter < 0x38e) {
		if (_skipRequested || Engine::shouldQuit() || pollEvents())
			return;

		++counter;
		bladeAccumulator += kScene3110ExteriorCounterMillis;
		while (bladeAccumulator >= kScene3110ExteriorBladeFrameMillis) {
			bladeAccumulator -= kScene3110ExteriorBladeFrameMillis;
			bladeFrame = (bladeFrame + 1) % kScene3110ExteriorBladeFrameCount;
		}

		if (stormActive) {
			paletteAccumulator += kScene3110ExteriorCounterMillis;
			while (paletteAccumulator >= kScene3110ExteriorPaletteStepMillis) {
				paletteAccumulator -= kScene3110ExteriorPaletteStepMillis;
				if (highPaletteStep < 0x0b) {
					darkenExteriorPaletteRange(stormReferencePalette, 0xa0, 0xff,
						highPaletteStep++, true);
				}
				if (lowPaletteStep < 6) {
					darkenExteriorPaletteRange(stormReferencePalette, 1, 0x9f,
						lowPaletteStep++, false);
				} else {
					stormActive = false;
					counter = 0x334;
					break;
				}
			}
		}

		if (lightningFlashActive) {
			lightningFlashAccumulator += kScene3110ExteriorCounterMillis;
			while (lightningFlashAccumulator >= kScene3110ExteriorLightningFlashMillis) {
				lightningFlashAccumulator -= kScene3110ExteriorLightningFlashMillis;
				++lightningFlashPhase;
				if (lightningFlashPhase == 1)
					_paletteCurrent = lightningDarkPalette;
				else if (lightningFlashPhase == 2)
					_paletteCurrent = clearPalette;
				else {
					_paletteCurrent = lightningDarkPalette;
					lightningFlashActive = false;
					break;
				}
			}
		}

		if (counter == 0x352 || counter == 0x370) {
			playSound(2, kScene3110SoundCueLightning, 100, false);
			lightningDarkPalette = _paletteCurrent;
			_paletteCurrent = clearPalette;
			lightningFlashPhase = 0;
			lightningFlashAccumulator = 0;
			lightningFlashActive = true;
		}

		drawExteriorFrame(0, bladeFrame);
		presentFrame();
		if (delay(kScene3110ExteriorCounterMillis))
			return;
	}
}

void Scene3110::drawExteriorFrame(byte foregroundFrameIndex, byte bladeFrame) {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	const uint frameMapIndex = MIN<uint>(foregroundFrameIndex,
		ARRAYSIZE(kScene3110ExteriorForegroundFrameMap) - 1);
	drawStripSpriteFrame(_chunks[3], 0, 0, 5,
		kScene3110ExteriorForegroundFrameMap[frameMapIndex], _sceneFramebuffer.surface());
	drawStripSpriteFrame(_chunks[2], 0, 0, 8, bladeFrame, _sceneFramebuffer.surface());
}

void Scene3110::darkenExteriorPaletteRange(const Common::Array<byte> &referencePalette,
		uint firstColor, uint lastColor, byte threshold, bool snapLowComponentsToBlack) {
	if (_paletteCurrent.size() < kPaletteSize || referencePalette.size() < kPaletteSize)
		return;

	for (uint color = firstColor; color <= lastColor; ++color) {
		for (uint channel = 0; channel < 3; ++channel) {
			const uint offset = color * 3 + channel;
			if (referencePalette[offset] < threshold)
				continue;
			if (snapLowComponentsToBlack && _paletteCurrent[offset] < 3)
				_paletteCurrent[offset] = 0;
			else if (_paletteCurrent[offset] != 0)
				--_paletteCurrent[offset];
		}
	}
}

void Scene3110::runMachineRoomSequence() {
	if (!loadFramebufferAndPalette(10, 11, kPaletteSize))
		return;

	for (uint chunkIndex = 12; chunkIndex <= 18; ++chunkIndex) {
		if (_chunks[chunkIndex].empty() && !_resources.loadVariableChunk(chunkIndex, _chunks[chunkIndex]))
			return;
	}

	initializeMemoryEffectPalette();

	MachineRoomState state;
	initializeMachineRoomState(state);
	composeMachineRoomFrame(state, false);
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _sceneFramebuffer.size());
	_sceneFramebuffer.clear(0);
	presentFrame(0, kScene3110MachineRoomViewportXOffset);

	for (int sweepOffset = kScene3110MachineRoomCurtainStartY;
			sweepOffset >= 0 && !_skipRequested && !Engine::shouldQuit();
			sweepOffset -= kScene3110MachineRoomCurtainBandHeight) {
		if (pollEvents())
			return;
		revealSavedFramebufferBand((uint)sweepOffset, kScene3110MachineRoomCurtainBandHeight,
			kScene3110MachineRoomViewportXOffset);
		presentFrame(0, kScene3110MachineRoomViewportXOffset);
		if (delay(kScene3110MachineRoomRevealMillis))
			return;
	}

	memcpy(_sceneFramebuffer.data(), _savedFramebuffer.data(), _sceneFramebuffer.size());
	presentFrame(0, kScene3110MachineRoomViewportXOffset);

	runMachineRoomInitialPhase(state);
	runMachineRoomFinalPhase(state);
}

void Scene3110::initializeMachineRoomState(MachineRoomState &state) const {
	memset(&state, 0, sizeof(state));
	state.initialGateDirty = true;
	state.finalOverlayDirty = true;
}

void Scene3110::composeMachineRoomFrame(const MachineRoomState &state, bool finalPhase) {
	restoreOriginalSpriteFrameBackground(_chunks[18], 9, state.fixedOverlayFrame, _sceneFramebuffer.surface());
	restoreOriginalSpriteFrameBackground(_chunks[16], 10, state.leftMemoryActorFrame, _sceneFramebuffer.surface());
	restoreOriginalSpriteFrameBackground(_chunks[17], 10, state.rightMemoryActorFrame, _sceneFramebuffer.surface());
	restoreOriginalSpriteFrameBackground(_chunks[14], 0x1f, state.leftArcFrame, _sceneFramebuffer.surface());
	restoreOriginalSpriteFrameBackground(_chunks[15], 0x1f, state.rightArcFrame, _sceneFramebuffer.surface());
	if (finalPhase)
		restoreOriginalSpriteFrameBackground(_chunks[13], 10, state.finalOverlayFrame, _sceneFramebuffer.surface());

	drawStripSpriteFrame(_chunks[18], 0, 0, 9, state.fixedOverlayFrame, _sceneFramebuffer.surface());
	drawStripSpriteFrame(_chunks[16], 0, 0, 10, state.leftMemoryActorFrame, _sceneFramebuffer.surface());
	drawStripSpriteFrame(_chunks[17], 0, 0, 10, state.rightMemoryActorFrame, _sceneFramebuffer.surface());
	drawStripSpriteFrame(_chunks[14], 0, 0, 0x1f, state.leftArcFrame, _sceneFramebuffer.surface());
	drawStripSpriteFrame(_chunks[15], 0, 0, 0x1f, state.rightArcFrame, _sceneFramebuffer.surface());
	if (finalPhase)
		drawStripSpriteFrame(_chunks[13], 0, 0, 10, state.finalOverlayFrame, _sceneFramebuffer.surface());
}

void Scene3110::restoreOriginalSpriteFrameBackground(const Common::Array<byte> &resource, uint descriptorCount,
		uint descriptorIndex, Graphics::Surface &destination) const {
	restoreSpriteBackground(resource, 0, 0, descriptorCount, descriptorIndex, _baseFramebuffer.surface(), destination);
}

void Scene3110::runMachineRoomInitialPhase(MachineRoomState &state) {
	uint32 elapsedMillis = 0;
	while ((state.finalOverlayFrame < kScene3110MachineRoomInitialGateEndFrame || state.initialGateDirty) &&
			!_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		advanceMachineRoomState(state, elapsedMillis, false);
		composeMachineRoomFrame(state, false);
		presentFrame(0, kScene3110MachineRoomViewportXOffset);
		state.initialGateDirty = false;

		if (delay(kScene3110MachineRoomPollMillis))
			return;
		elapsedMillis = kScene3110MachineRoomPollMillis;
	}
}

void Scene3110::runMachineRoomFinalPhase(MachineRoomState &state) {
	state.finalOverlayFrame = 0;
	state.finalOverlayAccumulator = 0;
	state.finalCounterAccumulator = 0;
	state.finalCounter = 0;
	state.finalOverlayDirty = true;

	composeMachineRoomFrame(state, true);
	presentFrame(0, kScene3110MachineRoomViewportXOffset);

	uint32 elapsedMillis = 0;
	while (state.finalCounter < kScene3110MachineRoomFinalCounterLimit &&
			!_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents())
			return;

		advanceMachineRoomState(state, elapsedMillis, true);
		composeMachineRoomFrame(state, true);
		presentFrame(0, kScene3110MachineRoomViewportXOffset);
		state.finalOverlayDirty = false;

		if (delay(kScene3110MachineRoomPollMillis))
			return;
		elapsedMillis = kScene3110MachineRoomPollMillis;
	}
}

void Scene3110::advanceMachineRoomState(MachineRoomState &state, uint32 elapsedMillis, bool finalPhase) {
	advanceMachineRoomPalette(state, elapsedMillis);

	state.leftMemoryActorAccumulator += elapsedMillis;
	while (state.leftMemoryActorAccumulator >= kScene3110MachineRoomLeftMemoryActorMillis) {
		state.leftMemoryActorAccumulator -= kScene3110MachineRoomLeftMemoryActorMillis;
		advanceLeftMemoryActor(state);
	}

	state.rightMemoryActorAccumulator += elapsedMillis;
	while (state.rightMemoryActorAccumulator >= kScene3110MachineRoomRightMemoryActorMillis) {
		state.rightMemoryActorAccumulator -= kScene3110MachineRoomRightMemoryActorMillis;
		advanceRightMemoryActor(state);
	}

	state.leftArcAccumulator += elapsedMillis;
	while (state.leftArcAccumulator >= kScene3110MachineRoomLeftArcMillis) {
		state.leftArcAccumulator -= kScene3110MachineRoomLeftArcMillis;
		advanceLeftElectricalArc(state);
	}

	state.rightArcAccumulator += elapsedMillis;
	while (state.rightArcAccumulator >= kScene3110MachineRoomRightArcMillis) {
		state.rightArcAccumulator -= kScene3110MachineRoomRightArcMillis;
		advanceRightElectricalArc(state);
	}

	state.finalOverlayAccumulator += elapsedMillis;
	while (state.finalOverlayAccumulator >= kScene3110MachineRoomInitialGateMillis) {
		state.finalOverlayAccumulator -= kScene3110MachineRoomInitialGateMillis;
		if (finalPhase)
			advanceFinalMemoryOverlay(state);
		else
			advanceInitialMemoryGate(state);
	}

	if (finalPhase) {
		state.finalCounterAccumulator += elapsedMillis;
		while (state.finalCounterAccumulator >= kScene3110MachineRoomFinalCounterMillis) {
			state.finalCounterAccumulator -= kScene3110MachineRoomFinalCounterMillis;
			++state.finalCounter;
		}
	}
}

void Scene3110::advanceInitialMemoryGate(MachineRoomState &state) {
	if (state.finalOverlayFrame < kScene3110MachineRoomInitialGateEndFrame) {
		++state.finalOverlayFrame;
		state.initialGateDirty = true;
	}
}

void Scene3110::advanceFinalMemoryOverlay(MachineRoomState &state) {
	if (state.finalOverlayFrame < 9)
		++state.finalOverlayFrame;
	else
		state.finalOverlayFrame = 0;
	state.finalOverlayDirty = true;
}

void Scene3110::advanceLeftMemoryActor(MachineRoomState &state) {
	if ((nextMemoryRandom15Bit() & 1) == 0) {
		if (state.leftMemoryActorFrame > 8)
			--state.leftMemoryActorFrame;
		else
			++state.leftMemoryActorFrame;
	} else if (state.leftMemoryActorFrame != 0) {
		--state.leftMemoryActorFrame;
	} else {
		++state.leftMemoryActorFrame;
	}
}

void Scene3110::advanceRightMemoryActor(MachineRoomState &state) {
	if ((nextMemoryRandom15Bit() & 1) == 0) {
		if (state.rightMemoryActorFrame > 8)
			--state.rightMemoryActorFrame;
		else
			++state.rightMemoryActorFrame;
	} else if (state.rightMemoryActorFrame != 0) {
		--state.rightMemoryActorFrame;
	} else {
		++state.rightMemoryActorFrame;
	}
}

void Scene3110::advanceLeftElectricalArc(MachineRoomState &state) {
	if (nextMemoryRandom15Bit() % 5 == 0) {
		if (state.leftArcFrame != 0)
			--state.leftArcFrame;
		else
			++state.leftArcFrame;
	} else if (state.leftArcFrame > 0x1d) {
		--state.leftArcFrame;
	} else {
		++state.leftArcFrame;
	}
}

void Scene3110::advanceRightElectricalArc(MachineRoomState &state) {
	if (nextMemoryRandom15Bit() % 4 == 0) {
		if (state.rightArcFrame != 0)
			--state.rightArcFrame;
		else
			++state.rightArcFrame;
	} else if (state.rightArcFrame > 0x1d) {
		--state.rightArcFrame;
	} else {
		++state.rightArcFrame;
	}
}

void Scene3110::advanceMachineRoomPalette(MachineRoomState &state, uint32 elapsedMillis) {
	state.paletteBandAccumulator += elapsedMillis;
	while (state.paletteBandAccumulator >= kScene3110MemoryPaletteBandMillis) {
		state.paletteBandAccumulator -= kScene3110MemoryPaletteBandMillis;
		applyMemoryPaletteBand(state.paletteBandPhase++);
	}

	state.palettePulseAccumulator += elapsedMillis;
	while (state.palettePulseAccumulator >= kScene3110MemoryPalettePulseMillis) {
		state.palettePulseAccumulator -= kScene3110MemoryPalettePulseMillis;
		processMemoryPalettePulseStep();
	}
}

void Scene3110::runMonsterTableElectricSequence() {
	const SpriteTrack tracks[] = {
		{6, 0x20, nullptr, 0x20, 30, true},
		{7, 10, nullptr, 10, 0, false},
		{8, 10, nullptr, 10, 0, false}
	};
	AnimationFrameRange range(0, 85, kScene3110DefaultFrameMillis);
	range.loopingSoundAt(30, 0x15, 75, 2)
		.soundAt(68, 0x1e, 100, 2);
	if (prepareScene(4, 5, kPaletteSize, tracks, ARRAYSIZE(tracks)))
		runSpriteSequence(range, 0, tracks, ARRAYSIZE(tracks));
}

void Scene3110::runMonsterTableFinalSequence() {
	const SpriteTrack tracks[] = {
		{9, 0x20, nullptr, 0x20, 0, true},
		{7, 10, nullptr, 10, 0, false},
		{8, 10, nullptr, 10, 0, false}
	};
	AnimationFrameRange range(0, 89, kScene3110DefaultFrameMillis);
	range.loopingSoundAt(0, 0x15, 75, 2);
	if (prepareScene(4, 5, kPaletteSize, tracks, ARRAYSIZE(tracks)))
		runSpriteSequence(range, 0, tracks, ARRAYSIZE(tracks));
	stopSound(2);
}

void Scene3110::runBlackFlashSequence() {
	const SpriteTrack tracks[] = {
		{26, 4, kScene3110BlackFlashFrameMap, ARRAYSIZE(kScene3110BlackFlashFrameMap), 30, false}
	};
	AnimationFrameRange range(0, 109, kScene3110DefaultFrameMillis);
	range.soundAt(30, kScene3110SoundCueLightning, 100, 2)
		.soundAt(80, kScene3110SoundCueLightning, 100, 2);
	if (!_resources.loadFixedChunk("scene 3110 palette", 25,
			_paletteCurrent, kPaletteSize))
		return;

	if (_chunks[26].empty() && !_resources.loadVariableChunk(26, _chunks[26]))
		return;

	_baseFramebuffer.clear(0);
	_sceneFramebuffer.clear(0);
	runSpriteSequence(range, 0, tracks, ARRAYSIZE(tracks));
}

void Scene3110::runCloseupFlashSequence() {
	const SpriteTrack tracks[] = {
		{29, 3, kScene3110CloseupFrameMap, ARRAYSIZE(kScene3110CloseupFrameMap), 0, false}
	};
	AnimationFrameRange range(0, 79, kScene3110DefaultFrameMillis);
	range.soundAt(30, kScene3110SoundCueLightning, 100, 2)
		.soundAt(50, kScene3110SoundCueLightning, 100, 2);
	if (prepareScene(27, 28, kPaletteSize, tracks, ARRAYSIZE(tracks)))
		runSpriteSequence(range, 0, tracks, ARRAYSIZE(tracks));
}

void Scene3110::runSpriteSequence(const AnimationFrameRange &range,
		uint16 viewportX, const SpriteTrack *tracks, uint trackCount) {
	_spriteSequenceTracks = tracks;
	_spriteSequenceTrackCount = trackCount;
	_spriteSequenceViewportX = viewportX;
	_animationPlayer.playAndPresent(_spriteSequenceTick, range);
	_spriteSequenceTracks = nullptr;
	_spriteSequenceTrackCount = 0;
}

void Scene3110::drawSpriteSequenceFrame(uint tick, const SpriteTrack *tracks, uint trackCount) {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	for (uint i = 0; i < trackCount; ++i) {
		const SpriteTrack &track = tracks[i];
		drawStripSpriteFrame(_chunks[track.chunkIndex], 0, 0, track.descriptorCount,
			frameForTrack(track, tick), _sceneFramebuffer.surface());
	}
}

uint Scene3110::frameForTrack(const SpriteTrack &track, uint tick) const {
	if (track.frameMapSize == 0)
		return 0;

	if (tick < track.firstTick)
		return track.frameMap ? track.frameMap[0] : 0;

	const uint frameStep = tick - track.firstTick;
	const uint frameIndex = track.holdLastFrame ?
		MIN<uint>(frameStep, track.frameMapSize - 1) :
		frameStep % track.frameMapSize;
	return track.frameMap ? track.frameMap[frameIndex] : frameIndex;
}

void Scene3110::initializeMemoryEffectPalette() {
	_memoryPulseActive = false;
	_memoryPulseLevel = 0;
	_memoryRandomState = 0x31103110;
	memcpy(_memoryPulseSavedPalette.data(), _paletteCurrent.data(), MIN<uint>(_memoryPulseSavedPalette.size(), _paletteCurrent.size()));
}

void Scene3110::applyMemoryPaletteBand(uint tick) {
	(void)tick;
	byte savedColor90[3];
	memcpy(savedColor90, _paletteCurrent.data() + 0x90 * 3, sizeof(savedColor90));
	for (uint color = 0x9f; color > 0x90; --color) {
		const uint offset = color * 3;
		memcpy(_paletteCurrent.data() + offset, _paletteCurrent.data() + offset - 3, 3);
	}
	memcpy(_paletteCurrent.data() + 0x90 * 3, savedColor90, sizeof(savedColor90));
}

void Scene3110::processMemoryPalettePulseStep() {
	if (!_memoryPulseActive) {
		if (nextMemoryRandom15Bit() % 100 == 0) {
			_memoryPulseActive = true;
			_memoryPulseLevel = 0;
			memcpy(_memoryPulseSavedPalette.data(), _paletteCurrent.data(),
				MIN<uint>(_memoryPulseSavedPalette.size(), _paletteCurrent.size()));
		}
		return;
	}

	if (_memoryPulseLevel == 4) {
		restoreMemoryPalettePulseRanges();
		_memoryPulseLevel = 0;
		_memoryPulseActive = false;
		return;
	}

	const bool brighten = (nextMemoryRandom15Bit() & 1) == 0;
	if (brighten) {
		if (_memoryPulseLevel != 0) {
			adjustMemoryPalettePulseRanges(1);
			--_memoryPulseLevel;
		}
	} else {
		adjustMemoryPalettePulseRanges(-1);
		++_memoryPulseLevel;
	}
}

void Scene3110::adjustMemoryPalettePulseRanges(int delta) {
	for (uint color = 1; color < 0x8f; ++color) {
		const uint offset = color * 3;
		if (delta > 0) {
			if (_paletteCurrent[offset] < 0x3f && _paletteCurrent[offset + 1] < 0x3f && _paletteCurrent[offset + 2] < 0x3f) {
				++_paletteCurrent[offset];
				++_paletteCurrent[offset + 1];
				++_paletteCurrent[offset + 2];
			}
		} else if (_paletteCurrent[offset] != 0 && _paletteCurrent[offset + 1] != 0 && _paletteCurrent[offset + 2] != 0) {
			--_paletteCurrent[offset];
			--_paletteCurrent[offset + 1];
			--_paletteCurrent[offset + 2];
		}
	}

	for (uint color = 0xa0; color < 0xff; ++color) {
		const uint offset = color * 3;
		if (delta > 0) {
			if (_paletteCurrent[offset] < 0x3f && _paletteCurrent[offset + 1] < 0x3f && _paletteCurrent[offset + 2] < 0x3f) {
				++_paletteCurrent[offset];
				++_paletteCurrent[offset + 1];
				++_paletteCurrent[offset + 2];
			}
		} else if (_paletteCurrent[offset] != 0 && _paletteCurrent[offset + 1] != 0 && _paletteCurrent[offset + 2] != 0) {
			--_paletteCurrent[offset];
			--_paletteCurrent[offset + 1];
			--_paletteCurrent[offset + 2];
		}
	}
}

void Scene3110::restoreMemoryPalettePulseRanges() {
	if (_memoryPulseSavedPalette.size() < kPaletteSize || _paletteCurrent.size() < kPaletteSize)
		return;

	memcpy(_paletteCurrent.data(), _memoryPulseSavedPalette.data(), 0x1b0);
	memcpy(_paletteCurrent.data() + 0x1e0, _memoryPulseSavedPalette.data() + 0x1e0, kPaletteSize - 0x1e0);
}

uint16 Scene3110::nextMemoryRandom15Bit() {
	_memoryRandomState = _memoryRandomState * 1103515245 + 12345;
	return (_memoryRandomState >> 16) & 0x7fff;
}

void Scene3110::playSound(uint slot, uint16 cueId, byte volumePercent, bool loop) {
	SoundBank0Player *player = nullptr;
	switch (slot) {
	case 0:
		player = &_sound0;
		break;
	case 1:
		player = &_sound1;
		break;
	case 2:
		player = &_sound2;
		break;
	default:
		return;
	}

	if (loop)
		player->playSampleLooping(cueId, volumePercent);
	else
		player->playSample(cueId, volumePercent, false);
}

void Scene3110::stopSound(uint slot) {
	switch (slot) {
	case 0:
		_sound0.stop();
		break;
	case 1:
		_sound1.stop();
		break;
	case 2:
		_sound2.stop();
		break;
	default:
		break;
	}
}

void Scene3110::stopSounds() {
	_sound0.stop();
	_sound1.stop();
	_sound2.stop();
}

void Scene3110::handleAnimationFrameEvent(const AnimationFrameEvent &event,
		uint frame) {
	if (event.type == AnimationFrameEvent::kSound ||
			event.type == AnimationFrameEvent::kLoopingSound) {
		const uint slot = event.soundSlot == AnimationFrameEvent::kMainSoundSlot ?
			0 : event.soundSlot;
		playSound(slot, event.soundId, event.soundVolumePercent,
			event.type == AnimationFrameEvent::kLoopingSound);
		return;
	}

	PresentationScene::handleAnimationFrameEvent(event, frame);
}

void Scene3110::presentAnimationFrame() {
	if (_spriteSequenceTracks == nullptr) {
		PresentationScene::presentAnimationFrame();
		return;
	}

	drawSpriteSequenceFrame(_spriteSequenceTick, _spriteSequenceTracks,
		_spriteSequenceTrackCount);
	presentFrame(0, _spriteSequenceViewportX);
}

void Scene3110::stopAudio() {
	stopSounds();
}

} // End of namespace Hollywood
