/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file for details.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hollywood/scenes/playable/scene6090.h"

#include "common/system.h"
#include "graphics/managed_surface.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene6090SoundArchiveName = "RESOURCE.S06";
const uint16 kScene9150State = 0x23be;
const uint kScene6090ActorBankTableEntry = 0x0038;
const uint kScene6090ActorPaletteTableEntry = 0x00cc;
const uint kScene6090Resource003RowsOffsetIndex = 0;
const uint32 kScene6090SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6090FrameMillis = 75;
const uint32 kScene6090FastFrameMillis = 40;
const uint32 kScene6090TiedRonFrameMillis = 150;
const uint32 kScene6090SpeechFrameMillis = 125;
const uint32 kScene6090AmbientCheckMillis = 250;
const byte kScene6090HannoverSpeechGroup = 0;
const byte kScene6090KarloffSpeechGroup = 1;
const byte kScene6090SueSpeechGroup = 2;
const byte kScene6090InvalidSpeechGroup = 0xff;
const byte kScene6090PrimarySpeechColor = 0xfb;

const byte kScene6090HannoverFrameMap[] = {
	0, 1, 2, 3, 4, 4, 5, 6, 7, 4, 3, 2, 1, 0, 8, 9,
	10, 11, 11, 12, 13, 14, 11, 10, 9, 8, 0, 0, 0, 0, 0, 0
};

const byte kScene6090KarloffFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 9, 18, 5, 4, 3, 2,
	1, 0, 1, 2, 3, 4, 5, 6, 10, 11, 12, 13, 13, 14, 15, 16,
	29, 12, 11, 10, 6, 5, 4, 3, 2, 1, 0, 19, 20, 21, 22, 23,
	24, 25, 24, 23, 24, 25, 0xff, 0xff, 24, 23, 22, 21, 20, 19, 0, 17
};

const byte kScene6090SueIdleFrameMap[] = {0, 1, 0, 2, 3, 4, 3, 2};

const byte kScene6090RescueRonFrameMap[] = {
	0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 28, 28, 28, 28, 28, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 41, 41, 41, 0
};

const byte kScene6090RescueApparatusFrameMap[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 22, 22, 22, 22, 0
};

const byte kScene6090RescueHannoverFrameMap[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 0
};

const byte kScene6090RescueKarloffFrameMap[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 30, 31, 32, 33, 34, 34, 34, 34, 34, 34, 35,
	36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 48, 48, 0
};

const byte kScene6090RescueEffectFrameMap[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0
};

const byte kScene6090FreedSueFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 26, 30, 28,
	30, 27, 26, 30, 27, 26, 28, 28, 30, 27, 26, 30, 27, 26, 28, 27,
	28, 26, 30, 26, 36, 35, 34, 35, 36, 26, 36, 35, 34, 35, 36, 26,
	36, 35, 34, 35, 36, 26, 36, 35, 34, 35, 36, 26, 37, 38, 30, 30,
	31, 32, 33, 30, 0, 0, 0, 0
};

const byte kScene6090SpecialEffectFrameMap[] = {0, 1, 2, 3, 4, 4, 3, 2, 1, 0};

static PlayableSceneConfig scene6090Config() {
	PlayableSceneConfig config(6090,
		SceneResourceLayout(25, 5, 18),
		SceneViewport(0),
		SceneActorPose(0x1e2, 0x10e, 4));
	config.setActorResources(kScene6090ActorBankTableEntry, kScene6090ActorPaletteTableEntry);
	config.setTextResources(kScene6090Resource003RowsOffsetIndex, kScene6090SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 1;
	return config;
}

Scene6090::Scene6090(HollywoodEngine *vm) :
		PlayableScene(vm, scene6090Config()),
		_leftAmbientLayer(),
		_rightAmbientLayer(),
		_tiedRonLayer(),
		_hannoverLayer(),
		_karloffLayer(),
		_sueFaceLayer(),
		_apparatusLayer(),
		_rescueRonLayer(),
		_rescueHannoverLayer(),
		_rescueKarloffLayer(),
		_rescueApparatusLayer(),
		_rescueEffectLayer(),
		_escapeBackdropLayer(),
		_freedSueLayer(),
		_specialEffectLayer(),
		_tiedRonChannel(),
		_ambientLayerChannel(),
		_sueIdleChannel(),
		_mechanismChannel(),
		_escapeChannel(),
		_paletteFadeChannel(),
		_secondaryEffectSound(),
		_escapePaletteSource(),
		_compositeMode(kIntroComposite),
		_tiedRonIdleFrame(0),
		_sueIdleMode(0),
		_sueIdleRepeatCount(0),
		_mechanismState(0),
		_speakerMode(0),
		_hannoverPoseMode(0),
		_interruptionCycleCount(1),
		_paletteFadeThreshold(1),
		_delayedEventDone(false),
		_postRescue(false),
		_automaticEventRunning(false),
		_manualSequenceActive(false),
		_freedSueActive(false),
		_escapeAnimationActive(false),
		_escapePaletteActive(false),
		_paletteLockedDark(false),
		_muffledSpeechStarted(false),
		_asyncPrimaryActive(false),
		_asyncPrimaryAnimated(false),
		_asyncTextRecordId(0),
		_asyncVoiceSampleId(0),
		_asyncCenterX(0),
		_asyncTopY(0),
		_asyncPartCount(0),
		_asyncPartIndex(0),
		_asyncAnimationGroup(kScene6090InvalidSpeechGroup),
		_asyncColorIndex(kScene6090PrimarySpeechColor),
		_asyncVolumePercent(100),
		_asyncPartRemainingMillis(0) {
	_leftAmbientLayer.configure(5, 0x1a, nullptr, 0);
	_rightAmbientLayer.configure(6, 0x1a, nullptr, 0);
	_tiedRonLayer.configure(7, 0x2a, nullptr, 0);
	_hannoverLayer.configure(8, 0x1a, kScene6090HannoverFrameMap,
		ARRAYSIZE(kScene6090HannoverFrameMap));
	_karloffLayer.configure(9, 0x31, kScene6090KarloffFrameMap,
		ARRAYSIZE(kScene6090KarloffFrameMap));
	_sueFaceLayer.configure(10, 5, kScene6090SueIdleFrameMap,
		ARRAYSIZE(kScene6090SueIdleFrameMap));
	_apparatusLayer.configure(15, 0x17, nullptr, 0);
	_rescueRonLayer.configure(7, 0x2a, kScene6090RescueRonFrameMap,
		ARRAYSIZE(kScene6090RescueRonFrameMap));
	_rescueHannoverLayer.configure(8, 0x1a, kScene6090RescueHannoverFrameMap,
		ARRAYSIZE(kScene6090RescueHannoverFrameMap));
	_rescueKarloffLayer.configure(9, 0x31, kScene6090RescueKarloffFrameMap,
		ARRAYSIZE(kScene6090RescueKarloffFrameMap));
	_rescueApparatusLayer.configure(15, 0x17, kScene6090RescueApparatusFrameMap,
		ARRAYSIZE(kScene6090RescueApparatusFrameMap));
	_rescueEffectLayer.configure(14, 0x0f, kScene6090RescueEffectFrameMap,
		ARRAYSIZE(kScene6090RescueEffectFrameMap));
	_escapeBackdropLayer.configure(14, 0x0f, nullptr, 0);
	_freedSueLayer.configure(18, 0x27, kScene6090FreedSueFrameMap,
		ARRAYSIZE(kScene6090FreedSueFrameMap));
	_specialEffectLayer.configure(16, 5, kScene6090SpecialEffectFrameMap,
		ARRAYSIZE(kScene6090SpecialEffectFrameMap));
	_secondaryEffectSound.setArchive(Common::Path(kScene6090SoundArchiveName));
}

void Scene6090::initializeCustomPreviewState() {
	const bool resumeGameplay = hasSavedActiveActorPoseForCurrentState();
	_activeActorWorldX = 0x1e2;
	_activeActorWorldY = 0x10e;
	_activeActorFacing = 4;
	clearRonInventory();
	restoreTiedSequencePalette();
	resetSceneLayers();
	if (resumeGameplay) {
		_karloffLayer.setFrame(0x31);
		_mechanismState = 3;
	}
	initializeDefaultPreviewState();
}

void Scene6090::clearRonInventory() {
	GameplayState &state = _vm->gameState();
	for (uint slot = 0; slot < GameplayState::kInventoryOwnerSlotStride; ++slot) {
		state.inventorySlotItemIdByOwner[0][slot] = 0;
		state.inventoryItemSlotByOwnerAndItemId[0][slot] = 0;
	}
	state.inventoryItemCountByOwner[0] = 0;
	state.inventoryFirstVisibleSlotByOwner[0] = GameplayState::kInventoryFirstSlot;
	state.inventoryPanelDirty = true;
}

void Scene6090::restoreTiedSequencePalette() {
	const uint paletteBytes = MIN<uint>(0x2bb,
		MIN<uint>(_paletteCurrent.size(), _paletteResource.size()));
	memcpy(_paletteCurrent.data(), _paletteResource.data(), paletteBytes);
	_surfaceState.rebuildPresentationPaletteRemapTable();
}

void Scene6090::resetSceneLayers() {
	_leftAmbientLayer.reset(0);
	_rightAmbientLayer.reset(0x0c);
	_tiedRonLayer.reset(0);
	_hannoverLayer.reset(0);
	_karloffLayer.reset(0);
	_sueFaceLayer.reset(0);
	_apparatusLayer.reset(0);
	_rescueRonLayer.reset(0);
	_rescueHannoverLayer.reset(0);
	_rescueKarloffLayer.reset(0);
	_rescueApparatusLayer.reset(0);
	_rescueEffectLayer.reset(0);
	_escapeBackdropLayer.reset(0x0e);
	_freedSueLayer.reset(0);
	_specialEffectLayer.reset(0);

	_leftAmbientLayer.visible = true;
	_rightAmbientLayer.visible = true;
	_tiedRonLayer.visible = true;
	_hannoverLayer.visible = true;
	_karloffLayer.visible = true;
	_sueFaceLayer.visible = true;
	_apparatusLayer.visible = true;
	_rescueRonLayer.visible = true;
	_rescueHannoverLayer.visible = true;
	_rescueKarloffLayer.visible = true;
	_rescueApparatusLayer.visible = true;
	_rescueEffectLayer.visible = true;
	_escapeBackdropLayer.visible = true;
	_freedSueLayer.visible = true;
	_specialEffectLayer.visible = true;

	_tiedRonChannel.reset(0, kScene6090TiedRonFrameMillis);
	_ambientLayerChannel.reset(0, kScene6090FrameMillis);
	_sueIdleChannel.reset(0, kScene6090FrameMillis);
	_mechanismChannel.reset(0, kScene6090FrameMillis);
	_escapeChannel.reset(0, kScene6090FrameMillis);
	_paletteFadeChannel.reset(0, kScene6090FrameMillis);
	_compositeMode = kIntroComposite;
	_tiedRonIdleFrame = 0;
	_sueIdleMode = 0;
	_sueIdleRepeatCount = 0;
	_mechanismState = 0;
	_speakerMode = 0;
	_hannoverPoseMode = 0;
	_interruptionCycleCount = 1;
	_paletteFadeThreshold = 1;
	_delayedEventDone = false;
	_postRescue = false;
	_automaticEventRunning = false;
	_manualSequenceActive = false;
	_freedSueActive = false;
	_escapeAnimationActive = false;
	_escapePaletteActive = false;
	_paletteLockedDark = false;
	_muffledSpeechStarted = false;
	_asyncPrimaryActive = false;
	_asyncPrimaryAnimated = false;
	_asyncPartRemainingMillis = 0;
	_escapePaletteSource.clear();
}

void Scene6090::drawActorFrames(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY) {
	if (!_paletteLockedDark) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel,
			activeWorldX, activeWorldY, drawSecondaryActor, secondaryFacing,
			secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		return;
	}

	if (drawSecondaryActor) {
		const int bottomY = drawSecondaryActorFrame(secondaryFacing, secondaryFrame,
			secondaryWorldX, secondaryWorldY);
		if (drawActiveActor)
			drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, bottomY);
	} else if (drawActiveActor) {
		drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, -1);
	}
}

void Scene6090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	copyBaseFramebufferToSceneFramebuffer();

	switch (_compositeMode) {
	case kIntroComposite:
		_tiedRonLayer.setFrame(drawSecondaryActor ? secondaryFrame : _tiedRonIdleFrame);
		drawResourceSpriteLayer(_leftAmbientLayer);
		drawResourceSpriteLayer(_rightAmbientLayer);
		drawResourceSpriteLayer(_tiedRonLayer);
		drawResourceSpriteLayer(_karloffLayer);
		drawResourceSpriteLayer(_apparatusLayer);
		drawResourceSpriteLayer(_hannoverLayer);
		drawResourceSpriteLayer(_sueFaceLayer);
		break;
	case kRescueComposite:
		drawResourceSpriteLayer(_leftAmbientLayer);
		drawResourceSpriteLayer(_rightAmbientLayer);
		drawResourceSpriteLayer(_rescueRonLayer);
		drawResourceSpriteLayer(_rescueKarloffLayer);
		drawResourceSpriteLayer(_rescueEffectLayer);
		drawResourceSpriteLayer(_sueFaceLayer);
		drawResourceSpriteLayer(_rescueApparatusLayer);
		drawResourceSpriteLayer(_rescueHannoverLayer);
		break;
	case kSpecialComposite:
		drawResourceSpriteLayer(_leftAmbientLayer);
		drawResourceSpriteLayer(_rightAmbientLayer);
		drawResourceSpriteLayer(_escapeBackdropLayer);
		drawResourceSpriteLayer(_specialEffectLayer);
		drawResourceSpriteLayer(_freedSueActive ? _freedSueLayer : _sueFaceLayer);
		break;
	case kEscapeComposite:
		drawResourceSpriteLayer(_leftAmbientLayer);
		drawResourceSpriteLayer(_rightAmbientLayer);
		drawResourceSpriteLayer(_escapeBackdropLayer);
		drawActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY);
		drawResourceSpriteLayer(_freedSueActive ? _freedSueLayer : _sueFaceLayer);
		break;
	}
}

bool Scene6090::shouldApplyGameplayPanelObjectPalette() const {
	return false;
}

void Scene6090::runCustomEntrySequence() {
	_manualSequenceActive = true;
	setActiveActorPose(0x1e2, 0x10e, 4);

	restoreTiedSequencePalette();
	resetSceneLayers();
	_manualSequenceActive = true;
	drawPlayableComposite();
	if (runCurtainRevealFromBlack())
		return;

	runOpeningConversation();
	_manualSequenceActive = false;
}

bool Scene6090::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene6090::playLayerTransition(ResourceSpriteLayer &layer, byte firstFrame, byte lastFrame,
		uint32 frameMillis) {
	return playAndPresentAnimationTransition(layer,
		AnimationTransition(firstFrame, lastFrame, lastFrame, frameMillis).unskippable());
}

bool Scene6090::runCurtainRevealFromBlack() {
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

void Scene6090::runOpeningConversation() {
	playLayerTransition(_karloffLayer, 0x11, 0x1b, kScene6090FastFrameMillis);
	_mechanismState = 2;
	beginPrimarySpeechLineWithAnimationGroup(15, 6, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);

	playLayerTransition(_hannoverLayer, 0, 4, kScene6090FastFrameMillis);
	_hannoverPoseMode = 1;
	beginPrimarySpeechLineWithAnimationGroup(15, 7, 0xb4, 0x7c,
		0x28, 0x16, 0x0b, kScene6090HannoverSpeechGroup);
	playLayerTransition(_karloffLayer, 0x20, 0x2a, kScene6090FastFrameMillis);
	_mechanismState = 0;
	playLayerTransition(_hannoverLayer, 9, 0x0d, kScene6090FastFrameMillis);
	_hannoverLayer.setFrame(0);
	_hannoverPoseMode = 0;

	playLayerTransition(_karloffLayer, 0x11, 0x1b, kScene6090FastFrameMillis);
	_mechanismState = 2;
	beginPrimarySpeechLineWithAnimationGroup(15, 8, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);
	beginSecondarySpeechLine(15, 9);

	playLayerTransition(_hannoverLayer, 0x0d, 0x11, kScene6090FrameMillis);
	_hannoverPoseMode = 2;
	beginPrimarySpeechLineWithAnimationGroup(15, 10, 0xbc, 0x7a,
		0x28, 0x16, 0x0b, kScene6090HannoverSpeechGroup);
	playLayerTransition(_karloffLayer, 0x20, 0x2a, kScene6090FastFrameMillis);
	_mechanismState = 0;
	playLayerTransition(_hannoverLayer, 0x16, 0x1a, kScene6090FrameMillis);
	_hannoverLayer.setFrame(0);
	_hannoverPoseMode = 0;
	beginSecondarySpeechLine(15, 11);

	playLayerTransition(_karloffLayer, 0x11, 0x1b, kScene6090FastFrameMillis);
	_mechanismState = 2;
	playLayerTransition(_hannoverLayer, 0x0d, 0x11, kScene6090FrameMillis);
	_hannoverPoseMode = 2;
	beginPrimarySpeechLineWithAnimationGroup(15, 12, 0xbc, 0x7a,
		0x28, 0x16, 0x0b, kScene6090HannoverSpeechGroup);
	playLayerTransition(_karloffLayer, 0x20, 0x2a, kScene6090FastFrameMillis);
	_mechanismState = 0;
	playLayerTransition(_hannoverLayer, 0x16, 0x1a, kScene6090FrameMillis);
	_hannoverLayer.setFrame(0);
	_hannoverPoseMode = 0;

	playLayerTransition(_karloffLayer, 0x11, 0x1b, kScene6090FastFrameMillis);
	_mechanismState = 2;
	beginPrimarySpeechLineWithAnimationGroup(15, 13, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);
	beginSecondarySpeechLine(15, 14);
	beginPrimarySpeechLineWithAnimationGroup(15, 15, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);

	playLayerTransition(_hannoverLayer, 0, 4, kScene6090FastFrameMillis);
	_hannoverPoseMode = 1;
	beginPrimarySpeechLineWithAnimationGroup(15, 16, 0xb4, 0x7c,
		0x28, 0x16, 0x0b, kScene6090HannoverSpeechGroup);
	playLayerTransition(_karloffLayer, 0x20, 0x2a, kScene6090FastFrameMillis);
	_mechanismState = 0;
	playLayerTransition(_hannoverLayer, 9, 0x0d, kScene6090FastFrameMillis);
	_hannoverLayer.setFrame(0);
	_hannoverPoseMode = 0;
	beginSecondarySpeechLine(15, 17);

	playLayerTransition(_karloffLayer, 0x11, 0x1b, kScene6090FastFrameMillis);
	_mechanismState = 2;
	playLayerTransition(_karloffLayer, 0x20, 0x2a, kScene6090FastFrameMillis);
	_mechanismState = 0;
	playLayerTransition(_karloffLayer, 0, 6, 10);
	_mechanismState = 1;
	beginPrimarySpeechLineWithAnimationGroup(15, 18, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);
	playLayerTransition(_karloffLayer, 0x0b, 0x11, kScene6090FastFrameMillis);
	_karloffLayer.setFrame(0);
	_mechanismState = 0;
	beginSecondarySpeechLine(15, 19);

	playLayerTransition(_karloffLayer, 0x11, 0x1b, kScene6090FastFrameMillis);
	_mechanismState = 2;
	playLayerTransition(_hannoverLayer, 0x0d, 0x11, kScene6090FrameMillis);
	_hannoverPoseMode = 2;
	beginPrimarySpeechLineWithAnimationGroup(15, 20, 0xbc, 0x7a,
		0x28, 0x16, 0x0b, kScene6090HannoverSpeechGroup);
	playLayerTransition(_karloffLayer, 0x20, 0x2a, kScene6090FastFrameMillis);
	_mechanismState = 0;
	playLayerTransition(_hannoverLayer, 0x16, 0x1a, kScene6090FrameMillis);
	_hannoverLayer.setFrame(0);
	_hannoverPoseMode = 0;
	playLayerTransition(_karloffLayer, 0, 6, 10);
	_mechanismState = 1;
	beginPrimarySpeechLineWithAnimationGroup(15, 21, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(15, 22, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);
	playLayerTransition(_karloffLayer, 0x0b, 0x11, kScene6090FastFrameMillis);
	_karloffLayer.setFrame(0);
	_mechanismState = 0;
	playLayerTransition(_karloffLayer, 0x11, 0x1b, kScene6090FastFrameMillis);
	_mechanismState = 2;
	beginPrimarySpeechLineWithAnimationGroup(15, 23, 0xd2, 0x82,
		0x20, 0x32, 0, kScene6090KarloffSpeechGroup);
	playLayerTransition(_karloffLayer, 0x20, 0x2a, kScene6090FastFrameMillis);
	_mechanismState = 0;
	playLayerTransition(_karloffLayer, 0x2a, 0x31, kScene6090FastFrameMillis);
	_mechanismState = 3;
}

bool Scene6090::prepareCustomGameplayLoop() {
	_tiedRonChannel.reset(_tiedRonIdleFrame, kScene6090TiedRonFrameMillis);
	_ambientLayerChannel.reset(_leftAmbientLayer.frameIndex, kScene6090FrameMillis);
	_sueIdleChannel.reset(_sueFaceLayer.frameIndex, kScene6090FrameMillis);
	_mechanismChannel.reset(_karloffLayer.frameIndex, kScene6090FrameMillis);
	_escapeChannel.reset(_freedSueLayer.frameIndex, kScene6090FrameMillis);
	_paletteFadeChannel.reset(0, kScene6090FrameMillis);
	_secondaryEffectSound.setArchive(Common::Path(kScene6090SoundArchiveName));
	_manualSequenceActive = false;
	return true;
}

bool Scene6090::advanceCustomGameplayLoop(uint32 delta) {
	advanceTiedRonIdle(delta);
	advanceAmbientLayers(delta);
	if (_asyncPrimaryActive)
		advanceAsyncPrimarySpeech(delta);
	else if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);

	if (_freedSueActive) {
		if (_escapeAnimationActive)
			advanceEscapeAnimation(delta);
	} else {
		advanceSueIdle(delta);
	}
	if (_escapePaletteActive)
		advanceEscapePalette(delta);
	if (!_manualSequenceActive && !_automaticEventRunning && _compositeMode == kIntroComposite)
		advanceMechanism(delta);

	updateAmbientAudioAndMusicCues(delta);
	return true;
}

void Scene6090::advanceTiedRonIdle(uint32 delta) {
	if (_compositeMode != kIntroComposite)
		return;
	if (_speechOverlay.visible) {
		_tiedRonIdleFrame = 0;
		_tiedRonChannel.resetTimer();
		return;
	}

	const uint frameCount = _tiedRonChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_tiedRonIdleFrame == 4)
			_tiedRonIdleFrame = 0;
		else if (_random.getRandomNumber(14) == 0)
			_tiedRonIdleFrame = 4;
	}
}

void Scene6090::advanceAmbientLayers(uint32 delta) {
	const uint frameCount = _ambientLayerChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		_leftAmbientLayer.setFrame((_leftAmbientLayer.frameIndex + 1) % 0x1a);
		_rightAmbientLayer.setFrame((_rightAmbientLayer.frameIndex + 1) % 0x1a);
	}
}

void Scene6090::advanceSueIdle(uint32 delta) {
	const uint frameCount = _sueIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		switch (_sueIdleMode) {
		case 0:
			if (_random.getRandomNumber(_postRescue ? 19 : 99) == 0) {
				_sueFaceLayer.setFrame(2);
				_sueIdleMode = 2;
				_sueIdleRepeatCount = (byte)(_random.getRandomNumber(5) + 2);
			} else if (_random.getRandomNumber(39) == 0) {
				_sueFaceLayer.setFrame(1);
				_sueIdleMode = 1;
			}
			break;
		case 1:
			_sueFaceLayer.setFrame(0);
			_sueIdleMode = 0;
			break;
		case 2:
			if (_sueFaceLayer.frameIndex == 7) {
				if (--_sueIdleRepeatCount == 0) {
					_sueFaceLayer.setFrame(0);
					_sueIdleMode = 0;
				} else {
					_sueFaceLayer.setFrame(2);
				}
			} else {
				_sueFaceLayer.setFrame(_sueFaceLayer.frameIndex + 1);
				if (_postRescue && !_asyncPrimaryActive &&
						!_primaryDialogueSpeechActive && !_speech.isPlaying()) {
					startAsyncPrimarySpeechLine(15, 33, 0x32, 0x78,
						0x3f, 0x28, 0x32, kScene6090InvalidSpeechGroup);
				}
			}
			break;
		}
	}
}

void Scene6090::advanceMechanism(uint32 delta) {
	const uint frameCount = _mechanismChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && !_automaticEventRunning; ++i) {
		switch (_mechanismState) {
		case 0:
			if (_speakerMode == 1) {
				_karloffLayer.setFrame(1);
				_mechanismState = 5;
			}
			break;
		case 1:
			if (!_asyncPrimaryActive) {
				if (_speakerMode == 1) {
					_speakerMode = 2;
					startAsyncPrimarySpeechLine(15, 27, 0xd2, 0x82,
						0x20, 0x32, 0, kScene6090KarloffSpeechGroup, 25);
				} else {
					_speakerMode = 0;
					_karloffLayer.setFrame(0x0c);
					_mechanismState = 6;
					if (_interruptionCycleCount == 8 && !_delayedEventDone) {
						_automaticEventRunning = true;
						runInterruptionClips();
						_delayedEventDone = true;
						if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested())
							beginSecondarySpeechLine(15, 25);
						_automaticEventRunning = false;
					} else {
						++_interruptionCycleCount;
					}
				}
			}
			break;
		case 3:
			if (_karloffLayer.frameIndex == 0x35) {
				if (_random.getRandomNumber(49) == 0) {
					_karloffLayer.setFrame(0x38);
					_mechanismState = 4;
					_hannoverLayer.setFrame(1);
					_hannoverPoseMode = 3;
				} else {
					_karloffLayer.setFrame(0x32);
				}
			} else {
				_karloffLayer.setFrame(_karloffLayer.frameIndex + 1);
			}
			break;
		case 4:
			if (_karloffLayer.frameIndex == 0x3e) {
				_karloffLayer.setFrame(0);
				_mechanismState = 0;
			} else {
				_karloffLayer.setFrame(_karloffLayer.frameIndex + 1);
			}
			break;
		case 5:
			if (_karloffLayer.frameIndex == 6)
				_mechanismState = 1;
			else
				_karloffLayer.setFrame(_karloffLayer.frameIndex + 1);
			break;
		case 6:
			if (_karloffLayer.frameIndex == 0x11) {
				_karloffLayer.setFrame(0x2b);
				_mechanismState = 7;
			} else {
				_karloffLayer.setFrame(_karloffLayer.frameIndex + 1);
			}
			break;
		case 7:
			if (_karloffLayer.frameIndex == 0x31)
				_mechanismState = 3;
			else
				_karloffLayer.setFrame(_karloffLayer.frameIndex + 1);
			break;
		default:
			break;
		}
		advanceHannoverPose();
	}
}

void Scene6090::advanceHannoverPose() {
	switch (_hannoverPoseMode) {
	case 1:
		if (_speakerMode == 2) {
			_hannoverLayer.setFrame(10);
			_hannoverPoseMode = 4;
		}
		break;
	case 3:
		if (_hannoverLayer.frameIndex == 4) {
			_hannoverPoseMode = 1;
			_speakerMode = 1;
			startAsyncPrimarySpeechLine(15, 26, 0xb4, 0x7c,
				0x28, 0x16, 0x0b, kScene6090HannoverSpeechGroup, 25);
		} else {
			_hannoverLayer.setFrame(_hannoverLayer.frameIndex + 1);
		}
		break;
	case 4:
		if (_hannoverLayer.frameIndex == 0x0d) {
			_hannoverLayer.setFrame(0);
			_hannoverPoseMode = 0;
		} else {
			_hannoverLayer.setFrame(_hannoverLayer.frameIndex + 1);
		}
		break;
	default:
		break;
	}
}

void Scene6090::startAsyncPrimarySpeechLine(uint16 rowIndex, byte frameIndex,
		uint16 centerX, uint16 topY, byte red, byte green, byte blue,
		byte animationGroup, byte volumePercent) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	stopAsyncPrimarySpeech();
	setPaletteEntry6Bit(kScene6090PrimarySpeechColor, red, green, blue);
	_asyncPrimaryActive = true;
	_asyncPrimaryAnimated = animationGroup != kScene6090InvalidSpeechGroup;
	_asyncTextRecordId = textRecordId;
	_asyncVoiceSampleId = voiceSampleId;
	_asyncCenterX = centerX;
	_asyncTopY = topY;
	_asyncPartCount = MAX<byte>(1, continuationCount);
	_asyncPartIndex = 0;
	_asyncAnimationGroup = animationGroup;
	_asyncColorIndex = kScene6090PrimarySpeechColor;
	_asyncVolumePercent = volumePercent;
	if (_asyncPrimaryAnimated) {
		const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
		_speechController.startPrimaryDialogueSpeech(animationGroup, baseFrame);
		primarySpeechAnimationStarted(animationGroup, baseFrame);
		setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
	}
	startAsyncPrimarySpeechPart();
}

void Scene6090::startAsyncPrimarySpeechPart() {
	while (_asyncPrimaryActive && _asyncPartIndex < _asyncPartCount) {
		const Common::String text = getResource003LargeTextRecord(
			_asyncTextRecordId + _asyncPartIndex);
		if (text.empty()) {
			++_asyncPartIndex;
			continue;
		}

		_primarySpeechOverlay.visible = true;
		_primarySpeechOverlay.colorIndex = _asyncColorIndex;
		wrapActorSpeechText(text, _asyncCenterX, _primarySpeechOverlay.lines);
		calculateSpeechOverlayBounds(_primarySpeechOverlay, _asyncCenterX, _asyncTopY,
			true, _activeActorWorldY);
		const uint16 sampleId = _asyncVoiceSampleId == 0 ? 0 :
			_asyncVoiceSampleId + _asyncPartIndex;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, _asyncVolumePercent);
		_asyncPartRemainingMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _primarySpeechOverlay.lines.size() * 1100);
		return;
	}
	stopAsyncPrimarySpeech();
}

void Scene6090::advanceAsyncPrimarySpeech(uint32 delta) {
	if (!_asyncPrimaryActive)
		return;
	if (_asyncPrimaryAnimated)
		advancePrimaryDialogueSpeechFrame(delta);

	while (_asyncPrimaryActive && delta >= _asyncPartRemainingMillis) {
		delta -= _asyncPartRemainingMillis;
		_speech.stop();
		_primarySpeechOverlay.visible = false;
		_primarySpeechOverlay.lines.clear();
		++_asyncPartIndex;
		startAsyncPrimarySpeechPart();
	}
	if (_asyncPrimaryActive)
		_asyncPartRemainingMillis -= delta;
}

void Scene6090::stopAsyncPrimarySpeech() {
	if (!_asyncPrimaryActive)
		return;

	_speech.stop();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
	if (_asyncPrimaryAnimated) {
		const byte baseFrame = primarySpeechAnimationBaseFrame(_asyncAnimationGroup);
		setPrimarySpeechAnimationFrame(_asyncAnimationGroup, baseFrame);
		_speechController.stopPrimaryDialogueSpeech(kScene6090InvalidSpeechGroup, 7);
		primarySpeechAnimationRestored(_asyncAnimationGroup, baseFrame);
	}
	_asyncPrimaryActive = false;
	_asyncPrimaryAnimated = false;
	_asyncPartRemainingMillis = 0;
}

void Scene6090::waitForAsyncPrimarySpeech() {
	while (_asyncPrimaryActive && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(25, false))
			break;
	}
}

void Scene6090::runInterruptionClips() {
	Common::Array<byte> firstPalette;
	Common::Array<byte> secondPalette;
	Common::Array<byte> firstDelta;
	Common::Array<byte> secondDelta;
	firstPalette.resize(0x300);
	secondPalette.resize(0x300);
	Graphics::ManagedSurface firstBase;
	Graphics::ManagedSurface secondBase;
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	firstBase.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight, format);
	secondBase.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight, format);

	if (!loadFixedChunk(19, firstPalette, 0x300) ||
			!loadFixedChunk(20, firstBase, kFrameBufferSize) ||
			!loadVariableChunk(21, firstDelta) ||
			!loadFixedChunk(22, secondPalette, 0x300) ||
			!loadFixedChunk(23, secondBase, kFrameBufferSize) ||
			!loadVariableChunk(24, secondDelta)) {
		warning("Scene 6090 failed to load an interruption clip resource");
		return;
	}

	Graphics::ManagedSurface savedScene;
	savedScene.copyFrom(_sceneFramebuffer);
	const Common::Array<byte> savedPalette = _paletteCurrent;
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	_paletteCurrent = firstPalette;
	_sceneFramebuffer.copyRectToSurface(firstBase.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	for (uint frame = 0; frame < 0x2e && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		if (frame == 4) {
			uint16 textRecordId = 0;
			byte continuationCount = 0;
			uint16 voiceSampleId = 0;
			if (getStage003Cue(15, 24, textRecordId, continuationCount, voiceSampleId) &&
					voiceSampleId != 0)
				_speech.playSample(voiceSampleId, 100);
		}
		drawClipFrameDeltaFromResource(firstDelta, 0, firstDelta.size(), 0x2f, (byte)frame);
		presentFrame();
		if (frame + 1 < 0x2e && waitDeltaClipFrameMillis(kScene6090FrameMillis))
			break;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	_paletteCurrent = secondPalette;
	_sceneFramebuffer.copyRectToSurface(secondBase.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	drawClipFrameDeltaFromResource(secondDelta, 0, secondDelta.size(), 0x38, 0);
	presentFrame();
	if (!waitDeltaClipFrameMillis(kScene6090FrameMillis)) {
		for (uint frame = 1; frame < 0x37 && !Engine::shouldQuit() &&
				!_vm->isSceneRestartRequested(); ++frame) {
			drawClipFrameDeltaFromResource(secondDelta, 0, secondDelta.size(), 0x38, (byte)frame);
			presentFrame();
			if (frame + 1 < 0x37 && waitDeltaClipFrameMillis(kScene6090FrameMillis))
				break;
		}
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	_sceneFramebuffer.copyRectToSurface(savedScene.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	_paletteCurrent = savedPalette;
	_surfaceState.rebuildPresentationPaletteRemapTable();
	presentFrame();
}

bool Scene6090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Acción imposible (impossible action).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(1, (byte)_random.getRandomNumber(1));
		return true;
	case 302: // Mirar cuerda (look at the rope).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Usar/Coger cuerda (use/take the rope): attempt the timed rescue.
		if (!_delayedEventDone) {
			stopAsyncPrimarySpeech();
			beginSecondarySpeechLine(1, (byte)_random.getRandomNumber(1));
		} else if (_speakerMode == 0) {
			stopAsyncPrimarySpeech();
			beginSecondarySpeechLine(3, 0);
		} else {
			stopAsyncPrimarySpeech();
			runRopeRescueSequence();
		}
		return true;
	case 304: // Mirar peso (look at the counterweight).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Mirar anilla (look at the ring).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(5, 0);
		return true;
	case 306: // Mirar brazo giratorio (look at the rotating arm).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(6, 0);
		return true;
	case 307: // Mirar gancho (look at the hook holding Sue).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(7, 0);
		return true;
	case 308: // Hablar con Sue (talk to Sue).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(8, 0);
		return true;
	case 309: // Mirar Sue (look at Sue).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(9, 0);
		return true;
	case 310: // Hablar con Hannover (talk to Hannover).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(10, 0);
		return true;
	case 311: // Mirar Hannover (look at Hannover).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(11, 0);
		return true;
	case 312: // Hablar con Karloff (talk to Karloff).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(12, 0);
		return true;
	case 313: // Mirar Karloff (look at Karloff).
		stopAsyncPrimarySpeech();
		beginSecondarySpeechLine(_mechanismState == 3 ? 14 : 13, 0);
		return true;
	default:
		return false;
	}
}

bool Scene6090::shouldPlayGameplayClickPath() const {
	return false;
}

byte Scene6090::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene6090HannoverSpeechGroup:
		return _hannoverPoseMode == 1 ? 5 : 0x12;
	case kScene6090KarloffSpeechGroup:
		return _mechanismState == 1 ? 7 : 0x1c;
	case kScene6090SueSpeechGroup:
		return 0x4f;
	default:
		return 0;
	}
}

uint32 Scene6090::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene6090SpeechFrameMillis;
}

void Scene6090::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	switch (animationGroup) {
	case kScene6090HannoverSpeechGroup:
		_hannoverLayer.setFrame(frameIndex);
		break;
	case kScene6090KarloffSpeechGroup:
		_karloffLayer.setFrame(frameIndex);
		break;
	case kScene6090SueSpeechGroup:
		_freedSueLayer.setFrame(frameIndex);
		break;
	default:
		break;
	}
}

void Scene6090::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
}

void Scene6090::applyPatchChunk(uint chunkIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _baseFramebuffer);
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene6090::runRopeRescueSequence() {
	_manualSequenceActive = true;
	_vm->gameState().currentAmbientMusicCueId = 0x11;
	_vm->gameplayMusic()->playMusicCue(0x11, 100);
	_compositeMode = kRescueComposite;
	for (uint frame = 0; frame < 0x2e && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		if (frame == 8)
			applyPatchChunk(11);
		else if (frame == 0x19) {
			_speech.stop();
			_soundBank0.playSample(0x16, 100);
		} else if (frame == 0x21) {
			_soundBank0.playSample(0x1f, 100);
		} else if (frame == 0x24) {
			_secondaryEffectSound.playSample(0x19, 75);
		}

		_rescueRonLayer.setFrame((byte)frame);
		_rescueHannoverLayer.setFrame((byte)frame);
		_rescueKarloffLayer.setFrame((byte)frame);
		_rescueApparatusLayer.setFrame((byte)frame);
		_rescueEffectLayer.setFrame((byte)frame);
		if (waitSceneMillis(kScene6090FrameMillis, false))
			return;
	}
	_rescueRonLayer.setFrame(0x2e);
	_rescueHannoverLayer.setFrame(0x2e);
	_rescueKarloffLayer.setFrame(0x2e);
	_rescueApparatusLayer.setFrame(0x2e);
	_rescueEffectLayer.setFrame(0x2e);
	drawPlayableComposite();
	presentFrame();
	_soundBank0.stop();
	_postRescue = true;
	applyPatchChunk(12);
	_compositeMode = kEscapeComposite;

	startAsyncPrimarySpeechLine(15, 30, 0x13e, 0x50,
		0x20, 0x32, 0, kScene6090InvalidSpeechGroup);
	walkActiveActorTo(0x17f, 0xf7, 5, 0);
	waitForAsyncPrimarySpeech();
	beginSecondarySpeechLine(15, 31);
	walkActiveActorTo(0xaa, 0x118, 5, 0);

	_compositeMode = kSpecialComposite;
	for (uint frame = 0; frame + 1 < ARRAYSIZE(kScene6090SpecialEffectFrameMap) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
		if (frame == 4) {
			applyPatchChunk(13);
			_secondaryEffectSound.playSample(0x17, 50);
		}
		_specialEffectLayer.setFrame((byte)frame);
		if (waitSceneMillis(kScene6090FrameMillis, false))
			return;
	}
	_specialEffectLayer.setFrame(ARRAYSIZE(kScene6090SpecialEffectFrameMap) - 1);
	drawPlayableComposite();
	presentFrame();
	stopAsyncPrimarySpeech();

	_compositeMode = kEscapeComposite;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_freedSueLayer.setFrame(0);
	_freedSueActive = true;
	_escapeAnimationActive = true;
	_escapeChannel.reset(0, kScene6090FrameMillis);
	_muffledSpeechStarted = false;
	beginSecondarySpeechLine(15, 34);
	walkActiveActorTo(0xf0, 0x115, 4, 0);

	_escapePaletteSource = _paletteCurrent;
	_paletteFadeThreshold = 1;
	_paletteFadeChannel.reset(0, kScene6090FrameMillis);
	_escapePaletteActive = true;
	_paletteLockedDark = true;
	while (_escapeAnimationActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(25, false))
			return;
	}
	stopAsyncPrimarySpeech();
	_freedSueLayer.setFrame(0x4f);

	beginPrimarySpeechLineWithAnimationGroup(15, 35, 0xe4, 0x78,
		0x3f, 0x28, 0x32, kScene6090SueSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(15, 36, 0xe4, 0x78,
		0x3f, 0x28, 0x32, kScene6090SueSpeechGroup);
	beginSecondarySpeechLine(15, 37);
	beginPrimarySpeechLineWithAnimationGroup(15, 38, 0xe4, 0x78,
		0x3f, 0x28, 0x32, kScene6090SueSpeechGroup);
	beginSecondarySpeechLine(15, 39);
	beginPrimarySpeechLineWithAnimationGroup(15, 40, 0xe4, 0x78,
		0x3f, 0x28, 0x32, kScene6090SueSpeechGroup);
	_soundBank0.playSample(0x15, 100);
	waitSceneMillis(2000, false);
	beginSecondarySpeechLine(15, 41);
	_vm->gameState().mainFlowStateId = kScene9150State;
}

void Scene6090::advanceEscapeAnimation(uint32 delta) {
	const uint frameCount = _escapeChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _escapeAnimationActive; ++i) {
		if (_freedSueLayer.frameIndex >= 0x4e) {
			_escapeAnimationActive = false;
			break;
		}
		_freedSueLayer.setFrame(_freedSueLayer.frameIndex + 1);
		if (_freedSueLayer.frameIndex > 0x1a && !_muffledSpeechStarted &&
				!_asyncPrimaryActive && !_primaryDialogueSpeechActive && !_speech.isPlaying()) {
			_muffledSpeechStarted = true;
			startAsyncPrimarySpeechLine(15, 33, 0xe4, 0x78,
				0x3f, 0x28, 0x32, kScene6090InvalidSpeechGroup);
		}
	}
}

void Scene6090::advanceEscapePalette(uint32 delta) {
	const uint frameCount = _paletteFadeChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _escapePaletteActive; ++i)
		dimEscapePaletteStep();
}

void Scene6090::dimEscapePaletteStep() {
	if (_escapePaletteSource.size() < 0x2bb || _paletteCurrent.size() < 0x2bb) {
		_escapePaletteActive = false;
		return;
	}

	for (uint color = 1; color < 0xe9; ++color) {
		for (uint component = 0; component < 3; ++component) {
			const uint offset = color * 3 + component;
			if (_escapePaletteSource[offset] >= _paletteFadeThreshold &&
					_paletteCurrent[offset] != 0)
				--_paletteCurrent[offset];
		}
	}
	if (_paletteFadeThreshold == 0x3f)
		_escapePaletteActive = false;
	else
		++_paletteFadeThreshold;
}

bool Scene6090::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene6090::runExitSideEffectsAfterLoop() {
	stopAsyncPrimarySpeech();
	_soundBank0.stop();
	_secondaryEffectSound.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
}

AmbientAudioProfile Scene6090::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene6090AmbientCheckMillis;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = 0x14;
	profile.soundVolumePercent = 60;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

} // End of namespace Hollywood
