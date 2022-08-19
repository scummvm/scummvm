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
#include "common/memstream.h"
#include "graphics/cursorman.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

FreescapeEngine *g_freescape = NULL;

static const entrancesTableEntry rawEntranceTable[] = {
	{183, {36, 137, 13}}, // Correct?
	{184, {36, 137, 13}}, // TODO
	{185, {36, 137, 13}}, // TODO
	{186, {36, 137, 13}}, // TODO
	{187, {36, 137, 13}}, // TODO
	{188, {36, 137, 13}}, // TODO
	{190, {36, 137, 13}}, // TODO
	{191, {36, 137, 13}}, // TODO
	{192, {36, 137, 13}}, // TODO
	{193, {36, 137, 13}}, // TODO
	{194, {36, 137, 13}}, // TODO
	{195, {36, 137, 13}}, // TODO
	{196, {36, 137, 13}}, // TODO
	{197, {203, 0, 31}},  // TODO
	{198, {36, 137, 13}}, // TODO
	{199, {36, 137, 13}}, // TODO
	{200, {36, 137, 13}}, // TODO
	{201, {36, 137, 13}}, // TODO
	{202, {360, 0, 373}}, // TODO
	{203, {207, 0, 384}},
	{204, {207, 0, 372}},
	{206, {36, 137, 13}}, // TODO
	{0, {0, 0, 0}},        // NULL
};

FreescapeEngine::FreescapeEngine(OSystem *syst)
	: Engine(syst), _screenW(320), _screenH(200), _border(nullptr), _gfx(nullptr) {
	g_freescape = this;
	if (!ConfMan.hasKey("render_mode") || ConfMan.get("render_mode").empty())
		_renderMode = "ega";
	else
		_renderMode = ConfMan.get("render_mode");

	_currentArea = nullptr;
	_rotation = Math::Vector3d(0.f, 0.f, 0.f);
	_position = Math::Vector3d(0.f, 0.f, 0.f);
	_lastPosition = Math::Vector3d(0.f, 0.f, 0.f);
	_velocity = Math::Vector3d(0.f, 0.f, 0.f);
	_cameraFront = Math::Vector3d(0.f, 0.f, 0.f);
	_cameraRight = Math::Vector3d(0.f, 0.f, 0.f);
	_movementSpeed = 1.5f;
	_mouseSensitivity = 0.25f;
	_flyMode = false;
	_playerHeightNumber = 1;
	_borderTexture = nullptr;
	_fullscreenViewArea = Common::Rect(0, 0, _screenW, _screenH);
	_viewArea = _fullscreenViewArea;

	// Total Eclipse specific
	const entrancesTableEntry *entry = rawEntranceTable;
	while (entry->id) {
		_entranceTable[entry->id] = entry;
		entry++;
	}

	_rnd = new Common::RandomSource("freescape");
}

FreescapeEngine::~FreescapeEngine() {
	delete _rnd;
	delete _border;
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
	} else if (isDriller()) {
		Common::File exe;
		if (_renderMode == "ega") {
			file = gameDir.createReadStreamForMember("DRILLE.EXE");

			if (file == nullptr)
				error("Failed to open DRILLE.EXE");

			load8bitBinary(file, 0x9b40, 16);
		} else if (_renderMode == "cga") {
			file = gameDir.createReadStreamForMember("DRILLC.EXE");

			if (file == nullptr)
				error("Failed to open DRILLC.EXE");
			load8bitBinary(file, 0x7bb0, 4);
		} else
			error("Invalid render mode %s for Driller", _renderMode.c_str());

	} else if (isDark()) {
		Common::File exe;
		if (_renderMode == "ega") {
			file = gameDir.createReadStreamForMember("DSIDEE.EXE");

			if (file == nullptr)
				error("Failed to open DSIDEE.EXE");

			load8bitBinary(file, 0xa280, 16);
		} else if (_renderMode == "cga") {
			file = gameDir.createReadStreamForMember("DSIDEC.EXE");

			if (file == nullptr)
				error("Failed to open DSIDE.EXE");
			load8bitBinary(file, 0x7bb0, 4);
		} else
			error("Invalid render mode %s for Dark Side", _renderMode.c_str());

	} else if (isEclipse()) {
		Common::File exe;
		if (_renderMode == "ega") {
			file = gameDir.createReadStreamForMember("TOTEE.EXE");

			if (file == nullptr)
				error("Failed to open TOTEE.EXE");

			load8bitBinary(file, 0x3ce0, 16);
		} else if (_renderMode == "cga") {
			file = gameDir.createReadStreamForMember("TOTEC.EXE");

			if (file == nullptr)
				error("Failed to open TOTEC.EXE");
			load8bitBinary(file, 0x7bb0, 4); // TODO
		} else
			error("Invalid render mode %s for Total Eclipse", _renderMode.c_str());
	   } else if (isCastle()) {
			_renderMode = "ega";

			file = gameDir.createReadStreamForMember("CMEDF");
			int size = file->size();
			byte *encryptedBuffer = (byte*) malloc(size);
			file->read(encryptedBuffer, size);

			int seed = 24;
			for (int i = 0; i < size; i++) {
				encryptedBuffer[i] ^= seed;
				seed = (seed + 1) & 0xff;
			}

			file = new Common::MemoryReadStream(encryptedBuffer, size);
			load8bitBinary(file, 0, 16);

			// CPC
			//file = gameDir.createReadStreamForMember("cm.bin");
			//if (file == nullptr)
			//	error("Failed to open cm.bin");
			//load8bitBinary(file, 0x791a, 16);
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

	return v;
}

void FreescapeEngine::drawUI() {
	_gfx->setViewport(_fullscreenViewArea);

	if (isDriller()) {
		int energy = _gameStateVars[k8bitVariableEnergy];
		int shield = _gameStateVars[k8bitVariableShield];
		if (_renderMode == "ega" && _border) {
			//Common::Rect black(20, 177, 87, 191);
			//_gfx->drawRect2D(black, 255, 0, 0, 0);

			if (energy >= 0) {
				Common::Rect black(20, 186, 87 - energy, 191);
				_gfx->drawRect2D(black, 255, 0, 0, 0);
				Common::Rect energyBar(87 - energy, 186, 87, 191);
				_gfx->drawRect2D(energyBar, 255, 0xfc, 0xfc, 0x54);
			}

			if (shield >= 0) {
				Common::Rect black(20, 178, 87 - shield, 183);
				_gfx->drawRect2D(black, 255, 0, 0, 0);

				Common::Rect shieldBar(87 - shield, 178, 87, 183);
				_gfx->drawRect2D(shieldBar, 255, 0xfc, 0xfc, 0x54);
			}
		}
	}
}


void FreescapeEngine::drawFrame() {
	_gfx->updateProjectionMatrix(60.0, _nearClipPlane, _farClipPlane);
	_gfx->positionCamera(_position, _position + _cameraFront);
	_currentArea->draw(_gfx);
	_gfx->renderCrossair(0);
	drawBorder();
	drawUI();
}

void FreescapeEngine::processInput() {
	float currentFrame = g_system->getMillis();
	float deltaTime = currentFrame - _lastFrame;
	_lastFrame = currentFrame;
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		Common::Point mousePos = g_system->getEventManager()->getMousePos();

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
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
			else if (event.kbd.keycode == Common::KEYCODE_n)
				gotoArea(_currentArea->getAreaID() + 1, 0);
			else if (event.kbd.keycode == Common::KEYCODE_d) {
				Common::Point gasPocket = _currentArea->gasPocketPosition;
				uint32 gasPocketRadius = _currentArea->gasPocketRadius;
				if (isDriller() && gasPocketRadius > 0 && !_currentArea->drillDeployed()) {
					_gameStateVars[32]++;
					// TODO: check if there is space for the drill
					// TODO: check if there is enough energy
					Math::Vector3d drillPosition = _position + _cameraFront * 128;
					drillPosition.setValue(1, _position.y() - _playerHeight * _currentArea->getScale());
					debugC(1, kFreescapeDebugMove, "Trying to adding drill at %f %f %f", drillPosition.x(), drillPosition.y(), drillPosition.z());
					const Math::Vector3d gasPocket3D(gasPocket.x, 1, gasPocket.y);
					_currentArea->addDrill(globalObjectsArea, drillPosition);
					float distance = (gasPocket3D - drillPosition).length();
					debugC(1, kFreescapeDebugMove, "length to gas pocket: %f with radius %d", distance, _currentArea->gasPocketRadius);
					// TODO check the result of the drilling
					// TODO: reduce energy
				}
			} else if (event.kbd.keycode == Common::KEYCODE_c) {
				// TODO: check if there is enough energy
				if (isDriller() && _currentArea->drillDeployed()) {
					_gameStateVars[32]--;
					_currentArea->removeDrill();
				}
			} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				openMainMenuDialog();
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
	_gfx->renderShoot(0);
	Math::Vector3d direction = directionToVector(_pitch, _yaw);
	Math::Ray ray(_position, direction);
	Object *shot = _currentArea->shootRay(ray);
	if (shot) {
		GeometricObject *gobj = (GeometricObject*) shot;
		debug("Shot object %d with flags %x", gobj->getObjectID(), gobj->getObjectFlags());

		if (gobj->conditionSource != nullptr)
			debug("Must use shot = true when executing: %s", gobj->conditionSource->c_str());

		executeConditions(gobj, true, false);
	}
}

Common::Error FreescapeEngine::run() {
	// Initialize graphics
	_gfx = createRenderer(_system);
	_gfx->init();
	_gfx->clear();

	// Load game data and init game state
	loadAssets();
	initGameState();
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

	if (_border) {
		if (saveSlot == -1) {
			drawBorder();
			_gfx->flipBuffer();
			g_system->updateScreen();
			if (isDriller())
				g_system->delayMillis(1000);
		}

		_borderTexture = nullptr;
		if (isDriller())
			_viewArea = Common::Rect(40, 16, 279, 116);
		else if (isEclipse())
			_viewArea = Common::Rect(40, 32, 280, 132);
		else
			error("Invalid target!");

		_border->fillRect(_viewArea, 0xA0A0A0FF);
	}
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else
		gotoArea(_startArea, _startEntrance);

	debugC(1, kFreescapeDebugMove, "Starting area %d", _currentArea->getAreaID());
	_system->lockMouse(true);
	while (!shouldQuit()) {
		drawFrame();
		processInput();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void FreescapeEngine::initGameState() {
	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (AreaMap::iterator it = _areaMap.begin(); it != _areaMap.end(); ++it)
		_gameStateBits[it->_key] = 0;

	_gameStateVars[k8bitVariableEnergy] = 43;
	_gameStateVars[k8bitVariableShield] = 48;
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

	_cameraFront = directionToVector(_pitch, _yaw);

	// // _right = _front x _up;
	Math::Vector3d up(0, 1, 0); // this should be const
	Math::Vector3d v = Math::Vector3d::crossProduct(_cameraFront, up);
	v.normalize();
	_cameraRight = v;
}

void FreescapeEngine::changePlayerHeight(int delta) {
	int scale = _currentArea->getScale();
	_position.setValue(1, _position.y() - scale * _playerHeight);
	_playerHeight = _playerHeight + delta;
	_position.setValue(1, _position.y() + scale * _playerHeight);
}

void FreescapeEngine::rise() {
	int previousAreaID = _currentArea->getAreaID();
	int scale = _currentArea->getScale();

	if (_flyMode) {
		_position.setValue(1, _position.y() + scale * 32);
	} else {
		if (_playerHeightNumber == 10) // TODO
			return;

		_playerHeightNumber++;
		changePlayerHeight(16);
	}

	bool collided = checkCollisions(true);
	if (collided) {
		if (_currentArea->getAreaID() == previousAreaID) {
			if (_flyMode)
				_position = _lastPosition;
			else {
				changePlayerHeight(-16);
			}
		}
	}

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
}

void FreescapeEngine::lower() {
	int previousAreaID = _currentArea->getAreaID();
	int scale = _currentArea->getScale();

	if (_flyMode) {
		_position.setValue(1, _position.y() - scale * 32);
		bool collided = checkCollisions(true);
		if (collided) {
			if (_currentArea->getAreaID() == previousAreaID) {
				_position = _lastPosition;
			}
		}
	} else {
		if (_playerHeightNumber == 0)
			return;

		_playerHeightNumber--;
		changePlayerHeight(-16);
	}

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
}


void FreescapeEngine::move(CameraMovement direction, uint8 scale, float deltaTime) {
	debugC(1, kFreescapeDebugMove, "old player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	int previousAreaID = _currentArea->getAreaID();

	float velocity = _movementSpeed * deltaTime;
	float positionY = _position.y();
	switch (direction) {
	case FORWARD:
		_position = _position + _cameraFront * velocity;
		break;
	case BACKWARD:
		_position = _position - _cameraFront * velocity;
		break;
	case RIGHT:
		_position = _position - _cameraRight * velocity;
		break;
	case LEFT:
		_position = _position + _cameraRight * velocity;
		break;
	}
	int areaScale = _currentArea->getScale();
	// restore y coordinate
	if (!_flyMode)
		_position.set(_position.x(), positionY, _position.z());

	bool collided = checkCollisions(false);

	if (!collided) {
		bool hasFloor = checkFloor(_position);
		if (!hasFloor  && !_flyMode) {
			int fallen;
			for (fallen = 1; fallen < 65 + 1; fallen++) {
				_position.set(_position.x(), positionY - fallen * areaScale, _position.z());
				if (tryStepDown(_position))
					break;
			}
			fallen++;
			fallen++;
			if (fallen >= 67) {
				_position = _lastPosition; //error("NASTY FALL!");
				return;
			}
			_position.set(_position.x(), positionY - fallen * areaScale, _position.z());
		}
		debugC(1, kFreescapeDebugCode, "Runing effects:");
		checkCollisions(true); // run the effects
	} else {
		debugC(1, kFreescapeDebugCode, "Runing effects:");
		checkCollisions(true); // run the effects
		if (_currentArea->getAreaID() == previousAreaID) {
			if (_flyMode)
				_position = _lastPosition;
			else {
				bool stepUp = tryStepUp(_position);
				if (stepUp) {
					debugC(1, kFreescapeDebugCode, "Runing effects:");
					checkCollisions(true); // run the effects (again)
				} else {
					_position = _lastPosition;
				}
			}
		}
	}
	areaScale = _currentArea->getScale();

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	debugC(1, kFreescapeDebugMove, "player height: %f", _position.y() - areaScale * _playerHeight);
}

bool FreescapeEngine::checkFloor(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Checking floor under the player");
	int areaScale = _currentArea->getScale();
	bool collided = checkCollisions(false);
	assert(!collided);

	_position.set(_position.x(), _position.y() - 2 * areaScale, _position.z());
	collided = checkCollisions(false);
	_position = currentPosition;
	return collided;
}

bool FreescapeEngine::tryStepUp(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Try to step up!");
	int areaScale = _currentArea->getScale();
	_position.set(_position.x(), _position.y() + 64 * areaScale, _position.z());
	bool collided = checkCollisions(false);
	if (collided) {
		_position = currentPosition;
		return false;
	} else {
		// Try to step down
		return true;
	}
}

bool FreescapeEngine::tryStepDown(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Try to step down!");
	int areaScale = _currentArea->getScale();
	_position.set(_position.x(), _position.y() - areaScale, _position.z());
	if (checkFloor(_position)) {
		return true;
	} else {
		_position = currentPosition;
		return false;
	}
}


bool FreescapeEngine::checkCollisions(bool executeCode) {
	int areaScale = _currentArea->getScale();
	Math::Vector3d v1(_position.x() - areaScale * _playerWidth / 2, _position.y() - areaScale * _playerHeight , _position.z() - areaScale * _playerDepth / 2);
	Math::Vector3d v2(_position.x() + areaScale * _playerWidth / 2, _position.y()                             , _position.z() + areaScale * _playerDepth / 2);

	const Math::AABB boundingBox(v1, v2);
	Object *obj = _currentArea->checkCollisions(boundingBox);

	if (obj != nullptr) {
		debugC(1, kFreescapeDebugMove, "Collided with object id %d of size %f %f %f", obj->getObjectID(), obj->getSize().x(), obj->getSize().y(), obj->getSize().z());
		GeometricObject *gobj = (GeometricObject*) obj;
		if (!executeCode) // Avoid executing code
			return true;

		executeConditions(gobj, false, true);
		return true;
	}
	return false;
}

void FreescapeEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	int scale = _currentArea->getScale();
	assert(scale > 0);

	Entrance *entrance = nullptr;
	if (entranceID > 0) {
		entrance = (Entrance*) _currentArea->entranceWithID(entranceID);

		if (!entrance) {
			assert(_entranceTable.contains(entranceID));
			const entrancesTableEntry *entry = _entranceTable[entranceID];
			_position = scale * Math::Vector3d(entry->position[0], entry->position[1], entry->position[2]);
		} else
			_position = entrance->getOrigin();

		if (_rotation == Math::Vector3d(0, 0, 0)) {
			_rotation = entrance->getRotation();
			_pitch = _rotation.x();
			_yaw = _rotation.y() - 260;
		}
		debugC(1, kFreescapeDebugMove, "entrace position: %f %f %f", _position.x(), _position.y(), _position.z());
		debugC(1, kFreescapeDebugMove, "player height: %d", scale * _playerHeight);
		_position.setValue(1, _position.y() + scale * _playerHeight);
	} else if (entranceID == 0) {
		Math::Vector3d diff = _lastPosition - _position;
		debug("dif: %f %f %f", diff.x(), diff.y(), diff.z());
		// diff should be used to determinate which entrance to use
		int newPos = -1;
		if (abs(diff.x()) < abs(diff.z())) {
			if (diff.z() > 0)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(2, newPos);
		} else {
			if (diff.x() > 0)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(0, newPos);
		}
		assert(newPos != -1);
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
}

bool FreescapeEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
}

void FreescapeEngine::playSound(int index) {
	_mixer->stopAll();
	debug("Playing sound %d", index);
	Audio::PCSpeaker *speaker = new Audio::PCSpeaker();
	speaker->play(Audio::PCSpeaker::kWaveFormSine, 2000, 100);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle, speaker);
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
		area->loadObjectFlags(stream);
		// Add drill, if available
		if (area->drillPosition != Math::Vector3d())
			area->addDrill(globalObjectsArea, area->drillPosition);
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
		it->_value->saveObjectFlags(stream);
	}

	stream->writeByte(_flyMode);
	return Common::kNoError;
}

} // namespace Freescape
