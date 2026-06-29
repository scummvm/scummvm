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

#include "hollywood/scenes/playable/sue_playable_scene.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/system.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

#include <math.h>

namespace Hollywood {

const char *const kResource000Name = "RESOURCE.000";
const char *const kStage003ArchiveName = "RESOURCE.003";
const char *const kGameplayMusicArchiveName = "RESOURCE.M07";
const char *const kGameplaySoundBank0ArchiveName = "RESOURCE.S07";
const uint16 kG04Chunk10DescriptorCount = 5;
const uint16 kG04Chunk11DescriptorCount = 0x1f;
const uint16 kG04Chunk12DescriptorCount = 3;
const uint16 kG04Chunk13DescriptorCount = 0x1c;
const uint16 kG04Chunk14ActionDescriptorCount = 0x32;
const uint16 kG04Chunk14AltDescriptorCount = 0x38;
const uint16 kG04Chunk16DescriptorCount = 0x0b;
const uint16 kG04Chunk17DescriptorCount = 9;
const uint16 kG04Chunk18DescriptorCount = 0x0a;
const uint16 kG05Chunk7DescriptorCount = 0x1b;
const uint16 kG05Chunk8DescriptorCount = 4;
const uint16 kG05Chunk11DescriptorCount = 0x0c;
const byte kG04AmbientMusicCueStillFrame = 0x0f;
const uint16 kG04State7041 = 0x1b81;
const uint16 kG04State7042 = 0x1b82;
const uint16 kG04ReturnState7031 = 0x1b77;
const uint16 kG04ExitState7050 = 0x1b8a;
const uint16 kG04ExitState7060 = 0x1b94;
const uint kSceneColorToItemMapOffset = 0x100;
const uint kSceneColorMapSize = 0x100;
const byte kG04SecondarySpeechTextColor = 0xfd;
const byte kG04PrimarySpeechTextColor = 0xfb;
const byte kG04PanelDarkColor = 0xe7;
const byte kG04PanelFillColor = 0xe8;
const byte kG04PanelSlotColor = 0xe9;
const byte kG04PanelLineColor = 0xea;
const byte kG04PanelSelectedColor = 0xf1;
const byte kG04PanelSelectedLineColor = 0xf2;
const byte kG04PanelTextColor = 0xfc;
const byte kG04Entry7040Facing = 1;
const int kG04Entry7040StartX = 100;
const int kG04Entry7040StartY = 0x1b1;
const int kG04Entry7040FirstTargetX = 0x14a;
const int kG04Entry7040FirstTargetY = 0x139;
const int kG04Entry7040RepeatTargetX = 0x16f;
const int kG04Entry7040RepeatTargetY = 0x177;
const byte kG04Entry7041Facing = 4;
const int kG04Entry7041StartX = 600;
const int kG04Entry7041StartY = 0x132;
const byte kG04Entry7042Facing = 4;
const int kG04Entry7042StartX = 0x322;
const int kG04Entry7042StartY = 0x1c9;
const int kG04Entry7042TargetX = 0x29e;
const int kG04Entry7042TargetY = 0x1cc;
const int kG05EntryX = 0x0a1;
const int kG05EntryY = 0x158;
const byte kG05EntryFacing = 2;
const int kG05MinimumWalkX = 0x7a;
const int kG05MaximumWalkX = 0x2c0;
const uint kResource000InventoryActionTablesEntry = 0xc8;
const uint kResource000FixedInventoryVerbTableOffset = 0xec54;
const uint32 kG04ActorPathFrameMillis = 60;
const uint32 kG04Chunk11FrameMillis = 75;
const uint32 kG04Chunk14FrameMillis = 75;
const uint32 kG04Chunk16FrameMillis = 75;
const uint32 kG04Chunk17FrameMillis = 125;
const uint32 kG04AmbientMusicCheckMillis = 250;
const uint32 kSecondaryActorSpeechFrameMillis = 150;
const byte kG04DialogueStageId = 0x62;
const byte kG04DialoguePrimaryRow = 99;
const uint16 kG04DialoguePrimaryCenterX = 0x1c2;
const uint16 kG04DialoguePrimaryTopY = 0x73;
const byte kG04DialoguePrimaryRed = 0x3f;
const byte kG04DialoguePrimaryGreen = 0x32;
const byte kG04DialoguePrimaryBlue = 0x0c;
const byte kG05DialoguePrimaryRed = 6;
const byte kG05DialoguePrimaryGreen = 0x3f;
const byte kG05DialoguePrimaryBlue = 0x2d;
const uint16 kG05DialoguePrimaryCenterX = 0x1d6;
const uint16 kG05DialoguePrimaryTopY = 0x95;
const uint16 kG05DialoguePrimaryAltCenterX = 0x1cb;
const uint16 kG05DialoguePrimaryAltTopY = 0x96;
const uint kG04DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kInvalidFacing = 0xff;
const byte kInvalidCel = 0xff;
const byte kInvalidPrimarySpeechAnimationGroup = 0xff;
const byte kG05PrimarySpeechNormalGroup = 7;
const byte kG05PrimarySpeechAltGroup = 8;
const byte kG04Chunk11FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 1, 28, 29, 30, 0, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
	27, 1, 0, 0, 0, 0, 0, 0
};
const byte kG04Chunk14ActionFrameMap[] = {
	49, 49, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 14, 18, 19, 20, 21, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 34, 33, 36, 35, 34, 35, 36, 34, 33, 36, 35,
	33, 36, 35, 33, 34, 35, 36, 34, 33, 34, 35, 36, 34, 33, 36, 35,
	34, 33, 34, 35, 36, 34, 33, 36, 35, 34, 35, 36, 34, 33, 36, 35,
	33, 36, 35, 33, 34, 35, 36, 34, 33, 34, 35, 36, 34, 33, 36, 35,
	34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 21, 22, 23, 24, 21, 37,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 0, 0, 0, 0
};
const byte kG04Chunk14AltFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	27, 26, 25, 24, 19, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 55, 55, 0
};
const byte kG04Chunk16PostItemFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 6,
	5, 4, 5, 6, 5, 4, 5, 6, 5, 4, 3, 2, 1, 0, 7, 8,
	9, 10, 7, 0, 0, 0, 0, 0, 0, 0
};
const byte kG04MajorHotspotFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 7, 6, 6, 7, 8,
	9, 8, 7, 6, 6, 7, 8, 9, 10, 9, 8, 7, 6, 6, 7, 8,
	9, 10, 11, 11, 11, 11, 12, 13, 14, 15, 16, 17, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27
};
const byte kG04Chunk10ExitFrameMap[] = { 0, 0, 1, 2, 3, 4 };
const byte kG04Chunk18PickupItem0FFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};
const byte kCloakroomAttendantFrameMap[] = {
	0, 0, 1, 2, 3, 25, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
	4, 18, 19, 20, 21, 22, 23, 24, 26, 20, 19, 18, 0
};
const byte kG05Chunk8ReturnFrameMap[] = { 0, 0, 1, 2, 3 };
const byte kG05Chunk11PickupItem10FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};
const byte kActorPathStepDeltaTableSet00[] = {
	2, 2, 2, 3, 3, 0, 2, 2, 2, 3, 3, 0,
	6, 7, 7, 5, 5, 5, 4, 6, 6, 5, 3, 4,
	5, 5, 5, 6, 2, 3, 4, 8, 10, 6, 3, 5,
	3, 0, 0, 3, 3, 3, 3, 0, 0, 3, 3, 3,
	4, 8, 10, 6, 3, 5, 5, 5, 5, 6, 2, 3,
	4, 6, 6, 5, 3, 4, 6, 7, 7, 5, 5, 5
};
const byte kActorFacingTurnTable[] = {
	0, 0, 0, 1, 0, 0, 1, 2, 0, 1, 2, 3, 5, 4, 0, 5, 0, 0,
	0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 3, 0, 0, 5, 4, 0, 5, 0,
	1, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 3, 4, 0, 1, 0, 5,
	4, 5, 0, 2, 1, 0, 2, 0, 0, 0, 0, 0, 4, 0, 0, 4, 5, 0,
	5, 0, 0, 5, 0, 1, 3, 2, 0, 3, 0, 0, 0, 0, 0, 5, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 1, 2, 4, 3, 0, 4, 0, 0, 0, 0, 0
};
const int8 kActorPathAxisDirectionByFacing[] = { -1, 1, 1, 1, -1, -1 };
const byte kActorInitialCelByFacing[] = { 0, 12, 12, 12, 1, 12 };
const float kActorPathDiagonalSlopeThreshold = 0.087488f;
const float kActorFacingSteepSlopeThreshold = 3.732051f;
const float kActorFacingDiagonalSlopeThreshold = 0.267949f;

int actorPathRoundToNearestEven(float value) {
	const float lower = floorf(value);
	const float fraction = value - lower;
	const int lowerInt = (int)lower;
	if (fraction > 0.5f)
		return lowerInt + 1;
	if (fraction < 0.5f)
		return lowerInt;
	return (lowerInt & 1) ? lowerInt + 1 : lowerInt;
}

SuePlayableScene::SuePlayableScene(HollywoodEngine *vm, const char *randomName, int defaultActorX, int defaultActorY,
		byte defaultActorFacing, byte secondarySpeechTextColor, byte primarySpeechTextColor) :
		_vm(vm),
		_resourceArenaCursor(0),
		_random(randomName),
		_primaryLeftSpeechLastFrame(0),
		_primaryDialogueSpeechLastFrame(7),
		_primaryLeftSpeechActive(false),
		_primaryDialogueSpeechActive(false),
		_primaryDialogueSpeechGroup(kInvalidPrimarySpeechAnimationGroup),
		_ambientMusicTimerAccumulator(0),
		_secondaryActorTimerAccumulator(0),
		_primaryLeftSpeechTimerAccumulator(0),
		_primaryDialogueSpeechTimerAccumulator(0),
		_previousAmbientMusicTrackId(0),
		_activeActorWorldX(defaultActorX),
		_activeActorWorldY(defaultActorY),
		_activeActorFacing(defaultActorFacing),
		_activeActorCel(0),
		_activeActorDrawOrderMode(0),
		_secondaryActorFrame(0),
		_actionOverlayVisible(false),
		_actionOverlayChunkIndex(0),
		_actionOverlayDescriptorCount(0),
		_actionOverlayFrameIndex(0),
		_chunk11FrameIndex(0),
		_chunk12FrameIndex(0),
		_chunk14ActionFrameIndex(0),
		_chunk14AltFrameIndex(0),
		_chunk14AltChunkIndex(14),
		_chunk16FrameIndex(1),
		_chunk17FrameIndex(0),
		_preItemIdleState(0),
		_postItemIdleState(0),
		_cloakroomAttendantFrame(1),
		_cloakroomAttendantState(0),
		_cloakroomAttendantRepeatCount(0),
		_chunk12OverlayVisible(false),
		_chunk14ActionVisible(false),
		_chunk14AltVisible(false),
		_hideActiveActor(false),
		_chunk11TimerAccumulator(0),
		_chunk12TimerAccumulator(0),
		_chunk16TimerAccumulator(0),
		_chunk17TimerAccumulator(0),
		_cloakroomAttendantTimerAccumulator(0),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_baseFramebufferOriginal.resize(kFrameBufferSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_paletteMaskOriginal.resize(0x700);
	_fullPaletteRegionMask.resize(kG04PaletteMaskUsedBytes);
	_walkablePaletteMask.resize(kG04PaletteMaskUsedBytes);
	_colorToActorDepthClassMap.resize(kScenePaletteMapPageSize);
	_actorDepthYThresholds.resize(kScenePaletteRegionCount);
	_drawActorDepthYThresholds.resize(kScenePaletteRegionCount);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
	_activeActorRunStreams.resize(kActorFacingCount * kActiveActorFacingRunStride);
	_secondaryActorRunStreams.resize(kActorFacingCount * kSecondaryActorFacingRunStride);
	_activeActorDescriptors.resize(kActorFacingCount * kActorCelsPerFacing);
	_secondaryActorDescriptors.resize(kActorFacingCount * kSecondaryActorFramesPerFacing);
	_stage003DecodeKey.resize(kStage003DecodeKeySize);
	_stage003StageBlock.resize(kStage003DescriptorTableSize);
	_sueSpeechCueDescriptors.resize(kSpeechCueDescriptorTableSize);
	_routeBoundaryPoints.resize(kSceneRouteBoundaryPointCount);
	_routeSteps.resize(kSceneRouteStepCount);
	_actorPathStepDeltas.resize(ARRAYSIZE(kActorPathStepDeltaTableSet00));
	memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());
	_speechOverlay.visible = false;
	_speechOverlay.colorIndex = secondarySpeechTextColor;
	_speechOverlay.centerX = 0;
	_speechOverlay.topY = 0;
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.colorIndex = primarySpeechTextColor;
	_primarySpeechOverlay.centerX = 0;
	_primarySpeechOverlay.topY = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
}

bool SuePlayableScene::play() {
	if (!load())
		return false;

	initializePreviewState();
	drawPreviewComposite();
	presentFrame();
	runEntryCutscene();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return true;

	_skipRequested = false;
	const bool result = runBasicGameplayLoop();
	if (!_vm->isSceneRestartRequested() && shouldRunExitSideEffectsAfterLoop())
		handleG04ExitSideEffects();
	return result;
}

bool SuePlayableScene::shouldLoadPaletteAfterFrankensteinNote() const {
	return false;
}

bool SuePlayableScene::shouldLoadInventoryActionTables() const {
	return true;
}

bool SuePlayableScene::shouldLoadActorDepthTables() const {
	return true;
}

bool SuePlayableScene::shouldConvertSavedFramebufferFF() const {
	return false;
}

bool SuePlayableScene::shouldRunExitSideEffectsAfterLoop() const {
	return false;
}

bool SuePlayableScene::usesActorDepthTest() const {
	return false;
}

bool SuePlayableScene::usesSingleSecondaryActorComposite() const {
	return false;
}

bool SuePlayableScene::usesG04PathRouteSpecialCase() const {
	return false;
}

bool SuePlayableScene::hasCustomPreviewState() const {
	return false;
}

void SuePlayableScene::initializeCustomPreviewState() {
}

bool SuePlayableScene::hasCustomComposite() const {
	return false;
}

void SuePlayableScene::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)drawActiveActor;
	(void)activeFacing;
	(void)activeCel;
	(void)activeWorldX;
	(void)activeWorldY;
	(void)drawSecondaryActor;
	(void)secondaryFacing;
	(void)secondaryFrame;
	(void)secondaryWorldX;
	(void)secondaryWorldY;
	(void)actorDrawOrderMode;
}

bool SuePlayableScene::shouldDrawSecondaryActorInPlayableComposite() const {
	return _speechOverlay.visible && !_actionOverlayVisible;
}

bool SuePlayableScene::hasCustomEntrySequence() const {
	return false;
}

void SuePlayableScene::runCustomEntrySequence() {
}

bool SuePlayableScene::prepareCustomGameplayLoop() {
	return false;
}

bool SuePlayableScene::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
	return false;
}

bool SuePlayableScene::dispatchCustomSceneAction(uint16 handlerId) {
	(void)handlerId;
	return false;
}

bool SuePlayableScene::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	(void)targetX;
	(void)targetY;
	return false;
}

bool SuePlayableScene::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)nextRegion;
	(void)state;
	(void)boundary;
	(void)requestedFacing;
	(void)restoredStepDeltas;
	return false;
}

bool SuePlayableScene::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)requestedFacing;
	(void)restoredStepDeltas;
	return false;
}

bool SuePlayableScene::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	return false;
}

bool SuePlayableScene::shouldAnimatePrimarySpeechLine() const {
	return true;
}

void SuePlayableScene::setPrimaryLeftSpeechFrame(byte frameIndex) {
	(void)frameIndex;
}

bool SuePlayableScene::load() {
	if (!loadResource000RuntimeTables(_resource000OffsetTable, _resource000SizeTable) ||
			!loadResource000ActorBankSet00(_resource000OffsetTable, _resource000SizeTable))
		return false;
	if (shouldLoadInventoryActionTables() &&
			!loadResource000InventoryActionTables(_resource000OffsetTable))
		return false;
	if (!_panelArt.load())
		return false;

	const char *archiveName = resourceArchiveName();
	if (!_vm->resources()->readChunkTable(Common::Path(archiveName), _sceneChunkTable)) {
		warning("Failed to read %s header", archiveName);
		return false;
	}

	for (uint i = 0; i < sceneInitialRequiredChunkCount(); ++i) {
		if (!_sceneChunkTable.isValidChunk(i)) {
			warning("%s is missing required %s chunk %u", archiveName, sceneDebugName(), i);
			return false;
		}
	}

	if (!loadFixedChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(1, _paletteResource, kPaletteSize) ||
			!loadVariableChunk(2, _fillRuns) ||
			!loadVariableChunk(3, _paletteMask) ||
			!loadVariableChunk(4, _metadata))
		return false;

	if (shouldLoadPaletteAfterFrankensteinNote() && _vm->gameState().reviewedFrankensteinNote &&
			!loadFixedChunk(19, _paletteResource, kPaletteSize))
		return false;

	_baseFramebufferOriginal = _baseFramebuffer;
	_paletteMaskOriginal = _paletteMask;

	if (_paletteMask.size() < kG04PaletteMaskUsedBytes) {
		warning("%s chunk 3 is shorter than the scene palette mask table", resourceArchiveName());
		return false;
	}
	if (shouldLoadActorDepthTables() && !initializeActorDepthTables())
		return false;
	if (!initializeScenePathTables())
		return false;

	uint32 arenaSize = 0;
	for (uint i = sceneArenaFirstChunk(); i <= sceneArenaLastChunk(); ++i)
		arenaSize += _sceneChunkTable.sizes[i];

	_resourceArena.resize(arenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	for (uint i = sceneArenaFirstChunk(); i <= sceneArenaLastChunk(); ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
	expandFillRunsToSavedFramebuffer();
	if (shouldConvertSavedFramebufferFF()) {
		for (uint i = 0; i < _savedFramebuffer.size(); ++i) {
			if (_savedFramebuffer[i] == 0xff)
				_savedFramebuffer[i] = 0xfa;
		}
	}
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (!loadResource000SueActorPalette(_resource000OffsetTable) ||
			!loadStage003SceneRows())
		return false;
	_panelArt.applyInteractiveObjectPalette(_paletteCurrent);

	if (!_hotspots.load(_paletteMask, _metadata, _stage003SmallRows))
		return false;

	_vm->gameplayMusic()->setArchive(Common::Path(kGameplayMusicArchiveName));
	_soundBank0.setArchive(Common::Path(kGameplaySoundBank0ArchiveName));
	_ambientSoundBank0.setArchive(Common::Path(kGameplaySoundBank0ArchiveName));

	debugC(1, kDebugScene, "%s loaded %s", sceneDebugName(), archiveName);
	return true;
}

bool SuePlayableScene::loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable) {
	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s for Scene 7040 actor resources", kResource000Name);
		return false;
	}

	if ((uint32)file.size() < 1 + (2 * kResource000TableByteCount)) {
		warning("%s is too small for Scene 7040 runtime tables", kResource000Name);
		return false;
	}

	file.seek(1);
	offsetTable.resize(kResource000TableByteCount);
	sizeTable.resize(kResource000TableByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s runtime tables for Scene 7040", kResource000Name);
		return false;
	}

	return true;
}

bool SuePlayableScene::loadResource000ActorBankSet00(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable) {
	if (kResource000ActorSet00TableEntry + 4 > offsetTable.size() ||
			kResource000ActorSet00TableEntry + kResource000ActorSet00SegmentCount * 4 > sizeTable.size()) {
		warning("%s actor bank set 00 table entries are out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s actor bank set 00", kResource000Name);
		return false;
	}

	const uint32 actorBankOffset = readUint32LE(offsetTable, kResource000ActorSet00TableEntry);
	if (actorBankOffset > (uint32)file.size()) {
		warning("%s actor bank set 00 offset is out of range", kResource000Name);
		return false;
	}

	file.seek(actorBankOffset);
	memset(_activeActorRunStreams.data(), 0, _activeActorRunStreams.size());
	memset(_secondaryActorRunStreams.data(), 0, _secondaryActorRunStreams.size());

	for (uint segment = 0; segment < kResource000ActorSet00SegmentCount; ++segment) {
		const uint32 segmentSize = readUint32LE(sizeTable, kResource000ActorSet00TableEntry + segment * 4);
		if (segment <= 5) {
			if (segmentSize > kActiveActorFacingRunStride) {
				warning("%s actor set 00 active run segment %u is too large", kResource000Name, segment);
				return false;
			}
			if (file.read(_activeActorRunStreams.data() + segment * kActiveActorFacingRunStride, segmentSize) != segmentSize) {
				warning("Failed to read %s actor set 00 active run segment %u", kResource000Name, segment);
				return false;
			}
		} else if (segment == 6) {
			if (segmentSize % kActiveActorDescriptorSize != 0) {
				warning("%s actor set 00 active descriptors have invalid size", kResource000Name);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("Failed to read %s actor set 00 active descriptors", kResource000Name);
				return false;
			}
			const uint descriptorCount = MIN<uint>(_activeActorDescriptors.size(), descriptors.size() / kActiveActorDescriptorSize);
			for (uint i = 0; i < descriptorCount; ++i) {
				const uint offset = i * kActiveActorDescriptorSize;
				_activeActorDescriptors[i].runStreamOffset = readUint32LE(descriptors, offset);
				_activeActorDescriptors[i].opaqueRunCount = readUint32LE(descriptors, offset + 4);
				_activeActorDescriptors[i].paletteRunCount = readUint32LE(descriptors, offset + 8);
				_activeActorDescriptors[i].anchorX = readSint16LE(descriptors, offset + 12);
				_activeActorDescriptors[i].anchorY = readSint16LE(descriptors, offset + 16);
				_activeActorDescriptors[i].width = readUint16LE(descriptors, offset + 20);
				_activeActorDescriptors[i].height = readUint16LE(descriptors, offset + 24);
			}
		} else if (segment <= 12) {
			const uint facing = segment - 7;
			if (segmentSize > kSecondaryActorFacingRunStride) {
				warning("%s actor set 00 secondary run segment %u is too large", kResource000Name, facing);
				return false;
			}
			if (file.read(_secondaryActorRunStreams.data() + facing * kSecondaryActorFacingRunStride, segmentSize) != segmentSize) {
				warning("Failed to read %s actor set 00 secondary run segment %u", kResource000Name, facing);
				return false;
			}
		} else {
			if (segmentSize % kSecondaryActorDescriptorSize != 0) {
				warning("%s actor set 00 secondary descriptors have invalid size", kResource000Name);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("Failed to read %s actor set 00 secondary descriptors", kResource000Name);
				return false;
			}
			const uint descriptorCount = MIN<uint>(_secondaryActorDescriptors.size(), descriptors.size() / kSecondaryActorDescriptorSize);
			for (uint i = 0; i < descriptorCount; ++i) {
				const uint offset = i * kSecondaryActorDescriptorSize;
				_secondaryActorDescriptors[i].runStreamOffset = readUint32LE(descriptors, offset);
				_secondaryActorDescriptors[i].runCount = readUint32LE(descriptors, offset + 4);
				_secondaryActorDescriptors[i].anchorX = readSint16LE(descriptors, offset + 8);
				_secondaryActorDescriptors[i].anchorY = readSint16LE(descriptors, offset + 12);
			}
		}
	}

	debugC(1, kDebugResources, "Loaded %s actor bank set 00 for Scene 7040", kResource000Name);
	return true;
}

bool SuePlayableScene::loadResource000SueActorPalette(const Common::Array<byte> &offsetTable) {
	if (kResource000SuePaletteTableEntry + 4 > offsetTable.size()) {
		warning("%s owner 1 palette table entry is out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s owner 1 palette", kResource000Name);
		return false;
	}

	const uint32 paletteOffset = readUint32LE(offsetTable, kResource000SuePaletteTableEntry);
	if (paletteOffset > (uint32)file.size() || kSueActorPaletteBytes > (uint32)file.size() - paletteOffset ||
			0x270 + kSueActorPaletteBytes > _paletteCurrent.size()) {
		warning("%s owner 1 palette is out of range", kResource000Name);
		return false;
	}

	file.seek(paletteOffset);
	if (file.read(_paletteCurrent.data() + 0x270, kSueActorPaletteBytes) != kSueActorPaletteBytes) {
		warning("Failed to read %s owner 1 palette", kResource000Name);
		return false;
	}

	return true;
}

bool SuePlayableScene::loadResource000InventoryActionTables(const Common::Array<byte> &offsetTable) {
	if (kResource000InventoryActionTablesEntry + 4 > offsetTable.size()) {
		warning("%s inventory action table entry is out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s inventory action tables", kResource000Name);
		return false;
	}

	const uint32 tableOffset = readUint32LE(offsetTable, kResource000InventoryActionTablesEntry);
	const uint32 fixedTableOffset = tableOffset + kResource000FixedInventoryVerbTableOffset;
	const uint fixedTableEntryCount = GameplayState::kFixedInventoryActionTableEntryCount - 1;
	const uint relationTableEntryCount = GameplayState::kInventoryItemRelationTableEntryCount;
	const uint32 requiredTableBytes = fixedTableEntryCount * 2 + relationTableEntryCount * 2 * 2;
	if (fixedTableOffset > (uint32)file.size() ||
			requiredTableBytes > (uint32)file.size() - fixedTableOffset) {
		warning("%s inventory action tables are out of range", kResource000Name);
		return false;
	}

	GameplayState &state = _vm->gameState();
	state.clearInventoryActionTables();

	file.seek(fixedTableOffset);
	for (uint i = 1; i < GameplayState::kFixedInventoryActionTableEntryCount; ++i)
		state.fixedInventoryVerbHandlerIdsByItemAndStrip[i] = file.readUint16LE();
	for (uint i = 0; i < GameplayState::kInventoryItemRelationTableEntryCount; ++i)
		state.dialogueRelationMode1HandlerIdsByItemPair[i] = file.readUint16LE();
	for (uint i = 0; i < GameplayState::kInventoryItemRelationTableEntryCount; ++i)
		state.dialogueRelationMode2HandlerIdsByItemPair[i] = file.readUint16LE();
	if (file.err()) {
		warning("Failed to read %s inventory action tables", kResource000Name);
		return false;
	}

	state.sueInventoryResourceTablesLoaded = true;
	return true;
}

bool SuePlayableScene::loadStage003SceneRows() {
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s for %s text", kStage003ArchiveName, sceneDebugName());
		return false;
	}

	if (file.read(_stage003DecodeKey.data(), _stage003DecodeKey.size()) != _stage003DecodeKey.size()) {
		warning("Failed to read %s row decode key", kStage003ArchiveName);
		return false;
	}

	if (kSueSpeechCueDescriptorTableOffset + kSpeechCueDescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s owner 1 speech cue table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(kSueSpeechCueDescriptorTableOffset);
	if (file.read(_sueSpeechCueDescriptors.data(), _sueSpeechCueDescriptors.size()) !=
			_sueSpeechCueDescriptors.size()) {
		warning("Failed to read %s owner 1 speech cue table", kStage003ArchiveName);
		return false;
	}

	const byte sueSmallRowCount = file.readByte();
	const uint16 sueLargeRowCount = file.readUint16LE();
	if (file.err()) {
		warning("Failed to read %s owner 1 text row counts", kStage003ArchiveName);
		return false;
	}

	const uint32 sueRowsOffsetEntry = kStage003DecodeKeySize + kSueResource003RowsOffsetIndex * 4;
	if (sueRowsOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			sueRowsOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s owner 1 text row offset entry is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(sueRowsOffsetEntry);
	const uint32 sueRowsOffset = file.readUint32LE();
	const uint32 sueSmallRowBytes = (uint32)sueSmallRowCount * kStage003SmallRowSize;
	const uint32 sueLargeRowBytes = (uint32)sueLargeRowCount * kStage003LargeRowSize;
	if (sueRowsOffset == 0 ||
			sueRowsOffset + sueSmallRowBytes + sueLargeRowBytes > (uint32)file.size()) {
		warning("%s owner 1 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	_sueSmallRows.resize((uint32)(sueSmallRowCount + 1) * kStage003SmallRowSize);
	memset(_sueSmallRows.data(), 0, _sueSmallRows.size());
	_sueLargeRows.resize((uint32)(sueLargeRowCount + 1) * kStage003LargeRowSize);
	memset(_sueLargeRows.data(), 0, _sueLargeRows.size());
	file.seek(sueRowsOffset);
	if (file.read(_sueSmallRows.data() + kStage003SmallRowSize, sueSmallRowBytes) != sueSmallRowBytes) {
		warning("Failed to read %s owner 1 small text rows", kStage003ArchiveName);
		return false;
	}
	if (file.read(_sueLargeRows.data() + kStage003LargeRowSize, sueLargeRowBytes) != sueLargeRowBytes) {
		warning("Failed to read %s owner 1 large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 1; row <= sueSmallRowCount; ++row) {
		for (uint column = 0; column < kStage003SmallRowSize; ++column)
			_sueSmallRows[row * kStage003SmallRowSize + column] -= _stage003DecodeKey[column];
	}

	for (uint row = 1; row <= sueLargeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_sueLargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	const uint stageIndex = sceneStageIndex();
	const uint32 stageOffsetEntry = kStage003DecodeKeySize + (stageIndex * 4);
	if (stageOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage %u offset entry", kStage003ArchiveName, stageIndex);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset + kStage003DescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s stage %u descriptor table is out of range", kStage003ArchiveName, stageIndex);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stage003StageBlock.data(), _stage003StageBlock.size()) != _stage003StageBlock.size()) {
		warning("Failed to read %s stage %u descriptor table", kStage003ArchiveName, stageIndex);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage %u text rows are out of range", kStage003ArchiveName, stageIndex);
		return false;
	}

	_stage003SmallRows.resize((uint32)(smallRowCount + 1) * kStage003SmallRowSize);
	memset(_stage003SmallRows.data(), 0, _stage003SmallRows.size());
	if (file.read(_stage003SmallRows.data() + kStage003SmallRowSize, smallRowBytes) != smallRowBytes) {
		warning("Failed to read %s stage %u small text rows", kStage003ArchiveName, stageIndex);
		return false;
	}

	_stage003LargeRows.resize(largeRowBytes);
	if (file.read(_stage003LargeRows.data(), _stage003LargeRows.size()) != _stage003LargeRows.size()) {
		warning("Failed to read %s stage %u large text rows", kStage003ArchiveName, stageIndex);
		return false;
	}

	for (uint row = 1; row <= smallRowCount; ++row) {
		for (uint column = 0; column < kStage003SmallRowSize; ++column)
			_stage003SmallRows[row * kStage003SmallRowSize + column] -= _stage003DecodeKey[column];
	}

	for (uint row = 0; row < largeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_stage003LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	debugC(1, kDebugResources, "Loaded %s stage %u text rows: smallRows=%u largeRows=%u",
		kStage003ArchiveName, stageIndex, smallRowCount, largeRowCount);
	return true;
}

bool SuePlayableScene::loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	const char *archiveName = resourceArchiveName();
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed %s destination", archiveName, index, sceneDebugName());
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool SuePlayableScene::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	const char *archiveName = resourceArchiveName();
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	destination.resize(stream->size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: size=%u", archiveName, index, (uint)stream->size());
	return true;
}

bool SuePlayableScene::loadArenaChunk(uint index) {
	const char *archiveName = resourceArchiveName();
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(archiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", archiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the %s resource arena", archiveName, index, sceneDebugName());
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", archiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		archiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

bool SuePlayableScene::initializeActorDepthTables() {
	if (_metadata.size() < kActorDepthThresholds + kScenePaletteRegionCount * 2) {
		warning("%s chunk 4 is too short for actor depth thresholds", resourceArchiveName());
		return false;
	}
	if (_paletteMask.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize) {
		warning("%s chunk 3 is too short for actor depth map", resourceArchiveName());
		return false;
	}

	for (uint i = 0; i < _actorDepthYThresholds.size(); ++i)
		_actorDepthYThresholds[i] = readUint16LE(_metadata, kActorDepthThresholds + i * 2);
	_drawActorDepthYThresholds = _actorDepthYThresholds;

	memcpy(_colorToActorDepthClassMap.data(),
		_paletteMask.data() + kSceneColorToActorDepthClassMap,
		_colorToActorDepthClassMap.size());
	return true;
}

void SuePlayableScene::updateActorDepthThresholds(byte actorDrawOrderMode) {
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	if (_drawActorDepthYThresholds.size() > 2)
		_drawActorDepthYThresholds[2] = actorDrawOrderMode == 6 ? 0x3e7 : 0x158;
}

void SuePlayableScene::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	while (destinationOffset < _savedFramebuffer.size() && sourceOffset + 3 <= _fillRuns.size()) {
		const byte fill = _fillRuns[sourceOffset];
		const uint16 runLength = readUint16LE(_fillRuns, sourceOffset + 1);
		sourceOffset += 3;

		const uint count = MIN<uint>(runLength, _savedFramebuffer.size() - destinationOffset);
		if (count != 0) {
			memset(_savedFramebuffer.data() + destinationOffset, fill, count);
			destinationOffset += count;
		}
	}
}

bool SuePlayableScene::initializeScenePathTables() {
	const uint boundaryBytes = kSceneRouteBoundaryPointCount * 4;
	if (_metadata.size() < kRouteBoundaryPoints + boundaryBytes ||
			_metadata.size() < kRouteBoundarySteps + kSceneRouteStepCount) {
		warning("%s chunk 4 is too short for path route tables", resourceArchiveName());
		return false;
	}

	memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _paletteMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 3)
			_walkablePaletteMask[i] = 0;
	}

	for (uint i = 0; i < _routeBoundaryPoints.size(); ++i) {
		const uint offset = kRouteBoundaryPoints + i * 4;
		_routeBoundaryPoints[i].x = readSint16LE(_metadata, offset);
		_routeBoundaryPoints[i].y = readSint16LE(_metadata, offset + 2);
	}
	memcpy(_routeSteps.data(), _metadata.data() + kRouteBoundarySteps, _routeSteps.size());
	return true;
}

void SuePlayableScene::initializePreviewState() {
	if (hasCustomPreviewState()) {
		initializeCustomPreviewState();
		return;
	}

	if (usesSingleSecondaryActorComposite()) {
		initializeG05PreviewState();
		return;
	}

	initializeG04PreviewState();
}

void SuePlayableScene::initializeG04PreviewState() {
	_primaryLeftSpeechLastFrame = 0;
	_primaryDialogueSpeechLastFrame = 7;
	_actionOverlayVisible = false;
	_actionOverlayChunkIndex = 0;
	_actionOverlayDescriptorCount = 0;
	_actionOverlayFrameIndex = 0;
	_chunk11FrameIndex = 0;
	_chunk12FrameIndex = 0;
	_chunk14ActionFrameIndex = 0;
	_chunk14AltFrameIndex = 0;
	_chunk14AltChunkIndex = 14;
	_chunk16FrameIndex = 1;
	_chunk17FrameIndex = 0;
	_preItemIdleState = 0;
	_postItemIdleState = 0;
	_chunk12OverlayVisible = false;
	_chunk14ActionVisible = false;
	_chunk14AltVisible = false;
	_hideActiveActor = false;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = kInvalidPrimarySpeechAnimationGroup;
	_ambientMusicTimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_chunk11TimerAccumulator = 0;
	_chunk12TimerAccumulator = 0;
	_chunk16TimerAccumulator = 0;
	_chunk17TimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	_activeActorWorldX = kG04Entry7040FirstTargetX;
	_activeActorWorldY = kG04Entry7040FirstTargetY;
	_activeActorFacing = kG04Entry7040Facing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void SuePlayableScene::initializeG05PreviewState() {
	_actionOverlayVisible = false;
	_actionOverlayChunkIndex = 0;
	_actionOverlayDescriptorCount = 0;
	_actionOverlayFrameIndex = 0;
	_hideActiveActor = false;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = kInvalidPrimarySpeechAnimationGroup;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_cloakroomAttendantFrame = 1;
	_cloakroomAttendantState = 0;
	_cloakroomAttendantRepeatCount = 0;
	_cloakroomAttendantTimerAccumulator = 0;
	_activeActorWorldX = kG05EntryX;
	_activeActorWorldY = kG05EntryY;
	_activeActorFacing = kG05EntryFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void SuePlayableScene::drawPreviewComposite() {
	drawCutsceneComposite(false, 0, 0, 0, 0, false, 0, 0, 0, 0);
}

void SuePlayableScene::drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	if (hasCustomComposite()) {
		drawCustomComposite(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY,
			actorDrawOrderMode);
		return;
	}

	if (usesSingleSecondaryActorComposite()) {
		(void)actorDrawOrderMode;
		drawG05Composite(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY);
		return;
	}

	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	if (_vm->gameState().reviewedFrankensteinNote) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[17], 0,
			kG04Chunk17DescriptorCount, _chunk17FrameIndex, _sceneFramebuffer);
		const byte frame = _chunk16FrameIndex < ARRAYSIZE(kG04Chunk16PostItemFrameMap) ?
			kG04Chunk16PostItemFrameMap[_chunk16FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[16], 0,
			kG04Chunk16DescriptorCount, frame, _sceneFramebuffer);
	} else {
		if (_chunk12OverlayVisible) {
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[12], 0,
				kG04Chunk12DescriptorCount, _chunk12FrameIndex, _sceneFramebuffer);
		}
		const byte frame = _chunk11FrameIndex < ARRAYSIZE(kG04Chunk11FrameMap) ?
			kG04Chunk11FrameMap[_chunk11FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[11], 0,
			kG04Chunk11DescriptorCount, frame, _sceneFramebuffer);
		if (_chunk14ActionVisible) {
			const byte actionFrame = _chunk14ActionFrameIndex < ARRAYSIZE(kG04Chunk14ActionFrameMap) ?
				kG04Chunk14ActionFrameMap[_chunk14ActionFrameIndex] : 0;
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[14], 0,
				kG04Chunk14ActionDescriptorCount, actionFrame, _sceneFramebuffer);
		}
		if (_chunk14AltVisible) {
			const byte altFrame = _chunk14AltFrameIndex < ARRAYSIZE(kG04Chunk14AltFrameMap) ?
				kG04Chunk14AltFrameMap[_chunk14AltFrameIndex] : 0;
			if (_chunk14AltChunkIndex < HollywoodEngine::kResourceChunkCount) {
				drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_chunk14AltChunkIndex], 0,
					kG04Chunk14AltDescriptorCount, altFrame, _sceneFramebuffer);
			}
		}
	}

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	if (_actionOverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
			_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
	}

	uint blockChunk = 5;
	if (actorDrawOrderMode == 2 || actorDrawOrderMode == 3) {
		blockChunk = activeWorldY <= 0x15f ? 6 : 0;
	} else if (actorDrawOrderMode == 6) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		blockChunk = _vm->gameState().officeNotePickupState == 1 ? 9 : 0;
	}
	if (blockChunk != 0)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void SuePlayableScene::drawActionOverlayComposite() {
	if (usesSingleSecondaryActorComposite()) {
		drawG05ActionOverlayComposite();
		return;
	}

	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	if (_chunk12OverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[12], 0,
			kG04Chunk12DescriptorCount, _chunk12FrameIndex, _sceneFramebuffer);
	}

	if (_vm->gameState().reviewedFrankensteinNote) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[17], 0,
			kG04Chunk17DescriptorCount, _chunk17FrameIndex, _sceneFramebuffer);
		const byte frame = _chunk16FrameIndex < ARRAYSIZE(kG04Chunk16PostItemFrameMap) ?
			kG04Chunk16PostItemFrameMap[_chunk16FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[16], 0,
			kG04Chunk16DescriptorCount, frame, _sceneFramebuffer);
		if (_actionOverlayVisible) {
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
				_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
		}
	} else {
		if (_actionOverlayVisible) {
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
				_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
		}
		const byte frame = _chunk11FrameIndex < ARRAYSIZE(kG04Chunk11FrameMap) ?
			kG04Chunk11FrameMap[_chunk11FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[11], 0,
			kG04Chunk11DescriptorCount, frame, _sceneFramebuffer);
	}

	uint blockChunk = 5;
	if (_activeActorDrawOrderMode == 2 || _activeActorDrawOrderMode == 3) {
		blockChunk = _activeActorWorldY <= 0x15f ? 6 : 0;
	} else if (_activeActorDrawOrderMode == 6) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		blockChunk = _vm->gameState().officeNotePickupState == 1 ? 9 : 0;
	}
	if (blockChunk != 0)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void SuePlayableScene::drawG05Composite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX,
		int activeWorldY, bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame,
		int secondaryWorldX, int secondaryWorldY) {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	const byte frame = _cloakroomAttendantFrame < ARRAYSIZE(kCloakroomAttendantFrameMap) ?
		kCloakroomAttendantFrameMap[_cloakroomAttendantFrame] : 0;
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[7], 0,
		kG05Chunk7DescriptorCount, frame, _sceneFramebuffer);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	const uint blockChunk = activeWorldX < 0x1a4 ? 5 : 6;
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void SuePlayableScene::drawG05ActionOverlayComposite() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	const byte frame = _cloakroomAttendantFrame < ARRAYSIZE(kCloakroomAttendantFrameMap) ?
		kCloakroomAttendantFrameMap[_cloakroomAttendantFrame] : 0;
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[7], 0,
		kG05Chunk7DescriptorCount, frame, _sceneFramebuffer);

	if (_actionOverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
			_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
	}

	const uint blockChunk = _activeActorWorldX < 0x1a4 ? 5 : 6;
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void SuePlayableScene::drawPlayableComposite() {
	const bool drawActiveActor = !_hideActiveActor;
	const bool drawSecondaryActor = shouldDrawSecondaryActorInPlayableComposite();
	drawCutsceneComposite(drawActiveActor, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
		drawSecondaryActor, _activeActorFacing, _secondaryActorFrame, _activeActorWorldX, _activeActorWorldY,
		_activeActorDrawOrderMode);
}

void SuePlayableScene::drawActiveAndSecondaryActorFrames(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame,
		int secondaryWorldX, int secondaryWorldY, int minimumYExclusive) {
	if (drawSecondaryActor) {
		const int secondaryActorBottomY = drawSecondaryActorFrame(secondaryFacing, secondaryFrame,
			secondaryWorldX, secondaryWorldY);
		if (drawActiveActor)
			drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, secondaryActorBottomY);
		return;
	}

	if (drawActiveActor)
		drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, minimumYExclusive);
}

void SuePlayableScene::drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= HollywoodEngine::kResourceChunkCount || frameIndex >= frameMapSize)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		descriptorCount, frameMap[frameIndex], _sceneFramebuffer);
}

void SuePlayableScene::drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive) {
	if (facing >= kActorFacingCount || cel >= kActorCelsPerFacing)
		return;

	const uint descriptorIndex = facing * kActorCelsPerFacing + cel;
	if (descriptorIndex >= _activeActorDescriptors.size())
		return;

	const ActiveActorSpriteDescriptor &descriptor = _activeActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	drawActorRun(_activeActorRunStreams, descriptor.runStreamOffset, facing * kActiveActorFacingRunStride,
		descriptor.opaqueRunCount, spriteX, spriteY, minimumYExclusive, worldY);
}

int SuePlayableScene::drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY) {
	if (facing >= kActorFacingCount || frame >= kSecondaryActorFramesPerFacing)
		return -1;

	const uint descriptorIndex = facing * kSecondaryActorFramesPerFacing + frame;
	if (descriptorIndex >= _secondaryActorDescriptors.size())
		return -1;

	const SecondaryActorSpriteDescriptor &descriptor = _secondaryActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	return drawActorRun(_secondaryActorRunStreams, descriptor.runStreamOffset, facing * kSecondaryActorFacingRunStride,
		descriptor.runCount, spriteX, spriteY, -1, worldY);
}

int SuePlayableScene::drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldY) {
	if (usesActorDepthTest()) {
		ActorDepthTest depthTest;
		depthTest.enabled = true;
		depthTest.savedFramebuffer = &_savedFramebuffer;
		depthTest.colorToDepthClassMap = &_colorToActorDepthClassMap;
		depthTest.depthYThresholds = &_drawActorDepthYThresholds;
		depthTest.actorWorldY = actorWorldY;

		return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
			minimumYExclusive, _sceneFramebuffer, &depthTest);
	}

	return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
		minimumYExclusive, _sceneFramebuffer, nullptr);
}

void SuePlayableScene::runEntryCutscene() {
	if (hasCustomEntrySequence()) {
		runCustomEntrySequence();
		return;
	}

	if (usesSingleSecondaryActorComposite()) {
		runG05EntrySequence();
		return;
	}

	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kG04State7041) {
		_soundBank0.playSample(4, 100);
		runEntryPath(kG04Entry7041StartX, kG04Entry7041StartY, kG04Entry7041Facing,
			kG04Entry7041StartX, kG04Entry7041StartY);
	} else if (state.mainFlowStateId == kG04State7042) {
		runEntryPath(kG04Entry7042StartX, kG04Entry7042StartY, kG04Entry7042Facing,
			kG04Entry7042TargetX, kG04Entry7042TargetY);
	} else {
		const int targetX = state.seenOfficeEntryConversation ?
			kG04Entry7040RepeatTargetX : kG04Entry7040FirstTargetX;
		const int targetY = state.seenOfficeEntryConversation ?
			kG04Entry7040RepeatTargetY : kG04Entry7040FirstTargetY;
		runEntryPath(kG04Entry7040StartX, kG04Entry7040StartY, kG04Entry7040Facing,
			targetX, targetY);
		state.seenOfficeEntryConversation = true;
	}
}

void SuePlayableScene::runG05EntrySequence() {
	_soundBank0.playSample(4, 100);
	_activeActorWorldX = kG05EntryX;
	_activeActorWorldY = kG05EntryY;
	_activeActorFacing = kG05EntryFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_cloakroomAttendantFrame = 1;
	_cloakroomAttendantState = 0;
	drawPlayableComposite();
	presentFrame();
	waitSceneMillis(kG04ActorPathFrameMillis);
}

void SuePlayableScene::runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);

	drawPlayableComposite();
	presentFrame();

	queueActorPathWithPaletteRegionRouting(startX, startY, targetX, targetY, kInvalidFacing, 0);
	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		if (waitSceneMillis(kG04ActorPathFrameMillis))
			return;
	}

	_activeActorWorldX = targetX;
	_activeActorWorldY = targetY;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_activeActorCel = 0;
	drawPlayableComposite();
	presentFrame();
}

bool SuePlayableScene::runBasicGameplayLoop() {
	GameplayLoop loop(_vm, this);
	return loop.run();
}

const SceneHotspotTable &SuePlayableScene::hotspots() const {
	return _hotspots;
}

const Common::Array<byte> &SuePlayableScene::savedFramebuffer() const {
	return _savedFramebuffer;
}

uint16 SuePlayableScene::viewportXOffset() const {
	return sceneViewportXOffset();
}

uint16 SuePlayableScene::viewportYOffset() const {
	return 0;
}

void SuePlayableScene::prepareGameplayLoop() {
	_skipRequested = false;
	clearAllSpeechOverlays();
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechGroup = kInvalidPrimarySpeechAnimationGroup;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	_actionOverlayVisible = false;
	_chunk12OverlayVisible = false;
	_chunk14ActionVisible = false;
	_chunk14AltVisible = false;
	_chunk14AltChunkIndex = 14;
	_hideActiveActor = false;
	if (prepareCustomGameplayLoop())
		return;
	if (usesSingleSecondaryActorComposite()) {
		_cloakroomAttendantTimerAccumulator = 0;
		if (_cloakroomAttendantFrame == 0)
			_cloakroomAttendantFrame = 1;
	}
}

void SuePlayableScene::advanceGameplayLoop(uint32 delta) {
	advanceSecondaryActorSpeechAnimation(delta);

	if (advanceCustomGameplayLoop(delta))
		return;

	if (usesSingleSecondaryActorComposite()) {
		if (_primaryDialogueSpeechActive)
			advancePrimaryDialogueSpeechFrame(delta);
		else
			advanceG05SecondaryActorAnimation(delta);
		updateAmbientAudioAndMusicCues(delta);
		return;
	}

	if (_vm->gameState().reviewedFrankensteinNote)
		advanceChunk16PostItemAnimation(delta);
	else if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceChunk11PreItemIdleAnimation(delta);

	updateAmbientAudioAndMusicCues(delta);
}

void SuePlayableScene::drawGameplayFrame() {
	drawPlayableComposite();
}

void SuePlayableScene::presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) {
	presentFrame(&hoverCaption, &panelState);
}

void SuePlayableScene::prepareOptionsMenuPalette(Common::Array<byte> &palette) const {
	palette = _paletteCurrent;
	_panelArt.applyInteractiveObjectPalette(palette);
}

bool SuePlayableScene::shouldExitGameplayLoop() const {
	if (_vm->isSceneRestartRequested())
		return true;

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !isMainFlowStateInScene(stateId);
}

Common::String SuePlayableScene::inventoryItemName(byte owner, byte itemId) const {
	if (owner != 1)
		return Common::String();

	const uint offset = (uint)itemId * kStage003SmallRowSize;
	if (offset >= _sueSmallRows.size())
		return Common::String();

	const byte *row = _sueSmallRows.data() + offset;
	uint length = 0;
	while (offset + length < _sueSmallRows.size() &&
			length < kStage003SmallRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

void SuePlayableScene::beginSharedInventorySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginStaticSecondarySpeechLine(rowIndex, frameIndex);
}

byte SuePlayableScene::randomSharedInventorySpeechFrame(byte maxFrameIndex) {
	return (byte)_random.getRandomNumber(maxFrameIndex);
}

void SuePlayableScene::playSharedInventorySound(byte sampleId) {
	_soundBank0.playSample(sampleId, 100);
}

void SuePlayableScene::handleLeftClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	_vm->cursor()->leaveInteractiveMode();
	processSceneActionClick(state);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_skipRequested = false;
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void SuePlayableScene::handleInventoryItemClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	_vm->cursor()->leaveInteractiveMode();
	dispatchSceneAction(state.inventoryActionHandlerId);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_skipRequested = false;
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void SuePlayableScene::processSceneActionClick(const GameplayLoopCursorState &state) {
	byte itemId = state.resolvedItem;
	if (state.relationModeActive) {
		processSceneRelationClick(state, itemId);
		return;
	}

	if (itemId == 0) {
		if (state.currentStrip != 1)
			return;

		int targetX = state.sceneX;
		int targetY = state.sceneY;
		adjustWalkTargetToFloorMask(targetX, targetY);
		walkActiveActorTo(targetX, targetY, kInvalidFacing, 0, true);
		return;
	}

	SceneVerbActionRecord actionRecord = _hotspots.verbActionRecord(itemId, state.currentStrip);
	if (actionRecord.actionHandlerId == 0)
		return;

	const SceneActionTarget target = _hotspots.actionTarget(itemId);
	int targetX = target.interactionPoint.x;
	int targetY = target.interactionPoint.y;
	byte finalFacing = kInvalidFacing;
	byte finalCel = kInvalidCel;

	if (actionRecord.movementMode == 0) {
		const bool atInteractionPoint =
			_activeActorWorldX == target.interactionPoint.x &&
			_activeActorWorldY == target.interactionPoint.y;
		if (atInteractionPoint) {
			if (_activeActorFacing != target.facing)
				finalFacing = target.facing;
		} else {
			targetX = _activeActorWorldX;
			targetY = _activeActorWorldY;
			if (target.approachPoint.x != 0 || target.approachPoint.y != 0) {
				finalFacing = calculateFacingTowardPoint(_activeActorWorldX, _activeActorWorldY,
					target.approachPoint.x, target.approachPoint.y);
			}
		}
	}
	if (actionRecord.movementMode == 1)
		finalFacing = target.facing;
	if (actionRecord.movementMode != 3)
		finalCel = 0;

	if (!walkActiveActorTo(targetX, targetY, finalFacing, finalCel, true))
		return;
	dispatchSceneAction(actionRecord.actionHandlerId);
}

void SuePlayableScene::processSceneRelationClick(const GameplayLoopCursorState &state, byte itemId) {
	if (itemId == 0)
		return;

	const SceneVerbActionRecord actionRecord =
		relationActionRecord(state.primaryInventoryItem, itemId, state.relationMode);
	if (actionRecord.actionHandlerId == 0)
		return;

	const SceneActionTarget target = _hotspots.actionTarget(itemId);
	int targetX = _activeActorWorldX;
	int targetY = _activeActorWorldY;
	byte finalFacing = kInvalidFacing;

	if (actionRecord.movementMode != 0) {
		targetX = target.interactionPoint.x;
		targetY = target.interactionPoint.y;
		finalFacing = target.facing;
	} else {
		const bool atInteractionPoint =
			_activeActorWorldX == target.interactionPoint.x &&
			_activeActorWorldY == target.interactionPoint.y;
		if (atInteractionPoint) {
			if (_activeActorFacing != target.facing)
				finalFacing = target.facing;
		} else if (target.approachPoint.x != 0 || target.approachPoint.y != 0) {
			finalFacing = calculateFacingTowardPoint(_activeActorWorldX, _activeActorWorldY,
				target.approachPoint.x, target.approachPoint.y);
		}
	}

	if (!walkActiveActorTo(targetX, targetY, finalFacing, 0, true))
		return;
	dispatchSceneAction(actionRecord.actionHandlerId);
}

SceneVerbActionRecord SuePlayableScene::relationActionRecord(byte inventoryItemId, byte sceneItemId, byte relationMode) const {
	return _hotspots.relationActionRecord(inventoryItemId, sceneItemId, relationMode);
}

void SuePlayableScene::dispatchSceneAction(uint16 handlerId) {
	if (dispatchCustomSceneAction(handlerId))
		return;

	if (dispatchGenericSceneAction(handlerId))
		return;

	warning("Unhandled %s action handler %u", sceneDebugName(), handlerId);
}

bool SuePlayableScene::dispatchGenericSceneAction(uint16 handlerId) {
	// Shared callback table installed by InstallSceneActionCallbackTable.
	switch (handlerId) {
	case 0:  // Shared no-op/default action slot.
	case 1:  // Usar/Dar inventory relation starter; no direct speech.
	case 35: // Mirar florero variant (look at vase): silent in original.
	case 41: // Mirar florero variant (look at vase): silent in original.
	case 45: // Mirar florero variant (look at vase): silent in original.
	case 49: // Mirar disco (look at record): silent in original.
		return true;
	case 2: // Generic failed use/combine: no effect.
		beginStaticSecondarySpeechLine(1, (byte)_random.getRandomNumber(1));
		return true;
	case 3: // Generic nonsensical action.
		beginStaticSecondarySpeechLine(2, 0);
		return true;
	case 4: // Generic impossible action/object too large.
		beginStaticSecondarySpeechLine(3, (byte)_random.getRandomNumber(1));
		return true;
	case 5: // Sue refuses risky physical action: impossible/stockings/nails.
	{
		const byte variant = (byte)_random.getRandomNumber(2);
		if (variant == 2)
			beginStaticSecondarySpeechLine(3, 1);
		else
			beginStaticSecondarySpeechLine(4, variant);
		return true;
	}
	case 6: // Coger inventory item already owned (take already-have item).
		beginStaticSecondarySpeechLine(5, 0);
		return true;
	case 7: // Abrir non-openable inventory object (open object).
		beginStaticSecondarySpeechLine(6, (byte)_random.getRandomNumber(1));
		return true;
	case 8: // Abrir bote de pintura (open paint can): already open.
		beginStaticSecondarySpeechLine(7, 0);
		return true;
	case 9: // Generic feminine object already open.
		beginStaticSecondarySpeechLine(8, 0);
		return true;
	case 10: // Cerrar non-closable inventory object (close object).
		beginStaticSecondarySpeechLine(9, (byte)_random.getRandomNumber(1));
		return true;
	case 11: // Cerrar closed masculine object: magnetófono/perfume/makeup/polish.
		beginStaticSecondarySpeechLine(0x0a, 0);
		return true;
	case 12: // Cerrar agenda/cartera (close notebook/wallet): already closed.
		beginStaticSecondarySpeechLine(0x0b, 0);
		return true;
	case 13: // Dar to uninterested target (give item): recipient would not want it.
		beginStaticSecondarySpeechLine(0x0c, (byte)_random.getRandomNumber(1));
		return true;
	case 14: // Dar owned item refusal: Sue wants to keep it.
		beginStaticSecondarySpeechLine(0x0d, (byte)_random.getRandomNumber(1));
		return true;
	case 15: // Generic immovable object.
		beginStaticSecondarySpeechLine(0x0e, 0);
		return true;
	case 16: // Hablar with object/non-responsive target (talk).
		beginStaticSecondarySpeechLine(0x0f, (byte)_random.getRandomNumber(2));
		return true;
	case 17: // Action requires an item Sue does not have yet.
		beginStaticSecondarySpeechLine(0x10, 0);
		return true;
	case 18: // Generic bad idea/refusal condition.
		beginStaticSecondarySpeechLine(0x11, (byte)_random.getRandomNumber(1));
		return true;
	case 19: // Keep item for later; it may be useful.
		beginStaticSecondarySpeechLine(0x12, (byte)_random.getRandomNumber(2));
		return true;
	case 20: // Wrong time for this action.
		beginStaticSecondarySpeechLine(0x13, 0);
		return true;
	case 21: // Wrong place for this action.
		beginStaticSecondarySpeechLine(0x14, 0);
		return true;
	case 22: // Action completed.
		beginStaticSecondarySpeechLine(0x15, 0);
		return true;
	case 23: // Generic no-reason/no-result action.
		beginStaticSecondarySpeechLine(0x16, (byte)_random.getRandomNumber(1));
		return true;
	case 24: // Generic unnecessary action.
		beginStaticSecondarySpeechLine(0x17, (byte)_random.getRandomNumber(1));
		return true;
	case 25: // Coger unavailable object (take object): cannot pick it up.
		beginStaticSecondarySpeechLine(0x18, (byte)_random.getRandomNumber(1));
		return true;
	case 26: // Usar magnetófono (use tape recorder): save tape for interviews.
		beginStaticSecondarySpeechLine(0x19, 0);
		return true;
	case 27: // Dar magnetófono (give tape recorder): Sue keeps it.
		beginStaticSecondarySpeechLine(0x1a, 0);
		return true;
	case 28: // Usar agenda (use notebook): no notes needed now.
		beginStaticSecondarySpeechLine(0x1b, 0);
		return true;
	case 29: // Dar agenda (give notebook): Sue keeps it.
		beginStaticSecondarySpeechLine(0x1c, 0);
		return true;
	case 30: // Usar/Dar cartera (use/give wallet): do not play with wallet.
		beginStaticSecondarySpeechLine(0x1d, 0);
		return true;
	case 31: // Mirar cartera (look at wallet): gift from Sue's father.
		beginStaticSecondarySpeechLine(0x1e, 0);
		return true;
	case 32: // Abrir cartera (open wallet): only documentation inside.
		beginStaticSecondarySpeechLine(0x1f, 0);
		return true;
	case 33: // Mirar/Abrir agenda (look/open notebook): no useful notes now.
		beginStaticSecondarySpeechLine(0x20, 0);
		return true;
	case 34: // Mirar florero (look at vase): empty.
		beginStaticSecondarySpeechLine(0x21, 0);
		return true;
	case 36: // Mirar invitación/pase de prensa (look at invitation/press pass).
		beginStaticSecondarySpeechLine(0x23, 0);
		return true;
	case 37: // Mirar hoja revelada (look at revealed Frankie note): read message.
		handleStaticSpeech43And24Sequence();
		return true;
	case 38: // Mirar magnetófono (look at tape recorder): enough tape left.
		beginStaticSecondarySpeechLine(0x25, 0);
		return true;
	case 39: // Mirar trapo con carbón (look at sooty rag): wrapped safely.
		beginStaticSecondarySpeechLine(0x26, 0);
		return true;
	case 40: // Mirar tarjeta (look at card): M.K.O./Otto J. Hannover.
		beginStaticSecondarySpeechLine(0x27, 0);
		return true;
	case 42: // Mirar hueso (look at bone): should return it to Húmero.
		beginStaticSecondarySpeechLine(0x29, 0);
		return true;
	case 43: // Mirar vaso vacío (look at empty glass).
		beginStaticSecondarySpeechLine(0x2a, 0);
		return true;
	case 44: // Mirar vaso con ponche (look at punch glass).
		beginStaticSecondarySpeechLine(0x2b, 0);
		return true;
	case 46: // Mirar hoja en blanco (look at blank sheet).
		beginStaticSecondarySpeechLine(0x2d, 0);
		return true;
	case 47: // Mirar trapo/gamuza (look at cloth).
		beginStaticSecondarySpeechLine(0x2e, 0);
		return true;
	case 48: // Mirar llave (look at key): Bruno will miss it.
		beginStaticSecondarySpeechLine(0x2f, 0);
		return true;
	case 50: // Mirar manivela (look at crank).
		beginStaticSecondarySpeechLine(0x31, 0);
		return true;
	case 51: // Mirar placa (look at plate/plaque): painted to blend into wall.
		beginStaticSecondarySpeechLine(0x32, 0);
		return true;
	case 52: // Mirar póster (look at poster): familiar face.
		beginStaticSecondarySpeechLine(0x33, 0);
		return true;
	case 53: // Mirar rata (look at rat): stunned after the hit.
		beginStaticSecondarySpeechLine(0x34, 0);
		return true;
	case 54: // Mirar bisturí (look at scalpel): Sue wonders where Ron got it.
		beginStaticSecondarySpeechLine(0x35, 0);
		return true;
	case 55: // Mirar pie de micro con algodón (look at mic stand with cotton).
		beginStaticSecondarySpeechLine(0x36, 0);
		return true;
	case 56: // Mirar pamela (look at hat): Sue keeps it.
		beginStaticSecondarySpeechLine(0x37, 0);
		return true;
	case 57: // Usar pamela (use hat): do not put it inside; it may stain.
		beginStaticSecondarySpeechLine(0x38, 0);
		return true;
	case 58: // Mirar navaja multiusos (look at multi-tool knife).
		beginStaticSecondarySpeechLine(0x39, 0);
		return true;
	case 59: // Mirar varita zahorí (look at divining rod).
		beginStaticSecondarySpeechLine(0x3a, 0);
		return true;
	case 60: // Mirar/Abrir frasco de perfume (look/open perfume bottle): empty.
		beginStaticSecondarySpeechLine(0x3b, 0);
		return true;
	case 61: // Mirar baraja de cartas (look at deck of cards).
		beginStaticSecondarySpeechLine(0x3c, 0);
		return true;
	case 62: // Mirar pinzas (look at tweezers): not for eyebrows.
		beginStaticSecondarySpeechLine(0x3d, 0);
		return true;
	case 63: // Mirar lupa (look at magnifying glass): smells of perfume.
		beginStaticSecondarySpeechLine(0x3e, 0);
		return true;
	case 64: // Mirar bote de pintura (look at black paint can).
		beginStaticSecondarySpeechLine(0x3f, 0);
		return true;
	case 65: // Mirar maletín de maquillaje (look at makeup case).
		beginStaticSecondarySpeechLine(0x40, 0);
		return true;
	case 66: // Abrir maletín de maquillaje: find pintauñas multicolor if missing.
		handleGrantItem22IfMissing();
		return true;
	case 67: // Mirar pintauñas multicolor (look at multicolor nail polish).
		beginStaticSecondarySpeechLine(0x42, 0);
		return true;
	case 68: // Usar trapo con carbón + hoja: reveal Frankie's note.
		handleSwapItems08And0FForItem06();
		return true;
	case 69: // Door/lock condition: no key needed, it is not locked.
		beginStaticSecondarySpeechLine(0x44, 0);
		return true;
	default:
		return false;
	}
}

bool SuePlayableScene::walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel, bool cancelOnSkip) {
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY, targetX, targetY,
		finalFacing, finalCel);

	if (_actorPathFrames.size() <= 1) {
		drawPlayableComposite();
		presentFrame();
		return true;
	}

	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		if (waitSceneMillis(kG04ActorPathFrameMillis)) {
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				return false;

			if (cancelOnSkip) {
				_skipRequested = false;
				drawPlayableComposite();
				presentFrame();
				return false;
			}

			const ActorPathFrame &lastFrame = _actorPathFrames.back();
			_activeActorWorldX = lastFrame.worldX;
			_activeActorWorldY = lastFrame.worldY;
			_activeActorFacing = lastFrame.facing;
			_activeActorCel = lastFrame.cel;
			_activeActorDrawOrderMode = lastFrame.drawOrderMode;
			drawPlayableComposite();
			presentFrame();
			return true;
		}
	}

	drawPlayableComposite();
	presentFrame();
	return !Engine::shouldQuit() && !_vm->isSceneRestartRequested();
}

void SuePlayableScene::adjustWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (adjustCustomWalkTargetToFloorMask(targetX, targetY))
		return;

	if (usesSingleSecondaryActorComposite()) {
		targetX = CLIP<int>(targetX, kG05MinimumWalkX, kG05MaximumWalkX);
		while (targetY < 0x1df) {
			const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
			if (offset < _savedFramebuffer.size() && _fullPaletteRegionMask[_savedFramebuffer[offset]] != 0)
				return;
			++targetY;
		}

		while (targetY > 0) {
			const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
			if (offset < _savedFramebuffer.size() && _fullPaletteRegionMask[_savedFramebuffer[offset]] != 0)
				return;
			--targetY;
		}
		return;
	}

	if (targetX > 0x30f)
		targetX = 0x30f;

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (offset < _savedFramebuffer.size() && _walkablePaletteMask[_savedFramebuffer[offset]] != 0)
			return;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (offset < _savedFramebuffer.size() && _walkablePaletteMask[_savedFramebuffer[offset]] != 0)
			return;
		--targetY;
	}
}

void SuePlayableScene::queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
		byte finalFacing, byte finalCel) {
	_actorPathFrames.clear();
	memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());

	ActorPathBuildState state;
	state.drawOrderMode = _activeActorDrawOrderMode;
	state.facing = _activeActorFacing;
	state.cel = nextActorPathCel(_activeActorCel);
	state.x = startX;
	state.y = startY;
	appendActorPathFrame(state);

	byte currentRegion = paletteRegionAt(startX, startY);
	if (currentRegion == 0)
		currentRegion = _activeActorDrawOrderMode;

	byte targetRegion = paletteRegionAt(targetX, targetY);
	if (targetRegion == 0)
		targetRegion = currentRegion;

	if (currentRegion != targetRegion) {
		const byte routeStartRegion = currentRegion;
		const byte routeTargetRegion = targetRegion;
		for (uint stepIndex = 0; stepIndex < kScenePaletteRegionRouteStepCount &&
				currentRegion != targetRegion; ++stepIndex) {
			const uint routeOffset =
				((uint)routeStartRegion * kScenePaletteRegionCount + routeTargetRegion) *
				kScenePaletteRegionRouteStepCount + stepIndex;
			if (routeOffset >= _routeSteps.size())
				break;

			const byte nextRegion = _routeSteps[routeOffset];
			if (nextRegion == 0 || nextRegion >= kScenePaletteRegionCount)
				break;

			state.drawOrderMode = currentRegion;
			const ScenePoint boundary = nextRegion == targetRegion ?
				bestPaletteRouteBoundaryPoint(state.x, state.y, targetX, targetY, currentRegion, nextRegion) :
				nearestPaletteRouteBoundaryPoint(state.x, state.y, currentRegion, nextRegion);

			byte segmentFinalFacing = kInvalidFacing;
			byte segmentFinalCel = kInvalidCel;
			if (boundary.x == targetX && boundary.y == targetY) {
				segmentFinalFacing = finalFacing;
				segmentFinalCel = finalCel;
			}

			int requestedFacing = -1;
			bool restoredStepDeltas = false;
			customizeRouteSegment(currentRegion, nextRegion, state, boundary,
				requestedFacing, restoredStepDeltas);
			if (usesG04PathRouteSpecialCase() && currentRegion == 3 && nextRegion == 3 &&
					state.x < boundary.x && boundary.y <= state.y)
				requestedFacing = 1;

			buildActorPathFramesBetweenPoints(state, boundary.x, boundary.y,
				segmentFinalFacing, segmentFinalCel, requestedFacing);
			if (restoredStepDeltas)
				memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());

			currentRegion = nextRegion;
		}
	}

	int requestedFacing = -1;
	bool restoredStepDeltas = false;
	customizeRouteFinal(currentRegion, targetRegion, state, targetX, targetY,
		requestedFacing, restoredStepDeltas);
	if (usesG04PathRouteSpecialCase() && currentRegion == 3 && targetRegion == 3 &&
			state.x < targetX && targetY <= state.y)
		requestedFacing = 1;

	state.drawOrderMode = currentRegion;
	buildActorPathFramesBetweenPoints(state, targetX, targetY, finalFacing, finalCel, requestedFacing);
	if (restoredStepDeltas)
		memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());

}

void SuePlayableScene::buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
		byte finalFacing, byte finalCel, int requestedFacing) {
	if (targetX == state.x && targetY == state.y) {
		if (finalFacing != kInvalidFacing && state.facing != finalFacing) {
			for (uint turnStep = 0; turnStep < 3 && state.facing != finalFacing; ++turnStep) {
				const uint turnOffset = ((uint)state.facing * kActorFacingCount + finalFacing) * 3 + turnStep;
				state.facing = kActorFacingTurnTable[turnOffset];
				state.cel = kActorInitialCelByFacing[state.facing];
				appendActorPathFrame(state);
				state.cel = nextActorPathCel(state.cel);
			}
		}
		if (finalCel != kInvalidCel)
			state.cel = finalCel;
		appendActorPathFrame(state);
		state.cel = nextActorPathCel(state.cel);
		return;
	}

	const byte movementFacing = calculateMovementFacingForPath(state.x, state.y, targetX, targetY, requestedFacing);
	if (state.facing != movementFacing) {
		for (uint turnStep = 0; turnStep < 3 && state.facing != movementFacing; ++turnStep) {
			const uint turnOffset = ((uint)state.facing * kActorFacingCount + movementFacing) * 3 + turnStep;
			state.facing = kActorFacingTurnTable[turnOffset];
			state.cel = kActorInitialCelByFacing[state.facing];
			appendActorPathFrame(state);
			state.cel = nextActorPathCel(state.cel);
		}
	}

	const int startX = state.x;
	const int startY = state.y;
	const int principalStart = (movementFacing == 0 || movementFacing == 3) ? startY : startX;
	const int principalTarget = (movementFacing == 0 || movementFacing == 3) ? targetY : targetX;
	const uint stepCount = calculateWalkStepCountForAxisDelta(principalStart, principalTarget,
		movementFacing, state.cel);

	if (stepCount != 0) {
		const bool verticalMovement = movementFacing == 0 || movementFacing == 3;
		const int secondaryDelta = verticalMovement ? ABS(startX - targetX) : ABS(startY - targetY);
		const float secondaryStep = (float)secondaryDelta / (float)stepCount;
		float secondaryAccumulator = secondaryStep;
		for (uint step = 1; step <= stepCount; ++step) {
			const int secondaryOffset = actorPathRoundToNearestEven(secondaryAccumulator);
			const int delta = actorPathStepDelta(movementFacing, state.cel);
			if (verticalMovement) {
				state.y += (startY < targetY) ? delta : -delta;
				state.x = startX < targetX ? startX + secondaryOffset : startX - secondaryOffset;
			} else {
				state.x += (startX < targetX) ? delta : -delta;
				state.y = startY < targetY ? startY + secondaryOffset : startY - secondaryOffset;
			}
			state.facing = movementFacing;
			appendActorPathFrame(state);
			state.cel = nextActorPathCel(state.cel);
			secondaryAccumulator += secondaryStep;
		}
	}

	state.x = targetX;
	state.y = targetY;
	if (finalFacing != kInvalidFacing && state.facing != finalFacing) {
		for (uint turnStep = 0; turnStep < 3 && state.facing != finalFacing; ++turnStep) {
			const uint turnOffset = ((uint)state.facing * kActorFacingCount + finalFacing) * 3 + turnStep;
			state.facing = kActorFacingTurnTable[turnOffset];
			state.cel = kActorInitialCelByFacing[state.facing];
			appendActorPathFrame(state);
			state.cel = nextActorPathCel(state.cel);
		}
	}
	if (finalCel != kInvalidCel)
		state.cel = finalCel;
	appendActorPathFrame(state);
	state.cel = nextActorPathCel(state.cel);
}

void SuePlayableScene::appendActorPathFrame(const ActorPathBuildState &state) {
	ActorPathFrame frame;
	frame.drawOrderMode = state.drawOrderMode;
	frame.facing = state.facing;
	frame.cel = state.cel;
	frame.worldX = (int16)CLIP<int>(state.x, -32768, 32767);
	frame.worldY = (int16)CLIP<int>(state.y, -32768, 32767);
	_actorPathFrames.push_back(frame);
}

ScenePoint SuePlayableScene::nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const {
	ScenePoint points[kScenePaletteRegionBoundaryCandidateCount];
	float scores[kScenePaletteRegionBoundaryCandidateCount];
	memset(points, 0, sizeof(points));
	memset(scores, 0, sizeof(scores));

	const uint baseIndex = ((uint)currentRegion * kScenePaletteRegionCount + nextRegion) *
		kScenePaletteRegionBoundaryCandidateCount;
	for (uint candidate = 0; candidate < kScenePaletteRegionBoundaryCandidateCount; ++candidate) {
		const uint pointIndex = baseIndex + candidate;
		if (pointIndex >= _routeBoundaryPoints.size())
			break;

		const ScenePoint point = _routeBoundaryPoints[pointIndex];
		points[candidate] = point;
		scores[candidate] =
			sqrtf((float)ABS(startX - point.x)) +
			sqrtf((float)ABS(startY - point.y));
	}

	if (scores[1] <= scores[0])
		return scores[2] < scores[1] ? points[2] : points[1];
	return scores[0] < scores[2] ? points[0] : points[2];
}

ScenePoint SuePlayableScene::bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
		byte currentRegion, byte targetRegion) const {
	ScenePoint points[kScenePaletteRegionBoundaryCandidateCount];
	float scores[kScenePaletteRegionBoundaryCandidateCount];
	memset(points, 0, sizeof(points));
	memset(scores, 0, sizeof(scores));

	const uint baseIndex = ((uint)currentRegion * kScenePaletteRegionCount + targetRegion) *
		kScenePaletteRegionBoundaryCandidateCount;
	for (uint candidate = 0; candidate < kScenePaletteRegionBoundaryCandidateCount; ++candidate) {
		const uint pointIndex = baseIndex + candidate;
		if (pointIndex >= _routeBoundaryPoints.size())
			break;

		const ScenePoint point = _routeBoundaryPoints[pointIndex];
		points[candidate] = point;
		scores[candidate] =
			sqrtf((float)ABS(startX - point.x)) +
			sqrtf((float)ABS(startY - point.y)) +
			sqrtf((float)ABS(targetX - point.x)) +
			sqrtf((float)ABS(targetY - point.y));
	}

	if (scores[1] <= scores[0])
		return scores[2] < scores[1] ? points[2] : points[1];
	return scores[0] < scores[2] ? points[0] : points[2];
}

byte SuePlayableScene::paletteRegionAt(int x, int y) const {
	if (x < 0 || y < 0 || x >= HollywoodEngine::kSceneBufferWidth || y >= HollywoodEngine::kSceneBufferHeight ||
			_fullPaletteRegionMask.empty())
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (offset >= _savedFramebuffer.size())
		return 0;

	return _fullPaletteRegionMask[_savedFramebuffer[offset]];
}

byte SuePlayableScene::calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const {
	if (requestedFacing >= 0)
		return (byte)requestedFacing;

	if (toX == fromX)
		return fromY < toY ? 3 : 0;

	const float slope = (float)ABS(toY - fromY) / (float)MAX<int>(1, ABS(toX - fromX));
	if (fromX < toX) {
		if (toY < fromY) {
			if (slope < 1.0f)
				return slope <= kActorPathDiagonalSlopeThreshold ? 2 : 1;
			return 0;
		}
		return slope < 1.0f ? 2 : 3;
	}

	if (toY < fromY) {
		if (slope > 1.0f)
			return 0;
		return slope > kActorPathDiagonalSlopeThreshold ? 5 : 4;
	}
	return slope > 1.0f ? 3 : 4;
}

uint SuePlayableScene::calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const {
	if (facing >= kActorFacingCount)
		return 0;

	const int direction = kActorPathAxisDirectionByFacing[facing];
	int remaining = (targetAxis - startAxis) * direction;
	if (remaining <= 0)
		return 0;

	uint steps = 0;
	byte nextCel = cel;
	while (actorPathStepDelta(facing, nextCel) < (uint)remaining) {
		remaining -= (int)actorPathStepDelta(facing, nextCel);
		nextCel = nextActorPathCel(nextCel);
		++steps;
		if (steps > 300)
			break;
	}

	return steps;
}

byte SuePlayableScene::nextActorPathCel(byte cel) const {
	return cel == 12 ? 1 : (byte)(cel + 1);
}

uint SuePlayableScene::actorPathStepDelta(byte facing, byte cel) const {
	if (facing >= kActorFacingCount || cel == 0 || cel > 12)
		return 0;

	const uint offset = (uint)facing * 12 + cel - 1;
	if (offset >= _actorPathStepDeltas.size())
		return 0;

	return _actorPathStepDeltas[offset];
}

byte SuePlayableScene::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
	if (toX == fromX)
		return fromY < toY ? 3 : 0;

	const float slope = (float)ABS(toY - fromY) / (float)MAX<int>(1, ABS(toX - fromX));
	if (toX > fromX) {
		if (toY < fromY)
			return slope > kActorFacingSteepSlopeThreshold ? 0 :
				(slope > kActorFacingDiagonalSlopeThreshold ? 1 : 2);
		return slope > kActorFacingSteepSlopeThreshold ? 3 : 2;
	}

	if (toY < fromY)
		return slope > kActorFacingSteepSlopeThreshold ? 0 :
			(slope > kActorFacingDiagonalSlopeThreshold ? 5 : 4);
	return slope > kActorFacingSteepSlopeThreshold ? 3 : 4;
}

void SuePlayableScene::applySceneStateToHotspotsAndPatches(byte selector) {
	if (applyCustomSceneStateToHotspotsAndPatches(selector))
		return;

	if (usesSingleSecondaryActorComposite()) {
		applyG05SceneStateToHotspotsAndPatches(selector);
		return;
	}

	if (selector == 0 || selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());

		// After the Frankenstein-note item sequence completes in G01, G04
		// remaps the left-side/doghouse colors and interaction points.
		for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i) {
			if (_paletteMaskOriginal[i] == 7)
				_fullPaletteRegionMask[i] = _vm->gameState().reviewedFrankensteinNote ? 0 : 1;
		}

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize &&
				_paletteMask.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize) {
			for (uint i = 0; i < kSceneColorMapSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMapOffset + i];
				if (!_vm->gameState().reviewedFrankensteinNote) {
					if (originalItem == 9)
						_paletteMask[kSceneColorToItemMapOffset + i] = 2;
					if (originalItem == 10)
						_paletteMask[kSceneColorToItemMapOffset + i] = 0;
				} else {
					if (originalItem == 8)
						_paletteMask[kSceneColorToItemMapOffset + i] = 0;
					if (originalItem == 9 || originalItem == 10)
						_paletteMask[kSceneColorToItemMapOffset + i] = 8;
				}
			}
		}

		if (_vm->gameState().reviewedFrankensteinNote) {
			if (_metadata.size() >= kSceneItemInteractionPoints + 9 * 4 &&
					_metadata.size() >= kSceneItemFacing + 3) {
				const uint item2Interaction = kSceneItemInteractionPoints + 2 * 4;
				_metadata[item2Interaction] = 0xf2;
				_metadata[item2Interaction + 1] = 0;
				_metadata[item2Interaction + 2] = 0x46;
				_metadata[item2Interaction + 3] = 1;
				const uint item8Interaction = kSceneItemInteractionPoints + 8 * 4;
				_metadata[item8Interaction] = 0xf2;
				_metadata[item8Interaction + 1] = 0;
				_metadata[item8Interaction + 2] = 0x46;
				_metadata[item8Interaction + 3] = 1;
				_metadata[kSceneItemFacing + 2] = 1;
			}
		}

		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	if (selector == 3 || selector == 0xff) {
		if (!_baseFramebufferOriginal.empty())
			memcpy(_baseFramebuffer.data(), _baseFramebufferOriginal.data(), _baseFramebuffer.size());

		if (_vm->gameState().officeNotePickupState == 1) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		} else {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		}
	}
}

void SuePlayableScene::applyG05SceneStateToHotspotsAndPatches(byte selector) {
	GameplayState &state = _vm->gameState();
	bool textRowsChanged = false;

	if ((selector == 0 || selector == 0xff) && state.spokenToCloakroomAttendant &&
			_stage003SmallRows.size() >= 0xcd + kStage003SmallRowSize &&
			_stage003SmallRows.size() >= 0x52 + kStage003SmallRowSize) {
		const byte *source = _stage003SmallRows.data() + 0xcd;
		byte *destination = _stage003SmallRows.data() + 0x52;
		uint length = 0;
		while (length < kStage003SmallRowSize && source[length] != 0)
			++length;
		if (length < kStage003SmallRowSize)
			++length;
		memcpy(destination, source, length);
		textRowsChanged = true;
	}

	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		if (!_baseFramebufferOriginal.empty())
			memcpy(_baseFramebuffer.data(), _baseFramebufferOriginal.data(), _baseFramebuffer.size());

		if (state.cloakroomRagVisible != 0) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
		} else {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
			if (_paletteMaskOriginal.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize &&
					_paletteMask.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize) {
				for (uint i = 0; i < kSceneColorMapSize; ++i) {
					if (_paletteMaskOriginal[kSceneColorToItemMapOffset + i] == 3)
						_paletteMask[kSceneColorToItemMapOffset + i] = 4;
				}
			}
		}

		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		textRowsChanged = false;
	}

	if (textRowsChanged)
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
}

void SuePlayableScene::rebuildWalkablePaletteMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 3)
			_walkablePaletteMask[i] = 0;
	}
}

bool SuePlayableScene::hasInventoryItem(byte itemId) const {
	const byte owner = _vm->gameState().currentInventoryOwnerIndex;
	return _vm->gameState().hasInventoryItem(owner, itemId);
}

void SuePlayableScene::addInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.addInventoryItem(state.currentInventoryOwnerIndex, itemId);
}

void SuePlayableScene::removeInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.removeInventoryItem(state.currentInventoryOwnerIndex, itemId);
}

void SuePlayableScene::handleActionSlot00ReturnToG03() {
	_vm->gameState().mainFlowStateId = kG04ReturnState7031;
}

void SuePlayableScene::handleActionSlot01ProgressSpeech() {
	beginSecondarySpeechLine(1, _vm->gameState().officeStatueActionProgress == 0 ? 0 : 1);
}

void SuePlayableScene::handleActionSlot02MajorHotspotAction() {
	GameplayState &state = _vm->gameState();
	if (state.reviewedFrankensteinNote) {
		beginSecondarySpeechLine(3, 0x0b);
		return;
	}

	_chunk12OverlayVisible = true;
	if (state.officeStatueActionProgress == 2) {
		runMappedActionOverlayRange(13, kG04Chunk13DescriptorCount, kG04MajorHotspotFrameMap,
			ARRAYSIZE(kG04MajorHotspotFrameMap), kG04Chunk14FrameMillis, 0, 0x2d, -1, false);
		_soundBank0.playSample(0x15, 100);
		runMajorHotspotFrankensteinBranch();
		_chunk12OverlayVisible = true;
		runMappedActionOverlayRange(13, kG04Chunk13DescriptorCount, kG04MajorHotspotFrameMap,
			ARRAYSIZE(kG04MajorHotspotFrameMap), kG04Chunk14FrameMillis, 0x35,
			ARRAYSIZE(kG04MajorHotspotFrameMap), -1, false);
	} else {
		runMappedActionOverlay(13, kG04Chunk13DescriptorCount, kG04MajorHotspotFrameMap,
			ARRAYSIZE(kG04MajorHotspotFrameMap), kG04Chunk14FrameMillis, 0x2c, false);
	}
	_chunk12OverlayVisible = false;

	walkActiveActorTo(0x10d, 0x124, state.officeStatueActionProgress == 2 ? 4 : 5, 0);
	switch (state.officeStatueActionProgress) {
	case 0:
		beginSecondarySpeechLine(2, 0);
		state.officeStatueActionProgress = 1;
		break;
	case 1:
		beginSecondarySpeechLine(2, 1);
		state.officeStatueActionProgress = 2;
		break;
	case 2:
		beginSecondarySpeechLine(3, 10);
		state.officeStatueActionProgress = 3;
		// Original RunG04MajorHotspotActionSequence primes the
		// Frankenstein-note overlay and later yard look lines here.
		state.frankensteinNoteOverlayMode = 1;
		break;
	default:
		beginSecondarySpeechLine(2, 2);
		break;
	}
}

void SuePlayableScene::handleActionSlot03TransitionToState7060() {
	_vm->gameState().mainFlowStateId = kG04ExitState7060;
}

void SuePlayableScene::handleActionSlot05ExitProgressSpeech() {
	beginSecondarySpeechLine(5, _vm->gameState().openedOfficeClosetDoor ? 1 : 0);
}

void SuePlayableScene::handleActionSlot06TransitionToG05() {
	runMappedActionOverlay(10, kG04Chunk10DescriptorCount, kG04Chunk10ExitFrameMap,
		ARRAYSIZE(kG04Chunk10ExitFrameMap), kG04Chunk14FrameMillis, -1, false);
	_vm->gameState().openedOfficeClosetDoor = true;
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kG04ExitState7050;
}

void SuePlayableScene::handleActionSlot09PickupItem0FThenExit() {
	GameplayState &state = _vm->gameState();
	if (state.officeStatueActionProgress <= 2 || state.officeNotePickupState == 2) {
		beginSecondarySpeechLine(9, 0);
		return;
	}
	if (!state.reviewedFrankensteinNote) {
		beginSecondarySpeechLine(8, 0);
		return;
	}

	beginSecondarySpeechLine(8, 1);
	runMappedActionOverlay(18, kG04Chunk18DescriptorCount, kG04Chunk18PickupItem0FFrameMap,
		ARRAYSIZE(kG04Chunk18PickupItem0FFrameMap), kG04Chunk14FrameMillis, -1, false);
	addInventoryItem(0x0f);
	_soundBank0.playSample(1, 100);
	state.officeNotePickupState = 2;
	beginSecondarySpeechLine(8, 2);
	walkActiveActorTo(600, 0x132, kInvalidFacing, 0);
	handleActionSlot06TransitionToG05();
}

void SuePlayableScene::handleActionSlot10CommonSpeech() {
	beginSecondarySpeechLine(9, 0);
}

void SuePlayableScene::handleActionHandler312ProgressSpeech() {
	GameplayState &state = _vm->gameState();
	if (state.officeStatueActionProgress == 3)
		beginSecondarySpeechLine(10, state.officeNotePickupState >= 2 ? 1 : 0);
	else
		beginStaticSecondarySpeechLine(0x2d, 0);
}

void SuePlayableScene::handleActionHandler313ConversationGate() {
	if (_vm->gameState().reviewedFrankensteinNote) {
		beginSecondarySpeechLine(11, 2);
		return;
	}
	runDialogueMenuRow98();
}

void SuePlayableScene::handleActionHandler314FrankensteinNoteSpeech() {
	beginSecondarySpeechLine(11, _vm->gameState().reviewedFrankensteinNote ? 1 : 0);
}

void SuePlayableScene::handleActionHandler315PickupItem0C() {
	if (hasInventoryItem(0x0c))
		return;

	addInventoryItem(0x0c);
	_soundBank0.playSample(1, 100);
}

void SuePlayableScene::handleStaticSpeech43And24Sequence() {
	beginStaticSecondarySpeechLine(0x43, 1);
	beginStaticSecondarySpeechLine(0x24, 0);
	beginStaticSecondarySpeechLine(0x43, 2);
}

void SuePlayableScene::handleGrantItem22IfMissing() {
	if (hasInventoryItem(0x22)) {
		beginStaticSecondarySpeechLine(0x41, 1);
		return;
	}

	addInventoryItem(0x22);
	_soundBank0.playSample(1, 100);
	beginStaticSecondarySpeechLine(0x41, 0);
}

void SuePlayableScene::handleSwapItems08And0FForItem06() {
	beginStaticSecondarySpeechLine(0x43, 0);
	removeInventoryItem(0x08);
	removeInventoryItem(0x0f);
	addInventoryItem(0x06);
	_soundBank0.playSample(1, 100);
	handleStaticSpeech43And24Sequence();
}

void SuePlayableScene::runDialogueMenuRow98() {
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	beginSecondarySpeechLine(kG04DialogueStageId, 0);
	_preItemIdleState = 3;
	_chunk11FrameIndex = 7;
	beginPrimarySpeechLine(kG04DialoguePrimaryRow, 0, kG04DialoguePrimaryCenterX,
		kG04DialoguePrimaryTopY, kG04DialoguePrimaryRed, kG04DialoguePrimaryGreen,
		kG04DialoguePrimaryBlue);

	while (!finished && !Engine::shouldQuit()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kG04DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kG04DialogueStageId, 5);
			beginPrimarySpeechLine(kG04DialoguePrimaryRow, 5, kG04DialoguePrimaryCenterX,
				kG04DialoguePrimaryTopY, kG04DialoguePrimaryRed, kG04DialoguePrimaryGreen,
				kG04DialoguePrimaryBlue);
			_chunk11FrameIndex = 0;
			_preItemIdleState = 0;
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kG04DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			beginPrimarySpeechLine(kG04DialoguePrimaryRow, record.responseFrameIndex,
				kG04DialoguePrimaryCenterX, kG04DialoguePrimaryTopY, kG04DialoguePrimaryRed,
				kG04DialoguePrimaryGreen, kG04DialoguePrimaryBlue);
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

	_chunk11FrameIndex = 0;
	_preItemIdleState = 0;
}

void SuePlayableScene::runG05DialogueMenuRow98() {
	Common::Array<DialogueChoiceRecord> records;
	initializeG05DialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	GameplayState &state = _vm->gameState();
	if (!state.spokenToCloakroomAttendant) {
		beginSecondarySpeechLine(kG04DialogueStageId, 0);
		beginG05PrimarySpeechLine(0, false);
		state.spokenToCloakroomAttendant = true;
	} else {
		beginSecondarySpeechLine(kG04DialogueStageId, 1);
		beginG05PrimarySpeechLine(1, false);
	}

	while (!finished && !Engine::shouldQuit()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kG04DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kG04DialogueStageId, 4);
			beginG05PrimarySpeechLine(4, false);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kG04DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			const bool alternatePose = record.transitionMode != 0;
			if (alternatePose)
				runG05SecondaryActorPoseIn();
			beginG05PrimarySpeechLine(record.responseFrameIndex, alternatePose);
			if (alternatePose)
				runG05SecondaryActorPoseOut();
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

void SuePlayableScene::initializeG05DialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kG04DialogueChoiceRecordCount);

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
	records[2].transitionMode = 0;
	records[2].playerTextRowId = 4;
	records[2].responseFrameIndex = 4;
	records[2].reserved = 0xff;
}

void SuePlayableScene::runG05SecondaryActorPoseIn() {
	_cloakroomAttendantFrame = 0x20;
	_cloakroomAttendantState = 5;
	for (byte frame = 0x20; frame <= 0x24 && !Engine::shouldQuit(); ++frame) {
		_cloakroomAttendantFrame = frame;
		if (waitSceneMillis(kG04Chunk11FrameMillis))
			break;
	}
	_cloakroomAttendantFrame = 0x24;
}

void SuePlayableScene::runG05SecondaryActorPoseOut() {
	for (byte frame = 0x28; frame <= 0x2c && !Engine::shouldQuit(); ++frame) {
		_cloakroomAttendantFrame = frame;
		if (waitSceneMillis(kG04Chunk11FrameMillis))
			break;
	}
	_cloakroomAttendantFrame = 1;
	_cloakroomAttendantState = 0;
}

void SuePlayableScene::beginG05PrimarySpeechLine(byte frameIndex, bool alternatePose) {
	const byte group = alternatePose ? kG05PrimarySpeechAltGroup : kG05PrimarySpeechNormalGroup;
	beginPrimarySpeechLineWithAnimationGroup(kG04DialoguePrimaryRow, frameIndex,
		alternatePose ? kG05DialoguePrimaryAltCenterX : kG05DialoguePrimaryCenterX,
		alternatePose ? kG05DialoguePrimaryAltTopY : kG05DialoguePrimaryTopY,
		kG05DialoguePrimaryRed, kG05DialoguePrimaryGreen, kG05DialoguePrimaryBlue, group);
}

void SuePlayableScene::handleG05ActionSlot01ReturnToG04() {
	runMappedActionOverlay(8, kG05Chunk8DescriptorCount, kG05Chunk8ReturnFrameMap,
		ARRAYSIZE(kG05Chunk8ReturnFrameMap), kG04Chunk14FrameMillis, -1, false);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kG04State7041;
}

void SuePlayableScene::handleG05ActionSlot10PickupItem10() {
	dispatchGenericSceneAction(19);
	for (uint frame = 0; frame < ARRAYSIZE(kG05Chunk11PickupItem10FrameMap) && !Engine::shouldQuit(); ++frame) {
		_actionOverlayVisible = true;
		_actionOverlayChunkIndex = 11;
		_actionOverlayDescriptorCount = kG05Chunk11DescriptorCount;
		_actionOverlayFrameIndex = kG05Chunk11PickupItem10FrameMap[frame];
		if (frame == 4) {
			_vm->gameState().cloakroomRagVisible = 0;
			applySceneStateToHotspotsAndPatches(1);
		}
		if (waitSceneMillis(kG04Chunk14FrameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayFrameIndex = 0;
	addInventoryItem(0x10);
	_soundBank0.playSample(1, 100);
	drawPlayableComposite();
	presentFrame();
}

void SuePlayableScene::handleG04ExitSideEffects() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kG04ExitState7050 &&
			state.reviewedFrankensteinNote && state.officeNotePickupState == 2) {
		state.reviewedFrankensteinNote = false;
	}
}

void SuePlayableScene::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kG04DialogueChoiceRecordCount);

	records[0].transitionMode = 3;
	records[0].playerTextRowId = 1;
	records[0].responseFrameIndex = 1;
	records[0].disableAfterUse = 1;
	records[0].reserved = 0xff;

	records[1].transitionMode = 3;
	records[1].playerTextRowId = 2;
	records[1].responseFrameIndex = 2;
	records[1].disableAfterUse = 1;
	records[1].reserved = 0xff;

	records[2].enabled = 1;
	records[2].transitionMode = 3;
	records[2].playerTextRowId = 3;
	records[2].responseFrameIndex = 3;
	records[2].disableAfterUse = 1;
	records[2].reserved = 0xff;

	records[3].transitionMode = 3;
	records[3].playerTextRowId = 4;
	records[3].responseFrameIndex = 4;
	records[3].disableAfterUse = 1;
	records[3].reserved = 0xff;

	records[4].enabled = 1;
	records[4].transitionMode = 0;
	records[4].playerTextRowId = 5;
	records[4].responseFrameIndex = 5;
	records[4].reserved = 0xff;

	const GameplayState &state = _vm->gameState();
	if (state.officeStatueActionProgress != 0)
		records[0].enabled = 1;
	if (state.officeStatueActionProgress == 3) {
		if (!state.hasInventoryItem(state.currentInventoryOwnerIndex, 6))
			records[1].enabled = 1;
		if (state.officeNotePickupState != 2)
			records[3].enabled = 1;
	}
}

Common::String SuePlayableScene::dialogueMenuText(byte stageId, byte textRowId) const {
	const uint offset = ((uint)stageId * 100 + textRowId) * 5;
	if (offset + 5 > _stage003StageBlock.size())
		return Common::String();

	const uint16 textRecordId = readUint16LE(_stage003StageBlock, offset);
	return getResource003LargeTextRecord(textRecordId);
}

void SuePlayableScene::advanceDialogueMenu(uint32 delta) {
	advanceGameplayLoop(delta);
}

void SuePlayableScene::drawDialogueMenuFrame() {
	drawPlayableComposite();
}

void SuePlayableScene::presentDialogueMenuFrame(const DialogueMenuState &state) {
	presentFrame(nullptr, nullptr, &state);
}

void SuePlayableScene::runMappedActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame) {
	runMappedActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis, statePatchFrame, false);
}

void SuePlayableScene::runMappedActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame, bool hideActiveActor) {
	runMappedActionOverlayRange(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis,
		0, frameMapSize, statePatchFrame, hideActiveActor);
	drawPlayableComposite();
	presentFrame();
}

void SuePlayableScene::runMappedActionOverlayRange(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, uint firstFrame, uint endFrame, int statePatchFrame, bool hideActiveActor) {
	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = hideActiveActor;
	_actionOverlayVisible = true;
	_actionOverlayChunkIndex = (byte)chunkIndex;
	_actionOverlayDescriptorCount = (byte)descriptorCount;
	const uint cappedEndFrame = MIN<uint>(endFrame, frameMapSize);
	for (uint frame = firstFrame; frame < cappedEndFrame && !Engine::shouldQuit(); ++frame) {
		_actionOverlayFrameIndex = frameMap[frame];
		if (statePatchFrame >= 0 && (int)frame == statePatchFrame) {
			_soundBank0.playSample(0x15, 100);
			if (_vm->gameState().officeStatueActionProgress == 2) {
				_vm->gameState().officeNotePickupState = 1;
				applySceneStateToHotspotsAndPatches(3);
			}
		}
		if (waitSceneMillis(frameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayFrameIndex = 0;
	_hideActiveActor = previousHideActiveActor;
}

void SuePlayableScene::runMajorHotspotFrankensteinBranch() {
	const bool previousHideActiveActor = _hideActiveActor;
	const byte previousPreItemIdleState = _preItemIdleState;
	const byte previousAltChunkIndex = _chunk14AltChunkIndex;
	_hideActiveActor = true;
	_preItemIdleState = 3;
	_chunk12OverlayVisible = true;
	_chunk12FrameIndex = 0;
	_chunk14ActionVisible = true;
	_chunk14AltVisible = false;

	runChunk14ActionRange(0, 0x10);
	beginPrimarySpeechLineWithAnimationGroup(3, 0, 0x154, 0x5f, 0x20, 0, 0x3f, 3);
	beginPrimarySpeechLineWithAnimationGroup(3, 1, 0x1c2, 0x73, 0x3f, 0x32, 0x0c, 0);
	runChunk14ActionRange(0x15, 0x61);
	_vm->gameState().officeNotePickupState = 1;
	applySceneStateToHotspotsAndPatches(3);
	runChunk14ActionRange(0x61, 0x6b);
	beginPrimarySpeechLineWithAnimationGroup(3, 2, 0x16d, 0x69, 0x20, 0, 0x3f, 4);
	runChunk14ActionRange(0x6f, 0x7c);
	_chunk14ActionVisible = false;
	_chunk12OverlayVisible = false;
	_chunk12FrameIndex = 0;

	_chunk14AltChunkIndex = 15;
	runChunk11Range(0x0b, 0x12);
	beginPrimarySpeechLineWithAnimationGroup(3, 3, 0x1a9, 0x82, 0x3f, 0x32, 0x0c, 1);
	_chunk14AltVisible = true;
	runChunk14AltRange(15, 0, 0x14);
	beginPrimarySpeechLineWithAnimationGroup(3, 4, 0x136, 0x6e, 0x0a, 0x3f, 0, 5);
	runChunk11Range(0x16, 0x1a);
	beginPrimarySpeechLineWithAnimationGroup(3, 5, 0x1a9, 0x82, 0x3f, 0x32, 0x0c, 2);
	runChunk11Range(0x1e, 0x21);
	runChunk14AltRange(15, 0x18, 0x1c);
	beginPrimarySpeechLineWithAnimationGroup(3, 6, 0x14f, 0x73, 0x0a, 0x3f, 0, 6);
	runChunk14AltRange(15, 0x20, 0x25);
	beginPrimarySpeechLineWithAnimationGroup(3, 7, 0x1c2, 0x73, 0x3f, 0x32, 0x0c, 0);
	runChunk14AltRange(15, 0x18, 0x1c);
	beginPrimarySpeechLineWithAnimationGroup(3, 8, 0x14f, 0x73, 0x0a, 0x3f, 0, 6);
	runChunk14AltRange(15, 0x25, 0x3f);
	_chunk14AltVisible = false;
	_chunk14AltChunkIndex = previousAltChunkIndex;
	beginPrimarySpeechLineWithAnimationGroup(3, 9, 0x1c2, 0x73, 0x3f, 0x32, 0x0c, 0);

	_chunk11FrameIndex = 0;
	_chunk14ActionVisible = false;
	_chunk14AltVisible = false;
	_chunk12OverlayVisible = false;
	_preItemIdleState = previousPreItemIdleState;
	_hideActiveActor = previousHideActiveActor;
}

void SuePlayableScene::runChunk11Range(byte firstFrame, byte endFrame) {
	const byte previousPreItemIdleState = _preItemIdleState;
	_preItemIdleState = 3;
	for (uint frame = firstFrame; frame < endFrame && !Engine::shouldQuit(); ++frame) {
		_chunk11FrameIndex = (byte)MIN<uint>(frame + 1, ARRAYSIZE(kG04Chunk11FrameMap) - 1);
		if (waitSceneMillis(kG04Chunk11FrameMillis))
			break;
	}
	_preItemIdleState = previousPreItemIdleState;
}

void SuePlayableScene::runChunk14ActionRange(byte firstFrame, byte endFrame) {
	_chunk14ActionVisible = true;
	for (uint frame = firstFrame; frame < endFrame && !Engine::shouldQuit(); ++frame) {
		applyChunk14ActionSideEffects((byte)frame);
		_chunk14ActionFrameIndex = (byte)MIN<uint>(frame + 1, ARRAYSIZE(kG04Chunk14ActionFrameMap) - 1);
		if (waitSceneMillis(kG04Chunk14FrameMillis))
			break;
	}
}

void SuePlayableScene::runChunk14AltRange(uint chunkIndex, byte firstFrame, byte endFrame) {
	_chunk14AltVisible = true;
	_chunk14AltChunkIndex = (byte)chunkIndex;
	for (uint frame = firstFrame; frame < endFrame && !Engine::shouldQuit(); ++frame) {
		applyChunk14AltSideEffects((byte)frame);
		_chunk14AltFrameIndex = (byte)MIN<uint>(frame + 1, ARRAYSIZE(kG04Chunk14AltFrameMap) - 1);
		if (waitSceneMillis(kG04Chunk14FrameMillis))
			break;
	}
}

void SuePlayableScene::applyChunk14ActionSideEffects(byte frameIndex) {
	switch (frameIndex) {
	case 0:
		_soundBank0.playSample(3, 100);
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 1;
		break;
	case 1:
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 2;
		break;
	case 8:
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 1;
		break;
	case 9:
		_soundBank0.playSample(4, 100);
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 0;
		break;
	case 0x0a:
		_chunk12OverlayVisible = false;
		break;
	case 0x22:
		_soundBank0.playSample(0x16, 50);
		break;
	case 0x6a:
		_soundBank0.stop();
		break;
	default:
		break;
	}
}

void SuePlayableScene::applyChunk14AltSideEffects(byte frameIndex) {
	switch (frameIndex) {
	case 0x2b:
		_soundBank0.playSample(0x17, 50);
		_vm->gameState().officeNotePickupState = 0;
		applySceneStateToHotspotsAndPatches(3);
		break;
	case 0x37:
		_soundBank0.playSample(3, 100);
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 1;
		break;
	case 0x38:
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 2;
		break;
	case 0x3d:
		_soundBank0.playSample(3, 100);
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 1;
		break;
	case 0x3e:
		_soundBank0.playSample(4, 100);
		_chunk12OverlayVisible = true;
		_chunk12FrameIndex = 0;
		break;
	case 0x3f:
		_chunk12OverlayVisible = false;
		break;
	default:
		break;
	}
}

byte SuePlayableScene::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (usesSingleSecondaryActorComposite()) {
		if (animationGroup == kG05PrimarySpeechAltGroup)
			return 0x24;
		return 1;
	}

	switch (animationGroup) {
	case 1:
		return 0x12;
	case 2:
		return 0x1a;
	case 3:
		return 0x11;
	case 4:
		return 0x6b;
	case 5:
		return 0x14;
	case 6:
		return 0x1c;
	default:
		return 7;
	}
}

void SuePlayableScene::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (usesSingleSecondaryActorComposite()) {
		(void)animationGroup;
		_cloakroomAttendantFrame = frameIndex;
		return;
	}

	switch (animationGroup) {
	case 3:
	case 4:
		_chunk14ActionVisible = true;
		_chunk14ActionFrameIndex = frameIndex;
		break;
	case 5:
	case 6:
		_chunk14AltVisible = true;
		_chunk14AltFrameIndex = frameIndex;
		break;
	default:
		_chunk11FrameIndex = frameIndex;
		break;
	}
}

bool SuePlayableScene::waitSceneMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		advanceGameplayLoop(slice);
		if (_actionOverlayVisible && !hasCustomComposite())
			drawActionOverlayComposite();
		else
			drawPlayableComposite();
		presentFrame();
		remaining -= slice;
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void SuePlayableScene::updateAmbientAudioAndMusicCues(uint32 delta) {
	_ambientMusicTimerAccumulator += delta;
	if (_ambientMusicTimerAccumulator < kG04AmbientMusicCheckMillis)
		return;
	_ambientMusicTimerAccumulator %= kG04AmbientMusicCheckMillis;

	if (!_ambientSoundBank0.isPlaying())
		_ambientSoundBank0.playSample(0x0b, 100);

	if (_vm->gameplayMusic()->isPlaying())
		return;

	GameplayState &state = _vm->gameState();
	if (state.currentAmbientMusicCueId != kG04AmbientMusicCueStillFrame) {
		_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
		state.currentAmbientMusicCueId = kG04AmbientMusicCueStillFrame;
		_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, 100);
		return;
	}

	byte nextTrack = 0;
	do {
		nextTrack = (byte)(0x0c + _random.getRandomNumber(2));
	} while (nextTrack == _previousAmbientMusicTrackId);

	_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
	state.currentAmbientMusicCueId = nextTrack;
	_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, 100);
}

void SuePlayableScene::advanceChunk11PreItemIdleAnimation(uint32 delta) {
	_chunk11TimerAccumulator += delta;
	while (_chunk11TimerAccumulator >= kG04Chunk11FrameMillis) {
		_chunk11TimerAccumulator -= kG04Chunk11FrameMillis;
		if (_preItemIdleState == 3)
			continue;

		if (_preItemIdleState == 0) {
			if (_random.getRandomNumber(0x31) == 0) {
				_preItemIdleState = 2;
				_chunk11FrameIndex = 0;
			} else if (_random.getRandomNumber(0x0e) == 0) {
				_preItemIdleState = 1;
				_chunk11FrameIndex = 1;
			}
		} else if (_preItemIdleState == 1) {
			_chunk11FrameIndex = 0;
			_preItemIdleState = 0;
		} else if (_preItemIdleState == 2) {
			if (_chunk11FrameIndex == 6) {
				_chunk11FrameIndex = 0;
				_preItemIdleState = 0;
			} else {
				++_chunk11FrameIndex;
			}
		}
	}
}

void SuePlayableScene::advanceChunk16PostItemAnimation(uint32 delta) {
	_chunk16TimerAccumulator += delta;
	while (_chunk16TimerAccumulator >= kG04Chunk16FrameMillis) {
		_chunk16TimerAccumulator -= kG04Chunk16FrameMillis;
		switch (_postItemIdleState) {
		case 0:
			++_chunk16FrameIndex;
			if (_chunk16FrameIndex >= 5)
				_postItemIdleState = 1;
			break;
		case 1:
			++_chunk16FrameIndex;
			if (_chunk16FrameIndex >= 0x1a)
				_postItemIdleState = 2;
			break;
		case 2:
			++_chunk16FrameIndex;
			if (_chunk16FrameIndex >= 0x1e)
				_postItemIdleState = 3;
			break;
		default:
			if (_random.getRandomNumber(0x0e) == 0)
				_chunk16FrameIndex = 0x22;
			else
				_chunk16FrameIndex = 0x1e;
			break;
		}
		if (_chunk16FrameIndex >= ARRAYSIZE(kG04Chunk16PostItemFrameMap))
			_chunk16FrameIndex = 1;
	}

	_chunk17TimerAccumulator += delta;
	while (_chunk17TimerAccumulator >= kG04Chunk17FrameMillis) {
		_chunk17TimerAccumulator -= kG04Chunk17FrameMillis;
		if (_postItemIdleState > 1 || _chunk17FrameIndex != 0)
			_chunk17FrameIndex = _chunk17FrameIndex == 8 ? 0 : (byte)(_chunk17FrameIndex + 1);
	}
}

void SuePlayableScene::advanceG05SecondaryActorAnimation(uint32 delta) {
	_cloakroomAttendantTimerAccumulator += delta;
	while (_cloakroomAttendantTimerAccumulator >= kG04Chunk11FrameMillis) {
		_cloakroomAttendantTimerAccumulator -= kG04Chunk11FrameMillis;

		switch (_cloakroomAttendantState) {
		case 0:
			if (_random.getRandomNumber(0x31) == 0) {
				_cloakroomAttendantFrame = 6;
				_cloakroomAttendantState = 2;
			} else if (_random.getRandomNumber(0x0e) == 0) {
				_cloakroomAttendantFrame = 5;
				_cloakroomAttendantState = 1;
			}
			break;
		case 1:
			_cloakroomAttendantFrame = 1;
			_cloakroomAttendantState = 0;
			break;
		case 2:
			if (_cloakroomAttendantFrame == 0x0e) {
				_cloakroomAttendantFrame = 0x0f;
				_cloakroomAttendantState = 3;
				_cloakroomAttendantRepeatCount = (byte)(_random.getRandomNumber(3) + 2);
			} else {
				++_cloakroomAttendantFrame;
			}
			break;
		case 3:
			if (_cloakroomAttendantFrame == 0x17) {
				if (_cloakroomAttendantRepeatCount == 0) {
					_cloakroomAttendantFrame = 0x18;
					_cloakroomAttendantState = 4;
				} else {
					_cloakroomAttendantFrame = 0x0f;
					--_cloakroomAttendantRepeatCount;
				}
			} else {
				++_cloakroomAttendantFrame;
			}
			break;
		case 4:
			if (_cloakroomAttendantFrame == 0x20) {
				_cloakroomAttendantFrame = 1;
				_cloakroomAttendantState = 0;
			} else {
				++_cloakroomAttendantFrame;
			}
			break;
		default:
			break;
		}
	}
}

void SuePlayableScene::advanceSecondaryActorSpeechAnimation(uint32 delta) {
	if (!_speechOverlay.visible) {
		_secondaryActorFrame = 0;
		_secondaryActorTimerAccumulator = 0;
		return;
	}

	_secondaryActorTimerAccumulator += delta;
	while (_secondaryActorTimerAccumulator >= kSecondaryActorSpeechFrameMillis) {
		_secondaryActorTimerAccumulator -= kSecondaryActorSpeechFrameMillis;
		advanceSecondaryActorSpeechFrame();
	}
}

void SuePlayableScene::advanceSecondaryActorSpeechFrame() {
	byte nextFrame = _secondaryActorFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _secondaryActorFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(kSecondaryActorFramesPerFacing - 1);

	if (nextFrame == _secondaryActorFrame)
		nextFrame = (byte)((_secondaryActorFrame + 1) % kSecondaryActorFramesPerFacing);

	_secondaryActorFrame = nextFrame;
}

void SuePlayableScene::advancePrimaryLeftSpeechFrame() {
	byte nextFrame = _primaryLeftSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _primaryLeftSpeechLastFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(3);

	if (nextFrame == _primaryLeftSpeechLastFrame)
		nextFrame = (byte)((_primaryLeftSpeechLastFrame + 1) % 4);

	_primaryLeftSpeechLastFrame = nextFrame;
	setPrimaryLeftSpeechFrame(nextFrame);
}

void SuePlayableScene::advancePrimaryDialogueSpeechFrame(uint32 delta) {
	_primaryDialogueSpeechTimerAccumulator += delta;
	while (_primaryDialogueSpeechTimerAccumulator >= kG04Chunk11FrameMillis) {
		_primaryDialogueSpeechTimerAccumulator -= kG04Chunk11FrameMillis;
		const byte baseFrame = primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup);
		byte nextFrame = _primaryDialogueSpeechLastFrame;
		for (uint attempt = 0; attempt < 8 && nextFrame == _primaryDialogueSpeechLastFrame; ++attempt)
			nextFrame = (byte)(baseFrame + _random.getRandomNumber(4));

		if (nextFrame == _primaryDialogueSpeechLastFrame)
			nextFrame = nextFrame >= baseFrame + 4 ? baseFrame : (byte)(nextFrame + 1);

		_primaryDialogueSpeechLastFrame = nextFrame;
		setPrimarySpeechAnimationFrame(_primaryDialogueSpeechGroup, nextFrame);
	}
}

void SuePlayableScene::clearSpeechOverlay() {
	_speechOverlay.visible = false;
	_speechOverlay.lines.clear();
}

void SuePlayableScene::clearAllSpeechOverlays() {
	clearSpeechOverlay();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
}

void SuePlayableScene::drawSpeechOverlay() {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return;

	drawSpeechOverlay(_speechOverlay);
	drawSpeechOverlay(_primarySpeechOverlay);
}

void SuePlayableScene::drawSpeechOverlay(const SpeechOverlay &overlay) {
	if (!overlay.visible)
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);

	Graphics::Surface screenSurface;
	screenSurface.init(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		HollywoodEngine::kScreenWidth, _screen.data(), Graphics::PixelFormat::createFormatCLUT8());

	for (uint lineIndex = 0; lineIndex < overlay.lines.size(); ++lineIndex) {
		const Common::String &line = overlay.lines[lineIndex];
		const int lineWidth = actorSpeechTextWidth(line);
		const int x = (int)overlay.centerX - (lineWidth >> 1) - viewportXOffset();
		const int y = (int)overlay.topY + lineIndex * kOriginalSpeechLineHeight;
		font->drawString(&screenSurface, line, x, y, lineWidth, overlay.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void SuePlayableScene::beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	runSpeechLine(_speechOverlay, rowIndex, frameIndex, _activeActorWorldX, 0,
		kG04SecondarySpeechTextColor, false, false, false);
}

bool SuePlayableScene::startSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return false;
	(void)continuationCount;

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return false;

	_speechOverlay.visible = true;
	_speechOverlay.colorIndex = kG04SecondarySpeechTextColor;
	wrapActorSpeechText(text, _activeActorWorldX, _speechOverlay.lines);
	calculateSecondarySpeechBounds(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	_secondaryActorTimerAccumulator = 0;

	return voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
}

void SuePlayableScene::beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(_speechOverlay, textRecordId, continuationCount, voiceSampleId, _activeActorWorldX, 0,
		kG04SecondarySpeechTextColor, false, false, false);
}

void SuePlayableScene::beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue) {
	if (!shouldAnimatePrimarySpeechLine()) {
		const uint paletteOffset = kG04PrimarySpeechTextColor * 3;
		if (_paletteCurrent.size() > paletteOffset + 2) {
			_paletteCurrent[paletteOffset] = red;
			_paletteCurrent[paletteOffset + 1] = green;
			_paletteCurrent[paletteOffset + 2] = blue;
		}

		runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, centerX, topY,
			kG04PrimarySpeechTextColor, true, false, false);
		return;
	}

	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, centerX, topY, red, green, blue, 0);
}

void SuePlayableScene::beginPrimarySpeechLineWithAnimationGroup(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, byte animationGroup) {
	const uint paletteOffset = kG04PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = red;
		_paletteCurrent[paletteOffset + 1] = green;
		_paletteCurrent[paletteOffset + 2] = blue;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, centerX, topY,
		kG04PrimarySpeechTextColor, true, false, true, animationGroup);
}

void SuePlayableScene::beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex) {
	const uint paletteOffset = kG04PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = 0x33;
		_paletteCurrent[paletteOffset + 1] = 0x22;
		_paletteCurrent[paletteOffset + 2] = 0x39;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, 0xfa, 0x136,
		kG04PrimarySpeechTextColor, true, true, false);
}

void SuePlayableScene::runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft, bool animatePrimaryDialogue,
		byte primaryAnimationGroup) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(overlay, textRecordId, continuationCount, voiceSampleId, centerX, topY, colorIndex,
		useRequestedTop, animatePrimaryLeft, animatePrimaryDialogue, primaryAnimationGroup);
}

void SuePlayableScene::runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
		uint16 voiceSampleId, uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop,
		bool animatePrimaryLeft, bool animatePrimaryDialogue, byte primaryAnimationGroup) {
	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit(); ++part) {
		const Common::String text = getResource003LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		overlay.visible = true;
		overlay.colorIndex = colorIndex;
		wrapActorSpeechText(text, centerX, overlay.lines);
		if (useRequestedTop) {
			overlay.centerX = centerX;
			overlay.topY = topY;
		} else {
			calculateSecondarySpeechBounds(centerX, _activeActorWorldY);
		}

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, overlay.lines.size() * 1100);
		_primaryLeftSpeechActive = animatePrimaryLeft;
		if (animatePrimaryLeft)
			setPrimaryLeftSpeechFrame(0);
		const byte animationGroup = animatePrimaryDialogue ?
			(primaryAnimationGroup == kInvalidPrimarySpeechAnimationGroup ? 0 : primaryAnimationGroup) :
			kInvalidPrimarySpeechAnimationGroup;
		_primaryDialogueSpeechActive = animationGroup != kInvalidPrimarySpeechAnimationGroup;
		if (_primaryDialogueSpeechActive) {
			_primaryDialogueSpeechGroup = animationGroup;
			_primaryDialogueSpeechLastFrame = primarySpeechAnimationBaseFrame(animationGroup);
			_primaryDialogueSpeechTimerAccumulator = 0;
			setPrimarySpeechAnimationFrame(animationGroup, _primaryDialogueSpeechLastFrame);
		}
		const bool interrupted = waitForSpeechOrDelay(duration, animatePrimaryLeft);
		if (animatePrimaryLeft) {
			_primaryLeftSpeechActive = false;
			_primaryLeftSpeechTimerAccumulator = 0;
			setPrimaryLeftSpeechFrame(0);
		}
		if (_primaryDialogueSpeechActive) {
			setPrimarySpeechAnimationFrame(_primaryDialogueSpeechGroup,
				primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup));
			_primaryDialogueSpeechActive = false;
			_primaryDialogueSpeechGroup = kInvalidPrimarySpeechAnimationGroup;
			_primaryDialogueSpeechTimerAccumulator = 0;
			_primaryDialogueSpeechLastFrame = 7;
		}
		_speech.stop();
		overlay.visible = false;
		overlay.lines.clear();
		if (interrupted)
			break;
	}
}

bool SuePlayableScene::getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 100) * 5;
	if (offset + 5 > _stage003StageBlock.size())
		return false;

	textRecordId = readUint16LE(_stage003StageBlock, offset);
	continuationCount = _stage003StageBlock[offset + 2];
	voiceSampleId = readUint16LE(_stage003StageBlock, offset + 3);
	return textRecordId != 0;
}

bool SuePlayableScene::getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > _sueSpeechCueDescriptors.size())
		return false;

	textRecordId = readUint16LE(_sueSpeechCueDescriptors, offset);
	continuationCount = _sueSpeechCueDescriptors[offset + 2];
	voiceSampleId = readUint16LE(_sueSpeechCueDescriptors, offset + 3);
	return textRecordId != 0;
}

void SuePlayableScene::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = 0x32;
	const int anchorX = anchorSceneX - viewportXOffset();
	if (anchorX < 0xa0)
		maxChars = (anchorX * 0x32) / 0xa0;
	else if (HollywoodEngine::kScreenWidth - anchorX < 0xa0)
		maxChars = ((HollywoodEngine::kScreenWidth - anchorX) * 0x32) / 0xa0;
	maxChars = MAX<uint>(maxChars, 0x18);

	const uint lineShrink = maxChars < 0x2a ? (maxChars > 0x20 ? 2 : 1) : 3;
	const char *source = text.c_str();
	const uint textLength = text.size();
	uint cursor = 0;
	while (cursor < textLength && lines.size() < 10) {
		uint end = textLength;
		if (cursor + maxChars < textLength) {
			end = cursor + maxChars;
			while (end > cursor && (byte)source[end] != 0x20 && source[end] != 0)
				--end;
			while (end > cursor && (byte)source[end - 1] == 0x20)
				--end;
			if (end == cursor)
				end = MIN<uint>(textLength, cursor + maxChars);
		}

		lines.push_back(Common::String(source + cursor, end - cursor));

		cursor = end;
		while (cursor < textLength && (byte)source[cursor] == 0x20)
			++cursor;

		maxChars = maxChars > lineShrink ? maxChars - lineShrink : 1;
	}
}

Common::String SuePlayableScene::getResource003LargeTextRecord(uint16 recordId) const {
	if (recordId < kStage003LargeRowBaseIndex) {
		const uint offset = (uint)recordId * kStage003LargeRowSize;
		if (recordId == 0 || offset >= _sueLargeRows.size())
			return Common::String();

		const byte *row = _sueLargeRows.data() + offset;
		uint length = 0;
		while (length < kStage003LargeRowSize && row[length] != 0)
			++length;

		return Common::String((const char *)row, length);
	}

	const uint localRecordId = recordId - kStage003LargeRowBaseIndex;
	const uint offset = localRecordId * kStage003LargeRowSize;
	if (offset >= _stage003LargeRows.size())
		return Common::String();

	const byte *row = _stage003LargeRows.data() + offset;
	uint length = 0;
	while (length < kStage003LargeRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

uint SuePlayableScene::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void SuePlayableScene::calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY) {
	uint textWidth = 0;
	for (uint i = 0; i < _speechOverlay.lines.size(); ++i)
		textWidth = MAX<uint>(textWidth, actorSpeechTextWidth(_speechOverlay.lines[i]));

	int centerX = actorWorldX;
	if (((centerX - (int)(textWidth >> 1)) - 1 + (int)textWidth) > 0x27e)
		centerX = (textWidth & 1) == 0 ? 0x27e - (textWidth >> 1) : 0x27d - (textWidth >> 1);
	if (centerX - (int)(textWidth >> 1) < 1)
		centerX = (textWidth >> 1) + 1;

	int topY = actorWorldY - (int)_speechOverlay.lines.size() * kOriginalSpeechLineHeight - 0xbe;
	if (topY < 1)
		topY = 1;

	_speechOverlay.centerX = (uint16)centerX;
	_speechOverlay.topY = (uint16)topY;
}

bool SuePlayableScene::waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft) {
	uint32 elapsed = 0;
	while (!Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= fallbackMillis)
			break;

		const uint32 slice = speechActive ? 50 : MIN<uint32>(50, fallbackMillis - elapsed);
		if (waitSceneMillis(slice))
			return true;
		if (animatePrimaryLeft && !_primarySpeechOverlay.visible)
			break;
		elapsed += slice;
	}

	return Engine::shouldQuit();
}

void SuePlayableScene::applyGameplayPanelPalette() {
	if (_paletteCurrent.size() <= kG04PanelTextColor * 3 + 2)
		return;

	const bool originalPaletteApplied = _panelArt.applyInteractiveObjectPalette(_paletteCurrent);
	if (!originalPaletteApplied) {
		const byte colors[] = {
			kG04PanelDarkColor, 0x05, 0x06, 0x08,
			kG04PanelFillColor, 0x0b, 0x0d, 0x11,
			kG04PanelSlotColor, 0x14, 0x16, 0x1a,
			kG04PanelLineColor, 0x24, 0x25, 0x28,
			kG04PanelSelectedColor, 0x2e, 0x1d, 0x0e,
			kG04PanelSelectedLineColor, 0x3a, 0x2d, 0x16
		};
		for (uint i = 0; i < ARRAYSIZE(colors); i += 4) {
			const uint paletteOffset = colors[i] * 3;
			if (paletteOffset + 2 < _paletteCurrent.size()) {
				_paletteCurrent[paletteOffset] = colors[i + 1];
				_paletteCurrent[paletteOffset + 1] = colors[i + 2];
				_paletteCurrent[paletteOffset + 2] = colors[i + 3];
			}
		}
	}

	const uint textOffset = kG04PanelTextColor * 3;
	_paletteCurrent[textOffset] = 0x32;
	_paletteCurrent[textOffset + 1] = _paletteCurrent[0x2d7];
	_paletteCurrent[textOffset + 2] = _paletteCurrent[0x2d8];
}

void SuePlayableScene::drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	if (panelState.inventoryPanelVisible)
		drawInventoryPanel(surface, panelState);
	else if (panelState.verbPanelVisible)
		drawVerbPanel(surface, panelState);
}

void SuePlayableScene::drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawVerbPanel(surface, _savedFramebuffer, viewportXOffset(), 0, panelState,
		_vm->font());
}

void SuePlayableScene::drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawDialogueInventoryPanel(surface, _savedFramebuffer, viewportXOffset(), 0,
		panelState, _vm->gameState(), _vm->font());
}

void SuePlayableScene::presentFrame(const SceneHoverCaption *hoverCaption, const GameplayPanelState *panelState,
		const DialogueMenuState *dialogueMenuState) {
	if (hoverCaption)
		hoverCaption->applyPalette(_paletteCurrent);
	if ((panelState && panelState->visible()) || (dialogueMenuState && dialogueMenuState->visible()))
		applyGameplayPanelPalette();
	uploadPalette6Bit(_paletteCurrent);

	const uint16 xOffset = viewportXOffset();
	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = xOffset + y * HollywoodEngine::kSceneBufferWidth;
		memcpy(_screen.data() + y * HollywoodEngine::kScreenWidth,
			_sceneFramebuffer.data() + sourceOffset,
			HollywoodEngine::kScreenWidth);
	}

	drawSpeechOverlay();
	Graphics::Surface screenSurface;
	screenSurface.init(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		HollywoodEngine::kScreenWidth, _screen.data(), Graphics::PixelFormat::createFormatCLUT8());
	if (dialogueMenuState && dialogueMenuState->visible())
		_panelArt.drawDialogueMenuPanel(screenSurface, *dialogueMenuState, _vm->font());
	else if (panelState && panelState->visible())
		drawGameplayPanel(screenSurface, *panelState);
	else if (_vm->font() && _vm->font()->isLoaded()) {
		if (hoverCaption)
			hoverCaption->draw(screenSurface, *_vm->font());
	}

	g_system->copyRectToScreen(_screen.data(), HollywoodEngine::kScreenWidth, 0, 0,
		HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight);
	g_system->updateScreen();
}

bool SuePlayableScene::pollEvents(bool allowSkip) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			return true;
		case Common::EVENT_MAINMENU:
			_vm->openMainMenuDialog();
			if (_vm->isSceneRestartRequested())
				return true;
			break;
		case Common::EVENT_KEYDOWN:
			if (allowSkip && (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_SPACE)) {
				_skipRequested = true;
				return true;
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			break;
		default:
			break;
		}
	}

	return false;
}

bool SuePlayableScene::delay(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;
	}

	return _skipRequested || Engine::shouldQuit();
}

} // End of namespace Hollywood
