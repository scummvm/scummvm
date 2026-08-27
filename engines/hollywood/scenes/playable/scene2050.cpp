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

#include "hollywood/scenes/playable/scene2050.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene2050LastState = 0x0803;
const uint16 kScene2040EntryFromInteriorState = 0x07f9;
const uint16 kScene2060EntryState = 0x080e;
const uint16 kScene2070SealSolvedEntryState = 0x0816;
const uint16 kScene2050ViewportXOffset = 0x0028;
const uint kScene2050ActorBankTableEntry = 0x0000;
const uint kScene2050ActorPaletteTableEntry = 0x00cc;
const uint kScene2050Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2050AmbientFrameMillis = 75;
const uint32 kScene2050OverlayFrameMillis = 75;
const uint kScene2050AmbientDescriptorCount = 0x1a;
const uint kScene2050MuralClipDescriptorCount = 8;
const uint kScene2050SealDiscoveryDescriptorCount = 0x33;
const uint kScene2050LabyrinthWalkDescriptorCount = 0x1f;
const uint kScene2050MuralPaletteChunk = 5;
const uint kScene2050MuralHitMaskChunk = 6;
const byte kScene2050LampInventoryItem = 0x3c;
const byte kScene2050LosaSmallRow = 6;
const byte kScene2050SelloSmallRow = 7;
const uint kScene2050LabyrinthGoToGlobalRecord = 0x29;
const uint kScene2050MuralSolvedTilePaletteMapChunk = 7;
const uint kScene2050MuralTileCount = 48;
const uint kScene2050MuralGridColumns = 6;
const int kScene2050MuralVisibleGridX = 0x129;
const int kScene2050MuralSourceGridX = 0x2b9;
const int kScene2050MuralGridY = 0x21;
const int kScene2050MuralTileSize = 0x32;
const int kScene2050MuralTileStep = 0x34;
const byte kScene2050MuralHighlightColor = 0xf0;
const byte kScene2050MuralLastHighlightColor = 0xf4;
const byte kScene2050MuralTileSelectSound = 0x25;
const byte kScene2050MuralTileLockedSound = 0x26;
const byte kScene2050MuralTileSwapSound = 0x27;
const byte kScene2050MuralTileImprovedSound = 0x28;
const byte kScene2050SealDiscoveryLoopSound = 0x24;
const byte kScene2050SealDiscoveryEndSound = 0x2f;
const byte kScene2050SealDiscoveryHook = 1;
const byte kScene2050LabyrinthFootstepHook = 2;
const uint kScene2050SealDiscoveryTurnFrame = 12;
const int kScene2050SealDiscoveryActorX = 0x189;
const int kScene2050SealDiscoveryActorY = 0x11d;
const int kScene2050WalkMinX = 0x0c2;
const int kScene2050WalkMaxX = 0x29a;
const int kScene2050WalkMaxY = 0x1df;

const byte kScene2050AmbientFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25
};

const byte kScene2050MuralClipForwardFrameMap[] = {
	1, 2, 3, 4, 5, 6, 7
};

const byte kScene2050MuralClipBackwardFrameMap[] = {
	6, 5, 4, 3, 2, 1, 0
};

const byte kScene2050SealDiscoveryFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50
};

const byte kScene2050LabyrinthWalkFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30
};

static_assert(ARRAYSIZE(kScene2050AmbientFrameMap) == 0x1a, "Scene 2050 ambient frame map size changed");
static_assert(ARRAYSIZE(kScene2050SealDiscoveryFrameMap) == 53, "Scene 2050 seal discovery frame map size changed");
static_assert(ARRAYSIZE(kScene2050LabyrinthWalkFrameMap) == 47, "Scene 2050 labyrinth walk frame map size changed");

class Scene2050DeltaFrameTarget {
public:
	explicit Scene2050DeltaFrameTarget(byte &frame) : _frame(frame) {}

	void setFrame(byte frame) { _frame = frame; }

private:
	byte &_frame;
};

static PlayableSceneConfig scene2050Config() {
	PlayableSceneConfig config(2050,
		SceneResourceLayout(15, 5, 14),
		SceneViewport(kScene2050ViewportXOffset, kScene2050ViewportXOffset, kScene2050ViewportXOffset),
		SceneActorPose(0x11a, 0x17c, 4));
	config.setActorResources(kScene2050ActorBankTableEntry, kScene2050ActorPaletteTableEntry);
	config.setTextResources(kScene2050Resource003RowsOffsetIndex, kScene2050SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	return config;
}

Scene2050::Scene2050(HollywoodEngine *vm) :
		PlayableScene(vm, scene2050Config()),
		_ambientChannel(),
		_ambientLayer(),
		_muralPermutationInitialized(false),
		_muralPermutationChunkIndex(0),
		_muralSelectedTile(0),
		_muralHitMask(),
		_muralPaletteAccumulator(0),
		_sealDiscoveryActive(false),
		_sealDiscoveryFrame(0),
		_sealDiscoveryActorPathFrameIndex(0),
		_sealDiscoverySpeechStarted(false),
		_sealDiscoverySpeechStartMillis(0),
		_sealDiscoverySpeechDurationMillis(0) {
	_ambientLayer.configure(10, kScene2050AmbientDescriptorCount,
		kScene2050AmbientFrameMap, ARRAYSIZE(kScene2050AmbientFrameMap));
}

void Scene2050::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAmbientLayer();
	applySceneStateToHotspotsAndPatches(0xff);

	if (_vm->gameState().mainFlowStateId == kScene2050LastState) {
		_activeActorWorldX = 0x24c;
		_activeActorWorldY = 0x108;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x11a;
		_activeActorWorldY = 0x17c;
		_activeActorFacing = 4;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene2050::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_ambientLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_sealDiscoveryActive)
		drawSealDiscoveryDeltaLayer();
}

void Scene2050::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2050LastState)
		runEntryFromLabyrinthReturn();
	else
		runEntryFromSphinxInterior();
}

bool Scene2050::prepareCustomGameplayLoop() {
	resetAmbientLayer();
	return true;
}

bool Scene2050::advanceCustomGameplayLoop(uint32 delta) {
	if (_vm->gameState().scene2050MuralPuzzleState == 2)
		runSealDiscoveryIfPending();
	advanceAmbientLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2050::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a escalera (go to stairs): return to the sphinx scene.
		_vm->gameState().mainFlowStateId = kScene2040EntryFromInteriorState;
		return true;
	case 302: // Mirar escalera (look at stairs).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar estatua (look at statue).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar mural (look at mural): open the puzzle without the papyrus guide.
		runMuralSubscreenAction();
		return true;
	case 305: // Coger antorcha (take torch).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 306: // Mirar antorcha (look at torch).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Ir a laberinto (go to labyrinth): requires a lit lamp.
		runGoToLabyrinth();
		return true;
	case 308: // Mirar laberinto (look at labyrinth): describe the dark passage.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 309: // Mirar losa (look at slab): identify it as a seal.
		runRevealFloorSeal();
		return true;
	case 310: // Usar papiro con mural (use papyrus on mural): open the guided puzzle.
		runGuidedMuralAction();
		return true;
	default:
		return false;
	}
}

bool Scene2050::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, kScene2050WalkMinX, kScene2050WalkMaxX);

	while (targetY < kScene2050WalkMaxY) {
		++targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}

	while (targetY > 0) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		--targetY;
	}

	return true;
}

bool Scene2050::customizeRouteSegment(byte currentRegion, byte nextRegion,
		const ActorPathBuildState &state, const ScenePoint &boundary,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	if (currentRegion != 3 || nextRegion != 1)
		return false;

	for (uint i = 0; i < 0x0c && 0x0c + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[0x0c + i] = kActorPathStepDeltaTableSetB4[0x24 + i];
	requestedFacing = 1;
	restoredStepDeltas = true;
	return true;
}

bool Scene2050::customizeRouteFinal(byte currentRegion, byte targetRegion,
		const ActorPathBuildState &state, int targetX, int targetY,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)targetX;
	(void)targetY;
	if (currentRegion != 3 || targetRegion != 3)
		return false;

	for (uint i = 0; i < 0x0c && 0x30 + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[0x30 + i] = kActorPathStepDeltaTableSetB4[i];
	requestedFacing = 4;
	restoredStepDeltas = true;
	return true;
}

bool Scene2050::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if ((selector == 2 || selector == 0xff) && state.scene2050SealRevealed)
		copyStageSmallRow(kScene2050LosaSmallRow, kScene2050SelloSmallRow);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	// Reloading the table restores resource movement modes too.
	if (state.scene2050LabyrinthLampReady)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene2050LabyrinthGoToGlobalRecord, 3);

	return true;
}

AmbientAudioProfile Scene2050::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2050::resetAmbientLayer() {
	_ambientChannel.reset(0, kScene2050AmbientFrameMillis);
	_ambientLayer.visible = true;
	_ambientLayer.reset(0);
}

void Scene2050::advanceAmbientLayer(uint32 delta) {
	const uint frameCount = _ambientChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_ambientChannel.frameIndex = (_ambientChannel.frameIndex + 1) % ARRAYSIZE(kScene2050AmbientFrameMap);
		_ambientLayer.setFrame(_ambientChannel.frameIndex);
	}
}

void Scene2050::runEntryFromSphinxInterior() {
	_vm->gameplayMusic()->playMusicCue(0x0e, 100);
	runEntryPath(0x4b, 0x15a, 2, 0x11a, 0x17c);

	GameplayState &state = _vm->gameState();
	if (state.scene2050EntrySpeechState == 0) {
		beginSecondarySpeechLine(0, 0);
		state.scene2050EntrySpeechState = 1;
	}
}

void Scene2050::runEntryFromLabyrinthReturn() {
	setActiveActorPose(0x2e4, 0x0ea, 4);
	drawPlayableComposite();
	presentFrame();
	beginSecondarySpeechLine(11, 0);
	runEntryPath(0x2e4, 0x0ea, 4, 0x24c, 0x108);

	GameplayState &state = _vm->gameState();
	if (state.scene2050EntrySpeechState < 2 && state.egyptSealPuzzleProgress == 2) {
		beginSecondarySpeechLine(10, 0);
		state.scene2050EntrySpeechState = 2;
	}
}

void Scene2050::runGoToLabyrinth() {
	GameplayState &state = _vm->gameState();

	if (state.scene2050LabyrinthLampReady) {
		if (state.egyptSealPuzzleProgress == 2) {
			state.mainFlowStateId = kScene2070SealSolvedEntryState;
			return;
		}

		state.egyptLabyrinthPositionIndex = 0x2a;
		state.mainFlowStateId = kScene2060EntryState;
		return;
	}

	if (!hasInventoryItem(kScene2050LampInventoryItem)) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	if (!state.ronLampFueled) {
		beginSecondarySpeechLine(5, 1);
		return;
	}

	beginSecondarySpeechLine(5, 2);
	walkActiveActorTo(0x1b6, 0x10f, 1, 0, false);
	runLongLabyrinthWalkClip();
	removeInventoryItem(kScene2050LampInventoryItem);
	state.scene2050LabyrinthLampReady = true;
	state.egyptLabyrinthPositionIndex = 0x2a;
	state.mainFlowStateId = kScene2060EntryState;
}

void Scene2050::runLongLabyrinthWalkClip() {
	ActionOverlaySpec spec(12, kScene2050LabyrinthWalkDescriptorCount,
		kScene2050LabyrinthWalkFrameMap, ARRAYSIZE(kScene2050LabyrinthWalkFrameMap),
		kScene2050OverlayFrameMillis);
	spec.startAt(1).hookEveryFrame(kScene2050LabyrinthFootstepHook).noRedrawAtEnd();
	runActorReplacement(spec);
}

void Scene2050::runRevealFloorSeal() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(9, 0);
	state.scene2050SealRevealed = true;
	applySceneStateToHotspotsAndPatches(2);
}

void Scene2050::runMuralSubscreenAction() {
	GameplayState &state = _vm->gameState();

	runMuralPuzzleSubscreen();
	if (state.scene2050MuralPuzzleState == 0) {
		beginSecondarySpeechLine(7, 0);
		return;
	}

	if (state.scene2050MuralPuzzleState == 2)
		runSealDiscoveryIfPending();
}

void Scene2050::runGuidedMuralAction() {
	GameplayState &state = _vm->gameState();

	if (state.scene2050MuralPuzzleState == 0)
		beginSecondarySpeechLine(12, 0);

	runMuralClipForward();
	if (state.scene2050MuralPuzzleState == 0)
		state.scene2050MuralPuzzleState = 1;
	else if (state.scene2050MuralPuzzleState == 2)
		state.scene2050MuralPuzzleState = 3;

	runMuralPuzzleSubscreen();
	runMuralClipBackward();

	if (state.scene2050MuralPuzzleState == 1) {
		state.scene2050MuralPuzzleState = 0;
		beginSecondarySpeechLine(7, 0);
		return;
	}

	if (state.scene2050MuralPuzzleState == 3)
		runSealDiscoveryIfPending();
	if (state.scene2050MuralPuzzleState == 3)
		state.scene2050MuralPuzzleState = 2;
}

void Scene2050::runMuralClipForward() {
	ActionOverlaySpec spec(9, kScene2050MuralClipDescriptorCount,
		kScene2050MuralClipForwardFrameMap, ARRAYSIZE(kScene2050MuralClipForwardFrameMap),
		kScene2050OverlayFrameMillis);
	spec.noRedrawAtEnd();
	runActorReplacement(spec);
}

void Scene2050::runMuralClipBackward() {
	ActionOverlaySpec spec(9, kScene2050MuralClipDescriptorCount,
		kScene2050MuralClipBackwardFrameMap, ARRAYSIZE(kScene2050MuralClipBackwardFrameMap),
		kScene2050OverlayFrameMillis);
	spec.noRedrawAtEnd();
	runActorReplacement(spec);
}

void Scene2050::runMuralPuzzleSubscreen() {
	const byte chunkIndex = (_vm->gameState().scene2050MuralPuzzleState == 1 ||
		_vm->gameState().scene2050MuralPuzzleState == 3) ? 14 : 13;

	if (!_sceneChunkTable.isValidChunk(chunkIndex) ||
			_sceneChunkTable.sizes[chunkIndex] < framebufferByteCount() ||
			_resourceChunkOffsets[chunkIndex] + framebufferByteCount() > _resourceArena.size() ||
			!loadMuralHitMask())
		return;

	if (_vm->gameState().scene2050MuralPuzzleState == 2 ||
			_vm->gameState().scene2050MuralPuzzleState == 3) {
		setMuralTilePermutationSolved();
	} else if (!initializeMuralTilePermutation(chunkIndex)) {
		return;
	}

	const uint16 previousViewportXOffset = _viewportXOffset;
	const Common::Array<byte> previousPalette = _paletteCurrent;
	_viewportXOffset = 0;
	_muralSelectedTile = 0;
	_muralPaletteAccumulator = 0;
	if (_sceneChunkTable.isValidChunk(kScene2050MuralPaletteChunk)) {
		const uint paletteBytes = MIN<uint>(_sceneChunkTable.sizes[kScene2050MuralPaletteChunk],
			_paletteCurrent.size());
		memcpy(_paletteCurrent.data(),
			_resourceArena.data() + _resourceChunkOffsets[kScene2050MuralPaletteChunk], paletteBytes);
		_displayPalette.markAllDirty();
	}
	drawRawSceneChunk(chunkIndex);
	drawMuralTileGrid(chunkIndex);
	presentFrame();

	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());

	bool done = false;
	uint32 lastMillis = g_system->getMillis();
	while (!done && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		const GameplayState &state = _vm->gameState();
		if (state.egyptSealPuzzleProgress == 0 &&
				state.scene2050MuralPuzzleState >= 2 && isMuralPuzzleSolved()) {
			drawMuralTileGrid(state.scene2050MuralPuzzleState == 3 ? 14 : 13);
			presentFrame();
			break;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				Engine::quitGame();
				done = true;
				break;
			case Common::EVENT_MAINMENU:
				_vm->openMainMenuDialog();
				if (_vm->isSceneRestartRequested()) {
					done = true;
					break;
				}
				_displayPalette.markAllDirty();
				drawMuralTileGrid(chunkIndex);
				presentFrame();
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					done = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_SPACE) {
					handleMuralTileClick(muralTileAtScreenPoint(
						_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY()), done);
				}
				break;
			case Common::EVENT_MOUSEMOVE:
				_vm->cursor()->updatePosition(event.mouse);
				break;
			case Common::EVENT_LBUTTONDOWN: {
				_vm->cursor()->updatePosition(event.mouse);
				const byte tileId = muralTileAtScreenPoint(event.mouse.x, event.mouse.y);
				handleMuralTileClick(tileId, done);
				break;
			}
			case Common::EVENT_RBUTTONDOWN:
				_vm->cursor()->updatePosition(event.mouse);
				done = true;
				break;
			default:
				break;
			}
		}

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		_muralPaletteAccumulator += delta;
		while (_muralPaletteAccumulator >= kScene2050OverlayFrameMillis) {
			_muralPaletteAccumulator -= kScene2050OverlayFrameMillis;
			rotateMuralHighlightPalette();
		}
		_vm->cursor()->advance(delta);
		updateAmbientAudioAndMusicCues(delta);
		presentFrame();
		g_system->delayMillis(10);
	}

	_vm->cursor()->leaveInteractiveMode();
	_viewportXOffset = previousViewportXOffset;
	_paletteCurrent = previousPalette;
	_muralHitMask.clear();
	_displayPalette.markAllDirty();
	drawPlayableComposite();
	presentFrame();
}

void Scene2050::runSealDiscoveryIfPending() {
	GameplayState &state = _vm->gameState();
	if (state.egyptSealPuzzleProgress != 0)
		return;

	state.egyptSealPuzzleProgress = 1;
	runSealDiscoverySequence();
}

void Scene2050::runSealDiscoverySequence() {
	_soundBank0.playSampleLooping(kScene2050SealDiscoveryLoopSound, 50);
	_sealDiscoveryActive = true;
	_sealDiscoveryFrame = 0;
	_sealDiscoveryActorPathFrameIndex = 0;
	_sealDiscoverySpeechStarted = false;
	_sealDiscoverySpeechStartMillis = 0;
	_sealDiscoverySpeechDurationMillis = 0;

	Scene2050DeltaFrameTarget target(_sealDiscoveryFrame);
	AnimationFrameRange range(1, ARRAYSIZE(kScene2050SealDiscoveryFrameMap) - 1,
		kScene2050OverlayFrameMillis);
	range.frameOrder = kScene2050SealDiscoveryFrameMap;
	range.hookEveryFrame(kScene2050SealDiscoveryHook);
	playAnimationFrames(target, range);

	_actorPathPlaybackActive = false;
	_soundBank0.playSample(kScene2050SealDiscoveryEndSound, 50);
	if (_sealDiscoverySpeechStarted) {
		const uint32 elapsed = g_system->getMillis() - _sealDiscoverySpeechStartMillis;
		const uint32 remaining = elapsed < _sealDiscoverySpeechDurationMillis ?
			_sealDiscoverySpeechDurationMillis - elapsed : 0;
		waitForSpeechOrDelay(remaining, false);
		_speech.stop();
		clearSpeechOverlay();
	}

	_sealDiscoveryActive = false;
	drawPlayableComposite();
	presentFrame();
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		beginSecondarySpeechLine(6, 1);
}

void Scene2050::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene2050LabyrinthFootstepHook) {
		if (frame == 0x1b || frame == 0x21 || frame == 0x27 || frame == 0x2d)
			playActiveActorFootstep();
		return;
	}

	if (hookId != kScene2050SealDiscoveryHook || !_sealDiscoveryActive)
		return;

	bool pathFinishedThisFrame = false;
	if (frame == kScene2050SealDiscoveryTurnFrame) {
		queueActorPathWithPaletteRegionRouting(kScene2050SealDiscoveryActorX,
			kScene2050SealDiscoveryActorY, kScene2050SealDiscoveryActorX,
			kScene2050SealDiscoveryActorY, 2, 0);
		_sealDiscoveryActorPathFrameIndex = 1;
		_actorPathPlaybackActive = _actorPathFrames.size() > 1;
		if (!_actorPathPlaybackActive) {
			setActiveActorPose(kScene2050SealDiscoveryActorX, kScene2050SealDiscoveryActorY, 2);
			pathFinishedThisFrame = true;
		}
	}

	if (frame >= kScene2050SealDiscoveryTurnFrame && _actorPathPlaybackActive) {
		if (_sealDiscoveryActorPathFrameIndex < _actorPathFrames.size()) {
			const ActorPathFrame &pathFrame = _actorPathFrames[_sealDiscoveryActorPathFrameIndex++];
			_activeActorWorldX = pathFrame.worldX;
			_activeActorWorldY = pathFrame.worldY;
			_activeActorFacing = pathFrame.facing;
			_activeActorCel = pathFrame.cel;
			_activeActorDrawOrderMode = pathFrame.drawOrderMode;
		}
		if (_sealDiscoveryActorPathFrameIndex >= _actorPathFrames.size()) {
			_actorPathPlaybackActive = false;
			pathFinishedThisFrame = true;
		}
	}

	if (frame <= kScene2050SealDiscoveryTurnFrame || _actorPathPlaybackActive ||
			pathFinishedThisFrame || _sealDiscoverySpeechStarted)
		return;

	_sealDiscoverySpeechStarted = true;
	const bool speechAudioStarted = startSecondarySpeechLine(6, 0);
	_sealDiscoverySpeechStartMillis = g_system->getMillis();
	_sealDiscoverySpeechDurationMillis = speechAudioStarted ?
		MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(1200, _speechOverlay.lines.size() * 1100);
}

void Scene2050::drawSealDiscoveryDeltaLayer() {
	const uint lastFrame = MIN<uint>(_sealDiscoveryFrame,
		kScene2050SealDiscoveryDescriptorCount - 1);
	// The clip patches prior frames, while this compositor rebuilds the base.
	for (uint frame = 0; frame <= lastFrame; ++frame)
		drawClipFrameDelta(11, kScene2050SealDiscoveryDescriptorCount, (byte)frame);
}

void Scene2050::drawRawSceneChunk(uint chunkIndex) {
	const uint32 baseOffset = _resourceChunkOffsets[chunkIndex];
	if (!_sceneChunkTable.isValidChunk(chunkIndex) ||
			_sceneChunkTable.sizes[chunkIndex] < framebufferByteCount() ||
			baseOffset + framebufferByteCount() > _resourceArena.size())
		return;

	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	memcpy(pixels, _resourceArena.data() + baseOffset, framebufferByteCount());
}

bool Scene2050::loadMuralHitMask() {
	_muralHitMask.clear();
	if (!_sceneChunkTable.isValidChunk(kScene2050MuralHitMaskChunk))
		return false;

	_muralHitMask.resize(framebufferByteCount());
	memset(_muralHitMask.data(), 0, _muralHitMask.size());
	const uint32 chunkOffset = _resourceChunkOffsets[kScene2050MuralHitMaskChunk];
	const uint32 chunkEnd = chunkOffset + _sceneChunkTable.sizes[kScene2050MuralHitMaskChunk];
	if (chunkEnd < chunkOffset || chunkEnd > _resourceArena.size()) {
		_muralHitMask.clear();
		return false;
	}
	uint32 sourceOffset = chunkOffset;
	uint destinationOffset = 0;
	while (destinationOffset < _muralHitMask.size() && sourceOffset + 3 <= chunkEnd) {
		const byte fill = _resourceArena[sourceOffset];
		const uint16 runLength = readUint16LE(_resourceArena, sourceOffset + 1);
		sourceOffset += 3;

		const uint count = MIN<uint>(runLength, _muralHitMask.size() - destinationOffset);
		if (count != 0) {
			memset(_muralHitMask.data() + destinationOffset, fill, count);
			destinationOffset += count;
		}
	}

	if (destinationOffset == _muralHitMask.size())
		return true;

	_muralHitMask.clear();
	return false;
}

bool Scene2050::initializeMuralTilePermutation(uint chunkIndex) {
	if (_muralPermutationInitialized && _muralPermutationChunkIndex == chunkIndex)
		return true;

	if (_vm->gameState().scene2050MuralPuzzleState >= 2 ||
			_vm->gameState().egyptSealPuzzleProgress != 0) {
		setMuralTilePermutationSolved();
	} else if (!isSavedMuralTilePermutationValid()) {
		randomizeMuralTilePermutation();
	}

	_muralPermutationInitialized = true;
	_muralPermutationChunkIndex = (byte)chunkIndex;
	return true;
}

bool Scene2050::isSavedMuralTilePermutationValid() const {
	bool seen[GameplayState::kScene2050MuralTilePermutationSize];
	memset(seen, 0, sizeof(seen));

	const byte *permutation = _vm->gameState().scene2050MuralTilePermutation;
	for (uint tile = 1; tile <= kScene2050MuralTileCount; ++tile) {
		const byte sourceTile = permutation[tile];
		if (sourceTile == 0 || sourceTile > kScene2050MuralTileCount || seen[sourceTile])
			return false;
		seen[sourceTile] = true;
	}

	return true;
}

void Scene2050::randomizeMuralTilePermutation() {
	byte *permutation = _vm->gameState().scene2050MuralTilePermutation;

	for (uint restart = 0; restart < 128; ++restart) {
		bool used[GameplayState::kScene2050MuralTilePermutationSize];
		memset(permutation, 0, sizeof(_vm->gameState().scene2050MuralTilePermutation));
		memset(used, 0, sizeof(used));

		bool complete = true;
		for (uint tile = 1; tile <= kScene2050MuralTileCount; ++tile) {
			byte candidate = 0;
			bool foundCandidate = false;
			for (uint attempt = 0; attempt < 512; ++attempt) {
				candidate = (byte)(_random.getRandomNumber(kScene2050MuralTileCount - 1) + 1);
				if (candidate != tile && !used[candidate]) {
					foundCandidate = true;
					break;
				}
			}
			if (!foundCandidate) {
				complete = false;
				break;
			}
			permutation[tile] = candidate;
			used[candidate] = true;
		}

		if (complete)
			return;
	}

	permutation[0] = 0;
	for (uint tile = 1; tile <= kScene2050MuralTileCount; ++tile)
		permutation[tile] = (byte)(tile == kScene2050MuralTileCount ? 1 : tile + 1);
}

void Scene2050::setMuralTilePermutationSolved() {
	byte *permutation = _vm->gameState().scene2050MuralTilePermutation;
	for (uint tile = 1; tile <= kScene2050MuralTileCount; ++tile)
		permutation[tile] = (byte)tile;
	permutation[0] = 0;
	_muralPermutationInitialized = true;
	_muralPermutationChunkIndex = 0xff;
}

void Scene2050::drawMuralTileGrid(uint chunkIndex) {
	const uint32 baseOffset = _resourceChunkOffsets[chunkIndex];
	if (!_sceneChunkTable.isValidChunk(chunkIndex) ||
			_sceneChunkTable.sizes[chunkIndex] < framebufferByteCount() ||
			baseOffset + framebufferByteCount() > _resourceArena.size())
		return;

	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	const byte *rawFrame = _resourceArena.data() + baseOffset;
	const byte *permutation = _vm->gameState().scene2050MuralTilePermutation;
	for (uint tile = 1; tile <= kScene2050MuralTileCount; ++tile)
		copyMuralTile((byte)tile, permutation[tile], rawFrame, pixels);

	if (_muralSelectedTile != 0)
		drawMuralSelectionHighlight(_muralSelectedTile);
}

void Scene2050::drawMuralSelectionHighlight(byte tileId) {
	if (tileId == 0 || tileId > kScene2050MuralTileCount)
		return;

	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	const uint tileIndex = tileId - 1;
	const int column = tileIndex % kScene2050MuralGridColumns;
	const int row = tileIndex / kScene2050MuralGridColumns;
	const int x = kScene2050MuralVisibleGridX + column * kScene2050MuralTileStep;
	const int y = kScene2050MuralGridY + row * kScene2050MuralTileStep;
	byte color = kScene2050MuralHighlightColor;
	for (int i = 0; i < kScene2050MuralTileSize; ++i) {
		pixels[(y - 1) * HollywoodEngine::kSceneBufferWidth + x + i] = color;
		color = color == kScene2050MuralLastHighlightColor ?
			kScene2050MuralHighlightColor : color + 1;
	}
	color = kScene2050MuralHighlightColor;
	for (int i = 0; i < kScene2050MuralTileSize; ++i) {
		pixels[(y + i) * HollywoodEngine::kSceneBufferWidth + x + kScene2050MuralTileSize] = color;
		color = color == kScene2050MuralLastHighlightColor ?
			kScene2050MuralHighlightColor : color + 1;
	}
	color = kScene2050MuralHighlightColor;
	for (int i = kScene2050MuralTileSize - 1; i >= 0; --i) {
		pixels[(y + kScene2050MuralTileSize) * HollywoodEngine::kSceneBufferWidth + x + i] = color;
		color = color == kScene2050MuralLastHighlightColor ?
			kScene2050MuralHighlightColor : color + 1;
	}
	color = kScene2050MuralHighlightColor;
	for (int i = kScene2050MuralTileSize - 1; i >= 0; --i) {
		pixels[(y + i) * HollywoodEngine::kSceneBufferWidth + x - 1] = color;
		color = color == kScene2050MuralLastHighlightColor ?
			kScene2050MuralHighlightColor : color + 1;
	}
}

void Scene2050::clearMuralSelectionHighlight(byte tileId) {
	if (tileId == 0 || tileId > kScene2050MuralTileCount)
		return;

	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	const uint tileIndex = tileId - 1;
	const int x = kScene2050MuralVisibleGridX +
		(tileIndex % kScene2050MuralGridColumns) * kScene2050MuralTileStep;
	const int y = kScene2050MuralGridY +
		(tileIndex / kScene2050MuralGridColumns) * kScene2050MuralTileStep;
	memset(pixels + (y - 1) * HollywoodEngine::kSceneBufferWidth + x, 0,
		kScene2050MuralTileSize);
	memset(pixels + (y + kScene2050MuralTileSize) * HollywoodEngine::kSceneBufferWidth + x, 0,
		kScene2050MuralTileSize);
	for (int i = 0; i < kScene2050MuralTileSize; ++i) {
		pixels[(y + i) * HollywoodEngine::kSceneBufferWidth + x - 1] = 0;
		pixels[(y + i) * HollywoodEngine::kSceneBufferWidth + x + kScene2050MuralTileSize] = 0;
	}
}

void Scene2050::rotateMuralHighlightPalette() {
	const uint firstOffset = kScene2050MuralHighlightColor * 3;
	const uint lastOffset = kScene2050MuralLastHighlightColor * 3;
	if (_paletteCurrent.size() < lastOffset + 3)
		return;

	byte lastColor[3];
	memcpy(lastColor, _paletteCurrent.data() + lastOffset, sizeof(lastColor));
	for (int color = kScene2050MuralLastHighlightColor;
			color > kScene2050MuralHighlightColor; --color) {
		memcpy(_paletteCurrent.data() + color * 3,
			_paletteCurrent.data() + (color - 1) * 3, 3);
	}
	memcpy(_paletteCurrent.data() + firstOffset, lastColor, sizeof(lastColor));
	_displayPalette.markAllDirty();
}

byte Scene2050::muralTileAtScreenPoint(int16 x, int16 y) const {
	if (x < 0 || x >= HollywoodEngine::kSceneBufferWidth ||
			y < 0 || y >= HollywoodEngine::kSceneBufferHeight ||
			_muralHitMask.size() != framebufferByteCount())
		return 0;

	const byte tileId = _muralHitMask[y * HollywoodEngine::kSceneBufferWidth + x];
	return tileId <= kScene2050MuralTileCount ? tileId : 0;
}

void Scene2050::handleMuralTileClick(byte tileId, bool &done) {
	if (tileId == 0 || tileId > kScene2050MuralTileCount)
		return;

	GameplayState &state = _vm->gameState();
	byte *permutation = state.scene2050MuralTilePermutation;
	if (state.scene2050MuralPuzzleState != 0 && permutation[tileId] == tileId) {
		_soundBank0.playSample(kScene2050MuralTileLockedSound, 50);
		return;
	}

	if (_muralSelectedTile == 0) {
		_muralSelectedTile = tileId;
		_soundBank0.playSample(kScene2050MuralTileSelectSound, 100);
		drawMuralTileGrid((state.scene2050MuralPuzzleState == 1 ||
			state.scene2050MuralPuzzleState == 3) ? 14 : 13);
		presentFrame();
		return;
	}

	const byte firstTile = _muralSelectedTile;
	clearMuralSelectionHighlight(firstTile);
	_muralSelectedTile = 0;
	if (firstTile != tileId) {
		const uint solvedBefore = solvedMuralTileCount();
		SWAP(permutation[firstTile], permutation[tileId]);
		_soundBank0.playSample(kScene2050MuralTileSwapSound, 50);
		if (solvedMuralTileCount() > solvedBefore)
			_ambientSoundBank0.playSample(kScene2050MuralTileImprovedSound, 100);
		if (isMuralPuzzleSolved()) {
			const bool redrawSolvedGrid = state.scene2050MuralPuzzleState == 0;
			state.scene2050MuralPuzzleState = redrawSolvedGrid ? 2 : 3;
			setMuralTilePermutationSolved();
			if (redrawSolvedGrid) {
				drawMuralTileGrid(13);
				presentFrame();
			}
			done = true;
			return;
		}
	}

	drawMuralTileGrid((state.scene2050MuralPuzzleState == 1 ||
		state.scene2050MuralPuzzleState == 3) ? 14 : 13);
	presentFrame();
}

bool Scene2050::isMuralPuzzleSolved() const {
	const byte *permutation = _vm->gameState().scene2050MuralTilePermutation;
	for (uint tile = 1; tile <= kScene2050MuralTileCount; ++tile) {
		if (permutation[tile] != tile)
			return false;
	}

	return true;
}

uint Scene2050::solvedMuralTileCount() const {
	uint count = 0;
	const byte *permutation = _vm->gameState().scene2050MuralTilePermutation;
	for (uint tile = 1; tile <= kScene2050MuralTileCount; ++tile) {
		if (permutation[tile] == tile)
			++count;
	}
	return count;
}

void Scene2050::copyMuralTile(byte visibleTileId, byte sourceTileId, const byte *rawFrame, byte *destination) {
	if (visibleTileId == 0 || sourceTileId == 0 ||
			visibleTileId > kScene2050MuralTileCount || sourceTileId > kScene2050MuralTileCount)
		return;

	const uint visibleIndex = visibleTileId - 1;
	const uint sourceIndex = sourceTileId - 1;
	const int visibleX = kScene2050MuralVisibleGridX +
		(visibleIndex % kScene2050MuralGridColumns) * kScene2050MuralTileStep;
	const int visibleY = kScene2050MuralGridY +
		(visibleIndex / kScene2050MuralGridColumns) * kScene2050MuralTileStep;
	const int sourceX = kScene2050MuralSourceGridX +
		(sourceIndex % kScene2050MuralGridColumns) * kScene2050MuralTileStep;
	const int sourceY = kScene2050MuralGridY +
		(sourceIndex / kScene2050MuralGridColumns) * kScene2050MuralTileStep;
	const bool remapSolvedTile = _vm->gameState().scene2050MuralPuzzleState != 0 &&
		sourceTileId == visibleTileId &&
		_sceneChunkTable.isValidChunk(kScene2050MuralSolvedTilePaletteMapChunk) &&
		_sceneChunkTable.sizes[kScene2050MuralSolvedTilePaletteMapChunk] >= kScenePaletteMapPageSize;
	const byte *paletteMap = remapSolvedTile ?
		_resourceArena.data() + _resourceChunkOffsets[kScene2050MuralSolvedTilePaletteMapChunk] : nullptr;

	for (int y = 0; y < kScene2050MuralTileSize; ++y) {
		const byte *source = rawFrame + (sourceY + y) * HollywoodEngine::kSceneBufferWidth + sourceX;
		byte *target = destination + (visibleY + y) * HollywoodEngine::kSceneBufferWidth + visibleX;
		if (!paletteMap) {
			memcpy(target, source, kScene2050MuralTileSize);
			continue;
		}
		for (int x = 0; x < kScene2050MuralTileSize; ++x)
			target[x] = paletteMap[source[x]];
	}
}

void Scene2050::copyStageSmallRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

} // End of namespace Hollywood
