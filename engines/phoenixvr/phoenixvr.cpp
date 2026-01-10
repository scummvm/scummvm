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
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/surface.h"
#include "image/pcx.h"
#include "phoenixvr/console.h"
#include "phoenixvr/pakf.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"
#include "phoenixvr/vr.h"
#include "video/4xm_decoder.h"

namespace PhoenixVR {

PhoenixVREngine *g_engine;

PhoenixVREngine::PhoenixVREngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																					 _gameDescription(gameDesc),
																					 _randomSource("PhoenixVR"),
																					 _pixelFormat(Graphics::BlendBlit::getSupportedPixelFormat()),
																					 _rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0),
																					 _thumbnail(139, 103, _rgb565),
																					 _lockKey(13),
																					 _fov(M_PI_2),
																					 _angleX(M_PI),
																					 _angleY(-M_PI_2),
																					 _mixer(syst->getMixer()) {
	g_engine = this;
}

PhoenixVREngine::~PhoenixVREngine() {
	for (auto it = _cursorCache.begin(); it != _cursorCache.end(); ++it) {
		auto *s = it->_value;
		s->free();
		delete s;
	}
	delete _screen;
}

uint32 PhoenixVREngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PhoenixVREngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::String PhoenixVREngine::removeDrive(const Common::String &path) {
	if (path.size() < 2 || path[1] != ':')
		return path;
	else
		return path.substr(2);
}

void PhoenixVREngine::setNextScript(const Common::String &nextScript) {
	debug("setNextScript %s", nextScript.c_str());
	_contextScript = nextScript;
	auto nextPath = Common::Path(removeDrive(nextScript), '\\');
	auto parentDir = nextPath.getParent();
	_nextScript = nextPath.getLastComponent();
	auto path = ConfMan.getPath("path");
	auto dataDir = path;
	dataDir.appendInPlace(Common::Path("/"));
	dataDir.appendInPlace(parentDir);

	SearchMan.clear();
	debug("adding %s to search man", dataDir.toString().c_str());
	SearchMan.addDirectory(dataDir, 0, 1, true);
}

void PhoenixVREngine::loadNextScript() {
	debug("loading script from %s", _nextScript.toString().c_str());
	auto nextScript = Common::move(_nextScript);
	_nextScript.clear();

	Common::File file;
	const Common::Path &nextPath(nextScript);
	if (file.open(nextPath)) {
		_script.reset(new Script(file));
	} else {
		auto pakFile = nextPath;
		pakFile = pakFile.removeExtension().append(".pak");
		file.open(pakFile);
		if (!file.isOpen())
			error("can't open script file %s", nextScript.toString().c_str());
		Common::ScopedPtr<Common::SeekableReadStream> scriptStream(unpack(file));
		_script.reset(new Script(*scriptStream));
	}
	for (auto &var : _script->getVarNames())
		declareVariable(var);

	int numWarps = _script->numWarps();
	_cursors.resize(numWarps);
	for (int i = 0; i != numWarps; ++i) {
		auto warp = _script->getWarp(i);
		_cursors[i].resize(warp->tests.size());
	}
}

void PhoenixVREngine::end() {
	debug("end");
	if (_nextScript.empty() && _nextWarp < 0 && _nextScript < 0) {
		debug("quit game");
		quitGame();
	}
}

void PhoenixVREngine::wait(float seconds) {
	debug("wait %gs", seconds);
	auto begin = g_system->getMillis();
	unsigned millis = seconds * 1000;
	Graphics::FrameLimiter limiter(g_system, kFPSLimit);
	bool waiting = true;
	while (!shouldQuit() && waiting && g_system->getMillis() - begin < millis) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.ascii == ' ') {
					waiting = false;
				}
				break;
			}

			default:
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}
}

void PhoenixVREngine::goToWarp(const Common::String &warp, bool savePrev) {
	debug("gotowarp %s", warp.c_str());
	_nextWarp = _script->getWarp(warp);
	if (savePrev) {
		assert(_warp);
		// Pretty much a hack to prevent user stuck in inventory
		if (_prevWarp < 0) {
			_prevWarp = _script->getWarp(_warp->vrFile);
			assert(_prevWarp >= 0);
			// saving thumbnail
			_thumbnail.simpleBlitFrom(*_screen, Graphics::FLIP_V);
		}
	}
}

void PhoenixVREngine::returnToWarp() {
	if (_prevWarp < 0) {
		warning("return: no previous warp");
	}
	debug("returning to previous warp: %d", _prevWarp);
	_nextWarp = _prevWarp;
	_prevWarp = -1;
}

const Region *PhoenixVREngine::getRegion(int idx) const {
	if (!_regSet)
		error("no region set");
	return (idx < static_cast<int>(_regSet->size())) ? &_regSet->getRegion(idx) : nullptr;
}

void PhoenixVREngine::setCursorDefault(int idx, const Common::String &path) {
	debug("setCursorDefault %d: %s", idx, path.c_str());
	if (idx == 0 || idx == 1) {
		_defaultCursor[idx] = path;
	} else
		warning("only 2 default cursors supported, got %d", idx);
}

void PhoenixVREngine::setCursor(const Common::String &path, const Common::String &wname, int idx) {
	debug("setCursor %s %s:%d", path.c_str(), wname.c_str(), idx);
	_cursors[_script->getWarp(wname)][idx] = path;
}

void PhoenixVREngine::hideCursor(const Common::String &warp, int idx) {
	debug("hide cursor %s:%d", warp.c_str(), idx);
	_cursors[_script->getWarp(warp)][idx].clear();
}

void PhoenixVREngine::declareVariable(const Common::String &name) {
	if (!_variables.contains(name))
		_variables.setVal(name, 0);
}

void PhoenixVREngine::setVariable(const Common::String &name, int value) {
	debug("set %s %d", name.c_str(), value);
	_variables.setVal(name, value);
}

int PhoenixVREngine::getVariable(const Common::String &name) const {
	return _variables.getVal(name);
}

void PhoenixVREngine::playSound(const Common::String &sound, uint8 volume, int loops, bool spatial, float angle) {
	debug("play sound %s %d %d 3d: %d, angle: %g", sound.c_str(), volume, loops, spatial, angle);
	Audio::SoundHandle h;
	Common::ScopedPtr<Common::File> f(new Common::File());
	if (!f->open(Common::Path(sound))) {
		warning("sound %s couldn't be found", sound.c_str());
		return;
	}

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &h, Audio::makeWAVStream(f.release(), DisposeAfterUse::YES), -1, volume);
	if (loops < 0)
		_mixer->loopChannel(h);
	_sounds[sound] = Sound{h, spatial, angle, volume};
}

void PhoenixVREngine::stopSound(const Common::String &sound) {
	debug("stop sound %s", sound.c_str());
	auto it = _sounds.find(sound);
	if (it != _sounds.end()) {
		_mixer->stopHandle(it->_value.handle);
		_sounds.erase(it);
	}
}

void PhoenixVREngine::playMovie(const Common::String &movie) {
	debug("playMovie %s", movie.c_str());
	Video::FourXMDecoder dec;

	if (dec.loadFile(Common::Path{movie})) {
		dec.start();

		bool playing = true;
		Graphics::FrameLimiter limiter(g_system, kFPSLimit);
		while (!shouldQuit() && playing && !dec.endOfVideo()) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_KEYDOWN: {
					if (event.kbd.ascii == ' ') {
						playing = false;
					}
					break;
				}

				default:
					break;
				}
			}
			if (dec.needsUpdate()) {
				auto *s = dec.decodeNextFrame();
				if (s) {
					Common::ScopedPtr<Graphics::Surface> converted;
					if (s->format != _pixelFormat) {
						converted.reset(s->convertTo(_pixelFormat));
						_screen->copyFrom(*converted);
						converted->free();
					} else
						_screen->copyFrom(*s);
				}
			}

			// Delay for a bit. All events loops should have a delay
			// to prevent the system being unduly loaded
			limiter.delayBeforeSwap();
			_screen->update();
			limiter.startFrame();
		}
	} else {
		warning("playMovie %s failed", movie.c_str());
	}
}
void PhoenixVREngine::playAnimation(const Common::String &name, const Common::String &var) {
	_vr.playAnimation(name);
}

void PhoenixVREngine::resetLockKey() {
	_prevWarp = -1; // original game does only this o_O
}

void PhoenixVREngine::lockKey(int idx, const Common::String &warp) {
	_lockKey[idx] = warp;
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

Graphics::Surface *PhoenixVREngine::loadCursor(const Common::String &path) {
	if (path.empty())
		return nullptr;
	auto it = _cursorCache.find(path);
	if (it != _cursorCache.end())
		return it->_value;
	auto s = loadSurface(path);
	if (!s)
		error("can't load cursor from %s", path.c_str());
	_cursorCache[path] = s;
	return s;
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

void PhoenixVREngine::startTimer(float seconds) {
	_timer = seconds;
	_timerFlags = 5;
}

void PhoenixVREngine::pauseTimer(bool pause, bool deactivate) {
	if (pause)
		_timerFlags |= 2;
	else
		_timerFlags &= ~2;
	if (deactivate)
		_timerFlags &= ~4;
	else
		_timerFlags |= 4;
}

void PhoenixVREngine::killTimer() {
	_timerFlags = 0;
}

void PhoenixVREngine::tickTimer(float dt) {
	if (_timerFlags) {
		if ((_timerFlags & 2) == 0) {
			if (_timer > dt) {
				_timer -= dt;
			} else {
				_timer = 0;
			}
			debug("timer tick %g", _timer);
		}
		if (_timerFlags & 4) {
			if (_timer <= 0) {
				debug("timer trigger");
				killTimer();
				executeTest(99);
			}
		}
	}
}

void PhoenixVREngine::tick(float dt) {
	tickTimer(dt);

	if (_vr.isVR() && (_mouseRel.x || _mouseRel.y)) {
		auto da = _mouseRel;
		_mouseRel = {};
		_mousePos = _screenCenter;
		static const float kSpeedX = 0.2f;
		static const float kSpeedY = 0.2f;
		_angleX.add(float(da.x) * kSpeedX * dt);
		_angleY.add(float(da.y) * kSpeedY * dt);
		debug("angle %g %g -> %s", _angleX.angle(), _angleY.angle(), currentVRPos().toString().c_str());
	}
	Common::Array<Common::String> finishedSounds;
	for (auto &kv : _sounds) {
		auto &sound = kv._value;
		if (!_mixer->isSoundHandleActive(sound.handle)) {
			finishedSounds.push_back(kv._key);
		}
		if (!sound.spatial)
			continue;

		int8 balance = 127 * sinf(sound.angle - _angleX.angle());
		_mixer->setChannelBalance(sound.handle, balance);
	}
	for (auto &sound : finishedSounds) {
		debug("sound %s stopped", sound.c_str());
		_sounds.erase(sound);
	}

	if (!_nextScript.empty()) {
		loadNextScript();
		goToWarp(_script->getInitScript()->vrFile);
	}
	if (_nextWarp >= 0) {
		_warpIdx = _nextWarp;
		_warp = _script->getWarp(_nextWarp);
		debug("warp %d -> %s %s", _nextWarp, _warp->vrFile.c_str(), _warp->testFile.c_str());
		_nextWarp = -1;

		Common::File vr;
		if (vr.open(Common::Path(_warp->vrFile))) {
			_vr = VR::loadStatic(_pixelFormat, vr);
			if (_vr.isVR()) {
				_mousePos = _screenCenter;
				_mouseRel = {};
			}
			_system->lockMouse(_vr.isVR());
		}

		_regSet.reset(new RegionSet(_warp->testFile));

		Script::ExecutionContext ctx;
		debug("execute warp script %s", _warp->vrFile.c_str());
		auto test = _warp->getDefaultTest();
		if (test)
			test->scope.exec(ctx);
		else
			warning("no default script!");
		_loading = false;
	}

	_vr.render(_screen, _angleX.angle(), _angleY.angle(), _fov);

	Graphics::Surface *cursor = nullptr;
	auto &cursors = _cursors[_warpIdx];
	for (uint i = 0; i != cursors.size(); ++i) {
		auto *region = getRegion(i);
		if (!region)
			continue;

		if (_vr.isVR() ? region->contains3D(currentVRPos()) : region->contains2D(_mousePos.x, _mousePos.y)) {
			auto &name = cursors[i];
			cursor = loadCursor(name);
			if (!cursor)
				cursor = loadCursor(_defaultCursor[1]);
			break;
		}
	}
	if (!cursor)
		cursor = loadCursor(_defaultCursor[0]);
	if (cursor) {
		paint(*cursor, _mousePos - Common::Point(cursor->w / 2, cursor->h / 2));
	}
}

Common::Error PhoenixVREngine::run() {
	initGraphics(640, 480, &_pixelFormat);
	_screen = new Graphics::Screen();
	_screenCenter = _screen->getBounds().center();
	{
		Common::File vars;
		if (!vars.open(Common::Path("variable.txt")))
			error("can't read variable.txt");

		while (!vars.eos()) {
			auto var = vars.readLine();
			if (var == "*")
				break;
			declareVariable(var);
			_variableOrder.push_back(Common::move(var));
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

	Graphics::FrameLimiter limiter(g_system, kFPSLimit);
	uint frameDuration = 0;
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				int code = -1;
				switch (event.kbd.keycode) {
				case Common::KeyCode::KEYCODE_ESCAPE:
					code = 0;
					break;
				case Common::KeyCode::KEYCODE_F1:
					code = 1;
					break;
				case Common::KeyCode::KEYCODE_F2:
					code = 2;
					break;
				case Common::KeyCode::KEYCODE_F3:
					code = 3;
					break;
				case Common::KeyCode::KEYCODE_F4:
					code = 4;
					break;
				case Common::KeyCode::KEYCODE_F5:
					code = 5;
					break;
				case Common::KeyCode::KEYCODE_F6:
					code = 6;
					break;
				case Common::KeyCode::KEYCODE_F7:
					code = 7;
					break;
				case Common::KeyCode::KEYCODE_F8:
					code = 8;
					break;
				case Common::KeyCode::KEYCODE_F9:
					code = 9;
					break;
				case Common::KeyCode::KEYCODE_F10:
					code = 10;
					break;
				case Common::KeyCode::KEYCODE_RETURN:
					code = 11;
					break;
				case Common::KeyCode::KEYCODE_TAB:
					code = 12;
					break;
				default:
					break;
				}

				if (code >= 0) {
					debug("matched code %d", static_cast<int>(event.kbd.keycode));
					if (!_lockKey[code].empty())
						goToWarp(_lockKey[code], true);
				}
			} break;
			case Common::EVENT_MOUSEMOVE:
				_mousePos = event.mouse;
				_mouseRel += event.relMouse;
				break;
			case Common::EVENT_LBUTTONUP: {
				auto vrPos = currentVRPos();
				if (_vr.isVR()) {
					debug("click ax: %g, ay: %g", vrPos.x, vrPos.y);
				} else
					debug("click %s", _mousePos.toString().c_str());

				auto &cursors = _cursors[_warpIdx];
				for (uint i = 0, n = cursors.size(); i != n; ++i) {
					auto *region = getRegion(i);
					if (!region)
						continue;

					if (_vr.isVR() ? region->contains3D(vrPos) : region->contains2D(event.mouse.x, event.mouse.y)) {
						debug("click region %u", i);
						executeTest(i);
						break;
					}
				}
			} break;
			case Common::EVENT_RBUTTONUP: {
				debug("right click");
				auto &rclick = _lockKey[12];
				if (!rclick.empty())
					goToWarp(rclick, true);
			}
			default:
				break;
			}
		}
		float dt = float(frameDuration) / 1000.0f;
		tick(dt);

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

bool PhoenixVREngine::testSaveSlot(int idx) const {
	return _saveFileMan->exists(getSaveStateName(idx));
}

void PhoenixVREngine::captureContext() {
	Common::MemoryWriteStreamDynamic ms(DisposeAfterUse::YES);

	auto writeString = [&ms](const Common::String &str) {
		assert(str.size() <= 256);
		ms.writeString(str);
		uint tail = 257 - str.size();
		while (tail--)
			ms.writeByte(0);
	};

	ms.writeSint32LE(fromAngle(_angleY.angle() + M_PI_2));
	ms.writeSint32LE(fromAngle(_angleX.angle()));
	ms.writeSint32LE(0);
	ms.writeSint32LE(0);
	ms.writeSint32LE(fromAngle(_angleY.rangeMax() + M_PI_2));
	ms.writeSint32LE(fromAngle(_angleX.rangeMin()));
	ms.writeSint32LE(fromAngle(_angleX.rangeMax()));
	ms.writeSint32LE(_warpIdx);
	ms.writeUint32LE(_warp->tests.size());
	writeString({});
	writeString({});
	for (auto &warpCursors : _cursors)
		for (auto &cursor : warpCursors)
			writeString(cursor);

	for (auto &name : _script->getVarNames()) {
		auto value = g_engine->getVariable(name);
		ms.writeUint32LE(value);
	}

	ms.writeUint32LE(0); // current subroutine
	ms.writeSint32LE(_prevWarp);

	for (uint i = 0; i != 12; ++i) {
		writeString(_lockKey[i]);
	}
	writeString({});     // music
	ms.writeUint32LE(0); // musicVolume

	struct SoundState {
		Common::String name;
		uint8 volume;
		int angle;
	};
	Common::Array<SoundState> sounds, sounds3d;
	for (auto &kv : _sounds) {
		auto &sound = kv._value;
		if (sound.spatial)
			sounds3d.push_back({kv._key, sound.volume, fromAngle(sound.angle)});
		else
			sounds.push_back({kv._key, sound.volume, 0});
	}

	// sound samples
	SoundState def{{}, 255, 0};
	for (uint i = 0; i != 8; ++i) {
		auto *soundState = i < sounds.size() ? &sounds[i] : &def;
		writeString(soundState->name);
		ms.writeUint32LE(soundState->volume);
		ms.writeUint32LE(0); // flags?
	}

	// sound samples 3D
	for (uint i = 0; i != 8; ++i) {
		auto *soundState = i < sounds3d.size() ? &sounds3d[i] : &def;
		writeString(soundState->name);
		ms.writeUint32LE(soundState->angle);
		ms.writeUint32LE(soundState->volume);
		ms.writeUint32LE(0); // flags?
	}

	auto *state = ms.getData();
	_capturedState.assign(state, state + ms.size());
	debug("captured %u bytes of state", _capturedState.size());
}

void PhoenixVREngine::loadSaveSlot(int idx) {
	Common::ScopedPtr<Common::InSaveFile> slot(_saveFileMan->openForLoading(getSaveStateName(idx)));
	if (!slot) {
		warning("loadSaveSlot: invalid save slot %d", idx);
		return;
	}
	auto state = loadGameStateObject(*slot);

	setNextScript(state.script);
	// keep it alive until loading finishes.
	auto currentScript = Common::move(_script);
	assert(!_nextScript.empty());
	loadNextScript();

	Common::MemoryReadStream ms(state.state.data(), state.state.size());

	auto angleX = ms.readSint32LE();
	auto angleY = ms.readSint32LE();
	auto soundVolumnPanY = ms.readSint32LE();
	auto soundVolumePanX = ms.readSint32LE();
	auto angleXMax = ms.readSint32LE();
	auto angleYMin = ms.readSint32LE();
	auto angleYMax = ms.readSint32LE();
	auto currentWarpIdx = ms.readSint32LE();
	auto currentWarpTests = ms.readUint32LE();
	auto printText = ms.readString(0, 257);
	auto text = ms.readString(0, 257);
	debug("angle: %d %d, sound pan: %d %d, angle X max %d, angle Y range %d %d, warp: %u, tests: %u",
		  angleX, angleY, soundVolumePanX, soundVolumnPanY,
		  angleXMax, angleYMin, angleYMax,
		  currentWarpIdx, currentWarpTests);

	setAngle(toAngle(angleX), toAngle(angleY));
	setXMax(toAngle(angleXMax));
	setYMax(toAngle(angleYMin), toAngle(angleYMax));

	_nextWarp = currentWarpIdx;

	for (uint warpIdx = 0; warpIdx != _script->numWarps(); ++warpIdx) {
		auto warp = _script->getWarp(warpIdx);
		assert(warp);
		auto &tests = warp->tests;
		auto &warpCursors = _cursors[warpIdx];
		for (uint testIdx = 0; testIdx < tests.size(); ++testIdx) {
			auto cursor = ms.readString(0, 257);
			// debug("warp %s.%d: %s", warp->vrFile.c_str(), testIdx, cursor.c_str());
			warpCursors[testIdx] = cursor;
		}
	}
	debug("vars at %08lx", ms.pos());
	for (auto &name : _script->getVarNames()) {
		auto value = ms.readUint32LE();
		debug("var %s: %u", name.c_str(), value);
		g_engine->setVariable(name, value);
	}
	debug("vars end at %08lx", ms.pos());
	auto currentSubroutine = ms.readSint32LE();
	_prevWarp = ms.readSint32LE();
	debug("currentSubroutine %d, prev warp %d", currentSubroutine, _prevWarp);
	for (uint i = 0; i != 12; ++i) {
		auto lockKey = ms.readString(0, 257);
		debug("lockKey %d %s", i, lockKey.c_str());
		_lockKey[i] = lockKey;
	}
	auto music = ms.readString(0, 257);
	auto musicVolume = ms.readUint32LE();
	debug("current music %s, volume: %u", music.c_str(), musicVolume);

	_mixer->stopAll();
	// sound samples
	for (uint i = 0; i != 8; ++i) {
		auto name = ms.readString(0, 257);
		auto vol = ms.readUint32LE();
		auto flags = ms.readUint32LE();
		debug("sound: %s vol: %u flags: %u", name.c_str(), vol, flags);
		if (!name.empty())
			playSound(name, vol, -1);
	}

	// sound samples 3D
	for (uint i = 0; i != 8; ++i) {
		auto name = ms.readString(0, 257);
		auto angle = ms.readUint32LE();
		auto vol = ms.readUint32LE();
		auto flags = ms.readUint32LE();
		debug("3d sound: %s vol: %u flags: %u angle: %u", name.c_str(), vol, flags, angle);
		if (!name.empty())
			playSound(name, vol, -1, true, float(angle * M_PI));
	}

	_loading = true;
}

void PhoenixVREngine::saveSaveSlot(int idx) {
	Common::ScopedPtr<Common::OutSaveFile> slot(_saveFileMan->openForSaving(getSaveStateName(idx), false));
	if (!slot) {
		warning("saveSaveSlot: invalid save slot %d", idx);
		return;
	}
	GameState state;
	state.script = _contextScript;
	state.game = _contextLabel;

	static const char *wday[] = {
		"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

	TimeDate td = {};
	g_system->getTimeAndDate(td);
	// Saturday 03 01 2026[\x00]23 h 17
	state.info = Common::String::format("%s %02d %02d %04d%c%02d h %02d", wday[td.tm_wday], td.tm_mday, td.tm_mon + 1, td.tm_year, 0, td.tm_hour, td.tm_min);

	state.thumbWidth = _thumbnail.w;
	state.thumbHeight = _thumbnail.h;
	auto *thumbnailPixels = static_cast<byte *>(_thumbnail.getPixels());
	auto thumbnailSize = _thumbnail.pitch * _thumbnail.h;

	Common::MemoryWriteStreamDynamic dib(DisposeAfterUse::YES);
	dib.writeUint32LE(0x28);
	dib.writeUint32LE(_thumbnail.w);
	dib.writeUint32LE(_thumbnail.h);
	dib.writeUint16LE(1); // planes
	dib.writeUint16LE(16);
	dib.writeUint32LE(3); // compression
	dib.writeUint32LE(thumbnailSize);
	dib.writeUint32LE(0);
	dib.writeUint32LE(0);
	dib.writeUint32LE(3);
	dib.writeUint32LE(0);

	// RGB masks
	dib.writeUint32LE(0xf800);
	dib.writeUint32LE(0x07e0);
	dib.writeUint32LE(0x001f);

	assert(dib.size() == 0x28 + 3 * 4);
	state.dibHeader.assign(dib.getData(), dib.getData() + dib.size());

	state.thumbnail.assign(thumbnailPixels, thumbnailPixels + thumbnailSize);
	state.state = Common::move(_capturedState);
	_capturedState.clear();

	saveGameStateObject(*slot, state);
}

void PhoenixVREngine::drawSlot(int idx, int face, int x, int y) {
	Common::ScopedPtr<Common::InSaveFile> slot(_saveFileMan->openForLoading(getSaveStateName(idx)));
	if (!slot)
		return;
	auto state = loadGameStateObject(*slot);

	Graphics::PixelFormat rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Graphics::Surface thumbnail;
	thumbnail.init(state.thumbWidth, state.thumbHeight, state.thumbnail.size() / state.thumbHeight, state.thumbnail.data(), rgb565);
	auto &dst = _vr.getSurface();
	Graphics::Surface *src = thumbnail.convertTo(dst.format);
	src->flipVertical(src->getRect());
	y += face * 4 * 256;
	if (x > 256) {
		x -= 256;
		y += 256;
	}
	// FIXME: clip vertically here.
	dst.copyRectToSurface(*src, x, y, src->getRect());
	src->free();
	delete src;
}

PhoenixVREngine::GameState PhoenixVREngine::loadGameStateObject(Common::SeekableReadStream &stream) {
	GameState state;

	auto readString = [&]() {
		auto size = stream.readUint32LE();
		return stream.readString(0, size);
	};

	state.script = readString();
	debug("save.script: %s", state.script.c_str());
	state.game = readString();
	debug("save.game: %s", state.game.c_str());
	state.info = readString();
	debug("save.datetime: %s", state.info.c_str());
	uint dibHeaderSize = stream.readUint32LE();
	stream.seek(-4, SEEK_CUR);
	state.dibHeader.resize(dibHeaderSize + 3 * 4); // rmask/gmask/bmask
	stream.read(state.dibHeader.data(), state.dibHeader.size());
	state.thumbWidth = READ_LE_UINT32(state.dibHeader.data() + 0x04);
	state.thumbHeight = READ_LE_UINT32(state.dibHeader.data() + 0x08);
	auto imageSize = READ_LE_UINT32(state.dibHeader.data() + 0x14);
	debug("save.image %dx%d, %u", state.thumbWidth, state.thumbHeight, imageSize);
	state.thumbnail.resize(imageSize);
	stream.read(state.thumbnail.data(), state.thumbnail.size());
	auto gameStateSize = stream.readUint32LE();
	debug("save.state %u bytes", gameStateSize);
	state.state.resize(gameStateSize);
	stream.read(state.state.data(), state.state.size());
	return state;
}

void PhoenixVREngine::saveGameStateObject(Common::SeekableWriteStream &stream, const GameState &state) {
	auto writeString = [&](const Common::String &str) {
		stream.writeUint32LE(str.size() + 1);
		stream.writeString(str);
		stream.writeByte(0);
	};

	writeString(state.script);
	writeString(state.game);
	writeString(state.info);

	assert(state.dibHeader.size() == 0x28 + 3 * 4);
	stream.write(state.dibHeader.data(), state.dibHeader.size());

	stream.write(state.thumbnail.data(), state.thumbnail.size());

	stream.writeUint32LE(state.state.size());
	stream.write(state.state.data(), state.state.size());
}

} // End of namespace PhoenixVR
