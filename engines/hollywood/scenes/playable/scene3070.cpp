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

#include "hollywood/scenes/playable/scene3070.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene3070FirstState = 0x0bfe;
const uint16 kScene3070EntryFromOtherSideState = 0x0bff;
const uint16 kScene3070LateCutsceneState = 0x0c00;
const uint16 kScene3060ReturnFromScene3070State = 0x0bf5;
const uint16 kScene3110LongTransitionState = 0x0c26;
const uint16 kScene3110ShortTransitionState = 0x0c27;
const uint16 kScene3070LateExitState = 0x23b4;
const uint16 kScene3070ViewportXOffset = 0x0090;
const uint16 kScene3070ViewportMinXOffset = 0x0068;
const uint16 kScene3070ViewportMaxXOffset = 0x0090;
const uint kScene3070ActorBankTableEntry = 0x0038;
const uint kScene3070ActorPaletteTableEntry = 0x00cc;
const uint kScene3070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3070OverlayFrameMillis = 75;
const uint kScene3070PatchOverlayDescriptorCount = 9;
const byte kScene3070InterludePrimaryRowLeft = 0x21;
const byte kScene3070InterludePrimaryRowRight = 0x22;
const byte kScene3070InterludeLeftSpeechGroup = 0;
const byte kScene3070InterludeRightSpeechGroup = 1;
const byte kScene3070LateSpeechGroup = 2;

const byte kScene3070SpecialActorPathStepDeltaTable[] = {
	4, 1, 1, 2, 2, 2, 5, 1, 0, 0, 3, 2
};

const byte kScene3070InterludeLeftFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
};

const byte kScene3070InterludeRightFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

const byte kScene3070InterludeLeftTransition[] = {
	0, 0, 0, 0, 5, 6, 7, 8, 9, 10
};

const byte kScene3070InterludeRightTransition[] = {
	5, 6, 7, 8, 8, 8, 8, 9, 10, 10
};

const byte kScene3070LateCutsceneFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16, 17, 18, 19,
	18, 17, 16, 15, 16, 17, 18, 19, 18, 17, 16, 15, 16, 17, 18, 19,
	18, 17, 16, 15, 16, 17, 18, 19, 18, 17, 16, 15, 14, 13, 12, 8,
	8, 9, 10, 11, 20
};

const byte kScene3070PatchOverlayFrameMap[] = {
	8, 0, 1, 2, 2, 1, 0, 8
};

const byte kScene3070ItemPatchPickupFrameMap[] = {
	8, 0, 1, 2, 3, 4, 5, 6, 7, 8
};

const byte kScene3070IngredientFrameMap[] = {
	10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 10
};

const byte kScene3070SyringeIngredientFrameMap[] = {
	10, 9, 8, 7, 6, 5, 4, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 2, 1, 0, 10
};

const byte kScene3070BrainInstallationFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 3, 4, 5, 6, 7, 8,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 15, 14, 13, 12, 8, 9, 10, 11
};

const byte kScene3070BodyAssemblyFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 3, 4, 5, 6, 7, 8,
	7, 6, 5, 4, 3, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 3, 4, 5, 6, 7, 8,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 17, 18, 19, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28,
	16, 15, 14, 13, 12, 8, 9, 10, 11
};

const byte kScene3070BodyAssemblyFinishFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
};

const byte kScene3070RevivalStartFrameMap[] = {
	5, 0, 1, 2, 3, 4, 4, 4, 4, 3, 2, 1, 0, 5
};

const byte kScene3070RevivalLoopFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	20, 21, 22, 23, 20, 21, 22, 23, 20, 21, 22, 23,
	20, 21, 22, 23, 20, 21, 22, 23, 20, 21, 22, 23,
	20, 21, 22, 23, 20, 21, 22, 23, 20, 21, 22, 23,
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4
};

const byte kScene3070RevivalFinishFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
};

static PlayableSceneConfig scene3070Config() {
	PlayableSceneConfig config(3070,
		SceneResourceLayout(24, 5, 36),
		SceneViewport(kScene3070ViewportXOffset, kScene3070ViewportMinXOffset, kScene3070ViewportMaxXOffset),
		SceneActorPose(0x23f, 0x192, 5));
	config.setActorResources(kScene3070ActorBankTableEntry, kScene3070ActorPaletteTableEntry);
	config.setTextResources(kScene3070Resource003RowsOffsetIndex, kScene3070SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 2;
	return config;
}

Scene3070::Scene3070(HollywoodEngine *vm) :
		PlayableScene(vm, scene3070Config()),
		_interludeLeftLayer(),
		_interludeRightLayer(),
		_lateCutsceneLayer(),
		_interludeActive(false),
		_interludeAlternatePose(false),
		_lateCutsceneActive(false) {
	_interludeLeftLayer.configure(36, 26,
		kScene3070InterludeLeftFrameMap, ARRAYSIZE(kScene3070InterludeLeftFrameMap));
	_interludeRightLayer.configure(35, 32,
		kScene3070InterludeRightFrameMap, ARRAYSIZE(kScene3070InterludeRightFrameMap));
	_lateCutsceneLayer.configure(22, 21,
		kScene3070LateCutsceneFrameMap, ARRAYSIZE(kScene3070LateCutsceneFrameMap));
}

bool Scene3070::shouldLoadArenaChunk(uint index) const {
	return (index >= 5 && index <= 23) || index == 35 || index == 36;
}

void Scene3070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetCutsceneLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene3070EntryFromOtherSideState || stateId == kScene3070LateCutsceneState) {
		_activeActorWorldX = 0x24a;
		_activeActorWorldY = 0x13d;
		_activeActorFacing = 5;
	} else {
		_activeActorWorldX = 0x23f;
		_activeActorWorldY = 0x192;
		_activeActorFacing = 5;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene3070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	if (_interludeActive) {
		drawResourceSpriteLayer(_interludeLeftLayer);
		drawResourceSpriteLayer(_interludeRightLayer);
		return;
	}

	if (_lateCutsceneActive)
		drawResourceSpriteLayer(_lateCutsceneLayer);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY, actorDrawOrderMode, !_lateCutsceneActive);
	drawActionOverlayLayer();
}

void Scene3070::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene3070EntryFromOtherSideState)
		runEntryFromOtherSide();
	else if (stateId == kScene3070LateCutsceneState)
		runLateCutsceneBranch();
	else if (stateId == kScene3070FirstState)
		runEntryFromSecretPassage();
}

bool Scene3070::prepareCustomGameplayLoop() {
	resetCutsceneLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene3070::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	if (!_interludeActive)
		updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3070::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 81: // Usar mando a distancia (use remote control): global route to the revival callback.
	case 317: // Reanimar a Frankenstein (revive Frankenstein): local callback invoked by handler 81.
		runFrankensteinRevival();
		return true;
	case 127: // Usar cerebro o parte del cuerpo con Frankenstein (use brain/body part): select the assembly step.
		if (_lastInventoryActionItemId == 0x25)
			runBrainInstallation();
		else
			runBodyAssembly();
		return true;
	case 301: // Ir a pasadizo secreto (go to secret passage): return to Scene 3060.
		state.mainFlowStateId = kScene3060ReturnFromScene3070State;
		return true;
	case 302: // Mirar escalera (look at stairs): it connects the laboratory to the library.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar riachuelo (look at brook).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar mesa de operaciones (look at operating table), changes once the body is present.
		beginSecondarySpeechLine(3, state.scene3070FrankensteinBodyState != 0 ? 1 : 0);
		return true;
	case 305: // Mirar mesa (look at table).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar cubetas (look at tubs).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar panel de controles/cajón (look at controls/drawer).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Usar panel de controles (use control panel).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar cajón (look at drawer): describe the surgical needle and thread when exposed.
		if (!state.scene3070DrawerOpen) {
			beginSecondarySpeechLine(8, 0);
		} else if (state.scene3070SurgicalNeedleThreadState < 2) {
			beginSecondarySpeechLine(8, 1);
			if (state.scene3070SurgicalNeedleThreadState == 0) {
				state.scene3070SurgicalNeedleThreadState = 1;
				applySceneStateToHotspotsAndPatches(5);
			}
		} else {
			beginSecondarySpeechLine(8, 2);
		}
		return true;
	case 310: // Abrir cajón (open drawer): reveal the surgical needle and thread.
		runDoorPatchOverlay(true);
		return true;
	case 311: // Cerrar cajón (close drawer).
		runDoorPatchOverlay(false);
		return true;
	case 312: // Coger aguja e hilo quirúrgicos (take surgical needle and thread).
		runItemPatchPickup();
		return true;
	case 313: // Mirar aguja e hilo quirúrgicos (look at surgical needle and thread).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 314: // Hablar con Frankenstein inconsciente (talk to unconscious Frankenstein).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 315: // Coger Frankenstein (take Frankenstein): leave him on the operating table.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 316: // Mirar Frankenstein (look at Frankenstein), changes after the experiment advances.
		beginSecondarySpeechLine(12, state.scene3070StoryPhase != 0 ? 1 : 0);
		return true;
	case 318: // Usar cerebro con Frankenstein (use brain with Frankenstein): install the brain.
		runBrainInstallation();
		return true;
	case 319: // Usar partes del cuerpo con Frankenstein (use body parts): assemble the body.
		runBodyAssembly();
		return true;
	case 320: // Usar gominola con cubetas (use gummy sweet with tubs): add it to the serum.
		addSerumIngredient(0x44, 18, false, false);
		return true;
	case 321: // Usar hueso con cubetas (use bone with tubs): add it to the serum.
		addSerumIngredient(0x3e, 19, true, false);
		return true;
	case 322: // Usar jeringuilla llena con cubetas (use filled syringe with tubs): add the sap.
		addSerumIngredient(0x38, 20, true, true);
		return true;
	case 323: // Usar cáscara de huevo con cubetas (use eggshell with tubs): add it to the serum.
		addSerumIngredient(0x5d, 21, true, false);
		return true;
	case 324: // Mirar aparato del experimento (look at experiment apparatus).
		beginSecondarySpeechLine(24, 0);
		return true;
	case 325: // Usar aparato del experimento (use experiment apparatus): report missing prerequisites.
		if (state.scene3070FrankensteinBodyState == 0)
			beginStaticSecondarySpeechLine(0x4c, 2);
		else if (state.scene3070SerumIngredientCount < 5)
			beginSecondarySpeechLine(22, 0);
		else if (!state.scene3030MachineActivated)
			beginStaticSecondarySpeechLine(0x4c, 0);
		else if (!hasInventoryItem(0x27))
			beginSecondarySpeechLine(22, 1);
		else if (state.scene3070StoryPhase == 1 && state.scene3070FrankensteinBodyState == 1)
			beginSecondarySpeechLine(14, 0);
		else
			beginSecondarySpeechLine(24, 1);
		return true;
	case 326: // Usar rodaja de limón con cubetas (use lemon slice with tubs): add it to the serum.
		addSerumIngredient(0x57, 25, true, false);
		return true;
	case 327: // Usar otro objeto con cubetas (use another item with tubs): reject an invalid ingredient.
		beginSecondarySpeechLine(17, state.scene3070SerumIngredientCount < 3 ? 0 : 1);
		return true;
	default:
		return false;
	}
}

bool Scene3070::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetY < 0x1df)
		++targetY;

	while (targetY <= 0x1df) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		++targetY;
	}

	targetY = 0x1df;
	while (targetY > 0) {
		--targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}
	return true;
}

bool Scene3070::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 3 && nextRegion == 2) {
		copySpecialStepDeltas(0x30, 0x3b);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 4 && nextRegion == 5) {
		copySpecialStepDeltas(0x3c, 0x47);
		requestedFacing = 5;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 5 && nextRegion == 6) {
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 6 && nextRegion == 5) {
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 5 && nextRegion == 4) {
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 4 && nextRegion == 2) {
		copySpecialStepDeltas(0x18, 0x23);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}
	return false;
}

bool Scene3070::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;

	bool customized = false;
	if (currentRegion == 3) {
		copySpecialStepDeltas(0x0c, 0x17);
		requestedFacing = 1;
		restoredStepDeltas = true;
		customized = true;
	} else if (currentRegion == 4) {
		copySpecialStepDeltas(0x3c, 0x47);
		requestedFacing = 5;
		restoredStepDeltas = true;
		customized = true;
	} else if (currentRegion == 6) {
		requestedFacing = 5;
		restoredStepDeltas = true;
		customized = true;
	}
	if (state.x == 0x0dc && state.y == 0x1b6 && targetX == 0x0e6 && targetY == 0x1b5) {
		requestedFacing = 5;
		restoredStepDeltas = true;
		customized = true;
	}
	return customized;
}

bool Scene3070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 1 || selector == 2 || selector == 5) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		const GameplayState &state = _vm->gameState();
		if (state.scene3070DrawerOpen) {
			if (_sceneChunkTable.isValidChunk(7))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
			replaceColorMapItemFromOriginal(9, state.scene3070SurgicalNeedleThreadState == 1 ? 9 : 8);
		} else {
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
			replaceColorMapItemFromOriginal(9, 8);
		}

		if (state.scene3070FrankensteinBodyState != 0) {
			if (_sceneChunkTable.isValidChunk(18))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[18], _baseFramebuffer);
			if (_sceneChunkTable.isValidChunk(17))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[17], _baseFramebuffer);
			replaceColorMapItemFromOriginal(10, 10);
		} else {
			removeColorMapItem(10);
		}

		if (state.scene3070SurgicalNeedleThreadState == 2 || state.scene3070SurgicalNeedleThreadTaken)
			removeColorMapItem(9);

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		fixControlPanelInteractionPoint();
	}

	return true;
}

void Scene3070::handleAnimationFrameHook(byte hookId, uint frame) {
	switch (hookId) {
	case 1:
		applyActionPatchChunk(21);
		break;
	case 2:
		_soundBank0.playSample(0x14, 100);
		_ambientSoundBank0.playSample(0x11, 50, true);
		applyActionPatchChunk(12);
		break;
	case 3:
		if (frame == 20)
			_soundBank0.playSample(0x0c, 25, true);
		else if (frame == 60)
			_soundBank0.stop();
		break;
	default:
		break;
	}
}

bool Scene3070::shouldAnimatePrimarySpeechLine() const {
	return _interludeActive || _lateCutsceneActive;
}

byte Scene3070::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene3070LateSpeechGroup)
		return 49;
	return _interludeAlternatePose ? 11 : 0;
}

void Scene3070::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene3070InterludeLeftSpeechGroup)
		_interludeLeftLayer.setFrame(frameIndex);
	else if (animationGroup == kScene3070InterludeRightSpeechGroup)
		_interludeRightLayer.setFrame(frameIndex);
	else if (animationGroup == kScene3070LateSpeechGroup)
		_lateCutsceneLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene3070::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = 0x18;
	profile.soundVolumePercent = 1; // Legacy 0x78 is about -40 dB.
	return profile;
}

void Scene3070::resetCutsceneLayers() {
	_interludeLeftLayer.reset(0);
	_interludeRightLayer.reset(0);
	_lateCutsceneLayer.reset(0);
	_interludeLeftLayer.visible = false;
	_interludeRightLayer.visible = false;
	_lateCutsceneLayer.visible = false;
	_interludeActive = false;
	_interludeAlternatePose = false;
	_lateCutsceneActive = false;
}

void Scene3070::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3070::removeColorMapItem(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == itemId)
			_paletteMask[kSceneColorToItemMap + i] = 0;
	}
}

void Scene3070::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene3070::fixControlPanelInteractionPoint() {
	const SceneActionTarget target = _hotspots.actionTarget(6);
	ScenePoint interactionPoint;
	interactionPoint.x = 0x0a7;
	interactionPoint.y = 0x1c0;
	_hotspots.setActionTarget(6, interactionPoint, target.approachPoint);
}

void Scene3070::copySpecialStepDeltas(uint firstOffset, uint lastOffset) {
	for (uint targetOffset = firstOffset, sourceOffset = 0;
			targetOffset <= lastOffset && targetOffset < _actorPathStepDeltas.size() &&
			sourceOffset < ARRAYSIZE(kScene3070SpecialActorPathStepDeltaTable);
			++targetOffset, ++sourceOffset)
		_actorPathStepDeltas[targetOffset] = kScene3070SpecialActorPathStepDeltaTable[sourceOffset];
}

void Scene3070::runEntryFromSecretPassage() {
	runEntryPath(0x2fe, 0x133, 5, 0x23f, 0x192);
	_activeActorFacing = 5;
	if (!_vm->gameState().scene3070EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene3070EntryLineSeen = true;
	}
}

void Scene3070::runEntryFromOtherSide() {
	GameplayState &state = _vm->gameState();
	setActiveActorPose(0x0dc, 0x1b6, 1);
	drawPlayableComposite();
	if (runCurtainRevealFromBlack())
		return;
	runEntryPath(0x0dc, 0x1b6, 1, 0x24a, 0x13d);
	_activeActorFacing = 5;

	if (state.scene3070StoryPhase == 0) {
		state.scene3070StoryPhase = 1;
		beginSecondarySpeechLine(0x0d, 2);
		runInterludeCutscene();
		beginSecondarySpeechLine(0x0d, 3);
		state.scene1070DoorOpened = true;
		return;
	}

	beginSecondarySpeechLine(0x0d, 5);
	state.mainFlowStateId = kScene3110ShortTransitionState;
}

void Scene3070::runLateCutsceneBranch() {
	runEntryPath(0x24a, 0x13d, 5, 0x24a, 0x13d);
	if (_sceneChunkTable.isValidChunk(23))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[23], _baseFramebuffer);
	_lateCutsceneActive = true;
	_lateCutsceneLayer.visible = true;
	playAnimationFrames(_lateCutsceneLayer,
		AnimationFrameRange(0, 48, kScene3070OverlayFrameMillis));
	beginPrimarySpeechLineWithAnimationGroup(0x0d, 6, 0x212, 0x09e,
		0x20, 0x00, 0x3f, kScene3070LateSpeechGroup);
	beginSecondarySpeechLine(0x0d, 7);
	beginPrimarySpeechLineWithAnimationGroup(0x0d, 8, 0x212, 0x09e,
		0x20, 0x00, 0x3f, kScene3070LateSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(0x0d, 9, 0x212, 0x09e,
		0x20, 0x00, 0x3f, kScene3070LateSpeechGroup);
	beginSecondarySpeechLine(0x0d, 10);
	GameplayState &state = _vm->gameState();
	state.scene3070StoryPhase = 2;
	state.scene3070LateCutscenePlayed = true;
	state.scene6010EndgameTravelExitBlocked = true;
	state.scene6010StudioEntryUnlocked = false;
	state.scene9140VariantIndex = 3;
	state.scene9140ReturnStateId = 6000;
	state.mainFlowStateId = kScene3070LateExitState;
	runCurtainClearToBlack();
}

void Scene3070::runInterludeCutscene() {
	const Common::Array<byte> savedPalette = _paletteCurrent;
	const uint16 savedViewportX = _viewportXOffset;
	_vm->gameplayMusic()->stop();
	_ambientSoundBank0.stop();
	_soundBank0.stop();
	fadePaletteToBlack();

	if (!loadFixedChunk(33, _baseFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(34, _paletteCurrent, kPaletteSize)) {
		applySceneStateToHotspotsAndPatches(0xff);
		_paletteCurrent = savedPalette;
		_viewportXOffset = savedViewportX;
		drawPlayableComposite();
		fadePaletteFromBlack();
		return;
	}

	_viewportXOffset = 0x10;
	_interludeActive = true;
	_interludeLeftLayer.visible = true;
	_interludeRightLayer.visible = true;
	_interludeLeftLayer.setFrame(0);
	_interludeRightLayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();
	fadePaletteFromBlack();

	beginPrimarySpeechLineWithAnimationGroup(kScene3070InterludePrimaryRowLeft, 0,
		0x0e3, 0x084, 0x0d, 0x32, 0x3a, kScene3070InterludeLeftSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(kScene3070InterludePrimaryRowRight, 0,
		0x079, 0x086, 0x0a, 0x3f, 0x00, kScene3070InterludeRightSpeechGroup);

	for (uint frame = 0; frame < ARRAYSIZE(kScene3070InterludeLeftTransition); ++frame) {
		_interludeLeftLayer.setFrame(kScene3070InterludeLeftTransition[frame]);
		_interludeRightLayer.setFrame(kScene3070InterludeRightTransition[frame]);
		if (waitSceneMillis(kScene3070OverlayFrameMillis))
			break;
	}

	_interludeAlternatePose = true;
	beginPrimarySpeechLineWithAnimationGroup(kScene3070InterludePrimaryRowLeft, 1,
		0x0e3, 0x084, 0x0d, 0x32, 0x3a, kScene3070InterludeLeftSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(kScene3070InterludePrimaryRowRight, 1,
		0x079, 0x086, 0x0a, 0x3f, 0x00, kScene3070InterludeRightSpeechGroup);

	for (byte frame = 16; frame < ARRAYSIZE(kScene3070InterludeRightFrameMap); ++frame) {
		if (frame < ARRAYSIZE(kScene3070InterludeLeftFrameMap))
			_interludeLeftLayer.setFrame(frame);
		_interludeRightLayer.setFrame(frame);
		if (waitSceneMillis(kScene3070OverlayFrameMillis))
			break;
	}

	fadePaletteToBlack();
	_interludeActive = false;
	_interludeAlternatePose = false;
	_interludeLeftLayer.visible = false;
	_interludeRightLayer.visible = false;
	applySceneStateToHotspotsAndPatches(0xff);
	_paletteCurrent = savedPalette;
	_viewportXOffset = savedViewportX;
	drawPlayableComposite();
	presentFrame();
	fadePaletteFromBlack();
}

void Scene3070::runDoorPatchOverlay(bool open) {
	GameplayState &state = _vm->gameState();
	if (state.scene3070DrawerOpen == open) {
		dispatchGenericSceneAction(open ? 20 : 12);
		return;
	}

	runActorReplacement(ActionOverlaySpec(9, kScene3070PatchOverlayDescriptorCount,
		kScene3070PatchOverlayFrameMap, ARRAYSIZE(kScene3070PatchOverlayFrameMap), kScene3070OverlayFrameMillis));
	state.scene3070DrawerOpen = open;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene3070::runItemPatchPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene3070SurgicalNeedleThreadTaken) {
		beginSecondarySpeechLine(8, 2);
		return;
	}

	runActorReplacement(ActionOverlaySpec(9, kScene3070PatchOverlayDescriptorCount,
		kScene3070ItemPatchPickupFrameMap, ARRAYSIZE(kScene3070ItemPatchPickupFrameMap), kScene3070OverlayFrameMillis));
	state.scene3070SurgicalNeedleThreadState = 2;
	state.scene3070SurgicalNeedleThreadTaken = true;
	applySceneStateToHotspotsAndPatches(0xff);
	addInventoryItem(0x32);
	_soundBank0.playSample(1, 100);
}

void Scene3070::runFrankensteinRevival() {
	GameplayState &state = _vm->gameState();
	if (state.scene3070FrankensteinBodyState == 0) {
		beginStaticSecondarySpeechLine(0x4c, 2);
		return;
	}
	if (!state.scene3030MachineActivated) {
		beginStaticSecondarySpeechLine(0x4c, 0);
		return;
	}
	if (state.scene3070SerumIngredientCount < 5) {
		beginSecondarySpeechLine(22, 0);
		return;
	}
	if (!hasInventoryItem(0x27)) {
		beginSecondarySpeechLine(22, 1);
		return;
	}
	if (state.scene3070StoryPhase == 1 && state.scene3070FrankensteinBodyState == 1) {
		beginSecondarySpeechLine(14, 0);
		return;
	}
	if (!walkActiveActorTo(0x0dc, 0x1b6, 5, 0))
		return;

	if (state.scene3070StoryPhase == 0)
		beginSecondarySpeechLine(13, 0);
	if (state.scene3070StoryPhase == 1 && state.scene3070FrankensteinBodyState == 2)
		beginSecondarySpeechLine(13, 4);

	_vm->gameplayMusic()->playMusicCue(0x12, 100, false);
	runActorReplacement(ActionOverlaySpec(13, 6, kScene3070RevivalStartFrameMap,
		ARRAYSIZE(kScene3070RevivalStartFrameMap), kScene3070OverlayFrameMillis).hookAt(5, 2));

	if (!walkActiveActorTo(0x0e6, 0x1b5, 1, 0)) {
		_soundBank0.stop();
		_ambientSoundBank0.stop();
		return;
	}
	runActorReplacement(ActionOverlaySpec(11, 24, kScene3070RevivalLoopFrameMap,
		ARRAYSIZE(kScene3070RevivalLoopFrameMap), kScene3070OverlayFrameMillis).hookEveryFrame(3));
	runActorReplacement(ActionOverlaySpec(14, 19, kScene3070RevivalFinishFrameMap,
		ARRAYSIZE(kScene3070RevivalFinishFrameMap), kScene3070OverlayFrameMillis).startAt(4));

	runCurtainClearToBlack();
	state.mainFlowStateId = kScene3110LongTransitionState;
	_soundBank0.stop();
	_ambientSoundBank0.stop();
}

void Scene3070::runBrainInstallation() {
	if (!hasInventoryItem(0x25)) {
		dispatchGenericSceneAction(231);
		return;
	}
	if (!walkActiveActorTo(0x225, 0x13e, 5, 0))
		return;

	runActorReplacement(ActionOverlaySpec(20, 29, kScene3070BrainInstallationFrameMap,
		ARRAYSIZE(kScene3070BrainInstallationFrameMap), kScene3070OverlayFrameMillis));
	GameplayState &state = _vm->gameState();
	state.scene3070FrankensteinBodyState = 2;
	applySceneStateToHotspotsAndPatches(2);
	removeInventoryItem(0x25);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(15, 0);
}

void Scene3070::runBodyAssembly() {
	if (!hasInventoryItem(0x30) || !hasInventoryItem(0x42) || !hasInventoryItem(0x4c)) {
		beginSecondarySpeechLine(16, 0);
		return;
	}
	if (!hasInventoryItem(0x32)) {
		beginSecondarySpeechLine(16, 1);
		return;
	}
	if (!walkActiveActorTo(0x24a, 0x13d, 5, 0))
		return;

	beginSecondarySpeechLine(16, 2);
	runActorReplacement(ActionOverlaySpec(15, 29, kScene3070BodyAssemblyFrameMap,
		ARRAYSIZE(kScene3070BodyAssemblyFrameMap), kScene3070OverlayFrameMillis).hookAt(10, 1));
	runActorReplacement(ActionOverlaySpec(16, 20, kScene3070BodyAssemblyFinishFrameMap,
		ARRAYSIZE(kScene3070BodyAssemblyFinishFrameMap), kScene3070OverlayFrameMillis));

	_vm->gameState().scene3070FrankensteinBodyState = 1;
	applySceneStateToHotspotsAndPatches(2);
	removeInventoryItem(0x30);
	removeInventoryItem(0x42);
	removeInventoryItem(0x4c);
	_soundBank0.playSample(1, 100);
}

void Scene3070::addSerumIngredient(byte itemId, uint16 speechRow, bool speakBefore, bool useSyringeAnimation) {
	GameplayState &state = _vm->gameState();
	if (!state.frankensteinDiaryRead) {
		beginSecondarySpeechLine(17, 0);
		return;
	}
	if (!hasInventoryItem(itemId)) {
		dispatchGenericSceneAction(231);
		return;
	}

	if (speakBefore)
		beginSecondarySpeechLine(speechRow, 0);
	const byte *frameMap = useSyringeAnimation ? kScene3070SyringeIngredientFrameMap : kScene3070IngredientFrameMap;
	const uint frameCount = useSyringeAnimation ? ARRAYSIZE(kScene3070SyringeIngredientFrameMap) :
		ARRAYSIZE(kScene3070IngredientFrameMap);
	runActorReplacement(ActionOverlaySpec(10, 11, frameMap, frameCount, kScene3070OverlayFrameMillis));
	state.scene3070SerumIngredientCount = MIN<byte>(5, state.scene3070SerumIngredientCount + 1);
	if (useSyringeAnimation)
		addInventoryItem(0x08);
	removeInventoryItem(itemId);
	_soundBank0.playSample(1, 100);
	if (!speakBefore)
		beginSecondarySpeechLine(speechRow, 0);
}

void Scene3070::applyActionPatchChunk(uint chunkIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;

	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _baseFramebuffer);
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

bool Scene3070::runCurtainRevealFromBlack() {
	Graphics::ManagedSurface savedScene;
	savedScene.copyFrom(_sceneFramebuffer);
	byte *destination = framebufferPixels(_sceneFramebuffer);
	const byte *source = framebufferPixels(savedScene);
	if (!destination || !source)
		return false;

	memset(destination, 0, framebufferByteCount());
	presentFrame();
	for (int sweep = 0xdc; sweep >= 0 && !_vm->isSceneRestartRequested(); sweep -= 0x14) {
		const uint bandWidth = 0x14;
		const uint innerWidth = HollywoodEngine::kScreenWidth - 2 * sweep;
		const uint middleInset = sweep + bandWidth;
		const uint middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
		const uint leftX = _viewportXOffset + sweep;
		const uint rightX = leftX + innerWidth - bandWidth;

		for (uint row = 0; row < bandWidth; ++row) {
			memcpy(destination + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX,
				source + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX, innerWidth);
			const uint bottomY = HollywoodEngine::kScreenHeight - bandWidth - sweep + row;
			memcpy(destination + bottomY * HollywoodEngine::kSceneBufferWidth + leftX,
				source + bottomY * HollywoodEngine::kSceneBufferWidth + leftX, innerWidth);
		}
		for (uint row = 0; row < middleHeight; ++row) {
			const uint y = middleInset + row;
			memcpy(destination + y * HollywoodEngine::kSceneBufferWidth + leftX,
				source + y * HollywoodEngine::kSceneBufferWidth + leftX, bandWidth);
			memcpy(destination + y * HollywoodEngine::kSceneBufferWidth + rightX,
				source + y * HollywoodEngine::kSceneBufferWidth + rightX, bandWidth);
		}
		presentFrame();
		if (pollEvents(false))
			return true;
	}

	_sceneFramebuffer.copyRectToSurface(savedScene.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	presentFrame();
	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene3070::runCurtainClearToBlack() {
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	for (uint sweep = 0; sweep < 0xf0 && !_vm->isSceneRestartRequested(); sweep += 0x14) {
		const uint bandWidth = 0x14;
		const uint innerWidth = HollywoodEngine::kScreenWidth - 2 * sweep;
		const uint middleInset = sweep + bandWidth;
		const uint middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
		const uint leftX = _viewportXOffset + sweep;
		const uint rightX = leftX + innerWidth - bandWidth;

		for (uint row = 0; row < bandWidth; ++row) {
			memset(pixels + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX, 0, innerWidth);
			memset(pixels + (HollywoodEngine::kScreenHeight - bandWidth - sweep + row) *
				HollywoodEngine::kSceneBufferWidth + leftX, 0, innerWidth);
		}
		for (uint row = 0; row < middleHeight; ++row) {
			const uint y = middleInset + row;
			memset(pixels + y * HollywoodEngine::kSceneBufferWidth + leftX, 0, bandWidth);
			memset(pixels + y * HollywoodEngine::kSceneBufferWidth + rightX, 0, bandWidth);
		}
		presentFrame();
		if (pollEvents(false))
			break;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_displayPalette.markAllDirty();
	presentFrame();
}

void Scene3070::drawForegroundBlocks(int activeWorldY, byte actorDrawOrderMode, bool drawNearForeground) {
	if (activeWorldY < 0x13e && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (activeWorldY < 0x179 && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (drawNearForeground && actorDrawOrderMode > 5 && _sceneChunkTable.isValidChunk(19))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[19], _sceneFramebuffer);
}

} // End of namespace Hollywood
