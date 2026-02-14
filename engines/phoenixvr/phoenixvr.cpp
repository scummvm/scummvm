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
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"
#include "graphics/framelimiter.h"
#include "graphics/managed_surface.h"
#include "image/pcx.h"
#include "phoenixvr/console.h"
#include "phoenixvr/game_state.h"
#include "phoenixvr/math.h"
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
																					 _rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0),
																					 _thumbnail(139, 103, _rgb565),
																					 _lockKey(13),
																					 _fov(kPi2),
																					 _angleX(kPi),
																					 _angleY(-kPi2),
																					 _mixer(syst->getMixer()) {
	g_engine = this;
	for (auto format : g_system->getSupportedFormats()) {
		debug("preferred format: %s", format.toString().c_str());
		_pixelFormat = format;
		break;
	}
	if (_pixelFormat.bytesPerPixel < 2)
		error("Expected at least 16 bit format");
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
Common::SeekableReadStream *PhoenixVREngine::open(const Common::String &name) {
	debug("open %s", name.c_str());
	auto packed = name.hasSuffixIgnoreCase(".lst");
	auto filename = packed ? name.substr(0, name.size() - 4) + ".pak" : name;
	auto p = _currentScriptPath.append(filename, '\\').normalize();
	Common::ScopedPtr<Common::File> s(new Common::File());
	debug("trying %s", p.toString().c_str());
	if (s->open(p)) {
		debug("opening %s: %s", name.c_str(), p.toString().c_str());
		return packed ? unpack(*s) : s.release();
	}
	p = filename;
	debug("trying %s", p.toString().c_str());
	if (s->open(p)) {
		debug("opening %s: %s", name.c_str(), p.toString().c_str());
		return packed ? unpack(*s) : s.release();
	}
	return nullptr;
}

void PhoenixVREngine::setNextScript(const Common::String &nextScript) {
	debug("setNextScript %s", nextScript.c_str());
	_contextScript = nextScript;
	if (nextScript.find('\\') == nextScript.npos) {
		// simple filename, e.g. "script.lst"
		_nextScript = nextScript;
		return;
	}

	auto nextPath = Common::Path(removeDrive(nextScript), '\\');
	_currentScriptPath = nextPath.getParent();
	debug("changed script directory to %s", _currentScriptPath.toString().c_str());
	_nextScript = nextPath.getLastComponent().toString();
}

void PhoenixVREngine::loadNextScript() {
	debug("loading script from %s", _nextScript.c_str());
	auto nextScript = Common::move(_nextScript);
	_nextScript.clear();

	Common::ScopedPtr<Common::SeekableReadStream> s(open(nextScript));
	if (!s)
		error("can't open script file %s", nextScript.c_str());

	_script.reset(new Script(*s));
	for (auto &var : _script->getVarNames())
		declareVariable(var);

	int numWarps = _script->numWarps();
	_cursors.clear();
	_cursors.resize(numWarps);
	for (int i = 0; i != numWarps; ++i) {
		auto warp = _script->getWarp(i);
		_cursors[i].resize(warp->tests.size());
	}
	_warpIdx = 0;
}

void PhoenixVREngine::end() {
	debug("end");
	if (_nextScript.empty() && _nextWarp < 0) {
		debug("quit game");
		quitGame();
	}
}

void PhoenixVREngine::until(const Common::String &var, int value) {
	debug("until %s %d", var.c_str(), value);
	Graphics::FrameLimiter limiter(g_system, kFPSLimit);
	unsigned frameDuration = 0;
	while (!shouldQuit() && getVariable(var) != value) {
		Common::Event event;
		renderVR(frameDuration / 1000.0f);
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			default:
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		_screen->update();
		frameDuration = limiter.startFrame();
	}
}

void PhoenixVREngine::wait(float seconds) {
	debug("wait %gs", seconds);
	auto begin = g_system->getMillis();
	unsigned millis = seconds * 1000;
	Graphics::FrameLimiter limiter(g_system, kFPSLimit);
	bool waiting = true;
	unsigned frameDuration = 0;
	while (!shouldQuit() && waiting && g_system->getMillis() - begin < millis) {
		Common::Event event;
		renderVR(frameDuration / 1000.0f);
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
		frameDuration = limiter.startFrame();
	}
}

void PhoenixVREngine::goToWarp(const Common::String &warp, bool savePrev) {
	debug("gotowarp %s, save prev: %d", warp.c_str(), savePrev);
	if (warp != "N3M09L03W515E1.vr") // typo in Script4.lst
		_nextWarp = _script->getWarp(warp);
	else
		_nextWarp = _script->getWarp("N3M09L03W51E1.vr");
	_hoverIndex = -1;
	if (savePrev) {
		assert(_warpIdx >= 0);
		_prevWarp = _warpIdx;
		// saving thumbnail
		Common::ScopedPtr<Graphics::ManagedSurface> screenshot(_screen->scale(_thumbnail.w, _thumbnail.h, true));
		screenshot->convertToInPlace(_rgb565);
		_thumbnail.simpleBlitFrom(*screenshot, Graphics::FLIP_V);
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
		return nullptr;
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
	Common::ScopedPtr<Common::SeekableReadStream> stream(open(sound));
	if (!stream) {
		warning("can't load sound %s", sound.c_str());
		return;
	}

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &h, Audio::makeWAVStream(stream.release(), DisposeAfterUse::YES), -1, volume);
	if (loops < 0)
		_mixer->loopChannel(h);
	_sounds[sound] = Sound{h, spatial, angle, volume, loops};
}

void PhoenixVREngine::stopSound(const Common::String &sound) {
	debug("stop sound %s", sound.c_str());
	if (sound == _currentMusic)
		_currentMusic.clear();
	auto it = _sounds.find(sound);
	if (it != _sounds.end()) {
		_mixer->stopHandle(it->_value.handle);
		_sounds.erase(it);
	}
}

void PhoenixVREngine::playMovie(const Common::String &movie) {
	debug("playMovie %s", movie.c_str());
	Video::FourXMDecoder dec;

	auto *stream = open(movie);
	if (!stream) {
		warning("can't load movie %s", movie.c_str());
		return;
	}
	if (dec.loadStream(stream)) {
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
				if (s)
					_screen->simpleBlitFrom(*s);
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
void PhoenixVREngine::playAnimation(const Common::String &name, const Common::String &var, int varValue, float speed) {
	_vr.playAnimation(name, var, varValue, speed);
}

void PhoenixVREngine::resetLockKey() {
	debug("resetlockkey");
	_prevWarp = -1; // original game does only this o_O
}

void PhoenixVREngine::lockKey(int idx, const Common::String &warp) {
	_lockKey[idx] = warp;
}

Graphics::Surface *PhoenixVREngine::loadSurface(const Common::String &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(open(path));
	if (!stream) {
		warning("can't find image %s", path.c_str());
		return nullptr;
	}
	if (path.hasSuffix(".pcx")) {
		Image::PCXDecoder pcx;
		if (pcx.loadStream(*stream)) {
			auto *s = pcx.getSurface()->convertTo(Graphics::BlendBlit::getSupportedPixelFormat(), pcx.hasPalette() ? pcx.getPalette().data() : nullptr);
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

void PhoenixVREngine::scheduleTest(int idx) {
	debug("schedule test %d for execution", idx);
	_nextTest = idx;
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
				scheduleTest(99);
			}
		}
	}
}

void PhoenixVREngine::renderVR(float dt) {
	_vr.render(_screen, _angleX.angle(), _angleY.angle(), _fov, dt, _showRegions ? _regSet.get() : nullptr);
	if (_text) {
		int16 x = _textRect.left + (_textRect.width() - _text->w) / 2;
		int16 y = _textRect.top + (_textRect.height() - _text->h) / 2;
		_screen->blitFrom(*_text, {x, y});
	}
}

void PhoenixVREngine::saveVariables() {
	debug("SaveVariable() - saving variable state");
	_variableSnapshot.resize(_variableOrder.size());
	for (uint i = 0, n = _variableOrder.size(); i != n; ++i) {
		_variableSnapshot[i] = _variables.getVal(_variableOrder[i]);
	}
}

void PhoenixVREngine::loadVariables() {
	debug("LoadVariable() - loading variable state");
	if (_variableSnapshot.empty()) {
		debug("skipping, no snapshot");
		return;
	}
	assert(_variableSnapshot.size() == _variableOrder.size());
	for (uint i = 0, n = _variableOrder.size(); i != n; ++i) {
		_variables.setVal(_variableOrder[i], _variableSnapshot[i]);
	}
	_variableSnapshot.clear();
}

void PhoenixVREngine::rollover(int textId, RolloverType type) {
	Common::Rect dstRect;
	int size = 12;
	bool bold = false;
	uint16 color = 0xFFFF;

	if (getGameId() == "lochness") {
		size = 12;
		bold = false;
		switch (type) {
		case RolloverType::Default: // no default in loch ness
		case RolloverType::Malette:
			dstRect = Common::Rect{20, 178, 230, 198};
			color = 0xD698;
			break;
		case RolloverType::Secretaire:
			dstRect = Common::Rect{60, 448, 270, 468};
			color = 0xFFFF;
			break;
		}
	} else {
		// using necrono
		bold = true;
		switch (type) {
		case RolloverType::Default:
			dstRect = Common::Rect{57, 427, 409, 480};
			size = 14;
			color = 0;
			break;
		case RolloverType::Malette:
			dstRect = Common::Rect{251, 346, 522, 394};
			size = 18;
			color = 0xD698;
			break;
		case RolloverType::Secretaire:
			dstRect = Common::Rect{216, 367, 536, 430};
			size = 12;
			color = 0xFFFF;
			break;
		}
	}

	const Graphics::Font *font = nullptr;
#ifdef USE_FREETYPE2
	if (size < 14)
		font = _font12.get();
	else if (size < 18)
		font = _font14.get();
	else
		font = _font18.get();
#else
	font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
#endif

	if (!font)
		return;

	if (!_textes.contains(textId)) {
		debug("rollover reset");
		_text.reset();
		return;
	}
	auto &text = _textes.getVal(textId);
	debug("rollover %s, %s font size: %d, bold: %d, color: %02x", dstRect.toString().c_str(), text.c_str(), size, bold, color);

	Common::Array<Common::String> lines;
	font->wordWrapText(text, dstRect.width(), lines, Graphics::kWordWrapDefault);

	auto fontH = font->getFontHeight();
	int textW = 0;
	Common::Array<int> widths(lines.size());
	for (uint i = 0, n = lines.size(); i != n; ++i) {
		auto w = font->getStringWidth(lines[i]);
		widths[i] = w;
		textW = MAX(textW, w);
	}

	auto numLines = static_cast<int>(lines.size());
	auto textH = fontH * numLines;
	debug("text %dx%d", textW, textH);
	_text.reset(new Graphics::ManagedSurface(textW, textH, Graphics::BlendBlit::getSupportedPixelFormat()));
	_text->clear();
	byte r, g, b;
	_rgb565.colorToRGB(color, r, g, b);
	auto textColor = _text->format.RGBToColor(r, g, b);
	for (int i = 0; i != numLines; ++i) {
		int dw = (textW - widths[i]) / 2;
		font->drawAlphaString(_text.get(), lines[i], dw, i * fontH, textW, textColor, Graphics::kTextAlignLeft);
	}
	_textRect = dstRect;
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
	} else
		_mouseRel = {};

	Common::Array<Common::String> finishedSounds;
	for (auto &kv : _sounds) {
		auto &sound = kv._value;
		if (!_mixer->isSoundHandleActive(sound.handle)) {
			_mixer->stopHandle(sound.handle);
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
		_text.reset();
		_warpIdx = _nextWarp;
		_warp = _script->getWarp(_nextWarp);
		debug("warp %d -> %s %s", _nextWarp, _warp->vrFile.c_str(), _warp->testFile.c_str());
		_nextWarp = -1;

		{
			Common::ScopedPtr<Common::SeekableReadStream> stream(open(_warp->vrFile));
			if (stream) {
				_vr = VR::loadStatic(_pixelFormat, *stream);
				if (_vr.isVR()) {
					_mousePos = _screenCenter;
					_mouseRel = {};
				}
				_system->lockMouse(_vr.isVR());
			} else
				debug("can't find vr file %s", _warp->vrFile.c_str());
		}

		{
			Common::ScopedPtr<Common::SeekableReadStream> stream(open(_warp->testFile));
			if (stream)
				_regSet.reset(new RegionSet(*stream));
			else
				debug("no region %s", _warp->testFile.c_str());
		}

		Script::ExecutionContext ctx;
		debug("execute warp script %s", _warp->vrFile.c_str());
		auto test = _warp->getDefaultTest();
		if (test)
			test->scope.exec(ctx);
		else
			warning("no default script!");
	}

	if (_nextTest >= 0) {
		auto nextTest = _nextTest;
		_nextTest = -1;
		executeTest(nextTest);
	}

	renderVR(dt);

	Graphics::Surface *cursor = nullptr;
	auto &cursors = _cursors[_warpIdx];
	bool anyMatched = false;
	for (int i = 0, n = cursors.size(); i != n; ++i) {
		auto *region = getRegion(i);
		if (!region)
			continue;

		if (_vr.isVR() ? region->contains3D(currentVRPos()) : region->contains2D(_mousePos.x, _mousePos.y)) {
			anyMatched = true;
			auto test = _warp->getTest(i);
			if (test && test->hover == 1 && _hoverIndex < 0) {
				debug("executing hover test %d", i);
				_hoverIndex = i;
				executeTest(i);
			}

			auto &name = cursors[i];
			if (!cursor) {
				cursor = loadCursor(name);
			}
		} else if (i == _hoverIndex) {
			debug("leaving hover region");
			auto leave = _warp->getTest(i + 1);
			if (leave && leave->hover == 2) {
				executeTest(i + 1);
			}
			_hoverIndex = -1;
		}
	}
	if (!cursor)
		cursor = loadCursor(anyMatched ? _defaultCursor[1] : _defaultCursor[0]);
	if (cursor) {
		paint(*cursor, _mousePos - Common::Point(cursor->w / 2, cursor->h / 2));
	}
}

Common::Error PhoenixVREngine::run() {
	initGraphics(640, 480, &_pixelFormat);
#ifdef USE_FREETYPE2
	static const Common::String family("NotoSerif-Bold.ttf");
	_font12.reset(Graphics::loadTTFFontFromArchive(family, 12));
	_font14.reset(Graphics::loadTTFFontFromArchive(family, 14));
	_font18.reset(Graphics::loadTTFFontFromArchive(family, 18));
#endif

	setCursorDefault(0, "Cursor1.pcx");
	setCursorDefault(1, "Cursor2.pcx");

	_screen = new Graphics::Screen();
	_screenCenter = _screen->getBounds().center();
	{
		Common::File vars;
		if (vars.open(Common::Path("variable.txt"))) {
			while (!vars.eos()) {
				auto var = vars.readLine();
				if (var == "*")
					break;
				declareVariable(var);
				_variableOrder.push_back(Common::move(var));
			}
		} else
			debug("no variables.txt");
	}
	{
		Common::File textes;
		if (!textes.open(Common::Path("textes.txt")))
			error("can't read textes.txt");
		while (!textes.eos()) {
			auto text = textes.readLine();
			if (text.empty() || text[0] != '*')
				continue;
			uint pos = 1;
			while (pos < text.size() && Common::isSpace(text[pos]))
				++pos;
			int textId = atoi(text.c_str() + pos);
			while (pos < text.size() && Common::isDigit(text[pos]))
				++pos;
			while (pos < text.size() && Common::isSpace(text[pos]))
				++pos;
			_textes.setVal(textId, text.substr(pos));
		}
		debug("loaded %u textes", _textes.size());
	}
	setNextScript("script.lst");

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1) {
		auto r = loadGameState(saveSlot);
		if (r.getCode() != Common::ErrorCode::kNoError)
			return r;
	}

	Common::Event event;

	Graphics::FrameLimiter limiter(g_system, kFPSLimit);
	uint frameDuration = 0;
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.keycode == Common::KeyCode::KEYCODE_h)
					_showRegions = !_showRegions;
				if (_prevWarp != -1)
					break;
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
			case Common::EVENT_RBUTTONUP: {
				if (_prevWarp != -1)
					break;
				debug("right click");
				auto &rclick = _lockKey[12];
				if (!rclick.empty())
					goToWarp(rclick, true);
			} break;
			case Common::EVENT_MOUSEMOVE:
				if (!_hasFocus)
					break;
				_mousePos = event.mouse;
				_mouseRel += event.relMouse;
				break;
			case Common::EVENT_LBUTTONUP: {
				if (!_hasFocus)
					break;
				auto vrPos = currentVRPos();
				if (_vr.isVR()) {
					debug("click ax: %g, ay: %g", vrPos.x, vrPos.y);
				} else
					debug("click %s", _mousePos.toString().c_str());

				if (_warpIdx < 0)
					break;
				auto &cursors = _cursors[_warpIdx];
				for (uint i = 0, n = cursors.size(); i != n; ++i) {
					auto *region = getRegion(i);
					if (!region)
						continue;

					auto test = _warp->getTest(i);
					if (test && test->hover != 0)
						continue;

					if (_vr.isVR() ? region->contains3D(vrPos) : region->contains2D(event.mouse.x, event.mouse.y)) {
						debug("click region %u", i);
						executeTest(i);
						break;
					}
				}
			} break;
			case Common::EVENT_FOCUS_GAINED:
				_hasFocus = true;
				break;
			case Common::EVENT_FOCUS_LOST:
				_hasFocus = false;
				break;
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
		_screen->blitFrom(src, srcRect, dstRect);
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

	ms.writeSint32LE(fromAngle(_angleY.angle() + kPi2));
	ms.writeSint32LE(fromAngle(_angleX.angle()));
	ms.writeSint32LE(0);
	ms.writeSint32LE(0);
	ms.writeSint32LE(fromAngle(_angleY.rangeMax() + kPi2));
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
	writeString(_currentMusic);
	ms.writeUint32LE(_currentMusicVolume);

	struct SoundState {
		Common::String name;
		uint8 volume;
		int angle;
	};
	Common::Array<SoundState> sounds, sounds3d;
	for (auto &kv : _sounds) {
		auto &sound = kv._value;
		if (sound.loops >= 0)
			continue;
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

bool PhoenixVREngine::enterScript() {
	if (_loadedState.empty())
		return false;

	Common::MemoryReadStream ms(_loadedState.data(), _loadedState.size());

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
	if (angleYMin != -1 && angleYMax != -1)
		setYMax(toAngle(angleYMin), toAngle(angleYMax));

	_nextWarp = currentWarpIdx;

	for (auto &warpCursors : _cursors) {
		for (auto &warpCursor : warpCursors) {
			auto cursor = ms.readString(0, 257);
			debug("cursor %s", cursor.c_str());
			if (cursor.hasSuffix(".VR") || cursor.hasSuffix(".vr")) {
				debug("ignoring VR cursor, original engine saves `LOAD.VR` as a cursor name at loading screen");
				cursor.clear();
			}
			warpCursor = cursor;
		}
	}
	debug("vars at %08x", (uint32)ms.pos());
	for (auto &name : _script->getVarNames()) {
		auto value = ms.readUint32LE();
		debug("var %s: %u", name.c_str(), value);
		g_engine->setVariable(name, value);
	}
	debug("vars end at %08x", (uint32)ms.pos());
	auto currentSubroutine = ms.readSint32LE();
	_prevWarp = ms.readSint32LE();
	debug("currentSubroutine %d, prev warp %d", currentSubroutine, _prevWarp);
	for (uint i = 0; i != 12; ++i) {
		auto lockKey = ms.readString(0, 257);
		debug("lockKey %d %s", i, lockKey.c_str());
		_lockKey[i] = lockKey;
	}

	_mixer->stopAll();

	_currentMusic = ms.readString(0, 257);
	_currentMusicVolume = ms.readUint32LE();
	debug("current music %s, volume: %u", _currentMusic.c_str(), _currentMusicVolume);
	if (!_currentMusic.empty() && _currentMusicVolume > 0)
		playSound(_currentMusic, _currentMusicVolume, -1);

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
			playSound(name, vol, -1, true, static_cast<float>(angle) * kPi);
	}
	_loadedState.clear();
	return true;
}

Common::Error PhoenixVREngine::loadGameStream(Common::SeekableReadStream *slot) {
	auto state = GameState::load(*slot);

	killTimer();
	setNextScript(state.script);
	// keep it alive until loading finishes.
	auto currentScript = Common::move(_script);
	assert(!_nextScript.empty());
	loadNextScript();

	_loadedState = state.state;
	{
		auto test = _script->getWarp(0)->getDefaultTest();
		Script::ExecutionContext ctx;
		test->scope.exec(ctx);
	}

	return Common::kNoError;
}

Common::Error PhoenixVREngine::saveGameStream(Common::WriteStream *slot, bool isAutosave) {
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

	state.save(*slot);
	return Common::kNoError;
}

void PhoenixVREngine::drawSlot(int idx, int face, int x, int y) {
	Common::ScopedPtr<Common::InSaveFile> slot(_saveFileMan->openForLoading(getSaveStateName(idx)));
	if (!slot)
		return;
	auto state = GameState::load(*slot);

	y += face * 4 * 256;
	bool splitV = true;
	if (x > 256) {
		x -= 256;
		y += 256;
		splitV = false;
	}

	auto &dst = _vr.getSurface();
	auto *src = state.getThumbnail(dst.format);
	int tileY = y / 256;
	auto srcRect = src->getRect();
	short srcSplitY = MIN(y + src->h, (tileY + 1) * 256) - y;
	if (splitV)
		srcRect.bottom = srcSplitY;
	dst.copyRectToSurface(*src, x, y, srcRect);
	if (splitV) {
		srcRect.top = srcSplitY;
		srcRect.bottom = src->h;
		dst.copyRectToSurface(*src, x, (tileY + 3) * 256, srcRect);
	}
	src->free();
	delete src;
}

} // End of namespace PhoenixVR
