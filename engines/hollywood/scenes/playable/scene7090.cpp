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

#include "hollywood/scenes/playable/scene7090.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene7090BackToG07State = 0x1ba0;
const uint16 kScene7090ViewportXOffset = 0x68;
const int kScene7090EntryX = 0x1dd;
const int kScene7090EntryY = 0x101;
const byte kScene7090EntryFacing = 2;
const byte kScene7090IntroTurnFacing = 4;
const uint16 kScene7090Chunk9DescriptorCount = 4;
const uint16 kScene7090Chunk10DescriptorCount = 0x16;
const uint32 kScene7090FrameMillis = 75;
const uint kScene7090Item08VerbRecordIndex = 0x45;
const uint kScene7090ActorPaletteOffset = 0x270;
const uint kScene7090ActorPaletteColorCount = 0x19;
const int kScene7090ForegroundYThreshold = 0x132;
const int kScene7090GatedActionTargetX = 0x2ce;
const int kScene7090GatedActionTargetY = 0x11b;
const byte kScene7090GatedActionTargetFacing = 5;
const int kScene7090GatedActionReturnX = 0x281;
const int kScene7090GatedActionReturnY = 0x10d;
const byte kScene7090GatedActionHook = 1;
const byte kScene7090BackToG07FrameMap[] = {
	0, 1, 2, 3
};
const byte kScene7090GatedActionFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

static PlayableSceneConfig scene7090Config() {
	PlayableSceneConfig config(7090,
		SceneResourceLayout(11, 5, 10),
		SceneViewport(kScene7090ViewportXOffset),
		SceneActorPose(kScene7090EntryX, kScene7090EntryY, kScene7090EntryFacing));
	return config;
}

Scene7090::Scene7090(HollywoodEngine *vm) :
		PlayableScene(vm, scene7090Config()),
		_prePatchChunk7Visible(false) {
}

void Scene7090::initializeCustomPreviewState() {
	_actionOverlayPlayer.reset();
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	_prePatchChunk7Visible = false;
	setActiveActorPose(kScene7090EntryX, kScene7090EntryY, kScene7090EntryFacing);
	_secondaryActorFrame = 0;
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
	darkenActorPaletteRange();
}

void Scene7090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();

	if (_prePatchChunk7Visible)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawActionOverlayLayer();

	if (activeWorldY < kScene7090ForegroundYThreshold)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

void Scene7090::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	setActiveActorPose(kScene7090EntryX, kScene7090EntryY, kScene7090EntryFacing);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (!state.seenHannoverBedroomIntro) {
		walkActiveActorTo(kScene7090EntryX, kScene7090EntryY, kScene7090IntroTurnFacing, 0);
		const byte pathFacing = _activeActorFacing;
		const byte pathCel = _activeActorCel;
		_activeActorFacing = kScene7090EntryFacing;
		_activeActorCel = 0;
		beginSecondarySpeechLine(0, 0);
		_activeActorFacing = pathFacing;
		_activeActorCel = pathCel;
		state.seenHannoverBedroomIntro = true;
	}
}

bool Scene7090::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene7090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Usar/Abrir puerta (use/open door)
		handleBackToG07();
		return true;
	case 302: // Mirar puerta (look at door)
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar ventana (look at window)
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar cama (look at bed)
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Usar cama (use bed)
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar mesa (look at table)
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar archivadores (look at filing cabinets)
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar botella y copas de champagne (look at champagne bottle and glasses)
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar papelera (look at wastebasket)
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Mirar armadura (look at armor)
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Usar armadura (use armor)
		handleGatedAction();
		return true;
	default:
		return false;
	}
}

bool Scene7090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		restoreBaseFramebufferFromOriginal();

		GameplayState &state = _vm->gameState();
		if (state.movedBedroomArmor) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		}

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
				_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
			for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
				if (state.movedBedroomArmor) {
					if (originalItem == 9 || originalItem == 0x0b)
						_paletteMask[kSceneColorToItemMap + i] = 8;
					else if (originalItem == 0x0a)
						_paletteMask[kSceneColorToItemMap + i] = 2;
					else if (originalItem == 0x0c)
						_paletteMask[kSceneColorToItemMap + i] = 0;
				} else {
					if (originalItem == 9)
						_paletteMask[kSceneColorToItemMap + i] = 2;
					else if (originalItem == 0x0a || originalItem == 0x0c)
						_paletteMask[kSceneColorToItemMap + i] = 8;
					else if (originalItem == 0x0b)
						_paletteMask[kSceneColorToItemMap + i] = 0;
				}
			}
		}

		const uint interactionOffset = kSceneItemInteractionPoints + 8 * sizeof(ScenePoint);
		if (_metadata.size() >= interactionOffset + sizeof(ScenePoint)) {
			const uint16 x = state.movedBedroomArmor ? 0x1fc : 0x245;
			const uint16 y = state.movedBedroomArmor ? 0x110 : 0x11f;
			_metadata[interactionOffset] = x & 0xff;
			_metadata[interactionOffset + 1] = x >> 8;
			_metadata[interactionOffset + 2] = y & 0xff;
			_metadata[interactionOffset + 3] = y >> 8;
		}

		const uint approachOffset = kSceneItemApproachPoints + 8 * sizeof(ScenePoint);
		if (_metadata.size() >= approachOffset + sizeof(ScenePoint)) {
			const uint16 x = state.movedBedroomArmor ? 0x23a : 0x288;
			const uint16 y = state.movedBedroomArmor ? 0x096 : 0x0a8;
			_metadata[approachOffset] = x & 0xff;
			_metadata[approachOffset + 1] = x >> 8;
			_metadata[approachOffset + 2] = y & 0xff;
			_metadata[approachOffset + 3] = y >> 8;
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		if (state.movedBedroomArmor)
			_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene7090Item08VerbRecordIndex, 0);
	}
	return true;
}

void Scene7090::darkenActorPaletteRange() {
	const uint byteCount = kScene7090ActorPaletteColorCount * 3;
	if (_actorPaletteBase.size() < byteCount ||
			_paletteCurrent.size() < kScene7090ActorPaletteOffset + byteCount)
		return;

	for (uint color = 0; color < kScene7090ActorPaletteColorCount; ++color) {
		const uint offset = color * 3;
		for (uint component = 1; component < 3; ++component) {
			const uint paletteOffset = offset + component;
			_actorPaletteBase[paletteOffset] -= _actorPaletteBase[paletteOffset] >> 2;
		}
	}
	memcpy(_paletteCurrent.data() + kScene7090ActorPaletteOffset,
		_actorPaletteBase.data(), byteCount);
}

void Scene7090::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 3)
			_walkablePaletteMask[i] = 0;
	}
}

AmbientAudioProfile Scene7090::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(50);
}

void Scene7090::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis) {
	runActorReplacement(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis);
}

void Scene7090::handleBackToG07() {
	runOverlaySequence(9, kScene7090Chunk9DescriptorCount,
		kScene7090BackToG07FrameMap, ARRAYSIZE(kScene7090BackToG07FrameMap),
		kScene7090FrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene7090BackToG07State;
}

void Scene7090::handleGatedAction() {
	GameplayState &state = _vm->gameState();
	if (state.movedBedroomArmor) {
		beginSecondarySpeechLine(10, 3);
		return;
	}

	// Original scene 7090 checks G01 state flag 6 here. That flag is set by
	// the one-time Hannover courtyard follow-up before the armor can be moved.
	if (!state.hannoverCourtyardFollowUpSeen) {
		beginSecondarySpeechLine(10, 0);
		return;
	}

	beginSecondarySpeechLine(10, 1);
	walkActiveActorTo(kScene7090GatedActionTargetX, kScene7090GatedActionTargetY,
		kScene7090GatedActionTargetFacing, 0);

	_prePatchChunk7Visible = true;
	runActorReplacement(ActionOverlaySpec(10, kScene7090Chunk10DescriptorCount,
		kScene7090GatedActionFrameMap, ARRAYSIZE(kScene7090GatedActionFrameMap), kScene7090FrameMillis)
		.hookEveryFrame(kScene7090GatedActionHook)
		.noRedrawAtEnd());
	_prePatchChunk7Visible = false;

	state.movedBedroomArmor = true;
	state.hannoverCourtyardDialogueState = 2;
	applySceneStateToHotspotsAndPatches(1);
	walkActiveActorTo(kScene7090GatedActionReturnX, kScene7090GatedActionReturnY,
		kScene7090GatedActionTargetFacing, 0);
	beginSecondarySpeechLine(10, 2);
}

void Scene7090::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene7090GatedActionHook) {
		if (frame == 3)
			_soundBank0.playSample(0x1b, 100);
		else if (frame == 0x12)
			_soundBank0.stop();
	}
}

} // End of namespace Hollywood
