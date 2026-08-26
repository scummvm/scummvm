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

#include "hollywood/scenes/playable/scene4030.h"

#include "common/system.h"
#include "graphics/managed_surface.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const uint16 kScene4030FirstState = 0x0fbe;
const uint16 kScene4020ReturnState = 0x0fb5;
const uint16 kScene4040FirstState = 0x0fc8;
const uint16 kScene4030ViewportXOffset = 0x0060;
const uint16 kScene4030ViewportMinXOffset = 0x0060;
const uint16 kScene4030ViewportMaxXOffset = 0x0180;
const uint kScene4030ActorBankTableEntry = 0x0038;
const uint kScene4030ActorPaletteTableEntry = 0x00cc;
const uint kScene4030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4030FrameMillis = 75;
const uint32 kScene4030PropFrameMillis = 75;
const uint kScene4030LeftPropDescriptorCount = 0x1a;
const uint kScene4030RightPropDescriptorCount = 0x18;
const uint kScene4030RopePickupChunk = 20;
const uint kScene4030RopePickupDescriptorCount = 0x0e;
const uint kScene4030BoneRevealChunk = 12;
const uint kScene4030BoneRevealDescriptorCount = 0x0c;
const uint kScene4030BonePickupChunk = 19;
const uint kScene4030BonePickupDescriptorCount = 0x0c;
const uint kScene4030LeverInstallChunk = 16;
const uint kScene4030LeverInstallDescriptorCount = 0x0c;
const uint kScene4030TowerTransitionClipChunk = 6;
// The outbound callback literal in MONSTERS.EXE is 0xb3, but the shipped
// full-game RESOURCE.D03 chunk 6 has a 0xb1-entry delta table; 0xb1/0xb2
// are payload. The valid 0xb1 clip is the return-to-D03 direction, so the
// 4030 -> 4040 transition renders its cumulative states in reverse order.
const uint kScene4030TowerTransitionClipDescriptorCount = 0xb1;
const byte kScene4030TowerTransitionFinalFrameIndex = 0xda;
const byte kScene4030TowerTransitionLastValidClipFrame = 0xb0;
const int kScene4030EntryWorldX = 0x02c0;
const int kScene4030EntryWorldY = 0x0196;
const byte kScene4030EntryFacing = 2;
const byte kScene4030EntrySpeechFacing = 5;
const uint16 kScene4030EntryViewportXOffset = 0x0180;
const int kScene4030ReturnFromTowerWorldX = 0x0213;
const int kScene4030ReturnFromTowerWorldY = 0x0157;
const byte kScene4030ReturnFromTowerFacing = 4;
const uint16 kScene4030ReturnFromTowerViewportXOffset = 0x00d4;
const uint kScene4030EntryOverlayChunk = 9;
const uint kScene4030EntryOverlayDescriptorCount = 0x08;
const byte kScene4030RopeItem = 0x3d;
const byte kScene4030BoneItem = 0x3e;
const byte kScene4030LeverItem = 0x35;
const byte kScene4030RopeSceneItem = 9;
const byte kScene4030BoneSceneItem = 11;
const byte kScene4030LeverSceneItem = 12;

const byte kScene4030RopePickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13
};

const byte kScene4030BoneRevealFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11
};

const byte kScene4030BonePickupFrameMap[] = {
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
	1, 0, 0
};

const byte kScene4030LeverInstallFrameMap[] = {
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
	2, 2, 1, 0, 11
};

const byte kScene4030EntryOverlayFrameMap[] = {
	7, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kScene4030RightPropFrameRemap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	9, 10, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23
};

byte towerTransitionResourceFrameForProgress(byte progressIndex) {
	if (progressIndex <= 0x6f)
		return progressIndex;
	if (progressIndex <= 0x98)
		return 0x70;
	const byte mappedFrame = progressIndex - 0x28;
	return MIN<byte>(mappedFrame, kScene4030TowerTransitionLastValidClipFrame);
}

byte towerTransitionProgressForOutboundFrame(byte frameIndex) {
	return (byte)(kScene4030TowerTransitionFinalFrameIndex - frameIndex);
}

PlayableSceneConfig scene4030Config() {
	PlayableSceneConfig config(4030,
		SceneResourceLayout(5, 7, 20),
		SceneViewport(kScene4030ViewportXOffset, kScene4030ViewportMinXOffset, kScene4030ViewportMaxXOffset),
		SceneActorPose(kScene4030EntryWorldX, kScene4030EntryWorldY, kScene4030EntryFacing));
	config.setActorResources(kScene4030ActorBankTableEntry, kScene4030ActorPaletteTableEntry);
	config.setTextResources(kScene4030Resource003RowsOffsetIndex, kScene4030SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene4030::Scene4030(HollywoodEngine *vm) :
		PlayableScene(vm, scene4030Config()),
		_leftPropLayer(),
		_rightPropLayer(),
		_leftPropChannel(),
		_rightPropChannel(),
		_rightPropState(0) {
}

void Scene4030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	initializeSpriteLayers();

	if (_vm->gameState().mainFlowStateId == kScene4030FirstState) {
		_activeActorWorldX = kScene4030EntryWorldX;
		_activeActorWorldY = kScene4030EntryWorldY;
		_activeActorFacing = kScene4030EntryFacing;
		_viewportXOffset = kScene4030EntryViewportXOffset;
	} else {
		_activeActorWorldX = kScene4030ReturnFromTowerWorldX;
		_activeActorWorldY = kScene4030ReturnFromTowerWorldY;
		_activeActorFacing = kScene4030ReturnFromTowerFacing;
		_viewportXOffset = kScene4030ReturnFromTowerViewportXOffset;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene4030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldY;
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_leftPropLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawResourceSpriteLayer(_rightPropLayer);
	drawActionOverlayLayer();
}

void Scene4030::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene4030FirstState) {
		setActiveActorPose(kScene4030EntryWorldX, kScene4030EntryWorldY, kScene4030EntryFacing);
		_viewportXOffset = kScene4030EntryViewportXOffset;

		drawPlayableComposite();
		presentFrame();
		runActorReplacement(ActionOverlaySpec(kScene4030EntryOverlayChunk, kScene4030EntryOverlayDescriptorCount,
			kScene4030EntryOverlayFrameMap, ARRAYSIZE(kScene4030EntryOverlayFrameMap), kScene4030FrameMillis));

		if (!state.scene4030InitialEntryLineSeen) {
			_activeActorFacing = kScene4030EntrySpeechFacing;
			_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
			drawPlayableComposite();
			presentFrame();
			beginSecondarySpeechLine(1, 0);
			state.scene4030InitialEntryLineSeen = true;
		}
		return;
	}

	drawPlayableComposite();
	presentFrame();
}

bool Scene4030::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackgroundAnimations(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene4030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Transicion automatica a torreon (automatic transition to tower).
		runTowerTransitionToScene4040();
		return true;
	case 302: // Mirar corredor (look at corridor): identifies the dungeon.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar mesa de tortura (look at torture table).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar mesa de tortura (use torture table): refuses to stretch Ron.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar dama de hierro (look at iron maiden).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Usar dama de hierro (use iron maiden): refuses to enter.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Abrir dama de hierro (open iron maiden): mechanism response.
		updateIronMaidenMechanism();
		return true;
	case 308: // Cerrar dama de hierro (close iron maiden): mechanism response.
		updateIronMaidenMechanism();
		return true;
	case 309: // Coger esqueleto tumbado (take resting skeleton).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar esqueleto tumbado (look at resting skeleton).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Usar esqueleto sentado (use seated skeleton): respect response.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Mirar escalera (look at stairs): exit to moat.
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Usar escalera (use stairs): return toward the moat.
		_vm->gameState().mainFlowStateId = kScene4020ReturnState;
		return true;
	case 314: // Mirar/usar/abrir puerta (look/use/open door): punishment cell warning.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Coger candil (take oil lamp): not movable here.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 316: // Mirar candil (look at oil lamp).
		beginSecondarySpeechLine(15, 0);
		return true;
	case 317: // Coger esqueleto sentado (take seated skeleton): too weak.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 318: // Mirar esqueleto sentado (look at seated skeleton).
		beginSecondarySpeechLine(15, 0);
		return true;
	case 319: // Coger cuerda (take rope): item 0x3d.
		takeRope();
		return true;
	case 320: // Mirar cuerda (look at rope): sturdy rope.
		beginSecondarySpeechLine(16, 0);
		return true;
	case 322: // Mirar agujero (look at hole): spring state.
		beginSecondarySpeechLine(18, _vm->gameState().scene4030ImprovisedLeverInstalled ? 1 : 0);
		return true;
	case 323: // Usar agujero/resorte (use hole/spring): iron maiden mechanism.
		updateIronMaidenMechanism();
		return true;
	case 324: // Hablar con esqueleto (talk to skeleton): reveal loose bone.
		talkToSkeleton();
		return true;
	case 325: // Coger hueso (take bone): item 0x3e.
		takeBone();
		return true;
	case 326: // Mirar hueso (look at bone).
		beginSecondarySpeechLine(21, 0);
		return true;
	case 327: // Usar palanca improvisada con agujero (use improvised lever with hole).
		installImprovisedLever();
		return true;
	default:
		return false;
	}
}

bool Scene4030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene4030RopeTaken) {
		removeColorMapItem(kScene4030RopeSceneItem);
		if (_sceneChunkTable.isValidChunk(18))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[18], _baseFramebuffer);
	} else if (_sceneChunkTable.isValidChunk(17)) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[17], _baseFramebuffer);
	}

	if (state.scene4030LooseBoneState == 1) {
		if (_sceneChunkTable.isValidChunk(10))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
	} else {
		removeColorMapItem(kScene4030BoneSceneItem);
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
	}

	if (state.scene4030ImprovisedLeverInstalled) {
		replaceColorMapItem(kScene4030LeverSceneItem, 3);
		if (_sceneChunkTable.isValidChunk(16))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[16], _baseFramebuffer);
	} else {
		removeColorMapItem(kScene4030LeverSceneItem);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	initializeSpriteLayers();
	return true;
}

AmbientAudioProfile Scene4030::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0b, 3, 20, 1, 0x0b, 5, 100, 50);
}

void Scene4030::initializeSpriteLayers() {
	_leftPropLayer.configure(7, kScene4030LeftPropDescriptorCount, nullptr, 0);
	_leftPropLayer.visible = true;
	_leftPropLayer.setFrame(0);
	_leftPropLayer.hasPreviousDescriptor = false;
	_leftPropChannel.reset(0, kScene4030PropFrameMillis);

	_rightPropLayer.configure(8, kScene4030RightPropDescriptorCount,
		kScene4030RightPropFrameRemap, ARRAYSIZE(kScene4030RightPropFrameRemap));
	_rightPropLayer.visible = true;
	_rightPropLayer.setFrame(0);
	_rightPropLayer.hasPreviousDescriptor = false;
	_rightPropChannel.reset(0, kScene4030PropFrameMillis);
	_rightPropState = 0;
}

void Scene4030::advanceBackgroundAnimations(uint32 delta) {
	const uint leftFrameCount = _leftPropChannel.consumeFrames(delta);
	if (leftFrameCount != 0)
		advanceLeftPropLayer(leftFrameCount);

	const uint rightFrameCount = _rightPropChannel.consumeFrames(delta);
	if (rightFrameCount != 0)
		advanceRightPropLayer(rightFrameCount);
}

void Scene4030::advanceLeftPropLayer(uint frameCount) {
	for (uint i = 0; i < frameCount; ++i) {
		const byte nextFrame = _leftPropLayer.frameIndex == 0x19 ? 0 : _leftPropLayer.frameIndex + 1;
		_leftPropLayer.setFrame(nextFrame);
	}
	_leftPropChannel.frameIndex = _leftPropLayer.frameIndex;
}

void Scene4030::advanceRightPropLayer(uint frameCount) {
	for (uint i = 0; i < frameCount; ++i) {
		switch (_rightPropState) {
		case 0:
			if (_rightPropLayer.frameIndex < 10)
				_rightPropLayer.setFrame(_rightPropLayer.frameIndex + 1);
			else
				_rightPropState = 1;
			break;
		case 1:
			if (_random.getRandomNumber(0x18) == 0) {
				_rightPropState = 2;
			} else if (_random.getRandomNumber(0x27) == 0) {
				_rightPropLayer.setFrame(0x0d);
				_rightPropState = 3;
			}
			break;
		case 2:
			if (_rightPropLayer.frameIndex < 0x0c) {
				_rightPropLayer.setFrame(_rightPropLayer.frameIndex + 1);
			} else {
				_rightPropLayer.setFrame(10);
				_rightPropState = 1;
			}
			break;
		case 3:
			if (_rightPropLayer.frameIndex < 0x1a)
				_rightPropLayer.setFrame(_rightPropLayer.frameIndex + 1);
			else
				_rightPropState = 4;
			break;
		case 4:
			if (_random.getRandomNumber(0x27) == 0) {
				_rightPropLayer.setFrame(0);
				_rightPropState = 0;
			}
			break;
		default:
			_rightPropLayer.setFrame(0);
			_rightPropState = 0;
			break;
		}
	}
	_rightPropChannel.frameIndex = _rightPropLayer.frameIndex;
}

void Scene4030::runTowerTransitionToScene4040() {
	Common::Array<byte> clipData;
	if (!loadVariableChunk(kScene4030TowerTransitionClipChunk, clipData)) {
		_vm->gameState().mainFlowStateId = kScene4040FirstState;
		return;
	}

	Graphics::ManagedSurface transitionBackground;
	transitionBackground.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	transitionBackground.copyRectToSurface(_baseFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	for (uint frameIndex = 0; frameIndex <= kScene4030TowerTransitionFinalFrameIndex &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frameIndex) {
		if (frameIndex != 0) {
			advanceBackgroundAnimations(kScene4030FrameMillis);
			updateAmbientAudioAndMusicCues(kScene4030FrameMillis);
		}

		drawTowerTransitionFrame(clipData, towerTransitionProgressForOutboundFrame((byte)frameIndex),
			transitionBackground);
		presentFrame();

		uint32 remaining = kScene4030FrameMillis;
		while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
			if (pollEvents(true)) {
				remaining = 0;
				frameIndex = kScene4030TowerTransitionFinalFrameIndex;
				break;
			}

			const uint32 slice = MIN<uint32>(remaining, 10);
			g_system->delayMillis(slice);
			remaining -= slice;
		}
	}

	_vm->gameState().mainFlowStateId = kScene4040FirstState;
}

void Scene4030::drawTowerTransitionFrame(const Common::Array<byte> &clipData, byte progressIndex,
		Graphics::ManagedSurface &transitionBackground) {
	transitionBackground.copyRectToSurface(_baseFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	byte previousResourceFrame = 0xff;
	for (uint progress = 0; progress <= progressIndex; ++progress) {
		const byte resourceFrame = towerTransitionResourceFrameForProgress((byte)progress);
		if (resourceFrame == previousResourceFrame)
			continue;

		drawClipFrameDeltaToSurface(clipData, kScene4030TowerTransitionClipDescriptorCount,
			resourceFrame, transitionBackground);
		previousResourceFrame = resourceFrame;
	}

	_sceneFramebuffer.copyRectToSurface(transitionBackground.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	drawResourceSpriteLayer(_leftPropLayer);
	drawResourceSpriteLayer(_rightPropLayer);
}

void Scene4030::drawClipFrameDeltaToSurface(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::ManagedSurface &destination) {
	ResourceDeltaClipPlayer::drawFrame(clipData, 0, clipData.size(), tableEntryCount,
		frameIndex, framebufferPixels(destination), destination.w, destination.h,
		destination.pitch, destination.pitch * destination.h);
}

void Scene4030::takeRope() {
	GameplayState &state = _vm->gameState();
	if (state.scene4030RopeTaken) {
		beginSecondarySpeechLine(17, 0);
		return;
	}

	state.scene4030RopeTaken = true;
	runActorReplacement(ActionOverlaySpec(kScene4030RopePickupChunk, kScene4030RopePickupDescriptorCount,
		kScene4030RopePickupFrameMap, ARRAYSIZE(kScene4030RopePickupFrameMap), kScene4030FrameMillis)
		.patchAt(4, 4));
	addInventoryItem(kScene4030RopeItem);
	_soundBank0.playSample(1, 100);
}

void Scene4030::talkToSkeleton() {
	GameplayState &state = _vm->gameState();
	if (state.scene4030LooseBoneState == 0) {
		beginSecondarySpeechLine(20, 0);
		state.scene4030LooseBoneState = 1;
		runActorReplacement(ActionOverlaySpec(kScene4030BoneRevealChunk, kScene4030BoneRevealDescriptorCount,
			kScene4030BoneRevealFrameMap, ARRAYSIZE(kScene4030BoneRevealFrameMap), kScene4030FrameMillis)
			.patchAt(9, 3)
			.soundAt(2, 0x3a, 25));
		applySceneStateToHotspotsAndPatches(3);
		beginSecondarySpeechLine(20, 1);
		return;
	}

	beginSecondarySpeechLine(15, 0);
}

void Scene4030::takeBone() {
	GameplayState &state = _vm->gameState();
	if (state.scene4030LooseBoneState != 1) {
		beginSecondarySpeechLine(21, 0);
		return;
	}

	state.scene4030LooseBoneState = 2;
	runActorReplacement(ActionOverlaySpec(kScene4030BonePickupChunk, kScene4030BonePickupDescriptorCount,
		kScene4030BonePickupFrameMap, ARRAYSIZE(kScene4030BonePickupFrameMap), kScene4030FrameMillis)
		.patchAt(7, 3));
	addInventoryItem(kScene4030BoneItem);
	_soundBank0.playSample(1, 100);
}

void Scene4030::installImprovisedLever() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene4030LeverItem)) {
		beginSecondarySpeechLine(18, state.scene4030ImprovisedLeverInstalled ? 1 : 0);
		return;
	}
	if (state.scene4030ImprovisedLeverInstalled) {
		beginSecondarySpeechLine(22, 0);
		return;
	}

	state.scene4030ImprovisedLeverInstalled = true;
	runActorReplacement(ActionOverlaySpec(kScene4030LeverInstallChunk, kScene4030LeverInstallDescriptorCount,
		kScene4030LeverInstallFrameMap, ARRAYSIZE(kScene4030LeverInstallFrameMap), kScene4030FrameMillis)
		.patchAt(9, 2));
	removeInventoryItem(kScene4030LeverItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(22, 0);
}

void Scene4030::updateIronMaidenMechanism() {
	if (!_vm->gameState().scene4030ImprovisedLeverInstalled) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	beginSecondarySpeechLine(19, 0);
}

void Scene4030::removeColorMapItem(byte itemId) {
	replaceColorMapItem(itemId, 0);
}

void Scene4030::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

} // End of namespace Hollywood
