/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file for details.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hollywood/scenes/playable/scene6080.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene6070ReturnState = 0x17b7;
const uint16 kScene6070EscapeState = 0x17b9;
const uint16 kScene6080ViewportXOffset = 0x00a8;
const uint kScene6080ActorBankTableEntry = 0x0000;
const uint kScene6080ActorPaletteTableEntry = 0x00cc;
const uint kScene6080Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6080SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6080SueFrameMillis = 60;
const uint32 kScene6080GuardFrameMillis = 75;
const uint32 kScene6080SpeechFrameMillis = 125;
const uint32 kScene6080WaxBallFrameMillis = 90;
const uint32 kScene6080EscapeGuardFrameMillis = 45;
const uint32 kScene6080ActorPathFrameMillis = 60;
const byte kScene6080SueSpeechGroup = 0;
const byte kScene6080GuardSpeechGroup = 1;

const byte kScene6080SueNormalFrameMap[] = {
	0, 1, 2, 3, 10, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 8, 7, 6, 5, 4, 0};

const byte kScene6080SueAlternateFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 19, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
	62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77,
	78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91};

const byte kScene6080GuardNormalFrameMap[] = {
	0, 1, 2, 2, 1, 0, 3, 4, 5, 6, 7, 8, 9, 6, 5, 4, 3, 0};

const byte kScene6080GuardAlternateFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

const byte kScene6080WaxBallFrameMap[] = {
	0, 1, 31, 32, 33, 34, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 35, 36, 37, 36, 19, 35, 37, 37,
	35, 19, 36, 19, 35, 36, 37, 36, 19, 35, 37, 37, 35, 37, 35, 36,
	19, 35, 37, 37, 35, 19, 36, 19, 35, 36, 37, 36, 19, 35, 37, 37,
	36, 19, 36, 19, 35, 36, 37, 36, 19, 35, 37, 37, 35, 37, 35, 36,
	19, 35, 37, 37, 35, 19, 36, 19, 35, 36, 37, 36, 19, 35, 37, 37,
	36, 19, 36, 19, 35, 36, 37, 36, 19, 35, 37, 37, 35, 37, 35, 36,
	19, 35, 19, 36, 37, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	0};

static PlayableSceneConfig scene6080Config() {
	PlayableSceneConfig config(6080,
		SceneResourceLayout(10, 5, 9),
		SceneViewport(kScene6080ViewportXOffset, 0, kScene6080ViewportXOffset),
		SceneActorPose(0x2cb, 0x156, 4));
	config.setActorResources(kScene6080ActorBankTableEntry, kScene6080ActorPaletteTableEntry);
	config.setTextResources(kScene6080Resource003RowsOffsetIndex, kScene6080SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet00);
	config.walkablePaletteMaxRegion = 1;
	return config;
}

Scene6080::Scene6080(HollywoodEngine *vm) :
		PlayableScene(vm, scene6080Config()),
		_sueNormalLayer(),
		_sueAlternateLayer(),
		_guardNormalLayer(),
		_guardAlternateLayer(),
		_waxBallLayer(),
		_sueIdleChannel(),
		_guardIdleChannel(),
		_waxBallChannel(),
		_escapeSueChannel(),
		_escapeGuardChannel(),
		_manualActorPathChannel(),
		_finalSueChannel(),
		_manualActorPathFrameIndex(0),
		_sueLongIdleActive(false),
		_guardManualSequenceActive(false),
		_manualSequenceActive(false),
		_waxBallAnimationActive(false),
		_escapeLayersSwitched(false),
		_manualActorPathActive(false),
		_finalSueAnimationActive(false) {
	_sueNormalLayer.configure(5, 0x0b, kScene6080SueNormalFrameMap,
		ARRAYSIZE(kScene6080SueNormalFrameMap));
	_sueAlternateLayer.configure(9, 0x5c, kScene6080SueAlternateFrameMap,
		ARRAYSIZE(kScene6080SueAlternateFrameMap));
	_guardNormalLayer.configure(6, 0x0a, kScene6080GuardNormalFrameMap,
		ARRAYSIZE(kScene6080GuardNormalFrameMap));
	_guardAlternateLayer.configure(7, 9, kScene6080GuardAlternateFrameMap,
		ARRAYSIZE(kScene6080GuardAlternateFrameMap));
	_waxBallLayer.configure(8, 0x26, kScene6080WaxBallFrameMap,
		ARRAYSIZE(kScene6080WaxBallFrameMap));
}

void Scene6080::initializeCustomPreviewState() {
	_activeActorWorldX = 0x305;
	_activeActorWorldY = 0x14d;
	_activeActorFacing = 4;
	resetSceneLayers();
	initializeDefaultPreviewState();
}

void Scene6080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel,
		activeWorldX, activeWorldY, drawActiveActor && drawSecondaryActor, secondaryFacing,
		secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawResourceSpriteLayer(_sueNormalLayer);
	drawResourceSpriteLayer(_sueAlternateLayer);
	drawResourceSpriteLayer(_waxBallLayer);
	drawResourceSpriteLayer(_guardNormalLayer);
	drawResourceSpriteLayer(_guardAlternateLayer);
}

void Scene6080::runCustomEntrySequence() {
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;

	runEntryPath(0x305, 0x14d, 4, 0x2cb, 0x156);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	GameplayState &state = _vm->gameState();
	if (state.scene6080Visited)
		runReturnConversation();
	else
		state.scene6080Visited = true;
}

bool Scene6080::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene6080::prepareCustomGameplayLoop() {
	_sueIdleChannel.reset(_sueNormalLayer.frameIndex, kScene6080SueFrameMillis);
	_guardIdleChannel.reset(_guardNormalLayer.frameIndex, kScene6080GuardFrameMillis);
	_manualActorPathChannel.reset(0, kScene6080ActorPathFrameMillis);
	_manualSequenceActive = false;
	_manualActorPathActive = false;
	return true;
}

bool Scene6080::advanceCustomGameplayLoop(uint32 delta) {
	advanceManualActorPath(delta);

	if (_waxBallAnimationActive)
		advanceWaxBallAnimation(delta);
	else if (_finalSueAnimationActive)
		advanceFinalSueAnimation(delta);
	else if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else if (!_manualSequenceActive) {
		advanceSueIdle(delta);
		advanceGuardIdle(delta);
	}

	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6080::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Hablar con guardia (talk to the guard).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar guardia (look at the guard).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Hablar con Sue (talk to Sue).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Mirar Sue (look at Sue).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Ir al pasillo (return to the cell corridor).
		state.mainFlowStateId = kScene6070ReturnState;
		return true;
	case 306: // Usar otro objeto con guardia (use any other item on the guard).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Usar pelota de cera con guardia (use the wax ball to escape).
		runWaxBallEscapeSequence();
		return true;
	default:
		return false;
	}
}

bool Scene6080::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = 0x2cb;
	targetY = 0x156;
	return true;
}

bool Scene6080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.size() < kPaletteMaskUsedBytes)
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(),
		_fullPaletteRegionMask.size());
	for (uint i = 0; i < kPaletteMaskUsedBytes; ++i) {
		if (_paletteMaskOriginal[i] != 2)
			continue;
		_paletteMask[i] = 0;
		_fullPaletteRegionMask[i] = 1;
	}
	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

byte Scene6080::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene6080GuardSpeechGroup)
		return 9;
	return _sueAlternateLayer.visible ? 20 : 0;
}

uint32 Scene6080::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene6080SpeechFrameMillis;
}

void Scene6080::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene6080GuardSpeechGroup) {
		if (_guardAlternateLayer.visible)
			_guardAlternateLayer.setFrame(frameIndex);
		else
			_guardNormalLayer.setFrame(frameIndex);
		return;
	}

	if (_sueAlternateLayer.visible)
		_sueAlternateLayer.setFrame(frameIndex);
	else
		_sueNormalLayer.setFrame(frameIndex);
}

void Scene6080::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
}

bool Scene6080::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene6080::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene6080::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6080::resetSceneLayers() {
	_sueNormalLayer.reset(0);
	_sueNormalLayer.visible = true;
	_sueAlternateLayer.reset(0);
	_sueAlternateLayer.visible = false;
	_guardNormalLayer.reset(0);
	_guardNormalLayer.visible = true;
	_guardAlternateLayer.reset(0);
	_guardAlternateLayer.visible = false;
	_waxBallLayer.reset(0);
	_waxBallLayer.visible = false;
	_sueIdleChannel.reset(0, kScene6080SueFrameMillis);
	_guardIdleChannel.reset(0, kScene6080GuardFrameMillis);
	_waxBallChannel.reset(0, kScene6080WaxBallFrameMillis);
	_escapeSueChannel.reset(0, kScene6080SueFrameMillis);
	_escapeGuardChannel.reset(0, kScene6080EscapeGuardFrameMillis);
	_manualActorPathChannel.reset(0, kScene6080ActorPathFrameMillis);
	_finalSueChannel.reset(0, kScene6080SueFrameMillis);
	_manualActorPathFrameIndex = 0;
	_sueLongIdleActive = false;
	_guardManualSequenceActive = false;
	_manualSequenceActive = false;
	_waxBallAnimationActive = false;
	_escapeLayersSwitched = false;
	_manualActorPathActive = false;
	_finalSueAnimationActive = false;
}

void Scene6080::advanceSueIdle(uint32 delta) {
	if (!_sueNormalLayer.visible)
		return;

	const uint frameCount = _sueIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_sueLongIdleActive) {
			if (_sueNormalLayer.frameIndex >= 20) {
				_sueNormalLayer.setFrame(0);
				_sueLongIdleActive = false;
			} else {
				_sueNormalLayer.setFrame(_sueNormalLayer.frameIndex + 1);
			}
			continue;
		}

		if (_random.getRandomNumber(199) == 0) {
			_sueNormalLayer.setFrame(5);
			_sueLongIdleActive = true;
		} else if (_sueNormalLayer.frameIndex == 4) {
			_sueNormalLayer.setFrame(0);
		} else if (_random.getRandomNumber(19) == 0) {
			_sueNormalLayer.setFrame(4);
		}
	}
}

void Scene6080::advanceGuardIdle(uint32 delta) {
	if (!_guardNormalLayer.visible || _guardManualSequenceActive)
		return;

	const uint frameCount = _guardIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_guardNormalLayer.frameIndex >= 5)
			_guardNormalLayer.setFrame(0);
		else
			_guardNormalLayer.setFrame(_guardNormalLayer.frameIndex + 1);
	}
}

void Scene6080::finishSueIdleSequence() {
	while (_sueLongIdleActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			break;
	}
}

void Scene6080::runReturnConversation() {
	_guardManualSequenceActive = true;
	if (!playAndPresentAnimationTransition(_guardNormalLayer,
			AnimationTransition(5, 9, 9, kScene6080GuardFrameMillis).unskippable()))
		return;
	_guardManualSequenceActive = false;

	beginPrimarySpeechLineWithAnimationGroup(7, 0, 0x1b4, 0xfa,
		0x30, 0x3f, 0, kScene6080GuardSpeechGroup);
	finishSueIdleSequence();
	beginPrimarySpeechLineWithAnimationGroup(7, 1, 0x154, 0xeb,
		0x3f, 0x28, 0x32, kScene6080SueSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(7, 2, 0x1b4, 0xfa,
		0x30, 0x3f, 0, kScene6080GuardSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(7, 3, 0x154, 0xeb,
		0x3f, 0x28, 0x32, kScene6080SueSpeechGroup);
	beginPrimarySpeechLineWithAnimationGroup(7, 4, 0x1b4, 0xfa,
		0x30, 0x3f, 0, kScene6080GuardSpeechGroup);

	_guardManualSequenceActive = true;
	playAndPresentAnimationTransition(_guardNormalLayer,
		AnimationTransition(13, 17, 17, kScene6080GuardFrameMillis).unskippable());
	_guardNormalLayer.setFrame(0);
	_guardManualSequenceActive = false;
}

void Scene6080::runWaxBallEscapeSequence() {
	_manualSequenceActive = true;
	_hideActiveActor = true;
	_waxBallLayer.visible = true;
	_waxBallLayer.setFrame(0);
	startWaxBallAnimation();
	beginSecondarySpeechLine(6, 0);
	while (_waxBallAnimationActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			break;
	}
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_waxBallAnimationActive = false;
	_guardIdleChannel.reset(_guardNormalLayer.frameIndex, kScene6080GuardFrameMillis);
	_waxBallLayer.visible = false;
	_hideActiveActor = false;
	drawPlayableComposite();
	presentFrame();

	beginSecondarySpeechLine(6, 1);
	startManualActorPath(0x276, 0x15c, 4);
	beginPrimarySpeechLineWithAnimationGroup(6, 2, 0x16e, 0xbe,
		0x3f, 0x28, 0x32, kScene6080SueSpeechGroup);
	beginSecondarySpeechLine(6, 3);
	beginPrimarySpeechLineWithAnimationGroup(6, 4, 0x16e, 0xbe,
		0x3f, 0x28, 0x32, kScene6080SueSpeechGroup);
	while (_manualActorPathActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			return;
	}

	runFinalSueAnimation();
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		_vm->gameState().mainFlowStateId = kScene6070EscapeState;
}

void Scene6080::startWaxBallAnimation() {
	_waxBallChannel.reset(0, kScene6080WaxBallFrameMillis);
	_escapeSueChannel.reset(0, kScene6080SueFrameMillis);
	_escapeGuardChannel.reset(0, kScene6080EscapeGuardFrameMillis);
	_guardIdleChannel.reset(_guardNormalLayer.frameIndex, kScene6080EscapeGuardFrameMillis);
	_escapeLayersSwitched = false;
	_waxBallAnimationActive = true;
}

void Scene6080::advanceWaxBallAnimation(uint32 delta) {
	if (!_escapeLayersSwitched) {
		if (_sueNormalLayer.frameIndex != 0)
			advanceSueIdle(delta);
		advanceGuardIdle(delta);
	}

	const uint waxBallFrames = _waxBallChannel.consumeFrames(delta);
	for (uint i = 0; i < waxBallFrames; ++i) {
		if (_waxBallLayer.frameIndex == 120 && !_escapeLayersSwitched) {
			_sueNormalLayer.visible = false;
			_sueAlternateLayer.visible = true;
			_sueAlternateLayer.setFrame(0);
			_guardNormalLayer.visible = false;
			_guardAlternateLayer.visible = true;
			_guardAlternateLayer.setFrame(0);
			_soundBank0.playSample(0x12, 100);
			_escapeLayersSwitched = true;
		}
		if (_waxBallLayer.frameIndex < 128)
			_waxBallLayer.setFrame(_waxBallLayer.frameIndex + 1);
	}

	if (!_escapeLayersSwitched)
		return;

	const uint sueFrames = _escapeSueChannel.consumeFrames(delta);
	for (uint i = 0; i < sueFrames && _sueAlternateLayer.frameIndex < 20; ++i)
		_sueAlternateLayer.setFrame(_sueAlternateLayer.frameIndex + 1);

	const uint guardFrames = _escapeGuardChannel.consumeFrames(delta);
	for (uint i = 0; i < guardFrames && _guardAlternateLayer.frameIndex < 9; ++i) {
		_guardAlternateLayer.setFrame(_guardAlternateLayer.frameIndex + 1);
		if (_guardAlternateLayer.frameIndex == 9)
			_soundBank0.playSample(0x13, 100);
	}

	if (_waxBallLayer.frameIndex >= 128 && _sueAlternateLayer.frameIndex >= 20 &&
			_guardAlternateLayer.frameIndex >= 9)
		_waxBallAnimationActive = false;
}

void Scene6080::startManualActorPath(int targetX, int targetY, byte finalFacing) {
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY,
		targetX, targetY, finalFacing, 0);
	_manualActorPathFrameIndex = 1;
	_manualActorPathChannel.reset(0, kScene6080ActorPathFrameMillis);
	_lastViewportScrollActorWorldX = _activeActorWorldX;
	_manualActorPathActive = _actorPathFrames.size() > 1;
	_actorPathPlaybackActive = _manualActorPathActive;
	if (!_manualActorPathActive && !_actorPathFrames.empty()) {
		const ActorPathFrame &frame = _actorPathFrames.back();
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	}
}

void Scene6080::advanceManualActorPath(uint32 delta) {
	if (!_manualActorPathActive)
		return;

	const uint frameCount = _manualActorPathChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _manualActorPathFrameIndex < _actorPathFrames.size(); ++i) {
		const ActorPathFrame &frame = _actorPathFrames[_manualActorPathFrameIndex++];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	}
	if (_manualActorPathFrameIndex >= _actorPathFrames.size()) {
		_manualActorPathActive = false;
		_actorPathPlaybackActive = false;
	}
}

void Scene6080::runFinalSueAnimation() {
	_sueAlternateLayer.setFrame(24);
	_finalSueChannel.reset(24, kScene6080SueFrameMillis);
	_finalSueAnimationActive = true;
	while (_finalSueAnimationActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10, false))
			break;
	}
	_manualActorPathActive = false;
	_actorPathPlaybackActive = false;
}

void Scene6080::advanceFinalSueAnimation(uint32 delta) {
	const uint frameCount = _finalSueChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _sueAlternateLayer.frameIndex < 93; ++i) {
		if (_sueAlternateLayer.frameIndex == 60 || _sueAlternateLayer.frameIndex == 80)
			dimEscapePalette();
		if (_sueAlternateLayer.frameIndex == 86)
			startManualActorPath(0x305, 0x14d, 1);
		_sueAlternateLayer.setFrame(_sueAlternateLayer.frameIndex + 1);
	}
	if (_sueAlternateLayer.frameIndex >= 93)
		_finalSueAnimationActive = false;
}

void Scene6080::dimEscapePalette() {
	for (uint color = 0xa0; color <= 0xa5; ++color) {
		for (uint component = 0; component < 3; ++component) {
			const uint offset = color * 3 + component;
			if (offset >= _paletteCurrent.size())
				continue;
			_paletteCurrent[offset] = _paletteCurrent[offset] < 9 ? 0 :
				_paletteCurrent[offset] - 8;
		}
	}
	invalidatePresentationPalette();
}

} // End of namespace Hollywood
