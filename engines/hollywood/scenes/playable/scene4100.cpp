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

#include "hollywood/scenes/playable/scene4100.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene4070ReturnState = 0x0fe6;
const uint16 kScene4080ReturnState = 0x0ff0;
const uint16 kScene4090ReturnState = 0x0ffa;
const uint16 kScene4110FirstState = 0x100e;
const uint16 kScene4100ViewportXOffset = 0x0040;
const uint16 kScene4100ViewportMinXOffset = 0x0040;
const uint16 kScene4100ViewportMaxXOffset = 0x0070;
const uint kScene4100ActorBankTableEntry = 0x0070;
const uint kScene4100ActorPaletteTableEntry = 0x00cc;
const uint kScene4100Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4100FrameMillis = 75;
const uint32 kScene4100PaletteCycleMillis = 300;
const byte kScene4100PaletteCycleFirstColor = 0xa8;
const byte kScene4100PaletteCycleLastColor = 0xaf;
const uint kScene4100DoorOverlayChunk = 6;
const uint kScene4100DoorOverlayDescriptorCount = 3;
const uint kScene4100CorridorOverlayChunk = 7;
const uint kScene4100CorridorOverlayDescriptorCount = 3;

const byte kScene4100DoorFrameMap[] = {
	0, 0, 1, 2
};

static PlayableSceneConfig scene4100Config() {
	PlayableSceneConfig config(4100,
		SceneResourceLayout(5, 5, 7),
		SceneViewport(kScene4100ViewportXOffset, kScene4100ViewportMinXOffset, kScene4100ViewportMaxXOffset),
		SceneActorPose(0x017e, 0x0138, 2));
	config.setActorResources(kScene4100ActorBankTableEntry, kScene4100ActorPaletteTableEntry);
	config.setTextResources(kScene4100Resource003RowsOffsetIndex, kScene4100SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene4100::Scene4100(HollywoodEngine *vm) :
		PlayableScene(vm, scene4100Config()) {
}

void Scene4100::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applyD10PaletteDimming();
	resetPaletteCycle();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	switch (stateId) {
	case 0x1005:
		_activeActorWorldX = 0x0093;
		_activeActorWorldY = 0x00a4;
		_activeActorFacing = 2;
		break;
	case 0x1006:
		_activeActorWorldX = 0x0166;
		_activeActorWorldY = 0x0075;
		_activeActorFacing = 3;
		break;
	case 0x1007:
		_activeActorWorldX = 0x01f1;
		_activeActorWorldY = 0x0077;
		_activeActorFacing = 3;
		break;
	case 0x1008:
		_activeActorWorldX = 0x0294;
		_activeActorWorldY = 0x008c;
		_activeActorFacing = 3;
		break;
	default:
		_activeActorWorldX = 0x011e;
		_activeActorWorldY = 0x00f5;
		_activeActorFacing = 2;
		break;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene4100::runCustomEntrySequence() {
	applyD10PaletteDimming();
	resetPaletteCycle();
	applySceneStateToHotspotsAndPatches(0xff);

	GameplayState &state = _vm->gameState();
	switch (state.mainFlowStateId) {
	case 0x1004:
		runEntryPath(0x011e, 0x00f5, 2, 0x017e, 0x0138);
		break;
	case 0x1005:
		_soundBank0.playSample(5, 100);
		setActiveActorPose(0x0093, 0x00a4, 2);
		drawPlayableComposite();
		presentFrame();
		if (!state.scene4100EntryLineSeen) {
			beginSecondarySpeechLine(1, 0);
			state.scene4100EntryLineSeen = true;
		}
		break;
	case 0x1006:
		_soundBank0.playSample(5, 100);
		setActiveActorPose(0x0166, 0x0075, 3);
		drawPlayableComposite();
		presentFrame();
		break;
	case 0x1007:
		_soundBank0.playSample(5, 100);
		setActiveActorPose(0x01f1, 0x0077, 3);
		drawPlayableComposite();
		presentFrame();
		break;
	case 0x1008:
		runEntryPath(0x0294, 0x008c, 3, 0x0238, 0x0074);
		break;
	default:
		break;
	}
}

bool Scene4100::prepareCustomGameplayLoop() {
	applyD10PaletteDimming();
	resetPaletteCycle();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene4100::advanceCustomGameplayLoop(uint32 delta) {
	advancePaletteCycle(delta);
	return false;
}

bool Scene4100::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Empty callback slot from the original D10 table.
	case 303: // Empty callback slot from the original D10 table.
	case 305: // Empty callback slot from the original D10 table.
		return true;
	case 302: // Ir a puerta hacia el pasillo con estatua (go to door toward statue corridor).
		runDoorTransition(kScene4100DoorOverlayChunk, kScene4100DoorOverlayDescriptorCount, kScene4110FirstState);
		return true;
	case 304: // Ir a puerta hacia sala del organo (go to organ room door).
		runDoorTransition(kScene4100DoorOverlayChunk, kScene4100DoorOverlayDescriptorCount, kScene4080ReturnState);
		return true;
	case 306: // Ir a puerta hacia habitacion del ataud (go to coffin room door).
		runDoorTransition(kScene4100CorridorOverlayChunk, kScene4100CorridorOverlayDescriptorCount, kScene4090ReturnState);
		return true;
	case 307: // Mirar pasillo (look at corridor).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 308: // Mirar puerta/salida (look at door/exit).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 309: // Ir a pasillo con estatua (go to statue corridor): direct state change.
		_vm->gameState().mainFlowStateId = kScene4110FirstState;
		return true;
	case 310: // Mirar pared del pasillo (look at corridor wall).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 311: // Ir a salon de Dracula (go to Dracula room).
		_vm->gameState().mainFlowStateId = kScene4070ReturnState;
		return true;
	case 312: // Mirar objeto decorativo del pasillo (look at corridor decoration).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 313: // Usar objeto decorativo del pasillo (use corridor decoration).
		beginSecondarySpeechLine(6, 0);
		return true;
	default:
		return false;
	}
}

bool Scene4100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	rebuildWalkablePaletteMask();

	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if ((_walkablePaletteMask[i] != 0 && _walkablePaletteMask[i] < 7) ||
				_walkablePaletteMask[i] == 0x14)
			_walkablePaletteMask[i] = 0;
	}

	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene4100::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0f, 8, 4, 25, 0x0b, 5, 100, 50);
}

void Scene4100::applyD10PaletteDimming() {
	if (_paletteResource.size() < 0xa8 * 3)
		return;

	for (byte color = 1; color < 0xa8; ++color) {
		const uint offset = color * 3;
		const byte red = _paletteResource[offset] > 4 ? _paletteResource[offset] - 4 : 0;
		const byte green = _paletteResource[offset + 1] > 4 ? _paletteResource[offset + 1] - 4 : 0;
		const byte blue = _paletteResource[offset + 2] > 4 ? _paletteResource[offset + 2] - 4 : 0;
		setPaletteEntry6Bit(color, red, green, blue);
	}
}

void Scene4100::resetPaletteCycle() {
	_paletteCycleChannel.reset(0, kScene4100PaletteCycleMillis);
}

void Scene4100::advancePaletteCycle(uint32 delta) {
	const uint frameCount = _paletteCycleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		rotatePaletteCycle();
}

void Scene4100::rotatePaletteCycle() {
	const uint lastOffset = kScene4100PaletteCycleLastColor * 3;
	if (_paletteCurrent.size() <= lastOffset + 2)
		return;

	byte saved[3];
	memcpy(saved, &_paletteCurrent[lastOffset], sizeof(saved));
	for (uint color = kScene4100PaletteCycleLastColor; color > kScene4100PaletteCycleFirstColor; --color)
		memcpy(&_paletteCurrent[color * 3], &_paletteCurrent[(color - 1) * 3], sizeof(saved));
	memcpy(&_paletteCurrent[kScene4100PaletteCycleFirstColor * 3], saved, sizeof(saved));
	invalidatePresentationPalette();
}

void Scene4100::runDoorTransition(uint chunkIndex, uint descriptorCount, uint16 targetState) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		kScene4100DoorFrameMap, ARRAYSIZE(kScene4100DoorFrameMap), kScene4100FrameMillis)
		.frameRange(1, ARRAYSIZE(kScene4100DoorFrameMap)));
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = targetState;
}

} // End of namespace Hollywood
