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

#include "hollywood/scenes/playable/scene2060.h"

#include "common/endian.h"

#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2060ArchiveName = "RESOURCE.B06";
const char *const kScene2060MusicArchiveName = "RESOURCE.M02";
const char *const kScene2060SoundArchiveName = "RESOURCE.S02";
const uint kScene2060InitialRequiredChunkCount = 32;
const uint kScene2060ArenaFirstChunk = 5;
const uint kScene2060ArenaLastChunk = 31;
const uint kScene2060StageIndex = 205;
const uint16 kScene2060FirstState = 0x080c;
const uint16 kScene2060LastState = 0x0811;
const uint16 kScene2050LabyrinthReturnState = 0x0803;
const uint16 kScene2070LabyrinthExitState = 0x0816;
const uint16 kScene2060ViewportXOffset = 0x0068;
const uint kScene2060WallRemapMinX = 0x0068;
const uint kScene2060WallRemapMaxXExclusive = 0x02e8;
const uint kScene2060WallRemapMaxYExclusive = 0x01e0;
const uint kScene2060ActorPaletteTableEntry = 0x00cc;
const uint kScene2060Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2060SpeechCueDescriptorTableOffset = 0x1135;
const byte kScene2060InitialMazeIndex = 0x2a;
const byte kScene2060FinalMazeIndex = 0x47;
const uint kScene2060MazeIndexCount = 0x48;
const int kScene2060MinWalkX = 0x0f9;
const int kScene2060MaxWalkX = 0x24d;
const int kScene2060MinWalkY = 0x076;
const int kScene2060MaxWalkY = 0x16c;
const uint32 kScene2060ActorPathFrameMillis = 60;
const byte kScene2060InvalidFacing = 0xff;
const uint kScene2060GuideChunkBase = 19;
const uint kScene2060GuideChunkCount = 6;
const int kScene2060GuideHalfSize = 0x22;
const uint kScene2060GuideSize = 0x45;
const uint kScene2060GuideMaskByteCount = kScene2060GuideSize * kScene2060GuideSize;
const uint32 kScene2060GuideFrameMillis = 60;
const byte kScene2060GuideSoundCue = 0x2f;
const byte kScene2060GuideSoundVolume = 0x32;

const byte kScene2060ActorPathStepDeltaTable[] = {
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10
};

const byte kScene2060PassageMaskByMazeIndex[] = {
	0x01, 0x05, 0x0e, 0x0d, 0x0e, 0x0b, 0x16, 0x1a,
	0x26, 0x1a, 0x27, 0x3b, 0x22, 0x26, 0x2d, 0x2e,
	0x3f, 0x39, 0x21, 0x26, 0x1a, 0x20, 0x30, 0x10,
	0x17, 0x28, 0x24, 0x0d, 0x0e, 0x0a, 0x31, 0x06,
	0x0e, 0x18, 0x24, 0x29, 0x16, 0x29, 0x22, 0x06,
	0x0a, 0x12, 0x29, 0x11, 0x21, 0x22, 0x25, 0x29,
	0x17, 0x1a, 0x14, 0x28, 0x12, 0x12, 0x32, 0x25,
	0x09, 0x05, 0x2c, 0x29, 0x25, 0x1b, 0x12, 0x14,
	0x09, 0x10, 0x16, 0x3a, 0x25, 0x08, 0x14, 0x0c
};

const byte kScene2060PassageBits[] = {
	0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

const uint kScene2060OpenPassageChunks[] = {
	5, 6, 7, 8, 9, 10
};

const uint kScene2060ClosedPassageChunks[] = {
	11, 12, 13, 14, 15, 16
};

const byte kScene2060GuideRoute[][2] = {
	{0x2a, 0}, {0x24, 3}, {0x25, 0}, {0x1f, 3}, {0x20, 4},
	{0x26, 4}, {0x2c, 5}, {0x32, 3}, {0x33, 0}, {0x2d, 0},
	{0x27, 3}, {0x28, 4}, {0x2e, 5}, {0x34, 4}, {0x3a, 2},
	{0x39, 5}, {0x3f, 3}, {0x40, 5}, {0x46, 3}, {0x47, 3}
};

static_assert(ARRAYSIZE(kScene2060ActorPathStepDeltaTable) == 72, "Scene 2060 actor path table size changed");
static_assert(ARRAYSIZE(kScene2060PassageMaskByMazeIndex) == kScene2060MazeIndexCount,
	"Scene 2060 passage mask table size changed");
static_assert(ARRAYSIZE(kScene2060PassageBits) == 6, "Scene 2060 passage bit count changed");
static_assert(ARRAYSIZE(kScene2060OpenPassageChunks) == ARRAYSIZE(kScene2060PassageBits),
	"Scene 2060 open passage chunk count changed");
static_assert(ARRAYSIZE(kScene2060ClosedPassageChunks) == ARRAYSIZE(kScene2060PassageBits),
	"Scene 2060 closed passage chunk count changed");
static_assert(ARRAYSIZE(kScene2060GuideRoute) == 20, "Scene 2060 guide route size changed");

static PlayableSceneConfig scene2060Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2060ArchiveName;
	config.initialRequiredChunkCount = kScene2060InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2060ArenaFirstChunk;
	config.arenaLastChunk = kScene2060ArenaLastChunk;
	config.stageIndex = kScene2060StageIndex;
	config.debugName = "Scene 2060";
	config.viewportXOffset = kScene2060ViewportXOffset;
	config.viewportMinXOffset = kScene2060ViewportXOffset;
	config.viewportMaxXOffset = kScene2060ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorPaletteTableEntry = kScene2060ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2060Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2060SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2060ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2060ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 1;
	config.musicArchiveName = kScene2060MusicArchiveName;
	config.soundBank0ArchiveName = kScene2060SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene2060FirstState;
	config.mainFlowLastState = kScene2060LastState;
	return config;
}

Scene2060::Scene2060(HollywoodEngine *vm) :
		PlayableScene(vm, scene2060Config(), "scene2060", 0x0f9, 0x0f4, 2, 0xfd, 0xfb),
		_sceneActorBankInstalled(false),
		_guideEffectPrepared(false),
		_guideEffectActive(false),
		_guideDirection(0),
		_guideFrameIndex(0),
		_guideFrameCount(0) {
}

bool Scene2060::hasCustomPreviewState() const {
	return true;
}

void Scene2060::initializeCustomPreviewState() {
	installSceneActorBank();
	rebuildScenePaletteRemapTable();
	initializeDefaultPreviewState();

	switch (_vm->gameState().mainFlowStateId) {
	case 0x080c:
		_activeActorWorldX = 0x17a;
		_activeActorWorldY = 0x076;
		_activeActorFacing = 3;
		break;
	case 0x080d:
		_activeActorWorldX = 0x210;
		_activeActorWorldY = 0x076;
		_activeActorFacing = 3;
		break;
	case 0x080e:
		_activeActorWorldX = 0x0f9;
		_activeActorWorldY = 0x0f4;
		_activeActorFacing = 2;
		break;
	case 0x080f:
		_activeActorWorldX = 0x24d;
		_activeActorWorldY = 0x0f0;
		_activeActorFacing = 4;
		break;
	case 0x0810:
		_activeActorWorldX = 0x152;
		_activeActorWorldY = 0x16c;
		_activeActorFacing = 0;
		break;
	case 0x0811:
		_activeActorWorldX = 0x1e8;
		_activeActorWorldY = 0x16c;
		_activeActorFacing = 0;
		break;
	default:
		break;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2060::hasCustomComposite() const {
	return true;
}

void Scene2060::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	remapWallPresentationPalette();
	restoreGuideBackgroundRect();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawGuideLayer();
	drawActionOverlayLayer();
}

bool Scene2060::hasCustomEntrySequence() const {
	return true;
}

void Scene2060::runCustomEntrySequence() {
	_guideEffectPrepared = prepareGuideEffectForCurrentMazePosition();
	_guideEffectActive = false;

	switch (_vm->gameState().mainFlowStateId) {
	case 0x080c:
		runEntryPathAndGuide(0x17a, 0x000, 3, 0x17a, 0x076);
		break;
	case 0x080d:
		runEntryPathAndGuide(0x210, 0x000, 3, 0x210, 0x076);
		break;
	case 0x080e:
		runEntryPathAndGuide(0x064, 0x0f4, 2, 0x0f9, 0x0f4);
		break;
	case 0x080f:
		runEntryPathAndGuide(0x2e3, 0x0f0, 4, 0x24d, 0x0f0);
		break;
	case 0x0810:
		runEntryPathWithInitialDrawOrder(0x152, 0x1e3, 0, 6, 0x152, 0x16c);
		break;
	case 0x0811:
		runEntryPathWithInitialDrawOrder(0x1e8, 0x1e3, 0, 7, 0x1e8, 0x16c);
		break;
	default:
		break;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2060::prepareCustomGameplayLoop() {
	installSceneActorBank();
	rebuildScenePaletteRemapTable();
	return true;
}

bool Scene2060::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2060::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir a pasadizo superior izquierdo (go to upper-left passage).
		moveThroughPassage(-6, 0x0810);
		return true;
	case 302: // Ir a pasadizo superior derecho (go to upper-right passage).
		moveThroughPassage(-6, 0x0811);
		return true;
	case 303: // Ir a pasadizo izquierdo (go to left passage): returns to the chamber at maze start.
		if (state.egyptLabyrinthPositionIndex == kScene2060InitialMazeIndex) {
			state.egyptLabyrinthPositionIndex = 0xff;
			state.mainFlowStateId = kScene2050LabyrinthReturnState;
			return true;
		}
		moveThroughPassage(-1, 0x080f);
		return true;
	case 304: // Ir a pasadizo derecho (go to right passage): exits the labyrinth at maze end.
		if (state.egyptLabyrinthPositionIndex == kScene2060FinalMazeIndex) {
			state.egyptLabyrinthPositionIndex = 0xff;
			state.mainFlowStateId = kScene2070LabyrinthExitState;
			return true;
		}
		moveThroughPassage(1, 0x080e);
		return true;
	case 305: // Ir a pasadizo inferior izquierdo (go to lower-left passage).
		moveThroughPassage(6, 0x080c);
		return true;
	case 306: // Ir a pasadizo inferior derecho (go to lower-right passage).
		moveThroughPassage(6, 0x080d);
		return true;
	default:
		return false;
	}
}

bool Scene2060::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, kScene2060MinWalkX, kScene2060MaxWalkX);
	targetY = CLIP<int>(targetY, kScene2060MinWalkY, kScene2060MaxWalkY);
	return true;
}

bool Scene2060::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;

	if (_paletteMaskOriginal.empty())
		return true;

	copyPassageTextRows();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	byte mazeIndex = _vm->gameState().egyptLabyrinthPositionIndex;
	if (mazeIndex >= kScene2060MazeIndexCount)
		mazeIndex = kScene2060InitialMazeIndex;

	const byte passageMask = kScene2060PassageMaskByMazeIndex[mazeIndex];
	for (uint passage = 0; passage < ARRAYSIZE(kScene2060PassageBits); ++passage) {
		const uint chunkIndex = (passageMask & kScene2060PassageBits[passage]) ?
			kScene2060OpenPassageChunks[passage] : kScene2060ClosedPassageChunks[passage];
		if (_sceneChunkTable.isValidChunk(chunkIndex))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _baseFramebuffer);
	}

	if (_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
		for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
			const byte item = _paletteMask[kSceneColorToItemMap + color];
			if (item >= 1 && item <= ARRAYSIZE(kScene2060PassageBits) &&
					(passageMask & kScene2060PassageBits[item - 1]) == 0)
				_paletteMask[kSceneColorToItemMap + color] = 0;
		}
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene2060::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2060::installSceneActorBank() {
	if (_sceneActorBankInstalled)
		return;

	for (uint facing = 0; facing < 6; ++facing) {
		const uint chunkIndex = 25 + facing;
		if (!_sceneChunkTable.isValidChunk(chunkIndex))
			return;
		const uint32 size = _sceneChunkTable.sizes[chunkIndex];
		if (size > kActiveActorFacingRunStride)
			return;
		memset(_activeActorRunStreams.data() + facing * kActiveActorFacingRunStride, 0,
			kActiveActorFacingRunStride);
		memcpy(_activeActorRunStreams.data() + facing * kActiveActorFacingRunStride,
			_resourceArena.data() + _resourceChunkOffsets[chunkIndex], size);
	}

	const uint descriptorChunkIndex = 31;
	if (!_sceneChunkTable.isValidChunk(descriptorChunkIndex))
		return;
	const uint32 descriptorSize = _sceneChunkTable.sizes[descriptorChunkIndex];
	if (descriptorSize % kActiveActorDescriptorSize != 0)
		return;

	const byte *descriptors = _resourceArena.data() + _resourceChunkOffsets[descriptorChunkIndex];
	const uint descriptorCount = MIN<uint>(_activeActorDescriptors.size(),
		descriptorSize / kActiveActorDescriptorSize);
	for (uint i = 0; i < descriptorCount; ++i) {
		const uint offset = i * kActiveActorDescriptorSize;
		_activeActorDescriptors[i].runStreamOffset = READ_LE_UINT32(descriptors + offset);
		_activeActorDescriptors[i].opaqueRunCount = READ_LE_UINT32(descriptors + offset + 4);
		_activeActorDescriptors[i].paletteRunCount = READ_LE_UINT32(descriptors + offset + 8);
		_activeActorDescriptors[i].anchorX = (int16)READ_LE_UINT16(descriptors + offset + 12);
		_activeActorDescriptors[i].anchorY = (int16)READ_LE_UINT16(descriptors + offset + 16);
		_activeActorDescriptors[i].width = READ_LE_UINT16(descriptors + offset + 20);
		_activeActorDescriptors[i].height = READ_LE_UINT16(descriptors + offset + 24);
	}

	_sceneActorBankInstalled = true;
}

void Scene2060::copyPassageTextRows() {
	static const char kPassageText[] = " pasadizo";

	for (uint row = 1; row <= 6; ++row) {
		const uint offset = row * kStage003SmallRowSize;
		if (offset + kStage003SmallRowSize > _stage003SmallRows.size())
			return;
		memset(_stage003SmallRows.data() + offset, 0, kStage003SmallRowSize);
		memcpy(_stage003SmallRows.data() + offset, kPassageText,
			MIN<uint>(sizeof(kPassageText), kStage003SmallRowSize));
	}
}

void Scene2060::rebuildScenePaletteRemapTable() {
	buildPresentationPaletteRemapTable(_paletteResource, _scenePaletteRemapTable);
}

void Scene2060::remapWallPresentationPalette() {
	if (_scenePaletteRemapTable.size() < kScenePaletteMapPageSize)
		rebuildScenePaletteRemapTable();
	if (_scenePaletteRemapTable.size() < kScenePaletteMapPageSize)
		return;

	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	const uint maxX = MIN<uint>(kScene2060WallRemapMaxXExclusive, HollywoodEngine::kSceneBufferWidth);
	const uint maxY = MIN<uint>(kScene2060WallRemapMaxYExclusive, HollywoodEngine::kSceneBufferHeight);
	for (uint y = 0; y < maxY; ++y) {
		byte *row = pixels + y * HollywoodEngine::kSceneBufferWidth;
		for (uint x = kScene2060WallRemapMinX; x < maxX; ++x) {
			byte color = row[x];
			row[x] = remapScenePaletteColor(color, 4);
		}
	}
}

byte Scene2060::remapScenePaletteColor(byte color, uint steps) const {
	if (_scenePaletteRemapTable.size() < kScenePaletteMapPageSize)
		return color;

	for (uint step = 0; step < steps; ++step)
		color = _scenePaletteRemapTable[color];
	return color;
}

bool Scene2060::prepareGuideEffectForCurrentMazePosition() {
	_guideDirection = 0;
	_guideFrameIndex = 0;
	_guideFrameCount = 0;

	const GameplayState &state = _vm->gameState();
	if (state.egyptSealPuzzleProgress == 0)
		return false;

	byte mazeIndex = state.egyptLabyrinthPositionIndex;
	if (mazeIndex >= kScene2060MazeIndexCount)
		mazeIndex = kScene2060InitialMazeIndex;

	for (uint i = 0; i < ARRAYSIZE(kScene2060GuideRoute); ++i) {
		if (kScene2060GuideRoute[i][0] != mazeIndex)
			continue;

		const byte direction = kScene2060GuideRoute[i][1];
		if (state.mainFlowStateId == kScene2060FirstState + direction)
			return false;

		_guideDirection = direction;
		_guideFrameCount = (direction == 2 || direction == 3) ? 30 : 24;
		return direction < kScene2060GuideChunkCount;
	}

	return false;
}

void Scene2060::playGuideEffectIfPrepared() {
	if (!_guideEffectPrepared || _skipRequested || Engine::shouldQuit())
		return;

	_guideEffectActive = true;
	_guideFrameIndex = 0;
	_soundBank0.playSample(kScene2060GuideSoundCue, kScene2060GuideSoundVolume);

	while (_guideEffectActive && !_skipRequested && !Engine::shouldQuit()) {
		drawPlayableComposite();
		presentFrame();

		if (waitSceneMillis(kScene2060GuideFrameMillis))
			break;

		++_guideFrameIndex;
		if (_guideFrameIndex >= _guideFrameCount)
			_guideEffectActive = false;
	}

	_guideEffectActive = false;
	drawPlayableComposite();
	presentFrame();
}

void Scene2060::restoreGuideBackgroundRect() {
	if (!_guideEffectActive || _guideFrameIndex >= _guideFrameCount)
		return;

	int centerX = 0;
	int centerY = 0;
	if (!guideCenterForFrame(_guideDirection, _guideFrameIndex, centerX, centerY))
		return;

	const byte *basePixels = framebufferPixels(_baseFramebuffer);
	byte *destinationPixels = framebufferPixels(_sceneFramebuffer);
	if (!basePixels || !destinationPixels)
		return;

	const int left = centerX - kScene2060GuideHalfSize;
	const int top = centerY - kScene2060GuideHalfSize;
	const int firstY = MAX<int>(0, top);
	const int lastY = MIN<int>((int)HollywoodEngine::kSceneBufferHeight - 1, centerY + kScene2060GuideHalfSize);

	for (int sceneY = firstY; sceneY <= lastY; ++sceneY) {
		const uint offset = sceneY * HollywoodEngine::kSceneBufferWidth + left;
		memcpy(destinationPixels + offset, basePixels + offset, kScene2060GuideSize);
	}
}

void Scene2060::drawGuideLayer() {
	if (!_guideEffectActive || _guideFrameIndex >= _guideFrameCount)
		return;
	if (_guideDirection >= kScene2060GuideChunkCount)
		return;

	const uint chunkIndex = kScene2060GuideChunkBase + _guideDirection;
	if (!_sceneChunkTable.isValidChunk(chunkIndex) ||
			_sceneChunkTable.sizes[chunkIndex] < kScene2060GuideMaskByteCount)
		return;

	int centerX = 0;
	int centerY = 0;
	if (!guideCenterForFrame(_guideDirection, _guideFrameIndex, centerX, centerY))
		return;

	const byte *basePixels = framebufferPixels(_baseFramebuffer);
	const byte *depthPixels = framebufferPixels(_savedFramebuffer);
	byte *destinationPixels = framebufferPixels(_sceneFramebuffer);
	if (!basePixels || !depthPixels || !destinationPixels)
		return;

	const byte *mask = _resourceArena.data() + _resourceChunkOffsets[chunkIndex];
	const int left = centerX - kScene2060GuideHalfSize;
	const int top = centerY - kScene2060GuideHalfSize;

	for (uint maskY = 0; maskY < kScene2060GuideSize; ++maskY) {
		const int sceneY = top + (int)maskY;
		if (sceneY < 0 || sceneY >= (int)HollywoodEngine::kSceneBufferHeight)
			continue;

		for (uint maskX = 0; maskX < kScene2060GuideSize; ++maskX) {
			const int sceneX = left + (int)maskX;
			if (sceneX < 0 || sceneX >= (int)HollywoodEngine::kSceneBufferWidth)
				continue;

			const uint offset = sceneY * HollywoodEngine::kSceneBufferWidth + sceneX;
			const byte sourceColor = basePixels[offset];
			const byte maskCode = mask[maskY * kScene2060GuideSize + maskX];
			byte depthClass = 0;
			const byte depthPixel = depthPixels[offset];
			if (depthPixel < _colorToActorDepthClassMap.size())
				depthClass = _colorToActorDepthClassMap[depthPixel];
			const uint16 depthThreshold = depthClass < _actorDepthYThresholds.size() ?
				_actorDepthYThresholds[depthClass] : 0;
			const bool foregroundIsBehindGuide = depthThreshold < centerY;

			byte outputColor = destinationPixels[offset];
			switch (maskCode) {
			case 0x00:
				if (centerY <= depthThreshold)
					outputColor = remapScenePaletteColor(sourceColor, 4);
				break;
			case 0x01:
				outputColor = foregroundIsBehindGuide ?
					remapScenePaletteColor(sourceColor, 1) : remapScenePaletteColor(sourceColor, 4);
				break;
			case 0x02:
				outputColor = foregroundIsBehindGuide ?
					remapScenePaletteColor(sourceColor, 2) : remapScenePaletteColor(sourceColor, 4);
				break;
			case 0x03:
				outputColor = foregroundIsBehindGuide ?
					remapScenePaletteColor(sourceColor, 3) : remapScenePaletteColor(sourceColor, 4);
				break;
			case 0xd0:
				outputColor = remapScenePaletteColor(sourceColor, 4);
				break;
			default:
				outputColor = foregroundIsBehindGuide ? maskCode : remapScenePaletteColor(sourceColor, 4);
				break;
			}
			destinationPixels[offset] = outputColor;
		}
	}
}

bool Scene2060::guideCenterForFrame(byte direction, byte frameIndex, int &centerX, int &centerY) const {
	if (direction >= kScene2060GuideChunkCount)
		return false;

	if ((direction == 2 || direction == 3) && frameIndex >= 30)
		return false;
	if (direction != 2 && direction != 3 && frameIndex >= 24)
		return false;

	switch (direction) {
	case 0:
		centerX = 0x017a;
		centerY = 0x0076 - frameIndex * 5;
		return true;
	case 1:
		centerX = 0x0210;
		centerY = 0x0076 - frameIndex * 5;
		return true;
	case 2:
		centerX = 0x00f9 - frameIndex * 5;
		centerY = 0x00f4;
		return true;
	case 3:
		centerX = 0x024d + frameIndex * 5;
		centerY = 0x00f0;
		return true;
	case 4:
		centerX = 0x0152;
		centerY = 0x016c + frameIndex * 5;
		return true;
	case 5:
		centerX = 0x01e8;
		centerY = 0x016c + frameIndex * 5;
		return true;
	default:
		return false;
	}
}

void Scene2060::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1 && _walkablePaletteMask[i] < 8)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene2060::runEntryPathAndGuide(int startX, int startY, byte startFacing, int targetX, int targetY) {
	runEntryPath(startX, startY, startFacing, targetX, targetY);
	playGuideEffectIfPrepared();
}

void Scene2060::runEntryPathWithInitialDrawOrder(int startX, int startY, byte startFacing,
		byte initialDrawOrder, int targetX, int targetY) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = initialDrawOrder;

	drawPlayableComposite();
	presentFrame();

	queueActorPathWithPaletteRegionRouting(startX, startY, targetX, targetY, kScene2060InvalidFacing, 0);
	_actorPathPlaybackActive = true;
	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		if (waitSceneMillis(kScene2060ActorPathFrameMillis)) {
			_actorPathPlaybackActive = false;
			return;
		}
	}
	_actorPathPlaybackActive = false;

	_activeActorWorldX = targetX;
	_activeActorWorldY = targetY;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_activeActorCel = 0;
	drawPlayableComposite();
	presentFrame();
	playGuideEffectIfPrepared();
}

void Scene2060::moveThroughPassage(int delta, uint16 nextState) {
	GameplayState &state = _vm->gameState();
	const byte currentIndex = state.egyptLabyrinthPositionIndex >= kScene2060MazeIndexCount ?
		kScene2060InitialMazeIndex : state.egyptLabyrinthPositionIndex;
	const int nextIndex = CLIP<int>((int)currentIndex + delta, 0, kScene2060MazeIndexCount - 1);
	state.egyptLabyrinthPositionIndex = (byte)nextIndex;
	state.mainFlowStateId = nextState;
	transitionToCurrentMazeState();
}

void Scene2060::transitionToCurrentMazeState() {
	applySceneStateToHotspotsAndPatches(0xff);
	drawPreviewComposite();
	presentFrame();
	runCustomEntrySequence();
	syncActiveActorPoseToGameState();
}

} // End of namespace Hollywood
