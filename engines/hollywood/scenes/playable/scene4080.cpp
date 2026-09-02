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
#include "hollywood/graphics.h"
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
const uint32 kScene4080GwendolynSpeechFrameMillis = 125;
const uint32 kScene4080AmbientCheckMillis = 250;
const uint32 kScene4080CoffinPaletteCycleMillis = 60;
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
const uint kScene4080SteakPickupChunk = 16;
const uint kScene4080SteakPickupDescriptorCount = 0x0c;
const uint kScene4080ExitOverlayChunk = 17;
const uint kScene4080ExitOverlayDescriptorCount = 6;
const uint kScene4080GominolaPickupChunk = 18;
const uint kScene4080GominolaPickupDescriptorCount = 0x0e;
const uint kScene4080VisibleGominolaPatchChunk = 19;
const uint kScene4080HiddenGominolaPatchChunk = 20;
const uint kScene4080FoodBagOverlayChunk = 21;
const uint kScene4080FoodBagOverlayDescriptorCount = 0x0c;
const uint kScene4080GwendolynReplyChunk = 22;
const uint kScene4080GwendolynReplyDescriptorCount = 6;
const uint kScene4080StakeSequenceChunk = 23;
const uint kScene4080StakeSequenceDescriptorCount = 0x1b;
const uint kScene4080CoffinBookendChunk = 24;
const uint kScene4080CoffinBookendDescriptorCount = 4;
const uint kScene4080CoffinFirstClipChunk = 25;
const uint kScene4080CoffinSecondClipChunk = 26;
const uint kScene4080CoffinClipDescriptorCount = 0x17;
const uint kScene4080CoffinFramebufferChunk = 27;
const uint kScene4080CoffinPaletteChunk = 28;
const uint kScene4080CoffinPaletteBytes = 0x210;
const uint16 kScene4080CoffinViewportX = 0x0070;
const byte kScene4080CoffinPaletteCycleFirstColor = 0xa8;
const byte kScene4080CoffinPaletteCycleLastColor = 0xaf;
const int kScene4080CurtainStartOffset = 0x00dc;
const byte kScene4080CurtainBandWidth = 0x14;
const uint kScene4080CurtainEndOffset = 0x00f0;
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

enum Scene4080GwendolynSpeechPoseMode {
	kScene4080GwendolynBodyAnimation,
	kScene4080GwendolynDialogueIdle,
	kScene4080GwendolynFrozenSpeechPose
};

enum {
	kScene4080PalettePatchLayer,
	kScene4080ForegroundFlickerLayer,
	kScene4080ScriptLayer
};

const SceneLayerSpec kScene4080LayerSpecs[] = {
	{ kSceneAnimationScenePlaced, 0, 0, nullptr, 0, false, 0 },
	{ kSceneAnimationScenePlaced, kScene4080ForegroundFlickerChunk,
		kScene4080ForegroundFlickerDescriptorCount, nullptr, 0, true, 0 },
	{ kSceneAnimationScenePlaced, 0, 0, nullptr, 0, false, 0 }
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

const byte kScene4080CoffinBookendForwardFrameMap[] = {
	0, 1, 2, 3
};

const byte kScene4080CoffinBookendReverseFrameMap[] = {
	0, 2, 1, 0
};

const byte kScene4080CoffinClipFrameMap[] = {
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22
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
		SceneResourceLayout(5, 5, 26),
		SceneViewport(kScene4080ViewportInitialX, kScene4080ViewportMinX, kScene4080ViewportMaxX),
		SceneActorPose(kScene4080DefaultActorX, kScene4080DefaultActorY, kScene4080DefaultActorFacing));
	config.setActorResources(kScene4080ActorBankTableEntry, kScene4080ActorPaletteTableEntry);
	config.setTextResources(kScene4080Resource003RowsOffsetIndex, kScene4080SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene4080::Scene4080(HollywoodEngine *vm) :
		PlayableScene(vm, scene4080Config()),
		_palettePatchChannel(),
		_gwendolynIdleChannel(),
		_foregroundFlickerTrack(RealtimeAnimationTracks::kInvalidTrack),
		_originalColorToItemMap(),
		_ambientSoundTimerAccumulator(0),
		_coffinPaletteCycleAccumulator(0),
		_previousAmbientSoundCue(0),
		_gwendolynSpeechPoseMode(kScene4080GwendolynBodyAnimation),
		_gwendolynSleepTransitionOnEntry(false) {
	_sceneLayers.configure(kScene4080LayerSpecs);
	_foregroundFlickerTrack = _realtimeAnimationTracks.addRandom(
		kScene4080ForegroundFlickerLayer, kScene4080FrameMillis, 0,
		kScene4080ForegroundFlickerDescriptorCount - 1, false);
}

void Scene4080::initializeCustomPreviewState() {
	GameplayState &state = _vm->gameState();
	_gwendolynSleepTransitionOnEntry = state.scene4080GwendolynStateTransition == 2;
	if (_gwendolynSleepTransitionOnEntry) {
		state.scene4080GwendolynStateTransition = 0;
		state.scene4080GwendolynState = 2;
	}

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

	const ResourceSpriteLayer &scriptLayer = _sceneLayers.layer(kScene4080ScriptLayer);
	copyBaseFramebufferToSceneFramebuffer();
	if (scriptLayer.visible) {
		const uint foregroundChunk = _vm->gameState().scene4080CoffinShiftedState != 0 ?
			kScene4080SideForegroundChunk : kScene4080PaletteForegroundChunk;

		if (activeWorldY < kScene4080ForegroundActorThresholdY) {
			drawSceneLayer(kScene4080ScriptLayer);
			if (_sceneChunkTable.isValidChunk(foregroundChunk))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[foregroundChunk], _sceneFramebuffer);
			drawSceneLayer(kScene4080PalettePatchLayer);
		} else {
			drawSceneLayer(kScene4080PalettePatchLayer);
			drawSceneLayer(kScene4080ScriptLayer);
		}

		if (scriptLayer.descriptorCount != kScene4080StakeSequenceDescriptorCount ||
				scriptLayer.descriptorIndex() < 0x14)
			drawSceneLayer(kScene4080ForegroundFlickerLayer);
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
		drawSceneLayer(kScene4080PalettePatchLayer);
		drawSceneLayer(kScene4080ScriptLayer);
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	drawSceneLayer(kScene4080ForegroundFlickerLayer);
	if (_sceneChunkTable.isValidChunk(kScene4080ForegroundBlockChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4080ForegroundBlockChunk], _sceneFramebuffer);
	drawActionOverlayLayer();
}

bool Scene4080::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene4080::runCustomEntrySequence() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	_soundBank0.playSample(5, 100);
	_sceneLayers.setLayerFrame(kScene4080PalettePatchLayer,
		_vm->gameState().scene4080GwendolynState != 0 ? 1 : 0);
	_sceneLayers.setLayerFrame(kScene4080ForegroundFlickerLayer, 0);
	setActiveActorPose(kScene4080DefaultActorX, kScene4080DefaultActorY,
		kScene4080DefaultActorFacing);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;

	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynStateTransition == 0) {
		if (state.scene4080GwendolynState == 1)
			beginSecondarySpeechLine(0, 0);
		state.scene4080GwendolynStateTransition = 1;
	}
	if (_gwendolynSleepTransitionOnEntry && _vm->restoredContentEnabled())
		beginSecondarySpeechLine(22, 0);
}

void Scene4080::runExitSideEffectsAfterLoop() {
	if (!didLeaveSceneAfterLoop())
		return;

	fadePaletteToBlack();
	stopAmbientSoundCues();
}

void Scene4080::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4080::advanceCustomGameplayLoop(uint32 delta) {
	advancePalettePatchLayer(delta);
	advanceAmbientSound(delta);
}

void Scene4080::advanceFullscreenAnimation(uint32 delta) {
	const bool paletteChanged = advanceCoffinPaletteCycle(delta);
	advanceAmbientSound(delta);
	PlayableScene::advanceFullscreenAnimation(delta);
	if (paletteChanged)
		presentFrame();
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
	case 304: // Abrir ventana / mirar gominola remapeada (open window / look at remapped gummy sweet).
		if (_vm->gameState().scene4080GwendolynState != 0)
			beginSecondarySpeechLine(16, 0);
		else
			beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Cerrar ventana / usar comida remapeada (close window / use remapped food).
		beginSecondarySpeechLine(_vm->gameState().scene4080GwendolynState != 0 ? 10 : 3, 0);
		return true;
	case 306: // Coger ataud (take coffin): move it after Gwendolyn is gone.
		runSidePatchSequence();
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
	case 317: // Mirar vampiresa/Gwendolyn (look at Gwendolyn): describe her.
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

	if (targetY < 0x01df)
		++targetY;
	while (targetY < 0x01df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;

	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

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
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4080AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

byte Scene4080::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return kScene4080GwendolynBaseFrame;
}

void Scene4080::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	configurePalettePatchLayerForState();
	_sceneLayers.setLayerFrame(kScene4080PalettePatchLayer, frameIndex);
}

void Scene4080::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	configurePalettePatchLayerForState();
	_sceneLayers.setLayerFrame(kScene4080PalettePatchLayer, baseFrame);
	if (_gwendolynSpeechPoseMode == kScene4080GwendolynDialogueIdle)
		_gwendolynIdleChannel.resetTimer();
}

void Scene4080::resetAnimationLayers() {
	const byte initialGwendolynFrame = _vm->gameState().scene4080GwendolynState != 0 ? 1 : 0;
	_sceneLayers.configure(kScene4080LayerSpecs);
	configurePalettePatchLayerForState();
	_sceneLayers.setLayerFrame(kScene4080PalettePatchLayer, initialGwendolynFrame);
	_palettePatchChannel.reset(_sceneLayers.layerFrame(kScene4080PalettePatchLayer),
		kScene4080FrameMillis);
	_realtimeAnimationTracks.reset(_foregroundFlickerTrack);
	_gwendolynIdleChannel.reset(kScene4080GwendolynBaseFrame,
		kScene4080GwendolynSpeechFrameMillis);
	_gwendolynSpeechPoseMode = kScene4080GwendolynBodyAnimation;
	_ambientSoundTimerAccumulator = 0;
	_previousAmbientSoundCue = 0;
	_coffinPaletteCycleAccumulator = 0;
	clearSceneLayer(kScene4080ScriptLayer);
}

void Scene4080::configurePalettePatchLayerForState() {
	ResourceSpriteLayer &palettePatchLayer = _sceneLayers.layer(kScene4080PalettePatchLayer);
	const byte paletteMapState = _vm->gameState().scene4080GwendolynState;
	if (paletteMapState == 1) {
		if (palettePatchLayer.chunkIndex != kScene4080PalettePatchState1Chunk ||
				palettePatchLayer.descriptorCount != kScene4080PalettePatchState1DescriptorCount ||
				palettePatchLayer.frameMap != kScene4080PalettePatchState1FrameMap) {
			_sceneLayers.setLayerResource(kScene4080PalettePatchLayer,
				kScene4080PalettePatchState1Chunk,
				kScene4080PalettePatchState1DescriptorCount,
				kScene4080PalettePatchState1FrameMap,
				ARRAYSIZE(kScene4080PalettePatchState1FrameMap));
		}
		_sceneLayers.setLayerVisible(kScene4080PalettePatchLayer, true);
		return;
	}
	if (paletteMapState == 2) {
		if (palettePatchLayer.chunkIndex != kScene4080PalettePatchState2Chunk ||
				palettePatchLayer.descriptorCount != kScene4080PalettePatchState2DescriptorCount ||
				palettePatchLayer.frameMap != kScene4080PalettePatchState2FrameMap) {
			_sceneLayers.setLayerResource(kScene4080PalettePatchLayer,
				kScene4080PalettePatchState2Chunk,
				kScene4080PalettePatchState2DescriptorCount,
				kScene4080PalettePatchState2FrameMap,
				ARRAYSIZE(kScene4080PalettePatchState2FrameMap));
		}
		_sceneLayers.setLayerVisible(kScene4080PalettePatchLayer, true);
		return;
	}

	_sceneLayers.setLayerVisible(kScene4080PalettePatchLayer, false);
}

void Scene4080::advancePalettePatchLayer(uint32 delta) {
	if (_vm->gameState().scene4080GwendolynState == 0 || _primaryDialogueSpeechActive)
		return;
	if (_gwendolynSpeechPoseMode == kScene4080GwendolynFrozenSpeechPose)
		return;
	ResourceSpriteLayer &palettePatchLayer = _sceneLayers.layer(kScene4080PalettePatchLayer);
	if (_gwendolynSpeechPoseMode == kScene4080GwendolynDialogueIdle) {
		const uint frameCount = _gwendolynIdleChannel.consumeFrames(delta);
		for (uint frame = 0; frame < frameCount; ++frame) {
			byte nextFrame = palettePatchLayer.frameIndex;
			if (nextFrame == 0x1d)
				nextFrame = kScene4080GwendolynBaseFrame;
			else if (_random.getRandomNumber(14) == 0)
				nextFrame = 0x1d;
			else
				continue;

			_gwendolynIdleChannel.frameIndex = nextFrame;
			palettePatchLayer.setFrame(nextFrame);
		}
		return;
	}

	const uint frameCount = _palettePatchChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte paletteMapState = _vm->gameState().scene4080GwendolynState;
		byte nextFrame = palettePatchLayer.frameIndex;
		if (paletteMapState == 1) {
			if (nextFrame > 0x18)
				nextFrame = 1;
			if (nextFrame == 0x12 && _random.getRandomNumber(2) != 0)
				nextFrame = 1;
			else if (nextFrame == 0x18)
				nextFrame = 1;
			else
				++nextFrame;
			if (nextFrame == 0x14)
				_additionalAmbientSoundBank0Slots[0].playSample(0x22, 100);
			const uint descriptorIndex = nextFrame < ARRAYSIZE(kScene4080PalettePatchState1FrameMap) ?
				kScene4080PalettePatchState1FrameMap[nextFrame] : 0xff;
			if (descriptorIndex < 10 && !_additionalAmbientSoundBank0Slots[0].isPlaying())
				_additionalAmbientSoundBank0Slots[0].playSample(
					(byte)(0x1e + _random.getRandomNumber(3)), 100);
		} else {
			nextFrame = nextFrame == 0x18 ? 1 : (byte)(nextFrame + 1);
		}

		_palettePatchChannel.frameIndex = nextFrame;
		configurePalettePatchLayerForState();
		palettePatchLayer.setFrame(nextFrame);
	}
}

void Scene4080::drawSceneLayers(int activeWorldY) {
	const uint foregroundChunk = _vm->gameState().scene4080CoffinShiftedState != 0 ?
		kScene4080SideForegroundChunk : kScene4080PaletteForegroundChunk;
	if (_sceneChunkTable.isValidChunk(foregroundChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[foregroundChunk], _sceneFramebuffer);
	drawSceneLayer(kScene4080PalettePatchLayer);
	drawSceneLayer(kScene4080ScriptLayer);
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

bool Scene4080::settleGwendolynForSpeech() {
	if (_vm->gameState().scene4080GwendolynState != 1)
		return true;

	ResourceSpriteLayer &palettePatchLayer = _sceneLayers.layer(kScene4080PalettePatchLayer);
	while (palettePatchLayer.descriptorIndex() > 3 && !animationPlaybackShouldStop()) {
		if (waitSceneMillis(kScene4080FrameMillis, false))
			return false;
	}
	if (animationPlaybackShouldStop())
		return false;

	palettePatchLayer.setFrame(kScene4080GwendolynBaseFrame);
	_palettePatchChannel.reset(kScene4080GwendolynBaseFrame, kScene4080FrameMillis);
	_gwendolynIdleChannel.reset(kScene4080GwendolynBaseFrame,
		kScene4080GwendolynSpeechFrameMillis);
	_additionalAmbientSoundBank0Slots[0].stop();
	drawPlayableComposite();
	presentFrame();
	return true;
}

void Scene4080::advanceAmbientSound(uint32 delta) {
	_ambientSoundTimerAccumulator += delta;
	while (_ambientSoundTimerAccumulator >= kScene4080AmbientCheckMillis) {
		_ambientSoundTimerAccumulator -= kScene4080AmbientCheckMillis;
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[1];
		if (_vm->gameState().scene4080GwendolynState != 0 ||
				player.isPlaying() || _random.getRandomNumber(24) != 0)
			continue;

		if (_random.getRandomNumber(9) == 0) {
			player.playSample(0x0e, 50);
			continue;
		}

		byte cue = 0;
		do {
			cue = (byte)(0x0f + _random.getRandomNumber(7));
		} while (cue == _previousAmbientSoundCue);
		_previousAmbientSoundCue = cue;
		player.playSample(cue, 8);
	}
}

void Scene4080::runCorridorExit() {
	BlockingSequence(*this)
		.resourceLayerFrames(kScene4080ScriptLayer, kScene4080ExitOverlayChunk,
			kScene4080ExitOverlayDescriptorCount, kScene4080ExitFrameMap,
			AnimationFrameRange(0, ARRAYSIZE(kScene4080ExitFrameMap) - 1,
				kScene4080FrameMillis).noFinalFrameDelay())
		.sound(3)
		.commit(_vm->gameState().mainFlowStateId, kScene4080ExitState);
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
	if (!runCoffinInsertSequence())
		return;
	beginSecondarySpeechLine(4, 3);
}

bool Scene4080::runCoffinInsertSequence() {
	for (uint chunk = kScene4080CoffinBookendChunk;
			chunk <= kScene4080CoffinPaletteChunk; ++chunk) {
		if (!_sceneChunkTable.isValidChunk(chunk)) {
			warning("Scene 4080 coffin insert is missing RESOURCE.D08 chunk %u", chunk);
			return false;
		}
	}

	Graphics::ManagedSurface insertFramebuffer;
	insertFramebuffer.create(HollywoodEngine::kSceneBufferWidth,
		HollywoodEngine::kSceneBufferHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Array<byte> insertPaletteChunk;
	insertPaletteChunk.resize(kPaletteSize);
	if (!loadFixedChunk(kScene4080CoffinFramebufferChunk, insertFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(kScene4080CoffinPaletteChunk, insertPaletteChunk, kPaletteSize)) {
		warning("Scene 4080 failed to load the coffin insert framebuffer or palette");
		return false;
	}

	const uint16 savedViewportX = _viewportXOffset;
	const Common::Array<byte> savedPalette = _paletteCurrent;
	Common::Array<byte> insertPalette = savedPalette;
	const uint paletteBytes = MIN<uint>(kScene4080CoffinPaletteBytes,
		MIN<uint>(_sceneChunkTable.sizes[kScene4080CoffinPaletteChunk], insertPalette.size()));
	memcpy(insertPalette.data(), insertPaletteChunk.data(), paletteBytes);
	if (insertPalette.size() >= kScene4080CoffinPaletteBytes + 3)
		memset(insertPalette.data() + kScene4080CoffinPaletteBytes, 0, 3);

	if (!playResourceLayerSequence(kScene4080ScriptLayer, kScene4080CoffinBookendChunk,
			kScene4080CoffinBookendDescriptorCount, kScene4080CoffinBookendForwardFrameMap,
			AnimationFrameRange(0, ARRAYSIZE(kScene4080CoffinBookendForwardFrameMap) - 1,
				kScene4080FrameMillis).unskippable().noFinalFrameDelay(), false)) {
		clearSceneLayer(kScene4080ScriptLayer);
		return false;
	}

	bool completed = runCoffinCurtainClear();
	if (completed) {
		_viewportXOffset = kScene4080CoffinViewportX;
		_sceneFramebuffer.copyRectToSurface(insertFramebuffer.rawSurface(), 0, 0,
			Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
				HollywoodEngine::kSceneBufferHeight));
		_coffinPaletteCycleAccumulator = 0;
		completed = runCoffinCurtainReveal(insertFramebuffer, insertPalette);
	}
	if (completed)
		completed = playCoffinDeltaClip(kScene4080CoffinFirstClipChunk);
	if (completed) {
		if (waitFullscreenAnimationFrame(kScene4080FrameMillis, false))
			completed = false;
		else
			completed = playCoffinDeltaClip(kScene4080CoffinSecondClipChunk);
	}
	if (completed)
		completed = runCoffinCurtainClear();

	_viewportXOffset = savedViewportX;
	if (completed) {
		GameplayState &state = _vm->gameState();
		state.scene4080CoffinShiftedState = 1;
		state.scene4090WideCoffinVariant = 1;
	}
	applySceneStateToHotspotsAndPatches(0xff);

	_sceneLayers.setLayerResource(kScene4080ScriptLayer, kScene4080CoffinBookendChunk,
		kScene4080CoffinBookendDescriptorCount, nullptr, 0);
	_sceneLayers.showLayerAtFrame(kScene4080ScriptLayer, 3);
	drawPlayableComposite();
	Graphics::ManagedSurface restoredRoom;
	restoredRoom.copyFrom(_sceneFramebuffer);

	if (!animationPlaybackShouldStop())
		completed = runCoffinCurtainReveal(restoredRoom, savedPalette) && completed;
	else {
		_sceneFramebuffer.copyRectToSurface(restoredRoom.rawSurface(), 0, 0,
			Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
				HollywoodEngine::kSceneBufferHeight));
		_paletteCurrent = savedPalette;
		invalidatePresentationPalette();
		completed = false;
	}

	if (completed) {
		completed = playResourceLayerSequence(kScene4080ScriptLayer, kScene4080CoffinBookendChunk,
			kScene4080CoffinBookendDescriptorCount, kScene4080CoffinBookendReverseFrameMap,
			AnimationFrameRange(0, ARRAYSIZE(kScene4080CoffinBookendReverseFrameMap) - 1,
				kScene4080FrameMillis).unskippable().noFinalFrameDelay());
	} else {
		clearSceneLayer(kScene4080ScriptLayer);
	}

	_paletteCurrent = savedPalette;
	invalidatePresentationPalette();
	return completed;
}

bool Scene4080::playCoffinDeltaClip(uint chunkIndex) {
	for (uint frame = 0; frame < ARRAYSIZE(kScene4080CoffinClipFrameMap); ++frame) {
		if (animationPlaybackShouldStop())
			return false;

		drawClipFrameDelta(chunkIndex, kScene4080CoffinClipDescriptorCount,
			kScene4080CoffinClipFrameMap[frame]);
		presentFrame();

		const uint frameCounter = frame + 1;
		if (frameCounter == 0x0b || frameCounter == 0x1b)
			_soundBank0.playSample(4, 100);
		if (frameCounter == 0x10 || frameCounter == 0x21)
			_soundBank0.playSample(5, 100);
		if (frameCounter == 0x0d || frameCounter == 0x0f || frameCounter == 0x14 ||
				frameCounter == 0x19 || frameCounter == 0x1d)
			playResidentSoundEffect(1);

		if (frame + 1 < ARRAYSIZE(kScene4080CoffinClipFrameMap) &&
				waitFullscreenAnimationFrame(kScene4080FrameMillis, false))
			return false;
	}
	return true;
}

bool Scene4080::advanceCoffinPaletteCycle(uint32 delta) {
	bool changed = false;
	_coffinPaletteCycleAccumulator += delta;
	while (_coffinPaletteCycleAccumulator >= kScene4080CoffinPaletteCycleMillis) {
		_coffinPaletteCycleAccumulator -= kScene4080CoffinPaletteCycleMillis;
		rotateCoffinPaletteCycle();
		changed = true;
	}
	return changed;
}

void Scene4080::rotateCoffinPaletteCycle() {
	const uint lastOffset = kScene4080CoffinPaletteCycleLastColor * 3;
	if (_paletteCurrent.size() <= lastOffset + 2)
		return;

	byte saved[3];
	memcpy(saved, _paletteCurrent.data() + lastOffset, sizeof(saved));
	for (uint color = kScene4080CoffinPaletteCycleLastColor;
			color > kScene4080CoffinPaletteCycleFirstColor; --color) {
		memcpy(_paletteCurrent.data() + color * 3,
			_paletteCurrent.data() + (color - 1) * 3, sizeof(saved));
	}
	memcpy(_paletteCurrent.data() + kScene4080CoffinPaletteCycleFirstColor * 3,
		saved, sizeof(saved));
	invalidatePresentationPalette();
}

bool Scene4080::runCoffinCurtainReveal(const Graphics::ManagedSurface &source,
		const Common::Array<byte> &palette) {
	byte *destination = framebufferPixels(_sceneFramebuffer);
	if (!destination || source.empty())
		return false;

	memset(destination, 0, framebufferByteCount());
	presentFrame();
	_paletteCurrent = palette;
	invalidatePresentationPalette();
	presentFrame();
	const Graphics::Surface &sourceSurface = source.rawSurface();
	for (int sweep = kScene4080CurtainStartOffset;
			sweep >= 0 && !animationPlaybackShouldStop(); sweep -= kScene4080CurtainBandWidth) {
		applyCoffinCurtainBand(&sourceSurface, (uint)sweep, kScene4080CurtainBandWidth);
		presentFrame();
		if (pollEvents(false))
			return false;
	}
	if (animationPlaybackShouldStop())
		return false;

	_sceneFramebuffer.copyRectToSurface(source.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
			HollywoodEngine::kSceneBufferHeight));
	presentFrame();
	return true;
}

bool Scene4080::runCoffinCurtainClear() {
	for (uint sweep = 0;
			sweep < kScene4080CurtainEndOffset && !animationPlaybackShouldStop();
			sweep += kScene4080CurtainBandWidth) {
		applyCoffinCurtainBand(nullptr, sweep, kScene4080CurtainBandWidth);
		presentFrame();
		if (pollEvents(false))
			break;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	invalidatePresentationPalette();
	presentFrame();
	return !animationPlaybackShouldStop();
}

void Scene4080::applyCoffinCurtainBand(const Graphics::Surface *source,
		uint sweepOffset, byte bandWidth) {
	const int innerWidth = HollywoodEngine::kScreenWidth - 2 * (int)sweepOffset;
	if (innerWidth <= 0)
		return;

	const int middleInset = sweepOffset + bandWidth;
	const int middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
	const int leftX = _viewportXOffset + sweepOffset;
	const int rightX = leftX + innerWidth - bandWidth;
	Graphics::Surface &destination = *_sceneFramebuffer.surfacePtr();

	for (uint row = 0; row < bandWidth; ++row) {
		const int topY = sweepOffset + row;
		const int bottomY = HollywoodEngine::kScreenHeight - bandWidth - sweepOffset + row;
		if (source) {
			copySurfaceRun(*source, destination, topY, leftX, innerWidth);
			copySurfaceRun(*source, destination, bottomY, leftX, innerWidth);
		} else {
			clearSurfaceRun(destination, topY, leftX, innerWidth);
			clearSurfaceRun(destination, bottomY, leftX, innerWidth);
		}
	}

	for (int row = 0; row < middleHeight; ++row) {
		const int y = middleInset + row;
		if (source) {
			copySurfaceRun(*source, destination, y, leftX, bandWidth);
			copySurfaceRun(*source, destination, y, rightX, bandWidth);
		} else {
			clearSurfaceRun(destination, y, leftX, bandWidth);
			clearSurfaceRun(destination, y, rightX, bandWidth);
		}
	}
}

void Scene4080::runBottlePickupSequence() {
	if (hasInventoryItem(kScene4080OilBottleItem)) {
		beginSecondarySpeechLine(7, 1);
		return;
	}

	dispatchGenericSceneAction(21);
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4080ScriptLayer, kScene4080BottlePickupChunk,
		kScene4080BottlePickupDescriptorCount, kScene4080BottlePickupFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4080BottlePickupFrameMap) - 1,
			kScene4080FrameMillis).noFinalFrameDelay());
	addInventoryItem(kScene4080OilBottleItem);
	sequence.sound(1)
		.commit(_vm->gameState().scene4080OilBottleState, (byte)0)
		.framebufferPatch(0xff);
}

void Scene4080::runGominolaPickupSequence() {
	if (hasInventoryItem(kScene4080GominolaItem)) {
		beginSecondarySpeechLine(16, 0);
		return;
	}

	dispatchGenericSceneAction(21);
	GameplayState &state = _vm->gameState();
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4080ScriptLayer, kScene4080GominolaPickupChunk,
		kScene4080GominolaPickupDescriptorCount, kScene4080GominolaPickupFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4080GominolaPickupFrameMap) - 1,
			kScene4080FrameMillis)
			.commitAt(7, state.scene4080GominolaVisibleState, (byte)0)
			.patchAt(7, 0xff)
			.noFinalFrameDelay())
		.commit(state.scene4080GominolaVisibleState, (byte)0)
		.framebufferPatch(0xff);
	addInventoryItem(kScene4080GominolaItem);
	sequence.sound(1);
}

void Scene4080::runSteakPickupSequence() {
	if (hasInventoryItem(kScene4080SteakItem)) {
		beginSecondarySpeechLine(17, 1);
		return;
	}

	beginSecondarySpeechLine(17, 0);
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4080ScriptLayer, kScene4080SteakPickupChunk,
		kScene4080SteakPickupDescriptorCount, kScene4080SteakPickupFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4080SteakPickupFrameMap) - 1,
			kScene4080FrameMillis).noFinalFrameDelay());
	addInventoryItem(kScene4080SteakItem);
	sequence.sound(1);
}

void Scene4080::runUseMabusePillsOnFoodBags() {
	if (!hasInventoryItem(kScene4080MabusePillsItem)) {
		beginSecondarySpeechLine(18, 0);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4080ScriptLayer, kScene4080FoodBagOverlayChunk,
		kScene4080FoodBagOverlayDescriptorCount, kScene4080FoodBagFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4080FoodBagFrameMap) - 1,
			kScene4080FrameMillis).noFinalFrameDelay());
	removeInventoryItem(kScene4080MabusePillsItem);
	sequence.sound(1)
		.commit(_vm->gameState().scene4080GwendolynStateTransition, (byte)2)
		.actorPath(SceneActorPose(0x00dc, 0x01cc, 3))
		.secondarySpeech(19, 0);
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
	if (!_sceneChunkTable.isValidChunk(kScene4080StakeSequenceChunk)) {
		warning("Scene 4080 stake sequence is missing RESOURCE.D08 chunk %u",
			kScene4080StakeSequenceChunk);
		return;
	}
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4080ScriptLayer, kScene4080StakeSequenceChunk,
		kScene4080StakeSequenceDescriptorCount, kScene4080StakeSequenceFrameMap,
		AnimationFrameRange(0, 0x17, 60).unskippable());
	if (!sequence.completed())
		return;
	sequence.resourceLayerFrames(kScene4080ScriptLayer, kScene4080StakeSequenceChunk,
		kScene4080StakeSequenceDescriptorCount, kScene4080StakeSequenceFrameMap,
		AnimationFrameRange(0x18, ARRAYSIZE(kScene4080StakeSequenceFrameMap) - 1, 40)
			.unskippable()
			.commitAt(0x1d, state.scene4080GwendolynState, (byte)0)
			.soundAt(0x1d, 0x18)
			.layerVisibleAt(0x1d, kScene4080PalettePatchLayer, false)
			.ambientSoundAt(0x20, 0x19, 100, 1)
			.ambientSoundAt(0x20, 0x1a, 100, 2)
			.noFinalFrameDelay());
	if (!sequence.completed())
		return;
	sequence.sound(0x1b)
		.commit(state.scene4080GominolaVisibleState, (byte)1)
		.framebufferPatch(0xff)
		.secondarySpeech(18, 2);
}

void Scene4080::runGwendolynScriptedReply(uint16 secondaryRow) {
	if (_vm->gameState().scene4080GwendolynState == 2) {
		beginSecondarySpeechLine(23, 0);
		return;
	}

	beginSecondarySpeechLine(secondaryRow, 0);
	if (!settleGwendolynForSpeech())
		return;
	_gwendolynSpeechPoseMode = kScene4080GwendolynFrozenSpeechPose;
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4080ScriptLayer, kScene4080GwendolynReplyChunk,
		kScene4080GwendolynReplyDescriptorCount, kScene4080GwendolynReplyFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4080GwendolynReplyFrameMap) - 1,
			kScene4080FrameMillis).unskippable().noFinalFrameDelay());
	if (sequence.completed())
		beginGwendolynSpeechLine(20, 1);
	_gwendolynSpeechPoseMode = kScene4080GwendolynBodyAnimation;
}

void Scene4080::runGwendolynDialogue() {
	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynState == 2) {
		beginSecondarySpeechLine(23, 0);
		return;
	}

	if (state.scene4080GwendolynNameState == 0) {
		beginSecondarySpeechLine(98, 0);
		if (!settleGwendolynForSpeech())
			return;
		_gwendolynSpeechPoseMode = kScene4080GwendolynDialogueIdle;
		beginGwendolynSpeechLine(99, 0);
		beginSecondarySpeechLine(98, 1);
		beginGwendolynSpeechLine(99, 1);
		state.scene4080GwendolynNameState = 1;
		applySceneStateToHotspotsAndPatches(0xff);
	} else {
		beginSecondarySpeechLine(98, 2);
		if (!settleGwendolynForSpeech())
			return;
		_gwendolynSpeechPoseMode = kScene4080GwendolynDialogueIdle;
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
			break;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

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
			finished = true;
			break;
		}
	}
	_gwendolynSpeechPoseMode = kScene4080GwendolynBodyAnimation;
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
