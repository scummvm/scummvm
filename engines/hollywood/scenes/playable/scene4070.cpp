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

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/frankenstein_reward.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/scenes/playable/scene4070.h"

namespace Hollywood {

const char *const kScene4070SoundArchiveName = "RESOURCE.S04";
const char *const kScene4070DemoSoundArchiveName = "RESOURCE.S0D";
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
const uint32 kScene4070TrophyOpenFrameMillis = 100;
const uint32 kScene4070TreatmentReturnFrameMillis = 100;
const uint32 kScene4070PrimarySpeechFrameMillis = 125;
const uint32 kScene4070DraculaIdleCheckMillis = 100;
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
const byte kScene4070CarpetHotspotItem = 4;
const byte kScene4070CarpetHotspotColor = 0x4d;
const byte kScene4070DraculaHotspotItem = 6;
const uint kScene4070DraculaMovementRecordIndex = 0x1e;
const byte kScene4070DraculaIdleFrame = 0x0b;
const byte kScene4070DraculaSpeechGroup = 0;
const byte kScene4070TrophySpeechGroup = 1;
const byte kScene4070DraculaIdleSpeechGroup = 2;
const byte kScene4070DraculaAlternateIdleSpeechGroup = 3;
const byte kScene4070TrophySpeechBaseFrame = 0x0a;
const byte kScene4070DraculaAlternateSpeechBaseFrame = 6;
const byte kScene4070DraculaIdleSpeechVolumePercent = 25;
const byte kScene4070DraculaTextRed = 0x0a;
const byte kScene4070DraculaTextGreen = 0x19;
const byte kScene4070DraculaTextBlue = 0x3f;
const uint16 kScene4070DraculaSpeechCenterX = 0x01c7;
const uint16 kScene4070DraculaSpeechTopY = 0x00a7;
const uint16 kScene4070DraculaAlternateSpeechCenterX = 0x019c;
const uint16 kScene4070DraculaAlternateSpeechTopY = 0x009f;
const uint16 kScene4070TrophySpeechCenterX = 0x02dc;
const uint16 kScene4070TrophySpeechTopY = 0x00ad;
const byte kScene4070SlimmingTreatmentItem = 0x15;
const byte kScene4070SlimmingFlyerItem = 0x58;

enum Scene4070AnimationHookId {
	kScene4070TrophyScrollHook = 1
};

enum Scene4070DraculaIdleState {
	kScene4070DraculaIdle,
	kScene4070DraculaTurning,
	kScene4070DraculaSpeaking,
	kScene4070DraculaReturning
};

enum Scene4070RealtimeSpeechId {
	kScene4070DraculaIdleSpeech = 1,
	kScene4070DraculaAlternateIdleSpeech
};

enum {
	kScene4070RandomAmbientLayer,
	kScene4070AmbientLayer,
	kScene4070DraculaLayer,
	kScene4070ScriptLayer
};

const byte kScene4070DraculaFrameMap[] = {
	1, 2, 3, 3, 2, 1, 3, 4, 5, 6, 11, 1, 7, 8, 9, 10,
	12, 13, 14, 15, 16, 17, 18, 19, 1, 0, 0, 0, 0, 0, 0, 0
};

const byte kScene4070TreatmentNearDescriptorIndices[] = {
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

const SceneLayerSpec kScene4070LayerSpecs[] = {
	{ kSceneAnimationBehindActors, kScene4070RandomAmbientChunk,
		kScene4070RandomAmbientDescriptorCount, nullptr, 0, true, 0 },
	{ kSceneAnimationBehindActors, kScene4070AmbientChunk,
		kScene4070AmbientDescriptorCount, nullptr, 0, true, 0 },
	{ kSceneAnimationBehindActors, kScene4070DraculaChunk,
		kScene4070DraculaDescriptorCount, kScene4070DraculaFrameMap,
		ARRAYSIZE(kScene4070DraculaFrameMap), false, kScene4070DraculaIdleFrame },
	{ kSceneAnimationBehindActors, 0, 0, nullptr, 0, false, 0 }
};

struct Scene4070DialogueSeedRecord {
	uint16 index;
	byte enabled;
	byte nextNodeIndex;
	byte transitionMode;
	byte playerTextRowId;
	byte responseFrameIndex;
	byte disableAfterUse;
};

const Scene4070DialogueSeedRecord kScene4070DialogueSeedRecords[] = {
	{ 0, 1, 0, 3, 2, 2, 1 },
	{ 1, 1, 0, 3, 3, 3, 1 },
	{ 2, 1, 0, 1, 4, 4, 1 },
	{ 3, 0, 1, 1, 5, 5, 1 },
	{ 4, 1, 2, 1, 6, 6, 1 },
	{ 5, 1, 0, 0, 7, 7, 0 },
	{ 70, 1, 0, 3, 8, 8, 1 },
	{ 71, 1, 0, 3, 9, 9, 1 },
	{ 72, 1, 0, 3, 10, 10, 1 },
	{ 73, 1, 0, 2, 11, 11, 0 },
	{ 77, 0, 1, 3, 12, 12, 1 },
	{ 78, 0, 1, 3, 13, 13, 1 },
	{ 79, 1, 1, 3, 14, 14, 1 },
	{ 80, 1, 0, 2, 15, 15, 0 },
	{ 84, 1, 2, 3, 16, 16, 1 },
	{ 85, 1, 2, 3, 17, 17, 1 },
	{ 86, 1, 2, 3, 18, 18, 1 },
	{ 87, 1, 2, 3, 19, 19, 1 },
	{ 88, 1, 0, 2, 15, 15, 0 }
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
		_draculaIdleChannel(),
		_ambientTrack(RealtimeAnimationTracks::kInvalidTrack),
		_randomAmbientTrack(RealtimeAnimationTracks::kInvalidTrack),
		_draculaIdleSpeechTimerAccumulator(0),
		_rightSidePatchActive(false),
		_draculaIdleState(kScene4070DraculaIdle),
		_draculaThrowAnimationActive(false),
		_draculaDialogueMenuActive(false),
		_loopingSoundBank0(),
		_originalColorToItemMap() {
	_loopingSoundBank0.setArchive(Common::Path(_vm->isDemo() ?
		kScene4070DemoSoundArchiveName : kScene4070SoundArchiveName));
	_sceneLayers.configure(kScene4070LayerSpecs);
	_ambientTrack = _realtimeAnimationTracks.addLoop(kScene4070AmbientLayer,
		kScene4070FrameMillis, kScene4070AmbientDescriptorCount);
	_randomAmbientTrack = _realtimeAnimationTracks.addRandom(kScene4070RandomAmbientLayer,
		kScene4070FrameMillis, 0, kScene4070RandomAmbientDescriptorCount - 1, false);
}

void Scene4070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	rememberOriginalColorMap();

	setActiveActorPose(kScene4070EntryTargetX, kScene4070EntryTargetY, kScene4070EntryFacing);
	_rightSidePatchActive = _activeActorWorldX >= kScene4070SidePatchThresholdX;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4070::prepareCustomComposite(bool drawActors, byte activeFacing,
		int activeWorldX, int activeWorldY, byte actorDrawOrderMode) {
	(void)activeFacing;
	(void)activeWorldY;
	(void)actorDrawOrderMode;
	if (drawActors)
		setRightSidePatchActive(activeWorldX >= kScene4070SidePatchThresholdX, false);
	_sceneLayers.setLayerVisible(kScene4070DraculaLayer,
		isDraculaVisible() && !_draculaThrowAnimationActive);
}

void Scene4070::drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldX;
	(void)actorDrawOrderMode;
	if (activeWorldY < kScene4070ForegroundYThreshold && !_rightSidePatchActive &&
			_sceneChunkTable.isValidChunk(kScene4070ForegroundHighChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4070ForegroundHighChunk], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(kScene4070ForegroundBlockChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4070ForegroundBlockChunk], _sceneFramebuffer);
}

void Scene4070::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	state.scene4070TrophyBaseOpened = state.scene4070DraculaStage >= 3;
	resetAnimationLayers();
	_rightSidePatchActive = kScene4070EntryStartX >= kScene4070SidePatchThresholdX;
	applySceneStateToHotspotsAndPatches(0xff);

	runEntryPath(kScene4070EntryStartX, kScene4070EntryStartY, kScene4070EntryFacing,
		kScene4070EntryTargetX, kScene4070EntryTargetY);
	if (!state.scene4070EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene4070EntryLineSeen = true;
	}
}

void Scene4070::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	_loopingSoundBank0.setArchive(Common::Path(_vm->isDemo() ?
		kScene4070DemoSoundArchiveName : kScene4070SoundArchiveName));
	_rightSidePatchActive = _activeActorWorldX >= kScene4070SidePatchThresholdX;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4070::advanceCustomGameplayLoop(uint32 delta) {
	advanceDraculaIdle(delta);
	updateSidePatchForActorPosition();
	// The demo keeps this same fireplace loop in its showcase bank.
	if (!_loopingSoundBank0.isPlaying())
		_loopingSoundBank0.playSampleLooping(_vm->isDemo() ? 0x0e : 0x33, 100);
}

bool Scene4070::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a corredor (go to corridor): leave Dracula's salon.
		runCorridorExit();
		return true;
	case 302: // Mirar corredor (look at corridor): it is dark/tentative.
		beginSecondarySpeechLine(1, 0);
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
		beginSecondarySpeechLine(_rightSidePatchActive ? 8 : 9, 0);
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
	case 315: // Usar colmillo o filete con cabeza de tigre (use fang or steak on tiger head).
		beginSecondarySpeechLine(13, 0);
		return true;
	case 316: // Usar paja con chimenea (use straw on fireplace): it is unnecessary.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 317: // Dar pildoras del Dr. Mabuse a Dracula (give Mabuse pills to Dracula).
		beginSecondarySpeechLine(15, 0);
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
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	if (selector == 0 || selector == 0xff) {
		restoreBaseFramebufferFromOriginal();
		const uint patchChunk = _rightSidePatchActive ?
			kScene4070RightPatchChunk : kScene4070LeftPatchChunk;
		if (_sceneChunkTable.isValidChunk(patchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
	}

	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		if (_vm->restoredContentEnabled())
			_paletteMask[kSceneColorToItemMap + kScene4070CarpetHotspotColor] = kScene4070CarpetHotspotItem;
		applyDraculaHotspotState();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene4070DraculaMovementRecordIndex,
			isDraculaVisible() ? 0 : 1);
	}

	if (selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(),
			_fullPaletteRegionMask.size());
		rebuildWalkablePaletteMask();
	}
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
	default:
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		break;
	}
}

byte Scene4070::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene4070TrophySpeechGroup:
		return kScene4070TrophySpeechBaseFrame;
	case kScene4070DraculaAlternateIdleSpeechGroup:
		return kScene4070DraculaAlternateSpeechBaseFrame;
	default:
		return kScene4070DraculaIdleFrame;
	}
}

byte Scene4070::primarySpeechAnimationFrameCount(byte animationGroup) const {
	return animationGroup == kScene4070TrophySpeechGroup ? 4 : 5;
}

uint32 Scene4070::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene4070PrimarySpeechFrameMillis;
}

byte Scene4070::primarySpeechVolumePercent(byte animationGroup) const {
	return animationGroup == kScene4070DraculaIdleSpeechGroup ||
		animationGroup == kScene4070DraculaAlternateIdleSpeechGroup ?
		kScene4070DraculaIdleSpeechVolumePercent : 100;
}

void Scene4070::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene4070TrophySpeechGroup) {
		_sceneLayers.setLayerVisible(kScene4070ScriptLayer, true);
		_sceneLayers.setLayerFrame(kScene4070ScriptLayer, frameIndex);
		return;
	}

	_sceneLayers.setLayerVisible(kScene4070DraculaLayer, isDraculaVisible());
	_sceneLayers.setLayerFrame(kScene4070DraculaLayer, frameIndex);
}

void Scene4070::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	if (animationGroup == kScene4070TrophySpeechGroup) {
		_sceneLayers.setLayerVisible(kScene4070ScriptLayer, true);
		_sceneLayers.setLayerFrame(kScene4070ScriptLayer, baseFrame);
		return;
	}

	_sceneLayers.setLayerVisible(kScene4070DraculaLayer, isDraculaVisible());
	_sceneLayers.setLayerFrame(kScene4070DraculaLayer, baseFrame);
}

void Scene4070::resetAnimationLayers() {
	_sceneLayers.configure(kScene4070LayerSpecs);
	_realtimeAnimationTracks.reset(_randomAmbientTrack);
	_realtimeAnimationTracks.reset(_ambientTrack);
	_sceneLayers.setLayerVisible(kScene4070DraculaLayer, isDraculaVisible());
	_draculaIdleChannel.reset(kScene4070DraculaIdleFrame, kScene4070PrimarySpeechFrameMillis);
	_draculaIdleSpeechTimerAccumulator = 0;
	_draculaIdleState = kScene4070DraculaIdle;
	_draculaThrowAnimationActive = false;
	_draculaDialogueMenuActive = false;
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

void Scene4070::advanceDraculaIdle(uint32 delta) {
	ResourceSpriteLayer &draculaLayer = _sceneLayers.layer(kScene4070DraculaLayer);
	if (!isDraculaVisible() || _sceneLayers.layerVisible(kScene4070ScriptLayer) || _hideActiveActor)
		return;

	if (_primaryDialogueSpeechActive && !isRealtimeSpeechActive())
		return;

	_draculaIdleSpeechTimerAccumulator += delta;
	while (_draculaIdleSpeechTimerAccumulator >= kScene4070DraculaIdleCheckMillis) {
		_draculaIdleSpeechTimerAccumulator -= kScene4070DraculaIdleCheckMillis;
		switch (_draculaIdleState) {
		case kScene4070DraculaTurning:
			if (draculaLayer.frameIndex < 2)
				draculaLayer.setFrame(draculaLayer.frameIndex + 1);
			else
				beginDraculaIdleSpeechLine((byte)_random.getRandomNumber(2), true);
			break;
		case kScene4070DraculaReturning:
			if (draculaLayer.frameIndex < 5) {
				draculaLayer.setFrame(draculaLayer.frameIndex + 1);
			} else {
				_draculaIdleState = kScene4070DraculaIdle;
				draculaLayer.setFrame(kScene4070DraculaIdleFrame);
				_draculaIdleChannel.resetTimer();
			}
			break;
		case kScene4070DraculaSpeaking:
			break;
		case kScene4070DraculaIdle:
			if (_draculaDialogueMenuActive || _speechOverlay.visible ||
					_primarySpeechOverlay.visible || _actorPathPlaybackActive ||
					_random.getRandomNumber(49) != 0)
				break;
			if (_random.getRandomNumber(1) == 0)
				beginDraculaIdleSpeechLine((byte)_random.getRandomNumber(1), false);
			else {
				_draculaIdleState = kScene4070DraculaTurning;
				draculaLayer.setFrame(0);
			}
			break;
		}
	}

	if (_draculaIdleState != kScene4070DraculaIdle || _primaryDialogueSpeechActive)
		return;

	const uint idleTicks = _draculaIdleChannel.consumeFrames(delta);
	for (uint tick = 0; tick < idleTicks; ++tick) {
		if (draculaLayer.frameIndex == 0x0f)
			draculaLayer.setFrame(kScene4070DraculaIdleFrame);
		else if (draculaLayer.frameIndex == kScene4070DraculaIdleFrame &&
				_random.getRandomNumber(14) == 0)
			draculaLayer.setFrame(0x0f);
	}
}

void Scene4070::updateSidePatchForActorPosition() {
	setRightSidePatchActive(_activeActorWorldX >= kScene4070SidePatchThresholdX, true);
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

void Scene4070::beginDraculaIdleSpeechLine(byte frameIndex, bool alternatePose) {
	_draculaIdleState = kScene4070DraculaSpeaking;
	if (!startRealtimePrimarySpeechLine(18, frameIndex,
		alternatePose ? kScene4070DraculaAlternateSpeechCenterX : kScene4070DraculaSpeechCenterX,
		alternatePose ? kScene4070DraculaAlternateSpeechTopY : kScene4070DraculaSpeechTopY,
		kScene4070DraculaTextRed, kScene4070DraculaTextGreen, kScene4070DraculaTextBlue,
		alternatePose ? kScene4070DraculaAlternateIdleSpeechGroup : kScene4070DraculaIdleSpeechGroup,
		alternatePose ? kScene4070DraculaAlternateIdleSpeech : kScene4070DraculaIdleSpeech))
		stopDraculaIdleSpeech();
}

void Scene4070::realtimeSpeechEnded(byte speechId, bool completed) {
	if (speechId != kScene4070DraculaIdleSpeech && speechId != kScene4070DraculaAlternateIdleSpeech)
		return;

	const bool returnFromAlternatePose = completed && speechId == kScene4070DraculaAlternateIdleSpeech;
	_draculaIdleState = returnFromAlternatePose ? kScene4070DraculaReturning : kScene4070DraculaIdle;
	_sceneLayers.setLayerFrame(kScene4070DraculaLayer, returnFromAlternatePose ? 3 : kScene4070DraculaIdleFrame);
	_draculaIdleSpeechTimerAccumulator = 0;
	_draculaIdleChannel.resetTimer();
}

void Scene4070::stopDraculaIdleSpeech() {
	if (_draculaIdleState == kScene4070DraculaIdle)
		return;

	stopRealtimeSpeech();
	_draculaIdleState = kScene4070DraculaIdle;
	_sceneLayers.setLayerFrame(kScene4070DraculaLayer, kScene4070DraculaIdleFrame);
	_draculaIdleSpeechTimerAccumulator = 0;
	_draculaIdleChannel.resetTimer();
}

void Scene4070::beginTrophySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex,
		kScene4070TrophySpeechCenterX, kScene4070TrophySpeechTopY,
		0x3f, 0x3f, 0x3f, kScene4070TrophySpeechGroup);
}

void Scene4070::runCorridorExit() {
	_vm->gameState().mainFlowStateId = kScene4100EntryFromScene4070State;
}

void Scene4070::runTrophyBaseOpenAction() {
	GameplayState &state = _vm->gameState();
	if (!isDraculaVisible() && state.scene4070DraculaStage == 0) {
		const bool previousHideActiveActor = _hideActiveActor;
		_hideActiveActor = true;
		BlockingSequence sequence(*this);
		sequence.resourceLayerFrames(kScene4070ScriptLayer,
			kScene4070TrophyBaseChunk,
			kScene4070TrophyBaseDescriptorCount, kFrankensteinRewardFrameMap,
			AnimationFrameRange(0, 4, kScene4070FrameMillis).unskippable());

		const uint scrollFrameCount = _viewportXOffset > _viewportMinXOffset ?
			(_viewportXOffset - _viewportMinXOffset + 7) / 8 : 0;
		if (scrollFrameCount != 0) {
			sequence.resourceLayerFrames(kScene4070ScriptLayer,
				kScene4070TrophyBaseChunk,
				kScene4070TrophyBaseDescriptorCount, kFrankensteinRewardFrameMap,
				AnimationFrameRange(0, scrollFrameCount - 1, kScene4070ScrollFrameMillis)
					.repeatFrame(4).hookEveryFrame(kScene4070TrophyScrollHook).unskippable());
		}
		_hideActiveActor = previousHideActiveActor;
		if (!sequence.completed())
			return;

		sequence.sound(0x30)
			.resourceLayerFrames(kScene4070ScriptLayer, kScene4070TrophyOpenChunk,
				kScene4070TrophyOpenDescriptorCount,
				AnimationFrameRange(0, kScene4070TrophyOpenDescriptorCount - 1,
					kScene4070TrophyOpenFrameMillis)
					.soundAt(0x11, 0x31)
					.residentSoundAt(0x15, 4)
					.residentSoundAt(0x18, 4)
					.soundAt(0x1c, 0x32)
					.unskippable());
		if (!sequence.completed())
			return;

		state.scene4070TrophyBaseOpened = true;
		applySceneStateToHotspotsAndPatches(1);
		_sceneLayers.showLayerAtFrame(kScene4070DraculaLayer, kScene4070DraculaIdleFrame);
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

	if (state.scene4070DraculaStage == 1)
		state.scene4070DraculaStage = 2;
	if (state.scene4070DraculaStage == 2)
		runFrankiePartGrantSequence();
}

void Scene4070::runFrankiePartGrantSequence() {
	GameplayState &state = _vm->gameState();
	const byte rewardIndex = state.frankensteinPartRewardIndex();
	const bool grantsReward = state.scene4070FrankiePartGranted == 0 &&
		rewardIndex < kFrankensteinPartCount;

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4070ScriptLayer, kScene4070TrophyBaseChunk,
		kScene4070TrophyBaseDescriptorCount, kFrankensteinRewardFrameMap,
		AnimationFrameRange(0, 0x0a, kScene4070FrameMillis).unskippable(), false);
	if (!sequence.completed()) {
		clearSceneLayer(kScene4070ScriptLayer);
		_hideActiveActor = previousHideActiveActor;
		return;
	}

	if (grantsReward) {
		beginTrophySpeechLine(0x16, (byte)(rewardIndex * 2));
		sequence.resourceLayerFrames(kScene4070ScriptLayer,
			kScene4070TrophyBaseChunk,
			kScene4070TrophyBaseDescriptorCount, kFrankensteinRewardFrameMap,
			AnimationFrameRange(0x0d, 0x23, kScene4070FrameMillis)
				.soundAt(0x18, 1).unskippable());
	} else {
		beginTrophySpeechLine(0x66, 0);
		sequence.resourceLayerFrames(kScene4070ScriptLayer,
			kScene4070TrophyBaseChunk,
			kScene4070TrophyBaseDescriptorCount, kFrankensteinRewardFrameMap,
			AnimationFrameRange(0x19, 0x23, kScene4070FrameMillis).unskippable());
	}
	_hideActiveActor = previousHideActiveActor;
	if (!sequence.completed())
		return;
	if (!grantsReward) {
		drawPlayableComposite();
		presentFrame();
		return;
	}

	if (!hasInventoryItem(kFrankensteinPartItems[rewardIndex]))
		addInventoryItem(kFrankensteinPartItems[rewardIndex]);
	setActiveActorPose(0x02f1, 0x0142, 3);
	drawPlayableComposite();
	presentFrame();
	beginSecondarySpeechLine(0x16, (byte)(rewardIndex * 2 + 1));
	state.setFrankensteinPartRewardIndex(rewardIndex + 1);
	state.scene4070FrankiePartGranted = 1;
}

void Scene4070::runSlimmingTreatmentSequence() {
	stopDraculaIdleSpeech();
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene4070SlimmingTreatmentItem)) {
		beginSecondarySpeechLine(16, 0);
		return;
	}
	if (!isDraculaVisible()) {
		beginSecondarySpeechLine(16, 0);
		return;
	}

	beginSecondarySpeechLine(16, 0);
	const Common::Array<byte> savedFullPaletteRegionMask = _fullPaletteRegionMask;
	const Common::Array<byte> savedWalkablePaletteMask = _walkablePaletteMask;
	for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i)
		_fullPaletteRegionMask[i] = 1;
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i)
		_walkablePaletteMask[i] = 1;
	if (!walkActiveActorTo(kScene4070PostTrophyRonX, kScene4070PostTrophyRonY, 1, 0, false) ||
			!walkActiveActorTo(0x0185, 0x015b, 1, 0, false)) {
		_fullPaletteRegionMask = savedFullPaletteRegionMask;
		_walkablePaletteMask = savedWalkablePaletteMask;
		return;
	}

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4070ScriptLayer,
		kScene4070TreatmentNearChunk,
		kScene4070TreatmentNearDescriptorCount,
		AnimationFrameRange(kScene4070TreatmentNearDescriptorIndices,
			kScene4070FrameMillis)
			.mappedLayerFrames(kScene4070DraculaLayer,
				kScene4070TreatmentNearDraculaFrameIndices,
				ARRAYSIZE(kScene4070TreatmentNearDraculaFrameIndices))
			.unskippable());
	_hideActiveActor = previousHideActiveActor;
	if (!sequence.completed()) {
		_fullPaletteRegionMask = savedFullPaletteRegionMask;
		_walkablePaletteMask = savedWalkablePaletteMask;
		return;
	}

	_sceneLayers.setLayerFrame(kScene4070DraculaLayer, kScene4070DraculaIdleFrame);
	removeInventoryItem(kScene4070SlimmingTreatmentItem);
	_soundBank0.playSample(1, 100);
	const bool returnedToStart = walkActiveActorTo(kScene4070PostTrophyRonX,
		kScene4070PostTrophyRonY, 1, 0, false);
	_fullPaletteRegionMask = savedFullPaletteRegionMask;
	_walkablePaletteMask = savedWalkablePaletteMask;
	if (!returnedToStart)
		return;

	beginDraculaSpeechLine(16, 1);
	_draculaThrowAnimationActive = true;
	sequence.resourceLayerFrames(kScene4070ScriptLayer, kScene4070TreatmentReturnChunk,
			kScene4070TreatmentReturnDescriptorCount,
			AnimationFrameRange(kScene4070TreatmentReturnFrameIndices,
				kScene4070TreatmentReturnFrameMillis).unskippable());
	_draculaThrowAnimationActive = false;
	_sceneLayers.setLayerFrame(kScene4070DraculaLayer, kScene4070DraculaIdleFrame);
	if (!sequence.completed())
		return;

	state.scene4010PillboxPickupState = 1;
}

void Scene4070::runFlyerOnDracula() {
	stopDraculaIdleSpeech();
	if (!hasInventoryItem(kScene4070SlimmingFlyerItem)) {
		beginSecondarySpeechLine(17, 0);
		return;
	}
	beginSecondarySpeechLine(17, 0);
	beginDraculaSpeechLine(17, 1);
}

void Scene4070::runDraculaDialogue() {
	stopDraculaIdleSpeech();
	GameplayState &state = _vm->gameState();
	if (state.scene4070DraculaStage >= 3) {
		runLaterDraculaConversation();
		return;
	}
	if (state.scene4070DraculaStage != 0 || !isDraculaVisible())
		return;

	if (!state.scene4070DraculaDialogueIntroSeen) {
		beginSecondarySpeechLine(98, 0);
		beginDraculaSpeechLine(99, 0);
		state.scene4070DraculaDialogueIntroSeen = true;
	} else {
		beginSecondarySpeechLine(98, 1);
		beginDraculaSpeechLine(99, 1);
	}

	Common::Array<DialogueChoiceRecord> records;
	initializeDraculaDialogueRecords(records);
	_draculaDialogueMenuActive = true;
	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(98, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(98, 7);
			beginDraculaSpeechLine(99, 7);
			_draculaDialogueMenuActive = false;
			return;
		}
		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			_draculaDialogueMenuActive = false;
			return;
		}

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
			_draculaDialogueMenuActive = false;
			return;
		}
	}
	_draculaDialogueMenuActive = false;
}

void Scene4070::runLaterDraculaConversation() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(19, 0);
	beginDraculaSpeechLine(19, 1);
	if (!walkActiveActorTo(0x00c8, 0x01d6, 3, 0, false))
		return;
	beginSecondarySpeechLine(19, 2);
	if (state.scene4070DraculaStage != 3)
		return;

	beginStaticSecondarySpeechLine(0xd4, state.ronTravelQuipIndex);
	if (state.ronTravelQuipIndex < 0xff)
		++state.ronTravelQuipIndex;
	state.scene4070DraculaStage = 4;
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
