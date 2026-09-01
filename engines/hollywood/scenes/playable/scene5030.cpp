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

#include "hollywood/scenes/playable/scene5030.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5030ViewportXOffset = 0x00c8;
const uint16 kScene5030ViewportMaxXOffset = 0x0108;
const uint kScene5030ActorBankTableEntry = 0x0000;
const uint kScene5030ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5030FrameMillis = 75;
const uint kScene5030MineCartEntryDescriptorCount = 0x3e;
const byte kScene5030MineCartSoundFrame = 0x28;
const uint kScene5030Chunk8DescriptorCount = 0x1a;
const uint kScene5030Chunk9DescriptorCount = 0x17;
const uint kScene5030Chunk10DescriptorCount = 0x0d;
const uint kScene5030DeckAnimationDescriptorCount = 0x0e;
const uint kScene5030ConversationDescriptorCount = 0x0b;
const uint kScene5030UnderpantsRonDescriptorCount = 8;
const uint kScene5030UnderpantsVanessaDescriptorCount = 0x0e;
const uint kScene5030MineCartEntryLayer = 0;
const uint kScene5030Chunk8Layer = 1;
const uint kScene5030Chunk9Layer = 2;
const uint kScene5030Chunk10Layer = 3;
const uint kScene5030ActorReplacementLayer = 4;
const uint kScene5030AlternateVanessaLayer = 5;
const byte kScene5030DeckOfCardsItem = 0x48;
const byte kScene5030UnderpantsItem = 0x53;
const byte kScene5030TakenSceneItemId = 6;
const byte kScene5030RenamedSmallRowA = 7;
const byte kScene5030RenamedSmallRowB = 8;
const byte kScene5030DocumentSmallRowA = 9;
const byte kScene5030DocumentSmallRowB = 10;
const byte kScene5030VanessaDialogueStageId = 0x5a;
const byte kScene5030VanessaPrimaryRow = 99;
const byte kScene5030GladysReplyToVanessaRow = 0x5f;
const byte kScene5030GladysDialogueStageId = 0x62;
const byte kScene5030GladysPrimaryRow = 0x60;
const byte kScene5030VanessaReplyToGladysRow = 0x61;
const uint kScene5030DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene5030DialogueNoResponseFrame = 0xff;
const byte kScene5030DialogueTransitionEnd = 0;
const byte kScene5030DialogueTransitionDown = 1;
const byte kScene5030DialogueTransitionUp = 2;
const byte kScene5030DialogueTransitionStay = 3;
const byte kScene5030DialogueTransitionUpTwo = 4;
const uint16 kScene5030RonDialogueCenterX = 0x217;
const uint16 kScene5030RonDialogueTopY = 0x099;
const uint16 kScene5030RonTradeCenterX = 0x1fa;
const uint16 kScene5030RonTradeTopY = 0x0ae;
const uint16 kScene5030VanessaDialogueCenterX = 0x1b9;
const uint16 kScene5030VanessaDialogueTopY = 0x124;
const uint16 kScene5030GladysDialogueCenterX = 0x274;
const uint16 kScene5030GladysDialogueTopY = 0x11b;
const byte kScene5030GladysSpeechGroup = 0;
const byte kScene5030VanessaSpeechGroup = 1;
const byte kScene5030RonSpeechGroup = 2;
const byte kScene5030RonTradeSpeechGroup = 3;
const byte kScene5030PrimarySpeechTextColor = 0xfb;
const byte kScene5030InvalidSpeechGroup = 0xff;
const byte kScene5030DefaultSpeechFrame = 7;
const byte kScene5030VanessaIdleFrame = 0x0f;
const byte kScene5030GladysIdleFrame = 0x15;
const uint kScene5030ScoutPlayingFrameCount = 8;
const uint32 kScene5030ScoutFrameMillis = 100;
const uint32 kScene5030ScoutSpeechFrameMillis = 125;
const uint32 kScene5030RonSpeechFrameMillis = 150;
const byte kScene5030ScoutStopVanessaFrame = 0x0b;
const byte kScene5030ScoutStopGladysFrame = 0x0e;
const byte kScene5030ScoutResumeVanessaFrame = 0x0e;
const byte kScene5030ScoutResumeGladysFrame = 0x14;
const byte kScene5030DeckPatchFrame = 7;
const byte kScene5030GrantDeckHook = 1;

const byte kScene5030MineCartEntryDelayBuckets[] = {
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 3, 3, 3, 3, 4,
	4, 4, 4, 5, 5, 5, 5, 6,
	6, 6, 6, 7, 7, 7, 7, 8,
	8, 9, 9, 10, 11, 12
};

const byte kScene5030Chunk8FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25
};

const byte kScene5030Chunk9FrameMap[] = {
	0, 1, 2, 3, 4, 5, 2, 6,
	0, 7, 8, 9, 9, 8, 7, 9,
	10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 0, 0, 0
};

const byte kScene5030Chunk10FrameMap[] = {
	0, 1, 2, 3, 4, 5, 2, 1,
	0, 1, 2, 3, 4, 6, 7, 7,
	6, 4, 3, 2, 1, 8, 9, 10,
	11, 12
};

const byte kScene5030DeckRefusalFrameMap[] = {
	0, 0, 1, 2, 3, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	4, 3, 2, 1, 0
};

const byte kScene5030DeckPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene5030RonTurnToVanessaFrameMap[] = { 5, 5, 0 };
const byte kScene5030RonTurnToGladysFrameMap[] = { 5, 5, 6 };
const byte kScene5030UnderpantsPresentationFrameMap[] = { 6, 2, 1, 0, 1, 2 };

const byte kScene5030UnderpantsHandoffRonFrameMap[] = {
	2, 2, 2, 2, 2, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

const byte kScene5030UnderpantsHandoffVanessaFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 13, 0
};

class ParallelResourceLayerFrameTarget {
public:
	ParallelResourceLayerFrameTarget(ResourceSpriteLayer &first, ResourceSpriteLayer &second) :
			_first(first), _second(second) {
	}

	void setFrame(byte frame) {
		_first.setFrame(frame);
		_second.setFrame(frame);
	}

private:
	ResourceSpriteLayer &_first;
	ResourceSpriteLayer &_second;
};

PlayableSceneConfig scene5030Config() {
	PlayableSceneConfig config(5030,
		SceneResourceLayout(5, 5, 16),
		SceneViewport(kScene5030ViewportXOffset, kScene5030ViewportXOffset, kScene5030ViewportMaxXOffset),
		SceneActorPose(0x152, 0x16b, 2));
	config.setActorResources(kScene5030ActorBankTableEntry, kScene5030ActorPaletteTableEntry);
	config.setTextResources(0, kScene5030SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene5030::Scene5030(HollywoodEngine *vm) :
		PlayableScene(vm, scene5030Config()),
		_chunk8Track(RealtimeAnimationTracks::kInvalidTrack),
		_chunk9Channel(),
		_chunk10Channel(),
		_ronDialogueIdleChannel(),
		_scoutStopTransitionActive(false),
		_scoutResumeTransitionActive(false),
		_scoutTransitionCompletionPending(false),
		_scoutsInDialoguePose(false),
		_musicSuppressed(false),
		_concurrentPrimarySpeechActive(false),
		_concurrentPrimarySpeechElapsed(0),
		_concurrentPrimarySpeechDuration(0),
		_ronSpeechBaseFrame(0),
		_ronConversationChunk(13) {
	_sceneLayers.configureLayer(kScene5030MineCartEntryLayer, kSceneAnimationScenePlaced, 5,
		kScene5030MineCartEntryDescriptorCount, nullptr, 0, false);
	_sceneLayers.configureLayer(kScene5030Chunk8Layer, kSceneAnimationScenePlaced,
		8, kScene5030Chunk8DescriptorCount,
		kScene5030Chunk8FrameMap, ARRAYSIZE(kScene5030Chunk8FrameMap));
	_sceneLayers.configureLayer(kScene5030Chunk9Layer, kSceneAnimationScenePlaced,
		9, kScene5030Chunk9DescriptorCount,
		kScene5030Chunk9FrameMap, ARRAYSIZE(kScene5030Chunk9FrameMap));
	_sceneLayers.configureLayer(kScene5030Chunk10Layer, kSceneAnimationScenePlaced,
		10, kScene5030Chunk10DescriptorCount,
		kScene5030Chunk10FrameMap, ARRAYSIZE(kScene5030Chunk10FrameMap));
	_sceneLayers.configureLayer(kScene5030ActorReplacementLayer,
		kSceneAnimationActorReplacement, 0, 0, nullptr, 0, false);
	_sceneLayers.configureLayer(kScene5030AlternateVanessaLayer,
		kSceneAnimationActorReplacement, 0, 0, nullptr, 0, false);
	_chunk8Track = _realtimeAnimationTracks.addFrameMap(kScene5030Chunk8Layer, kScene5030FrameMillis);
}

void Scene5030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	setActiveActorPose(0x152, 0x16b, 2);
}

void Scene5030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_sceneLayers.layerVisible(kScene5030MineCartEntryLayer)) {
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5030MineCartEntryLayer));
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk10Layer));
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk9Layer));
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk8Layer));
		drawActionOverlayLayer();
		return;
	}
	if (_sceneLayers.layerVisible(kScene5030ActorReplacementLayer)) {
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5030ActorReplacementLayer));
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk10Layer));
		if (_sceneLayers.layerVisible(kScene5030AlternateVanessaLayer))
			drawResourceSpriteLayer(_sceneLayers.layer(kScene5030AlternateVanessaLayer));
		else
			drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk9Layer));
		drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk8Layer));
		drawActionOverlayLayer();
		return;
	}

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk9Layer));
	drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk10Layer));
	drawResourceSpriteLayer(_sceneLayers.layer(kScene5030Chunk8Layer));
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	drawActionOverlayLayer();
}

bool Scene5030::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene5030::runCustomEntrySequence() {
	setActiveActorPose(0x061, 0x19b, 2);
	drawPlayableComposite();
	fadePaletteFromBlack();
	runMineCartEntryAnimation();
	runEntryPath(0x061, 0x19b, 2, 0x152, 0x16b);
	_activeActorFacing = 2;
	_activeActorCel = 0;

	GameplayState &state = _vm->gameState();
	if (!state.scene5030EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5030EntryLineSeen = true;
	}
}

bool Scene5030::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene5030::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
	_vm->gameplayMusic()->stop();
}

void Scene5030::advanceCustomGameplayLoop(uint32 delta) {
	advanceConcurrentPrimarySpeech(delta);
	if (_scoutStopTransitionActive || _scoutResumeTransitionActive) {
		advanceScoutTransitions(delta);
		advanceRonDialogueIdle(delta);
	} else if (!_scoutsInDialoguePose) {
		advanceLayer(_chunk9Channel, kScene5030Chunk9Layer, kScene5030ScoutPlayingFrameCount, delta);
		advanceLayer(_chunk10Channel, kScene5030Chunk10Layer, kScene5030ScoutPlayingFrameCount, delta);
	} else {
		if (!_primaryDialogueSpeechActive || _primaryDialogueSpeechGroup != kScene5030VanessaSpeechGroup)
			advanceDialogueIdleLayer(_chunk9Channel, kScene5030Chunk9Layer,
				kScene5030VanessaIdleFrame, kScene5030VanessaIdleFrame + 4, delta);
		else
			_chunk9Channel.consumeFrames(delta);
		if (!_primaryDialogueSpeechActive || _primaryDialogueSpeechGroup != kScene5030GladysSpeechGroup)
			advanceDialogueIdleLayer(_chunk10Channel, kScene5030Chunk10Layer,
				kScene5030GladysIdleFrame, kScene5030GladysIdleFrame + 4, delta);
		else
			_chunk10Channel.consumeFrames(delta);
		advanceRonDialogueIdle(delta);
	}
	ensureAmbientSoundCuePlaying(1, 0x19, 75);
}

bool Scene5030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida/vagoneta (go to exit/mine cart): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Coger poste deteriorado (take rotten pole): Ron says it is useless.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar pala (look at shovel): it is in bad shape.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Coger tienda de campana (take tent): the musicians stop Ron.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar tienda de campana (look at tent).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Coger baraja de cartas (take deck of cards): grants item 0x48 after the underpants exchange.
		runDeckOfCardsAction();
		return true;
	case 307: // Mirar baraja de cartas (look at deck of cards).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Hablar con chica de la guitarra/Vanessa (talk to guitar girl/Vanessa): opens her dialogue tree.
		runVanessaConversation();
		return true;
	case 309: // Mirar chica de la guitarra/Vanessa (look at guitar girl/Vanessa).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar chica de las maracas/Gladys (look at maracas girl/Gladys).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Usar calzoncillo con Vanessa/Gladys (use underpants with Vanessa/Gladys): exchange for the deck.
		runSpecialInventorySequence();
		return true;
	case 312: // Hablar con chica de las maracas/Gladys (talk to maracas girl/Gladys): opens her dialogue tree.
		runGladysConversation();
		return true;
	default:
		return false;
	}
}

bool Scene5030::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MIN<int>(targetX, 0x214);
	if (targetY < 0x1df)
		++targetY;

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return true;
}

bool Scene5030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if (state.scene5030DeckOfCardsState >= 2 || hasInventoryItem(kScene5030DeckOfCardsItem)) {
		if (_sceneChunkTable.isValidChunk(12))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5030TakenSceneItemId);
	}
	if (state.scene5030MusiciansNamed) {
		copyStageSmallRowLabel(kScene5030RenamedSmallRowA, kScene5030DocumentSmallRowA);
		copyStageSmallRowLabel(kScene5030RenamedSmallRowB, kScene5030DocumentSmallRowB);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene5030::ambientAudioProfile() const {
	AmbientAudioProfile profile = createRandomAmbientAudioProfile(0x0d, 8, 10, 25, 0x10, 1, 100, 1);
	if (_musicSuppressed)
		profile.musicMode = kAmbientMusicNone;
	return profile;
}

byte Scene5030::ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const {
	static const byte kCueVolumes[] = { 10, 10, 10, 2, 10, 10, 10, 100 };
	if (cueId < 0x0d || cueId >= 0x0d + ARRAYSIZE(kCueVolumes))
		return defaultVolumePercent;
	return kCueVolumes[cueId - 0x0d];
}

void Scene5030::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId == kScene5030GrantDeckHook)
		grantDeckOfCards();
}

void Scene5030::resetAnimationLayers() {
	_realtimeAnimationTracks.reset(_chunk8Track);
	_chunk9Channel.reset(0, kScene5030ScoutFrameMillis);
	_chunk10Channel.reset(0, kScene5030ScoutFrameMillis);
	_ronDialogueIdleChannel.reset(0, kScene5030RonSpeechFrameMillis);
	clearSceneLayer(kScene5030ActorReplacementLayer);
	clearSceneLayer(kScene5030AlternateVanessaLayer);
	_scoutStopTransitionActive = false;
	_scoutResumeTransitionActive = false;
	_scoutTransitionCompletionPending = false;
	_scoutsInDialoguePose = false;
	_musicSuppressed = false;
	_concurrentPrimarySpeechActive = false;
	_concurrentPrimarySpeechElapsed = 0;
	_concurrentPrimarySpeechDuration = 0;
	_ronSpeechBaseFrame = 0;
	_ronConversationChunk = 13;
	_sceneLayers.setLayerVisible(kScene5030MineCartEntryLayer, false);
	_sceneLayers.setLayerVisible(kScene5030Chunk8Layer, true);
	_sceneLayers.setLayerVisible(kScene5030Chunk9Layer, true);
	_sceneLayers.setLayerFrame(kScene5030Chunk9Layer, 0);
	_sceneLayers.setLayerVisible(kScene5030Chunk10Layer, true);
	_sceneLayers.setLayerFrame(kScene5030Chunk10Layer, 0);
}

void Scene5030::advanceLayer(TimedAnimationChannel &channel, uint layerIndex, uint frameCount, uint32 delta) {
	const uint consumedFrames = channel.consumeFrames(delta);
	for (uint i = 0; i < consumedFrames; ++i) {
		byte nextFrame = (byte)(_sceneLayers.layerFrame(layerIndex) + 1);
		if (nextFrame >= frameCount)
			nextFrame = 0;
		_sceneLayers.setVisibleLayerFrame(layerIndex, nextFrame);
	}
}

void Scene5030::advanceDialogueIdleLayer(TimedAnimationChannel &channel, uint layerIndex,
		byte baseFrame, byte accentFrame, uint32 delta) {
	const uint consumedFrames = channel.consumeFrames(delta);
	for (uint i = 0; i < consumedFrames; ++i) {
		const byte frame = _sceneLayers.layerFrame(layerIndex);
		if (frame != baseFrame)
			_sceneLayers.setVisibleLayerFrame(layerIndex, baseFrame);
		else if (_random.getRandomNumber(14) == 0)
			_sceneLayers.setVisibleLayerFrame(layerIndex, accentFrame);
	}
}

void Scene5030::advanceRonDialogueIdle(uint32 delta) {
	ResourceSpriteLayer &actorReplacementLayer = _sceneLayers.layer(kScene5030ActorReplacementLayer);
	if (!actorReplacementLayer.visible ||
			(_ronConversationChunk != 13 && _ronConversationChunk != 14) ||
			actorReplacementLayer.chunkIndex != _ronConversationChunk ||
			actorReplacementLayer.frameMap != nullptr ||
			(_primaryDialogueSpeechActive && _primaryDialogueSpeechGroup == kScene5030RonSpeechGroup)) {
		_ronDialogueIdleChannel.consumeFrames(delta);
		return;
	}

	const uint consumedFrames = _ronDialogueIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < consumedFrames; ++i) {
		if (actorReplacementLayer.frameIndex != _ronSpeechBaseFrame)
			actorReplacementLayer.setFrame(_ronSpeechBaseFrame);
		else if (_random.getRandomNumber(14) == 0)
			actorReplacementLayer.setFrame(_ronSpeechBaseFrame + 4);
	}
}

void Scene5030::advanceConcurrentPrimarySpeech(uint32 delta) {
	if (!_concurrentPrimarySpeechActive)
		return;

	_concurrentPrimarySpeechElapsed += delta;
	if (!_speech.isPlaying() && _concurrentPrimarySpeechElapsed >= _concurrentPrimarySpeechDuration)
		finishConcurrentPrimarySpeech();
}

void Scene5030::advanceScoutTransitions(uint32 delta) {
	if (_scoutTransitionCompletionPending) {
		_scoutTransitionCompletionPending = false;
		if (_scoutStopTransitionActive)
			finishScoutStopTransition();
		else if (_scoutResumeTransitionActive)
			finishScoutResumeTransition();
		return;
	}

	const uint vanessaFrames = _chunk9Channel.consumeFrames(delta);
	for (uint i = 0; i < vanessaFrames; ++i) {
		byte frame = _sceneLayers.layerFrame(kScene5030Chunk9Layer);
		const byte target = _scoutStopTransitionActive ?
			kScene5030ScoutStopVanessaFrame : kScene5030ScoutResumeVanessaFrame;
		if (frame < target)
			_sceneLayers.setVisibleLayerFrame(kScene5030Chunk9Layer, frame + 1);
	}

	const uint gladysFrames = _chunk10Channel.consumeFrames(delta);
	for (uint i = 0; i < gladysFrames; ++i) {
		byte frame = _sceneLayers.layerFrame(kScene5030Chunk10Layer);
		const byte target = _scoutStopTransitionActive ?
			kScene5030ScoutStopGladysFrame : kScene5030ScoutResumeGladysFrame;
		if (frame < target)
			_sceneLayers.setVisibleLayerFrame(kScene5030Chunk10Layer, frame + 1);
	}

	if (_scoutStopTransitionActive &&
			_sceneLayers.layerFrame(kScene5030Chunk9Layer) == kScene5030ScoutStopVanessaFrame &&
			_sceneLayers.layerFrame(kScene5030Chunk10Layer) == kScene5030ScoutStopGladysFrame)
		_scoutTransitionCompletionPending = true;
	else if (_scoutResumeTransitionActive &&
			_sceneLayers.layerFrame(kScene5030Chunk9Layer) == kScene5030ScoutResumeVanessaFrame &&
			_sceneLayers.layerFrame(kScene5030Chunk10Layer) == kScene5030ScoutResumeGladysFrame)
		_scoutTransitionCompletionPending = true;
}

void Scene5030::startScoutStopTransition() {
	_scoutResumeTransitionActive = false;
	_scoutStopTransitionActive = true;
	_scoutTransitionCompletionPending = false;
	_scoutsInDialoguePose = false;
	_chunk9Channel.reset(0, kScene5030ScoutFrameMillis);
	_chunk10Channel.reset(0, kScene5030ScoutFrameMillis);
}

void Scene5030::startScoutResumeTransition() {
	_scoutStopTransitionActive = false;
	_scoutResumeTransitionActive = true;
	_scoutTransitionCompletionPending = false;
	_scoutsInDialoguePose = true;
	_sceneLayers.setVisibleLayerFrame(kScene5030Chunk9Layer, kScene5030ScoutStopVanessaFrame);
	_sceneLayers.setVisibleLayerFrame(kScene5030Chunk10Layer, kScene5030ScoutStopGladysFrame);
	_chunk9Channel.reset(0, kScene5030ScoutFrameMillis);
	_chunk10Channel.reset(0, kScene5030ScoutFrameMillis);
}

void Scene5030::waitForScoutTransition() {
	while ((_scoutStopTransitionActive || _scoutResumeTransitionActive) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			break;
	}
}

void Scene5030::finishScoutStopTransition() {
	_scoutStopTransitionActive = false;
	_scoutTransitionCompletionPending = false;
	_scoutsInDialoguePose = true;
	_sceneLayers.setVisibleLayerFrame(kScene5030Chunk9Layer, kScene5030VanessaIdleFrame);
	_sceneLayers.setVisibleLayerFrame(kScene5030Chunk10Layer, kScene5030GladysIdleFrame);
	_chunk9Channel.reset(0, kScene5030ScoutSpeechFrameMillis);
	_chunk10Channel.reset(0, kScene5030ScoutSpeechFrameMillis);
	_ronDialogueIdleChannel.reset(0, kScene5030RonSpeechFrameMillis);
}

void Scene5030::finishScoutResumeTransition() {
	_scoutResumeTransitionActive = false;
	_scoutTransitionCompletionPending = false;
	_scoutsInDialoguePose = false;
	_sceneLayers.setVisibleLayerFrame(kScene5030Chunk9Layer, 0);
	_sceneLayers.setVisibleLayerFrame(kScene5030Chunk10Layer, 0);
	_chunk9Channel.reset(0, kScene5030ScoutFrameMillis);
	_chunk10Channel.reset(0, kScene5030ScoutFrameMillis);
	_ronDialogueIdleChannel.reset(0, kScene5030RonSpeechFrameMillis);
	_musicSuppressed = false;
}

void Scene5030::showRonConversationLayer(uint chunkIndex, byte baseFrame) {
	clearSceneLayer(kScene5030AlternateVanessaLayer);
	_sceneLayers.setLayerResource(kScene5030ActorReplacementLayer,
		chunkIndex, kScene5030ConversationDescriptorCount, nullptr, 0);
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		_sceneLayers.showLayerAtFrame(kScene5030ActorReplacementLayer, baseFrame);
	else
		_sceneLayers.setLayerVisible(kScene5030ActorReplacementLayer, false);
	_ronSpeechBaseFrame = baseFrame;
	_ronConversationChunk = chunkIndex;
	_ronDialogueIdleChannel.reset(0, kScene5030RonSpeechFrameMillis);
}

void Scene5030::clearActorReplacementLayers() {
	clearSceneLayer(kScene5030ActorReplacementLayer);
	clearSceneLayer(kScene5030AlternateVanessaLayer);
}

void Scene5030::finishScoutConversation() {
	clearActorReplacementLayers();
	startScoutResumeTransition();
	waitForScoutTransition();
}

void Scene5030::runRonPoseTransition(bool faceGladys) {
	const byte *frameMap = faceGladys ? kScene5030RonTurnToGladysFrameMap : kScene5030RonTurnToVanessaFrameMap;
	const uint frameCount = faceGladys ? ARRAYSIZE(kScene5030RonTurnToGladysFrameMap) :
		ARRAYSIZE(kScene5030RonTurnToVanessaFrameMap);
	BlockingSequence(*this).resourceLayerFrames(kScene5030ActorReplacementLayer,
		_ronConversationChunk, kScene5030ConversationDescriptorCount,
		frameMap, frameCount, AnimationFrameRange(0, frameCount - 1,
			kScene5030FrameMillis).unskippable().noFinalFrameDelay(), false);
	_sceneLayers.setLayerResource(kScene5030ActorReplacementLayer,
		_ronConversationChunk, kScene5030ConversationDescriptorCount, nullptr, 0);
	if (_sceneChunkTable.isValidChunk(_ronConversationChunk))
		_sceneLayers.showLayerAtFrame(kScene5030ActorReplacementLayer, faceGladys ? 6 : 0);
	else
		_sceneLayers.setLayerVisible(kScene5030ActorReplacementLayer, false);
	_ronSpeechBaseFrame = faceGladys ? 6 : 0;
	_ronDialogueIdleChannel.reset(0, kScene5030RonSpeechFrameMillis);
}

void Scene5030::runDeckRefusalSequence() {
	startScoutStopTransition();
	BlockingSequence(*this).resourceLayerFrames(kScene5030ActorReplacementLayer,
		11, kScene5030DeckAnimationDescriptorCount,
		kScene5030DeckRefusalFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene5030DeckRefusalFrameMap) - 1,
			kScene5030FrameMillis).unskippable().noFinalFrameDelay());
	waitForScoutTransition();

	const bool vanessaSpeechStarted = startConcurrentPrimarySpeechLine(5, 0,
		kScene5030VanessaDialogueCenterX, kScene5030VanessaDialogueTopY,
		0, 0x20, 0x3f, kScene5030VanessaSpeechGroup);
	walkActiveActorTo(0x214, 0x162, 4, 0, false);
	showRonConversationLayer(13, 0);
	if (vanessaSpeechStarted)
		waitForConcurrentPrimarySpeech();
	else
		beginVanessaSpeechLine(5, 0);
	runScoutSpeechLineDuringRonTurn(true, 5, 1);
	runRonPoseTransition(false);
	finishScoutConversation();
}

void Scene5030::runDeckPickupSequence() {
	_sceneLayers.setLayerResource(kScene5030ActorReplacementLayer,
		11, kScene5030DeckAnimationDescriptorCount,
		kScene5030DeckPickupFrameMap, ARRAYSIZE(kScene5030DeckPickupFrameMap));
	_sceneLayers.setLayerVisible(kScene5030ActorReplacementLayer, _sceneChunkTable.isValidChunk(11));

	BlockingSequence sequence(*this);
	sequence.presentedLayerFrames(kScene5030ActorReplacementLayer,
		AnimationFrameRange(0, kScene5030DeckPatchFrame - 1,
			kScene5030FrameMillis).unskippable());
	if (_sceneChunkTable.isValidChunk(12))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
	clearSceneItemFromColorMap(kScene5030TakenSceneItemId);
	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	sequence.presentedLayerFrames(kScene5030ActorReplacementLayer,
		AnimationFrameRange(kScene5030DeckPatchFrame, ARRAYSIZE(kScene5030DeckPickupFrameMap) - 1,
			kScene5030FrameMillis).unskippable().noFinalFrameDelay().hookAt(14, kScene5030GrantDeckHook));
	clearSceneLayer(kScene5030ActorReplacementLayer);

	sequence.actorPath(SceneActorPose(0x214, 0x162, 4));
}

void Scene5030::runUnderpantsPresentationAnimation() {
	BlockingSequence(*this).resourceLayerFrames(kScene5030ActorReplacementLayer,
		15, kScene5030UnderpantsRonDescriptorCount,
		kScene5030UnderpantsPresentationFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene5030UnderpantsPresentationFrameMap) - 1,
			kScene5030FrameMillis).unskippable().noFinalFrameDelay(), false);
	_sceneLayers.setLayerResource(kScene5030ActorReplacementLayer,
		15, kScene5030UnderpantsRonDescriptorCount, nullptr, 0);
	if (_sceneChunkTable.isValidChunk(15))
		_sceneLayers.showLayerAtFrame(kScene5030ActorReplacementLayer, 2);
	else
		_sceneLayers.setLayerVisible(kScene5030ActorReplacementLayer, false);
}

void Scene5030::runUnderpantsHandoffAnimation() {
	_sceneLayers.setLayerResource(kScene5030ActorReplacementLayer,
		15, kScene5030UnderpantsRonDescriptorCount,
		kScene5030UnderpantsHandoffRonFrameMap, ARRAYSIZE(kScene5030UnderpantsHandoffRonFrameMap));
	_sceneLayers.setLayerVisible(kScene5030ActorReplacementLayer, _sceneChunkTable.isValidChunk(15));
	_sceneLayers.setLayerResource(kScene5030AlternateVanessaLayer,
		16, kScene5030UnderpantsVanessaDescriptorCount,
		kScene5030UnderpantsHandoffVanessaFrameMap, ARRAYSIZE(kScene5030UnderpantsHandoffVanessaFrameMap));
	_sceneLayers.setLayerVisible(kScene5030AlternateVanessaLayer, _sceneChunkTable.isValidChunk(16));

	ParallelResourceLayerFrameTarget target(
		_sceneLayers.layer(kScene5030ActorReplacementLayer),
		_sceneLayers.layer(kScene5030AlternateVanessaLayer));
	BlockingSequence(*this).presentedLayerFrames(target,
		AnimationFrameRange(0, ARRAYSIZE(kScene5030UnderpantsHandoffRonFrameMap) - 1,
			kScene5030FrameMillis).unskippable().noFinalFrameDelay());
	clearActorReplacementLayers();
}

void Scene5030::beginConversationMusicSuppression() {
	_musicSuppressed = true;
	_vm->gameplayMusic()->stop();
}

void Scene5030::runMineCartEntryAnimation() {
	if (!_sceneChunkTable.isValidChunk(5))
		return;

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_sceneLayers.setLayerVisible(kScene5030MineCartEntryLayer, true);
	_sceneLayers.setLayerFrame(kScene5030MineCartEntryLayer, 0);
	drawPlayableComposite();
	presentFrame();

	for (uint frame = 0; frame < ARRAYSIZE(kScene5030MineCartEntryDelayBuckets) && !Engine::shouldQuit(); ++frame) {
		_sceneLayers.setVisibleLayerFrame(kScene5030MineCartEntryLayer, (byte)frame);
		if (frame == kScene5030MineCartSoundFrame)
			_soundBank0.playSample(0x16, 100);

		if (frame + 1 == ARRAYSIZE(kScene5030MineCartEntryDelayBuckets)) {
			drawPlayableComposite();
			presentFrame();
			break;
		}

		const byte delayBucket = kScene5030MineCartEntryDelayBuckets[frame];
		const uint32 frameMillis = 200 / MAX<uint32>(1, 13 - delayBucket);
		if (waitSceneMillis(frameMillis, false))
			break;
	}

	_sceneLayers.setLayerVisible(kScene5030MineCartEntryLayer, false);
	_hideActiveActor = previousHideActiveActor;
}

byte Scene5030::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene5030VanessaSpeechGroup:
		return kScene5030VanessaIdleFrame;
	case kScene5030GladysSpeechGroup:
		return kScene5030GladysIdleFrame;
	case kScene5030RonSpeechGroup:
	case kScene5030RonTradeSpeechGroup:
		return _ronSpeechBaseFrame;
	default:
		return 0;
	}
}

byte Scene5030::primarySpeechAnimationFrameCount(byte animationGroup) const {
	return animationGroup == kScene5030RonTradeSpeechGroup ? 4 : 5;
}

uint32 Scene5030::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	if (animationGroup == kScene5030VanessaSpeechGroup || animationGroup == kScene5030GladysSpeechGroup)
		return kScene5030ScoutSpeechFrameMillis;
	return kScene5030RonSpeechFrameMillis;
}

void Scene5030::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	switch (animationGroup) {
	case kScene5030VanessaSpeechGroup:
		_sceneLayers.setVisibleLayerFrame(kScene5030Chunk9Layer, frameIndex);
		break;
	case kScene5030GladysSpeechGroup:
		_sceneLayers.setVisibleLayerFrame(kScene5030Chunk10Layer, frameIndex);
		break;
	case kScene5030RonSpeechGroup:
	case kScene5030RonTradeSpeechGroup:
		_sceneLayers.setVisibleLayerFrame(kScene5030ActorReplacementLayer, frameIndex);
		break;
	default:
		break;
	}
}

void Scene5030::runExitToMineSwitches() {
	walkActiveActorTo(0x061, 0x19b, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5030::runDeckOfCardsAction(bool fromUnderpantsExchange) {
	GameplayState &state = _vm->gameState();
	if (state.scene5030DeckOfCardsState >= 2 || hasInventoryItem(kScene5030DeckOfCardsItem)) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	if (state.scene5030DeckOfCardsState == 0) {
		runDeckRefusalSequence();
		return;
	}

	runDeckPickupSequence();
	showRonConversationLayer(13, 0);
	if (fromUnderpantsExchange) {
		waitForConcurrentPrimarySpeech();
		runScoutSpeechLineDuringRonTurn(true, 9, 11);
	}
	runRonPoseTransition(false);
	finishScoutConversation();
}

void Scene5030::runVanessaConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeVanessaDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	const bool firstConversation = !state.scene5030MusiciansNamed;
	beginConversationMusicSuppression();
	showRonConversationLayer(13, 0);
	startScoutStopTransition();
	beginRonDialogueLine(kScene5030VanessaDialogueStageId, firstConversation ? 0 : 1);
	waitForScoutTransition();
	beginVanessaSpeechLine(kScene5030VanessaPrimaryRow, firstConversation ? 0 : 1);
	if (firstConversation) {
		state.scene5030MusiciansNamed = true;
		applySceneStateToHotspotsAndPatches(3);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5030VanessaDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginRonDialogueLine(kScene5030VanessaDialogueStageId, 6);
			beginVanessaSpeechLine(kScene5030VanessaPrimaryRow, 6);
			runScoutSpeechLineDuringRonTurn(true, kScene5030GladysReplyToVanessaRow, 4);
			runRonPoseTransition(false);
			finishScoutConversation();
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			finishScoutConversation();
			return;
		}

		DialogueChoiceRecord &record = records[recordIndex];
		beginRonDialogueLine(kScene5030VanessaDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5030DialogueNoResponseFrame) {
			beginVanessaSpeechLine(kScene5030VanessaPrimaryRow, record.responseFrameIndex);
			runScoutSpeechLineDuringRonTurn(true, kScene5030GladysReplyToVanessaRow, record.reserved);
		}
		runRonPoseTransition(false);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (applyDialogueTransition(record, depthIndex, nodeIndex)) {
			finishScoutConversation();
			return;
		}
	}

	finishScoutConversation();
}

void Scene5030::runGladysConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeGladysDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	const bool firstConversation = !state.scene5030MusiciansNamed;
	beginConversationMusicSuppression();
	showRonConversationLayer(14, 6);
	startScoutStopTransition();
	beginRonDialogueLine(kScene5030GladysDialogueStageId, firstConversation ? 0 : 1);
	waitForScoutTransition();
	beginGladysSpeechLine(kScene5030GladysPrimaryRow, firstConversation ? 0 : 1);
	if (firstConversation) {
		state.scene5030MusiciansNamed = true;
		applySceneStateToHotspotsAndPatches(3);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5030GladysDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginRonDialogueLine(kScene5030GladysDialogueStageId, 6);
			beginGladysSpeechLine(kScene5030GladysPrimaryRow, 6);
			runScoutSpeechLineDuringRonTurn(false, kScene5030VanessaReplyToGladysRow, 4);
			runRonPoseTransition(true);
			finishScoutConversation();
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			finishScoutConversation();
			return;
		}

		DialogueChoiceRecord &record = records[recordIndex];
		beginRonDialogueLine(kScene5030GladysDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5030DialogueNoResponseFrame) {
			beginGladysSpeechLine(kScene5030GladysPrimaryRow, record.responseFrameIndex);
			runScoutSpeechLineDuringRonTurn(false, kScene5030VanessaReplyToGladysRow, record.reserved);
		}
		runRonPoseTransition(true);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (applyDialogueTransition(record, depthIndex, nodeIndex)) {
			finishScoutConversation();
			return;
		}
	}

	finishScoutConversation();
}

void Scene5030::runSpecialInventorySequence() {
	GameplayState &state = _vm->gameState();
	if (!state.scene5030MusiciansNamed) {
		beginStaticSecondarySpeechLine(0xda, 0);
		return;
	}

	beginConversationMusicSuppression();
	showRonConversationLayer(13, 0);
	startScoutStopTransition();
	beginRonDialogueLine(9, 0);
	waitForScoutTransition();
	beginVanessaSpeechLine(9, 1);
	runRonPoseTransition(true);
	beginGladysSpeechLine(9, 2);
	runRonPoseTransition(false);
	clearActorReplacementLayers();

	const Common::Array<byte> savedFullPaletteRegionMask = _fullPaletteRegionMask;
	const Common::Array<byte> savedWalkablePaletteMask = _walkablePaletteMask;
	for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i)
		_fullPaletteRegionMask[i] = 1;
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i)
		_walkablePaletteMask[i] = 1;
	walkActiveActorTo(0x1fa, 0x178, 5, 0, false);

	runUnderpantsPresentationAnimation();
	_ronSpeechBaseFrame = 2;
	beginPrimarySpeechLineWithAnimationGroup(9, 3, kScene5030RonTradeCenterX,
		kScene5030RonTradeTopY, 0x3f, 0x3f, 0x3f, kScene5030RonTradeSpeechGroup);
	beginVanessaSpeechLine(9, 4);
	beginGladysSpeechLine(9, 5);
	beginPrimarySpeechLineWithAnimationGroup(9, 6, kScene5030RonTradeCenterX,
		kScene5030RonTradeTopY, 0x3f, 0x3f, 0x3f, kScene5030RonTradeSpeechGroup);
	beginVanessaSpeechLine(9, 7);
	beginGladysSpeechLine(9, 8);
	beginPrimarySpeechLineWithAnimationGroup(9, 9, kScene5030RonTradeCenterX,
		kScene5030RonTradeTopY, 0x3f, 0x3f, 0x3f, kScene5030RonTradeSpeechGroup);
	runUnderpantsHandoffAnimation();

	if (hasInventoryItem(kScene5030UnderpantsItem))
		removeInventoryItem(kScene5030UnderpantsItem);
	_soundBank0.playSample(1, 100);
	_sceneLayers.setVisibleLayerFrame(kScene5030Chunk9Layer, kScene5030VanessaIdleFrame);
	const bool vanessaSpeechStarted = startConcurrentPrimarySpeechLine(9, 10,
		kScene5030VanessaDialogueCenterX, kScene5030VanessaDialogueTopY,
		0, 0x20, 0x3f, kScene5030VanessaSpeechGroup);
	walkActiveActorTo(0x20a, 0x15e, 1, 0, false);
	if (!vanessaSpeechStarted)
		beginVanessaSpeechLine(9, 10);
	_fullPaletteRegionMask = savedFullPaletteRegionMask;
	_walkablePaletteMask = savedWalkablePaletteMask;

	state.scene5030DeckOfCardsState = 1;
	runDeckOfCardsAction(true);
	startScoutResumeTransition();
	waitForScoutTransition();
}

void Scene5030::grantDeckOfCards() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene5030DeckOfCardsItem))
		addInventoryItem(kScene5030DeckOfCardsItem);
	_soundBank0.playSample(1, 100);
	state.scene5030DeckOfCardsState = 2;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5030::initializeVanessaDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5030DialogueChoiceRecordCount);

	// DAT_00507e68: Vanessa root choices and the nested werewolf branch.
	setDialogueRecord(records, 0, 0, kScene5030DialogueTransitionDown, 2, 2, 1, 0);
	setDialogueRecord(records, 1, 0, kScene5030DialogueTransitionStay, 3, 3, 1, 1);
	setDialogueRecord(records, 2, 0, kScene5030DialogueTransitionStay, 4, 4, 1, 2);
	setDialogueRecord(records, 3, 0, kScene5030DialogueTransitionStay, 5, 5, 1, 3);
	setDialogueRecord(records, 4, 0, kScene5030DialogueTransitionEnd, 6, 6, 0, 4);
	setDialogueRecord(records, 70, 0, kScene5030DialogueTransitionStay, 7, 7, 1, 5);
	setDialogueRecord(records, 71, 0, kScene5030DialogueTransitionStay, 8, 8, 1, 6);
	setDialogueRecord(records, 72, 0, kScene5030DialogueTransitionStay, 9, 9, 1, 7);
	setDialogueRecord(records, 73, 0, kScene5030DialogueTransitionUp, 10, 10, 0, 8);
}

void Scene5030::initializeGladysDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5030DialogueChoiceRecordCount);

	// DAT_00506b40: Gladys root choices and the nested werewolf branch.
	setDialogueRecord(records, 0, 0, kScene5030DialogueTransitionDown, 2, 2, 1, 0);
	setDialogueRecord(records, 1, 0, kScene5030DialogueTransitionStay, 3, 3, 1, 1);
	setDialogueRecord(records, 2, 0, kScene5030DialogueTransitionStay, 4, 4, 1, 2);
	setDialogueRecord(records, 3, 0, kScene5030DialogueTransitionStay, 5, 5, 1, 3);
	setDialogueRecord(records, 4, 0, kScene5030DialogueTransitionEnd, 6, 6, 0, 4);
	setDialogueRecord(records, 70, 0, kScene5030DialogueTransitionStay, 7, 7, 1, 5);
	setDialogueRecord(records, 71, 0, kScene5030DialogueTransitionStay, 8, 8, 1, 6);
	setDialogueRecord(records, 72, 0, kScene5030DialogueTransitionUp, 9, 9, 0, 7);
}

void Scene5030::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte otherScoutFrameIndex) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = 1;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = otherScoutFrameIndex;
	record.selectable = 1;
}

bool Scene5030::applyDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const {
	const byte previousDepth = depthIndex;
	switch (record.transitionMode) {
	case kScene5030DialogueTransitionEnd:
		return true;
	case kScene5030DialogueTransitionDown:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth + 1;
		break;
	case kScene5030DialogueTransitionUp:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth == 0 ? 0 : (byte)(previousDepth - 1);
		break;
	case kScene5030DialogueTransitionUpTwo:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
		break;
	case kScene5030DialogueTransitionStay:
	default:
		break;
	}

	return false;
}

void Scene5030::beginRonDialogueLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, kScene5030RonDialogueCenterX,
		kScene5030RonDialogueTopY, 0x3f, 0x3f, 0x3f, kScene5030RonSpeechGroup);
}

void Scene5030::beginVanessaSpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, kScene5030VanessaDialogueCenterX,
		kScene5030VanessaDialogueTopY, 0, 0x20, 0x3f, kScene5030VanessaSpeechGroup);
}

void Scene5030::beginGladysSpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, kScene5030GladysDialogueCenterX,
		kScene5030GladysDialogueTopY, 0x3f, 0x20, 0, kScene5030GladysSpeechGroup);
}

bool Scene5030::startConcurrentPrimarySpeechCue(uint16 textRecordId, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte red, byte green, byte blue, byte animationGroup) {
	if (_concurrentPrimarySpeechActive)
		return false;

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return false;

	setPaletteEntry6Bit(kScene5030PrimarySpeechTextColor, red, green, blue);
	_primarySpeechOverlay.visible = true;
	_primarySpeechOverlay.colorIndex = kScene5030PrimarySpeechTextColor;
	wrapActorSpeechText(text, centerX, _primarySpeechOverlay.lines);
	calculateSpeechOverlayBounds(_primarySpeechOverlay, centerX, topY, true, _activeActorWorldY);

	const bool started = voiceSampleId != 0 &&
		_speech.playSample(voiceSampleId, primarySpeechVolumePercent(animationGroup));
	_concurrentPrimarySpeechDuration = started ?
		MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(1200, _primarySpeechOverlay.lines.size() * 1100);
	_concurrentPrimarySpeechElapsed = 0;
	_concurrentPrimarySpeechActive = true;

	const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
	_speechController.startPrimaryDialogueSpeech(animationGroup, baseFrame);
	primarySpeechAnimationStarted(animationGroup, baseFrame);
	setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
	return true;
}

bool Scene5030::startConcurrentPrimarySpeechLine(uint16 rowIndex, byte frameIndex,
		uint16 centerX, uint16 topY, byte red, byte green, byte blue, byte animationGroup) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId) ||
			MAX<byte>(1, continuationCount) != 1)
		return false;

	return startConcurrentPrimarySpeechCue(textRecordId, voiceSampleId,
		centerX, topY, red, green, blue, animationGroup);
}

bool Scene5030::waitForConcurrentPrimarySpeech() {
	bool interrupted = false;
	while (_concurrentPrimarySpeechActive && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10)) {
			interrupted = true;
			break;
		}
	}
	if (_concurrentPrimarySpeechActive)
		finishConcurrentPrimarySpeech();
	return interrupted;
}

void Scene5030::finishConcurrentPrimarySpeech() {
	if (!_concurrentPrimarySpeechActive)
		return;

	const byte animationGroup = _primaryDialogueSpeechGroup;
	const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
	setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
	_speechController.stopPrimaryDialogueSpeech(kScene5030InvalidSpeechGroup, kScene5030DefaultSpeechFrame);
	_speech.stop();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
	_concurrentPrimarySpeechActive = false;
	_concurrentPrimarySpeechElapsed = 0;
	_concurrentPrimarySpeechDuration = 0;
	primarySpeechAnimationRestored(animationGroup, baseFrame);
}

void Scene5030::runScoutSpeechLineDuringRonTurn(bool gladys, uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId)) {
		runRonPoseTransition(gladys);
		return;
	}

	const uint16 centerX = gladys ? kScene5030GladysDialogueCenterX : kScene5030VanessaDialogueCenterX;
	const uint16 topY = gladys ? kScene5030GladysDialogueTopY : kScene5030VanessaDialogueTopY;
	const byte red = gladys ? 0x3f : 0;
	const byte green = 0x20;
	const byte blue = gladys ? 0 : 0x3f;
	const byte animationGroup = gladys ? kScene5030GladysSpeechGroup : kScene5030VanessaSpeechGroup;
	if (!startConcurrentPrimarySpeechCue(textRecordId, voiceSampleId,
			centerX, topY, red, green, blue, animationGroup)) {
		runRonPoseTransition(gladys);
		if (gladys)
			beginGladysSpeechLine(rowIndex, frameIndex);
		else
			beginVanessaSpeechLine(rowIndex, frameIndex);
		return;
	}

	runRonPoseTransition(gladys);
	const bool interrupted = waitForConcurrentPrimarySpeech();
	const byte lineCount = MAX<byte>(1, continuationCount);
	if (!interrupted && lineCount > 1) {
		runSpeechCue(_primarySpeechOverlay, textRecordId + 1, lineCount - 1,
			voiceSampleId == 0 ? 0 : voiceSampleId + 1, centerX, topY,
			kScene5030PrimarySpeechTextColor, true, false, true, animationGroup);
	}
}

void Scene5030::copyStageSmallRowLabel(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	byte *destination = _stage003SmallRows.data() + destinationOffset;
	const byte *source = _stage003SmallRows.data() + sourceOffset;
	uint length = 0;
	while (length + 1 < kStage003SmallRowSize && source[length] != 0)
		++length;
	memcpy(destination, source, length);
	destination[length] = 0;
}

void Scene5030::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
