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

#include "hollywood/scenes/playable/scene7100.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene7100DialogueEntryState = 0x1bbd;
const uint16 kScene7100ExitState6072 = 0x17b8;
const uint16 kScene7100ExitState6075 = 0x17bb;
const uint16 kScene7100RescueExitState6074 = 0x17ba;
const uint16 kScene7100ViewportXOffset = 0x0a8;
const int kScene7100EntryX = 0x2e0;
const int kScene7100EntryY = 0x145;
const byte kScene7100EntryFacing = 1;
const int kScene7100DialogueEntryStartX = 0x29b;
const int kScene7100DialogueEntryStartY = 0x139;
const byte kScene7100DialogueEntryFacing = 2;
const uint16 kScene7100Chunk7DescriptorCount = 0x0d;
const uint16 kScene7100Chunk8DescriptorCount = 0x14;
const uint16 kScene7100Chunk12DescriptorCount = 8;
const uint16 kScene7100Chunk13DescriptorCount = 10;
const uint16 kScene7100Chunk14DescriptorCount = 5;
const uint16 kScene7100Chunk15DescriptorCount = 0x0b;
const uint16 kScene7100Chunk16DescriptorCount = 0x0c;
const uint16 kScene7100Chunk18DescriptorCount = 0x13;
const uint16 kScene7100Chunk19DescriptorCount = 0x18;
const uint16 kScene7100Chunk20DescriptorCount = 0x18;
const uint32 kScene7100FrameMillis = 75;
const byte kScene7100PrimarySpeechGroupA = 0;
const byte kScene7100PrimarySpeechGroupB = 1;
const byte kScene7100PrimarySpeechGroupBanter = 2;
const byte kScene7100BanterVolumePercent = 25;
const byte kScene7100RealtimeSpeechRonBanter = 1;
const byte kScene7100RealtimeSpeechSueBanter = 2;
const byte kScene7100RealtimeSpeechRonScripted = 3;
const byte kScene7100RonEscapeResponseFrame = 4;
const byte kScene7100PrimaryFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 6, 5
};
const byte kScene7100PrimaryAltFrameMap[] = {
	5, 6, 7, 8, 9, 4, 3, 2, 1, 0
};
const byte kScene7100Chunk8FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 17, 18, 19
};
const byte kScene7100Chunk7FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
const byte kScene7100PickupItem15FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0
};
const byte kScene7100Handler315FrameMap[] = {
	0, 1, 2, 3, 4, 4, 4, 4, 4
};
const byte kScene7100Item14FrameMap[] = {
	0, 17, 16, 15, 14, 13, 7, 6, 5, 18, 1, 1, 2, 3, 2, 1,
	2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2, 1,
	2, 3, 2, 1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 0
};
const byte kScene7100Item16FirstFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 17, 18, 19, 20, 21, 20, 19, 18, 17, 18, 19, 20, 21, 20,
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
	3, 2, 1, 0
};
const byte kScene7100Chunk8ScriptFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16
};
const byte kScene7100Item16SecondFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 17, 18, 19, 20, 21, 22, 23, 22, 21, 20, 19, 18, 17, 16,
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};
const byte kScene7100Extended337FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 22, 21, 16, 15, 14, 13, 12, 11,
	10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};
const byte kScene7100TransferFrameMap[] = {
	0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0
};
const byte kScene7100TransferRonFrameMap[] = {
	0, 0, 0, 0, 0, 5, 6, 7, 6, 5, 6, 5, 0
};
const byte kScene7100TransferRonFrameHook = 1;
const byte kScene7100FirstTransferableSueItem = 0x14;
const byte kScene7100RonInventoryOwner = 0;
const byte kScene7100SueInventoryOwner = 1;
const byte kScene7100RonItemBySueItem[] = {
	0x61, 0x63, 0x64, 0x03, 0x10, 0x28, 0x2d,
	0x37, 0x41, 0x48, 0x56, 0x5a, 0x60, 0x65
};

struct Scene7100DialogueSeedRecord {
	uint16 index;
	byte enabled;
	byte nextNodeIndex;
	byte transitionMode;
	byte playerTextRowId;
	byte responseFrameIndex;
	byte disableAfterUse;
	byte reserved;
};

const Scene7100DialogueSeedRecord kScene7100RonDialogueSeedRecords[] = {
	{ 0, 1, 0, 0, 4, 4, 1, 0xff },
	{ 1, 1, 0, 3, 1, 1, 1, 0xff },
	{ 2, 1, 0, 3, 2, 2, 1, 0xff },
	{ 3, 1, 0, 3, 3, 3, 1, 0xff },
	{ 4, 1, 0, 3, 5, 5, 1, 0xff },
	{ 5, 1, 0, 0, 6, 6, 1, 0xff }
};

const Scene7100DialogueSeedRecord kScene7100RescueDialogueSeedRecords[] = {
	{ 0, 1, 0, 3, 0, 1, 1, 0xff },
	{ 1, 1, 0, 3, 1, 2, 1, 0xff },
	{ 2, 1, 0, 3, 2, 3, 1, 0xff },
	{ 3, 1, 0, 3, 3, 4, 1, 0xff },
	{ 4, 1, 0, 1, 4, 5, 1, 0xff },
	{ 5, 1, 0, 0, 5, 0xff, 1, 0xff },
	{ 70, 1, 0, 3, 6, 7, 1, 0xff },
	{ 71, 1, 0, 3, 8, 9, 1, 0xff },
	{ 72, 1, 0, 3, 10, 11, 1, 0xff },
	{ 73, 1, 0, 3, 11, 12, 1, 0xff },
	{ 74, 0, 0, 0, 7, 8, 1, 0xff },
	{ 75, 1, 0, 2, 9, 10, 1, 0xff }
};

static PlayableSceneConfig scene7100Config() {
	PlayableSceneConfig config(7100,
		SceneResourceLayout(21, 5, 20),
		SceneViewport(kScene7100ViewportXOffset),
		SceneActorPose(kScene7100EntryX, kScene7100EntryY, kScene7100EntryFacing));
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene7100::Scene7100(HollywoodEngine *vm) :
		PlayableScene(vm, scene7100Config()),
		_primaryTimerAccumulator(0),
		_banterTimerAccumulator(0),
		_environmentTimerAccumulator(0),
		_primaryMode(0),
		_primaryFrame(0),
		_primaryAltFrame(0),
		_environmentState(2),
		_environmentFrame(0),
		_lastBanterFrame(0xff),
		_banterRemarkCount(0),
		_manualPrimaryAnimationActive(false),
		_dialogueMenuActive(false),
		_specialBanterUsed(false) {
}

void Scene7100::initializeCustomPreviewState() {
	_actionOverlayPlayer.reset();
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_primaryTimerAccumulator = 0;
	_banterTimerAccumulator = 0;
	_environmentTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	_primaryMode = _vm->gameState().mainFlowStateId == kScene7100DialogueEntryState ? 1 : 0;
	_primaryFrame = 0;
	_primaryAltFrame = 0;
	_environmentState = 2;
	_environmentFrame = 0;
	_lastBanterFrame = 0xff;
	_banterRemarkCount = 0;
	_manualPrimaryAnimationActive = false;
	_dialogueMenuActive = false;
	_specialBanterUsed = false;

	if (_vm->gameState().mainFlowStateId == kScene7100DialogueEntryState) {
		_activeActorWorldX = kScene7100DialogueEntryStartX;
		_activeActorWorldY = kScene7100DialogueEntryStartY;
		_activeActorFacing = kScene7100DialogueEntryFacing;
	} else {
		_activeActorWorldX = kScene7100EntryX;
		_activeActorWorldY = kScene7100EntryY;
		_activeActorFacing = kScene7100EntryFacing;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7100::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	(void)activeWorldY;

	copyBaseFramebufferToSceneFramebuffer();

	drawPrimaryNpc();
	drawEnvironmentOverlayBeforeActor();
	if (_actionOverlayPlayer.isVisible() &&
			_actionOverlayPlayer.stratum == kSceneAnimationBehindActors) {
		drawActionOverlayLayer();
	}
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_actionOverlayPlayer.isVisible() &&
			(_actionOverlayPlayer.stratum == kSceneAnimationActorReplacement ||
			 _actionOverlayPlayer.stratum == kSceneAnimationScenePlaced)) {
		drawActionOverlayLayer();
	}

	const uint foregroundChunk = activeWorldX < 0x156 ? 5 : 6;
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[foregroundChunk], _sceneFramebuffer);
	drawEnvironmentOverlayAfterForeground();
	if (_actionOverlayPlayer.isVisible() &&
			_actionOverlayPlayer.stratum == kSceneAnimationInFrontOfActors) {
		drawActionOverlayLayer();
	}
}

void Scene7100::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene7100DialogueEntryState) {
		runRescueEntrySequence();
	} else {
		setActiveActorPose(kScene7100EntryX, kScene7100EntryY, kScene7100EntryFacing);
		drawPlayableComposite();
		runCurtainRevealFromBlack();
	}
}

void Scene7100::prepareCustomGameplayLoop() {
	_primaryTimerAccumulator = 0;
	_banterTimerAccumulator = 0;
	_environmentTimerAccumulator = 0;
}

void Scene7100::advanceCustomGameplayLoop(uint32 delta) {
	if (!_primaryDialogueSpeechActive && !_manualPrimaryAnimationActive)
		advancePrimaryIdleFrame(delta);

	advanceAutonomousBanter(delta);
	advanceEnvironmentFrame(delta);
}

bool Scene7100::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta (look at door)
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Abrir/Cerrar puerta (open/close door)
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Hablar con Ron (talk to Ron)
		runRonDialogue();
		return true;
	case 304: // Coger Ron (take Ron)
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Usar póster (use poster, taking it down)
		handlePickupItem15();
		return true;
	case 306: // Coger póster (take poster)
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Coger ratonera (take mousetrap)
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Usar placa (use plate)
		beginSecondarySpeechLine(7, _vm->gameState().cellPlateRatProgress == 0 ? 0 : 1);
		return true;
	case 309: // Coger placa (take plate)
		beginSecondarySpeechLine(8, MIN<byte>(_vm->gameState().cellPlateRatProgress, 2));
		return true;
	case 310: // Coger camastro (take cot)
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Abrir camastro (open cot)
		beginSecondarySpeechLine(10, 0);
		return true;
	case 312: // Coger escalera (take ladder)
		beginSecondarySpeechLine(0x0b, 0);
		return true;
	case 313: // Abrir escalera (open ladder)
		beginSecondarySpeechLine(0x0c, 0);
		return true;
	case 314: // Coger pulsador (take push button)
		beginSecondarySpeechLine(0x0d, 0);
		return true;
	case 315: // Abrir pulsador (open push button)
		handleActionHandler315();
		return true;
	case 318: // Coger gancho (take hook)
		beginSecondarySpeechLine(0x11, 0);
		return true;
	case 319: // Abrir gancho (open hook)
		beginSecondarySpeechLine(0x12, 0);
		return true;
	case 320: // Coger tubería (take pipe)
		beginSecondarySpeechLine(0x13, 0);
		return true;
	case 321: // Usar tablones (use boards)
		beginSecondarySpeechLine(0x14, 0);
		return true;
	case 322: // Coger tablones (take boards)
		beginSecondarySpeechLine(0x15, 0);
		return true;
	case 323: // Usar/Coger trozo de tubería (use/take pipe piece)
		beginSecondarySpeechLine(0x16, 0);
		return true;
	case 324: // Coger almohada (take pillow)
		beginSecondarySpeechLine(0x17, 0);
		return true;
	case 325: // Coger cables (take wires)
		beginSecondarySpeechLine(0x18, 0);
		return true;
	case 326: // Usar placa con ratonera (use plate on mousetrap)
		handlePlateOnMousetrap();
		return true;
	case 327: // Usar bote de pintura con placa (use paint pot on plate)
		handleCaptureRat();
		return true;
	case 328: // Usar navaja-destornillador con placa (use screwdriver on plate)
		handleRemovePlate();
		return true;
	case 329: // Usar placa con cables (use plate on wires)
		beginSecondarySpeechLine(0x1c, 0);
		return true;
	case 330: // Usar objetos inadecuados con puerta (use unsuitable items on door)
		beginSecondarySpeechLine(0x1d, (byte)_random.getRandomNumber(1));
		return true;
	case 331: // Usar lupa en la celda (use magnifying glass in cell)
		beginSecondarySpeechLine(0x1e, 0);
		return true;
	case 332: // Usar navaja con puerta (use multi-tool on door)
		beginSecondarySpeechLine(0x1f, 0);
		return true;
	case 333: // Usar herramientas con Ron (use tools on Ron)
		beginSecondarySpeechLine(0x20, 0);
		return true;
	case 334: // Usar baraja con Ron (use cards on Ron)
		beginSecondarySpeechLine(0x21, 0);
		return true;
	case 335: // Usar objetos que podrían estropearse (use damageable items)
		beginSecondarySpeechLine(0x22, (byte)_random.getRandomNumber(1));
		return true;
	case 336: // Usar pamela/perfume en la celda (use hat/perfume in cell)
		beginSecondarySpeechLine(0x23, 0);
		return true;
	case 337: // Usar rata con gancho/tubería/cables (use rat on hook/pipes/wires)
		beginSecondarySpeechLine(0x24, 0);
		return true;
	case 340: // Dar objeto a Ron (give item to Ron)
		handleInventoryTransferAction();
		return true;
	default:
		return false;
	}
}

bool Scene7100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 2 || selector == 3 || selector == 4 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		restoreBaseFramebufferFromOriginal();

		GameplayState &state = _vm->gameState();
		if (state.cellPlateRatProgress != 0) {
			drawResourceBlockList(_resourceArena,
				_resourceChunkOffsets[state.cellPlateRatProgress == 1 ? 10 : 11],
				_baseFramebuffer);
		}
		if (!state.posterOnCellWall)
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[17], _baseFramebuffer);
		if (state.cellPlateRemoved)
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
				_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
			for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
				if (state.cellPlateRatProgress == 0) {
					if (originalItem == 0x10)
						_paletteMask[kSceneColorToItemMap + i] = 0;
				} else if (originalItem == 4 || originalItem == 0x10) {
					_paletteMask[kSceneColorToItemMap + i] = 5;
				}

				if (!state.posterOnCellWall && originalItem == 3)
					_paletteMask[kSceneColorToItemMap + i] = 0;

				if (state.cellPlateRemoved) {
					if (originalItem == 0x0f)
						_paletteMask[kSceneColorToItemMap + i] = 0x0f;
					else if (originalItem == 5)
						_paletteMask[kSceneColorToItemMap + i] = 0;
				} else if (originalItem == 0x0f) {
					_paletteMask[kSceneColorToItemMap + i] = 5;
				}
			}
		}

		if (state.cellPlateRatProgress != 0) {
			const uint item4Offset = 4 * sizeof(ScenePoint);
			const uint item5Offset = 5 * sizeof(ScenePoint);
			if (_metadata.size() >= kSceneItemInteractionPoints + item5Offset + sizeof(ScenePoint)) {
				memcpy(_metadata.data() + kSceneItemInteractionPoints + item5Offset,
					_metadata.data() + kSceneItemInteractionPoints + item4Offset, sizeof(ScenePoint));
			}
			if (_metadata.size() >= kSceneItemApproachPoints + item5Offset + sizeof(ScenePoint)) {
				memcpy(_metadata.data() + kSceneItemApproachPoints + item5Offset,
					_metadata.data() + kSceneItemApproachPoints + item4Offset, sizeof(ScenePoint));
			}
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}
	return true;
}

byte Scene7100::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0;
}

byte Scene7100::primarySpeechVolumePercent(byte animationGroup) const {
	return animationGroup == kScene7100PrimarySpeechGroupBanter ?
		kScene7100BanterVolumePercent : PlayableScene::primarySpeechVolumePercent(animationGroup);
}

void Scene7100::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene7100PrimarySpeechGroupB) {
		_primaryMode = 1;
		_primaryAltFrame = MIN<byte>(frameIndex, ARRAYSIZE(kScene7100PrimaryAltFrameMap) - 1);
		return;
	}

	_primaryMode = 0;
	_primaryFrame = MIN<byte>(frameIndex, ARRAYSIZE(kScene7100PrimaryFrameMap) - 1);
}

void Scene7100::handleAnimationFrameHook(byte hookId, uint frame) {
	switch (hookId) {
	case kScene7100TransferRonFrameHook:
		_primaryMode = 0;
		_primaryFrame = kScene7100TransferRonFrameMap[
			MIN<uint>(frame, ARRAYSIZE(kScene7100TransferRonFrameMap) - 1)];
		break;
	default:
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		break;
	}
}

void Scene7100::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 6)
			_walkablePaletteMask[i] = 0;
	}
}

AmbientAudioProfile Scene7100::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(50);
}

void Scene7100::advancePrimaryIdleFrame(uint32 delta) {
	_primaryTimerAccumulator += delta;
	while (_primaryTimerAccumulator >= kScene7100FrameMillis) {
		_primaryTimerAccumulator -= kScene7100FrameMillis;
		byte &frame = _primaryMode == 0 ? _primaryFrame : _primaryAltFrame;
		if (frame == 4) {
			frame = 0;
		} else if (_random.getRandomNumber(0x0e) == 0) {
			frame = 4;
		}
	}
}

void Scene7100::advanceAutonomousBanter(uint32 delta) {
	_banterTimerAccumulator += delta;
	if (_banterTimerAccumulator < kScene7100FrameMillis)
		return;

	_banterTimerAccumulator %= kScene7100FrameMillis;
	if (isRealtimeSpeechActive())
		return;

	if (_primaryMode != 0 || _primaryDialogueSpeechActive ||
			_manualPrimaryAnimationActive || _dialogueMenuActive || _actorPathPlaybackActive ||
			_speechOverlay.visible || _primarySpeechOverlay.visible) {
		return;
	}

	if (_random.getRandomNumber(99) == 0)
		startAutonomousBanter();
}

void Scene7100::advanceEnvironmentFrame(uint32 delta) {
	if (!_vm->gameState().cellPipesActive)
		return;

	_environmentTimerAccumulator += delta;
	while (_environmentTimerAccumulator >= kScene7100FrameMillis) {
		_environmentTimerAccumulator -= kScene7100FrameMillis;
		if (_environmentState == 0) {
			if (_environmentFrame == 0x0f) {
				_environmentState = 3;
			} else {
				++_environmentFrame;
			}
		} else if (_environmentState == 1) {
			if (_environmentFrame == 0x0c) {
				_environmentState = 2;
			} else {
				++_environmentFrame;
			}
		} else if (_activeActorWorldX > 0x168 && _random.getRandomNumber(99) == 0) {
			_environmentState -= 2;
			_environmentFrame = 0;
			_soundBank0.playSample(0x18, 25);
		}
	}
}

void Scene7100::drawPrimaryNpc() {
	if (_primaryMode == 0) {
		const byte frame = _primaryFrame < ARRAYSIZE(kScene7100PrimaryFrameMap) ?
			kScene7100PrimaryFrameMap[_primaryFrame] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[12], 0,
			kScene7100Chunk12DescriptorCount, frame, _sceneFramebuffer);
		return;
	}

	const byte frame = _primaryAltFrame < ARRAYSIZE(kScene7100PrimaryAltFrameMap) ?
		kScene7100PrimaryAltFrameMap[_primaryAltFrame] : 5;
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[13], 0,
		kScene7100Chunk13DescriptorCount, frame, _sceneFramebuffer);
}

void Scene7100::drawEnvironmentOverlayBeforeActor() {
	if (_environmentState == 4) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[8], 0,
			kScene7100Chunk8DescriptorCount, 0x10, _sceneFramebuffer);
	}
}

void Scene7100::drawEnvironmentOverlayAfterForeground() {
	if (_environmentState == 0) {
		const byte frame = _environmentFrame < ARRAYSIZE(kScene7100Chunk8FrameMap) ?
			kScene7100Chunk8FrameMap[_environmentFrame] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[8], 0,
			kScene7100Chunk8DescriptorCount, frame, _sceneFramebuffer);
	} else if (_environmentState == 1) {
		const byte frame = _environmentFrame < ARRAYSIZE(kScene7100Chunk7FrameMap) ?
			kScene7100Chunk7FrameMap[_environmentFrame] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[7], 0,
			kScene7100Chunk7DescriptorCount, frame, _sceneFramebuffer);
	}
}

void Scene7100::startAutonomousBanter() {
	byte frameIndex;
	const byte maximumFrame = _specialBanterUsed ? 2 : 3;
	do {
		frameIndex = (byte)_random.getRandomNumber(maximumFrame);
	} while (frameIndex == _lastBanterFrame);

	_lastBanterFrame = frameIndex;
	if (frameIndex == 3)
		_specialBanterUsed = true;

	startRealtimePrimarySpeechLine(0x2a, frameIndex, 0x310, 0x8a,
		0x3f, 0x3f, 0x3f, kScene7100PrimarySpeechGroupBanter,
		kScene7100RealtimeSpeechRonBanter);
}

void Scene7100::startScriptedRonSpeech(uint16 rowIndex, byte frameIndex,
		uint16 centerX, byte red, byte green, byte blue, byte animationGroup) {
	startRealtimePrimarySpeechLine(rowIndex, frameIndex, centerX, 0x8a,
		red, green, blue, animationGroup, kScene7100RealtimeSpeechRonScripted);
}

void Scene7100::realtimeSpeechEnded(byte speechId, bool completed) {
	if (completed && speechId == kScene7100RealtimeSpeechRonBanter &&
			(++_banterRemarkCount >= 4 || _lastBanterFrame == 3)) {
		_banterRemarkCount = 0;
		if (!_actorPathPlaybackActive && !_speechOverlay.visible &&
				_vm->cursor()->isInteractiveMode()) {
			startAutonomousSueReply();
		}
	}
}

void Scene7100::waitForScriptedRonSpeech() {
	while (realtimeSpeechId() == kScene7100RealtimeSpeechRonScripted &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(50))
			break;
	}
	if (realtimeSpeechId() == kScene7100RealtimeSpeechRonScripted)
		stopRealtimeSpeech();
}

void Scene7100::startAutonomousSueReply() {
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	faceSueTowardRon();
	startRealtimeSecondarySpeechLine(0x2a, 4, kScene7100RealtimeSpeechSueBanter);
}

void Scene7100::faceSueTowardRon() {
	const SceneActionTarget target = _hotspots.actionTarget(2);
	byte facing = target.facing;
	if (_activeActorWorldX != target.interactionPoint.x ||
			_activeActorWorldY != target.interactionPoint.y) {
		facing = calculateFacingTowardPoint(_activeActorWorldX, _activeActorWorldY,
			target.approachPoint.x, target.approachPoint.y);
	}
	setActiveActorPose(_activeActorWorldX, _activeActorWorldY, facing);
}

void Scene7100::runRonDialogue() {
	stopRealtimeSpeech();
	Common::Array<DialogueChoiceRecord> records;
	initializeRonDialogueRecords(records);

	beginSecondarySpeechLine(0x62, 0);
	beginPrimarySpeechLineWithAnimationGroup(99, 0, 0x310, 0x8a,
		0x3f, 0x3f, 0x3f, kScene7100PrimarySpeechGroupA);

	_dialogueMenuActive = true;
	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(0x62, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(0x62, 6);
			beginPrimarySpeechLineWithAnimationGroup(99, 6, 0x310, 0x8a,
				0x3f, 0x3f, 0x3f, kScene7100PrimarySpeechGroupA);
			_dialogueMenuActive = false;
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			_dialogueMenuActive = false;
			return;
		}

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(0x62, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			beginPrimarySpeechLineWithAnimationGroup(99, record.responseFrameIndex, 0x310, 0x8a,
				0x3f, 0x3f, 0x3f, kScene7100PrimarySpeechGroupA);
		}
		if (record.disableAfterUse == 1)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			if (record.responseFrameIndex == kScene7100RonEscapeResponseFrame) {
				runCurtainClearToBlack();
				_vm->gameState().mainFlowStateId = kScene7100ExitState6075;
			}
			_dialogueMenuActive = false;
			return;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth != 0 ? (byte)(previousDepth - 1) : 0;
			break;
		case 3:
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
			break;
		default:
			_dialogueMenuActive = false;
			return;
		}
	}
	_dialogueMenuActive = false;
}

void Scene7100::initializeRonDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(10 * 10 * 7);
	for (uint i = 0; i < ARRAYSIZE(kScene7100RonDialogueSeedRecords); ++i) {
		const Scene7100DialogueSeedRecord &seed = kScene7100RonDialogueSeedRecords[i];
		DialogueChoiceRecord &record = records[seed.index];
		record.enabled = seed.enabled;
		record.nextNodeIndex = seed.nextNodeIndex;
		record.transitionMode = seed.transitionMode;
		record.playerTextRowId = seed.playerTextRowId;
		record.responseFrameIndex = seed.responseFrameIndex;
		record.disableAfterUse = seed.disableAfterUse;
		record.reserved = seed.reserved;
	}
}

void Scene7100::runRescueEntrySequence() {
	drawPlayableComposite();
	if (runCurtainRevealFromBlack())
		return;

	_primaryMode = 1;
	startScriptedRonSpeech(0x61, 0, 0x314, 0x30, 0x3f, 0,
		kScene7100PrimarySpeechGroupB);
	runEntryPath(kScene7100DialogueEntryStartX, kScene7100DialogueEntryStartY,
		kScene7100DialogueEntryFacing, kScene7100EntryX, kScene7100EntryY);
	waitForScriptedRonSpeech();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	if (runRescueDialogue()) {
		_manualPrimaryAnimationActive = true;
		for (byte frame = 4; frame <= 9; ++frame) {
			_primaryMode = 1;
			_primaryAltFrame = frame;
			if (waitSceneMillis(kScene7100FrameMillis))
				break;
		}
		_manualPrimaryAnimationActive = false;
		if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
			return;

		runCurtainClearToBlack();
		if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
			return;

		GameplayState &state = _vm->gameState();
		state.scene6070SuePresent = false;
		state.scene6070CellDoorOpen = true;
		state.scene6050GuardPresent = false;
		state.mainFlowStateId = kScene7100RescueExitState6074;
		return;
	}
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_primaryMode = 1;
	startScriptedRonSpeech(0x61, 6, 0x314, 0x30, 0x3f, 0,
		kScene7100PrimarySpeechGroupB);
	walkActiveActorTo(0x19f, 0x184, 4, 0);
	waitForScriptedRonSpeech();
	if (waitSceneMillis(4000, false))
		return;

	_primaryMode = 0;
	_primaryFrame = 0;
	drawPlayableComposite();
	presentFrame();
	const byte failureVariant = _vm->nextScene7100RescueFailureVariant();
	startScriptedRonSpeech(0x28, failureVariant, 0x310, 0x3f, 0x3f, 0x3f,
		kScene7100PrimarySpeechGroupA);
	walkActiveActorTo(0x21c, 0x171, 1, 0);
	waitForScriptedRonSpeech();
	beginSecondarySpeechLine(0x29, failureVariant);
}

bool Scene7100::runRescueDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeRescueDialogueRecords(records);
	const GameplayState &state = _vm->gameState();
	if (state.hasInventoryItem(state.currentInventoryOwnerIndex, 0x22))
		records[74].enabled = 1;

	byte depthIndex = 0;
	byte nodeIndex = 0;
	_dialogueMenuActive = true;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(0x60, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(0x60, 5);
			_dialogueMenuActive = false;
			return false;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size()) {
			_dialogueMenuActive = false;
			return false;
		}

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(0x60, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			beginPrimarySpeechLineWithAnimationGroup(0x61, record.responseFrameIndex,
				0x314, 0x8a, 0x30, 0x3f, 0, kScene7100PrimarySpeechGroupB);
		}
		if (record.disableAfterUse == 1)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			_dialogueMenuActive = false;
			return record.responseFrameIndex == 8;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth != 0 ? (byte)(previousDepth - 1) : 0;
			break;
		case 3:
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
			break;
		default:
			_dialogueMenuActive = false;
			return false;
		}
	}

	_dialogueMenuActive = false;
	return false;
}

void Scene7100::initializeRescueDialogueRecords(
		Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(10 * 10 * 7);
	for (uint i = 0; i < ARRAYSIZE(kScene7100RescueDialogueSeedRecords); ++i) {
		const Scene7100DialogueSeedRecord &seed = kScene7100RescueDialogueSeedRecords[i];
		DialogueChoiceRecord &record = records[seed.index];
		record.enabled = seed.enabled;
		record.nextNodeIndex = seed.nextNodeIndex;
		record.transitionMode = seed.transitionMode;
		record.playerTextRowId = seed.playerTextRowId;
		record.responseFrameIndex = seed.responseFrameIndex;
		record.disableAfterUse = seed.disableAfterUse;
		record.reserved = seed.reserved;
	}
}

bool Scene7100::runCurtainRevealFromBlack() {
	Graphics::ManagedSurface savedScene;
	savedScene.copyFrom(_sceneFramebuffer);
	byte *destination = framebufferPixels(_sceneFramebuffer);
	const byte *source = framebufferPixels(savedScene);
	if (!destination || !source)
		return false;

	memset(destination, 0, framebufferByteCount());
	presentFrame();
	for (int sweep = 0x0dc; sweep >= 0 && !_vm->isSceneRestartRequested(); sweep -= 0x14) {
		const uint bandWidth = 0x14;
		const uint innerWidth = HollywoodEngine::kScreenWidth - 2 * sweep;
		const uint middleInset = sweep + bandWidth;
		const uint middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
		const uint leftX = _viewportXOffset + sweep;
		const uint rightX = leftX + innerWidth - bandWidth;

		for (uint row = 0; row < bandWidth; ++row) {
			memcpy(destination + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX,
				source + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX,
				innerWidth);
			const uint bottomY = HollywoodEngine::kScreenHeight - bandWidth - sweep + row;
			memcpy(destination + bottomY * HollywoodEngine::kSceneBufferWidth + leftX,
				source + bottomY * HollywoodEngine::kSceneBufferWidth + leftX,
				innerWidth);
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
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
			HollywoodEngine::kSceneBufferHeight));
	presentFrame();
	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene7100::runCurtainClearToBlack() {
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	for (uint sweep = 0; sweep < 0xf0 && !_vm->isSceneRestartRequested(); sweep += 0x14) {
		const uint bandWidth = 0x14;
		const uint innerWidth = HollywoodEngine::kScreenWidth - 2 * sweep;
		const uint middleInset = sweep + bandWidth;
		const uint middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
		const uint leftX = _viewportXOffset + sweep;
		const uint rightX = leftX + innerWidth - bandWidth;

		for (uint row = 0; row < bandWidth; ++row) {
			memset(pixels + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX, 0, innerWidth);
			memset(pixels + (HollywoodEngine::kScreenHeight - bandWidth - sweep + row) *
				HollywoodEngine::kSceneBufferWidth + leftX, 0, innerWidth);
		}
		for (uint row = 0; row < middleHeight; ++row) {
			const uint y = middleInset + row;
			memset(pixels + y * HollywoodEngine::kSceneBufferWidth + leftX, 0, bandWidth);
			memset(pixels + y * HollywoodEngine::kSceneBufferWidth + rightX, 0, bandWidth);
		}
		presentFrame();
		if (pollEvents(false))
			break;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_displayPalette.markAllDirty();
	presentFrame();
}

void Scene7100::handlePickupItem15() {
	BlockingSequence sequence(*this);
	sequence.secondarySpeech(4, 0)
		.actorReplacement(16, kScene7100Chunk16DescriptorCount,
			kScene7100PickupItem15FrameMap, ARRAYSIZE(kScene7100PickupItem15FrameMap),
			kScene7100FrameMillis);
	addInventoryItem(0x15);
	sequence.sound(1)
		.commit(_vm->gameState().posterOnCellWall, false)
		.framebufferPatch(3);
}

void Scene7100::handleActionHandler315() {
	stopRealtimeSpeech();
	beginSecondarySpeechLine(0x0e, 0);
	beginPrimarySpeechLineWithAnimationGroup(0x0e, 1, 0x310, 0x8a,
		0x3f, 0x3f, 0x3f, kScene7100PrimarySpeechGroupA);
	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(14, kScene7100Chunk14DescriptorCount,
			kScene7100Handler315FrameMap, ARRAYSIZE(kScene7100Handler315FrameMap),
			kScene7100FrameMillis)
			.soundAt(4, 0x0f, 75))
		.stopSound();
	if (sequence.completed())
		runCurtainClearToBlack();
	sequence.commit(_vm->gameState().mainFlowStateId, kScene7100ExitState6072);
}

void Scene7100::handlePlateOnMousetrap() {
	if (_environmentState == 3) {
		beginSecondarySpeechLine(0x19, 1);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.secondarySpeech(0x19, 0)
		.actorReplacement(ActionOverlaySpec(19, kScene7100Chunk19DescriptorCount,
			kScene7100Extended337FrameMap, ARRAYSIZE(kScene7100Extended337FrameMap),
			kScene7100FrameMillis)
			.commitAt(0x18, _vm->gameState().cellPlateRatProgress, (byte)1)
			.patchAt(0x18, 2));
	if (_vm->gameState().cellPlateRatProgress != 1) {
		_vm->gameState().cellPlateRatProgress = 1;
		applySceneStateToHotspotsAndPatches(2);
	}
	removeInventoryItem(0x14);
	sequence.sound(1)
		.commit(_vm->gameState().cellPipesActive, false);
}

void Scene7100::handleCaptureRat() {
	BlockingSequence sequence(*this);
	sequence.secondarySpeech(0x1a, 0)
		.actorReplacement(ActionOverlaySpec(19, kScene7100Chunk19DescriptorCount,
			kScene7100Item16FirstFrameMap, ARRAYSIZE(kScene7100Item16FirstFrameMap),
			kScene7100FrameMillis)
			.commitAt(0x1f, _vm->gameState().cellPlateRatProgress, (byte)2)
			.patchAt(0x1f, 2));
	if (_vm->gameState().cellPlateRatProgress != 2) {
		_vm->gameState().cellPlateRatProgress = 2;
		applySceneStateToHotspotsAndPatches(2);
	}
	sequence.actorPath(SceneActorPose(0x168, 0x198, 4));
	if (sequence.completed()) {
		runSceneOverlay(ActionOverlaySpec(8, kScene7100Chunk8DescriptorCount,
			kScene7100Chunk8ScriptFrameMap, ARRAYSIZE(kScene7100Chunk8ScriptFrameMap),
			kScene7100FrameMillis)
			.soundAt(0x0e, 0x16)
			.noRedrawAtEnd());
	}
	sequence.secondarySpeech(0x1a, 1)
		.commit(_environmentState, (byte)4)
		.actorPath(SceneActorPose(0x0ad, 0x17b, 5))
		.secondarySpeech(0x10, 0)
		.actorReplacement(ActionOverlaySpec(20, kScene7100Chunk20DescriptorCount,
			kScene7100Item16SecondFrameMap, ARRAYSIZE(kScene7100Item16SecondFrameMap),
			kScene7100FrameMillis)
			.commitAt(0x18, _environmentState, (byte)5));
	_environmentState = 5;
	addInventoryItem(0x16);
	sequence.sound(1)
		.secondarySpeech(0x0f, 0);
}

void Scene7100::handleRemovePlate() {
	if (_vm->gameState().cellPlateRatProgress != 0) {
		dispatchGenericSceneAction(18);
		return;
	}
	if (_vm->gameState().multiToolKnifeState != 9) {
		dispatchGenericSceneAction(23);
		return;
	}

	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(18, kScene7100Chunk18DescriptorCount,
		kScene7100Item14FrameMap, ARRAYSIZE(kScene7100Item14FrameMap),
		kScene7100FrameMillis)
		.commitAt(0x24, _vm->gameState().cellPlateRemoved, true)
		.patchAt(0x24, 4));
	if (!_vm->gameState().cellPlateRemoved) {
		_vm->gameState().cellPlateRemoved = true;
		applySceneStateToHotspotsAndPatches(4);
	}
	addInventoryItem(0x14);
	sequence.sound(1)
		.secondarySpeech(0x1b, 0);
	dispatchGenericSceneAction(19);
}

void Scene7100::handleInventoryTransferAction() {
	stopRealtimeSpeech();
	const byte sueItemId = _lastInventoryPrimaryItemId;
	const uint mappingIndex = sueItemId - kScene7100FirstTransferableSueItem;
	if (mappingIndex >= ARRAYSIZE(kScene7100RonItemBySueItem)) {
		beginSecondarySpeechLine(0x23, 0);
		return;
	}

	_manualPrimaryAnimationActive = true;
	_primaryMode = 0;
	_primaryFrame = 0;
	BlockingSequence sequence(*this);
	sequence.secondarySpeech(0x27, 0)
		.actorReplacement(ActionOverlaySpec(15, kScene7100Chunk15DescriptorCount,
			kScene7100TransferFrameMap, ARRAYSIZE(kScene7100TransferFrameMap),
			kScene7100FrameMillis)
			.hookEveryFrame(kScene7100TransferRonFrameHook));
	_manualPrimaryAnimationActive = false;
	_primaryFrame = 0;

	GameplayState &state = _vm->gameState();
	state.removeInventoryItem(kScene7100SueInventoryOwner, sueItemId);
	state.addInventoryItem(kScene7100RonInventoryOwner, kScene7100RonItemBySueItem[mappingIndex]);
	sequence.sound(1);
}

} // End of namespace Hollywood
