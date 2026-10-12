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

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"

namespace Nancy {
namespace Script {

// Set in the metatable of every kind of record
static const char *const kRecordMarker = "__record";

Record::Record(SceneContext &context, const char *typeName) :
		_context(context),
		_typeName(typeName) {}

Record::~Record() {}

lua_State *Record::getLua() const {
	return _context.getLua();
}

// What scripts store in a record is kept in a table of the registry
void Record::pushStorage() {
	lua_State *L = getLua();

	lua_pushlightuserdata(L, this);
	lua_rawget(L, LUA_REGISTRYINDEX);
	if (lua_istable(L, -1)) {
		return;
	}

	lua_pop(L, 1);
	lua_newtable(L);
	lua_pushlightuserdata(L, this);
	lua_pushvalue(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

void Record::pushStoredValue(const char *name) {
	lua_State *L = getLua();

	pushStorage();
	lua_getfield(L, -1, name);
	lua_remove(L, -2);
}

void Record::storeValue(const char *name) {
	lua_State *L = getLua();

	pushStorage();
	lua_insert(L, -2);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

void Record::create(int tableIndex) {
	lua_State *L = getLua();

	// Everything in the table that has a name is kept, whether the record
	// knows the name or not
	lua_pushnil(L);
	while (lua_next(L, tableIndex)) {
		if (lua_type(L, -2) == LUA_TSTRING) {
			lua_pushvalue(L, -1);
			storeValue(lua_tostring(L, -3));
		}

		lua_pop(L, 1);
	}

	readField(L, tableIndex, "z", _z);

	lua_getfield(L, tableIndex, "active");
	if (lua_isboolean(L, -1)) {
		_isActive = lua_toboolean(L, -1) != 0;
	} else if (lua_isfunction(L, -1)) {
		_hasActiveFunction = true;
	}

	lua_pop(L, 1);

	if (hasTransform() && pushField(L, tableIndex, "transform")) {
		_transform = checkMatrix(L, -1);
		lua_pop(L, 1);
	}

	readTable(tableIndex);
}

void Record::push() {
	Record **object = (Record **)pushUserData(getLua(), sizeof(Record *), _typeName);
	*object = this;
}

void Record::update(double sceneTime, double elapsedTime) {
	if (_isDone) {
		return;
	}

	lua_State *L = getLua();

	if (_hasActiveFunction) {
		pushStoredValue("active");
		push();
		if (LuaState::call(L, 1, 1, "Error in AR's active function")) {
			_isActive = lua_toboolean(L, -1) != 0;
			lua_pop(L, 1);
		}
	}

	if (_isActive && !_wasActive) {
		onActivation();
	} else if (!_isActive && _wasActive) {
		onDeactivation();
	}

	_wasActive = _isActive;

	if (!_isActive) {
		return;
	}

	_hasStarted = true;

	if (!_hasRunOnce) {
		_hasRunOnce = true;
		runOnce();
		callFunction("RunOnce");
	}

	if (isScripted()) {
		lua_pushnumber(L, sceneTime);
		lua_pushnumber(L, elapsedTime);
		callFunction("Run", 2);
	} else {
		run(sceneTime, elapsedTime);
	}
}

void Record::hold() {
	_isActive = false;
	_hasActiveFunction = false;
}

void Record::release() {
	_isActive = true;
	_hasActiveFunction = false;
	update(0.0, 0.0);
}

void Record::render() {
	if (isActive()) {
		renderNow();
	}
}

void Record::limitZ(double z) {
	if (_z > z) {
		_z = z;
		onZChanged();
	}
}

void Record::finish() {
	_isDone = true;
	callFunction("OnDone");
}

bool Record::reset() {
	if (!prepareReset()) {
		return false;
	}

	_hasRunOnce = false;
	_isDone = false;
	_hasStarted = false;
	callFunction("OnReset");
	return true;
}

bool Record::callFunction(const char *name, int numArgs, int numResults) {
	lua_State *L = getLua();

	pushStoredValue(name);
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, numArgs + 1);
		return false;
	}

	// The function and the record go below the arguments
	push();
	if (numArgs > 0) {
		lua_insert(L, -2 - numArgs);
		lua_insert(L, -2 - numArgs);
	}

	const Common::String what = Common::String::format("Error running override '%s'", name);
	return LuaState::call(L, numArgs + 1, numResults, what.c_str());
}

int Record::getScriptField() {
	lua_State *L = getLua();
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "active") {
		lua_pushboolean(L, isActive());
		return 1;
	} else if (key == "started") {
		lua_pushboolean(L, _hasStarted);
		return 1;
	} else if (key == "done") {
		lua_pushboolean(L, _isDone);
		return 1;
	} else if (key == "z") {
		lua_pushnumber(L, _z);
		return 1;
	} else if (key == "transform" && hasTransform()) {
		pushMatrix(L, _transform);
		return 1;
	}

	// The functions every record has
	indexFunctions(L);
	if (!lua_isnil(L, -1)) {
		return 1;
	}

	lua_pop(L, 1);

	if (getField(key)) {
		return 1;
	}

	pushStoredValue(key.c_str());
	return 1;
}

int Record::setScriptField() {
	lua_State *L = getLua();
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "active") {
		if (lua_isfunction(L, 3)) {
			lua_pushvalue(L, 3);
			storeValue("active");
			_hasActiveFunction = true;
		} else {
			_isActive = lua_toboolean(L, 3) != 0;
			_hasActiveFunction = false;
		}
	} else if (key == "z") {
		_z = luaL_checknumber(L, 3);
		onZChanged();
	} else if (key == "transform" && hasTransform()) {
		_transform = checkMatrix(L, 3);
		onTransformChanged();
	} else if (!setField(key)) {
		lua_pushvalue(L, 3);
		storeValue(key.c_str());
	}

	return 0;
}

// The functions scripts can give a record for what the player does
// TODO: OnMouseScroll
static const char *const kEventFunctionNames[kNumRecordEvents] = {
	"OnLeftClick", "OnRightClick", "OnLeftDown", "OnLeftUp", "OnRightDown", "OnRightUp",
	"OnKeyDown", "OnKey", "OnKeyUp"
};

void Record::sendMouseMove(const ScriptPoint &point, bool inViewport) {
	if (!isActive()) {
		return;
	}

	_hasStarted = true;

	if (isScripted()) {
		lua_State *L = getLua();
		pushPoint(L, point);
		lua_pushboolean(L, inViewport);
		callFunction("OnMouseMove", 2);
	} else {
		onMouseMove(point, inViewport);
	}
}

bool Record::sendChangeCursor(const ScriptPoint &point, bool inViewport, uint &cursorType, bool &isHot) {
	if (!isActive()) {
		return false;
	}

	_hasStarted = true;

	if (!isScripted()) {
		return getCursor(point, inViewport, cursorType, isHot);
	}

	// The function returns the cursor it wants, if any
	lua_State *L = getLua();
	pushPoint(L, point);
	lua_pushboolean(L, inViewport);
	if (!callFunction("ChangeCursor", 2, 1)) {
		return false;
	}

	const bool hasCursor = testCursor(L, -1, cursorType, isHot);
	lua_pop(L, 1);
	return hasCursor;
}

bool Record::sendMouseEvent(RecordEvent event, const ScriptPoint &point, bool inViewport) {
	if (!isActive()) {
		return false;
	}

	_hasStarted = true;

	if (!isScripted()) {
		return onMouseEvent(event, point, inViewport);
	}

	lua_State *L = getLua();
	pushPoint(L, point);
	lua_pushboolean(L, inViewport);
	if (!callFunction(kEventFunctionNames[event], 2, 1)) {
		return false;
	}

	const bool isHandled = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);
	return isHandled;
}

bool Record::sendKeyEvent(RecordEvent event, Common::KeyCode key) {
	// Only the records made of script functions care about the keyboard
	if (!isActive() || !isScripted()) {
		return false;
	}

	_hasStarted = true;

	lua_State *L = getLua();
	pushKey(L, key);
	if (!callFunction(kEventFunctionNames[event], 1, 1)) {
		return false;
	}

	const bool isHandled = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);
	return isHandled;
}

Record *checkRecord(lua_State *L, int index) {
	Record **object = (Record **)lua_touserdata(L, index);
	if (object && lua_getmetatable(L, index)) {
		lua_getfield(L, -1, kRecordMarker);
		const bool isRecord = lua_toboolean(L, -1) != 0;
		lua_pop(L, 2);

		if (isRecord) {
			return *object;
		}
	}

	luaL_typerror(L, index, "AR");
	return nullptr;
}

static int recordIndex(lua_State *L) {
	return checkRecord(L, 1)->getScriptField();
}

static int recordNewIndex(lua_State *L) {
	return checkRecord(L, 1)->setScriptField();
}

// Every access makes a new object for the same record
static int recordEquals(lua_State *L) {
	lua_pushboolean(L, checkRecord(L, 1) == checkRecord(L, 2));
	return 1;
}

static int recordReset(lua_State *L) {
	if (!checkRecord(L, 1)->reset()) {
		return luaL_error(L, "Cannot reset AR");
	}

	return 0;
}

// ar:Render()
static int recordRender(lua_State *L) {
	checkRecord(L, 1)->render();
	return 0;
}

// ar:Run(sceneTime, elapsedTime) runs a whole frame of the record
static int recordRun(lua_State *L) {
	Record *record = checkRecord(L, 1);
	const double sceneTime = luaL_checknumber(L, 2);
	const double elapsedTime = luaL_checknumber(L, 3);
	record->update(sceneTime, elapsedTime);
	return 0;
}

// ar:OnMouseMove(point, inViewport)
static int recordOnMouseMove(lua_State *L) {
	Record *record = checkRecord(L, 1);

	ScriptPoint point;
	if (!readPoint(L, 2, point)) {
		return luaL_argerror(L, 2, "Single parameter must be a Point or a table with x and y fields");
	}

	record->sendMouseMove(point, lua_toboolean(L, 3) != 0);
	return 0;
}

// ar:ChangeCursor(point, inViewport), which returns the cursor the record wants
static int recordChangeCursor(lua_State *L) {
	Record *record = checkRecord(L, 1);

	ScriptPoint point;
	if (!readPoint(L, 2, point)) {
		return luaL_argerror(L, 2, "Single parameter must be a Point or a table with x and y fields");
	}

	uint cursorType = 0;
	bool isHot = false;
	if (!record->sendChangeCursor(point, lua_toboolean(L, 3) != 0, cursorType, isHot)) {
		return 0;
	}

	pushCursor(L, cursorType, isHot);
	return 1;
}

// ar:OnLeftClick(point, inViewport) and the like; the event is kept with the function
static int recordOnMouseEvent(lua_State *L) {
	const RecordEvent event = (RecordEvent)lua_tointeger(L, lua_upvalueindex(1));
	Record *record = checkRecord(L, 1);

	ScriptPoint point;
	if (!readPoint(L, 2, point)) {
		return luaL_argerror(L, 2, "Single parameter must be a Point or a table with x and y fields");
	}

	lua_pushboolean(L, record->sendMouseEvent(event, point, lua_toboolean(L, 3) != 0));
	return 1;
}

// ar:OnKeyDown(key) and the like
static int recordOnKeyEvent(lua_State *L) {
	const RecordEvent event = (RecordEvent)lua_tointeger(L, lua_upvalueindex(1));
	Record *record = checkRecord(L, 1);

	lua_pushboolean(L, record->sendKeyEvent(event, checkKey(L, 2)));
	return 1;
}

static const luaL_Reg kRecordFunctions[] = {
	{ "__index", recordIndex },
	{ "__newindex", recordNewIndex },
	{ "__eq", recordEquals },
	{ "Reset", recordReset },
	{ "Run", recordRun },
	{ "Render", recordRender },
	{ "OnMouseMove", recordOnMouseMove },
	{ "ChangeCursor", recordChangeCursor },
	{ nullptr, nullptr }
};

// AR:<kind>(table)
static int createRecord(lua_State *L) {
	const RecordKind kind = (RecordKind)lua_tointeger(L, lua_upvalueindex(1));
	const char *kindName = lua_tostring(L, lua_upvalueindex(2));

	SceneContext *context = LuaState::getContext(L);
	if (!context) {
		return luaL_error(L, "AR:%s can only be used by the script of a scene", kindName);
	}

	// Only the table is left, at index 1
	lua_remove(L, 1);
	lua_settop(L, 1);
	if (!lua_istable(L, 1)) {
		return luaL_argerror(L, 1, "Expected a table as the parameter");
	}

	if (kind != kRecordSummary && !context->hasRecords()) {
		return luaL_error(L, "Summary AR must be the first AR in the scene");
	}

	Record *record = makeRecord(kind, *context);
	if (!record) {
		return luaL_error(L, "AR:%s is not supported yet", kindName);
	}

	context->addRecord(record);
	record->create(1);
	record->push();
	return 1;
}

void registerRecordAPI(lua_State *L) {
	for (uint i = 0; i < kNumRecordKinds; ++i) {
		const char *typeName = getRecordTypeName((RecordKind)i);
		if (luaL_newmetatable(L, typeName)) {
			luaL_register(L, nullptr, kRecordFunctions);

			lua_pushstring(L, typeName);
			lua_setfield(L, -2, "__metatable");

			lua_pushboolean(L, true);
			lua_setfield(L, -2, kRecordMarker);

			for (int event = 0; event < kNumRecordEvents; ++event) {
				lua_pushinteger(L, event);
				lua_pushcclosure(L, event >= kEventKeyDown ? recordOnKeyEvent : recordOnMouseEvent, 1);
				lua_setfield(L, -2, kEventFunctionNames[event]);
			}
		}

		lua_pop(L, 1);
	}

	static const luaL_Reg kFactoryFunctions[] = {
		{ "__index", indexFunctions },
		{ nullptr, nullptr }
	};

	registerGlobal(L, "AR", "ND.AR", kFactoryFunctions);

	// One function for every kind, which knows the kind it makes
	luaL_getmetatable(L, "ND.AR");
	for (uint i = 0; i < kNumRecordKinds; ++i) {
		lua_pushinteger(L, i);
		lua_pushstring(L, getRecordKindName((RecordKind)i));
		lua_pushcclosure(L, createRecord, 2);
		lua_setfield(L, -2, getRecordKindName((RecordKind)i));
	}

	lua_pop(L, 1);
}

} // End of namespace Script
} // End of namespace Nancy
