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

#include "engines/nancy/nancy.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/mathtypes.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

static const char *const kRectType = "ND.Rect";
static const char *const kRectHelperType = "ND.RectHelper";
static const char *const kPointType = "ND.Point";
static const char *const kPointHelperType = "ND.PointHelper";
static const char *const kColorType = "ND.Color";
static const char *const kColorHelperType = "ND.ColorHelper";

// What a script point holds
struct PointValue {
	float x = 0.0f;
	float y = 0.0f;

	PointValue() {}
	PointValue(double px, double py) : x((float)px), y((float)py) {}
};

static bool hasFractionalPoints();
static void pushPointValue(lua_State *L, const PointValue &point);

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
static Common::Point32 checkPointArguments(lua_State *L, int index) {
	Common::Point32 point;

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

// The same, without rounding the numbers
static void checkPlaceArguments(lua_State *L, int index, double &x, double &y) {
	if (lua_gettop(L) != index) {
		x = luaL_checknumber(L, index);
		y = luaL_checknumber(L, index + 1);
		return;
	}

	if (lua_type(L, index) == LUA_TUSERDATA || lua_type(L, index) == LUA_TTABLE) {
		lua_getfield(L, index, "x");
		lua_getfield(L, index, "y");
		const bool hasBoth = lua_isnumber(L, -1) && lua_isnumber(L, -2);
		x = lua_tonumber(L, -2);
		y = lua_tonumber(L, -1);
		lua_pop(L, 2);

		if (hasBoth) {
			return;
		}
	}

	luaL_argerror(L, index, "Single parameter must be a Point or a table with x and y fields");
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
		// From Nancy26 on, the middle of a rect can be half a pixel in
		if (hasFractionalPoints()) {
			pushPointValue(L, PointValue(rect.left + rect.width() * 0.5f, rect.top + rect.height() * 0.5f));
		} else {
			pushPoint(L, Common::Point32(rect.left + rect.width() / 2, rect.top + rect.height() / 2));
		}
	} else if (key == "centerV") {
		// Not rounded, unlike "center"
		ScriptVector center;
		center.size = 2;
		center.v[0] = rect.left + rect.width() * 0.5f;
		center.v[1] = rect.top + rect.height() * 0.5f;
		pushVector(L, center);
	} else if (key == "size") {
		// Nancy22: the width and the height as a point
		pushPoint(L, Common::Point32(rect.width(), rect.height()));
	} else if (key == "leftTop") {
		pushPoint(L, Common::Point32(rect.left, rect.top));
	} else if (key == "rightTop") {
		pushPoint(L, Common::Point32(rect.right, rect.top));
	} else if (key == "leftBottom") {
		pushPoint(L, Common::Point32(rect.left, rect.bottom));
	} else if (key == "rightBottom") {
		pushPoint(L, Common::Point32(rect.right, rect.bottom));
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

	Common::Point32 point;
	if (!readPoint(L, 2, point)) {
		return luaL_error(L, "Single parameter must be a Point, a Vector or a table with x and y fields");
	}

	lua_pushboolean(L, rect.contains(point.x, point.y));
	return 1;
}

// Rect:Clamp(point) and Rect:Clamp(x, y), Nancy27: the nearest point of the
// rect, right and bottom edges included
static int rectClamp(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 1);
	const Common::Point32 point = checkPointArguments(L, 2);
	pushPoint(L, Common::Point32(CLIP<int32>(point.x, rect.left, MAX(rect.left, rect.right)),
								 CLIP<int32>(point.y, rect.top, MAX(rect.top, rect.bottom))));
	return 1;
}

static int rectOffset(lua_State *L) {
	ScriptRect rect = checkRect(L, 1);
	Common::Point32 offset = checkPointArguments(L, 2);
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
	Common::Point32 center = checkPointArguments(L, 2);
	rect.translate(center.x - (rect.left + rect.width() / 2), center.y - (rect.top + rect.height() / 2));
	pushRect(L, rect);
	return 1;
}

// rect:Expand(amount) grows the rectangle by as much on every side
static int rectExpand(lua_State *L) {
	ScriptRect rect = checkRect(L, 1);
	const int32 amount = luaL_checkint(L, 2);
	pushRect(L, ScriptRect(rect.left - amount, rect.top - amount, rect.right + amount, rect.bottom + amount));
	return 1;
}

// rect:CenterSet(place) moves the rectangle so that its middle is at the place
static int rectCenterSet(lua_State *L) {
	ScriptRect rect = checkRect(L, 1);
	double x, y;
	checkPlaceArguments(L, 2, x, y);

	const float centerX = rect.left + rect.width() * 0.5f;
	const float centerY = rect.top + rect.height() * 0.5f;
	rect.translate((int32)(x - centerX), (int32)(y - centerY));
	pushRect(L, rect);
	return 1;
}

// rect:LeftTopSet(place) and rect:LeftBottomSet(place) move the rectangle so that
// that corner is at the place
static int rectLeftTopSet(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 1);
	double x, y;
	checkPlaceArguments(L, 2, x, y);
	pushRect(L, ScriptRect((int32)x, (int32)y, (int32)(rect.width() + x), (int32)(rect.height() + y)));
	return 1;
}

static int rectLeftBottomSet(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 1);
	double x, y;
	checkPlaceArguments(L, 2, x, y);
	pushRect(L, ScriptRect((int32)x, (int32)(y - rect.height()), (int32)(rect.width() + x), (int32)y));
	return 1;
}

// rect:RightBottomSet(place), Nancy20
static int rectRightBottomSet(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 1);
	double x, y;
	checkPlaceArguments(L, 2, x, y);
	pushRect(L, ScriptRect((int32)(x - rect.width()), (int32)(y - rect.height()), (int32)x, (int32)y));
	return 1;
}

static int rectToString(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 1);
	lua_pushstring(L, Common::String::format("R(%d, %d, %d, %d)", rect.left, rect.top, rect.right, rect.bottom).c_str());
	return 1;
}

// rect .. text and point .. text, Nancy26, joins what both print as
static int concatToStrings(lua_State *L) {
	for (int i = 1; i <= 2; ++i) {
		if (!luaL_callmeta(L, i, "__tostring")) {
			lua_pushvalue(L, i);
		}

		if (!lua_isstring(L, -1)) {
			return luaL_error(L, "attempt to concatenate a %s value", luaL_typename(L, i));
		}
	}

	lua_concat(L, 2);
	return 1;
}

static const luaL_Reg kRectFunctions[] = {
	{ "__index", rectIndex },
	{ "__tostring", rectToString },
	{ "__concat", concatToStrings },
	{ "__newindex", rectNewIndex },
	{ "__eq", rectEquals },
	{ "Contains", rectContains },
	{ "Clamp", rectClamp },
	{ "Offset", rectOffset },
	{ "Intersect", rectIntersect },
	{ "CenteredOn", rectCenteredOn },
	{ "Expand", rectExpand },
	{ "CenterSet", rectCenterSet },
	{ "LeftTopSet", rectLeftTopSet },
	{ "LeftBottomSet", rectLeftBottomSet },
	{ "RightBottomSet", rectRightBottomSet },
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

// The coordinates of points can have fractions from Nancy26 on. Before that,
// numbers lose theirs when they become coordinates.
static bool hasFractionalPoints() {
	return g_nancy->getGameType() >= kGameTypeNancy26;
}

static double checkCoordinate(lua_State *L, int index) {
	return hasFractionalPoints() ? luaL_checknumber(L, index) : luaL_checkint(L, index);
}

static void pushPointValue(lua_State *L, const PointValue &point) {
	pushValue(L, kPointType, point);
}

void pushPoint(lua_State *L, const Common::Point32 &point) {
	pushPointValue(L, PointValue(point.x, point.y));
}

static PointValue &checkPoint(lua_State *L, int index) {
	return *(PointValue *)checkUserData(L, index, kPointType);
}

bool readPoint(lua_State *L, int index, Common::Point32 &point) {
	// A point's own coordinates lose their fractions on the way to the screen
	const PointValue *value = (const PointValue *)testUserData(L, index, kPointType);
	if (value) {
		point.x = (int32)value->x;
		point.y = (int32)value->y;
		return true;
	}

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

bool readPointCoordinates(lua_State *L, int index, double &x, double &y) {
	const PointValue *value = (const PointValue *)testUserData(L, index, kPointType);
	if (value) {
		x = value->x;
		y = value->y;
		return true;
	}

	Common::Point32 point;
	if (!readPoint(L, index, point)) {
		return false;
	}

	x = point.x;
	y = point.y;
	return true;
}

bool copyPoint(lua_State *from, int index, lua_State *to) {
	const PointValue *value = (const PointValue *)testUserData(from, index, kPointType);
	if (value) {
		pushPointValue(to, *value);
		return true;
	}

	Common::Point32 point;
	if (lua_isuserdata(from, index) && readPoint(from, index, point)) {
		pushPoint(to, point);
		return true;
	}

	return false;
}

// Point:New(x, y), or Point:New(place) in Nancy20
static int pointNew(lua_State *L) {
	PointValue point;
	if (lua_gettop(L) == 2) {
		double x, y;
		checkPlaceArguments(L, 2, x, y);
		point = hasFractionalPoints() ? PointValue(x, y) : PointValue((int32)x, (int32)y);
	} else {
		point = PointValue(checkCoordinate(L, 2), checkCoordinate(L, 3));
	}

	pushPointValue(L, point);
	return 1;
}

static int pointIndex(lua_State *L) {
	const PointValue &point = checkPoint(L, 1);
	Common::String key = luaL_checkstring(L, 2);

	if (key == "x") {
		lua_pushnumber(L, point.x);
	} else if (key == "y") {
		lua_pushnumber(L, point.y);
	} else {
		return indexFunctions(L);
	}

	return 1;
}

static int pointNewIndex(lua_State *L) {
	PointValue &point = checkPoint(L, 1);
	Common::String key = luaL_checkstring(L, 2);

	if (key == "x") {
		point.x = (float)checkCoordinate(L, 3);
	} else if (key == "y") {
		point.y = (float)checkCoordinate(L, 3);
	} else {
		return luaL_error(L, "Object %s does not have field '%s'", kPointType, key.c_str());
	}

	return 0;
}

static int pointEquals(lua_State *L) {
	const PointValue &a = checkPoint(L, 1);
	const PointValue &b = checkPoint(L, 2);
	lua_pushboolean(L, a.x == b.x && a.y == b.y);
	return 1;
}

static int pointAdd(lua_State *L) {
	const PointValue &a = checkPoint(L, 1);
	const PointValue &b = checkPoint(L, 2);
	pushPointValue(L, PointValue(a.x + b.x, a.y + b.y));
	return 1;
}

static int pointSubtract(lua_State *L) {
	const PointValue &a = checkPoint(L, 1);
	const PointValue &b = checkPoint(L, 2);
	pushPointValue(L, PointValue(a.x - b.x, a.y - b.y));
	return 1;
}

// Either side can be a number, which multiplies or divides both coordinates.
// Before Nancy26, the number is truncated first.
static void checkPointFactor(lua_State *L, int index, float &x, float &y) {
	if (lua_isnumber(L, index)) {
		const double factor = lua_tonumber(L, index);
		x = y = hasFractionalPoints() ? (float)factor : (float)(int32)factor;
		return;
	}

	const PointValue &point = checkPoint(L, index);
	x = point.x;
	y = point.y;
}

static int pointMultiply(lua_State *L) {
	float ax, ay, bx, by;
	checkPointFactor(L, 1, ax, ay);
	checkPointFactor(L, 2, bx, by);
	pushPointValue(L, PointValue(ax * bx, ay * by));
	return 1;
}

// Nancy27
static int pointDivide(lua_State *L) {
	float ax, ay, bx, by;
	checkPointFactor(L, 1, ax, ay);
	checkPointFactor(L, 2, bx, by);
	pushPointValue(L, PointValue(ax / bx, ay / by));
	return 1;
}

// Halves go away from zero
static double roundCoordinate(float value) {
	return value > 0.0f ? floor(value + 0.5) : ceil(value - 0.5);
}

// Point:Rounded() and Point:Floored(), Nancy27
static int pointRounded(lua_State *L) {
	const PointValue &point = checkPoint(L, 1);
	pushPointValue(L, PointValue(roundCoordinate(point.x), roundCoordinate(point.y)));
	return 1;
}

static int pointFloored(lua_State *L) {
	const PointValue &point = checkPoint(L, 1);
	pushPointValue(L, PointValue(floor(point.x), floor(point.y)));
	return 1;
}

static int pointNegate(lua_State *L) {
	const PointValue &point = checkPoint(L, 1);
	pushPointValue(L, PointValue(-point.x, -point.y));
	return 1;
}

static int pointToString(lua_State *L) {
	const PointValue &point = checkPoint(L, 1);
	if (hasFractionalPoints()) {
		lua_pushstring(L, Common::String::format("<%f, %f>", point.x, point.y).c_str());
	} else {
		lua_pushstring(L, Common::String::format("P(%d, %d)", (int32)point.x, (int32)point.y).c_str());
	}

	return 1;
}

static const luaL_Reg kPointFunctions[] = {
	{ "__index", pointIndex },
	{ "__newindex", pointNewIndex },
	{ "__eq", pointEquals },
	{ "__add", pointAdd },
	{ "__mul", pointMultiply },
	{ "__div", pointDivide },
	{ "__sub", pointSubtract },
	{ "__unm", pointNegate },
	{ "__tostring", pointToString },
	{ "__concat", concatToStrings },
	{ "Rounded", pointRounded },
	{ "Floored", pointFloored },
	{ nullptr, nullptr }
};

// Point.origin is a new point every time, since scripts can change points
static int pointHelperIndex(lua_State *L) {
	if (lua_type(L, 2) == LUA_TSTRING && strcmp(lua_tostring(L, 2), "origin") == 0) {
		pushPointValue(L, PointValue(0.0, 0.0));
		return 1;
	}

	return indexFunctions(L);
}

static const luaL_Reg kPointHelperFunctions[] = {
	{ "__index", pointHelperIndex },
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

// Color:New(a, r, g, b), or Color:New(color) in Nancy20, where the color can
// also be one number holding all four
static int colorNew(lua_State *L) {
	if (lua_gettop(L) == 2) {
		if (lua_isnumber(L, 2)) {
			pushColor(L, (uint32)(int64)lua_tonumber(L, 2));
		} else {
			pushColor(L, checkColor(L, 2));
		}

		return 1;
	}

	uint32 a = luaL_checkint(L, 2) & 0xFF;
	uint32 r = luaL_checkint(L, 3) & 0xFF;
	uint32 g = luaL_checkint(L, 4) & 0xFF;
	uint32 b = luaL_checkint(L, 5) & 0xFF;
	pushColor(L, (a << 24) | (r << 16) | (g << 8) | b);
	return 1;
}

// color.number, Nancy20: all four components in one number, as a signed one
static int colorIndex(lua_State *L) {
	uint32 color = checkColor(L, 1);
	const char *key = luaL_checkstring(L, 2);
	if (strcmp(key, "number") == 0) {
		lua_pushnumber(L, (int32)color);
		return 1;
	}

	int shift = getColorComponentShift(key);
	if (shift < 0) {
		return indexFunctions(L);
	}

	lua_pushnumber(L, (color >> shift) & 0xFF);
	return 1;
}

static int colorNewIndex(lua_State *L) {
	uint32 *color = (uint32 *)checkUserData(L, 1, kColorType);
	const char *key = luaL_checkstring(L, 2);
	if (strcmp(key, "number") == 0) {
		*color = (uint32)(int32)luaL_checknumber(L, 3);
		return 0;
	}

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
	pushPoint(L, Common::Point32());
	lua_setfield(L, -2, "origin");
	lua_pop(L, 1);

	registerType(L, kColorType, kColorFunctions);
	registerGlobal(L, "Color", kColorHelperType, kColorHelperFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
