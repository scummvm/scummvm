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
#include "common/file.h"
#include "common/math.h"
#include "common/unzip.h"
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
		_renderMode = "ega";
	else
		_renderMode = ConfMan.get("render_mode");

	_screenW = 320;
	_screenH = 200;

	if (isAmiga()) {
		//_screenW = 640;
		//_screenH = 480;
		_renderMode = "amiga";
	} else if (isAtariST()) {
		_renderMode = "atari";
	}

	if (gd->extra)
		_variant = gd->extra;
	else
		_variant = "FullGame";

	if (!Common::parseBool(ConfMan.get("prerecorded_sounds"), _usePrerecordedSounds))
		error("Failed to parse bool from prerecorded_sounds option");

	_currentArea = nullptr;
	_rotation = Math::Vector3d(0.f, 0.f, 0.f);
	_position = Math::Vector3d(0.f, 0.f, 0.f);
	_lastPosition = Math::Vector3d(0.f, 0.f, 0.f);
	_velocity = Math::Vector3d(0.f, 0.f, 0.f);
	_cameraFront = Math::Vector3d(0.f, 0.f, 0.f);
	_cameraRight = Math::Vector3d(0.f, 0.f, 0.f);
	_movementSpeed = 1.5f;
	_mouseSensitivity = 0.25f;
	_demoMode = false;
	_flyMode = false;
	_noClipMode = false;
	_playerHeightNumber = 1;

	_border = nullptr;
	_title = nullptr;
	_titleTexture = nullptr;
	_borderTexture = nullptr;
	_uiTexture = nullptr;
	_fontLoaded = false;

	_fullscreenViewArea = Common::Rect(0, 0, _screenW, _screenH);
	_viewArea = _fullscreenViewArea;
	_rnd = new Common::RandomSource("freescape");
}

FreescapeEngine::~FreescapeEngine() {
	delete _rnd;
	delete _border;
	delete _borderTexture;
	delete _uiTexture;

	delete _gfx;
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
	Common::SeekableReadStream *file = nullptr;
	Common::String path = ConfMan.get("path");
	Common::FSDirectory gameDir(path);

	if (_targetName.hasPrefix("3dkit") || _targetName == "menace") {
		Common::ArchiveMemberList files;
        gameDir.listMatchingMembers(files, "*.RUN");

		if (files.size() == 0) {
			error("No .RUN was found in %s", path.c_str());
		} else if (files.size() > 1) {
			debugC(1, kFreescapeDebugParser, "More than one .RUN file found, only the first one will be used!");
		}

		file = files.begin()->get()->createReadStream();
		load16bitBinary(file);
	} else
		error("'%s' is an invalid game", _targetName.c_str());

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
	_gfx->renderCrossair(0);
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
	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = Common::KEYCODE_UP;
	event.customType = 0xde00;
	g_system->getEventManager()->pushEvent(event);

	event.type = Common::EVENT_KEYDOWN;
	g_system->getEventManager()->pushEvent(event);

	g_system->delayMillis(500);
}

void FreescapeEngine::processInput() {
	float currentFrame = g_system->getMillis();
	float deltaTime = 20.0;
	_lastFrame = currentFrame;
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		Common::Point mousePos = g_system->getEventManager()->getMousePos();

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (_demoMode && event.customType != 0xde00) {
				continue;
			}

			if (event.kbd.keycode == Common::KEYCODE_o || event.kbd.keycode == Common::KEYCODE_UP)
				move(FORWARD, _scaleVector.x(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_k || event.kbd.keycode == Common::KEYCODE_DOWN)
				move(BACKWARD, _scaleVector.x(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_q || event.kbd.keycode == Common::KEYCODE_LEFT)
				move(LEFT, _scaleVector.y(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_w || event.kbd.keycode == Common::KEYCODE_RIGHT)
				move(RIGHT, _scaleVector.y(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_KP5 || event.kbd.keycode == Common::KEYCODE_KP0)
				shoot();
			else if (event.kbd.keycode == Common::KEYCODE_r)
				rise();
			else if (event.kbd.keycode == Common::KEYCODE_f)
				lower();
			else if (event.kbd.keycode == Common::KEYCODE_n) {
				_noClipMode = !_noClipMode;
				_flyMode = _noClipMode;
			} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				openMainMenuDialog();
			else
				pressedKey(event.kbd.keycode);
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitGame();
			return;
			break;

		case Common::EVENT_MOUSEMOVE:
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

void FreescapeEngine::shoot() {
	playSound(1, true);
	_mixer->stopAll();
	_gfx->renderShoot(0);
	Math::Vector3d direction = directionToVector(_pitch, _yaw);
	Math::Ray ray(_position, direction);
	Object *shot = _currentArea->shootRay(ray);
	if (shot) {
		GeometricObject *gobj = (GeometricObject*) shot;
		debug("Shot object %d with flags %x", gobj->getObjectID(), gobj->getObjectFlags());

		if (gobj->conditionSource != nullptr)
			debug("Must use shot = true when executing: %s", gobj->conditionSource->c_str());

		executeObjectConditions(gobj, true, false);
	}
	executeLocalGlobalConditions(true, false); // Only execute "on shot" room/global conditions
}

Common::Error FreescapeEngine::run() {
	// Initialize graphics
	_gfx = createRenderer(_system, _screenW, _screenH);
	_gfx->_isAmiga = isAmiga();
	_gfx->_isAtariST = isAtariST();
	_gfx->init();
	_gfx->clear();

	// Load game data and init game state
	loadDataBundle();
	loadAssets();
	initGameState();
	loadColorPalette();

	// Simple main event loop
	_lastMousePos = Common::Point(0, 0);
	_lastFrame = 0.f;
	// used to create a projection matrix;
	_nearClipPlane = 1.f;

	if (_binaryBits == 16) {
		// Do not render face if color is zero
		_gfx->_keyColor = 0;
		// the 16-bit kit permits the range 0-8192 to be used along all three axes and from that comes the far plane distance of 14189.
		_farClipPlane = 14189.f;
		_startArea = 1;
	} else {
		_farClipPlane = 8192.f;
	}
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
		_border->fillRect(_viewArea, 0xA0A0A0FF);
	}
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else
		gotoArea(_startArea, _startEntrance);

	debugC(1, kFreescapeDebugMove, "Starting area %d", _currentArea->getAreaID());
	_system->lockMouse(true);
	bool endGame = false;
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

	for (AreaMap::iterator it = _areaMap.begin(); it != _areaMap.end(); ++it)
		_gameStateBits[it->_key] = 0;
}

void FreescapeEngine::rotate(Common::Point lastMousePos, Common::Point mousePos) {
	if (lastMousePos == Common::Point(0, 0))
		return;
	//debug("x: %d, y: %d", mousePos.x, mousePos.y);
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

	_cameraFront = directionToVector(_pitch, _yaw);

	// // _right = _front x _up;
	Math::Vector3d up(0, 1, 0); // this should be const
	Math::Vector3d v = Math::Vector3d::crossProduct(_cameraFront, up);
	v.normalize();
	_cameraRight = v;
}


bool FreescapeEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
}

void FreescapeEngine::drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface) {
	if (!_fontLoaded)
		return;
	Common::String ustr = str;
	ustr.toUppercase();
	for (uint32 c = 0; c < ustr.size(); c++) {
		for (int j = 0; j < 6; j++) {
			for (int i = 0; i < 8; i++) {
				if (_font.get(48*(ustr[c] - 32) + 1 + j*8 + i))
					surface->setPixel(x + 8 - i + 8*c, y + j, fontColor);
				else
					surface->setPixel(x + 8 - i + 8*c, y + j, backColor);

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
	for (int i = 0; i < int(_gameStateVars.size()); i++) {
		uint16 key = stream->readUint16LE();
		_gameStateVars[key] = stream->readUint32LE();
	}

	for (int i = 0; i < int(_gameStateBits.size()); i++) {
		uint16 key = stream->readUint16LE();
		_gameStateBits[key] = stream->readUint32LE();
	}

	for (int i = 0; i < int(_areaMap.size()); i++) {
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
	for (StateVars::iterator it = _gameStateVars.begin(); it != _gameStateVars.end(); ++it) {
		stream->writeUint16LE(it->_key);
		stream->writeUint32LE(it->_value);
	}

	for (StateBits::iterator it = _gameStateBits.begin(); it != _gameStateBits.end(); ++it) {
		stream->writeUint16LE(it->_key);
		stream->writeUint32LE(it->_value);
	}

	for (AreaMap::iterator it = _areaMap.begin(); it != _areaMap.end(); ++it) {
		stream->writeUint16LE(it->_key);
		it->_value->saveObjects(stream);
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
	byte *palette = (byte*) malloc(16 * 3 * sizeof(byte));
	memcpy(palette, decoder.getPalette(), 16 * 3 * sizeof(byte));
	return palette;
}


Graphics::Surface *FreescapeEngine::loadAndConvertNeoImage(Common::SeekableReadStream *stream, int offset, byte* palette) {
	stream->seek(offset);
	Image::NeoDecoder decoder(palette);
	decoder.loadStream(*stream);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->copyFrom(*decoder.getSurface());
	surface->convertToInPlace(_gfx->_currentPixelFormat, decoder.getPalette());
	return surface;
}


} // namespace Freescape
