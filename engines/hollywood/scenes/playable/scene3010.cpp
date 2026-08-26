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

#include "hollywood/scenes/playable/scene3010.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const uint16 kScene3010FirstState = 0x0bc2;
const uint16 kScene3010EntryFromScene3020State = 0x0bc3;
const uint16 kScene3010EntryFromPathState = 0x0bc4;
const uint16 kScene3020State = 0x0bcc;
const uint16 kScene3040State = 0x0be0;
const uint16 kScene3050State = 0x0bea;
const uint16 kScene3080EntryFromScene3010State = 0x0c08;
const uint16 kScene3010ViewportXOffset = 0x0078;
const uint kScene3010ActorBankTableEntry = 0x0000;
const uint kScene3010ActorPaletteTableEntry = 0x00cc;
const uint kScene3010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3010SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3010WindmillFrameMillis = 125;
const uint32 kScene3010ForestIdleFrameMillis = 75;
const uint kScene3010WindmillDescriptorCount = 0x1e;
const uint kScene3010ForestIdleDescriptorCount = 0x0f;
const uint kScene3010ExitDescriptorCount = 6;
const uint kScene3010DepartureFirstChunk = 10;
const uint kScene3010DepartureFirstFrameCount = 0x76;
const uint kScene3010DepartureSecondChunk = 11;
const uint kScene3010DepartureSecondFrameCount = 0x51;
const uint32 kScene3010DeparturePauseMillis = 2000;
const uint kScene3010ClimbChunk = 13;
const uint kScene3010ClimbDescriptorCount = 0x12;
const byte kScene3010InitialWindmillFrame = 4;

const byte kScene3010WindmillFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

const byte kScene3010ForestIdleFrameMap[] = {
	0, 1, 1, 1, 1, 1, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11, 12, 13, 14
};

const byte kScene3010ExitFrameMap[] = { 1, 2, 3, 4, 5 };

const byte kScene3010ClimbFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17
};

static PlayableSceneConfig scene3010Config() {
	PlayableSceneConfig config(3010,
		SceneResourceLayout(14, 5, 13),
		SceneViewport(kScene3010ViewportXOffset),
		SceneActorPose(0x160, 0x1ca, 1));
	config.setActorResources(kScene3010ActorBankTableEntry, kScene3010ActorPaletteTableEntry);
	config.setTextResources(kScene3010Resource003RowsOffsetIndex, kScene3010SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene3010::Scene3010(HollywoodEngine *vm) :
		PlayableScene(vm, scene3010Config()),
		_windmillChannel(),
		_forestIdleChannel(),
		_windmillLayer(),
		_forestIdleLayer(),
		_forestIdleState(0),
		_climbOverlayActive(false) {
	_windmillLayer.configure(7, kScene3010WindmillDescriptorCount,
		kScene3010WindmillFrameMap, ARRAYSIZE(kScene3010WindmillFrameMap));
	_forestIdleLayer.configure(9, kScene3010ForestIdleDescriptorCount,
		kScene3010ForestIdleFrameMap, ARRAYSIZE(kScene3010ForestIdleFrameMap));
}

bool Scene3010::shouldLoadArenaChunk(uint index) const {
	return index < 10 || index == 13;
}

void Scene3010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	switch (_vm->gameState().mainFlowStateId) {
	case kScene3010EntryFromScene3020State:
		_activeActorWorldX = 0x278;
		_activeActorWorldY = 0x15d;
		_activeActorFacing = 4;
		break;
	case kScene3010EntryFromPathState:
		_activeActorWorldX = 0x129;
		_activeActorWorldY = 0x13f;
		_activeActorFacing = 2;
		break;
	default:
		_activeActorWorldX = 0x160;
		_activeActorWorldY = 0x1ca;
		_activeActorFacing = 1;
		break;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene3010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	(void)activeWorldX;

	copyBaseFramebufferToSceneFramebuffer();
	if (_climbOverlayActive) {
		drawResourceSpriteLayer(_windmillLayer);
		drawActionOverlayLayer();
		drawResourceSpriteLayer(_forestIdleLayer);
		return;
	}
	if (_actionOverlayPlayer.replacesActor()) {
		drawForegroundBlocks(activeWorldY);
		restoreResourceSpriteLayerBackground(_actionOverlayPlayer.layer, _baseFramebuffer);
		drawResourceSpriteLayer(_windmillLayer);
		drawActionOverlayLayer();
		drawResourceSpriteLayer(_forestIdleLayer);
		return;
	}

	drawResourceSpriteLayer(_windmillLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
	drawResourceSpriteLayer(_forestIdleLayer);
}

void Scene3010::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene3010FirstState:
		runEntryFromChapterStart();
		break;
	case kScene3010EntryFromScene3020State:
		runEntryFromScene3020();
		break;
	case kScene3010EntryFromPathState:
		runEntryFromPath();
		break;
	default:
		drawPlayableComposite();
		presentFrame();
		break;
	}
}

bool Scene3010::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene3010::advanceCustomGameplayLoop(uint32 delta) {
	if (_vm->gameState().windmillBladesMoving)
		advanceWindmillLayer(delta);
	advanceForestIdleLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3010::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta de la casa de Frankenstein (look at the front door).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar/abrir puerta de la casa de Frankenstein (use/open front door): enter scene 3050.
		runExitToScene3050();
		return true;
	case 303: // Mirar aspas (look at windmill blades): blocked/stopped state-aware line.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar aspas/molino (use blades/windmill): Ron has no idea yet.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Ir al bosque (go to the forest): play the departure and enter scene 3020.
		runExitToScene3020();
		return true;
	case 306: // Mirar molino (look at windmill).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Ir por el camino del bosque (take the forest path): enter scene 3080.
		runExitToScene3080();
		return true;
	case 308: // Mirar bosque (look at forest): dense forest.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Coger hacha (take axe): too large to handle.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Mirar hacha (look at axe): Frankie's axe.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Usar paraguas con aspas (use umbrella with blades): climb to scene 3040.
		runUmbrellaClimb();
		return true;
	default:
		return false;
	}
}

AmbientAudioProfile Scene3010::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3010::resetAnimationLayers() {
	_windmillChannel.reset(kScene3010InitialWindmillFrame, kScene3010WindmillFrameMillis);
	_forestIdleChannel.reset(0, kScene3010ForestIdleFrameMillis);
	_forestIdleState = 0;
	_windmillLayer.visible = true;
	_forestIdleLayer.visible = true;
	_windmillLayer.reset(kScene3010InitialWindmillFrame);
	_forestIdleLayer.reset(0);
}

void Scene3010::advanceWindmillLayer(uint32 delta) {
	const uint frameCount = _windmillChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_windmillChannel.frameIndex = _windmillChannel.frameIndex + 1 < ARRAYSIZE(kScene3010WindmillFrameMap) ?
			_windmillChannel.frameIndex + 1 : 0;
		_windmillLayer.setFrame(_windmillChannel.frameIndex);
	}
}

void Scene3010::advanceForestIdleLayer(uint32 delta) {
	const uint frameCount = _forestIdleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_forestIdleState == 0) {
			if (_forestIdleChannel.frameIndex == 0 && _random.getRandomNumber(999) == 0) {
				_forestIdleState = 1;
				_forestIdleChannel.frameIndex = 3;
			} else if (_random.getRandomNumber(24) == 0) {
				_forestIdleState = 2;
				_forestIdleChannel.frameIndex = 1;
			}
		} else if (_forestIdleState == 1) {
			if (_forestIdleChannel.frameIndex < 20)
				++_forestIdleChannel.frameIndex;
			else
				_forestIdleState = 3;
		} else if (_forestIdleState == 2) {
			if (_forestIdleChannel.frameIndex < 6) {
				++_forestIdleChannel.frameIndex;
			} else {
				_forestIdleState = 0;
				_forestIdleChannel.frameIndex = 0;
			}
		}
		_forestIdleLayer.setFrame(_forestIdleChannel.frameIndex);
	}
}

void Scene3010::drawForegroundBlocks(int activeWorldY) {
	const uint chunkIndex = activeWorldY < 0x14c ? 6 : 5;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene3010::runEntryFromChapterStart() {
	runEntryPath(0x13d, 0x1df, 1, 0x160, 0x1ca);
	if (!_vm->gameState().scene3010EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene3010EntryLineSeen = true;
	}
}

void Scene3010::runEntryFromScene3020() {
	runEntryPath(0x278, 0x15d, 4, 0x278, 0x15d);
}

void Scene3010::runEntryFromPath() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x0ef, 0x139, 2, 0x129, 0x13f);
}

void Scene3010::runExitToScene3050() {
	runActorReplacement(ActionOverlaySpec(8, kScene3010ExitDescriptorCount, kScene3010ExitFrameMap,
		ARRAYSIZE(kScene3010ExitFrameMap), kScene3010ForestIdleFrameMillis)
		.noRedrawAtEnd());
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene3050State;
}

void Scene3010::runExitToScene3020() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(4, state.scene3010ForestExitSeen ? 1 : 0);
	runDepartureTransition(true);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	state.scene3010ForestExitSeen = true;
	state.mainFlowStateId = kScene3020State;
}

void Scene3010::runExitToScene3080() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(6, state.scene3080EntryLineSeen ? 1 : 0);
	runDepartureTransition(false);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	state.mainFlowStateId = kScene3080EntryFromScene3010State;
}

void Scene3010::runUmbrellaClimb() {
	GameplayState &state = _vm->gameState();
	if (!state.scene3030MachineActivated) {
		beginSecondarySpeechLine(10, 2);
		return;
	}

	beginSecondarySpeechLine(10, 0);
	runDepartureTransition(true);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	runWindmillClimbOverlay();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	state.mainFlowStateId = kScene3040State;
}

void Scene3010::runDepartureTransition(bool includeSecondClip) {
	Graphics::ManagedSurface transitionBackground;
	transitionBackground.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(true, _activeActorFacing, _activeActorCel,
		_activeActorWorldX, _activeActorWorldY, false, 0, 0, 0, 0, -1);
	drawForegroundBlocks(_activeActorWorldY);
	transitionBackground.copyRectToSurface(_sceneFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	bool skipWaits = false;
	Common::Array<byte> clipData;
	if (loadVariableChunk(kScene3010DepartureFirstChunk, clipData))
		skipWaits = playDepartureClip(clipData, kScene3010DepartureFirstFrameCount,
			transitionBackground, skipWaits, 0);

	if (includeSecondClip) {
		clipData.clear();
		if (loadVariableChunk(kScene3010DepartureSecondChunk, clipData))
			playDepartureClip(clipData, kScene3010DepartureSecondFrameCount,
				transitionBackground, skipWaits, kScene3010DeparturePauseMillis);
	}
}

bool Scene3010::playDepartureClip(const Common::Array<byte> &clipData, uint tableEntryCount,
		Graphics::ManagedSurface &transitionBackground, bool skipWaits, uint32 firstFrameHoldMillis) {
	if (tableEntryCount == 0)
		return skipWaits;

	drawDepartureFrame(clipData, tableEntryCount, 0, transitionBackground, true);
	bool waitBeforeNextFrame = firstFrameHoldMillis == 0;
	if (!skipWaits && firstFrameHoldMillis != 0)
		skipWaits = waitDepartureFrame(firstFrameHoldMillis, clipData, tableEntryCount, 0,
			transitionBackground);

	for (uint frame = 1; frame < tableEntryCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		if (!skipWaits && waitBeforeNextFrame)
			skipWaits = waitDepartureFrame(kScene3010ForestIdleFrameMillis, clipData,
				tableEntryCount, (byte)(frame - 1), transitionBackground);
		waitBeforeNextFrame = true;
		drawDepartureFrame(clipData, tableEntryCount, (byte)frame, transitionBackground, true);
	}

	return skipWaits;
}

bool Scene3010::waitDepartureFrame(uint32 millis, const Common::Array<byte> &clipData,
		uint tableEntryCount, byte frameIndex, Graphics::ManagedSurface &transitionBackground) {
	uint32 remaining = millis;
	uint32 redrawAccumulator = 0;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		if (_vm->gameState().windmillBladesMoving)
			advanceWindmillLayer(slice);
		advanceForestIdleLayer(slice);
		remaining -= slice;
		redrawAccumulator += slice;
		if (remaining != 0 && redrawAccumulator >= kScene3010ForestIdleFrameMillis) {
			drawDepartureFrame(clipData, tableEntryCount, frameIndex, transitionBackground, false);
			redrawAccumulator = 0;
		}
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene3010::drawDepartureFrame(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::ManagedSurface &transitionBackground, bool applyFrame) {
	if (applyFrame) {
		ResourceDeltaClipPlayer::drawFrame(clipData, 0, clipData.size(), tableEntryCount,
			frameIndex, (byte *)transitionBackground.getPixels(), transitionBackground.w,
			transitionBackground.h, transitionBackground.pitch,
			transitionBackground.pitch * transitionBackground.h);
	}

	_sceneFramebuffer.copyRectToSurface(transitionBackground.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	drawResourceSpriteLayer(_windmillLayer);
	drawResourceSpriteLayer(_forestIdleLayer);
	ResourceDeltaClipPlayer::drawFrame(clipData, 0, clipData.size(), tableEntryCount,
		frameIndex, (byte *)_sceneFramebuffer.getPixels(), _sceneFramebuffer.w,
		_sceneFramebuffer.h, _sceneFramebuffer.pitch,
		_sceneFramebuffer.pitch * _sceneFramebuffer.h);
	presentFrame();
}

void Scene3010::runWindmillClimbOverlay() {
	_climbOverlayActive = true;
	const bool previousHideActiveActor = _actionOverlayPlayer.beginActorReplacement(kScene3010ClimbChunk,
		kScene3010ClimbDescriptorCount, kScene3010ClimbFrameMap,
		ARRAYSIZE(kScene3010ClimbFrameMap));

	for (uint frame = 0; frame < 7 && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		_actionOverlayPlayer.setFrame(frame);
		drawPlayableComposite();
		presentFrame();
		if (waitSceneMillis(kScene3010ForestIdleFrameMillis, false))
			break;
	}

	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		_actionOverlayPlayer.setFrame(7);
		drawPlayableComposite();
		presentFrame();
		while (_vm->gameState().windmillBladesMoving &&
				_windmillChannel.frameIndex != 9 && _windmillChannel.frameIndex != 24 &&
				!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
			if (waitSceneMillis(10, false))
				break;
		}
	}

	for (uint frame = 8; frame + 1 < ARRAYSIZE(kScene3010ClimbFrameMap) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
		_actionOverlayPlayer.setFrame(frame);
		drawPlayableComposite();
		presentFrame();
		if (waitSceneMillis(kScene3010WindmillFrameMillis, false))
			break;
	}

	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		_actionOverlayPlayer.setFrame(ARRAYSIZE(kScene3010ClimbFrameMap) - 1);
		drawPlayableComposite();
		presentFrame();
	}
	_actionOverlayPlayer.finish(previousHideActiveActor);
	_climbOverlayActive = false;
}

} // End of namespace Hollywood
