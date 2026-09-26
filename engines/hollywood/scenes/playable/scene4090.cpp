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
#include "hollywood/game_strings.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/scene4090.h"
#include "hollywood/scenes/shared_frame_sequences.h"

namespace Hollywood {

const uint16 kScene4090FirstState = 0x0ffa;
const uint16 kScene4090DoorExitState = 0x1007;
const uint16 kScene4090FinalReturnState = 0x0fd3;
const int kScene4090DefaultActorX = 0x013b;
const int kScene4090DefaultActorY = 0x0164;
const byte kScene4090DefaultActorFacing = 2;
const int kScene4090ReturnEntryStartX = 0x0064;
const int kScene4090ReturnEntryStartY = 0x01d6;
const int kScene4090ReturnEntryTargetX = 0x0190;
const int kScene4090ReturnEntryTargetY = 0x01c2;
const uint16 kScene4090ViewportInitialX = 0x0090;
const uint16 kScene4090ViewportMinX = 0x0090;
const uint16 kScene4090ViewportMaxX = 0x00f0;
const uint kScene4090ActorBankTableEntry = 0x0000;
const uint kScene4090ActorPaletteTableEntry = 0x00cc;
const uint kScene4090Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4090SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4090FrameMillis = 75;
const uint32 kScene4090FastFrameMillis = 50;
const uint32 kScene4090OrganBodyFrameMillis = 10;
const uint32 kScene4090AmbientCheckMillis = 250;
const uint32 kScene4090CoffinPaletteCycleMillis = 60;
const uint kScene4090ForegroundLowChunk = 5;
const uint kScene4090ForegroundHighChunk = 6;
const uint kScene4090DoorExitChunk = 7;
const uint kScene4090DoorExitDescriptorCount = 6;
const uint kScene4090OrganBodyChunk = 8;
const uint kScene4090OrganBodyDescriptorCount = 0x0b;
const uint kScene4090AmbientRandomLayer = 0;
const uint kScene4090AmbientFixedLayer = 1;
const uint kScene4090OrganBodyLayer = 2;
const uint kScene4090ScriptLayer = 3;
const uint kScene4090OrganOverlayChunk = 9;
const uint kScene4090OrganOverlayDescriptorCount = 4;
const uint kScene4090AlternatePatchChunk = 10;
const uint kScene4090FinalOverlayChunk = 11;
const uint kScene4090FinalOverlayDescriptorCount = 0x11;
const uint kScene4090AmbientRandomChunk = 12;
const uint kScene4090AmbientRandomDescriptorCount = 0x0b;
const uint kScene4090AmbientFixedChunk = 13;
const uint kScene4090AmbientFixedDescriptorCount = 5;
const uint kScene4090CoffinBookendChunk = 14;
const uint kScene4090CoffinBookendDescriptorCount = 4;
const uint kScene4090FinalVariantBaseChunk = 15;
const uint kScene4090FinalVariantAlternateBaseChunk = 17;
const uint kScene4090FinalCloseDescriptorCount = 0x0f;
const uint kScene4090FinalOpenDescriptorCount = 0x23;
const byte kScene4090FinalPrimarySpeechNormalGroup = 0;
const byte kScene4090FinalPrimarySpeechAlternateGroup = 1;
const uint kScene4090FinalPrimarySpeechFramesPerGroup = 5;
const uint32 kScene4090FinalRoomOverlayHoldFrames = 100;
const uint kScene4090CoffinFirstClipChunk = 20;
const uint kScene4090CoffinSecondClipChunk = 19;
const uint kScene4090CoffinClipDescriptorCount = 0x17;
const uint kScene4090CoffinFramebufferChunk = 21;
const uint kScene4090CoffinPaletteChunk = 22;
const uint kScene4090CoffinPaletteBytes = 0x210;
const uint16 kScene4090CoffinViewportX = 0x0070;
const byte kScene4090CoffinPaletteCycleFirstColor = 0xa8;
const byte kScene4090CoffinPaletteCycleLastColor = 0xaf;
const int kScene4090CurtainStartOffset = 0x00dc;
const byte kScene4090CurtainBandWidth = 0x14;
const uint kScene4090CurtainEndOffset = 0x00f0;
const int kScene4090ForegroundActorThresholdY = 0x0172;

const byte kScene4090OrganOverlayHook = 1;

const byte kScene4090OrganOverlayFrameMap[] = {
	0, 1, 2, 3, 3, 3, 3, 2, 1, 0
};

const byte kScene4090OrganBodyFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 5,
	4, 3, 2, 1, 0
};

const byte kScene4090FinalRoomChunk13FrameMap[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 1, 2, 3, 3
};

const byte kScene4090FinalFadeFrameMap[] = {
	1, 2, 3, 2, 1, 0, 1, 2, 3, 2, 1, 0, 1, 2, 3, 2,
	1, 2, 3, 4, 5, 4, 3, 4, 5, 4, 3, 4, 5
};

const byte kScene4090FinalPrimarySpeechFrameMap[] = {
	5, 9, 10, 11, 6,
	8, 12, 13, 14, 7
};

PlayableSceneConfig scene4090Config() {
	PlayableSceneConfig config(4090,
		SceneResourceLayout(5, 5, 20),
		SceneViewport(kScene4090ViewportInitialX, kScene4090ViewportMinX, kScene4090ViewportMaxX),
		SceneActorPose(kScene4090DefaultActorX, kScene4090DefaultActorY, kScene4090DefaultActorFacing));
	config.setActorResources(kScene4090ActorBankTableEntry, kScene4090ActorPaletteTableEntry);
	config.setTextResources(kScene4090Resource003RowsOffsetIndex, kScene4090SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene4090::Scene4090(HollywoodEngine *vm) :
		PlayableScene(vm, scene4090Config()),
		_randomAmbientTrack(RealtimeAnimationTracks::kInvalidTrack),
		_organBodyChannel(),
		_originalColorToItemMap(),
		_ambientSoundTimerAccumulator(0),
		_coffinPaletteCycleAccumulator(0),
		_previousAmbientSoundCue(0),
		_organBodyTargetFrame(0),
		_organBodyAnimationActive(false),
		_organBodyWaitForSound(false),
		_randomAmbientAnimationActive(false),
		_multiSpriteCompositeActive(false) {
	_sceneLayers.configureLayer(kScene4090AmbientRandomLayer, kSceneAnimationBehindActors,
		kScene4090AmbientRandomChunk,
		kScene4090AmbientRandomDescriptorCount, nullptr, 0, false);
	_sceneLayers.configureLayer(kScene4090AmbientFixedLayer, kSceneAnimationBehindActors,
		kScene4090AmbientFixedChunk,
		kScene4090AmbientFixedDescriptorCount, nullptr, 0, false);
	_sceneLayers.configureLayer(kScene4090OrganBodyLayer, kSceneAnimationBehindActors,
		kScene4090OrganBodyChunk,
		kScene4090OrganBodyDescriptorCount,
		kScene4090OrganBodyFrameMap, ARRAYSIZE(kScene4090OrganBodyFrameMap), false);
	_sceneLayers.configureLayer(kScene4090ScriptLayer, kSceneAnimationActorReplacement,
		0, 0, nullptr, 0, false);
	_randomAmbientTrack = _realtimeAnimationTracks.addRandom(kScene4090AmbientRandomLayer, 150, 0, 9, false, false);
}

void Scene4090::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	rememberOriginalColorMap();

	setActiveActorPose(kScene4090DefaultActorX, kScene4090DefaultActorY, kScene4090DefaultActorFacing);
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4090::drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldX;
	(void)actorDrawOrderMode;

	if (_sceneLayers.layerVisible(kScene4090ScriptLayer))
		return;

	drawForegroundLayers(activeWorldY, !_multiSpriteCompositeActive);
}

void Scene4090::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	if (state.mainFlowStateId == kScene4090FirstState) {
		_soundBank0.playSample(5, 100);
		const bool playInitialGreeting = !state.scene4090InitialGreetingSeen;
		setActiveActorPose(kScene4090DefaultActorX, kScene4090DefaultActorY,
			playInitialGreeting ? 1 : kScene4090DefaultActorFacing);
		drawPlayableComposite();
		if (fadePaletteFromBlack())
			return;
		if (playInitialGreeting) {
			beginSecondarySpeechLine(0, 0);
			state.scene4090InitialGreetingSeen = true;
		}
		return;
	}

	setActiveActorPose(kScene4090ReturnEntryStartX, kScene4090ReturnEntryStartY, 1);
	drawPlayableComposite();
	Graphics::ManagedSurface entryFrame;
	entryFrame.copyFrom(_sceneFramebuffer);
	const Common::Array<byte> entryPalette = _paletteCurrent;
	if (!runCurtainReveal(entryFrame, entryPalette) ||
			!walkActiveActorTo(kScene4090ReturnEntryTargetX, kScene4090ReturnEntryTargetY,
				2, 0, false))
		return;
	if (state.scene4090WideCoffinVariant == 0) {
		beginSecondarySpeechLine(8, 7);
	} else {
		beginSecondarySpeechLine(8, 8);
		state.scene4090FinalCutsceneCompleted = 1;
		if (state.scene4070DraculaStage == 0)
			state.scene4070DraculaStage = 1;
	}
}

void Scene4090::runExitSideEffectsAfterLoop() {
	if (!didLeaveSceneAfterLoop())
		return;

	if (_vm->gameState().mainFlowStateId != kScene4090FinalReturnState)
		fadePaletteToBlack();
	stopAmbientSoundCues();
}

void Scene4090::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4090::advanceCustomGameplayLoop(uint32 delta) {
	advanceAmbientSound(delta);
	advanceOrganBodyAnimation(delta);
	_realtimeAnimationTracks.setActive(_randomAmbientTrack,
		_randomAmbientAnimationActive &&
		_sceneLayers.layer(kScene4090AmbientRandomLayer).visible);
}

void Scene4090::advanceFullscreenAnimation(uint32 delta) {
	const bool paletteChanged = advanceCoffinPaletteCycle(delta);
	advanceAmbientSound(delta);
	PlayableScene::advanceFullscreenAnimation(delta);
	if (paletteChanged)
		presentFrame();
}

bool Scene4090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta (look at door): corridor exit description.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Ir/usar/abrir puerta (go/use/open door): exit toward the following D10/B09 flow.
		runDoorExit();
		return true;
	case 303: // Mirar organo (look at organ): sinister music taste.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar organo (use organ): play Bach and trigger the room reveal sequence.
		runOrganRevealSequence();
		return true;
	case 305: // Coger ataud (take coffin): swap the paired coffins after Gwendolyn leaves.
		runCoffinSwapSequence();
		return true;
	case 306: // Mirar ataud (look at coffin): normal/wide coffin description.
		beginSecondarySpeechLine(5, _vm->gameState().scene4090WideCoffinVariant != 0 ? 1 : 0);
		return true;
	case 307: // Coger partitura (take sheet music): Ron refuses to take it.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar partitura (look at sheet music): Bach's Tocata and Fugue.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Usar camafeo de Samarkanda con organo (use Samarkanda cameo with organ): final D09 cutscene.
		runFinalCutscene();
		return true;
	case 231: // Usar camafeo/otros objetos con partitura/objetos D09: generic Ron item-combination failure.
		beginSharedInventorySpeechLine(0xda, randomSharedInventorySpeechFrame(1));
		return true;
	default:
		return false;
	}
}

bool Scene4090::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0, 0x036f);
	targetY = CLIP<int>(targetY, 0, HollywoodEngine::kSceneBufferHeight - 1);

	do {
		if (targetY < HollywoodEngine::kSceneBufferHeight - 1)
			++targetY;
	} while (walkableMaskAt(targetX, targetY) == 0 &&
		targetY != HollywoodEngine::kSceneBufferHeight - 1);
	return true;
}

bool Scene4090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	GameplayState &state = _vm->gameState();
	if (state.scene4080CoffinShiftedState != 0)
		state.scene4090WideCoffinVariant = 1;
	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (state.scene4090WideCoffinVariant != 0) {
		if (_sceneChunkTable.isValidChunk(kScene4090AlternatePatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4090AlternatePatchChunk], _baseFramebuffer);
		setSmallRowText(3, getGameStrings(_vm->getLanguage()).wideCoffinName);
		replaceColorMapItemFromOriginal(5, 3);
	} else {
		replaceColorMapItemFromOriginal(5, 0);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene4090::shouldConvertSavedFramebufferFF() const {
	return true;
}

AmbientAudioProfile Scene4090::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4090AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

void Scene4090::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId == kScene4090OrganOverlayHook)
		startOrganBodyAnimation(0, ARRAYSIZE(kScene4090OrganBodyFrameMap) - 1, true);
}

byte Scene4090::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	const uint groupOffset = animationGroup == kScene4090FinalPrimarySpeechAlternateGroup ?
		kScene4090FinalPrimarySpeechFramesPerGroup : 0;
	return kScene4090FinalPrimarySpeechFrameMap[groupOffset];
}

void Scene4090::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	const uint groupOffset = animationGroup == kScene4090FinalPrimarySpeechAlternateGroup ?
		kScene4090FinalPrimarySpeechFramesPerGroup : 0;
	const byte baseFrame = kScene4090FinalPrimarySpeechFrameMap[groupOffset];
	const byte frameOffset = frameIndex >= baseFrame ?
		MIN<byte>(frameIndex - baseFrame, kScene4090FinalPrimarySpeechFramesPerGroup - 1) : 0;
	const uint mappedFrameOffset = groupOffset + frameOffset;
	if (mappedFrameOffset >= ARRAYSIZE(kScene4090FinalPrimarySpeechFrameMap))
		return;

	const uint finalBaseChunk = _vm->gameState().scene4090WideCoffinVariant != 0 ?
		kScene4090FinalVariantAlternateBaseChunk : kScene4090FinalVariantBaseChunk;
	_sceneLayers.setLayerResource(kScene4090ScriptLayer, finalBaseChunk,
		kScene4090FinalCloseDescriptorCount, nullptr, 0);
	_sceneLayers.setLayerVisible(kScene4090ScriptLayer, true);
	_sceneLayers.setLayerFrame(kScene4090ScriptLayer,
		kScene4090FinalPrimarySpeechFrameMap[mappedFrameOffset]);
}

void Scene4090::resetAnimationLayers() {
	clearSceneLayer(kScene4090ScriptLayer);
	_realtimeAnimationTracks.reset(_randomAmbientTrack);
	_realtimeAnimationTracks.setActive(_randomAmbientTrack, false);
	_sceneLayers.setLayerFrame(kScene4090AmbientRandomLayer, 10);
	_sceneLayers.setLayerFrame(kScene4090AmbientFixedLayer, 4);
	_sceneLayers.setLayerFrame(kScene4090OrganBodyLayer, 0);
	_sceneLayers.setLayerVisible(kScene4090AmbientRandomLayer, false);
	_sceneLayers.setLayerVisible(kScene4090AmbientFixedLayer, false);
	_sceneLayers.setLayerVisible(kScene4090OrganBodyLayer, false);
	_organBodyChannel.reset(0, kScene4090OrganBodyFrameMillis);
	_ambientSoundTimerAccumulator = 0;
	_organBodyTargetFrame = 0;
	_organBodyAnimationActive = false;
	_organBodyWaitForSound = false;
	_randomAmbientAnimationActive = false;
	_multiSpriteCompositeActive = false;
}

void Scene4090::drawForegroundLayers(int activeWorldY, bool includeHighLayer) {
	if (includeHighLayer && activeWorldY < kScene4090ForegroundActorThresholdY &&
			_sceneChunkTable.isValidChunk(kScene4090ForegroundHighChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4090ForegroundHighChunk],
			_sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(kScene4090ForegroundLowChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4090ForegroundLowChunk],
			_sceneFramebuffer);
}

void Scene4090::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
		_originalColorToItemMap.size());
}

void Scene4090::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene4090::setSmallRowText(byte row, const char *text) {
	const uint destinationOffset = (uint)row * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	byte *destination = _stage003SmallRows.data() + destinationOffset;
	memset(destination, 0, kStage003SmallRowSize);
	const uint textSize = MIN<uint>((uint)strlen(text), kStage003SmallRowSize - 1);
	memcpy(destination, text, textSize);
}

void Scene4090::advanceAmbientSound(uint32 delta) {
	_ambientSoundTimerAccumulator += delta;
	while (_ambientSoundTimerAccumulator >= kScene4090AmbientCheckMillis) {
		_ambientSoundTimerAccumulator -= kScene4090AmbientCheckMillis;
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[1];
		if (player.isPlaying() || _random.getRandomNumber(24) != 0)
			continue;

		if (_random.getRandomNumber(9) == 0) {
			player.playSample(0x0e, 50);
			continue;
		}

		byte cue = 0;
		do {
			cue = (byte)(0x0f + _random.getRandomNumber(4));
		} while (cue == _previousAmbientSoundCue);
		_previousAmbientSoundCue = cue;
		player.playSample(cue, 8);
	}
}

void Scene4090::startOrganBodyAnimation(byte firstFrame, byte targetFrame,
		bool waitForSound) {
	_organBodyChannel.reset(firstFrame, kScene4090OrganBodyFrameMillis);
	_organBodyTargetFrame = targetFrame;
	_organBodyAnimationActive = firstFrame < targetFrame;
	_organBodyWaitForSound = waitForSound;
	if (_sceneLayers.hasLayer(kScene4090OrganBodyLayer))
		_sceneLayers.setVisibleLayerFrame(kScene4090OrganBodyLayer, firstFrame);
}

void Scene4090::advanceOrganBodyAnimation(uint32 delta) {
	if (!_organBodyAnimationActive)
		return;

	const uint frameCount = _organBodyChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _organBodyAnimationActive; ++i) {
		if (_organBodyWaitForSound && _organBodyChannel.frameIndex >= 10 &&
				_soundBank0.isPlaying())
			continue;

		if (_organBodyChannel.frameIndex < _organBodyTargetFrame)
			++_organBodyChannel.frameIndex;
		_sceneLayers.setVisibleLayerFrame(kScene4090OrganBodyLayer,
			_organBodyChannel.frameIndex);
		if (_organBodyChannel.frameIndex >= _organBodyTargetFrame)
			_organBodyAnimationActive = false;
	}
}

bool Scene4090::waitForOrganBodyAnimation() {
	while (_organBodyAnimationActive && !animationPlaybackShouldStop()) {
		if (waitSceneMillis(kScene4090OrganBodyFrameMillis, false))
			return false;
	}
	return !animationPlaybackShouldStop();
}

void Scene4090::setMultiSpriteLayersVisible(bool visible) {
	_sceneLayers.setLayerVisible(kScene4090AmbientRandomLayer,
		visible && _sceneChunkTable.isValidChunk(kScene4090AmbientRandomChunk));
	_sceneLayers.setLayerVisible(kScene4090AmbientFixedLayer,
		visible && _sceneChunkTable.isValidChunk(kScene4090AmbientFixedChunk));
	_sceneLayers.setLayerVisible(kScene4090OrganBodyLayer,
		visible && _sceneChunkTable.isValidChunk(kScene4090OrganBodyChunk));
}

void Scene4090::stopMultiSpriteAnimation() {
	_randomAmbientAnimationActive = false;
	_organBodyAnimationActive = false;
	setMultiSpriteLayersVisible(false);
	_multiSpriteCompositeActive = false;
}

void Scene4090::runDoorExit() {
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4090ScriptLayer, kScene4090DoorExitChunk,
		kScene4090DoorExitDescriptorCount,
		AnimationFrameRange(kScene4090DoorExitDescriptorCount, kScene4090FrameMillis)
			.holdFirstFrame().unskippable().noFinalFrameDelay());
	if (!sequence.completed())
		return;
	sequence.sound(3)
		.commit(_vm->gameState().mainFlowStateId, kScene4090DoorExitState);
}

void Scene4090::runOrganRevealSequence() {
	GameplayState &state = _vm->gameState();
	if (!state.scene4090OrganRevealDialogueSeen) {
		beginSecondarySpeechLine(3, 0);
		state.scene4090OrganRevealDialogueSeen = true;
	} else {
		beginSecondarySpeechLine(3, 2);
	}

	if (!walkActiveActorTo(0x01fc, 0x015b, 5, 0, false) ||
			waitSceneMillis(1000, false) ||
			!walkActiveActorTo(0x01fc, 0x015b, 1, 0, false) ||
			waitSceneMillis(1000, false))
		return;

	_multiSpriteCompositeActive = true;
	setMultiSpriteLayersVisible(true);
	_sceneLayers.setVisibleLayerFrame(kScene4090AmbientRandomLayer, 10);
	_sceneLayers.setVisibleLayerFrame(kScene4090AmbientFixedLayer, 4);
	_sceneLayers.setVisibleLayerFrame(kScene4090OrganBodyLayer, 0);
	_randomAmbientAnimationActive = false;
	_organBodyAnimationActive = false;
	const bool spanishDemo = _vm->isDemo() && _vm->getLanguage() == Common::ES_ESP;
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene4090ScriptLayer,
		kScene4090OrganOverlayChunk, kScene4090OrganOverlayDescriptorCount,
		kScene4090OrganOverlayFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4090OrganOverlayFrameMap) - 1,
			kScene4090FrameMillis)
			.soundAt(3, spanishDemo ? 0x26 : 0x3d)
			.hookAt(3, kScene4090OrganOverlayHook)
			.unskippable())
		.actorPath(SceneActorPose(0x0294, 0x0175, 5));
	const bool bodyComplete = sequence.completed() && waitForOrganBodyAnimation();

	stopMultiSpriteAnimation();
	if (!bodyComplete)
		return;
	beginSecondarySpeechLine(3, 1);
}

void Scene4090::runCoffinSwapSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene4080GwendolynState != 0) {
		beginSecondarySpeechLine(4, 0);
		return;
	}
	if (state.scene4090WideCoffinVariant != 0) {
		beginSecondarySpeechLine(4, 3);
		return;
	}

	beginSecondarySpeechLine(4, 1);
	if (!runCoffinInsertSequence())
		return;
	beginSecondarySpeechLine(4, 2);
}

bool Scene4090::runCoffinInsertSequence() {
	const uint requiredChunks[] = {
		kScene4090CoffinBookendChunk,
		kScene4090CoffinFirstClipChunk,
		kScene4090CoffinSecondClipChunk,
		kScene4090CoffinFramebufferChunk,
		kScene4090CoffinPaletteChunk
	};
	for (uint i = 0; i < ARRAYSIZE(requiredChunks); ++i) {
		if (!_sceneChunkTable.isValidChunk(requiredChunks[i])) {
			warning("Scene 4090 coffin insert is missing RESOURCE.D09 chunk %u",
				requiredChunks[i]);
			return false;
		}
	}

	Graphics::ManagedSurface insertFramebuffer;
	insertFramebuffer.create(HollywoodEngine::kSceneBufferWidth,
		HollywoodEngine::kSceneBufferHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Array<byte> insertPaletteChunk;
	insertPaletteChunk.resize(kPaletteSize);
	if (!loadFixedChunk(kScene4090CoffinFramebufferChunk, insertFramebuffer,
			kSceneBufferByteCount) ||
			!loadFixedChunk(kScene4090CoffinPaletteChunk, insertPaletteChunk,
				kPaletteSize)) {
		warning("Scene 4090 failed to load the coffin insert framebuffer or palette");
		return false;
	}

	const uint16 savedViewportX = _viewportXOffset;
	const Common::Array<byte> savedPalette = _paletteCurrent;
	Common::Array<byte> insertPalette = savedPalette;
	const uint paletteBytes = MIN<uint>(kScene4090CoffinPaletteBytes,
		MIN<uint>(_sceneChunkTable.sizes[kScene4090CoffinPaletteChunk],
			insertPalette.size()));
	memcpy(insertPalette.data(), insertPaletteChunk.data(), paletteBytes);
	if (insertPalette.size() >= kScene4090CoffinPaletteBytes + 3)
		memset(insertPalette.data() + kScene4090CoffinPaletteBytes, 0, 3);

	bool completed = playResourceLayerSequence(kScene4090ScriptLayer,
		kScene4090CoffinBookendChunk, kScene4090CoffinBookendDescriptorCount,
		AnimationFrameRange(0, kScene4090CoffinBookendDescriptorCount - 1,
			kScene4090FrameMillis).unskippable().noFinalFrameDelay(), false);
	if (!completed) {
		clearSceneLayer(kScene4090ScriptLayer);
		return false;
	}

	completed = runCurtainClearToBlack();
	if (completed) {
		_viewportXOffset = kScene4090CoffinViewportX;
		_sceneFramebuffer.copyRectToSurface(insertFramebuffer.rawSurface(), 0, 0,
			Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
				HollywoodEngine::kSceneBufferHeight));
		_coffinPaletteCycleAccumulator = 0;
		completed = runCurtainReveal(insertFramebuffer, insertPalette);
	}
	if (completed)
		completed = !waitFullscreenAnimationFrame(1000, false);
	if (completed)
		completed = playCoffinDeltaClip(kScene4090CoffinFirstClipChunk);
	if (completed)
		completed = !waitFullscreenAnimationFrame(1000, false);
	if (completed)
		completed = playCoffinDeltaClip(kScene4090CoffinSecondClipChunk);
	if (completed)
		completed = runCurtainClearToBlack();

	_viewportXOffset = savedViewportX;
	if (completed) {
		GameplayState &state = _vm->gameState();
		state.scene4080CoffinShiftedState = 1;
		state.scene4090WideCoffinVariant = 1;
	}
	applySceneStateToHotspotsAndPatches(0xff);

	_sceneLayers.setLayerResource(kScene4090ScriptLayer,
		kScene4090CoffinBookendChunk, kScene4090CoffinBookendDescriptorCount,
		nullptr, 0);
	_sceneLayers.setLayerVisible(kScene4090ScriptLayer, true);
	_sceneLayers.setLayerFrame(kScene4090ScriptLayer, 3);
	drawPlayableComposite();
	Graphics::ManagedSurface restoredRoom;
	restoredRoom.copyFrom(_sceneFramebuffer);

	if (!animationPlaybackShouldStop()) {
		completed = runCurtainReveal(restoredRoom, savedPalette) && completed;
	} else {
		_sceneFramebuffer.copyRectToSurface(restoredRoom.rawSurface(), 0, 0,
			Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
				HollywoodEngine::kSceneBufferHeight));
		_paletteCurrent = savedPalette;
		invalidatePresentationPalette();
		completed = false;
	}

	if (completed) {
		completed = playResourceLayerSequence(kScene4090ScriptLayer,
			kScene4090CoffinBookendChunk, kScene4090CoffinBookendDescriptorCount,
			AnimationFrameRange(3, 0, kScene4090FrameMillis)
				.unskippable().noFinalFrameDelay());
	} else {
		clearSceneLayer(kScene4090ScriptLayer);
	}

	_paletteCurrent = savedPalette;
	invalidatePresentationPalette();
	return completed;
}

bool Scene4090::playCoffinDeltaClip(uint chunkIndex) {
	for (uint frame = 0; frame < kCoffinDeltaClipFrameCount; ++frame) {
		if (animationPlaybackShouldStop())
			return false;

		drawClipFrameDelta(chunkIndex, kScene4090CoffinClipDescriptorCount,
			kCoffinDeltaClipFrames[frame]);
		presentFrame();

		const uint frameCounter = frame + 1;
		if (frameCounter == 0x0b || frameCounter == 0x1b)
			_soundBank0.playSample(4, 100);
		if (frameCounter == 0x10 || frameCounter == 0x21)
			_soundBank0.playSample(5, 100);
		if (frameCounter == 0x0d || frameCounter == 0x0f ||
				frameCounter == 0x14 || frameCounter == 0x19 ||
				frameCounter == 0x1d)
			playResidentSoundEffect(1);

		if (frame + 1 < kCoffinDeltaClipFrameCount &&
				waitFullscreenAnimationFrame(kScene4090FrameMillis, false))
			return false;
	}
	return true;
}

bool Scene4090::advanceCoffinPaletteCycle(uint32 delta) {
	bool changed = false;
	_coffinPaletteCycleAccumulator += delta;
	while (_coffinPaletteCycleAccumulator >= kScene4090CoffinPaletteCycleMillis) {
		_coffinPaletteCycleAccumulator -= kScene4090CoffinPaletteCycleMillis;
		rotateCoffinPaletteCycle();
		changed = true;
	}
	return changed;
}

void Scene4090::rotateCoffinPaletteCycle() {
	const uint lastOffset = kScene4090CoffinPaletteCycleLastColor * 3;
	if (_paletteCurrent.size() <= lastOffset + 2)
		return;

	byte saved[3];
	memcpy(saved, _paletteCurrent.data() + lastOffset, sizeof(saved));
	for (uint color = kScene4090CoffinPaletteCycleLastColor;
			color > kScene4090CoffinPaletteCycleFirstColor; --color) {
		memcpy(_paletteCurrent.data() + color * 3,
			_paletteCurrent.data() + (color - 1) * 3, sizeof(saved));
	}
	memcpy(_paletteCurrent.data() + kScene4090CoffinPaletteCycleFirstColor * 3,
		saved, sizeof(saved));
	invalidatePresentationPalette();
}

bool Scene4090::runCurtainReveal(const Graphics::ManagedSurface &source,
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
	for (int sweep = kScene4090CurtainStartOffset;
			sweep >= 0 && !animationPlaybackShouldStop();
			sweep -= kScene4090CurtainBandWidth) {
		applyCurtainBand(&sourceSurface, (uint)sweep, kScene4090CurtainBandWidth);
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

bool Scene4090::runCurtainClearToBlack() {
	for (uint sweep = 0;
			sweep < kScene4090CurtainEndOffset && !animationPlaybackShouldStop();
			sweep += kScene4090CurtainBandWidth) {
		applyCurtainBand(nullptr, sweep, kScene4090CurtainBandWidth);
		presentFrame();
		if (pollEvents(false))
			break;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	invalidatePresentationPalette();
	presentFrame();
	return !animationPlaybackShouldStop();
}

void Scene4090::applyCurtainBand(const Graphics::Surface *source,
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

void Scene4090::runFinalCutscene() {
	GameplayState &state = _vm->gameState();
	if (state.scene4090FinalCutsceneCompleted != 0) {
		beginStaticSecondarySpeechLine(0x50, 1);
		return;
	}

	const uint finalBaseChunk = state.scene4090WideCoffinVariant != 0 ?
		kScene4090FinalVariantAlternateBaseChunk : kScene4090FinalVariantBaseChunk;
	const uint requiredChunks[] = {
		kScene4090OrganBodyChunk,
		kScene4090FinalOverlayChunk,
		kScene4090AmbientRandomChunk,
		kScene4090AmbientFixedChunk,
		finalBaseChunk,
		finalBaseChunk + 1
	};
	for (uint i = 0; i < ARRAYSIZE(requiredChunks); ++i) {
		if (!_sceneChunkTable.isValidChunk(requiredChunks[i])) {
			warning("Scene 4090 final cutscene is missing RESOURCE.D09 chunk %u",
				requiredChunks[i]);
			return;
		}
	}

	BlockingSequence sequence(*this);
	sequence.actorPath(SceneActorPose(0x0166, 0x0171, 1));
	if (!sequence.completed())
		return;
	const bool firstDialogue = !state.scene4090FinalCutsceneDialogueSeen;
	if (firstDialogue)
		beginSecondarySpeechLine(8, 0);

	_multiSpriteCompositeActive = true;
	setMultiSpriteLayersVisible(true);
	_sceneLayers.setVisibleLayerFrame(kScene4090AmbientRandomLayer, 10);
	_sceneLayers.setVisibleLayerFrame(kScene4090AmbientFixedLayer, 4);
	_sceneLayers.setVisibleLayerFrame(kScene4090OrganBodyLayer, 0);
	_randomAmbientAnimationActive = false;
	_organBodyAnimationActive = false;
	sequence.resourceLayerFrames(kScene4090ScriptLayer, kScene4090FinalOverlayChunk,
		kScene4090FinalOverlayDescriptorCount,
		AnimationFrameRange(kScene4090FinalOverlayDescriptorCount, kScene4090FrameMillis)
			.visibleMappedLayerFrames(kScene4090AmbientFixedLayer,
				kScene4090FinalRoomChunk13FrameMap,
				ARRAYSIZE(kScene4090FinalRoomChunk13FrameMap))
			.unskippable());
	if (!sequence.completed()) {
		stopMultiSpriteAnimation();
		return;
	}

	state.currentAmbientMusicCueId = 0x10;
	_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, 100);
	startOrganBodyAnimation(2, 10, false);
	_randomAmbientAnimationActive = true;
	sequence.resourceLayerFrames(kScene4090ScriptLayer, kScene4090FinalOverlayChunk,
		kScene4090FinalOverlayDescriptorCount,
		AnimationFrameRange(0, kScene4090FinalRoomOverlayHoldFrames - 1,
			kScene4090FrameMillis).repeatFrame(
				kScene4090FinalOverlayDescriptorCount - 1).unskippable())
		.resourceLayerFrames(kScene4090ScriptLayer, kScene4090FinalOverlayChunk,
			kScene4090FinalOverlayDescriptorCount,
			AnimationFrameRange(kScene4090FinalOverlayDescriptorCount - 1, 0,
				kScene4090FrameMillis)
				.visibleMappedLayerFrames(kScene4090AmbientFixedLayer,
					kScene4090FinalRoomChunk13FrameMap,
					ARRAYSIZE(kScene4090FinalRoomChunk13FrameMap))
				.unskippable());
	if (!sequence.completed()) {
		stopMultiSpriteAnimation();
		return;
	}

	if (firstDialogue)
		beginSecondarySpeechLine(8, 1);
	beginSecondarySpeechLine(8, 2);
	sequence.actorPath(SceneActorPose(kScene4090ReturnEntryStartX,
		kScene4090ReturnEntryStartY, 1));
	if (!sequence.completed()) {
		stopMultiSpriteAnimation();
		return;
	}

	_organBodyAnimationActive = false;
	_randomAmbientAnimationActive = false;
	const uint organTailFirstFrame = _organBodyChannel.frameIndex + 1;
	if (organTailFirstFrame < ARRAYSIZE(kScene4090OrganBodyFrameMap)) {
		sequence.resourceLayerFrames(kScene4090OrganBodyLayer,
			kScene4090OrganBodyChunk, kScene4090OrganBodyDescriptorCount,
			kScene4090OrganBodyFrameMap,
			AnimationFrameRange(organTailFirstFrame,
				ARRAYSIZE(kScene4090OrganBodyFrameMap) - 1,
				kScene4090OrganBodyFrameMillis).unskippable().noFinalFrameDelay(),
			false);
		if (!sequence.completed()) {
			stopMultiSpriteAnimation();
			return;
		}
	}
	_sceneLayers.setVisibleLayerFrame(kScene4090AmbientFixedLayer, 4);
	_sceneLayers.setVisibleLayerFrame(kScene4090AmbientRandomLayer, 10);
	drawPlayableComposite();
	presentFrame();

	sequence.resourceLayerFrames(kScene4090ScriptLayer, finalBaseChunk + 1,
			kScene4090FinalOpenDescriptorCount,
			AnimationFrameRange(kScene4090FinalOpenDescriptorCount,
				kScene4090FrameMillis).unskippable().noFinalFrameDelay())
		.resourceLayerFrames(kScene4090ScriptLayer, finalBaseChunk,
			kScene4090FinalCloseDescriptorCount,
			AnimationFrameRange(kScene4090FinalCloseDescriptorCount,
				kScene4090FrameMillis).unskippable().noFinalFrameDelay());
	if (!sequence.completed()) {
		stopMultiSpriteAnimation();
		return;
	}

	if (firstDialogue) {
		beginPrimarySpeechLineWithAnimationGroup(8, 3, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechNormalGroup);
		state.scene4090FinalCutsceneDialogueSeen = true;
	} else {
		beginPrimarySpeechLineWithAnimationGroup(8, 4, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechNormalGroup);
	}
	sequence.resourceLayerFrames(kScene4090ScriptLayer, finalBaseChunk,
		kScene4090FinalCloseDescriptorCount,
		AnimationFrameRange(kScene4090FinalCloseDescriptorCount - 1, 0,
			kScene4090FrameMillis).unskippable().noFinalFrameDelay(), false);
	if (!sequence.completed() || waitSceneMillis(kScene4090FastFrameMillis, false)) {
		stopMultiSpriteAnimation();
		return;
	}
	sequence.resourceLayerFrames(kScene4090ScriptLayer, finalBaseChunk,
		kScene4090FinalCloseDescriptorCount, kScene4090FinalFadeFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4090FinalFadeFrameMap) - 1,
			kScene4090FastFrameMillis).unskippable().noFinalFrameDelay());
	if (!sequence.completed()) {
		stopMultiSpriteAnimation();
		return;
	}
	if (state.scene4090WideCoffinVariant != 0) {
		beginPrimarySpeechLineWithAnimationGroup(8, 6, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechAlternateGroup);
	} else {
		beginPrimarySpeechLineWithAnimationGroup(8, 5, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechNormalGroup);
	}

	if (!runCurtainClearToBlack())
		return;
	clearSceneLayer(kScene4090ScriptLayer);
	stopMultiSpriteAnimation();
	state.mainFlowStateId = kScene4090FinalReturnState;
}

} // End of namespace Hollywood
