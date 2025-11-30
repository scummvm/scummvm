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

#include "phoenixvr/phoenixvr.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "image/pcx.h"
#include "phoenixvr/console.h"
#include "phoenixvr/pakf.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"
#include "phoenixvr/vr.h"

namespace PhoenixVR {

PhoenixVREngine *g_engine;

PhoenixVREngine::PhoenixVREngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																					 _gameDescription(gameDesc),
																					 _randomSource("PhoenixVR"),
																					 _pixelFormat(Graphics::BlendBlit::getSupportedPixelFormat()),
																					 _mixer(syst->getMixer()) {
	g_engine = this;
	auto path = Common::FSNode(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(path, "NecroES/Data", 1, 1, true);
	SearchMan.addSubDirectoryMatching(path, "NecroES/Data2", 2, 1, true);
}

PhoenixVREngine::~PhoenixVREngine() {
	delete _screen;
}

uint32 PhoenixVREngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PhoenixVREngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::String PhoenixVREngine::resolvePath(const Common::String &path) {
	auto resolved = removeDrive(path);
	resolved.replace('\\', '/');
	return resolved;
}

Common::String PhoenixVREngine::removeDrive(const Common::String &path) {
	if (path.size() < 2 || path[1] != ':')
		return path;
	else
		return path.substr(2);
}

void PhoenixVREngine::setNextScript(const Common::String &path) {
	_nextScript = resolvePath(path);
	debug("setNextScript %s", _nextScript.c_str());
}

void PhoenixVREngine::end() {
	debug("end");
	if (_nextScript.empty())
		quitGame();
}

void PhoenixVREngine::goToWarp(const Common::String &warp) {
	debug("gotowarp %s", warp.c_str());
	_nextWarp = warp;
}

Script::ConstWarpPtr PhoenixVREngine::getWarp(const Common::String &name) {
	return _script->getWarp(name);
}

Region PhoenixVREngine::getRegion(int idx) const {
	if (!_regSet)
		error("no region set");
	return _regSet->getRegion(idx);
}

void PhoenixVREngine::setCursorDefault(int idx, const Common::String &path) {
	debug("setCursorDefault %d: %s", idx, path.c_str());
	if (idx == 0) {
		_defaultCursor.free();
		_defaultCursor.surface = loadSurface(path);
	}
}

void PhoenixVREngine::setCursor(const Common::String &path, const Common::String &wname, int idx) {
	auto reg = g_engine->getRegion(idx);
	auto &cursor = _cursors[idx];
	auto rect = reg.toRect();
	debug("cursor region %s:%d: %s, %s", wname.c_str(), idx, rect.toString().c_str(), path.c_str());
	if (!_warp || !_warp->vrFile.equalsIgnoreCase(wname)) {
		warning("setting cursor for different warp, active: %s, required: %s", _warp ? _warp->vrFile.c_str() : "null", wname.c_str());
		return;
	}
	cursor.free();
	cursor.surface = loadSurface(path);
	cursor.rect = rect;
}

void PhoenixVREngine::hideCursor(const Common::String &warp, int idx) {
	debug("hide cursor %s:%d", warp.c_str(), idx);
	_cursors[idx].free();
}

void PhoenixVREngine::declareVariable(const Common::String &name) {
	_variables.setVal(name, 0);
}

void PhoenixVREngine::setVariable(const Common::String &name, int value) {
	debug("set %s %d", name.c_str(), value);
	_variables.setVal(name, value);
}

int PhoenixVREngine::getVariable(const Common::String &name) const {
	return _variables.getVal(name);
}

void PhoenixVREngine::playSound(const Common::String &sound, uint8 volume, int loops) {
	debug("play sound %s %d %d", sound.c_str(), volume, loops);
	Audio::SoundHandle h;
	Common::ScopedPtr<Common::File> f(new Common::File());
	if (!f->open(Common::Path(sound))) {
		warning("sound %s couldn't be found", sound.c_str());
		return;
	}

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &h, Audio::makeWAVStream(f.release(), DisposeAfterUse::YES), -1, volume);
	if (loops < 0)
		_mixer->loopChannel(h);
	_sounds[sound] = h;
}

void PhoenixVREngine::stopSound(const Common::String &sound) {
	debug("stop sound %s", sound.c_str());
	auto it = _sounds.find(sound);
	if (it != _sounds.end()) {
		_mixer->stopHandle(it->_value);
		_sounds.erase(it);
	}
}

Graphics::Surface *PhoenixVREngine::loadSurface(const Common::String &path) {
	Common::File file;
	if (!file.open(Common::Path(path))) {
		warning("can't find %s", path.c_str());
		return nullptr;
	}
	if (path.hasSuffix(".pcx")) {
		Image::PCXDecoder pcx;
		if (pcx.loadStream(file)) {
			auto *s = pcx.getSurface()->convertTo(_pixelFormat, pcx.hasPalette() ? pcx.getPalette().data() : nullptr);
			if (s) {
				byte r = 0, g = 0, b = 0;
				if (pcx.hasPalette())
					pcx.getPalette().get(0, r, g, b);
				s->applyColorKey(r, g, b);
			}
			return s;
		}
		warning("pcx decode failed on %s", path.c_str());
		return nullptr;
	}
	warning("can't find decoder for %s", path.c_str());
	return nullptr;
}

void PhoenixVREngine::Cursor::free() {
	if (surface) {
		surface->free();
		delete surface;
		surface = nullptr;
	}
	rect.setEmpty();
}

void PhoenixVREngine::executeTest(int idx) {
	debug("execute test %d", idx);
	auto test = _warp->getTest(idx);
	if (test) {
		Script::ExecutionContext ctx;
		test->scope.exec(ctx);
	} else
		warning("invalid test id %d", idx);
}

Common::Error PhoenixVREngine::run() {
	initGraphics(640, 480, &_pixelFormat);
	_screen = new Graphics::Screen();
	_screenCenter = _screen->getBounds().center();
	{
		Common::File vars;
		if (vars.open(Common::Path("variable.txt"))) {
			while (!vars.eos()) {
				auto var = vars.readLine();
				declareVariable(var);
			}
		}
	}
	setNextScript("script.lst");

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Common::Event event;

	Graphics::FrameLimiter limiter(g_system, 60);
	uint frameDuration = 0;
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.ascii == ' ')
					goToWarp("N1M01L03W02E0.vr");
				break;
			case Common::EVENT_MOUSEMOVE:
				_mousePos = event.mouse;
				break;
			case Common::EVENT_LBUTTONUP:
				debug("click %s", _mousePos.toString().c_str());
				for (uint i = 0, n = _cursors.size(); i != n; ++i) {
					auto &rect = _cursors[i].rect;
					if (rect.contains(event.mouse.x, event.mouse.y)) {
						debug("click region %u", i);
						executeTest(i);
					}
				}
				break;
			default:
				break;
			}
		}
		if (_vr.isVR()) {
			auto da = _mousePos - _screenCenter;
			_system->warpMouse(_screenCenter.x, _screenCenter.y);
			_mousePos = _screenCenter;
			static const float kSpeedX = 0.2f;
			static const float kSpeedY = 0.2f;
			static const float PIx2 = M_PI * 2;
			const auto dt = float(frameDuration) / 1000.0f;
			_angleX += float(da.x) * kSpeedX * dt;
			_angleX = fmodf(_angleX, PIx2);
			if (_angleX < 0)
				_angleX += PIx2;
			_angleY += float(da.y) * kSpeedY * dt;
			_angleY = fmodf(_angleY, M_PI * 2);
			if (_angleY < 0)
				_angleY += PIx2;
		}
		if (!_nextScript.empty()) {
			debug("loading script from %s", _nextScript.c_str());
			auto nextScript = Common::move(_nextScript);
			_nextScript.clear();

			nextScript = removeDrive(nextScript);

			Common::File file;
			Common::Path nextPath(nextScript);
			if (file.open(nextPath)) {
				_script.reset(new Script(file));
			} else {
				auto pakFile = nextPath;
				pakFile = pakFile.removeExtension().append(".pak");
				file.open(pakFile);
				if (!file.isOpen())
					error("can't open script file %s", nextScript.c_str());
				Common::ScopedPtr<Common::SeekableReadStream> scriptStream(unpack(file));
				_script.reset(new Script(*scriptStream));
			}
			goToWarp(_script->getInitScript()->vrFile);
		}
		if (!_nextWarp.empty()) {
			_warp = _script->getWarp(_nextWarp);
			_nextWarp.clear();

			debug("warp %s %s", _warp->vrFile.c_str(), _warp->testFile.c_str());

			Common::File vr;
			if (vr.open(Common::Path(_warp->vrFile))) {
				_vr = VR::loadStatic(_pixelFormat, vr);
				if (_vr.isVR()) {
					_mousePos = _screenCenter;
					_system->warpMouse(_screenCenter.x, _screenCenter.y);
				}
			}

			_regSet.reset(new RegionSet(_warp->testFile));

			for (auto &c : _cursors)
				c.free();

			_cursors.resize(_regSet->size());
			for (uint i = 0; i != _regSet->size(); ++i) {
				_cursors[i].rect = _regSet->getRegion(i).toRect();
			}

			Script::ExecutionContext ctx;
			debug("execute warp script %s", _warp->vrFile.c_str());
			auto &test = _warp->getDefaultTest();
			test->scope.exec(ctx);
		}

		_vr.render(_screen, _angleX, _angleY);

		Graphics::Surface *cursor = nullptr;
		{
			float x, y;
			if (_vr.isVR()) {
				x = _angleX / M_PI_4;
				y = (2 * M_PI - _angleY) / M_PI_4;
				debug("vr %g %g", x, y);
			} else {
				x = _mousePos.x;
				y = _mousePos.y;
			}
			for (auto &c : _cursors) {
				if (c.rect.contains(x, y)) {
					cursor = c.surface;
					if (cursor)
						break;
				}
			}
		}
		if (!cursor)
			cursor = _defaultCursor.surface;
		if (cursor) {
			paint(*cursor, _mousePos - Common::Point(cursor->w / 2, cursor->h / 2));
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		_screen->update();
		frameDuration = limiter.startFrame();
	}

	return Common::kNoError;
}

void PhoenixVREngine::paint(Graphics::Surface &src, Common::Point dst) {
	Common::Rect srcRect = src.getRect();
	Common::Rect clip = _screen->getBounds();
	if (Common::Rect::getBlitRect(dst, srcRect, clip)) {
		Common::Rect dstRect(dst.x, dst.y, dst.x + srcRect.width(), dst.y + srcRect.height());
		_screen->blendBlitFrom(src, srcRect, dstRect);
	}
}

Common::Error PhoenixVREngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace PhoenixVR
