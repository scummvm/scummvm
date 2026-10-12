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

#include "common/memstream.h"
#include "common/random.h"
#include "common/textconsole.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/convo.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/viewport.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace Script {

struct RecordKindNames {
	const char *kindName;
	const char *typeName;
};

static const RecordKindNames kRecordKindNames[kNumRecordKinds] = {
	{ "NavLogic", "ND.NavLogic" },
	{ "Summary", "ND.Summary" },
	{ "Overlay", "ND.Overlay" },
	{ "Movie", "ND.Movie" },
	{ "Sound", "ND.SoundAR" },
	{ "VideoConvo", "ND.ConvoAR" },
	{ "SoundConvo", "ND.ConvoAR" },
	{ "Text", "ND.TextAR" },
	{ "Override", "ND.OverrideAR" },
	{ "Particle", "ND.ParticleAR" },
	{ "AnimOverlay", "ND.AnimOvlAR" }
};

const char *getRecordKindName(RecordKind kind) {
	return kRecordKindNames[kind].kindName;
}

const char *getRecordTypeName(RecordKind kind) {
	return kRecordKindNames[kind].typeName;
}

Record *makeRecord(RecordKind kind, SceneContext &context) {
	switch (kind) {
	case kRecordNavLogic:
		return new NavLogicRecord(context);
	case kRecordSummary:
		return new SummaryRecord(context);
	case kRecordOverlay:
		return new OverlayRecord(context);
	case kRecordMovie:
		return new MovieRecord(context);
	case kRecordSound:
		return new SoundRecord(context);
	case kRecordOverride:
		return new OverrideRecord(context);
	case kRecordAnimOverlay:
		return new AnimOverlayRecord(context);
	case kRecordVideoConvo:
		return new ConvoRecord(context, true);
	case kRecordSoundConvo:
		return new ConvoRecord(context, false);
	case kRecordText:
		return new TextRecord(context);
	default:
		// TODO: The Particle record
		return nullptr;
	}
}

// The background of a scene is as far back as anything gets, and the records
// default to the middle. The viewport has its place among what the engine draws,
// so the numbers of the scripts are counted from there.
static const double kBackgroundScriptZ = -5.0;
static const int kBackgroundZ = 6;

void RecordGraphic::setScriptZ(double z, int32 streamZOffset) {
	const int engineZ = kBackgroundZ + (int)(z - kBackgroundScriptZ) + streamZOffset;
	setZOrder((uint16)CLIP<int>(engineZ, 0, 0xFFFF));
}

// Where something that was placed in the viewport ends up once the transform
// of its record has moved it.
// TODO: Movies only take their place from the transform. Draw what it rotates
// and scales as well.
static Common::Rect getTransformedDest(const ScriptRect &dest, const ScriptMatrix &transform, int width, int height) {
	// Far enough for anything a script moves around, and safe to add a size to
	const int kLimit = 16000;

	double x = dest.left;
	double y = dest.top;
	transform.transformPoint(x, y);

	const int left = CLIP<int>((int)x, -kLimit, kLimit);
	const int top = CLIP<int>((int)y, -kLimit, kLimit);
	return Common::Rect(left, top, left + width, top + height);
}

static const uint32 kNoModColor = 0xFFFFFFFF;

// Shows a part of an image where the transform of its record puts it. A transform
// that turns or resizes the picture has it drawn anew, into the area its corners
// end up around, and so does a color the picture gets multiplied with, which
// can darken it or make it see-through.
static void placePicture(RecordGraphic &graphic, Graphics::ManagedSurface &image, const Common::Rect &source, const ScriptRect &dest, const ScriptMatrix &transform, uint32 modColor) {
	// Larger than anything a script shows, and small enough to draw quickly
	const int kMaxSize = 2048;
	const int kLimit = 16000;

	const bool isMovedOnly = transform.isTranslation();
	if (isMovedOnly && modColor == kNoModColor) {
		graphic._drawSurface.create(image, source);
		graphic.setTransparent(true);
		graphic.moveTo(getTransformedDest(dest, transform, source.width(), source.height()));
		graphic.setNeedsRedraw(true);
		return;
	}

	// Points are rows, so the matrix turns (x, y) into (x * a + y * c, x * b + y * d),
	// and then moves it
	const double a = transform.m[0][0];
	const double b = transform.m[0][1];
	const double c = transform.m[1][0];
	const double d = transform.m[1][1];
	const double offsetX = transform.m[3][0];
	const double offsetY = transform.m[3][1];
	const double determinant = a * d - b * c;

	Common::Rect area = getTransformedDest(dest, transform, source.width(), source.height());
	if (!isMovedOnly) {
		double minX = 0.0;
		double minY = 0.0;
		double maxX = 0.0;
		double maxY = 0.0;
		for (uint corner = 0; corner < 4; ++corner) {
			double x = (corner & 1) ? dest.right : dest.left;
			double y = (corner & 2) ? dest.bottom : dest.top;
			transform.transformPoint(x, y);

			minX = corner == 0 ? x : MIN(minX, x);
			maxX = corner == 0 ? x : MAX(maxX, x);
			minY = corner == 0 ? y : MIN(minY, y);
			maxY = corner == 0 ? y : MAX(maxY, y);
		}

		area.left = CLIP<int>((int)floor(minX), -kLimit, kLimit);
		area.top = CLIP<int>((int)floor(minY), -kLimit, kLimit);
		area.right = area.left + CLIP<int>((int)ceil(maxX) - area.left, 1, kMaxSize);
		area.bottom = area.top + CLIP<int>((int)ceil(maxY) - area.top, 1, kMaxSize);
	}

	// Drawn with an alpha of its own, in place of the color that stands for
	// see-through in the image
	Graphics::ManagedSurface &out = graphic._drawSurface;
	out.create(area.width(), area.height(), g_nancy->_graphics->getTransparentPixelFormat());
	out.clear(0);
	graphic.setTransparent(false);

	byte transR, transG, transB;
	image.format.colorToRGB(g_nancy->_graphics->getTransColor(), transR, transG, transB);
	const bool hasAlpha = image.format.aBits() != 0;

	const uint modA = modColor >> 24;
	const uint modR = (modColor >> 16) & 0xFF;
	const uint modG = (modColor >> 8) & 0xFF;
	const uint modB = modColor & 0xFF;

	// A picture squashed flat, or one without a size, shows nothing
	const bool canDraw = isMovedOnly || (fabs(determinant) > 1e-9 && dest.width() != 0 && dest.height() != 0);
	const double scaleX = dest.width() != 0 ? (double)source.width() / dest.width() : 0.0;
	const double scaleY = dest.height() != 0 ? (double)source.height() / dest.height() : 0.0;

	for (int y = 0; canDraw && y < out.h; ++y) {
		for (int x = 0; x < out.w; ++x) {
			int sourceX = x;
			int sourceY = y;

			if (!isMovedOnly) {
				// Back from the viewport to where the pixel is in the picture
				const double movedX = area.left + x + 0.5 - offsetX;
				const double movedY = area.top + y + 0.5 - offsetY;
				const double placedX = (movedX * d - movedY * c) / determinant;
				const double placedY = (movedY * a - movedX * b) / determinant;

				sourceX = (int)floor((placedX - dest.left) * scaleX);
				sourceY = (int)floor((placedY - dest.top) * scaleY);
			}

			if (sourceX < 0 || sourceX >= source.width() || sourceY < 0 || sourceY >= source.height()) {
				continue;
			}

			byte alpha, red, green, blue;
			image.format.colorToARGB(image.getPixel(source.left + sourceX, source.top + sourceY), alpha, red, green, blue);
			if (red == transR && green == transG && blue == transB) {
				continue;
			}

			if (!hasAlpha) {
				alpha = 0xFF;
			}

			out.setPixel(x, y, out.format.ARGBToColor(alpha * modA / 255, red * modR / 255, green * modG / 255, blue * modB / 255));
		}
	}

	graphic.moveTo(area);
	graphic.setNeedsRedraw(true);
}

static Graphics::ManagedSurface *s_drawTarget = nullptr;

Graphics::ManagedSurface *getDrawTarget() {
	return s_drawTarget;
}

// Summary

void SummaryRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (!readField(L, tableIndex, "env", _environment)) {
		luaL_error(L, "No environment specified");
	}

	if (!readField(L, tableIndex, "bg", _backgroundName)) {
		luaL_error(L, "No background specified");
	}

	// Either whether themes keep playing, or the pause between them as well
	lua_getfield(L, tableIndex, "loopTheme");
	if (lua_type(L, -1) == LUA_TNUMBER) {
		_loopsTheme = true;
		_themePause = ABS(lua_tonumber(L, -1));
	} else if (!lua_isnil(L, -1)) {
		_loopsTheme = lua_toboolean(L, -1) != 0;
	}

	lua_pop(L, 1);

	// The background is behind everything else
	limitZ(kBackgroundScriptZ);
}

// The channel and the volume of the themes
static const uint16 kThemeChannel = 0;
static const uint16 kThemeVolume = 85;

// Starts one of the themes of the area, unless one is playing
void SummaryRecord::playTheme() {
	if (g_nancy->_sound->isSoundPlaying(kThemeChannel)) {
		return;
	}

	const ENVS *environments = GetEngineData(ENVS);
	if (!environments) {
		return;
	}

	for (const ENVS::Environment &environment : environments->environments) {
		if (!environment.code.equalsIgnoreCase(_environment) || environment.musicNames.empty()) {
			continue;
		}

		SoundDescription sound;
		sound.name = environment.musicNames[g_nancy->_randomSource->getRandomNumber(environment.musicNames.size() - 1)];
		sound.channelID = kThemeChannel;
		sound.numLoops = 1;
		sound.volume = kThemeVolume;

		g_nancy->_sound->loadSound(sound);
		g_nancy->_sound->playSound(sound);
		return;
	}
}

void SummaryRecord::updateTheme(double sceneTime) {
	// How often the scene looks whether the theme is over
	static const double kThemeCheckSeconds = 1.0;

	if (!_loopsTheme || sceneTime <= _lastThemeCheck + kThemeCheckSeconds) {
		return;
	}

	_lastThemeCheck = sceneTime;

	if (_themeEndTime == 0.0 && !g_nancy->_sound->isSoundPlaying(kThemeChannel)) {
		_themeEndTime = sceneTime;
	}

	if (_themeEndTime != 0.0 && _themeEndTime + _themePause <= sceneTime) {
		_themeEndTime = 0.0;
		playTheme();
	}
}

void SummaryRecord::runOnce() {
	playTheme();
}

void SummaryRecord::setBackground(UI::Viewport *background) {
	_background = background;
	_numFrames = background ? background->getFrameCount() : 0;
	_frame = getContext().getFrame();
}

void SummaryRecord::setFrame(uint frame) {
	_frame = frame;
	_lastTurnTime = _sceneTime;
	getContext().setFrame(frame);

	if (_background) {
		_background->setFrame(frame);
	}
}

SummaryRecord::Turn SummaryRecord::getTurnAt(const ScriptPoint &point, bool inViewport) const {
	// Only a background with several frames can be turned around in
	if (_numFrames <= 1 || !inViewport) {
		return kNoTurn;
	}

	const ScriptBootData *boot = GetEngineData(ScriptBootData);
	assert(boot);

	const Common::Point place = point.toPoint();
	if (boot->leftNodeHotspot.contains(place)) {
		return kTurnLeft;
	} else if (boot->rightNodeHotspot.contains(place)) {
		return kTurnRight;
	} else if (boot->uturnNodeHotspot.contains(place)) {
		return kTurnAround;
	}

	return kNoTurn;
}

void SummaryRecord::onMouseMove(const ScriptPoint &point, bool inViewport) {
	// The view turns by itself while the mouse stays on a side, after a first wait
	Turn turn = getTurnAt(point, inViewport);
	if (turn == kTurnAround) {
		turn = kNoTurn;
	}

	if (turn != kNoTurn && turn != _turn) {
		_lastTurnTime = _sceneTime;
	}

	_turn = turn;
}

bool SummaryRecord::getCursor(const ScriptPoint &point, bool inViewport, uint &cursorType, bool &isHot) {
	const ScriptBootData *boot = GetEngineData(ScriptBootData);
	assert(boot);

	int type = -1;
	switch (getTurnAt(point, inViewport)) {
	case kTurnLeft:
		type = g_nancy->_cursor->getCursorTypeByName(boot->leftNodeCursorName);
		break;
	case kTurnRight:
		type = g_nancy->_cursor->getCursorTypeByName(boot->rightNodeCursorName);
		break;
	case kTurnAround:
		type = g_nancy->_cursor->getCursorTypeByName(boot->uturnNodeCursorName);
		break;
	default:
		break;
	}

	if (type < 0) {
		return false;
	}

	cursorType = type;
	isHot = false;
	return true;
}

bool SummaryRecord::onMouseEvent(RecordEvent event, const ScriptPoint &point, bool inViewport) {
	switch (event) {
	case kEventRightDown:
		_isTurningFast = true;
		return false;
	case kEventRightUp:
		_isTurningFast = false;
		return false;
	case kEventLeftClick:
		break;
	default:
		return false;
	}

	switch (getTurnAt(point, inViewport)) {
	case kTurnLeft:
	case kTurnRight:
		// A click doesn't wait for the turn
		_lastTurnTime = _sceneTime - 0.5;
		return true;
	case kTurnAround:
		setFrame((_frame + _numFrames / 2) % _numFrames);
		return true;
	default:
		return false;
	}
}

void SummaryRecord::run(double sceneTime, double elapsedTime) {
	_sceneTime = sceneTime;
	updateTheme(sceneTime);

	if (_numFrames <= 1 || _turn == kNoTurn) {
		return;
	}

	if (_lastTurnTime + 0.5 < sceneTime || (_isTurningFast && _lastTurnTime + 0.2 < sceneTime)) {
		if (_turn == kTurnLeft) {
			setFrame((_frame + 1) % _numFrames);
		} else {
			setFrame((_frame + _numFrames - 1) % _numFrames);
		}
	}
}

bool SummaryRecord::getField(const Common::String &key) {
	if (key == "frame") {
		lua_pushinteger(getLua(), _frame);
		return true;
	}

	return false;
}

// NavLogic

void NavLogicRecord::readScene(int valueIndex) {
	lua_State *L = getLua();

	// Scenes that go by number are named after it
	if (lua_type(L, valueIndex) == LUA_TNUMBER) {
		_sceneName = Common::String::format("s%d", (int)lua_tointeger(L, valueIndex));
	} else if (lua_isnil(L, valueIndex)) {
		_sceneName.clear();
	} else {
		_sceneName = luaL_checkstring(L, valueIndex);
	}
}

void NavLogicRecord::readCursor(int valueIndex) {
	_hasCursor = testCursor(getLua(), valueIndex, _cursorType, _cursorIsHot);
}

void NavLogicRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "scene")) {
		readScene(-1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "cursor")) {
		readCursor(-1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "frame", _frame);
	readField(L, tableIndex, "stream", _streamName);

	if (pushField(L, tableIndex, "onScreen")) {
		_hotspot = checkRect(L, -1);
		lua_pop(L, 1);
	}

	// The flags to set, by name
	if (pushField(L, tableIndex, "flags")) {
		luaL_checktype(L, -1, LUA_TTABLE);
		int flags = lua_gettop(L);

		lua_pushnil(L);
		while (lua_next(L, flags)) {
			if (lua_type(L, -2) == LUA_TSTRING) {
				Flag flag;
				flag.name = lua_tostring(L, -2);
				flag.value = lua_toboolean(L, -1) != 0;
				_flags.push_back(flag);
			}

			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}
}

void NavLogicRecord::run(double sceneTime, double elapsedTime) {
	// Without a place to click, there is nothing to wait for
	if (_hotspot.isEmpty()) {
		trigger();
	}
}

bool NavLogicRecord::isOver(const ScriptPoint &point) const {
	return transformRect(_hotspot, getTransform()).contains(point.x, point.y);
}

bool NavLogicRecord::getCursor(const ScriptPoint &point, bool inViewport, uint &cursorType, bool &isHot) {
	if (!_hasCursor || !isOver(point)) {
		return false;
	}

	cursorType = _cursorType;
	isHot = _cursorIsHot;
	return true;
}

bool NavLogicRecord::onMouseEvent(RecordEvent event, const ScriptPoint &point, bool inViewport) {
	if (event != kEventLeftClick || !isOver(point)) {
		return false;
	}

	trigger();
	return true;
}

void NavLogicRecord::trigger() {
	for (const Flag &flag : _flags) {
		if (!setFlagByName(flag.name, flag.value)) {
			warning("Attempted to set flag \"%s\" but it does not exist.", flag.name.c_str());
		}
	}

	if (!_sceneName.empty()) {
		getContext().changeScene(_streamName, _sceneName, (uint16)_frame);
	}

	finish();
}

bool NavLogicRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "scene") {
		lua_pushstring(L, _sceneName.c_str());
	} else if (key == "stream") {
		lua_pushstring(L, _streamName.c_str());
	} else if (key == "frame") {
		lua_pushinteger(L, _frame);
	} else if (key == "onScreen") {
		pushRect(L, _hotspot);
	} else {
		return false;
	}

	return true;
}

bool NavLogicRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "scene") {
		readScene(3);
	} else if (key == "stream") {
		_streamName = luaL_checkstring(L, 3);
	} else if (key == "cursor") {
		readCursor(3);
	} else if (key == "frame") {
		_frame = luaL_checkint(L, 3);
	} else if (key == "onScreen") {
		_hotspot = checkRect(L, 3);
	} else {
		return false;
	}

	return true;
}

// Override

// What the engine calls in an override, all of which have to be functions
static const char *const kOverrideFunctionNames[] = {
	"OnReset", "OnActivation", "OnDeactivation", "OnDone", "OnMouseMove", "ChangeCursor",
	"OnLeftClick", "OnRightClick", "OnMouseScroll", "OnLeftDown", "OnLeftUp", "OnRightDown",
	"OnRightUp", "OnKey", "Run", "RunOnce", "Render", "OnSave", "ReceiveMessage"
};

static bool isOverrideFunctionName(const Common::String &name) {
	for (const char *functionName : kOverrideFunctionNames) {
		if (name == functionName) {
			return true;
		}
	}

	return false;
}

OverrideRecord::~OverrideRecord() {
	delete _canvas;
}

void OverrideRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	for (const char *functionName : kOverrideFunctionNames) {
		lua_getfield(L, tableIndex, functionName);
		if (!lua_isnil(L, -1) && !lua_isfunction(L, -1)) {
			luaL_error(L, "%s must be set to a function", functionName);
		}

		lua_pop(L, 1);
	}
}

bool OverrideRecord::hasMessagingName(const Common::String &name) {
	lua_State *L = getLua();

	pushStoredValue("messagingName");
	const bool isNamed = lua_type(L, -1) == LUA_TSTRING && name.equalsIgnoreCase(lua_tostring(L, -1));
	lua_pop(L, 1);
	return isNamed;
}

void OverrideRecord::onZChanged() {
	if (_canvas) {
		_canvas->setScriptZ(getZ(), getContext().getZOffset());
	}
}

// An override draws by having its "Render" function called every frame, which
// draws through the "Draw" object. Here that goes onto a picture as large as
// the viewport, which is started over every frame.
void OverrideRecord::updateGraphics() {
	lua_State *L = getLua();

	pushStoredValue("Render");
	const bool hasRender = lua_isfunction(L, -1);
	lua_pop(L, 1);

	if (!hasRender || !isActive()) {
		if (_canvas) {
			_canvas->setVisible(false);
		}

		return;
	}

	if (!_canvas) {
		const VIEW *viewport = GetEngineData(VIEW);
		assert(viewport);

		const Common::Rect bounds(viewport->screenPosition.width(), viewport->screenPosition.height());

		_canvas = new RecordGraphic();
		_canvas->init();
		_canvas->_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
		_canvas->moveTo(bounds);
		_canvas->setScriptZ(getZ(), getContext().getZOffset());
		_canvas->registerGraphics();
	}

	_canvas->_drawSurface.clear(0);

	s_drawTarget = &_canvas->_drawSurface;
	callFunction("Render");
	s_drawTarget = nullptr;

	_canvas->setVisible(true);
	_canvas->setNeedsRedraw(true);
}

// this:Done()
static int overrideDone(lua_State *L) {
	checkRecord(L, 1)->finish();
	return 0;
}

bool OverrideRecord::getField(const Common::String &key) {
	if (key == "Done") {
		lua_pushcfunction(getLua(), overrideDone);
		return true;
	}

	return false;
}

bool OverrideRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (isOverrideFunctionName(key) && !lua_isnil(L, 3) && !lua_isfunction(L, 3)) {
		luaL_error(L, "%s must be set to a function or to nil", key.c_str());
	}

	// Kept with everything else scripts store in the record
	return false;
}

// Overlay

void OverlayRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	Common::String imageName;
	if (!readField(L, tableIndex, "ovl", imageName)) {
		luaL_error(L, "OverlayAR must be created with a ovl parameter");
	}

	if (!g_nancy->_resource->loadImage(Common::Path(imageName), _image)) {
		luaL_error(L, "Could not load overlay %s", imageName.c_str());
	}

	// Both default to the whole image
	_source = ScriptRect(0, 0, _image.w, _image.h);
	if (pushField(L, tableIndex, "source")) {
		_source = checkRect(L, -1);
		lua_pop(L, 1);
	}

	_dest = _source;
	if (pushField(L, tableIndex, "onScreen")) {
		_dest = checkRect(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "modColor")) {
		_modColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	_graphic.init();
	_graphic.setVisible(false);
	applyRects();
	onZChanged();
	_graphic.registerGraphics();
}

void OverlayRecord::applyRects() {
	Common::Rect source = _source.toRect();
	source.clip(Common::Rect(_image.w, _image.h));

	// A script can leave the picture out until it knows what to show
	_hasPicture = !source.isEmpty();
	if (!_hasPicture) {
		return;
	}

	placePicture(_graphic, _image, source, _dest, getTransform(), _modColor);
}

void OverlayRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext().getZOffset());
}

void OverlayRecord::updateGraphics() {
	_graphic.setVisible(isActive() && _hasPicture);

	if (_hasAttachment) {
		_attachment.setVisible(_isAttachmentDrawn);
		_isAttachmentDrawn = false;
	}
}

// What a script draws by itself goes over everything else the scene shows
static const uint16 kAttachmentZ = 999;

// TODO: The picture is cut off at the edges of the viewport
void OverlayRecord::renderNow() {
	Common::Rect source = _source.toRect();
	source.clip(Common::Rect(_image.w, _image.h));
	if (source.isEmpty()) {
		return;
	}

	if (!_hasAttachment) {
		_hasAttachment = true;
		_attachment.init();
		_attachment.setVisible(false);
		_attachment.setZOrder(kAttachmentZ);
		_attachment.registerGraphics();
	}

	// The transform places the picture on the screen, and the records draw
	// in the viewport
	const VIEW *viewport = GetEngineData(VIEW);
	assert(viewport);

	ScriptMatrix transform = getTransform();
	transform.m[3][0] -= viewport->screenPosition.left;
	transform.m[3][1] -= viewport->screenPosition.top;

	placePicture(_attachment, _image, source, _dest, transform, _modColor);
	_isAttachmentDrawn = true;
}

bool OverlayRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "source") {
		pushRect(L, _source);
	} else if (key == "modColor") {
		pushColor(L, _modColor);
	} else {
		return false;
	}

	return true;
}

bool OverlayRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_dest = checkRect(L, 3);
	} else if (key == "source") {
		_source = checkRect(L, 3);
	} else if (key == "modColor") {
		_modColor = checkColor(L, 3);
	} else {
		return false;
	}

	applyRects();
	return true;
}

// AnimOverlay

void AnimOverlayRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	readField(L, tableIndex, "firstFrame", _firstFrame);
	readField(L, tableIndex, "lastFrame", _lastFrame);
	readField(L, tableIndex, "fps", _framesPerSecond);
	readField(L, tableIndex, "pauseOnLastFrame", _pausesOnLastFrame);
	readField(L, tableIndex, "visible", _isVisible);

	if (pushField(L, tableIndex, "modColor")) {
		_modColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	if (!pushField(L, tableIndex, "onScreen")) {
		luaL_error(L, "AnimOverlays must have a onScreen field");
	}

	_dest = checkRect(L, -1);
	lua_pop(L, 1);

	// Either whether to loop forever, or how many times to play
	if (pushField(L, tableIndex, "loop")) {
		if (lua_isboolean(L, -1)) {
			_numLoops = lua_toboolean(L, -1) ? 0 : 1;
		} else if (lua_isnumber(L, -1)) {
			_numLoops = (int)lua_tointeger(L, -1);
		} else {
			luaL_error(L, "loop must be either a boolean or a number");
		}

		lua_pop(L, 1);
	}

	if (!pushField(L, tableIndex, "sources")) {
		luaL_error(L, "AnimOverlays must have a sources field");
	}

	luaL_checktype(L, -1, LUA_TTABLE);
	for (int i = 1; ; ++i) {
		lua_rawgeti(L, -1, i);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}

		_sources.push_back(checkRect(L, -1));
		lua_pop(L, 1);
	}

	lua_pop(L, 1);

	if (_sources.empty()) {
		luaL_error(L, "Field sources must be a table with Rects in it");
	}

	Common::String imageName;
	if (!readField(L, tableIndex, "ovl", imageName)) {
		luaL_error(L, "AnimOverlays must have a ovl field");
	}

	if (!g_nancy->_resource->loadImage(Common::Path(imageName), _image)) {
		luaL_error(L, "Could not load overlay %s", imageName.c_str());
	}

	_graphic.init();
	_graphic.setVisible(false);
	setFrame(getFirstFrame());
	onZChanged();
	_graphic.registerGraphics();
}

void AnimOverlayRecord::setFrame(int frame) {
	_frame = CLIP<int>(frame, 1, _sources.size());
	applyFrame();
}

void AnimOverlayRecord::applyFrame() {
	Common::Rect source = _sources[_frame - 1].toRect();
	source.clip(Common::Rect(_image.w, _image.h));

	_hasPicture = !source.isEmpty();
	if (!_hasPicture) {
		return;
	}

	placePicture(_graphic, _image, source, _dest, getTransform(), _modColor);
}

// Starts over from the first frame
void AnimOverlayRecord::beginLoop() {
	_progress = 0.0;
	setFrame(getFirstFrame());

	const int numSteps = getLastFrame() - getFirstFrame();
	_endTime = getContext().getTime() + (_framesPerSecond > 0.0 ? numSteps / _framesPerSecond : 0.0);
}

void AnimOverlayRecord::runOnce() {
	_numLoopsDone = 0;
	beginLoop();
}

void AnimOverlayRecord::run(double sceneTime, double elapsedTime) {
	if (_isPaused) {
		return;
	}

	const int numSteps = getLastFrame() - getFirstFrame();
	if (numSteps > 0) {
		_progress = CLIP(_progress + _framesPerSecond * elapsedTime / numSteps, 0.0, 1.0);
		setFrame(getFirstFrame() + (int)floor(_progress * numSteps));
	}

	if (_endTime >= sceneTime) {
		return;
	}

	++_numLoopsDone;
	if (_numLoops != 0 && _numLoopsDone >= _numLoops) {
		if (_pausesOnLastFrame) {
			_isPaused = true;
		} else {
			finish();
		}
	} else {
		beginLoop();
	}
}

bool AnimOverlayRecord::prepareReset() {
	setFrame(getFirstFrame());
	_isPaused = false;
	return true;
}

void AnimOverlayRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext().getZOffset());
}

void AnimOverlayRecord::updateGraphics() {
	_graphic.setVisible(isActive() && _isVisible && _hasPicture);
}

// anim:Play() and anim:Pause(), for the records that play something
static int animOverlayPlay(lua_State *L) {
	AnimOverlayRecord *record = dynamic_cast<AnimOverlayRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(false);
	}

	return 0;
}

static int animOverlayPause(lua_State *L) {
	AnimOverlayRecord *record = dynamic_cast<AnimOverlayRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(true);
	}

	return 0;
}

bool AnimOverlayRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "Play") {
		lua_pushcfunction(L, animOverlayPlay);
	} else if (key == "Pause") {
		lua_pushcfunction(L, animOverlayPause);
	} else if (key == "paused") {
		lua_pushboolean(L, _isPaused);
	} else if (key == "pauseOnLastFrame") {
		lua_pushboolean(L, _pausesOnLastFrame);
	} else if (key == "visible") {
		lua_pushboolean(L, _isVisible);
	} else if (key == "frame") {
		lua_pushinteger(L, _frame);
	} else if (key == "endTime") {
		lua_pushnumber(L, _endTime);
	} else if (key == "firstFrame") {
		lua_pushinteger(L, getFirstFrame());
	} else if (key == "lastFrame") {
		lua_pushinteger(L, getLastFrame());
	} else if (key == "frames") {
		lua_pushinteger(L, _sources.size());
	} else if (key == "fps") {
		lua_pushnumber(L, _framesPerSecond);
	} else {
		return false;
	}

	return true;
}

bool AnimOverlayRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_dest = checkRect(L, 3);
		applyFrame();
	} else if (key == "modColor") {
		_modColor = checkColor(L, 3);
		applyFrame();
	} else if (key == "pauseOnLastFrame") {
		_pausesOnLastFrame = lua_toboolean(L, 3) != 0;
	} else if (key == "visible") {
		_isVisible = lua_toboolean(L, 3) != 0;
	} else if (key == "frame") {
		setFrame(luaL_checkint(L, 3));
	} else if (key == "fps") {
		_framesPerSecond = luaL_checknumber(L, 3);
	} else {
		return false;
	}

	return true;
}

// Sound

void SoundRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "channel")) {
		_channel = checkSoundChannel(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "volume", _volume);
	readField(L, tableIndex, "loop", _isLooping);
	readField(L, tableIndex, "useCC", _usesCaption);

	if (pushField(L, tableIndex, "sounds")) {
		readStringList(L, -1, _names);
		lua_pop(L, 1);
	}

	if (_names.empty()) {
		luaL_error(L, "sounds must have one or more fields in its table");
	}
}

// A sound that is playing while a movie gets skipped is cut short
void SoundRecord::onSkip() {
	if (isActive() && hasStarted() && !_isLooping) {
		g_nancy->_sound->stopSound(_sound);
		_endTime = 0.0;
	}
}

void SoundRecord::runOnce() {
	// A sound that would start in a movie that is being skipped is over at once,
	// unless it is one that keeps playing afterwards
	if (getContext().isSkipping() && !_isLooping) {
		_sound = SoundDescription();
		_isPlaying = false;
		_wasPreempted = false;
		_caption.clear();
		_endTime = getContext().getTime();
		return;
	}

	_sound = SoundDescription();
	_sound.name = _names[g_nancy->_randomSource->getRandomNumber(_names.size() - 1)];
	_sound.channelID = _channel;
	_sound.numLoops = _isLooping ? 0 : 1;
	_sound.volume = (uint16)CLIP<int>((int)(_volume * 100.0), 0, 100);

	g_nancy->_sound->loadSound(_sound);
	g_nancy->_sound->playSound(_sound);

	_isPlaying = g_nancy->_sound->isSoundPlaying(_sound);
	_wasPreempted = false;

	// The record is done once the sound has played through, whether it loops or not
	const double length = _isPlaying ? g_nancy->_sound->getLength(_sound).msecs() / 1000.0 : 0.0;
	_endTime = getContext().getTime() + length;

	// What the sound says is kept under its name
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	_caption.clear();
	if (_isPlaying && _usesCaption && ui && findText(_sound.name, _caption)) {
		ui->setCaption(_caption);
	}
}

void SoundRecord::run(double sceneTime, double elapsedTime) {
	// Something else taking over the channel ends the record early
	if (_isPlaying && !_isLooping && !g_nancy->_sound->isSoundPlaying(_sound) && sceneTime < _endTime) {
		_wasPreempted = true;
	}

	if (_endTime <= sceneTime || _wasPreempted) {
		// The caption goes with the sound, unless another one has replaced it
		UI::NDUI::Manager *ui = NancySceneState.getNDUI();
		if (!_caption.empty() && ui && ui->getCaption() == _caption) {
			ui->setCaption(Common::String());
		}

		_caption.clear();
		finish();
	}
}

bool SoundRecord::prepareReset() {
	_endTime = 0.0;
	_wasPreempted = false;
	_isPlaying = false;
	return true;
}

bool SoundRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "channel") {
		pushSoundChannel(L, _channel);
	} else if (key == "volume") {
		lua_pushnumber(L, _volume);
	} else if (key == "loop") {
		lua_pushboolean(L, _isLooping);
	} else if (key == "endTime") {
		lua_pushnumber(L, _endTime);
	} else if (key == "preempted") {
		lua_pushboolean(L, _wasPreempted);
	} else {
		return false;
	}

	return true;
}

// Text

// How quickly a text fades in; a lower rate is faster
static const float kTextBlendRate = 0.7f;

void TextRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "onScreen")) {
		_dest = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "yOffset", _yOffset);
	readField(L, tableIndex, "yVelocity", _yVelocity);
	readField(L, tableIndex, "useAutotext", _usesAutotext);
	readField(L, tableIndex, "revealAll", _revealsAll);

	_initialDest = _dest;
	_initialYOffset = _yOffset;
	_initialYVelocity = _yVelocity;

	int fontNumber = 0;
	int colorNumber = 0;
	bool isBold = false;
	readField(L, tableIndex, "fontNum", fontNumber);
	readField(L, tableIndex, "fontColor", colorNumber);
	readField(L, tableIndex, "bold", isBold);

	// Fonts and their colors go by number
	Common::String fontName;
	uint32 color = 0xFFFFFFFF;
	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	if (boot) {
		for (const ScriptBootData::SystemFont &font : boot->fonts) {
			if (font.id == fontNumber) {
				fontName = font.name;
			}
		}

		for (const ScriptBootData::FontColor &fontColor : boot->fontColors) {
			if (fontColor.id == colorNumber) {
				color = fontColor.normal;
			}
		}
	}

	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeTextElement(L, tableIndex, fontName, isBold, color, data);
	Common::MemoryReadStream stream(data.getData(), data.size());
	_element.read(stream);

	Common::String text;
	if (!readField(L, tableIndex, "text", text)) {
		luaL_error(L, "Text AR requires field %s", "text");
	}

	setText(text);

	_graphic.init();
	_graphic.setVisible(false);
	applyDest();
	onZChanged();
	_graphic.registerGraphics();
}

void TextRecord::applyDest() {
	const Common::Rect dest = _dest.toRect();
	_graphic._drawSurface.create(MAX<int>(dest.width(), 1), MAX<int>(dest.height(), 1), g_nancy->_graphics->getTransparentPixelFormat());
	_graphic.moveTo(dest);
	_isDirty = true;
}

void TextRecord::setText(const Common::String &text) {
	_text = text;
	if (_usesAutotext && !findText(text, _text)) {
		_text = "Missing text: " + text;
	}

	_numRevealed = 0;
	_shownText = _revealsAll ? _text : Common::String();
	_isDirty = true;
}

bool TextRecord::revealNext() {
	if (_revealsAll || _numRevealed >= _text.size()) {
		return false;
	}

	// Markup is never shown in part
	uint end = _numRevealed;
	bool isInTag = false;
	while (end < _text.size()) {
		if (isInTag) {
			isInTag = _text[end] != '>';
		} else if (_text[end] == '<') {
			isInTag = true;
		} else {
			break;
		}

		++end;
	}

	end = MIN<uint>(end + 1, _text.size());
	_shownText += _text.substr(_numRevealed, end - _numRevealed);
	_numRevealed = end;
	_isDirty = true;
	return true;
}

void TextRecord::run(double sceneTime, double elapsedTime) {
	if (_element.blend(UI::NDUI::kStateNormal, (float)elapsedTime, kTextBlendRate)) {
		_isDirty = true;
	}

	if (_yVelocity == 0.0) {
		return;
	}

	_yOffset += _yVelocity * elapsedTime;
	_isDirty = true;

	// A text that moves is done once it has left its place
	const Common::Rect dest = _dest.toRect();
	const int textHeight = _element.measureTextHeight(_shownText, dest.width());
	if ((_yOffset > textHeight && _yVelocity > 0.0) || (_yOffset < -dest.height() && _yVelocity < 0.0)) {
		finish();
	}
}

void TextRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext().getZOffset());
}

void TextRecord::updateGraphics() {
	_graphic.setVisible(isActive());

	if (_isDirty) {
		_isDirty = false;

		Common::Rect area(_graphic._drawSurface.w, _graphic._drawSurface.h);
		area.translate(0, -(int)_yOffset);

		_graphic._drawSurface.clear(0);
		_element.drawText(_graphic._drawSurface, _shownText, area, false);
		_graphic.setNeedsRedraw(true);
	}
}

bool TextRecord::prepareReset() {
	_dest = _initialDest;
	_yOffset = _initialYOffset;
	_yVelocity = _initialYVelocity;
	applyDest();
	return true;
}

// text:RevealNext()
static int textRevealNext(lua_State *L) {
	TextRecord *text = dynamic_cast<TextRecord *>(checkRecord(L, 1));
	if (!text) {
		return luaL_typerror(L, 1, "ND.TextAR");
	}

	lua_pushboolean(L, text->revealNext());
	return 1;
}

bool TextRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "yOffset") {
		lua_pushnumber(L, _yOffset);
	} else if (key == "yVelocity") {
		lua_pushnumber(L, _yVelocity);
	} else if (key == "RevealNext") {
		lua_pushcfunction(L, textRevealNext);
	} else {
		return false;
	}

	return true;
}

bool TextRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_dest = checkRect(L, 3);
		applyDest();
	} else if (key == "text") {
		setText(luaL_checkstring(L, 3));
	} else if (key == "yOffset") {
		_yOffset = luaL_checknumber(L, 3);
		_isDirty = true;
	} else if (key == "yVelocity") {
		_yVelocity = luaL_checknumber(L, 3);
	} else {
		return false;
	}

	return true;
}

// Movie

void MovieRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (!readField(L, tableIndex, "movie", _name)) {
		luaL_error(L, "MovieAR must be created with a movie parameter");
	}

	if (!pushField(L, tableIndex, "onScreen")) {
		luaL_error(L, "MovieAR must be created with an 'onScreen' parameter");
	}

	_dest = checkRect(L, -1);
	lua_pop(L, 1);

	if (!_player.loadFile(Common::Path(_name))) {
		luaL_error(L, "Could not load movie %s", _name.c_str());
	}

	_firstFrame = 0;
	_lastFrame = _player.getFrameCount() - 1;
	readField(L, tableIndex, "firstFrame", _firstFrame);
	readField(L, tableIndex, "lastFrame", _lastFrame);
	readField(L, tableIndex, "pauseOnLastFrame", _pausesOnLastFrame);
	readField(L, tableIndex, "visible", _isVisible);

	_graphic.init();
	_graphic._drawSurface.create(_player.getWidth(), _player.getHeight(), g_nancy->_graphics->getScreenPixelFormat());
	_graphic.setVisible(false);
	applyPosition();
	onZChanged();
	_graphic.registerGraphics();

	_player.goToFrame(_firstFrame);
	drawFrame();
}

void MovieRecord::applyPosition() {
	_graphic.moveTo(getTransformedDest(_dest, getTransform(), _player.getWidth(), _player.getHeight()));
}

void MovieRecord::drawFrame() {
	// A frame with see-through parts would let the one before it show
	_graphic._drawSurface.clear(0);
	_player.drawFrame(_graphic._drawSurface, Common::Point(0, 0));
	_graphic.setNeedsRedraw(true);
}

void MovieRecord::runOnce() {
	_player.playRange(_firstFrame, _lastFrame);
	drawFrame();
}

// A movie that plays through and ends
bool MovieRecord::isSkippable() const {
	return isActive() && hasStarted() && !_isPaused && !_pausesOnLastFrame;
}

void MovieRecord::run(double sceneTime, double elapsedTime) {
	// A movie that is being skipped moves on by one frame every time it runs
	if (getContext().isSkipping() && isSkippable()) {
		const int frame = _player.getCurrentFrame();
		bool hasEnded = frame == _lastFrame;

		if (!hasEnded && _lastFrame > frame) {
			hasEnded = _player.decodeNextFrame() == nullptr;
		} else if (!hasEnded) {
			_player.goToFrame(frame - 1);
		}

		if (hasEnded) {
			_player.goToFrame(_lastFrame);
			drawFrame();
			_endTime = sceneTime;
			finish();
		}

		return;
	}

	if (!_isPaused && _player.update()) {
		drawFrame();
	}

	if (!_player.isRangePlaying() && !_pausesOnLastFrame) {
		_endTime = sceneTime;
		finish();
	}
}

bool MovieRecord::prepareReset() {
	_endTime = 0.0;
	_isPaused = false;
	_player.goToFrame(_firstFrame);
	drawFrame();
	return true;
}

void MovieRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext().getZOffset());
}

void MovieRecord::updateGraphics() {
	_graphic.setVisible(isActive() && _isVisible);
}

// movie:Play() and movie:Pause()
static int moviePlay(lua_State *L) {
	MovieRecord *record = dynamic_cast<MovieRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(false);
	}

	return 0;
}

static int moviePause(lua_State *L) {
	MovieRecord *record = dynamic_cast<MovieRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(true);
	}

	return 0;
}

bool MovieRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "movie") {
		lua_pushstring(L, _name.c_str());
	} else if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "Play") {
		lua_pushcfunction(L, moviePlay);
	} else if (key == "Pause") {
		lua_pushcfunction(L, moviePause);
	} else if (key == "paused") {
		lua_pushboolean(L, _isPaused || !_player.isRangePlaying());
	} else if (key == "pauseOnLastFrame") {
		lua_pushboolean(L, _pausesOnLastFrame);
	} else if (key == "visible") {
		lua_pushboolean(L, _isVisible);
	} else if (key == "frame") {
		lua_pushinteger(L, _player.getCurrentFrame());
	} else if (key == "endTime") {
		lua_pushnumber(L, _endTime);
	} else if (key == "firstFrame") {
		lua_pushinteger(L, _firstFrame);
	} else if (key == "lastFrame") {
		lua_pushinteger(L, _lastFrame);
	} else if (key == "frames") {
		lua_pushinteger(L, _player.getFrameCount());
	} else {
		return false;
	}

	return true;
}

bool MovieRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_dest = checkRect(L, 3);
		applyPosition();
	} else if (key == "pauseOnLastFrame") {
		_pausesOnLastFrame = lua_toboolean(L, 3) != 0;
	} else if (key == "visible") {
		_isVisible = lua_toboolean(L, 3) != 0;
	} else if (key == "frame") {
		_player.goToFrame(luaL_checkint(L, 3));
		drawFrame();
	} else {
		return false;
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
