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

#ifndef NANCY_SCRIPT_MATHTYPES_H
#define NANCY_SCRIPT_MATHTYPES_H

#include "engines/nancy/script/types.h"

struct lua_State;

namespace Nancy {
namespace Script {

// The vectors and matrices of the scripts. Scripts move what they draw around
// with matrices: every record that shows something has a "transform", and the
// puzzles and the map of Nancy19 are built on changing those.

// Two to four numbers
struct ScriptVector {
	int size = 3;
	double v[4] = { 0.0, 0.0, 0.0, 0.0 };

	ScriptVector() {}
	ScriptVector(double x, double y, double z) { v[0] = x; v[1] = y; v[2] = z; }
};

// Four rows of four. Vectors are rows, so the offset a matrix moves things by is
// its last row, and a product does what its left side does first.
struct ScriptMatrix {
	double m[4][4];

	ScriptMatrix();	// the identity

	bool operator==(const ScriptMatrix &other) const;
	ScriptMatrix operator*(const ScriptMatrix &other) const;

	// A point moved by the matrix
	void transformPoint(double &x, double &y) const;

	// Whether the matrix does nothing but move things
	bool isTranslation() const;
};

void registerMathTypes(lua_State *L);

void pushVector(lua_State *L, const ScriptVector &vector);
ScriptVector *testVector(lua_State *L, int index);

void pushMatrix(lua_State *L, const ScriptMatrix &matrix);
ScriptMatrix *testMatrix(lua_State *L, int index);
ScriptMatrix &checkMatrix(lua_State *L, int index);

// The rectangle between two opposite corners of a rectangle after they were
// moved by a matrix
ScriptRect transformRect(const ScriptRect &rect, const ScriptMatrix &matrix);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_MATHTYPES_H
