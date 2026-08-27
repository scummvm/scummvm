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

#include "hollywood/scenes/playable/scene1070.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene1070FirstState = 0x042e;
const uint16 kScene1070ExitState1060 = 0x0425;
const uint16 kScene1070ExitStateNext = 0x0438;
const uint16 kScene1070ViewportXOffset = 0x00a0;
const uint16 kScene1070ViewportMinXOffset = 0x0068;
const uint16 kScene1070ViewportMaxXOffset = 0x00c8;
const uint kScene1070ActorBankTableEntry = 0x0000;
const uint kScene1070ActorPaletteTableEntry = 0x00cc;
const uint kScene1070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1070FrameMillis = 75;
const uint32 kScene1070BackFrameMillis = 60;
const uint32 kScene1070IdleFrameMillis = 125;
const uint32 kScene1070GhostSequenceFrameMillis = 65;
const uint32 kScene1070GhostSpecialFrameMillis = 50;
const uint32 kScene1070SpencerAmbientFrameMillis = 250;
const uint32 kScene1070SpencerLongFrameMillis = 65;
const uint32 kScene1070SpencerTransitionFrameMillis = 75;
const uint kScene1070BackDescriptorCount = 0x14;
const uint kScene1070GhostDescriptorCount = 0x12;
const uint kScene1070SpencerDescriptorCount = 0x19;
const uint kScene1070MicrophoneDescriptorCount = 0x0c;
const uint kScene1070MicrophoneStandDescriptorCount = 0x0d;
const uint kScene1070BallDescriptorCount = 0x0b;
const uint kScene1070TravelUnlockDescriptorCount = 0x11;
const byte kScene1070SpencerSpeechGroup = 1;
const byte kScene1070GhostSpeechGroup = 2;
const byte kScene1070SpencerDialogueStageId = 0x60;
const byte kScene1070SpencerPrimaryDialogueRow = 0x61;
const byte kScene1070QuasimodoDialogueStageId = 0x62;
const byte kScene1070QuasimodoPrimaryDialogueRow = 99;
const uint kScene1070DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene1070DialogueNoResponseFrame = 0xff;
const byte kScene1070TravelUnlockDestinationId = 6;
const byte kScene1070GhostAmbientSoundFirstCue = 0x1c;
const byte kScene1070SpencerAmbientSoundFirstCue = 0x1f;
const byte kScene1070CharacterAmbientSoundCueCount = 3;
const byte kScene1070FirstAmbientMusicCue = 0x0b;
const byte kScene1070AmbientMusicCueCount = 5;
const byte kScene1070AmbientMusicProbabilityModulus = 50;

const byte kScene1070BackFrameMap[] = {
	0, 1, 2, 3, 2, 1, 0, 4, 5, 6, 5, 4, 0, 1, 2, 3,
	2, 1, 0, 7, 8, 9, 8, 7, 10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 3
};

const byte kScene1070GhostFrameMap[] = {
	0, 9, 10, 11, 12, 12, 13, 14, 15, 16, 12, 11, 10, 9, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 5, 6, 7, 8,
	7, 6, 5, 4, 5, 6, 7, 8, 7, 6, 5, 3, 2, 1, 0, 17
};

const byte kScene1070SpencerFrameMap[] = {
	10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7,
	6, 5, 4, 3, 2, 1, 0, 10, 11, 12, 13, 14, 14, 15, 16, 17,
	18, 10, 19, 20, 21, 22, 23, 22, 21, 22, 23, 22, 21, 22, 23, 22,
	21, 20, 19, 10, 24, 14, 13, 12, 11, 10, 10
};

const byte kScene1070MicrophoneFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene1070MicrophoneStandFrameMap[] = {
	12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene1070BallFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 1, 0,
	1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10
};

const byte kScene1070TravelUnlockFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	12, 13, 14, 15, 12, 13, 14, 15, 12, 13, 14, 15,
	12, 13, 14, 15, 12, 13, 14, 15, 12, 16, 11, 10,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

static PlayableSceneConfig scene1070Config() {
	PlayableSceneConfig config(1070,
		SceneResourceLayout(19, 5, 18),
		SceneViewport(kScene1070ViewportXOffset, kScene1070ViewportMinXOffset, kScene1070ViewportMaxXOffset),
		SceneActorPose(0x0aa, 0x1b3, 2));
	config.setActorResources(kScene1070ActorBankTableEntry, kScene1070ActorPaletteTableEntry);
	config.setTextResources(kScene1070Resource003RowsOffsetIndex, kScene1070SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 6;
	return config;
}

Scene1070::Scene1070(HollywoodEngine *vm) :
		PlayableScene(vm, scene1070Config()),
		_backLayerChannel(),
		_ghostIdleChannel(),
		_ghostSequenceChannel(),
		_ghostSpecialChannel(),
		_spencerAmbientChannel(),
		_spencerIdleChannel(),
		_spencerLongChannel(),
		_spencerTransitionChannel(),
		_backLayer(),
		_ghostLayer(),
		_spencerLayer(),
		_backLayerMode(0),
		_ghostMode(0),
		_spencerMode(0),
		_spencerAmbientState(1),
		_lastGhostAmbientSound(0xff),
		_lastSpencerAmbientSound(0xff),
		_suppressRandomLayerStarts(false) {
	_backLayer.configure(9, kScene1070BackDescriptorCount,
		kScene1070BackFrameMap, ARRAYSIZE(kScene1070BackFrameMap));
	_ghostLayer.configure(10, kScene1070GhostDescriptorCount,
		kScene1070GhostFrameMap, ARRAYSIZE(kScene1070GhostFrameMap));
	_spencerLayer.configure(11, kScene1070SpencerDescriptorCount,
		kScene1070SpencerFrameMap, ARRAYSIZE(kScene1070SpencerFrameMap));
}

void Scene1070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_vm->gameState().scene1070SpiritBlockingHotspot = false;
	resetAnimationLayers();
	if (_vm->gameState().mainFlowStateId == kScene1070FirstState) {
		_activeActorWorldX = 0x0aa;
		_activeActorWorldY = 0x1b3;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x16a;
		_activeActorWorldY = 0x141;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene1070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_backLayer);
	drawResourceSpriteLayer(_spencerLayer);
	drawResourceSpriteLayer(_ghostLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
}

void Scene1070::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene1070FirstState)
		runEntryPath(0x064, 0x1b3, 2, 0x0aa, 0x1b3);
	else
		runEntryPath(0x1df, 0x132, 4, 0x16a, 0x141);
	drawPlayableComposite();
	presentFrame();
}

bool Scene1070::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene1070::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackLayer(delta);
	advanceCharacterAmbientAudio();
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else {
		advanceSpencerAmbientTrigger(delta);
		advanceGhostLayer(delta);
		advanceSpencerLayer(delta);
	}
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1070::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida izquierda (go to left exit).
		_vm->gameState().mainFlowStateId = kScene1070ExitState1060;
		return true;
	case 302: // Ir a puerta (go to door).
		if (_vm->gameState().scene1070DoorOpened)
			_vm->gameState().mainFlowStateId = kScene1070ExitStateNext;
		return true;
	case 303: // Mirar puerta (look at door).
		beginSecondarySpeechLine(1, _vm->gameState().scene1070DoorOpened ? 1 : 0);
		return true;
	case 304: // Usar/abrir puerta (use/open door).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Hablar con Spencer McDundee (talk to Spencer McDundee).
		runSpencerConversation();
		return true;
	case 306: // Mirar Spencer McDundee (look at Spencer McDundee).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 307: // Coger microfono (take microphone).
		handleMicrophonePickup();
		return true;
	case 308: // Mirar microfono (look at microphone).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 309: // Coger pie de micro (take microphone stand).
		handleMicStandPickup();
		return true;
	case 310: // Mirar pie de micro (look at microphone stand).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 311: // Hablar con Quasimodo (talk to Quasimodo).
		runQuasimodoConversation();
		return true;
	case 312: // Mirar Quasimodo (look at Quasimodo).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 313: // Mirar fantasma (look at ghost).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 314: // Mirar bateria (look at drums).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 315: // Usar bateria (use drums).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 316: // Coger cartel (take sign).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 317: // Mirar cartel (look at sign).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 318: // Coger cadena (take chain).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 319: // Mirar cadena (look at chain).
		beginSecondarySpeechLine(13, 0);
		return true;
	case 320: // Mirar bola (look at ball).
		beginSecondarySpeechLine(14, 0);
		return true;
	case 321: // Usar serrucho con cadena (use saw with chain): releases the ball.
		handleBallChainPickup();
		return true;
	default:
		return false;
	}
}

bool Scene1070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene1070DoorOpened) {
		replaceColorMapItem(9, 2);
		if (_sceneChunkTable.isValidChunk(13))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
		copyStageSmallRow(12, 2);
	}

	if (state.scene1070ChainRemoved)
		replaceColorMapItem(10, 0);

	if (state.scene1070SpiritBlockingHotspot)
		replaceColorMapItem(3, 0);

	if (state.scene1070MicrophoneStandTaken) {
		if (_sceneChunkTable.isValidChunk(17))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[17], _baseFramebuffer);
		replaceColorMapItem(5, 8);
	}

	if (state.scene1070MicrophoneTaken) {
		if (_sceneChunkTable.isValidChunk(15))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _baseFramebuffer);
		replaceColorMapItem(4, 0);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene1070DoorOpened) {
		ScenePoint point;
		point.x = 0x1df;
		point.y = 0x132;
		_hotspots.setActionTarget(2, point, point);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x11, 3);
	}
	return true;
}

bool Scene1070::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene1070::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene1070SpencerSpeechGroup)
		return 0x1c;
	if (animationGroup == kScene1070GhostSpeechGroup)
		return 5;
	return 0;
}

void Scene1070::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene1070SpencerSpeechGroup)
		_spencerLayer.setFrame(frameIndex);
	else if (animationGroup == kScene1070GhostSpeechGroup)
		_ghostLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene1070::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = kScene1070FirstAmbientMusicCue;
	profile.musicCueCount = kScene1070AmbientMusicCueCount;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = kScene1070AmbientMusicProbabilityModulus;
	return profile;
}

void Scene1070::resetAnimationLayers() {
	_backLayerChannel.reset(0, kScene1070BackFrameMillis);
	_ghostIdleChannel.reset(0, kScene1070IdleFrameMillis);
	_ghostSequenceChannel.reset(0, kScene1070GhostSequenceFrameMillis);
	_ghostSpecialChannel.reset(0, kScene1070GhostSpecialFrameMillis);
	_spencerAmbientChannel.reset(0, kScene1070SpencerAmbientFrameMillis);
	_spencerIdleChannel.reset(0, kScene1070IdleFrameMillis);
	_spencerLongChannel.reset(0, kScene1070SpencerLongFrameMillis);
	_spencerTransitionChannel.reset(0, kScene1070SpencerTransitionFrameMillis);
	_backLayer.reset(_vm->gameState().scene1070ChainRemoved ? 0x18 : 0);
	_ghostLayer.reset(0);
	_spencerLayer.reset(0x1c);
	_backLayer.visible = true;
	_ghostLayer.visible = true;
	_spencerLayer.visible = true;
	_backLayerMode = 0;
	_ghostMode = 0;
	_spencerMode = 5;
	_spencerAmbientState = 1;
	_lastGhostAmbientSound = 0xff;
	_lastSpencerAmbientSound = 0xff;
	_suppressRandomLayerStarts = false;
}

void Scene1070::advanceBackLayer(uint32 delta) {
	const uint frameCount = _backLayerChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		const bool chainRemoved = _vm->gameState().scene1070ChainRemoved;
		const byte startFrame = chainRemoved ? 0x18 : 0;
		const byte endFrame = chainRemoved ? 0x22 : 0x17;
		if (_backLayer.frameIndex < endFrame)
			_backLayer.setFrame(_backLayer.frameIndex + 1);
		else
			_backLayer.setFrame(startFrame);
	}
}

void Scene1070::advanceCharacterAmbientAudio() {
	SoundBank0Player &player = _additionalAmbientSoundBank0Slots[1];
	if (player.isPlaying() || _spencerAmbientState >= 2)
		return;

	byte *previous = nullptr;
	byte firstCue = 0;
	if (_spencerAmbientState == 0 && (_spencerMode == 0 || _spencerMode > 2)) {
		previous = &_lastGhostAmbientSound;
		firstCue = kScene1070GhostAmbientSoundFirstCue;
	} else if (_spencerAmbientState == 1) {
		previous = &_lastSpencerAmbientSound;
		firstCue = kScene1070SpencerAmbientSoundFirstCue;
	} else {
		return;
	}

	byte next = *previous;
	do {
		next = (byte)_random.getRandomNumber(kScene1070CharacterAmbientSoundCueCount - 1);
	} while (next == *previous);
	*previous = next;
	player.playSample((byte)(firstCue + next), 5);
}

void Scene1070::advanceSpencerAmbientTrigger(uint32 delta) {
	if (_suppressRandomLayerStarts)
		return;

	const uint frameCount = _spencerAmbientChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_spencerAmbientState == 2) {
			if (_spencerMode == 0) {
				_additionalAmbientSoundBank0Slots[1].stop();
				_spencerAmbientState = 1;
				_spencerMode = 4;
				_spencerLayer.setFrame(0x17);
			}
		} else if (_spencerAmbientState == 3) {
			if (_ghostMode != 4) {
				_additionalAmbientSoundBank0Slots[1].stop();
				_spencerAmbientState = 0;
				_ghostMode = 1;
				_ghostLayer.setFrame(0);
			}
		} else if (_random.getRandomNumber(19) == 1 &&
				((_ghostMode == 2 && _spencerMode == 0) ||
				(_ghostMode == 0 && _spencerMode == 5))) {
			_additionalAmbientSoundBank0Slots[1].stop();
			if (_spencerAmbientState != 0) {
				_spencerAmbientState = 0;
				_ghostMode = 1;
				_ghostLayer.setFrame(0);
				_spencerMode = 6;
				_spencerLayer.setFrame(0x35);
			} else {
				_spencerAmbientState = 1;
				_ghostMode = 3;
				_ghostLayer.setFrame(10);
				_spencerMode = 4;
				_spencerLayer.setFrame(0x17);
			}
		}
	}
}

void Scene1070::advanceGhostLayer(uint32 delta) {
	uint frameCount = _ghostIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_ghostMode != 0 && _ghostMode != 2)
			continue;

		byte nextFrame = _ghostLayer.frameIndex;
		if (_ghostMode == 2) {
			do {
				nextFrame = 5 + _random.getRandomNumber(4);
			} while (nextFrame == _ghostLayer.frameIndex);
			_ghostLayer.setFrame(nextFrame);
		}

		if (_ghostMode == 0 && !_suppressRandomLayerStarts) {
			if (_ghostLayer.frameIndex == 0x2f) {
				_ghostLayer.setFrame(0);
			} else if (_random.getRandomNumber(14) == 0) {
				_ghostLayer.setFrame(0x2f);
			} else if (_random.getRandomNumber(19) == 0) {
				_ghostMode = 4;
				_ghostLayer.setFrame(0x0e);
			}
		}
	}

	frameCount = _ghostSequenceChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_ghostMode == 1) {
			if (_ghostLayer.frameIndex < 4) {
				_ghostLayer.setFrame(_ghostLayer.frameIndex + 1);
			} else {
				_ghostMode = 2;
			}
		} else if (_ghostMode == 3) {
			if (_ghostLayer.frameIndex < 0x0e) {
				_ghostLayer.setFrame(_ghostLayer.frameIndex + 1);
			} else {
				_ghostMode = 0;
				_ghostLayer.setFrame(0);
			}
		}
	}

	frameCount = _ghostSpecialChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_ghostMode != 4)
			continue;

		if (_ghostLayer.frameIndex < 0x2e)
			_ghostLayer.setFrame(_ghostLayer.frameIndex + 1);
		else
			_ghostMode = 0;
	}
}

void Scene1070::advanceSpencerLayer(uint32 delta) {
	uint frameCount = _spencerIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_spencerMode != 0 && _spencerMode != 5)
			continue;

		if (_spencerMode == 5) {
			byte nextFrame = _spencerLayer.frameIndex;
			do {
				nextFrame = 0x1c + _random.getRandomNumber(4);
			} while (nextFrame == _spencerLayer.frameIndex);
			_spencerLayer.setFrame(nextFrame);
		}

		if (_spencerMode == 0 && !_suppressRandomLayerStarts) {
			if (_spencerLayer.frameIndex == 0x34) {
				_spencerLayer.setFrame(0);
			} else if (_random.getRandomNumber(14) == 0) {
				_spencerLayer.setFrame(0x34);
			} else if (_random.getRandomNumber(29) == 0) {
				if (_random.getRandomBit() == 0) {
					_spencerMode = 1;
					_spencerLayer.setFrame(0x0c);
					_ghostMode = 3;
					_ghostLayer.setFrame(10);
				} else {
					_spencerMode = 7;
					_spencerLayer.setFrame(0x21);
				}
			}
		}
	}

	frameCount = _spencerLongChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_spencerMode == 2 && _random.getRandomNumber(99) == 0) {
			_additionalAmbientSoundBank0Slots[1].stop();
			_spencerMode = 3;
			_spencerLayer.setFrame(1);
			_spencerAmbientState = 0;
			_ghostMode = 1;
			_ghostLayer.setFrame(0);
			updateSpiritBlockingHotspot(false);
		}

		if (_spencerMode == 4) {
			if (_spencerLayer.frameIndex < 0x1b)
				_spencerLayer.setFrame(_spencerLayer.frameIndex + 1);
			else
				_spencerMode = 5;
		}

		if (_spencerMode == 6) {
			if (_spencerLayer.frameIndex < 0x39) {
				_spencerLayer.setFrame(_spencerLayer.frameIndex + 1);
			} else {
				_spencerMode = 0;
				_spencerLayer.setFrame(0);
			}
		}
	}

	frameCount = _spencerTransitionChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_spencerMode == 1) {
			if (_spencerLayer.frameIndex < 0x16) {
				_spencerLayer.setFrame(_spencerLayer.frameIndex + 1);
			} else {
				_spencerMode = 2;
				updateSpiritBlockingHotspot(true);
			}
		}

		if (_spencerMode == 3) {
			if (_spencerLayer.frameIndex < 0x0b) {
				_spencerLayer.setFrame(_spencerLayer.frameIndex + 1);
			} else {
				_spencerMode = 0;
				_spencerLayer.setFrame(0);
				updateSpiritBlockingHotspot(false);
			}
		}

		if (_spencerMode == 7) {
			if (_spencerLayer.frameIndex < 0x33)
				_spencerLayer.setFrame(_spencerLayer.frameIndex + 1);
			else
				_spencerMode = 0;
		}
	}
}

void Scene1070::updateSpiritBlockingHotspot(bool blocked) {
	GameplayState &state = _vm->gameState();
	if (state.scene1070SpiritBlockingHotspot == blocked)
		return;

	state.scene1070SpiritBlockingHotspot = blocked;
	applySceneStateToHotspotsAndPatches(2);
}

bool Scene1070::hasActiveCharacterAnimation() const {
	return _ghostMode != 0 || _spencerMode != 0;
}

void Scene1070::settleCharacterAnimations() {
	_suppressRandomLayerStarts = true;

	while (hasActiveCharacterAnimation() && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (_ghostMode == 2) {
			_ghostMode = 3;
			_ghostLayer.setFrame(9);
		}
		if (_spencerMode == 2) {
			_spencerMode = 3;
			_spencerLayer.setFrame(1);
			updateSpiritBlockingHotspot(false);
		}
		if (_spencerMode == 5) {
			_spencerMode = 6;
			_spencerLayer.setFrame(0x34);
		}

		if (waitSceneMillis(10))
			break;
	}

	_suppressRandomLayerStarts = false;
}

void Scene1070::waitForGhostMode(byte mode) {
	while (_ghostMode != mode && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10))
			break;
	}
}

void Scene1070::waitForSpencerMode(byte mode) {
	while (_spencerMode != mode && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10))
			break;
	}
}

void Scene1070::beginSpencerPrimarySpeechLine(byte frameIndex, byte openFrame) {
	_suppressRandomLayerStarts = true;
	_spencerMode = 4;
	_spencerLayer.setFrame(openFrame);
	waitForSpencerMode(5);
	beginPrimarySpeechLineWithAnimationGroup(kScene1070SpencerPrimaryDialogueRow, frameIndex,
		0x0f1, 0x0b8, 0x3f, 0x0d, 0x0d, kScene1070SpencerSpeechGroup);
	_spencerMode = 6;
	_spencerLayer.setFrame(0x35);
	waitForSpencerMode(0);
	_suppressRandomLayerStarts = false;
}

void Scene1070::beginQuasimodoPrimarySpeechLine(byte frameIndex) {
	_suppressRandomLayerStarts = true;
	_ghostMode = 1;
	_ghostLayer.setFrame(0);
	waitForGhostMode(2);
	beginPrimarySpeechLineWithAnimationGroup(kScene1070QuasimodoPrimaryDialogueRow, frameIndex,
		0x0a9, 0x0c8, 0x20, 0x3f, 0, kScene1070GhostSpeechGroup);
	_ghostMode = 3;
	_ghostLayer.setFrame(9);
	waitForGhostMode(0);
	_suppressRandomLayerStarts = false;
}

void Scene1070::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x138 && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (activeWorldX > 0x18a && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (activeWorldY < 0x132 && _sceneChunkTable.isValidChunk(8))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _sceneFramebuffer);
	if (activeWorldY <= 0x132 && activeWorldX >= 399 && _sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	if (_vm->gameState().scene1070MicrophoneTaken && _sceneChunkTable.isValidChunk(15))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _sceneFramebuffer);
}

void Scene1070::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene1070::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene1070::runSpencerConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeSpencerDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	const byte line = state.scene1070SpencerConversationSeen ? 1 : 0;
	beginSecondarySpeechLine(kScene1070SpencerDialogueStageId, line);
	settleCharacterAnimations();
	beginSpencerPrimarySpeechLine(line, 0x17);
	state.scene1070SpencerConversationSeen = true;

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene1070SpencerDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene1070SpencerDialogueStageId, 7);
			beginSpencerPrimarySpeechLine(7, 0x16);
			finishSpencerConversation();
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene1070SpencerDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene1070DialogueNoResponseFrame)
			beginSpencerPrimarySpeechLine(record.responseFrameIndex, 0x16);

		handleSpencerDialogueEffect(record.disableAfterUse, records, recordIndex);

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 1;
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 2;
			break;
		default:
			break;
		}
	}
	finishSpencerConversation();
}

void Scene1070::runQuasimodoConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeQuasimodoDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	const byte line = state.scene1070QuasimodoConversationSeen ? 1 : 0;
	beginSecondarySpeechLine(kScene1070QuasimodoDialogueStageId, line);
	settleCharacterAnimations();
	beginQuasimodoPrimarySpeechLine(line);
	state.scene1070QuasimodoConversationSeen = true;

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene1070QuasimodoDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene1070QuasimodoDialogueStageId, 5);
			beginQuasimodoPrimarySpeechLine(5);
			finishQuasimodoConversation();
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene1070QuasimodoDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene1070DialogueNoResponseFrame)
			beginQuasimodoPrimarySpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse == 1)
			record.enabled = 0;
		if (record.disableAfterUse == 2)
			_vm->gameState().mainFlowStateId = 0x03fd;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 1;
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 2;
			break;
		default:
			break;
		}
	}
	finishQuasimodoConversation();
}

void Scene1070::finishSpencerConversation() {
	_additionalAmbientSoundBank0Slots[1].stop();
	_spencerAmbientState = 3;
}

void Scene1070::finishQuasimodoConversation() {
	_additionalAmbientSoundBank0Slots[1].stop();
	_spencerAmbientState = 2;
}

void Scene1070::initializeSpencerDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene1070DialogueChoiceRecordCount);

	// g_abA07SpencerDialogueChoiceRecords: root choices.
	setDialogueRecord(records, 0, 1, 0, 1, 2, 2, 1);
	setDialogueRecord(records, 1, 1, 1, 1, 4, 4, 2);
	setDialogueRecord(records, 2, 0, 0, 3, 5, 5, 4);
	setDialogueRecord(records, 3, 0, 1, 1, 6, 6, 1);
	setDialogueRecord(records, 4, 1, 0, 0, 7, 7, 1);

	// Depth 1, node 0: follow-up after root choice 0.
	setDialogueRecord(records, 70, 1, 0, 2, 3, kScene1070DialogueNoResponseFrame, 0);

	// Depth 1, node 1: follow-up after root choice 1.
	setDialogueRecord(records, 77, 1, 1, 3, 8, 8, 1);
	setDialogueRecord(records, 78, 1, 1, 3, 9, 9, 1);
	setDialogueRecord(records, 79, 1, 1, 3, 10, 10, 1);
	setDialogueRecord(records, 80, 1, 1, 3, 11, 11, 3);
	setDialogueRecord(records, 81, 1, 1, 3, 13, 13, 5);
	setDialogueRecord(records, 82, 1, 0, 2, 12, 12, 0);

	const byte progress = _vm->gameState().scene1070SpencerTravelClueProgress;
	if (progress != 0)
		records[3].enabled = 1;
	if (progress == 2)
		records[2].enabled = 1;
	if (_vm->gameState().scene1070SpencerCocktailRecipeLearned)
		records[81].enabled = 0;
}

void Scene1070::initializeQuasimodoDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene1070DialogueChoiceRecordCount);
	GameplayState &state = _vm->gameState();

	// g_abA07QuasimodoDialogueChoiceRecords: root choices.
	setDialogueRecord(records, 0, 1, 0, 3, 2, 2, 1);
	setDialogueRecord(records, 1, 1, 0, 1, 3, 3, 1);
	setDialogueRecord(records, 2, 1, 1, 1, 4, 4, 1);
	setDialogueRecord(records, 3, 1, 0, 0, 5, 5, 1);

	// Depth 1, node 0: follow-up after root choice 1.
	setDialogueRecord(records, 70, 1, 0, 2, 6, 6, 0);

	// Depth 1, node 1: follow-up after root choice 2.
	setDialogueRecord(records, 77, 1, 1, 3, 7, 7, 1);
	setDialogueRecord(records, 78, 0, 0, 1, 8, 8, 1);
	setDialogueRecord(records, 79, 1, 1, 3, 9, 9, 1);
	setDialogueRecord(records, 80, 1, 0, 2, 10, 10, 1);
	if (state.scene1020HookPositionState == 2 &&
			state.scene1020ChainAttachedToGrate == 1 &&
			!state.scene1020GrateRaised)
		records[78].enabled = 1;

	// Depth 2, node 0: branch after Ron attaches the chain to the scene1020 grate.
	setDialogueRecord(records, 140, 1, 0, 1, 11, 11, 0);
	setDialogueRecord(records, 210, 1, 0, 0, 12, 12, 2);
}

void Scene1070::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = enabled;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = 0xff;
}

void Scene1070::handleSpencerDialogueEffect(byte effect, Common::Array<DialogueChoiceRecord> &records,
		uint recordIndex) {
	if (effect == 0)
		return;

	if (recordIndex < records.size())
		records[recordIndex].enabled = 0;

	GameplayState &state = _vm->gameState();
	if (effect == 2 && state.scene1070SpencerTravelClueProgress == 0) {
		state.scene1070SpencerTravelClueProgress = 1;
		records[3].enabled = 1;
	} else if (effect == 3 && state.scene1070SpencerTravelClueProgress == 1) {
		state.scene1070SpencerTravelClueProgress = 2;
		records[2].enabled = 1;
	} else if (effect == 4 && state.scene1070SpencerTravelClueProgress == 2) {
		state.scene1070SpencerTravelClueProgress = 3;
		runSpencerTravelUnlockEffect();
	} else if (effect == 5) {
		state.scene1070SpencerCocktailRecipeLearned = true;
	}
}

void Scene1070::runSpencerTravelUnlockEffect() {
	if (_vm->gameState().hasTravelScreenDestination(kScene1070TravelUnlockDestinationId))
		return;

	runActorReplacement(ActionOverlaySpec(16, kScene1070TravelUnlockDescriptorCount,
		kScene1070TravelUnlockFrameMap, ARRAYSIZE(kScene1070TravelUnlockFrameMap), kScene1070FrameMillis));
	_soundBank0.playSample(1, 100);
	_vm->gameState().unlockTravelScreenDestination(kScene1070TravelUnlockDestinationId);
}

void Scene1070::handleMicrophonePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1070MicrophoneTaken || hasInventoryItem(0x23))
		return;

	state.scene1070MicrophoneTaken = true;
	runOverlaySequence(14, kScene1070MicrophoneDescriptorCount, kScene1070MicrophoneFrameMap,
		ARRAYSIZE(kScene1070MicrophoneFrameMap), kScene1070FrameMillis, 5, 7);
	addInventoryItem(0x23);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene1070::handleMicStandPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1070MicrophoneStandTaken || hasInventoryItem(0x24))
		return;

	state.scene1070MicrophoneStandTaken = true;
	runOverlaySequence(18, kScene1070MicrophoneStandDescriptorCount, kScene1070MicrophoneStandFrameMap,
		ARRAYSIZE(kScene1070MicrophoneStandFrameMap), kScene1070FrameMillis, 6, 6);
	addInventoryItem(0x24);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene1070::handleBallChainPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1070ChainRemoved || hasInventoryItem(0x1e))
		return;

	dispatchGenericSceneAction(21);
	_soundBank0.playSampleLooping(0x22, 15);
	runOverlaySequence(12, kScene1070BallDescriptorCount, kScene1070BallFrameMap,
		ARRAYSIZE(kScene1070BallFrameMap), kScene1070FrameMillis, -1, 1);
	_soundBank0.stop();
	state.scene1070ChainRemoved = true;
	addInventoryItem(0x1e);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(15, 0);
}

void Scene1070::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame, byte patchSelector) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		frameMap, frameMapSize, frameMillis)
		.patchAt(patchFrame, patchSelector));
}

} // End of namespace Hollywood
