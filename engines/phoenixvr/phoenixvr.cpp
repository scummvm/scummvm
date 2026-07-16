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
#include "common/language.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"
#include "graphics/framelimiter.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "image/gif.h"
#include "image/pcx.h"
#include "phoenixvr/arn.h"
#include "phoenixvr/console.h"
#include "phoenixvr/game_state.h"
#include "phoenixvr/math.h"
#include "phoenixvr/pakf.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"
#include "phoenixvr/vr.h"
#include "video/4xm_decoder.h"
#include "video/smk_decoder.h"
#include "video/subtitles.h"

namespace PhoenixVR {

PhoenixVREngine *g_engine;

static Common::CodePage getTextCodePage(Common::Language language) {
	switch (language) {
	case Common::RU_RUS:
		return Common::kWindows1251;
	default:
		return Common::kWindows1252;
	}
}

static bool isAmerzoneGame(const ADGameDescription *gameDesc) {
	return !strcmp(gameDesc->gameId, "amerzone");
}

static Common::String getAmerzoneLevelLabel(const Common::String &script) {
	static const struct {
		const char *prefix;
		const char *label;
	} levels[] = {
		{"01VR_PHARE", "Le Phare"},
		{"02VR_ILE", "L'Ile"},
		{"03VR_PUEBLO", "Le Pueblo"},
		{"04VR_FLEUVE", "Le Fleuve"},
		{"05VR_VILLAGEMARAIS", "Le Village"},
		{"07VRTEMPLE_VOLCAN", "Le Temple"}};

	for (const auto &level : levels) {
		if (script.hasPrefixIgnoreCase(level.prefix))
			return level.label;
	}

	return "Amerzone";
}

static const char *mfull[] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"};

static const char *wday[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static Common::String makeSaveText(const Common::String &firstLine, const Common::String &secondLine) {
	Common::String result = firstLine;
	result += '\0';
	result += secondLine;
	return result;
}

static Common::String makeSaveText(const Common::String &firstLine, const Common::String &secondLine, const Common::String &thirdLine) {
	Common::String result = makeSaveText(firstLine, secondLine);
	result += '\0';
	result += thirdLine;
	return result;
}

static Common::String formatSaveInfo(const TimeDate &td, bool longDate, const Common::String &place = Common::String()) {
	if (longDate) {
		return makeSaveText(
			Common::String::format("%s, %s %d, %04d", wday[td.tm_wday], mfull[td.tm_mon], td.tm_mday, td.tm_year + 1900),
			Common::String::format("%02d:%02d:%02d %s", td.tm_hour, td.tm_min, td.tm_sec, td.tm_hour < 12 ? "AM" : "PM"),
			place);
	}

	return makeSaveText(
		Common::String::format("%s %02d %02d %04d", wday[td.tm_wday], td.tm_mday, td.tm_mon + 1, td.tm_year + 1900),
		Common::String::format("%02d h %02d", td.tm_hour, td.tm_min));
}

static int mapSaveSlotY(int y, bool splitV, int tileY) {
	int splitLine = (tileY + 1) * 256;
	if (splitV && y >= splitLine)
		return (tileY + 3) * 256 + y - splitLine;
	return y;
}

static void fillSaveSlotRect(Graphics::Surface &dst, const Common::Rect &rect, uint32 color, bool splitV, int tileY) {
	if (splitV) {
		int splitLine = (tileY + 1) * 256;
		int topH = CLIP<int>(splitLine - rect.top, 0, rect.height());
		if (topH > 0) {
			Common::Rect top = rect;
			top.bottom = rect.top + topH;
			dst.fillRect(top, color);
		}
		if (topH < rect.height()) {
			int bottomY = (tileY + 3) * 256 + MAX(rect.top - splitLine, 0);
			Common::Rect bottom(rect.left, bottomY, rect.right, bottomY + rect.height() - topH);
			dst.fillRect(bottom, color);
		}
	} else {
		dst.fillRect(rect, color);
	}
}

static int drawSaveTextBlock(Graphics::Surface &dst, const Graphics::Font *font, const Common::String &text,
							 int x, int y, int width, uint32 color, Graphics::TextAlign align, int lineHeight, bool splitV, int tileY,
							 bool reserveEmptyFinalLine = false) {
	bool hasText = false;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] != '\n' && text[i] != '\0') {
			hasText = true;
			break;
		}
	}
	if (!hasText)
		return y;

	uint start = 0;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] == '\n' || text[i] == '\0') {
			if (i > start) {
				Common::String line;
				for (uint j = start; j < i; ++j)
					line += text[j];
				font->drawString(&dst, line, x, mapSaveSlotY(y, splitV, tileY), width, color, align);
				y += lineHeight;
			} else if (reserveEmptyFinalLine && i == text.size() - 1) {
				y += lineHeight;
			}
			start = i + 1;
		}
	}
	if (start < text.size()) {
		Common::String line;
		for (uint j = start; j < text.size(); ++j)
			line += text[j];
		font->drawString(&dst, line, x, mapSaveSlotY(y, splitV, tileY), width, color, align);
		y += lineHeight;
	}

	return y;
}

static int saveCardTileId(int face, int x, int y) {
	return (face << 2) + ((y < 256) ? (x < 256 ? 0 : 1) : (x < 256 ? 3 : 2));
}

static void copyCubeFaceToSurface(Graphics::ManagedSurface &faceSurface, const Graphics::Surface &vrSurface, int face) {
	for (int y = 0; y < 512; ++y) {
		for (int x = 0; x < 512; ++x) {
			const int tileId = saveCardTileId(face, x, y);
			faceSurface.setPixel(x, y, vrSurface.getPixel(x & 0xff, (tileId << 8) + (y & 0xff)));
		}
	}
}

static void copySurfaceToCubeFace(Graphics::Surface &vrSurface, const Graphics::ManagedSurface &faceSurface, int face) {
	for (int y = 0; y < 512; ++y) {
		for (int x = 0; x < 512; ++x) {
			const int tileId = saveCardTileId(face, x, y);
			vrSurface.setPixel(x & 0xff, (tileId << 8) + (y & 0xff), faceSurface.getPixel(x, y));
		}
	}
}

static void projectSaveCard(Graphics::ManagedSurface &faceSurface, const Graphics::ManagedSurface &card, float angle) {
	struct Vertex {
		float x;
		float y;
		float invW;
		float uOverW;
		float vOverW;
	};

	const float srcW = static_cast<float>(card.w);
	const float srcH = static_cast<float>(card.h);
	const float distance = srcW / 8.0f + srcW * 8.0f / 6.283100128173828f;
	const float cosA = cosf(angle);
	const float sinA = sinf(angle);

	auto makeVertex = [&](float modelU, float modelV, float textureU, float textureV) {
		const float modelX = modelU - srcW / 2.0f;
		const float modelY = distance;
		const float modelZ = srcH / 2.0f - modelV + 32.0f;
		const float projectedW = (modelX * sinA - modelY * cosA) / 256.0f;
		const float invW = 1.0f / projectedW;

		Vertex vertex;
		vertex.x = (modelX * (cosA + sinA) + modelY * (sinA - cosA)) * invW;
		vertex.y = (modelX * sinA - modelY * cosA - modelZ) * invW;
		vertex.invW = invW;
		vertex.uOverW = textureU * invW;
		vertex.vOverW = textureV * invW;
		return vertex;
	};

	Vertex vertices[4] = {
		makeVertex(0.0f, 0.0f, srcW, srcH),
		makeVertex(static_cast<float>(card.w), 0.0f, 0.0f, srcH),
		makeVertex(static_cast<float>(card.w), static_cast<float>(card.h), 0.0f, 0.0f),
		makeVertex(0.0f, static_cast<float>(card.h), srcW, 0.0f)};

	auto rasterizeTriangle = [&](const Vertex &a, const Vertex &b, const Vertex &c) {
		const float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		if (ABS(area) < 0.0001f)
			return;

		int minX = CLIP<int>(static_cast<int>(floorf(MIN(a.x, MIN(b.x, c.x)))), 0, faceSurface.w - 1);
		int maxX = CLIP<int>(static_cast<int>(ceilf(MAX(a.x, MAX(b.x, c.x)))), 0, faceSurface.w - 1);
		int minY = CLIP<int>(static_cast<int>(floorf(MIN(a.y, MIN(b.y, c.y)))), 0, faceSurface.h - 1);
		int maxY = CLIP<int>(static_cast<int>(ceilf(MAX(a.y, MAX(b.y, c.y)))), 0, faceSurface.h - 1);

		for (int y = minY; y <= maxY; ++y) {
			for (int x = minX; x <= maxX; ++x) {
				const float px = static_cast<float>(x) + 0.5f;
				const float py = static_cast<float>(y) + 0.5f;
				const float w0 = ((b.x - px) * (c.y - py) - (b.y - py) * (c.x - px)) / area;
				const float w1 = ((c.x - px) * (a.y - py) - (c.y - py) * (a.x - px)) / area;
				const float w2 = 1.0f - w0 - w1;
				if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
					continue;

				const float invW = w0 * a.invW + w1 * b.invW + w2 * c.invW;
				if (ABS(invW) < 0.0001f)
					continue;
				const float u = (w0 * a.uOverW + w1 * b.uOverW + w2 * c.uOverW) / invW;
				const float v = (w0 * a.vOverW + w1 * b.vOverW + w2 * c.vOverW) / invW;
				if (u < 0.0f || u > srcW || v < 0.0f || v > srcH)
					continue;

				const int srcX = CLIP<int>(static_cast<int>(floorf(u)), 0, card.w - 1);
				const int srcY = CLIP<int>(static_cast<int>(floorf(v)), 0, card.h - 1);
				faceSurface.setPixel(x, y, card.getPixel(srcX, srcY));
			}
		}
	};

	rasterizeTriangle(vertices[0], vertices[1], vertices[2]);
	rasterizeTriangle(vertices[0], vertices[2], vertices[3]);
}

PhoenixVREngine::PhoenixVREngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																					 _frameLimiter(g_system, kFPSLimit),
																					 _gameDescription(gameDesc),
																					 _randomSource("PhoenixVR"),
																					 _rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0),
																					 _thumbnail(isAmerzoneGame(gameDesc) ? 232 : 139, isAmerzoneGame(gameDesc) ? 174 : 103, _rgb565),
																					 _lockKey(13),
																					 _fov(kPi2),
																					 _angleX(0),
																					 _angleY(-kPi2),
																					 _mixer(syst->getMixer()) {
	g_engine = this;

	if (gameIdMatches("amerzone")) {
		_levels.push_back("01VR_PHARE");
		_levels.push_back("02VR_ILE");
		_levels.push_back("03VR_PUEBLO");
		_levels.push_back("04VR_FLEUVE");
		_levels.push_back("05VR_VILLAGEMARAIS");
		_levels.push_back("07VRTEMPLE_VOLCAN");
	}
}

void PhoenixVREngine::resetState() {
	_angleX.resetRange();
	_angleX.set(0);
	_angleY.resetRange();
	_angleY.set(-kPi2);
	_imageOverlay.reset();
	_cibleActive = false;
	_cibleBounds.clear();
}

PhoenixVREngine::~PhoenixVREngine() {
	_system->lockMouse(false);
	for (auto it = _cursorCache.begin(); it != _cursorCache.end(); ++it) {
		auto *s = it->_value;
		s->free();
		delete s;
	}
	delete _screen;
}

void PhoenixVREngine::showWaves() {
	_vr.showWaves();
}

uint32 PhoenixVREngine::getFeatures() const {
	return _gameDescription->flags;
}

bool PhoenixVREngine::gameIdMatches(const char *gameId) const {
	return strcmp(_gameDescription->gameId, gameId) == 0;
}

uint PhoenixVREngine::currentAmerzoneLevel() const {
	if (!gameIdMatches("amerzone"))
		return 0;

	uint index = 0;
	for (const Common::String &level : _levels) {
		++index;
		if (_contextScript.hasPrefixIgnoreCase(level))
			return index;
	}

	error("currentAmerzoneLevel: can't find current script");
}

Common::String PhoenixVREngine::removeDrive(const Common::String &path) {
	if (path.size() < 2 || path[1] != ':')
		return path;
	else
		return path.substr(2);
}

Common::SeekableReadStream *PhoenixVREngine::tryOpen(const Common::Path &name, Common::String *origName) {
	Common::ScopedPtr<Common::File> s(new Common::File());
	if (s->open(name)) {
		auto nameStr = name.toString();
		debug("opened %s", nameStr.c_str());
		if (nameStr.hasSuffixIgnoreCase(".pak"))
			return unpack(*s, origName);
		return s.release();
	}
	auto pakName = name.toString();
	auto dotPos = pakName.rfind('.');
	if (dotPos == pakName.npos)
		return nullptr;
	pakName = pakName.substr(0, dotPos) + ".pak";
	if (s->open(Common::Path{pakName})) {
		debug("opened %s", pakName.c_str());
		return unpack(*s, origName);
	}

	return nullptr;
}

Common::SeekableReadStream *PhoenixVREngine::open(const Common::String &filename, Common::String *origName) {
	debug("open %s", filename.c_str());
	auto *stream = tryOpen(_currentScriptPath.append(filename, '\\').normalize(), origName);
	if (stream)
		return stream;

	stream = tryOpen(Common::Path{filename}, origName);
	if (stream)
		return stream;

	return nullptr;
}

bool PhoenixVREngine::setNextLevel() {
	if (_nextLevel < _levels.size()) {
		auto &level = _levels[_nextLevel++];
		debug("next level is %s", level.c_str());
		setNextScript(Common::String::format("%s\\%s.lst", level.c_str(), _gameDescription->gameId));
		_loaded = true;

		// reset flag or interface.vr will skip menu
		if (_nextLevel == 1)
			_loaded = false;
		return true;
	} else
		return false;
}

void PhoenixVREngine::setNextScript(const Common::String &nextScript) {
	debug("setNextScript %s", nextScript.c_str());
	_contextScript = nextScript;
	const Common::String scriptPath = removeDrive(nextScript);
	if (scriptPath.find('\\') == scriptPath.npos) {
		// simple filename, e.g. "script.lst"
		_nextScript = scriptPath;
		return;
	}

	auto nextPath = Common::Path(scriptPath, '\\');
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
	if (gameIdMatches("amerzone"))
		declareVariable("oeuf_pose"); // crash in chapter 7
	if (gameIdMatches("dracula1")) {
		declareVariable("P_Alliance"); // Referenced by 0M1Script.lst, declared by 0M2Script.lst
		declareVariable("reloaddone"); // Referenced by InsertCD.lst, declared by chapter scripts
	}

	int numWarps = _script->numWarps();
	_cursors.clear();
	_cursors.resize(numWarps);
	for (int i = 0; i != numWarps; ++i) {
		auto warp = _script->getWarp(i);
		_cursors[i].resize(warp->tests.size());
	}
	_warpIdx = 0;
	resetState();
}

void PhoenixVREngine::end() {
	debug("end");
	stopAllSounds();
	if (_nextScript.empty() && _nextWarp < 0) {
		if (!setNextLevel()) {
			debug("quit game");
			quitGame();
		}
	}
}

void PhoenixVREngine::interpolateAngle(float x, float y, float speed, float zoom) {
	debug("interpolateAngle %g,%g, speed: %g, zoom: %g", x, y, speed, zoom);
	unsigned frameDuration = 0;
	static constexpr float kDuration = 4096 * 16 / 1000.0f;
	auto x0 = _angleY.angle() + kPi2, y0 = _angleX.angle(), z0 = _fov;
	auto dx = x - x0, dy = y - y0, dz = zoom - z0;
	if (dy < -kPi)
		dy += kTau;
	if (dy > kPi)
		dy -= kTau;
	if (dx < -kPi)
		dx += kTau;
	if (dx > kPi)
		dx -= kTau;
	debug("dx: %g, dy: %g, dz: %g", dx, dy, dz);
	float t = 0;
	bool waiting = true;
	while (!shouldQuit() && waiting && t < kDuration) {
		auto t1 = t / kDuration; // normalise to 0..1 range
		// angles are animated using square function, zoom is linear
		auto t2 = t1 * t1;

		setAngle(x0 + t2 * dx, y0 + t2 * dy);
		if (zoom > 0) {
			setZoom(z0 + t1 * dz);
		}

		renderVR(frameDuration / 1000.0f);

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
				processGenericEvents(event);
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
		t += frameDuration / 1000.0f * speed;
	}
	setAngle(x, y);
	if (zoom > 0)
		setZoom(zoom);
}

void PhoenixVREngine::renderFade(int color) {
	auto &format = _screen->format;
	for (int y = 0; y != _screen->h; ++y) {
		for (int x = 0; x != _screen->w; ++x) {
			uint8 r, g, b;
			format.colorToRGB(_screen->getPixel(x, y), r, g, b);
			int ri = CLIP(static_cast<int>(r) + color, 0, 255);
			int gi = CLIP(static_cast<int>(g) + color, 0, 255);
			int bi = CLIP(static_cast<int>(b) + color, 0, 255);
			_screen->setPixel(x, y, format.RGBToColor(ri, gi, bi));
		}
	}
}

void PhoenixVREngine::fade(int start, int stop, int speed) {
	debug("fade %d %d speed: %d", start, stop, speed);

	if (start == stop)
		return;

	bool waiting = true;
	float pos = start, dt = 0;
	bool increment = start < stop;
	if (!increment)
		speed = -speed;

	float speedMs = speed * 1000.0f / 16;

	while (!shouldQuit() && waiting && (increment ? pos < stop : pos > stop)) {
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
				processGenericEvents(event);
				break;
			}
		}
		renderVR(dt);
		renderFade(pos);

		pos += dt * speedMs + ((speed >= 0) ? 1 : -1);

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		_screen->update();
		dt = _frameLimiter.startFrame() / 1000.0f;
	}
}

static uint32 transFadePixel(const Graphics::PixelFormat &format, uint32 left, int leftAmount, uint32 right, int rightAmount) {
	uint8 lr, lg, lb, rr, rg, rb;
	format.colorToRGB(left, lr, lg, lb);
	format.colorToRGB(right, rr, rg, rb);
	return format.RGBToColor(
		CLIP(CLIP(static_cast<int>(lr) + leftAmount, 0, 255) + CLIP(static_cast<int>(rr) + rightAmount, 0, 255), 0, 255),
		CLIP(CLIP(static_cast<int>(lg) + leftAmount, 0, 255) + CLIP(static_cast<int>(rg) + rightAmount, 0, 255), 0, 255),
		CLIP(CLIP(static_cast<int>(lb) + leftAmount, 0, 255) + CLIP(static_cast<int>(rb) + rightAmount, 0, 255), 0, 255));
}

void PhoenixVREngine::transFade(int speed) {
	debug("transfade speed: %d", speed);

	Graphics::ManagedSurface oldFrame(_screen->w, _screen->h, _screen->format);
	Graphics::ManagedSurface newFrame(_screen->w, _screen->h, _screen->format);
	Graphics::ManagedSurface workFrame(_screen->w, _screen->h, _screen->format);

	oldFrame.simpleBlitFrom(*_screen);
	renderVR(0);
	newFrame.simpleBlitFrom(*_screen);

	bool waiting = true;
	float dt = 0;

	auto renderTransition = [&](int oldAmount, int newAmount) {
		for (int y = 0; y < _screen->h; ++y) {
			for (int x = 0; x < _screen->w; ++x) {
				workFrame.setPixel(x, y, transFadePixel(_screen->format, oldFrame.getPixel(x, y), oldAmount, newFrame.getPixel(x, y), newAmount));
			}
		}
		_screen->simpleBlitFrom(workFrame);
	};

	auto runTransition = [&](int pos, int direction) {
		while (!shouldQuit() && waiting && (direction > 0 ? pos < 0 : pos > -256)) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_KEYDOWN:
					if (event.kbd.ascii == ' ')
						waiting = false;
					break;
				default:
					processGenericEvents(event);
					break;
				}
			}

			renderTransition(direction > 0 ? 0 : pos, direction > 0 ? pos : 0);
			_frameLimiter.delayBeforeSwap();
			_screen->update();
			dt = _frameLimiter.startFrame() / 1000.0f;

			pos += direction * static_cast<int>(dt * speed * 1000.0f / 16);
			if (direction > 0 ? pos < 0 : pos > -256)
				pos += direction;
		}
	};

	runTransition(-255, 1);
	runTransition(0, -1);
}

void PhoenixVREngine::until(const Common::String &var, int value) {
	debug("until %s %d", var.c_str(), value);
	unsigned frameDuration = 0;
	while (!shouldQuit() && getVariable(var) != value) {
		Common::Event event;
		renderVR(frameDuration / 1000.0f);
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			default:
				processGenericEvents(event);
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		drawAudioSubtitles();
		_frameLimiter.delayBeforeSwap();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
	}
}

void PhoenixVREngine::wait(float seconds) {
	debug("wait %gs", seconds);
	auto begin = g_system->getMillis();
	unsigned millis = seconds * 1000;
	bool waiting = true;
	unsigned frameDuration = 0;
	while (!shouldQuit() && waiting && g_system->getMillis() - begin < millis) {
		Common::Event event;
		renderVR(frameDuration / 1000.0f);
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			default:
				processGenericEvents(event);
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		drawAudioSubtitles();
		_frameLimiter.delayBeforeSwap();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
	}
}

void PhoenixVREngine::restart() {
	debug("restart");
	resetState();
	_restarted = true;
	_nextLevel = 0;
	setNextLevel();
	_prevWarp = -1;
	_loaded = false;
}

bool PhoenixVREngine::goToWarp(const Common::String &warp, bool savePrev) {
	debug("gotowarp %s, save prev: %d", warp.c_str(), savePrev);
	if (_warp && _warp->vrFile == warp) {
		debug("already at this location, skipping");
		return false;
	}

	// Typo in Necronomicon's Script4.lst
	if (gameIdMatches("necrono") && warp == "N3M09L03W515E1.vr")
		_nextWarp = _script->getWarp("N3M09L03W51E1.vr");
	else
		_nextWarp = _script->getWarp(warp);

	_hoverIndex = -1;
	_messengerInventoryHover = -1;
	if (savePrev) {
		assert(_warpIdx >= 0);
		_prevWarp = _warpIdx;
		// saving thumbnail
		Common::ScopedPtr<Graphics::ManagedSurface> screenshot(_screen->scale(_thumbnail.w, _thumbnail.h, true, Graphics::FLIP_V));
		_thumbnail.simpleBlitFrom(*screenshot);
	}
	return true;
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
	auto warp = _script->getWarp(wname);
	if (warp < 0) {
		debug("no warp %s", wname.c_str());
		return;
	}
	auto &cursors = _cursors[warp];
	if (idx >= 0 && idx < static_cast<int>(cursors.size()))
		cursors[idx] = path;
	else
		debug("index %d is out of range", idx);
}

void PhoenixVREngine::hideCursor(const Common::String &wname, int idx) {
	debug("hide cursor %s:%d", wname.c_str(), idx);
	auto warp = _script->getWarp(wname);
	if (warp < 0) {
		debug("no warp %s", wname.c_str());
		return;
	}
	auto &cursors = _cursors[warp];
	if (idx >= 0 && idx < static_cast<int>(cursors.size()))
		cursors[idx].clear();
	else
		debug("index %d is out of range", idx);
}

void PhoenixVREngine::declareVariable(const Common::String &name) {
	if (!_variables.contains(name))
		_variables.setVal(name, 0);
}

bool PhoenixVREngine::hasVariable(const Common::String &name) const {
	return _variables.contains(name);
}

void PhoenixVREngine::setVariable(const Common::String &name, int value) {
	if (!hasVariable(name)) {
		debug("set %s %d - ignored, variable was not declared", name.c_str(), value);
		return;
	}
	debug("set %s %d", name.c_str(), value);
	_variables.setVal(name, value);
}

int PhoenixVREngine::getVariable(const Common::String &name) const {
	if (!hasVariable(name))
		warning("get %s - variable was not declared", name.c_str());
	return _variables.getValOrDefault(name, 0);
}

static int8 panToBalance(int pan) {
	pan = CLIP(pan, 0, 255);
	return static_cast<int8>((pan * 254 + 127) / 255 - 127);
}

void PhoenixVREngine::playSound(const Common::String &sound, Audio::Mixer::SoundType type, uint8 volume, int loops, bool spatial, float angle) {
	const bool music = type == Audio::Mixer::kMusicSoundType;
	debug("play sound %s %d %d, music: %d, 3d: %d, angle: %g", sound.c_str(), volume, loops, music, spatial, angle);
	if (_sounds.contains(sound)) {
		debug("already playing, skipping...");
		return;
	}
	Audio::SoundHandle h;
	Common::ScopedPtr<Common::SeekableReadStream> stream(open(sound));
	if (!stream) {
		warning("can't load sound %s", sound.c_str());
		return;
	}

	if (music) {
		if (!_currentMusic.empty())
			stopSound(_currentMusic);
		_currentMusic = sound;
		_currentMusicVolume = volume;
	}

	_mixer->playStream(type, &h, Audio::makeWAVStream(stream.release(), DisposeAfterUse::YES), -1, volume, spatial ? 0 : panToBalance(_globalPan));
	if (loops < 0 || music)
		_mixer->loopChannel(h);
	Common::SharedPtr<Video::Subtitles> subtitles;
	if (!music)
		subtitles = loadSubtitles(sound);

	_sounds[sound] = Sound{h, spatial, angle, volume, loops, subtitles};
}

void PhoenixVREngine::playRandomSound(const Common::String &sound, Audio::Mixer::SoundType type, uint8 volume, int probability, int loops) {
	debug("register random sound %s %d %d, probability: %d", sound.c_str(), volume, loops, probability);
	_randomSounds.push_back(RandomSound{
		sound,
		type,
		volume,
		probability,
		loops});
}

void PhoenixVREngine::stopSound(const Common::String &sound) {
	debug("stop sound %s", sound.c_str());
	if (sound == _currentMusic)
		_currentMusic.clear();
	auto it = _sounds.find(sound);
	if (it != _sounds.end()) {
		_mixer->stopHandle(it->_value.handle);
		if (it->_value.subtitles)
			it->_value.subtitles->clearSubtitle();
		_sounds.erase(it);
	}
}

void PhoenixVREngine::stopAllSounds() {
	_mixer->stopAll();
	_currentMusic.clear();
	for (auto &kv : _sounds) {
		if (kv._value.subtitles)
			kv._value.subtitles->clearSubtitle();
	}
	_sounds.clear();
	_randomSounds.clear();
}

Common::Path PhoenixVREngine::getSubtitlePath(const Common::String &path) const {
	Common::Path assetPath(removeDrive(path), '\\');
	Common::String filename = assetPath.toString('/') + ".srt";
	filename.replace('/', '_');
	filename.replace('\\', '_');

	Common::String language = Common::getLanguageCode(_gameDescription->language);
	if (language == "us")
		language = "en";

	return Common::Path("subtitle").appendComponent(language).appendComponent(filename);
}

Common::SharedPtr<Video::Subtitles> PhoenixVREngine::loadSubtitles(const Common::String &path) const {
	Common::SharedPtr<Video::Subtitles> subtitles;
	if (!ConfMan.getBool("subtitles"))
		return subtitles;

	subtitles = Common::SharedPtr<Video::Subtitles>(new Video::Subtitles());
	subtitles->loadSRTFile(getSubtitlePath(path));
	if (!subtitles->isLoaded())
		return Common::SharedPtr<Video::Subtitles>();

	setupSubtitles(*subtitles);
	return subtitles;
}

void PhoenixVREngine::setupSubtitles(Video::Subtitles &subtitles) const {
	// Subtitle positioning constants (as percentages of screen height)
	const int HORIZONTAL_MARGIN = 20;
	const int MIN_BOTTOM_MARGIN = 4;
	const int MIN_SUBTITLE_HEIGHT = 90;
	const float BOTTOM_MARGIN_PERCENT = 0.01f;
	const float SUBTITLE_HEIGHT_PERCENT = 0.2f;

	// Font sizing constants (as percentage of screen height)
	const int MIN_FONT_SIZE = 18;
	const float BASE_FONT_SIZE_PERCENT = 1.0f / 36.0f;

	int16 h = g_system->getOverlayHeight();
	int16 w = g_system->getOverlayWidth();
	int bottomMargin = MAX<int>(MIN_BOTTOM_MARGIN, int(h * BOTTOM_MARGIN_PERCENT));
	int topOffset = MAX<int>(MIN_SUBTITLE_HEIGHT, int(h * SUBTITLE_HEIGHT_PERCENT));
	int fontSize = MAX<int>(MIN_FONT_SIZE, int(h * BASE_FONT_SIZE_PERCENT));

	subtitles.setBBox(Common::Rect(HORIZONTAL_MARGIN, h - topOffset, w - HORIZONTAL_MARGIN, h - bottomMargin));
	subtitles.setColor(0xff, 0xff, 0x80);
	subtitles.setFont("LiberationSans-Regular.ttf", fontSize, Video::Subtitles::kFontStyleRegular);
	subtitles.setFont("LiberationSans-Italic.ttf", fontSize, Video::Subtitles::kFontStyleItalic);
}

void PhoenixVREngine::playMovie(const Common::String &movie) {
	debug("playMovie %s", movie.c_str());
	Common::ScopedPtr<Video::VideoDecoder> dec;
	if (movie.hasSuffixIgnoreCase(".4xm")) {
		dec.reset(new Video::FourXMDecoder);
	} else if (movie.hasSuffixIgnoreCase(".smk")) {
		dec.reset(new Video::SmackerDecoder);
	} else {
		warning("can't play %s", movie.c_str());
		return;
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(open(movie));
	if (!stream) {
		warning("can't open movie %s", movie.c_str());
		return;
	}
	if (!dec->loadStream(stream.release())) {
		warning("loading movie stream %s failed", movie.c_str());
		return;
	}

	_system->lockMouse(false);
	dec->start();

	Common::SharedPtr<Video::Subtitles> subtitles = loadSubtitles(movie);
	if (subtitles) {
		g_system->showOverlay(false);
		g_system->clearOverlay();
	}

	bool playing = true;
	Common::ScopedPtr<Graphics::Palette> palette;
	while (!shouldQuit() && playing && !dec->endOfVideo()) {
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
				processGenericEvents(event);
				break;
			}
		}
		if (dec->needsUpdate()) {
			auto *s = dec->decodeNextFrame();
			if (dec->hasDirtyPalette()) {
				palette.reset(new Graphics::Palette(dec->getPalette(), 256));
			}
			if (s) {
				if (!s->format.isCLUT8() || palette)
					_screen->simpleBlitFrom(*s, Graphics::FLIP_NONE, false, 0xff, palette.get());
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		if (subtitles && !dec->isPaused())
			subtitles->drawSubtitle(dec->getTime(), false);
		_screen->update();
		_frameLimiter.startFrame();
	}
	if (subtitles)
		g_system->hideOverlay();
	_system->lockMouse(_vr.isVR());
}

void PhoenixVREngine::playAnimation(const Common::String &name, const Common::String &var, int varValue, float speed) {
	_vr.playAnimation(name, var, varValue, speed);
}

void PhoenixVREngine::stopAnimation(const Common::String &name) {
	_vr.stopAnimation(name);
}

void PhoenixVREngine::resetLockKey() {
	debug("resetlockkey");
	_prevWarp = -1; // original game does only this o_O
}

const Graphics::Surface *PhoenixVREngine::findArchiveImage(const Common::String &image) const {
	const Graphics::Surface *surface = _arn ? _arn->get(image) : nullptr;
	if (!surface && !image.contains('.'))
		surface = _arn ? _arn->get(image + ".bmp") : nullptr;
	return surface;
}

void PhoenixVREngine::drawArchiveImage(const Common::String &image, int x, int y) {
	const Graphics::Surface *surface = findArchiveImage(image);
	if (!surface) {
		warning("can't find archive image %s", image.c_str());
		return;
	}

	_screen->simpleBlitFrom(*surface, Common::Point(x, y));

	Common::Point pos(x, y);
	for (ArchiveImage &archiveImage : _archiveImages) {
		if (archiveImage.pos == pos) {
			archiveImage.image = image;
			return;
		}
	}

	_archiveImages.push_back({image, pos});
}

void PhoenixVREngine::drawArchiveText(int textId, const Common::Rect &dstRect, int size, bool bold, uint16 color) {
	for (TextState &archiveText : _archiveTexts) {
		if (archiveText.rect == dstRect) {
			archiveText = {textId, dstRect, size, bold, color};
			paintText(archiveText);
			return;
		}
	}

	_archiveTexts.push_back({textId, dstRect, size, bold, color});
	paintText(_archiveTexts.back());
}

void PhoenixVREngine::clearArchiveText(const Common::Rect &dstRect) {
	for (uint i = 0; i < _archiveTexts.size(); ++i) {
		if (_archiveTexts[i].rect == dstRect) {
			_archiveTexts.remove_at(i);
			return;
		}
	}
}

void PhoenixVREngine::showImageOverlay(const Common::String &image, int x, int y) {
	debug("AfficheImage %s %d %d", image.c_str(), x, y);
	_imageOverlay.reset();

	const Graphics::Surface *surface = findArchiveImage(image);
	if (!surface) {
		warning("can't find image overlay %s", image.c_str());
		return;
	}

	_imageOverlay.reset(new Graphics::ManagedSurface());
	_imageOverlay->convertFrom(*surface, _pixelFormat);
	_imageOverlay->setTransparentColor(_imageOverlay->getPixel(_imageOverlay->w - 1, _imageOverlay->h - 1));
	_imageOverlayPos = Common::Point(x, y);
}

void PhoenixVREngine::stopImageOverlay() {
	debug("StopAffiche");
	_imageOverlay.reset();
	updateStage();
}

void PhoenixVREngine::updateStage() {
	renderVR(0);
	_screen->update();
}

void PhoenixVREngine::startCible(const Common::String &name, int periodSeconds, const Common::Array<int> &bounds) {
	debug("StartCible %s %d", name.c_str(), periodSeconds);
	_cibleActive = true;
	_cibleStartMillis = g_system->getMillis();
	_ciblePeriodSeconds = periodSeconds;
	_cibleBounds = bounds;
}

void PhoenixVREngine::stopCible() {
	debug("StopCible");
	_cibleActive = false;
}

void PhoenixVREngine::testCible(const Common::String &insideVar, const Common::String &outsideVar) {
	debug("TestCible %s %s", insideVar.c_str(), outsideVar.c_str());
	if (!_cibleActive)
		return;

	bool inside = false;
	int periodMillis = _ciblePeriodSeconds * 1000;
	if (periodMillis > 0) {
		int elapsed = (g_system->getMillis() - _cibleStartMillis) % periodMillis;
		for (uint i = 0; i + 1 < _cibleBounds.size() && _cibleBounds[i] != 0; i += 2) {
			if (_cibleBounds[i] * 1000 < elapsed && elapsed < _cibleBounds[i + 1] * 1000) {
				inside = true;
				break;
			}
		}
	}

	setVariable(insideVar, inside ? 1 : 0);
	setVariable(outsideVar, inside ? 0 : 1);
}

void PhoenixVREngine::lockKey(int idx, const Common::String &warp) {
	_lockKey[idx] = warp;
}

Graphics::ManagedSurface *PhoenixVREngine::loadSurface(const Common::String &path) {
	Common::String filename = path;
	Common::ScopedPtr<Common::SeekableReadStream> stream(open(path, &filename));
	if (!stream) {
		warning("can't find image %s", path.c_str());
		return nullptr;
	}
	Common::ScopedPtr<Image::ImageDecoder> dec;
	if (filename.hasSuffixIgnoreCase(".pcx")) {
		dec.reset(new Image::PCXDecoder);
	} else if (filename.hasSuffixIgnoreCase(".gif")) {
		dec.reset(new Image::GIFDecoder);
	} else {
		warning("can't find decoder for %s", filename.c_str());
		return nullptr;
	}
	if (!dec->loadStream(*stream)) {
		warning("decoding %s failed", filename.c_str());
		return nullptr;
	}
	auto *s = new Graphics::ManagedSurface();
	s->copyFrom(*dec->getSurface());
	if (dec->hasPalette())
		s->setPalette(dec->getPalette().data(), 0, dec->getPalette().size());
	// TODO: Skip conversion for surfaces with palettes?
	s->convertToInPlace(_pixelFormat);
	s->setTransparentColor(s->format.RGBToColor(0, 0, 0));
	return s;
}

Graphics::ManagedSurface *PhoenixVREngine::loadCursor(const Common::String &path) {
	if (path.empty())
		return nullptr;
	auto it = _cursorCache.find(path);
	if (it != _cursorCache.end())
		return it->_value;
	auto s = loadSurface(path);
	if (!s) {
		warning("can't load cursor from %s", path.c_str());
		return nullptr;
	}
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

void PhoenixVREngine::startTimer(float seconds, bool showTimer) {
	_timer = seconds;
	_initialTimer = seconds;
	_timerFlags = 5;
	_showTimer = showTimer;
}

void PhoenixVREngine::pauseTimer(bool pause, bool deactivate) {
	if (_timerFlags) {
		if (pause)
			_timerFlags |= 2;
		else
			_timerFlags &= ~2;
		if (deactivate)
			_timerFlags &= ~4;
		else
			_timerFlags |= 4;
	}
}

void PhoenixVREngine::killTimer() {
	_timerFlags = 0;
	_showTimer = false;
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

void PhoenixVREngine::renderTimer() {
	if ((_timerFlags & 4) == 0 || !_showTimer || !_arn)
		return;
	auto timerBg = _arn->get("cadre.bmp");
	auto timerFg = _arn->get("cadreB.bmp");
	if (!timerBg || !timerFg)
		return;

	// Necronomicon has timer in scripts, but does not contain bitmaps for timers.
	Common::Rect bgRect{320, 16, 632, 44};
	Common::Rect fgRect{333, 23, 619, 38};
	if (gameIdMatches("dracula2") || gameIdMatches("messenger")) {
		bgRect = Common::Rect(165, 15, 474, 48);
		fgRect = Common::Rect(177, 15, 461, 48);
	}
	assert(_initialTimer > 0);
	auto timeLeft = _timer / _initialTimer;
	fgRect.right = fgRect.left + fgRect.width() * timeLeft;
	Common::Rect fgSrcRect(0, 0, timerFg->w, timerFg->h);
	fgSrcRect.right = fgSrcRect.left + fgSrcRect.width() * timeLeft;
	if (!fgRect.isValidRect() || !fgSrcRect.isValidRect())
		return;
	_screen->simpleBlitFrom(*timerBg, bgRect.origin());
	_screen->simpleBlitFrom(*timerFg, fgSrcRect, fgRect.origin());
}

void PhoenixVREngine::renderVR(float dt) {
	_vr.render(_screen, _angleX.angle(), _angleY.angle(), _fov, dt, _showRegions ? _regSet.get() : nullptr);
	paintText(_rolloverText);
	renderArchiveImages();
	renderArchiveTexts();
	renderImageOverlay();
	renderTimer();
}

void PhoenixVREngine::renderArchiveImages() {
	for (const ArchiveImage &archiveImage : _archiveImages) {
		const Graphics::Surface *surface = findArchiveImage(archiveImage.image);
		if (!surface)
			continue;

		_screen->simpleBlitFrom(*surface, archiveImage.pos);
	}
}

void PhoenixVREngine::renderArchiveTexts() {
	for (const TextState &archiveText : _archiveTexts)
		paintText(archiveText);
}

void PhoenixVREngine::paintText(const TextState &textState) {
	auto *font = getFont(textState.size, textState.bold);
	if (!font || !_textes.contains(textState.textId))
		return;

	Common::Array<Common::U32String> lines;
	font->wordWrapText(_textes.getVal(textState.textId), textState.rect.width(), lines, Graphics::kWordWrapDefault);
	if (lines.empty())
		return;

	int fontH = font->getFontHeight();
	int textW = 0;
	for (uint i = 0; i < lines.size(); ++i) {
		textW = MAX(textW, font->getStringWidth(lines[i]));
	}

	byte r, g, b;
	_rgb565.colorToRGB(textState.color, r, g, b);
	uint32 textColor = _screen->format.RGBToColor(r, g, b);
	int16 dstX = textState.rect.left + (textState.rect.width() - textW) / 2;
	int16 dstY = textState.rect.top + (textState.rect.height() - fontH * lines.size()) / 2;
	for (uint i = 0; i < lines.size(); ++i) {
		int16 lineX = dstX + (textW - font->getStringWidth(lines[i])) / 2;
		font->drawString(_screen, lines[i], lineX, dstY + i * fontH, textState.rect.right - lineX, textColor, Graphics::kTextAlignLeft);
	}
}

void PhoenixVREngine::renderImageOverlay() {
	if (_imageOverlay)
		_screen->simpleBlitFrom(*_imageOverlay, _imageOverlayPos);
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

const Graphics::Font *PhoenixVREngine::getFont(int size, bool bold) const {
#ifdef USE_FREETYPE2
	const int fontMaxSizes[] = {10, 12, 14, 16, 18, INT_MAX};

	for (uint i = 0; i < ARRAYSIZE(fontMaxSizes); ++i) {
		if (size < fontMaxSizes[i]) {
			const Graphics::Font *font = bold ? _boldFonts[i].get() : nullptr;
			return font ? font : _regularFonts[i].get();
		}
	}

	return _regularFonts[ARRAYSIZE(fontMaxSizes) - 1].get();
#else
	return FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
#endif
}

void PhoenixVREngine::rollover(int textId, RolloverType type) {
	Common::Rect dstRect;
	int size = 12;
	bool bold = false;
	uint16 color = 0xFFFF;

	if (gameIdMatches("lochness")) {
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

	auto *font = getFont(size, bold);
	if (!font)
		return;

	if (!_textes.contains(textId)) {
		debug("text cleared");
		_rolloverText = TextState();
		return;
	}
	auto &text = _textes.getVal(textId);
	debug("rollover %s, %s font size: %d, bold: %d, color: %02x", dstRect.toString().c_str(), text.encode(Common::kUtf8).c_str(), size, bold, color);

	_rolloverText = TextState(textId, dstRect, size, bold, color);
}

void PhoenixVREngine::clearText() {
	_rolloverText = TextState();
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
			continue;
		}
		if (!sound.spatial)
			continue;

		int8 balance = 127 * sinf(sound.angle - _angleX.angle());
		_mixer->setChannelBalance(sound.handle, balance);
	}
	for (auto &sound : finishedSounds) {
		debug("sound %s stopped", sound.c_str());
		auto it = _sounds.find(sound);
		if (it != _sounds.end()) {
			if (it->_value.subtitles)
				it->_value.subtitles->clearSubtitle();
			_sounds.erase(it);
		}
	}
	if (!_randomSounds.empty()) {
		for (auto &sound : _randomSounds) {
			uint32 rnd = getRandomNumber(UINT_MAX);
			rnd /= (((0x3C0000 * sound.probability) >> 8) + 0x40000);
			uint32 lastFrameMs = dt * 1000u;
			if (rnd < lastFrameMs) {
				debug("random sound %s triggered: rnd: %d -> %d < %u", sound.sound.c_str(), sound.probability, rnd, lastFrameMs);
				playSound(sound.sound, sound.type, sound.volume, sound.loops);
			}
		}
	}

	if (!_nextScript.empty()) {
		loadNextScript();
		goToWarp(_script->getInitScript()->vrFile);
	}
	while (_nextWarp >= 0) {
		_rolloverText = TextState();
		_archiveImages.clear();
		_archiveTexts.clear();
		_warpIdx = _nextWarp;
		_warp = _script->getWarp(_nextWarp);
		debug("warp %d -> %s %s", _nextWarp, _warp->vrFile.c_str(), _warp->testFile.c_str());
		_nextWarp = -1;
		_randomSounds.clear();

		{
			Common::String origName;
			Common::ScopedPtr<Common::SeekableReadStream> stream(open(_warp->vrFile, &origName));
			bool isVr = origName.empty() || origName.hasSuffixIgnoreCase(".vr");
			if (stream && isVr) {
				_vr = VR::loadStatic(_rgb565, *stream);
				if (_pixelFormat != _rgb565)
					_vr.getSurface().convertToInPlace(_pixelFormat);
				if (_vr.isVR()) {
					_mousePos = _screenCenter;
					_mouseRel = {};
				}
				_system->lockMouse(_vr.isVR());
			} else
				debug("can't find vr file %s", _warp->vrFile.c_str());
		}

		{
			Common::ScopedPtr<Common::SeekableReadStream> stream(!_warp->testFile.empty() ? open(_warp->testFile) : nullptr);
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
		_restarted = false;
	}

	if (_nextTest >= 0) {
		auto nextTest = _nextTest;
		_nextTest = -1;
		executeTest(nextTest);
	}

	renderVR(dt);

	Graphics::ManagedSurface *cursor = nullptr;
	auto &cursors = _cursors[_warpIdx];
	bool anyMatched = false;
	int messengerInventoryHover = -1;
	int regionCount = _regSet ? _regSet->size() : 0;
	for (int i = 0, n = MAX<int>(regionCount, cursors.size()); i != n; ++i) {
		auto *region = getRegion(i);
		if (!region)
			continue;

		if (_vr.isVR() ? region->contains3D(currentVRPos()) : region->contains2D(_mousePos.x, _mousePos.y)) {
			anyMatched = true;
			if (gameIdMatches("messenger") && _warp && _warp->vrFile.equalsIgnoreCase("portef.vr") && i >= 0 && i < 12)
				messengerInventoryHover = i;

			auto test = _warp->getTest(i);
			if (test && test->hover == 1 && _hoverIndex < 0) {
				debug("executing hover test %d", i);
				_hoverIndex = i;
				executeTest(i);
			}

			if (!cursor && i < static_cast<int>(cursors.size())) {
				auto &name = cursors[i];
				cursor = loadCursor(name);
			}
		} else if (i == _hoverIndex) {
			debug("leaving hover region");
			auto leave = _warp->getTest(i - 1);
			if (!leave || leave->hover != 2)
				leave = _warp->getTest(i + 1);
			if (leave && leave->hover == 2) {
				executeTest(leave->idx);
			}
			_hoverIndex = -1;
		}
	}

	if (messengerInventoryHover != _messengerInventoryHover) {
		_messengerInventoryHover = messengerInventoryHover;
		if (gameIdMatches("messenger") && _warp && _warp->vrFile.equalsIgnoreCase("portef.vr")) {
			const Common::Rect textRect(62, 402, 354, 466);
			drawArchiveImage("EffaceText.bmp", textRect.left, textRect.top);
			clearArchiveText(textRect);

			int textId = 0;
			if (_messengerInventoryHover >= 0)
				textId = getVariable(Common::String::format("Pos%d", _messengerInventoryHover + 1));
			else {
				int selection = getVariable("Selection");
				if (selection > 100)
					selection -= 100;
				if (selection >= 1 && selection <= 12)
					textId = getVariable(Common::String::format("Pos%d", selection));
			}

			if (textId != 0)
				drawArchiveText(textId, textRect, 14, true, 1987);
		}
	}

	if (!cursor)
		cursor = loadCursor(anyMatched ? _defaultCursor[1] : _defaultCursor[0]);
	if (cursor) {
		_screen->simpleBlitFrom(*cursor, _mousePos - Common::Point(cursor->w / 2, cursor->h / 2));
	}
}

void PhoenixVREngine::drawAudioSubtitles() {
	if (!ConfMan.getBool("subtitles"))
		return;

	for (auto &kv : _sounds) {
		auto &sound = kv._value;
		if (sound.subtitles && _mixer->isSoundHandleActive(sound.handle))
			sound.subtitles->drawSubtitle(_mixer->getElapsedTime(sound.handle).msecs(), false);
	}
}

Common::Error PhoenixVREngine::run() {
	Common::List<Graphics::PixelFormat> formats;
	formats.push_back(_rgb565);
	initGraphics(640, 480, formats);

	_pixelFormat = g_system->getScreenFormat();
	if (_pixelFormat.isCLUT8())
		return Common::kUnsupportedColorMode;

	_arn.reset(ARN::create());
#ifdef USE_FREETYPE2
	static const Common::String regular("NotoSans-Regular.ttf");
	static const Common::String bold("NotoSans-Bold.ttf");
	const int fontSizes[] = {8, 10, 12, 14, 16, 18};
	for (uint i = 0; i < ARRAYSIZE(fontSizes); ++i) {
		_regularFonts[i].reset(Graphics::loadTTFFontFromArchive(regular, fontSizes[i]));
		_boldFonts[i].reset(Graphics::loadTTFFontFromArchive(bold, fontSizes[i]));
	}
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
		if (textes.open(Common::Path("textes.txt"))) {
			Common::CodePage textCodePage = getTextCodePage(_gameDescription->language);
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
				_textes.setVal(textId, Common::convertToU32String(text.c_str() + pos, textCodePage));
			}
			debug("loaded %u textes", _textes.size());
		}
	}

	// try load level-specific script first (amerzone)
	if (gameIdMatches("amerzone")) {
		setNextScript("intro.lst");
	} else if (gameIdMatches("lochness"))
		setNextScript("first.lst");
	else
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
				if (!_hasFocus)
					break;
				if (_prevWarp != -1) {
					returnToWarp();
					break;
				}
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
				int regionCount = _regSet ? _regSet->size() : 0;
				for (uint i = 0, n = MAX<int>(regionCount, cursors.size()); i != n; ++i) {
					auto *region = getRegion(i);
					if (!region)
						continue;

					auto test = _warp->getTest(i);
					if (test && test->hover != 0)
						continue;

					if (_vr.isVR() ? region->contains3D(vrPos) : region->contains2D(event.mouse.x, event.mouse.y)) {
						debug("click region %u", i);
						if (auto clickTest = _warp->getLastTest(i)) {
							Script::ExecutionContext ctx;
							clickTest->scope.exec(ctx);
						} else
							warning("invalid test id %u", i);
						break;
					}
				}
			} break;
			default:
				processGenericEvents(event);
				break;
			}
		}
		float dt = float(frameDuration) / 1000.0f;
		if (dt > kMaxTick)
			dt = kMaxTick;
		tick(dt);

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		drawAudioSubtitles();
		_frameLimiter.delayBeforeSwap();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
	}

	return Common::kNoError;
}

void PhoenixVREngine::processGenericEvents(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_FOCUS_GAINED:
		_hasFocus = true;
		_mouseRel = {};
		break;
	case Common::EVENT_FOCUS_LOST:
		_hasFocus = false;
		break;
	default:
		break;
	}
}

void PhoenixVREngine::pauseEngineIntern(bool pause) {
	// this is called when main menu appears on the screen
	Engine::pauseEngineIntern(pause);
	if (pause) {
		_system->lockMouse(false);
	} else {
		_system->lockMouse(_vr.isVR());
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
	debug("captureContext: warpIdx: %d, prev: %d", _warpIdx, _prevWarp);
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
		auto &name = kv._key;
		auto &sound = kv._value;
		if (sound.loops >= 0 || name == _currentMusic)
			continue;
		if (sound.spatial)
			sounds3d.push_back({name, sound.volume, fromAngle(sound.angle)});
		else
			sounds.push_back({name, sound.volume, 0});
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
	if (angleXMax != -1)
		setXMax(toAngle(angleXMax));
	if (angleYMin != -1 && angleYMax != -1)
		setYMax(toAngle(angleYMin), toAngle(angleYMax));

	_nextWarp = currentWarpIdx;

	for (auto &warpCursors : _cursors) {
		for (auto &warpCursor : warpCursors) {
			auto cursor = ms.readString(0, 257);
			if (cursor.hasSuffix(".VR") || cursor.hasSuffix(".vr")) {
				debug("ignoring VR cursor, original engine saves `LOAD.VR` as a cursor name at loading screen");
				cursor.clear();
			}
			warpCursor = cursor;
		}
	}
	debug("vars at %08x", (uint32)ms.pos());
	for (auto &name : _script->getVarNames()) {
		auto value = ms.readSint32LE();
		debug("var %s: %d", name.c_str(), value);
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

	stopAllSounds();

	_currentMusic = ms.readString(0, 257);
	_currentMusicVolume = ms.readUint32LE();
	debug("current music %s, volume: %u", _currentMusic.c_str(), _currentMusicVolume);
	if (!_currentMusic.empty() && _currentMusicVolume > 0)
		playSound(_currentMusic, Audio::Mixer::kMusicSoundType, _currentMusicVolume, -1);

	// sound samples
	for (uint i = 0; i != 8; ++i) {
		auto name = ms.readString(0, 257);
		auto vol = ms.readUint32LE();
		auto flags = ms.readUint32LE();
		debug("sound: %s vol: %u flags: %u", name.c_str(), vol, flags);
		if (!name.empty() && name != _currentMusic)
			playSound(name, Audio::Mixer::kSFXSoundType, vol, -1);
	}

	// sound samples 3D
	for (uint i = 0; i != 8; ++i) {
		auto name = ms.readString(0, 257);
		auto angle = ms.readUint32LE();
		auto vol = ms.readUint32LE();
		auto flags = ms.readUint32LE();
		debug("3d sound: %s vol: %u flags: %u angle: %u", name.c_str(), vol, flags, angle);
		if (!name.empty())
			playSound(name, Audio::Mixer::kSFXSoundType, vol, -1, true, static_cast<float>(angle) * kPi);
	}
	_loadedState.clear();
	return true;
}

Common::Error PhoenixVREngine::loadGameStream(Common::SeekableReadStream *slot) {
	auto state = GameState::load(*slot);
	while (!state.script.empty() &&
		   (state.script.lastChar() == '\n' || state.script.lastChar() == '\r'))
		state.script = state.script.substr(0, state.script.size() - 1);

	_loaded = true;
	killTimer();
	setNextScript(state.script);
	if (!_levels.empty()) {
		uint i = 0, n = _levels.size();
		for (; i != n; ++i) {
			auto &level = _levels[i];
			if (state.script.hasPrefixIgnoreCase(level)) {
				_nextLevel = i + 1;
				debug("current level is %u", _nextLevel);
				break;
			}
		}
		if (i == n)
			warning("couldn't find current level index for script %s", state.script.c_str());
	}
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
	_loaded = false;

	return Common::kNoError;
}

Common::Error PhoenixVREngine::saveGameStream(Common::WriteStream *slot, bool isAutosave) {
	GameState state;
	state.script = _contextScript;
	state.game = _contextLabel;
	const bool isAmerzone = gameIdMatches("amerzone");
	Common::String amerzoneLevelLabel;
	if (isAmerzone) {
		amerzoneLevelLabel = getAmerzoneLevelLabel(state.script);
		state.game.clear();
	}

	TimeDate td = {};
	g_system->getTimeAndDate(td);
	state.info = formatSaveInfo(td, isAmerzone, amerzoneLevelLabel);

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
	state.state = _capturedState;

	state.save(*slot);
	return Common::kNoError;
}

void PhoenixVREngine::drawSaveCard(int idx) {
	if (!gameIdMatches("amerzone")) {
		static const int faces[] = {4, 3, 5, 1};
		const int face = faces[(idx - 1) / 2];
		const bool odd = (idx - 1) & 1;
		drawSlot(idx, face, odd ? 275 : 97, 200);
		return;
	}

	Common::ScopedPtr<Common::InSaveFile> slot(_saveFileMan->openForLoading(getSaveStateName(idx)));
	if (!slot)
		return;

	auto state = GameState::load(*slot);
	auto &dst = _vr.getSurface();
	Graphics::Surface *thumbnail = state.getThumbnail(dst.format, 232);
	const int cardW = thumbnail->w + 6;
	const int cardH = thumbnail->w + 30;

	Graphics::ManagedSurface card(cardW, cardH, dst.format);
	const uint32 white = dst.format.RGBToColor(0xff, 0xff, 0xff);
	const uint32 black = dst.format.RGBToColor(0, 0, 0);
	card.fillRect(Common::Rect(0, 0, cardW, cardH), white);
	card.fillRect(Common::Rect(0, 0, cardW, 1), black);
	card.fillRect(Common::Rect(0, cardH - 1, cardW, cardH), black);
	card.fillRect(Common::Rect(0, 0, 1, cardH), black);
	card.fillRect(Common::Rect(cardW - 1, 0, cardW, cardH), black);
	card.copyRectToSurface(*thumbnail, 3, 6, thumbnail->getRect());

	const Graphics::Font *font = getFont(16, true);
	if (font) {
		int textY = thumbnail->h + 18;
		textY = drawSaveTextBlock(*card.surfacePtr(), font, state.game, 0, textY, cardW, black, Graphics::kTextAlignCenter, 18, false, 0);
		drawSaveTextBlock(*card.surfacePtr(), font, state.info, 0, textY, cardW, black, Graphics::kTextAlignCenter, 18, false, 0);
	}

	static const int faces[] = {4, 3, 5, 1};
	const int face = faces[(idx - 1) / 2];
	const float angle = ((idx - 1) & 1) ? -kPi / 8.0f : kPi / 8.0f;
	Graphics::ManagedSurface faceSurface(512, 512, dst.format);
	copyCubeFaceToSurface(faceSurface, dst, face);
	projectSaveCard(faceSurface, card, angle);
	copySurfaceToCubeFace(dst, faceSurface, face);

	thumbnail->free();
	delete thumbnail;
}

void PhoenixVREngine::drawSlot(int idx, int face, int x, int y) {
	Common::ScopedPtr<Common::InSaveFile> slot(_saveFileMan->openForLoading(getSaveStateName(idx)));
	if (!slot)
		return;
	auto state = GameState::load(*slot);
	const bool isAmerzone = gameIdMatches("amerzone");

	y += face * 4 * 256;
	bool splitV = true;
	if (x > 256) {
		x -= 256;
		y += 256;
		splitV = false;
	}

	auto &dst = _vr.getSurface();
	auto *src = state.getThumbnail(dst.format, isAmerzone ? 232 : 0);
	int tileY = y / 256;
	if (isAmerzone) {
		const int cardX = x - 3;
		const int cardY = y - 6;
		const int cardW = src->w + 6;
		const int cardH = src->w + 30;
		uint32 white = dst.format.RGBToColor(0xff, 0xff, 0xff);
		uint32 black = dst.format.RGBToColor(0, 0, 0);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY, cardX + cardW, cardY + cardH), white, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY, cardX + cardW, cardY + 1), black, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY + cardH - 1, cardX + cardW, cardY + cardH), black, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY, cardX + 1, cardY + cardH), black, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX + cardW - 1, cardY, cardX + cardW, cardY + cardH), black, splitV, tileY);
		x = cardX + 3;
		y = cardY + 6;
		tileY = y / 256;
	}
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
	auto *font = getFont(isAmerzone ? 10 : 12, isAmerzone);
	if (font) {
		auto color = dst.format.RGBToColor(0, 0, 0);
		int textX = x;
		int textW = src->w;
		Graphics::TextAlign textAlign = Graphics::kTextAlignLeft;
		int textY = y + 0x72;
		int lineHeight = 14;
		if (isAmerzone) {
			textX = x - 3;
			textW = src->w + 6;
			textY = y - 6 + src->h + 14;
			textAlign = Graphics::kTextAlignCenter;

			textY = drawSaveTextBlock(dst, font, state.game, textX, textY, textW, color, textAlign, lineHeight, splitV, tileY);
			drawSaveTextBlock(dst, font, state.info, textX, textY, textW, color, textAlign, lineHeight, splitV, tileY);
		} else {
			drawSaveTextBlock(dst, font, state.game, textX, textY, textW, color, textAlign, lineHeight, splitV, tileY, true);
			drawSaveTextBlock(dst, font, state.info, textX, textY + lineHeight, textW, color, textAlign, lineHeight, splitV, tileY);
		}
	}

	src->free();
	delete src;
}

void PhoenixVREngine::setGlobalVolume(int volume) {
	ConfMan.setInt("music_volume", volume);
	ConfMan.setInt("sfx_volume", volume);
	syncSoundSettings();
}

void PhoenixVREngine::setGlobalPan(int pan) {
	_globalPan = CLIP(pan, 0, 255);
	int8 balance = panToBalance(_globalPan);

	for (auto &kv : _sounds) {
		if (!kv._value.spatial && _mixer->isSoundHandleActive(kv._value.handle))
			_mixer->setChannelBalance(kv._value.handle, balance);
	}
}

void PhoenixVREngine::syncSoundSettings() {
	int musicVolume = ConfMan.getInt("music_volume");
	int sfxVolume = ConfMan.getInt("sfx_volume");
	debug("syncSoundSettings, music: %d, sfx: %d", musicVolume, sfxVolume);
	bool muted = false;
	if (ConfMan.hasKey("mute")) {
		muted = ConfMan.getBool("mute");
	}
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, muted ? 0 : musicVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, muted ? 0 : sfxVolume);
}

} // End of namespace PhoenixVR
