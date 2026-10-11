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

#include "common/textconsole.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/ui/viewport.h"

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
	case kRecordOverride:
		return new OverrideRecord(context);
	default:
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

} // End of namespace Script
} // End of namespace Nancy
