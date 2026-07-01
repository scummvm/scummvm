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

#include "hollywood/scenes/playable/playable_scene.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/gameplay/travel_screen.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kResource000Name = "RESOURCE.000";
const char *const kStage003ArchiveName = "RESOURCE.003";
const char *const kDefaultGameplayMusicArchiveName = "RESOURCE.M07";
const char *const kDefaultGameplaySoundBank0ArchiveName = "RESOURCE.S07";
const byte kAmbientMusicCueStillFrame = 0x0f;
const byte kAmbientMusicCueFirstRandomTrack = 0x0c;
const byte kAmbientMusicCueRandomTrackCount = 3;
const byte kAmbientLoopingSoundCue = 0x0b;
const byte kDefaultSecondarySpeechTextColor = 0xfd;
const byte kDefaultPrimarySpeechTextColor = 0xfb;
const byte kPanelDarkColor = 0xe7;
const byte kPanelFillColor = 0xe8;
const byte kPanelSlotColor = 0xe9;
const byte kPanelLineColor = 0xea;
const byte kPanelSelectedColor = 0xf1;
const byte kPanelSelectedLineColor = 0xf2;
const byte kPanelTextColor = 0xfc;
const uint kResource000InventoryActionTablesEntry = 0xc8;
const uint kResource000FixedInventoryVerbTableOffset = 0xec54;
const uint32 kActorPathFrameMillis = 60;
const uint32 kPrimaryDialogueSpeechFrameMillis = 75;
const uint32 kAmbientMusicCheckMillis = 250;
const uint32 kSecondaryActorSpeechFrameMillis = 150;
const byte kInvalidFacing = 0xff;
const byte kInvalidCel = 0xff;
const byte kInvalidPrimarySpeechAnimationGroup = 0xff;
const uint32 kViewportScrollTickMillis = 10;
const uint16 kViewportScrollRightThreshold = 0x144;
const uint16 kViewportScrollLeftThreshold = 0x13c;
const uint16 kViewportScrollStep = 2;

PlayableScene::PlayableScene(HollywoodEngine *vm, const char *randomName, int defaultActorX, int defaultActorY,
		byte defaultActorFacing, byte secondarySpeechTextColor, byte primarySpeechTextColor) :
		PlayableScene(vm, PlayableSceneConfig(), randomName, defaultActorX, defaultActorY, defaultActorFacing,
			secondarySpeechTextColor, primarySpeechTextColor) {
}

PlayableScene::PlayableScene(HollywoodEngine *vm, const PlayableSceneConfig &config, const char *randomName,
		int defaultActorX, int defaultActorY,
		byte defaultActorFacing, byte secondarySpeechTextColor, byte primarySpeechTextColor) :
		_vm(vm),
		_config(config),
		_resources(),
		_sceneChunkTable(_resources.chunkTable),
		_resourceChunkOffsets(_resources.chunkOffsets),
		_resourceArenaCursor(_resources.arenaCursor),
		_resourceArena(_resources.arena),
		_metadata(_resources.metadata),
		_surfaceState(),
		_paletteResource(_surfaceState.paletteResource),
		_paletteCurrent(_surfaceState.paletteCurrent),
		_baseFramebufferOriginal(_surfaceState.baseFramebufferOriginal),
		_baseFramebuffer(_surfaceState.baseFramebuffer),
		_sceneFramebuffer(_surfaceState.sceneFramebuffer),
		_savedFramebuffer(_surfaceState.savedFramebuffer),
		_fillRuns(_surfaceState.fillRuns),
		_paletteMaskOriginal(_surfaceState.paletteMaskOriginal),
		_paletteMask(_surfaceState.paletteMask),
		_fullPaletteRegionMask(_surfaceState.fullPaletteRegionMask),
		_walkablePaletteMask(_surfaceState.walkablePaletteMask),
		_colorToActorDepthClassMap(_surfaceState.colorToActorDepthClassMap),
		_actorDepthYThresholds(_surfaceState.actorDepthYThresholds),
		_drawActorDepthYThresholds(_surfaceState.drawActorDepthYThresholds),
		_screen(_surfaceState.screen),
		_displayPalette(_surfaceState.displayPalette),
		_textStore(),
		_stage003DecodeKey(_textStore.decodeKey),
		_stage003StageBlock(_textStore.stageBlock),
		_stage003SmallRows(_textStore.stageSmallRows),
		_stage003LargeRows(_textStore.stageLargeRows),
		_staticSpeechCueDescriptors(_textStore.staticSpeechCueDescriptors),
		_inventoryOwnerSmallRows(_textStore.inventoryOwnerSmallRows),
		_inventoryOwnerLargeRows(_textStore.inventoryOwnerLargeRows),
		_pathController(),
		_routeBoundaryPoints(_pathController.routeBoundaryPoints),
		_routeSteps(_pathController.routeSteps),
		_actorPathFrames(_pathController.frames),
		_actorPathStepDeltas(_pathController.stepDeltas),
		_random(randomName),
		_speechController(),
		_speech(_speechController.player),
		_speechOverlay(_speechController.secondaryOverlay),
		_primarySpeechOverlay(_speechController.primaryOverlay),
		_primaryLeftSpeechLastFrame(_speechController.primaryLeftSpeechLastFrame),
		_primaryDialogueSpeechLastFrame(_speechController.primaryDialogueSpeechLastFrame),
		_primaryDialogueSpeechGroup(_speechController.primaryDialogueSpeechGroup),
		_primaryLeftSpeechActive(_speechController.primaryLeftSpeechActive),
		_primaryDialogueSpeechActive(_speechController.primaryDialogueSpeechActive),
		_secondaryActorTimerAccumulator(_speechController.secondaryActorTimerAccumulator),
		_primaryLeftSpeechTimerAccumulator(_speechController.primaryLeftSpeechTimerAccumulator),
		_primaryDialogueSpeechTimerAccumulator(_speechController.primaryDialogueSpeechTimerAccumulator),
		_secondaryActorFrame(_speechController.secondaryActorFrame),
		_actionOverlayPlayer(),
		_actionOverlayVisible(_actionOverlayPlayer.visible),
		_actionOverlayLayer(_actionOverlayPlayer.layer),
		_actionOverlayChunkIndex(_actionOverlayPlayer.chunkIndex),
		_actionOverlayDescriptorCount(_actionOverlayPlayer.descriptorCount),
		_actionOverlayFrameIndex(_actionOverlayPlayer.frameIndex),
		_hideActiveActor(_actionOverlayPlayer.hideActiveActor),
		_ambientMusicTimerAccumulator(0),
		_previousAmbientMusicTrackId(0),
		_currentAmbientSoundCueId(0),
		_previousAmbientSoundCueId(0),
		_viewportXOffset(0),
		_viewportMinXOffset(0),
		_viewportMaxXOffset(0),
		_viewportScrollTimerAccumulator(0),
		_actorPathPlaybackActive(false),
		_activeActorWorldX(defaultActorX),
		_activeActorWorldY(defaultActorY),
		_activeActorFacing(defaultActorFacing),
		_activeActorCel(0),
		_activeActorDrawOrderMode(0),
		_lastSceneActionItemId(0),
		_skipRequested(false) {
	_surfaceState.initialize(kPaletteSize, 0x700, kPaletteMaskUsedBytes, kScenePaletteMapPageSize, kScenePaletteRegionCount);
	_speechController.initialize(secondarySpeechTextColor, primarySpeechTextColor);
	_speechController.resetRuntimeState(kInvalidPrimarySpeechAnimationGroup, 7);
	_activeActorRunStreams.resize(kActorFacingCount * kActiveActorFacingRunStride);
	_secondaryActorRunStreams.resize(kActorFacingCount * kSecondaryActorFacingRunStride);
	_activeActorDescriptors.resize(kActorFacingCount * kActorCelsPerFacing);
	_secondaryActorDescriptors.resize(kActorFacingCount * kSecondaryActorFramesPerFacing);
	_pathController.initialize(kSceneRouteBoundaryPointCount, kSceneRouteStepCount,
		kScenePaletteRegionCount, kScenePaletteRegionBoundaryCandidateCount,
		kScenePaletteRegionRouteStepCount, defaultActorPathStepDeltaTable(),
		defaultActorPathStepDeltaTableSize());
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
}

PlayableSceneConfig::PlayableSceneConfig() :
		resourceArchiveName(nullptr),
		initialRequiredChunkCount(0),
		arenaFirstChunk(0),
		arenaLastChunk(0),
		stageIndex(0),
		debugName("Playable scene"),
		viewportXOffset(0),
		viewportMinXOffset(kSceneConfigUseViewportOffset),
		viewportMaxXOffset(kSceneConfigUseViewportOffset),
		inventoryOwnerIndex(1),
		activeAudioChapterIndex(kSceneConfigNoAudioChapter),
		actorBankTableEntry(0xd0),
		actorBankSegmentCount(14),
		actorPaletteTableEntry(0x108),
		inventoryActionTableExtraOffset(kResource000FixedInventoryVerbTableOffset),
		inventoryRowsOffsetIndex(0x32),
		speechCueDescriptorTableOffset(0x5f58),
		actorPathStepDeltaTable(defaultActorPathStepDeltaTable()),
		actorPathStepDeltaTableSize(defaultActorPathStepDeltaTableSize()),
		walkablePaletteMaxRegion(3),
		musicArchiveName(kDefaultGameplayMusicArchiveName),
		soundBank0ArchiveName(kDefaultGameplaySoundBank0ArchiveName),
		loadActorDepthTables(true),
		useActorDepthTest(false),
		mainFlowFirstState(kSceneConfigNoMainFlowRangeStart),
		mainFlowLastState(kSceneConfigNoMainFlowRangeEnd) {
}

void PlayableScene::initializeFramebuffers() {
	_surfaceState.initializeFramebuffers();
}

uint PlayableScene::framebufferByteCount() const {
	return _surfaceState.framebufferByteCount();
}

byte *PlayableScene::framebufferPixels(Graphics::ManagedSurface &surface) {
	return _surfaceState.framebufferPixels(surface);
}

const byte *PlayableScene::framebufferPixels(const Graphics::ManagedSurface &surface) const {
	return _surfaceState.framebufferPixels(surface);
}

void PlayableScene::copyBaseFramebufferToSceneFramebuffer() {
	_surfaceState.copyBaseFramebufferToSceneFramebuffer();
}

void PlayableScene::restoreBaseFramebufferFromOriginal() {
	_surfaceState.restoreBaseFramebufferFromOriginal();
}

bool PlayableScene::isFramebufferOffsetValid(uint offset) const {
	return _surfaceState.isFramebufferOffsetValid(offset);
}

byte PlayableScene::savedFramebufferPixelAt(uint offset) const {
	return _surfaceState.savedFramebufferPixelAt(offset);
}

bool PlayableScene::play() {
	if (!load())
		return false;

	const bool resumeGameplayPose = hasSavedActiveActorPoseForCurrentState();
	initializePreviewState();
	if (!resumeGameplayPose) {
		drawPreviewComposite();
		presentFrame();
		runEntryCutscene();
		if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
			return true;
	}

	_skipRequested = false;
	const bool result = runBasicGameplayLoop();
	if (!_vm->isSceneRestartRequested()) {
		if (_vm->gameState().activeActorPoseStateId != _vm->gameState().mainFlowStateId)
			_vm->gameState().activeActorPoseValid = false;
		if (shouldRunExitSideEffectsAfterLoop())
			runExitSideEffectsAfterLoop();
	}
	return result;
}

bool PlayableScene::hasSavedActiveActorPoseForCurrentState() const {
	const GameplayState &state = _vm->gameState();
	return state.activeActorPoseValid &&
		state.activeActorPoseStateId == state.mainFlowStateId &&
		isMainFlowStateInScene(state.mainFlowStateId);
}

void PlayableScene::restoreActiveActorPoseFromGameState() {
	if (!hasSavedActiveActorPoseForCurrentState())
		return;

	const GameplayState &state = _vm->gameState();
	_activeActorWorldX = state.activeActorWorldX;
	_activeActorWorldY = state.activeActorWorldY;
	_activeActorFacing = state.activeActorFacing;
	_activeActorCel = state.activeActorCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);

	_viewportXOffset = state.activeViewportXOffset;
	if (_viewportXOffset < _viewportMinXOffset)
		_viewportXOffset = _viewportMinXOffset;
	else if (_viewportXOffset > _viewportMaxXOffset)
		_viewportXOffset = _viewportMaxXOffset;
}

void PlayableScene::syncActiveActorPoseToGameState() {
	GameplayState &state = _vm->gameState();
	if (!isMainFlowStateInScene(state.mainFlowStateId)) {
		state.activeActorPoseValid = false;
		return;
	}

	int actorX = _activeActorWorldX;
	int actorY = _activeActorWorldY;
	if (actorX < 0)
		actorX = 0;
	else if (actorX >= HollywoodEngine::kSceneBufferWidth)
		actorX = HollywoodEngine::kSceneBufferWidth - 1;
	if (actorY < 0)
		actorY = 0;
	else if (actorY >= HollywoodEngine::kSceneBufferHeight)
		actorY = HollywoodEngine::kSceneBufferHeight - 1;

	state.activeActorPoseValid = true;
	state.activeActorPoseStateId = state.mainFlowStateId;
	state.activeActorWorldX = (uint16)actorX;
	state.activeActorWorldY = (uint16)actorY;
	state.activeActorFacing = _activeActorFacing;
	state.activeActorCel = _activeActorCel;
	state.activeViewportXOffset = _viewportXOffset;
}

const char *PlayableScene::resourceArchiveName() const {
	return _config.resourceArchiveName;
}

uint PlayableScene::sceneInitialRequiredChunkCount() const {
	return _config.initialRequiredChunkCount;
}

uint PlayableScene::sceneArenaFirstChunk() const {
	return _config.arenaFirstChunk;
}

uint PlayableScene::sceneArenaLastChunk() const {
	return _config.arenaLastChunk;
}

uint PlayableScene::sceneStageIndex() const {
	return _config.stageIndex;
}

const char *PlayableScene::sceneDebugName() const {
	return _config.debugName;
}

uint16 PlayableScene::sceneViewportXOffset() const {
	return _config.viewportXOffset;
}

uint16 PlayableScene::sceneViewportMinXOffset() const {
	if (_config.viewportMinXOffset == kSceneConfigUseViewportOffset)
		return sceneViewportXOffset();
	return _config.viewportMinXOffset;
}

uint16 PlayableScene::sceneViewportMaxXOffset() const {
	if (_config.viewportMaxXOffset == kSceneConfigUseViewportOffset)
		return sceneViewportXOffset();
	return _config.viewportMaxXOffset;
}

byte PlayableScene::inventoryOwnerIndex() const {
	return _config.inventoryOwnerIndex;
}

void PlayableScene::initializeInventoryOwnerState() {
	GameplayState &state = _vm->gameState();
	if (_config.inventoryOwnerIndex == 0) {
		state.initializeRonItemResourcePages();
		if (state.inventoryItemCountByOwner[0] == 0)
			state.initializeRonInventoryItems();
	} else {
		state.initializeSueItemResourcePages();
	}
	if (_config.activeAudioChapterIndex != kSceneConfigNoAudioChapter)
		state.activeAudioChapterIndex = _config.activeAudioChapterIndex;
}

uint PlayableScene::resource000ActorBankTableEntry() const {
	return _config.actorBankTableEntry;
}

uint PlayableScene::resource000ActorBankSegmentCount() const {
	return _config.actorBankSegmentCount;
}

uint PlayableScene::resource000ActorPaletteTableEntry() const {
	return _config.actorPaletteTableEntry;
}

uint32 PlayableScene::inventoryActionTableExtraOffset() const {
	return _config.inventoryActionTableExtraOffset;
}

uint PlayableScene::resource003InventoryRowsOffsetIndex() const {
	return _config.inventoryRowsOffsetIndex;
}

uint32 PlayableScene::speechCueDescriptorTableOffset() const {
	return _config.speechCueDescriptorTableOffset;
}

const byte *PlayableScene::actorPathStepDeltaTable() const {
	return _config.actorPathStepDeltaTable;
}

uint PlayableScene::actorPathStepDeltaTableSize() const {
	return _config.actorPathStepDeltaTableSize;
}

byte PlayableScene::walkablePaletteMaxRegion() const {
	return _config.walkablePaletteMaxRegion;
}

const char *PlayableScene::musicArchiveName() const {
	return _config.musicArchiveName;
}

const char *PlayableScene::soundBank0ArchiveName() const {
	return _config.soundBank0ArchiveName;
}

int PlayableScene::alternatePaletteResourceChunkIndex() const {
	return -1;
}

bool PlayableScene::isAlternatePaletteResourceActive() const {
	return false;
}

bool PlayableScene::shouldLoadInventoryActionTables() const {
	return true;
}

bool PlayableScene::shouldLoadActorDepthTables() const {
	return _config.loadActorDepthTables;
}

bool PlayableScene::shouldConvertSavedFramebufferFF() const {
	return false;
}

bool PlayableScene::shouldLoadArenaChunk(uint index) const {
	(void)index;
	return true;
}

bool PlayableScene::shouldRunExitSideEffectsAfterLoop() const {
	return false;
}

void PlayableScene::runExitSideEffectsAfterLoop() {
}

bool PlayableScene::usesActorDepthTest() const {
	return _config.useActorDepthTest;
}

bool PlayableScene::isMainFlowStateInScene(uint16 stateId) const {
	if (_config.mainFlowFirstState > _config.mainFlowLastState)
		return false;
	return stateId >= _config.mainFlowFirstState && stateId <= _config.mainFlowLastState;
}

bool PlayableScene::hasCustomPreviewState() const {
	return false;
}

void PlayableScene::initializeCustomPreviewState() {
}

bool PlayableScene::hasCustomComposite() const {
	return false;
}

void PlayableScene::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
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

bool PlayableScene::shouldDrawSecondaryActorInPlayableComposite() const {
	return _speechOverlay.visible && !_actionOverlayVisible;
}

bool PlayableScene::hasCustomEntrySequence() const {
	return false;
}

void PlayableScene::runCustomEntrySequence() {
}

bool PlayableScene::prepareCustomGameplayLoop() {
	return false;
}

bool PlayableScene::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
	return false;
}

bool PlayableScene::dispatchCustomSceneAction(uint16 handlerId) {
	(void)handlerId;
	return false;
}

bool PlayableScene::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	(void)targetX;
	(void)targetY;
	return false;
}

bool PlayableScene::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)nextRegion;
	(void)state;
	(void)boundary;
	(void)requestedFacing;
	(void)restoredStepDeltas;
	return false;
}

bool PlayableScene::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
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

bool PlayableScene::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	return false;
}

AmbientAudioProfile PlayableScene::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(100);
}

void PlayableScene::handleActionOverlayFrameHook(byte hookId, uint frame) {
	(void)hookId;
	(void)frame;
}

AmbientAudioProfile PlayableScene::createLoopingAmbientAudioProfile(byte volumePercent) const {
	AmbientAudioProfile profile;
	profile.checkMillis = kAmbientMusicCheckMillis;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = kAmbientLoopingSoundCue;
	profile.soundVolumePercent = volumePercent;
	profile.musicMode = kAmbientMusicLoopRotation;
	profile.musicStillCueId = kAmbientMusicCueStillFrame;
	profile.musicFirstCueId = kAmbientMusicCueFirstRandomTrack;
	profile.musicCueCount = kAmbientMusicCueRandomTrackCount;
	profile.musicVolumePercent = volumePercent;
	return profile;
}

AmbientAudioProfile PlayableScene::createRandomAmbientAudioProfile(byte soundFirstCueId, byte soundCueCount,
		byte soundVolumePercent, byte soundProbabilityModulus, byte musicFirstCueId,
		byte musicCueCount, byte musicVolumePercent, byte musicProbabilityModulus) const {
	AmbientAudioProfile profile;
	profile.checkMillis = kAmbientMusicCheckMillis;
	profile.soundMode = kAmbientSoundRandomRange;
	profile.soundFirstCueId = soundFirstCueId;
	profile.soundCueCount = soundCueCount;
	profile.soundProbabilityModulus = soundProbabilityModulus;
	profile.soundVolumePercent = soundVolumePercent;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = musicFirstCueId;
	profile.musicCueCount = musicCueCount;
	profile.musicProbabilityModulus = musicProbabilityModulus;
	profile.musicVolumePercent = musicVolumePercent;
	return profile;
}

bool PlayableScene::shouldAnimatePrimarySpeechLine() const {
	return true;
}

void PlayableScene::setPrimaryLeftSpeechFrame(byte frameIndex) {
	(void)frameIndex;
}

bool PlayableScene::load() {
	initializeInventoryOwnerState();
	_vm->gameState().currentInventoryOwnerIndex = inventoryOwnerIndex();

	if (!loadResource000RuntimeTables(_resource000OffsetTable, _resource000SizeTable) ||
			!loadResource000ActorBank(_resource000OffsetTable, _resource000SizeTable))
		return false;
	if (shouldLoadInventoryActionTables() &&
			!loadResource000InventoryActionTables(_resource000OffsetTable))
		return false;
	if (!_panelArt.load())
		return false;

	const char *archiveName = resourceArchiveName();
	if (!archiveName || !archiveName[0]) {
		warning("%s has no resource archive configured", sceneDebugName());
		return false;
	}
	if (!_resources.loadChunkTable(_vm->resources(), archiveName)) {
		warning("Failed to read %s header", archiveName);
		return false;
	}

	if (!_resources.validateRequiredChunks(archiveName, sceneDebugName(), sceneInitialRequiredChunkCount()))
		return false;

	if (!loadFixedChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(1, _paletteResource, kPaletteSize) ||
			!loadVariableChunk(2, _fillRuns) ||
			!loadVariableChunk(3, _paletteMask) ||
			!loadVariableChunk(4, _metadata))
		return false;

	const int alternatePaletteChunkIndex = alternatePaletteResourceChunkIndex();
	if (alternatePaletteChunkIndex >= 0 && isAlternatePaletteResourceActive()) {
		if (!loadFixedChunk((uint)alternatePaletteChunkIndex, _paletteResource, kPaletteSize))
			return false;
	}

	_baseFramebufferOriginal.copyFrom(_baseFramebuffer);
	_paletteMaskOriginal = _paletteMask;

	if (_paletteMask.size() < kPaletteMaskUsedBytes) {
		warning("%s chunk 3 is shorter than the scene palette mask table", resourceArchiveName());
		return false;
	}
	if (shouldLoadActorDepthTables() && !initializeActorDepthTables())
		return false;
	if (!initializeScenePathTables())
		return false;

	uint32 arenaSize = 0;
	for (uint i = sceneArenaFirstChunk(); i <= sceneArenaLastChunk(); ++i) {
		if (!shouldLoadArenaChunk(i))
			continue;
		arenaSize += _sceneChunkTable.sizes[i];
	}

	_resources.allocateArena(arenaSize);

	for (uint i = sceneArenaFirstChunk(); i <= sceneArenaLastChunk(); ++i) {
		if (!shouldLoadArenaChunk(i))
			continue;
		if (!loadArenaChunk(i))
			return false;
	}

	memset(framebufferPixels(_savedFramebuffer), 0, framebufferByteCount());
	expandFillRunsToSavedFramebuffer();
	if (shouldConvertSavedFramebufferFF()) {
		byte *savedPixels = framebufferPixels(_savedFramebuffer);
		for (uint i = 0; i < framebufferByteCount(); ++i) {
			if (savedPixels[i] == 0xff)
				savedPixels[i] = 0xfa;
		}
	}
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (!loadResource000ActorPalette(_resource000OffsetTable) ||
			!loadStage003SceneRows())
		return false;
	_panelArt.applyInteractiveObjectPalette(_paletteCurrent);

	if (!_hotspots.load(_paletteMask, _metadata, _stage003SmallRows))
		return false;

	resetViewportFromScene();

	_vm->gameplayMusic()->setArchive(Common::Path(musicArchiveName()));
	_soundBank0.setArchive(Common::Path(soundBank0ArchiveName()));
	_ambientSoundBank0.setArchive(Common::Path(soundBank0ArchiveName()));

	debugC(1, kDebugScene, "%s loaded %s", sceneDebugName(), archiveName);
	return true;
}

bool PlayableScene::loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable) {
	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s for actor resources", kResource000Name);
		return false;
	}

	if ((uint32)file.size() < 1 + (2 * kResource000TableByteCount)) {
		warning("%s is too small for runtime tables", kResource000Name);
		return false;
	}

	file.seek(1);
	offsetTable.resize(kResource000TableByteCount);
	sizeTable.resize(kResource000TableByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s runtime tables", kResource000Name);
		return false;
	}

	return true;
}

bool PlayableScene::loadResource000ActorBank(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable) {
	const uint tableEntry = resource000ActorBankTableEntry();
	const uint segmentCount = resource000ActorBankSegmentCount();
	if (tableEntry + 4 > offsetTable.size() ||
			tableEntry + segmentCount * 4 > sizeTable.size()) {
		warning("%s actor bank table entries are out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s actor bank", kResource000Name);
		return false;
	}

	const uint32 actorBankOffset = readUint32LE(offsetTable, tableEntry);
	if (actorBankOffset > (uint32)file.size()) {
		warning("%s actor bank offset is out of range", kResource000Name);
		return false;
	}

	file.seek(actorBankOffset);
	memset(_activeActorRunStreams.data(), 0, _activeActorRunStreams.size());
	memset(_secondaryActorRunStreams.data(), 0, _secondaryActorRunStreams.size());

	for (uint segment = 0; segment < segmentCount; ++segment) {
		const uint32 segmentSize = readUint32LE(sizeTable, tableEntry + segment * 4);
		if (segment <= 5) {
			if (segmentSize > kActiveActorFacingRunStride) {
				warning("%s actor active run segment %u is too large", kResource000Name, segment);
				return false;
			}
			if (file.read(_activeActorRunStreams.data() + segment * kActiveActorFacingRunStride, segmentSize) != segmentSize) {
				warning("Failed to read %s actor active run segment %u", kResource000Name, segment);
				return false;
			}
		} else if (segment == 6) {
			if (segmentSize % kActiveActorDescriptorSize != 0) {
				warning("%s actor active descriptors have invalid size", kResource000Name);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("Failed to read %s actor active descriptors", kResource000Name);
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
				warning("%s actor secondary run segment %u is too large", kResource000Name, facing);
				return false;
			}
			if (file.read(_secondaryActorRunStreams.data() + facing * kSecondaryActorFacingRunStride, segmentSize) != segmentSize) {
				warning("Failed to read %s actor secondary run segment %u", kResource000Name, facing);
				return false;
			}
		} else {
			if (segmentSize % kSecondaryActorDescriptorSize != 0) {
				warning("%s actor secondary descriptors have invalid size", kResource000Name);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("Failed to read %s actor secondary descriptors", kResource000Name);
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

	resetActorPathStepDeltas();
	debugC(1, kDebugResources, "Loaded %s actor bank for %s", kResource000Name, sceneDebugName());
	return true;
}

bool PlayableScene::loadResource000ActorPalette(const Common::Array<byte> &offsetTable) {
	const uint tableEntry = resource000ActorPaletteTableEntry();
	if (tableEntry + 4 > offsetTable.size()) {
		warning("%s actor palette table entry is out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s actor palette", kResource000Name);
		return false;
	}

	const uint32 paletteOffset = readUint32LE(offsetTable, tableEntry);
	if (paletteOffset > (uint32)file.size() || kActorPaletteBytes > (uint32)file.size() - paletteOffset ||
			0x270 + kActorPaletteBytes > _paletteCurrent.size()) {
		warning("%s actor palette is out of range", kResource000Name);
		return false;
	}

	file.seek(paletteOffset);
	if (file.read(_paletteCurrent.data() + 0x270, kActorPaletteBytes) != kActorPaletteBytes) {
		warning("Failed to read %s actor palette", kResource000Name);
		return false;
	}

	return true;
}

bool PlayableScene::loadResource000InventoryActionTables(const Common::Array<byte> &offsetTable) {
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
	const uint32 fixedTableOffset = tableOffset + inventoryActionTableExtraOffset();
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

bool PlayableScene::loadStage003SceneRows() {
	return _textStore.load(kStage003ArchiveName, sceneDebugName(), sceneStageIndex(),
		resource003InventoryRowsOffsetIndex(), speechCueDescriptorTableOffset());
}

bool PlayableScene::loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_vm->resources(), resourceArchiveName(), sceneDebugName(),
		index, destination, fixedSize);
}

bool PlayableScene::loadFixedChunk(uint index, Graphics::ManagedSurface &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_vm->resources(), resourceArchiveName(), sceneDebugName(),
		index, destination, fixedSize);
}

bool PlayableScene::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	return _resources.loadVariableChunk(_vm->resources(), resourceArchiveName(), index, destination);
}

bool PlayableScene::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_vm->resources(), resourceArchiveName(), sceneDebugName(), index);
}

bool PlayableScene::initializeActorDepthTables() {
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

void PlayableScene::updateActorDepthThresholds(byte actorDrawOrderMode) {
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	if (_drawActorDepthYThresholds.size() > 2)
		_drawActorDepthYThresholds[2] = actorDrawOrderMode == 6 ? 0x3e7 : 0x158;
}

void PlayableScene::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	byte *savedPixels = framebufferPixels(_savedFramebuffer);
	const uint savedSize = framebufferByteCount();
	while (destinationOffset < savedSize && sourceOffset + 3 <= _fillRuns.size()) {
		const byte fill = _fillRuns[sourceOffset];
		const uint16 runLength = readUint16LE(_fillRuns, sourceOffset + 1);
		sourceOffset += 3;

		const uint count = MIN<uint>(runLength, savedSize - destinationOffset);
		if (count != 0) {
			memset(savedPixels + destinationOffset, fill, count);
			destinationOffset += count;
		}
	}
}

bool PlayableScene::initializeScenePathTables() {
	if (!_pathController.loadRouteTables(_metadata, kRouteBoundaryPoints, kRouteBoundarySteps,
			resourceArchiveName()))
		return false;

	memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _paletteMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}

	return true;
}

void PlayableScene::initializePreviewState() {
	if (hasCustomPreviewState()) {
		initializeCustomPreviewState();
		return;
	}

	initializeDefaultPreviewState();
}

void PlayableScene::initializeDefaultPreviewState() {
	_speechController.resetRuntimeState(kInvalidPrimarySpeechAnimationGroup, 7);
	_actionOverlayPlayer.reset();
	resetAmbientAudioState();
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_lastSceneActionItemId = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void PlayableScene::drawPreviewComposite() {
	drawCutsceneComposite(false, 0, 0, 0, 0, false, 0, 0, 0, 0);
}

void PlayableScene::drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	if (hasCustomComposite()) {
		drawCustomComposite(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY,
			actorDrawOrderMode);
		return;
	}

	(void)actorDrawOrderMode;
	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawActionOverlayLayer();
}

void PlayableScene::drawPlayableComposite() {
	const bool drawActiveActor = !_hideActiveActor;
	const bool drawSecondaryActor = shouldDrawSecondaryActorInPlayableComposite();
	drawCutsceneComposite(drawActiveActor, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
		drawSecondaryActor, _activeActorFacing, _secondaryActorFrame, _activeActorWorldX, _activeActorWorldY,
		_activeActorDrawOrderMode);
}

void PlayableScene::drawActiveAndSecondaryActorFrames(bool drawActiveActor, byte activeFacing, byte activeCel,
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

void PlayableScene::drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= HollywoodEngine::kResourceChunkCount || frameIndex >= frameMapSize)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		descriptorCount, frameMap[frameIndex], _sceneFramebuffer);
}

void PlayableScene::restoreResourceSpriteLayerBackground(const ResourceSpriteLayer &layer, const Graphics::Surface &background) {
	if (!layer.visible || layer.chunkIndex >= HollywoodEngine::kResourceChunkCount)
		return;

	if (layer.hasPreviousDescriptor) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[layer.chunkIndex], 0,
			layer.descriptorCount, layer.previousDescriptorIndex, background, *_sceneFramebuffer.surfacePtr());
	}
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[layer.chunkIndex], 0,
		layer.descriptorCount, layer.descriptorIndex(), background, *_sceneFramebuffer.surfacePtr());
}

void PlayableScene::drawResourceSpriteLayer(const ResourceSpriteLayer &layer) {
	if (!layer.visible || layer.chunkIndex >= HollywoodEngine::kResourceChunkCount)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[layer.chunkIndex], 0,
		layer.descriptorCount, layer.descriptorIndex(), _sceneFramebuffer);
}

void PlayableScene::drawActionOverlayLayer() {
	if (_actionOverlayLayer.visible) {
		drawResourceSpriteLayer(_actionOverlayLayer);
		return;
	}

	if (_actionOverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
			_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
	}
}

void PlayableScene::drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive) {
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

int PlayableScene::drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY) {
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

int PlayableScene::drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldY) {
	if (usesActorDepthTest()) {
		ActorDepthTest depthTest;
		depthTest.enabled = true;
		depthTest.savedFramebuffer = &_savedFramebuffer.rawSurface();
		depthTest.colorToDepthClassMap = &_colorToActorDepthClassMap;
		depthTest.depthYThresholds = &_drawActorDepthYThresholds;
		depthTest.actorWorldY = actorWorldY;

		return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
			minimumYExclusive, *_sceneFramebuffer.surfacePtr(), &depthTest);
	}

	return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
		minimumYExclusive, *_sceneFramebuffer.surfacePtr(), nullptr);
}

void PlayableScene::runEntryCutscene() {
	if (hasCustomEntrySequence()) {
		runCustomEntrySequence();
		return;
	}

	drawPlayableComposite();
	presentFrame();
}

void PlayableScene::runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);

	drawPlayableComposite();
	presentFrame();

	queueActorPathWithPaletteRegionRouting(startX, startY, targetX, targetY, kInvalidFacing, 0);
	_actorPathPlaybackActive = true;
	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		if (waitSceneMillis(kActorPathFrameMillis)) {
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
}

bool PlayableScene::runBasicGameplayLoop() {
	GameplayLoop loop(_vm, this);
	return loop.run();
}

const SceneHotspotTable &PlayableScene::hotspots() const {
	return _hotspots;
}

const Graphics::Surface &PlayableScene::savedFramebuffer() const {
	return _savedFramebuffer.rawSurface();
}

uint16 PlayableScene::viewportXOffset() const {
	return _viewportXOffset;
}

uint16 PlayableScene::viewportYOffset() const {
	return 0;
}

void PlayableScene::prepareGameplayLoop() {
	_skipRequested = false;
	_actorPathPlaybackActive = false;
	_speechController.resetRuntimeState(kInvalidPrimarySpeechAnimationGroup, 7);
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_actionOverlayPlayer.reset();
	resetAmbientAudioState();
	prepareCustomGameplayLoop();
	restoreActiveActorPoseFromGameState();
	syncActiveActorPoseToGameState();
}

void PlayableScene::advanceGameplayLoop(uint32 delta) {
	advanceSecondaryActorSpeechAnimation(delta);

	if (advanceCustomGameplayLoop(delta)) {
		advanceViewportScroll(delta);
		syncActiveActorPoseToGameState();
		return;
	}

	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);

	updateAmbientAudioAndMusicCues(delta);
	advanceViewportScroll(delta);
	syncActiveActorPoseToGameState();
}

void PlayableScene::drawGameplayFrame() {
	drawPlayableComposite();
}

void PlayableScene::presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) {
	syncActiveActorPoseToGameState();
	presentFrame(&hoverCaption, &panelState);
}

void PlayableScene::invalidatePresentationPalette() {
	_displayPalette.markAllDirty();
}

void PlayableScene::prepareOptionsMenuPalette(Common::Array<byte> &palette) const {
	palette = _paletteCurrent;
	_panelArt.applyInteractiveObjectPalette(palette);
}

bool PlayableScene::shouldExitGameplayLoop() const {
	if (_vm->isSceneRestartRequested())
		return true;

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !isMainFlowStateInScene(stateId);
}

Common::String PlayableScene::inventoryItemName(byte owner, byte itemId) const {
	if (owner != inventoryOwnerIndex())
		return Common::String();

	return _textStore.inventoryItemName(itemId);
}

void PlayableScene::beginSharedInventorySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginStaticSecondarySpeechLine(rowIndex, frameIndex);
}

byte PlayableScene::randomSharedInventorySpeechFrame(byte maxFrameIndex) {
	return (byte)_random.getRandomNumber(maxFrameIndex);
}

void PlayableScene::playSharedInventorySound(byte sampleId) {
	_soundBank0.playSample(sampleId, 100);
}

void PlayableScene::showTravelScreenViewer() {
	TravelScreen travelScreen(_vm);
	if (!travelScreen.showViewer() || Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_displayPalette.markAllDirty();
	presentFrame();
}

bool PlayableScene::walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel, bool cancelOnSkip) {
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY, targetX, targetY,
		finalFacing, finalCel);

	if (_actorPathFrames.size() <= 1) {
		drawPlayableComposite();
		presentFrame();
		return true;
	}

	_actorPathPlaybackActive = true;
	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		if (waitSceneMillis(kActorPathFrameMillis)) {
			_actorPathPlaybackActive = false;
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
			_actorPathPlaybackActive = false;
			drawPlayableComposite();
			presentFrame();
			return true;
		}
	}

	_actorPathPlaybackActive = false;
	drawPlayableComposite();
	presentFrame();
	return !Engine::shouldQuit() && !_vm->isSceneRestartRequested();
}

void PlayableScene::adjustWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (adjustCustomWalkTargetToFloorMask(targetX, targetY))
		return;

	if (targetX > 0x30f)
		targetX = 0x30f;

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return;
		--targetY;
	}
}

void PlayableScene::queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
		byte finalFacing, byte finalCel) {
	_pathController.queueWithPaletteRegionRouting(*this, startX, startY, targetX, targetY,
		_activeActorDrawOrderMode, _activeActorFacing, _activeActorCel, finalFacing, finalCel,
		kInvalidFacing, kInvalidCel, actorPathStepDeltaTable(), actorPathStepDeltaTableSize());
}

void PlayableScene::buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
		byte finalFacing, byte finalCel, int requestedFacing) {
	_pathController.buildFramesBetweenPoints(state, targetX, targetY, finalFacing, finalCel,
		requestedFacing, kInvalidFacing, kInvalidCel);
}

void PlayableScene::appendActorPathFrame(const ActorPathBuildState &state) {
	_pathController.appendFrame(state);
}

ScenePoint PlayableScene::nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const {
	return _pathController.nearestPaletteRouteBoundaryPoint(startX, startY, currentRegion, nextRegion);
}

ScenePoint PlayableScene::bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
		byte currentRegion, byte targetRegion) const {
	return _pathController.bestPaletteRouteBoundaryPoint(startX, startY, targetX, targetY,
		currentRegion, targetRegion);
}

byte PlayableScene::paletteRegionAt(int x, int y) const {
	if (x < 0 || y < 0 || x >= HollywoodEngine::kSceneBufferWidth || y >= HollywoodEngine::kSceneBufferHeight ||
			_fullPaletteRegionMask.empty())
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (!isFramebufferOffsetValid(offset))
		return 0;

	return _fullPaletteRegionMask[savedFramebufferPixelAt(offset)];
}

byte PlayableScene::calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const {
	return _pathController.calculateMovementFacingForPath(fromX, fromY, toX, toY, requestedFacing);
}

uint PlayableScene::calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const {
	return _pathController.calculateWalkStepCountForAxisDelta(startAxis, targetAxis, facing, cel);
}

byte PlayableScene::nextActorPathCel(byte cel) const {
	return _pathController.nextCel(cel);
}

uint PlayableScene::actorPathStepDelta(byte facing, byte cel) const {
	return _pathController.stepDelta(facing, cel);
}

void PlayableScene::resetActorPathStepDeltas() {
	_pathController.resetStepDeltas(actorPathStepDeltaTable(), actorPathStepDeltaTableSize());
}

byte PlayableScene::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
	return _pathController.calculateFacingTowardPoint(fromX, fromY, toX, toY);
}

void PlayableScene::applySceneStateToHotspotsAndPatches(byte selector) {
	if (applyCustomSceneStateToHotspotsAndPatches(selector))
		return;

	if (selector == 0 || selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	if (selector == 3 || selector == 0xff) {
		restoreBaseFramebufferFromOriginal();
	}
}

void PlayableScene::rebuildWalkablePaletteMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

bool PlayableScene::hasInventoryItem(byte itemId) const {
	const byte owner = _vm->gameState().currentInventoryOwnerIndex;
	return _vm->gameState().hasInventoryItem(owner, itemId);
}

void PlayableScene::addInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.addInventoryItem(state.currentInventoryOwnerIndex, itemId);
}

void PlayableScene::removeInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.removeInventoryItem(state.currentInventoryOwnerIndex, itemId);
}

Common::String PlayableScene::dialogueMenuText(byte stageId, byte textRowId) const {
	return _textStore.dialogueMenuText(stageId, textRowId);
}

void PlayableScene::advanceDialogueMenu(uint32 delta) {
	advanceGameplayLoop(delta);
}

void PlayableScene::drawDialogueMenuFrame() {
	drawPlayableComposite();
}

void PlayableScene::presentDialogueMenuFrame(const DialogueMenuState &state) {
	presentFrame(nullptr, nullptr, &state);
}

void PlayableScene::runActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis) {
	ActionOverlayOptions options;
	runActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis, options);
}

void PlayableScene::runActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, const ActionOverlayOptions &options) {
	const bool previousHideActiveActor = _actionOverlayPlayer.applyActorVisibility(options.actorVisibility);
	_actionOverlayPlayer.begin(chunkIndex, descriptorCount, frameMap, frameMapSize);

	const uint firstFrame = MIN<uint>(options.firstFrame, frameMapSize);
	const uint requestedEndFrame = options.endFrame == 0 ? frameMapSize : options.endFrame;
	const uint cappedEndFrame = MIN<uint>(requestedEndFrame, frameMapSize);
	for (uint frame = firstFrame; frame < cappedEndFrame && !Engine::shouldQuit(); ++frame) {
		_actionOverlayPlayer.setFrame(frame);
		if (options.statePatchFrame >= 0 && (int)frame == options.statePatchFrame)
			applySceneStateToHotspotsAndPatches(options.statePatchSelector);
		if (options.soundFrame >= 0 && (int)frame == options.soundFrame)
			_soundBank0.playSample(options.soundId, options.soundVolumePercent);
		if (options.hookId != 0 && (options.hookFrame < 0 || (int)frame == options.hookFrame))
			handleActionOverlayFrameHook(options.hookId, frame);
		if (waitSceneMillis(frameMillis))
			break;
	}
	_actionOverlayPlayer.finish(previousHideActiveActor);

	if (options.redrawAtEnd) {
		drawPlayableComposite();
		presentFrame();
	}
}

void PlayableScene::runHiddenActorActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis) {
	runConfiguredActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis,
		kActionOverlayHideActiveActor);
}

void PlayableScene::runVisibleActorActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis) {
	runConfiguredActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis,
		kActionOverlayShowActiveActor);
}

void PlayableScene::runConfiguredActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, ActionOverlayActorVisibility actorVisibility,
		int statePatchFrame, byte statePatchSelector, int soundFrame, byte soundId,
		byte soundVolumePercent, int hookFrame, byte hookId, bool redrawAtEnd, uint firstFrame, uint endFrame) {
	ActionOverlayOptions options;
	options.actorVisibility = actorVisibility;
	options.statePatchFrame = statePatchFrame;
	options.statePatchSelector = statePatchSelector;
	options.soundFrame = soundFrame;
	options.soundId = soundId;
	options.soundVolumePercent = soundVolumePercent;
	options.hookFrame = hookFrame;
	options.hookId = hookId;
	options.redrawAtEnd = redrawAtEnd;
	options.firstFrame = firstFrame;
	options.endFrame = endFrame;
	runActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis, options);
}

byte PlayableScene::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0;
}

void PlayableScene::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	(void)frameIndex;
}

void PlayableScene::primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	(void)baseFrame;
}

void PlayableScene::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	(void)baseFrame;
}

bool PlayableScene::waitSceneMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		advanceGameplayLoop(slice);
		drawPlayableComposite();
		presentFrame();
		remaining -= slice;
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void PlayableScene::resetViewportFromScene() {
	const uint16 maximumFramebufferOffset =
		HollywoodEngine::kSceneBufferWidth - HollywoodEngine::kScreenWidth;

	_viewportMinXOffset = MIN<uint16>(sceneViewportMinXOffset(), maximumFramebufferOffset);
	_viewportMaxXOffset = MIN<uint16>(sceneViewportMaxXOffset(), maximumFramebufferOffset);
	if (_viewportMaxXOffset < _viewportMinXOffset)
		_viewportMaxXOffset = _viewportMinXOffset;

	_viewportXOffset = CLIP<uint16>(sceneViewportXOffset(), _viewportMinXOffset, _viewportMaxXOffset);
	_viewportScrollTimerAccumulator = 0;
}

void PlayableScene::advanceViewportScroll(uint32 delta) {
	if (_actorPathPlaybackActive) {
		_viewportScrollTimerAccumulator = 0;
		return;
	}

	if (_viewportMinXOffset >= _viewportMaxXOffset) {
		_viewportScrollTimerAccumulator = 0;
		return;
	}

	_viewportScrollTimerAccumulator += delta;
	while (_viewportScrollTimerAccumulator >= kViewportScrollTickMillis) {
		_viewportScrollTimerAccumulator -= kViewportScrollTickMillis;
		const int actorScreenX = _activeActorWorldX - _viewportXOffset;

		if (actorScreenX > kViewportScrollRightThreshold && _viewportXOffset < _viewportMaxXOffset) {
			_viewportXOffset = MIN<uint16>(_viewportXOffset + kViewportScrollStep, _viewportMaxXOffset);
			continue;
		}

		if (actorScreenX < kViewportScrollLeftThreshold && _viewportMinXOffset < _viewportXOffset) {
			if (_viewportXOffset > _viewportMinXOffset + kViewportScrollStep)
				_viewportXOffset -= kViewportScrollStep;
			else
				_viewportXOffset = _viewportMinXOffset;
			continue;
		}

		_viewportScrollTimerAccumulator = 0;
		break;
	}
}

void PlayableScene::updateAmbientAudioAndMusicCues(uint32 delta) {
	const AmbientAudioProfile profile = ambientAudioProfile();
	if (profile.checkMillis == 0)
		return;

	_ambientMusicTimerAccumulator += delta;
	if (_ambientMusicTimerAccumulator < profile.checkMillis)
		return;
	_ambientMusicTimerAccumulator %= profile.checkMillis;

	updateAmbientSoundCue(profile);
	updateAmbientMusicCue(profile);
}

void PlayableScene::resetAmbientAudioState() {
	_ambientMusicTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	_currentAmbientSoundCueId = 0;
	_previousAmbientSoundCueId = 0;
}

void PlayableScene::updateAmbientSoundCue(const AmbientAudioProfile &profile) {
	if (_ambientSoundBank0.isPlaying())
		return;

	if (profile.soundMode == kAmbientSoundLoop) {
		_ambientSoundBank0.playSampleLooping(profile.soundCueId, profile.soundVolumePercent);
		return;
	}

	if (profile.soundMode != kAmbientSoundRandomRange ||
			profile.soundCueCount == 0 || profile.soundProbabilityModulus == 0)
		return;

	if (_random.getRandomNumber(profile.soundProbabilityModulus - 1) != 0)
		return;

	_previousAmbientSoundCueId = _currentAmbientSoundCueId;
	do {
		_currentAmbientSoundCueId = (byte)(profile.soundFirstCueId +
			_random.getRandomNumber(profile.soundCueCount - 1));
	} while (profile.soundCueCount > 1 && _currentAmbientSoundCueId == _previousAmbientSoundCueId);
	_ambientSoundBank0.playSample(_currentAmbientSoundCueId, profile.soundVolumePercent);
}

void PlayableScene::updateAmbientMusicCue(const AmbientAudioProfile &profile) {
	if (_vm->gameplayMusic()->isPlaying())
		return;

	GameplayState &state = _vm->gameState();
	if (profile.musicMode == kAmbientMusicLoopRotation) {
		if (state.currentAmbientMusicCueId != profile.musicStillCueId) {
			_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
			state.currentAmbientMusicCueId = profile.musicStillCueId;
			_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, profile.musicVolumePercent);
			return;
		}

		if (profile.musicCueCount == 0)
			return;

		byte nextTrack = 0;
		do {
			nextTrack = (byte)(profile.musicFirstCueId + _random.getRandomNumber(profile.musicCueCount - 1));
		} while (profile.musicCueCount > 1 && nextTrack == _previousAmbientMusicTrackId);

		_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
		state.currentAmbientMusicCueId = nextTrack;
		_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, profile.musicVolumePercent);
		return;
	}

	if (profile.musicMode != kAmbientMusicRandomRange ||
			profile.musicCueCount == 0 || profile.musicProbabilityModulus == 0)
		return;

	if (_random.getRandomNumber(profile.musicProbabilityModulus - 1) != 0)
		return;

	byte nextTrack = 0;
	_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
	do {
		nextTrack = (byte)(profile.musicFirstCueId + _random.getRandomNumber(profile.musicCueCount - 1));
	} while (profile.musicCueCount > 1 && nextTrack == _previousAmbientMusicTrackId);
	_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
	state.currentAmbientMusicCueId = nextTrack;
	_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, profile.musicVolumePercent);
}

void PlayableScene::advanceSecondaryActorSpeechAnimation(uint32 delta) {
	_speechController.advanceSecondaryActorSpeechAnimation(delta, _random,
		kSecondaryActorSpeechFrameMillis, kSecondaryActorFramesPerFacing);
}

void PlayableScene::advanceSecondaryActorSpeechFrame() {
	_speechController.advanceSecondaryActorSpeechFrame(_random, kSecondaryActorFramesPerFacing);
}

void PlayableScene::advancePrimaryLeftSpeechFrame() {
	const byte nextFrame = _speechController.advancePrimaryLeftSpeechFrame(_random);
	setPrimaryLeftSpeechFrame(nextFrame);
}

void PlayableScene::advancePrimaryDialogueSpeechFrame(uint32 delta) {
	_primaryDialogueSpeechTimerAccumulator += delta;
	while (_primaryDialogueSpeechTimerAccumulator >= kPrimaryDialogueSpeechFrameMillis) {
		_primaryDialogueSpeechTimerAccumulator -= kPrimaryDialogueSpeechFrameMillis;
		const byte baseFrame = primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup);
		const byte nextFrame = _speechController.advancePrimaryDialogueSpeechFrame(_random, baseFrame);
		setPrimarySpeechAnimationFrame(_primaryDialogueSpeechGroup, nextFrame);
	}
}

void PlayableScene::clearSpeechOverlay() {
	_speechController.clearSecondaryOverlay();
}

void PlayableScene::clearAllSpeechOverlays() {
	_speechController.clearAllOverlays();
}

void PlayableScene::drawSpeechOverlay() {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return;

	drawSpeechOverlay(_speechOverlay);
	drawSpeechOverlay(_primarySpeechOverlay);
}

void PlayableScene::drawSpeechOverlay(const SpeechOverlay &overlay) {
	if (!overlay.visible)
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);

	Graphics::Surface *screenSurface = _screen.surfacePtr();

	for (uint lineIndex = 0; lineIndex < overlay.lines.size(); ++lineIndex) {
		const Common::String &line = overlay.lines[lineIndex];
		const int lineWidth = actorSpeechTextWidth(line);
		const int x = (int)overlay.centerX - (lineWidth >> 1) - viewportXOffset();
		const int y = (int)overlay.topY + lineIndex * kOriginalSpeechLineHeight;
		font->drawString(screenSurface, line, x, y, lineWidth, overlay.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void PlayableScene::beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	runSpeechLine(_speechOverlay, rowIndex, frameIndex, _activeActorWorldX, 0,
		kDefaultSecondarySpeechTextColor, false, false, false);
}

bool PlayableScene::startSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
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
	_speechOverlay.colorIndex = kDefaultSecondarySpeechTextColor;
	wrapActorSpeechText(text, _activeActorWorldX, _speechOverlay.lines);
	calculateSecondarySpeechBounds(_activeActorWorldX, _activeActorWorldY);
	_speechController.prepareSecondaryActorSpeech();

	return voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
}

void PlayableScene::beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(_speechOverlay, textRecordId, continuationCount, voiceSampleId, _activeActorWorldX, 0,
		kDefaultSecondarySpeechTextColor, false, false, false);
}

void PlayableScene::beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue) {
	if (!shouldAnimatePrimarySpeechLine()) {
		setPaletteEntry6Bit(kDefaultPrimarySpeechTextColor, red, green, blue);

		runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, centerX, topY,
			kDefaultPrimarySpeechTextColor, true, false, false);
		return;
	}

	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, centerX, topY, red, green, blue, 0);
}

void PlayableScene::beginPrimarySpeechLineWithAnimationGroup(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, byte animationGroup) {
	setPaletteEntry6Bit(kDefaultPrimarySpeechTextColor, red, green, blue);

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, centerX, topY,
		kDefaultPrimarySpeechTextColor, true, false, true, animationGroup);
}

void PlayableScene::beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex) {
	setPaletteEntry6Bit(kDefaultPrimarySpeechTextColor, 0x33, 0x22, 0x39);

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, 0xfa, 0x136,
		kDefaultPrimarySpeechTextColor, true, true, false);
}

void PlayableScene::runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
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

void PlayableScene::runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
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
		calculateSpeechOverlayBounds(overlay, centerX, topY, useRequestedTop, _activeActorWorldY);

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, overlay.lines.size() * 1100);
		if (animatePrimaryLeft) {
			_speechController.startPrimaryLeftSpeech();
			setPrimaryLeftSpeechFrame(0);
		}
		const byte animationGroup = animatePrimaryDialogue ?
			(primaryAnimationGroup == kInvalidPrimarySpeechAnimationGroup ? 0 : primaryAnimationGroup) :
			kInvalidPrimarySpeechAnimationGroup;
		if (animationGroup != kInvalidPrimarySpeechAnimationGroup) {
			const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
			_speechController.startPrimaryDialogueSpeech(animationGroup, baseFrame);
			primarySpeechAnimationStarted(animationGroup, baseFrame);
			setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
		}
		const bool interrupted = waitForSpeechOrDelay(duration, animatePrimaryLeft);
		if (animatePrimaryLeft) {
			_speechController.stopPrimaryLeftSpeech();
			setPrimaryLeftSpeechFrame(0);
		}
		bool restoredPrimarySpeechAnimation = false;
		byte restoredPrimarySpeechAnimationGroup = 0;
		byte restoredPrimarySpeechBaseFrame = 0;
		if (_primaryDialogueSpeechActive) {
			restoredPrimarySpeechAnimation = true;
			restoredPrimarySpeechAnimationGroup = _primaryDialogueSpeechGroup;
			restoredPrimarySpeechBaseFrame = primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup);
			setPrimarySpeechAnimationFrame(restoredPrimarySpeechAnimationGroup, restoredPrimarySpeechBaseFrame);
			_speechController.stopPrimaryDialogueSpeech(kInvalidPrimarySpeechAnimationGroup, 7);
		}
		_speech.stop();
		overlay.visible = false;
		overlay.lines.clear();
		if (restoredPrimarySpeechAnimation)
			primarySpeechAnimationRestored(restoredPrimarySpeechAnimationGroup, restoredPrimarySpeechBaseFrame);
		if (interrupted)
			break;
	}
}

bool PlayableScene::getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	return _textStore.getStageCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId);
}

bool PlayableScene::getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	return _textStore.getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId);
}

void PlayableScene::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = 0x32;
	const int viewportX = viewportXOffset();
	int clampedAnchorSceneX = anchorSceneX;
	if (viewportX < clampedAnchorSceneX) {
		if (clampedAnchorSceneX - viewportX > HollywoodEngine::kScreenWidth - 1)
			clampedAnchorSceneX = viewportX + 0x276;
	} else {
		clampedAnchorSceneX = viewportX + 10;
	}

	const int anchorX = clampedAnchorSceneX - viewportX;
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

Common::String PlayableScene::getResource003LargeTextRecord(uint16 recordId) const {
	return _textStore.largeTextRecord(recordId);
}

uint PlayableScene::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

uint PlayableScene::speechOverlayTextWidth(const SpeechOverlay &overlay) const {
	uint textWidth = 0;
	for (uint i = 0; i < overlay.lines.size(); ++i)
		textWidth = MAX<uint>(textWidth, actorSpeechTextWidth(overlay.lines[i]));

	return textWidth;
}

void PlayableScene::calculateSpeechOverlayBounds(SpeechOverlay &overlay, int centerX, int topY, bool useRequestedTop,
		int actorWorldY) {
	const uint textWidth = speechOverlayTextWidth(overlay);
	int adjustedCenterX = centerX - viewportXOffset();
	if (((adjustedCenterX - (int)(textWidth >> 1)) - 1 + (int)textWidth) > 0x27e)
		adjustedCenterX = (textWidth & 1) == 0 ? 0x27e - (textWidth >> 1) : 0x27d - (textWidth >> 1);
	if (adjustedCenterX - (int)(textWidth >> 1) < 1)
		adjustedCenterX = (textWidth >> 1) + 1;
	adjustedCenterX += viewportXOffset();

	int adjustedTopY = 0;
	if (useRequestedTop)
		adjustedTopY = topY - (int)overlay.lines.size() * kOriginalSpeechLineHeight;
	else
		adjustedTopY = actorWorldY - (int)overlay.lines.size() * kOriginalSpeechLineHeight - 0xbe;
	if (adjustedTopY < 1)
		adjustedTopY = 1;

	overlay.centerX = (uint16)adjustedCenterX;
	overlay.topY = (uint16)adjustedTopY;
}

void PlayableScene::calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY) {
	calculateSpeechOverlayBounds(_speechOverlay, actorWorldX, 0, false, actorWorldY);
}

bool PlayableScene::waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft) {
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

void PlayableScene::setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue) {
	_surfaceState.setPaletteEntry6Bit(colorIndex, red, green, blue);
}

byte PlayableScene::paletteEntryComponent6Bit(byte colorIndex, uint component) const {
	return _surfaceState.paletteEntryComponent6Bit(colorIndex, component);
}

void PlayableScene::applyGameplayPanelPalette() {
	if (_paletteCurrent.size() <= kPanelTextColor * 3 + 2)
		return;

	const bool originalPaletteApplied = _panelArt.applyInteractiveObjectPalette(_paletteCurrent);
	if (!originalPaletteApplied) {
		const byte colors[] = {
			kPanelDarkColor, 0x05, 0x06, 0x08,
			kPanelFillColor, 0x0b, 0x0d, 0x11,
			kPanelSlotColor, 0x14, 0x16, 0x1a,
			kPanelLineColor, 0x24, 0x25, 0x28,
			kPanelSelectedColor, 0x2e, 0x1d, 0x0e,
			kPanelSelectedLineColor, 0x3a, 0x2d, 0x16
		};
		for (uint i = 0; i < ARRAYSIZE(colors); i += 4)
			setPaletteEntry6Bit(colors[i], colors[i + 1], colors[i + 2], colors[i + 3]);
	}

	setPaletteEntry6Bit(kPanelTextColor, 0x32,
		paletteEntryComponent6Bit(kPanelSelectedLineColor, 1),
		paletteEntryComponent6Bit(kPanelSelectedLineColor, 2));
}

void PlayableScene::drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	if (panelState.inventoryPanelVisible)
		drawInventoryPanel(surface, panelState);
	else if (panelState.verbPanelVisible)
		drawVerbPanel(surface, panelState);
}

void PlayableScene::drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawVerbPanel(surface, _savedFramebuffer, viewportXOffset(), 0, panelState,
		_vm->font());
}

void PlayableScene::drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawDialogueInventoryPanel(surface, _savedFramebuffer, viewportXOffset(), 0,
		panelState, _vm->gameState(), _vm->font());
}

void PlayableScene::presentFrame(const SceneHoverCaption *hoverCaption, const GameplayPanelState *panelState,
		const DialogueMenuState *dialogueMenuState) {
	if (hoverCaption)
		hoverCaption->applyPalette(_paletteCurrent);
	if ((panelState && panelState->visible()) || (dialogueMenuState && dialogueMenuState->visible()))
		applyGameplayPanelPalette();
	const uint16 xOffset = viewportXOffset();
	_displayPalette.uploadFrom6Bit(_paletteCurrent);
	_screen.copyRectToSurface(_sceneFramebuffer.rawSurface(), 0, 0,
		Common::Rect(xOffset, 0, xOffset + HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight));

	drawSpeechOverlay();
	Graphics::Surface *screenSurface = _screen.surfacePtr();
	if (dialogueMenuState && dialogueMenuState->visible())
		_panelArt.drawDialogueMenuPanel(*screenSurface, *dialogueMenuState, _vm->font());
	else if (panelState && panelState->visible())
		drawGameplayPanel(*screenSurface, *panelState);
	else if (_vm->font() && _vm->font()->isLoaded()) {
		if (hoverCaption)
			hoverCaption->draw(*screenSurface, *_vm->font());
	}

	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);
	g_system->updateScreen();
}

bool PlayableScene::pollEvents(bool allowSkip) {
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
			_displayPalette.markAllDirty();
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

} // End of namespace Hollywood
