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

#include "hollywood/scenes/playable/scene2100.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene2010PatchedEntryState = 0x07dc;
const uint16 kScene2100EntryState = 0x0834;
const uint16 kScene2100ReturnFromTreasureState = 0x0835;
const uint16 kScene2100LeftPassageState = 0x0836;
const uint16 kScene2110EntryState = 0x083e;
const uint16 kScene2110ScriptedReturnState = 0x083f;
const uint16 kScene2100ViewportXOffset = 0x00c8;
const uint16 kScene2100ViewportMaxXOffset = 0x0108;
const uint kScene2100ActorBankTableEntry = 0x0000;
const uint kScene2100ActorPaletteTableEntry = 0x00cc;
const uint kScene2100Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2100ForegroundFrameMillis = 85;
const uint32 kScene2100PrimarySpeechFrameMillis = 125;
const uint32 kScene2100AuxFrameMillis = 75;
const uint32 kScene2100ActorPathFrameMillis = 75;
const byte kScene2100InvalidFacing = 0xff;
const byte kScene2100InvalidCel = 0xff;
const byte kScene2100MummyChunk = 5;
const byte kScene2100MummyDescriptorCount = 0x0d;
const byte kScene2100ReturnForegroundChunk = 6;
const byte kScene2100ReturnForegroundDescriptorCount = 6;
const byte kScene2100PrincessArrivalChunk = 7;
const byte kScene2100PrincessArrivalDescriptorCount = 0x1b;
const byte kScene2100PickupChunk = 11;
const byte kScene2100PickupDescriptorCount = 0x0e;
const byte kScene2100PassageChunk = 12;
const byte kScene2100PassageDescriptorCount = 9;
const byte kScene2100SpecialSpeechChunk = 14;
const byte kScene2100SpecialSpeechDescriptorCount = 0x2d;
const byte kScene2100SpecialExitChunk = 15;
const byte kScene2100SpecialExitDescriptorCount = 0x1b;
const byte kScene2100DoorChunk = 16;
const byte kScene2100DoorDescriptorCount = 0x2f;
const byte kScene2100RaStaffItem = 0x2f;
const byte kScene2100MummyDialogueStageId = 0x62;
const byte kScene2100MummyPrimaryRow = 99;
const uint kScene2100MummyDialogueChoiceRecordCount = 10 * 10 * 7;

// These are the choices whose disabled state survives the trip through scene 2110.
const uint16 kScene2100MummyDialogueTrackedRecordIndices[] = {
	0, 1, 2, 3, 4, 70, 77, 84, 85, 86, 87, 91, 140, 210, 280
};

const byte kScene2100ForegroundFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
	9, 10, 11, 12, 7, 6, 5, 0
};

const byte kScene2100PrincessArrivalFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
	9, 10, 11, 12, 8, 7, 6, 5, 0, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26
};

const byte kScene2100ReturnForegroundFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5
};

const byte kScene2100PassageFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8
};

const byte kScene2100PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13
};

const byte kScene2100TransitionFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 11, 8, 9,
	10, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
	44, 45, 46
};

const byte kScene2100SpecialIntroFrameMap[] = {
	19, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21
};

const byte kScene2100SpecialSpeechShortFrameMap[] = {
	22, 22, 23, 24, 25, 26, 27, 28, 29
};

const byte kScene2100SpecialSpeechLongFrameMap[] = {
	24, 24, 25, 26, 27, 30, 31, 32, 33, 34,
	35, 36, 37, 38, 39, 40, 41, 42, 43, 44
};

const byte kScene2100SpecialExitFrameMap[] = {
	1, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26
};

static_assert(ARRAYSIZE(kScene2100ForegroundFrameMap) == 18,
	"Scene 2100 foreground frame map size changed");
static_assert(ARRAYSIZE(kScene2100PrincessArrivalFrameMap) == 0x21,
	"Scene 2100 princess-arrival frame map size changed");
static_assert(ARRAYSIZE(kScene2100ReturnForegroundFrameMap) == 7,
	"Scene 2100 return foreground frame map size changed");
static_assert(ARRAYSIZE(kScene2100PassageFrameMap) == 10,
	"Scene 2100 passage frame map size changed");
static_assert(ARRAYSIZE(kScene2100PickupFrameMap) == 15,
	"Scene 2100 pickup frame map size changed");
static_assert(ARRAYSIZE(kScene2100TransitionFrameMap) == 0x35,
	"Scene 2100 transition frame map size changed");
static_assert(kScene2100DoorDescriptorCount > 46,
	"Scene 2100 transition descriptor count is too small");
static_assert(ARRAYSIZE(kScene2100SpecialIntroFrameMap) == 0x17,
	"Scene 2100 special intro frame map size changed");
static_assert(ARRAYSIZE(kScene2100SpecialSpeechShortFrameMap) == 9,
	"Scene 2100 short speech frame map size changed");
static_assert(ARRAYSIZE(kScene2100SpecialSpeechLongFrameMap) == 20,
	"Scene 2100 long speech frame map size changed");
static_assert(ARRAYSIZE(kScene2100SpecialExitFrameMap) == 0x1b,
	"Scene 2100 special exit frame map size changed");
static_assert(ARRAYSIZE(kScene2100MummyDialogueTrackedRecordIndices) <= 16,
	"Scene 2100 mummy dialogue choice mask is too small");

const SceneLayerSpec kScene2100LayerSpecs[] = {
	{kSceneAnimationBehindActors, kScene2100MummyChunk, kScene2100MummyDescriptorCount,
		kScene2100ForegroundFrameMap, ARRAYSIZE(kScene2100ForegroundFrameMap), true, 0},
	{kSceneAnimationBehindActors, kScene2100PassageChunk, kScene2100PassageDescriptorCount,
		kScene2100PassageFrameMap, ARRAYSIZE(kScene2100PassageFrameMap), false, 0},
	{kSceneAnimationInFrontOfActors, kScene2100DoorChunk, kScene2100DoorDescriptorCount,
		kScene2100TransitionFrameMap, ARRAYSIZE(kScene2100TransitionFrameMap), false, 0}
};

static uint16 mummyDialogueChoiceBit(uint recordIndex) {
	for (uint bit = 0; bit < ARRAYSIZE(kScene2100MummyDialogueTrackedRecordIndices); ++bit) {
		if (kScene2100MummyDialogueTrackedRecordIndices[bit] == recordIndex)
			return (uint16)(1 << bit);
	}

	return 0;
}

static PlayableSceneConfig scene2100Config() {
	PlayableSceneConfig config(2100,
		SceneResourceLayout(17, 5, 16),
		SceneViewport(kScene2100ViewportXOffset, kScene2100ViewportXOffset, kScene2100ViewportMaxXOffset),
		SceneActorPose(0x277, 0x168, 1));
	config.setActorResources(kScene2100ActorBankTableEntry, kScene2100ActorPaletteTableEntry);
	config.setTextResources(kScene2100Resource003RowsOffsetIndex, kScene2100SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	return config;
}

Scene2100::Scene2100(HollywoodEngine *vm) :
		PlayableScene(vm, scene2100Config()),
		_foregroundChannel(),
		_auxChannel(),
		_foregroundAlternateFrameSet(false),
		_mummySpeechUsesFrontLayer(false),
		_suppressMummySpeechAnimation(false),
		_returnLayerAnimationActive(false),
		_mummyIdleEnabled(false),
		_doorCeremonyAnimationActive(false),
		_doorCeremonyFinishing(false),
		_doorCeremonyState(0),
		_specialSpeechAnimationActive(false),
		_specialSpeechFinishing(false),
		_specialSpeechVariant(0) {
	_sceneLayers.configure(kScene2100LayerSpecs);
}

void Scene2100::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	const GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene2100ReturnFromTreasureState) {
		_activeActorWorldX = 0x1d0;
		_activeActorWorldY = 0x14c;
		_activeActorFacing = 2;
		_activeActorCel = 5;
	} else if (state.mainFlowStateId == kScene2100LeftPassageState) {
		_activeActorWorldX = 0x107;
		_activeActorWorldY = 0x120;
		_activeActorFacing = 2;
		_activeActorCel = 0;
	} else {
		_activeActorWorldX = 0x3b0;
		_activeActorWorldY = 0x1ba;
		_activeActorFacing = 4;
		_activeActorCel = 0;
	}

	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene2100::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene2100ReturnFromTreasureState:
		runEntryFromScene2110();
		break;
	case kScene2100LeftPassageState:
		runEntryFromLeftPassage();
		break;
	default:
		runEntryFromScene2010();
		break;
	}
}

bool Scene2100::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene2100::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene2100::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

void Scene2100::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene2100::advanceCustomGameplayLoop(uint32 delta) {
	if (_returnLayerAnimationActive)
		advanceReturnLayers(delta);
	else if (_doorCeremonyAnimationActive)
		advanceDoorCeremony(delta);
	else if (_specialSpeechAnimationActive)
		advanceSpecialSpeechAnimation(delta);
	else
		advanceMummyIdle(delta);
}

bool Scene2100::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta de piedra (look at stone door): unusual carving.
		beginSecondarySpeechLine(0, 0);
		return true;
	case 302: // Usar/abrir puerta de piedra (use/open stone door): it cannot be opened normally.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Ir al exterior de la pirámide (go to pyramid exterior): return to scene 2010.
		_vm->gameState().mainFlowStateId = kScene2010PatchedEntryState;
		return true;
	case 304: // Mirar estantería (look at bookshelf): Egypt books and one odd novel.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Mirar sillón (look at armchair): comfortable armchair.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 306: // Usar sillón (use armchair): Ron refuses to rest now.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 142: // Usar bastón de Rá (use Ra staff): inventory callback redirected here in scene 2100.
	case 307: // Usar bastón de Rá (use Ra staff): open the passage to the treasure room.
		runStoneDoorToTreasureRoom();
		return true;
	case 308: // Coger bastón de Rá (take Ra staff): grants the Ra staff.
		runRaStaffPickup();
		return true;
	default:
		return false;
	}
}

bool Scene2100::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MAX<int>(targetX, 0x118);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene2100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;

	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (isRaStaffAvailable() && _sceneChunkTable.isValidChunk(10)) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
	} else {
		if (_sceneChunkTable.isValidChunk(9))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		removeColorMapItemFromOriginal(5);
	}

	const uint16 threshold = state.scene2100PassageOpen ? 0x145 : 10;
	if (_actorDepthYThresholds.size() > 2)
		_actorDepthYThresholds[2] = threshold;
	if (_drawActorDepthYThresholds.size() > 2)
		_drawActorDepthYThresholds[2] = threshold;

	const byte passagePatchChunk = state.scene2100PassageOpen ? 13 : 8;
	if (_sceneChunkTable.isValidChunk(passagePatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[passagePatchChunk], _baseFramebuffer);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

byte Scene2100::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return _foregroundAlternateFrameSet ? 9 : 0;
}

uint32 Scene2100::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene2100PrimarySpeechFrameMillis;
}

void Scene2100::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	if (_suppressMummySpeechAnimation)
		return;

	ResourceSpriteLayer &layer = _mummySpeechUsesFrontLayer ? frontLayer() : mummyLayer();
	layer.visible = _mummySpeechUsesFrontLayer ||
		_vm->gameState().scene2100MummyBranchState != 1;
	layer.setFrame(frameIndex);
}

void Scene2100::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	if (_suppressMummySpeechAnimation)
		return;

	ResourceSpriteLayer &layer = _mummySpeechUsesFrontLayer ? frontLayer() : mummyLayer();
	layer.visible = _mummySpeechUsesFrontLayer ||
		_vm->gameState().scene2100MummyBranchState != 1;
	layer.setFrame(baseFrame);
}

AmbientAudioProfile Scene2100::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2100::resetAnimationLayers() {
	_sceneLayers.configure(kScene2100LayerSpecs);
	_sceneLayers.setLayerVisible(kMummyLayer,
		_vm->gameState().scene2100MummyBranchState != 1);
	_foregroundChannel.reset(0, kScene2100PrimarySpeechFrameMillis);
	_auxChannel.reset(0, kScene2100AuxFrameMillis);
	_foregroundAlternateFrameSet = false;
	_mummySpeechUsesFrontLayer = false;
	_suppressMummySpeechAnimation = false;
	_returnLayerAnimationActive = false;
	_mummyIdleEnabled = _vm->gameState().scene2020PrincessGone &&
		_vm->gameState().scene2100MummyBranchState == 0;
	_doorCeremonyAnimationActive = false;
	_doorCeremonyFinishing = false;
	_doorCeremonyState = 0;
	_specialSpeechAnimationActive = false;
	_specialSpeechFinishing = false;
	_specialSpeechVariant = 0;
}

void Scene2100::advanceReturnLayers(uint32 delta) {
	const uint foregroundFrames = _foregroundChannel.consumeFrames(delta);
	for (uint i = 0; i < foregroundFrames; ++i)
		_sceneLayers.advanceLayerFrame(kFrontLayer, 6);

	const uint auxFrames = _auxChannel.consumeFrames(delta);
	for (uint i = 0; i < auxFrames; ++i)
		_sceneLayers.advanceLayerFrame(kAuxLayer, 9);

	if (_sceneLayers.layerFrame(kFrontLayer) >= 6 &&
			_sceneLayers.layerFrame(kAuxLayer) >= 9 && !concurrentActorPathActive())
		_returnLayerAnimationActive = false;
}

void Scene2100::advanceMummyIdle(uint32 delta) {
	ResourceSpriteLayer &layer = _mummySpeechUsesFrontLayer ? frontLayer() : mummyLayer();
	if (!_mummyIdleEnabled || _primaryDialogueSpeechActive || !layer.visible)
		return;

	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		const byte baseFrame = _foregroundAlternateFrameSet ? 9 : 0;
		const byte idleFrame = _foregroundAlternateFrameSet ? 13 : 4;
		if (layer.frameIndex == idleFrame)
			layer.setFrame(baseFrame);
		else if (_random.getRandomNumber(14) == 0)
			layer.setFrame(idleFrame);
	}
}

void Scene2100::advanceDoorCeremony(uint32 delta) {
	const uint frameCount = _auxChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _doorCeremonyAnimationActive; ++i) {
		switch (_doorCeremonyState) {
		case 0:
			if (_sceneLayers.layerFrame(kFrontLayer) < 29) {
				_sceneLayers.advanceLayerFrame(kFrontLayer, 29);
			} else {
				_doorCeremonyState = 2;
				_sceneLayers.setLayerFrame(kFrontLayer, 4);
			}
			break;
		case 1:
			if (_sceneLayers.layerFrame(kFrontLayer) < 11) {
				_sceneLayers.advanceLayerFrame(kFrontLayer, 11);
			} else {
				_doorCeremonyState = 3;
				_sceneLayers.setLayerFrame(kFrontLayer, 12);
			}
			break;
		case 2:
			if (_sceneLayers.layerFrame(kFrontLayer) < 8) {
				_sceneLayers.advanceLayerFrame(kFrontLayer, 8);
			} else if (_doorCeremonyFinishing) {
				_doorCeremonyAnimationActive = false;
			} else if (_random.getRandomBit() == 0) {
				_sceneLayers.setLayerFrame(kFrontLayer, 4);
			} else {
				_doorCeremonyState = 1;
				_sceneLayers.setLayerFrame(kFrontLayer, 9);
			}
			break;
		case 3:
			if (_sceneLayers.layerFrame(kFrontLayer) < 25) {
				_sceneLayers.advanceLayerFrame(kFrontLayer, 25);
			} else if (_doorCeremonyFinishing) {
				_doorCeremonyAnimationActive = false;
			} else if (_random.getRandomBit() == 0) {
				_sceneLayers.setLayerFrame(kFrontLayer, 12);
			} else {
				_doorCeremonyState = 0;
				_sceneLayers.setLayerFrame(kFrontLayer, 26);
			}
			break;
		default:
			_doorCeremonyState = 2;
			_sceneLayers.setLayerFrame(kFrontLayer, 4);
			break;
		}
	}
}

void Scene2100::advanceSpecialSpeechAnimation(uint32 delta) {
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _specialSpeechAnimationActive; ++i) {
		const byte lastFrame = _specialSpeechVariant == 0 ?
			ARRAYSIZE(kScene2100SpecialSpeechShortFrameMap) - 1 :
			ARRAYSIZE(kScene2100SpecialSpeechLongFrameMap) - 1;
		if (_sceneLayers.layerFrame(kFrontLayer) < lastFrame) {
			_sceneLayers.advanceLayerFrame(kFrontLayer, lastFrame);
			if (_specialSpeechFinishing &&
					_sceneLayers.layerFrame(kFrontLayer) == lastFrame)
				_specialSpeechAnimationActive = false;
		} else if (_specialSpeechFinishing) {
			_specialSpeechAnimationActive = false;
		} else {
			_specialSpeechVariant = _random.getRandomBit();
			_sceneLayers.setLayerFrameMap(kFrontLayer,
				_specialSpeechVariant == 0 ? kScene2100SpecialSpeechShortFrameMap :
					kScene2100SpecialSpeechLongFrameMap,
				_specialSpeechVariant == 0 ? ARRAYSIZE(kScene2100SpecialSpeechShortFrameMap) :
					ARRAYSIZE(kScene2100SpecialSpeechLongFrameMap));
			_sceneLayers.setLayerFrame(kFrontLayer, 1);
		}
	}
}

bool Scene2100::isRaStaffAvailable() const {
	const GameplayState &state = _vm->gameState();
	return state.scene2020PrincessGone && !state.scene2100RaStaffTaken;
}

void Scene2100::runEntryFromScene2010() {
	GameplayState &state = _vm->gameState();
	const bool princessArrivalPending = state.scene2020PrincessGone &&
		state.scene2100MummyBranchState == 0;
	if (princessArrivalPending) {
		_sceneLayers.setLayerVisible(kMummyLayer, false);
		_sceneLayers.setLayerResource(kFrontLayer, kScene2100PrincessArrivalChunk,
			kScene2100PrincessArrivalDescriptorCount,
			kScene2100PrincessArrivalFrameMap,
			ARRAYSIZE(kScene2100PrincessArrivalFrameMap));
		_sceneLayers.setLayerVisible(kFrontLayer, true);
		_mummySpeechUsesFrontLayer = true;
	}

	if (state.scene2100MummyBranchState == 1) {
		runEntryPathWithFinalFacing(0x3b0, 0x1ba, 4, 0,
			0x2da, 0x19a, 5, 0);
	} else if (princessArrivalPending) {
		runEntryPathWithFinalFacing(0x3b0, 0x1ba, 4, 0,
			0x35e, 0x191, 5, 0);
	} else {
		runEntryPathWithFinalFacing(0x3b0, 0x1ba, 4, 0,
			0x277, 0x168, 1, 0);
	}

	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	if (state.scene2100MummyBranchState == 0) {
		if (princessArrivalPending) {
			runPrincessArrivalSequence();
		} else {
			const bool enterTreasureRoom = runMummyDialogue(true);
			if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
				return;
			if (enterTreasureRoom) {
				state.scene2100PassageOpen = true;
				state.mainFlowStateId = kScene2110ScriptedReturnState;
			} else if (state.mainFlowStateId == kScene2100EntryState) {
				runExitToScene2010(0x3b0, 0x1ba);
			}
		}
	} else if (state.scene2100MummyBranchState == 2) {
		runLateReturnSequence();
	}
}

void Scene2100::runEntryFromScene2110() {
	GameplayState &state = _vm->gameState();
	setActiveActorPose(0x1d0, 0x14c, 2, 5);
	_sceneLayers.setLayerVisible(kMummyLayer, false);
	_sceneLayers.setLayerResource(kFrontLayer, kScene2100ReturnForegroundChunk,
		kScene2100ReturnForegroundDescriptorCount,
		kScene2100ReturnForegroundFrameMap,
		ARRAYSIZE(kScene2100ReturnForegroundFrameMap));
	_sceneLayers.setLayerVisible(kFrontLayer, true);
	_sceneLayers.setLayerResource(kAuxLayer, kScene2100PassageChunk,
		kScene2100PassageDescriptorCount,
		kScene2100PassageFrameMap, ARRAYSIZE(kScene2100PassageFrameMap));
	_sceneLayers.setLayerVisible(kAuxLayer, true);
	_sceneLayers.setLayerStratum(kAuxLayer, kSceneAnimationBehindActors);
	_foregroundChannel.reset(0, kScene2100ForegroundFrameMillis);
	_auxChannel.reset(0, kScene2100AuxFrameMillis);
	startConcurrentActorPath(0x277, 0x168, 1, 0, kScene2100ActorPathFrameMillis);
	_returnLayerAnimationActive = true;

	drawPlayableComposite();
	presentFrame();
	while (_returnLayerAnimationActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			return;
	}
	finishConcurrentActorPath();
	_sceneLayers.setLayerVisible(kFrontLayer, false);
	_sceneLayers.setLayerVisible(kAuxLayer, false);
	_sceneLayers.setLayerVisible(kMummyLayer, true);
	_sceneLayers.resetLayer(kMummyLayer, 0);

	state.scene2100PassageOpen = false;
	applySceneStateToHotspotsAndPatches(4);
	(void)runMummyDialogue(false);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	if (state.mainFlowStateId == kScene2100ReturnFromTreasureState)
		runExitToScene2010(0x3b9, 0x1bb);
}

void Scene2100::runEntryFromLeftPassage() {
	GameplayState &state = _vm->gameState();
	runEntryPathWithFinalFacing(0x107, 0x120, 2, 0,
		0x1f0, 0x17c, kScene2100InvalidFacing, 0);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_sceneLayers.setLayerResource(kAuxLayer, kScene2100PassageChunk,
		kScene2100PassageDescriptorCount,
		kScene2100PassageFrameMap, ARRAYSIZE(kScene2100PassageFrameMap));
	_sceneLayers.setLayerVisible(kAuxLayer, true);
	_sceneLayers.setLayerStratum(kAuxLayer, kSceneAnimationInFrontOfActors);
	BlockingSequence sequence(*this);
	sequence.presentedLayerTransition(kAuxLayer,
		AnimationTransition(0, 9, 9, kScene2100AuxFrameMillis).unskippable());
	_sceneLayers.setLayerVisible(kAuxLayer, false);
	_sceneLayers.setLayerStratum(kAuxLayer, kSceneAnimationBehindActors);
	if (!sequence.completed())
		return;

	sequence.commit(state.scene2100PassageOpen, false)
		.framebufferPatch(4);
	drawPlayableComposite();
	presentFrame();
}

void Scene2100::runPrincessArrivalSequence() {
	GameplayState &state = _vm->gameState();
	_sceneLayers.setLayerVisible(kMummyLayer, false);
	_sceneLayers.setLayerVisible(kFrontLayer, true);
	_mummySpeechUsesFrontLayer = true;
	runPrincessArrivalPrimarySpeechLine(0);
	BlockingSequence sequence(*this);
	sequence.layerFrames(kFrontLayer,
		AnimationFrameRange(19, 31, kScene2100ForegroundFrameMillis)
			.unskippable()
			.actorPathAt(25, 0x281, 0x191, 2, 0,
				kScene2100ActorPathFrameMillis));
	if (!sequence.completed())
		return;
	_sceneLayers.setLayerFrame(kFrontLayer, 32);
	drawPlayableComposite();
	presentFrame();

	sequence.commit(state.scene2100MummyBranchState, (byte)1);
	_sceneLayers.setLayerVisible(kFrontLayer, false);
	_mummySpeechUsesFrontLayer = false;
	finishConcurrentActorPath();
	beginSecondarySpeechLine(7, 1);
	_mummyIdleEnabled = false;
	_sceneLayers.setLayerVisible(kMummyLayer, false);
	_sceneLayers.resetLayer(kMummyLayer, 0);
	drawPlayableComposite();
	presentFrame();
}

void Scene2100::runLateReturnSequence() {
	beginSecondarySpeechLine(6, 0);
	runMummyPrimarySpeechLine(1);
	beginSecondarySpeechLine(6, 2);
	runMummyPrimarySpeechLine(3);
	beginSecondarySpeechLine(6, 4);
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		runExitToScene2010(0x3b0, 0x1ba);
}

void Scene2100::runExitToScene2010(int targetX, int targetY) {
	walkActiveActorTo(targetX, targetY, kScene2100InvalidFacing, 0, false);
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		_vm->gameState().mainFlowStateId = kScene2010PatchedEntryState;
}

void Scene2100::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		byte startCel, int targetX, int targetY, byte finalFacing, byte finalCel) {
	setActiveActorPose(startX, startY, startFacing, startCel);

	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;
	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	if (finalFacing != kScene2100InvalidFacing)
		_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

bool Scene2100::runMummyDialogue(bool playGreeting) {
	GameplayState &state = _vm->gameState();
	if (playGreeting)
		state.scene2100MummyDialogueUsedChoiceMask = 0;

	Common::Array<DialogueChoiceRecord> records;
	initializeMummyDialogueRecords(records);

	if (playGreeting && !state.scene2100MummyGreetingSeen) {
		runMummyPrimarySpeechLine(0);
		beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, 0);
		runMummyPrimarySpeechLine(1);
		state.scene2100MummyGreetingSeen = true;
	} else if (playGreeting) {
		beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, 1);
		runMummyPrimarySpeechLine(2);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene2100MummyDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, 8);
			runMummyPrimarySpeechLine(9);
			return false;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return false;

		DialogueChoiceRecord &record = records[recordIndex];
		beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			runMummyPrimarySpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
			state.scene2100MummyDialogueUsedChoiceMask |= mummyDialogueChoiceBit(recordIndex);
		}
		if (record.disableAfterUse == 2 && !state.scene2100AfterlifeBranchUnlocked) {
			if (records.size() > 86) {
				records[86].enabled = 1;
				records[86].selectable = 1;
			}
			state.scene2100AfterlifeBranchUnlocked = true;
		} else if (record.disableAfterUse == 3 && state.scene2040SphinxExitInterviewState == 0) {
			if (records.size() > 3) {
				records[3].enabled = 1;
				records[3].selectable = 1;
			}
			state.scene2040SphinxExitInterviewState = 1;
			state.scene2030SeedOfferState = 1;
			state.scene2030MerchantItem2AOfferState = 1;
		} else if (record.disableAfterUse == 4 && state.scene2040SphinxExitInterviewState == 1) {
			state.scene2040SphinxExitInterviewState = 2;
		} else if (record.disableAfterUse == 5) {
			runTreasureIntroductionSequence();
			return true;
		}

		switch (record.transitionMode) {
		case 0:
			return false;
		case 1:
			nodeIndex = record.nextNodeIndex;
			++depthIndex;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			if (depthIndex != 0)
				--depthIndex;
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 1 ? (byte)(depthIndex - 2) : 0;
			break;
		case 5:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 3 ? (byte)(depthIndex - 4) : 0;
			break;
		default:
			break;
		}
	}

	return false;
}

void Scene2100::initializeMummyDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	const GameplayState &state = _vm->gameState();
	records.resize(kScene2100MummyDialogueChoiceRecordCount);

	setDialogueRecord(records, 0, 1, 0, 3, 2, 3, 1, 0xff);
	setDialogueRecord(records, 1, state.scene2020PrincessConversationSeen ? 1 : 0, 2, 1, 3, 4, 1, 0xff);
	setDialogueRecord(records, 2, 1, 1, 1, 4, 5, 1, 0xff);
	setDialogueRecord(records, 3, state.scene2040SphinxExitInterviewState != 0 ? 1 : 0, 3, 1, 5, 6, 4, 0xff);
	setDialogueRecord(records, 4,
		state.mainFlowStateId == kScene2100EntryState &&
			state.scene2100MummyBranchState == 0 ? 1 : 0,
		0, 0, 7, 8, 5, 0xff);
	setDialogueRecord(records, 5, 1, 0, 0, 8, 9, 0, 0xff);
	setDialogueRecord(records, 70, 1, 0, 2, 9, 10, 1, 0xff);
	setDialogueRecord(records, 77, 1, 0, 2, 10, 11, 1, 0xff);
	setDialogueRecord(records, 84, 1, 2, 3, 11, 12, 2, 0xff);
	setDialogueRecord(records, 85, 1, 2, 3, 12, 13, 1, 0xff);
	setDialogueRecord(records, 86, state.scene2100AfterlifeBranchUnlocked ? 1 : 0,
		0, 1, 13, 14, 3, 0xff);
	setDialogueRecord(records, 87, 1, 0, 2, 15, 16, 1, 0xff);
	setDialogueRecord(records, 91, 1, 0, 2, 6, 7, 1, 0xff);
	setDialogueRecord(records, 140, 1, 0, 1, 14, 15, 1, 0xff);
	setDialogueRecord(records, 210, 1, 0, 1, 16, 17, 1, 0xff);
	setDialogueRecord(records, 280, 1, 0, 5, 17, 18, 1, 0xff);

	for (uint bit = 0; bit < ARRAYSIZE(kScene2100MummyDialogueTrackedRecordIndices); ++bit) {
		if ((state.scene2100MummyDialogueUsedChoiceMask & (1 << bit)) == 0)
			continue;

		DialogueChoiceRecord &record = records[kScene2100MummyDialogueTrackedRecordIndices[bit]];
		record.enabled = 0;
		record.selectable = 0;
	}
}

void Scene2100::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = enabled;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = reserved;
	record.selectable = enabled != 0;
}

void Scene2100::runMummyPrimarySpeechLine(byte frameIndex) {
	_mummySpeechUsesFrontLayer = false;
	const bool useAlternateFrameSet = _random.getRandomBit() != 0;
	if (useAlternateFrameSet)
		runMummyFrameSetTransition(kMummyLayer, true, 17);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	beginPrimarySpeechLineWithAnimationGroup(kScene2100MummyPrimaryRow, frameIndex,
		0x2e7, 0x83, 0x32, 0x32, 0x3f, 0);
	if (useAlternateFrameSet && !Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		runMummyFrameSetTransition(kMummyLayer, false, 17);
}

void Scene2100::runPrincessArrivalPrimarySpeechLine(byte frameIndex) {
	_mummySpeechUsesFrontLayer = true;
	const bool useAlternateFrameSet = _random.getRandomBit() != 0;
	if (useAlternateFrameSet)
		runMummyFrameSetTransition(kFrontLayer, true, 18);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	beginPrimarySpeechLineWithAnimationGroup(7, frameIndex,
		0x2f3, 0xa1, 0x32, 0x32, 0x3f, 0);
	if (useAlternateFrameSet && !Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		runMummyFrameSetTransition(kFrontLayer, false, 18);
}

void Scene2100::runMummyFrameSetTransition(uint layerId, bool opening, byte closingFinalFrame) {
	const bool previousIdleEnabled = _mummyIdleEnabled;
	BlockingSequence sequence(*this);
	sequence.commit(_mummyIdleEnabled, false);
	if (opening) {
		sequence.presentedLayerTransition(layerId,
			AnimationTransition(5, 8, 8, kScene2100ForegroundFrameMillis).unskippable());
	} else {
		sequence.presentedLayerTransition(layerId,
			AnimationTransition(14, closingFinalFrame, closingFinalFrame,
				kScene2100ForegroundFrameMillis).unskippable());
	}
	sequence.commit(_foregroundAlternateFrameSet, opening);
	_foregroundChannel.reset(_sceneLayers.layerFrame(layerId),
		kScene2100PrimarySpeechFrameMillis);
	sequence.commit(_mummyIdleEnabled, previousIdleEnabled);
}

void Scene2100::beginMummyDialogueSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginSecondarySpeechLine(rowIndex, frameIndex);
}

void Scene2100::runTreasureIntroductionSequence() {
	_mummyIdleEnabled = false;
	_sceneLayers.setLayerVisible(kMummyLayer, false);
	_sceneLayers.setLayerResource(kFrontLayer, kScene2100SpecialSpeechChunk,
		kScene2100SpecialSpeechDescriptorCount,
		kScene2100SpecialIntroFrameMap,
		ARRAYSIZE(kScene2100SpecialIntroFrameMap));
	_sceneLayers.setLayerVisible(kFrontLayer, true);
	startConcurrentActorPath(0x277, 0x168, 2, 0, kScene2100ActorPathFrameMillis);
	BlockingSequence sequence(*this);
	sequence.presentedLayerTransition(kFrontLayer,
		AnimationTransition(0, ARRAYSIZE(kScene2100SpecialIntroFrameMap) - 1,
			ARRAYSIZE(kScene2100SpecialIntroFrameMap) - 1,
			kScene2100ForegroundFrameMillis).unskippable());
	if (!sequence.completed())
		return;
	finishConcurrentActorPath();
	if (!sequence.completed())
		return;

	runSpecialTransitionSpeech();
	if (!sequence.completed())
		return;

	_sceneLayers.setLayerResource(kFrontLayer, kScene2100SpecialExitChunk,
		kScene2100SpecialExitDescriptorCount,
		kScene2100SpecialExitFrameMap,
		ARRAYSIZE(kScene2100SpecialExitFrameMap));
	_sceneLayers.setLayerVisible(kFrontLayer, true);
	sequence.layerFrames(kFrontLayer,
		AnimationFrameRange(0, 25, kScene2100ForegroundFrameMillis)
			.unskippable()
			.soundAt(3, 0x10)
			.stopSoundAt(14)
			.actorPathAt(14, 0x1b5, 0x151,
				kScene2100InvalidFacing, kScene2100InvalidCel,
				kScene2100ActorPathFrameMillis));
	if (!sequence.completed()) {
		_soundBank0.stop();
		return;
	}
	_sceneLayers.setLayerFrame(kFrontLayer, 26);
	drawPlayableComposite();
	presentFrame();
	finishConcurrentActorPath();
	_sceneLayers.setLayerVisible(kFrontLayer, false);
}

void Scene2100::runSpecialTransitionSpeech() {
	_specialSpeechVariant = _random.getRandomBit();
	_sceneLayers.setLayerFrameMap(kFrontLayer,
		_specialSpeechVariant == 0 ? kScene2100SpecialSpeechShortFrameMap :
			kScene2100SpecialSpeechLongFrameMap,
		_specialSpeechVariant == 0 ? ARRAYSIZE(kScene2100SpecialSpeechShortFrameMap) :
			ARRAYSIZE(kScene2100SpecialSpeechLongFrameMap));
	_sceneLayers.resetLayer(kFrontLayer, 0);
	_foregroundChannel.reset(0, kScene2100ForegroundFrameMillis);
	_specialSpeechAnimationActive = true;
	_specialSpeechFinishing = false;
	_suppressMummySpeechAnimation = true;

	for (byte frame = 19; frame <= 22 && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		beginPrimarySpeechLineWithAnimationGroup(kScene2100MummyPrimaryRow, frame,
			700, 0xaf, 0x32, 0x32, 0x3f, 0);
	}

	_specialSpeechFinishing = true;
	while (_specialSpeechAnimationActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			break;
	}
	_specialSpeechAnimationActive = false;
	_specialSpeechFinishing = false;
	_suppressMummySpeechAnimation = false;
}

void Scene2100::runStoneDoorToTreasureRoom() {
	if (!hasInventoryItem(kScene2100RaStaffItem)) {
		beginSecondarySpeechLine(1, 0);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.actorPath(SceneActorPose(0x24a, 0x163, 4));
	if (!sequence.completed())
		return;

	_sceneLayers.setLayerResource(kFrontLayer, kScene2100DoorChunk,
		kScene2100DoorDescriptorCount,
		kScene2100TransitionFrameMap, ARRAYSIZE(kScene2100TransitionFrameMap));
	_sceneLayers.setLayerVisible(kFrontLayer, true);
	sequence.presentedLayerTransition(kFrontLayer,
		AnimationTransition(0, 3, 3, kScene2100AuxFrameMillis).unskippable());
	if (!sequence.completed())
		return;
	_auxChannel.reset(3, kScene2100AuxFrameMillis);
	_doorCeremonyState = 2;
	_doorCeremonyFinishing = false;
	_doorCeremonyAnimationActive = true;

	for (byte speechFrame = 0; speechFrame < 4 && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++speechFrame) {
		beginSecondarySpeechLine(5, speechFrame);
	}
	_doorCeremonyFinishing = true;
	while (_doorCeremonyAnimationActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			return;
	}
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_sceneLayers.setLayerFrame(kFrontLayer, 31);
	drawPlayableComposite();
	presentFrame();
	if (waitSceneMillis(kScene2100AuxFrameMillis, false))
		return;
	_soundBank0.playSample(0x10, 100);
	if (waitSceneMillis(kScene2100AuxFrameMillis, false)) {
		_soundBank0.stop();
		return;
	}
	for (byte frame = 32; frame <= 50; ++frame) {
		_sceneLayers.setLayerFrame(kFrontLayer, frame);
		if (waitSceneMillis(kScene2100AuxFrameMillis, false)) {
			_soundBank0.stop();
			return;
		}
	}
	_soundBank0.stop();
	_sceneLayers.setLayerFrame(kFrontLayer, 51);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	sequence.commit(state.scene2100PassageOpen, true)
		.commit(state.mainFlowStateId, kScene2110EntryState);
}

void Scene2100::runRaStaffPickup() {
	if (!isRaStaffAvailable()) {
		dispatchGenericSceneAction(6);
		return;
	}

	GameplayState &state = _vm->gameState();
	BlockingSequence sequence(*this);
	sequence.commit(state.scene2100RaStaffTaken, true)
		.actorReplacement(ActionOverlaySpec(kScene2100PickupChunk, kScene2100PickupDescriptorCount,
			kScene2100PickupFrameMap, ARRAYSIZE(kScene2100PickupFrameMap),
			kScene2100AuxFrameMillis)
			.patchAt(5, 1)
			.noFinalFrameDelay())
		.framebufferPatch(1);
	if (!hasInventoryItem(kScene2100RaStaffItem))
		addInventoryItem(kScene2100RaStaffItem);
	sequence.sound(1);
	dispatchGenericSceneAction(21);
}

void Scene2100::removeColorMapItemFromOriginal(byte itemId) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
