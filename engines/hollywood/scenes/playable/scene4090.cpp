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

#include "hollywood/scenes/playable/scene4090.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

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
const uint kScene4090ForegroundLowChunk = 5;
const uint kScene4090ForegroundHighChunk = 6;
const uint kScene4090DoorExitChunk = 7;
const uint kScene4090DoorExitDescriptorCount = 6;
const uint kScene4090OrganBodyChunk = 8;
const uint kScene4090OrganBodyDescriptorCount = 0x0b;
const uint kScene4090AmbientRandomLayer = 0;
const uint kScene4090AmbientFixedLayer = 1;
const uint kScene4090OrganBodyLayer = 2;
const uint kScene4090OrganOverlayChunk = 9;
const uint kScene4090OrganOverlayDescriptorCount = 4;
const uint kScene4090AlternatePatchChunk = 10;
const uint kScene4090FinalOverlayChunk = 11;
const uint kScene4090FinalOverlayDescriptorCount = 0x11;
const uint kScene4090AmbientRandomChunk = 12;
const uint kScene4090AmbientRandomDescriptorCount = 0x0b;
const uint kScene4090AmbientFixedChunk = 13;
const uint kScene4090AmbientFixedDescriptorCount = 5;
const uint kScene4090FinalVariantBaseChunk = 15;
const uint kScene4090FinalVariantAlternateBaseChunk = 17;
const uint kScene4090FinalCloseDescriptorCount = 0x0f;
const uint kScene4090FinalOpenDescriptorCount = 0x23;
const byte kScene4090FinalPrimarySpeechNormalGroup = 0;
const byte kScene4090FinalPrimarySpeechAlternateGroup = 1;
const uint kScene4090FinalPrimarySpeechFramesPerGroup = 5;
const uint32 kScene4090FinalRoomOverlayHoldFrames = 100;
const int kScene4090ForegroundActorThresholdY = 0x0172;
const char *const kScene4090WideCoffinName = " ata\xa3""d ancho";

enum Scene4090AnimationHookId {
	kScene4090OrganOverlayHook = 1,
	kScene4090OrganBodyContinuationHook,
	kScene4090FinalRoomOverlayHook,
	kScene4090FinalRoomHoldHook
};

const byte kScene4090DoorExitFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5
};

const byte kScene4090OrganOverlayFrameMap[] = {
	0, 1, 2, 3, 3, 3, 3, 2, 1, 0
};

const byte kScene4090OrganBodyFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 5,
	4, 3, 2, 1, 0
};

const byte kScene4090FinalOpenFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34
};

const byte kScene4090FinalRoomOverlayFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

const byte kScene4090FinalRoomChunk13FrameMap[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 1, 2, 3
};

const byte kScene4090FinalCloseFrameMap[] = {
	0, 1, 2, 3, 4, 5
};

const byte kScene4090FinalCloseReverseFrameMap[] = {
	5, 4, 3, 2, 1, 0
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
		SceneResourceLayout(5, 5, 18),
		SceneViewport(kScene4090ViewportInitialX, kScene4090ViewportMinX, kScene4090ViewportMaxX),
		SceneActorPose(kScene4090DefaultActorX, kScene4090DefaultActorY, kScene4090DefaultActorFacing));
	config.setActorResources(kScene4090ActorBankTableEntry, kScene4090ActorPaletteTableEntry);
	config.setTextResources(kScene4090Resource003RowsOffsetIndex, kScene4090SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene4090::Scene4090(HollywoodEngine *vm) :
		PlayableScene(vm, scene4090Config()),
		_ambientLayers(),
		_scriptLayer(),
		_chunk12Channel(),
		_originalColorToItemMap() {
}

void Scene4090::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	rememberOriginalColorMap();

	setActiveActorPose(kScene4090DefaultActorX, kScene4090DefaultActorY, kScene4090DefaultActorFacing);
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawTransientLayers(_ambientLayers);

	if (_scriptLayer.visible) {
		drawResourceSpriteLayer(_scriptLayer);
		drawActionOverlayLayer();
		return;
	}
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawForegroundLayers(activeWorldY);
	drawActionOverlayLayer();
}

void Scene4090::runCustomEntrySequence() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	_soundBank0.playSample(5, 100);

	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene4090FirstState) {
		setActiveActorPose(kScene4090DefaultActorX, kScene4090DefaultActorY, state.scene4090InitialGreetingSeen ? kScene4090DefaultActorFacing : 1);
		drawPlayableComposite();
		presentFrame();
		if (!state.scene4090InitialGreetingSeen) {
			beginSecondarySpeechLine(0, 0);
			state.scene4090InitialGreetingSeen = true;
		}
		return;
	}

	runEntryPath(kScene4090ReturnEntryStartX, kScene4090ReturnEntryStartY, 1,
		kScene4090ReturnEntryTargetX, kScene4090ReturnEntryTargetY);
	if (state.scene4090WideCoffinVariant == 0) {
		beginSecondarySpeechLine(8, 7);
	} else {
		beginSecondarySpeechLine(8, 8);
		state.scene4090FinalCutsceneCompleted = 1;
	}
}

bool Scene4090::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene4090::advanceCustomGameplayLoop(uint32 delta) {
	if (!_ambientLayers.hasLayer(kScene4090AmbientRandomLayer) ||
			!_ambientLayers.layer(kScene4090AmbientRandomLayer).visible ||
			_primaryDialogueSpeechActive)
		return false;

	const uint frameCount = _chunk12Channel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		const byte nextFrame = (byte)_random.getRandomNumber(10);
		_chunk12Channel.frameIndex = nextFrame;
		_ambientLayers.setLayerFrame(kScene4090AmbientRandomLayer, nextFrame);
	}
	return false;
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
	case 305: // Coger ataud (take coffin): unavailable in the live room.
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
	targetX = CLIP<int>(targetX, 0, HollywoodEngine::kSceneBufferWidth - 1);
	targetY = CLIP<int>(targetY, 0, HollywoodEngine::kSceneBufferHeight - 1);

	while (targetY < HollywoodEngine::kSceneBufferHeight - 1) {
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

bool Scene4090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (_vm->gameState().scene4090WideCoffinVariant != 0) {
		if (_sceneChunkTable.isValidChunk(kScene4090AlternatePatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4090AlternatePatchChunk], _baseFramebuffer);
		setSmallRowText(3, kScene4090WideCoffinName);
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
	return createRandomAmbientAudioProfile(0x0f, 5, 8, 25, 0x0b, 5, 100, 50);
}

void Scene4090::handleAnimationFrameHook(byte hookId, uint frame) {
	switch (hookId) {
	case kScene4090OrganOverlayHook:
		if (frame == 3)
			_soundBank0.playSample(0x3d, 100);
		if (frame >= 3) {
			const byte bodyFrame = (byte)MIN<uint>(frame - 3,
				ARRAYSIZE(kScene4090OrganBodyFrameMap) - 1);
			_ambientLayers.setLayerFrame(kScene4090OrganBodyLayer, bodyFrame);
		}
		break;
	case kScene4090OrganBodyContinuationHook:
		_ambientLayers.setLayerFrame(kScene4090OrganBodyLayer, (byte)(frame + 7));
		break;
	case kScene4090FinalRoomOverlayHook:
		if (frame < ARRAYSIZE(kScene4090FinalRoomChunk13FrameMap))
			_ambientLayers.setLayerFrame(kScene4090AmbientFixedLayer,
				kScene4090FinalRoomChunk13FrameMap[frame]);
		break;
	case kScene4090FinalRoomHoldHook: {
		const byte bodyFrame = _ambientLayers.layerFrame(kScene4090OrganBodyLayer);
		if (bodyFrame < 10)
			_ambientLayers.setLayerFrame(kScene4090OrganBodyLayer, bodyFrame + 1);
		_ambientLayers.setLayerFrame(kScene4090AmbientFixedLayer, 3);
		break;
	}
	default:
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		break;
	}
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
	_scriptLayer.configure(finalBaseChunk, kScene4090FinalCloseDescriptorCount, nullptr, 0);
	_scriptLayer.visible = true;
	_scriptLayer.setFrame(kScene4090FinalPrimarySpeechFrameMap[mappedFrameOffset]);
}

void Scene4090::resetAnimationLayers() {
	clearResourceLayer(_scriptLayer);
	_ambientLayers.clear();
	_ambientLayers.configureLayer(kScene4090AmbientRandomLayer, kScene4090AmbientRandomChunk,
		kScene4090AmbientRandomDescriptorCount, nullptr, 0, false);
	_ambientLayers.setLayerFramePreservingVisibility(kScene4090AmbientRandomLayer, 10);
	_ambientLayers.configureLayer(kScene4090AmbientFixedLayer, kScene4090AmbientFixedChunk,
		kScene4090AmbientFixedDescriptorCount, nullptr, 0, false);
	_ambientLayers.setLayerFramePreservingVisibility(kScene4090AmbientFixedLayer, 4);
	_ambientLayers.configureLayer(kScene4090OrganBodyLayer, kScene4090OrganBodyChunk,
		kScene4090OrganBodyDescriptorCount,
		kScene4090OrganBodyFrameMap, ARRAYSIZE(kScene4090OrganBodyFrameMap));
	_ambientLayers.setLayerVisible(kScene4090OrganBodyLayer, false);
	_chunk12Channel.reset(10, 150);
}

void Scene4090::drawForegroundLayers(int activeWorldY) {
	if (activeWorldY < kScene4090ForegroundActorThresholdY &&
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

void Scene4090::runDoorExit() {
	runActorReplacement(ActionOverlaySpec(kScene4090DoorExitChunk, kScene4090DoorExitDescriptorCount,
		kScene4090DoorExitFrameMap, ARRAYSIZE(kScene4090DoorExitFrameMap), kScene4090FrameMillis)
		.endAt(ARRAYSIZE(kScene4090DoorExitFrameMap)));
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene4090DoorExitState;
}

void Scene4090::runOrganRevealSequence() {
	GameplayState &state = _vm->gameState();
	if (!state.scene4090OrganRevealDialogueSeen) {
		beginSecondarySpeechLine(3, 0);
		state.scene4090OrganRevealDialogueSeen = true;
	} else {
		beginSecondarySpeechLine(3, 2);
	}

	setActiveActorPose(0x01fc, 0x015b, 5);
	drawPlayableComposite();
	presentFrame();
	waitSceneMillis(1000);
	_activeActorFacing = 1;
	drawPlayableComposite();
	presentFrame();
	waitSceneMillis(1000);

	_ambientLayers.setLayerVisible(kScene4090AmbientRandomLayer, true);
	_ambientLayers.setLayerFrame(kScene4090AmbientRandomLayer, 10);
	_ambientLayers.setLayerVisible(kScene4090AmbientFixedLayer, true);
	_ambientLayers.setLayerFrame(kScene4090AmbientFixedLayer, 4);
	_ambientLayers.setLayerVisible(kScene4090OrganBodyLayer, true);
	_ambientLayers.setLayerFrame(kScene4090OrganBodyLayer, 0);
	playResourceLayerSequence(_scriptLayer, kScene4090OrganOverlayChunk, kScene4090OrganOverlayDescriptorCount,
		kScene4090OrganOverlayFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4090OrganOverlayFrameMap) - 1,
			kScene4090FrameMillis).hookEveryFrame(kScene4090OrganOverlayHook));
	playResourceLayerSequence(_scriptLayer, kScene4090OrganOverlayChunk, kScene4090OrganOverlayDescriptorCount,
		kScene4090OrganOverlayFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4090OrganBodyFrameMap) - 8,
			kScene4090FrameMillis).repeatFrame(ARRAYSIZE(kScene4090OrganOverlayFrameMap) - 1)
			.hookEveryFrame(kScene4090OrganBodyContinuationHook));

	_ambientLayers.setLayerVisible(kScene4090OrganBodyLayer, false);
	_ambientLayers.setLayerVisible(kScene4090AmbientRandomLayer, false);
	_ambientLayers.setLayerVisible(kScene4090AmbientFixedLayer, false);
	walkActiveActorTo(0x0294, 0x0175, 5, 0, false);
	beginSecondarySpeechLine(3, 1);
}

void Scene4090::runCoffinSwapSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene4090WideCoffinVariant != 0) {
		beginSecondarySpeechLine(4, 3);
		return;
	}

	beginSecondarySpeechLine(4, 0);
}

void Scene4090::runFinalCutscene() {
	GameplayState &state = _vm->gameState();
	if (state.scene4090FinalCutsceneCompleted != 0) {
		beginStaticSecondarySpeechLine(0x50, 1);
		return;
	}

	walkActiveActorTo(0x0166, 0x0171, 1, 0, false);
	if (!state.scene4090FinalCutsceneDialogueSeen)
		beginSecondarySpeechLine(8, 0);

	_ambientLayers.setLayerVisible(kScene4090AmbientRandomLayer, true);
	_ambientLayers.setLayerFrame(kScene4090AmbientRandomLayer, 10);
	_ambientLayers.setLayerVisible(kScene4090AmbientFixedLayer, true);
	_ambientLayers.setLayerFrame(kScene4090AmbientFixedLayer, 4);
	_ambientLayers.setLayerVisible(kScene4090OrganBodyLayer, true);
	_ambientLayers.setLayerFrame(kScene4090OrganBodyLayer, 2);
	playResourceLayerSequence(_scriptLayer, kScene4090FinalOverlayChunk, kScene4090FinalOverlayDescriptorCount,
		kScene4090FinalRoomOverlayFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene4090FinalRoomOverlayFrameMap) - 1,
			kScene4090FrameMillis).hookEveryFrame(kScene4090FinalRoomOverlayHook));

	state.currentAmbientMusicCueId = 0x10;
	_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, 100);
	playResourceLayerSequence(_scriptLayer, kScene4090FinalOverlayChunk, kScene4090FinalOverlayDescriptorCount,
		kScene4090FinalRoomOverlayFrameMap,
		AnimationFrameRange(0, kScene4090FinalRoomOverlayHoldFrames - 1,
			kScene4090FrameMillis).repeatFrame(
				ARRAYSIZE(kScene4090FinalRoomOverlayFrameMap) - 1)
			.hookEveryFrame(kScene4090FinalRoomHoldHook));
	playResourceLayerSequence(_scriptLayer, kScene4090FinalOverlayChunk, kScene4090FinalOverlayDescriptorCount,
		kScene4090FinalRoomOverlayFrameMap,
		AnimationFrameRange(ARRAYSIZE(kScene4090FinalRoomOverlayFrameMap) - 1, 1,
			kScene4090FrameMillis).hookEveryFrame(kScene4090FinalRoomOverlayHook));

	if (!state.scene4090FinalCutsceneDialogueSeen) {
		beginSecondarySpeechLine(8, 1);
		beginSecondarySpeechLine(8, 2);
	}
	walkActiveActorTo(kScene4090ReturnEntryStartX, kScene4090ReturnEntryStartY, 1, 0, false);
	const uint organTailFirstFrame = _ambientLayers.layerFrame(kScene4090OrganBodyLayer) + 1;
	if (organTailFirstFrame < ARRAYSIZE(kScene4090OrganBodyFrameMap)) {
		playResourceLayerSequence(_ambientLayers.layer(kScene4090OrganBodyLayer),
			kScene4090OrganBodyChunk, kScene4090OrganBodyDescriptorCount,
			kScene4090OrganBodyFrameMap, AnimationFrameRange(organTailFirstFrame,
				ARRAYSIZE(kScene4090OrganBodyFrameMap) - 1, 10), false); // Keep the final ambient frame.
	}
	_ambientLayers.setLayerFrame(kScene4090AmbientFixedLayer, 4);
	_ambientLayers.setLayerFrame(kScene4090AmbientRandomLayer, 10);
	drawPlayableComposite();
	presentFrame();

	const uint finalBaseChunk = state.scene4090WideCoffinVariant != 0 ?
		kScene4090FinalVariantAlternateBaseChunk : kScene4090FinalVariantBaseChunk;
	playResourceLayerSequence(_scriptLayer, finalBaseChunk + 1, kScene4090FinalOpenDescriptorCount,
		kScene4090FinalOpenFrameMap, kScene4090FrameMillis);
	playResourceLayerSequence(_scriptLayer, finalBaseChunk, kScene4090FinalCloseDescriptorCount,
		kScene4090FinalCloseFrameMap, kScene4090FrameMillis);

	if (!state.scene4090FinalCutsceneDialogueSeen) {
		beginPrimarySpeechLineWithAnimationGroup(8, 3, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechNormalGroup);
		state.scene4090FinalCutsceneDialogueSeen = true;
	} else {
		beginPrimarySpeechLineWithAnimationGroup(8, 4, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechNormalGroup);
	}
	playResourceLayerSequence(_scriptLayer, finalBaseChunk, kScene4090FinalCloseDescriptorCount,
		kScene4090FinalCloseReverseFrameMap, kScene4090FrameMillis);
	playResourceLayerSequence(_scriptLayer, finalBaseChunk, kScene4090FinalCloseDescriptorCount,
		kScene4090FinalFadeFrameMap, kScene4090FastFrameMillis);
	if (state.scene4090WideCoffinVariant != 0) {
		beginPrimarySpeechLineWithAnimationGroup(8, 6, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechAlternateGroup);
	} else {
		beginPrimarySpeechLineWithAnimationGroup(8, 5, 0x02c8, 0x0099, 0x0a, 0x19, 0x3f,
			kScene4090FinalPrimarySpeechNormalGroup);
	}

	clearResourceLayer(_scriptLayer);
	_ambientLayers.setLayerVisible(kScene4090OrganBodyLayer, false);
	_ambientLayers.setLayerVisible(kScene4090AmbientRandomLayer, false);
	_ambientLayers.setLayerVisible(kScene4090AmbientFixedLayer, false);
	state.mainFlowStateId = kScene4090FinalReturnState;
}

} // End of namespace Hollywood
