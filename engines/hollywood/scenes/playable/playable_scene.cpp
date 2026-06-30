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

#include <math.h>

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
const uint32 kViewportScrollTickMillis = 10;
const uint16 kViewportScrollRightThreshold = 0x144;
const uint16 kViewportScrollLeftThreshold = 0x13c;
const uint16 kViewportScrollStep = 2;
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

PlayableScene::PlayableScene(HollywoodEngine *vm, const char *randomName, int defaultActorX, int defaultActorY,
		byte defaultActorFacing, byte secondarySpeechTextColor, byte primarySpeechTextColor) :
		_vm(vm),
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
		_secondaryActorFrame(0),
		_lastSceneActionItemId(0),
		_actionOverlayVisible(false),
		_actionOverlayLayer(),
		_actionOverlayChunkIndex(0),
		_actionOverlayDescriptorCount(0),
		_actionOverlayFrameIndex(0),
		_hideActiveActor(false),
		_skipRequested(false) {
	_surfaceState.initialize(kPaletteSize, 0x700, kPaletteMaskUsedBytes, kScenePaletteMapPageSize, kScenePaletteRegionCount);
	_activeActorRunStreams.resize(kActorFacingCount * kActiveActorFacingRunStride);
	_secondaryActorRunStreams.resize(kActorFacingCount * kSecondaryActorFacingRunStride);
	_activeActorDescriptors.resize(kActorFacingCount * kActorCelsPerFacing);
	_secondaryActorDescriptors.resize(kActorFacingCount * kSecondaryActorFramesPerFacing);
	_stage003DecodeKey.resize(kStage003DecodeKeySize);
	_stage003StageBlock.resize(kStage003DescriptorTableSize);
	_staticSpeechCueDescriptors.resize(kSpeechCueDescriptorTableSize);
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

uint16 PlayableScene::sceneViewportMinXOffset() const {
	return sceneViewportXOffset();
}

uint16 PlayableScene::sceneViewportMaxXOffset() const {
	return sceneViewportXOffset();
}

byte PlayableScene::inventoryOwnerIndex() const {
	return 1;
}

void PlayableScene::initializeInventoryOwnerState() {
	_vm->gameState().initializeSueItemResourcePages();
}

uint PlayableScene::resource000ActorBankTableEntry() const {
	return kResource000DefaultActorBankTableEntry;
}

uint PlayableScene::resource000ActorBankSegmentCount() const {
	return kResource000DefaultActorBankSegmentCount;
}

uint PlayableScene::resource000ActorPaletteTableEntry() const {
	return kResource000DefaultActorPaletteTableEntry;
}

uint32 PlayableScene::inventoryActionTableExtraOffset() const {
	return kResource000FixedInventoryVerbTableOffset;
}

uint PlayableScene::resource003InventoryRowsOffsetIndex() const {
	return kDefaultResource003InventoryRowsOffsetIndex;
}

uint32 PlayableScene::speechCueDescriptorTableOffset() const {
	return kDefaultSpeechCueDescriptorTableOffset;
}

const byte *PlayableScene::actorPathStepDeltaTable() const {
	return kActorPathStepDeltaTableSet00;
}

uint PlayableScene::actorPathStepDeltaTableSize() const {
	return ARRAYSIZE(kActorPathStepDeltaTableSet00);
}

byte PlayableScene::walkablePaletteMaxRegion() const {
	return 3;
}

const char *PlayableScene::musicArchiveName() const {
	return kDefaultGameplayMusicArchiveName;
}

const char *PlayableScene::soundBank0ArchiveName() const {
	return kDefaultGameplaySoundBank0ArchiveName;
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
	return true;
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
	return false;
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
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s for %s text", kStage003ArchiveName, sceneDebugName());
		return false;
	}

	if (file.read(_stage003DecodeKey.data(), _stage003DecodeKey.size()) != _stage003DecodeKey.size()) {
		warning("Failed to read %s row decode key", kStage003ArchiveName);
		return false;
	}

	const uint32 speechCueOffset = speechCueDescriptorTableOffset();
	if (speechCueOffset + kSpeechCueDescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s static speech cue table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(speechCueOffset);
	if (file.read(_staticSpeechCueDescriptors.data(), _staticSpeechCueDescriptors.size()) !=
			_staticSpeechCueDescriptors.size()) {
		warning("Failed to read %s static speech cue table", kStage003ArchiveName);
		return false;
	}

	const byte ownerSmallRowCount = file.readByte();
	const uint16 ownerLargeRowCount = file.readUint16LE();
	if (file.err()) {
		warning("Failed to read %s static text row counts", kStage003ArchiveName);
		return false;
	}

	const uint32 ownerRowsOffsetEntry = kStage003DecodeKeySize + resource003InventoryRowsOffsetIndex() * 4;
	if (ownerRowsOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			ownerRowsOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s static text row offset entry is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(ownerRowsOffsetEntry);
	const uint32 ownerRowsOffset = file.readUint32LE();
	const uint32 ownerSmallRowBytes = (uint32)ownerSmallRowCount * kStage003SmallRowSize;
	const uint32 ownerLargeRowBytes = (uint32)ownerLargeRowCount * kStage003LargeRowSize;
	if (ownerRowsOffset == 0 ||
			ownerRowsOffset + ownerSmallRowBytes + ownerLargeRowBytes > (uint32)file.size()) {
		warning("%s static text rows are out of range", kStage003ArchiveName);
		return false;
	}

	_inventoryOwnerSmallRows.resize((uint32)(ownerSmallRowCount + 1) * kStage003SmallRowSize);
	memset(_inventoryOwnerSmallRows.data(), 0, _inventoryOwnerSmallRows.size());
	_inventoryOwnerLargeRows.resize((uint32)(ownerLargeRowCount + 1) * kStage003LargeRowSize);
	memset(_inventoryOwnerLargeRows.data(), 0, _inventoryOwnerLargeRows.size());
	file.seek(ownerRowsOffset);
	if (file.read(_inventoryOwnerSmallRows.data() + kStage003SmallRowSize, ownerSmallRowBytes) != ownerSmallRowBytes) {
		warning("Failed to read %s static small text rows", kStage003ArchiveName);
		return false;
	}
	if (file.read(_inventoryOwnerLargeRows.data() + kStage003LargeRowSize, ownerLargeRowBytes) != ownerLargeRowBytes) {
		warning("Failed to read %s static large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 1; row <= ownerSmallRowCount; ++row) {
		for (uint column = 0; column < kStage003SmallRowSize; ++column)
			_inventoryOwnerSmallRows[row * kStage003SmallRowSize + column] -= _stage003DecodeKey[column];
	}

	for (uint row = 1; row <= ownerLargeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_inventoryOwnerLargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
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
	const uint boundaryBytes = kSceneRouteBoundaryPointCount * 4;
	if (_metadata.size() < kRouteBoundaryPoints + boundaryBytes ||
			_metadata.size() < kRouteBoundarySteps + kSceneRouteStepCount) {
		warning("%s chunk 4 is too short for path route tables", resourceArchiveName());
		return false;
	}

	memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _paletteMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
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

void PlayableScene::initializePreviewState() {
	if (hasCustomPreviewState()) {
		initializeCustomPreviewState();
		return;
	}

	initializeDefaultPreviewState();
}

void PlayableScene::initializeDefaultPreviewState() {
	_primaryLeftSpeechLastFrame = 0;
	_primaryDialogueSpeechLastFrame = 7;
	_actionOverlayVisible = false;
	_actionOverlayLayer.visible = false;
	_actionOverlayChunkIndex = 0;
	_actionOverlayDescriptorCount = 0;
	_actionOverlayFrameIndex = 0;
	_hideActiveActor = false;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = kInvalidPrimarySpeechAnimationGroup;
	resetAmbientAudioState();
	_secondaryActorTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
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
	clearAllSpeechOverlays();
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechGroup = kInvalidPrimarySpeechAnimationGroup;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	_actionOverlayVisible = false;
	_actionOverlayLayer.visible = false;
	_hideActiveActor = false;
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

	const uint offset = (uint)itemId * kStage003SmallRowSize;
	if (offset >= _inventoryOwnerSmallRows.size())
		return Common::String();

	const byte *row = _inventoryOwnerSmallRows.data() + offset;
	uint length = 0;
	while (offset + length < _inventoryOwnerSmallRows.size() &&
			length < kStage003SmallRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
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

void PlayableScene::handleLeftClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	_vm->cursor()->leaveInteractiveMode();
	processSceneActionClick(state);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_skipRequested = false;
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void PlayableScene::handleInventoryItemClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	_vm->cursor()->leaveInteractiveMode();
	dispatchSceneAction(state.inventoryActionHandlerId);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_skipRequested = false;
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void PlayableScene::processSceneActionClick(const GameplayLoopCursorState &state) {
	byte itemId = state.resolvedItem;
	if (state.relationModeActive) {
		processSceneRelationClick(state, itemId);
		return;
	}

	if (itemId == 0) {
		_lastSceneActionItemId = 0;
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
	_lastSceneActionItemId = itemId;

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

void PlayableScene::processSceneRelationClick(const GameplayLoopCursorState &state, byte itemId) {
	if (itemId == 0)
		return;

	const SceneVerbActionRecord actionRecord =
		_hotspots.relationActionRecord(state.primaryInventoryItem, itemId, state.relationMode);
	if (actionRecord.actionHandlerId == 0)
		return;
	_lastSceneActionItemId = itemId;

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

void PlayableScene::dispatchSceneAction(uint16 handlerId) {
	if (dispatchCustomSceneAction(handlerId))
		return;

	if (dispatchGenericSceneAction(handlerId))
		return;

	warning("Unhandled %s action handler %u", sceneDebugName(), handlerId);
}

bool PlayableScene::dispatchGenericSceneAction(uint16 handlerId) {
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
		beginStaticSecondarySpeechLine(0x43, 1);
		beginStaticSecondarySpeechLine(0x24, 0);
		beginStaticSecondarySpeechLine(0x43, 2);
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
		if (hasInventoryItem(0x22)) {
			beginStaticSecondarySpeechLine(0x41, 1);
			return true;
		}
		addInventoryItem(0x22);
		_soundBank0.playSample(1, 100);
		beginStaticSecondarySpeechLine(0x41, 0);
		return true;
	case 67: // Mirar pintauñas multicolor (look at multicolor nail polish).
		beginStaticSecondarySpeechLine(0x42, 0);
		return true;
	case 68: // Usar trapo con carbón + hoja: reveal Frankie's note.
		beginStaticSecondarySpeechLine(0x43, 0);
		removeInventoryItem(0x08);
		removeInventoryItem(0x0f);
		addInventoryItem(0x06);
		_soundBank0.playSample(1, 100);
		beginStaticSecondarySpeechLine(0x43, 1);
		beginStaticSecondarySpeechLine(0x24, 0);
		beginStaticSecondarySpeechLine(0x43, 2);
		return true;
	case 69: // Door/lock condition: no key needed, it is not locked.
		beginStaticSecondarySpeechLine(0x44, 0);
		return true;
	default:
		return false;
	}
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
	_actorPathFrames.clear();
	resetActorPathStepDeltas();

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
			buildActorPathFramesBetweenPoints(state, boundary.x, boundary.y,
				segmentFinalFacing, segmentFinalCel, requestedFacing);
			if (restoredStepDeltas)
				resetActorPathStepDeltas();

			currentRegion = nextRegion;
		}
	}

	int requestedFacing = -1;
	bool restoredStepDeltas = false;
	customizeRouteFinal(currentRegion, targetRegion, state, targetX, targetY,
		requestedFacing, restoredStepDeltas);
	state.drawOrderMode = currentRegion;
	buildActorPathFramesBetweenPoints(state, targetX, targetY, finalFacing, finalCel, requestedFacing);
	if (restoredStepDeltas)
		resetActorPathStepDeltas();

}

void PlayableScene::buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
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

void PlayableScene::appendActorPathFrame(const ActorPathBuildState &state) {
	ActorPathFrame frame;
	frame.drawOrderMode = state.drawOrderMode;
	frame.facing = state.facing;
	frame.cel = state.cel;
	frame.worldX = (int16)CLIP<int>(state.x, -32768, 32767);
	frame.worldY = (int16)CLIP<int>(state.y, -32768, 32767);
	_actorPathFrames.push_back(frame);
}

ScenePoint PlayableScene::nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const {
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

ScenePoint PlayableScene::bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
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

uint PlayableScene::calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const {
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

byte PlayableScene::nextActorPathCel(byte cel) const {
	return cel == 12 ? 1 : (byte)(cel + 1);
}

uint PlayableScene::actorPathStepDelta(byte facing, byte cel) const {
	if (facing >= kActorFacingCount || cel == 0 || cel > 12)
		return 0;

	const uint offset = (uint)facing * 12 + cel - 1;
	if (offset >= _actorPathStepDeltas.size())
		return 0;

	return _actorPathStepDeltas[offset];
}

void PlayableScene::resetActorPathStepDeltas() {
	const byte *table = actorPathStepDeltaTable();
	const uint tableSize = actorPathStepDeltaTableSize();
	_actorPathStepDeltas.resize(tableSize);
	if (table != nullptr && tableSize != 0)
		memcpy(_actorPathStepDeltas.data(), table, tableSize);
}

byte PlayableScene::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
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
	const uint offset = ((uint)stageId * 100 + textRowId) * 5;
	if (offset + 5 > _stage003StageBlock.size())
		return Common::String();

	const uint16 textRecordId = readUint16LE(_stage003StageBlock, offset);
	return getResource003LargeTextRecord(textRecordId);
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
	const bool previousHideActiveActor = _hideActiveActor;
	if (options.actorVisibility == kActionOverlayShowActiveActor)
		_hideActiveActor = false;
	else if (options.actorVisibility == kActionOverlayHideActiveActor)
		_hideActiveActor = true;

	_actionOverlayVisible = true;
	_actionOverlayChunkIndex = (byte)chunkIndex;
	_actionOverlayDescriptorCount = (byte)descriptorCount;
	_actionOverlayLayer.configure(chunkIndex, (uint16)descriptorCount, frameMap, frameMapSize);
	_actionOverlayLayer.visible = true;

	const uint firstFrame = MIN<uint>(options.firstFrame, frameMapSize);
	const uint requestedEndFrame = options.endFrame == 0 ? frameMapSize : options.endFrame;
	const uint cappedEndFrame = MIN<uint>(requestedEndFrame, frameMapSize);
	for (uint frame = firstFrame; frame < cappedEndFrame && !Engine::shouldQuit(); ++frame) {
		_actionOverlayLayer.setFrame((byte)frame);
		_actionOverlayFrameIndex = (byte)_actionOverlayLayer.descriptorIndex();
		if (options.statePatchFrame >= 0 && (int)frame == options.statePatchFrame)
			applySceneStateToHotspotsAndPatches(options.statePatchSelector);
		if (options.soundFrame >= 0 && (int)frame == options.soundFrame)
			_soundBank0.playSample(options.soundId, options.soundVolumePercent);
		if (options.hookId != 0 && (options.hookFrame < 0 || (int)frame == options.hookFrame))
			handleActionOverlayFrameHook(options.hookId, frame);
		if (waitSceneMillis(frameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayLayer.visible = false;
	_actionOverlayFrameIndex = 0;
	_hideActiveActor = previousHideActiveActor;

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

void PlayableScene::advanceSecondaryActorSpeechFrame() {
	byte nextFrame = _secondaryActorFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _secondaryActorFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(kSecondaryActorFramesPerFacing - 1);

	if (nextFrame == _secondaryActorFrame)
		nextFrame = (byte)((_secondaryActorFrame + 1) % kSecondaryActorFramesPerFacing);

	_secondaryActorFrame = nextFrame;
}

void PlayableScene::advancePrimaryLeftSpeechFrame() {
	byte nextFrame = _primaryLeftSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _primaryLeftSpeechLastFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(3);

	if (nextFrame == _primaryLeftSpeechLastFrame)
		nextFrame = (byte)((_primaryLeftSpeechLastFrame + 1) % 4);

	_primaryLeftSpeechLastFrame = nextFrame;
	setPrimaryLeftSpeechFrame(nextFrame);
}

void PlayableScene::advancePrimaryDialogueSpeechFrame(uint32 delta) {
	_primaryDialogueSpeechTimerAccumulator += delta;
	while (_primaryDialogueSpeechTimerAccumulator >= kPrimaryDialogueSpeechFrameMillis) {
		_primaryDialogueSpeechTimerAccumulator -= kPrimaryDialogueSpeechFrameMillis;
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

void PlayableScene::clearSpeechOverlay() {
	_speechOverlay.visible = false;
	_speechOverlay.lines.clear();
}

void PlayableScene::clearAllSpeechOverlays() {
	clearSpeechOverlay();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
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
	_secondaryActorFrame = 0;
	_secondaryActorTimerAccumulator = 0;

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

bool PlayableScene::getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 100) * 5;
	if (offset + 5 > _stage003StageBlock.size())
		return false;

	textRecordId = readUint16LE(_stage003StageBlock, offset);
	continuationCount = _stage003StageBlock[offset + 2];
	voiceSampleId = readUint16LE(_stage003StageBlock, offset + 3);
	return textRecordId != 0;
}

bool PlayableScene::getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > _staticSpeechCueDescriptors.size())
		return false;

	textRecordId = readUint16LE(_staticSpeechCueDescriptors, offset);
	continuationCount = _staticSpeechCueDescriptors[offset + 2];
	voiceSampleId = readUint16LE(_staticSpeechCueDescriptors, offset + 3);
	return textRecordId != 0;
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
	if (recordId < kStage003LargeRowBaseIndex) {
		const uint offset = (uint)recordId * kStage003LargeRowSize;
		if (recordId == 0 || offset >= _inventoryOwnerLargeRows.size())
			return Common::String();

		const byte *row = _inventoryOwnerLargeRows.data() + offset;
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
