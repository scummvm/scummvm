#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/math.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/renderer.h"
#include "math/ray.h"

#include "freescape/freescape.h"
#include "freescape/gfx.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/language/token.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

FreescapeEngine *g_freescape = NULL;

FreescapeEngine::FreescapeEngine(OSystem *syst)
	: Engine(syst), _screenW(320), _screenH(200), _border(nullptr), _gfx(nullptr) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().

	g_freescape = this;
	// Do not initialize graphics here
	// Do not initialize audio devices here
	_hasReceivedTime = false;
	if (!ConfMan.hasKey("render_mode"))
		_renderMode = "ega";
	else
		_renderMode = ConfMan.get("render_mode");

	_currentArea = nullptr;
	_rotation = Math::Vector3d(0.f, 0.f, 0.f);
	_position = Math::Vector3d(0.f, 0.f, 0.f);
	_velocity = Math::Vector3d(0.f, 0.f, 0.f);
	_cameraFront = Math::Vector3d(0.f, 0.f, 0.f);
	_cameraRight = Math::Vector3d(0.f, 0.f, 0.f);
	_movementSpeed = 2.5f;
	_mouseSensitivity = 0.1f;
	_borderTexture = nullptr;

	// Here is the right place to set up the engine specific debug channels
	DebugMan.addDebugChannel(kFreescapeDebug, "example", "this is just an example for a engine specific debug channel");
	DebugMan.addDebugChannel(kFreescapeDebug2, "example2", "also an example");

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("freescape");

	debug("FreescapeEngine::FreescapeEngine");
}

FreescapeEngine::~FreescapeEngine() {
	debug("FreescapeEngine::~FreescapeEngine");

	// Dispose your resources here
	delete _rnd;
	//delete _areasByAreaID;
	delete _border;
	delete _gfx;

	// Remove all of our debug levels here
	//DebugMan.clearAllDebugChannels();
}

void FreescapeEngine::drawBorder() {
	if (!_border)
		return;

	if (!_borderTexture)
		_borderTexture = _gfx->createTexture(_border);
	const Common::Rect rect(0, 0, _screenW, _screenH);
	_gfx->drawTexturedRect2D(rect, rect, _borderTexture);
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
			warning("More than one .RUN file found, only the first one will be used!");
		}

		file = files.begin()->get()->createReadStream();
		load16bitBinary(file);
	} else if (_targetName.hasPrefix("driller") || _targetName.hasPrefix("spacestationoblivion")) {
		Common::File exe;
		bool success = false;
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

	} else if (_targetName.hasPrefix("darkside")) {
		Common::File exe;
		bool success = false;
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
			error("Invalid render mode %s for Driller", _renderMode.c_str());

	} else if (_targetName == "totaleclipse") {
		Common::File exe;
		bool success = false;
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
	   } else if (_targetName.hasPrefix("castlemaster")) {
			_renderMode = "cga";
			file = gameDir.createReadStreamForMember("castle.sna");

			if (file == nullptr)
				error("Failed to open castle.sna");
			load8bitBinary(file, 0x84da, 4);
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

void FreescapeEngine::drawFrame() {
	_gfx->updateProjectionMatrix(60.0, _nearClipPlane, _farClipPlane);
	_gfx->positionCamera(_position, _position + _cameraFront);
	_currentArea->draw(_gfx);
	_gfx->renderCrossair(0);
	drawBorder();
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
			if (event.kbd.keycode == Common::KEYCODE_w || event.kbd.keycode == Common::KEYCODE_UP)
				move(FORWARD, _scaleVector.x(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_s || event.kbd.keycode == Common::KEYCODE_DOWN)
				move(BACKWARD, _scaleVector.x(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_a || event.kbd.keycode == Common::KEYCODE_LEFT)
				move(LEFT, _scaleVector.y(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_d || event.kbd.keycode == Common::KEYCODE_RIGHT)
				move(RIGHT, _scaleVector.y(), deltaTime);
			else if (event.kbd.keycode == Common::KEYCODE_f)
				_position.setValue(1, _position.y() + 12);
			else if (event.kbd.keycode == Common::KEYCODE_v)
				_position.setValue(1, _position.y() - 12);
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitGame();
			return;
			break;

		case Common::EVENT_MOUSEMOVE:
			rotate(_lastMousePos, mousePos);
			_lastMousePos = mousePos;
			if (mousePos.x <= 5 || mousePos.x >= _screenW - 5) {
				g_system->warpMouse(_screenW/2, mousePos.y);
				_lastMousePos.x = _screenW/2;
				_lastMousePos.y = mousePos.y;
			} else if (mousePos.y <= 5 || mousePos.y >= _screenH - 5) {
				g_system->warpMouse(mousePos.x, _screenH/2);
				_lastMousePos.x = mousePos.x;
				_lastMousePos.y = _screenH/2;
			}
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
		if (gobj->conditionSource != nullptr) {
			debug("Must use shot = true when executing: %s", gobj->conditionSource->c_str());
			executeCode(gobj->condition, true, false);
		}
	}
	//debug("camera front: %f %f %f", _cameraFront.x(), _rotation.y(), _rotation.z());
}

Common::Error FreescapeEngine::run() {
	initGameState();

	// Initialize graphics
	_gfx = createRenderer(_system);
	_gfx->init();
	_gfx->clear();
	loadAssets();
	gotoArea(_startArea, _startEntrance);
	debug("FreescapeEngine::init");
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
	} else {
		_farClipPlane = 8192.f;
	}

	if (_border) {
		drawBorder();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(1000);

		_borderTexture = nullptr;
		Common::Rect viewArea(40, 16, 279, 116);
		_border->fillRect(viewArea, 0xA0A0A0FF);
	}

	debug("Starting area %d", _currentArea->getAreaID());
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
	_areaBits.resize(33);
	for (int16 i = 0; i <= 32; i++) {
		_areaBits[i] = false;
	}

	_gameState[k8bitVariableEnergy] = 100;
	_gameState[k8bitVariableShield] = 100;
}

void FreescapeEngine::rotate(Common::Point lastMousePos, Common::Point mousePos) {
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

void FreescapeEngine::move(CameraMovement direction, uint8 scale, float deltaTime) {
	Math::Vector3d previousPosition = _position;
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
	// Is threre some floor under the player?
	_position.set(_position.x(), positionY - 16, _position.z());
	bool collided = checkCollisions();

	if (!collided && _targetName == "driller") {  // Player is falling, let's try to step down
		_position.set(_position.x(), positionY - 16 - 64, _position.z());
		collided = checkCollisions();
		assert(collided);

		// restore position
		_position.set(_position.x(), positionY - 64, _position.z());
	} else {
		// restore position
		_position.set(_position.x(), positionY, _position.z());

		collided = checkCollisions();
		if (collided && previousAreaID == _currentArea->getAreaID()) {
			// We found a collisions, let's try to step up
			_position.set(_position.x(), positionY + 64, _position.z());
			collided = checkCollisions();
			if (collided)
				_position = previousPosition;
		}
	}

	debug("new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
}

bool FreescapeEngine::checkCollisions() {

	Math::Vector3d v1(_position.x() - _playerWidth / 2, _position.y() - _playerHeight     , _position.z() - _playerDepth / 2);
	Math::Vector3d v2(_position.x() + _playerWidth / 2, _position.y(), _position.z() + _playerDepth / 2);

	const Math::AABB boundingBox(v1, v2);
	Object *obj = _currentArea->checkCollisions(boundingBox);

	if (obj != nullptr) {
		debug("Collided with object id %d of size %f %f %f", obj->getObjectID(), obj->getSize().x(), obj->getSize().y(), obj->getSize().z());
		GeometricObject *gobj = (GeometricObject*) obj;
		if (gobj->conditionSource != nullptr) {
			debug("Must use collision = true when executing: %s", gobj->conditionSource->c_str());
			executeCode(gobj->condition, false, true);
		}
		return true;
	}
	return false;
}

void FreescapeEngine::gotoArea(uint16 areaID, uint16 entranceID) {
	debug("go to area: %d, entrance: %d", areaID, entranceID);

	assert(_areasByAreaID->contains(areaID));
	assert(entranceID > 0);
	_currentArea = (*_areasByAreaID)[areaID];
	_currentArea->show();

	Entrance *entrance = (Entrance*) _currentArea->entranceWithID(entranceID);
	if (!entrance)
		entrance = (Entrance*) _currentArea->firstEntrance();

	_position = entrance->getOrigin();
	_rotation = entrance->getRotation();
	int scale = _currentArea->getScale();
	assert(scale > 0);

	debug("entrace position: %f %f %f", _position.x(), _position.y(), _position.z());
	debug("player height: %d", scale * _playerHeight);
	_position.setValue(1, _position.y() + scale * _playerHeight);

	_pitch = _rotation.x() - 180.f;
	_yaw = _rotation.y() - 180.f;

	for (int16 i = 0; i <= 32; i++) {
		_areaBits[i] = false;
	}
}

bool FreescapeEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
}

Common::Error FreescapeEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncGameStream(s);
	return Common::kNoError;
}

Common::Error FreescapeEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncGameStream(s);
	return Common::kNoError;
}

void FreescapeEngine::syncGameStream(Common::Serializer &s) {
	// Use methods of Serializer to save/load fields
	int dummy = 0;
	s.syncAsUint16LE(dummy);
}

} // namespace Freescape
