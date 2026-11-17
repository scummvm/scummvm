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
// The original does its math with D3DX, the math library of the DirectX SDK, and
// the functions here follow its conventions and formulas, e.g. Matrix:Perspective
// is D3DXMatrixPerspectiveFovLH.

// Two to four numbers
struct ScriptVector {
	int size = 3;
	double v[4] = { 0.0, 0.0, 0.0, 0.0 };

	ScriptVector() {}
	ScriptVector(double x, double y, double z) { v[0] = x; v[1] = y; v[2] = z; }
};

// Four rows of four. Vectors are rows, so the offset a matrix moves things by is
// its last row, and a product does what its left side does first.
//
// Nancy32 replaced the matrices with SRTs (scale, rotation, translation) in two
// dimensions. An SRT rotates a point, then scales it, then translates it. Two
// of them combine part by part: the scales multiply, the rotations add up, and
// the second one moves the translation of the first. That only comes out as the
// product of their matrices when they scale the same along both axes, so a
// matrix also keeps the parts it was made from.
struct ScriptMatrix {
	double m[4][4];

	// The parts of the matrix as an SRT. The translation is m[3][0] and m[3][1].
	// A matrix that was filled in some other way isn't an SRT.
	bool isSRT = true;
	double scaleX = 1.0;
	double scaleY = 1.0;
	double rotation = 0.0;	// in radians

	ScriptMatrix();	// the identity

	static ScriptMatrix makeSRT(double scaleX, double scaleY, double rotation, double x, double y);

	bool operator==(const ScriptMatrix &other) const;
	bool operator!=(const ScriptMatrix &other) const { return !(*this == other); }
	ScriptMatrix operator*(const ScriptMatrix &other) const;

	// A point moved by the matrix
	void transformPoint(double &x, double &y) const;

	// Whether the matrix does nothing but move things
	bool isTranslation() const;

	// The matrix that undoes this one; one that can't be undone comes back as it is
	ScriptMatrix inverse() const;
};

void registerMathTypes(lua_State *L);

void pushVector(lua_State *L, const ScriptVector &vector);
bool testVector(lua_State *L, int index, ScriptVector &vector);	// false when it is something else

void pushMatrix(lua_State *L, const ScriptMatrix &matrix);
ScriptMatrix *testMatrix(lua_State *L, int index);
ScriptMatrix &checkMatrix(lua_State *L, int index);

// The rectangle between two opposite corners of a rectangle after they were
// moved by a matrix
ScriptRect transformRect(const ScriptRect &rect, const ScriptMatrix &matrix);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_MATHTYPES_H
