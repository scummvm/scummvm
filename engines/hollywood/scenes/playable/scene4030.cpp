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

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene4030ArchiveName = "RESOURCE.D03";
const char *const kScene4030MusicArchiveName = "RESOURCE.M04";
const char *const kScene4030SoundArchiveName = "RESOURCE.S04";
const uint kScene4030InitialRequiredChunkCount = 5;
const uint kScene4030ArenaFirstChunk = 7;
const uint kScene4030ArenaLastChunk = 20;
const uint kScene4030StageIndex = 403;
const uint16 kScene4030FirstState = 0x0fbe;
const uint16 kScene4030LastState = 0x0fc7;
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

const byte kScene4030ActorPathStepDeltaTableSet87[] = {
	6, 1, 1, 3, 3, 3, 7, 1, 0, 0, 4, 3,
	3, 2, 8, 6, 6, 7, 6, 4, 10, 3, 2, 9,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	4, 3, 3, 4, 0, 4, 4, 2, 0, 4, 2, 5,
	6, 10, 10, 4, 6, 4, 10, 8, 8, 7, 5, 10,
	6, 4, 10, 3, 2, 9, 3, 2, 8, 6, 6, 7
};

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

PlayableSceneConfig scene4030Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene4030ArchiveName;
	config.initialRequiredChunkCount = kScene4030InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene4030ArenaFirstChunk;
	config.arenaLastChunk = kScene4030ArenaLastChunk;
	config.stageIndex = kScene4030StageIndex;
	config.debugName = "Scene 4030";
	config.viewportXOffset = kScene4030ViewportXOffset;
	config.viewportMinXOffset = kScene4030ViewportMinXOffset;
	config.viewportMaxXOffset = kScene4030ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 4;
	config.actorBankTableEntry = kScene4030ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene4030ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene4030Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene4030SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene4030ActorPathStepDeltaTableSet87;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene4030ActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene4030MusicArchiveName;
	config.soundBank0ArchiveName = kScene4030SoundArchiveName;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene4030FirstState;
	config.mainFlowLastState = kScene4030LastState;
	return config;
}

Scene4030::Scene4030(HollywoodEngine *vm) :
		PlayableScene(vm, scene4030Config(), "scene4030", kScene4030EntryWorldX, kScene4030EntryWorldY,
			kScene4030EntryFacing, 0xfd, 0xfb),
		_leftPropLayer(),
		_rightPropLayer() {
}

bool Scene4030::hasCustomPreviewState() const {
	return true;
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

bool Scene4030::hasCustomComposite() const {
	return true;
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

bool Scene4030::hasCustomEntrySequence() const {
	return true;
}

void Scene4030::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene4030FirstState) {
		_activeActorWorldX = kScene4030EntryWorldX;
		_activeActorWorldY = kScene4030EntryWorldY;
		_activeActorFacing = kScene4030EntryFacing;
		_activeActorCel = 0;
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
		_viewportXOffset = kScene4030EntryViewportXOffset;

		drawPlayableComposite();
		presentFrame();
		runConfiguredActionOverlay(kScene4030EntryOverlayChunk, kScene4030EntryOverlayDescriptorCount,
			kScene4030EntryOverlayFrameMap, ARRAYSIZE(kScene4030EntryOverlayFrameMap),
			kScene4030FrameMillis, kActionOverlayKeepActiveActorVisibility);

		if (!state.seenScene4030EntryLine) {
			_activeActorFacing = kScene4030EntrySpeechFacing;
			_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
			drawPlayableComposite();
			presentFrame();
			beginSecondarySpeechLine(1, 1);
			state.seenScene4030EntryLine = true;
		}
		return;
	}

	drawPlayableComposite();
	presentFrame();
}

bool Scene4030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Transicion automatica a torreon (automatic transition to tower).
		_vm->gameState().mainFlowStateId = kScene4040FirstState;
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
		beginSecondarySpeechLine(18, _vm->gameState().scene4030LeverInstalled ? 1 : 0);
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

	if (state.scene4030BoneState == 1) {
		if (_sceneChunkTable.isValidChunk(10))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
	} else {
		removeColorMapItem(kScene4030BoneSceneItem);
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
	}

	if (state.scene4030LeverInstalled) {
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

	_rightPropLayer.configure(8, kScene4030RightPropDescriptorCount, nullptr, 0);
	_rightPropLayer.visible = true;
	_rightPropLayer.setFrame(0);
	_rightPropLayer.hasPreviousDescriptor = false;
}

void Scene4030::takeRope() {
	GameplayState &state = _vm->gameState();
	if (state.scene4030RopeTaken) {
		beginSecondarySpeechLine(17, 0);
		return;
	}

	state.scene4030RopeTaken = true;
	runConfiguredActionOverlay(kScene4030RopePickupChunk, kScene4030RopePickupDescriptorCount,
		kScene4030RopePickupFrameMap, ARRAYSIZE(kScene4030RopePickupFrameMap),
		kScene4030FrameMillis, kActionOverlayKeepActiveActorVisibility, 4, 4);
	addInventoryItem(kScene4030RopeItem);
	_soundBank0.playSample(1, 100);
}

void Scene4030::talkToSkeleton() {
	GameplayState &state = _vm->gameState();
	if (state.scene4030BoneState == 0) {
		beginSecondarySpeechLine(20, 0);
		state.scene4030BoneState = 1;
		runConfiguredActionOverlay(kScene4030BoneRevealChunk, kScene4030BoneRevealDescriptorCount,
			kScene4030BoneRevealFrameMap, ARRAYSIZE(kScene4030BoneRevealFrameMap),
			kScene4030FrameMillis, kActionOverlayKeepActiveActorVisibility, 9, 3, 2, 0x3a, 25);
		applySceneStateToHotspotsAndPatches(3);
		beginSecondarySpeechLine(20, 1);
		return;
	}

	beginSecondarySpeechLine(15, 0);
}

void Scene4030::takeBone() {
	GameplayState &state = _vm->gameState();
	if (state.scene4030BoneState != 1) {
		beginSecondarySpeechLine(21, 0);
		return;
	}

	state.scene4030BoneState = 2;
	runConfiguredActionOverlay(kScene4030BonePickupChunk, kScene4030BonePickupDescriptorCount,
		kScene4030BonePickupFrameMap, ARRAYSIZE(kScene4030BonePickupFrameMap),
		kScene4030FrameMillis, kActionOverlayKeepActiveActorVisibility, 7, 3);
	addInventoryItem(kScene4030BoneItem);
	_soundBank0.playSample(1, 100);
}

void Scene4030::installImprovisedLever() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene4030LeverItem)) {
		beginSecondarySpeechLine(18, state.scene4030LeverInstalled ? 1 : 0);
		return;
	}
	if (state.scene4030LeverInstalled) {
		beginSecondarySpeechLine(22, 0);
		return;
	}

	state.scene4030LeverInstalled = true;
	runConfiguredActionOverlay(kScene4030LeverInstallChunk, kScene4030LeverInstallDescriptorCount,
		kScene4030LeverInstallFrameMap, ARRAYSIZE(kScene4030LeverInstallFrameMap),
		kScene4030FrameMillis, kActionOverlayKeepActiveActorVisibility, 9, 2);
	removeInventoryItem(kScene4030LeverItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(22, 0);
}

void Scene4030::updateIronMaidenMechanism() {
	if (!_vm->gameState().scene4030LeverInstalled) {
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
