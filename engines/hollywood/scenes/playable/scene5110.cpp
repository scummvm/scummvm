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

#include "hollywood/scenes/playable/scene5110.h"

#include "engines/engine.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene5110FirstState = 0x13f6;
const uint16 kScene5100ReturnState = 0x13ed;
const uint16 kScene5120FirstState = 0x1400;
const uint16 kScene5110ViewportXOffset = 0x0068;
const uint kScene5110ActorBankTableEntry = 0x0000;
const uint kScene5110ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5110SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5110FrameMillis = 75;
const uint32 kScene5110SpeechFrameMillis = 125;
const uint32 kScene5110ElevatorFrameMillis = 125;
const uint32 kScene5110ElevatorHoldMillis = 1000;
const uint kScene5110PickupDescriptorCount = 0x0c;
const uint kScene5110ElevatorDescriptorCount = 0x11;
const uint kScene5110ReplacementFillRunsChunk = 0x1a;
const uint kScene5110ReplacementPaletteMaskChunk = 0x1b;
const uint kScene5110WerewolfDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene5110UnderwearInventoryItem = 0x53;
const byte kScene5110BottleInventoryItem = 0x54;
const byte kScene5110MirrorInventoryItem = 0x55;
const byte kScene5110UnderwearSceneItem = 5;
const byte kScene5110BottleSceneItem = 6;
const byte kScene5110MirrorSceneItem = 12;
const byte kScene5110DialogueStageId = 0x62;
const byte kScene5110SalonResponseRow = 99;
const byte kScene5110DialogueNoResponseFrame = 0xff;

enum Scene5110LayerId {
	kScene5110RandomDetailLayer,
	kScene5110CenterSalonLayer,
	kScene5110WerewolfLayer,
	kScene5110CenterSalonDetailLayer,
	kScene5110LeftSalonLayer,
	kScene5110UpperRightSalonLayer,
	kScene5110RightSalonDetailLayer,
	kScene5110LowerSalonDetailLayer,
	kScene5110RightStaticSalonLayer,
	kScene5110ElevatorLayer,
	kScene5110LayerCount
};

enum {
	kScene5110DialogueTransitionEnd = 0,
	kScene5110DialogueTransitionDown = 1,
	kScene5110DialogueTransitionUp = 2,
	kScene5110DialogueTransitionStay = 3,
	kScene5110DialogueTransitionUpTwo = 4
};

enum {
	kScene5110PatchUnderwear = 1,
	kScene5110PatchBottle = 2,
	kScene5110PatchMirror = 3
};

const byte kScene5110PickupFrameMap[] = {
	11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene5110FirstElevatorFrameMap[] = { 0, 1, 2, 3, 4, 5 };
const byte kScene5110AlternateElevatorFrameMap[] = { 16, 15, 14, 13, 12, 5 };
const byte kScene5110ElevatorCloseFrameMap[] = { 10, 9, 8, 7, 6, 5 };
const byte kScene5110ElevatorInnerFrameMap[] = { 6, 7, 8, 9, 10, 11 };
const byte kScene5110ElevatorUpperFrameMap[] = { 12, 13, 14, 15, 16 };
const byte kScene5110ElevatorLowerFrameMap[] = { 5, 4, 3, 2, 1, 0 };

const byte kScene5110WerewolfFrameMap[] = {
	0, 1, 2, 3, 4, 5, 5, 6, 7, 8, 9, 5, 4, 3, 2, 0
};
const byte kScene5110CenterDetailFrameMap[] = { 0, 1, 2, 3, 4, 3, 2, 1, 0 };
const byte kScene5110LeftSalonFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0
};
const byte kScene5110UpperRightFrameMap[] = { 0, 1, 2, 3, 4, 3, 2, 1, 0 };
const byte kScene5110LowerDetailFrameMap[] = { 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0 };

const SceneLayerSpec kScene5110LayerSpecs[] = {
	{kSceneAnimationScenePlaced, 7, 4, nullptr, 0, true, 0},
	{kSceneAnimationScenePlaced, 8, 5, nullptr, 0, true, 1},
	{kSceneAnimationScenePlaced, 9, 10, kScene5110WerewolfFrameMap,
		ARRAYSIZE(kScene5110WerewolfFrameMap), true, 0},
	{kSceneAnimationScenePlaced, 10, 5, kScene5110CenterDetailFrameMap,
		ARRAYSIZE(kScene5110CenterDetailFrameMap), true, 0},
	{kSceneAnimationScenePlaced, 15, 9, kScene5110LeftSalonFrameMap,
		ARRAYSIZE(kScene5110LeftSalonFrameMap), true, 0},
	{kSceneAnimationScenePlaced, 16, 8, kScene5110UpperRightFrameMap,
		ARRAYSIZE(kScene5110UpperRightFrameMap), true, 0},
	{kSceneAnimationScenePlaced, 17, 7, nullptr, 0, true, 0},
	{kSceneAnimationScenePlaced, 18, 7, kScene5110LowerDetailFrameMap,
		ARRAYSIZE(kScene5110LowerDetailFrameMap), true, 0},
	{kSceneAnimationScenePlaced, 19, 1, nullptr, 0, true, 0},
	{kSceneAnimationScenePlaced, 22, kScene5110ElevatorDescriptorCount,
		nullptr, 0, false, 0}
};

static_assert(ARRAYSIZE(kScene5110LayerSpecs) == kScene5110LayerCount,
	"Scene 5110 layer specification count changed");

static PlayableSceneConfig scene5110Config() {
	PlayableSceneConfig config(5110,
		SceneResourceLayout(5, 5, 0x19),
		SceneViewport(kScene5110ViewportXOffset, kScene5110ViewportXOffset, kScene5110ViewportXOffset),
		SceneActorPose(0x0b4, 0x174, 5));
	config.setActorResources(kScene5110ActorBankTableEntry, kScene5110ActorPaletteTableEntry);
	config.setTextResources(0, kScene5110SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = false;
	return config;
}

Scene5110::Scene5110(HollywoodEngine *vm) :
		PlayableScene(vm, scene5110Config()),
		_randomDetailTrack(RealtimeAnimationTracks::kInvalidTrack),
		_salonAnimationChannel(),
		_upperRightSalonState(1),
		_rightSalonDetailDirection(0),
		_rightSalonDetailDwell(0),
		_lowerSalonDetailState(1),
		_leftSalonRepeatCount(0),
		_werewolfState(1),
		_centerSalonDetailRepeatCount(0),
		_centerSalonToggleA(false),
		_centerSalonToggleB(false),
		_centerSalonDetailSequenceActive(false),
		_werewolfDialogueActive(false) {
	_sceneLayers.configure(kScene5110LayerSpecs);
	_randomDetailTrack = _realtimeAnimationTracks.addRandom(_sceneLayers,
		kScene5110RandomDetailLayer, kScene5110FrameMillis, 0, 3, true);
}

int Scene5110::replacementFillRunsResourceChunkIndex() const {
	return _vm->gameState().scene5110SalonTransformState >= 2 ?
		(int)kScene5110ReplacementFillRunsChunk : -1;
}

int Scene5110::replacementPaletteMaskResourceChunkIndex() const {
	return _vm->gameState().scene5110SalonTransformState >= 2 ?
		(int)kScene5110ReplacementPaletteMaskChunk : -1;
}

void Scene5110::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	initializeSalonLayers();
	initializeSceneItemLabels();
	applySceneStateToHotspotsAndPatches(0xff);
	if (hasSavedActiveActorPoseForCurrentState() && _sceneChunkTable.isValidChunk(23))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[23], _baseFramebuffer);

	setActiveActorPose(0x084, 0x169, 2);
}

void Scene5110::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_actionOverlayPlayer.replacesActor()) {
		drawActionOverlayLayer();
		drawSalonSpriteLayers();
		return;
	}
	if (_sceneLayers.layerVisible(kScene5110ElevatorLayer)) {
		drawElevatorComposite(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY);
		return;
	}

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawNormalRoomLayers(activeWorldY);
	drawActionOverlayLayer();
}

bool Scene5110::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene5110::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	const bool alternateEntry = state.mainFlowStateId != kScene5110FirstState;
	if (!runEntryElevatorSequence(alternateEntry))
		return;

	if (!state.scene5110IntroSeen && state.scene5110SalonTransformState < 2) {
		walkActiveActorTo(0x197, 0x108, 0xff, 0, false);
		beginSalonPrimarySpeechLine(0, 0);
		beginSecondarySpeechLine(0, 1);
		beginSalonPrimarySpeechLine(0, 2);
		state.scene5110IntroSeen = true;
		return;
	}

	walkActiveActorTo(0x0b4, 0x174, 0xff, 0, false);
}

void Scene5110::advanceCustomGameplayLoop(uint32 delta) {
	advanceSalonAnimations(delta);
}

bool Scene5110::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar ascensor (look at elevator).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar boton superior (use upper button): exits to scene 5120.
		runButtonExitToState(kScene5120FirstState);
		return true;
	case 303: // Usar boton central (use middle button): elevator returns to this room.
		runButtonReturnSequence();
		return true;
	case 304: // Usar boton inferior (use lower button): exits back to scene 5100.
		runButtonExitToState(kScene5100ReturnState);
		return true;
	case 305: // Coger calzoncillo (take underwear): grants item 0x53.
		runUnderwearPickup();
		return true;
	case 306: // Mirar calzoncillo (look at underwear).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 307: // Coger frasco (take bottle): grants item 0x54 after the salon is transformed.
		runBottlePickup();
		return true;
	case 308: // Mirar frasco (look at bottle): identifies it before pickup.
		if (_vm->gameState().scene5110BottleState == 0) {
			beginSecondarySpeechLine(4, 0);
			_vm->gameState().scene5110BottleState = 1;
			applySceneStateToHotspotsAndPatches(2);
		} else {
			beginSecondarySpeechLine(4, 1);
		}
		return true;
	case 309: // Hablar con hombre lobo (talk to werewolf): opens the werewolf dialogue.
		runWerewolfDialogue();
		return true;
	case 310: // Mirar hombre lobo (look at werewolf).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 311: // Hablar con chica haciendo la manicura (talk to manicure girl).
		runManicureGirlTalk();
		return true;
	case 312: // Mirar chica haciendo la manicura (look at manicure girl).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 313: // Hablar con chica del jacuzzi (talk to jacuzzi girl).
		runJacuzziGirlTalk();
		return true;
	case 314: // Mirar chica del jacuzzi (look at jacuzzi girl).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 315: // Ir a tocador/espejo (go to dressing table/mirror area).
		runMirrorApproach();
		return true;
	case 316: // Mirar jacuzzi (look at jacuzzi).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 317: // Coger espejo (take mirror): grants item 0x55 after the salon is transformed.
		runMirrorPickup();
		return true;
	case 318: // Mirar/usar espejo (look/use mirror).
		beginSecondarySpeechLine(13, 0);
		return true;
	case 319: // Mirar jacuzzi despues del dialogo (look at jacuzzi after dialogue state change).
		runJacuzziInspection();
		return true;
	case 320: // Mirar tratamiento capilar, primera zona (look at hair treatment, first area).
		beginSecondarySpeechLine(16, 0);
		return true;
	case 321: // Mirar tratamiento capilar, segunda zona (look at hair treatment, second area).
		beginSecondarySpeechLine(17, 0);
		return true;
	case 322: // Mirar tratamiento capilar, tercera zona (look at hair treatment, third area).
		beginSecondarySpeechLine(18, 0);
		return true;
	case 323: // Mirar tratamiento capilar, cuarta zona (look at hair treatment, fourth area).
		beginSecondarySpeechLine(15, 0);
		return true;
	case 324: // Hablar con tratamiento capilar (talk to hair treatment customer).
		runHairTreatmentTalk();
		return true;
	default:
		return false;
	}
}

bool Scene5110::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	const int maximumX = _vm->gameState().scene5110SalonTransformState < 2 ? 0x197 : 0x286;
	targetX = CLIP<int>(targetX, 0x0b4, maximumX);
	if (targetX > 0x0c9 && targetX < 0x168)
		targetX = targetX - 0x0c9 <= 0x168 - targetX ? 0x0c9 : 0x168;

	if (targetY < 0x1df)
		++targetY;
	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene5110::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	if (currentRegion != 2 || (nextRegion != 1 && nextRegion != 3))
		return false;

	const uint destinationOffset = nextRegion == 3 ? 0x18 : 0x3c;
	for (uint i = 0; i < 0x0c && destinationOffset + i < _actorPathStepDeltas.size() &&
			0x18 + i < ARRAYSIZE(kActorPathStepDeltaTableSet5A); ++i)
		_actorPathStepDeltas[destinationOffset + i] = kActorPathStepDeltaTableSet5A[0x18 + i];
	requestedFacing = nextRegion == 3 ? 2 : 5;
	restoredStepDeltas = true;
	return true;
}

bool Scene5110::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	GameplayState &state = _vm->gameState();
	const bool applyAll = selector == 0xff;
	if (applyAll) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	}

	if ((selector == 1 || applyAll) &&
			(state.scene5110UnderwearTaken || hasInventoryItem(kScene5110UnderwearInventoryItem))) {
		state.scene5110UnderwearTaken = true;
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5110UnderwearSceneItem);
	}

	if (selector == 2 || applyAll) {
		if (hasInventoryItem(kScene5110BottleInventoryItem))
			state.scene5110BottleState = 2;
		if (state.scene5110BottleState < 2)
			replaceColorMapItemFromOriginal(17, kScene5110BottleSceneItem);
		if (state.scene5110BottleState != 0)
			copyStageSmallRow(17, kScene5110BottleSceneItem);
		if (state.scene5110BottleState == 2) {
			if (_sceneChunkTable.isValidChunk(5))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
			replaceColorMapItemFromOriginal(17, 0);
		}
	}

	if (selector == 3 || applyAll) {
		const bool transformed = state.scene5110SalonTransformState >= 2;
		if (!applyAll)
			memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		if (transformed) {
			const byte removedItems[] = { 7, 8, 9, 13, 14, 15, 16 };
			for (uint i = 0; i < ARRAYSIZE(removedItems); ++i)
				replaceColorMapItemFromOriginal(removedItems[i], 0);
		}

		if (_metadata.size() > kSceneItemFacing + kScene5110MirrorSceneItem) {
			_metadata[kSceneItemFacing + kScene5110BottleSceneItem] = transformed ? 5 : 2;
			_metadata[kSceneItemFacing + kScene5110MirrorSceneItem] = transformed ? 5 : 2;
		}
		const uint bottlePointOffset = kSceneItemInteractionPoints + kScene5110BottleSceneItem * 4;
		const uint mirrorPointOffset = kSceneItemInteractionPoints + kScene5110MirrorSceneItem * 4;
		if (transformed && _metadata.size() >= mirrorPointOffset + 4) {
			WRITE_LE_UINT16(&_metadata[bottlePointOffset], 0x1d6);
			WRITE_LE_UINT16(&_metadata[bottlePointOffset + 2], 0x106);
			WRITE_LE_UINT16(&_metadata[mirrorPointOffset], 0x20a);
			WRITE_LE_UINT16(&_metadata[mirrorPointOffset + 2], 0x104);
		}
	}

	if ((selector == 4 || applyAll) && state.scene5110ElevatorTransitionSeen &&
			_metadata.size() > kSceneItemDefaultStrip + 4) {
		_metadata[kSceneItemDefaultStrip + 2] = 5;
		_metadata[kSceneItemDefaultStrip + 3] = 5;
		_metadata[kSceneItemDefaultStrip + 4] = 5;
	}

	if ((selector == 5 || applyAll) &&
			(state.scene5110MirrorTaken || hasInventoryItem(kScene5110MirrorInventoryItem))) {
		state.scene5110MirrorTaken = true;
		if (_sceneChunkTable.isValidChunk(13))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5110MirrorSceneItem);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	updateElevatorButtonActionTargets(false);
	return true;
}

bool Scene5110::shouldConvertSavedFramebufferFF() const {
	return true;
}

bool Scene5110::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene5110::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene5110::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = 0x28;
	profile.soundVolumePercent = 15;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

byte Scene5110::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 6;
}

byte Scene5110::primarySpeechAnimationFrameCount(byte animationGroup) const {
	(void)animationGroup;
	return 5;
}

uint32 Scene5110::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene5110SpeechFrameMillis;
}

void Scene5110::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_sceneLayers.setLayerFrame(kScene5110WerewolfLayer, frameIndex);
}

void Scene5110::primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	_sceneLayers.setLayerFrame(kScene5110WerewolfLayer, baseFrame);
	_werewolfState = 0;
}

void Scene5110::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	_sceneLayers.setLayerFrame(kScene5110WerewolfLayer, baseFrame);
	_werewolfState = 0;
}

void Scene5110::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	switch (hookId) {
	case kScene5110PatchUnderwear:
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
		break;
	case kScene5110PatchBottle:
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
		break;
	case kScene5110PatchMirror:
		if (_sceneChunkTable.isValidChunk(13))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
		break;
	default:
		break;
	}
}

void Scene5110::handleLeftClick(const GameplayLoopCursorState &state) {
	updateElevatorButtonActionTargets(state.currentStrip == 5);
	PlayableScene::handleLeftClick(state);
}

bool Scene5110::runEntryElevatorSequence(bool alternateEntry) {
	const byte *entryFrames = alternateEntry ?
		kScene5110AlternateElevatorFrameMap : kScene5110FirstElevatorFrameMap;
	const uint entryFrameCount = alternateEntry ?
		ARRAYSIZE(kScene5110AlternateElevatorFrameMap) : ARRAYSIZE(kScene5110FirstElevatorFrameMap);

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_sceneLayers.setLayerVisible(kScene5110ElevatorLayer,
		_sceneChunkTable.isValidChunk(22));
	_sceneLayers.resetLayer(kScene5110ElevatorLayer, entryFrames[0]);
	drawPlayableComposite();
	if (fadePaletteFromBlack()) {
		_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
		_hideActiveActor = previousHideActiveActor;
		return false;
	}

	if (!playElevatorSequence(entryFrames, entryFrameCount, false)) {
		_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
		_hideActiveActor = previousHideActiveActor;
		return false;
	}
	if (_sceneChunkTable.isValidChunk(23))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[23], _baseFramebuffer);
	setActiveActorPose(0x084, 0x169, 2);
	if (!holdElevatorFrame(true)) {
		_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
		_hideActiveActor = previousHideActiveActor;
		return false;
	}

	_soundBank0.playSample(0x1d, 100, true);
	const bool completed = playElevatorSequence(kScene5110ElevatorInnerFrameMap,
		ARRAYSIZE(kScene5110ElevatorInnerFrameMap), false);
	_soundBank0.stop();
	_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
	_hideActiveActor = previousHideActiveActor;
	drawPlayableComposite();
	presentFrame();
	return completed;
}

bool Scene5110::playElevatorSequence(const byte *frameMap, uint frameMapSize, bool drawActor) {
	if (!_sceneChunkTable.isValidChunk(22) || frameMap == nullptr || frameMapSize == 0)
		return true;

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = !drawActor;
	_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, true);
	AnimationFrameRange range(0, frameMapSize - 1, kScene5110ElevatorFrameMillis);
	range.frameOrder = frameMap;
	const bool completed = playAndPresentAnimationFrames(kScene5110ElevatorLayer,
		range.unskippable().noFinalFrameDelay());
	_hideActiveActor = previousHideActiveActor;
	return completed;
}

bool Scene5110::holdElevatorFrame(bool drawActor) {
	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = !drawActor;
	drawPlayableComposite();
	presentFrame();
	const bool interrupted = waitSceneMillis(kScene5110ElevatorHoldMillis, false);
	_hideActiveActor = previousHideActiveActor;
	return !interrupted;
}

void Scene5110::runButtonExitToState(uint16 nextState) {
	_soundBank0.playSample(0x1d, 100, true);
	if (!playElevatorSequence(kScene5110ElevatorCloseFrameMap,
			ARRAYSIZE(kScene5110ElevatorCloseFrameMap), true) || !holdElevatorFrame(false)) {
		_soundBank0.stop();
		_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
		return;
	}
	if (_sceneChunkTable.isValidChunk(24))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[24], _baseFramebuffer);
	_soundBank0.playSample(0x1e, 100, true);
	const byte *exitFrames = nextState == kScene5120FirstState ?
		kScene5110ElevatorUpperFrameMap : kScene5110ElevatorLowerFrameMap;
	const uint exitFrameCount = nextState == kScene5120FirstState ?
		ARRAYSIZE(kScene5110ElevatorUpperFrameMap) : ARRAYSIZE(kScene5110ElevatorLowerFrameMap);
	if (!playElevatorSequence(exitFrames, exitFrameCount, false)) {
		_soundBank0.stop();
		_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
		return;
	}
	_vm->gameState().scene5110ElevatorTransitionSeen = true;
	applySceneStateToHotspotsAndPatches(4);
	_vm->gameState().mainFlowStateId = nextState;
}

void Scene5110::runButtonReturnSequence() {
	_soundBank0.playSample(0x1d, 100, true);
	if (!playElevatorSequence(kScene5110ElevatorCloseFrameMap,
			ARRAYSIZE(kScene5110ElevatorCloseFrameMap), true) || !holdElevatorFrame(true) ||
			!playElevatorSequence(kScene5110ElevatorInnerFrameMap,
				ARRAYSIZE(kScene5110ElevatorInnerFrameMap), true)) {
		_soundBank0.stop();
		_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
		return;
	}
	_soundBank0.stop();
	_sceneLayers.setLayerVisible(kScene5110ElevatorLayer, false);
	_vm->gameState().scene5110ElevatorTransitionSeen = true;
	applySceneStateToHotspotsAndPatches(4);
	setActiveActorPose(0x084, 0x169, 2);
	walkActiveActorTo(0x0b4, 0x174, 0xff, 0, false);
}

void Scene5110::runUnderwearPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110UnderwearTaken || hasInventoryItem(kScene5110UnderwearInventoryItem)) {
		beginSecondarySpeechLine(2, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(12, kScene5110PickupDescriptorCount,
		kScene5110PickupFrameMap, ARRAYSIZE(kScene5110PickupFrameMap), kScene5110FrameMillis)
		.hookAt(6, kScene5110PatchUnderwear)
		.noFinalFrameDelay());
	state.scene5110UnderwearTaken = true;
	applySceneStateToHotspotsAndPatches(1);
	addInventoryItem(kScene5110UnderwearInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene5110::runBottlePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110BottleState == 2 || hasInventoryItem(kScene5110BottleInventoryItem)) {
		beginSecondarySpeechLine(3, 2);
		return;
	}
	if (state.scene5110SalonTransformState < 2) {
		beginSecondarySpeechLine(3, 0);
		return;
	}
	if (state.scene5110BottleState == 0) {
		beginSecondarySpeechLine(4, 0);
	}

	beginSecondarySpeechLine(3, 1);
	runActorReplacement(ActionOverlaySpec(6, kScene5110PickupDescriptorCount,
		kScene5110PickupFrameMap, ARRAYSIZE(kScene5110PickupFrameMap), kScene5110FrameMillis)
		.hookAt(6, kScene5110PatchBottle)
		.noFinalFrameDelay());
	state.scene5110BottleState = 2;
	applySceneStateToHotspotsAndPatches(2);
	addInventoryItem(kScene5110BottleInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene5110::runWerewolfDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeWerewolfDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	if (!state.scene5110IntroSeen) {
		beginSecondarySpeechLine(kScene5110DialogueStageId, 0);
		beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, 0);
		state.scene5110IntroSeen = true;
	} else {
		beginSecondarySpeechLine(kScene5110DialogueStageId, 1);
		beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, 1);
	}

	_werewolfDialogueActive = true;
	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5110DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene5110DialogueStageId, 7);
			beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, 7);
			_werewolfDialogueActive = false;
			return;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			_werewolfDialogueActive = false;
			return;
		}

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene5110DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5110DialogueNoResponseFrame)
			beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, record.responseFrameIndex);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (record.disableAfterUse == 2 && state.scene5110JacuzziInspectionState == 0)
			state.scene5110JacuzziInspectionState = 1;

		if (applyWerewolfDialogueTransition(record, depthIndex, nodeIndex)) {
			_werewolfDialogueActive = false;
			return;
		}
	}
	_werewolfDialogueActive = false;
}

void Scene5110::runManicureGirlTalk() {
	beginSecondarySpeechLine(6, 0);
	beginSalonPrimarySpeechLine(6, 1);
}

void Scene5110::runJacuzziGirlTalk() {
	beginSecondarySpeechLine(6, 0);
	beginSalonPrimarySpeechLine(8, 0);
}

void Scene5110::runMirrorApproach() {
	if (_vm->gameState().scene5110SalonTransformState < 2) {
		beginSecondarySpeechLine(10, 0);
		return;
	}

	walkActiveActorTo(0x30d, 0x0f5, 0xff, 0, false);
	beginSecondarySpeechLine(10, 1);
	walkActiveActorTo(0x258, 0x101, 0xff, 0, false);
	beginSecondarySpeechLine(10, 2);
}

void Scene5110::runMirrorPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110MirrorTaken || hasInventoryItem(kScene5110MirrorInventoryItem)) {
		beginSecondarySpeechLine(13, 0);
		return;
	}
	if (state.scene5110SalonTransformState < 2) {
		beginSecondarySpeechLine(12, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(14, kScene5110PickupDescriptorCount,
		kScene5110PickupFrameMap, ARRAYSIZE(kScene5110PickupFrameMap), kScene5110FrameMillis)
		.hookAt(6, kScene5110PatchMirror)
		.noFinalFrameDelay());
	state.scene5110MirrorTaken = true;
	applySceneStateToHotspotsAndPatches(5);
	addInventoryItem(kScene5110MirrorInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene5110::runJacuzziInspection() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110JacuzziInspectionState == 0) {
		runJacuzziGirlTalk();
		return;
	}
	if (state.scene5110JacuzziInspectionState == 1) {
		beginSecondarySpeechLine(14, 0);
		beginSalonPrimarySpeechLine(14, 1);
		state.scene5110JacuzziInspectionState = 2;
		return;
	}
	beginSecondarySpeechLine(14, 2);
}

void Scene5110::runHairTreatmentTalk() {
	beginSecondarySpeechLine(6, 0);
	beginSalonPrimarySpeechLine(19, (byte)_random.getRandomNumber(1));
}

void Scene5110::beginSalonPrimarySpeechLine(uint16 rowIndex, byte frameIndex) {
	settleWerewolfForSpeech();
	beginPrimarySpeechLine(rowIndex, frameIndex, 599, 0x00b4, 0x17, 0x33, 0x2c);
}

void Scene5110::settleWerewolfForSpeech() {
	if (_werewolfState == 0)
		return;
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene5110WerewolfLayer);

	if (_werewolfState == 1) {
		layer.setFrame(2);
		_werewolfState = 2;
	} else if (_werewolfState == 3) {
		switch (layer.frameIndex) {
		case 11:
			layer.setFrame(5);
			break;
		case 12:
			layer.setFrame(4);
			break;
		case 13:
			layer.setFrame(3);
			break;
		default:
			layer.setFrame(2);
			break;
		}
		_werewolfState = 2;
	}

	while (_werewolfState != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(kScene5110FrameMillis, false))
			break;
	}
}

void Scene5110::initializeWerewolfDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5110WerewolfDialogueChoiceRecordCount);

	// Root choices for the werewolf in the spa.
	setWerewolfDialogueRecord(records, 0, 1, 0, kScene5110DialogueTransitionDown, 2, 2, 1);
	setWerewolfDialogueRecord(records, 1, 1, 0, kScene5110DialogueTransitionStay, 3, 3, 1);
	setWerewolfDialogueRecord(records, 2, 1, 0, kScene5110DialogueTransitionStay, 4, 4, 1);
	setWerewolfDialogueRecord(records, 3, 1, 0, kScene5110DialogueTransitionStay, 5, 5, 1);
	setWerewolfDialogueRecord(records, 4,
		_vm->gameState().scene5110WerewolfDialogueChoiceUnlocked ? 1 : 0,
		0, kScene5110DialogueTransitionStay, 6, 6, 2);
	setWerewolfDialogueRecord(records, 5, 1, 0, kScene5110DialogueTransitionEnd, 7, 7, 0);

	// Depth 1, node 0: follow-ups after asking why the werewolf is here.
	setWerewolfDialogueRecord(records, 70, 1, 0, kScene5110DialogueTransitionStay, 8, 8, 1);
	setWerewolfDialogueRecord(records, 71, 1, 0, kScene5110DialogueTransitionStay, 9, 9, 1);
	setWerewolfDialogueRecord(records, 72, 1, 0, kScene5110DialogueTransitionStay, 10, 10, 1);
	setWerewolfDialogueRecord(records, 73, 1, 0, kScene5110DialogueTransitionUp, 11, 11, 0);
}

void Scene5110::setWerewolfDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = enabled;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = 0xff;
	record.selectable = 1;
}

bool Scene5110::applyWerewolfDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) {
	const byte previousDepth = depthIndex;
	switch (record.transitionMode) {
	case kScene5110DialogueTransitionEnd:
		return true;
	case kScene5110DialogueTransitionDown:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth + 1;
		break;
	case kScene5110DialogueTransitionUp:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth == 0 ? 0 : (byte)(previousDepth - 1);
		break;
	case kScene5110DialogueTransitionUpTwo:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
		break;
	case kScene5110DialogueTransitionStay:
	default:
		break;
	}

	return false;
}

void Scene5110::initializeSalonLayers() {
	_sceneLayers.configure(kScene5110LayerSpecs);
	_realtimeAnimationTracks.reset(_randomDetailTrack);
	for (uint layerId = kScene5110RandomDetailLayer;
			layerId < kScene5110ElevatorLayer; ++layerId) {
		_sceneLayers.setLayerVisible(layerId,
			_sceneChunkTable.isValidChunk(_sceneLayers.layer(layerId).chunkIndex));
	}
	_salonAnimationChannel.reset(0, kScene5110FrameMillis);
	_upperRightSalonState = 1;
	_rightSalonDetailDirection = 0;
	_rightSalonDetailDwell = 0;
	_lowerSalonDetailState = 1;
	_leftSalonRepeatCount = 0;
	_werewolfState = 1;
	_centerSalonDetailRepeatCount = 0;
	_centerSalonToggleA = false;
	_centerSalonToggleB = false;
	_centerSalonDetailSequenceActive = false;
	_werewolfDialogueActive = false;
}

void Scene5110::advanceSalonAnimations(uint32 delta) {
	const uint tickCount = _salonAnimationChannel.consumeFrames(delta);
	for (uint tick = 0; tick < tickCount; ++tick)
		advanceSalonAnimationTick();
}

void Scene5110::advanceSalonAnimationTick() {
	if (_vm->gameState().scene5110SalonTransformState >= 2)
		return;

	advanceUpperRightSalonTick();
	advanceRightSalonDetailTick();
	advanceLeftSalonTick();
	advanceLowerSalonDetailTick();
	advanceCenterSalonTick();
	if (_primaryDialogueSpeechActive) {
		// The shared speech controller owns the werewolf frame while he talks.
	} else if (_werewolfDialogueActive) {
		advanceWerewolfDialogueTick();
	} else {
		advanceWerewolfTick(!_speechOverlay.visible &&
			!_sceneLayers.layerVisible(kScene5110ElevatorLayer));
	}
	advanceCenterSalonDetailTick();
}

void Scene5110::advanceUpperRightSalonTick() {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene5110UpperRightSalonLayer);
	switch (_upperRightSalonState) {
	case 0:
		if (_random.getRandomNumber(19) == 0) {
			layer.setFrame(5);
			_upperRightSalonState = 3;
		}
		break;
	case 1:
		if (layer.frameIndex != 0) {
			layer.setFrame(0);
		} else if (_random.getRandomNumber(14) == 0) {
			layer.setFrame(1);
		} else if (_random.getRandomNumber(24) == 0) {
			layer.setFrame(2);
			_upperRightSalonState = 2;
		}
		break;
	case 2:
		if (layer.frameIndex > 3)
			_upperRightSalonState = 0;
		else
			layer.setFrame(layer.frameIndex + 1);
		break;
	case 3:
		if (layer.frameIndex > 7) {
			layer.setFrame(0);
			_upperRightSalonState = 1;
		} else {
			layer.setFrame(layer.frameIndex + 1);
		}
		break;
	default:
		_upperRightSalonState = 1;
		layer.setFrame(0);
		break;
	}
}

void Scene5110::advanceRightSalonDetailTick() {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene5110RightSalonDetailLayer);
	if (_rightSalonDetailDirection == 2) {
		if (_rightSalonDetailDwell != 0 && layer.frameIndex < 6) {
			layer.setFrame(layer.frameIndex + 1);
			--_rightSalonDetailDwell;
		} else {
			layer.setFrame(layer.frameIndex - 1);
			_rightSalonDetailDirection = 3;
			_rightSalonDetailDwell = (byte)(_random.getRandomNumber(5) + 1);
		}
		return;
	}

	if (_rightSalonDetailDwell != 0 && layer.frameIndex != 0) {
		layer.setFrame(layer.frameIndex - 1);
		--_rightSalonDetailDwell;
	} else {
		layer.setFrame(layer.frameIndex + 1);
		_rightSalonDetailDirection = 2;
		_rightSalonDetailDwell = (byte)(_random.getRandomNumber(5) + 1);
	}
}

void Scene5110::advanceLowerSalonDetailTick() {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene5110LowerSalonDetailLayer);
	switch (_lowerSalonDetailState) {
	case 0:
		if (_random.getRandomNumber(24) == 0) {
			layer.setFrame(layer.frameIndex + 1);
			_lowerSalonDetailState = 3;
		}
		break;
	case 1:
		if (_random.getRandomNumber(24) == 0) {
			layer.setFrame(1);
			_lowerSalonDetailState = 2;
		}
		break;
	case 2:
		if (layer.frameIndex > 6)
			_lowerSalonDetailState = 0;
		else
			layer.setFrame(layer.frameIndex + 1);
		break;
	case 3:
		if (layer.frameIndex > 11) {
			layer.setFrame(0);
			_lowerSalonDetailState = 1;
		} else {
			layer.setFrame(layer.frameIndex + 1);
		}
		break;
	default:
		_lowerSalonDetailState = 1;
		layer.setFrame(0);
		break;
	}
}

void Scene5110::advanceCenterSalonTick() {
	if (_centerSalonToggleA)
		_centerSalonToggleA = false;
	else if (_random.getRandomNumber(14) == 0)
		_centerSalonToggleA = true;

	if (_centerSalonToggleB)
		_centerSalonToggleB = false;
	else if (_random.getRandomNumber(14) == 0)
		_centerSalonToggleB = true;

	byte frameIndex = _centerSalonToggleB ? 0 : 1;
	if (_centerSalonToggleA)
		frameIndex = _centerSalonToggleB ? 3 : 4;
	_sceneLayers.setLayerFrame(kScene5110CenterSalonLayer, frameIndex);
}

void Scene5110::advanceLeftSalonTick() {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene5110LeftSalonLayer);
	if (_leftSalonRepeatCount != 0) {
		if (layer.frameIndex < 16) {
			layer.setFrame(layer.frameIndex + 1);
		} else {
			--_leftSalonRepeatCount;
			layer.setFrame(_leftSalonRepeatCount != 0 ? 1 : 0);
		}
		return;
	}

	if (_random.getRandomNumber(49) == 0) {
		layer.setFrame(1);
		_leftSalonRepeatCount = (byte)(_random.getRandomNumber(4) + 1);
	}
}

void Scene5110::advanceWerewolfTick(bool allowLongGesture) {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene5110WerewolfLayer);
	switch (_werewolfState) {
	case 0:
		if (layer.frameIndex == 10) {
			layer.setFrame(6);
		} else if (_random.getRandomNumber(14) == 0) {
			layer.setFrame(10);
		} else if (allowLongGesture && _random.getRandomNumber(39) == 0) {
			layer.setFrame(11);
			_werewolfState = 3;
		}
		break;
	case 1:
		if (layer.frameIndex == 1) {
			layer.setFrame(0);
		} else if (_random.getRandomNumber(14) == 0) {
			layer.setFrame(1);
		} else if (_random.getRandomNumber(39) == 0) {
			layer.setFrame(2);
			_werewolfState = 2;
		}
		break;
	case 2:
		if (layer.frameIndex > 4) {
			layer.setFrame(6);
			_werewolfState = 0;
		} else {
			layer.setFrame(layer.frameIndex + 1);
		}
		break;
	case 3:
		if (layer.frameIndex < 15) {
			layer.setFrame(layer.frameIndex + 1);
		} else {
			layer.setFrame(0);
			_werewolfState = 1;
		}
		break;
	default:
		_werewolfState = 1;
		layer.setFrame(0);
		break;
	}
}

void Scene5110::advanceWerewolfDialogueTick() {
	_werewolfState = 0;
	if (_sceneLayers.layerFrame(kScene5110WerewolfLayer) != 6)
		_sceneLayers.setLayerFrame(kScene5110WerewolfLayer, 6);
	else if (_random.getRandomNumber(14) == 0)
		_sceneLayers.setLayerFrame(kScene5110WerewolfLayer, 10);
}

void Scene5110::advanceCenterSalonDetailTick() {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene5110CenterSalonDetailLayer);
	if (!_centerSalonDetailSequenceActive) {
		if (_random.getRandomNumber(49) == 0) {
			_centerSalonDetailSequenceActive = true;
			layer.setFrame(1);
			_centerSalonDetailRepeatCount = (byte)(_random.getRandomNumber(4) + 1);
		}
		return;
	}

	if (layer.frameIndex < 7) {
		layer.setFrame(layer.frameIndex + 1);
		return;
	}

	layer.setFrame(0);
	--_centerSalonDetailRepeatCount;
	if (_centerSalonDetailRepeatCount == 0)
		_centerSalonDetailSequenceActive = false;
}

void Scene5110::drawNormalRoomLayers(int activeWorldY) {
	drawSceneLayer(kScene5110RandomDetailLayer);
	if (_vm->gameState().scene5110SalonTransformState < 2) {
		drawSceneLayer(kScene5110UpperRightSalonLayer);
		drawSceneLayer(kScene5110RightStaticSalonLayer);
		drawSceneLayer(kScene5110CenterSalonLayer);
		const byte werewolfFrame = _sceneLayers.layerFrame(kScene5110WerewolfLayer);
		if (werewolfFrame >= 3 && werewolfFrame <= 13) {
			drawSceneLayer(kScene5110WerewolfLayer);
			drawSceneLayer(kScene5110RightSalonDetailLayer);
		} else {
			drawSceneLayer(kScene5110RightSalonDetailLayer);
			drawSceneLayer(kScene5110WerewolfLayer);
		}
		drawSceneLayer(kScene5110CenterSalonDetailLayer);
		if (_sceneChunkTable.isValidChunk(20))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[20], _sceneFramebuffer);
		if (activeWorldY > 0x109 && _sceneChunkTable.isValidChunk(21))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[21], _sceneFramebuffer);
		drawSceneLayer(kScene5110LeftSalonLayer);
		drawSceneLayer(kScene5110LowerSalonDetailLayer);
		return;
	}

	if (_sceneChunkTable.isValidChunk(20))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[20], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(25))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[25], _sceneFramebuffer);
	if (activeWorldY > 0x109 && _sceneChunkTable.isValidChunk(21))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[21], _sceneFramebuffer);
}

void Scene5110::drawSalonSpriteLayers() {
	drawSceneLayer(kScene5110RandomDetailLayer);
	if (_vm->gameState().scene5110SalonTransformState >= 2)
		return;

	drawSceneLayer(kScene5110UpperRightSalonLayer);
	drawSceneLayer(kScene5110RightStaticSalonLayer);
	drawSceneLayer(kScene5110CenterSalonLayer);
	drawSceneLayer(kScene5110RightSalonDetailLayer);
	drawSceneLayer(kScene5110WerewolfLayer);
	drawSceneLayer(kScene5110CenterSalonDetailLayer);
	drawSceneLayer(kScene5110LeftSalonLayer);
	drawSceneLayer(kScene5110LowerSalonDetailLayer);
}

void Scene5110::drawElevatorComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY) {
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_sceneChunkTable.isValidChunk(20))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[20], _sceneFramebuffer);
	drawSceneLayer(kScene5110ElevatorLayer);
	drawSalonSpriteLayers();
}

void Scene5110::updateElevatorButtonActionTargets(bool useStrip) {
	ScenePoint interactionPoint;
	interactionPoint.x = useStrip ? 0x084 : 0x0b4;
	interactionPoint.y = useStrip ? 0x169 : 0x174;
	const byte facing = useStrip ? 2 : 5;
	_hotspots.setActionInteraction(2, interactionPoint, facing);
	_hotspots.setActionInteraction(3, interactionPoint, facing);
	_hotspots.setActionInteraction(4, interactionPoint, facing);
}

void Scene5110::initializeSceneItemLabels() {
	copyStageSmallRow(13, 17);
	if (_vm->gameState().scene5110SalonTransformState < 2) {
		for (byte row = 13; row <= 16; ++row)
			copyStageSmallRow(9, row);
	}
	if (_vm->gameState().scene5110BottleState != 0)
		copyStageSmallRow(17, kScene5110BottleSceneItem);
}

void Scene5110::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene5110::clearSceneItemFromColorMap(byte itemId) {
	replaceColorMapItemFromOriginal(itemId, 0);
}

void Scene5110::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == sourceItem)
			_paletteMask[kSceneColorToItemMap + color] = destinationItem;
	}
}

void Scene5110::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	const bool transformed = _vm->gameState().scene5110SalonTransformState >= 2;
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		const byte region = _walkablePaletteMask[i];
		if (region > 20 || region == 2 || region == 5 || (!transformed && region == 4))
			_walkablePaletteMask[i] = 0;
	}
}

} // End of namespace Hollywood
