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
#include "common/random.h"
#include "common/timer.h"
#include "graphics/cursorman.h"
#include "image/neo.h"
#include "image/scr.h"
#include "math/utils.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/objects/sensor.h"
#include "freescape/sweepAABB.h"

namespace Freescape {

FreescapeEngine *g_freescape;

FreescapeEngine::FreescapeEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _gfx(nullptr) {
	if (!ConfMan.hasKey("render_mode") || ConfMan.get("render_mode").empty())
		_renderMode = Common::kRenderEGA;
	else
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));

	_binaryBits = 0;
	if (_renderMode == Common::kRenderHercG) {
		_screenW = 720;
		_screenH = 348;
	} else {
		_screenW = 320;
		_screenH = 200;
	}

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
		error("Failed to parse bool from invert_y option");

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
	_playerHeightMaxNumber = 1;
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
	_background = nullptr;
	_titleTexture = nullptr;
	_borderTexture = nullptr;
	_skyTexture = nullptr;
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

	_soundIndexShoot = 1;
	_soundIndexCollide = -1;
	_soundIndexFall = -1;
	_soundIndexClimb = -1;
	_soundIndexMenu = -1;
	_soundIndexStart = -1;
	_soundIndexAreaChange = -1;
	_soundIndexHit = -1;

	_soundIndexNoShield = -1;
	_soundIndexNoEnergy = -1;
	_soundIndexFallen = -1;
	_soundIndexTimeout = -1;
	_soundIndexForceEndGame = -1;
	_soundIndexCrushed = -1;
	_soundIndexMissionComplete = -1;

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
	_executingGlobalCode = false;

	_underFireFrames = 0;
	_shootingFrames = 0;
	_delayedShootObject = nullptr;
	_avoidRenderingFrames = 0;
	_endGamePlayerEndArea = false;
	_endGameKeyPressed = false;
	_endGameDelayTicks = 0;

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

	if (_background) {
		_background->free();
		delete _background;
	}

	if (_gfx->_isAccelerated) {
		delete _borderTexture;
		delete _uiTexture;
		delete _titleTexture;
		delete _skyTexture;
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

	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
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

static uint8 kCosineSineTable [72][2] {
	// Each "dw" contains (cos, sin) (one byte each):
	// [-64, 64]
	// 72 steps is a whole turn.
	{ 0x40, 0x00 }, { 0x40, 0x06 }, { 0x3f, 0x0b }, { 0x3e, 0x11 },
	{ 0x3c, 0x16 }, { 0x3a, 0x1b }, { 0x37, 0x20 }, { 0x34, 0x25 },
	{ 0x31, 0x29 }, { 0x2d, 0x2d }, { 0x29, 0x31 }, { 0x25, 0x34 },
	{ 0x20, 0x37 }, { 0x1b, 0x3a }, { 0x16, 0x3c }, { 0x11, 0x3e },
	{ 0x0b, 0x3f }, { 0x06, 0x40 }, { 0x00, 0x40 }, { 0xfa, 0x40 },
	{ 0xf5, 0x3f }, { 0xef, 0x3e }, { 0xea, 0x3c }, { 0xe5, 0x3a },
	{ 0xe0, 0x37 }, { 0xdb, 0x34 }, { 0xd7, 0x31 }, { 0xd3, 0x2d },
	{ 0xcf, 0x29 }, { 0xcc, 0x25 }, { 0xc9, 0x20 }, { 0xc6, 0x1b },
	{ 0xc4, 0x16 }, { 0xc2, 0x11 }, { 0xc1, 0x0b }, { 0xc0, 0x06 },
	{ 0xc0, 0x00 }, { 0xc0, 0xfa }, { 0xc1, 0xf5 }, { 0xc2, 0xef },
	{ 0xc4, 0xea }, { 0xc6, 0xe5 }, { 0xc9, 0xe0 }, { 0xcc, 0xdb },
	{ 0xcf, 0xd7 }, { 0xd3, 0xd3 }, { 0xd7, 0xcf }, { 0xdb, 0xcc },
	{ 0xe0, 0xc9 }, { 0xe5, 0xc6 }, { 0xea, 0xc4 }, { 0xef, 0xc2 },
	{ 0xf5, 0xc1 }, { 0xfa, 0xc0 }, { 0x00, 0xc0 }, { 0x06, 0xc0 },
	{ 0x0b, 0xc1 }, { 0x11, 0xc2 }, { 0x16, 0xc4 }, { 0x1b, 0xc6 },
	{ 0x20, 0xc9 }, { 0x25, 0xcc }, { 0x29, 0xcf }, { 0x2d, 0xd3 },
	{ 0x31, 0xd7 }, { 0x34, 0xdb }, { 0x37, 0xe0 }, { 0x3a, 0xe5 },
	{ 0x3c, 0xea }, { 0x3e, 0xef }, { 0x3f, 0xf5 }, { 0x40, 0xfa }
};

// Taken from the Myst 3 codebase, it should be abstracted
Math::Vector3d FreescapeEngine::directionToVector(float pitch, float heading, bool useTable) {
	Math::Vector3d v;

	if (useTable) {
		int pitchInt = (int)pitch;
		int headingInt = (int)heading;

		if (pitchInt < 0)
			pitchInt = 360 + pitchInt;
		if (pitchInt == 360)
			pitchInt = 0;

		if (headingInt < 0)
			headingInt = 360 + headingInt;
		if (headingInt == 360)
			headingInt = 0;

		int headingIndex = headingInt / 5;
		int pitchIndex = pitchInt / 5;

		v.setValue(0, ((int8)kCosineSineTable[pitchIndex][0] / 64.0) * ((int8)kCosineSineTable[headingIndex][0] / 64.0));
		v.setValue(1, (int8)kCosineSineTable[pitchIndex][1] / 64.0);
		v.setValue(2, ((int8)kCosineSineTable[pitchIndex][0] / 64.0) * (int8)kCosineSineTable[headingIndex][1] / 64.0);
	} else {
		float radHeading = Math::deg2rad(heading);
		float radPitch = Math::deg2rad(pitch);

		v.setValue(0, cos(radPitch) * cos(radHeading));
		v.setValue(1, sin(radPitch));
		v.setValue(2, cos(radPitch) * sin(radHeading));
	}
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

	if (isCastle() && _background) {
		if (!_skyTexture)
			_skyTexture = _gfx->createTexture(_background->surfacePtr(), true);
		_gfx->drawSkybox(_skyTexture, _position);
	}
}

void FreescapeEngine::drawFrame() {
	int farClipPlane = _farClipPlane;
	if (_currentArea->isOutside())
		farClipPlane *= 100;

	float aspectRatio = isCastle() ? 1.6 : 2.18;
	_gfx->updateProjectionMatrix(75.0, aspectRatio, _nearClipPlane, farClipPlane);
	_gfx->positionCamera(_position, _position + _cameraFront);

	if (_underFireFrames > 0) {
		int underFireColor = _currentArea->_underFireBackgroundColor;

		if (isDriller() && (isDOS() || isAmiga() || isAtariST()))
			underFireColor = 1;
		else if (isDark() && (isDOS() || isAmiga() || isAtariST())) {
			if (_renderMode == Common::kRenderCGA)
				underFireColor = 3;
			else
				underFireColor = 4;
		}

		_currentArea->remapColor(_currentArea->_usualBackgroundColor, underFireColor);
		_currentArea->remapColor(_currentArea->_skyColor, underFireColor);
	}

	drawBackground();
	if (_avoidRenderingFrames == 0) { // Avoid rendering inside objects
		_currentArea->draw(_gfx, _ticks / 10, _position, _cameraFront);
		if (_gameStateControl == kFreescapeGameStatePlaying &&
		    _currentArea->hasActiveGroups() && _ticks % 50 == 0) {
			executeMovementConditions();
		}
	} else
		_avoidRenderingFrames--;

	if (_underFireFrames > 0) {
		for (auto &it : _sensors) {
			Sensor *sensor = (Sensor *)it;
			if (it->isDestroyed() || it->isInvisible())
				continue;
			if (isCastle() || sensor->isShooting())
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
			else if (_gameStateControl == kFreescapeGameStateEnd
						&& (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START)) {
				_endGameKeyPressed = true;
				continue;
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START && event.customType == kActionEscape)
				; // Allow event
			else if (event.customType != 0xde00)
				continue;
		}

		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (_hasFallen || _playerWasCrushed)
				break;
			switch (event.customType) {
			case kActionMoveUp:
				move(kForwardMovement, _scaleVector.x(), deltaTime);
				break;
			case kActionMoveDown:
				move(kBackwardMovement, _scaleVector.x(), deltaTime);
				break;
			case kActionMoveLeft:
				move(kLeftMovement, _scaleVector.y(), deltaTime);
				break;
			case kActionMoveRight:
				move(kRightMovement, _scaleVector.y(), deltaTime);
				break;
			case kActionShoot:
				shoot();
				break;
			case kActionRotateUp:
				rotate(0, 5);
				break;
			case kActionRotateDown:
				rotate(0, -5);
				break;
			case kActionTurnBack:
				rotate(180, 0);
				break;
			case kActionToggleClipMode:
				_noClipMode = !_noClipMode;
				_flyMode = _noClipMode;
				break;
			case kActionEscape:
				drawFrame();
				_savedScreen = _gfx->getScreenshot();
				openMainMenuDialog();
				_gfx->computeScreenViewport();
				_savedScreen->free();
				delete _savedScreen;
				_savedScreen = nullptr;
				break;
			case kActionChangeMode:
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
			case kActionInfoMenu:
				drawInfoMenu();
				break;
			default:
				pressedKey(event.customType);
				break;
			}
			break;
		case Common::EVENT_KEYDOWN:
			if (_hasFallen || _playerWasCrushed)
				break;

			pressedKey(event.kbd.keycode);
			break;

		case Common::EVENT_KEYUP:
			if (_hasFallen || _playerWasCrushed)
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
			if (_hasFallen || _playerWasCrushed)
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
			if (_hasFallen || _playerWasCrushed)
				break;
			mousePos = event.mouse;
			{
				bool touchedScreenControls = false;
				mousePos = getNormalizedPosition(mousePos);
				touchedScreenControls = onScreenControls(mousePos);

				if (!touchedScreenControls) {
					if (_viewArea.contains(_shootMode ? _crossairPosition : mousePos))
						shoot();
				}

			}
			break;

		case Common::EVENT_RBUTTONDOWN:
			if (_hasFallen || _playerWasCrushed || !isCastle())
				break;
			activate();
			break;

		default:
			break;
		}
	}
}

Common::Point FreescapeEngine::getNormalizedPosition(Common::Point position) {
	// Retrieve the screen and viewport dimensions
	Common::Point resolution(g_system->getWidth(), g_system->getHeight());

	int32 viewportWidth = MIN<int32>(resolution.x, resolution.y * float(4) / 3);
	int32 viewportHeight = MIN<int32>(resolution.y, resolution.x * float(3) / 4);

	// Calculate pillarbox offset
	int32 offsetX = (resolution.x - viewportWidth) / 2;
	int32 offsetY = (resolution.y - viewportHeight) / 2;

	// Adjust mouse position by removing the offset
	position.x -= offsetX;
	position.y -= offsetY;

	// Scale position to match the 320x200 internal resolution
	position.x = position.x * 320 / viewportWidth;
	position.y = position.y * 200 / viewportHeight;

	return position;
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
	//_screenW = g_system->getWidth();
	//_screenH = g_system->getHeight();
	_gfx = createRenderer(_screenW, _screenH, _renderMode, ConfMan.getBool("authentic_graphics"));
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
		checkIfPlayerWasCrushed();
		drawFrame();

		if (_shootingFrames == 0) {
			if (_delayedShootObject) {
				bool rockTravel = isSpectrum() && isCastle() && getGameBit(k8bitGameBitTravelRock);
				// If rock travel is enabled for other platforms than ZX and CPC,
				// then this variable should be false since the game scripts will take care
				executeObjectConditions(_delayedShootObject, true, rockTravel, false);
				executeLocalGlobalConditions(true, false, false); // Only execute "on shot" room/global conditions
				_delayedShootObject = nullptr;
			}
		}

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
	if (_gameStateControl == kFreescapeGameStateEnd) {
		if (_endGameDelayTicks > 0) {
			_endGameDelayTicks--;
			return;
		}
	}

	_shootingFrames = 0;
	_delayedShootObject = nullptr;
	if (_gameStateControl == kFreescapeGameStateEnd && !isPlayingSound() && !_endGamePlayerEndArea) {
		_endGamePlayerEndArea = true;
		gotoArea(_endArea, _endEntrance);
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
		playSound(_soundIndexNoShield, true);

		if (!_noShieldMessage.empty())
			insertTemporaryMessage(_noShieldMessage, _countdown - 2);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_gameStateVars[k8bitVariableEnergy] == 0) {
		playSound(_soundIndexNoEnergy, true);

		if (!_noEnergyMessage.empty())
			insertTemporaryMessage(_noEnergyMessage, _countdown - 2);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_hasFallen) {
		_hasFallen = false;
		playSound(_soundIndexFallen, false);

		if (!_fallenMessage.empty())
			insertTemporaryMessage(_fallenMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_countdown <= 0) {
		playSound(_soundIndexTimeout, false);

		if (!_timeoutMessage.empty())
			insertTemporaryMessage(_timeoutMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	} else if (_playerWasCrushed) {
		playSound(_soundIndexCrushed, true);

		_playerWasCrushed = false;
		if (!_crushedMessage.empty())
			insertTemporaryMessage(_crushedMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
		// If the player is crushed, there are a few skipped frames
		// so no need to wait for the end of the game
		_endGameDelayTicks = 0;
	} else if (_forceEndGame) {
		playSound(_soundIndexForceEndGame, true);

		_forceEndGame = false;
		if (!_forceEndGameMessage.empty())
			insertTemporaryMessage(_forceEndGameMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	}
	return false;
}

void FreescapeEngine::checkIfPlayerWasCrushed() {
	Math::AABB boundingBox = createPlayerAABB(_position, _playerHeight);
	if (!_playerWasCrushed && _currentArea->checkIfPlayerWasCrushed(boundingBox)) {
		_avoidRenderingFrames = 60 * 3;
		_playerWasCrushed = true;
	}
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
	_endGameDelayTicks = int(2 * 60); // 2.5 seconds at 60 frames per second

	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap)
		it._value->resetArea();

	_gameStateBits = 0;

	_flyMode = false;
	_noClipMode = false;
	_playerWasCrushed = false;
	_shootingFrames = 0;
	_delayedShootObject = nullptr;
	_underFireFrames = 0;
	_avoidRenderingFrames = 0;
	_yaw = 0;
	_pitch = 0;
	_endGameKeyPressed = false;
	_endGamePlayerEndArea = false;

	_demoIndex = 0;
	_demoEvents.clear();

	removeTimers();
	startCountdown(_initialCountdown - 1);
	clearTemporalMessages();
	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	_lastMinute = minutes;
	_exploredAreas.clear();
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
	bool useTable = _demoMode;
	_cameraFront = directionToVector(_pitch, _yaw, useTable);
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
	_playerStepIndex = stream->readUint32LE();
	_countdown = stream->readUint32LE();
	_ticks = 0;
	if (!_currentArea || _currentArea->getAreaID() != areaID)
		gotoArea(areaID, -1); // Do not change position nor rotation

	_playerHeight = 32 * (_playerHeightNumber + 1) - 16 / float(_currentArea->_scale);
	return loadGameStreamExtended(stream);
}

Common::Error FreescapeEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (isAutosave)
		return Common::kNoError;

	assert(_currentArea);
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
	stream->writeUint32LE(_playerStepIndex);
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
	surface->convertToInPlace(_gfx->_currentPixelFormat, decoder.getPalette(), decoder.getPaletteColorCount());
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
	drawFrame();
	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}
	_savedScreen = _gfx->getScreenshot();

	Engine::pauseEngineIntern(pause);

	// TODO: Handle the viewport here
	if (_frameLimiter)
		_frameLimiter->pause(pause);

	// Unlock the mouse so that the cursor is usable when the GMM opens
	if (!_shootMode) {
		_system->lockMouse(!pause);
	}

	// We don't know when savedScreen will be used, so we do not deallocate it here
}

} // namespace Freescape
