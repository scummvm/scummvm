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
#include "common/unzip.h"
#include "common/random.h"
#include "graphics/cursorman.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/neo.h"

namespace Freescape {

FreescapeEngine *g_freescape = NULL;

FreescapeEngine::FreescapeEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _gfx(nullptr) {
	g_freescape = this;
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
	}

	_variant = gd->flags;

	if (!Common::parseBool(ConfMan.get("prerecorded_sounds"), _usePrerecordedSounds))
		error("Failed to parse bool from prerecorded_sounds option");

	_startArea = 0;
	_startEntrance = 0;
	_currentArea = nullptr;
	_rotation = Math::Vector3d(0, 0, 0);
	_position = Math::Vector3d(0, 0, 0);
	_lastPosition = Math::Vector3d(0, 0, 0);
	_velocity = Math::Vector3d(0, 0, 0);
	_cameraFront = Math::Vector3d(0, 0, 0);
	_cameraRight = Math::Vector3d(0, 0, 0);
	_yaw = 0;
	_pitch = 0;
	_upVector = Math::Vector3d(0, 1, 0);
	_movementSpeed = 1.5f;
	_mouseSensitivity = 0.25f;
	_demoMode = false;
	_shootMode = false;
	_crossairPosition.x = _screenW / 2;
	_crossairPosition.y = _screenH / 2;
	_demoIndex = 0;
	_currentDemoInputCode = 0;
	_currentDemoInputRepetition = 0;
	_flyMode = false;
	_noClipMode = false;
	_playerHeightNumber = 1;
	_angleRotationIndex = 0;
	_border = nullptr;
	_title = nullptr;
	_titleTexture = nullptr;
	_borderTexture = nullptr;
	_uiTexture = nullptr;
	_fontLoaded = false;
	_dataBundle = nullptr;

	_lastMousePos = Common::Point(0, 0);
	_lastFrame = 0;
	_nearClipPlane = 1;
	_farClipPlane = 8192 + 1802; // Added some extra distance to avoid flickering

	// These depends on the specific game
	_playerHeight = 0;
	_playerWidth = 0;
	_playerDepth = 0;
	_colorNumber = 0;

	_fullscreenViewArea = Common::Rect(0, 0, _screenW, _screenH);
	_viewArea = _fullscreenViewArea;
	_rnd = new Common::RandomSource("freescape");
	_gfx = nullptr;
}

FreescapeEngine::~FreescapeEngine() {
	delete _rnd;

	if (_title && _title != _border) {
		_title->free();
		delete _title;
	}

	if (_border) {
		_border->free();
		delete _border;
	}

	delete _borderTexture;
	delete _uiTexture;
	delete _titleTexture;

	for (auto &it : _areaMap) {
		if (it._value->getAreaID() != 255)
			delete it._value;
	}

	if (_areaMap.contains(255))
		delete _areaMap[255];

	delete _gfx;
	delete _dataBundle;
}

void FreescapeEngine::drawBorder() {
	if (!_border)
		return;

	_gfx->setViewport(_fullscreenViewArea);
	if (!_borderTexture)
		_borderTexture = _gfx->createTexture(_border);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _borderTexture);
	_gfx->setViewport(_viewArea);
}

void FreescapeEngine::drawTitle() {
	if (!_title)
		return;

	_gfx->setViewport(_fullscreenViewArea);
	if (!_titleTexture)
		_titleTexture = _gfx->createTexture(_title);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _titleTexture);
	_gfx->setViewport(_viewArea);
}

void FreescapeEngine::loadAssets() {
	error("Function \"%s\" not implemented", __FUNCTION__);
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

void FreescapeEngine::drawUI() {
	_gfx->renderCrossair(0, _crossairPosition);
	_gfx->setViewport(_viewArea);
}

void FreescapeEngine::drawFrame() {
	_gfx->updateProjectionMatrix(60.0, _nearClipPlane, _farClipPlane);
	_gfx->positionCamera(_position, _position + _cameraFront);
	_currentArea->draw(_gfx);
	drawBorder();
	drawUI();
}

void FreescapeEngine::pressedKey(const int keycode) {}

void FreescapeEngine::generateInput() {
	Common::Event event;
	if (isDOS()) {

		if (_currentDemoInputRepetition == 0) {
			_currentDemoInputRepetition = 1;
			_currentDemoInputCode = _demoData[_demoIndex++];
			if (_currentDemoInputCode & 0x80) {
				_currentDemoInputRepetition = (_currentDemoInputCode & 0x7F) + 1;
				if (_currentDemoInputRepetition == 1)
					_currentDemoInputRepetition = 255;
				_currentDemoInputCode = _demoData[_demoIndex++];
			}
		}

		if (_currentDemoInputCode >= 0x16 && _currentDemoInputCode <= 0x1a) {
			// 0x16 -> left click / shoot
			// 0x17 -> right?
			// 0x18 -> left
			// 0x19 -> down?
			// 0x1a -> up
			// TODO: mouse events
		} else if (_currentDemoInputCode == 0x7f) {
			// NOP
		} else {
			event = Common::Event();
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = (Common::KeyCode)decodeDOSKey(_currentDemoInputCode);
			event.customType = 0xde00;
			g_system->getEventManager()->pushEvent(event);
			debugC(1, kFreescapeDebugMove, "Pushing key: %x with repetition %d", event.kbd.keycode, _currentDemoInputRepetition);
			g_system->delayMillis(100);
		}
		_currentDemoInputRepetition--;
		return;
	}

	int mouseX = _demoData[_demoIndex++] << 1;
	int mouseY = _demoData[_demoIndex++];
	debugC(1, kFreescapeDebugMove, "Mouse moved to: %d, %d", mouseX, mouseY);

	event.type = Common::EVENT_MOUSEMOVE;
	event.mouse = Common::Point(mouseX, mouseY);
	event.customType = 0xde00;
	g_system->getEventManager()->pushEvent(event);

	byte nextKeyCode = _demoData[_demoIndex++];

	if (nextKeyCode == 0x30) {
		event.type = Common::EVENT_LBUTTONDOWN; // Keep same event fields
		g_system->getEventManager()->pushEvent(event);
		nextKeyCode = _demoData[_demoIndex++];
	} else {
		while (nextKeyCode != 0) {
			event = Common::Event();
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = (Common::KeyCode)decodeAmigaAtariKey(nextKeyCode);
			debugC(1, kFreescapeDebugMove, "Pushing key: %x", event.kbd.keycode);
			event.customType = 0xde00;
			g_system->getEventManager()->pushEvent(event);
			nextKeyCode = _demoData[_demoIndex++];
		}
	}
	assert(!nextKeyCode);
	g_system->delayMillis(50);
}

void FreescapeEngine::processInput() {
	float currentFrame = g_system->getMillis();
	float deltaTime = 20.0;
	_lastFrame = currentFrame;
	Common::Event event;
	Common::Point mousePos;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (_demoMode) {
			if (event.customType != 0xde00)
				continue;
		}

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
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
			case Common::KEYCODE_r:
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
				openMainMenuDialog();
				break;
			case Common::KEYCODE_SPACE:
				_shootMode = !_shootMode;
				if (!_shootMode) {
					_crossairPosition.x = _screenW / 2;
					_crossairPosition.y = _screenH / 2;
				}
				break;
			default:
				pressedKey(event.kbd.keycode);
				break;
			}
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitGame();
			return;

		case Common::EVENT_SCREEN_CHANGED:
			_gfx->computeScreenViewport();
			break;

		case Common::EVENT_MOUSEMOVE:
			mousePos = event.mouse;

			if (_demoMode)
				g_system->warpMouse(mousePos.x, mousePos.y);

			if (_shootMode) {
				_crossairPosition = mousePos;
				break;
			}

			if (mousePos.x <= 5 || mousePos.x >= _screenW - 5) {
				g_system->warpMouse(_screenW / 2, mousePos.y);

				_lastMousePos.x = _screenW / 2;
				_lastMousePos.y = mousePos.y;
				if (mousePos.x <= 5)
					mousePos.x = _lastMousePos.x + 3;
				else
					mousePos.x = _lastMousePos.x - 3;

				mousePos.y = _lastMousePos.y;

			} else if (mousePos.y <= 5 || mousePos.y >= _screenH - 5) {
				g_system->warpMouse(mousePos.x, _screenH / 2);
				_lastMousePos.x = mousePos.x;
				_lastMousePos.y = _screenH / 2;
				if (mousePos.y <= 5)
					mousePos.y = _lastMousePos.y + 3;
				else
					mousePos.y = _lastMousePos.y - 3;

				mousePos.x = _lastMousePos.x;
			}
			rotate(_lastMousePos, mousePos);
			_lastMousePos = mousePos;
			break;

		case Common::EVENT_LBUTTONDOWN:
			shoot();
			break;

		default:
			break;
		}
	}
}

Common::Error FreescapeEngine::run() {
	// Initialize graphics
	_gfx = createRenderer(_system, _screenW, _screenH, _renderMode);
	// The following error code will force return to launcher
	// but it will not force any other GUI message to be displayed
	if (!_gfx)
		return Common::kUserCanceled;

	_gfx->init();
	_gfx->clear();

	// Load game data and init game state
	loadDataBundle();
	loadAssets();
	initGameState();
	loadColorPalette();


	_title = _gfx->convertImageFormatIfNecessary(_title);
	_border = _gfx->convertImageFormatIfNecessary(_border);

	// Simple main event loop
	int saveSlot = ConfMan.getInt("save_slot");

	if (_title) {
		if (saveSlot == -1) {
			drawTitle();
			_gfx->flipBuffer();
			g_system->updateScreen();
			g_system->delayMillis(3000);
		}
	}

	if (_border) {
		_borderTexture = nullptr;
		_border->fillRect(_viewArea, _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xA0, 0xA0, 0xA0));
	}
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else
		gotoArea(_startArea, _startEntrance);

	debugC(1, kFreescapeDebugMove, "Starting area %d", _currentArea->getAreaID());
	_system->lockMouse(true);
	bool endGame = false;
	// Draw first frame

	rotate(_lastMousePos, _lastMousePos);
	drawFrame();
	_gfx->flipBuffer();
	g_system->updateScreen();

	while (!shouldQuit() && !endGame) {
		drawFrame();
		if (_demoMode)
			generateInput();

		processInput();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(10);
		endGame = checkIfGameEnded();
	}

	return Common::kNoError;
}

bool FreescapeEngine::checkIfGameEnded() {
	return false; // TODO
}

void FreescapeEngine::initGameState() {
	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap)
		_gameStateBits[it._key] = 0;
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

void FreescapeEngine::rotate(Common::Point lastMousePos, Common::Point mousePos) {
	if (lastMousePos != Common::Point(0, 0)) {
		float xoffset = mousePos.x - lastMousePos.x;
		float yoffset = mousePos.y - lastMousePos.y;

		xoffset *= _mouseSensitivity;
		yoffset *= _mouseSensitivity;

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
	}
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

void FreescapeEngine::drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface) {
	if (!_fontLoaded)
		return;
	Common::String ustr = str;
	ustr.toUppercase();
	for (uint32 c = 0; c < ustr.size(); c++) {
		for (int j = 0; j < 6; j++) {
			for (int i = 0; i < 8; i++) {
				if (_font.get(48 * (ustr[c] - 32) + 1 + j * 8 + i))
					surface->setPixel(x + 8 - i + 8 * c, y + j, fontColor);
				else
					surface->setPixel(x + 8 - i + 8 * c, y + j, backColor);
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

	for (uint i = 0; i < _gameStateBits.size(); i++) {
		uint16 key = stream->readUint16LE();
		_gameStateBits[key] = stream->readUint32LE();
	}

	for (uint i = 0; i < _areaMap.size(); i++) {
		uint16 key = stream->readUint16LE();
		assert(_areaMap.contains(key));
		Area *area = _areaMap[key];
		area->loadObjects(stream, _areaMap[255]);
	}

	_flyMode = stream->readByte();
	if (!_currentArea || _currentArea->getAreaID() != areaID)
		gotoArea(areaID, -1); // Do not change position nor rotation
	return Common::kNoError;
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

	for (auto &it : _gameStateBits) {
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(it._value);
	}

	for (auto &it : _areaMap) {
		stream->writeUint16LE(it._key);
		it._value->saveObjects(stream);
	}

	stream->writeByte(_flyMode);
	return Common::kNoError;
}

void FreescapeEngine::loadDataBundle() {
	_dataBundle = Common::makeZipArchive(FREESCAPE_DATA_BUNDLE);
	if (!_dataBundle) {
		error("ENGINE: Couldn't load data bundle '%s'.", FREESCAPE_DATA_BUNDLE.c_str());
	}
}

byte *FreescapeEngine::getPaletteFromNeoImage(Common::SeekableReadStream *stream, int offset) {
	stream->seek(offset);
	Image::NeoDecoder decoder;
	decoder.loadStream(*stream);
	byte *palette = (byte *)malloc(16 * 3 * sizeof(byte));
	memcpy(palette, decoder.getPalette(), 16 * 3 * sizeof(byte));
	return palette;
}

Graphics::Surface *FreescapeEngine::loadAndConvertNeoImage(Common::SeekableReadStream *stream, int offset, byte *palette) {
	stream->seek(offset);
	Image::NeoDecoder decoder(palette);
	decoder.loadStream(*stream);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->copyFrom(*decoder.getSurface());
	surface->convertToInPlace(_gfx->_currentPixelFormat, decoder.getPalette());
	return surface;
}

} // namespace Freescape
