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

#include "hollywood/scenes/playable/scene5120.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene5120FirstState = 0x1400;
const uint16 kScene5120UseShakerExitState = 0x140a;
const uint16 kScene5110ReturnState = 0x13f7;
const uint16 kScene5100ReturnState = 0x13ed;
const uint16 kScene5120ViewportXOffset = 0x00a0;
const uint kScene5120ActorBankTableEntry = 0x0000;
const uint kScene5120ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5120SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5120ActionFrameMillis = 75;
const uint32 kScene5120ElevatorFrameMillis = 60;
const uint32 kScene5120MovingWallFrameMillis = 60;
const uint32 kScene5120ProjectionFrameMillis = 125;
const uint32 kScene5120SideLoopFrameMillis = 120;
const uint32 kScene5120ToggleFrameMillis = 200;
const uint32 kScene5120RandomDetailFrameMillis = 280;
const uint32 kScene5120AmbientCheckMillis = 250;
const uint32 kScene5120ProjectorLeadInMillis = 2000;
const uint kScene5120ElevatorDescriptorCount = 0x12;
const uint kScene5120TongsPickupDescriptorCount = 10;
const uint kScene5120PillboxFillDescriptorCount = 0x0c;
const uint kScene5120ProjectorInstallDescriptorCount = 0x0c;
const uint kScene5120ProjectorDescriptorCount = 0x31;
const uint kScene5120ProjectorSpeechDescriptorCount = 5;
const uint kScene5120TransformationDescriptorCount = 0x14;
const byte kScene5120TongsSceneItem = 13;
const byte kScene5120CupSceneItem = 7;
const byte kScene5120TongsInventoryItem = 0x56;
const byte kScene5120FilmInventoryItem = 0x59;
const byte kScene5120MagnetPillboxInventoryItem = 0x0d;
const byte kScene5120EmptyPillboxInventoryItem = 0x0f;
const byte kScene5120BombMagnetPillboxInventoryItem = 0x0b;
const byte kScene5120BombPillboxInventoryItem = 0x0c;
const byte kScene5120CocktailPaletteIndex = 0xa0;

const uint kScene5120MovingWallLayer = 0;
const uint kScene5120MainProjectionLayer = 1;
const uint kScene5120SideLoopLayer = 2;
const uint kScene5120ToggleLayer = 3;
const uint kScene5120RandomDetailLayer = 4;
const uint kScene5120TransformationLayer = 5;
const uint kScene5120ElevatorLayer = 6;
const uint kScene5120ProjectorSpeechLayer = 7;

const byte kScene5120WerewolfSpeechGroup = 0;
const byte kScene5120ProjectorSpeechGroup = 1;

const byte kScene5120ElevatorOpenFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
};

const byte kScene5120ElevatorCloseFrameMap[] = {
	17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kScene5120TongsPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};

const byte kScene5120PillboxFillFrameMap[] = {
	11, 11, 10, 9, 8, 7, 6, 5,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 6, 7, 8, 9, 10, 11
};

const byte kScene5120ProjectorInstallFrameMap[] = {
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 11
};

const byte kScene5120ProjectorFirstFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17
};

const byte kScene5120ProjectorSecondFrameMap[] = {
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47
};

const byte kScene5120MainProjectionFrameMap[] = {
	0, 10, 11, 12, 13, 5, 6, 7, 7, 7, 7, 7, 8, 9,
	1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 9, 8, 7, 6, 5,
	0, 0, 1, 2, 3, 4
};

static PlayableSceneConfig scene5120Config() {
	PlayableSceneConfig config(5120,
		SceneResourceLayout(5, 5, 0x15),
		SceneViewport(kScene5120ViewportXOffset, kScene5120ViewportXOffset, kScene5120ViewportXOffset),
		SceneActorPose(0x127, 0x12c, 5));
	config.setActorResources(kScene5120ActorBankTableEntry, kScene5120ActorPaletteTableEntry);
	config.setTextResources(0, kScene5120SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene5120::Scene5120(HollywoodEngine *vm) :
		PlayableScene(vm, scene5120Config()),
		_sideLoopTrack(RealtimeAnimationTracks::kInvalidTrack),
		_toggleTrack(RealtimeAnimationTracks::kInvalidTrack),
		_randomDetailTrack(RealtimeAnimationTracks::kInvalidTrack),
		_projectorSpeechActive(false),
		_roomTransformationActive(false),
		_transformationOverlayStarted(false),
		_transformationPaletteDelta(0) {
	initializeTransformedRoomLayers();
}

void Scene5120::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);
	resetTransformedRoomLayers();

	if (_vm->gameState().mainFlowStateId == kScene5120FirstState) {
		_activeActorWorldX = 0x08e;
		_activeActorWorldY = 0x0f5;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x169;
		_activeActorWorldY = 0x113;
		_activeActorFacing = 5;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene5120::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	const bool foregroundActorMode = actorDrawOrderMode == 2 || actorDrawOrderMode == 7 || actorDrawOrderMode == 8;
	const bool actionVisible = _actionOverlayPlayer.isVisible();
	const bool actionBehindActors = actionVisible &&
		_actionOverlayPlayer._stratum == kSceneAnimationBehindActors;
	const bool actionReplacesActor = actionVisible &&
		_actionOverlayPlayer._stratum == kSceneAnimationActorReplacement;

	if (_projectorSpeechActive) {
		copyBaseFramebufferToSceneFramebuffer();
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5120ProjectorSpeechLayer));
		return;
	}

	if (_roomTransformationActive) {
		copyBaseFramebufferToSceneFramebuffer();
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		drawTransformedRoomLayers();
		return;
	}

	copyBaseFramebufferToSceneFramebuffer();
	if (!foregroundActorMode)
		drawTransformedRoomLayers();
	if (actionBehindActors)
		drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (actionReplacesActor)
		drawActionOverlayLayer();
	else
		drawStaticForegroundLayers(actorDrawOrderMode);
	if (foregroundActorMode)
		drawTransformedRoomLayers();
	if (actionVisible && !actionBehindActors && !actionReplacesActor)
		drawActionOverlayLayer();
	drawResourceSpriteLayer(_sceneLayers.layer(kScene5120ElevatorLayer));
}

void Scene5120::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene5120FirstState)
		runFirstEntrySequence();
	else
		runAlternateEntrySequence();
}

void Scene5120::prepareCustomGameplayLoop() {
	resetTransformedRoomLayers();
}

void Scene5120::advanceCustomGameplayLoop(uint32 delta) {
	advanceTransformedRoomLayers(delta);
}

void Scene5120::advancePrimarySpeechAnimation(uint32 delta) {
	if (!_roomTransformationActive)
		PlayableScene::advancePrimarySpeechAnimation(delta);
}

void Scene5120::advanceAmbientAudio(uint32 delta) {
	if (!_roomTransformationActive)
		PlayableScene::advanceAmbientAudio(delta);
}

bool Scene5120::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar ascensor (look at elevator).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar boton superior (use upper elevator button): returns to this screening room.
		runElevatorReturnSequence();
		return true;
	case 303: // Usar boton central (use middle elevator button): exits to salon/spa, scene 5110.
		runElevatorExitToState(kScene5110ReturnState);
		return true;
	case 304: // Usar boton inferior (use lower elevator button): exits back to mine elevator, scene 5100.
		runElevatorExitToState(kScene5100ReturnState);
		return true;
	case 305: // Coger botella (take bottle): bottles stay on the bar.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 306: // Mirar botella (look at bottle): random fullness description.
		beginSecondarySpeechLine(3, (byte)_random.getRandomNumber(2));
		return true;
	case 307: // Usar botella (use bottle): original no-op action slot.
		return true;
	case 308: // Coger coctelera (take shaker): Ron leaves it for mixing drinks.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 309: // Mirar coctelera (look at shaker).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 310: // Usar coctelera (use shaker): enters the Nessie Boom mixing substate when unlocked.
		runUseShaker();
		return true;
	case 311: // Coger copa (take glass): too dangerous with Nessie Boom inside.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 312: // Mirar copa (look at glass).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Usar copa (use glass).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Mirar proyector (look at projector).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Usar proyector (use projector).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 316: // Mirar butacon (look at armchair).
		beginSecondarySpeechLine(13, 0);
		return true;
	case 317: // Usar butacon (use armchair): Ron postpones watching the film.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 318: // Mirar estanteria con peliculas (look at movie shelf): changes after projection.
		beginSecondarySpeechLine(_vm->gameState().scene5110SalonTransformState < 2 ? 15 : 16, 0);
		return true;
	case 319: // Unreferenced callback-table gap: use the ice-bucket response if reached.
		beginSecondarySpeechLine(18, 0);
		return true;
	case 320: // Mirar pantalla (look at screen).
		beginSecondarySpeechLine(17, 0);
		return true;
	case 321: // Coger cubitera (take ice bucket).
		beginSecondarySpeechLine(18, 0);
		return true;
	case 322: // Mirar cubitera (look at ice bucket).
		beginSecondarySpeechLine(18, 1);
		return true;
	case 323: // Usar cubitera/pinzas (use ice bucket/tongs).
		beginSecondarySpeechLine(19, 0);
		return true;
	case 324: // Coger pinzas (take tongs).
		runTongsPickup();
		return true;
	case 325: // Mirar pinzas (look at tongs).
		beginSecondarySpeechLine(20, 0);
		return true;
	case 326: // Hablar con hombre lobo (talk to werewolf): do not disturb him while watching.
		beginSecondarySpeechLine(21, 0);
		return true;
	case 327: // Mirar hombre lobo (look at werewolf).
		beginSecondarySpeechLine(22, 0);
		return true;
	case 328: // Coger palanca (take lever): fixed to the bottle shelf.
		beginSecondarySpeechLine(23, 0);
		return true;
	case 329: // Usar palanca (use lever): describes its purpose.
		beginSecondarySpeechLine(24, 0);
		return true;
	case 330: // Mirar palanca (look at lever).
		beginSecondarySpeechLine(24, 0);
		return true;
	case 331: // Usar pastillero/pastillero con iman con copa (use pillbox with glass): fills it with Nessie Boom.
		runCocktailFillPillbox();
		return true;
	case 332: // Usar pelicula con proyector (use film with projector): projects the werewolf film.
		runFilmProjectorSequence();
		return true;
	case 333: // Ir a estanteria/pantalla (go to movie shelf/screen): blocked after the werewolf starts watching.
		if (_vm->gameState().scene5110SalonTransformState >= 2)
			beginSecondarySpeechLine(16, 0);
		return true;
	case 334: // Usar copa/pastillero variant: no extra effect.
		beginSecondarySpeechLine(25, 2);
		return true;
	case 335: // Usar copa/pastillero variant: asks to empty the pillbox first.
		beginSecondarySpeechLine(25, 3);
		return true;
	default:
		return false;
	}
}

bool Scene5120::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	const int maximumX = _vm->gameState().scene5110SalonTransformState < 2 ? 0x2b1 : 0x293;
	targetX = MIN<int>(targetX, maximumX);
	targetY = CLIP<int>(targetY, 0, 0x1df);

	if (targetY < 0x1df)
		++targetY;
	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	if (targetY == 0x1df) {
		while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
			--targetY;
	}

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene5120::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	GameplayState &state = _vm->gameState();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (state.scene5120TongsTaken || hasInventoryItem(kScene5120TongsInventoryItem)) {
		state.scene5120TongsTaken = true;
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5120TongsSceneItem);
	}

	if (state.scene5110SalonTransformState < 2) {
		clearSceneItemFromColorMap(11);
		clearSceneItemFromColorMap(14);
		clearSceneItemFromColorMap(16);
		if (_metadata.size() > kSceneItemFacing + 10) {
			_metadata[kSceneItemFacing + 10] = 5;
			WRITE_LE_UINT16(&_metadata[kSceneItemInteractionPoints + 10 * 4], 0x0285);
			WRITE_LE_UINT16(&_metadata[kSceneItemInteractionPoints + 10 * 4 + 2], 0x0115);
		}
	} else {
		replaceColorMapItemFromOriginal(14, 14);
		replaceColorMapItemFromOriginal(16, 14);
		replaceColorMapItemFromOriginal(11, 11);
		for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i) {
			if (_fullPaletteRegionMask[i] == 6 || _fullPaletteRegionMask[i] == 7 || _fullPaletteRegionMask[i] == 8)
				_fullPaletteRegionMask[i] = 0;
		}
		if (_sceneChunkTable.isValidChunk(7))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		if (_metadata.size() > kSceneItemFacing + 10) {
			_metadata[kSceneItemFacing + 10] = 1;
			WRITE_LE_UINT16(&_metadata[kSceneItemInteractionPoints + 10 * 4], 0x01e0);
			WRITE_LE_UINT16(&_metadata[kSceneItemInteractionPoints + 10 * 4 + 2], 0x012b);
		}
	}

	if (state.scene5120CocktailState == 0) {
		if (_sceneChunkTable.isValidChunk(15))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5120CupSceneItem);
	} else {
		if (_sceneChunkTable.isValidChunk(14))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[14], _baseFramebuffer);
		applyCocktailPalette();
		if (state.scene5120CocktailState < 2)
			clearSceneItemFromColorMap(kScene5120CupSceneItem);
		else
			replaceColorMapItemFromOriginal(kScene5120CupSceneItem, kScene5120CupSceneItem);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	updateElevatorButtonActionTargets(false);
	return true;
}

bool Scene5120::shouldConvertSavedFramebufferFF() const {
	return true;
}

void Scene5120::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene5120::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene5120AmbientCheckMillis;
	if (_vm->gameState().scene5110SalonTransformState > 1) {
		profile.soundMode = kAmbientSoundLoop;
		profile.soundCueId = 0x1f;
		profile.soundVolumePercent = 5;
	}
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

byte Scene5120::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0;
}

byte Scene5120::primarySpeechAnimationFrameCount(byte animationGroup) const {
	(void)animationGroup;
	return 5;
}

uint32 Scene5120::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene5120ProjectionFrameMillis;
}

void Scene5120::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene5120ProjectorSpeechGroup) {
		_sceneLayers.setLayerFrame(kScene5120ProjectorSpeechLayer, frameIndex);
		return;
	}

	_sceneLayers.setVisibleLayerFrame(kScene5120MainProjectionLayer, frameIndex);
}

void Scene5120::handleLeftClick(const GameplayLoopCursorState &state) {
	updateElevatorButtonActionTargets(state.currentStrip == 5);
	PlayableScene::handleLeftClick(state);
}

void Scene5120::runFirstEntrySequence() {
	setActiveActorPose(0x08e, 0x0f5, 2);
	_sceneLayers.showLayerAtFrame(kScene5120ElevatorLayer, 0);
	drawPlayableComposite();
	if (fadePaletteFromBlack()) {
		_sceneLayers.setLayerVisible(kScene5120ElevatorLayer, false);
		return;
	}

	_soundBank0.playSample(0x1d, 100, true);
	runElevatorDoorClip(true);
	_soundBank0.stop();
	walkActiveActorTo(0x127, 0x12c, 0xff, 0, false);

	GameplayState &state = _vm->gameState();
	if (!state.scene5110WerewolfDialogueChoiceUnlocked) {
		beginSecondarySpeechLine(0, 0);
		state.scene5110WerewolfDialogueChoiceUnlocked = true;
		return;
	}

	if (state.scene5110SalonTransformState == 3) {
		beginSecondarySpeechLine(0, 1);
		walkActiveActorTo(0x1c5, 0x130, 0xff, 0, false);
		beginSecondarySpeechLine(0, 2);
		_activeActorFacing = 4;
		_activeActorCel = 0;
		beginSecondarySpeechLine(21, 1);
		beginPrimarySpeechLineWithAnimationGroup(21, 2, 0x175, 0x00b6,
			0x17, 0x33, 0x2c, kScene5120WerewolfSpeechGroup);
		state.scene5110SalonTransformState = 4;
		walkActiveActorTo(0x111, 0x14e, 3, 0, false);
		beginStaticSecondarySpeechLine(0xd4, state.ronTravelQuipIndex);
		if (state.ronTravelQuipIndex < 0xff)
			++state.ronTravelQuipIndex;
	}
}

void Scene5120::runAlternateEntrySequence() {
	setActiveActorPose(0x169, 0x113, 5);
	drawPlayableComposite();
	fadePaletteFromBlack();
}

void Scene5120::runElevatorDoorClip(bool opening) {
	_sceneLayers.setLayerVisible(kScene5120ElevatorLayer, false);
	ActionOverlaySpec spec(8, kScene5120ElevatorDescriptorCount,
		opening ? kScene5120ElevatorOpenFrameMap : kScene5120ElevatorCloseFrameMap,
		ARRAYSIZE(kScene5120ElevatorOpenFrameMap), kScene5120ElevatorFrameMillis);
	runSceneOverlay(spec.noRedrawAtEnd());
	_sceneLayers.showLayerAtFrame(kScene5120ElevatorLayer,
		opening ? kScene5120ElevatorDescriptorCount - 1 : 0);
}

void Scene5120::runElevatorReturnSequence() {
	_soundBank0.playSample(0x1d, 100, true);
	runElevatorDoorClip(false);
	runElevatorDoorClip(true);
	_soundBank0.stop();

	setActiveActorPose(0x08e, 0x0f5, 2);
	walkActiveActorTo(0x127, 0x12c, 0xff, 0, false);
}

void Scene5120::runElevatorExitToState(uint16 nextState) {
	_soundBank0.playSample(0x1d, 100, true);
	runElevatorDoorClip(false);
	_soundBank0.playSample(0x1e, 100, true);
	_vm->gameState().mainFlowStateId = nextState;
}

void Scene5120::runTongsPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5120TongsTaken || hasInventoryItem(kScene5120TongsInventoryItem)) {
		beginSecondarySpeechLine(20, 0);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(20, kScene5120TongsPickupDescriptorCount,
			kScene5120TongsPickupFrameMap, ARRAYSIZE(kScene5120TongsPickupFrameMap), kScene5120ActionFrameMillis)
			.resourcePatchAt(10, 6))
		.commit(state.scene5120TongsTaken, true)
		.framebufferPatch(1);
	addInventoryItem(kScene5120TongsInventoryItem);
	sequence.sound(1);
	beginSharedInventorySpeechLine(0x14, randomSharedInventorySpeechFrame(4));
}

void Scene5120::runCocktailFillPillbox() {
	GameplayState &state = _vm->gameState();
	if (state.ronPendingMabusePillsInMagnetPillbox) {
		beginSecondarySpeechLine(25, 3);
		return;
	}

	byte pillboxItem = _lastInventoryPrimaryItemId;
	if (pillboxItem != kScene5120MagnetPillboxInventoryItem &&
			pillboxItem != kScene5120EmptyPillboxInventoryItem) {
		pillboxItem = hasInventoryItem(kScene5120MagnetPillboxInventoryItem) ?
			kScene5120MagnetPillboxInventoryItem : kScene5120EmptyPillboxInventoryItem;
	}
	if (!hasInventoryItem(pillboxItem)) {
		beginSecondarySpeechLine(25, 2);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.secondarySpeech(25, 0)
		.actorReplacement(ActionOverlaySpec(21, kScene5120PillboxFillDescriptorCount,
			kScene5120PillboxFillFrameMap, ARRAYSIZE(kScene5120PillboxFillFrameMap), kScene5120ActionFrameMillis))
		.commit(state.scene5120CocktailState, (byte)3)
		.framebufferPatch(2);
	removeInventoryItem(pillboxItem);
	addInventoryItem(pillboxItem == kScene5120MagnetPillboxInventoryItem ?
		kScene5120BombMagnetPillboxInventoryItem : kScene5120BombPillboxInventoryItem);
	sequence.sound(1)
		.secondarySpeech(25, 1);
}

void Scene5120::runFilmProjectorSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110JacuzziInspectionState == 0) {
		beginStaticSecondarySpeechLine(1, (byte)_random.getRandomNumber(1));
		return;
	}
	if (!hasInventoryItem(kScene5120FilmInventoryItem)) {
		beginSecondarySpeechLine(12, 0);
		return;
	}

	_soundBank0.playSample(0x1d, 100, true);
	runElevatorDoorClip(false);
	_soundBank0.playSample(0x1e, 100, true);
	runActorReplacement(ActionOverlaySpec(18, kScene5120ProjectorInstallDescriptorCount,
		kScene5120ProjectorInstallFrameMap, ARRAYSIZE(kScene5120ProjectorInstallFrameMap),
		kScene5120ActionFrameMillis));
	removeInventoryItem(kScene5120FilmInventoryItem);
	_ambientSoundBank0.playSample(1, 100);
	beginSecondarySpeechLine(26, 0);

	_soundBank0.playSample(0x1d, 100, true);
	if (waitSceneMillis(kScene5120ProjectorLeadInMillis, false)) {
		_soundBank0.stop();
		return;
	}
	runElevatorDoorClip(true);
	runActorReplacement(ActionOverlaySpec(17, kScene5120ProjectorDescriptorCount,
		kScene5120ProjectorFirstFrameMap, ARRAYSIZE(kScene5120ProjectorFirstFrameMap),
		kScene5120ActionFrameMillis).noRedrawAtEnd());
	_soundBank0.stop();

	_projectorSpeechActive = true;
	_sceneLayers.showLayerAtFrame(kScene5120ProjectorSpeechLayer, 0);
	beginPrimarySpeechLineWithAnimationGroup(26, 1, 0x10b, 0x0047,
		0x17, 0x33, 0x2c, kScene5120ProjectorSpeechGroup);
	_projectorSpeechActive = false;
	_sceneLayers.setLayerVisible(kScene5120ProjectorSpeechLayer, false);

	runActorReplacement(ActionOverlaySpec(17, kScene5120ProjectorDescriptorCount,
		kScene5120ProjectorSecondFrameMap, ARRAYSIZE(kScene5120ProjectorSecondFrameMap),
		kScene5120ActionFrameMillis).noRedrawAtEnd());
	if (!runRoomTransformationSequence())
		return;

	applySceneStateToHotspotsAndPatches(0xff);
	resetTransformedRoomLayers();
	drawPlayableComposite();
	presentFrame();
	beginSecondarySpeechLine(26, 2);
}

void Scene5120::runUseShaker() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1070SpencerCocktailRecipeLearned) {
		beginSecondarySpeechLine(4, 9);
		return;
	}
	if (state.scene5120CocktailState > 1) {
		beginSecondarySpeechLine(4, 8);
		return;
	}

	state.mainFlowStateId = kScene5120UseShakerExitState;
}

void Scene5120::initializeTransformedRoomLayers() {
	_sceneLayers.clear();
	_sceneLayers.configureLayer(kScene5120MovingWallLayer,
		kSceneAnimationScenePlaced, 9, 5, nullptr, 0);
	_sceneLayers.configureLayer(kScene5120MainProjectionLayer,
		kSceneAnimationScenePlaced, 10, 0x0e, kScene5120MainProjectionFrameMap,
		ARRAYSIZE(kScene5120MainProjectionFrameMap));
	_sceneLayers.configureLayer(kScene5120SideLoopLayer,
		kSceneAnimationScenePlaced, 11, 4, nullptr, 0);
	_sceneLayers.configureLayer(kScene5120ToggleLayer,
		kSceneAnimationScenePlaced, 12, 2, nullptr, 0);
	_sceneLayers.configureLayer(kScene5120RandomDetailLayer,
		kSceneAnimationScenePlaced, 13, 6, nullptr, 0);
	_sceneLayers.configureLayer(kScene5120TransformationLayer,
		kSceneAnimationScenePlaced, 19,
		kScene5120TransformationDescriptorCount, nullptr, 0, false);
	_sceneLayers.configureLayer(kScene5120ElevatorLayer,
		kSceneAnimationScenePlaced, 8, kScene5120ElevatorDescriptorCount,
		kScene5120ElevatorOpenFrameMap, ARRAYSIZE(kScene5120ElevatorOpenFrameMap), false);
	_sceneLayers.configureLayer(kScene5120ProjectorSpeechLayer,
		kSceneAnimationScenePlaced, 16, kScene5120ProjectorSpeechDescriptorCount,
		nullptr, 0, false);
	_sideLoopTrack = _realtimeAnimationTracks.addPingPong(kScene5120SideLoopLayer, kScene5120SideLoopFrameMillis,
		0, 3, false);
	_toggleTrack = _realtimeAnimationTracks.addLoop(kScene5120ToggleLayer, kScene5120ToggleFrameMillis, 2, false);
	_randomDetailTrack = _realtimeAnimationTracks.addRandom(kScene5120RandomDetailLayer,
		kScene5120RandomDetailFrameMillis, 0, 5, true, false);
	resetTransformedRoomLayers();
}

void Scene5120::resetTransformedRoomLayers() {
	_movingWallChannel.reset(4, kScene5120MovingWallFrameMillis);
	_mainProjectionChannel.reset(0, kScene5120ProjectionFrameMillis);
	_transformationOverlayChannel.reset(0, kScene5120MovingWallFrameMillis);
	_transformationPaletteChannel.reset(0, kScene5120MovingWallFrameMillis);
	_realtimeAnimationTracks.reset(_sideLoopTrack);
	_realtimeAnimationTracks.reset(_toggleTrack);
	_realtimeAnimationTracks.reset(_randomDetailTrack);
	const bool normalTracksActive = _vm->gameState().scene5110SalonTransformState >= 2;
	_realtimeAnimationTracks.setActive(_sideLoopTrack, normalTracksActive);
	_realtimeAnimationTracks.setActive(_toggleTrack, normalTracksActive);
	_realtimeAnimationTracks.setActive(_randomDetailTrack, normalTracksActive);
	_sceneLayers.setLayerVisible(kScene5120MovingWallLayer, true);
	_sceneLayers.setLayerFrame(kScene5120MovingWallLayer, 4);
	_sceneLayers.setLayerVisible(kScene5120MainProjectionLayer, true);
	_sceneLayers.setLayerFrame(kScene5120MainProjectionLayer, 0);
	_sceneLayers.setLayerVisible(kScene5120SideLoopLayer, true);
	_sceneLayers.setLayerVisible(kScene5120ToggleLayer, true);
	_sceneLayers.setLayerVisible(kScene5120RandomDetailLayer, true);
	_sceneLayers.setLayerVisible(kScene5120TransformationLayer, false);
	_sceneLayers.setLayerFrame(kScene5120TransformationLayer, 0);
	_sceneLayers.resetLayer(kScene5120ProjectorSpeechLayer, 0);
	_sceneLayers.setLayerVisible(kScene5120ProjectorSpeechLayer, false);
	_projectorSpeechActive = false;
	_roomTransformationActive = false;
	_transformationOverlayStarted = false;
	_transformationPaletteDelta = 0;
}

bool Scene5120::runRoomTransformationSequence() {
	_roomTransformationActive = true;
	_realtimeAnimationTracks.setActive(_sideLoopTrack, false);
	_realtimeAnimationTracks.setActive(_toggleTrack, false);
	_realtimeAnimationTracks.setActive(_randomDetailTrack, false);
	_transformationOverlayStarted = false;
	_transformationPaletteDelta = 0;
	_mainProjectionChannel.reset(5, kScene5120ActionFrameMillis);
	_transformationOverlayChannel.reset(0, kScene5120MovingWallFrameMillis);
	_transformationPaletteChannel.reset(0, kScene5120MovingWallFrameMillis);

	_sceneLayers.setLayerVisible(kScene5120MovingWallLayer, false);
	_sceneLayers.setLayerVisible(kScene5120MainProjectionLayer, true);
	_sceneLayers.setLayerFrame(kScene5120MainProjectionLayer, 5);
	_sceneLayers.setLayerVisible(kScene5120SideLoopLayer, false);
	_sceneLayers.setLayerVisible(kScene5120ToggleLayer, false);
	_sceneLayers.setLayerVisible(kScene5120RandomDetailLayer, false);
	_sceneLayers.setLayerVisible(kScene5120TransformationLayer, false);
	_sceneLayers.setLayerFrame(kScene5120TransformationLayer, 0);

	while (!roomTransformationComplete() && !animationPlaybackShouldStop()) {
		if (waitSceneMillis(10, false))
			break;
	}

	const bool completed = roomTransformationComplete() && !animationPlaybackShouldStop();
	_roomTransformationActive = false;
	_soundBank0.stop();
	if (!completed) {
		applyTransformationPaletteDelta(0);
		resetTransformedRoomLayers();
	}
	return completed;
}

void Scene5120::advanceRoomTransformation(uint32 delta) {
	bool startedOverlay = false;
	const uint projectionTicks = _mainProjectionChannel.consumeFrames(delta);
	for (uint i = 0; i < projectionTicks; ++i) {
		byte frame = _sceneLayers.layerFrame(kScene5120MainProjectionLayer);
		if (frame < 30)
			++frame;
		_mainProjectionChannel.frameIndex = frame;
		_sceneLayers.setVisibleLayerFrame(kScene5120MainProjectionLayer, frame);
		if (frame == 18 && !_transformationOverlayStarted) {
			_transformationOverlayStarted = true;
			startedOverlay = true;
			_soundBank0.playSample(0x1d, 100, true);
			_sceneLayers.setLayerVisible(kScene5120TransformationLayer, true);
			_sceneLayers.setLayerFrame(kScene5120TransformationLayer, 0);
			_transformationOverlayChannel.reset(0, kScene5120MovingWallFrameMillis);
			_transformationPaletteChannel.reset(0, kScene5120MovingWallFrameMillis);
		}
	}

	if (!_transformationOverlayStarted || startedOverlay)
		return;

	const uint overlayTicks = _transformationOverlayChannel.consumeFrames(delta);
	for (uint i = 0; i < overlayTicks; ++i) {
		byte frame = _sceneLayers.layerFrame(kScene5120TransformationLayer);
		if (frame < 19)
			++frame;
		_transformationOverlayChannel.frameIndex = frame;
		_sceneLayers.setVisibleLayerFrame(kScene5120TransformationLayer, frame);
		if (frame == 19) {
			_vm->gameState().scene5110SalonTransformState = 2;
			_sceneLayers.setLayerVisible(kScene5120MovingWallLayer, true);
			_sceneLayers.setLayerVisible(kScene5120SideLoopLayer, true);
			_sceneLayers.setLayerVisible(kScene5120ToggleLayer, true);
			_sceneLayers.setLayerVisible(kScene5120RandomDetailLayer, true);
		}
	}

	const uint paletteTicks = _transformationPaletteChannel.consumeFrames(delta);
	for (uint i = 0; i < paletteTicks && _transformationPaletteDelta > -3; ++i) {
		--_transformationPaletteDelta;
		applyTransformationPaletteDelta(_transformationPaletteDelta);
	}
}

bool Scene5120::roomTransformationComplete() const {
	return _sceneLayers.layerFrame(kScene5120MainProjectionLayer) >= 30 &&
		_sceneLayers.layerFrame(kScene5120TransformationLayer) >= 19 &&
		_transformationPaletteDelta <= -3;
}

void Scene5120::applyTransformationPaletteDelta(int delta) {
	for (uint color = 0; color < 0xb0; ++color) {
		for (uint component = 0; component < 3; ++component) {
			const uint offset = color * 3 + component;
			_paletteCurrent[offset] = (byte)MAX<int>(0, (int)_paletteResource[offset] + delta);
		}
	}
	for (uint color = 0xd0; color <= 0xe8; ++color) {
		for (uint component = 0; component < 3; ++component) {
			const uint offset = color * 3 + component;
			_paletteCurrent[offset] = (byte)MAX<int>(0, (int)_paletteResource[offset] + delta);
		}
	}
}

void Scene5120::advanceTransformedRoomLayers(uint32 delta) {
	if (_roomTransformationActive) {
		advanceRoomTransformation(delta);
		return;
	}

	if (_vm->gameState().scene5110SalonTransformState < 2)
		return;

	const uint movingTicks = _movingWallChannel.consumeFrames(delta);
	for (uint i = 0; i < movingTicks; ++i) {
		byte nextFrame = _sceneLayers.layerFrame(kScene5120MovingWallLayer);
		if (nextFrame == 0) {
			++nextFrame;
		} else if (nextFrame == 4 || _random.getRandomBit())
			--nextFrame;
		else
			++nextFrame;
		_sceneLayers.setVisibleLayerFrame(kScene5120MovingWallLayer, nextFrame);
	}

	if (!_primaryDialogueSpeechActive) {
		const uint projectionTicks = _mainProjectionChannel.consumeFrames(delta);
		for (uint i = 0; i < projectionTicks; ++i) {
			byte nextFrame = _sceneLayers.layerFrame(kScene5120MainProjectionLayer);
			if (nextFrame == 0) {
				if (_random.getRandomNumber(14) == 0)
					nextFrame = 4;
			} else {
				nextFrame = 0;
			}
			_sceneLayers.setVisibleLayerFrame(kScene5120MainProjectionLayer, nextFrame);
		}
	}

}

void Scene5120::drawTransformedRoomLayers() {
	if (_vm->gameState().scene5110SalonTransformState < 2 && !_roomTransformationActive)
		return;

	drawLayerStack(kSceneAnimationScenePlaced);
}

void Scene5120::drawStaticForegroundLayers(byte actorDrawOrderMode) {
	if (actorDrawOrderMode != 2 && actorDrawOrderMode != 7 && actorDrawOrderMode != 8)
		return;

	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_vm->gameState().scene5120TongsTaken && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

void Scene5120::updateElevatorButtonActionTargets(bool useStrip) {
	ScenePoint interactionPoint;
	interactionPoint.x = useStrip ? 0x08e : 0x0bc;
	interactionPoint.y = useStrip ? 0x0f5 : 0x0fe;
	const byte facing = useStrip ? 2 : 5;
	_hotspots.setActionInteraction(2, interactionPoint, facing);
	_hotspots.setActionInteraction(3, interactionPoint, facing);
	_hotspots.setActionInteraction(4, interactionPoint, facing);
}

void Scene5120::applyCocktailPalette() {
	GameplayState &state = _vm->gameState();
	byte red = state.scene5120CocktailRed;
	byte green = state.scene5120CocktailGreen;
	byte blue = state.scene5120CocktailBlue;
	if (red == 0 && green == 0 && blue == 0) {
		if (state.scene5120CocktailState == 1) {
			red = green = blue = 0x39;
		} else {
			red = 0x18;
			green = 0x30;
			blue = 0x3c;
		}
	}

	const uint offset = kScene5120CocktailPaletteIndex * 3;
	if (_paletteResource.size() < offset + 3 || _paletteCurrent.size() < offset + 3)
		return;
	_paletteResource[offset] = _paletteCurrent[offset] = red;
	_paletteResource[offset + 1] = _paletteCurrent[offset + 1] = green;
	_paletteResource[offset + 2] = _paletteCurrent[offset + 2] = blue;
}

void Scene5120::clearSceneItemFromColorMap(byte itemId) {
	replaceColorMapItemFromOriginal(itemId, 0);
}

void Scene5120::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == sourceItem)
			_paletteMask[kSceneColorToItemMap + color] = destinationItem;
	}
}

void Scene5120::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 20)
			_walkablePaletteMask[i] = 0;
	}
}

} // End of namespace Hollywood
