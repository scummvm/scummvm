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

#include "common/random.h"
#include "common/textconsole.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/script/api.h"
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
	default:
		// TODO: The Text and Particle records
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
// TODO: Only the place is taken from the transform. Draw what it rotates and
// scales as well.
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
	"OnRightUp", "OnKey", "Run", "RunOnce", "Render", "OnSave"
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

	_graphic._drawSurface.create(_image, source);
	_graphic.setTransparent(true);
	_graphic.moveTo(getTransformedDest(_dest, getTransform(), source.width(), source.height()));
	_graphic.setNeedsRedraw(true);
}

void OverlayRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext().getZOffset());
}

void OverlayRecord::updateGraphics() {
	_graphic.setVisible(isActive() && _hasPicture);
}

bool OverlayRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "source") {
		pushRect(L, _source);
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

	_graphic._drawSurface.create(_image, source);
	_graphic.setTransparent(true);
	_graphic.moveTo(getTransformedDest(_dest, getTransform(), source.width(), source.height()));
	_graphic.setNeedsRedraw(true);
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

void SoundRecord::runOnce() {
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

void MovieRecord::run(double sceneTime, double elapsedTime) {
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
