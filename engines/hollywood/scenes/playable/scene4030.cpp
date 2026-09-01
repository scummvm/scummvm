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

#include "graphics/managed_surface.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const uint16 kScene4030FirstState = 0x0fbe;
const uint16 kScene4030ReturnFromTowerState = 0x0fbf;
const uint16 kScene4020ReturnState = 0x0fb5;
const uint16 kScene4010DemoReturnState = 0x0fac;
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
const uint kScene4030BoneRevealSecondaryChunk = 13;
const uint kScene4030BoneRevealSecondaryDescriptorCount = 0x0d;
const uint kScene4030BonePickupChunk = 14;
const uint kScene4030BonePickupDescriptorCount = 0x0d;
const uint kScene4030LeverInstallChunk = 19;
const uint kScene4030LeverInstallDescriptorCount = 0x0c;
const uint kScene4030MechanismSecondaryChunk = 15;
const uint kScene4030MechanismSecondaryDescriptorCount = 6;
const uint kScene4030TowerExitClipChunk = 5;
const uint kScene4030TowerExitClipDescriptorCount = 0xb3;
const byte kScene4030TowerExitFinalFrameIndex = 0xda;
const uint kScene4030TowerReturnClipChunk = 6;
const uint kScene4030TowerReturnClipDescriptorCount = 0xb1;
const byte kScene4030TowerReturnFinalFrameIndex = 0xd8;
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
const uint32 kScene4030MarkerFrameMillis = 125;
const byte kScene4030MarkerColor = 0xb3;

enum {
	kScene4030LeftPropLayer,
	kScene4030RightPropLayer,
	kScene4030SecondaryActionLayer,
	kScene4030PrimaryActionLayer
};

struct Scene4030MarkerPoint {
	uint16 x;
	uint16 y;
};

const Scene4030MarkerPoint kScene4030MarkerPoints[][2] = {
	{ { 0x15a, 0x05f }, { 0x15d, 0x05f } },
	{ { 0x000, 0x000 }, { 0x000, 0x000 } },
	{ { 0x2bc, 0x0d0 }, { 0x2c0, 0x0d0 } },
	{ { 0x2d2, 0x02f }, { 0x2d5, 0x02f } },
	{ { 0x2fe, 0x078 }, { 0x302, 0x078 } },
	{ { 0x307, 0x0ce }, { 0x30b, 0x0ce } },
	{ { 0x366, 0x067 }, { 0x36a, 0x067 } }
};

const byte kScene4030RopePickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13
};

const byte kScene4030BoneRevealFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 10, 9, 10, 11, 10, 9, 10,
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
	1, 0
};

const byte kScene4030BoneRevealSecondaryFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12
};

const byte kScene4030BonePickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12
};

const byte kScene4030LeverInstallFrameMap[] = {
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
	2, 2, 1, 0, 11
};

const byte kScene4030EntryOverlayFrameMap[] = {
	7, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kScene4030StairExitFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7
};

const byte kScene4030MechanismPrimaryFrameMap[] = {
	0, 1, 2, 3, 4, 3, 2, 3, 4, 3, 2, 1, 0, 11
};

const byte kScene4030MechanismSecondaryFrameGroups[][3] = {
	{ 4, 3, 0 },
	{ 2, 1, 0 },
	{ 0, 3, 4 },
	{ 0, 1, 2 }
};

const byte kScene4030RightPropFrameRemap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	9, 10, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23
};

const SceneLayerSpec kScene4030LayerSpecs[] = {
	{ kSceneAnimationBehindActors, 7, kScene4030LeftPropDescriptorCount,
		nullptr, 0, true, 0 },
	{ kSceneAnimationInFrontOfActors, 8, kScene4030RightPropDescriptorCount,
		kScene4030RightPropFrameRemap, ARRAYSIZE(kScene4030RightPropFrameRemap), true, 0 },
	{ kSceneAnimationInFrontOfActors, 0, 0, nullptr, 0, false, 0 },
	{ kSceneAnimationInFrontOfActors, 0, 0, nullptr, 0, false, 0 }
};

const byte kScene4030TowerExitFootstepFrames[] = {
	3, 9, 15, 21, 27, 31, 37, 43, 49, 55,
	61, 67, 70, 76, 82, 88, 94, 100, 106, 112,
	118, 124, 130, 136, 142, 148, 156, 162, 168, 174,
	180, 186, 192, 198, 204, 210, 216
};

const byte kScene4030TowerReturnFootstepFrames[] = {
	2, 8, 14, 20, 26, 32, 38, 44, 50, 56,
	62, 68, 74, 80, 86, 94, 101, 107, 114, 120,
	125, 131, 137, 143, 149, 155, 161, 167, 173, 179,
	185, 193, 199, 205, 211
};

byte towerTransitionResourceFrameForProgress(byte progressIndex, bool returningFromTower) {
	const byte holdFrame = returningFromTower ? 0x41 : 0x70;
	const byte holdEnd = returningFromTower ? 0x69 : 0x98;
	if (progressIndex <= holdFrame)
		return progressIndex;
	if (progressIndex <= holdEnd)
		return holdFrame;
	return progressIndex - 0x28;
}

template<uint size>
bool containsFrame(const byte (&frames)[size], byte frame) {
	for (uint i = 0; i < size; ++i) {
		if (frames[i] == frame)
			return true;
	}
	return false;
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
		_leftPropTrack(RealtimeAnimationTracks::kInvalidTrack),
		_rightPropChannel(),
		_markerChannel(),
		_originalStageSmallRows(),
		_rightPropState(0) {
	_sceneLayers.configure(kScene4030LayerSpecs);
	_leftPropTrack = _realtimeAnimationTracks.addLoop(_sceneLayers, kScene4030LeftPropLayer,
		kScene4030PropFrameMillis, kScene4030LeftPropDescriptorCount);
	memset(_markerDark, 0, sizeof(_markerDark));
}

void Scene4030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	initializeSpriteLayers();

	if (_vm->gameState().mainFlowStateId != kScene4030ReturnFromTowerState) {
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
	drawMarkerPixels();
	drawLayerStack(_sceneLayers, kSceneAnimationBehindActors);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawLayerStack(_sceneLayers, kSceneAnimationInFrontOfActors);
	drawActionOverlayLayer();
}

void Scene4030::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene4030FirstState) {
		setActiveActorPose(kScene4030EntryWorldX, kScene4030EntryWorldY, kScene4030EntryFacing);
		_viewportXOffset = kScene4030EntryViewportXOffset;

		const bool previousHideActiveActor = _actionOverlayPlayer.beginActorReplacement(
			kScene4030EntryOverlayChunk, kScene4030EntryOverlayDescriptorCount,
			kScene4030EntryOverlayFrameMap, ARRAYSIZE(kScene4030EntryOverlayFrameMap));
		_actionOverlayPlayer.setFrame(0);
		drawPlayableComposite();
		presentFrame();
		fadePaletteFromBlack();
		_actionOverlayPlayer.finish(previousHideActiveActor);
		runActorReplacement(ActionOverlaySpec(kScene4030EntryOverlayChunk, kScene4030EntryOverlayDescriptorCount,
			kScene4030EntryOverlayFrameMap, ARRAYSIZE(kScene4030EntryOverlayFrameMap), kScene4030FrameMillis)
			.startAt(1));

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

	setActiveActorPose(kScene4030ReturnFromTowerWorldX, kScene4030ReturnFromTowerWorldY,
		kScene4030ReturnFromTowerFacing);
	_viewportXOffset = kScene4030ReturnFromTowerViewportXOffset;
	if (!runTowerTransitionClip(true)) {
		drawPlayableComposite();
		presentFrame();
		fadePaletteFromBlack();
	}
	drawPlayableComposite();
	presentFrame();
}

bool Scene4030::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene4030::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene4030::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

void Scene4030::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackgroundAnimations(delta);
}

bool Scene4030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Transicion automatica a torreon (automatic transition to tower).
		runTowerTransitionToScene4040();
		return true;
	case 302: // Mirar corredor (look at corridor): changes after visiting the tower.
		beginSecondarySpeechLine(0, _vm->gameState().scene4030TowerVisited ? 1 : 0);
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
	case 307: // Abrir dama de hierro (open iron maiden): state-dependent refusal.
		if (_vm->gameState().scene4030IronMaidenOpen)
			beginSharedInventorySpeechLine(2, 0);
		else
			beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Cerrar dama de hierro (close iron maiden): state-dependent refusal.
		if (_vm->gameState().scene4030IronMaidenOpen)
			beginSecondarySpeechLine(6, 0);
		else
			beginSharedInventorySpeechLine(5, 0);
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
		runActorReplacement(ActionOverlaySpec(kScene4030EntryOverlayChunk,
			kScene4030EntryOverlayDescriptorCount, kScene4030StairExitFrameMap,
			ARRAYSIZE(kScene4030StairExitFrameMap), kScene4030FrameMillis));
		_vm->gameState().mainFlowStateId = _vm->isDemo() ?
			kScene4010DemoReturnState : kScene4020ReturnState;
		return true;
	case 314: // Mirar/usar/abrir puerta (look/use/open door): punishment cell warning.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Coger candil (take oil lamp): not movable here.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 316: // Mirar candil (look at oil lamp).
		beginSecondarySpeechLine(15, _vm->gameState().scene4030LooseBoneState != 0 ? 1 : 0);
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
	case 321: // Usar cuerda (use rope): no useful purpose here.
		beginSecondarySpeechLine(17, 0);
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

bool Scene4030::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MIN<int>(targetX, 0x37f);
	if (targetY < 0x1df)
		++targetY;
	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;
	return true;
}

bool Scene4030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	if (_originalStageSmallRows.empty())
		_originalStageSmallRows = _stage003SmallRows;
	else
		_stage003SmallRows = _originalStageSmallRows;

	GameplayState &state = _vm->gameState();
	uint mechanismPatchChunk = 0;
	if (!state.scene4030IronMaidenOpen) {
		removeColorMapItem(kScene4030LeverSceneItem);
		copyStageItemName(10, 12);
		if (state.scene4030RopeTaken) {
			replaceColorMapItem(kScene4030RopeSceneItem, 3);
			mechanismPatchChunk = 18;
		} else {
			mechanismPatchChunk = 17;
		}
	} else {
		replaceColorMapItem(kScene4030LeverSceneItem, 3);
		replaceColorMapItem(kScene4030RopeSceneItem, 3);
		if (state.scene4030ImprovisedLeverInstalled) {
			copyStageItemName(10, 12);
			mechanismPatchChunk = 16;
		}
	}
	if (mechanismPatchChunk != 0 && _sceneChunkTable.isValidChunk(mechanismPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[mechanismPatchChunk], _baseFramebuffer);

	if (state.scene4030LooseBoneState == 1) {
		if (_sceneChunkTable.isValidChunk(10))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
	} else {
		removeColorMapItem(kScene4030BoneSceneItem);
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	_hotspots.setVerbMovementModeByGlobalRecordIndex(0x55,
		!state.scene4030IronMaidenOpen || state.scene4030ImprovisedLeverInstalled ? 1 : 0);
	return true;
}

AmbientAudioProfile Scene4030::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = 0x28;
	profile.soundVolumePercent = 50;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

void Scene4030::initializeSpriteLayers() {
	_sceneLayers.configure(kScene4030LayerSpecs);
	_realtimeAnimationTracks.reset(_leftPropTrack);
	_rightPropChannel.reset(0, kScene4030PropFrameMillis);
	_rightPropState = 0;

	_markerChannel.reset(0, kScene4030MarkerFrameMillis);
	memset(_markerDark, 0, sizeof(_markerDark));
}

void Scene4030::advanceBackgroundAnimations(uint32 delta) {
	advanceMarkerPixels(delta);

	const uint rightFrameCount = _rightPropChannel.consumeFrames(delta);
	if (rightFrameCount != 0)
		advanceRightPropLayer(rightFrameCount);
}

void Scene4030::advanceRightPropLayer(uint frameCount) {
	ResourceSpriteLayer &rightPropLayer = _sceneLayers.layer(kScene4030RightPropLayer);
	for (uint i = 0; i < frameCount; ++i) {
		switch (_rightPropState) {
		case 0:
			if (rightPropLayer.frameIndex < 10)
				rightPropLayer.setFrame(rightPropLayer.frameIndex + 1);
			else
				_rightPropState = 1;
			break;
		case 1:
			if (_random.getRandomNumber(0x18) == 0) {
				_rightPropState = 2;
			} else if (_random.getRandomNumber(0x27) == 0) {
				rightPropLayer.setFrame(0x0d);
				_rightPropState = 3;
			}
			break;
		case 2:
			if (rightPropLayer.frameIndex < 0x0c) {
				rightPropLayer.setFrame(rightPropLayer.frameIndex + 1);
			} else {
				rightPropLayer.setFrame(10);
				_rightPropState = 1;
			}
			break;
		case 3:
			if (rightPropLayer.frameIndex < 0x1a)
				rightPropLayer.setFrame(rightPropLayer.frameIndex + 1);
			else
				_rightPropState = 4;
			break;
		case 4:
			if (_random.getRandomNumber(0x27) == 0) {
				rightPropLayer.setFrame(0);
				_rightPropState = 0;
			}
			break;
		default:
			rightPropLayer.setFrame(0);
			_rightPropState = 0;
			break;
		}
	}
	_rightPropChannel.frameIndex = rightPropLayer.frameIndex;
}

void Scene4030::advanceMarkerPixels(uint32 delta) {
	const uint tickCount = _markerChannel.consumeFrames(delta);
	for (uint tick = 0; tick < tickCount; ++tick) {
		for (uint marker = 0; marker < ARRAYSIZE(_markerDark); ++marker) {
			if (_markerDark[marker])
				_markerDark[marker] = false;
			else if (_random.getRandomNumber(14) == 0)
				_markerDark[marker] = true;
		}
	}
}

void Scene4030::drawMarkerPixels() {
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	for (uint marker = 0; marker < ARRAYSIZE(kScene4030MarkerPoints); ++marker) {
		const byte color = _markerDark[marker] ? 0 : kScene4030MarkerColor;
		for (uint point = 0; point < ARRAYSIZE(kScene4030MarkerPoints[marker]); ++point) {
			const Scene4030MarkerPoint &position = kScene4030MarkerPoints[marker][point];
			if (position.x < (uint)_sceneFramebuffer.w && position.y < (uint)_sceneFramebuffer.h)
				pixels[position.y * _sceneFramebuffer.pitch + position.x] = color;
		}
	}
}

void Scene4030::runTowerTransitionToScene4040() {
	runTowerTransitionClip(false);
	_vm->gameState().scene4030TowerVisited = true;
	_vm->gameState().mainFlowStateId = kScene4040FirstState;
}

bool Scene4030::runTowerTransitionClip(bool returningFromTower) {
	const uint clipChunk = returningFromTower ? kScene4030TowerReturnClipChunk : kScene4030TowerExitClipChunk;
	const uint descriptorCount = returningFromTower ?
		kScene4030TowerReturnClipDescriptorCount : kScene4030TowerExitClipDescriptorCount;
	const byte finalFrame = returningFromTower ?
		kScene4030TowerReturnFinalFrameIndex : kScene4030TowerExitFinalFrameIndex;

	Common::Array<byte> clipData;
	if (!loadVariableChunk(clipChunk, clipData))
		return false;

	Graphics::ManagedSurface transitionBackground;
	transitionBackground.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	transitionBackground.copyRectToSurface(_baseFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	byte previousResourceFrame = 0xff;
	for (uint logicalFrame = 0; logicalFrame <= finalFrame && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++logicalFrame) {
		const byte resourceFrame = towerTransitionResourceFrameForProgress(
			(byte)logicalFrame, returningFromTower);
		const uint firstResourceFrame = previousResourceFrame == 0xff ? 0 : previousResourceFrame + 1;
		for (uint frame = firstResourceFrame; frame <= resourceFrame; ++frame) {
			if (!drawClipFrameDeltaToSurface(clipData, descriptorCount,
					(byte)frame, transitionBackground)) {
				warning("%s failed to decode tower transition chunk %u frame %u",
					sceneDebugName(), clipChunk, frame);
				return false;
			}
		}
		previousResourceFrame = resourceFrame;

		drawTowerTransitionFrame(transitionBackground);
		presentFrame();
		if (logicalFrame == 0 && returningFromTower)
			fadePaletteFromBlack();

		const bool playFootstep = returningFromTower ?
			containsFrame(kScene4030TowerReturnFootstepFrames, (byte)logicalFrame) :
			containsFrame(kScene4030TowerExitFootstepFrames, (byte)logicalFrame);
		if (playFootstep)
			playActiveActorFootstep();

		if (waitDeltaClipFrameMillis(kScene4030FrameMillis))
			break;
		advanceBackgroundAnimations(kScene4030FrameMillis);
		updateAmbientAudioAndMusicCues(kScene4030FrameMillis);
	}

	return !Engine::shouldQuit() && !_vm->isSceneRestartRequested();
}

void Scene4030::drawTowerTransitionFrame(const Graphics::ManagedSurface &transitionBackground) {
	_sceneFramebuffer.copyRectToSurface(transitionBackground.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	drawMarkerPixels();
	drawResourceSpriteLayer(_sceneLayers.layer(kScene4030LeftPropLayer));
	drawResourceSpriteLayer(_sceneLayers.layer(kScene4030RightPropLayer));
}

bool Scene4030::drawClipFrameDeltaToSurface(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::ManagedSurface &destination) {
	return ResourceDeltaClipPlayer::drawFrame(clipData, 0, clipData.size(), tableEntryCount,
		frameIndex, framebufferPixels(destination), destination.w, destination.h,
		destination.pitch, destination.pitch * destination.h);
}

void Scene4030::takeRope() {
	GameplayState &state = _vm->gameState();
	if (state.scene4030RopeTaken) {
		beginSecondarySpeechLine(17, 0);
		return;
	}

	beginSharedInventorySpeechLine(0x14, randomSharedInventorySpeechFrame(4));
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
		playBoneRevealAnimation();
		beginSecondarySpeechLine(20, 1);
		return;
	}

	beginSharedInventorySpeechLine(0x10, randomSharedInventorySpeechFrame(2));
}

void Scene4030::playBoneRevealAnimation() {
	_sceneLayers.setLayerResource(kScene4030PrimaryActionLayer,
		kScene4030BoneRevealChunk, kScene4030BoneRevealDescriptorCount,
		kScene4030BoneRevealFrameMap, ARRAYSIZE(kScene4030BoneRevealFrameMap));
	_sceneLayers.showLayerAtFrame(kScene4030PrimaryActionLayer, 0);
	_sceneLayers.setLayerResource(kScene4030SecondaryActionLayer, kScene4030BoneRevealSecondaryChunk,
		kScene4030BoneRevealSecondaryDescriptorCount, kScene4030BoneRevealSecondaryFrameMap,
		ARRAYSIZE(kScene4030BoneRevealSecondaryFrameMap));
	_sceneLayers.setLayerVisible(kScene4030SecondaryActionLayer, false);

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	bool stateApplied = false;
	for (uint frame = 0; frame <= 34 && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		_sceneLayers.setLayerFrame(kScene4030PrimaryActionLayer, (byte)MIN<uint>(frame,
			ARRAYSIZE(kScene4030BoneRevealFrameMap) - 1));
		if (frame >= 22) {
			const byte secondaryFrame = (byte)MIN<uint>(frame - 21,
				ARRAYSIZE(kScene4030BoneRevealSecondaryFrameMap) - 1);
			_sceneLayers.setLayerVisible(kScene4030SecondaryActionLayer, true);
			_sceneLayers.setLayerFrame(kScene4030SecondaryActionLayer, secondaryFrame);
			if (secondaryFrame == 2)
				_soundBank0.playSample(0x3a, 25);
			if (secondaryFrame == 13) {
				_vm->gameState().scene4030LooseBoneState = 1;
				applySceneStateToHotspotsAndPatches(3);
				stateApplied = true;
			}
		}
		if (waitSceneMillis(kScene4030FrameMillis))
			break;
	}

	if (!stateApplied) {
		_vm->gameState().scene4030LooseBoneState = 1;
		applySceneStateToHotspotsAndPatches(3);
	}
	clearSceneLayer(kScene4030SecondaryActionLayer);
	clearSceneLayer(kScene4030PrimaryActionLayer);
	_hideActiveActor = previousHideActiveActor;
	drawPlayableComposite();
	presentFrame();
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
	beginSharedInventorySpeechLine(0x14, randomSharedInventorySpeechFrame(4));
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

	runActorReplacement(ActionOverlaySpec(kScene4030LeverInstallChunk, kScene4030LeverInstallDescriptorCount,
		kScene4030LeverInstallFrameMap, ARRAYSIZE(kScene4030LeverInstallFrameMap), kScene4030FrameMillis)
		.resourcePatchAt(9, 16));
	state.scene4030ImprovisedLeverInstalled = true;
	applySceneStateToHotspotsAndPatches(2);
	removeInventoryItem(kScene4030LeverItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(22, 0);
}

void Scene4030::updateIronMaidenMechanism() {
	GameplayState &state = _vm->gameState();
	if (!state.scene4030ImprovisedLeverInstalled) {
		beginSharedInventorySpeechLine(2, 0);
		return;
	}

	const byte secondaryFrameGroup = (state.scene4030IronMaidenOpen ? 2 : 0) +
		(state.scene4030RopeTaken ? 1 : 0);
	state.scene4030IronMaidenOpen = !state.scene4030IronMaidenOpen;
	playIronMaidenMechanismAnimation(secondaryFrameGroup);
	if (!state.scene4030MechanismRemarkSeen) {
		beginSecondarySpeechLine(19, 0);
		state.scene4030MechanismRemarkSeen = true;
	}
	applySceneStateToHotspotsAndPatches(1);
}

void Scene4030::playIronMaidenMechanismAnimation(byte secondaryFrameGroup) {
	_sceneLayers.setLayerResource(kScene4030PrimaryActionLayer,
		kScene4030LeverInstallChunk, kScene4030LeverInstallDescriptorCount,
		kScene4030MechanismPrimaryFrameMap, ARRAYSIZE(kScene4030MechanismPrimaryFrameMap));
	_sceneLayers.showLayerAtFrame(kScene4030PrimaryActionLayer, 0);
	_sceneLayers.setLayerResource(kScene4030SecondaryActionLayer, kScene4030MechanismSecondaryChunk,
		kScene4030MechanismSecondaryDescriptorCount,
		kScene4030MechanismSecondaryFrameGroups[secondaryFrameGroup],
		ARRAYSIZE(kScene4030MechanismSecondaryFrameGroups[secondaryFrameGroup]));
	_sceneLayers.setLayerVisible(kScene4030SecondaryActionLayer, false);

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	for (uint frame = 0; frame < ARRAYSIZE(kScene4030MechanismPrimaryFrameMap) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
		_sceneLayers.setLayerFrame(kScene4030PrimaryActionLayer, (byte)frame);
		if (frame >= 6) {
			_sceneLayers.setLayerVisible(kScene4030SecondaryActionLayer, true);
			_sceneLayers.setLayerFrame(kScene4030SecondaryActionLayer,
				(byte)MIN<uint>(frame - 6, 2));
			if (frame == 6)
				_soundBank0.playSample(0x2b, 100);
		}
		if (waitSceneMillis(kScene4030FrameMillis))
			break;
	}

	clearSceneLayer(kScene4030SecondaryActionLayer);
	clearSceneLayer(kScene4030PrimaryActionLayer);
	_hideActiveActor = previousHideActiveActor;
	drawPlayableComposite();
	presentFrame();
}

void Scene4030::copyStageItemName(byte destinationItem, byte sourceItem) {
	const uint destinationOffset = (uint)destinationItem * kStage003SmallRowSize;
	const uint sourceOffset = (uint)sourceItem * kStage003SmallRowSize;
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
