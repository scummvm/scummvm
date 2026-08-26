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

#include "hollywood/scenes/playable/scene4080.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene4080ExitState = 0x1006;
const int kScene4080DefaultActorX = 0x0121;
const int kScene4080DefaultActorY = 0x012e;
const byte kScene4080DefaultActorFacing = 2;
const uint16 kScene4080ViewportInitialX = 0x00a0;
const uint16 kScene4080ViewportMinX = 0x0090;
const uint16 kScene4080ViewportMaxX = 0x00e4;
const uint kScene4080ActorBankTableEntry = 0x0000;
const uint kScene4080ActorPaletteTableEntry = 0x00cc;
const uint kScene4080Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4080SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4080FrameMillis = 75;
const uint kScene4080PalettePatchState1Chunk = 5;
const uint kScene4080PalettePatchState1DescriptorCount = 0x12;
const uint kScene4080PalettePatchState2Chunk = 6;
const uint kScene4080PalettePatchState2DescriptorCount = 5;
const uint kScene4080ForegroundFlickerChunk = 7;
const uint kScene4080ForegroundFlickerDescriptorCount = 4;
const uint kScene4080ForegroundBlockChunk = 8;
const uint kScene4080PaletteForegroundChunk = 9;
const uint kScene4080SideForegroundChunk = 10;
const uint kScene4080PassagePatchChunk = 11;
const uint kScene4080PaletteState0PatchChunk = 12;
const uint kScene4080SidePatchChunk = 13;
const uint kScene4080BaseSidePatchChunk = 14;
const uint kScene4080BottlePickupChunk = 15;
const uint kScene4080BottlePickupDescriptorCount = 0x0c;
const uint kScene4080StakeSequenceChunk = 16;
const uint kScene4080StakeSequenceDescriptorCount = 0x1b;
const uint kScene4080ExitOverlayChunk = 17;
const uint kScene4080ExitOverlayDescriptorCount = 6;
const uint kScene4080GominolaPickupChunk = 18;
const uint kScene4080GominolaPickupDescriptorCount = 0x0e;
const uint kScene4080VisibleGominolaPatchChunk = 19;
const uint kScene4080HiddenGominolaPatchChunk = 20;
const uint kScene4080SteakPickupChunk = 16;
const uint kScene4080SteakPickupDescriptorCount = 0x0c;
const uint kScene4080FoodBagOverlayChunk = 21;
const uint kScene4080FoodBagOverlayDescriptorCount = 0x0c;
const uint kScene4080GwendolynReplyChunk = 22;
const uint kScene4080GwendolynReplyDescriptorCount = 6;
const int kScene4080ForegroundActorThresholdY = 0x0150;
const byte kScene4080GwendolynBaseFrame = 0x19;
const byte kScene4080GwendolynSpeechGroup = 0;
const byte kScene4080GwendolynSpeechRed = 0x20;
const byte kScene4080GwendolynSpeechGreen = 0x3f;
const byte kScene4080GwendolynSpeechBlue = 0x00;
const uint16 kScene4080GwendolynSpeechCenterX = 0x0212;
const uint16 kScene4080GwendolynSpeechTopY = 0x006e;
const byte kScene4080MabusePillsItem = 0x0e;
const byte kScene4080StakeItem = 0x31;
const byte kScene4080OilBottleItem = 0x43;
const byte kScene4080GominolaItem = 0x44;
const byte kScene4080SteakItem = 0x45;

enum Scene4080AnimationHookId {
	kScene4080GominolaPickupHook = 1,
	kScene4080StakeSequenceHook
};

const byte kScene4080PalettePatchState1FrameMap[] = {
	0, 0, 1, 2, 3, 2, 1, 0, 1, 4, 5, 6, 7, 8, 9, 2,
	3, 2, 1, 0, 10, 11, 12, 11, 10, 13, 14, 15, 16, 17, 0, 0
};

const byte kScene4080PalettePatchState2FrameMap[] = {
	0, 0, 1, 2, 3, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const byte kScene4080ExitFrameMap[] = {
	0, 1, 2, 3, 4, 5
};

const byte kScene4080SidePatchFrameMap[] = {
	0, 1, 2, 3
};

const byte kScene4080BottlePickupFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0
};

const byte kScene4080GominolaPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene4080SteakPickupFrameMap[] = {
	0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 1, 0,
	1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 1, 0,
	1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11
};

const byte kScene4080StakeSequenceFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 14, 15, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25, 26
};

const byte kScene4080FoodBagFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene4080GwendolynReplyFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 4, 3, 2, 1, 0
};

struct Scene4080DialogueSeedRecord {
	uint16 index;
	byte enabled;
	byte nextNodeIndex;
	byte transitionMode;
	byte playerTextRowId;
	byte responseFrameIndex;
	byte disableAfterUse;
	byte reserved;
};

const Scene4080DialogueSeedRecord kScene4080DialogueSeedRecords[] = {
	{ 0, 1, 0, 3, 3, 3, 1, 0xff },     // Hay gusa, eh? (hungry?).
	{ 1, 1, 0, 1, 4, 4, 1, 0xff },     // Tentempie (snack): enter Dracula topics.
	{ 2, 1, 1, 1, 5, 5, 1, 0xff },     // Conde Dracula (Count Dracula): enter count topics.
	{ 3, 1, 0, 3, 6, 6, 1, 0xff },     // Sherilyn.
	{ 4, 1, 0, 0, 7, 7, 0, 0xff },     // Despedida (exit).
	{ 70, 1, 0, 1, 8, 8, 1, 0xff },    // Rellenita? (a bit plump?): enter weight topics.
	{ 71, 1, 0, 3, 9, 9, 1, 0xff },    // Sangre (blood).
	{ 72, 1, 0, 2, 10, 10, 0, 0xff },  // Hablar de otra cosa (talk about something else).
	{ 77, 1, 1, 3, 11, 11, 1, 0xff },  // Donde esta Dracula? (where is Dracula?).
	{ 78, 1, 1, 3, 12, 12, 1, 0xff },  // Premio (award).
	{ 79, 1, 1, 3, 13, 13, 1, 0xff },  // Depresion (depression).
	{ 80, 1, 0, 2, 10, 10, 0, 0xff },  // Hablar de otra cosa (talk about something else).
	{ 140, 1, 0, 3, 14, 14, 2, 0xff }, // Trapecio (trapeze): reveals Gwendolyn's full name.
	{ 141, 1, 0, 3, 15, 14, 2, 0xff }, // Comentario sobre trapecio (trapeze follow-up).
	{ 142, 1, 0, 3, 16, 15, 1, 0xff }, // Markus Hecker.
	{ 143, 1, 0, 3, 17, 16, 1, 0xff }, // Metodo adelgazante (slimming method).
	{ 144, 1, 0, 4, 10, 10, 0, 0x00 }  // Hablar de otra cosa (talk about something else).
};

PlayableSceneConfig scene4080Config() {
	PlayableSceneConfig config(4080,
		SceneResourceLayout(5, 5, 22),
		SceneViewport(kScene4080ViewportInitialX, kScene4080ViewportMinX, kScene4080ViewportMaxX),
		SceneActorPose(kScene4080DefaultActorX, kScene4080DefaultActorY, kScene4080DefaultActorFacing));
	config.setActorResources(kScene4080ActorBankTableEntry, kScene4080ActorPaletteTableEntry);
	config.setTextResources(kScene4080Resource003RowsOffsetIndex, kScene4080SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene4080::Scene4080(HollywoodEngine *vm) :
		PlayableScene(vm, scene4080Config()),
		_palettePatchLayer(),
		_foregroundFlickerLayer(),
		_scriptLayer(),
		_palettePatchChannel(),
		_foregroundFlickerChannel(),
		_originalColorToItemMap() {
}

void Scene4080::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	rememberOriginalColorMap();

	setActiveActorPose(kScene4080DefaultActorX, kScene4080DefaultActorY, kScene4080DefaultActorFacing);
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_scriptLayer.visible) {
		const uint foregroundChunk = _vm->gameState().scene4080CoffinShiftedState != 0 ?
			kScene4080SideForegroundChunk : kScene4080PaletteForegroundChunk;

		if (activeWorldY < kScene4080ForegroundActorThresholdY) {
			drawResourceSpriteLayer(_scriptLayer);
			if (_sceneChunkTable.isValidChunk(foregroundChunk))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[foregroundChunk], _sceneFramebuffer);
			drawResourceSpriteLayer(_palettePatchLayer);
		} else {
			drawResourceSpriteLayer(_palettePatchLayer);
			drawResourceSpriteLayer(_scriptLayer);
		}

		if (_scriptLayer.descriptorCount != kScene4080StakeSequenceDescriptorCount ||
				_scriptLayer.descriptorIndex() < 0x14)
			drawResourceSpriteLayer(_foregroundFlickerLayer);
		if (_sceneChunkTable.isValidChunk(kScene4080ForegroundBlockChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080ForegroundBlockChunk],
				_sceneFramebuffer);
		drawActionOverlayLayer();
		return;
	}

	if (activeWorldY < kScene4080ForegroundActorThresholdY) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		drawSceneLayers(activeWorldY);
	} else {
		drawResourceSpriteLayer(_palettePatchLayer);
		drawResourceSpriteLayer(_scriptLayer);
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	drawResourceSpriteLayer(_foregroundFlickerLayer);
	if (_sceneChunkTable.isValidChunk(kScene4080ForegroundBlockChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080ForegroundBlockChunk], _sceneFramebuffer);
	drawActionOverlayLayer();
}

void Scene4080::runCustomEntrySequence() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	_soundBank0.playSample(5, 100);
	_palettePatchLayer.setFrame(_vm->gameState().scene4080GwendolynState != 0 ? 1 : 0);
	_foregroundFlickerLayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynStateTransition == 0) {
		if (state.scene4080GwendolynState == 1)
			beginSecondarySpeechLine(0, 0);
		state.scene4080GwendolynStateTransition = 1;
	}
}

bool Scene4080::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene4080::advanceCustomGameplayLoop(uint32 delta) {
	advancePalettePatchLayer(delta);
	advanceForegroundFlickerLayer(delta);
	return false;
}

bool Scene4080::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta (look at door): corridor line.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Ir/usar/abrir puerta (go/use/open door): exit to the next D09/B09 flow state.
		runCorridorExit();
		return true;
	case 303: // Mirar ventana (look at window): full moon.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Abrir ventana (open window): broken unless Gwendolyn branch redirects.
		if (_vm->gameState().scene4080GwendolynState != 0)
			beginSecondarySpeechLine(20, 0);
		else
			beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Cerrar ventana / activar cambiazo lateral (close window / trigger side patch sequence).
		runSidePatchSequence();
		return true;
	case 306: // Coger ataud (take coffin): state-aware coffin/steak setup line.
		if (_vm->gameState().scene4080GwendolynState != 0)
			beginSecondarySpeechLine(4, 0);
		else if (_vm->gameState().scene4080CoffinShiftedState != 0)
			beginSecondarySpeechLine(4, 2);
		else
			beginSecondarySpeechLine(4, 1);
		return true;
	case 307: // Mirar ataud (look at coffin): wide/narrow coffin variant.
		beginSecondarySpeechLine(5, _vm->gameState().scene4080CoffinShiftedState == 0 ? 0 : 1);
		return true;
	case 308: // Mirar carne (look at meat): steak description.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Coger botella (take bottle): pick up bottle of oil/item 0x43.
		runBottlePickupSequence();
		return true;
	case 310: // Mirar botella (look at bottle): reveal/rename oil bottle if still present.
		if (_vm->gameState().scene4080OilBottleState == 1) {
			beginSecondarySpeechLine(7, 0);
			_vm->gameState().scene4080OilBottleState = 2;
			applySceneStateToHotspotsAndPatches(0xff);
		} else {
			beginSecondarySpeechLine(7, 1);
		}
		return true;
	case 311: // Ir/coger/usar lampara (go/take/use lamp): too high.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 312: // Mirar lampara (look at lamp): loose lamp.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Ir/coger/usar comida (go/take/use food): not hungry.
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Mirar comida (look at food): sweets and food everywhere.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Hablar con vampiresa/Gwendolyn (talk to vampiress/Gwendolyn): dialogue tree.
		runGwendolynDialogue();
		return true;
	case 316: // Coger vampiresa/Gwendolyn (take Gwendolyn): she would not allow it.
		beginSecondarySpeechLine(12, 0);
		return true;
	case 317: // Mirar vampiresa/Gwendolyn (look at Gwendolyn): state-aware description/dialogue.
		if (_vm->gameState().scene4080GwendolynState == 2)
			beginSecondarySpeechLine(23, 0);
		else
			beginSecondarySpeechLine(13, 0);
		return true;
	case 318: // Coger/usar bolsas de comida (take/use food bags): needs a tractor.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 319: // Mirar bolsas de comida (look at food bags): food bag description.
		beginSecondarySpeechLine(15, 0);
		return true;
	case 320: // Coger gominola (take gummy sweet): pick up dropped sweet/item 0x44.
		runGominolaPickupSequence();
		return true;
	case 321: // Mirar gominola (look at gummy sweet): Gwendolyn dropped it.
		beginSecondarySpeechLine(16, 0);
		return true;
	case 322: // Usar navaja con carne (use knife with meat): cut steak/item 0x45.
		runSteakPickupSequence();
		return true;
	case 323: // Usar estaca/golpe contra Gwendolyn (use stake/hit Gwendolyn): sleeping-state sequence.
		runUseStakeOnGwendolyn();
		return true;
	case 324: // Usar pildoras del Dr. Mabuse con bolsas de comida (use Mabuse pills with food bags).
		runUseMabusePillsOnFoodBags();
		return true;
	case 325: // Dar pildoras/tratamiento adelgazante a Gwendolyn (give slimming item to Gwendolyn).
		runGwendolynScriptedReply(20);
		return true;
	case 326: // Dar folleto adelgazante a Gwendolyn (give slimming flyer to Gwendolyn).
		runGwendolynScriptedReply(21);
		return true;
	default:
		return false;
	}
}

bool Scene4080::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0, 0x02c8);
	targetY = CLIP<int>(targetY, 0, 0x01df);

	while (targetY < 0x01df) {
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

bool Scene4080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynStateTransition == 2) {
		state.scene4080GwendolynStateTransition = 0;
		state.scene4080GwendolynState = 2;
	}

	if (state.scene4080GwendolynState == 0) {
		if (_sceneChunkTable.isValidChunk(kScene4080PaletteState0PatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080PaletteState0PatchChunk], _baseFramebuffer);
		replaceColorMapItemFromOriginal(8, 3);
		replaceColorMapItemFromOriginal(9, 3);
		replaceColorMapItemFromOriginal(13, 3);
		replaceColorMapItemFromOriginal(14, 3);
		replaceColorMapItemFromOriginal(16, 3);
		replaceColorMapItemFromOriginal(11, 0);
		replaceColorMapItemFromOriginal(12, 0);
		replaceColorMapItemFromOriginal(15, 0);
	} else if (state.scene4080GwendolynState == 1) {
		replaceColorMapItemFromOriginal(11, 8);
		replaceColorMapItemFromOriginal(14, 8);
		replaceColorMapItemFromOriginal(15, 8);
		replaceColorMapItemFromOriginal(12, 9);
		replaceColorMapItemFromOriginal(13, 9);
		replaceColorMapItemFromOriginal(16, 3);
	} else {
		replaceColorMapItemFromOriginal(8, 3);
		replaceColorMapItemFromOriginal(9, 3);
		replaceColorMapItemFromOriginal(11, 0);
		replaceColorMapItemFromOriginal(12, 0);
		replaceColorMapItemFromOriginal(13, 8);
		replaceColorMapItemFromOriginal(14, 8);
		replaceColorMapItemFromOriginal(15, 8);
		replaceColorMapItemFromOriginal(16, 8);
	}

	if (state.scene4080CoffinShiftedState == 0) {
		if (_sceneChunkTable.isValidChunk(kScene4080BaseSidePatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080BaseSidePatchChunk], _baseFramebuffer);
	} else {
		if (_sceneChunkTable.isValidChunk(kScene4080SidePatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080SidePatchChunk], _baseFramebuffer);
		copySmallTextRow(3, 12);
	}

	if (state.scene4080OilBottleState == 0) {
		if (_sceneChunkTable.isValidChunk(kScene4080PassagePatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080PassagePatchChunk], _baseFramebuffer);
		replaceColorMapItemFromOriginal(5, 0);
	} else if (state.scene4080OilBottleState == 2) {
		copySmallTextRow(5, 11);
	}

	if (state.scene4080GwendolynNameState == 1)
		copySmallTextRow(8, 13);
	else if (state.scene4080GwendolynNameState == 2)
		copySmallTextRow(8, 14);

	if (state.scene4080GominolaVisibleState != 0) {
		if (_sceneChunkTable.isValidChunk(kScene4080VisibleGominolaPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080VisibleGominolaPatchChunk], _baseFramebuffer);
		replaceColorMapItemFromOriginal(10, 10);
	} else {
		if (_sceneChunkTable.isValidChunk(kScene4080HiddenGominolaPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080HiddenGominolaPatchChunk], _baseFramebuffer);
		replaceColorMapItemFromOriginal(10, 0);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	configurePalettePatchLayerForState();
	return true;
}

AmbientAudioProfile Scene4080::ambientAudioProfile() const {
	AmbientAudioProfile profile =
		createRandomAmbientAudioProfile(0x0f, 8, 8, 25, 0x0b, 5, 100, 50);
	if (_vm->gameState().scene4080GwendolynState != 0)
		profile.soundMode = kAmbientSoundNone;
	return profile;
}

void Scene4080::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene4080GominolaPickupHook) {
		_vm->gameState().scene4080GominolaVisibleState = 0;
		applySceneStateToHotspotsAndPatches(0xff);
		return;
	}

	if (hookId == kScene4080StakeSequenceHook) {
		GameplayState &state = _vm->gameState();
		if (frame == 0x1d) {
			state.scene4080GwendolynState = 0;
			_soundBank0.playSample(0x18, 100);
			configurePalettePatchLayerForState();
		} else if (frame == 0x20) {
			_soundBank0.playSample(0x19, 100);
		}
		return;
	}

	PlayableScene::handleAnimationFrameHook(hookId, frame);
}

byte Scene4080::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return kScene4080GwendolynBaseFrame;
}

void Scene4080::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	configurePalettePatchLayerForState();
	_palettePatchLayer.setFrame(frameIndex);
}

void Scene4080::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	configurePalettePatchLayerForState();
	_palettePatchLayer.setFrame(baseFrame);
}

void Scene4080::resetAnimationLayers() {
	configurePalettePatchLayerForState();
	_palettePatchLayer.setFrame(_vm->gameState().scene4080GwendolynState != 0 ?
		kScene4080GwendolynBaseFrame : 0);
	_palettePatchChannel.reset(_palettePatchLayer.frameIndex, kScene4080FrameMillis);
	_foregroundFlickerLayer.configure(kScene4080ForegroundFlickerChunk, kScene4080ForegroundFlickerDescriptorCount,
		nullptr, 0);
	_foregroundFlickerLayer.visible = true;
	_foregroundFlickerLayer.setFrame(0);
	_foregroundFlickerChannel.reset(0, kScene4080FrameMillis);
	clearResourceLayer(_scriptLayer);
}

void Scene4080::configurePalettePatchLayerForState() {
	const byte paletteMapState = _vm->gameState().scene4080GwendolynState;
	if (paletteMapState == 1) {
		_palettePatchLayer.configure(kScene4080PalettePatchState1Chunk, kScene4080PalettePatchState1DescriptorCount,
			kScene4080PalettePatchState1FrameMap, ARRAYSIZE(kScene4080PalettePatchState1FrameMap));
		_palettePatchLayer.visible = true;
		return;
	}
	if (paletteMapState == 2) {
		_palettePatchLayer.configure(kScene4080PalettePatchState2Chunk, kScene4080PalettePatchState2DescriptorCount,
			kScene4080PalettePatchState2FrameMap, ARRAYSIZE(kScene4080PalettePatchState2FrameMap));
		_palettePatchLayer.visible = true;
		return;
	}

	_palettePatchLayer.visible = false;
}

void Scene4080::advancePalettePatchLayer(uint32 delta) {
	if (_vm->gameState().scene4080GwendolynState == 0 || _primaryDialogueSpeechActive)
		return;

	const uint frameCount = _palettePatchChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte paletteMapState = _vm->gameState().scene4080GwendolynState;
		byte nextFrame = _palettePatchLayer.frameIndex;
		if (paletteMapState == 1) {
			if (nextFrame > 0x18)
				nextFrame = 1;
			else if (nextFrame == 0x12 && _random.getRandomNumber(2) != 0)
				nextFrame = 1;
			else if (nextFrame == 0x18)
				nextFrame = 1;
			else
				++nextFrame;
			if (nextFrame == 0x14)
				_soundBank0.playSample(0x22, 100);
			const uint descriptorIndex = nextFrame < ARRAYSIZE(kScene4080PalettePatchState1FrameMap) ?
				kScene4080PalettePatchState1FrameMap[nextFrame] : 0xff;
			if (descriptorIndex < 10 && !_soundBank0.isPlaying())
				_soundBank0.playSample((byte)(0x1e + _random.getRandomNumber(3)), 100);
		} else {
			nextFrame = nextFrame == 0x18 ? 1 : (byte)(nextFrame + 1);
		}

		_palettePatchChannel.frameIndex = nextFrame;
		configurePalettePatchLayerForState();
		_palettePatchLayer.setFrame(nextFrame);
	}
}

void Scene4080::advanceForegroundFlickerLayer(uint32 delta) {
	const uint frameCount = _foregroundFlickerChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte nextFrame = (byte)_random.getRandomNumber(3);
		_foregroundFlickerChannel.frameIndex = nextFrame;
		_foregroundFlickerLayer.setFrame(nextFrame);
	}
}

void Scene4080::drawSceneLayers(int activeWorldY) {
	const uint foregroundChunk = _vm->gameState().scene4080CoffinShiftedState != 0 ?
		kScene4080SideForegroundChunk : kScene4080PaletteForegroundChunk;
	if (_sceneChunkTable.isValidChunk(foregroundChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[foregroundChunk], _sceneFramebuffer);
	drawResourceSpriteLayer(_palettePatchLayer);
	drawResourceSpriteLayer(_scriptLayer);
	(void)activeWorldY;
}

void Scene4080::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
		_originalColorToItemMap.size());
}

void Scene4080::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene4080::copySmallTextRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene4080::beginGwendolynSpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex,
		kScene4080GwendolynSpeechCenterX, kScene4080GwendolynSpeechTopY,
		kScene4080GwendolynSpeechRed, kScene4080GwendolynSpeechGreen, kScene4080GwendolynSpeechBlue,
		kScene4080GwendolynSpeechGroup);
}

void Scene4080::runCorridorExit() {
	playResourceLayerSequence(_scriptLayer, kScene4080ExitOverlayChunk,
		kScene4080ExitOverlayDescriptorCount, kScene4080ExitFrameMap, kScene4080FrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene4080ExitState;
}

void Scene4080::runSidePatchSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynState != 0) {
		beginSecondarySpeechLine(4, 0);
		return;
	}
	if (state.scene4080CoffinShiftedState != 0) {
		beginSecondarySpeechLine(4, 2);
		return;
	}

	beginSecondarySpeechLine(4, 1);
	playResourceLayerSequence(_scriptLayer, kScene4080GwendolynReplyChunk, 4,
		kScene4080SidePatchFrameMap, kScene4080FrameMillis);
	state.scene4080CoffinShiftedState = 1;
	applySceneStateToHotspotsAndPatches(0xff);
	beginSecondarySpeechLine(4, 3);
}

void Scene4080::runBottlePickupSequence() {
	if (hasInventoryItem(kScene4080OilBottleItem)) {
		beginSecondarySpeechLine(7, 1);
		return;
	}

	dispatchGenericSceneAction(21);
	runActorReplacement(ActionOverlaySpec(kScene4080BottlePickupChunk, kScene4080BottlePickupDescriptorCount,
		kScene4080BottlePickupFrameMap, ARRAYSIZE(kScene4080BottlePickupFrameMap), kScene4080FrameMillis)
		.endAt(ARRAYSIZE(kScene4080BottlePickupFrameMap)));
	addInventoryItem(kScene4080OilBottleItem);
	_soundBank0.playSample(1, 100);
	_vm->gameState().scene4080OilBottleState = 0;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4080::runGominolaPickupSequence() {
	if (hasInventoryItem(kScene4080GominolaItem)) {
		beginSecondarySpeechLine(16, 0);
		return;
	}

	dispatchGenericSceneAction(21);
	playResourceLayerSequence(_scriptLayer, kScene4080GominolaPickupChunk,
		kScene4080GominolaPickupDescriptorCount, kScene4080GominolaPickupFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4080GominolaPickupFrameMap) - 1,
			kScene4080FrameMillis).hookAt(7, kScene4080GominolaPickupHook));
	addInventoryItem(kScene4080GominolaItem);
	_soundBank0.playSample(1, 100);
}

void Scene4080::runSteakPickupSequence() {
	if (hasInventoryItem(kScene4080SteakItem)) {
		beginSecondarySpeechLine(17, 1);
		return;
	}

	beginSecondarySpeechLine(17, 0);
	runActorReplacement(ActionOverlaySpec(kScene4080SteakPickupChunk, kScene4080SteakPickupDescriptorCount,
		kScene4080SteakPickupFrameMap, ARRAYSIZE(kScene4080SteakPickupFrameMap), kScene4080FrameMillis)
		.endAt(ARRAYSIZE(kScene4080SteakPickupFrameMap)));
	addInventoryItem(kScene4080SteakItem);
	_soundBank0.playSample(1, 100);
}

void Scene4080::runUseMabusePillsOnFoodBags() {
	if (!hasInventoryItem(kScene4080MabusePillsItem)) {
		beginSecondarySpeechLine(18, 0);
		return;
	}

	playResourceLayerSequence(_scriptLayer, kScene4080FoodBagOverlayChunk,
		kScene4080FoodBagOverlayDescriptorCount, kScene4080FoodBagFrameMap,
		kScene4080FrameMillis);
	removeInventoryItem(kScene4080MabusePillsItem);
	_soundBank0.playSample(1, 100);
	_vm->gameState().scene4080GwendolynStateTransition = 2;
	applySceneStateToHotspotsAndPatches(0xff);
	beginSecondarySpeechLine(19, 0);
}

void Scene4080::runUseStakeOnGwendolyn() {
	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynState == 1) {
		beginSecondarySpeechLine(18, 3);
		return;
	}
	if (!hasInventoryItem(kScene4080StakeItem)) {
		beginSecondarySpeechLine(18, 0);
		return;
	}

	beginSecondarySpeechLine(18, 1);
	const bool completed = playResourceLayerSequence(_scriptLayer,
		kScene4080StakeSequenceChunk, kScene4080StakeSequenceDescriptorCount,
		kScene4080StakeSequenceFrameMap, AnimationFrameRange(0, 0x17, 60));
	if (completed) {
		playResourceLayerSequence(_scriptLayer, kScene4080StakeSequenceChunk,
			kScene4080StakeSequenceDescriptorCount, kScene4080StakeSequenceFrameMap,
			AnimationFrameRange(0x18, ARRAYSIZE(kScene4080StakeSequenceFrameMap) - 1, 40)
				.hookEveryFrame(kScene4080StakeSequenceHook));
	}
	_soundBank0.playSample(0x1b, 100);
	state.scene4080GominolaVisibleState = 1;
	applySceneStateToHotspotsAndPatches(0xff);
	beginSecondarySpeechLine(18, 2);
}

void Scene4080::runGwendolynScriptedReply(uint16 secondaryRow) {
	if (_vm->gameState().scene4080GwendolynState == 2) {
		beginSecondarySpeechLine(23, 0);
		return;
	}

	beginSecondarySpeechLine(secondaryRow, 0);
	playResourceLayerSequence(_scriptLayer, kScene4080GwendolynReplyChunk,
		kScene4080GwendolynReplyDescriptorCount, kScene4080GwendolynReplyFrameMap,
		kScene4080FrameMillis);
	beginGwendolynSpeechLine(20, 1);
}

void Scene4080::runGwendolynDialogue() {
	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynState == 2) {
		beginSecondarySpeechLine(23, 0);
		return;
	}

	if (state.scene4080GwendolynNameState == 0) {
		beginSecondarySpeechLine(98, 0);
		beginGwendolynSpeechLine(99, 0);
		beginSecondarySpeechLine(98, 1);
		beginGwendolynSpeechLine(99, 1);
		state.scene4080GwendolynNameState = 1;
		applySceneStateToHotspotsAndPatches(0xff);
	} else {
		beginSecondarySpeechLine(98, 2);
		beginGwendolynSpeechLine(99, 2);
	}

	Common::Array<DialogueChoiceRecord> records;
	initializeGwendolynDialogueRecords(records);
	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(98, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(98, 7);
			beginGwendolynSpeechLine(99, 7);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(98, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			beginGwendolynSpeechLine(99, record.responseFrameIndex);
		if (record.disableAfterUse != 0)
			record.enabled = 0;
		if (record.disableAfterUse == 2) {
			if (records.size() > 141) {
				records[140].enabled = 0;
				records[141].enabled = 0;
			}
			if (state.scene4080GwendolynNameState == 1) {
				state.scene4080GwendolynNameState = 2;
				applySceneStateToHotspotsAndPatches(0xff);
			}
		}

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth != 0 ? (byte)(previousDepth - 1) : 0;
			break;
		case 3:
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
			break;
		default:
			return;
		}
	}
}

void Scene4080::initializeGwendolynDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(10 * 10 * 7);
	for (uint i = 0; i < ARRAYSIZE(kScene4080DialogueSeedRecords); ++i) {
		const Scene4080DialogueSeedRecord &seed = kScene4080DialogueSeedRecords[i];
		DialogueChoiceRecord &record = records[seed.index];
		record.enabled = seed.enabled;
		record.nextNodeIndex = seed.nextNodeIndex;
		record.transitionMode = seed.transitionMode;
		record.playerTextRowId = seed.playerTextRowId;
		record.responseFrameIndex = seed.responseFrameIndex;
		record.disableAfterUse = seed.disableAfterUse;
		record.reserved = seed.reserved;
	}
}

} // End of namespace Hollywood
