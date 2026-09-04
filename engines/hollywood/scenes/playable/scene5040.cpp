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
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/resource.h"
#include "hollywood/scenes/playable/scene5040.h"

namespace Hollywood {

const uint16 kScene5040FirstState = 0x13b0;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5050EntryFromKarlGalleryState = 0x13bb;
const uint kScene5040ActorBankTableEntry = 0x0000;
const uint kScene5040ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5040SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5040FrameMillis = 75;
const uint32 kScene5040MineCartFrameMillis = 40;
const uint kScene5040KarlDescriptorCount = 0x2e;
const uint kScene5040MineBoxPickupDescriptorCount = 0x0d;
const uint kScene5040MineCartDescriptorCount = 0x59;
const uint kScene5040ItemSwapDescriptorCount = 0x0c;
const byte kScene5040KarlDialogueSpeechGroup = 0;
const byte kScene5040KarlMiningSpeechGroup = 1;
const byte kScene5040KarlDialogueSpeechBaseFrame = 0;
const byte kScene5040KarlMiningSpeechBaseFrame = 0x4c;
const byte kScene5040KarlMiningSpeechLastFrame = 0x4f;
const byte kScene5040KarlSpeechFrameCount = 4;
const byte kScene5040KarlDialogueStageId = 0x62;
const byte kScene5040KarlPrimaryRow = 99;
const byte kScene5040DialogueNoResponseFrame = 0xff;
const uint kScene5040KarlDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene5040MagneticBombPillboxItem = 0x0b;
const byte kScene5040DowsingRodItem = 0x37;
const byte kScene5040PatchedSockItem = 0x49;
const byte kScene5040KarlPrizeItem = 0x4a;
const byte kScene5040KeyItem = 0x4b;
const byte kScene5040KarlSceneItem = 4;

enum {
	kScene5040KarlLayer
};

enum {
	kScene5040DialogueTransitionEnd = 0,
	kScene5040DialogueTransitionDown = 1,
	kScene5040DialogueTransitionUp = 2,
	kScene5040DialogueTransitionStay = 3,
	kScene5040DialogueTransitionUpTwo = 4
};

enum {
	kScene5040KarlIdleModeWait = 0,
	kScene5040KarlIdleModeShort = 1,
	kScene5040KarlIdleModeRevealTool = 2,
	kScene5040KarlIdleModeToolExposedWait = 3,
	kScene5040KarlIdleModeHideTool = 4,
	kScene5040KarlIdleModeStrikeLoop = 5,
	kScene5040KarlIdleModeResumeStriking = 6,
	kScene5040KarlIdleModeStopStriking = 7,
	kScene5040KarlIdleModePausedAtWall = 8
};

const byte kScene5040KarlFrameMap[] = {
	0, 43, 44, 45, 0, 1, 2, 3, 4, 3, 4, 3, 4, 3, 4, 3,
	4, 3, 2, 1, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14,
	13, 12, 11, 10, 9, 8, 7, 6, 5, 0, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, 26, 27, 28, 0, 29, 30, 31, 32, 33, 34, 34,
	34, 34, 35, 36, 37, 0, 37, 38, 39, 40, 41, 42, 20, 21, 22, 21
};

const byte kScene5040DowsingRodSwapFrameMap[] = {
	11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 11
};

PlayableSceneConfig scene5040Config() {
	PlayableSceneConfig config(5040,
		SceneResourceLayout(5, 5, 17),
		SceneViewport(0),
		SceneActorPose(0x218, 0x161, 5));
	config.setActorResources(kScene5040ActorBankTableEntry, kScene5040ActorPaletteTableEntry);
	config.setTextResources(0, kScene5040SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene5040::Scene5040(HollywoodEngine *vm) :
		PlayableScene(vm, scene5040Config()),
		_karlIdleChannel(),
		_karlIdleFrame(0),
		_karlIdleMode(kScene5040KarlIdleModeWait),
		_karlStrikeRepeatCount(0),
		_suspendKarlIdle(false),
		_mineCartRumbleActive(false),
		_lastKarlMiningSpeechFrame(kScene5040KarlMiningSpeechBaseFrame),
		_previousKarlMiningSpeechFrame(kScene5040KarlMiningSpeechBaseFrame) {
	_sceneLayers.configureLayer(kScene5040KarlLayer, kSceneAnimationBehindActors,
		9, kScene5040KarlDescriptorCount,
		kScene5040KarlFrameMap, ARRAYSIZE(kScene5040KarlFrameMap));
}

void Scene5040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	const GameplayState &state = _vm->gameState();
	if (state.scene5040SpecialTransitionState == 1)
		setActiveActorPose(0x32d, 0x12c, 4);
	else if (state.mainFlowStateId == kScene5040FirstState)
		setActiveActorPose(0x32d, 0x110, 4);
	else
		setActiveActorPose(0x0f6, 0x0e9, 2);
}

void Scene5040::prepareCustomComposite(bool drawActors, byte activeFacing,
		int activeWorldX, int activeWorldY, byte actorDrawOrderMode) {
	(void)drawActors;
	(void)activeFacing;
	(void)activeWorldX;
	(void)activeWorldY;
	(void)actorDrawOrderMode;
	if (_vm->gameState().scene5040MineGalleryState >= 2)
		_sceneLayers.setLayerVisible(kScene5040KarlLayer, false);
}

void Scene5040::drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldX;
	(void)actorDrawOrderMode;
	if (activeWorldY < 0x138 && _sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(16))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[16], _sceneFramebuffer);
}

void Scene5040::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();

	if (state.scene5040SpecialTransitionState == 1) {
		const Common::Array<byte> savedPaletteRegionMask = _fullPaletteRegionMask;
		memset(_fullPaletteRegionMask.data(), 1, _fullPaletteRegionMask.size());
		setActiveActorPose(0x32d, 0x12c, 4);
		_mineCartRumbleActive = true;
		runMineCartEntryClip();
		walkActiveActorTo(0x238, 0x145, 5, 0, false);
		beginStaticSecondarySpeechLine(0xd0, 0);
		walkActiveActorTo(0x238, 0x145, 1, 0, false);
		beginSecondarySpeechLine(8, 0);
		runActorReplacement(ActionOverlaySpec(15, 0x0e, kScene5040FrameMillis)
			.bookendWithLastFrame()
			.resourcePatchAt(6, 14));
		state.scene5040SpecialTransitionState = 2;
		addInventoryItem(kScene5040KarlPrizeItem);
		_soundBank0.playSample(1, 100);
		walkActiveActorTo(0x208, 0x15e, 4, 0, false);
		_fullPaletteRegionMask = savedPaletteRegionMask;
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
		return;
	}

	if (state.mainFlowStateId == kScene5040FirstState) {
		setActiveActorPose(0x32d, 0x110, 4);
		_mineCartRumbleActive = true;
		runMineCartEntryClip();
		if (state.scene5040EntryLineSeen) {
			walkActiveActorTo(0x19e, 0x172, 4, 0, false);
		} else {
			walkActiveActorTo(0x218, 0x161, 5, 0, false);
			beginSecondarySpeechLine(0, 0);
			state.scene5040EntryLineSeen = true;
		}
		return;
	}

	setActiveActorPose(0x0f6, 0x0e9, 2);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;
	walkActiveActorTo(0x1c3, 0x15e, 0xff, 0, false);
}

void Scene5040::advanceCustomGameplayLoop(uint32 delta) {
	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	if (_mineCartRumbleActive && !_soundBank0.isPlaying())
		_soundBank0.playSample(0x18, 100);

	if (!_primaryDialogueSpeechActive && !_suspendKarlIdle && !_actionOverlayPlayer.isVisible()) {
		advanceKarlLayer(delta);
	}
}

void Scene5040::advancePrimarySpeechAnimation(uint32 delta) {
	if (!_primaryDialogueSpeechActive)
		return;
	if (_primaryDialogueSpeechGroup == kScene5040KarlMiningSpeechGroup)
		advanceKarlMiningSpeech(delta);
	else
		PlayableScene::advancePrimarySpeechAnimation(delta);
}

bool Scene5040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a vagoneta/salida (go to mine cart/exit): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Ir a/usar boquete (go/use hole): enter Karl's adjacent gallery, scene 5050 state 0x13bb.
		runExitToMineHole();
		return true;
	case 303: // Mirar boquete (look at hole): Ron remembers Karl was right about the diamond.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 304: // Hablar con minero/Karl Hecker (talk to miner/Karl Hecker).
		runKarlConversation();
		return true;
	case 305: // Coger minero/Karl Hecker (take miner): Ron refuses, state-aware.
		beginSecondarySpeechLine(3, _vm->gameState().scene5040MineGalleryState == 0 ? 0 : 1);
		return true;
	case 306: // Mirar minero/Karl Hecker (look at miner): describes Karl or his condition, state-aware.
		beginSecondarySpeechLine(4, _vm->gameState().scene5040MineGalleryState < 2 ? 0 : 1);
		return true;
	case 307: // Coger varita (take dowsing rod): Ron avoids angering Karl.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Mirar varita (look at wand): it locates diamonds.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar pico (look at pickaxe): standard miner tool.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Coger diamante: dormant original record; the diamond has no live hotspot.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Mirar diamante: dormant original record; the diamond has no live hotspot.
		beginSecondarySpeechLine(10, 0);
		return true;
	case 312: // Mirar caja (look at box): cycles caja -> calcetín viejo -> llave -> empty.
		runMineBoxLook();
		return true;
	case 313: // Coger calcetín viejo (take old sock): grants calcetín remendado, item 0x49.
		runPatchedSockPickup();
		return true;
	case 314: // Mirar calcetín viejo (look at old sock): it is full of patches.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Coger llave (take key): grants llave, item 0x4b.
		runMineKeyPickup();
		return true;
	case 316: // Mirar llave (look at key): it must belong to Karl.
		beginSecondarySpeechLine(12, 0);
		return true;
	case 317: // Usar pastillero bomba con imán con pico: plant it while Karl is facing the wall.
		runSpecialMineExitWithMagneticPillbox();
		return true;
	case 318: // Usar pastillero bomba sin imán con pico: it needs something to hold it in place.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 319: // Usar varita con varita del minero: exchange the rods while Karl is distracted.
		runDowsingRodSwap();
		return true;
	case 320: // Dar varita zahorí a Karl: restored offer before the swap or refusal afterward.
		runRestoredDowsingRodOffer();
		return true;
	default:
		return false;
	}
}

bool Scene5040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	switch (state.scene5040MineGalleryState) {
	case 0:
		remapSceneColors(3, 0);
		remapSceneColors(4, 4);
		remapSceneColors(0x0e, 4);
		remapSceneColors(0x0f, 0);
		remapSceneColors(0x10, 0);
		remapSceneColors(0x11, 0);
		break;
	case 1:
		remapSceneColors(3, 0);
		remapSceneColors(4, 0);
		remapSceneColors(0x0e, 4);
		remapSceneColors(0x0f, 4);
		remapSceneColors(0x10, 4);
		remapSceneColors(0x11, 0);
		break;
	case 2:
		remapSceneColors(3, 3);
		remapSceneColors(4, 0);
		remapSceneColors(0x0e, 0);
		remapSceneColors(0x0f, 0);
		remapSceneColors(0x10, 3);
		remapSceneColors(0x11, 4);
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		if (_sceneChunkTable.isValidChunk(8))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		break;
	default:
		break;
	}
	if (state.scene5040LooseObjectTaken) {
		remapSceneColors(5, 0);
		if (state.scene5040MineGalleryState != 1)
			remapSceneColors(0x0d, 0);
	}
	if (state.scene5040OldSockTaken) {
		remapSceneColors(6, 0);
		if (state.scene5040MineGalleryState != 1)
			remapSceneColors(0x0b, 0);
		remapSceneColors(0x0c, 0);
		clearSceneItemFromColorMap(6);
	}
	switch (state.scene5040DialState) {
	case 1:
		remapSceneColors(0x12, 9);
		break;
	case 3:
		remapSceneColors(0x12, 10);
		break;
	default:
		remapSceneColors(0x12, 8);
		break;
	}
	if (state.scene5040KarlDialogueIntroSeen)
		copyStageSmallRow(4, 11);
	if (state.scene5040SpecialTransitionState == 1 &&
			_sceneChunkTable.isValidChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);

	rebuildWalkablePaletteMask();
	if (!_hotspots.load(_paletteMask, _metadata, _stage003SmallRows))
		return true;
	if (_vm->restoredContentEnabled())
		_hotspots.setRelationActionHandler(kScene5040DowsingRodItem,
			kScene5040KarlSceneItem, 2, 320);
	return true;
}

bool Scene5040::customizeRouteFinal(byte currentRegion, byte targetRegion,
		const ActorPathBuildState &state, int targetX, int targetY,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 7) {
		requestedFacing = 5;
		return true;
	}
	return false;
}

void Scene5040::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene5040::ambientAudioProfile() const {
	return createMineAmbientAudioProfile();
}

byte Scene5040::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	return animationGroup == kScene5040KarlMiningSpeechGroup ?
		kScene5040KarlMiningSpeechBaseFrame : kScene5040KarlDialogueSpeechBaseFrame;
}

byte Scene5040::primarySpeechAnimationFrameCount(byte animationGroup) const {
	(void)animationGroup;
	return kScene5040KarlSpeechFrameCount;
}

void Scene5040::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene5040KarlDialogueSpeechGroup) {
		if (frameIndex >= kScene5040KarlSpeechFrameCount)
			frameIndex = kScene5040KarlDialogueSpeechBaseFrame;
	} else if (animationGroup == kScene5040KarlMiningSpeechGroup) {
		if (frameIndex < kScene5040KarlMiningSpeechBaseFrame ||
				frameIndex > kScene5040KarlMiningSpeechLastFrame)
			frameIndex = kScene5040KarlMiningSpeechBaseFrame;
		_previousKarlMiningSpeechFrame = _lastKarlMiningSpeechFrame;
		_lastKarlMiningSpeechFrame = frameIndex;
	} else {
		return;
	}
	_sceneLayers.setLayerFrame(kScene5040KarlLayer, frameIndex);
}

void Scene5040::primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) {
	if (animationGroup == kScene5040KarlDialogueSpeechGroup ||
			animationGroup == kScene5040KarlMiningSpeechGroup) {
		_sceneLayers.setLayerVisible(kScene5040KarlLayer,
			_vm->gameState().scene5040MineGalleryState < 2);
		if (animationGroup == kScene5040KarlMiningSpeechGroup) {
			_lastKarlMiningSpeechFrame = baseFrame;
			_previousKarlMiningSpeechFrame = baseFrame;
		}
	}
}

void Scene5040::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)baseFrame;
	if (animationGroup != kScene5040KarlDialogueSpeechGroup &&
			animationGroup != kScene5040KarlMiningSpeechGroup)
		return;

	_sceneLayers.setLayerVisible(kScene5040KarlLayer,
		_vm->gameState().scene5040MineGalleryState < 2);
	if (!_sceneLayers.layerVisible(kScene5040KarlLayer))
		return;

	if (animationGroup == kScene5040KarlDialogueSpeechGroup) {
		_karlIdleFrame = 0;
		_karlIdleMode = kScene5040KarlIdleModeWait;
	} else if (_previousKarlMiningSpeechFrame == 0x4d ||
			_previousKarlMiningSpeechFrame == 0x4f) {
		_karlIdleFrame = 0x31;
	} else if (_previousKarlMiningSpeechFrame == 0x4e) {
		_karlIdleFrame = 0x32;
	} else {
		_karlIdleFrame = 0x30;
	}
	_sceneLayers.setLayerFrame(kScene5040KarlLayer, _karlIdleFrame);
}

void Scene5040::resetAnimationLayers() {
	GameplayState &state = _vm->gameState();
	state.scene5040LooseObjectTaken = true;
	state.scene5040OldSockTaken = state.scene5040MineGalleryState != 1;
	_karlIdleChannel.reset(0, kScene5040FrameMillis);
	const bool karlPausedAtWall = state.scene5040MineGalleryState == 1;
	_sceneLayers.resetLayer(kScene5040KarlLayer, karlPausedAtWall ? 0x49 : 0);
	_sceneLayers.setLayerVisible(kScene5040KarlLayer, state.scene5040MineGalleryState < 2);
	_karlIdleFrame = _sceneLayers.layerFrame(kScene5040KarlLayer);
	_karlIdleMode = karlPausedAtWall ? kScene5040KarlIdleModePausedAtWall : kScene5040KarlIdleModeWait;
	_karlStrikeRepeatCount = 0;
	_suspendKarlIdle = false;
	_mineCartRumbleActive = false;
	_lastKarlMiningSpeechFrame = kScene5040KarlMiningSpeechBaseFrame;
	_previousKarlMiningSpeechFrame = kScene5040KarlMiningSpeechBaseFrame;
}

void Scene5040::advanceKarlLayer(uint32 delta) {
	if (!_sceneLayers.layerVisible(kScene5040KarlLayer))
		return;

	GameplayState &state = _vm->gameState();
	const uint consumedFrames = _karlIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < consumedFrames; ++i) {
		bool updateFrame = true;
		switch (_karlIdleMode) {
		case kScene5040KarlIdleModeWait:
			if (_random.getRandomNumber(49) == 0) {
				_karlIdleFrame = 4;
				_karlIdleMode = kScene5040KarlIdleModeShort;
			} else if (_random.getRandomNumber(49) == 0) {
				_karlIdleFrame = 0x14;
				_karlIdleMode = kScene5040KarlIdleModeRevealTool;
			} else {
				updateFrame = false;
			}
			break;
		case kScene5040KarlIdleModeShort:
			if (_karlIdleFrame < 0x13) {
				++_karlIdleFrame;
			} else {
				_karlIdleFrame = 0;
				_karlIdleMode = kScene5040KarlIdleModeWait;
			}
			break;
		case kScene5040KarlIdleModeRevealTool:
			if (_karlIdleFrame < 0x1e) {
				++_karlIdleFrame;
			} else {
				_karlIdleMode = kScene5040KarlIdleModeToolExposedWait;
				state.scene5040LooseObjectTaken = false;
				applySceneStateToHotspotsAndPatches(2);
				updateFrame = false;
			}
			break;
		case kScene5040KarlIdleModeToolExposedWait:
			if (_random.getRandomNumber(149) == 0) {
				_karlIdleFrame = 0x1f;
				_karlIdleMode = kScene5040KarlIdleModeHideTool;
				state.scene5040LooseObjectTaken = true;
				applySceneStateToHotspotsAndPatches(2);
			} else {
				updateFrame = false;
			}
			break;
		case kScene5040KarlIdleModeHideTool:
			if (_karlIdleFrame < 0x29) {
				++_karlIdleFrame;
			} else {
				_karlIdleFrame = 0;
				_karlIdleMode = kScene5040KarlIdleModeWait;
			}
			break;
		case kScene5040KarlIdleModeStrikeLoop:
			if (_karlIdleFrame < 0x44) {
				++_karlIdleFrame;
				if (_karlIdleFrame == 0x3e)
					_soundBank0.playSample(0x1a, 100);
			} else if (_karlStrikeRepeatCount > 1) {
				--_karlStrikeRepeatCount;
				_karlIdleFrame = 0x39;
			} else {
				_karlStrikeRepeatCount = 0;
				_karlIdleFrame = 0x47;
				_karlIdleMode = kScene5040KarlIdleModeStopStriking;
			}
			break;
		case kScene5040KarlIdleModeResumeStriking:
			if (_karlIdleFrame < 0x4b) {
				++_karlIdleFrame;
			} else {
				_karlIdleFrame = 0x39;
				_karlStrikeRepeatCount = (byte)(_random.getRandomNumber(7) + 1);
				_karlIdleMode = kScene5040KarlIdleModeStrikeLoop;
			}
			break;
		case kScene5040KarlIdleModeStopStriking:
			if (_karlIdleFrame < 0x49) {
				++_karlIdleFrame;
			} else {
				_karlIdleMode = kScene5040KarlIdleModePausedAtWall;
				state.scene5040OldSockTaken = false;
				applySceneStateToHotspotsAndPatches(3);
				updateFrame = false;
			}
			break;
		case kScene5040KarlIdleModePausedAtWall:
			if (_random.getRandomNumber(99) == 0) {
				_karlIdleFrame = 0x4a;
				_karlIdleMode = kScene5040KarlIdleModeResumeStriking;
				state.scene5040OldSockTaken = true;
				applySceneStateToHotspotsAndPatches(3);
			} else {
				updateFrame = false;
			}
			break;
		default:
			_karlIdleFrame = 0;
			_karlIdleMode = kScene5040KarlIdleModeWait;
			break;
		}

		if (updateFrame)
			_sceneLayers.setLayerFrame(kScene5040KarlLayer, _karlIdleFrame);
	}
}

void Scene5040::advanceKarlMiningSpeech(uint32 delta) {
	_primaryDialogueSpeechTimerAccumulator += delta;
	while (_primaryDialogueSpeechTimerAccumulator >= kScene5040FrameMillis) {
		_primaryDialogueSpeechTimerAccumulator -= kScene5040FrameMillis;
		const byte nextFrame = _lastKarlMiningSpeechFrame < kScene5040KarlMiningSpeechLastFrame ?
			(byte)(_lastKarlMiningSpeechFrame + 1) : kScene5040KarlMiningSpeechBaseFrame;
		setPrimarySpeechAnimationFrame(kScene5040KarlMiningSpeechGroup, nextFrame);
	}
}

bool Scene5040::playKarlFrames(byte firstFrame, byte lastFrame, uint32 frameMillis) {
	const bool previousSuspendKarlIdle = _suspendKarlIdle;
	_suspendKarlIdle = true;
	_sceneLayers.setLayerVisible(kScene5040KarlLayer, true);
	const bool completed = playAndPresentAnimationFrames(kScene5040KarlLayer,
		AnimationFrameRange(firstFrame, lastFrame, frameMillis));
	_karlIdleFrame = _sceneLayers.layerFrame(kScene5040KarlLayer);
	_suspendKarlIdle = previousSuspendKarlIdle;
	return completed;
}

bool Scene5040::playKarlTransition(byte firstFrame, byte lastFrame, byte finalFrame,
		uint32 frameMillis) {
	const bool previousSuspendKarlIdle = _suspendKarlIdle;
	_suspendKarlIdle = true;
	_sceneLayers.setLayerVisible(kScene5040KarlLayer, true);
	const bool completed = playAndPresentAnimationTransition(kScene5040KarlLayer,
		AnimationTransition(firstFrame, lastFrame, finalFrame, frameMillis));
	_karlIdleFrame = _sceneLayers.layerFrame(kScene5040KarlLayer);
	_suspendKarlIdle = previousSuspendKarlIdle;
	return completed;
}

void Scene5040::settleKarlForConversation() {
	if (_karlIdleMode == kScene5040KarlIdleModeWait)
		return;

	const bool previousSuspendKarlIdle = _suspendKarlIdle;
	_suspendKarlIdle = true;
	if (_karlIdleMode == kScene5040KarlIdleModeToolExposedWait) {
		_karlIdleFrame = 0x1f;
		_sceneLayers.setLayerFrame(kScene5040KarlLayer, _karlIdleFrame);
		_karlIdleMode = kScene5040KarlIdleModeHideTool;
	}

	if (_karlIdleMode == kScene5040KarlIdleModeShort)
		playKarlFrames(_karlIdleFrame, 0x13, kScene5040FrameMillis);
	else
		playKarlFrames(_karlIdleFrame, 0x29, kScene5040FrameMillis);

	_karlIdleFrame = 0;
	_karlIdleMode = kScene5040KarlIdleModeWait;
	_karlIdleChannel.reset(0, kScene5040FrameMillis);
	_sceneLayers.setLayerFrame(kScene5040KarlLayer, 0);
	_vm->gameState().scene5040LooseObjectTaken = true;
	applySceneStateToHotspotsAndPatches(2);
	drawPlayableComposite();
	presentFrame();
	_suspendKarlIdle = previousSuspendKarlIdle;
}

void Scene5040::runMineCartEntryClip() {
	Common::Array<byte> frameMap;
	frameMap.resize(kScene5040MineCartDescriptorCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;

	runActorReplacement(ActionOverlaySpec(17, kScene5040MineCartDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5040MineCartFrameMillis)
		.fadeFromBlackAt(0)
		.commitAt(0x32, _mineCartRumbleActive, false)
		.soundAt(0x32, 0x16));
}

void Scene5040::runExitToMineSwitches() {
	walkActiveActorTo(0x32d, 0x110, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5040::runExitToMineHole() {
	_vm->gameState().mainFlowStateId = kScene5050EntryFromKarlGalleryState;
}

void Scene5040::runKarlConversation() {
	GameplayState &state = _vm->gameState();
	if (state.scene5040MineGalleryState == 1) {
		beginSecondarySpeechLine(2, 1);
		return;
	}
	if (state.scene5040MineGalleryState == 2) {
		beginSecondarySpeechLine(2, 0);
		return;
	}

	Common::Array<DialogueChoiceRecord> records;
	initializeKarlDialogueRecords(records);

	if (!state.scene5040KarlDialogueIntroSeen) {
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 0);
		settleKarlForConversation();
		_suspendKarlIdle = true;
		beginKarlSpeechLine(0);
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 1);
		beginKarlSpeechLine(1);
		if (_vm->restoredContentEnabled()) {
			beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 2);
			beginKarlSpeechLine(2);
		}
		state.scene5040KarlDialogueIntroSeen = true;
		applySceneStateToHotspotsAndPatches(5);
	} else {
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 0);
		settleKarlForConversation();
		_suspendKarlIdle = true;
		beginKarlSpeechLine(0);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5040KarlDialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 7);
			beginKarlSpeechLine(7);
			break;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5040DialogueNoResponseFrame)
			beginKarlSpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}

		if (applyKarlDialogueTransition(record, depthIndex, nodeIndex))
			break;
	}
	_suspendKarlIdle = false;
}

void Scene5040::runMineBoxLook() {
	GameplayState &state = _vm->gameState();
	switch (state.scene5040DialState) {
	case 0:
	case 1:
		beginSecondarySpeechLine(10, 0);
		state.scene5040DialState = 1;
		applySceneStateToHotspotsAndPatches(4);
		break;
	case 2:
	case 3:
		beginSecondarySpeechLine(10, 1);
		state.scene5040DialState = 3;
		applySceneStateToHotspotsAndPatches(4);
		break;
	default:
		beginSecondarySpeechLine(10, 2);
		break;
	}
}

void Scene5040::runPatchedSockPickup() {
	GameplayState &state = _vm->gameState();
	runActorReplacement(ActionOverlaySpec(10, kScene5040MineBoxPickupDescriptorCount,
		kScene5040FrameMillis).bookendWithLastFrame());
	state.scene5040DialState = 2;
	applySceneStateToHotspotsAndPatches(4);
	addInventoryItem(kScene5040PatchedSockItem);
	_soundBank0.playSample(1, 100);
	drawPlayableComposite();
	presentFrame();
}

void Scene5040::runMineKeyPickup() {
	GameplayState &state = _vm->gameState();
	runActorReplacement(ActionOverlaySpec(10, kScene5040MineBoxPickupDescriptorCount,
		kScene5040FrameMillis).bookendWithLastFrame());
	state.scene5040DialState = 4;
	applySceneStateToHotspotsAndPatches(4);
	addInventoryItem(kScene5040KeyItem);
	_soundBank0.playSample(1, 100);
	drawPlayableComposite();
	presentFrame();
}

void Scene5040::runSpecialMineExitWithMagneticPillbox() {
	GameplayState &state = _vm->gameState();
	walkActiveActorTo(0x1bf, 0x14f, 5, 0, false);
	if (_karlIdleMode != kScene5040KarlIdleModePausedAtWall) {
		walkActiveActorTo(0x1bf, 0x14f, 3, 0, false);
		beginSecondarySpeechLine(15, 2);
		return;
	}

	_suspendKarlIdle = true;
	runActorReplacement(ActionOverlaySpec(12, kScene5040ItemSwapDescriptorCount,
		kScene5040FrameMillis).bookendWithLastFrame().reverse());
	removeInventoryItem(kScene5040MagneticBombPillboxItem);
	_soundBank0.playSample(1, 100);
	walkActiveActorTo(0x230, 0x16b, 3, 0, false);
	beginSecondarySpeechLine(13, 0);
	walkActiveActorTo(0x32d, 0x110, 0xff, 0xff, false);
	_soundBank0.playSample(0x15, 100);
	state.scene5040SpecialTransitionState = 1;
	state.scene5040MineGalleryState = 2;
	state.scene5010MineTransportState = 4;
	state.mainFlowStateId = kScene5010ReturnState;
	_suspendKarlIdle = false;
}

void Scene5040::runDowsingRodSwap() {
	GameplayState &state = _vm->gameState();
	if (state.dowsingRodKarlExchangeState == 0) {
		dispatchGenericSceneAction(231);
		return;
	}
	if (state.dowsingRodKarlExchangeState == 2) {
		beginSecondarySpeechLine(15, 1);
		return;
	}

	walkActiveActorTo(0x16b, 0x133, 1, 0, false);
	if (_karlIdleMode != kScene5040KarlIdleModeToolExposedWait) {
		walkActiveActorTo(0x230, 0x16b, 5, 0, false);
		beginSecondarySpeechLine(15, 2);
		return;
	}

	state.dowsingRodKarlExchangeState = 2;
	_suspendKarlIdle = true;
	_karlIdleFrame = 0x1e;
	_sceneLayers.setLayerFrame(kScene5040KarlLayer, _karlIdleFrame);
	runActorReplacement(ActionOverlaySpec(11, kScene5040ItemSwapDescriptorCount,
		kScene5040DowsingRodSwapFrameMap, ARRAYSIZE(kScene5040DowsingRodSwapFrameMap),
		kScene5040MineCartFrameMillis));
	walkActiveActorTo(0x230, 0x16b, 5, 0, false);
	beginSecondarySpeechLine(15, 0);
	playKarlFrames(0x1f, 0x2e, kScene5040FrameMillis);
	beginKarlMiningSpeechLine(3);
	playKarlTransition(_karlIdleFrame, 0x38, 0x39, kScene5040FrameMillis);
	_karlIdleMode = kScene5040KarlIdleModeStrikeLoop;
	_karlStrikeRepeatCount = 10;
	_karlIdleChannel.reset(_karlIdleFrame, kScene5040FrameMillis);
	_suspendKarlIdle = false;
	walkActiveActorTo(_activeActorWorldX, _activeActorWorldY, 3, 0, false);

	state.scene5040MineGalleryState = 1;
	state.scene5040LooseObjectTaken = true;
	state.scene5040OldSockTaken = true;
	applySceneStateToHotspotsAndPatches(0xff);
	beginSecondarySpeechLine(15, 4);
}

void Scene5040::runRestoredDowsingRodOffer() {
	if (!_vm->restoredContentEnabled()) {
		dispatchGenericSceneAction(13);
		return;
	}

	if (_vm->gameState().dowsingRodKarlExchangeState == 2) {
		beginSecondarySpeechLine(16, 2);
		return;
	}

	beginSecondarySpeechLine(16, 0);
	settleKarlForConversation();
	const bool previousSuspendKarlIdle = _suspendKarlIdle;
	_suspendKarlIdle = true;
	beginKarlRestoredDowsingRodReply();
	_suspendKarlIdle = previousSuspendKarlIdle;
}

void Scene5040::initializeKarlDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5040KarlDialogueChoiceRecordCount);

	// Karl Hecker's root choices.
	setKarlDialogueRecord(records, 0, 0, kScene5040DialogueTransitionDown, 3, 3, 1);
	setKarlDialogueRecord(records, 1, 0, kScene5040DialogueTransitionStay, 4, 4, 1);
	setKarlDialogueRecord(records, 2, 0, kScene5040DialogueTransitionStay, 5, 5, 1);
	setKarlDialogueRecord(records, 3, 0, kScene5040DialogueTransitionStay, 6, 6, 1);
	setKarlDialogueRecord(records, 4, 0, kScene5040DialogueTransitionEnd, 7, 7, 0);

	// Depth 1, node 0: follow-up choices after Karl explains he is looking for the diamond.
	setKarlDialogueRecord(records, 70, 0, kScene5040DialogueTransitionStay, 8, 8, 1);
	setKarlDialogueRecord(records, 71, 0, kScene5040DialogueTransitionStay, 9, 9, 1);
	setKarlDialogueRecord(records, 72, 0, kScene5040DialogueTransitionStay, 10, 10, 1);
	setKarlDialogueRecord(records, 73, 0, kScene5040DialogueTransitionUp, 11, 11, 0);
}

void Scene5040::setKarlDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = 1;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.selectable = 1;
}

bool Scene5040::applyKarlDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const {
	const byte previousDepth = depthIndex;
	switch (record.transitionMode) {
	case kScene5040DialogueTransitionEnd:
		return true;
	case kScene5040DialogueTransitionDown:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth + 1;
		break;
	case kScene5040DialogueTransitionUp:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth == 0 ? 0 : (byte)(previousDepth - 1);
		break;
	case kScene5040DialogueTransitionUpTwo:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
		break;
	default:
		break;
	}

	return false;
}

void Scene5040::beginKarlSpeechLine(byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(kScene5040KarlPrimaryRow, frameIndex,
		0x1b7, 0x067, 0x20, 0x30, 0x3f, kScene5040KarlDialogueSpeechGroup);
}

void Scene5040::beginKarlRestoredDowsingRodReply() {
	beginPrimarySpeechLineWithAnimationGroup(16, 1,
		0x1b7, 0x067, 0x20, 0x30, 0x3f, kScene5040KarlDialogueSpeechGroup);
}

void Scene5040::beginKarlMiningSpeechLine(byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(15, frameIndex,
		0x182, 0x087, 0x20, 0x30, 0x3f, kScene5040KarlMiningSpeechGroup);
}

void Scene5040::copyStageSmallRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene5040::remapSceneColors(byte sourceColor, byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == sourceColor)
			_paletteMask[kSceneColorToItemMap + color] = itemId;
	}
}

void Scene5040::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
