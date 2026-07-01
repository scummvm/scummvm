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

#include "hollywood/scenes/playable/scene6030.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene6030ArchiveName = "RESOURCE.F03";
const char *const kScene6030MusicArchiveName = "RESOURCE.M06";
const char *const kScene6030SoundArchiveName = "RESOURCE.S06";
const uint kScene6030InitialRequiredChunkCount = 12;
const uint kScene6030ArenaFirstChunk = 5;
const uint kScene6030ArenaLastChunk = 11;
const uint kScene6030StageIndex = 603;
const uint16 kScene6030FirstState = 0x178e;
const uint16 kScene6030LastState = 0x1797;
const uint16 kScene6020ReturnFromScene6030State = 0x1785;
const uint16 kScene6030ViewportXOffset = 0x0064;
const uint kScene6030ActorBankTableEntry = 0x0000;
const uint kScene6030ActorPaletteTableEntry = 0x00cc;
const uint kScene6030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6030HannoverFrameMillis = 75;
const uint kScene6030LargeLayerDescriptorCount = 0x1b;
const uint kScene6030SmallLayerDescriptorCount = 3;

const byte kScene6030ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene6030LargeLayerFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
};

const byte kScene6030SmallLayerFrameMap[] = {
	0, 1, 2
};

static PlayableSceneConfig scene6030Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene6030ArchiveName;
	config.initialRequiredChunkCount = kScene6030InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene6030ArenaFirstChunk;
	config.arenaLastChunk = kScene6030ArenaLastChunk;
	config.stageIndex = kScene6030StageIndex;
	config.debugName = "Scene 6030";
	config.viewportXOffset = kScene6030ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 6;
	config.actorBankTableEntry = kScene6030ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene6030ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene6030Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene6030SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene6030ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene6030ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene6030MusicArchiveName;
	config.soundBank0ArchiveName = kScene6030SoundArchiveName;
	config.mainFlowFirstState = kScene6030FirstState;
	config.mainFlowLastState = kScene6030LastState;
	return config;
}

Scene6030::Scene6030(HollywoodEngine *vm) :
		PlayableScene(vm, scene6030Config(), "scene6030", 0x1e5, 0x186, 5, 0xfd, 0xfb),
		_largeBackgroundChannel(),
		_largeBackgroundLayer(),
		_smallForegroundLayer() {
	_largeBackgroundLayer.configure(6, kScene6030LargeLayerDescriptorCount,
		kScene6030LargeLayerFrameMap, ARRAYSIZE(kScene6030LargeLayerFrameMap));
	_smallForegroundLayer.configure(8, kScene6030SmallLayerDescriptorCount,
		kScene6030SmallLayerFrameMap, ARRAYSIZE(kScene6030SmallLayerFrameMap));
}

bool Scene6030::hasCustomPreviewState() const {
	return true;
}

void Scene6030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	_activeActorWorldX = 0x1e5;
	_activeActorWorldY = 0x186;
	_activeActorFacing = 5;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene6030::hasCustomComposite() const {
	return true;
}

void Scene6030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_largeBackgroundLayer);
	drawResourceSpriteLayer(_smallForegroundLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
}

bool Scene6030::hasCustomEntrySequence() const {
	return true;
}

void Scene6030::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x384, 0x1de, 5, 0x1e5, 0x186);
	runEntryConversation();
}

bool Scene6030::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene6030::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceHannoverLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301:
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302:
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303:
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304:
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Original patch-backed raw callback label.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306:
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307:
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Original raw callback label near the scene exit path.
		returnToScene6020();
		return true;
	case 309:
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310:
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311:
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312:
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313:
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314:
		beginSecondarySpeechLine(13, 0);
		return true;
	case 315:
		beginSecondarySpeechLine(14, 0);
		return true;
	case 316:
		return true;
	default:
		return false;
	}
}

bool Scene6030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	rebuildWorkingWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene6030::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene6030::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0;
}

void Scene6030::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_largeBackgroundLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene6030::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6030::rebuildWorkingWalkableMask() {
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 2)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene6030::resetAnimationLayers() {
	_largeBackgroundChannel.reset(0, kScene6030HannoverFrameMillis);
	_largeBackgroundLayer.reset(0);
	_smallForegroundLayer.reset(0);
	_largeBackgroundLayer.visible = true;
	_smallForegroundLayer.visible = true;
}

void Scene6030::advanceHannoverLayer(uint32 delta) {
	const uint frameCount = _largeBackgroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_largeBackgroundLayer.frameIndex == 0) {
			if (_random.getRandomNumber(14) == 0)
				_largeBackgroundLayer.setFrame(4);
		} else {
			_largeBackgroundLayer.setFrame(0);
		}
	}
}

void Scene6030::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x184 && activeWorldX > 0x200 && _sceneChunkTable.isValidChunk(11))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _sceneFramebuffer);
}

void Scene6030::runEntryConversation() {
	beginPrimarySpeechLine(99, 0, 0x173, 0x0a9, 0x28, 0x16, 0x0b);
	beginSecondarySpeechLine(0x62, 0);
	beginPrimarySpeechLine(99, 1, 0x173, 0x0a9, 0x28, 0x16, 0x0b);
	walkActiveActorTo(0x25b, 0x17d, 4, 0, false);
	walkActiveActorTo(0x1e5, 0x186, 5, 0, false);
	beginPrimarySpeechLine(99, 8, 0x173, 0x0a9, 0x28, 0x16, 0x0b);
}

void Scene6030::returnToScene6020() {
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene6020ReturnFromScene6030State;
}

} // End of namespace Hollywood
