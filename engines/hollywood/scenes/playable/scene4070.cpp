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

#include "hollywood/scenes/playable/scene4070.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene4070SoundArchiveName = "RESOURCE.S04";
const uint16 kScene4100EntryFromScene4070State = 0x1008;
const int kScene4070EntryStartX = 0x03a7;
const int kScene4070EntryStartY = 0x013a;
const byte kScene4070EntryFacing = 4;
const int kScene4070EntryTargetX = 0x02ee;
const int kScene4070EntryTargetY = 0x0168;
const int kScene4070PostTrophyRonX = 0x013b;
const int kScene4070PostTrophyRonY = 0x016e;
const uint16 kScene4070ViewportInitialX = 0x0128;
const uint16 kScene4070ViewportMinX = 0x0058;
const uint16 kScene4070ViewportMaxX = 0x0128;
const uint kScene4070ActorBankTableEntry = 0x0000;
const uint kScene4070ActorPaletteTableEntry = 0x00cc;
const uint kScene4070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4070FrameMillis = 75;
const uint32 kScene4070ScrollFrameMillis = 10;
const uint kScene4070ForegroundBlockChunk = 5;
const uint kScene4070RightPatchChunk = 6;
const uint kScene4070LeftPatchChunk = 7;
const uint kScene4070TrophyOpenChunk = 8;
const uint kScene4070TrophyOpenDescriptorCount = 0x1f;
const uint kScene4070DraculaChunk = 9;
const uint kScene4070DraculaDescriptorCount = 0x14;
const uint kScene4070TreatmentReturnChunk = 10;
const uint kScene4070TreatmentReturnDescriptorCount = 0x13;
const uint kScene4070RandomAmbientChunk = 11;
const uint kScene4070RandomAmbientDescriptorCount = 2;
const uint kScene4070AmbientChunk = 12;
const uint kScene4070AmbientDescriptorCount = 0x1a;
const uint kScene4070TrophyBaseChunk = 13;
const uint kScene4070TrophyBaseDescriptorCount = 0x13;
const uint kScene4070ForegroundHighChunk = 14;
const uint kScene4070TreatmentNearChunk = 15;
const uint kScene4070TreatmentNearDescriptorCount = 0x0e;
const int kScene4070SidePatchThresholdX = 0x0259;
const int kScene4070ForegroundYThreshold = 0x0186;
const byte kScene4070DraculaHotspotItem = 6;
const uint kScene4070DraculaMovementRecordIndex = 0x1e;
const byte kScene4070DraculaIdleFrame = 0x0b;
const byte kScene4070DraculaSpeechGroup = 0;
const byte kScene4070DraculaTextRed = 0x0a;
const byte kScene4070DraculaTextGreen = 0x19;
const byte kScene4070DraculaTextBlue = 0x3f;
const uint16 kScene4070DraculaSpeechCenterX = 0x01c7;
const uint16 kScene4070DraculaSpeechTopY = 0x00a7;
const byte kScene4070MabusePillsItem = 0x0e;
const byte kScene4070SlimmingTreatmentItem = 0x15;
const byte kScene4070SlimmingFlyerItem = 0x58;
const byte kScene4070FrankieLegsItem = 0x30;
const byte kScene4070FrankieTorsoItem = 0x42;
const byte kScene4070FrankieHeadItem = 0x4c;

enum Scene4070AnimationHookId {
	kScene4070TrophyScrollHook = 1,
	kScene4070TrophyOpenHook,
	kScene4070FrankiePartHook,
	kScene4070TreatmentNearHook,
	kScene4070TreatmentReturnHook
};

const byte kScene4070DraculaFrameMap[] = {
	1, 2, 3, 3, 2, 1, 3, 4, 5, 6, 11, 1, 7, 8, 9, 10,
	12, 13, 14, 15, 16, 17, 18, 19, 1, 0, 0, 0, 0, 0, 0, 0
};

const byte kScene4070TrophyBaseFrameMap[] = {
	0, 1, 2, 3, 4, 8, 9, 10, 9, 8, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 4, 8, 9, 10, 9, 8, 4,
	3, 2, 1, 0
};

const byte kScene4070TreatmentNearScriptFrameIndices[] = {
	0x0d, 0x0c, 0x0b, 0x0a, 9, 8, 7, 6, 5, 4, 3, 3, 3, 3, 2, 1,
	0, 0, 0, 0
};

const byte kScene4070TreatmentNearDraculaFrameIndices[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18
};

const byte kScene4070TreatmentReturnFrameIndices[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 0
};

const byte kScene4070FrankiePartItems[] = {
	kScene4070FrankieLegsItem,
	kScene4070FrankieTorsoItem,
	kScene4070FrankieHeadItem
};

struct Scene4070DialogueSeedRecord {
	uint16 index;
	byte enabled;
	byte nextNodeIndex;
	byte transitionMode;
	byte playerTextRowId;
	byte responseFrameIndex;
	byte disableAfterUse;
	byte reserved;
};

const Scene4070DialogueSeedRecord kScene4070DialogueSeedRecords[] = {
	{ 0, 1, 0, 3, 2, 2, 1, 0xff },   // Ensayando algun nuevo papel... (rehearsing a new role).
	{ 1, 1, 0, 3, 3, 3, 1, 0xff },   // Premios cinematograficos (movie awards).
	{ 2, 1, 0, 1, 4, 4, 1, 0xff },   // Actor favorito (favorite actor): enter personal topics.
	{ 3, 0, 1, 1, 5, 5, 1, 0xff },   // Acompanantes (companions): conditional Sherilyn/Gwendolyn branch.
	{ 4, 1, 2, 1, 6, 6, 1, 0xff },   // Fiesta de Hannover (Hannover party): enter party topics.
	{ 5, 1, 0, 0, 7, 7, 0, 0xff },   // Despedida (exit).
	{ 70, 1, 0, 3, 8, 8, 1, 0xff },  // Ejercicio (exercise).
	{ 71, 1, 0, 3, 9, 9, 1, 0xff },  // Metodo adelgazante (slimming method).
	{ 72, 1, 0, 3, 10, 10, 1, 0xff }, // Sueno/descanso (sleep/rest).
	{ 73, 1, 0, 2, 11, 11, 0, 0xff }, // Peso y edad (weight and age): return to main menu.
	{ 77, 0, 1, 3, 12, 12, 1, 0xff }, // Sherilyn: enabled after meeting Sherilyn.
	{ 78, 0, 1, 3, 13, 13, 1, 0xff }, // Gwendolyn: enabled after meeting Gwendolyn.
	{ 79, 1, 1, 3, 14, 14, 1, 0xff }, // Esqueletos (skeletons).
	{ 80, 1, 0, 2, 15, 15, 0, 0xff }, // Hablar de otra cosa (talk about something else): return to main menu.
	{ 84, 1, 2, 3, 16, 16, 1, 0xff }, // Frankenstein.
	{ 85, 1, 2, 3, 17, 17, 1, 0xff }, // Entrevista de Sue (Sue's interview).
	{ 86, 1, 2, 3, 18, 18, 1, 0xff }, // Relaciones (relationships).
	{ 87, 1, 2, 3, 19, 19, 1, 0xff }, // Sospechas (suspicions).
	{ 88, 1, 0, 2, 15, 15, 0, 0xff }  // Hablar de otra cosa (talk about something else): return to main menu.
};

PlayableSceneConfig scene4070Config() {
	PlayableSceneConfig config(4070,
		SceneResourceLayout(5, 5, 15),
		SceneViewport(kScene4070ViewportInitialX, kScene4070ViewportMinX, kScene4070ViewportMaxX),
		SceneActorPose(kScene4070EntryTargetX, kScene4070EntryTargetY, kScene4070EntryFacing));
	config.setActorResources(kScene4070ActorBankTableEntry, kScene4070ActorPaletteTableEntry);
	config.setTextResources(kScene4070Resource003RowsOffsetIndex, kScene4070SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene4070::Scene4070(HollywoodEngine *vm) :
		PlayableScene(vm, scene4070Config()),
		_randomAmbientLayer(),
		_ambientLayer(),
		_draculaLayer(),
		_scriptLayer(),
		_ambientChannel(),
		_rightSidePatchActive(false),
		_draculaDialogueIntroPlayed(false),
		_loopingSoundBank0(),
		_originalColorToItemMap() {
	_loopingSoundBank0.setArchive(Common::Path(kScene4070SoundArchiveName));
}

void Scene4070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	rememberOriginalColorMap();

	setActiveActorPose(kScene4070EntryTargetX, kScene4070EntryTargetY, kScene4070EntryFacing);
	_rightSidePatchActive = _activeActorWorldX >= kScene4070SidePatchThresholdX;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	if (drawActiveActor)
		setRightSidePatchActive(activeWorldX >= kScene4070SidePatchThresholdX, false);
	copyBaseFramebufferToSceneFramebuffer();
	drawSceneLayers(activeWorldY);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (activeWorldY < kScene4070ForegroundYThreshold && !_rightSidePatchActive &&
			_sceneChunkTable.isValidChunk(kScene4070ForegroundHighChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4070ForegroundHighChunk], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(kScene4070ForegroundBlockChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4070ForegroundBlockChunk], _sceneFramebuffer);
}

void Scene4070::runCustomEntrySequence() {
	if (_vm->gameState().scene4070DraculaStage >= 3)
		_vm->gameState().scene4070TrophyBaseOpened = true;
	resetAnimationLayers();
	_rightSidePatchActive = kScene4070EntryStartX >= kScene4070SidePatchThresholdX;
	applySceneStateToHotspotsAndPatches(0xff);

	runEntryPath(kScene4070EntryStartX, kScene4070EntryStartY, kScene4070EntryFacing,
		kScene4070EntryTargetX, kScene4070EntryTargetY);
	if (!_vm->gameState().scene4070EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene4070EntryLineSeen = true;
	}
}

bool Scene4070::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	_loopingSoundBank0.setArchive(Common::Path(kScene4070SoundArchiveName));
	_rightSidePatchActive = _activeActorWorldX >= kScene4070SidePatchThresholdX;
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene4070::advanceCustomGameplayLoop(uint32 delta) {
	advanceAmbientLayers(delta);
	updateSidePatchForActorPosition();
	if (!_loopingSoundBank0.isPlaying())
		_loopingSoundBank0.playSampleLooping(0x33, 100);
	return false;
}

bool Scene4070::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a corredor (go to corridor): leave Dracula's salon.
		runCorridorExit();
		return true;
	case 302: // Mirar corredor (look at corridor): it is dark/tentative.
		beginSecondarySpeechLine(1, _rightSidePatchActive ? 1 : 0);
		return true;
	case 303: // Mirar trofeo (look at trophy): award plaque.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar base del trofeo (look at trophy base): plaque/secret base hint.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Abrir base del trofeo (open trophy base): reveal Dracula or later grant Frankie parts.
		runTrophyBaseOpenAction();
		return true;
	case 306: // Mirar alfombra (look at carpet): carpet response.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar ventana (look at window): window/head variant.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Hablar con Dracula (talk to Dracula): Dracula dialogue.
		runDraculaDialogue();
		return true;
	case 309: // Mirar Dracula (look at Dracula): lion stare/Dracula look line.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar cabeza de tigre (look at tiger head).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Abrir cabeza de tigre (open tiger head): state-aware mouth response.
		beginSecondarySpeechLine(9, _rightSidePatchActive ? 1 : 0);
		return true;
	case 312: // Mirar cabeza de leon (look at lion head): Nairobi plaque.
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Mirar chimenea (look at fireplace): purgatory/fireplace line.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Mirar estanteria (look at bookcase): bookcase browsing line.
		beginSecondarySpeechLine(12, 0);
		return true;
	case 317: // Dar pildoras del Dr. Mabuse a Dracula (give Mabuse pills to Dracula).
		runPillsOnDracula();
		return true;
	case 318: // Dar tratamiento adelgazante a Dracula (give slimming treatment to Dracula).
		runSlimmingTreatmentSequence();
		return true;
	case 319: // Dar folleto de adelgazamiento a Dracula (give slimming flyer to Dracula).
		runFlyerOnDracula();
		return true;
	default:
		return false;
	}
}

bool Scene4070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const uint patchChunk = _rightSidePatchActive ? kScene4070RightPatchChunk : kScene4070LeftPatchChunk;
	if (_sceneChunkTable.isValidChunk(patchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
	applyDraculaHotspotState();

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene4070DraculaMovementRecordIndex,
		isDraculaVisible() ? 0 : 1);
	return true;
}

AmbientAudioProfile Scene4070::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0f, 5, 8, 25, 0x0b, 5, 100, 50);
}

void Scene4070::handleAnimationFrameHook(byte hookId, uint frame) {
	switch (hookId) {
	case kScene4070TrophyScrollHook:
		if (_viewportXOffset > _viewportMinXOffset + 8)
			_viewportXOffset -= 8;
		else
			_viewportXOffset = _viewportMinXOffset;
		break;
	case kScene4070TrophyOpenHook:
		if (frame == 0x11)
			_soundBank0.playSample(0x31, 100);
		else if (frame == 0x1c)
			_soundBank0.playSample(0x32, 100);
		break;
	case kScene4070FrankiePartHook:
		_soundBank0.playSample(1, 100);
		break;
	case kScene4070TreatmentNearHook:
		if (frame < ARRAYSIZE(kScene4070TreatmentNearDraculaFrameIndices))
			_draculaLayer.setFrame(kScene4070TreatmentNearDraculaFrameIndices[frame]);
		break;
	case kScene4070TreatmentReturnHook:
		if (frame < ARRAYSIZE(kScene4070TreatmentReturnFrameIndices))
			_draculaLayer.setFrame(kScene4070TreatmentReturnFrameIndices[frame]);
		break;
	default:
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		break;
	}
}

byte Scene4070::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return kScene4070DraculaIdleFrame;
}

void Scene4070::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_draculaLayer.visible = isDraculaVisible();
	_draculaLayer.setFrame(frameIndex);
}

void Scene4070::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	_draculaLayer.visible = isDraculaVisible();
	_draculaLayer.setFrame(baseFrame);
}

void Scene4070::resetAnimationLayers() {
	_randomAmbientLayer.configure(kScene4070RandomAmbientChunk, kScene4070RandomAmbientDescriptorCount, nullptr, 0);
	_randomAmbientLayer.visible = true;
	_randomAmbientLayer.setFrame(0);
	_ambientLayer.configure(kScene4070AmbientChunk, kScene4070AmbientDescriptorCount, nullptr, 0);
	_ambientLayer.visible = true;
	_ambientLayer.setFrame(0);
	_ambientChannel.reset(0, kScene4070FrameMillis);
	_draculaLayer.configure(kScene4070DraculaChunk, kScene4070DraculaDescriptorCount,
		kScene4070DraculaFrameMap, ARRAYSIZE(kScene4070DraculaFrameMap));
	_draculaLayer.visible = isDraculaVisible();
	_draculaLayer.setFrame(kScene4070DraculaIdleFrame);
	clearResourceLayer(_scriptLayer);
}

bool Scene4070::isDraculaVisible() const {
	const GameplayState &state = _vm->gameState();
	return state.scene4070TrophyBaseOpened || state.scene4070DraculaStage >= 3;
}

void Scene4070::setRightSidePatchActive(bool active, bool playSound) {
	if (_rightSidePatchActive == active)
		return;

	_rightSidePatchActive = active;
	applySceneStateToHotspotsAndPatches(0);
	if (playSound)
		_soundBank0.playSample(0x3c, 50);
}

void Scene4070::advanceAmbientLayers(uint32 delta) {
	const uint frameCount = _ambientChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte nextAmbientFrame = _ambientLayer.frameIndex == 0x19 ? 0 : (byte)(_ambientLayer.frameIndex + 1);
		_ambientChannel.frameIndex = nextAmbientFrame;
		_ambientLayer.setFrame(nextAmbientFrame);
		_randomAmbientLayer.setFrame((byte)_random.getRandomNumber(1));
	}
}

void Scene4070::updateSidePatchForActorPosition() {
	setRightSidePatchActive(_activeActorWorldX >= kScene4070SidePatchThresholdX, true);
}

void Scene4070::drawSceneLayers(int activeWorldY) {
	(void)activeWorldY;
	drawResourceSpriteLayer(_randomAmbientLayer);
	drawResourceSpriteLayer(_ambientLayer);
	if (isDraculaVisible()) {
		_draculaLayer.visible = true;
		drawResourceSpriteLayer(_draculaLayer);
	}
	drawResourceSpriteLayer(_scriptLayer);
	drawActionOverlayLayer();
}

void Scene4070::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
		_originalColorToItemMap.size());
}

void Scene4070::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene4070::applyDraculaHotspotState() {
	if (!isDraculaVisible())
		replaceColorMapItemFromOriginal(kScene4070DraculaHotspotItem, 0);
}

void Scene4070::beginDraculaSpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex,
		kScene4070DraculaSpeechCenterX, kScene4070DraculaSpeechTopY,
		kScene4070DraculaTextRed, kScene4070DraculaTextGreen, kScene4070DraculaTextBlue,
		kScene4070DraculaSpeechGroup);
}

void Scene4070::runCorridorExit() {
	_vm->gameState().mainFlowStateId = kScene4100EntryFromScene4070State;
}

void Scene4070::runTrophyBaseOpenAction() {
	GameplayState &state = _vm->gameState();
	if (!isDraculaVisible() && state.scene4070DraculaStage == 0) {
		const bool previousHideActiveActor = _hideActiveActor;
		_hideActiveActor = true;
		playResourceLayerSequence(_scriptLayer, kScene4070TrophyBaseChunk,
			kScene4070TrophyBaseDescriptorCount, kScene4070TrophyBaseFrameMap,
			AnimationFrameRange(0, 4, kScene4070FrameMillis));

		const uint scrollFrameCount = _viewportXOffset > _viewportMinXOffset ?
			(_viewportXOffset - _viewportMinXOffset + 7) / 8 : 0;
		if (scrollFrameCount != 0) {
			playResourceLayerSequence(_scriptLayer, kScene4070TrophyBaseChunk,
				kScene4070TrophyBaseDescriptorCount, kScene4070TrophyBaseFrameMap,
				AnimationFrameRange(0, scrollFrameCount - 1, kScene4070ScrollFrameMillis)
					.repeatFrame(4).hookEveryFrame(kScene4070TrophyScrollHook));
		}
		_hideActiveActor = previousHideActiveActor;

		_soundBank0.playSample(0x30, 100);
		playResourceLayerSequence(_scriptLayer, kScene4070TrophyOpenChunk,
			kScene4070TrophyOpenDescriptorCount,
			AnimationFrameRange(0, kScene4070TrophyOpenDescriptorCount - 1,
				kScene4070FrameMillis).hookEveryFrame(kScene4070TrophyOpenHook));

		state.scene4070TrophyBaseOpened = true;
		applySceneStateToHotspotsAndPatches(1);
		_draculaLayer.visible = true;
		_draculaLayer.setFrame(kScene4070DraculaIdleFrame);
		beginDraculaSpeechLine(4, 1);
		walkActiveActorTo(0x02f1, 0x0142, 1, 0, false);
		walkActiveActorTo(kScene4070PostTrophyRonX, kScene4070PostTrophyRonY, 1, 0, false);
		beginSecondarySpeechLine(4, 2);
		return;
	}

	if (isDraculaVisible()) {
		beginSecondarySpeechLine(4, 4);
		return;
	}

	runFrankiePartGrantSequence();
}

void Scene4070::runFrankiePartGrantSequence() {
	GameplayState &state = _vm->gameState();
	const byte rewardIndex = state.frankensteinPartRewardIndex();
	if (state.scene4070FrankiePartGranted != 0 ||
			rewardIndex >= ARRAYSIZE(kScene4070FrankiePartItems)) {
		beginSecondarySpeechLine(4, 4);
		return;
	}

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	playResourceLayerSequence(_scriptLayer, kScene4070TrophyBaseChunk,
		kScene4070TrophyBaseDescriptorCount, kScene4070TrophyBaseFrameMap,
		AnimationFrameRange(0, 0x0a, kScene4070FrameMillis));
	playResourceLayerSequence(_scriptLayer, kScene4070TrophyBaseChunk,
		kScene4070TrophyBaseDescriptorCount, kScene4070TrophyBaseFrameMap,
		AnimationFrameRange(0x0d, 0x23, kScene4070FrameMillis)
			.hookAt(0x18, kScene4070FrankiePartHook));
	_hideActiveActor = previousHideActiveActor;

	addInventoryItem(kScene4070FrankiePartItems[rewardIndex]);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(0x16, (byte)(rewardIndex * 2 + 1));
	state.setFrankensteinPartRewardIndex(rewardIndex + 1);
	state.scene4070FrankiePartGranted = 1;
}

void Scene4070::runSlimmingTreatmentSequence() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene4070SlimmingTreatmentItem) || state.scene4070SlimmingTreatmentApplied) {
		beginSecondarySpeechLine(16, 0);
		return;
	}
	if (!isDraculaVisible()) {
		beginSecondarySpeechLine(16, 0);
		return;
	}

	beginSecondarySpeechLine(16, 0);
	walkActiveActorTo(kScene4070PostTrophyRonX, kScene4070PostTrophyRonY, 1, 0, false);
	walkActiveActorTo(0x0185, 0x015b, 1, 0, false);
	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	playResourceLayerSequence(_scriptLayer, kScene4070TreatmentNearChunk,
		kScene4070TreatmentNearDescriptorCount, kScene4070TrophyBaseFrameMap,
		AnimationFrameRange(kScene4070TreatmentNearScriptFrameIndices,
			kScene4070FrameMillis).hookEveryFrame(kScene4070TreatmentNearHook));
	_hideActiveActor = previousHideActiveActor;
	removeInventoryItem(kScene4070SlimmingTreatmentItem);
	_soundBank0.playSample(1, 100);
	walkActiveActorTo(kScene4070PostTrophyRonX, kScene4070PostTrophyRonY, 1, 0, false);
	beginDraculaSpeechLine(16, 1);

	playResourceLayerSequence(_scriptLayer, kScene4070TreatmentReturnChunk,
		kScene4070TreatmentReturnDescriptorCount,
		AnimationFrameRange(kScene4070TreatmentReturnFrameIndices,
			kScene4070FrameMillis).hookEveryFrame(kScene4070TreatmentReturnHook));
	state.scene4070SlimmingTreatmentApplied = true;
	if (state.scene4010PillboxPickupState == 0)
		state.scene4010PillboxPickupState = 1;
	_draculaLayer.setFrame(kScene4070DraculaIdleFrame);
}

void Scene4070::runPillsOnDracula() {
	if (!hasInventoryItem(kScene4070MabusePillsItem)) {
		beginSecondarySpeechLine(15, 0);
		return;
	}
	beginSecondarySpeechLine(15, 2);
	beginDraculaSpeechLine(15, 3);
	beginSecondarySpeechLine(15, 4);
}

void Scene4070::runFlyerOnDracula() {
	if (!hasInventoryItem(kScene4070SlimmingFlyerItem)) {
		beginSecondarySpeechLine(17, 0);
		return;
	}
	beginSecondarySpeechLine(17, 0);
	beginDraculaSpeechLine(17, 1);
	beginSecondarySpeechLine(17, 2);
}

void Scene4070::runDraculaDialogue() {
	if (!isDraculaVisible()) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	if (!_draculaDialogueIntroPlayed) {
		beginSecondarySpeechLine(98, 0);
		beginDraculaSpeechLine(99, 0);
		_draculaDialogueIntroPlayed = true;
	} else {
		beginSecondarySpeechLine(98, 1);
		beginDraculaSpeechLine(99, 1);
	}

	Common::Array<DialogueChoiceRecord> records;
	initializeDraculaDialogueRecords(records);
	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(98, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(98, 7);
			beginDraculaSpeechLine(99, 7);
			return;
		}
		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(98, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			beginDraculaSpeechLine(99, record.responseFrameIndex);
		if (record.disableAfterUse == 1)
			record.enabled = 0;

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

void Scene4070::initializeDraculaDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(10 * 10 * 7);
	for (uint i = 0; i < ARRAYSIZE(kScene4070DialogueSeedRecords); ++i) {
		const Scene4070DialogueSeedRecord &seed = kScene4070DialogueSeedRecords[i];
		DialogueChoiceRecord &record = records[seed.index];
		record.enabled = seed.enabled;
		record.nextNodeIndex = seed.nextNodeIndex;
		record.transitionMode = seed.transitionMode;
		record.playerTextRowId = seed.playerTextRowId;
		record.responseFrameIndex = seed.responseFrameIndex;
		record.disableAfterUse = seed.disableAfterUse;
		record.reserved = seed.reserved;
	}

	const GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynNameState != 0) {
		records[3].enabled = 1;
		records[78].enabled = 1;
	}
	if (state.scene4060SherilynDialogueIntroSeen) {
		records[3].enabled = 1;
		records[77].enabled = 1;
	}
}

} // End of namespace Hollywood
