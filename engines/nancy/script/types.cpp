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

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

static const char *const kRectType = "ND.Rect";
static const char *const kRectHelperType = "ND.RectHelper";
static const char *const kPointType = "ND.Point";
static const char *const kPointHelperType = "ND.PointHelper";
static const char *const kColorType = "ND.Color";
static const char *const kColorHelperType = "ND.ColorHelper";

Common::Rect ScriptRect::toRect() const {
	if (isEmpty()) {
		return Common::Rect();
	}

	return Common::Rect(CLIP<int32>(left, -32768, 32767), CLIP<int32>(top, -32768, 32767),
						CLIP<int32>(right, -32768, 32767), CLIP<int32>(bottom, -32768, 32767));
}

// Rect

void pushRect(lua_State *L, const ScriptRect &rect) {
	pushValue(L, kRectType, rect);
}

ScriptRect *testRect(lua_State *L, int index) {
	return (ScriptRect *)testUserData(L, index, kRectType);
}

ScriptRect &checkRect(lua_State *L, int index) {
	return *(ScriptRect *)checkUserData(L, index, kRectType);
}

// The functions that take a place accept either a point or two numbers
static ScriptPoint checkPointArguments(lua_State *L, int index) {
	ScriptPoint point;

	if (lua_gettop(L) == index) {
		if (!readPoint(L, index, point)) {
			luaL_argerror(L, index, "Single parameter must be a Point or a table with x and y fields");
		}
	} else {
		point.x = luaL_checkint(L, index);
		point.y = luaL_checkint(L, index + 1);
	}

	return point;
}

static int rectNew(lua_State *L) {
	ScriptRect rect;
	rect.left = luaL_checkint(L, 2);
	rect.top = luaL_checkint(L, 3);
	rect.right = luaL_checkint(L, 4);
	rect.bottom = luaL_checkint(L, 5);
	pushRect(L, rect);
	return 1;
}

static int rectNewLTWH(lua_State *L) {
	ScriptRect rect;
	rect.left = luaL_checkint(L, 2);
	rect.top = luaL_checkint(L, 3);
	rect.right = rect.left + luaL_checkint(L, 4);
	rect.bottom = rect.top + luaL_checkint(L, 5);
	pushRect(L, rect);
	return 1;
}

static int rectIndex(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 1);
	Common::String key = luaL_checkstring(L, 2);

	if (key == "left") {
		lua_pushinteger(L, rect.left);
	} else if (key == "right") {
		lua_pushinteger(L, rect.right);
	} else if (key == "top") {
		lua_pushinteger(L, rect.top);
	} else if (key == "bottom") {
		lua_pushinteger(L, rect.bottom);
	} else if (key == "width") {
		lua_pushinteger(L, rect.width());
	} else if (key == "height") {
		lua_pushinteger(L, rect.height());
	} else if (key == "center") {
		pushPoint(L, ScriptPoint(rect.left + rect.width() / 2, rect.top + rect.height() / 2));
	} else {
		return indexFunctions(L);
	}

	return 1;
}

static int rectNewIndex(lua_State *L) {
	ScriptRect &rect = checkRect(L, 1);
	Common::String key = luaL_checkstring(L, 2);

	if (key == "left") {
		rect.left = luaL_checkint(L, 3);
	} else if (key == "right") {
		rect.right = luaL_checkint(L, 3);
	} else if (key == "top") {
		rect.top = luaL_checkint(L, 3);
	} else if (key == "bottom") {
		rect.bottom = luaL_checkint(L, 3);
	} else if (key == "width") {
		rect.right = rect.left + luaL_checkint(L, 3);
	} else if (key == "height") {
		rect.bottom = rect.top + luaL_checkint(L, 3);
	} else {
		return luaL_error(L, "Object %s does not have field '%s'", kRectType, key.c_str());
	}

	return 0;
}

static int rectEquals(lua_State *L) {
	lua_pushboolean(L, checkRect(L, 1) == checkRect(L, 2));
	return 1;
}

static int rectContains(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 1);

	ScriptPoint point;
	if (!readPoint(L, 2, point)) {
		return luaL_error(L, "Single parameter must be a Point, a Vector or a table with x and y fields");
	}

	lua_pushboolean(L, rect.contains(point.x, point.y));
	return 1;
}

static int rectOffset(lua_State *L) {
	ScriptRect rect = checkRect(L, 1);
	ScriptPoint offset = checkPointArguments(L, 2);
	rect.translate(offset.x, offset.y);
	pushRect(L, rect);
	return 1;
}

// Returns whether the two overlap, and the part they share
static int rectIntersect(lua_State *L) {
	const ScriptRect &a = checkRect(L, 1);
	const ScriptRect &b = checkRect(L, 2);

	ScriptRect shared(MAX(a.left, b.left), MAX(a.top, b.top), MIN(a.right, b.right), MIN(a.bottom, b.bottom));
	if (a.isEmpty() || b.isEmpty() || shared.isEmpty()) {
		shared = ScriptRect();
	}

	lua_pushboolean(L, !shared.isEmpty());
	pushRect(L, shared);
	return 2;
}

static int rectCenteredOn(lua_State *L) {
	ScriptRect rect = checkRect(L, 1);
	ScriptPoint center = checkPointArguments(L, 2);
	rect.translate(center.x - (rect.left + rect.width() / 2), center.y - (rect.top + rect.height() / 2));
	pushRect(L, rect);
	return 1;
}

static const luaL_Reg kRectFunctions[] = {
	{ "__index", rectIndex },
	{ "__newindex", rectNewIndex },
	{ "__eq", rectEquals },
	{ "Contains", rectContains },
	{ "Offset", rectOffset },
	{ "Intersect", rectIntersect },
	{ "CenteredOn", rectCenteredOn },
	{ nullptr, nullptr }
};

static const luaL_Reg kRectHelperFunctions[] = {
	{ "__index", indexFunctions },
	{ "New", rectNew },
	{ "LTRB", rectNew },
	{ "LTWH", rectNewLTWH },
	{ nullptr, nullptr }
};

// Point

void pushPoint(lua_State *L, const ScriptPoint &point) {
	pushValue(L, kPointType, point);
}

static ScriptPoint &checkPoint(lua_State *L, int index) {
	return *(ScriptPoint *)checkUserData(L, index, kPointType);
}

bool readPoint(lua_State *L, int index, ScriptPoint &point) {
	int type = lua_type(L, index);
	if (type != LUA_TUSERDATA && type != LUA_TTABLE) {
		return false;
	}

	index = absoluteIndex(L, index);

	lua_getfield(L, index, "x");
	lua_getfield(L, index, "y");
	bool hasBoth = lua_isnumber(L, -1) && lua_isnumber(L, -2);
	if (hasBoth) {
		point.x = (int32)lua_tointeger(L, -2);
		point.y = (int32)lua_tointeger(L, -1);
	}

	lua_pop(L, 2);
	return hasBoth;
}

static int pointNew(lua_State *L) {
	ScriptPoint point;
	point.x = luaL_checkint(L, 2);
	point.y = luaL_checkint(L, 3);
	pushPoint(L, point);
	return 1;
}

static int pointIndex(lua_State *L) {
	const ScriptPoint &point = checkPoint(L, 1);
	Common::String key = luaL_checkstring(L, 2);

	if (key == "x") {
		lua_pushinteger(L, point.x);
	} else if (key == "y") {
		lua_pushinteger(L, point.y);
	} else {
		return indexFunctions(L);
	}

	return 1;
}

static int pointNewIndex(lua_State *L) {
	ScriptPoint &point = checkPoint(L, 1);
	Common::String key = luaL_checkstring(L, 2);

	if (key == "x") {
		point.x = luaL_checkint(L, 3);
	} else if (key == "y") {
		point.y = luaL_checkint(L, 3);
	} else {
		return luaL_error(L, "Object %s does not have field '%s'", kPointType, key.c_str());
	}

	return 0;
}

static int pointEquals(lua_State *L) {
	const ScriptPoint &a = checkPoint(L, 1);
	const ScriptPoint &b = checkPoint(L, 2);
	lua_pushboolean(L, a.x == b.x && a.y == b.y);
	return 1;
}

static int pointAdd(lua_State *L) {
	const ScriptPoint &a = checkPoint(L, 1);
	const ScriptPoint &b = checkPoint(L, 2);
	pushPoint(L, ScriptPoint(a.x + b.x, a.y + b.y));
	return 1;
}

static int pointSubtract(lua_State *L) {
	const ScriptPoint &a = checkPoint(L, 1);
	const ScriptPoint &b = checkPoint(L, 2);
	pushPoint(L, ScriptPoint(a.x - b.x, a.y - b.y));
	return 1;
}

static const luaL_Reg kPointFunctions[] = {
	{ "__index", pointIndex },
	{ "__newindex", pointNewIndex },
	{ "__eq", pointEquals },
	{ "__add", pointAdd },
	{ "__sub", pointSubtract },
	{ nullptr, nullptr }
};

static const luaL_Reg kPointHelperFunctions[] = {
	{ "__index", indexFunctions },
	{ "New", pointNew },
	{ nullptr, nullptr }
};

// Color

void pushColor(lua_State *L, uint32 argb) {
	pushValue(L, kColorType, argb);
}

uint32 *testColor(lua_State *L, int index) {
	return (uint32 *)testUserData(L, index, kColorType);
}

uint32 checkColor(lua_State *L, int index) {
	return *(uint32 *)checkUserData(L, index, kColorType);
}

// The shift of a color's component, or -1 when there is none by that name
static int getColorComponentShift(const Common::String &name) {
	if (name == "r") {
		return 16;
	} else if (name == "g") {
		return 8;
	} else if (name == "b") {
		return 0;
	} else if (name == "a") {
		return 24;
	}

	return -1;
}

static int colorNew(lua_State *L) {
	uint32 a = luaL_checkint(L, 2) & 0xFF;
	uint32 r = luaL_checkint(L, 3) & 0xFF;
	uint32 g = luaL_checkint(L, 4) & 0xFF;
	uint32 b = luaL_checkint(L, 5) & 0xFF;
	pushColor(L, (a << 24) | (r << 16) | (g << 8) | b);
	return 1;
}

static int colorIndex(lua_State *L) {
	uint32 color = checkColor(L, 1);
	int shift = getColorComponentShift(luaL_checkstring(L, 2));
	if (shift < 0) {
		return indexFunctions(L);
	}

	lua_pushnumber(L, (color >> shift) & 0xFF);
	return 1;
}

static int colorNewIndex(lua_State *L) {
	uint32 *color = (uint32 *)checkUserData(L, 1, kColorType);
	const char *key = luaL_checkstring(L, 2);
	uint32 value = (uint32)(int32)luaL_checknumber(L, 3) & 0xFF;

	int shift = getColorComponentShift(key);
	if (shift < 0) {
		return luaL_error(L, "color does not have %s as a field", key);
	}

	*color = (*color & ~((uint32)0xFF << shift)) | (value << shift);
	return 0;
}

static int colorEquals(lua_State *L) {
	lua_pushboolean(L, checkColor(L, 1) == checkColor(L, 2));
	return 1;
}

static const luaL_Reg kColorFunctions[] = {
	{ "__index", colorIndex },
	{ "__newindex", colorNewIndex },
	{ "__eq", colorEquals },
	{ nullptr, nullptr }
};

static const luaL_Reg kColorHelperFunctions[] = {
	{ "__index", indexFunctions },
	{ "New", colorNew },
	{ nullptr, nullptr }
};

void registerTypes(lua_State *L) {
	registerType(L, kRectType, kRectFunctions);
	registerGlobal(L, "Rect", kRectHelperType, kRectHelperFunctions);

	// The helpers also carry the constants of their type, which can't go into the
	// userdata, so they live in the metatable next to the functions
	luaL_getmetatable(L, kRectHelperType);
	pushRect(L, ScriptRect());
	lua_setfield(L, -2, "empty");
	lua_pop(L, 1);

	registerType(L, kPointType, kPointFunctions);
	registerGlobal(L, "Point", kPointHelperType, kPointHelperFunctions);

	luaL_getmetatable(L, kPointHelperType);
	pushPoint(L, ScriptPoint());
	lua_setfield(L, -2, "origin");
	lua_pop(L, 1);

	registerType(L, kColorType, kColorFunctions);
	registerGlobal(L, "Color", kColorHelperType, kColorHelperFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
