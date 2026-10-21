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

#include "common/debug.h"
#include "common/memstream.h"
#include "common/system.h"

#include "image/png.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/ndui/common.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace Script {

// Flags.<name>

// The label of a flag, or -1 when there is none of that name
static int16 findFlag(const Common::String &name) {
	const EVNT *flags = GetEngineData(EVNT);
	if (flags) {
		for (uint i = 0; i < flags->eventFlagNames.size(); ++i) {
			if (flags->eventFlagNames[i].equalsIgnoreCase(name)) {
				return (int16)(EVNT::kFirstEventFlag + i);
			}
		}
	}

	return -1;
}

bool setFlagByName(const Common::String &name, bool value) {
	const int16 label = findFlag(name);
	if (label < 0) {
		return false;
	}

	NancySceneState.setEventFlag(label, value ? g_nancy->_true : g_nancy->_false);
	return true;
}

bool getFlagByName(const Common::String &name, bool &value) {
	const int16 label = findFlag(name);
	if (label < 0) {
		return false;
	}

	value = NancySceneState.getEventFlag(label, g_nancy->_true);
	return true;
}

static int flagsIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	bool value = false;
	if (!getFlagByName(name, value)) {
		return luaL_error(L, "Flag \"%s\" referenced but does not exist.", name);
	}

	lua_pushboolean(L, value);
	return 1;
}

static int flagsNewIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	if (!setFlagByName(name, lua_toboolean(L, 3) != 0)) {
		return luaL_error(L, "Attempted to set flag \"%s\" but it does not exist.", name);
	}

	return 0;
}

static const luaL_Reg kFlagsFunctions[] = {
	{ "__index", flagsIndex },
	{ "__newindex", flagsNewIndex },
	{ nullptr, nullptr }
};

// VarTable.<name>, the whole numbers scripts keep by name

static int varTableIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	ScriptValueData *values = (ScriptValueData *)NancySceneState.getPuzzleData(ScriptValueData::getTag());
	lua_pushnumber(L, values->values.getValOrDefault(name, 0));
	return 1;
}

static int varTableNewIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);
	const int32 value = (int32)luaL_checknumber(L, 3);

	ScriptValueData *values = (ScriptValueData *)NancySceneState.getPuzzleData(ScriptValueData::getTag());
	values->values.setVal(name, value);
	return 0;
}

static const luaL_Reg kVarTableFunctions[] = {
	{ "__index", varTableIndex },
	{ "__newindex", varTableNewIndex },
	{ nullptr, nullptr }
};

// Z, which makes the numbers records are ordered by. Higher is closer to the player;
// both functions can take the number to count from in place of the middle.

static int zCloser(lua_State *L) {
	const double amount = luaL_checknumber(L, 2);
	const double from = lua_gettop(L) >= 3 ? luaL_checknumber(L, 3) : 0.0;
	lua_pushnumber(L, from + amount);
	return 1;
}

static int zFarther(lua_State *L) {
	const double amount = luaL_checknumber(L, 2);
	const double from = lua_gettop(L) >= 3 ? luaL_checknumber(L, 3) : 0.0;
	lua_pushnumber(L, from - amount);
	return 1;
}

static const luaL_Reg kZFunctions[] = {
	{ "__index", indexFunctions },
	{ "Closer", zCloser },
	{ "Farther", zFarther },
	{ nullptr, nullptr }
};

// Cursor.<name>, and the variants of a cursor: Cursor.<name>.hot

static const char *const kCursorIndexType = "ND.CursorIndex";

struct CursorIndex {
	uint type;
	bool isHot;
};

bool testCursor(lua_State *L, int index, uint &cursorType, bool &isHot) {
	const CursorIndex *cursor = (const CursorIndex *)testUserData(L, index, kCursorIndexType);
	if (!cursor) {
		return false;
	}

	cursorType = cursor->type;
	isHot = cursor->isHot;
	return true;
}

void pushCursor(lua_State *L, uint cursorType, bool isHot) {
	CursorIndex cursor;
	cursor.type = cursorType;
	cursor.isHot = isHot;
	pushValue(L, kCursorIndexType, cursor);
}

// What "Cursor.currentInv" is while no item is held
static const uint kNoCursorType = 0xFFFFFFFF;

// Cursor.<name>: one of the cursors of the game or of an item, or what the
// mouse is up to
static int cursorIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	if (strcmp(name, "MoveTo") == 0) {
		return indexFunctions(L);
	}

	const int type = g_nancy->_cursor->getCursorTypeByName(name);
	if (type >= 0) {
		pushCursor(L, type, false);
		return 1;
	}

	const INVD *inventory = GetEngineData(INVD);
	if (inventory) {
		for (const INVD::Item &item : inventory->items) {
			if (item.name.equalsIgnoreCase(name)) {
				pushCursor(L, g_nancy->_cursor->getItemCursorType(item.id), false);
				return 1;
			}
		}
	}

	const Common::String key = name;
	if (key == "currentInv") {
		// The cursor of the item that is being held
		const int16 heldItem = NancySceneState.getHeldItem();
		pushCursor(L, heldItem >= 0 ? g_nancy->_cursor->getItemCursorType(heldItem) : kNoCursorType, false);
		return 1;
	}

	if (key == "activePoint") {
		// Where the mouse is in the viewport, while it is in there
		ScriptPoint point;
		if (!SceneContext::getViewportMouse(g_nancy->_input->getInput(), point)) {
			point = ScriptPoint(-1, -1);
		}

		pushPoint(L, point);
		return 1;
	}

	if (key == "activeUIPoint") {
		// Where the mouse is on the screen
		pushPoint(L, ScriptPoint(g_nancy->_input->getInput().mousePos));
		return 1;
	}

	if (key == "inViewport") {
		ScriptPoint point;
		lua_pushboolean(L, SceneContext::getViewportMouse(g_nancy->_input->getInput(), point));
		return 1;
	}

	return luaL_error(L, "Tried to change the cursor to %s, which is not valid.", name);
}

static int cursorEquals(lua_State *L) {
	const CursorIndex &a = *(const CursorIndex *)checkUserData(L, 1, kCursorIndexType);
	const CursorIndex &b = *(const CursorIndex *)checkUserData(L, 2, kCursorIndexType);
	lua_pushboolean(L, a.type == b.type && a.isHot == b.isHot);
	return 1;
}

static int cursorVariantIndex(lua_State *L) {
	CursorIndex cursor = *(const CursorIndex *)checkUserData(L, 1, kCursorIndexType);
	const Common::String key = luaL_checkstring(L, 2);

	if (key.equalsIgnoreCase("Hot")) {
		cursor.isHot = true;
	} else if (key.equalsIgnoreCase("Regular")) {
		cursor.isHot = false;
	} else {
		lua_pushnil(L);
		return 1;
	}

	pushValue(L, kCursorIndexType, cursor);
	return 1;
}

// Nancy20: "Cursor.currentInv = nil" puts the item that is held back into the
// inventory. Items are only ever put into the hand by Player:AddInventoryItem().
static int cursorNewIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);
	if (strcmp(name, "currentInv") != 0) {
		return luaL_error(L, "Cursor does not have field %s", name);
	}

	if (!lua_isnil(L, 3)) {
		return luaL_error(L, "Inventory cursor is only settable to nil. Use Player:AddInventoryItem(...) to put in hand.");
	}

	NancySceneState.setNoHeldItem();
	return 0;
}

// Cursor:MoveTo(point), Nancy21: puts the mouse at a place in the viewport
static int cursorMoveTo(lua_State *L) {
	ScriptPoint point;
	if (!readPoint(L, 2, point)) {
		return luaL_argerror(L, 2, "expected a point");
	}

	const Common::Rect screen = NancySceneState.getViewport().convertViewportToScreen(Common::Rect(point.x, point.y, point.x + 1, point.y + 1));
	g_nancy->_cursor->warpCursor(Common::Point(screen.left, screen.top));
	return 0;
}

static const luaL_Reg kCursorFunctions[] = {
	{ "__index", cursorIndex },
	{ "__newindex", cursorNewIndex },
	{ "MoveTo", cursorMoveTo },
	{ nullptr, nullptr }
};

static const luaL_Reg kCursorIndexFunctions[] = {
	{ "__index", cursorVariantIndex },
	{ "__eq", cursorEquals },
	{ nullptr, nullptr }
};

// Key.<name>

static const char *const kKeyType = "ND.Key";

struct KeyName {
	const char *name;
	Common::KeyCode key;
};

static const KeyName kKeyNames[] = {
	{ "SPACE", Common::KEYCODE_SPACE },
	{ "END", Common::KEYCODE_END },
	{ "HOME", Common::KEYCODE_HOME },
	{ "PGUP", Common::KEYCODE_PAGEUP },
	{ "PGDOWN", Common::KEYCODE_PAGEDOWN },
	{ "LEFT", Common::KEYCODE_LEFT },
	{ "UP", Common::KEYCODE_UP },
	{ "RIGHT", Common::KEYCODE_RIGHT },
	{ "DOWN", Common::KEYCODE_DOWN },
	{ "INSERT", Common::KEYCODE_INSERT },
	{ "DELETE", Common::KEYCODE_DELETE },
	{ "ESC", Common::KEYCODE_ESCAPE },
	{ "ESCAPE", Common::KEYCODE_ESCAPE },
	{ "RETURN", Common::KEYCODE_RETURN },
	{ "TAB", Common::KEYCODE_TAB },
	{ "CONTROL", Common::KEYCODE_LCTRL },
	{ "CTRL", Common::KEYCODE_LCTRL },
	{ "SHIFT", Common::KEYCODE_LSHIFT },
	{ "BACKSPACE", Common::KEYCODE_BACKSPACE },	// Nancy20
	{ "NUMPAD0", Common::KEYCODE_KP0 },
	{ "NUMPAD1", Common::KEYCODE_KP1 },
	{ "NUMPAD2", Common::KEYCODE_KP2 },
	{ "NUMPAD3", Common::KEYCODE_KP3 },
	{ "NUMPAD4", Common::KEYCODE_KP4 },
	{ "NUMPAD5", Common::KEYCODE_KP5 },
	{ "NUMPAD6", Common::KEYCODE_KP6 },
	{ "NUMPAD7", Common::KEYCODE_KP7 },
	{ "NUMPAD8", Common::KEYCODE_KP8 },
	{ "NUMPAD9", Common::KEYCODE_KP9 }
};

void pushKey(lua_State *L, Common::KeyCode key) {
	pushValue(L, kKeyType, (int)key);
}

Common::KeyCode checkKey(lua_State *L, int index) {
	return (Common::KeyCode)*(const int *)checkUserData(L, index, kKeyType);
}

static int keyIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	for (const KeyName &key : kKeyNames) {
		if (scumm_stricmp(key.name, name) == 0) {
			pushKey(L, key.key);
			return 1;
		}
	}

	// A letter or a digit stands for its own key
	if (name[0] != 0 && name[1] == 0 && Common::isAlnum(name[0])) {
		pushKey(L, (Common::KeyCode)tolower(name[0]));
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

static int keyEquals(lua_State *L) {
	lua_pushboolean(L, checkKey(L, 1) == checkKey(L, 2));
	return 1;
}

static const luaL_Reg kKeyHelperFunctions[] = {
	{ "__index", keyIndex },
	{ nullptr, nullptr }
};

static const luaL_Reg kKeyFunctions[] = {
	{ "__eq", keyEquals },
	{ nullptr, nullptr }
};

// Game
// TODO: Game.time

// The two levels the game can be played at
static const uint kDifficultyJunior = 0;
static const uint kDifficultySenior = 2;

static int gameIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "difficulty") {
		lua_pushstring(L, NancySceneState.getDifficulty() == kDifficultySenior ? "senior" : "junior");
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

static int gameNewIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key != "difficulty") {
		return luaL_error(L, "Game does not have field %s", key.c_str());
	}

	const Common::String value = luaL_checkstring(L, 3);
	if (value == "senior") {
		NancySceneState.setDifficulty(kDifficultySenior);
	} else if (value == "junior") {
		NancySceneState.setDifficulty(kDifficultyJunior);
	} else {
		return luaL_error(L, "difficulty must be set to \"senior\" or \"junior\"");
	}

	return 0;
}

static const luaL_Reg kGameFunctions[] = {
	{ "__index", gameIndex },
	{ "__newindex", gameNewIndex },
	{ nullptr, nullptr }
};

// Draw, for the "Render" functions of the records

static Graphics::ManagedSurface *checkDrawTarget(lua_State *L) {
	Graphics::ManagedSurface *target = getDrawTarget();
	if (!target) {
		luaL_error(L, "Draw can only be used while a record is rendering");
	}

	return target;
}

static uint32 getDrawColor(const Graphics::ManagedSurface &target, uint32 argb) {
	return target.format.ARGBToColor(argb >> 24, (argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);
}

// A place given as a point, a vector or a table with x and y
static void checkDrawPlace(lua_State *L, int index, double &x, double &y) {
	index = absoluteIndex(L, index);
	if (lua_type(L, index) != LUA_TUSERDATA && lua_type(L, index) != LUA_TTABLE) {
		luaL_argerror(L, index, "Parameter must be a Point, a Vector or a table with x and y fields");
	}

	lua_getfield(L, index, "x");
	lua_getfield(L, index, "y");
	if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
		luaL_argerror(L, index, "Parameter must be a Point, a Vector or a table with x and y fields");
	}

	x = lua_tonumber(L, -2);
	y = lua_tonumber(L, -1);
	lua_pop(L, 2);
}

// Draw:Rect(rect, color)
static int drawRect(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 2);
	const uint32 color = checkColor(L, 3);
	Graphics::ManagedSurface *target = checkDrawTarget(L);

	Common::Rect area = rect.toRect();
	area.clip(Common::Rect(target->w, target->h));
	if (!area.isEmpty()) {
		target->fillRect(area, getDrawColor(*target, color));
	}

	return 0;
}

// Draw:Line(from, to, color), Nancy20
static int drawLine(lua_State *L) {
	double x0, y0, x1, y1;
	checkDrawPlace(L, 2, x0, y0);
	checkDrawPlace(L, 3, x1, y1);
	const uint32 color = checkColor(L, 4);
	Graphics::ManagedSurface *target = checkDrawTarget(L);

	target->drawLine((int)x0, (int)y0, (int)x1, (int)y1, getDrawColor(*target, color));
	return 0;
}

// Draw:Tri(a, b, c, color[, colorB[, colorC]]), Nancy19: a filled triangle.
// From Nancy20 on, each corner can have a color of its own, and the color goes
// from one corner to the others. The corners given no color take the first one.
static int drawTri(lua_State *L) {
	double x[3], y[3];
	for (int i = 0; i < 3; ++i) {
		checkDrawPlace(L, i + 2, x[i], y[i]);
	}

	uint32 colors[3];
	colors[0] = checkColor(L, 5);
	colors[1] = lua_isnoneornil(L, 6) ? colors[0] : checkColor(L, 6);
	colors[2] = lua_isnoneornil(L, 7) ? colors[0] : checkColor(L, 7);
	Graphics::ManagedSurface *target = checkDrawTarget(L);

	const double area = (x[1] - x[0]) * (y[2] - y[0]) - (x[2] - x[0]) * (y[1] - y[0]);
	if (area == 0.0) {
		return 0;
	}

	const int minX = MAX<int>(0, (int)floor(MIN(x[0], MIN(x[1], x[2]))));
	const int maxX = MIN<int>(target->w - 1, (int)ceil(MAX(x[0], MAX(x[1], x[2]))));
	const int minY = MAX<int>(0, (int)floor(MIN(y[0], MIN(y[1], y[2]))));
	const int maxY = MIN<int>(target->h - 1, (int)ceil(MAX(y[0], MAX(y[1], y[2]))));

	for (int py = minY; py <= maxY; ++py) {
		for (int px = minX; px <= maxX; ++px) {
			// How much of each corner the middle of the pixel has
			const double cx = px + 0.5;
			const double cy = py + 0.5;
			const double w0 = ((x[1] - cx) * (y[2] - cy) - (x[2] - cx) * (y[1] - cy)) / area;
			const double w1 = ((x[2] - cx) * (y[0] - cy) - (x[0] - cx) * (y[2] - cy)) / area;
			const double w2 = 1.0 - w0 - w1;
			if (w0 < 0.0 || w1 < 0.0 || w2 < 0.0) {
				continue;
			}

			uint32 argb = 0;
			for (int shift = 0; shift < 32; shift += 8) {
				const double channel = w0 * ((colors[0] >> shift) & 0xFF) + w1 * ((colors[1] >> shift) & 0xFF) + w2 * ((colors[2] >> shift) & 0xFF);
				argb |= (uint32)CLIP<int>((int)(channel + 0.5), 0, 255) << shift;
			}

			target->setPixel(px, py, getDrawColor(*target, argb));
		}
	}

	return 0;
}

// Draw:Circle(circle, color), Nancy20, where the circle is a table with a
// center and a radius. Only the outline is drawn, as a polygon of 19 sides.
static int drawCircle(lua_State *L) {
	static const int kNumPoints = 20;

	luaL_checktype(L, 2, LUA_TTABLE);
	lua_getfield(L, 2, "center");
	lua_getfield(L, 2, "radius");
	if (lua_isnil(L, -2) || !lua_isnumber(L, -1)) {
		return luaL_argerror(L, 2, "Parameter is not a circle (table with center and radius fields)");
	}

	double centerX, centerY;
	checkDrawPlace(L, -2, centerX, centerY);
	const double radius = lua_tonumber(L, -1);
	lua_pop(L, 2);

	const uint32 color = checkColor(L, 3);
	Graphics::ManagedSurface *target = checkDrawTarget(L);
	const uint32 drawColor = getDrawColor(*target, color);

	int lastX = 0;
	int lastY = 0;
	for (int i = 0; i < kNumPoints; ++i) {
		const double angle = i / (double)(kNumPoints - 1) * 2.0 * M_PI;
		const int x = (int)(cos(angle) * radius + centerX);
		const int y = (int)(sin(angle) * radius + centerY);
		if (i > 0) {
			target->drawLine(lastX, lastY, x, y, drawColor);
		}

		lastX = x;
		lastY = y;
	}

	return 0;
}

static const luaL_Reg kDrawFunctions[] = {
	{ "__index", indexFunctions },
	{ "Rect", drawRect },
	{ "Line", drawLine },
	{ "Tri", drawTri },
	{ "Circle", drawCircle },
	{ nullptr, nullptr }
};

// Save and Load, which keep what a script hands them in the saved game

enum SavedValueTag {
	kSavedNil		= 0,
	kSavedFalse		= 1,
	kSavedTrue		= 2,
	kSavedNumber	= 3,
	kSavedString	= 4,
	kSavedTable		= 5
};

static void writeSavedUint32(Common::Array<byte> &out, uint32 value) {
	for (int i = 0; i < 4; ++i) {
		out.push_back((byte)(value >> (8 * i)));
	}
}

// Packs the value at a stack index. Returns false for what can't be kept.
static bool writeSavedValue(lua_State *L, int index, Common::Array<byte> &out, int depth) {
	index = absoluteIndex(L, index);

	switch (lua_type(L, index)) {
	case LUA_TNIL:
		out.push_back(kSavedNil);
		return true;
	case LUA_TBOOLEAN:
		out.push_back(lua_toboolean(L, index) ? kSavedTrue : kSavedFalse);
		return true;
	case LUA_TNUMBER: {
		const double number = lua_tonumber(L, index);
		uint64 bits;
		memcpy(&bits, &number, sizeof(bits));

		out.push_back(kSavedNumber);
		writeSavedUint32(out, (uint32)bits);
		writeSavedUint32(out, (uint32)(bits >> 32));
		return true;
	}
	case LUA_TSTRING: {
		size_t length;
		const char *text = lua_tolstring(L, index, &length);

		out.push_back(kSavedString);
		writeSavedUint32(out, (uint32)length);
		for (size_t i = 0; i < length; ++i) {
			out.push_back((byte)text[i]);
		}

		return true;
	}
	case LUA_TTABLE: {
		// Tables that hold themselves would never end
		if (depth > 16) {
			return false;
		}

		Common::Array<byte> pairs;
		uint32 numPairs = 0;
		bool isValid = true;

		lua_pushnil(L);
		while (lua_next(L, index)) {
			// Copies the key, so reading a number as a string can't confuse lua_next
			lua_pushvalue(L, -2);
			isValid = isValid && writeSavedValue(L, -1, pairs, depth + 1) && writeSavedValue(L, -2, pairs, depth + 1);
			lua_pop(L, 2);
			++numPairs;
		}

		if (!isValid) {
			return false;
		}

		out.push_back(kSavedTable);
		writeSavedUint32(out, numPairs);
		out.push_back(pairs);
		return true;
	}
	default:
		return false;
	}
}

static uint32 readSavedUint32(const Common::Array<byte> &in, uint &pos) {
	uint32 value = 0;
	for (int i = 0; i < 4 && pos < in.size(); ++i) {
		value |= (uint32)in[pos++] << (8 * i);
	}

	return value;
}

// Pushes the value packed at a position
static void readSavedValue(lua_State *L, const Common::Array<byte> &in, uint &pos) {
	const byte tag = pos < in.size() ? in[pos++] : (byte)kSavedNil;

	switch (tag) {
	case kSavedFalse:
		lua_pushboolean(L, false);
		break;
	case kSavedTrue:
		lua_pushboolean(L, true);
		break;
	case kSavedNumber: {
		uint64 bits = readSavedUint32(in, pos);
		bits |= (uint64)readSavedUint32(in, pos) << 32;

		double number;
		memcpy(&number, &bits, sizeof(number));
		lua_pushnumber(L, number);
		break;
	}
	case kSavedString: {
		const uint32 length = MIN<uint32>(readSavedUint32(in, pos), in.size() - pos);
		lua_pushlstring(L, (const char *)in.data() + pos, length);
		pos += length;
		break;
	}
	case kSavedTable: {
		const uint32 numPairs = readSavedUint32(in, pos);
		lua_newtable(L);
		luaL_checkstack(L, 4, "saved data is nested too deeply");

		for (uint32 i = 0; i < numPairs && pos < in.size(); ++i) {
			readSavedValue(L, in, pos);
			readSavedValue(L, in, pos);
			if (lua_isnil(L, -2)) {
				lua_pop(L, 2);
			} else {
				lua_rawset(L, -3);
			}
		}

		break;
	}
	default:
		lua_pushnil(L);
		break;
	}
}

// Save:Data(name, value)
static int saveData(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);
	luaL_checkany(L, 3);

	Common::Array<byte> packed;
	if (!writeSavedValue(L, 3, packed, 0)) {
		return luaL_error(L, "Save:Data can only save tables, numbers, booleans, strings and nils");
	}

	ScriptSavedData *saved = (ScriptSavedData *)NancySceneState.getPuzzleData(ScriptSavedData::getTag());
	saved->values.setVal(name, packed);
	return 0;
}

// Load:Data(name), which gives nil for a name nothing was saved under
static int loadData(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	ScriptSavedData *saved = (ScriptSavedData *)NancySceneState.getPuzzleData(ScriptSavedData::getTag());
	if (!saved->values.contains(name)) {
		lua_pushnil(L);
		return 1;
	}

	uint pos = 0;
	readSavedValue(L, saved->values[name], pos);
	return 1;
}

// Save:Game(name), which saves the game under a name of the scripts' choosing
static int saveGame(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (ui) {
		ui->requestSave(name);
	}

	return 0;
}

// Save:Screenshot(rect[, name]), Nancy21: keeps a picture of a part of the
// viewport with the game, for the camera of the cell phone. Overlays show it
// from the "LOADSAVE" tree. Gives the name of the picture, or an empty one when
// it couldn't be taken.
static int saveScreenshot(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 2);
	const Common::String name = lua_gettop(L) > 2 ? Common::String(luaL_checkstring(L, 3)) : Common::String::format("%u_OVL", g_system->getMillis());

	// The viewport as it was drawn last, before it got scaled onto the screen
	const Graphics::ManagedSurface &image = g_nancy->_graphics->getGameImage();
	Common::Rect area = NancySceneState.getViewport().convertViewportToScreen(rect.toRect());
	area.clip(Common::Rect(image.w, image.h));

	bool isTaken = false;
	if (!area.isEmpty()) {
		const Graphics::Surface part = image.rawSurface().getSubArea(area);
		Graphics::Surface *picture = part.convertTo(Graphics::PixelFormat::createFormatRGB24());

		Common::MemoryWriteStreamDynamic png(DisposeAfterUse::YES);
		if (Image::writePNG(png, *picture)) {
			ScriptPictureData *pictures = (ScriptPictureData *)NancySceneState.getPuzzleData(ScriptPictureData::getTag());
			Common::Array<byte> &data = pictures->pictures[name];
			data.resize((uint)png.size());
			memcpy(data.data(), png.getData(), png.size());
			isTaken = true;
		}

		picture->free();
		delete picture;
	}

	lua_pushstring(L, isTaken ? name.c_str() : "");
	return 1;
}

static const luaL_Reg kSaveFunctions[] = {
	{ "__index", indexFunctions },
	{ "Data", saveData },
	{ "Game", saveGame },
	{ "Screenshot", saveScreenshot },
	{ nullptr, nullptr }
};

static const luaL_Reg kLoadFunctions[] = {
	{ "__index", indexFunctions },
	{ "Data", loadData },
	{ nullptr, nullptr }
};

// SoundChannel.<name> and Sound

static const char *const kSoundChannelType = "ND.SoundChannel";

enum SoundChannelID {
	kChannelFirstMusic		= 0,	// Theme
	kChannelFirstAmbient	= 6,
	kChannelFirstFX			= 14,
	kChannelFirstVoice		= 24,
	kNumSoundChannels		= 30,

	// These stand for every channel of a kind
	kChannelAll				= 0x21,
	kChannelAllMusic		= 0x22,
	kChannelAllFX			= 0x23,
	kChannelAllAmbient		= 0x24,
	kChannelAllVoice		= 0x25
};

struct SoundChannelName {
	const char *name;
	uint16 channel;
};

static const SoundChannelName kSoundChannelNames[] = {
	{ "Theme", 0 }, { "Music1", 1 }, { "Music2", 2 }, { "Music3", 3 }, { "UIMusic", 4 }, { "EngineMusic", 5 },
	{ "Ambient1", 6 }, { "Ambient2", 7 }, { "Ambient3", 8 }, { "Ambient4", 9 },
	{ "Ambient5", 10 }, { "Ambient6", 11 }, { "Ambient7", 12 }, { "Ambient8", 13 },
	{ "FX1", 14 }, { "FX2", 15 }, { "FX3", 16 }, { "FX4", 17 }, { "FX5", 18 }, { "FX6", 19 }, { "FX7", 20 }, { "FX8", 21 },
	{ "UIFX", 22 }, { "EngineFX", 23 },
	{ "NPCVoice", 24 }, { "PlayerVoice", 25 }, { "Voice1", 26 }, { "Voice2", 27 }, { "UIVoice", 28 }, { "EngineVoice", 29 },
	{ "All", kChannelAll }, { "AllMusic", kChannelAllMusic }, { "AllFX", kChannelAllFX },
	{ "AllAmbient", kChannelAllAmbient }, { "AllVoice", kChannelAllVoice }
};

void pushSoundChannel(lua_State *L, uint16 channel) {
	pushValue(L, kSoundChannelType, channel);
}

uint16 checkSoundChannel(lua_State *L, int index) {
	return *(const uint16 *)checkUserData(L, index, kSoundChannelType);
}

static int soundChannelIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	for (const SoundChannelName &channel : kSoundChannelNames) {
		if (scumm_stricmp(channel.name, name) == 0) {
			pushSoundChannel(L, channel.channel);
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

// The channels a channel object stands for
static void getChannelRange(uint16 channel, uint16 &first, uint16 &last) {
	switch (channel) {
	case kChannelAll:
		first = 0;
		last = kNumSoundChannels - 1;
		break;
	case kChannelAllMusic:
		first = kChannelFirstMusic;
		last = kChannelFirstAmbient - 1;
		break;
	case kChannelAllAmbient:
		first = kChannelFirstAmbient;
		last = kChannelFirstFX - 1;
		break;
	case kChannelAllFX:
		first = kChannelFirstFX;
		last = kChannelFirstVoice - 1;
		break;
	case kChannelAllVoice:
		first = kChannelFirstVoice;
		last = kNumSoundChannels - 1;
		break;
	default:
		first = last = channel;
		break;
	}
}

// Sound:Stop(channel)
static int soundStop(lua_State *L) {
	uint16 first, last;
	getChannelRange(checkSoundChannel(L, 2), first, last);

	for (uint16 channel = first; channel <= last; ++channel) {
		g_nancy->_sound->stopSound(channel);
	}

	return 0;
}

// Sound:SetVolume(channel, volume), with the volume going from 0 to 1
static int soundSetVolume(lua_State *L) {
	uint16 first, last;
	getChannelRange(checkSoundChannel(L, 2), first, last);
	const uint16 volume = (uint16)CLIP<int>((int)(luaL_checknumber(L, 3) * 100.0), 0, 100);

	for (uint16 channel = first; channel <= last; ++channel) {
		g_nancy->_sound->setVolume(channel, volume);
	}

	return 0;
}

// Sound:GetVolume(channel), from 0 to 1. It has to be a single channel.
static int soundGetVolume(lua_State *L) {
	const uint16 channel = checkSoundChannel(L, 2);
	uint16 first, last;
	getChannelRange(channel, first, last);
	if (first != last) {
		return luaL_error(L, "Passed in Channel Group to GetVolume.");
	}

	lua_pushnumber(L, g_nancy->_sound->getRequestedVolume(channel) / 100.0);
	return 1;
}

// Sound:IsPlaying(channel), which is true for a group when any of its channels plays
static int soundIsPlaying(lua_State *L) {
	uint16 first, last;
	getChannelRange(checkSoundChannel(L, 2), first, last);

	bool isPlaying = false;
	for (uint16 channel = first; channel <= last && !isPlaying; ++channel) {
		isPlaying = g_nancy->_sound->isSoundPlaying(channel);
	}

	lua_pushboolean(L, isPlaying);
	return 1;
}

static const luaL_Reg kSoundChannelHelperFunctions[] = {
	{ "__index", soundChannelIndex },
	{ nullptr, nullptr }
};

static const luaL_Reg kSoundChannelFunctions[] = {
	{ nullptr, nullptr }
};

static const luaL_Reg kSoundFunctions[] = {
	{ "__index", indexFunctions },
	{ "Stop", soundStop },
	{ "SetVolume", soundSetVolume },
	{ "GetVolume", soundGetVolume },
	{ "IsPlaying", soundIsPlaying },
	{ nullptr, nullptr }
};

// Scene, which is the scene the script belongs to

static SceneContext *checkContext(lua_State *L) {
	SceneContext *context = LuaState::getContext(L);
	if (!context) {
		luaL_error(L, "The Scene object can only be used by the script of a scene");
	}

	return context;
}

static int sceneIndex(lua_State *L) {
	SceneContext *context = checkContext(L);
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "name") {
		lua_pushstring(L, context->getSceneName().c_str());
	} else if (key == "time") {
		lua_pushnumber(L, context->getTime());
	} else if (key == "elapsedTime") {
		lua_pushnumber(L, context->getElapsedTime());
	} else if (key == "frame") {
		lua_pushinteger(L, context->getFrame());
	} else if (key == "streamName") {
		lua_pushstring(L, context->getStreamName().c_str());
	} else {
		return indexFunctions(L);
	}

	return 1;
}

// The parts of a scene a script can take over
static const char *const kSceneFunctionNames[] = {
	"RunOnce", "Run", "RenderAttachment", "ReceiveMessage", "OnSave"
};

static int sceneNewIndex(lua_State *L) {
	SceneContext *context = checkContext(L);
	const char *key = luaL_checkstring(L, 2);

	if (!lua_isfunction(L, 3)) {
		return luaL_error(L, "Can only set functions to the Scene object");
	}

	for (const char *name : kSceneFunctionNames) {
		if (strcmp(name, key) == 0) {
			lua_pushvalue(L, 3);
			context->setSceneFunction(name);
			return 0;
		}
	}

	return luaL_error(L, "Field %s is not overridable from script", key);
}

// Scene:Include(script)
static int sceneInclude(lua_State *L) {
	SceneContext *context = checkContext(L);
	const char *name = luaL_checkstring(L, 2);

	// A scene can't include itself
	if (!context->getSceneName().equalsIgnoreCase(name)) {
		context->include(name);
	}

	return 0;
}

// Scene:ChangeScene(stream, scene), which goes to the first frame of the scene
static int sceneChangeScene(lua_State *L) {
	SceneContext *context = checkContext(L);
	const char *streamName = luaL_checkstring(L, 2);
	const char *sceneName = luaL_checkstring(L, 3);

	context->changeScene(streamName, sceneName, 0);
	lua_pushboolean(L, true);
	return 1;
}

// How long a stream lives: "manual" (until it ends itself), "withScene" or "withStream"
static bool readStreamLifetime(const Common::String &lifetime, State::StreamFlags &flags) {
	if (lifetime == "manual") {
		flags.lifetime = State::StreamFlags::kIndependent;
	} else if (lifetime == "withScene") {
		flags.lifetime = State::StreamFlags::kEndsWithParentScene;
	} else if (lifetime == "withStream") {
		flags.lifetime = State::StreamFlags::kEndsWithParent;
	} else {
		return false;
	}

	return true;
}

// Scene:BeginStream(name, scene, frame, lifetime, capturesInput, runsFirst), or
// from Nancy21 on, Scene:BeginStream({stream, scene, frame, ends, captureInput})
static int sceneBeginStream(lua_State *L) {
	checkContext(L);

	Common::String name;
	Common::String sceneName;
	int frame = 0;
	State::StreamFlags flags;

	if (lua_istable(L, 2)) {
		if (!readField(L, 2, "stream", name)) {
			return luaL_argerror(L, 2, "Table must have 'stream' field");
		}

		if (!readField(L, 2, "scene", sceneName)) {
			return luaL_argerror(L, 2, "Table must have 'scene' field");
		}

		readField(L, 2, "frame", frame);

		Common::String lifetime = "manual";
		readField(L, 2, "ends", lifetime);
		if (!readStreamLifetime(lifetime, flags)) {
			return luaL_argerror(L, 2, "ends field must be \"manual\", \"withScene\" or \"withStream\"");
		}

		// TODO: "restartParent" and "isUI", which no script sets
		readField(L, 2, "captureInput", flags.capturesInput);

		// A stream is a step closer than the one that began it
		flags.zOffset = 1;
	} else {
		name = luaL_checkstring(L, 2);
		sceneName = luaL_checkstring(L, 3);
		frame = luaL_checkint(L, 4);

		if (!readStreamLifetime(luaL_checkstring(L, 5), flags)) {
			return luaL_argerror(L, 5, "parameter must be \"manual\", \"withScene\" or \"withStream\"");
		}

		luaL_checktype(L, 6, LUA_TBOOLEAN);
		luaL_checktype(L, 7, LUA_TBOOLEAN);
		flags.capturesInput = lua_toboolean(L, 6) != 0;
		flags.runsFirst = lua_toboolean(L, 7) != 0;
	}

	State::SceneStreamManager &streams = NancySceneState.getStreams();
	const bool isNew = !streams.isRunning(name);
	streams.begin(name, sceneName, (uint16)frame, flags);

	lua_pushboolean(L, isNew);
	return 1;
}

// Scene:GraphNode(), Nancy21: a new node to group records of the scene under
static int sceneGraphNode(lua_State *L) {
	checkContext(L)->createGraphNode()->push(L);
	return 1;
}

// Scene:TransferChild(child), Nancy21: takes a record or a node out of the node
// it was put under
static int sceneTransferChild(lua_State *L) {
	checkContext(L);

	GraphNode *node = testGraphNode(L, 2);
	if (node) {
		node->setParent(nullptr);
	} else {
		checkRecord(L, 2)->setParentNode(nullptr);
	}

	return 0;
}

// Scene:EndStream(name)
static int sceneEndStream(lua_State *L) {
	checkContext(L);
	const char *name = luaL_checkstring(L, 2);

	State::SceneStreamManager &streams = NancySceneState.getStreams();
	const bool wasRunning = streams.isRunning(name);
	streams.end(name);

	lua_pushboolean(L, wasRunning);
	return 1;
}

// What "for key, ar in Scene:ARs(direction)" calls for every round: gives the
// next record, and its place in the order asked for
static int sceneNextRecord(lua_State *L) {
	SceneContext *context = checkContext(L);
	const bool isForward = lua_toboolean(L, lua_upvalueindex(1)) != 0;
	const uint numDone = (uint)lua_tointeger(L, lua_upvalueindex(2));

	const Common::Array<Record *> &records = context->getRecords();
	if (numDone >= records.size()) {
		return 0;
	}

	lua_pushinteger(L, numDone + 1);
	lua_replace(L, lua_upvalueindex(2));

	lua_pushinteger(L, numDone + 1);
	records[isForward ? numDone : records.size() - 1 - numDone]->push();
	return 2;
}

// Scene:ARs(direction), which goes over the records of the scene, in the order
// they were made in ("forward") or the other way around ("backward")
static int sceneRecords(lua_State *L) {
	checkContext(L);
	const Common::String direction = luaL_checkstring(L, 2);

	if (direction != "forward" && direction != "backward") {
		return luaL_error(L, "Iteration direction (must be forward or backward) is %s", direction.c_str());
	}

	lua_pushboolean(L, direction == "forward");
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, sceneNextRecord, 2);
	return 1;
}

static const luaL_Reg kSceneFunctions[] = {
	{ "__index", sceneIndex },
	{ "__newindex", sceneNewIndex },
	{ "ARs", sceneRecords },
	{ "Include", sceneInclude },
	{ "BeginStream", sceneBeginStream },
	{ "EndStream", sceneEndStream },
	{ "ChangeScene", sceneChangeScene },
	{ "GraphNode", sceneGraphNode },
	{ "TransferChild", sceneTransferChild },
	{ nullptr, nullptr }
};

// Streams

// Streams.environment is the environment of the main stream's scene
static int streamsIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "environment") {
		lua_pushstring(L, NancySceneState.getStreams().getEnvironment().c_str());
		return 1;
	}

	return indexFunctions(L);
}

// Streams:GetScene(stream), which gives no name for a stream that isn't running
static int streamsGetScene(lua_State *L) {
	const Common::String streamName = luaL_checkstring(L, 2);
	lua_pushstring(L, NancySceneState.getStreams().getSceneName(streamName).c_str());
	return 1;
}

// Streams:GetStreams(withUI), Nancy21: the names of the running streams, the
// main stream among them. The one of the UI is only in there when asked for.
static int streamsGetStreams(lua_State *L) {
	const bool withUI = lua_toboolean(L, 2) != 0;

	Common::Array<Common::String> names;
	NancySceneState.getStreams().getStreamNames(names, withUI);

	lua_createtable(L, names.size(), 0);
	for (uint i = 0; i < names.size(); ++i) {
		lua_pushstring(L, names[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

// Streams:GetZ(stream), Nancy21
static int streamsGetZ(lua_State *L) {
	lua_pushnumber(L, NancySceneState.getStreams().getZ(luaL_checkstring(L, 2)));
	return 1;
}

// Streams:GetTransform(stream) and Streams:SetTransform(stream, matrix), Nancy21
static int streamsGetTransform(lua_State *L) {
	const ScriptMatrix *transform = NancySceneState.getStreams().getTransform(luaL_checkstring(L, 2));
	pushMatrix(L, transform ? *transform : ScriptMatrix());
	return 1;
}

static int streamsSetTransform(lua_State *L) {
	const char *streamName = luaL_checkstring(L, 2);
	NancySceneState.getStreams().setTransform(streamName, checkMatrix(L, 3));
	return 0;
}

static const luaL_Reg kStreamsFunctions[] = {
	{ "__index", streamsIndex },
	{ "GetScene", streamsGetScene },
	{ "GetStreams", streamsGetStreams },
	{ "GetZ", streamsGetZ },
	{ "GetTransform", streamsGetTransform },
	{ "SetTransform", streamsSetTransform },
	{ nullptr, nullptr }
};

// Hands a message to the scene or the record it is for, or else to the UI.
// Returns whether a scene or a record took it.
static bool deliverMessage(lua_State *L, const char *actionName, UI::NDUI::Action &action) {
	// The targets that answer do so to the control whose event is being handled
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (ui) {
		action.sender = ui->getEventSender();
	}

	// The scripts of a scene send in its name, and the ones of the UI in the
	// name of that control
	Message message;
	message.action = actionName;
	message.target = action.target;
	message.text = action.text;
	message.value = action.value;

	const SceneContext *context = LuaState::getContext(L);
	if (context && !context->getStreamName().equalsIgnoreCase(State::SceneStreamManager::getUIStreamName())) {
		message.sender = context->getSceneName();
	} else {
		message.sender = action.sender;
	}

	bool wasTaken = false;
	if (!NancySceneState.receiveMessage(message, wasTaken) && ui) {
		ui->takeAction(action);
	}

	return wasTaken;
}

// Player, who is the character being played unless a function is told otherwise

// The number of a character, or -1 when there is none of that name
static int findCharacter(const char *name) {
	const PCUI *players = GetEngineData(PCUI);
	if (players) {
		for (uint i = 0; i < players->characters.size(); ++i) {
			if (players->characters[i].name.equalsIgnoreCase(name)) {
				return i;
			}
		}
	}

	return -1;
}

// The character named by an optional argument
static uint checkCharacter(lua_State *L, int index) {
	if (lua_gettop(L) < index) {
		return g_nancy->getPlayerCharacter();
	}

	const char *name = luaL_checkstring(L, index);
	const int character = findCharacter(name);
	if (character < 0) {
		luaL_error(L, "Invalid player %s", name);
	}

	return character;
}

// Player:Switch(character), which hands the game over to another character.
// Nothing happens when they are the one being played, or when there is no such
// character.
static int playerSwitch(lua_State *L) {
	const int character = findCharacter(luaL_checkstring(L, 2));
	if (character >= 0) {
		NancySceneState.changePlayerCharacter(character);
	}

	return 0;
}

// Player:HasInventoryItem(item[, character])
static int playerHasInventoryItem(lua_State *L) {
	const int16 itemID = checkInventoryItem(L, 2);
	const uint character = checkCharacter(L, 3);

	lua_pushboolean(L, NancySceneState.hasCharacterItem(character, itemID) == g_nancy->_true);
	return 1;
}

// Player:AddInventoryItem(item, intoHand[, character])
static int playerAddInventoryItem(lua_State *L) {
	const int16 itemID = checkInventoryItem(L, 2);
	luaL_checktype(L, 3, LUA_TBOOLEAN);
	const bool intoHand = lua_toboolean(L, 3) != 0;
	const uint character = checkCharacter(L, 4);

	NancySceneState.giveItemToCharacter(character, itemID, intoHand, false);
	return 0;
}

// Player:RemoveInventoryItem(item[, character])
static int playerRemoveInventoryItem(lua_State *L) {
	const int16 itemID = checkInventoryItem(L, 2);
	const uint character = checkCharacter(L, 3);

	NancySceneState.removeItemFromCharacterInventory(character, itemID);
	return 0;
}

// The journal and the task list are filled by scripts, which hand out an entry
// when its conditions hold. Every entry is handed out once per character; the UI
// only shows the ones of the character being played.

static ScriptJournalData *getJournalData(uint character) {
	if (character >= kMaxPlayerCharacters) {
		return nullptr;
	}

	return (ScriptJournalData *)NancySceneState.getPuzzleData(ScriptJournalData::getTag());
}

// Player:AddJournalEntry(heading, text[, character])
static int playerAddJournalEntry(lua_State *L) {
	const Common::String heading = luaL_checkstring(L, 2);
	const Common::String text = luaL_checkstring(L, 3);
	const uint character = checkCharacter(L, 4);

	ScriptJournalData *data = getJournalData(character);
	if (!data) {
		return 0;
	}

	for (const ScriptJournalData::JournalEntry &entry : data->journalEntries[character]) {
		if (entry.heading.equalsIgnoreCase(heading) && entry.text.equalsIgnoreCase(text)) {
			return 0;
		}
	}

	ScriptJournalData::JournalEntry entry;
	entry.heading = heading;
	entry.text = text;
	data->journalEntries[character].push_back(entry);

	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (ui && character == g_nancy->getPlayerCharacter()) {
		ui->addJournalEntry(heading, text);
	}

	return 0;
}

// Player:AddTask(text, flag[, character]): the task can be checked off once the flag is set
static int playerAddTask(lua_State *L) {
	const Common::String text = luaL_checkstring(L, 2);
	const Common::String flagName = luaL_checkstring(L, 3);
	const uint character = checkCharacter(L, 4);

	ScriptJournalData *data = getJournalData(character);
	if (!data) {
		return 0;
	}

	for (const ScriptJournalData::Task &task : data->tasks[character]) {
		if (task.text.equalsIgnoreCase(text)) {
			return 0;
		}
	}

	ScriptJournalData::Task task;
	task.text = text;
	task.flagName = flagName;
	data->tasks[character].push_back(task);

	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (ui && character == g_nancy->getPlayerCharacter()) {
		ui->addTasklistEntry(text, findFlag(flagName));
	}

	return 0;
}

// Player:LoadTaskFlag(text, flag[, character]): gives a task that has none its flag
static int playerLoadTaskFlag(lua_State *L) {
	const Common::String text = luaL_checkstring(L, 2);
	const Common::String flagName = luaL_checkstring(L, 3);
	const uint character = checkCharacter(L, 4);

	ScriptJournalData *data = getJournalData(character);
	if (!data) {
		return 0;
	}

	for (ScriptJournalData::Task &task : data->tasks[character]) {
		if (task.text.equalsIgnoreCase(text) && task.flagName.empty()) {
			task.flagName = flagName;
		}
	}

	return 0;
}

void showPlayerEntries() {
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	const uint character = g_nancy->getPlayerCharacter();
	ScriptJournalData *data = getJournalData(character);
	if (!ui || !data) {
		return;
	}

	for (const ScriptJournalData::JournalEntry &entry : data->journalEntries[character]) {
		ui->addJournalEntry(entry.heading, entry.text);
	}

	for (const ScriptJournalData::Task &task : data->tasks[character]) {
		ui->addTasklistEntry(task.text, findFlag(task.flagName));
	}
}

// Player.time is the clock of the game, as {days, hours, minutes}. Player.name
// is the character being played.
static int playerIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "time") {
		const Time time = NancySceneState.getPlayerTime();
		lua_createtable(L, 0, 3);
		lua_pushinteger(L, time.getDays());
		lua_setfield(L, -2, "days");
		lua_pushinteger(L, time.getHours());
		lua_setfield(L, -2, "hours");
		lua_pushinteger(L, time.getMinutes());
		lua_setfield(L, -2, "minutes");
		return 1;
	}

	if (key == "name") {
		const PCUI *players = GetEngineData(PCUI);
		const uint character = g_nancy->getPlayerCharacter();
		lua_pushstring(L, players && character < players->characters.size() ? players->characters[character].name.c_str() : "");
		return 1;
	}

	return indexFunctions(L);
}

static int playerNewIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);
	if (key != "time") {
		return luaL_error(L, "Player does not have field %s", key.c_str());
	}

	luaL_checktype(L, 3, LUA_TTABLE);

	static const char *const kFieldNames[] = { "days", "hours", "minutes" };
	int values[3] = { 0, 0, 0 };
	for (uint i = 0; i < 3; ++i) {
		if (!readField(L, 3, kFieldNames[i], values[i])) {
			return luaL_error(L, "Player.time must have a field '%s'", kFieldNames[i]);
		}
	}

	NancySceneState.setPlayerClock((uint32)values[0] * 86400000 + (uint32)values[1] * 3600000 + (uint32)values[2] * 60000);
	return 0;
}

// The phone book and the text messages of the cell phone are kept by a record of
// the scripts, which goes by the name "PhoneManager"
static const char *const kPhoneManagerName = "PhoneManager";

// Player:AddCellPhoneNumber(contact[, character])
static int playerAddCellPhoneNumber(lua_State *L) {
	UI::NDUI::Action action;
	action.type = UI::NDUI::kActionSetText;
	action.target = kPhoneManagerName;
	action.text = luaL_checkstring(L, 2);
	deliverMessage(L, "setText", action);
	return 0;
}

// Player:SendCellPhoneText(message[, character]), where the message is a number
static int playerSendCellPhoneText(lua_State *L) {
	UI::NDUI::Action action;
	action.type = UI::NDUI::kActionSetValue;
	action.target = kPhoneManagerName;
	action.hasValue = true;
	action.value = luaL_checkinteger(L, 2);
	deliverMessage(L, "setValue", action);
	return 0;
}

static const luaL_Reg kPlayerFunctions[] = {
	{ "__index", playerIndex },
	{ "__newindex", playerNewIndex },
	{ "AddCellPhoneNumber", playerAddCellPhoneNumber },
	{ "SendCellPhoneText", playerSendCellPhoneText },
	{ "Switch", playerSwitch },
	{ "HasInventoryItem", playerHasInventoryItem },
	{ "AddInventoryItem", playerAddInventoryItem },
	{ "RemoveInventoryItem", playerRemoveInventoryItem },
	{ "AddJournalEntry", playerAddJournalEntry },
	{ "AddTask", playerAddTask },
	{ "LoadTaskFlag", playerLoadTaskFlag },
	{ nullptr, nullptr }
};

bool findText(const Common::String &key, Common::String &text) {
	static const char *const kTableNames[] = { "AUTOTEXT", "CONVO" };

	for (const char *tableName : kTableNames) {
		const CVTX *table = (const CVTX *)g_nancy->getEngineData(tableName);
		if (table && table->texts.contains(key)) {
			text = table->texts[key];
			return true;
		}
	}

	return false;
}

// Message, which has the UI do something to one of its panels or controls

struct MessageActionName {
	const char *name;
	int32 type;
};

static const MessageActionName kMessageActionNames[] = {
	{ "none", UI::NDUI::kActionNone },
	{ "show", UI::NDUI::kActionShow },
	{ "hide", UI::NDUI::kActionHide },
	{ "toggleHidden", UI::NDUI::kActionToggle },
	{ "enable", UI::NDUI::kActionEnable },
	{ "disable", UI::NDUI::kActionDisable },
	{ "toggleDisabled", UI::NDUI::kActionToggleEnabled },
	{ "addText", UI::NDUI::kActionAddText },
	{ "setText", UI::NDUI::kActionSetText },
	{ "setValue", UI::NDUI::kActionSetValue },
	{ "setImage", UI::NDUI::kActionSetImage },
	{ "expand", UI::NDUI::kActionExpand },
	{ "collapse", UI::NDUI::kActionCollapse },
	{ "toggleCollapsed", UI::NDUI::kActionToggleCollapsed },
	{ "getValue", UI::NDUI::kActionQuery },
	{ "showItem", UI::NDUI::kActionShowItem },
	{ "hideItem", UI::NDUI::kActionHideItem }
};

// Message:Send(action, target[, value[, text[, isTextKey]]])
// The target is a scene, a record that was given a name for it, or a part of
// the UI. Returns whether a scene or a record took the message.
// The text is the key of an autotext entry, unless it says otherwise.
static int messageSend(lua_State *L) {
	const char *actionName = luaL_checkstring(L, 2);
	const char *target = luaL_checkstring(L, 3);

	const MessageActionName *named = nullptr;
	for (const MessageActionName &action : kMessageActionNames) {
		if (scumm_stricmp(action.name, actionName) == 0) {
			named = &action;
		}
	}

	if (!named) {
		return luaL_error(L, "Action type '%s' invalid", actionName);
	}

	UI::NDUI::Action action;
	action.type = named->type;
	action.target = target;
	action.hasValue = true;

	const int numArgs = lua_gettop(L);
	if (numArgs > 3) {
		action.value = luaL_checknumber(L, 4);
	}

	if (numArgs > 4) {
		action.text = luaL_checkstring(L, 5);
	}

	const bool isTextKey = numArgs > 5 ? lua_toboolean(L, 6) : true;
	if (isTextKey && !action.text.empty()) {
		const Common::String key = action.text;
		if (!findText(key, action.text)) {
			return luaL_error(L, "Autotext reference '%s' invalid", key.c_str());
		}
	}

	lua_pushboolean(L, deliverMessage(L, named->name, action));
	return 1;
}

// Message:EncodeRect(rect), the value "setImage" takes a source rect as
static int messageEncodeRect(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 2);
	lua_pushnumber(L, UI::NDUI::encodeRect(rect.left, rect.top, rect.right, rect.bottom));
	return 1;
}

static const luaL_Reg kMessageFunctions[] = {
	{ "__index", indexFunctions },
	{ "Send", messageSend },
	{ "EncodeRect", messageEncodeRect },
	{ nullptr, nullptr }
};

// Viewport

static int viewportIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "size") {
		const VIEW *viewport = GetEngineData(VIEW);
		assert(viewport);
		pushRect(L, ScriptRect(0, 0, viewport->screenPosition.width(), viewport->screenPosition.height()));
		return 1;
	} else if (key == "location") {
		// Where the viewport is on the screen
		const VIEW *viewport = GetEngineData(VIEW);
		assert(viewport);
		pushRect(L, ScriptRect(viewport->screenPosition));
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

static const luaL_Reg kViewportFunctions[] = {
	{ "__index", viewportIndex },
	{ nullptr, nullptr }
};

// Log:Write(text)
static int logWrite(lua_State *L) {
	debugC(1, kDebugScene, "Script: %s", luaL_checkstring(L, 2));
	return 0;
}

// Log:Assert(condition, text) writes down the text when the condition doesn't hold
static int logAssert(lua_State *L) {
	if (!lua_toboolean(L, 2)) {
		luaL_where(L, 1);
		debugC(1, kDebugScene, "Script assertion failed: %s%s", lua_tostring(L, -1), luaL_optstring(L, 3, ""));
		lua_pop(L, 1);
	}

	return 0;
}

// Log:Watch(function) and Log:Clear() are for the debug console of the original,
// which isn't there
static int logWatch(lua_State *L) {
	luaL_checktype(L, 2, LUA_TFUNCTION);
	return 0;
}

static int logClear(lua_State *L) {
	return 0;
}

static const luaL_Reg kLogFunctions[] = {
	{ "__index", indexFunctions },
	{ "Write", logWrite },
	{ "Assert", logAssert },
	{ "Watch", logWatch },
	{ "Clear", logClear },
	{ nullptr, nullptr }
};

// Autotext, for the captions

// Autotext:RemoveCC(anything) takes down the caption that shows. The original
// only checks which caption it is when no parameter is given, which no script does.
static int autotextRemoveCC(lua_State *L) {
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (ui) {
		ui->setCaption(Common::String());
	}

	lua_pushboolean(L, true);
	return 1;
}

// Autotext.<key> is the text of that key, or nil when there is none
static int autotextIndex(lua_State *L) {
	indexFunctions(L);
	if (!lua_isnil(L, -1) || lua_type(L, 2) != LUA_TSTRING) {
		return 1;
	}

	lua_pop(L, 1);

	Common::String text;
	if (findText(lua_tostring(L, 2), text)) {
		lua_pushstring(L, text.c_str());
	} else {
		lua_pushnil(L);
	}

	return 1;
}

static const luaL_Reg kAutotextFunctions[] = {
	{ "__index", autotextIndex },
	{ "RemoveCC", autotextRemoveCC },
	{ nullptr, nullptr }
};

// TODO: Autotext:ShowCC, the Generics and UIMgr objects, and Log:WriteAll
void registerGameAPI(lua_State *L) {
	registerGlobal(L, "Flags", "ND.Flags", kFlagsFunctions);
	registerGlobal(L, "VarTable", "ND.VarTable", kVarTableFunctions);

	registerGlobal(L, "Z", "ND.ZHelper", kZFunctions);
	luaL_getmetatable(L, "ND.ZHelper");
	lua_pushnumber(L, 0.0);
	lua_setfield(L, -2, "middle");
	if (g_nancy->getGameType() >= kGameTypeNancy21) {
		// The range z gets kept in
		lua_pushnumber(L, -1.0);
		lua_setfield(L, -2, "min");
		lua_pushnumber(L, 8.9);
		lua_setfield(L, -2, "max");
	}
	lua_pop(L, 1);

	registerType(L, kCursorIndexType, kCursorIndexFunctions);
	registerGlobal(L, "Cursor", "ND.Cursor", kCursorFunctions);

	registerType(L, kSoundChannelType, kSoundChannelFunctions);
	registerGlobal(L, "SoundChannel", "ND.SoundHelper", kSoundChannelHelperFunctions);
	registerGlobal(L, "Sound", "ND.Sound", kSoundFunctions);

	registerGlobal(L, "Scene", "ND.Scene", kSceneFunctions);
	registerGlobal(L, "Autotext", "ND.Autotext", kAutotextFunctions);
	registerGlobal(L, "Streams", "ND.Streams", kStreamsFunctions);
	luaL_getmetatable(L, "ND.Streams");
	lua_pushstring(L, State::SceneStreamManager::getMainStreamName());
	lua_setfield(L, -2, "mainName");
	lua_pop(L, 1);
	registerGlobal(L, "Player", "ND.CurPlayer", kPlayerFunctions);
	registerGlobal(L, "Message", "ND.Message", kMessageFunctions);
	registerGlobal(L, "Viewport", "ND.Viewport", kViewportFunctions);

	registerType(L, kKeyType, kKeyFunctions);
	registerGlobal(L, "Key", "ND.KeyHelper", kKeyHelperFunctions);

	registerGlobal(L, "Game", "ND.Game", kGameFunctions);
	registerGlobal(L, "Draw", "ND.Draw", kDrawFunctions);
	registerGlobal(L, "Save", "ND.Save", kSaveFunctions);
	registerGlobal(L, "Load", "ND.Load", kLoadFunctions);
	registerGlobal(L, "Log", "ND.Log", kLogFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
