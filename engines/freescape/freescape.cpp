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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/math.h"
#include "common/random.h"
#include "common/timer.h"
#include "graphics/cursorman.h"
#include "image/neo.h"
#include "image/scr.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/objects/sensor.h"

namespace Freescape {

FreescapeEngine *g_freescape;

FreescapeEngine::FreescapeEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _gfx(nullptr) {
	if (!ConfMan.hasKey("render_mode") || ConfMan.get("render_mode").empty())
		_renderMode = Common::kRenderEGA;
	else
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));

	_binaryBits = 0;
	_screenW = 320;
	_screenH = 200;

	if (isAmiga()) {
		_renderMode = Common::kRenderAmiga;
	} else if (isAtariST()) {
		_renderMode = Common::kRenderAtariST;
	} else if (isCPC()) {
		_renderMode = Common::kRenderCPC;
	} else if (isSpectrum()) {
		_renderMode = Common::kRenderZX;
	} else if (isC64()) {
		_renderMode = Common::kRenderC64;
	}

	_variant = gd->flags;

	_language = Common::parseLanguage(ConfMan.get("language"));

	if (!Common::parseBool(ConfMan.get("prerecorded_sounds"), _usePrerecordedSounds))
		error("Failed to parse bool from prerecorded_sounds option");

	if (!Common::parseBool(ConfMan.get("extended_timer"), _useExtendedTimer))
		error("Failed to parse bool from extended_timer option");

	if (!Common::parseBool(ConfMan.get("disable_demo_mode"), _disableDemoMode))
		error("Failed to parse bool from disable_demo_mode option");

	if (!Common::parseBool(ConfMan.get("disable_sensors"), _disableSensors))
		error("Failed to parse bool from disable_sensors option");

	if (!Common::parseBool(ConfMan.get("disable_falling"), _disableFalling))
		error("Failed to parse bool from disable_falling option");

	if (!Common::parseBool(ConfMan.get("invert_y"), _invertY))
		error("Failed to parse bool from disable_falling option");

	_gameStateControl = kFreescapeGameStateStart;
	_startArea = 0;
	_startEntrance = 0;
	_endArea = 0;
	_endEntrance = 0;
	_currentArea = nullptr;
	_gotoExecuted = false;
	_rotation = Math::Vector3d(0, 0, 0);
	_position = Math::Vector3d(0, 0, 0);
	_lastPosition = Math::Vector3d(0, 0, 0);
	_hasFallen = false;
	_maxFallingDistance = 64;
	_velocity = Math::Vector3d(0, 0, 0);
	_cameraFront = Math::Vector3d(0, 0, 0);
	_cameraRight = Math::Vector3d(0, 0, 0);
	_yaw = 0;
	_pitch = 0;
	_upVector = Math::Vector3d(0, 1, 0);
	_mouseSensitivity = 0.25f;
	_demoMode = false;
	_shootMode = false;
	_demoIndex = 0;
	_currentDemoInputCode = 0;
	_currentDemoInputRepetition = 0;
	_currentDemoMousePosition = _crossairPosition;
	_flyMode = false;
	_noClipMode = false;
	_playerWasCrushed = false;
	_forceEndGame = false;
	_syncSound = false;
	_firstSound = false;
	_playerHeightNumber = 1;
	_angleRotationIndex = 0;

	// TODO: this is not the same for every game
	_playerStepIndex = 6;
	_playerSteps.push_back(1);
	_playerSteps.push_back(2);
	_playerSteps.push_back(5);
	_playerSteps.push_back(10);
	_playerSteps.push_back(25);
	_playerSteps.push_back(50);
	_playerSteps.push_back(100);

	_border = nullptr;
	_title = nullptr;
	_titleTexture = nullptr;
	_borderTexture = nullptr;
	_uiTexture = nullptr;
	_fontLoaded = false;
	_dataBundle = nullptr;

	_lastFrame = 0;
	_nearClipPlane = 2;
	_farClipPlane = 8192 + 1802; // Added some extra distance to avoid flickering

	// These depends on the specific game
	_playerHeight = 0;
	_playerWidth = 0;
	_playerDepth = 0;
	_stepUpDistance = 0;
	_colorNumber = 0;

	_fullscreenViewArea = Common::Rect(0, 0, _screenW, _screenH);
	_viewArea = _fullscreenViewArea;
	_rnd = new Common::RandomSource("freescape");
	_gfx = nullptr;
	_rawCGAPaletteByArea = nullptr;
	_speaker = nullptr;
	_savedScreen = nullptr;

	_timerStarted = false;
	_initialCountdown = 0;
	_countdown = 0;
	_ticks = 0;
	_ticksFromEnd = 0;
	_lastTick = -1;
	_lastMinute = -1;
	_frameLimiter = nullptr;
	_vsyncEnabled = false;

	_underFireFrames = 0;
	_shootingFrames = 0;

	_maxShield = 63;
	_maxEnergy = 63;
	_gameStateBits = 0;
	_eventManager = new EventManagerWrapper(g_system->getEventManager());

	// Workaround to make the game playable on iOS: remove when there
	// is a better way to hint the best controls
#ifdef IPHONE
	const Common::String &gameDomain = ConfMan.getActiveDomainName();
	ConfMan.setBool("gamepad_controller", true, gameDomain);
	ConfMan.setBool("gamepad_controller_minimal_layout", true, gameDomain);
	ConfMan.setInt("gamepad_controller_directional_input", 1 /* kDirectionalInputDpad */, gameDomain);
#endif

	g_freescape = this;
}

FreescapeEngine::~FreescapeEngine() {
	removeTimers();
	delete _rnd;

	if (_title && _title != _border) {
		_title->free();
		delete _title;
	}

	if (_border) {
		_border->free();
		delete _border;
	}

	if (_gfx->_isAccelerated) {
		delete _borderTexture;
		delete _uiTexture;
		delete _titleTexture;
	}

	for (auto &it : _areaMap) {
		delete it._value;
	}

	delete _gfx;
	delete _dataBundle;
	delete _speaker;

	for (auto &it : _indicators) {
		it->free();
		delete it;
	}

	for (auto &it : _soundsFx) {
		if (it._value) {
			free(it._value->data);
			free(it._value);
		}
	}
}

void FreescapeEngine::drawBorder() {
	if (!_border)
		return;

	_gfx->setViewport(_fullscreenViewArea);
	assert(_borderTexture);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _borderTexture);
	_gfx->setViewport(_viewArea);
}

void FreescapeEngine::drawTitle() {
	_gfx->setViewport(_fullscreenViewArea);
	if (_title) {
		if (!_titleTexture) {
			Graphics::Surface *title = _gfx->convertImageFormatIfNecessary(_title);
			_titleTexture = _gfx->createTexture(title);
			title->free();
			delete title;
		}
		_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _titleTexture);
	}
	_gfx->setViewport(_viewArea);
}

// Taken from the Myst 3 codebase, it should be abstracted
Math::Vector3d FreescapeEngine::directionToVector(float pitch, float heading) {
	Math::Vector3d v;

	float radHeading = Common::deg2rad(heading);
	float radPitch = Common::deg2rad(pitch);

	v.setValue(0, cos(radPitch) * cos(radHeading));
	v.setValue(1, sin(radPitch));
	v.setValue(2, cos(radPitch) * sin(radHeading));
	v.normalize();

	return v;
}

void FreescapeEngine::centerCrossair() {
	_crossairPosition.x = _viewArea.left + _viewArea.width() / 2;
	_crossairPosition.y = _viewArea.top + _viewArea.height() / 2;
	_currentDemoMousePosition = _crossairPosition;
}

void FreescapeEngine::checkSensors() {
	if (_disableSensors)
		return;

	if (_lastTick == _ticks)
		return;

	_lastTick = _ticks;
	for (auto &it : _sensors) {
		Sensor *sensor = (Sensor *)it;
		bool playerDetected = sensor->playerDetected(_position, _currentArea);
		if (playerDetected) {
			if (_ticks % sensor->_firingInterval == 0) {
				if (_underFireFrames <= 0)
					_underFireFrames = 4;
				takeDamageFromSensor();
			}
		}
		sensor->shouldShoot(playerDetected);
	}
}

void FreescapeEngine::drawSensorShoot(Sensor *sensor) {}

void FreescapeEngine::flashScreen(int backgroundColor) {
	if (backgroundColor >= 16)
		return;
	_currentArea->remapColor(_currentArea->_usualBackgroundColor, backgroundColor);
	_currentArea->remapColor(_currentArea->_skyColor, backgroundColor);
	drawFrame();
	_currentArea->unremapColor(_currentArea->_usualBackgroundColor);
	_currentArea->unremapColor(_currentArea->_skyColor);
}

void FreescapeEngine::takeDamageFromSensor() {
	_gameStateVars[k8bitVariableShield]--;
}

void FreescapeEngine::clearBackground() {
	_gfx->clear(0, 0, 0, true);
	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawBackground(_currentArea->_usualBackgroundColor);
	_gfx->setViewport(_viewArea);
}

void FreescapeEngine::drawBackground() {
	clearBackground();
	_gfx->drawBackground(_currentArea->_skyColor);
}

void FreescapeEngine::drawFrame() {
	_gfx->updateProjectionMatrix(70.0, _nearClipPlane, _farClipPlane);
	_gfx->positionCamera(_position, _position + _cameraFront);

	if (_underFireFrames > 0) {
		int underFireColor = _currentArea->_underFireBackgroundColor;

		if (isDriller() && (isDOS() || isAmiga() || isAtariST()))
			underFireColor = 1;
		else if (isDark() && (isDOS() || isAmiga() || isAtariST()))
			underFireColor = 4;

		_currentArea->remapColor(_currentArea->_usualBackgroundColor, underFireColor);
		_currentArea->remapColor(_currentArea->_skyColor, underFireColor);
	}

	drawBackground();
	if (_avoidRenderingFrames == 0) // Avoid rendering inside objects
		_currentArea->draw(_gfx, _ticks / 10);
	else
		_avoidRenderingFrames--;

	if (_underFireFrames > 0) {
		for (auto &it : _sensors) {
			Sensor *sensor = (Sensor *)it;
			if (sensor->isShooting())
				drawSensorShoot(sensor);
		}
		_underFireFrames--;
		if (_underFireFrames == 0) {
			_currentArea->unremapColor(_currentArea->_usualBackgroundColor);
			_currentArea->unremapColor(_currentArea->_skyColor);
		}
	}

	if (_shootingFrames > 0) {
		_gfx->setViewport(_fullscreenViewArea);
		if (isDriller() || isDark())
			_gfx->renderPlayerShootRay(0, _crossairPosition, _viewArea);
		else
			_gfx->renderPlayerShootBall(0, _crossairPosition, _shootingFrames, _viewArea);

		_gfx->setViewport(_viewArea);
		_shootingFrames--;
	}

	drawBorder();
	drawUI();
}

void FreescapeEngine::pressedKey(const int keycode) {}

void FreescapeEngine::releasedKey(const int keycode) {}

void FreescapeEngine::resetInput() {
	_shootMode = false;
	centerCrossair();
	g_system->warpMouse(_crossairPosition.x, _crossairPosition.y);
	_eventManager->purgeMouseEvents();
	_eventManager->purgeKeyboardEvents();
	rotate(0, 0);
}


void FreescapeEngine::processInput() {
	float currentFrame = g_system->getMillis();
	float deltaTime = 20.0;
	_lastFrame = currentFrame;
	Common::Event event;
	Common::Point mousePos;

	if (_demoMode && !_demoEvents.empty()) {
		_eventManager->purgeMouseEvents();
		_eventManager->purgeKeyboardEvents();
		_eventManager->pushEvent(_demoEvents.front());
		_demoEvents.remove_at(0);
	}

	while (_eventManager->pollEvent(event)) {
		if (_gameStateControl != kFreescapeGameStatePlaying) {
			if (event.type == Common::EVENT_SCREEN_CHANGED)
				; // Allow event
			else if (_gameStateControl == kFreescapeGameStateEnd && event.type == Common::EVENT_KEYDOWN) {
				_endGameKeyPressed = true;
				continue;
			} else if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				; // Allow event
			else if (event.customType != 0xde00)
				continue;
		}

		switch (event.type) {
		case Common::EVENT_JOYBUTTON_DOWN:
			if (_hasFallen || _playerWasCrushed)
				break;
			switch (event.joystick.button) {
			case Common::JOYSTICK_BUTTON_B:
			case Common::JOYSTICK_BUTTON_DPAD_UP:
				move(kForwardMovement, _scaleVector.x(), deltaTime);
				break;
			case Common::JOYSTICK_BUTTON_DPAD_DOWN:
				move(kBackwardMovement, _scaleVector.x(), deltaTime);
				break;
			case Common::JOYSTICK_BUTTON_DPAD_LEFT:
				move(kLeftMovement, _scaleVector.y(), deltaTime);
				break;
			case Common::JOYSTICK_BUTTON_DPAD_RIGHT:
				move(kRightMovement, _scaleVector.y(), deltaTime);
				break;
			}
		break;
		case Common::EVENT_KEYDOWN:
			if (_hasFallen)
				break;
			switch (event.kbd.keycode) {
			case Common::KEYCODE_o:
			case Common::KEYCODE_UP:
				move(kForwardMovement, _scaleVector.x(), deltaTime);
				break;
			case Common::KEYCODE_k:
			case Common::KEYCODE_DOWN:
				move(kBackwardMovement, _scaleVector.x(), deltaTime);
				break;
			case Common::KEYCODE_LEFT:
				move(kLeftMovement, _scaleVector.y(), deltaTime);
				break;
			case Common::KEYCODE_RIGHT:
				move(kRightMovement, _scaleVector.y(), deltaTime);
				break;
			case Common::KEYCODE_KP5:
			case Common::KEYCODE_KP0:
			case Common::KEYCODE_0:
				shoot();
				break;
			case Common::KEYCODE_p:
				rotate(0, 5);
				break;
			case Common::KEYCODE_l:
				rotate(0, -5);
				break;
			case Common::KEYCODE_u:
				rotate(180, 0);
				break;
			case Common::KEYCODE_q:
				rotate(-_angleRotations[_angleRotationIndex], 0);
				break;
			case Common::KEYCODE_w:
				rotate(_angleRotations[_angleRotationIndex], 0);
				break;
			case Common::KEYCODE_s:
				increaseStepSize();
				break;
			case Common::KEYCODE_x:
				decreaseStepSize();
				break;
			case Common::KEYCODE_r:
				if (isEclipse())
					pressedKey(Common::KEYCODE_r);
				else
					rise();
				break;
			case Common::KEYCODE_f:
				lower();
				break;
			case Common::KEYCODE_n:
				_noClipMode = !_noClipMode;
				_flyMode = _noClipMode;
				break;
			case Common::KEYCODE_ESCAPE:
				drawFrame();
				_savedScreen = _gfx->getScreenshot();
				openMainMenuDialog();
				_gfx->computeScreenViewport();
				_savedScreen->free();
				delete _savedScreen;
				break;
			case Common::KEYCODE_SPACE:
				_shootMode = !_shootMode;
				centerCrossair();
				if (!_shootMode) {
					g_system->lockMouse(true);
				} else {
					g_system->lockMouse(false);
					g_system->warpMouse(_crossairPosition.x, _crossairPosition.y);
					_eventManager->purgeMouseEvents();
					_eventManager->purgeKeyboardEvents();
				}
				break;
			case Common::KEYCODE_i:
				drawInfoMenu();
				break;
			default:
				pressedKey(event.kbd.keycode);
				break;
			}
			break;

		case Common::EVENT_KEYUP:
			if (_hasFallen)
				break;

			releasedKey(event.kbd.keycode);
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitGame();
			return;

		case Common::EVENT_SCREEN_CHANGED:
			_gfx->computeScreenViewport();
			_gfx->clear(0, 0, 0, true);
			break;

		case Common::EVENT_MOUSEMOVE:
			if (_hasFallen)
				break;
			mousePos = event.mouse;

			if (_demoMode)
				g_system->warpMouse(mousePos.x, mousePos.y);

			if (_shootMode) {
				_crossairPosition.x = _screenW * mousePos.x / g_system->getWidth();
				_crossairPosition.y = _screenH * mousePos.y / g_system->getHeight();
				break;
			} else {
				// Mouse pointer is locked into the the middle of the screen
				// since we only need the relative movements. This will not affect any touchscreen device
				// so on-screen controls are still accesible
				mousePos.x = g_system->getWidth() * ( _viewArea.left + _viewArea.width() / 2) / _screenW;
				mousePos.y = g_system->getHeight() * (_viewArea.top + _viewArea.height() / 2) / _screenW;
				if (_invertY)
					event.relMouse.y = -event.relMouse.y;

				g_system->warpMouse(mousePos.x, mousePos.y);
				_eventManager->purgeMouseEvents();
			}

			rotate(event.relMouse.x * _mouseSensitivity, event.relMouse.y * _mouseSensitivity);
			break;

		case Common::EVENT_LBUTTONDOWN:
			if (_hasFallen)
				break;
			mousePos = event.mouse;
			{
				bool touchedScreenControls = false;

				Common::Point resolution(g_system->getWidth(), g_system->getHeight());
				mousePos.x = _screenW * mousePos.x / resolution.x;
				mousePos.y = _screenH * mousePos.y / resolution.y;
				touchedScreenControls = onScreenControls(mousePos);

				if (!touchedScreenControls) {
					if (_viewArea.contains(_shootMode ? _crossairPosition : mousePos))
						shoot();
				}

			}
			break;

		case Common::EVENT_RBUTTONDOWN:
			if (_hasFallen || !isCastle())
				break;
			activate();
			break;

		default:
			break;
		}
	}
}

bool FreescapeEngine::onScreenControls(Common::Point mouse) {
	return false;
}

void FreescapeEngine::executeMovementConditions() {
	// Only execute "on collision" room/global conditions
	executeLocalGlobalConditions(false, true, false);
}

void FreescapeEngine::updateTimeVariables() {}

Common::Error FreescapeEngine::run() {
	_vsyncEnabled = g_system->getFeatureState(OSystem::kFeatureVSync);
	_frameLimiter = new Graphics::FrameLimiter(g_system, ConfMan.getInt("engine_speed"));
	// Initialize graphics
	_screenW = g_system->getWidth();
	_screenH = g_system->getHeight();
	_gfx = createRenderer(_screenW, _screenH, _renderMode);
	_speaker = new SizedPCSpeaker();
	_speaker->setVolume(50);
	_crossairPosition.x = _screenW / 2;
	_crossairPosition.y = _screenH / 2;

	// The following error code will force return to launcher
	// but it will not force any other GUI message to be displayed
	if (!_gfx)
		return Common::kUserCanceled;

	_gfx->init();

	// Load game data and init game state
	loadDataBundle();
	loadAssets();
	initGameState();
	loadColorPalette();

	g_system->showMouse(true);
	g_system->lockMouse(false);

	// Simple main event loop
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot == -1)
		titleScreen();
	loadBorder(); // Border is load unmodified
	if (saveSlot == -1)
		borderScreen();
	processBorder(); // Border is processed to use during the game

	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else
		gotoArea(_startArea, _startEntrance);

	debugC(1, kFreescapeDebugMove, "Starting area %d", _currentArea->getAreaID());
	// Draw first frame

	g_system->showMouse(false);
	g_system->lockMouse(true);
	resetInput();
	_gfx->computeScreenViewport();
	_gfx->clear(0, 0, 0, true);
	_gfx->flipBuffer();
	g_system->updateScreen();

	while (!shouldQuit()) {
		updateTimeVariables();
		if (_gameStateControl == kFreescapeGameStateRestart) {
			initGameState();
			gotoArea(_startArea, _startEntrance);
		} else if (_gameStateControl == kFreescapeGameStateEnd)
			endGame();

		processInput();
		if (_demoMode)
			generateDemoInput();

		checkSensors();
		drawFrame();

		_gfx->flipBuffer();
		_frameLimiter->delayBeforeSwap();
		g_system->updateScreen();
		_frameLimiter->startFrame();
		if (_vsyncEnabled) // if vsync is enabled, the framelimiter will not work
			g_system->delayMillis(15); // try to target ~60 FPS

		checkIfGameEnded();
	}

	_eventManager->clearExitEvents();
	return Common::kNoError;
}

void FreescapeEngine::endGame() {
	if (_endGameKeyPressed) {
		_gameStateControl = kFreescapeGameStateRestart;
	}
}

void FreescapeEngine::loadBorder() {
	if (_border) {
		Graphics::Surface *border = _gfx->convertImageFormatIfNecessary(_border);
		_borderTexture = _gfx->createTexture(border);
		border->free();
		delete border;
	}
}

void FreescapeEngine::processBorder() {
	if (_border) {
		if (_borderTexture)
			delete _borderTexture;
		Graphics::Surface *border = _gfx->convertImageFormatIfNecessary(_border);

		uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
		border->fillRect(_viewArea, gray);

		// Replace black pixel for transparent ones
		uint32 black = border->format.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
		uint32 transparent = border->format.ARGBToColor(0x00, 0x00, 0x00, 0x00);

		for (int i = 0; i < border->w; i++) {
			for (int j = 0; j < border->h; j++) {
				if (!isCastle() && border->getPixel(i, j) == black)
					border->setPixel(i, j, transparent);
			}
		}

		_borderTexture = _gfx->createTexture(border);
		border->free();
		delete border;
	}
}

bool FreescapeEngine::checkIfGameEnded() {
	if (_gameStateControl != kFreescapeGameStatePlaying)
		return false;

	if (_avoidRenderingFrames > 0)
		return false;

	if (_gameStateVars[k8bitVariableShield] == 0) {
		if (!_noShieldMessage.empty())
			insertTemporaryMessage(_noShieldMessage, _countdown - 2);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_gameStateVars[k8bitVariableEnergy] == 0) {
		if (!_noEnergyMessage.empty())
			insertTemporaryMessage(_noEnergyMessage, _countdown - 2);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_hasFallen) {
		_hasFallen = false;
		playSound(14, false);
		if (!_fallenMessage.empty())
			insertTemporaryMessage(_fallenMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_countdown <= 0) {
		if (!_timeoutMessage.empty())
			insertTemporaryMessage(_timeoutMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_playerWasCrushed) {
		_playerWasCrushed = false;
		if (!_crushedMessage.empty())
			insertTemporaryMessage(_crushedMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_forceEndGame) {
		if (isSpectrum())
			playSound(8, true);
		_forceEndGame = false;
		if (!_forceEndGameMessage.empty())
			insertTemporaryMessage(_forceEndGameMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	}

	if (_gameStateControl == kFreescapeGameStateEnd)
		gotoArea(_endArea, _endEntrance);

	return false; // TODO
}

void FreescapeEngine::setGameBit(int index) {
	_gameStateBits |= (1 << (index - 1));
}

void FreescapeEngine::clearGameBit(int index) {
	_gameStateBits &= ~(1 << (index - 1));
}

void FreescapeEngine::toggleGameBit(int index) {
	_gameStateBits ^= (1 << (index - 1));
}

uint16 FreescapeEngine::getGameBit(int index) {
	return (_gameStateBits >> (index - 1)) & 1;
}

void FreescapeEngine::initGameState() {
	_gameStateControl = kFreescapeGameStatePlaying;

	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap)
		it._value->resetArea();

	_gameStateBits = 0;

	_flyMode = false;
	_noClipMode = false;
	_playerWasCrushed = false;
	_shootingFrames = 0;
	_underFireFrames = 0;
	_avoidRenderingFrames = 0;
	_yaw = 0;
	_pitch = 0;
	_endGameKeyPressed = false;

	_demoIndex = 0;
	_demoEvents.clear();

	removeTimers();
	startCountdown(_initialCountdown - 1);
	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	_lastMinute = minutes;
}

void FreescapeEngine::rotate(float xoffset, float yoffset) {
	_yaw -= xoffset;
	_pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (_pitch > 360.0f)
		_pitch -= 360.0f;
	if (_pitch < 0.0f)
		_pitch += 360.0f;

	if (_yaw > 360.0f)
		_yaw -= 360.0f;
	if (_yaw < 0.0f)
		_yaw += 360.0f;

	updateCamera();
}

void FreescapeEngine::updateCamera() {
	_cameraFront = directionToVector(_pitch, _yaw);
	// _right = _front x _up;
	Math::Vector3d v = Math::Vector3d::crossProduct(_cameraFront, _upVector);
	v.normalize();
	_cameraRight = v;
}

bool FreescapeEngine::hasFeature(EngineFeature f) const {
	// The TinyGL renderer does not support arbitrary resolutions for now
	bool softRenderer = determinateRenderType() == Graphics::kRendererTypeTinyGL;
	return (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime) ||
		   (f == kSupportsArbitraryResolutions && !softRenderer);
}

void FreescapeEngine::drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface, int offset) {
	if (!_fontLoaded)
		return;
	Common::String ustr = str;
	ustr.toUppercase();

	int sizeX = 8;
	int sizeY = isCastle() ? 8 : 6;
	int sep = isCastle() ? 9 : 8;
	int additional = isCastle() || isEclipse() ? 0 : 1;

	if (isDOS() || isSpectrum() || isCPC() || isC64()) {
		for (uint32 c = 0; c < ustr.size(); c++) {
			assert(ustr[c] >= 32);
			for (int j = 0; j < sizeY; j++) {
				for (int i = 0; i < sizeX; i++) {
					if (_font.get(sizeX * sizeY * (offset + ustr[c] - 32) + additional + j * 8 + i))
						surface->setPixel(x + 8 - i + sep * c, y + j, fontColor);
					else
						surface->setPixel(x + 8 - i + sep * c, y + j, backColor);
				}
			}
		}
	} else if (isAmiga() || isAtariST()) {
		for (uint32 c = 0; c < ustr.size(); c++) {
			assert(ustr[c] >= 32);
			int position = 8 * (33*(offset + ustr[c] - 32) + 1);
			for (int j = 0; j < 8; j++) {
				for (int i = 0; i < 8; i++) {
					if (_font.get(position + j * 32 + i))
						surface->setPixel(x + 8 - i + 8 * c, y + j, fontColor);
					else
						surface->setPixel(x + 8 - i + 8 * c, y + j, backColor);
				}
			}
		}
	}
}

Common::Error FreescapeEngine::loadGameStream(Common::SeekableReadStream *stream) {

	uint16 areaID = stream->readUint16LE();
	for (int i = 0; i < 3; i++)
		_position.setValue(i, stream->readFloatLE());

	for (int i = 0; i < 3; i++)
		_rotation.setValue(i, stream->readFloatLE());

	_yaw = stream->readFloatLE();
	_pitch = stream->readFloatLE();

	// Level state
	for (uint i = 0; i < _gameStateVars.size(); i++) {
		uint16 key = stream->readUint16LE();
		_gameStateVars[key] = stream->readUint32LE();
	}

	_gameStateBits = stream->readUint32LE();

	for (uint i = 0; i < _areaMap.size(); i++) {
		uint16 key = stream->readUint16LE();
		assert(_areaMap.contains(key));
		Area *area = _areaMap[key];
		area->loadObjects(stream, _areaMap[255]);
	}

	_flyMode = stream->readByte();
	_noClipMode = false;
	_playerHeightNumber = stream->readUint32LE();
	_countdown = stream->readUint32LE();
	_ticks = 0;
	if (!_currentArea || _currentArea->getAreaID() != areaID)
		gotoArea(areaID, -1); // Do not change position nor rotation
	return loadGameStreamExtended(stream);
}

Common::Error FreescapeEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (isAutosave)
		return Common::kNoError;

	stream->writeUint16LE(_currentArea->getAreaID());

	for (int i = 0; i < 3; i++)
		stream->writeFloatLE(_position.getValue(i));

	for (int i = 0; i < 3; i++)
		stream->writeFloatLE(_rotation.getValue(i));

	stream->writeFloatLE(_yaw);
	stream->writeFloatLE(_pitch);

	// Level state
	for (auto &it : _gameStateVars) {
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(it._value);
	}

	stream->writeUint32LE(_gameStateBits);

	for (auto &it : _areaMap) {
		stream->writeUint16LE(it._key);
		it._value->saveObjects(stream);
	}

	stream->writeByte(_flyMode);
	stream->writeUint32LE(_playerHeightNumber);
	stream->writeUint32LE(_countdown);
	return saveGameStreamExtended(stream, isAutosave);
}

Common::Error FreescapeEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error FreescapeEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	return Common::kNoError;
}

void FreescapeEngine::insertTemporaryMessage(const Common::String message, int deadline) {
	_temporaryMessages.insert_at(0, message);
	_temporaryMessageDeadlines.insert_at(0, deadline);
}

void FreescapeEngine::getLatestMessages(Common::String &message, int &deadline) {
	deadline = _countdown + 1;
	message.clear();
	while (!_temporaryMessages.empty() && deadline > _countdown) {
		message = _temporaryMessages.back();
		deadline = _temporaryMessageDeadlines.back();
		_temporaryMessages.pop_back();
		_temporaryMessageDeadlines.pop_back();
	}
}

void FreescapeEngine::clearTemporalMessages() {
	_temporaryMessages.clear();
	_temporaryMessageDeadlines.clear();
}

byte *FreescapeEngine::getPaletteFromNeoImage(Common::SeekableReadStream *stream, int offset) {
	stream->seek(offset);
	Image::NeoDecoder decoder;
	decoder.loadStream(*stream);
	byte *palette = (byte *)malloc(16 * 3 * sizeof(byte));
	memcpy(palette, decoder.getPalette(), 16 * 3 * sizeof(byte));
	return palette;
}

Graphics::ManagedSurface *FreescapeEngine::loadAndConvertNeoImage(Common::SeekableReadStream *stream, int offset, byte *palette) {
	stream->seek(offset);
	Image::NeoDecoder decoder(palette);
	decoder.loadStream(*stream);
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->copyFrom(*decoder.getSurface());
	surface->convertToInPlace(_gfx->_currentPixelFormat, decoder.getPalette(),
		decoder.getPaletteStartIndex(), decoder.getPaletteColorCount());
	return surface;
}

Graphics::ManagedSurface *FreescapeEngine::loadAndCenterScrImage(Common::SeekableReadStream *stream) {
	Image::ScrDecoder decoder;
	decoder.loadStream(*stream);
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	const Graphics::Surface *decoded = decoder.getSurface();
	surface->create(320, 200, decoded->format);
	surface->copyRectToSurface(*decoded, (320 - decoded->w) / 2, (200 - decoded->h) / 2, Common::Rect(decoded->w, decoded->h));
	return surface;
}

void FreescapeEngine::getTimeFromCountdown(int &seconds, int &minutes, int &hours) {
	int countdown = _countdown;
	int h = countdown <= 0 ? 0 : countdown / 3600;
	int m = countdown <= 0 ? 0 : (countdown - h * 3600) / 60;
	int s = countdown <= 0 ? 0 : countdown - h * 3600 - m * 60;

	seconds = s;
	minutes = m;
	hours = h;
}

static void countdownCallback(void *refCon) {
	FreescapeEngine* self = (FreescapeEngine *)refCon;
	if (self->isPaused())
		return;
	self->_ticks++;
	if (self->_ticks % 50 == 0)
		self->_countdown--;
}

bool FreescapeEngine::startCountdown(uint32 delay) {
	_countdown = delay;
	_ticks = 0;
	_timerStarted = true;
	uint32 oneTick = 1000000 / 50;
	return g_system->getTimerManager()->installTimerProc(&countdownCallback, oneTick, (void *)this, "countdown");
}

void FreescapeEngine::removeTimers() {
	_timerStarted = false;
	g_system->getTimerManager()->removeTimerProc(&countdownCallback);
}

void FreescapeEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	// TODO: Handle the viewport here
	if (_frameLimiter)
		_frameLimiter->pause(pause);

	// Unlock the mouse so that the cursor is usable when the GMM opens
	if (!_shootMode) {
		_system->lockMouse(!pause);
	}
}

} // namespace Freescape
