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

#ifndef NANCY_SCRIPT_TYPES_H
#define NANCY_SCRIPT_TYPES_H

#include "common/rect.h"
#include "common/str.h"

struct lua_State;

namespace Nancy {
namespace Script {

// The plain values scripts pass around: rectangles, points and colors.
//
// Scripts are free to build rectangles that are empty or inside out, and to move
// them far off the screen, so they are kept as they are and only turned into the
// engine's own type where they get used.

struct ScriptRect {
	int32 left = 0;
	int32 top = 0;
	int32 right = 0;
	int32 bottom = 0;

	ScriptRect() {}
	ScriptRect(int32 l, int32 t, int32 r, int32 b) : left(l), top(t), right(r), bottom(b) {}
	ScriptRect(const Common::Rect &rect) : left(rect.left), top(rect.top), right(rect.right), bottom(rect.bottom) {}

	int32 width() const { return right - left; }
	int32 height() const { return bottom - top; }
	bool isEmpty() const { return right <= left || bottom <= top; }
	bool contains(int32 x, int32 y) const { return x >= left && x < right && y >= top && y < bottom; }
	void translate(int32 dx, int32 dy) { left += dx; right += dx; top += dy; bottom += dy; }
	bool operator==(const ScriptRect &other) const {
		return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
	}

	// An empty or inside out rectangle becomes an empty one
	Common::Rect toRect() const;
};

struct ScriptPoint {
	int32 x = 0;
	int32 y = 0;

	ScriptPoint() {}
	ScriptPoint(int32 px, int32 py) : x(px), y(py) {}
	ScriptPoint(const Common::Point &point) : x(point.x), y(point.y) {}

	Common::Point toPoint() const { return Common::Point(x, y); }
};

void registerTypes(lua_State *L);

void pushRect(lua_State *L, const ScriptRect &rect);
ScriptRect *testRect(lua_State *L, int index);		// null when it is something else
ScriptRect &checkRect(lua_State *L, int index);

void pushPoint(lua_State *L, const ScriptPoint &point);

// Reads anything that has an x and a y: a point, a vector or a table.
// Returns false when the value has neither.
bool readPoint(lua_State *L, int index, ScriptPoint &point);

// Colors are 0xAARRGGBB
void pushColor(lua_State *L, uint32 argb);
uint32 *testColor(lua_State *L, int index);
uint32 checkColor(lua_State *L, int index);

// What a script sends to a scene, or to the records of one that go by a name
struct Message {
	Common::String action;
	Common::String target;
	Common::String sender;
	Common::String text;
	double value = 0;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_TYPES_H
