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
const uint kScene5120ActorPaletteTableEntry = 0x00b0;
const uint32 kScene5120SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5120FrameMillis = 75;
const uint32 kScene5120SlowFrameMillis = 250;
const uint kScene5120ElevatorDescriptorCount = 0x12;
const uint kScene5120TongsPickupDescriptorCount = 0x14;
const uint kScene5120PillboxFillDescriptorCount = 0x0c;
const uint kScene5120ProjectorInstallDescriptorCount = 0x0c;
const uint kScene5120ProjectorFirstDescriptorCount = 0x31;
const uint kScene5120ProjectorSecondDescriptorCount = 0x14;
const byte kScene5120TongsSceneItem = 13;
const byte kScene5120CupSceneItem = 7;
const byte kScene5120TongsInventoryItem = 0x56;
const byte kScene5120FilmInventoryItem = 0x59;
const byte kScene5120MagnetPillboxInventoryItem = 0x0d;
const byte kScene5120EmptyPillboxInventoryItem = 0x0f;
const byte kScene5120BombMagnetPillboxInventoryItem = 0x0b;
const byte kScene5120BombPillboxInventoryItem = 0x0c;

enum {
	kScene5120PatchTongs = 1
};

const uint kScene5120MovingWallLayer = 0;
const uint kScene5120MainProjectionLayer = 1;
const uint kScene5120SideLoopLayer = 2;
const uint kScene5120ToggleLayer = 3;
const uint kScene5120RandomDetailLayer = 4;

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
	11, 11, 10, 9, 8, 7, 6, 5, 4, 4, 4, 4, 4,
	5, 6, 7, 8, 9, 10, 11
};

const byte kScene5120ProjectorInstallFrameMap[] = {
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 11
};

const byte kScene5120ProjectorFirstFrameMap[] = {
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
	0x34, 0x35
};

const byte kScene5120ProjectorSecondFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19
};

const byte kScene5120MainProjectionFrameMap[] = {
	0, 10, 11, 12, 13, 5, 6, 7, 7, 7, 7, 7, 8, 9,
	1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 9, 8, 7, 6, 5,
	0, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
	38, 39, 40, 41, 42, 43
};

const byte kScene5120SideLoopFrameMap[] = {
	0, 1, 2, 3, 2, 1
};

static PlayableSceneConfig scene5120Config() {
	PlayableSceneConfig config(5120,
		SceneResourceLayout(5, 5, 0x15),
		SceneViewport(kScene5120ViewportXOffset, kScene5120ViewportXOffset, kScene5120ViewportXOffset),
		SceneActorPose(0x127, 0x12c, 5));
	config.setActorResources(kScene5120ActorBankTableEntry, kScene5120ActorPaletteTableEntry);
	config.setTextResources(0, kScene5120SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene5120::Scene5120(HollywoodEngine *vm) :
		PlayableScene(vm, scene5120Config()),
		_transformedRoomLayers() {
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

	copyBaseFramebufferToSceneFramebuffer();
	if (!foregroundActorMode)
		drawTransformedRoomLayers();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawStaticForegroundLayers(actorDrawOrderMode);
	if (foregroundActorMode)
		drawTransformedRoomLayers();
	drawActionOverlayLayer();
}

void Scene5120::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene5120FirstState)
		runFirstEntrySequence();
	else
		runAlternateEntrySequence();
}

bool Scene5120::prepareCustomGameplayLoop() {
	resetTransformedRoomLayers();
	return true;
}

bool Scene5120::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	advanceTransformedRoomLayers(delta);
	return true;
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
	case 319: // Mirar pelicula en el butacon (look at film/armchair).
		beginSecondarySpeechLine(17, 0);
		return true;
	case 320: // Mirar pantalla (look at screen).
		beginSecondarySpeechLine(18, 0);
		return true;
	case 321: // Coger cubitera (take ice bucket).
		beginSecondarySpeechLine(18, 1);
		return true;
	case 322: // Mirar cubitera (look at ice bucket).
		beginSecondarySpeechLine(19, 0);
		return true;
	case 323: // Usar cubitera/pinzas (use ice bucket/tongs): takes the tongs from the bucket.
		runTongsPickup();
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
	case 329: // Usar palanca (use lever): original 5120 callback table leaves this as a description.
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
	targetX = CLIP<int>(targetX, 0x08e, 0x31a);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

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
		if (state.scene5120CocktailState < 2)
			clearSceneItemFromColorMap(kScene5120CupSceneItem);
		else
			replaceColorMapItemFromOriginal(kScene5120CupSceneItem, kScene5120CupSceneItem);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene5120::shouldConvertSavedFramebufferFF() const {
	return true;
}

AmbientAudioProfile Scene5120::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene5120SlowFrameMillis;
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

void Scene5120::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId != kScene5120PatchTongs)
		return;

	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
}

void Scene5120::runFirstEntrySequence() {
	setActiveActorPose(0x08e, 0x0f5, 2);
	drawPlayableComposite();
	presentFrame();

	runElevatorDoorClip(true);
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
		beginPrimarySpeechLine(21, 2, 0x175, 0x00b6, 0x17, 0x33, 0x2c);
		state.scene5110SalonTransformState = 4;
		walkActiveActorTo(0x111, 0x14e, 3, 0, false);
	}
}

void Scene5120::runAlternateEntrySequence() {
	setActiveActorPose(0x169, 0x113, 5);
	drawPlayableComposite();
	presentFrame();
}

void Scene5120::runElevatorDoorClip(bool opening) {
	_soundBank0.playSample(0x1d, 100);
	runSceneOverlay(ActionOverlaySpec(8, kScene5120ElevatorDescriptorCount,
		opening ? kScene5120ElevatorOpenFrameMap : kScene5120ElevatorCloseFrameMap, ARRAYSIZE(kScene5120ElevatorOpenFrameMap), kScene5120FrameMillis));
}

void Scene5120::runElevatorReturnSequence() {
	runElevatorDoorClip(false);
	runElevatorDoorClip(true);

	setActiveActorPose(0x08e, 0x0f5, 2);
	walkActiveActorTo(0x127, 0x12c, 0xff, 0, false);
}

void Scene5120::runElevatorExitToState(uint16 nextState) {
	runElevatorDoorClip(false);
	_soundBank0.playSample(0x1e, 100);
	_vm->gameState().mainFlowStateId = nextState;
}

void Scene5120::runTongsPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5120TongsTaken || hasInventoryItem(kScene5120TongsInventoryItem)) {
		beginSecondarySpeechLine(20, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(10, kScene5120TongsPickupDescriptorCount,
		kScene5120TongsPickupFrameMap, ARRAYSIZE(kScene5120TongsPickupFrameMap), kScene5120FrameMillis)
		.hookAt(10, kScene5120PatchTongs));
	state.scene5120TongsTaken = true;
	applySceneStateToHotspotsAndPatches(1);
	addInventoryItem(kScene5120TongsInventoryItem);
	_soundBank0.playSample(1, 100);
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

	beginSecondarySpeechLine(25, 0);
	runActorReplacement(ActionOverlaySpec(21, kScene5120PillboxFillDescriptorCount,
		kScene5120PillboxFillFrameMap, ARRAYSIZE(kScene5120PillboxFillFrameMap), kScene5120FrameMillis));
	state.scene5120CocktailState = 3;
	applySceneStateToHotspotsAndPatches(2);
	removeInventoryItem(pillboxItem);
	addInventoryItem(pillboxItem == kScene5120MagnetPillboxInventoryItem ?
		kScene5120BombMagnetPillboxInventoryItem : kScene5120BombPillboxInventoryItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(25, 1);
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

	runSceneOverlay(ActionOverlaySpec(18, kScene5120ProjectorInstallDescriptorCount,
		kScene5120ProjectorInstallFrameMap, ARRAYSIZE(kScene5120ProjectorInstallFrameMap), kScene5120FrameMillis));
	removeInventoryItem(kScene5120FilmInventoryItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(26, 0);

	runSceneOverlay(ActionOverlaySpec(17, kScene5120ProjectorFirstDescriptorCount,
		kScene5120ProjectorFirstFrameMap, ARRAYSIZE(kScene5120ProjectorFirstFrameMap), kScene5120FrameMillis));
	state.scene5110SalonTransformState = MAX<byte>(state.scene5110SalonTransformState, 2);
	applySceneStateToHotspotsAndPatches(0xff);
	resetTransformedRoomLayers();
	runSceneOverlay(ActionOverlaySpec(19, kScene5120ProjectorSecondDescriptorCount,
		kScene5120ProjectorSecondFrameMap, ARRAYSIZE(kScene5120ProjectorSecondFrameMap), kScene5120FrameMillis));
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
	_transformedRoomLayers.clear();
	_transformedRoomLayers.configureLayer(kScene5120MovingWallLayer, 9, 5, nullptr, 0);
	_transformedRoomLayers.configureLayer(kScene5120MainProjectionLayer, 10, 0x0e, kScene5120MainProjectionFrameMap,
		ARRAYSIZE(kScene5120MainProjectionFrameMap));
	_transformedRoomLayers.configureLayer(kScene5120SideLoopLayer, 11, 4,
		kScene5120SideLoopFrameMap, ARRAYSIZE(kScene5120SideLoopFrameMap));
	_transformedRoomLayers.configureLayer(kScene5120ToggleLayer, 12, 2, nullptr, 0);
	_transformedRoomLayers.configureLayer(kScene5120RandomDetailLayer, 13, 6, nullptr, 0);
	resetTransformedRoomLayers();
}

void Scene5120::resetTransformedRoomLayers() {
	_movingWallChannel.reset(4, kScene5120SlowFrameMillis);
	_mainProjectionChannel.reset(0, kScene5120SlowFrameMillis);
	_sideLoopChannel.reset(0, kScene5120FrameMillis);
	_toggleChannel.reset(0, kScene5120SlowFrameMillis);
	_randomDetailChannel.reset(0, kScene5120SlowFrameMillis);
	_transformedRoomLayers.setLayerVisible(kScene5120MovingWallLayer, true);
	_transformedRoomLayers.setLayerFramePreservingVisibility(kScene5120MovingWallLayer, 4);
	_transformedRoomLayers.setLayerVisible(kScene5120MainProjectionLayer, true);
	_transformedRoomLayers.setLayerFramePreservingVisibility(kScene5120MainProjectionLayer, 0);
	_transformedRoomLayers.setLayerVisible(kScene5120SideLoopLayer, true);
	_transformedRoomLayers.setLayerFramePreservingVisibility(kScene5120SideLoopLayer, 0);
	_transformedRoomLayers.setLayerVisible(kScene5120ToggleLayer, true);
	_transformedRoomLayers.setLayerFramePreservingVisibility(kScene5120ToggleLayer, 0);
	_transformedRoomLayers.setLayerVisible(kScene5120RandomDetailLayer, true);
	_transformedRoomLayers.setLayerFramePreservingVisibility(kScene5120RandomDetailLayer, 0);
}

void Scene5120::advanceTransformedRoomLayers(uint32 delta) {
	if (_vm->gameState().scene5110SalonTransformState < 2)
		return;

	const uint movingTicks = _movingWallChannel.consumeFrames(delta);
	for (uint i = 0; i < movingTicks; ++i) {
		byte nextFrame = _transformedRoomLayers.layerFrame(kScene5120MovingWallLayer);
		if (_random.getRandomBit()) {
			if (nextFrame > 0)
				--nextFrame;
		} else if (nextFrame < 4) {
			++nextFrame;
		}
		_transformedRoomLayers.setLayerFrame(kScene5120MovingWallLayer, nextFrame);
	}

	const uint projectionTicks = _mainProjectionChannel.consumeFrames(delta);
	for (uint i = 0; i < projectionTicks; ++i) {
		byte nextFrame = _transformedRoomLayers.layerFrame(kScene5120MainProjectionLayer);
		if (nextFrame == 0) {
			if (_random.getRandomNumber(14) == 0)
				nextFrame = 4;
		} else {
			nextFrame = 0;
		}
		_transformedRoomLayers.setLayerFrame(kScene5120MainProjectionLayer, nextFrame);
	}

	const uint sideTicks = _sideLoopChannel.consumeFrames(delta);
	for (uint i = 0; i < sideTicks; ++i)
		_transformedRoomLayers.setLayerFrame(kScene5120SideLoopLayer,
			(byte)((_transformedRoomLayers.layerFrame(kScene5120SideLoopLayer) + 1) % ARRAYSIZE(kScene5120SideLoopFrameMap)));

	const uint toggleTicks = _toggleChannel.consumeFrames(delta);
	for (uint i = 0; i < toggleTicks; ++i)
		_transformedRoomLayers.setLayerFrame(kScene5120ToggleLayer,
			_transformedRoomLayers.layerFrame(kScene5120ToggleLayer) == 0 ? 1 : 0);

	const uint randomTicks = _randomDetailChannel.consumeFrames(delta);
	for (uint i = 0; i < randomTicks; ++i) {
		const byte previousFrame = _transformedRoomLayers.layerFrame(kScene5120RandomDetailLayer);
		byte nextFrame = previousFrame;
		while (nextFrame == previousFrame)
			nextFrame = (byte)_random.getRandomNumber(5);
		_transformedRoomLayers.setLayerFrame(kScene5120RandomDetailLayer, nextFrame);
	}
}

void Scene5120::drawTransformedRoomLayers() {
	if (_vm->gameState().scene5110SalonTransformState < 2)
		return;

	drawTransientLayers(_transformedRoomLayers);
}

void Scene5120::drawStaticForegroundLayers(byte actorDrawOrderMode) {
	if (actorDrawOrderMode != 2 && actorDrawOrderMode != 7 && actorDrawOrderMode != 8)
		return;

	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_vm->gameState().scene5120TongsTaken && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
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
