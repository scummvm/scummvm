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

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "hollywood/hollywood.h"
#include "hollywood/debug.h"
#include "hollywood/font.h"
#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/gameplay/inventory_media.h"
#include "hollywood/gameplay/travel_screen.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/playable_scene.h"
#include "hollywood/scenes/resource_delta_clip_player.h"
#include "hollywood/scenes/scene_registry.h"

namespace Hollywood {

const byte kMineAmbientSoundCueVolumes[] = {
	10, 10, 10, 2, 10, 10, 10, 100
};

const char *const kResource000Name = "RESOURCE.000";
const char *const kStage003ArchiveName = "RESOURCE.003";
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
const uint16 kViewportScrollRightThreshold = 0x144;
const uint16 kViewportScrollLeftThreshold = 0x13c;
const uint16 kViewportScrollStep = 2;
const uint32 kSueTapeFrameMillis = 100;
const uint32 kPaletteFadeStepMillis = 10;
const uint32 kBlockingSceneMaxFrameDeltaMillis = 250;

// Five base shades followed by the two alternate shade sets used by actor sprites.
const byte kActorPaletteAdjustmentSources[] = {
	0xd1, 0xd2, 0xd3, 0xd4, 0xd5
};

const byte kActorPaletteAdjustmentDestinations[][5] = {
	{ 0xe0, 0xe1, 0xe2, 0xe3, 0xd6 },
	{ 0xe4, 0xe5, 0xe6, 0xe7, 0xe8 }
};

const byte kActorPaletteAdjustmentCopyColor = 0xdc;
const uint kActorPaletteAdjustmentCopyColorCount = 4;

struct SueTapeSpeechCue {
	uint16 rowIndex;
	byte frameIndex;
	byte red;
	byte green;
	byte blue;
};

// The recording starts at row 0xe7; row 0x51 frames 0 and 9 are Ron's bookends.
const SueTapeSpeechCue kSueTapeSpeechCues[] = {
	{ 0xe7, 0, 0x20, 0x00, 0x3f },
	{ 0x51, 1, 0x3f, 0x28, 0x32 },
	{ 0x51, 2, 0x3f, 0x00, 0x00 },
	{ 0x51, 3, 0x3f, 0x28, 0x32 },
	{ 0x51, 4, 0x20, 0x00, 0x3f },
	{ 0x51, 5, 0x3f, 0x3f, 0x00 },
	{ 0x51, 6, 0x3f, 0x00, 0x00 },
	{ 0x51, 7, 0x3f, 0x3f, 0x00 },
	{ 0x51, 8, 0x3f, 0x00, 0x00 }
};

PlayableScene::PlayableScene(HollywoodEngine *vm, const PlayableSceneConfig &config) :
		_vm(vm),
		_config(config),
		_sceneStateId(vm->gameState().mainFlowStateId),
		_resources(),
		_sceneChunkTable(_resources._chunkTable),
		_resourceChunkOffsets(_resources._chunkOffsets),
		_resourceArena(_resources._arena),
		_metadata(_resources._metadata),
		_surfaceState(),
		_paletteResource(_surfaceState._paletteResource),
		_paletteCurrent(_surfaceState._paletteCurrent),
		_actorPaletteBase(_surfaceState._actorPaletteBase),
		_baseFramebufferOriginal(_surfaceState._baseFramebufferOriginal),
		_baseFramebuffer(_surfaceState._baseFramebuffer),
		_sceneFramebuffer(_surfaceState._sceneFramebuffer),
		_savedFramebuffer(_surfaceState._savedFramebuffer),
		_fillRuns(_surfaceState._fillRuns),
		_paletteMaskOriginal(_surfaceState._paletteMaskOriginal),
		_paletteMask(_surfaceState._paletteMask),
		_fullPaletteRegionMask(_surfaceState._fullPaletteRegionMask),
		_walkablePaletteMask(_surfaceState._walkablePaletteMask),
		_colorToActorDepthClassMap(_surfaceState._colorToActorDepthClassMap),
		_presentationPaletteRemapTable(_surfaceState._presentationPaletteRemapTable),
		_actorDepthYThresholds(_surfaceState._actorDepthYThresholds),
		_drawActorDepthYThresholds(_surfaceState._drawActorDepthYThresholds),
		_screen(_surfaceState._screen),
		_displayPalette(_surfaceState._displayPalette),
		_textStore(),
		_stage003DecodeKey(_textStore._decodeKey),
		_stage003SmallRows(_textStore._stageSmallRows),
		_pathController(),
		_routeBoundaryPoints(_pathController._routeBoundaryPoints),
		_routeSteps(_pathController._routeSteps),
		_actorPathFrames(_pathController._frames),
		_actorPathStepDeltas(_pathController._stepDeltas),
		_panelArt(vm->getLanguage()),
		_residentSoundEffects(vm->isDemo() && vm->getPlatform() == Common::kPlatformDOS),
		_random(Common::String::format("scene%u", config.sceneId)),
		_sceneLayers(),
		_realtimeAnimationTracks(_sceneLayers),
		_animationPlayer(*this),
		_concurrentActorPathChannel(),
		_speechController(vm->getLanguage(), vm->hasSpeechData()),
		_realtimeSpeechPlayer(vm->getLanguage(), vm->hasSpeechData()),
		_speech(_speechController._player),
		_speechOverlay(_speechController._secondaryOverlay),
		_primarySpeechOverlay(_speechController._primaryOverlay),
		_primaryLeftSpeechLastFrame(_speechController._primaryLeftSpeechLastFrame),
		_primaryDialogueSpeechLastFrame(_speechController._primaryDialogueSpeechLastFrame),
		_primaryDialogueSpeechGroup(_speechController._primaryDialogueSpeechGroup),
		_primaryLeftSpeechActive(_speechController._primaryLeftSpeechActive),
		_primaryDialogueSpeechActive(_speechController._primaryDialogueSpeechActive),
		_secondaryActorTimerAccumulator(_speechController._secondaryActorTimerAccumulator),
		_primaryLeftSpeechTimerAccumulator(_speechController._primaryLeftSpeechTimerAccumulator),
		_primaryDialogueSpeechTimerAccumulator(_speechController._primaryDialogueSpeechTimerAccumulator),
		_secondaryActorFrame(_speechController._secondaryActorFrame),
		_realtimeSpeechElapsed(0),
		_realtimeSpeechDuration(0),
		_realtimeSpeechTextRecordId(0),
		_realtimeSpeechVoiceSampleId(0),
		_realtimeSpeechCenterX(0),
		_realtimeSpeechTopY(0),
		_realtimeSpeechContinuationPart(0),
		_realtimeSpeechContinuationCount(0),
		_realtimeSpeechAnimationGroup(kInvalidPrimarySpeechAnimationGroup),
		_realtimeSpeechId(0),
		_realtimeSpeechVolumePercent(100),
		_realtimeSpeechActive(false),
		_realtimeSpeechPrimary(false),
		_actionOverlayPlayer(),
		_hideActiveActor(_actionOverlayPlayer._hideActiveActor),
		_ambientMusicTimerAccumulator(0),
		_previousAmbientMusicTrackId(0),
		_currentAmbientSoundCueId(0),
		_previousAmbientSoundCueId(0),
		_viewportXOffset(0),
		_viewportMinXOffset(0),
		_viewportMaxXOffset(0),
		_lastViewportScrollActorWorldX(config.defaultActorPose.x),
		_concurrentActorPathFrameIndex(0),
		_concurrentActorPathActive(false),
		_actorPathPlaybackActive(false),
		_playerDirectedActorPathActive(false),
		_playerDirectedActorPathFootstepPlayed(false),
		_pendingGameplayActionHandlerId(0),
		_activeActorWorldX(config.defaultActorPose.x),
		_activeActorWorldY(config.defaultActorPose.y),
		_activeActorFacing(config.defaultActorPose.facing),
		_activeActorCel(0),
		_activeActorDrawOrderMode(0),
		_lastSceneActionItemId(0),
		_lastInventoryActionItemId(0),
		_lastInventoryPrimaryItemId(0),
		_skipRequested(false),
		_stepAdvanceRequested(false) {
	_surfaceState.initialize(kPaletteSize, kScenePaletteMapByteCount, kPaletteMaskUsedBytes, kScenePaletteMapPageSize,
		kScenePaletteRegionCount, kActorPaletteBaseBytes);
	_speechController.initialize(kDefaultSecondarySpeechTextColor, kDefaultPrimarySpeechTextColor);
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

PlayableScene::BlockingSequence::BlockingSequence(PlayableScene &scene) :
		_scene(scene),
		_running(true),
		_aborted(false) {
	refresh();
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::layerFrames(
		uint layerId, const AnimationFrameRange &range) {
	if (canRun())
		finishMediaStep(_scene.playAnimationFrames(layerId, range));
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::presentedLayerFrames(
		uint layerId, const AnimationFrameRange &range) {
	if (canRun())
		finishMediaStep(_scene.playAndPresentAnimationFrames(layerId, range));
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::presentedLayerTransition(
		uint layerId, const AnimationTransition &transition) {
	if (canRun())
		finishMediaStep(_scene.playAndPresentAnimationTransition(layerId, transition));
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::resourceLayerFrames(
		uint layerId, uint chunkIndex, uint16 descriptorCount, const byte *frameMap,
		uint frameMapSize, const AnimationFrameRange &range, bool clearAtEnd) {
	if (canRun()) {
		finishMediaStep(_scene.playResourceLayerSequence(layerId, chunkIndex,
			descriptorCount, frameMap, frameMapSize, range, clearAtEnd));
	}
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::actorReplacement(
		uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis) {
	return actorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		frameMap, frameMapSize, frameMillis));
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::actorReplacement(
		const ActionOverlaySpec &spec) {
	if (canRun()) {
		_scene.runActorReplacement(spec);
		refresh();
	}
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::actorPose(
		const SceneActorPose &pose, byte cel) {
	if (canFinalize())
		_scene.setActiveActorPose(pose.x, pose.y, pose.facing, cel);
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::actorPath(
		const SceneActorPose &target, byte cel, bool cancelOnSkip) {
	if (canRun()) {
		_running = _scene.walkActiveActorTo(target.x, target.y, target.facing, cel, cancelOnSkip);
		refresh();
	}
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::sound(
		uint16 cueId, byte volumePercent) {
	if (canRun())
		_scene._soundBank0.playSample(cueId, volumePercent);
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::loopingSound(
		uint16 cueId, byte volumePercent) {
	if (canRun())
		_scene._soundBank0.playSampleLooping(cueId, volumePercent);
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::stopSound() {
	_scene._soundBank0.stop();
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::secondarySpeech(
		uint16 rowIndex, byte frameIndex) {
	if (canRun()) {
		_scene.beginSecondarySpeechLine(rowIndex, frameIndex);
		refresh();
	}
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::primarySpeech(
		uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue) {
	if (canRun()) {
		_scene.beginPrimarySpeechLine(rowIndex, frameIndex, centerX, topY, red, green, blue);
		refresh();
	}
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::framebufferPatch(byte selector) {
	if (canFinalize())
		_scene.applySceneStateToHotspotsAndPatches(selector);
	return *this;
}

PlayableScene::BlockingSequence &PlayableScene::BlockingSequence::fadeFromBlack() {
	if (canRun()) {
		if (_scene.fadePaletteFromBlack())
			_running = false;
		refresh();
	}
	return *this;
}

bool PlayableScene::BlockingSequence::canRun() {
	refresh();
	return _running;
}

bool PlayableScene::BlockingSequence::canFinalize() {
	refresh();
	return !_aborted;
}

void PlayableScene::BlockingSequence::finishMediaStep(bool completed) {
	if (!completed) {
		const bool stepAdvanced = _scene.consumeStepAdvanceRequest();
		if (!stepAdvanced && !_scene._skipRequested)
			_running = false;
	}
	refresh();
}

void PlayableScene::BlockingSequence::refresh() {
	if (_scene.animationPlaybackShouldStop()) {
		_running = false;
		_aborted = true;
	}
}

PlayableSceneConfig::PlayableSceneConfig(uint16 sceneNumber, const SceneResourceLayout &resourceLayout,
		const SceneViewport &sceneViewport, const SceneActorPose &actorPose) :
		sceneId(sceneNumber),
		resourceArchiveName(Common::String::format("RESOURCE.%c%02u",
			(char)('A' + sceneNumber / 1000 - 1), (sceneNumber % 1000) / 10)),
		initialRequiredChunkCount(resourceLayout.initialRequiredChunkCount),
		arenaFirstChunk(resourceLayout.arenaFirstChunk),
		arenaLastChunk(resourceLayout.arenaLastChunk),
		stageIndex(sceneNumber / 10),
		debugName(Common::String::format("Scene %u", sceneNumber)),
		viewportXOffset(sceneViewport.xOffset),
		viewportMinXOffset(sceneViewport.minXOffset),
		viewportMaxXOffset(sceneViewport.maxXOffset),
		defaultActorPose(actorPose),
		drawDefaultActor(true),
		entrySequenceOwnsFirstPresentation(false),
		inventoryOwnerIndex(sceneNumber / 1000 == 7 ? 1 : 0),
		activeAudioChapterIndex(sceneNumber / 1000 == 7 ? kSceneConfigNoAudioChapter : sceneNumber / 1000),
		actorBankTableEntry(0xd0),
		actorPaletteTableEntry(0x108),
		inventoryActionTableExtraOffset(sceneNumber / 1000 == 7 ? kResource000FixedInventoryVerbTableOffset : 0),
		inventoryRowsOffsetIndex(0x32),
		speechCueDescriptorTableOffset(0x5f58),
		actorPathStepDeltaTable(sceneNumber / 1000 == 7 ?
			kActorPathStepDeltaTableSet00 : kActorPathStepDeltaTableSetB4),
		actorPathStepDeltaTableSize(kActorPathStepDeltaTableSize),
		walkablePaletteMaxRegion(3),
		musicArchiveName(Common::String::format("RESOURCE.M%02u", sceneNumber / 1000)),
		soundBank0ArchiveName(Common::String::format("RESOURCE.S%02u", sceneNumber / 1000)),
		loadInventoryActionTables(true),
		loadActorDepthTables(true),
		useActorDepthTest(false) {
}

void PlayableSceneConfig::setActorResources(uint bankTableEntry, uint paletteTableEntry) {
	actorBankTableEntry = bankTableEntry;
	actorPaletteTableEntry = paletteTableEntry;
}

void PlayableSceneConfig::setTextResources(uint rowsOffsetIndex, uint32 cueDescriptorTableOffset) {
	inventoryRowsOffsetIndex = rowsOffsetIndex;
	speechCueDescriptorTableOffset = cueDescriptorTableOffset;
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
	if (!load()) {
		warning("%s load failed before gameplay loop", sceneDebugName());
		return false;
	}

	const bool resumeGameplayPose = hasSavedActiveActorPoseForCurrentState();
	initializePreviewState();
	if (!resumeGameplayPose) {
		drawPreviewComposite();
		if (shouldPresentPreviewBeforeEntrySequence())
			presentFrame();
		runEntryCutscene();
		if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
			return true;
		if (_vm->isDemo() && _vm->gameState().mainFlowStateId == 0xff)
			return true;
	}

	_skipRequested = false;
	const bool result = runBasicGameplayLoop();
	stopRealtimeSpeech();
	if (!_vm->isSceneRestartRequested()) {
		if (_vm->gameState().activeActorPoseStateId != _vm->gameState().mainFlowStateId)
			_vm->gameState().activeActorPoseValid = false;
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

void PlayableScene::setActiveActorPose(int worldX, int worldY, byte facing, byte cel) {
	_activeActorWorldX = worldX;
	_activeActorWorldY = worldY;
	_activeActorFacing = facing;
	_activeActorCel = cel;
	_activeActorDrawOrderMode = paletteRegionAt(worldX, worldY);
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
	return _config.resourceArchiveName.c_str();
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
	return _config.debugName.c_str();
}

uint16 PlayableScene::sceneViewportXOffset() const {
	return _config.viewportXOffset;
}

uint16 PlayableScene::sceneViewportMinXOffset() const {
	return _config.viewportMinXOffset;
}

uint16 PlayableScene::sceneViewportMaxXOffset() const {
	return _config.viewportMaxXOffset;
}

byte PlayableScene::inventoryOwnerIndex() const {
	return _config.inventoryOwnerIndex;
}

void PlayableScene::initializeInventoryOwnerState() {
	GameplayState &state = _vm->gameState();
	if (_config.inventoryOwnerIndex == 0) {
		state.initializeRonItemResourcePages();
	} else {
		state.initializeSueItemResourcePages();
	}
	if (_config.activeAudioChapterIndex != kSceneConfigNoAudioChapter)
		state.activeAudioChapterIndex = _config.activeAudioChapterIndex;
}

uint PlayableScene::resource000ActorBankTableEntry() const {
	return _config.actorBankTableEntry;
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

uint PlayableScene::framebufferResourceChunkIndex() const {
	return 0;
}

byte PlayableScene::walkablePaletteMaxRegion() const {
	return _config.walkablePaletteMaxRegion;
}

const char *PlayableScene::musicArchiveName() const {
	return _config.musicArchiveName.c_str();
}

const char *PlayableScene::soundBank0ArchiveName() const {
	return _config.soundBank0ArchiveName.c_str();
}

int PlayableScene::alternatePaletteResourceChunkIndex() const {
	return -1;
}

bool PlayableScene::isAlternatePaletteResourceActive() const {
	return false;
}

int PlayableScene::replacementFillRunsResourceChunkIndex() const {
	return -1;
}

int PlayableScene::replacementPaletteMaskResourceChunkIndex() const {
	return -1;
}

bool PlayableScene::shouldLoadInventoryActionTables() const {
	return _config.loadInventoryActionTables;
}

bool PlayableScene::shouldLoadActorDepthTables() const {
	return _config.loadActorDepthTables;
}

bool PlayableScene::shouldConvertSavedFramebufferFF() const {
	return false;
}

bool PlayableScene::shouldLoadArenaChunk(uint index) const {
	// Demo archives omit chunks for branches disabled by their preset state.
	return !_vm->isDemo() || _sceneChunkTable.isValidChunk(index);
}

void PlayableScene::runExitSideEffectsAfterLoop() {
}

bool PlayableScene::shouldPresentPreviewBeforeEntrySequence() const {
	return !_config.entrySequenceOwnsFirstPresentation;
}

bool PlayableScene::shouldUseActorDepthTest(int actorWorldX, int actorWorldY) const {
	(void)actorWorldX;
	(void)actorWorldY;
	return usesActorDepthTest();
}

bool PlayableScene::usesActorDepthTest() const {
	return _config.useActorDepthTest;
}

bool PlayableScene::isMainFlowStateInScene(uint16 stateId) const {
	return isGameplayStateInScene(_sceneStateId, stateId);
}

bool PlayableScene::didLeaveSceneAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void PlayableScene::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
}

void PlayableScene::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	prepareCustomComposite(drawActiveActor || drawSecondaryActor, activeFacing,
		activeWorldX, activeWorldY, actorDrawOrderMode);
	copyBaseFramebufferToSceneFramebuffer();
	drawCustomBackgroundComposite(activeWorldX, activeWorldY);
	drawLayerStack(kSceneAnimationBehindActors);
	drawActionOverlayAtStratum(kSceneAnimationBehindActors);

	if (_sceneLayers.hasVisibleLayers(kSceneAnimationActorReplacement)) {
		if (drawActiveActor || drawSecondaryActor)
			updateActorPaletteForWorldPoint(activeWorldX, activeWorldY);
		drawLayerStack(kSceneAnimationActorReplacement);
	} else {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}
	drawCustomActorForegroundComposite(activeWorldX, activeWorldY, actorDrawOrderMode);
	drawActionOverlayAtStratum(kSceneAnimationActorReplacement);
	drawLayerStack(kSceneAnimationInFrontOfActors);
	drawActionOverlayAtStratum(kSceneAnimationInFrontOfActors);
	drawActionOverlayAtStratum(kSceneAnimationScenePlaced);
	drawCustomForegroundComposite(activeWorldX, activeWorldY);
}

void PlayableScene::prepareCustomComposite(bool drawActors, byte activeFacing,
		int activeWorldX, int activeWorldY, byte actorDrawOrderMode) {
	(void)drawActors;
	(void)activeFacing;
	(void)activeWorldX;
	(void)activeWorldY;
	(void)actorDrawOrderMode;
}

void PlayableScene::drawCustomBackgroundComposite(int activeWorldX, int activeWorldY) {
	(void)activeWorldX;
	(void)activeWorldY;
}

void PlayableScene::drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldX;
	(void)activeWorldY;
	(void)actorDrawOrderMode;
}

void PlayableScene::drawCustomForegroundComposite(int activeWorldX, int activeWorldY) {
	(void)activeWorldX;
	(void)activeWorldY;
}

bool PlayableScene::shouldDrawSecondaryActorInPlayableComposite() const {
	return _speechOverlay.visible && !_actionOverlayPlayer.isVisible();
}

bool PlayableScene::shouldApplyGameplayPanelObjectPalette() const {
	return true;
}

void PlayableScene::runCustomEntrySequence() {
	drawPlayableComposite();
	presentFrame();
}

void PlayableScene::prepareCustomGameplayLoop() {
}

void PlayableScene::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
}

void PlayableScene::advancePrimarySpeechAnimation(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
}

void PlayableScene::advanceAmbientAudio(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
}

void PlayableScene::realtimeSpeechEnded(byte speechId, bool completed) {
	(void)speechId;
	(void)completed;
}

bool PlayableScene::dispatchCustomSceneAction(uint16 handlerId) {
	(void)handlerId;
	return false;
}

bool PlayableScene::shouldPlayGameplayClickPath() const {
	return true;
}

byte PlayableScene::customizeSceneActionFacing(uint16 handlerId, byte calculatedFacing) const {
	(void)handlerId;
	return calculatedFacing;
}

bool PlayableScene::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	(void)targetX;
	(void)targetY;
	return false;
}

void PlayableScene::prepareCustomActorPathRoute(int startX, int startY) {
	(void)startX;
	(void)startY;
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

void PlayableScene::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)hookId;
	(void)frame;
}

void PlayableScene::handleAnimationFrameEvent(const AnimationFrameEvent &event, uint frame) {
	switch (event.type) {
	case AnimationFrameEvent::kFramebufferPatch:
		applySceneStateToHotspotsAndPatches(event.selector);
		break;
	case AnimationFrameEvent::kResourcePatch:
		if (_sceneChunkTable.isValidChunk(event.resourceChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[event.resourceChunk], _baseFramebuffer);
		break;
	case AnimationFrameEvent::kSound:
	case AnimationFrameEvent::kLoopingSound: {
		SoundBank0Player *player = &_soundBank0;
		if (event.soundSlot != AnimationFrameEvent::kMainSoundSlot) {
			if (event.soundSlot >= kAmbientSoundSlotCount)
				break;
			player = event.soundSlot == 0 ?
				&_ambientSoundBank0 : &_additionalAmbientSoundBank0Slots[event.soundSlot - 1];
		}
		if (event.type == AnimationFrameEvent::kLoopingSound)
			player->playSampleLooping(event.soundId, event.soundVolumePercent);
		else
			player->playSample(event.soundId, event.soundVolumePercent);
		break;
	}
	case AnimationFrameEvent::kResidentSound:
		playResidentSoundEffect((byte)event.soundId, event.soundVolumePercent);
		break;
	case AnimationFrameEvent::kStopSound:
		_soundBank0.stop();
		break;
	case AnimationFrameEvent::kSecondarySpeech:
		startRealtimeSecondarySpeechLine(event.speechRow, event.speechFrame, event.speechId);
		break;
	case AnimationFrameEvent::kPrimarySpeech:
		beginPrimarySpeechLine(event.speechRow, event.speechFrame,
			event.speechCenterX, event.speechTopY,
			event.speechRed, event.speechGreen, event.speechBlue);
		break;
	case AnimationFrameEvent::kActorPath:
		startConcurrentActorPath(event.actorX, event.actorY, event.actorFacing,
			event.actorCel, event.actorPathFrameMillis);
		break;
	case AnimationFrameEvent::kLayerFrame:
		_sceneLayers.setLayerFrame(event.layerId, event.layerFrame);
		break;
	case AnimationFrameEvent::kLayerFrameMap: {
		const int mapIndex = (int)frame - event.layerFrameMapFirstFrame;
		if (event.layerFrameMap != nullptr && mapIndex >= 0 &&
				(uint)mapIndex < event.layerFrameMapSize) {
			const byte mappedFrame = event.layerFrameMap[mapIndex];
			if (event.showMappedLayer)
				_sceneLayers.setVisibleLayerFrame(event.layerId, mappedFrame);
			else
				_sceneLayers.setLayerFrame(event.layerId, mappedFrame);
		}
		break;
	}
	case AnimationFrameEvent::kLayerReset:
		_sceneLayers.resetLayer(event.layerId, event.layerFrame);
		break;
	case AnimationFrameEvent::kLayerVisibility:
		_sceneLayers.setLayerVisible(event.layerId, event.layerVisible);
		break;
	case AnimationFrameEvent::kStateCommit:
		if (event.commitFunction != nullptr)
			event.commitFunction(event.commitTarget, event.commitValue);
		break;
	case AnimationFrameEvent::kInvalidatePalette:
		invalidatePresentationPalette();
		break;
	case AnimationFrameEvent::kFadeFromBlack:
		drawPlayableComposite();
		fadePaletteFromBlack();
		break;
	case AnimationFrameEvent::kCustomHook:
		handleAnimationFrameHook(event.hookId, frame);
		break;
	}
}

void PlayableScene::advanceFullscreenAnimation(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
}

void PlayableScene::advanceTransitionAnimation(uint32 delta) {
	advanceCustomGameplayLoop(delta);
	_realtimeAnimationTracks.advance(delta, _random);
	advanceAmbientAudio(delta);
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

AmbientAudioProfile PlayableScene::createMineAmbientAudioProfile(byte musicFirstCueId,
		byte musicCueCount, byte musicProbabilityModulus) const {
	AmbientAudioProfile profile = createRandomAmbientAudioProfile(0x0d, 8, 10, 25,
		musicFirstCueId, musicCueCount, 100, musicProbabilityModulus);
	profile.soundCueVolumes = kMineAmbientSoundCueVolumes;
	profile.soundCueVolumeCount = ARRAYSIZE(kMineAmbientSoundCueVolumes);
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

	const char *archiveName = resourceArchiveName();
	debugC(1, kDebugResources, "%s load begin: state=0x%04x archive=%s stage=%u actorBankEntry=0x%04x actorPaletteEntry=0x%04x",
		sceneDebugName(), _vm->gameState().mainFlowStateId,
		archiveName != nullptr ? archiveName : "<none>", sceneStageIndex(),
		resource000ActorBankTableEntry(), resource000ActorPaletteTableEntry());

	if (!loadResource000RuntimeTables(_resource000OffsetTable, _resource000SizeTable)) {
		warning("%s load failed: RESOURCE.000 runtime tables", sceneDebugName());
		return false;
	}
	if (!loadResource000ActorBank(_resource000OffsetTable, _resource000SizeTable)) {
		warning("%s load failed: RESOURCE.000 actor bank entry 0x%04x", sceneDebugName(),
			resource000ActorBankTableEntry());
		return false;
	}
	if (shouldLoadInventoryActionTables() &&
			!loadResource000InventoryActionTables(_resource000OffsetTable)) {
		warning("%s load failed: RESOURCE.000 inventory action tables", sceneDebugName());
		return false;
	}
	if (!_panelArt.load()) {
		warning("%s load failed: bottom panel art", sceneDebugName());
		return false;
	}

	if (!archiveName || !archiveName[0]) {
		warning("%s has no resource archive configured", sceneDebugName());
		return false;
	}
	if (!_resources.loadChunkTable(archiveName)) {
		warning("%s load failed: %s chunk table", sceneDebugName(), archiveName);
		return false;
	}

	const uint framebufferChunkIndex = framebufferResourceChunkIndex();
	const uint requiredChunkCount = _vm->isDemo() ?
		MIN<uint>(sceneInitialRequiredChunkCount(), 5) : sceneInitialRequiredChunkCount();
	if (!_resources.validateRequiredChunks(archiveName, sceneDebugName(),
			requiredChunkCount, framebufferChunkIndex)) {
		warning("%s load failed: %s required chunk validation", sceneDebugName(), archiveName);
		return false;
	}

	if (!loadFixedChunk(framebufferChunkIndex, _baseFramebuffer, kSceneBufferByteCount)) {
		warning("%s load failed: %s chunk %u framebuffer", sceneDebugName(), archiveName,
			framebufferChunkIndex);
		return false;
	}
	if (!loadFixedChunk(1, _paletteResource, kPaletteSize)) {
		warning("%s load failed: %s chunk 1 palette", sceneDebugName(), archiveName);
		return false;
	}
	if (!loadVariableChunk(2, _fillRuns)) {
		warning("%s load failed: %s chunk 2 fill runs", sceneDebugName(), archiveName);
		return false;
	}
	if (!loadVariableChunk(3, _paletteMask)) {
		warning("%s load failed: %s chunk 3 palette mask", sceneDebugName(), archiveName);
		return false;
	}
	if (!loadVariableChunk(4, _metadata)) {
		warning("%s load failed: %s chunk 4 metadata", sceneDebugName(), archiveName);
		return false;
	}

	const int replacementFillRunsChunkIndex = replacementFillRunsResourceChunkIndex();
	if (replacementFillRunsChunkIndex >= 0 &&
			!loadVariableChunk((uint)replacementFillRunsChunkIndex, _fillRuns)) {
		warning("%s load failed: %s replacement fill runs chunk %d", sceneDebugName(),
			archiveName, replacementFillRunsChunkIndex);
		return false;
	}
	const int replacementPaletteMaskChunkIndex = replacementPaletteMaskResourceChunkIndex();
	if (replacementPaletteMaskChunkIndex >= 0 &&
			!loadVariableChunk((uint)replacementPaletteMaskChunkIndex, _paletteMask)) {
		warning("%s load failed: %s replacement palette mask chunk %d", sceneDebugName(),
			archiveName, replacementPaletteMaskChunkIndex);
		return false;
	}

	const int alternatePaletteChunkIndex = alternatePaletteResourceChunkIndex();
	if (alternatePaletteChunkIndex >= 0 && isAlternatePaletteResourceActive()) {
		if (!loadFixedChunk((uint)alternatePaletteChunkIndex, _paletteResource, kPaletteSize)) {
			warning("%s load failed: %s alternate palette chunk %d", sceneDebugName(),
				archiveName, alternatePaletteChunkIndex);
			return false;
		}
	}

	_baseFramebufferOriginal.copyFrom(_baseFramebuffer);
	_paletteMaskOriginal = _paletteMask;

	if (_paletteMask.size() < kPaletteMaskUsedBytes) {
		warning("%s chunk 3 is shorter than the scene palette mask table", resourceArchiveName());
		return false;
	}
	if (shouldLoadActorDepthTables() && !initializeActorDepthTables()) {
		warning("%s load failed: actor depth tables", sceneDebugName());
		return false;
	}
	if (!initializeScenePathTables()) {
		warning("%s load failed: path tables", sceneDebugName());
		return false;
	}

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
		if (!loadArenaChunk(i)) {
			warning("%s load failed: %s arena chunk %u", sceneDebugName(), archiveName, i);
			return false;
		}
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
	if (!loadResource000ActorPalette(_resource000OffsetTable)) {
		warning("%s load failed: RESOURCE.000 actor palette entry 0x%04x", sceneDebugName(),
			resource000ActorPaletteTableEntry());
		return false;
	}
	if (!loadStage003SceneRows()) {
		warning("%s load failed: RESOURCE.003 stage %u rows", sceneDebugName(), sceneStageIndex());
		return false;
	}
	if (shouldApplyGameplayPanelObjectPalette())
		_panelArt.applyInteractiveObjectPalette(_paletteCurrent);
	if (_paletteMaskOriginal.size() >= kScenePresentationPaletteRemapMap + kScenePaletteMapPageSize) {
		memcpy(_presentationPaletteRemapTable.data(),
			_paletteMaskOriginal.data() + kScenePresentationPaletteRemapMap,
			kScenePaletteMapPageSize);
	} else {
		_surfaceState.rebuildPresentationPaletteRemapTable();
	}

	if (!_hotspots.load(_paletteMask, _metadata, _stage003SmallRows)) {
		warning("%s load failed: hotspot table", sceneDebugName());
		return false;
	}

	resetViewportFromScene();

	_vm->gameplayMusic()->setArchive(Common::Path(musicArchiveName()));
	_soundBank0.setArchive(Common::Path(soundBank0ArchiveName()));
	_ambientSoundBank0.setArchive(Common::Path(soundBank0ArchiveName()));
	for (uint i = 0; i < ARRAYSIZE(_additionalAmbientSoundBank0Slots); ++i)
		_additionalAmbientSoundBank0Slots[i].setArchive(Common::Path(soundBank0ArchiveName()));

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
	const uint segmentCount = kActorBankSegmentCount;
	if (tableEntry + 4 > offsetTable.size() ||
			tableEntry + segmentCount * 4 > sizeTable.size()) {
		warning("%s %s actor bank table entries are out of range: tableEntry=0x%04x segments=%u offsetTableSize=%u sizeTableSize=%u",
			sceneDebugName(), kResource000Name, tableEntry, segmentCount,
			(uint)offsetTable.size(), (uint)sizeTable.size());
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s actor bank", kResource000Name);
		return false;
	}

	const uint32 actorBankOffset = readUint32LE(offsetTable, tableEntry);
	if (actorBankOffset > (uint32)file.size()) {
		warning("%s %s actor bank offset is out of range: tableEntry=0x%04x offset=%u fileSize=%u",
			sceneDebugName(), kResource000Name, tableEntry, actorBankOffset, (uint)file.size());
		return false;
	}

	debugC(1, kDebugResources, "%s loading %s actor bank: tableEntry=0x%04x offset=%u segments=%u",
		sceneDebugName(), kResource000Name, tableEntry, actorBankOffset, segmentCount);

	file.seek(actorBankOffset);
	memset(_activeActorRunStreams.data(), 0, _activeActorRunStreams.size());
	memset(_secondaryActorRunStreams.data(), 0, _secondaryActorRunStreams.size());

	for (uint segment = 0; segment < segmentCount; ++segment) {
		const uint32 segmentSize = readUint32LE(sizeTable, tableEntry + segment * 4);
		const uint32 segmentFileOffset = (uint32)file.pos();
		debugC(2, kDebugResources, "%s %s actor bank segment %u: fileOffset=%u size=%u",
			sceneDebugName(), kResource000Name, segment, segmentFileOffset, segmentSize);
		if (segment <= 5) {
			if (segmentSize > kActiveActorFacingRunStride) {
				warning("%s %s actor active run segment %u is too large: tableEntry=0x%04x fileOffset=%u size=%u limit=%u",
					sceneDebugName(), kResource000Name, segment, tableEntry, segmentFileOffset,
					segmentSize, (uint)kActiveActorFacingRunStride);
				return false;
			}
			if (file.read(_activeActorRunStreams.data() + segment * kActiveActorFacingRunStride, segmentSize) != segmentSize) {
				warning("%s failed to read %s actor active run segment %u: tableEntry=0x%04x fileOffset=%u size=%u",
					sceneDebugName(), kResource000Name, segment, tableEntry, segmentFileOffset, segmentSize);
				return false;
			}
		} else if (segment == 6) {
			if (segmentSize % kActiveActorDescriptorSize != 0) {
				warning("%s %s actor active descriptors have invalid size: tableEntry=0x%04x fileOffset=%u size=%u descriptorSize=%u",
					sceneDebugName(), kResource000Name, tableEntry, segmentFileOffset,
					segmentSize, (uint)kActiveActorDescriptorSize);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("%s failed to read %s actor active descriptors: tableEntry=0x%04x fileOffset=%u size=%u",
					sceneDebugName(), kResource000Name, tableEntry, segmentFileOffset, segmentSize);
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
				warning("%s %s actor secondary run segment %u is too large: tableEntry=0x%04x fileOffset=%u size=%u limit=%u",
					sceneDebugName(), kResource000Name, facing, tableEntry, segmentFileOffset,
					segmentSize, (uint)kSecondaryActorFacingRunStride);
				return false;
			}
			if (file.read(_secondaryActorRunStreams.data() + facing * kSecondaryActorFacingRunStride, segmentSize) != segmentSize) {
				warning("%s failed to read %s actor secondary run segment %u: tableEntry=0x%04x fileOffset=%u size=%u",
					sceneDebugName(), kResource000Name, facing, tableEntry, segmentFileOffset, segmentSize);
				return false;
			}
		} else {
			if (segmentSize % kSecondaryActorDescriptorSize != 0) {
				warning("%s %s actor secondary descriptors have invalid size: tableEntry=0x%04x fileOffset=%u size=%u descriptorSize=%u",
					sceneDebugName(), kResource000Name, tableEntry, segmentFileOffset,
					segmentSize, (uint)kSecondaryActorDescriptorSize);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("%s failed to read %s actor secondary descriptors: tableEntry=0x%04x fileOffset=%u size=%u",
					sceneDebugName(), kResource000Name, tableEntry, segmentFileOffset, segmentSize);
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
	captureActorPaletteBase();

	return true;
}

void PlayableScene::captureActorPaletteBase() {
	const uint paletteOffset = kActorPaletteFirstColor * 3;
	if (_paletteCurrent.size() < paletteOffset + _actorPaletteBase.size())
		return;

	memcpy(_actorPaletteBase.data(), _paletteCurrent.data() + paletteOffset,
		_actorPaletteBase.size());
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
	const uint stageIndex = sceneStageIndex();
	// Demo recordings use compact sample numbers, not the full chapter's sequence.
	const bool validateSequentialVoiceMap = !_vm->isDemo() && stageIndex >= 201 && stageIndex <= 211;
	return _textStore.load(kStage003ArchiveName, sceneDebugName(), stageIndex,
		resource003InventoryRowsOffsetIndex(), speechCueDescriptorTableOffset(),
		validateSequentialVoiceMap);
}

bool PlayableScene::loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(sceneDebugName(), index, destination, fixedSize);
}

bool PlayableScene::loadFixedChunk(uint index, Graphics::ManagedSurface &destination, uint fixedSize) {
	return _resources.loadFixedChunk(sceneDebugName(), index, destination, fixedSize);
}

bool PlayableScene::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	return _resources.loadVariableChunk(index, destination);
}

bool PlayableScene::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(sceneDebugName(), index);
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
	initializeCustomPreviewState();
}

void PlayableScene::initializeDefaultPreviewState() {
	_speechController.resetRuntimeState(kInvalidPrimarySpeechAnimationGroup, 7);
	_actionOverlayPlayer.reset();
	_sceneLayers.reset();
	cancelConcurrentActorPath();
	resetAmbientAudioState();
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_lastSceneActionItemId = 0;
	_lastInventoryActionItemId = 0;
	_lastInventoryPrimaryItemId = 0;
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
	drawCustomComposite(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY,
		actorDrawOrderMode);
}

void PlayableScene::drawPlayableComposite() {
	const bool drawActiveActor = _config.drawDefaultActor && !_hideActiveActor;
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
	} else if (drawActiveActor) {
		drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, minimumYExclusive);
	}

	if (drawActiveActor || drawSecondaryActor)
		updateActorPaletteForWorldPoint(activeWorldX, activeWorldY);
}

void PlayableScene::updateActorPaletteForWorldPoint(int worldX, int worldY) {
	const uint paletteOffset = kActorPaletteFirstColor * 3;
	if (_actorPaletteBase.size() != kActorPaletteBaseBytes ||
			_paletteCurrent.size() < paletteOffset + _actorPaletteBase.size() ||
			_paletteMask.size() < kSceneColorToActorPaletteAdjustmentClassMap + kScenePaletteMapPageSize ||
			_metadata.size() < kPaletteAdjustTable + kActorPaletteAdjustmentClassCount * 2)
		return;

	memcpy(_paletteCurrent.data() + paletteOffset, _actorPaletteBase.data(),
		_actorPaletteBase.size());
	if (worldX < 0 || worldX >= HollywoodEngine::kSceneBufferWidth ||
			worldY < 0 || worldY >= HollywoodEngine::kSceneBufferHeight)
		return;

	const byte sceneColor = *(const byte *)_savedFramebuffer.getBasePtr(worldX, worldY);
	const byte deltaClass = _paletteMask[kSceneColorToActorPaletteDeltaClassMap + sceneColor];
	int paletteDelta = 0;
	if (deltaClass < kScenePaletteRegionCount)
		paletteDelta = (int8)_metadata[kPaletteDeltaTable + deltaClass];

	for (uint i = 0; i < _actorPaletteBase.size(); ++i) {
		_paletteCurrent[paletteOffset + i] =
			(byte)CLIP<int>((int)_actorPaletteBase[i] + paletteDelta, 0, 0x3f);
	}

	const byte adjustmentClass =
		_paletteMask[kSceneColorToActorPaletteAdjustmentClassMap + sceneColor];
	if (adjustmentClass >= kActorPaletteAdjustmentClassCount)
		return;

	const uint adjustmentOffset = kPaletteAdjustTable + adjustmentClass * 2;
	const byte specialPaletteSet = _metadata[adjustmentOffset];
	if (specialPaletteSet == 0 || specialPaletteSet > ARRAYSIZE(kActorPaletteAdjustmentDestinations))
		return;

	const int adjustmentDelta = paletteDelta + (int8)_metadata[adjustmentOffset + 1];
	const byte *destinations = kActorPaletteAdjustmentDestinations[specialPaletteSet - 1];
	for (uint color = 0; color < ARRAYSIZE(kActorPaletteAdjustmentSources); ++color) {
		const uint sourceOffset = (kActorPaletteAdjustmentSources[color] - kActorPaletteFirstColor) * 3;
		const uint destinationOffset = destinations[color] * 3;
		for (uint component = 0; component < 3; ++component) {
			_paletteCurrent[destinationOffset + component] = (byte)CLIP<int>(
				(int)_actorPaletteBase[sourceOffset + component] + adjustmentDelta, 0, 0x3f);
		}
	}

	const byte copySourceColor = destinations[0];
	memcpy(_paletteCurrent.data() + kActorPaletteAdjustmentCopyColor * 3,
		_paletteCurrent.data() + copySourceColor * 3,
		kActorPaletteAdjustmentCopyColorCount * 3);
}

void PlayableScene::drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= kResourceChunkCount || frameIndex >= frameMapSize)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		descriptorCount, frameMap[frameIndex], _sceneFramebuffer);
}

void PlayableScene::restoreResourceSpriteLayerBackground(const ResourceSpriteLayer &layer, const Graphics::Surface &background) {
	if (!layer.visible || layer.chunkIndex >= kResourceChunkCount)
		return;

	if (layer.hasPreviousDescriptor) {
		restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[layer.chunkIndex], 0,
			layer.descriptorCount, layer.previousDescriptorIndex, background, *_sceneFramebuffer.surfacePtr());
	}
	restoreSpriteBackground(_resourceArena, _resourceChunkOffsets[layer.chunkIndex], 0,
		layer.descriptorCount, layer.descriptorIndex(), background, *_sceneFramebuffer.surfacePtr());
}

void PlayableScene::drawResourceSpriteLayer(const ResourceSpriteLayer &layer) {
	if (!layer.visible || layer.chunkIndex >= kResourceChunkCount)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[layer.chunkIndex], 0,
		layer.descriptorCount, layer.descriptorIndex(), _sceneFramebuffer);
}

void PlayableScene::restoreSceneLayerBackground(uint layerId,
		const Graphics::Surface &background) {
	if (_sceneLayers.hasLayer(layerId))
		restoreResourceSpriteLayerBackground(_sceneLayers.layer(layerId), background);
}

void PlayableScene::drawSceneLayer(uint layerId) {
	if (_sceneLayers.hasLayer(layerId))
		drawResourceSpriteLayer(_sceneLayers.layer(layerId));
}

void PlayableScene::drawLayerStack(SceneAnimationStratum stratum) {
	for (uint i = 0; i < _sceneLayers.layerCount(); ++i) {
		if (_sceneLayers.isInStratum(i, stratum))
			drawResourceSpriteLayer(_sceneLayers.layer(i));
	}
}

void PlayableScene::drawActionOverlayAtStratum(SceneAnimationStratum stratum) {
	if (_actionOverlayPlayer.isVisible() && _actionOverlayPlayer._stratum == stratum)
		drawActionOverlayLayer();
}

bool PlayableScene::playResourceLayerSequence(uint layerId, uint chunkIndex,
		uint16 descriptorCount, const byte *frameMap, uint frameMapSize,
		const AnimationFrameRange &range, bool clearAtEnd) {
	if (!_sceneLayers.hasLayer(layerId))
		return false;

	ResourceSpriteLayer &layer = _sceneLayers.layer(layerId);
	layer.configure(chunkIndex, descriptorCount, frameMap, frameMapSize);
	layer.visible = true;
	const bool completed = _animationPlayer.playAndPresent(layer, range);
	if (clearAtEnd) {
		layer.visible = false;
		layer.configure(0, 0, nullptr, 0);
	}
	return completed;
}

void PlayableScene::clearSceneLayer(uint layerId) {
	if (!_sceneLayers.hasLayer(layerId))
		return;

	ResourceSpriteLayer &layer = _sceneLayers.layer(layerId);
	layer.visible = false;
	layer.configure(0, 0, nullptr, 0);
}

void PlayableScene::drawActionOverlayLayer() {
	if (_actionOverlayPlayer.isVisible() &&
			_actionOverlayPlayer._restoreBackgroundBeforeDraw)
		restoreResourceSpriteLayerBackground(_actionOverlayPlayer._layer, _baseFramebuffer);
	drawResourceSpriteLayer(_actionOverlayPlayer._layer);
}

void PlayableScene::drawClipFrameDeltaFromResource(const Common::Array<byte> &resource,
		uint32 frameTableOffset, uint32 chunkSize, uint tableEntryCount, byte frameIndex) {
	drawResourceDeltaClipFrame(resource, frameTableOffset, chunkSize, tableEntryCount,
		frameIndex, framebufferPixels(_sceneFramebuffer), framebufferByteCount());
}

void PlayableScene::drawClipFrameDelta(uint chunkIndex, uint tableEntryCount, byte frameIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;

	drawClipFrameDeltaFromResource(_resourceArena, _resourceChunkOffsets[chunkIndex],
		_sceneChunkTable.sizes[chunkIndex], tableEntryCount, frameIndex);
}

void PlayableScene::playDeltaClip(uint chunkIndex, uint tableEntryCount, uint frameCount,
		uint32 frameMillis, uint firstFrame) {
	for (uint frame = firstFrame; frame < frameCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		drawClipFrameDelta(chunkIndex, tableEntryCount, (byte)frame);
		presentFrame();
		if (waitDeltaClipFrameMillis(frameMillis)) {
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				break;
			consumeStepAdvanceRequest();
			for (++frame; frame < frameCount; ++frame)
				drawClipFrameDelta(chunkIndex, tableEntryCount, (byte)frame);
			presentFrame();
			break;
		}
	}
}

bool PlayableScene::playTransitionFrames(TransitionFrameRenderer &renderer, byte firstFrame,
		byte lastFrame, uint32 frameMillis, TransitionFrameMode mode, bool allowSkip) {
	if (firstFrame > lastFrame || frameMillis == 0)
		return false;

	uint frameIndex = firstFrame;
	uint32 frameAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	renderer.drawFrame((byte)frameIndex);
	presentFrame();

	while (frameIndex < lastFrame && !animationPlaybackShouldStop()) {
		if (pollEvents(allowSkip)) {
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				return false;

			const bool stepAdvanced = consumeStepAdvanceRequest();
			if (mode == kCumulativeTransitionFrames) {
				while (frameIndex < lastFrame)
					renderer.drawFrame((byte)++frameIndex);
			} else {
				frameIndex = lastFrame;
				renderer.drawFrame((byte)frameIndex);
			}
			presentFrame();
			return stepAdvanced;
		}

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		if (delta != 0)
			advanceTransitionAnimation(MIN<uint32>(delta, kBlockingSceneMaxFrameDeltaMillis));

		const uint previousFrame = frameIndex;
		while (frameAccumulator >= frameMillis && frameIndex < lastFrame) {
			frameAccumulator -= frameMillis;
			++frameIndex;
			if (mode == kCumulativeTransitionFrames)
				renderer.drawFrame((byte)frameIndex);
		}

		if (frameIndex != previousFrame) {
			if (mode == kIndependentTransitionFrames)
				renderer.drawFrame((byte)frameIndex);
			presentFrame();
		}

		if (frameIndex < lastFrame && frameAccumulator < frameMillis)
			g_system->delayMillis(MIN<uint32>(frameMillis - frameAccumulator, 10));
	}

	return frameIndex == lastFrame && !animationPlaybackShouldStop();
}

bool PlayableScene::waitDeltaClipFrameMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

bool PlayableScene::waitFullscreenAnimationFrame(uint32 millis, bool allowSkip) {
	const uint32 startMillis = g_system->getMillis();
	uint32 lastMillis = startMillis;
	while (!animationPlaybackShouldStop()) {
		if (pollEvents(allowSkip))
			return true;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		if (delta != 0)
			advanceFullscreenAnimation(MIN<uint32>(delta, kBlockingSceneMaxFrameDeltaMillis));

		const uint32 elapsed = g_system->getMillis() - startMillis;
		if (elapsed >= millis)
			break;
		g_system->delayMillis(MIN<uint32>(millis - elapsed, 10));
	}

	return animationPlaybackShouldStop();
}

bool PlayableScene::playFullscreenDeltaAnimation(const Common::Array<byte> &base,
		const Common::Array<byte> &palette, const Common::Array<byte> &frames,
		uint frameCount, uint32 frameMillis) {
	if (palette.size() != kPaletteSize || frameCount == 0 ||
			frameCount > 0x100 || frameCount > frames.size() / 4) {
		warning("%s fullscreen delta animation has invalid resources", sceneDebugName());
		return false;
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

	bool completed = true;
	for (uint frame = 0; frame < frameCount && !animationPlaybackShouldStop(); ++frame) {
		if (!drawResourceDeltaClipFrame(frames, 0, frames.size(),
				frameCount, (byte)frame, framebufferPixels(_sceneFramebuffer),
				framebufferByteCount())) {
			warning("%s failed to decode fullscreen delta frame %u", sceneDebugName(), frame);
			completed = false;
			break;
		}
		presentFrame();

		if (waitFullscreenAnimationFrame(frameMillis, false)) {
			completed = false;
			break;
		}
	}

	_paletteCurrent = blackPalette;
	_displayPalette.markAllDirty();
	presentFrame();

	_sceneFramebuffer.copyRectToSurface(savedScene.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	_viewportXOffset = savedViewportX;
	presentFrame();
	drawPlayableComposite();
	presentFrame();

	_paletteCurrent = savedPalette;
	_displayPalette.markAllDirty();
	presentFrame();
	return completed && !animationPlaybackShouldStop();
}

bool PlayableScene::fadePaletteFromBlack() {
	const Common::Array<byte> targetPalette = _paletteCurrent;
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());

	byte threshold = 0x3f;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		for (uint i = 0; i < _paletteCurrent.size(); ++i) {
			if (targetPalette[i] >= threshold)
				_paletteCurrent[i] = MIN<byte>(targetPalette[i], _paletteCurrent[i] + 3);
		}
		presentFrame();
		if (threshold == 0)
			return false;

		threshold = threshold > 3 ? threshold - 3 : 0;
		if (pollEvents(false))
			return true;
		g_system->delayMillis(kPaletteFadeStepMillis);
	}

	return true;
}

bool PlayableScene::fadePaletteToBlack() {
	const Common::Array<byte> sourcePalette = _paletteCurrent;
	byte threshold = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		for (uint i = 0; i < _paletteCurrent.size(); ++i) {
			if (sourcePalette[i] >= threshold)
				_paletteCurrent[i] = _paletteCurrent[i] >= 3 ? _paletteCurrent[i] - 3 : 0;
		}
		presentFrame();
		if (threshold >= 0x3f)
			return false;

		threshold = MIN<byte>(0x3f, threshold + 3);
		if (pollEvents(false))
			return true;
		g_system->delayMillis(kPaletteFadeStepMillis);
	}

	return true;
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
	const uint runBase = facing * kActiveActorFacingRunStride;
	const uint paletteRunCursor = skipActorRunStream(_activeActorRunStreams,
		descriptor.runStreamOffset, runBase, descriptor.opaqueRunCount);
	drawActorPaletteRemapRun(_activeActorRunStreams, paletteRunCursor, runBase,
		descriptor.paletteRunCount, spriteX, spriteY, minimumYExclusive, worldX, worldY);
	drawActorRun(_activeActorRunStreams, descriptor.runStreamOffset, runBase,
		descriptor.opaqueRunCount, spriteX, spriteY, minimumYExclusive, worldX, worldY);
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
		descriptor.runCount, spriteX, spriteY, -1, worldX, worldY);
}

int PlayableScene::drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldX, int actorWorldY, uint *nextCursor) {
	if (shouldUseActorDepthTest(actorWorldX, actorWorldY)) {
		ActorDepthTest depthTest;
		depthTest.enabled = true;
		depthTest.savedFramebuffer = &_savedFramebuffer.rawSurface();
		depthTest.colorToDepthClassMap = &_colorToActorDepthClassMap;
		depthTest.depthYThresholds = &_drawActorDepthYThresholds;
		depthTest.actorWorldY = actorWorldY;

		return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
			minimumYExclusive, *_sceneFramebuffer.surfacePtr(), &depthTest, nextCursor);
	}

	return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
		minimumYExclusive, *_sceneFramebuffer.surfacePtr(), nullptr, nextCursor);
}

int PlayableScene::drawActorPaletteRemapRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldX, int actorWorldY) {
	if (shouldUseActorDepthTest(actorWorldX, actorWorldY)) {
		ActorDepthTest depthTest;
		depthTest.enabled = true;
		depthTest.savedFramebuffer = &_savedFramebuffer.rawSurface();
		depthTest.colorToDepthClassMap = &_colorToActorDepthClassMap;
		depthTest.depthYThresholds = &_drawActorDepthYThresholds;
		depthTest.actorWorldY = actorWorldY;

		return drawActorPaletteRemapRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
			minimumYExclusive, *_sceneFramebuffer.surfacePtr(), _presentationPaletteRemapTable, &depthTest);
	}

	return drawActorPaletteRemapRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
		minimumYExclusive, *_sceneFramebuffer.surfacePtr(), _presentationPaletteRemapTable, nullptr);
}

void PlayableScene::runEntryCutscene() {
	runCustomEntrySequence();
}

void PlayableScene::runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	centerViewportOnActor(_activeActorWorldX);

	drawPlayableComposite();
	presentFrame();

	queueActorPathWithPaletteRegionRouting(startX, startY, targetX, targetY, kInvalidFacing, 0);
	_lastViewportScrollActorWorldX = _activeActorWorldX;
	_actorPathPlaybackActive = true;
	bool footstepPlayed = false;
	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		handleActorPathFootstep(frameIndex + 1 == _actorPathFrames.size(), footstepPlayed);
		if (waitSceneMillis(kActorPathFrameMillis)) {
			_actorPathPlaybackActive = false;
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				return;
			consumeStepAdvanceRequest();
			break;
		}
	}
	_actorPathPlaybackActive = false;

	_activeActorWorldX = targetX;
	_activeActorWorldY = targetY;
	if (!_actorPathFrames.empty())
		_activeActorFacing = _actorPathFrames.back().facing;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_activeActorCel = 0;
	_lastViewportScrollActorWorldX = _activeActorWorldX;
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
	_stepAdvanceRequested = false;
	cancelConcurrentActorPath();
	stopRealtimeSpeech();
	_speechController.resetRuntimeState(kInvalidPrimarySpeechAnimationGroup, 7);
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_actionOverlayPlayer.reset();
	resetAmbientAudioState();
	prepareCustomGameplayLoop();
	restoreActiveActorPoseFromGameState();
	_lastViewportScrollActorWorldX = _activeActorWorldX;
	syncActiveActorPoseToGameState();
}

void PlayableScene::advanceGameplayLoop(uint32 delta) {
	advanceSecondaryActorSpeechAnimation(delta);
	advanceConcurrentActorPath(delta);
	advanceCustomGameplayLoop(delta);
	advanceRealtimeSpeech(delta);
	_realtimeAnimationTracks.advance(delta, _random);
	advancePrimarySpeechAnimation(delta);
	advanceAmbientAudio(delta);
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

Common::Path PlayableScene::optionsMenuSoundBank0ArchiveName() const {
	return Common::Path(soundBank0ArchiveName());
}

bool PlayableScene::optionsMenuSpeechPreviewSampleId(uint16 &sampleId) const {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	return getStaticSpeechCue(0, 0, textRecordId, continuationCount, sampleId);
}

void PlayableScene::suspendAudioForOptionsMenu() {
	stopRealtimeSpeech();
	_speech.stop();
	_soundBank0.stop();
	stopAmbientSoundCues();
	_residentSoundEffects.stop();
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

	if (owner == 0 && itemId == 0x5f &&
			_vm->gameState().inventoryItemResourcePageByOwnerAndItemId[owner][itemId] == 0x40) {
		const Common::String name = _textStore.inventoryItemName(114);
		if (!name.empty())
			return name;
	}

	if ((owner == 0 && itemId == 0x2d) || (owner == 1 && itemId == 0x1a)) {
		const byte knifeState = _vm->gameState().multiToolKnifeState;
		// Each open attachment has a localized name after the regular item rows.
		if (knifeState < 10 && (knifeState & 1) != 0) {
			const byte nameRow = (owner == 0 ? 109 : 35) + knifeState / 2;
			const Common::String name = _textStore.inventoryItemName(nameRow);
			if (!name.empty())
				return name;
		}
	}

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
	stopRealtimeSpeech();
	TravelScreen travelScreen(_vm);
	if (!travelScreen.showViewer() || Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_displayPalette.markAllDirty();
	presentFrame();
}

bool PlayableScene::showInventoryMedia(InventoryMediaId mediaId) {
	InventoryMediaPlayer media;
	if (!media.loadStill(mediaId))
		return false;

	Graphics::ManagedSurface savedScene;
	Common::Array<byte> savedPalette;
	uint16 savedViewportX = 0;
	installFullscreenInventoryMedia(media, savedScene, savedPalette, savedViewportX);
	presentFrame();
	waitForFullscreenMediaDismissal();
	restoreFullscreenInventoryMedia(savedScene, savedPalette, savedViewportX);
	return true;
}

bool PlayableScene::playSueTapeRecording() {
	InventoryMediaPlayer media;
	if (!media.loadSueTape())
		return false;

	Graphics::ManagedSurface savedScene;
	Common::Array<byte> savedPalette;
	uint16 savedViewportX = 0;
	installFullscreenInventoryMedia(media, savedScene, savedPalette, savedViewportX);
	presentFrame();

	byte animationFrame = 0;
	uint32 animationMillis = 0;
	for (uint cueIndex = 0; cueIndex < ARRAYSIZE(kSueTapeSpeechCues) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++cueIndex) {
		const SueTapeSpeechCue &cue = kSueTapeSpeechCues[cueIndex];
		if (runSueTapeSpeechLine(media, cue.rowIndex, cue.frameIndex,
				cue.red, cue.green, cue.blue, animationFrame, animationMillis))
			break;
	}

	_speech.stop();
	clearAllSpeechOverlays();
	restoreFullscreenInventoryMedia(savedScene, savedPalette, savedViewportX);
	return true;
}

void PlayableScene::installFullscreenInventoryMedia(const InventoryMediaPlayer &media,
		Graphics::ManagedSurface &savedScene, Common::Array<byte> &savedPalette, uint16 &savedViewportX) {
	savedScene.copyFrom(_sceneFramebuffer);
	savedPalette = _paletteCurrent;
	savedViewportX = _viewportXOffset;

	_vm->gameplayMusic()->stop();
	stopRealtimeSpeech();
	_speech.stop();
	_soundBank0.stop();
	stopAmbientSoundCues();
	_residentSoundEffects.stop();
	clearAllSpeechOverlays();
	_skipRequested = false;
	_stepAdvanceRequested = false;

	_sceneFramebuffer.copyRectToSurface(media.framebuffer(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	_paletteCurrent = media.palette();
	_viewportXOffset = 0;
	_displayPalette.markAllDirty();
}

void PlayableScene::restoreFullscreenInventoryMedia(const Graphics::ManagedSurface &savedScene,
		const Common::Array<byte> &savedPalette, uint16 savedViewportX) {
	_sceneFramebuffer.copyRectToSurface(savedScene.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	_paletteCurrent = savedPalette;
	_viewportXOffset = savedViewportX;
	_skipRequested = false;
	_stepAdvanceRequested = false;
	_displayPalette.markAllDirty();
	presentFrame();
}

bool PlayableScene::waitForFullscreenMediaDismissal() {
	bool dismissed = false;
	while (!dismissed && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollFullscreenMediaEvents(dismissed))
			break;
		if (!dismissed)
			g_system->delayMillis(10);
	}
	return dismissed;
}

bool PlayableScene::pollFullscreenMediaEvents(bool &dismissed) {
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
			presentFrame();
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				dismissed = true;
			} else if (!event.kbdRepeat &&
					(event.kbd.keycode == Common::KEYCODE_RETURN ||
					 event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					 event.kbd.keycode == Common::KEYCODE_SPACE)) {
				_stepAdvanceRequested = true;
				dismissed = true;
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			dismissed = true;
			break;
		default:
			break;
		}
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

bool PlayableScene::runSueTapeSpeechLine(InventoryMediaPlayer &media, uint16 rowIndex, byte frameIndex,
		byte red, byte green, byte blue, byte &animationFrame, uint32 &animationMillis) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return false;

	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit(); ++part) {
		const Common::String text = getResource003LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		setPaletteEntry6Bit(kDefaultPrimarySpeechTextColor, red, green, blue);
		_primarySpeechOverlay.visible = true;
		_primarySpeechOverlay.colorIndex = kDefaultPrimarySpeechTextColor;
		wrapActorSpeechText(text, 0x140, _primarySpeechOverlay.lines);
		calculateSpeechOverlayBounds(_primarySpeechOverlay, 0x140, 0x64, true, _activeActorWorldY);

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _primarySpeechOverlay.lines.size() * 1100);
		const uint32 startMillis = g_system->getMillis();
		uint32 lastMillis = startMillis;
		bool dismissed = false;
		presentFrame();

		while (!dismissed && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
			const uint32 elapsed = g_system->getMillis() - startMillis;
			if (!_speech.isPlaying() && elapsed >= duration)
				break;
			if (pollFullscreenMediaEvents(dismissed))
				break;

			g_system->delayMillis(10);
			const uint32 now = g_system->getMillis();
			animationMillis += MIN<uint32>(now - lastMillis, kBlockingSceneMaxFrameDeltaMillis);
			lastMillis = now;
			bool animationChanged = false;
			while (animationMillis >= kSueTapeFrameMillis) {
				animationMillis -= kSueTapeFrameMillis;
				media.drawSueTapeFrame(animationFrame);
				animationFrame = (byte)((animationFrame + 1) % InventoryMediaPlayer::kSueTapeFrameCount);
				animationChanged = true;
			}
			if (animationChanged) {
				_sceneFramebuffer.copyRectToSurface(media.framebuffer(), 0, 0,
					Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
				presentFrame();
			}
		}

		_speech.stop();
		_primarySpeechOverlay.visible = false;
		_primarySpeechOverlay.lines.clear();
		if (dismissed && consumeStepAdvanceRequest()) {
			dismissed = false;
			continue;
		}
		if (dismissed || Engine::shouldQuit() || _vm->isSceneRestartRequested())
			return true;
	}

	return false;
}

void PlayableScene::startConcurrentActorPath(int targetX, int targetY, byte finalFacing,
		byte finalCel, uint32 frameMillis, bool snapToTargetIfNoPath) {
	_playerDirectedActorPathActive = false;
	_playerDirectedActorPathFootstepPlayed = false;
	_pendingGameplayActionHandlerId = 0;
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY,
		targetX, targetY, finalFacing, finalCel);
	_concurrentActorPathFrameIndex = 1;
	_concurrentActorPathChannel.reset(0, frameMillis);
	_lastViewportScrollActorWorldX = _activeActorWorldX;
	_concurrentActorPathActive = _actorPathFrames.size() > 1;
	_actorPathPlaybackActive = _concurrentActorPathActive;
	if (!_concurrentActorPathActive && !_actorPathFrames.empty()) {
		const ActorPathFrame &frame = _actorPathFrames.back();
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	} else if (!_concurrentActorPathActive && snapToTargetIfNoPath) {
		setActiveActorPose(targetX, targetY, finalFacing, finalCel);
	}
}

bool PlayableScene::startPlayerDirectedActorPath(int targetX, int targetY,
		byte finalFacing, byte finalCel, uint16 actionHandlerId) {
	startConcurrentActorPath(targetX, targetY, finalFacing, finalCel,
		kActorPathFrameMillis);
	if (!_concurrentActorPathActive)
		return false;

	_playerDirectedActorPathActive = true;
	_pendingGameplayActionHandlerId = actionHandlerId;
	return true;
}

void PlayableScene::advanceConcurrentActorPath(uint32 delta) {
	if (!_concurrentActorPathActive)
		return;

	const uint frameCount = _concurrentActorPathChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount &&
			_concurrentActorPathFrameIndex < _actorPathFrames.size(); ++i) {
		const ActorPathFrame &frame = _actorPathFrames[_concurrentActorPathFrameIndex++];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		if (_playerDirectedActorPathActive) {
			const bool terminalFrame = _concurrentActorPathFrameIndex >= _actorPathFrames.size();
			handleActorPathFootstep(terminalFrame, _playerDirectedActorPathFootstepPlayed);
		}
	}
	if (_concurrentActorPathFrameIndex >= _actorPathFrames.size()) {
		const uint16 actionHandlerId = _playerDirectedActorPathActive ?
			_pendingGameplayActionHandlerId : 0;
		cancelConcurrentActorPath();
		if (actionHandlerId != 0)
			runGameplayAction(actionHandlerId);
	}
}

void PlayableScene::finishConcurrentActorPath(bool allowSkip) {
	bool interrupted = false;
	while (_concurrentActorPathActive && !animationPlaybackShouldStop()) {
		if (waitSceneMillis(10, allowSkip)) {
			interrupted = true;
			break;
		}
	}

	if (_concurrentActorPathActive && !_actorPathFrames.empty()) {
		const ActorPathFrame &frame = _actorPathFrames.back();
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	}
	cancelConcurrentActorPath();
	if (interrupted && !animationPlaybackShouldStop())
		consumeStepAdvanceRequest();
}

void PlayableScene::cancelConcurrentActorPath() {
	_concurrentActorPathChannel.reset(0, 0);
	_concurrentActorPathFrameIndex = 0;
	_concurrentActorPathActive = false;
	_actorPathPlaybackActive = false;
	_playerDirectedActorPathActive = false;
	_playerDirectedActorPathFootstepPlayed = false;
	_pendingGameplayActionHandlerId = 0;
}

bool PlayableScene::walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel, bool cancelOnSkip) {
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY, targetX, targetY,
		finalFacing, finalCel);

	if (_actorPathFrames.size() <= 1) {
		drawPlayableComposite();
		presentFrame();
		return true;
	}

	_lastViewportScrollActorWorldX = _activeActorWorldX;
	_actorPathPlaybackActive = true;
	bool footstepPlayed = false;
	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		handleActorPathFootstep(frameIndex + 1 == _actorPathFrames.size(), footstepPlayed);
		if (waitSceneMillis(kActorPathFrameMillis)) {
			_actorPathPlaybackActive = false;
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				return false;

			if (cancelOnSkip) {
				consumeStepAdvanceRequest();
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
			_lastViewportScrollActorWorldX = _activeActorWorldX;
			consumeStepAdvanceRequest();
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

bool PlayableScene::playResidentSoundEffect(byte soundEffectId, byte volumePercent) {
	return _residentSoundEffects.playSample(soundEffectId, volumePercent);
}

bool PlayableScene::playActiveActorFootstep() {
	if (_activeActorWorldX < 0 || _activeActorWorldX >= HollywoodEngine::kSceneBufferWidth ||
			_activeActorWorldY < 0 || _activeActorWorldY >= HollywoodEngine::kSceneBufferHeight ||
			_paletteMask.size() < kSceneColorToFootstepSoundMap + kScenePaletteMapPageSize)
		return false;

	const uint framebufferOffset = (uint)_activeActorWorldY * HollywoodEngine::kSceneBufferWidth +
		(uint)_activeActorWorldX;
	const byte floorColor = savedFramebufferPixelAt(framebufferOffset);
	const byte soundEffectId = _paletteMask[kSceneColorToFootstepSoundMap + floorColor];
	return playResidentSoundEffect(soundEffectId);
}

void PlayableScene::handleActorPathFootstep(bool terminalFrame, bool &footstepPlayed) {
	if (_activeActorCel == 4 || _activeActorCel == 10) {
		playActiveActorFootstep();
		footstepPlayed = true;
	}

	if (terminalFrame) {
		if (!footstepPlayed)
			playActiveActorFootstep();
		footstepPlayed = false;
	}
}

bool PlayableScene::adjustWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (adjustCustomWalkTargetToFloorMask(targetX, targetY))
		return walkableMaskAt(targetX, targetY) != 0;

	if (targetX > 0x30f)
		targetX = 0x30f;

	while (targetY < 0x1df) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		++targetY;
	}

	while (targetY > 0) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		--targetY;
	}

	return walkableMaskAt(targetX, targetY) != 0;
}

void PlayableScene::queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
		byte finalFacing, byte finalCel) {
	prepareCustomActorPathRoute(startX, startY);
	const byte startRegion = paletteRegionAt(startX, startY);
	const byte targetRegion = paletteRegionAt(targetX, targetY);
	debugC(1, kDebugPath,
		"%s queue path: start=(%d,%d) startRegion=%u initialDrawOrder=%u facing=%u cel=%u target=(%d,%d) targetRegion=%u finalFacing=%u finalCel=%u",
		sceneDebugName(), startX, startY, startRegion, _activeActorDrawOrderMode, _activeActorFacing,
		_activeActorCel, targetX, targetY, targetRegion, finalFacing, finalCel);
	_pathController.queueWithPaletteRegionRouting(*this, startX, startY, targetX, targetY,
		_activeActorDrawOrderMode, _activeActorFacing, _activeActorCel, finalFacing, finalCel,
		kInvalidFacing, kInvalidCel, actorPathStepDeltaTable(), actorPathStepDeltaTableSize());
	if (_actorPathFrames.empty()) {
		debugC(1, kDebugPath, "%s queued empty path", sceneDebugName());
		return;
	}

	const ActorPathFrame &firstFrame = _actorPathFrames.front();
	const ActorPathFrame &lastFrame = _actorPathFrames.back();
	debugC(1, kDebugPath,
		"%s queued path frames=%u first=(%d,%d f=%u c=%u d=%u) last=(%d,%d f=%u c=%u d=%u)",
		sceneDebugName(), _actorPathFrames.size(), firstFrame.worldX, firstFrame.worldY,
		firstFrame.facing, firstFrame.cel, firstFrame.drawOrderMode, lastFrame.worldX,
		lastFrame.worldY, lastFrame.facing, lastFrame.cel, lastFrame.drawOrderMode);
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

byte PlayableScene::walkableMaskAt(int x, int y) const {
	if (x < 0 || y < 0 || x >= HollywoodEngine::kSceneBufferWidth || y >= HollywoodEngine::kSceneBufferHeight ||
			_walkablePaletteMask.empty())
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (!isFramebufferOffsetValid(offset))
		return 0;

	const byte pixel = savedFramebufferPixelAt(offset);
	return pixel < _walkablePaletteMask.size() ? _walkablePaletteMask[pixel] : 0;
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

bool PlayableScene::unlockTravelDestination(byte destinationId) {
	GameplayState &state = _vm->gameState();
	if (!state.unlockTravelScreenDestination(destinationId))
		return false;

	_soundBank0.playSample(1, 100);

	byte slotIndex = GameplayState::kTravelScreenDisabledSlot;
	for (byte slot = 0; slot < GameplayState::kTravelScreenSlotCount; ++slot) {
		if (state.travelScreenSlotIds[slot] == destinationId) {
			slotIndex = slot;
			break;
		}
	}

	if (slotIndex != GameplayState::kTravelScreenDisabledSlot) {
		TravelScreen travelScreen(_vm);
		travelScreen.showUnlockTransition(slotIndex);
		if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
			invalidatePresentationPalette();
			drawPlayableComposite();
			presentFrame();
		}
	}

	return true;
}

void PlayableScene::requestTravelScreenSelection(byte currentChapterId) {
	syncActiveActorPoseToGameState();
	_vm->gameState().requestTravelScreenSelection(currentChapterId);
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

void PlayableScene::runActorReplacement(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis));
}

void PlayableScene::runActorReplacement(const ActionOverlaySpec &spec) {
	const SceneAnimationStratum stratum = spec.hasDrawStratum ?
		spec.drawStratum : kSceneAnimationActorReplacement;
	runActionOverlay(spec, stratum, true);
}

void PlayableScene::runSceneOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis) {
	runSceneOverlay(ActionOverlaySpec(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis));
}

void PlayableScene::runSceneOverlay(const ActionOverlaySpec &spec) {
	const SceneAnimationStratum stratum = spec.hasDrawStratum ?
		spec.drawStratum : kSceneAnimationInFrontOfActors;
	runActionOverlay(spec, stratum, false);
}

void PlayableScene::runActionOverlay(const ActionOverlaySpec &spec,
		SceneAnimationStratum stratum, bool hideActiveActor) {
	const ActionOverlayOptions &options = spec.options;
	const bool previousHideActiveActor = _actionOverlayPlayer.begin(spec.chunkIndex,
		spec.descriptorCount, spec.frameMap, spec.frameMapSize, stratum,
		hideActiveActor, spec.restoreBackgroundBeforeDraw);

	const uint playbackFrameCount = spec.playbackFrameCount();
	const uint firstFrame = MIN<uint>(options.firstFrame, playbackFrameCount);
	const uint requestedEndFrame = options.endFrame == 0 ? playbackFrameCount : options.endFrame;
	const uint cappedEndFrame = MIN<uint>(requestedEndFrame, playbackFrameCount);
	for (uint frame = firstFrame; frame < cappedEndFrame && !Engine::shouldQuit(); ++frame) {
		_actionOverlayPlayer.setFrame(spec.targetFrame(frame));
		const Common::Array<AnimationFrameEvent> &events = spec._events;
		for (uint eventIndex = 0; eventIndex < events.size(); ++eventIndex) {
			if (events[eventIndex].matches(frame))
				handleAnimationFrameEvent(events[eventIndex], frame);
		}
		const bool terminalFrame = frame + 1 >= cappedEndFrame;
		if (terminalFrame && !options.waitAfterFinalFrame) {
			drawPlayableComposite();
			presentFrame();
		} else if (waitSceneMillis(spec.frameMillis, options.allowSkip)) {
			if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested() &&
				consumeStepAdvanceRequest()) {
				for (++frame; frame < cappedEndFrame; ++frame) {
					_actionOverlayPlayer.setFrame(spec.targetFrame(frame));
					for (uint eventIndex = 0; eventIndex < events.size(); ++eventIndex) {
						if (events[eventIndex].matches(frame))
							handleAnimationFrameEvent(events[eventIndex], frame);
					}
				}
			}
			break;
		}
	}
	_actionOverlayPlayer.finish(previousHideActiveActor);
}

byte PlayableScene::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0;
}

byte PlayableScene::primarySpeechAnimationInitialFrame(byte animationGroup, byte baseFrame) const {
	(void)animationGroup;
	return baseFrame;
}

byte PlayableScene::primarySpeechAnimationFrameCount(byte animationGroup) const {
	(void)animationGroup;
	return 5;
}

uint32 PlayableScene::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kPrimaryDialogueSpeechFrameMillis;
}

byte PlayableScene::primarySpeechVolumePercent(byte animationGroup) const {
	(void)animationGroup;
	return 100;
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

bool PlayableScene::waitSceneMillis(uint32 millis, bool allowSkip) {
	if (millis == 0)
		return Engine::shouldQuit() || _vm->isSceneRestartRequested();

	// Presentation may block, so blocking animation must use elapsed wall time.
	const uint32 startMillis = g_system->getMillis();
	uint32 lastMillis = startMillis;
	bool framePresented = false;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(allowSkip))
			return true;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		advanceGameplayLoop(MIN<uint32>(delta, kBlockingSceneMaxFrameDeltaMillis));

		if (framePresented && now - startMillis >= millis)
			break;

		drawPlayableComposite();
		presentFrame();
		framePresented = true;

		const uint32 elapsed = g_system->getMillis() - startMillis;
		if (elapsed < millis)
			g_system->delayMillis(MIN<uint32>(millis - elapsed, 10));
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

bool PlayableScene::animationPlaybackShouldStop() const {
	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void PlayableScene::presentAnimationFrame() {
	drawPlayableComposite();
	presentFrame();
}

bool PlayableScene::waitForAnimationFrame(uint32 millis, bool allowSkip) {
	return waitSceneMillis(millis, allowSkip);
}

bool PlayableScene::consumeAnimationFastForwardRequest() {
	return consumeStepAdvanceRequest();
}

void PlayableScene::resetViewportFromScene() {
	const uint16 maximumFramebufferOffset =
		HollywoodEngine::kSceneBufferWidth - HollywoodEngine::kScreenWidth;

	_viewportMinXOffset = MIN<uint16>(sceneViewportMinXOffset(), maximumFramebufferOffset);
	_viewportMaxXOffset = MIN<uint16>(sceneViewportMaxXOffset(), maximumFramebufferOffset);
	if (_viewportMaxXOffset < _viewportMinXOffset)
		_viewportMaxXOffset = _viewportMinXOffset;

	_viewportXOffset = CLIP<uint16>(sceneViewportXOffset(), _viewportMinXOffset, _viewportMaxXOffset);
	_lastViewportScrollActorWorldX = _activeActorWorldX;
}

void PlayableScene::centerViewportOnActor(int actorWorldX) {
	const int centeredOffset = actorWorldX - HollywoodEngine::kScreenWidth / 2;
	if (centeredOffset <= _viewportMinXOffset) {
		_viewportXOffset = _viewportMinXOffset;
	} else if (centeredOffset >= _viewportMaxXOffset) {
		_viewportXOffset = _viewportMaxXOffset;
	} else {
		_viewportXOffset = (uint16)CLIP<int>((centeredOffset + 2) & ~3,
			_viewportMinXOffset, _viewportMaxXOffset);
	}
}

void PlayableScene::advanceViewportScroll(uint32 delta) {
	(void)delta;
	if (_viewportMinXOffset >= _viewportMaxXOffset) {
		_lastViewportScrollActorWorldX = _activeActorWorldX;
		return;
	}

	const int actorScreenX = _activeActorWorldX - _viewportXOffset;
	if ((!_actorPathPlaybackActive || _lastViewportScrollActorWorldX < _activeActorWorldX) &&
			actorScreenX > kViewportScrollRightThreshold && _viewportXOffset < _viewportMaxXOffset) {
		_viewportXOffset = MIN<uint16>(_viewportXOffset + kViewportScrollStep, _viewportMaxXOffset);
	} else if ((!_actorPathPlaybackActive || _activeActorWorldX < _lastViewportScrollActorWorldX) &&
			actorScreenX < kViewportScrollLeftThreshold && _viewportMinXOffset < _viewportXOffset) {
		if (_viewportXOffset > _viewportMinXOffset + kViewportScrollStep)
			_viewportXOffset -= kViewportScrollStep;
		else
			_viewportXOffset = _viewportMinXOffset;
	}
	_lastViewportScrollActorWorldX = _activeActorWorldX;
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

void PlayableScene::ensureAmbientSoundCuePlaying(byte slotIndex, uint16 cueId, byte volumePercent) {
	SoundBank0Player *player = nullptr;
	if (slotIndex == 0) {
		player = &_ambientSoundBank0;
	} else if (slotIndex < kAmbientSoundSlotCount) {
		player = &_additionalAmbientSoundBank0Slots[slotIndex - 1];
	}

	if (player != nullptr && !player->isPlaying())
		player->playSample(cueId, volumePercent);
}

void PlayableScene::stopAmbientSoundCues() {
	_ambientSoundBank0.stop();
	for (uint i = 0; i < ARRAYSIZE(_additionalAmbientSoundBank0Slots); ++i)
		_additionalAmbientSoundBank0Slots[i].stop();
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
	byte volumePercent = profile.soundVolumePercent;
	if (profile.soundCueVolumes != nullptr &&
			_currentAmbientSoundCueId >= profile.soundFirstCueId) {
		const uint volumeIndex = _currentAmbientSoundCueId - profile.soundFirstCueId;
		if (volumeIndex < profile.soundCueVolumeCount)
			volumePercent = profile.soundCueVolumes[volumeIndex];
	}
	_ambientSoundBank0.playSample(_currentAmbientSoundCueId, volumePercent);
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
	const uint32 frameMillis = primarySpeechAnimationFrameMillis(_primaryDialogueSpeechGroup);
	if (frameMillis == 0)
		return;
	while (_primaryDialogueSpeechTimerAccumulator >= frameMillis) {
		_primaryDialogueSpeechTimerAccumulator -= frameMillis;
		const byte baseFrame = primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup);
		const byte frameCount = primarySpeechAnimationFrameCount(_primaryDialogueSpeechGroup);
		const byte nextFrame = _speechController.advancePrimaryDialogueSpeechFrame(
			_random, baseFrame, frameCount);
		setPrimarySpeechAnimationFrame(_primaryDialogueSpeechGroup, nextFrame);
	}
}

bool PlayableScene::startRealtimePrimarySpeechLine(uint16 rowIndex, byte frameIndex,
		uint16 centerX, uint16 topY, byte red, byte green, byte blue,
		byte animationGroup, byte speechId, byte volumePercent) {
	stopRealtimeSpeech();
	if (!getStage003Cue(rowIndex, frameIndex, _realtimeSpeechTextRecordId,
			_realtimeSpeechContinuationCount, _realtimeSpeechVoiceSampleId)) {
		return false;
	}

	_realtimeSpeechContinuationPart = 0;
	_realtimeSpeechContinuationCount = MAX<byte>(1, _realtimeSpeechContinuationCount);
	_realtimeSpeechCenterX = centerX;
	_realtimeSpeechTopY = topY;
	_realtimeSpeechAnimationGroup = animationGroup;
	_realtimeSpeechId = speechId;
	_realtimeSpeechVolumePercent = volumePercent == 0xff ?
		primarySpeechVolumePercent(animationGroup) : volumePercent;
	_realtimeSpeechPrimary = true;
	_realtimeSpeechActive = true;
	setPaletteEntry6Bit(kDefaultPrimarySpeechTextColor, red, green, blue);
	_primarySpeechOverlay.colorIndex = kDefaultPrimarySpeechTextColor;
	if (animationGroup != kInvalidPrimarySpeechAnimationGroup) {
		const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
		const byte initialFrame = primarySpeechAnimationInitialFrame(animationGroup, baseFrame);
		_speechController.startPrimaryDialogueSpeech(animationGroup, initialFrame);
		primarySpeechAnimationStarted(animationGroup, baseFrame);
		setPrimarySpeechAnimationFrame(animationGroup, initialFrame);
	}
	startRealtimeSpeechPart();
	return _realtimeSpeechActive;
}

bool PlayableScene::startRealtimeSecondarySpeechLine(uint16 rowIndex, byte frameIndex,
		byte speechId) {
	stopRealtimeSpeech();
	if (!getStage003Cue(rowIndex, frameIndex, _realtimeSpeechTextRecordId,
			_realtimeSpeechContinuationCount, _realtimeSpeechVoiceSampleId)) {
		return false;
	}

	_realtimeSpeechContinuationPart = 0;
	_realtimeSpeechContinuationCount = MAX<byte>(1, _realtimeSpeechContinuationCount);
	_realtimeSpeechCenterX = _activeActorWorldX;
	_realtimeSpeechTopY = 0;
	_realtimeSpeechAnimationGroup = kInvalidPrimarySpeechAnimationGroup;
	_realtimeSpeechId = speechId;
	_realtimeSpeechVolumePercent = 100;
	_realtimeSpeechPrimary = false;
	_realtimeSpeechActive = true;
	_speechOverlay.colorIndex = kDefaultSecondarySpeechTextColor;
	_speechController.prepareSecondaryActorSpeech();
	startRealtimeSpeechPart();
	return _realtimeSpeechActive;
}

void PlayableScene::startRealtimeSpeechPart() {
	while (_realtimeSpeechContinuationPart < _realtimeSpeechContinuationCount) {
		const Common::String text = getResource003LargeTextRecord(
			_realtimeSpeechTextRecordId + _realtimeSpeechContinuationPart);
		if (text.empty()) {
			++_realtimeSpeechContinuationPart;
			continue;
		}

		SpeechOverlay &overlay = _realtimeSpeechPrimary ? _primarySpeechOverlay : _speechOverlay;
		overlay.visible = true;
		wrapActorSpeechText(text, _realtimeSpeechCenterX, overlay.lines);
		calculateSpeechOverlayBounds(overlay, _realtimeSpeechCenterX, _realtimeSpeechTopY,
			_realtimeSpeechPrimary, _activeActorWorldY);
		const uint16 sampleId = _realtimeSpeechVoiceSampleId == 0 ? 0 :
			_realtimeSpeechVoiceSampleId + _realtimeSpeechContinuationPart;
		const bool started = sampleId != 0 && _realtimeSpeechPlayer.playSample(sampleId,
			_realtimeSpeechVolumePercent);
		_realtimeSpeechElapsed = 0;
		_realtimeSpeechDuration = started ?
			MAX<uint32>(_realtimeSpeechPlayer.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, overlay.lines.size() * 1100);
		return;
	}

	finishRealtimeSpeech(true);
}

void PlayableScene::advanceRealtimeSpeech(uint32 delta) {
	if (!_realtimeSpeechActive)
		return;

	_realtimeSpeechElapsed += delta;
	if (_realtimeSpeechPlayer.isPlaying() ||
			_realtimeSpeechElapsed < _realtimeSpeechDuration) {
		return;
	}

	++_realtimeSpeechContinuationPart;
	startRealtimeSpeechPart();
}

bool PlayableScene::waitForRealtimeSpeech(bool allowSkip) {
	while (isRealtimeSpeechActive() && !animationPlaybackShouldStop()) {
		if (waitSceneMillis(50, allowSkip)) {
			if (consumeStepAdvanceRequest()) {
				_realtimeSpeechPlayer.stop();
				++_realtimeSpeechContinuationPart;
				startRealtimeSpeechPart();
				continue;
			}
			stopRealtimeSpeech();
			return true;
		}
	}

	if (isRealtimeSpeechActive())
		stopRealtimeSpeech();
	return animationPlaybackShouldStop();
}

void PlayableScene::stopRealtimeSpeech() {
	finishRealtimeSpeech(false);
}

void PlayableScene::finishRealtimeSpeech(bool completed) {
	_realtimeSpeechPlayer.stop();
	if (!_realtimeSpeechActive)
		return;

	const bool primary = _realtimeSpeechPrimary;
	const byte animationGroup = _realtimeSpeechAnimationGroup;
	const byte speechId = _realtimeSpeechId;
	_realtimeSpeechActive = false;
	_realtimeSpeechElapsed = 0;
	_realtimeSpeechDuration = 0;
	_realtimeSpeechContinuationPart = 0;
	_realtimeSpeechContinuationCount = 0;
	_realtimeSpeechId = 0;
	if (primary) {
		_speechController.clearPrimaryOverlay();
		if (animationGroup != kInvalidPrimarySpeechAnimationGroup) {
			const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
			setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
			_speechController.stopPrimaryDialogueSpeech(kInvalidPrimarySpeechAnimationGroup, 7);
			if (completed)
				primarySpeechAnimationRestored(animationGroup, baseFrame);
		}
	} else {
		_speechController.clearSecondaryOverlay();
		_speechController.prepareSecondaryActorSpeech();
	}

	realtimeSpeechEnded(speechId, completed);
}

void PlayableScene::clearSpeechOverlay() {
	_speechController.clearSecondaryOverlay();
}

void PlayableScene::clearAllSpeechOverlays() {
	_speechController.clearAllOverlays();
}

void PlayableScene::drawSpeechOverlay() {
	drawSpeechOverlayText(_speechOverlay, _vm->font(), *_screen.surfacePtr(), viewportXOffset());
	drawSpeechOverlayText(_primarySpeechOverlay, _vm->font(), *_screen.surfacePtr(), viewportXOffset());
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
	if (&overlay == &_primarySpeechOverlay || isRealtimeSecondarySpeechActive())
		stopRealtimeSpeech();

	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit(); ++part) {
		const Common::String text = getResource003LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		overlay.visible = true;
		overlay.colorIndex = colorIndex;
		wrapActorSpeechText(text, centerX, overlay.lines);
		calculateSpeechOverlayBounds(overlay, centerX, topY, useRequestedTop, _activeActorWorldY);

		const byte animationGroup = animatePrimaryDialogue ?
			(primaryAnimationGroup == kInvalidPrimarySpeechAnimationGroup ? 0 : primaryAnimationGroup) :
			kInvalidPrimarySpeechAnimationGroup;
		const byte volumePercent = animationGroup == kInvalidPrimarySpeechAnimationGroup ?
			100 : primarySpeechVolumePercent(animationGroup);
		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, volumePercent);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, overlay.lines.size() * 1100);
		if (animatePrimaryLeft) {
			_speechController.startPrimaryLeftSpeech();
			setPrimaryLeftSpeechFrame(0);
		}
		if (animationGroup != kInvalidPrimarySpeechAnimationGroup) {
			const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
			const byte initialFrame = primarySpeechAnimationInitialFrame(animationGroup, baseFrame);
			_speechController.startPrimaryDialogueSpeech(animationGroup, initialFrame);
			primarySpeechAnimationStarted(animationGroup, baseFrame);
			setPrimarySpeechAnimationFrame(animationGroup, initialFrame);
		}
		const bool interrupted = waitForSpeechOrDelay(duration, animatePrimaryLeft);
		if (animatePrimaryLeft) {
			_speechController.stopPrimaryLeftSpeech();
			setPrimaryLeftSpeechFrame(0);
		}
		bool restoredPrimarySpeechAnimation = false;
		byte restoredPrimarySpeechAnimationGroup = 0;
		byte restoredPrimarySpeechBaseFrame = 0;
		if (_primaryDialogueSpeechActive && !(_realtimeSpeechActive && _realtimeSpeechPrimary)) {
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
	const int viewportX = viewportXOffset();
	int clampedAnchorSceneX = anchorSceneX;
	if (viewportX < clampedAnchorSceneX) {
		if (clampedAnchorSceneX - viewportX > HollywoodEngine::kScreenWidth - 1)
			clampedAnchorSceneX = viewportX + 0x276;
	} else {
		clampedAnchorSceneX = viewportX + 10;
	}

	wrapSpeechOverlayText(text, clampedAnchorSceneX - viewportX, lines);
}

Common::String PlayableScene::getResource003LargeTextRecord(uint16 recordId) const {
	return _textStore.largeTextRecord(recordId);
}

void PlayableScene::calculateSpeechOverlayBounds(SpeechOverlay &overlay, int centerX, int topY, bool useRequestedTop,
		int actorWorldY) {
	const int anchorBottomY = useRequestedTop ? topY : actorWorldY - 0xbe;
	layoutSpeechOverlay(overlay, _vm->font(), centerX, anchorBottomY, viewportXOffset());
}

void PlayableScene::calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY) {
	calculateSpeechOverlayBounds(_speechOverlay, actorWorldX, 0, false, actorWorldY);
}

bool PlayableScene::waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft) {
	const uint32 startMillis = g_system->getMillis();
	while (!Engine::shouldQuit()) {
		const uint32 elapsed = g_system->getMillis() - startMillis;
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= fallbackMillis)
			break;

		const uint32 slice = speechActive ? 50 : MIN<uint32>(50, fallbackMillis - elapsed);
		if (waitSceneMillis(slice)) {
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				return true;
			if (consumeStepAdvanceRequest())
				return false;
			return true;
		}
		if (animatePrimaryLeft && !_primarySpeechOverlay.visible)
			break;
	}

	return Engine::shouldQuit();
}

bool PlayableScene::consumeStepAdvanceRequest() {
	if (!_stepAdvanceRequested)
		return false;

	_stepAdvanceRequested = false;
	_skipRequested = false;
	return true;
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

	const bool originalPaletteApplied = !shouldApplyGameplayPanelObjectPalette() ||
		_panelArt.applyInteractiveObjectPalette(_paletteCurrent);
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
	_panelArt.drawVerbPanel(surface, _sceneFramebuffer.rawSurface(), viewportXOffset(), 0, panelState,
		_vm->font());
}

void PlayableScene::drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawDialogueInventoryPanel(surface, _sceneFramebuffer.rawSurface(), viewportXOffset(), 0,
		panelState, _vm->gameState(), _vm->font());
}

void PlayableScene::presentFrame(const SceneHoverCaption *hoverCaption, const GameplayPanelState *panelState,
		const DialogueMenuState *dialogueMenuState) {
	if (hoverCaption)
		hoverCaption->applyPalette(_paletteCurrent);
	if ((panelState && panelState->visible()) || (dialogueMenuState && dialogueMenuState->visible()))
		applyGameplayPanelPalette();
	const uint16 xOffset = viewportXOffset();
	const bool paletteChanged = _displayPalette.uploadFrom6Bit(_paletteCurrent);
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

	const bool screenChanged = _surfaceState.updatePresentedScreenCache();
	if (screenChanged || paletteChanged)
		g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);
	g_system->updateScreen();
}

bool PlayableScene::pollEvents(bool allowSkip) {
	_stepAdvanceRequested = false;
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
			if (allowSkip && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				_skipRequested = true;
				return true;
			}
			if (allowSkip && !event.kbdRepeat &&
					(event.kbd.keycode == Common::KEYCODE_RETURN ||
					 event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					 event.kbd.keycode == Common::KEYCODE_SPACE)) {
				_skipRequested = true;
				_stepAdvanceRequested = true;
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
