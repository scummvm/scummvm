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

#include "common/file.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"
#include "common/translation.h"

#include "freescape/freescape.h"
#include "freescape/wb.h"
#include "freescape/games/eclipse/c64.music.h"
#include "freescape/games/eclipse/c64.sfx.h"
#include "freescape/games/eclipse/ay.music.h"
#include "freescape/games/eclipse/opl.music.h"
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/objects/entrance.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

// Wally Beben table offsets for Total Eclipse Amiga TEMUSIC.AM
static const WBTableOffsets kEclipseAmigaMusicOffsets = {
	0x0ACA, // periodTable
	0x0C5E, // samplePtrTable
	0x0CA6, // instrumentTable
	0x0D16, // arpeggioIntervals
	0x0D1E, // envelopeTable
	0x0D8E, // songTable
	0x0D9E, // patternPtrTable (songTable + 16, overlaps Song 2 like Dark Side)
	14, 14, 14 // numSamples, numInstruments, numEnvelopes
};

EclipseEngine::EclipseEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_playerC64Sfx = nullptr;
	_playerMusic = nullptr;
	_c64UseSFX = false;

	// These sounds can be overriden by the class of each platform
	_soundIndexStartFalling = -1;
	_soundIndexEndFalling = -1;

	_soundIndexNoShield = -1;
	_soundIndexNoEnergy = -1;
	_soundIndexFallen = -1;
	_soundIndexTimeout = -1;
	_soundIndexForceEndGame = -1;
	_soundIndexCrushed = -1;
	_soundIndexMissionComplete = -1;

	_maxEnergy = 27;
	_maxShield = 50;

	_initialEnergy = 16;
	_initialShield = 50;

	if (isDOS())
		initDOS();
	else if (isCPC())
		initCPC();
	else if (isC64())
		initC64();
	else if (isSpectrum())
		initZX();
	else if (isAmiga() || isAtariST())
		initAmigaAtari();

	_playerHeightNumber = 1;
	_playerHeightMaxNumber = 1;

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;

	_playerStepIndex = 2;
	_playerSteps.clear();
	_playerSteps.push_back(2);
	_playerSteps.push_back(30);
	_playerSteps.push_back(60);

	_angleRotationIndex = 1;
	_angleRotations.push_back(5);
	_angleRotations.push_back(10);
	_angleRotations.push_back(15);

	_endArea = 1;
	_endEntrance = 33;

	_lastThirtySeconds = 0;
	_lastFiveSeconds = 0;
	_lastHeartbeatSoundTick = -1;
	_lastHeartIndicatorFrame = 1;
	_lastSecond = -1;
	_compassBackground = nullptr;
	_atariWaterBody = nullptr;
	_atariCompassPhase = 0;
	_atariCompassTargetPhase = 0;
	_atariCompassTargetRemainder = 0.0f;
	_atariCompassLastUpdateTick = -1;
	_atariCompassPhaseInitialized = false;
	_atariLanternLightFrame = -1;
	_atariLanternAnimationDirection = 0;
	_atariLanternLastUpdateTick = -1;
	_lanternBatteryLevel = 5;
	_atariAreaDark = false;
	_resting = false;
	_flashlightOn = false;
}

void EclipseEngine::stopBackgroundMusic() {
	if (_playerMusic)
		_playerMusic->stopMusic();
	if (_mixer)
		_mixer->stopHandle(_musicHandle);
}

void EclipseEngine::restartBackgroundMusic() {
	if (_playerMusic) {
		_playerMusic->startMusic();
	} else if (isAmiga() && !_musicData.empty()) {
		if (_mixer)
			_mixer->stopHandle(_musicHandle);
		Audio::AudioStream *musicStream = makeWallyBebenStream(
			_musicData.data(), _musicData.size(), 1, 44100, true,
			&kEclipseAmigaMusicOffsets);
		if (musicStream) {
			_mixer->playStream(Audio::Mixer::kMusicSoundType,
				&_musicHandle, musicStream);
		}
	} else {
		playMusic("Total Eclipse Theme");
	}
}

EclipseEngine::~EclipseEngine() {
	stopBackgroundMusic();
	if (_atariWaterBody) {
		_atariWaterBody->free();
		delete _atariWaterBody;
	}
	if (_compassBackground) {
		_compassBackground->free();
		delete _compassBackground;
	}
	delete _playerMusic;
	delete _playerC64Sfx;
}

void EclipseEngine::initGameState() {
	FreescapeEngine::initGameState();

	_playerHeightNumber = 1;

	_gameStateVars[k8bitVariableEnergy] = _initialEnergy;
	_gameStateVars[k8bitVariableShield] = _initialShield;

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	_lastThirtySeconds = seconds / 30;
	_lastFiveSeconds = seconds / 5;
	_lastHeartbeatSoundTick = -1;
	_lastHeartIndicatorFrame = 1;
	_atariCompassPhase = 0;
	_atariCompassTargetPhase = 0;
	_atariCompassTargetRemainder = 0.0f;
	_atariCompassLastUpdateTick = -1;
	_atariCompassPhaseInitialized = false;
	_atariLanternLightFrame = -1;
	_atariLanternAnimationDirection = 0;
	_atariLanternLastUpdateTick = -1;
	_lanternBatteryLevel = 5;
	_atariAreaDark = false;
	_resting = false;
	_flashlightOn = false;
	restartBackgroundMusic();
}

void EclipseEngine::loadAssets() {
	FreescapeEngine::loadAssets();

	Common::List<int> globalIds = _areaMap[255]->getEntranceIds();
	for (auto &it : _areaMap) {
		if (it._value->getAreaID() == 255)
			continue;

		it._value->addStructure(_areaMap[255]);

		if (isDemo()) {
			it._value->_name = "  NOW TRAINING  ";
		}

		for (auto &id : globalIds) {
			if (it._value->entranceWithID(id))
				continue;

			Object *obj = _areaMap[255]->entranceWithID(id);
			assert(obj);
			assert(obj->getType() == ObjectType::kEntranceType);
			// The entrance is not in the current area, so we need to add it
			it._value->addObjectFromArea(id, _areaMap[255]);
		}
	}

	_timeoutMessage = _messagesList[1];
	_noShieldMessage = _messagesList[0];
	//_noEnergyMessage = _messagesList[16];
	_fallenMessage = _messagesList[3];
	_crushedMessage = _messagesList[2];

	_areaMap[1]->addFloor();
	if (isSpectrum())
		_areaMap[1]->_paperColor = 1;

	if (!isDemo() && !isEclipse2()) {
		_areaMap[51]->addFloor();
		_areaMap[51]->_paperColor = 1;

		// Workaround for fixing some planar objects from area 9 that have null size
		Object *obj = nullptr;
		obj = _areaMap[9]->objectWithID(7);
		assert(obj);
		obj->_size = 32 * Math::Vector3d(3, 0, 2);

		obj = _areaMap[9]->objectWithID(8);
		assert(obj);
		obj->_size = 32 * Math::Vector3d(3, 0, 2);

		obj = _areaMap[9]->objectWithID(9);
		assert(obj);
		obj->_size = 32 * Math::Vector3d(3, 0, 2);
	}
}

bool EclipseEngine::checkIfGameEnded() {
	if (_gameStateControl == kFreescapeGameStatePlaying) {
		if (_hasFallen && _avoidRenderingFrames == 0) {
			_hasFallen = false;
			if (isDOS())
				playSoundFx(4, false);
			else
				playSound(_soundIndexStartFalling, false, _soundFxHandle);

			stopMovement();
			// If shield is less than 11 after a fall, the game ends
			if (_gameStateVars[k8bitVariableShield] > 15 + 11) {
				_gameStateVars[k8bitVariableShield] -= 15;
				return false; // Game can continue
			}
			if (!_fallenMessage.empty())
				insertTemporaryMessage(_fallenMessage, _countdown - 4);
			_gameStateControl = kFreescapeGameStateEnd;
		} else if (getGameBit(16)) {
			_gameStateControl = kFreescapeGameStateEnd;
			insertTemporaryMessage(_messagesList[4], INT_MIN);
		}

		FreescapeEngine::checkIfGameEnded();
	}
	return false;
}

bool EclipseEngine::triggerWinCondition() {
	setGameBit(16);
	_endGameDelayTicks = 0;
	_endGameKeyPressed = false;
	_endGamePlayerEndArea = false;
	_gameStateControl = kFreescapeGameStateEnd;
	return true;
}

void EclipseEngine::endGame() {
	bool enteringEndArea = (_gameStateControl == kFreescapeGameStateEnd && !_endGamePlayerEndArea);
	if (enteringEndArea)
		restartBackgroundMusic();

	FreescapeEngine::endGame();

	if (!_endGamePlayerEndArea)
		return;

	if (_gameStateControl == kFreescapeGameStateEnd) {
		removeTimers();
		if (getGameBit(16)) {
			if (_countdown > - 3600)
				_countdown -= 10;
			else
				_countdown = -3600;
		} else {
			if (_countdown > 0)
				_countdown -= 10;
			else
				_countdown = 0;
		}
	}

	if (_endGameKeyPressed && (_countdown == 0 || _countdown == -3600)) {
		if (isSpectrum())
			playSound(5, true, _soundFxHandle);
		_gameStateControl = kFreescapeGameStateRestart;
	}
	_endGameKeyPressed = false;
}

void EclipseEngine::initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) {
	FreescapeEngine::initKeymaps(engineKeyMap, infoScreenKeyMap, target);
	Common::Action *act;

	act = new Common::Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("s");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("l");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("QUIT", _("Quit game"));
	act->setCustomEngineActionEvent(kActionEscape);
	if (isSpectrum())
		act->addDefaultInputMapping("1");
	else
		act->addDefaultInputMapping("ESCAPE");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("TOGGLESOUND", _("Toggle sound"));
	act->setCustomEngineActionEvent(kActionToggleSound);
	act->addDefaultInputMapping("t");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("ROTL", _("Rotate left"));
	act->setCustomEngineActionEvent(kActionRotateLeft);
	act->addDefaultInputMapping("q");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTR", _("Rotate right"));
	act->setCustomEngineActionEvent(kActionRotateRight);
	act->addDefaultInputMapping(_useWASDControls ? "e" : "w");
	engineKeyMap->addAction(act);

	// I18N: Illustrates the angle at which you turn left or right.
	act = new Common::Action("CHNGANGLE", _("Change angle"));
	act->setCustomEngineActionEvent(kActionIncreaseAngle);
	act->addDefaultInputMapping(_useWASDControls ? "v" : "a");
	engineKeyMap->addAction(act);

	// I18N: STEP SIZE: Measures the size of one movement in the direction you are facing (1-250 standard distance units (SDUs))
	act = new Common::Action("CHNGSTEPSIZE", _("Change step size"));
	act->setCustomEngineActionEvent(kActionChangeStepSize);
	act->addDefaultInputMapping(_useWASDControls ? "x" : "s");
	engineKeyMap->addAction(act);

	if (_useWASDControls) {
		act = new Common::Action("RUNMOD", _("Sprint (hold)"));
		act->setCustomEngineActionEvent(kActionRunModifier);
		act->addDefaultInputMapping("LSHIFT");
		act->addDefaultInputMapping("RSHIFT");
		act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
		engineKeyMap->addAction(act);
	}

	act = new Common::Action("TGGLHEIGHT", _("Toggle height"));
	act->setCustomEngineActionEvent(kActionToggleRiseLower);
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("h");
	engineKeyMap->addAction(act);

	act = new Common::Action("REST", _("Rest"));
	act->setCustomEngineActionEvent(kActionRest);
	act->addDefaultInputMapping("JOY_Y");
	act->addDefaultInputMapping("r");
	engineKeyMap->addAction(act);

	act = new Common::Action("FACEFRWARD", _("Face forward"));
	act->setCustomEngineActionEvent(kActionFaceForward);
	act->addDefaultInputMapping("f");
	engineKeyMap->addAction(act);

	act = new Common::Action("FLASHLIGHT", _("Toggle flashlight"));
	act->setCustomEngineActionEvent(kActionToggleFlashlight);
	act->addDefaultInputMapping("t");
	engineKeyMap->addAction(act);
}

void EclipseEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();
	_atariAreaDark = (isAtariST() || isAmiga()) && isAtariDarkArea(areaID);

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->_name);

	if (entranceID > 0)
		traverseEntrance(entranceID);
	else if (entranceID == -1)
		debugC(1, kFreescapeDebugMove, "Loading game, no change in position");
	else
		error("Invalid area change!");

	_lastPosition = _position;

	if (areaID == _startArea && entranceID == _startEntrance) {
		if (_pitch >= 180)
			_pitch = 360 - _pitch;
		playSound(_soundIndexStart, false, _soundFxHandle);
		if (isEclipse2()) {
			_gameStateControl = kFreescapeGameStateStart;
			_pitch = -10;
		}

	} if (areaID == _endArea && entranceID == _endEntrance) {
		_flyMode = true;
		if (isDemo())
			_pitch = 20;
		else
			_pitch = 10;
	} else {
		playSound(_soundIndexAreaChange, false, _soundFxHandle);
	}

	_gfx->_keyColor = 0;
	if ((isAtariST() || isAmiga()) && isAtariDarkArea(areaID))
		applyEclipseFadePalette(areaID, _lanternBatteryLevel);
	swapPalette(areaID);
	if (isCPC())
		updateHeartFramesCPC();
	if (isAmiga() || isAtariST())
		_currentArea->_skyColor = 15;

	if ((isAtariST() || isAmiga()) && entranceID > 0) {
		Entrance *entrance = (Entrance *)_currentArea->entranceWithID(entranceID);
		if (entrance) {
			int phase = atariCompassPhaseFromRotationY(entrance->getRotation().y());
			_atariCompassPhase = phase;
			_atariCompassTargetPhase = phase;
			_atariCompassTargetRemainder = 0.0f;
			_atariCompassLastUpdateTick = _ticks;
			_atariCompassPhaseInitialized = true;
		}
	}

	// Start background music (Amiga)
	if (isAmiga() && !_musicData.empty() && !_mixer->isSoundHandleActive(_musicHandle)) {
		Audio::AudioStream *musicStream = makeWallyBebenStream(
			_musicData.data(), _musicData.size(), 1, 44100, true,
			&kEclipseAmigaMusicOffsets);
		if (musicStream) {
			_mixer->playStream(Audio::Mixer::kMusicSoundType,
				&_musicHandle, musicStream);
		}
	}

	resetInput();
}

bool EclipseEngine::isAtariDarkArea(uint16 areaID) const {
	for (uint i = 0; i < _atariDarkAreas.size(); i++) {
		if (_atariDarkAreas[i] == areaID)
			return true;
	}
	return false;
}

void EclipseEngine::drawBackground() {
	clearBackground();
	_gfx->drawBackground(_currentArea->_skyColor);
	if (_currentArea && _currentArea->getAreaID() == 1) {
		if (ABS(_countdown) <= 15 * 60) // Last 15 minutes
			_gfx->drawBackground(5);
		if (ABS(_countdown) <= 10) // Last 10 seconds
			_gfx->drawBackground(1);

		float progress = 0;
		if (_countdown >= 0 || getGameBit(16))
			progress = float(_countdown) / _initialCountdown;

		uint8 color1 = 15;
		uint8 color2 = 10;

		if (isSpectrum() || isCPC() || isC64()) {
			color1 = 2;
			color2 = 10;
		} else if (isAmiga() || isAtariST()) {
			color1 = 8;
			color2 = 14;
		} else if (isDOS() && _renderMode == Common::kRenderCGA) {
			color1 = 2;
			color2 = 8;
		}

		_gfx->drawEclipse(color1, color2, progress);
	}
}

void EclipseEngine::titleScreen() {
	if (isDOS())
		playSoundFx(2, true);
	FreescapeEngine::titleScreen();
}


void EclipseEngine::borderScreen() {
	if (_border) {
		drawBorder();
		if (isDemo() && isCPC()) {
			drawFullscreenMessageAndWait(_messagesList[23]);
			drawFullscreenMessageAndWait(_messagesList[24]);
			drawFullscreenMessageAndWait(_messagesList[25]);
		} else if (isDemo() && isSpectrum()) {
			if (_variant & GF_ZX_DEMO_MICROHOBBY) {
				drawFullscreenMessageAndWait(_messagesList[23]);
			} else if (_variant & GF_ZX_DEMO_CRASH) {
				drawFullscreenMessageAndWait(_messagesList[9]);
				drawFullscreenMessageAndWait(_messagesList[10]);
				drawFullscreenMessageAndWait(_messagesList[11]);
			}
		} else {
			FreescapeEngine::borderScreen();
		}
	}
}

void EclipseEngine::drawInfoMenu() {
	PauseToken pauseToken = pauseEngine();
	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}
	_savedScreen = _gfx->getScreenshot();
	uint32 color = 0;
	switch (_renderMode) {
		case Common::kRenderCGA:
			color = 1;
			break;
		case Common::kRenderZX:
			color = 6;
			break;
		default:
			color = 14;
	}
	uint8 r, g, b;
	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);

	surface->fillRect(Common::Rect(88, 48, 231, 103), black);
	surface->frameRect(Common::Rect(88, 48, 231, 103), front);

	surface->frameRect(Common::Rect(90, 50, 229, 101), front);

	drawStringInSurface("L-LOAD S-SAVE", 105, 56, front, black, surface);
	if (isSpectrum())
		drawStringInSurface("1-TERMINATE", 105, 64, front, black, surface);
	else
		drawStringInSurface("ESC-TERMINATE", 105, 64, front, black, surface);

	drawStringInSurface("T-TOGGLE", 128, 81, front, black, surface);
	drawStringInSurface("SOUND ON/OFF", 113, 88, front, black, surface);

	Texture *menuTexture = _gfx->createTexture(surface);
	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
				case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionLoad) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					loadGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionSave) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionToggleSound) {
					if (isC64() && _playerC64Sfx) {
						toggleC64Sound();
						_eventManager->purgeKeyboardEvents();
					} else {
						playSound(_soundIndexMenu, false, _soundFxHandle);
					}
				} else if ((isDOS() || isCPC() || isSpectrum()) && event.customType == kActionEscape) {
					_forceEndGame = true;
					cont = false;
				} else
					cont = false;
				break;
				case Common::EVENT_KEYDOWN:
					cont = false;
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				break;

			default:
				break;
			}
		}
		drawFrame();
		_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, menuTexture);

		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	_savedScreen = nullptr;
	surface->free();
	delete surface;
	delete menuTexture;
	pauseToken.clear();
}

void EclipseEngine::pressedKey(const int keycode) {
	if (keycode == kActionIncreaseAngle) {
		changeAngle(1, true);
	} else if (keycode == kActionChangeStepSize) {
		changeStepSize();
	} else if (keycode == kActionToggleRiseLower) {
		if (_playerHeightNumber == 0)
			rise();
		else if (_playerHeightNumber == 1)
			lower();
		else
			error("Invalid player height index: %d", _playerHeightNumber);
	} else if (keycode == kActionRest) {
		if (_currentArea->getAreaID() == 1 || _currentArea->getAreaID() == 51) {
			playSoundFx(3, false);
			insertTemporaryMessage(_messagesList[6], _countdown - 2);
		} else {
			_resting = true;
			insertTemporaryMessage(_messagesList[7], _countdown - 2);
			_countdown = _countdown - 5;
		}
	} else if (keycode == kActionFaceForward) {
		_pitch = 0;
		updateCamera();
	} else if (keycode == kActionToggleFlashlight) {
		if (isAtariST() || isAmiga()) {
			if (_flashlightOn) {
				_flashlightOn = false;
				if (_atariLanternLightFrame < 0)
					_atariLanternLightFrame = 0;
				_atariLanternAnimationDirection = 1;
			} else if (_lanternBatteryLevel >= 0) {
				_flashlightOn = true;
				if (_atariLanternLightFrame < 0 || _atariLanternLightFrame > 5)
					_atariLanternLightFrame = 5;
				_atariLanternAnimationDirection = -1;
			}
			_atariLanternLastUpdateTick = -1;
		} else {
			_flashlightOn = !_flashlightOn;
		}
	} else if (keycode == kActionRunModifier) {
		// Shift-to-sprint: save current step, switch to max while held
		if (_savedPlayerStepIndex < 0) {
			_savedPlayerStepIndex = _playerStepIndex;
			_playerStepIndex = (int)_playerSteps.size() - 1;
		}
	}
}

void EclipseEngine::onRotate(float xoffset, float yoffset, float zoffset) {
	(void)yoffset;
	(void)zoffset;

	if ((!isAtariST() && !isAmiga()) || xoffset == 0.0f)
		return;

	if (!_atariCompassPhaseInitialized) {
		int phase = atariCompassTargetPhaseFromYaw(_yaw, 0);
		_atariCompassPhase = phase;
		_atariCompassTargetPhase = phase;
		_atariCompassTargetRemainder = 0.0f;
		_atariCompassLastUpdateTick = _ticks;
		_atariCompassPhaseInitialized = true;
	}

	_atariCompassTargetRemainder += xoffset / 5.0f;
	int phaseDelta = 0;
	while (_atariCompassTargetRemainder >= 1.0f) {
		_atariCompassTargetRemainder -= 1.0f;
		phaseDelta++;
	}
	while (_atariCompassTargetRemainder <= -1.0f) {
		_atariCompassTargetRemainder += 1.0f;
		phaseDelta--;
	}

	if (phaseDelta == 0)
		return;

	_atariCompassTargetPhase = _atariCompassTargetPhase + phaseDelta;

	// The original ST draw routine at $1CC0 always moves by the shortest path
	// to a stored target phase. Clamp the target so that queued ScummVM input
	// cannot place it more than half a turn away, which would otherwise trigger
	// a wraparound reversal the original transition-driven code never presents.
	while (_atariCompassTargetPhase < 0)
		_atariCompassTargetPhase += 72;
	while (_atariCompassTargetPhase >= 72)
		_atariCompassTargetPhase -= 72;

	if (phaseDelta > 0) {
		int forwardDistance = (_atariCompassTargetPhase - _atariCompassPhase + 72) % 72;
		if (forwardDistance > 35)
			_atariCompassTargetPhase = (_atariCompassPhase + 35) % 72;
	} else {
		int backwardDistance = (_atariCompassPhase - _atariCompassTargetPhase + 72) % 72;
		if (backwardDistance > 35)
			_atariCompassTargetPhase = (_atariCompassPhase + 72 - 35) % 72;
	}
}

bool EclipseEngine::onScreenControls(Common::Point mouse) {
	if (!isAmiga() && !isAtariST())
		return false;

	// Right-side arrow buttons
	if (_lookUpArea.contains(mouse)) {
		rotate(0, -5, 0);
		return true;
	} else if (_lookDownArea.contains(mouse)) {
		rotate(0, 5, 0);
		return true;
	} else if (_turnLeftArea.contains(mouse)) {
		rotate(-5, 0, 0);
		return true;
	} else if (_turnRightArea.contains(mouse)) {
		rotate(5, 0, 0);
		return true;
	} else if (_uTurnArea.contains(mouse)) {
		rotate(180, 0, 0);
		return true;
	} else if (_faceForwardArea.contains(mouse)) {
		pressedKey(kActionFaceForward);
		return true;
	}

	// Left-side buttons (movement buttons just consume click, like Driller)
	if (_moveBackwardArea.contains(mouse)) {
		return true;
	} else if (_stepBackwardArea.contains(mouse)) {
		return true;
	} else if (_interactArea.contains(mouse)) {
		activate();
		return true;
	} else if (_infoDisplayArea.contains(mouse)) {
		drawInfoMenu();
		return true;
	}

	// Center/functional areas
	if (_lanternArea.contains(mouse)) {
		pressedKey(kActionToggleFlashlight);
		return true;
	} else if (_restArea.contains(mouse)) {
		pressedKey(kActionRest);
		return true;
	}

	// Status bar indicators
	if (_stepSizeArea.contains(mouse)) {
		pressedKey(kActionChangeStepSize);
		return true;
	} else if (_heightArea.contains(mouse)) {
		pressedKey(kActionToggleRiseLower);
		return true;
	}

	// Save/load
	if (_saveGameArea.contains(mouse)) {
		_gfx->setViewport(_fullscreenViewArea);
		saveGameDialog();
		_gfx->setViewport(_viewArea);
		return true;
	} else if (_loadGameArea.contains(mouse)) {
		_gfx->setViewport(_fullscreenViewArea);
		loadGameDialog();
		_gfx->setViewport(_viewArea);
		return true;
	}

	return false;
}

void EclipseEngine::releasedKey(const int keycode) {
	if (keycode == kActionRiseOrFlyUp)
		_resting = false;
	else if (keycode == kActionRunModifier) {
		// Shift released: restore previous step size
		if (_savedPlayerStepIndex >= 0) {
			_playerStepIndex = _savedPlayerStepIndex;
			_savedPlayerStepIndex = -1;
		}
	}
}

void EclipseEngine::drawAnalogClock(Graphics::Surface *surface, int x, int y, uint32 colorHand1, uint32 colorHand2, uint32 colorBack) {
	// These calls will cover the pixels of the hardcoded clock image
	drawAnalogClockHand(surface, x, y, 6 * 6 - 90, 12, colorBack);
	drawAnalogClockHand(surface, x, y, 7 * 6 - 90, 12, colorBack);
	drawAnalogClockHand(surface, x, y, 41 * 6 - 90, 11, colorBack);
	drawAnalogClockHand(surface, x, y, 42 * 6 - 90, 11, colorBack);
	drawAnalogClockHand(surface, x, y, 0 * 6 - 90, 11, colorBack);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	hours = 7 + 2 - hours; // It's 7 o-clock when the game starts
	minutes = 59 - minutes;
	seconds = 59 - seconds;
	drawAnalogClockHand(surface, x, y, hours * 30 - 90, 11, colorHand1);
	drawAnalogClockHand(surface, x, y, minutes * 6 - 90, 11, colorHand1);
	drawAnalogClockHand(surface, x, y, seconds * 6 - 90, 11, colorHand2);
}

void EclipseEngine::drawAnalogClockHand(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color) {
	const double degtorad = (M_PI * 2) / 360;
	double w = magnitude * cos(degrees * degtorad);
	double h = magnitude * sin(degrees * degtorad);
	surface->drawLine(x, y, x+(int)w, y+(int)h, color);
	if (isC64()) {
		surface->drawLine(x+1, y, x+1+(int)w, y+(int)h, color);
	}
}

void EclipseEngine::drawCompass(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color) {
	const double degtorad = (M_PI * 2) / 360;
	double w = magnitude * cos(-degrees * degtorad);
	double h = magnitude * sin(-degrees * degtorad);

	int dx = 0;
	int dy = 0;

	// Adjust dx and dy to make the compass look like a compass
	if (degrees == 0 || degrees == 360) {
		dx = 1;
		dy = 2;
	} else if (degrees > 0 && degrees < 90) {
		dx = 2;
		dy = 1;
	} else if (degrees == 90) {
		dx = 2;
		dy = 1;
	} else if (degrees > 90 && degrees < 180) {
		dx = 2;
		dy = -1;
	} else if (degrees == 180) {
		dx = 1;
		dy = 2;
	} else if (degrees > 180 && degrees < 270) {
		dx = -2;
		dy = -1;
	} else if (degrees == 270) {
		dx = 2;
		dy = 1;
	} else if (degrees > 270 && degrees < 360) {
		dx = -2;
		dy = 1;
	}

	surface->drawLine(x, y, x+(int)w, y+(int)h, color);
	surface->drawLine(x - dx, y - dy, x+(int)w, y+(int)h, color);
	surface->drawLine(x + dx, y + dy, x+(int)w, y+(int)h, color);

	surface->drawLine(x - dx, y - dy, x+(int)-w, y+(int)-h, color);
	surface->drawLine(x + dx, y + dy, x+(int)-w, y+(int)-h, color);
}

// Copied from BITMAP::circlefill in engines/ags/lib/allegro/surface.cpp
void fillCircle(Graphics::Surface *surface, int x, int y, int radius, int color) {
	int cx = 0;
	int cy = radius;
	int df = 1 - radius;
	int d_e = 3;
	int d_se = -2 * radius + 5;

	do {
		surface->hLine(x - cy, y - cx, x + cy, color);

		if (cx)
			surface->hLine(x - cy, y + cx, x + cy, color);

		if (df < 0) {
			df += d_e;
			d_e += 2;
			d_se += 2;
		} else {
			if (cx != cy) {
				surface->hLine(x - cx, y - cy, x + cx, color);

				if (cy)
					surface->hLine(x - cx, y + cy, x + cx, color);
			}

			df += d_se;
			d_e += 2;
			d_se += 4;
			cy--;
		}

		cx++;

	} while (cx <= cy);
}

void EclipseEngine::drawEclipseIndicator(Graphics::Surface *surface, int x, int y, uint32 color1, uint32 color2, uint32 color3) {
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	surface->fillRect(Common::Rect(x, y, x + 50, y + 20), black);
	float progress = 0;
	if (_countdown >= 0)
		progress = float(_countdown) / _initialCountdown;
	int difference = 14 * progress;
	int radius = 7;
	int sunX = x + 7;
	int sunY = y + 10;
	int moonX = x + 7 + difference;
	int moonY = y + 10;
	fillCircle(surface, sunX, sunY, radius, color1);
	if (color3 != 0) {
		for (int dy = -radius; dy <= radius; ++dy) {
			for (int dx = -radius; dx <= radius; ++dx) {
				if (dx * dx + dy * dy <= radius * radius) {
					int px = moonX + dx;
					int py = moonY + dy;
					if ((px + py) % 2 == 0) {
						surface->setPixel(px, py, color2);
					} else {
						surface->setPixel(px, py, color3);
					}
				}
			}
		}
	} else {
		fillCircle(surface, moonX, moonY, radius, color2);
	}
}

void EclipseEngine::drawIndicator(Graphics::Surface *surface, int xPosition, int yPosition, int separation) {
	if (_indicators.size() == 0)
		return;

	for (int i = 0; i < 5; i++) {
		if (isSpectrum() || isC64()) {
			if (_gameStateVars[kVariableEclipseAnkhs] <= i)
				continue;
		} else if (_gameStateVars[kVariableEclipseAnkhs] > i)
			continue;
		surface->copyRectToSurface(*_indicators[0], xPosition + separation * i, yPosition, Common::Rect(_indicators[0]->w, _indicators[0]->h));
	}
}

void EclipseEngine::drawHeartIndicator(Graphics::Surface *surface, int x, int y) {
	// Heartbeat animation shared across platforms.
	// Timer counts down from shield at 50Hz (_ticks rate).
	// Beat frame shown for last 5 ticks of each cycle, rest frame for the remainder.
	// Lower shield = faster heartbeat. At shield <= 5, heart beats constantly.
	if (_eclipseSprites.size() < 2)
		return;

	int shield = _gameStateVars[k8bitVariableShield];
	int beatCycle = MAX(shield, 1);
	int phase = _ticks % beatCycle;
	int beatStart = MAX(beatCycle - 5, 0);
	int frame = _lastHeartIndicatorFrame;

	if (shield <= 5 || _avoidRenderingFrames > 0 || _hasFallen) {
		frame = 1;
		_lastHeartIndicatorFrame = frame;
	} else if (!_inWaitLoop) {
		frame = (phase >= beatStart) ? 0 : 1;
		_lastHeartIndicatorFrame = frame;

		if (!isPaused() && phase == beatStart && _lastHeartbeatSoundTick != _ticks) {
			playSound(1, false, _soundFxHandle);
			_lastHeartbeatSoundTick = _ticks;
		}
	}

	surface->copyRectToSurface(*_eclipseSprites[frame], x, y,
		Common::Rect(_eclipseSprites[frame]->w, _eclipseSprites[frame]->h));
}

void EclipseEngine::drawSensorShoot(Sensor *sensor) {
	Math::Vector3d target;
	float distance = 5;
	int axisToSkip = -1;

	if (sensor->_axis == 0x1 || sensor->_axis == 0x2)
		axisToSkip = 0;

	if (sensor->_axis == 0x10 || sensor->_axis == 0x20)
		axisToSkip = 2;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == j) {
				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) + distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();

				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) - distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);
			} else {
				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) + distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) + distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) + distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) - distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();

				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) - distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) - distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) - distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) + distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);
			}
		}
	}
}

void EclipseEngine::drawScoreString(int score, int x, int y, uint32 front, uint32 back, Graphics::Surface *surface) {
	Common::String scoreStr = Common::String::format("%07d", score);

	if (isDOS() || isCPC() || isSpectrum()) {
		scoreStr = shiftStr(scoreStr, 'Z' - '0' + 1);
		if (_renderMode == Common::RenderMode::kRenderEGA || isSpectrum()) {
			drawStringInSurface(scoreStr, x, y, front, back, surface);
			return;
		}
	}

	// Atari ST: use Font B (_fontScore) with dedicated score digit glyphs.
	// Font B has 10 glyphs (0-9) for digits. In the original, the score bytes
	// have $2F subtracted to map '0'→glyph 0, '1'→glyph 1, etc.
	// For drawChar: chr = glyph_index + 32, so digit '0' → chr 32, '9' → chr 41.
	if (isAtariST() || isAmiga()) {
		_fontScore.setBackground(back);
		_fontScore.setSecondaryColor(front);
		// Font B uses palette indices 1-4 like Font A
		uint32 pal2 = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 182, 109, 36);
		uint32 pal3 = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 218, 145, 36);
		uint32 pal4 = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 255, 182, 36);
		_fontScore.setSecondaryColor(pal2);
		_fontScore.setTertiaryColor(pal3);
		_fontScore.setQuaternaryColor(pal4);
		for (int i = 0; i < int(scoreStr.size()); i++) {
			int chr = (scoreStr[i] - '0') + 32;
			_fontScore.drawChar(surface, chr, x, y, front);
			x += 8;
		}
		return;
	}

	// Start in x,y and draw each digit, from left to right, adding a gap every 3 digits
	int gapSize = isC64() ? 8 : 4;
	int charStep = 8;

	Font *scoreFont = &_font;
	scoreFont->setBackground(back);
	scoreFont->setSecondaryColor(front);

	for (int i = 0; i < int(scoreStr.size()); i++) {
		Common::String digit(scoreStr[i]);
		if (!isCPC())
			digit.toUppercase();
		scoreFont->drawString(surface, digit, x, y, _screenW, front);
		x += charStep;
		if ((i - scoreStr.size() + 1) % 3 == 1)
			x += gapSize;
	}
}


void EclipseEngine::updateTimeVariables() {
	if (isEclipse2() && _gameStateControl == kFreescapeGameStateStart) {
		executeLocalGlobalConditions(false, true, false);
		_gameStateControl = kFreescapeGameStatePlaying;
	}

	if (_gameStateControl != kFreescapeGameStatePlaying)
		return;
	// This function only executes "on collision" room/global conditions
	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);


	if (_lastFiveSeconds != seconds / 5) {
		_lastFiveSeconds = seconds / 5;
		executeLocalGlobalConditions(false, false, true);
	}

	if (_lastThirtySeconds != seconds / 30) {
		_lastThirtySeconds = seconds / 30;

		if (!_resting && _gameStateVars[k8bitVariableEnergy] > 0) {
			_gameStateVars[k8bitVariableEnergy] -= 1;
		}

		if (_gameStateVars[k8bitVariableShield] < _maxShield) {
			_gameStateVars[k8bitVariableShield] += 1;
		}

		// Lantern battery drain: non-rechargeable, one level per 30-second tick
		// while the flashlight is on. ROM drains TeLanternBrightnessFrame ($7f6c)
		// from 5 (brightest) down to -1 (dead). 6 levels total.
		if ((isAtariST() || isAmiga()) && _flashlightOn && _lanternBatteryLevel >= 0) {
			_lanternBatteryLevel--;
			if (_lanternBatteryLevel < 0) {
				_flashlightOn = false;
				_atariLanternLightFrame = -1;
				_atariLanternAnimationDirection = 0;
			}
			if (_atariAreaDark && _currentArea) {
				applyEclipseFadePalette(_currentArea->getAreaID(), _lanternBatteryLevel);
				swapPalette(_currentArea->getAreaID());
			}
		}
	}

	if (isEclipse() && isSpectrum() && _currentArea->getAreaID() == 42) {
		if (_lastSecond != seconds) { // Swap ink and paper colors every second
			_lastSecond = seconds;
			int tmp = _gfx->_inkColor;
			_gfx->_inkColor = _gfx->_paperColor;
			_gfx->_paperColor = tmp;
		}
	}
}

void EclipseEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source - 1;
	debugC(1, kFreescapeDebugCode, "Printing message %d", index);
	if (index > 127) {
		index = _messagesList.size() - (index - 254) - 2;
		drawFullscreenMessageAndWait(_messagesList[index]);
		return;
	}
	Common::String message = _messagesList[index];
	if (isEclipse2()) {
		// Message 16 (1-based, index 15) contains the "NO. OF PARTS XX" placeholder.
		// The original routine patches those two bytes immediately before drawing the string.
		Common::String::size_type pos = message.find("XX");
		if (pos != Common::String::npos) {
			int parts = _gameStateVars[kVariableEclipse2SphinxParts];
			Common::String replacement;
			if (parts < 10)
				replacement = Common::String::format("%d ", parts);
			else
				replacement = Common::String::format("%d", parts);
			message.replace(pos, 2, replacement);
		}
	}
	insertTemporaryMessage(message, _countdown - 2);
}

Common::Error EclipseEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error EclipseEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	(void)stream;
	_lastHeartbeatSoundTick = -1;
	_lastHeartIndicatorFrame = 1;
	_atariCompassPhase = 0;
	_atariCompassTargetPhase = 0;
	_atariCompassTargetRemainder = 0.0f;
	_atariCompassLastUpdateTick = -1;
	_atariCompassPhaseInitialized = false;
	_atariLanternAnimationDirection = 0;
	_atariLanternLightFrame = _flashlightOn ? 0 : -1;
	_atariLanternLastUpdateTick = -1;
	_atariAreaDark = (isAtariST() || isAmiga()) && _currentArea && isAtariDarkArea(_currentArea->getAreaID());
	return Common::kNoError;
}

} // End of namespace Freescape
