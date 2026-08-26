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

#include "hollywood/scenes/playable/scene1030.h"

#include "common/debug.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene1030FirstEntryState = 0x0406;
const uint16 kScene1030LeftEntryState = 0x0407;
const uint16 kScene1030ExitState1040 = 0x0412;
const uint16 kScene1030ExitState1010LeftEntry = 0x03f2;
const uint16 kScene1030ViewportXOffset = 0x0010;
const uint kScene1030ActorBankTableEntry = 0x0000;
const uint kScene1030ActorPaletteTableEntry = 0x00cc;
const uint kScene1030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1030SpeechCueDescriptorTableOffset = 0x1135;
const int kScene1030FirstEntryStartX = 0x294;
const int kScene1030FirstEntryStartY = 0x177;
const int kScene1030FirstEntryTargetX = 0x17d;
const int kScene1030FirstEntryTargetY = 0x13a;
const byte kScene1030FirstEntryFacing = 4;
const int kScene1030ReturnEntryStartX = 0x294;
const int kScene1030ReturnEntryStartY = 0x177;
const int kScene1030ReturnEntryTargetX = 0x0dc;
const int kScene1030ReturnEntryTargetY = 0x15e;
const byte kScene1030ReturnEntryFacing = 4;
const int kScene1030LeftEntryStartX = 0x051;
const int kScene1030LeftEntryStartY = 0x11d;
const int kScene1030LeftEntryTargetX = 0x0dc;
const int kScene1030LeftEntryTargetY = 0x15e;
const byte kScene1030LeftEntryFacing = 2;
const int kScene1030WalkTargetMinX = 0x050;
const int kScene1030WalkTargetMaxX = 0x1f3;
const byte kScene1030FirstAmbientSoundCue = 0x25;
const byte kScene1030AmbientSoundCueCount = 7;
const byte kScene1030FirstAmbientMusicCue = 0x0b;
const byte kScene1030AmbientMusicCueCount = 5;
const byte kScene1030AmbientSoundProbabilityModulus = 25;
const byte kScene1030AmbientMusicProbabilityModulus = 50;
const byte kScene1030InvalidActorFacing = 0xff;
const uint32 kScene1030ForegroundFrameMillis = 75;
const uint32 kScene1030SmallForegroundTickMillis = 150;
const uint kScene1030LargeForegroundDescriptorCount = 0x3f;
const uint kScene1030SmallForegroundDescriptorCount = 2;
const uint kScene1030LeftEntryActorDescriptorCount = 0x1a;
const uint kScene1030RightEntryActorDescriptorCount = 0x20;
const uint kScene1030PickupDescriptorCount = 0x0e;
const uint kScene1030GreasyCottonDescriptorCount = 0x10;
const uint kScene1030EntryPaletteByteCount = 0x210;
const byte kScene1030EntryLeftSpeechGroup = 0;
const byte kScene1030EntryRightSpeechGroup = 1;

const byte kScene1030LargeForegroundFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 27, 0, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
	62
};

const byte kScene1030LeftEntryActorFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25
};

const byte kScene1030RightEntryActorFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25, 26, 27, 28, 29,
	30, 31
};

const byte kScene1030EntryGestureRightFrames[] = {
	5, 6, 7, 8, 8, 8, 8, 9, 10, 10
};

const byte kScene1030EntryGestureLeftFrames[] = {
	0, 0, 0, 0, 5, 6, 7, 8, 9, 10
};

const byte kScene1030PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13
};

const byte kScene1030GreasyCottonFrameMap[] = {
	12, 12, 11, 10, 9, 8, 7, 6,
	5, 4, 3, 2, 13, 14, 15, 14,
	13, 14, 15, 14, 13, 14, 15, 14,
	13, 14, 15, 14, 13, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11, 12
};

static PlayableSceneConfig scene1030Config() {
	PlayableSceneConfig config(1030,
		SceneResourceLayout(15, 5, 14),
		SceneViewport(kScene1030ViewportXOffset),
		SceneActorPose(kScene1030FirstEntryTargetX, kScene1030FirstEntryTargetY, kScene1030FirstEntryFacing));
	config.setActorResources(kScene1030ActorBankTableEntry, kScene1030ActorPaletteTableEntry);
	config.setTextResources(kScene1030Resource003RowsOffsetIndex, kScene1030SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 5;
	config.useActorDepthTest = true;
	return config;
}

Scene1030::Scene1030(HollywoodEngine *vm) :
		PlayableScene(vm, scene1030Config()),
		_largeForegroundChannel(),
		_smallForegroundChannel(),
		_largeForegroundLayer(),
		_smallForegroundLayer(),
		_leftEntryActorLayer(),
		_rightEntryActorLayer(),
		_largeForegroundMode(0),
		_smallForegroundTickCount(0),
		_entryActorPathFrameIndex(0),
		_entryActorPathTimerAccumulator(0),
		_entryActorsVisible(false),
		_entryActorsAlternatePose(false),
		_entryActorPathActive(false) {
	_largeForegroundLayer.configure(5, kScene1030LargeForegroundDescriptorCount,
		kScene1030LargeForegroundFrameMap, ARRAYSIZE(kScene1030LargeForegroundFrameMap));
	_smallForegroundLayer.configure(13, kScene1030SmallForegroundDescriptorCount, nullptr, 0);
	_leftEntryActorLayer.configure(9, kScene1030LeftEntryActorDescriptorCount,
		kScene1030LeftEntryActorFrameMap, ARRAYSIZE(kScene1030LeftEntryActorFrameMap));
	_rightEntryActorLayer.configure(10, kScene1030RightEntryActorDescriptorCount,
		kScene1030RightEntryActorFrameMap, ARRAYSIZE(kScene1030RightEntryActorFrameMap));
	_largeForegroundLayer.visible = true;
}

void Scene1030::initializeCustomPreviewState() {
	GameplayState &state = _vm->gameState();
	const bool firstEntryConversationPending = isFirstEntryState() && !state.scene1030EntryConversationSeen;

	initializeDefaultPreviewState();
	_largeForegroundChannel.reset(0, kScene1030ForegroundFrameMillis);
	_smallForegroundChannel.reset(0, kScene1030SmallForegroundTickMillis);
	_largeForegroundLayer.reset(state.scene1030EntryConversationSeen ? 0x1c : 0);
	_largeForegroundLayer.visible = true;
	_largeForegroundMode = state.scene1030EntryConversationSeen ? 1 : 0;
	_smallForegroundLayer.reset(0);
	_smallForegroundLayer.visible = false;
	_smallForegroundTickCount = 0;
	_entryActorPathFrameIndex = 0;
	_entryActorPathTimerAccumulator = 0;
	_entryActorPathActive = false;
	_entryActorsVisible = firstEntryConversationPending;
	_entryActorsAlternatePose = false;
	_leftEntryActorLayer.reset(0);
	_rightEntryActorLayer.reset(0);
	_leftEntryActorLayer.visible = firstEntryConversationPending;
	_rightEntryActorLayer.visible = firstEntryConversationPending;

	if (isLeftEntryState()) {
		_activeActorWorldX = kScene1030LeftEntryTargetX;
		_activeActorWorldY = kScene1030LeftEntryTargetY;
		_activeActorFacing = kScene1030LeftEntryFacing;
	} else if (state.scene1030EntryConversationSeen) {
		_activeActorWorldX = kScene1030ReturnEntryTargetX;
		_activeActorWorldY = kScene1030ReturnEntryTargetY;
		_activeActorFacing = kScene1030ReturnEntryFacing;
	} else {
		_activeActorWorldX = kScene1030FirstEntryStartX;
		_activeActorWorldY = kScene1030FirstEntryStartY;
		_activeActorFacing = kScene1030FirstEntryFacing;
		applyFirstEntryPalette();
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene1030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	applyActorDepthClipForDrawOrder(actorDrawOrderMode);

	if (_entryActorsVisible) {
		restoreResourceSpriteLayerBackground(_leftEntryActorLayer, _baseFramebuffer);
		restoreResourceSpriteLayerBackground(_rightEntryActorLayer, _baseFramebuffer);
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		drawEntryActors();
		return;
	}

	restoreResourceSpriteLayerBackground(_largeForegroundLayer, _baseFramebuffer);
	_smallForegroundLayer.visible = _vm->gameState().scene1030TablePickupState == 1;
	restoreResourceSpriteLayerBackground(_smallForegroundLayer, _baseFramebuffer);

	if (_actionOverlayPlayer.replacesActor()) {
		drawLargeForegroundActor();
		drawSmallForegroundActor();
		drawActionOverlayLayer();
		return;
	}

	if ((drawActiveActor || drawSecondaryActor) && actorDrawOrderMode < 3) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	drawSmallForegroundActor();

	if ((drawActiveActor || drawSecondaryActor) && actorDrawOrderMode >= 3) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	drawLargeForegroundActor();

	drawActionOverlayLayer();
}

void Scene1030::applyActorDepthClipForDrawOrder(byte actorDrawOrderMode) {
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	if (_drawActorDepthYThresholds.size() > 3)
		_drawActorDepthYThresholds[3] = actorDrawOrderMode < 3 ? 0x1e0 : 0;
}

void Scene1030::runCustomEntrySequence() {
	if (!isFirstEntryState() && !isLeftEntryState()) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	GameplayState &state = _vm->gameState();

	if (isLeftEntryState()) {
		runEntryPath(kScene1030LeftEntryStartX, kScene1030LeftEntryStartY,
			kScene1030LeftEntryFacing, kScene1030LeftEntryTargetX, kScene1030LeftEntryTargetY);
		return;
	}

	if (state.scene1030EntryConversationSeen) {
		runEntryPath(kScene1030ReturnEntryStartX, kScene1030ReturnEntryStartY,
			kScene1030ReturnEntryFacing, kScene1030ReturnEntryTargetX, kScene1030ReturnEntryTargetY);
		return;
	}

	applyFirstEntryPalette();
	startFirstEntryActorPath();
	runFirstEntryConversation();
}

bool Scene1030::prepareCustomGameplayLoop() {
	_largeForegroundChannel.reset(0, kScene1030ForegroundFrameMillis);
	_smallForegroundChannel.reset(0, kScene1030SmallForegroundTickMillis);
	_smallForegroundTickCount = 0;
	return true;
}

bool Scene1030::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	if (_entryActorsVisible) {
		advanceFirstEntryActorPath(delta);
		updateAmbientAudioAndMusicCues(delta);
		return true;
	}
	advanceLargeForegroundActor(delta);
	advanceSmallForegroundActor(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 108: // Hablar con pequeño bulto en movimiento / hombre menguante (talk to moving lump / shrinking man).
		beginStaticSecondarySpeechLine(0x64, 0);
		return true;
	case 301: // Ir a entrada (go to entrance).
		_vm->gameState().mainFlowStateId = kScene1030ExitState1040;
		return true;
	case 302: // Mirar entrada (look at entrance).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Hablar con tipo durmiendo la mona (talk to sleeping drunk).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar tipo durmiendo la mona (look at sleeping drunk).
		handleSceneEventFlag0();
		return true;
	case 305: // Coger hombre menguante (take shrinking man).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar hombre menguante (look at shrinking man).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar plato (look at plate).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 308: // Coger plato (take plate).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Coger ponchera (take punch bowl).
		handlePickupPunchBowl();
		return true;
	case 310: // Mirar ponchera (look at punch bowl).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Coger rodaja de limón (take lemon slice).
		handlePickupLemonSlice();
		return true;
	case 312: // Mirar rodaja de limón (look at lemon slice).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Ir a escaleras (go to stairs).
		_vm->gameState().mainFlowStateId = kScene1030ExitState1010LeftEntry;
		return true;
	case 314: // Mirar escaleras (look at stairs).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 315: // Coger pequeño bulto en movimiento / hombre menguante (take moving lump / shrinking man).
		handlePickupShrinkingMan();
		return true;
	case 316: // Mirar pequeño bulto en movimiento / hombre menguante (look at moving lump / shrinking man).
		handleShrinkingManDescription();
		return true;
	case 317: // Usar algodón con plato (use cotton with plate).
		handleGreasyCottonExchange();
		return true;
	default:
		return false;
	}
}

bool Scene1030::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	const int rawTargetX = targetX;
	const int rawTargetY = targetY;
	targetX = CLIP<int>(targetX, kScene1030WalkTargetMinX, kScene1030WalkTargetMaxX);
	targetY = CLIP<int>(targetY, 0, 0x1df);

	do {
		if (targetY < 0x1df)
			++targetY;
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0) {
			debugC(1, kDebugPath,
				"%s custom path adjust: raw=(%d,%d) clampedX=%d adjusted=(%d,%d) scan=down pixel=%u walk=%u region=%u",
				sceneDebugName(), rawTargetX, rawTargetY, targetX, targetX, targetY,
				savedFramebufferPixelAt(offset), _walkablePaletteMask[savedFramebufferPixelAt(offset)],
				paletteRegionAt(targetX, targetY));
			return true;
		}
	} while (targetY != 0x1df);

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0) {
			debugC(1, kDebugPath,
				"%s custom path adjust: raw=(%d,%d) clampedX=%d adjusted=(%d,%d) scan=up pixel=%u walk=%u region=%u",
				sceneDebugName(), rawTargetX, rawTargetY, targetX, targetX, targetY,
				savedFramebufferPixelAt(offset), _walkablePaletteMask[savedFramebufferPixelAt(offset)],
				paletteRegionAt(targetX, targetY));
			return true;
		}
		--targetY;
	}
	if (targetY == 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0) {
			debugC(1, kDebugPath,
				"%s custom path adjust: raw=(%d,%d) clampedX=%d adjusted=(%d,%d) scan=top pixel=%u walk=%u region=%u",
				sceneDebugName(), rawTargetX, rawTargetY, targetX, targetX, targetY,
				savedFramebufferPixelAt(offset), _walkablePaletteMask[savedFramebufferPixelAt(offset)],
				paletteRegionAt(targetX, targetY));
			return true;
		}
	}

	debugC(1, kDebugPath, "%s custom path adjust failed to find walkable target: raw=(%d,%d) clamped=(%d,%d)",
		sceneDebugName(), rawTargetX, rawTargetY, targetX, targetY);
	return true;
}

bool Scene1030::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	if (currentRegion == 1 && nextRegion == 2) {
		for (uint offset = 0x30; offset <= 0x3b && offset < _actorPathStepDeltas.size(); ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene1030::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	if (currentRegion == 1) {
		for (uint offset = 0x0c; offset <= 0x17 && offset < _actorPathStepDeltas.size(); ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 1;
		restoredStepDeltas = true;
	}

	return restoredStepDeltas || requestedFacing >= 0;
}

bool Scene1030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector != 1 && selector != 2 && selector != 0xff)
		return false;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	applyPatchStateColorMaps();
	rebuildScene1030WalkableMask();
	if (_vm->gameState().scene1030ShrinkingManNamed)
		copyStageSmallRow(10, 8);
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene1030::shouldAnimatePrimarySpeechLine() const {
	return _entryActorsVisible;
}

byte Scene1030::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return _entryActorsAlternatePose ? 0x0b : 0;
}

void Scene1030::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene1030EntryLeftSpeechGroup)
		_leftEntryActorLayer.setFrame(frameIndex);
	else if (animationGroup == kScene1030EntryRightSpeechGroup)
		_rightEntryActorLayer.setFrame(frameIndex);
}

void Scene1030::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId == 0 || hookId > 3)
		return;

	GameplayState &state = _vm->gameState();
	state.scene1030TablePickupState = hookId;
	if (hookId == 2) {
		_smallForegroundLayer.reset(0);
		_smallForegroundLayer.visible = false;
	}
	applySceneStateToHotspotsAndPatches(1);
}

AmbientAudioProfile Scene1030::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1030FirstAmbientSoundCue,
		kScene1030AmbientSoundCueCount, 15, kScene1030AmbientSoundProbabilityModulus,
		kScene1030FirstAmbientMusicCue, kScene1030AmbientMusicCueCount, 100,
		kScene1030AmbientMusicProbabilityModulus);
}

void Scene1030::applyFirstEntryPalette() {
	if (_vm->gameState().scene1030EntryConversationSeen || !_sceneChunkTable.isValidChunk(8))
		return;

	const uint32 sourceOffset = _resourceChunkOffsets[8];
	if (sourceOffset >= _resourceArena.size())
		return;

	const uint32 byteCount = MIN<uint32>(_sceneChunkTable.sizes[8],
		MIN<uint32>((uint32)kScene1030EntryPaletteByteCount, (uint32)_paletteCurrent.size()));
	if (sourceOffset + byteCount <= _resourceArena.size()) {
		memcpy(_paletteCurrent.data(), _resourceArena.data() + sourceOffset, byteCount);
		_surfaceState.rebuildPresentationPaletteRemapTable();
	}
}

void Scene1030::restoreNormalPalette() {
	const uint32 byteCount = MIN<uint32>((uint32)kScene1030EntryPaletteByteCount,
		MIN<uint32>((uint32)_paletteResource.size(), (uint32)_paletteCurrent.size()));
	memcpy(_paletteCurrent.data(), _paletteResource.data(), byteCount);
	_surfaceState.rebuildPresentationPaletteRemapTable();
}

void Scene1030::runFirstEntryConversation() {
	GameplayState &state = _vm->gameState();

	_entryActorsVisible = true;
	_entryActorsAlternatePose = false;
	_leftEntryActorLayer.reset(0);
	_rightEntryActorLayer.reset(0);
	_leftEntryActorLayer.visible = true;
	_rightEntryActorLayer.visible = true;
	drawPlayableComposite();
	presentFrame();

	beginPrimarySpeechLineWithAnimationGroup(0x0d, 0, 0x0e3, 0x084, 0x0d, 0x32, 0x3a,
		kScene1030EntryLeftSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(0x0e, 0, 0x079, 0x086, 0x0a, 0x3f, 0,
		kScene1030EntryRightSpeechGroup);

	runEntryGestureSequence();

	_entryActorsAlternatePose = true;
	beginPrimarySpeechLineWithAnimationGroup(0x0d, 1, 0x0e3, 0x084, 0x0d, 0x32, 0x3a,
		kScene1030EntryLeftSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(0x0e, 1, 0x079, 0x086, 0x0a, 0x3f, 0,
		kScene1030EntryRightSpeechGroup);

	runEntryOpenSequence();

	restoreNormalPalette();
	finishFirstEntryActorPath();
	_entryActorsVisible = false;
	_entryActorsAlternatePose = false;
	_leftEntryActorLayer.visible = false;
	_rightEntryActorLayer.visible = false;
	_largeForegroundLayer.setFrame(0x1c);
	_largeForegroundMode = 1;
	state.scene1030EntryConversationSeen = true;
	drawPlayableComposite();
	presentFrame();
	beginSecondarySpeechLine(0, 0);
}

void Scene1030::startFirstEntryActorPath() {
	setActiveActorPose(kScene1030FirstEntryStartX, kScene1030FirstEntryStartY, kScene1030FirstEntryFacing);

	queueActorPathWithPaletteRegionRouting(kScene1030FirstEntryStartX, kScene1030FirstEntryStartY,
		kScene1030FirstEntryTargetX, kScene1030FirstEntryTargetY, kScene1030InvalidActorFacing, 0);
	_entryActorPathFrameIndex = _actorPathFrames.size() > 1 ? 1 : 0;
	_entryActorPathTimerAccumulator = 0;
	_entryActorPathActive = _actorPathFrames.size() > 1;
	_actorPathPlaybackActive = _entryActorPathActive;
}

void Scene1030::advanceFirstEntryActorPath(uint32 delta) {
	if (!_entryActorPathActive)
		return;

	_entryActorPathTimerAccumulator += delta;
	while (_entryActorPathTimerAccumulator >= kScene1030ForegroundFrameMillis && _entryActorPathActive) {
		_entryActorPathTimerAccumulator -= kScene1030ForegroundFrameMillis;

		const ActorPathFrame &frame = _actorPathFrames[_entryActorPathFrameIndex++];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;

		if (_entryActorPathFrameIndex >= _actorPathFrames.size()) {
			_entryActorPathActive = false;
			_actorPathPlaybackActive = false;
		}
	}
}

void Scene1030::finishFirstEntryActorPath() {
	if (_actorPathFrames.size() > 1) {
		const ActorPathFrame &frame = _actorPathFrames.back();
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	} else {
		setActiveActorPose(kScene1030FirstEntryTargetX, kScene1030FirstEntryTargetY, kScene1030FirstEntryFacing);
	}

	_entryActorPathFrameIndex = 0;
	_entryActorPathTimerAccumulator = 0;
	_entryActorPathActive = false;
	_actorPathPlaybackActive = false;
}

void Scene1030::runEntryGestureSequence() {
	for (uint i = 0; i < ARRAYSIZE(kScene1030EntryGestureRightFrames) && !Engine::shouldQuit(); ++i) {
		_rightEntryActorLayer.setFrame(kScene1030EntryGestureRightFrames[i]);
		_leftEntryActorLayer.setFrame(kScene1030EntryGestureLeftFrames[i]);
		if (waitSceneMillis(kScene1030ForegroundFrameMillis))
			return;
	}
}

void Scene1030::runEntryOpenSequence() {
	for (uint i = 0; i <= 0x10 && !Engine::shouldQuit(); ++i) {
		_rightEntryActorLayer.setFrame(MIN<byte>(0x20, 0x10 + i));
		_leftEntryActorLayer.setFrame(MIN<byte>(0x1a, 0x10 + i));
		if (waitSceneMillis(kScene1030ForegroundFrameMillis))
			return;
	}
}

void Scene1030::drawEntryActors() {
	drawResourceSpriteLayer(_leftEntryActorLayer);
	drawResourceSpriteLayer(_rightEntryActorLayer);
}

void Scene1030::drawSmallForegroundActor() {
	_smallForegroundLayer.visible = _vm->gameState().scene1030TablePickupState == 1;
	drawResourceSpriteLayer(_smallForegroundLayer);
}

void Scene1030::drawLargeForegroundActor() {
	drawResourceSpriteLayer(_largeForegroundLayer);
}

void Scene1030::advanceLargeForegroundActor(uint32 delta) {
	const uint frameCount = _largeForegroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		const bool actorNearForeground = _activeActorWorldX < 0x0da && _activeActorWorldY > 0x16e;
		if (_largeForegroundMode == 0) {
			if (_largeForegroundLayer.frameIndex < 0x1b) {
				_largeForegroundLayer.setFrame(_largeForegroundLayer.frameIndex + 1);
				if (_largeForegroundLayer.frameIndex == 9 || _largeForegroundLayer.frameIndex == 0x12 ||
						_largeForegroundLayer.frameIndex == 0x1b)
					_soundBank0.playSample(0x24, 20);
			} else if (actorNearForeground) {
				_largeForegroundLayer.setFrame(0x25);
				_largeForegroundMode = 2;
			} else {
				_largeForegroundLayer.setFrame(0x1c);
				_largeForegroundMode = 1;
			}
		} else if (_largeForegroundMode == 1) {
			if (_largeForegroundLayer.frameIndex < 0x24) {
				_largeForegroundLayer.setFrame(_largeForegroundLayer.frameIndex + 1);
				if (_largeForegroundLayer.frameIndex == 0x24)
					_soundBank0.playSample(0x24, 20);
			} else if (actorNearForeground) {
				_largeForegroundLayer.setFrame(0x25);
				_largeForegroundMode = 2;
			} else {
				_largeForegroundLayer.setFrame(0x1c);
			}
		} else {
			if (_largeForegroundLayer.frameIndex < 0x48) {
				_largeForegroundLayer.setFrame(_largeForegroundLayer.frameIndex + 1);
				if (_largeForegroundLayer.frameIndex == 0x2e || _largeForegroundLayer.frameIndex == 0x37 ||
						_largeForegroundLayer.frameIndex == 0x40)
					_soundBank0.playSample(0x24, 20);
			} else if (!actorNearForeground) {
				_largeForegroundLayer.setFrame(0);
				_largeForegroundMode = 0;
			}
		}
	}
}

void Scene1030::advanceSmallForegroundActor(uint32 delta) {
	if (_vm->gameState().scene1030TablePickupState != 1)
		return;

	const uint frameCount = _smallForegroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_smallForegroundTickCount < 0x0f) {
			++_smallForegroundTickCount;
			continue;
		}
		_smallForegroundTickCount = 0;
		_smallForegroundLayer.setFrame(_smallForegroundLayer.frameIndex == 0 ? 1 : 0);
		if (_smallForegroundLayer.frameIndex == 1) {
			const byte soundId = (byte)(0x0f + _random.getRandomNumber(2));
			_soundBank0.playSample(soundId, 5);
		}
	}
}

void Scene1030::runPickupOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, int patchFrame, byte patchState) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		frameMap, frameMapSize, kScene1030ForegroundFrameMillis)
		.hookAt(patchFrame, patchState));
}

void Scene1030::handleSceneEventFlag0() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1030SleepingDrunkInspected) {
		state.scene1030SleepingDrunkInspected = true;
		applySceneStateToHotspotsAndPatches(2);
		beginSecondarySpeechLine(3, 0);
		return;
	}

	beginStaticSecondarySpeechLine(0x0e, 0);
}

void Scene1030::handlePickupPunchBowl() {
	runPickupOverlay(7, kScene1030PickupDescriptorCount, kScene1030PickupFrameMap,
		ARRAYSIZE(kScene1030PickupFrameMap), 4, 1);
	addInventoryItem(0x1a);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(0x14, (byte)_random.getRandomNumber(4));
}

void Scene1030::handlePickupLemonSlice() {
	runPickupOverlay(7, kScene1030PickupDescriptorCount, kScene1030PickupFrameMap,
		ARRAYSIZE(kScene1030PickupFrameMap), 4, 3);
	addInventoryItem(0x57);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(0x14, (byte)_random.getRandomNumber(4));
}

void Scene1030::handlePickupShrinkingMan() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1030ShrinkingManNamed)
		beginSecondarySpeechLine(0x0c, 0);
	beginSecondarySpeechLine(0x0b, 0);
	runPickupOverlay(7, kScene1030PickupDescriptorCount, kScene1030PickupFrameMap,
		ARRAYSIZE(kScene1030PickupFrameMap), 4, 2);
	addInventoryItem(0x18);
	_soundBank0.playSample(1, 100);
}

void Scene1030::handleShrinkingManDescription() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1030ShrinkingManNamed) {
		beginSecondarySpeechLine(0x0c, 0);
		state.scene1030ShrinkingManNamed = true;
		applySceneStateToHotspotsAndPatches(2);
		return;
	}

	beginSecondarySpeechLine(0x0c, 1);
}

void Scene1030::handleGreasyCottonExchange() {
	beginSecondarySpeechLine(0x0f, 0);
	runPickupOverlay(14, kScene1030GreasyCottonDescriptorCount, kScene1030GreasyCottonFrameMap,
		ARRAYSIZE(kScene1030GreasyCottonFrameMap), -1, 0);
	removeInventoryItem(0x02);
	addInventoryItem(0x17);
	_soundBank0.playSample(1, 100);
}

void Scene1030::applyPatchStateColorMaps() {
	if (_paletteMaskOriginal.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize ||
			_colorToActorDepthClassMap.size() < kScenePaletteMapPageSize)
		return;

	const byte patchState = MIN<byte>(_vm->gameState().scene1030TablePickupState, 3);
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
		const byte originalDepth = _paletteMaskOriginal[kSceneColorToActorDepthClassMap + i];
		byte item = originalItem;
		byte depth = originalDepth;

		if (patchState == 0) {
			if (depth == 5)
				depth = 3;
			if (item == 6 || item == 8)
				item = 5;
		} else if (patchState == 1) {
			if (item == 5 || item == 6)
				item = 0;
			if (originalItem == 8)
				item = 8;
			if (originalDepth == 5)
				depth = 0;
		} else if (patchState == 2) {
			if (item == 5 || item == 8)
				item = 0;
			if (originalItem == 8)
				item = 6;
			if (originalDepth == 5)
				depth = 0;
		} else {
			if (item == 5 || item == 6 || item == 8)
				item = 0;
			if (originalDepth == 5)
				depth = 0;
		}

		_paletteMask[kSceneColorToItemMap + i] = item;
		_paletteMask[kSceneColorToActorDepthClassMap + i] = depth;
		_colorToActorDepthClassMap[i] = depth;
	}

	if (patchState == 1 && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
	else if (patchState == 2 && _sceneChunkTable.isValidChunk(11))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
	else if (patchState == 3 && _sceneChunkTable.isValidChunk(12))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
}

void Scene1030::rebuildScene1030WalkableMask() {
	rebuildWalkablePaletteMask();
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 1 || _walkablePaletteMask[i] == 5)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene1030::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

bool Scene1030::isFirstEntryState() const {
	return _vm->gameState().mainFlowStateId == kScene1030FirstEntryState;
}

bool Scene1030::isLeftEntryState() const {
	return _vm->gameState().mainFlowStateId == kScene1030LeftEntryState;
}

} // End of namespace Hollywood
