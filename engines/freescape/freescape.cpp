#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/renderer.h"

#include "freescape/freescape.h"
#include "freescape/gfx.h"

#include "freescape/loaders/16bitBinaryLoader.h"
#include "freescape/loaders/8bitBinaryLoader.h"


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

	_rotation = Vector3d(0.f, 0.f, 0.f);
	_position = Vector3d(1000.0f, 0.0f, 1000.0f);
	_velocity = Vector3d(0.0f, 0.0f, 0.0f);
	_front = Vector3d(0.0f, 0.0f, 0.0f);
	_right = Vector3d(0.0f, 0.0f, 0.0f);

	_yaw = 90.0f;
	_pitch = 0.0f;
	_movementSpeed = 4.5f;
	_mouseSensitivity = 0.05f;

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
	if (_border == nullptr)
		return;

	Texture *t = _gfx->createTexture(_border);
	const Common::Rect rect(0, 0, _screenW, _screenH);

	_gfx->drawTexturedRect2D(rect, rect, t, 1.0, false);
	_gfx->flipBuffer();
	_system->updateScreen();
	_gfx->freeTexture(t);
}

void FreescapeEngine::loadAssets() {
	Common::String renderMode = "";
	Common::SeekableReadStream *file = nullptr;
	Common::String path = ConfMan.get("path");
	Common::FSDirectory gameDir(path);

	if (_targetName.hasSuffix("3dkit")) {
		Common::ArchiveMemberList files;
        gameDir.listMatchingMembers(files, "*.RUN");

		if (files.size() == 0) {
			error("No .RUN was found in %s", path.c_str());
		} else if (files.size() > 1) {
			warning("More than one .RUN file found, only the first one will be used!");
		}

		file = files.begin()->get()->createReadStream();
		load16bitBinary(file);
	} else if (_targetName == "Driller") {
		if (!ConfMan.hasKey("render_mode"))
			renderMode = "ega";
		else
			renderMode = ConfMan.get("render_mode");

		Common::File exe;	
		debug("renderMode: %s", renderMode.c_str());
		bool success = false;
		if (renderMode == "ega") {
			file = gameDir.createReadStreamForMember("DRILLE.EXE");

			if (file == nullptr)
				error("Failed to open DRILLE.EXE");

			load8bitBinary(file, 0x9b40, 8);
		} else if (renderMode == "cga") {
			file = gameDir.createReadStreamForMember("DRILLC.EXE");

			if (file == nullptr)
				error("Failed to open DRILLC.EXE");
			load8bitBinary(file, 0x7bb0, 4);
		} else
			error("Invalid render mode %s for Driller", renderMode.c_str());

	   } else if (_targetName == "Castle") {
			error("Unsupported game");
	   } else
		error("'%s' is an invalid game", _targetName.c_str());

}

Common::Error FreescapeEngine::run() {
	// Initialize graphics:
	_gfx = Freescape::createRenderer(_system);
	_gfx->init();
	_gfx->clear();
	loadAssets();

	if (_areasByAreaID) {
		_startArea = 1; //binary.startArea;

		assert(_areasByAreaID->contains(_startArea));
		Area *area = (*_areasByAreaID)[_startArea];
		assert(area);
		//_gfx->renderPalette(area->raw_palette, binary.ncolors);
		drawBorder();
		area->draw(_gfx);
	}	
	debug("FreescapeEngine::init");
	// Simple main event loop
	Common::Event event;
	Common::Point lastMousePos(0, 0);
	float lastFrame = 0.f;

	while (!shouldQuit()) {
        float currentFrame = g_system->getMillis();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Point mousePos = g_system->getEventManager()->getMousePos();

			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_w || event.kbd.keycode == Common::KEYCODE_UP)
					move(FORWARD, deltaTime);
				else if (event.kbd.keycode == Common::KEYCODE_s || event.kbd.keycode == Common::KEYCODE_DOWN)
					move(BACKWARD, deltaTime);
				else if (event.kbd.keycode == Common::KEYCODE_a || event.kbd.keycode == Common::KEYCODE_LEFT)
					move(LEFT, deltaTime);
				else if (event.kbd.keycode == Common::KEYCODE_d || event.kbd.keycode == Common::KEYCODE_RIGHT)
					move(RIGHT, deltaTime);
				
				debug("player position: %f %f %f", _position.x(), _position.y(), _position.z());
				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				return Common::kNoError;
				break;
			
			case Common::EVENT_MOUSEMOVE:
				rotate(lastMousePos, mousePos);
				lastMousePos = mousePos;
				debug("player rotation (front): %f %f %f", _front.x(), _front.y(), _front.z());
				break;
			default:
				break;

			}
		}

		g_system->delayMillis(10);
	}

	return Common::kNoError;
}


void FreescapeEngine::rotate(Common::Point lastMousePos, Common::Point mousePos) {
	float xoffset = mousePos.x - lastMousePos.x;
	float yoffset = mousePos.y - lastMousePos.y;

	xoffset *= _mouseSensitivity;
	yoffset *= _mouseSensitivity;

	_yaw += xoffset;
	_pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (_pitch > 89.0f)
		_pitch = 89.0f;
	if (_pitch < -89.0f)
		_pitch = -89.0f;

	Vector3d v;
	float x = cos(_yaw  * M_PI / 180.0) * cos(_pitch  * M_PI / 180.0);
	float y = sin(_pitch * M_PI / 180.0);
	float z = sin(_yaw * M_PI / 180.0) * cos(_pitch * M_PI / 180.0);
	v.set(x, y, z);
	v.normalize();
	_front = v;

	// _right = _front x _up;
	Vector3d up(0, 1, 0); // this should be const
	v = Math::Vector3d::crossProduct(_front, up);
	v.normalize();
	_right = v;
}

void FreescapeEngine::move(CameraMovement direction, float deltaTime) {
	float velocity = _movementSpeed * deltaTime;
	switch (direction) {
	case FORWARD:
		_position = _position + _front * velocity;
		break;
	case BACKWARD:
		_position = _position - _front * velocity;
		break;
	case RIGHT:
		_position = _position + _right * velocity;
		break;
	case LEFT:
		_position = _position - _right * velocity;
		break;
	}
	// Make sure the user stays at the ground level
	// this one-liner keeps the user at the ground level (xz plane)
	_position.set(_position.x(), 0, _position.z());
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
