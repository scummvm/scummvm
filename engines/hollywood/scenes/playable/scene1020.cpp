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

#include "hollywood/scenes/playable/scene1020.h"

#include "common/debug.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint kScene1020ArenaFirstChunk = 5;
const uint kScene1020ArenaLastChunk = 22;
const uint16 kScene1020RightEntryState = 0x03fc;
const uint16 kScene1020OverlayEntryState = 0x03fd;
const uint16 kScene1020ExitState1010RightEntry = 0x03f3;
const uint16 kScene1020ViewportXOffset = 0x0010;
const uint kScene1020ActorBankTableEntry = 0x0000;
const uint kScene1020ActorPaletteTableEntry = 0x00cc;
const uint kScene1020Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1020SpeechCueDescriptorTableOffset = 0x1135;
const int kScene1020DefaultActorX = 0x116;
const int kScene1020DefaultActorY = 0x143;
const byte kScene1020DefaultActorFacing = 2;
const int kScene1020RightEntryStartX = 0x050;
const int kScene1020RightEntryStartY = 0x108;
const int kScene1020RightEntryTargetX = 0x116;
const int kScene1020RightEntryTargetY = 0x143;
const byte kScene1020RightEntryFacing = 2;
const int kScene1020OverlayEntryX = 0x18d;
const int kScene1020OverlayEntryY = 0x155;
const byte kScene1020OverlayEntryFacing = 5;
const byte kScene1020FirstAmbientSoundCue = 0x25;
const byte kScene1020AmbientSoundCueCount = 7;
const byte kScene1020FirstAmbientMusicCue = 0x0b;
const byte kScene1020AmbientMusicCueCount = 5;
const byte kScene1020AmbientSoundProbabilityModulus = 25;
const byte kScene1020AmbientMusicProbabilityModulus = 50;
const uint32 kScene1020OverlayFrameMillis = 75;
// The cutscene is voiced from stage 107 row 0x10, not this room's own stage 102.
const char *const kScene1020TextArchiveName = "RESOURCE.003";
const uint kScene1020CutsceneStageIndex = 107;
const byte kScene1020CutsceneSpeechRow = 0x10;
const byte kScene1020QuasimodoSpeechRed = 0x20;
const byte kScene1020QuasimodoSpeechGreen = 0x3f;
const byte kScene1020QuasimodoSpeechBlue = 0;
// Quasimodo speaks from the winch until he lets go, then from where he ends up.
const uint16 kScene1020QuasimodoWinchSpeechCenterX = 0x118;
const uint16 kScene1020QuasimodoWinchSpeechTopY = 0x0a5;
const uint16 kScene1020QuasimodoExitSpeechCenterX = 0x104;
const uint16 kScene1020QuasimodoExitSpeechTopY = 0x08c;
// The grate slams up on this frame of the lift: sound, then a viewport shake.
const byte kScene1020CutsceneHookId = 1;
const uint kScene1020GrateSlamFrame = 6;
const byte kScene1020GrateLiftSoundId = 0x0d;
const uint kScene1020GrateShakeSteps = 0x14;
const uint16 kScene1020GrateShakeOffset = 4;
const uint32 kScene1020GrateShakeStepMillis = 4;
const byte kScene1020LeverSoundId = 0x2d;
const byte kScene1020JammedRailSoundId = 0x37;
const byte kScene1020ChainMotorSoundId = 0x2f;
const byte kScene1020AttachChainSoundId = 0x31;
const byte kScene1020GreaseSoundId = 0x2e;
const byte kScene1020JammedRailHookId = 2;
const byte kScene1020ChainLoopHookId = 3;
const byte kScene1020ChainForwardHookId = 4;
const byte kScene1020AttachChainHookId = 5;
const byte kScene1020GreaseHookId = 6;
// The lift pauses on this frame for the closing exchange, then resumes from it.
const uint kScene1020LiftPauseFrame = 8;
// Quasimodo is a sprite layer of his own, not part of the lift frames: upright at the
// rope (0-8), bent over it (8-16), and the frame map's tail walks him back upright.
const uint kScene1020ActionChunk20DescriptorCount = 0x11;
const byte kScene1020QuasimodoUprightSpeechGroup = 1;
const byte kScene1020QuasimodoBentSpeechGroup = 2;
const byte kScene1020QuasimodoHiddenFrame = 0;
const byte kScene1020QuasimodoUprightFrame = 4;
const byte kScene1020QuasimodoBentEnterFrame = 8;
const byte kScene1020QuasimodoBentFrame = 0x0c;
const byte kScene1020QuasimodoBentLeaveFrame = 0x10;
const byte kScene1020QuasimodoUprightRestoreFrame = 0x14;
const uint kScene1020ActionChunk14DescriptorCount = 6;
const uint kScene1020ActionChunk15DescriptorCount = 0x15;
const uint kScene1020ActionChunk16DescriptorCount = 0x0b;
const uint kScene1020ActionChunk17DescriptorCount = 0x15;
const uint kScene1020ActionChunk18DescriptorCount = 0x12;
const uint kScene1020ActionChunk19DescriptorCount = 0x0b;
const uint kScene1020ActionChunk21DescriptorCount = 0x17;
const uint kScene1020ActionChunk22DescriptorCount = 0x0d;

const byte kScene1020Chunk14ForwardFrameMap[] = {
	0, 1, 2, 3, 4, 5
};

const byte kScene1020Chunk14ReverseFrameMap[] = {
	0, 4, 3, 2, 1, 0
};

const byte kScene1020Chunk15PingPongFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 20, 20,
	20, 20, 19, 18, 17, 16, 15, 14,
	13, 12, 11, 10, 9, 8, 7, 6,
	5, 4, 3, 2, 1
};

const byte kScene1020Chunk16AlternatingFrameMap[] = {
	0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0
};

const byte kScene1020Chunk16ForwardFrameMap[] = {
	0, 0, 2, 3, 4, 5, 6, 7,
	8, 9, 10
};

const byte kScene1020Chunk17ForwardFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20
};

const byte kScene1020Chunk18StateChangeFrameMap[] = {
	0, 0, 1, 2, 3, 4, 17, 16,
	15, 14, 13, 12, 11, 8, 9, 10,
	9, 8, 9, 10, 9, 8, 9, 10,
	9, 8, 9, 10, 9, 8, 9, 10,
	9, 8, 9, 10, 9, 8, 4, 3,
	2, 1, 0
};

const byte kScene1020Chunk19EventFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 9, 8, 9, 10, 9,
	8, 9, 10, 9, 8, 9, 10, 9,
	8, 9, 10, 9, 8, 9, 10, 9,
	8, 7, 6, 5, 4, 3, 2, 1,
	0
};

const byte kScene1020Chunk20SpeakerFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 11, 10, 9, 4
};

const byte kScene1020Chunk21GrateLiftFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22
};

const byte kScene1020Chunk22PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12
};

// Chunks 15/16/17 animate the hook and 21 lifts the grate off it, so they repaint the
// state blocks applyResourceBlockBackground() bakes into _baseFramebuffer: clean their
// dirty rect to the pristine background or the baked-in copy shows through as a
// duplicate. Every other overlay must clean to _baseFramebuffer or those blocks vanish.
static bool overlayRedrawsSceneStateBlocks(uint chunkIndex) {
	return (chunkIndex >= 15 && chunkIndex <= 17) || chunkIndex == 21;
}

static PlayableSceneConfig scene1020Config() {
	PlayableSceneConfig config(1020,
		SceneResourceLayout(23, 5, 22),
		SceneViewport(kScene1020ViewportXOffset),
		SceneActorPose(kScene1020DefaultActorX, kScene1020DefaultActorY, kScene1020DefaultActorFacing));
	config.setActorResources(kScene1020ActorBankTableEntry, kScene1020ActorPaletteTableEntry);
	config.setTextResources(kScene1020Resource003RowsOffsetIndex, kScene1020SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 2;
	config.useActorDepthTest = true;
	return config;
}

Scene1020::Scene1020(HollywoodEngine *vm) :
		PlayableScene(vm, scene1020Config()) {
}

bool Scene1020::shouldLoadArenaChunk(uint index) const {
	if (index < kScene1020ArenaFirstChunk || index > kScene1020ArenaLastChunk)
		return false;
	if (index <= 13)
		return true;
	if (isFirstEntryState())
		return index >= 14 && index <= 19;
	return index >= 20 && index <= 22;
}

void Scene1020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();

	if (isFirstEntryState()) {
		_activeActorWorldX = kScene1020RightEntryTargetX;
		_activeActorWorldY = kScene1020RightEntryTargetY;
		_activeActorFacing = kScene1020RightEntryFacing;
	} else if (isSpecialOverlayEntryState()) {
		_activeActorWorldX = kScene1020OverlayEntryX;
		_activeActorWorldY = kScene1020OverlayEntryY;
		_activeActorFacing = kScene1020OverlayEntryFacing;
	} else {
		_activeActorWorldX = kScene1020DefaultActorX;
		_activeActorWorldY = kScene1020DefaultActorY;
		_activeActorFacing = kScene1020DefaultActorFacing;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene1020::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_actionOverlayPlayer.isVisible()) {
		const Graphics::Surface &background = overlayRedrawsSceneStateBlocks(_actionOverlayPlayer.layer.chunkIndex) ?
			_baseFramebufferOriginal.rawSurface() : _baseFramebuffer.rawSurface();
		restoreResourceSpriteLayerBackground(_actionOverlayPlayer.layer, background);
	}
	drawActionOverlayLayer();
	drawResourceSpriteLayer(_quasimodoLayer);
}

void Scene1020::runCustomEntrySequence() {
	if (!isFirstEntryState() && !isSpecialOverlayEntryState()) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	GameplayState &state = _vm->gameState();
	if (isFirstEntryState()) {
		runEntryPath(kScene1020RightEntryStartX, kScene1020RightEntryStartY,
			kScene1020RightEntryFacing, kScene1020RightEntryTargetX, kScene1020RightEntryTargetY);
		if (!state.scene1020EntryLineSeen) {
			beginSecondarySpeechLine(0, 0);
			state.scene1020EntryLineSeen = true;
		}
		return;
	}

	setActiveActorPose(kScene1020OverlayEntryX, kScene1020OverlayEntryY, kScene1020OverlayEntryFacing);
	drawPlayableComposite();
	presentFrame();
	runQuasimodoGrateCutscene();
}

void Scene1020::runQuasimodoLayerTransition(byte fromFrame, byte toFrame) {
	_quasimodoLayer.setFrame(fromFrame);
	byte frame = fromFrame;
	while (frame != toFrame && !Engine::shouldQuit()) {
		frame = (byte)(toFrame > frame ? frame + 1 : frame - 1);
		_quasimodoLayer.setFrame(frame);
		if (waitSceneMillis(kScene1020OverlayFrameMillis))
			return;
	}
}

void Scene1020::runQuasimodoSpeechLine(byte frameIndex, bool bendToRope) {
	runQuasimodoLayerTransition(kScene1020QuasimodoHiddenFrame, kScene1020QuasimodoUprightFrame);
	if (bendToRope)
		runQuasimodoLayerTransition(kScene1020QuasimodoBentEnterFrame, kScene1020QuasimodoBentFrame);
	beginPrimarySpeechLineWithAnimationGroup(kScene1020CutsceneSpeechRow, frameIndex,
		bendToRope ? kScene1020QuasimodoExitSpeechCenterX : kScene1020QuasimodoWinchSpeechCenterX,
		bendToRope ? kScene1020QuasimodoExitSpeechTopY : kScene1020QuasimodoWinchSpeechTopY,
		kScene1020QuasimodoSpeechRed, kScene1020QuasimodoSpeechGreen, kScene1020QuasimodoSpeechBlue,
		bendToRope ? kScene1020QuasimodoBentSpeechGroup : kScene1020QuasimodoUprightSpeechGroup);
	if (bendToRope)
		runQuasimodoLayerTransition(kScene1020QuasimodoBentLeaveFrame, kScene1020QuasimodoUprightRestoreFrame);
	runQuasimodoLayerTransition(kScene1020QuasimodoUprightFrame, kScene1020QuasimodoHiddenFrame);
}

byte Scene1020::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene1020QuasimodoUprightSpeechGroup)
		return kScene1020QuasimodoUprightFrame;
	if (animationGroup == kScene1020QuasimodoBentSpeechGroup)
		return kScene1020QuasimodoBentFrame;
	return 0;
}

void Scene1020::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene1020QuasimodoUprightSpeechGroup ||
			animationGroup == kScene1020QuasimodoBentSpeechGroup)
		_quasimodoLayer.setFrame(frameIndex);
}

void Scene1020::runGrateLiftShake() {
	const uint16 baseOffset = _viewportXOffset;
	for (uint step = 0; step < kScene1020GrateShakeSteps; ++step) {
		_viewportXOffset = baseOffset + (_random.getRandomBit() != 0 ? kScene1020GrateShakeOffset : 0);
		if (waitSceneMillis(kScene1020GrateShakeStepMillis))
			break;
	}
	_viewportXOffset = baseOffset;
}

void Scene1020::runQuasimodoGrateCutscene() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020GrateRaised)
		return;

	// Borrow stage 107's text for the spoken lines and put this room's back afterwards.
	// Keep our own small rows: those are the hotspot captions, and the patch below
	// snapshots them -- stage 107's item 3 is "Spencer McDundee", not "cinta de Sue".
	const Common::Array<byte> roomHotspotCaptions = _textStore.stageSmallRows;
	const bool spoken = _textStore.load(kScene1020TextArchiveName, sceneDebugName(),
		kScene1020CutsceneStageIndex, resource003InventoryRowsOffsetIndex(),
		speechCueDescriptorTableOffset());
	_textStore.stageSmallRows = roomHotspotCaptions;
	if (!spoken) {
		warning("%s failed to load stage %u cutscene text", sceneDebugName(),
			kScene1020CutsceneStageIndex);
	}

	_quasimodoLayer.configure(20, kScene1020ActionChunk20DescriptorCount,
		kScene1020Chunk20SpeakerFrameMap, ARRAYSIZE(kScene1020Chunk20SpeakerFrameMap));
	_quasimodoLayer.reset(kScene1020QuasimodoHiddenFrame);
	_quasimodoLayer.visible = true;
	drawPlayableComposite();
	presentFrame();

	if (spoken) {
		beginSecondarySpeechLine(kScene1020CutsceneSpeechRow, 0);
		runQuasimodoSpeechLine(1, false);
	}

	// The lift frames draw Quasimodo themselves, so his layer comes down for them.
	_quasimodoLayer.visible = false;

	// noRedrawAtEnd: do not present the still-unpatched base between the halves.
	runOverlaySequence(ActionOverlaySpec(21, kScene1020ActionChunk21DescriptorCount,
		kScene1020Chunk21GrateLiftFrameMap, ARRAYSIZE(kScene1020Chunk21GrateLiftFrameMap),
		kScene1020OverlayFrameMillis)
		.endAt(kScene1020LiftPauseFrame + 1)
		.soundAt(kScene1020GrateSlamFrame, kScene1020GrateLiftSoundId)
		.hookEveryFrame(kScene1020CutsceneHookId)
		.noRedrawAtEnd());

	state.scene1020GrateRaised = true;
	if (!state.scene1020SueTapeNoticed) {
		state.scene1020SueTapeNoticed = true;
		state.scene1020SueTapeVisible = true;
	}
	applySceneStateToHotspotsAndPatches(0xff);

	if (spoken) {
		_quasimodoLayer.visible = true;
		runQuasimodoSpeechLine(2, false);
		// Frame 3 carries continuationCount 2, so it speaks both of Ron's lines.
		beginSecondarySpeechLine(kScene1020CutsceneSpeechRow, 3);
		runQuasimodoSpeechLine(4, true);
		_quasimodoLayer.visible = false;
	}

	runOverlaySequence(ActionOverlaySpec(21, kScene1020ActionChunk21DescriptorCount,
		kScene1020Chunk21GrateLiftFrameMap, ARRAYSIZE(kScene1020Chunk21GrateLiftFrameMap),
		kScene1020OverlayFrameMillis)
		.startAt(kScene1020LiftPauseFrame));

	if (spoken && !loadStage003SceneRows())
		warning("%s failed to restore stage %u text", sceneDebugName(), sceneStageIndex());
}

void Scene1020::handleAnimationFrameHook(byte hookId, uint frame) {
	GameplayState &state = _vm->gameState();
	switch (hookId) {
	case kScene1020CutsceneHookId:
		if (frame == kScene1020GrateSlamFrame)
			runGrateLiftShake();
		break;
	case kScene1020JammedRailHookId:
		if (frame == 1)
			_soundBank0.playSample(kScene1020JammedRailSoundId, 100, true);
		else if (frame == 10)
			_soundBank0.stop();
		break;
	case kScene1020ChainLoopHookId:
		if (frame == 1)
			_soundBank0.playSample(kScene1020ChainMotorSoundId, 100, true);
		else if (frame == 44)
			_soundBank0.stop();
		break;
	case kScene1020ChainForwardHookId:
		if (frame == 1)
			_soundBank0.playSample(kScene1020ChainMotorSoundId, 100, true);
		else if (frame == 21)
			_soundBank0.stop();
		break;
	case kScene1020AttachChainHookId:
		switch (frame) {
		case 5:
			beginPrimarySpeechLine(0x15, 1, 0x00aa, 0x00f5, 0x3f, 0x3f, 0x3f);
			break;
		case 12:
			if (hasInventoryItem(0x1e)) {
				removeInventoryItem(0x1e);
				_soundBank0.playSample(1, 100);
			}
			break;
		case 13:
			_soundBank0.playSample(kScene1020AttachChainSoundId, 100, true);
			break;
		case 34:
			state.scene1020ChainAttachedToGrate = 1;
			applySceneStateToHotspotsAndPatches(2);
			break;
		case 38:
			_soundBank0.stop();
			beginPrimarySpeechLine(0x15, 2, 0x00aa, 0x00f5, 0x3f, 0x3f, 0x3f);
			break;
		default:
			break;
		}
		break;
	case kScene1020GreaseHookId:
		if (frame == 8)
			_soundBank0.playSample(kScene1020GreaseSoundId, 5, true);
		else if (frame == 40)
			_soundBank0.stop();
		break;
	default:
		break;
	}
}

bool Scene1020::prepareCustomGameplayLoop() {
	return true;
}

bool Scene1020::advanceCustomGameplayLoop(uint32 delta) {
	// Returning true skips the base loop's speech advance, so drive Quasimodo's here.
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a camino/jardines (go to path/gardens): return to scene 1010.
		_vm->gameState().mainFlowStateId = kScene1020ExitState1010RightEntry;
		return true;
	case 302: // Mirar camino (look at path): leads back to the gardens.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar/usar rejilla (look/use grate): too heavy until rigged.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar cinta de Sue bajo la rejilla (look at Sue's tape under grate).
		handleSceneEventFlag0();
		return true;
	case 305: // Coger cinta de Sue (take Sue's tape): requires the grate to be raised.
		handleSceneEventFlag0Overlay();
		return true;
	case 306: // Mirar cinta/rail segun estado (look at tape/rail, state-aware).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar rail oxidado (look at rusty rail).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar gancho/cadena/barriles (look at hook/chain/barrels).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Usar gancho/rail (use hook/rail): response depends on hook position.
		handleResourceBlockChoiceSpeech();
		return true;
	case 310: // Coger/usar cuerda (take/use rope): Sue keeps it in place.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Mirar panel de mandos (look at control panel).
		beginSecondarySpeechLine(0x0a, 0);
		return true;
	case 312: // Mirar palancas/torno (look at either lever or the mechanical winch).
		beginSecondarySpeechLine(0x0b, 0);
		return true;
	case 313: // Usar palancas izquierda/derecha (use left/right levers): move hook.
		handleSceneVerb7Or8DescriptorAction();
		return true;
	case 314: // Mirar/usar cadena (look/use chain): attached to hook and motor.
		beginSecondarySpeechLine(0x0d, 0);
		return true;
	case 315: // Mirar cadena/polea (look at chain/pulley).
		beginSecondarySpeechLine(0x0e, 0);
		return true;
	case 316: // Mirar aparato roto / magnetofon roto (look at broken device/tape recorder).
		beginSecondarySpeechLine(0x0f, _vm->gameState().scene1020GrateRaised ? 1 : 0);
		return true;
	case 317: // Mirar palanca/polea (look at lever/pulley).
		beginSecondarySpeechLine(0x10, 0);
		return true;
	case 318: // Usar panel/palancas (use control panel/levers): points to both levers.
		beginSecondarySpeechLine(0x11, 0);
		return true;
	case 319: // Mirar barriles / magnetofon de Sue (look at barrels/Sue recorder).
		beginSecondarySpeechLine(0x12, 0);
		return true;
	case 320: // Mirar aparato roto tras identificarlo (look at broken recorder follow-up).
		handleSpeech19AfterEventFlag1();
		return true;
	case 321: // Mirar magnetofon roto de Sue (look at Sue's broken recorder).
		handleSceneEventFlag1Speech();
		return true;
	case 322: // Usar cadena con rejilla (use chain with grate): attach and lift setup.
		handleResourceOverlayChunk18StateChange();
		return true;
	case 323: // Usar grasa/objeto con rail oxidado (use grease/item on rusty rail).
		handleResourceOverlayChunk19EventFlag();
		return true;
	default:
		return false;
	}
}

bool Scene1020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	restoreBaseFramebufferFromOriginal();

	applyResourceBlockBackground();
	applySceneColorMapRules(0xff);
	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	GameplayState &state = _vm->gameState();
	if (!state.scene1020GrateRaised) {
		ScenePoint interactionPoint;
		ScenePoint approachPoint;
		if (state.scene1020HookPositionState == 0) {
			interactionPoint.x = 0x11e;
			interactionPoint.y = 0x14d;
			approachPoint.x = 0x11d;
			approachPoint.y = 0x14c;
		} else {
			interactionPoint.x = 0x0b7;
			interactionPoint.y = 0x18a;
			approachPoint.x = 0x0b6;
			approachPoint.y = 0x189;
		}
		_hotspots.setActionTarget(5, interactionPoint, approachPoint);
	}

	if (state.scene1020BrokenRecorderIdentified) {
		_hotspots.setVerbMovementModeByGlobalRecordIndex(99, 0);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(100, 0);
	}

	return true;
}

bool Scene1020::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)nextRegion;
	(void)state;
	(void)boundary;
	(void)requestedFacing;
	(void)restoredStepDeltas;
	return false;
}

bool Scene1020::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)requestedFacing;
	(void)restoredStepDeltas;
	return false;
}

void Scene1020::applyResourceBlockBackground() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020GrateRaised) {
		if (shouldLoadArenaChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		if (shouldLoadArenaChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
		return;
	}

	if (shouldLoadArenaChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);

	const byte choice = MIN<byte>(state.scene1020HookPositionState, 2);
	const byte variant = MIN<byte>(state.scene1020ChainAttachedToGrate, 1);
	const uint chunkIndex = 7 + choice + variant;
	if (chunkIndex <= kScene1020ArenaLastChunk && shouldLoadArenaChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _baseFramebuffer);
}

void Scene1020::applySceneColorMapRules(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	GameplayState &state = _vm->gameState();
	const byte choice = MIN<byte>(state.scene1020HookPositionState, 2);
	const byte variant = MIN<byte>(state.scene1020ChainAttachedToGrate, 1);
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
		byte item = originalItem;

		if (state.scene1020GrateRaised) {
			if (originalItem == 5 || originalItem == 0x0d || originalItem == 0x10)
				item = 1;
			else if (originalItem == 0x0e || originalItem == 0x0f)
				item = 0;
		} else if (choice == 0) {
			if (originalItem == 5)
				item = 5;
			else if (originalItem == 0x0d || originalItem == 0x10)
				item = 1;
			else if (originalItem == 0x0e)
				item = 0;
			else if (originalItem == 0x0f)
				item = 2;
		} else if (choice == 1) {
			if (originalItem == 0x0d)
				item = 5;
			else if (originalItem == 5 || originalItem == 0x10)
				item = 1;
			else if (originalItem == 0x0e)
				item = 0;
			else if (originalItem == 0x0f)
				item = 2;
		} else {
			if (originalItem == 0x0e || originalItem == 0x0f || originalItem == 0x10)
				item = 5;
			else if (originalItem == 5 || originalItem == 0x0d)
				item = 1;
		}

		if (state.scene1020GrateRaised) {
			if (originalItem == 6 || originalItem == 0x0f)
				item = 0;
		} else if (variant == 0) {
			if (originalItem == 6)
				item = 2;
		} else if (originalItem == 6 || originalItem == 0x0f) {
			item = 6;
		}

		if (state.scene1020GrateRaised && (originalItem == 2 || originalItem == 9))
			item = 0;

		if (state.scene1020SueTapeVisible) {
			if (originalItem == 3)
				item = 3;
		} else if (state.scene1020GrateRaised) {
			if (originalItem == 3)
				item = 0;
		} else if (originalItem == 3) {
			item = 2;
		}

		_paletteMask[kSceneColorToItemMap + i] = item;
	}

	if (state.scene1020GrateRaised && shouldLoadArenaChunk(12))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
	if (state.scene1020SueTapeVisible && state.scene1020GrateRaised && shouldLoadArenaChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
	if (state.scene1020BrokenRecorderIdentified)
		copyStageSmallRow(13, 12);
}

void Scene1020::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene1020::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame) {
	ActionOverlaySpec spec(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis);
	spec.patchAt(patchFrame, 0xff);
	if (chunkIndex == 14 && frameMap == kScene1020Chunk14ForwardFrameMap)
		spec.soundAt(4, kScene1020LeverSoundId, 50);
	else if (chunkIndex == 15)
		spec.hookEveryFrame(kScene1020ChainLoopHookId);
	else if (chunkIndex == 16)
		spec.hookEveryFrame(kScene1020JammedRailHookId);
	else if (chunkIndex == 17)
		spec.hookEveryFrame(kScene1020ChainForwardHookId);

	runOverlaySequence(spec);
	if (chunkIndex >= 15 && chunkIndex <= 17)
		_soundBank0.stop();
}

void Scene1020::runOverlaySequence(const ActionOverlaySpec &spec) {
	if (!shouldLoadArenaChunk(spec.chunkIndex)) {
		debugC(2, kDebugScene, "Scene1020 skipped unloaded overlay chunk %u", spec.chunkIndex);
		return;
	}

	runSceneOverlay(spec);
}

void Scene1020::handleSceneEventFlag0() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1020SueTapeVisible) {
		state.scene1020SueTapeVisible = true;
		state.scene1020SueTapeNoticed = true;
		applySceneStateToHotspotsAndPatches(4);
		beginSecondarySpeechLine(3, 0);
		return;
	}

	beginStaticSecondarySpeechLine(0x0e, 0);
}

void Scene1020::handleSceneEventFlag0Overlay() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1020GrateRaised) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	beginSecondarySpeechLine(4, 1);
	runOverlaySequence(22, kScene1020ActionChunk22DescriptorCount, kScene1020Chunk22PickupFrameMap,
		ARRAYSIZE(kScene1020Chunk22PickupFrameMap), kScene1020OverlayFrameMillis, 8);
	addInventoryItem(0x16);
	_soundBank0.playSample(1, 100);
	state.scene1020SueTapeVisible = false;
	applySceneStateToHotspotsAndPatches(4);
}

void Scene1020::handleResourceBlockChoiceSpeech() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020HookPositionState < 2)
		beginSecondarySpeechLine(0x11, 0);
	else
		beginSecondarySpeechLine(8, 2);
}

void Scene1020::handleSceneVerb7Or8DescriptorAction() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020GrateRaised) {
		dispatchGenericSceneAction(20);
		return;
	}

	if (_lastSceneActionItemId == 8) {
		if (state.scene1020HookPositionState == 0) {
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ForwardFrameMap,
				ARRAYSIZE(kScene1020Chunk14ForwardFrameMap), kScene1020OverlayFrameMillis);
			runOverlaySequence(15, kScene1020ActionChunk15DescriptorCount, kScene1020Chunk15PingPongFrameMap,
				ARRAYSIZE(kScene1020Chunk15PingPongFrameMap), kScene1020OverlayFrameMillis);
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
				ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
			beginSecondarySpeechLine(8, 2);
		} else if (state.scene1020HookPositionState == 1) {
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ForwardFrameMap,
				ARRAYSIZE(kScene1020Chunk14ForwardFrameMap), kScene1020OverlayFrameMillis);
			// Commit the move before the slide: overlay playback presents one frame from
			// the base framebuffer when it ends, so patching after flashes the old position.
			state.scene1020HookPositionState = 2;
			applySceneStateToHotspotsAndPatches(1);
			runOverlaySequence(17, kScene1020ActionChunk17DescriptorCount, kScene1020Chunk17ForwardFrameMap,
				ARRAYSIZE(kScene1020Chunk17ForwardFrameMap), kScene1020OverlayFrameMillis);
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
				ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
			beginStaticSecondarySpeechLine(0x35, 0);
		} else if (state.scene1020ChainAttachedToGrate == 0) {
			beginSecondarySpeechLine(8, 2);
		} else {
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ForwardFrameMap,
				ARRAYSIZE(kScene1020Chunk14ForwardFrameMap), kScene1020OverlayFrameMillis);
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
				ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
			beginSecondarySpeechLine(0x0c, 0);
		}
		return;
	}

	if (_lastSceneActionItemId != 7)
		return;

	if (state.scene1020HookPositionState != 0) {
		beginSecondarySpeechLine(8, 2);
		return;
	}

	runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ForwardFrameMap,
		ARRAYSIZE(kScene1020Chunk14ForwardFrameMap), kScene1020OverlayFrameMillis);
	if (!state.scene1020RustyRailGreased) {
		runOverlaySequence(16, kScene1020ActionChunk16DescriptorCount, kScene1020Chunk16AlternatingFrameMap,
			ARRAYSIZE(kScene1020Chunk16AlternatingFrameMap), kScene1020OverlayFrameMillis);
		runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
			ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
		beginSecondarySpeechLine(8, 0);
		return;
	}

	// Commit before the slide; see the 1 -> 2 branch.
	state.scene1020HookPositionState = 1;
	applySceneStateToHotspotsAndPatches(1);
	runOverlaySequence(16, kScene1020ActionChunk16DescriptorCount, kScene1020Chunk16ForwardFrameMap,
		ARRAYSIZE(kScene1020Chunk16ForwardFrameMap), kScene1020OverlayFrameMillis);
	runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
		ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
	beginSecondarySpeechLine(8, 1);
}

void Scene1020::handleSceneEventFlag1Speech() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1020BrokenRecorderIdentified) {
		state.scene1020BrokenRecorderIdentified = true;
		applySceneStateToHotspotsAndPatches(5);
		beginSecondarySpeechLine(0x14, 0);
		return;
	}

	beginSecondarySpeechLine(0x14, 1);
}

void Scene1020::handleSpeech19AfterEventFlag1() {
	if (!_vm->gameState().scene1020BrokenRecorderIdentified)
		handleSceneEventFlag1Speech();
	beginSecondarySpeechLine(0x13, 0);
}

void Scene1020::handleResourceOverlayChunk18StateChange() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020HookPositionState < 2) {
		beginSecondarySpeechLine(0x15, 0);
		return;
	}

	runOverlaySequence(ActionOverlaySpec(18, kScene1020ActionChunk18DescriptorCount,
		kScene1020Chunk18StateChangeFrameMap, ARRAYSIZE(kScene1020Chunk18StateChangeFrameMap),
		kScene1020OverlayFrameMillis).hookEveryFrame(kScene1020AttachChainHookId));
	_soundBank0.stop();
	if (hasInventoryItem(0x1e)) {
		removeInventoryItem(0x1e);
		_soundBank0.playSample(1, 100);
	}
	if (!state.scene1020ChainAttachedToGrate) {
		state.scene1020ChainAttachedToGrate = 1;
		applySceneStateToHotspotsAndPatches(2);
	}
}

void Scene1020::handleResourceOverlayChunk19EventFlag() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020RustyRailGreased) {
		beginSecondarySpeechLine(0x16, 1);
		return;
	}

	beginSecondarySpeechLine(0x16, 0);
	runOverlaySequence(ActionOverlaySpec(19, kScene1020ActionChunk19DescriptorCount,
		kScene1020Chunk19EventFrameMap, ARRAYSIZE(kScene1020Chunk19EventFrameMap),
		kScene1020OverlayFrameMillis).hookEveryFrame(kScene1020GreaseHookId));
	_soundBank0.stop();
	state.scene1020RustyRailGreased = true;
}

AmbientAudioProfile Scene1020::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1020FirstAmbientSoundCue,
		kScene1020AmbientSoundCueCount, 15, kScene1020AmbientSoundProbabilityModulus,
		kScene1020FirstAmbientMusicCue, kScene1020AmbientMusicCueCount, 100,
		kScene1020AmbientMusicProbabilityModulus);
}

bool Scene1020::isFirstEntryState() const {
	return _vm->gameState().mainFlowStateId == kScene1020RightEntryState;
}

bool Scene1020::isSpecialOverlayEntryState() const {
	return _vm->gameState().mainFlowStateId == kScene1020OverlayEntryState;
}

} // End of namespace Hollywood
