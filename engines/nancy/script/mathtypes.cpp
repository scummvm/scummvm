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

#include "common/scummsys.h"
#include "common/util.h"

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/mathtypes.h"

namespace Nancy {
namespace Script {

static const char *const kVectorType = "ND.Vector";
static const char *const kVectorHelperType = "ND.VectorHelper";
static const char *const kMatrixType = "ND.Matrix";
static const char *const kMatrixHelperType = "ND.MatrixHelper";
static const char *const kQuaternionType = "ND.Quaternion";
static const char *const kQuaternionHelperType = "ND.QuaternionHelper";

// Nancy20. A rotation, kept in single precision as the DirectX math of the
// original does
struct ScriptQuaternion {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;
};

ScriptMatrix::ScriptMatrix() {
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			m[row][column] = row == column ? 1.0 : 0.0;
		}
	}
}

bool ScriptMatrix::operator==(const ScriptMatrix &other) const {
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			if (m[row][column] != other.m[row][column]) {
				return false;
			}
		}
	}

	return true;
}

ScriptMatrix ScriptMatrix::operator*(const ScriptMatrix &other) const {
	ScriptMatrix result;

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			double sum = 0.0;
			for (int i = 0; i < 4; ++i) {
				sum += m[row][i] * other.m[i][column];
			}

			result.m[row][column] = sum;
		}
	}

	return result;
}

// Moves a vector by a matrix; w is 1 for a place and 0 for a direction
static void transformVector(const ScriptMatrix &matrix, const double *in, double w, double *out) {
	for (int column = 0; column < 4; ++column) {
		out[column] = in[0] * matrix.m[0][column] + in[1] * matrix.m[1][column] +
					  in[2] * matrix.m[2][column] + w * matrix.m[3][column];
	}
}

void ScriptMatrix::transformPoint(double &x, double &y) const {
	const double in[3] = { x, y, 0.0 };
	double out[4];
	transformVector(*this, in, 1.0, out);

	if (out[3] != 0.0 && out[3] != 1.0) {
		out[0] /= out[3];
		out[1] /= out[3];
	}

	x = out[0];
	y = out[1];
}

bool ScriptMatrix::isTranslation() const {
	ScriptMatrix withoutOffset = *this;
	withoutOffset.m[3][0] = 0.0;
	withoutOffset.m[3][1] = 0.0;
	withoutOffset.m[3][2] = 0.0;
	return withoutOffset == ScriptMatrix();
}

ScriptRect transformRect(const ScriptRect &rect, const ScriptMatrix &matrix) {
	double left = rect.left;
	double top = rect.top;
	double right = rect.right;
	double bottom = rect.bottom;

	matrix.transformPoint(left, top);
	matrix.transformPoint(right, bottom);

	return ScriptRect((int32)left, (int32)top, (int32)right, (int32)bottom);
}

// Vector

void pushVector(lua_State *L, const ScriptVector &vector) {
	pushValue(L, kVectorType, vector);
}

ScriptVector *testVector(lua_State *L, int index) {
	return (ScriptVector *)testUserData(L, index, kVectorType);
}

static ScriptVector &checkVector(lua_State *L, int index) {
	return *(ScriptVector *)checkUserData(L, index, kVectorType);
}

// Vector:New(x, y[, z[, w]]), or Vector:New(vector or place) in Nancy20
static int vectorNew(lua_State *L) {
	ScriptVector vector;
	vector.size = lua_gettop(L) - 1;
	if (vector.size == 1) {
		// A copy of a vector, or a place made into a 2D vector
		const ScriptVector *other = testVector(L, 2);
		if (other) {
			pushVector(L, *other);
			return 1;
		}

		lua_getfield(L, 2, "x");
		lua_getfield(L, 2, "y");
		if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
			return luaL_argerror(L, 2, "Single parameter must be a Vector, a Point or a table with x and y fields");
		}

		vector.size = 2;
		vector.v[0] = lua_tonumber(L, -2);
		vector.v[1] = lua_tonumber(L, -1);
		pushVector(L, vector);
		return 1;
	}

	if (vector.size < 2 || vector.size > 4) {
		return luaL_error(L, "Only 2D, 3D and 4D vectors are supported");
	}

	for (int i = 0; i < vector.size; ++i) {
		vector.v[i] = luaL_checknumber(L, i + 2);
	}

	pushVector(L, vector);
	return 1;
}

// The component a key stands for: a number from 1, or one of the letters of a
// place or a color. Returns -1 for anything else.
static int getVectorComponent(lua_State *L, int keyIndex) {
	if (lua_type(L, keyIndex) == LUA_TNUMBER) {
		const int component = (int)lua_tointeger(L, keyIndex) - 1;
		return component >= 0 && component < 4 ? component : -1;
	}

	if (lua_type(L, keyIndex) != LUA_TSTRING) {
		return -1;
	}

	static const char *const kNames[4][2] = { { "x", "r" }, { "y", "g" }, { "z", "b" }, { "w", "a" } };

	const char *key = lua_tostring(L, keyIndex);
	for (int i = 0; i < 4; ++i) {
		if (strcmp(key, kNames[i][0]) == 0 || strcmp(key, kNames[i][1]) == 0) {
			return i;
		}
	}

	return -1;
}

static double getVectorLength(const ScriptVector &vector) {
	double sum = 0.0;
	for (int i = 0; i < vector.size; ++i) {
		sum += vector.v[i] * vector.v[i];
	}

	return sqrt(sum);
}

static int vectorIndex(lua_State *L) {
	const ScriptVector &vector = checkVector(L, 1);

	if (lua_type(L, 2) == LUA_TSTRING) {
		const char *key = lua_tostring(L, 2);
		if (strcmp(key, "len") == 0 || strcmp(key, "length") == 0) {
			lua_pushnumber(L, getVectorLength(vector));
			return 1;
		}

		if (strcmp(key, "lenSq") == 0 || strcmp(key, "lengthSq") == 0) {
			const double length = getVectorLength(vector);
			lua_pushnumber(L, length * length);
			return 1;
		}
	}

	const int component = getVectorComponent(L, 2);
	if (component >= 0) {
		if (component >= vector.size) {
			return luaL_error(L, "Vector does not have field requested");
		}

		lua_pushnumber(L, vector.v[component]);
		return 1;
	}

	return indexFunctions(L);
}

static int vectorNewIndex(lua_State *L) {
	ScriptVector &vector = checkVector(L, 1);

	const int component = getVectorComponent(L, 2);
	if (component < 0 || component >= vector.size) {
		return luaL_error(L, "Vector does not have field requested");
	}

	vector.v[component] = luaL_checknumber(L, 3);
	return 0;
}

static void checkSameSize(lua_State *L, const ScriptVector &a, const ScriptVector &b) {
	if (a.size != b.size) {
		luaL_error(L, "Vector 1 has %d dimensions but vector 2 has %d dimensions", a.size, b.size);
	}
}

static int vectorAdd(lua_State *L) {
	ScriptVector result = checkVector(L, 1);
	const ScriptVector &other = checkVector(L, 2);
	checkSameSize(L, result, other);

	for (int i = 0; i < result.size; ++i) {
		result.v[i] += other.v[i];
	}

	pushVector(L, result);
	return 1;
}

static int vectorSubtract(lua_State *L) {
	ScriptVector result = checkVector(L, 1);
	const ScriptVector &other = checkVector(L, 2);
	checkSameSize(L, result, other);

	for (int i = 0; i < result.size; ++i) {
		result.v[i] -= other.v[i];
	}

	pushVector(L, result);
	return 1;
}

// A vector times a number, either way around, or two vectors part by part
static int vectorMultiply(lua_State *L) {
	const ScriptVector *left = testVector(L, 1);
	const ScriptVector *right = testVector(L, 2);

	ScriptVector result;
	if (left && right) {
		checkSameSize(L, *left, *right);
		result = *left;
		for (int i = 0; i < result.size; ++i) {
			result.v[i] *= right->v[i];
		}
	} else {
		result = left ? *left : checkVector(L, 2);
		const double factor = luaL_checknumber(L, left ? 2 : 1);
		for (int i = 0; i < result.size; ++i) {
			result.v[i] *= factor;
		}
	}

	pushVector(L, result);
	return 1;
}

static int vectorDivide(lua_State *L) {
	if (!testVector(L, 1)) {
		return luaL_error(L, "Cannot divide a scalar by a vector");
	}

	ScriptVector result = checkVector(L, 1);
	const ScriptVector *right = testVector(L, 2);

	if (right) {
		checkSameSize(L, result, *right);
		for (int i = 0; i < result.size; ++i) {
			result.v[i] /= right->v[i];
		}
	} else {
		const double divisor = luaL_checknumber(L, 2);
		for (int i = 0; i < result.size; ++i) {
			result.v[i] /= divisor;
		}
	}

	pushVector(L, result);
	return 1;
}

static int vectorNegate(lua_State *L) {
	ScriptVector result = checkVector(L, 1);
	for (int i = 0; i < result.size; ++i) {
		result.v[i] = -result.v[i];
	}

	pushVector(L, result);
	return 1;
}

static int vectorDot(lua_State *L) {
	const ScriptVector &a = checkVector(L, 1);
	const ScriptVector &b = checkVector(L, 2);
	checkSameSize(L, a, b);

	double sum = 0.0;
	for (int i = 0; i < a.size; ++i) {
		sum += a.v[i] * b.v[i];
	}

	lua_pushnumber(L, sum);
	return 1;
}

static int vectorCross(lua_State *L) {
	const ScriptVector &a = checkVector(L, 1);
	const ScriptVector &b = checkVector(L, 2);
	if (a.size != 3 || b.size != 3) {
		return luaL_error(L, "Both vectors must be 3D");
	}

	pushVector(L, ScriptVector(a.v[1] * b.v[2] - a.v[2] * b.v[1],
							   a.v[2] * b.v[0] - a.v[0] * b.v[2],
							   a.v[0] * b.v[1] - a.v[1] * b.v[0]));
	return 1;
}

// The vector made one unit long
static int vectorNormalize(lua_State *L) {
	ScriptVector result = checkVector(L, 1);

	const double length = getVectorLength(result);
	if (length != 0.0) {
		for (int i = 0; i < result.size; ++i) {
			result.v[i] /= length;
		}
	}

	pushVector(L, result);
	return 1;
}

// vector:Lerp(other, amount)
static int vectorLerp(lua_State *L) {
	ScriptVector result = checkVector(L, 1);
	const ScriptVector &other = checkVector(L, 2);
	const double amount = luaL_checknumber(L, 3);
	checkSameSize(L, result, other);

	for (int i = 0; i < result.size; ++i) {
		result.v[i] += (other.v[i] - result.v[i]) * amount;
	}

	pushVector(L, result);
	return 1;
}

// vector:Reflect(normal), the vector bounced off a surface facing that way
static int vectorReflect(lua_State *L) {
	ScriptVector result = checkVector(L, 1);
	const ScriptVector &normal = checkVector(L, 2);
	checkSameSize(L, result, normal);

	double dot = 0.0;
	for (int i = 0; i < result.size; ++i) {
		dot += result.v[i] * normal.v[i];
	}

	for (int i = 0; i < result.size; ++i) {
		result.v[i] -= 2.0 * dot * normal.v[i];
	}

	pushVector(L, result);
	return 1;
}

// Moves a vector by a matrix, as a place ("coord") or as a direction ("normal")
static int pushTransformedVector(lua_State *L, const ScriptVector &vector, const ScriptMatrix &matrix, const char *kind) {
	bool isPlace;
	if (strcmp(kind, "coord") == 0) {
		isPlace = true;
	} else if (strcmp(kind, "normal") == 0) {
		isPlace = false;
	} else {
		return luaL_error(L, "Transform type must be coord or normal");
	}

	if (vector.size < 3) {
		return luaL_error(L, "Vector transform must be on a 3D or 4D vector");
	}

	ScriptVector result = vector;
	double out[4];
	transformVector(matrix, vector.v, vector.size == 4 ? vector.v[3] : (isPlace ? 1.0 : 0.0), out);

	if (vector.size == 3 && isPlace && out[3] != 0.0) {
		for (int i = 0; i < 3; ++i) {
			out[i] /= out[3];
		}
	}

	for (int i = 0; i < result.size; ++i) {
		result.v[i] = out[i];
	}

	pushVector(L, result);
	return 1;
}

// vector:Transform(kind, matrix)
static int vectorTransform(lua_State *L) {
	const ScriptVector &vector = checkVector(L, 1);
	const char *kind = luaL_checkstring(L, 2);
	return pushTransformedVector(L, vector, checkMatrix(L, 3), kind);
}

static const luaL_Reg kVectorFunctions[] = {
	{ "__index", vectorIndex },
	{ "__newindex", vectorNewIndex },
	{ "__add", vectorAdd },
	{ "__sub", vectorSubtract },
	{ "__mul", vectorMultiply },
	{ "__div", vectorDivide },
	{ "__unm", vectorNegate },
	{ "Dot", vectorDot },
	{ "Cross", vectorCross },
	{ "Norm", vectorNormalize },
	{ "Normalize", vectorNormalize },
	{ "Lerp", vectorLerp },
	{ "Reflect", vectorReflect },
	{ "Transform", vectorTransform },
	{ nullptr, nullptr }
};

// Vector.origin is a new vector every time, since scripts can change vectors
static int vectorHelperIndex(lua_State *L) {
	if (lua_type(L, 2) == LUA_TSTRING && strcmp(lua_tostring(L, 2), "origin") == 0) {
		pushVector(L, ScriptVector(0.0, 0.0, 0.0));
		return 1;
	}

	return indexFunctions(L);
}

static const luaL_Reg kVectorHelperFunctions[] = {
	{ "__index", vectorHelperIndex },
	{ "New", vectorNew },
	{ nullptr, nullptr }
};

// Matrix

void pushMatrix(lua_State *L, const ScriptMatrix &matrix) {
	pushValue(L, kMatrixType, matrix);
}

ScriptMatrix *testMatrix(lua_State *L, int index) {
	return (ScriptMatrix *)testUserData(L, index, kMatrixType);
}

ScriptMatrix &checkMatrix(lua_State *L, int index) {
	return *(ScriptMatrix *)checkUserData(L, index, kMatrixType);
}

// Matrix:New(sixteen numbers, row by row)
static int matrixNew(lua_State *L) {
	ScriptMatrix matrix;
	for (int i = 0; i < 16; ++i) {
		matrix.m[i / 4][i % 4] = luaL_checknumber(L, i + 2);
	}

	pushMatrix(L, matrix);
	return 1;
}

// Matrix:Translate(x, y, z)
static int matrixTranslate(lua_State *L) {
	ScriptMatrix matrix;
	matrix.m[3][0] = luaL_checknumber(L, 2);
	matrix.m[3][1] = luaL_checknumber(L, 3);
	matrix.m[3][2] = luaL_checknumber(L, 4);
	pushMatrix(L, matrix);
	return 1;
}

// Matrix:Perspective(fovy, aspectRatio, zNear, zFar), Nancy21: the projection of a
// camera looking down the z axis, for the particles
static int matrixPerspective(lua_State *L) {
	const double fovy = luaL_checknumber(L, 2);
	const double aspectRatio = luaL_checknumber(L, 3);
	const double zNear = luaL_checknumber(L, 4);
	const double zFar = luaL_checknumber(L, 5);

	const double yScale = 1.0 / tan(fovy / 2.0);

	ScriptMatrix matrix;
	matrix.m[0][0] = yScale / aspectRatio;
	matrix.m[1][1] = yScale;
	matrix.m[2][2] = zFar / (zFar - zNear);
	matrix.m[2][3] = 1.0;
	matrix.m[3][2] = -zNear * zFar / (zFar - zNear);
	matrix.m[3][3] = 0.0;
	pushMatrix(L, matrix);
	return 1;
}

// Matrix:Scale(x, y, z)
static int matrixScale(lua_State *L) {
	ScriptMatrix matrix;
	matrix.m[0][0] = luaL_checknumber(L, 2);
	matrix.m[1][1] = luaL_checknumber(L, 3);
	matrix.m[2][2] = luaL_checknumber(L, 4);
	pushMatrix(L, matrix);
	return 1;
}

// Matrix:RotateAngle(axis, angle), with the axis one of "x", "y" and "z"
// and the angle in radians
static int matrixRotateAngle(lua_State *L) {
	const char *axis = luaL_checkstring(L, 2);
	const double angle = luaL_checknumber(L, 3);
	const double sine = sin(angle);
	const double cosine = cos(angle);

	// The two axes the rotation turns into each other
	int first, second;
	if (scumm_stricmp(axis, "x") == 0) {
		first = 1;
		second = 2;
	} else if (scumm_stricmp(axis, "y") == 0) {
		first = 2;
		second = 0;
	} else if (scumm_stricmp(axis, "z") == 0) {
		first = 0;
		second = 1;
	} else {
		return luaL_error(L, "Axis must be x, y or z");
	}

	ScriptMatrix matrix;
	matrix.m[first][first] = cosine;
	matrix.m[first][second] = sine;
	matrix.m[second][first] = -sine;
	matrix.m[second][second] = cosine;
	pushMatrix(L, matrix);
	return 1;
}

// A rotation around the vector v, which RotateAxis and RotateQuaternion both
// come down to: a multiple of v times itself, a number on the diagonal, and the
// turn, which goes in the same direction as RotateAngle
static ScriptMatrix makeRotation(const double v[3], double outer, double diagonal, double turn) {
	ScriptMatrix matrix;
	for (int row = 0; row < 3; ++row) {
		for (int column = 0; column < 3; ++column) {
			matrix.m[row][column] = outer * v[row] * v[column];
		}

		matrix.m[row][row] += diagonal;
	}

	matrix.m[0][1] += turn * v[2];
	matrix.m[1][0] -= turn * v[2];
	matrix.m[2][0] += turn * v[1];
	matrix.m[0][2] -= turn * v[1];
	matrix.m[1][2] += turn * v[0];
	matrix.m[2][1] -= turn * v[0];
	return matrix;
}

// Matrix:RotateAxis(axis, angle), a turn by the angle in radians around a 3D vector
static int matrixRotateAxis(lua_State *L) {
	const ScriptVector *axis = testVector(L, 2);
	if (!axis || axis->size != 3) {
		return luaL_argerror(L, 2, "Axis must be a 3D vector");
	}

	const double angle = luaL_checknumber(L, 3);

	const double length = getVectorLength(*axis);
	double n[3];
	for (int i = 0; i < 3; ++i) {
		n[i] = length != 0.0 ? axis->v[i] / length : axis->v[i];
	}

	pushMatrix(L, makeRotation(n, 1.0 - cos(angle), cos(angle), sin(angle)));
	return 1;
}

// Matrix:RotateQuaternion(quaternion)
static int matrixRotateQuaternion(lua_State *L) {
	const ScriptQuaternion &q = *(ScriptQuaternion *)checkUserData(L, 2, kQuaternionType);

	const double v[3] = { q.x, q.y, q.z };
	const double lengthSquared = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	pushMatrix(L, makeRotation(v, 2.0, 1.0 - 2.0 * lengthSquared, 2.0 * q.w));
	return 1;
}

// A row of the matrix, as a vector
static int matrixIndex(lua_State *L) {
	const ScriptMatrix &matrix = checkMatrix(L, 1);

	if (lua_type(L, 2) != LUA_TNUMBER) {
		return indexFunctions(L);
	}

	const int row = (int)lua_tointeger(L, 2) - 1;
	if (row < 0 || row >= 4) {
		return luaL_error(L, "Matrix does not have field %d", row + 1);
	}

	ScriptVector vector;
	vector.size = 4;
	for (int i = 0; i < 4; ++i) {
		vector.v[i] = matrix.m[row][i];
	}

	pushVector(L, vector);
	return 1;
}

static int matrixNewIndex(lua_State *L) {
	ScriptMatrix &matrix = checkMatrix(L, 1);

	const int row = luaL_checkint(L, 2) - 1;
	if (row < 0 || row >= 4) {
		return luaL_error(L, "Matrix does not have field %d", row + 1);
	}

	const ScriptVector &vector = checkVector(L, 3);
	if (vector.size != 4) {
		return luaL_error(L, "Matrix rows are 4D not %dD", vector.size);
	}

	for (int i = 0; i < 4; ++i) {
		matrix.m[row][i] = vector.v[i];
	}

	return 0;
}

static int matrixMultiply(lua_State *L) {
	pushMatrix(L, checkMatrix(L, 1) * checkMatrix(L, 2));
	return 1;
}

static int matrixEquals(lua_State *L) {
	lua_pushboolean(L, checkMatrix(L, 1) == checkMatrix(L, 2));
	return 1;
}

static int matrixTranspose(lua_State *L) {
	const ScriptMatrix &matrix = checkMatrix(L, 1);

	ScriptMatrix result;
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			result.m[row][column] = matrix.m[column][row];
		}
	}

	pushMatrix(L, result);
	return 1;
}

ScriptMatrix ScriptMatrix::inverse() const {
	ScriptMatrix work = *this;
	ScriptMatrix result;

	for (int column = 0; column < 4; ++column) {
		// The row with the largest value in this column goes on the diagonal
		int pivot = column;
		for (int row = column + 1; row < 4; ++row) {
			if (fabs(work.m[row][column]) > fabs(work.m[pivot][column])) {
				pivot = row;
			}
		}

		if (work.m[pivot][column] == 0.0) {
			return *this;
		}

		for (int i = 0; i < 4; ++i) {
			SWAP(work.m[column][i], work.m[pivot][i]);
			SWAP(result.m[column][i], result.m[pivot][i]);
		}

		const double divisor = work.m[column][column];
		for (int i = 0; i < 4; ++i) {
			work.m[column][i] /= divisor;
			result.m[column][i] /= divisor;
		}

		for (int row = 0; row < 4; ++row) {
			if (row == column) {
				continue;
			}

			const double factor = work.m[row][column];
			for (int i = 0; i < 4; ++i) {
				work.m[row][i] -= factor * work.m[column][i];
				result.m[row][i] -= factor * result.m[column][i];
			}
		}
	}

	return result;
}

static int matrixInverse(lua_State *L) {
	pushMatrix(L, checkMatrix(L, 1).inverse());
	return 1;
}

// matrix:Transform(kind, vector)
static int matrixTransform(lua_State *L) {
	const ScriptMatrix &matrix = checkMatrix(L, 1);
	const char *kind = luaL_checkstring(L, 2);
	return pushTransformedVector(L, checkVector(L, 3), matrix, kind);
}

static const luaL_Reg kMatrixFunctions[] = {
	{ "__index", matrixIndex },
	{ "__newindex", matrixNewIndex },
	{ "__mul", matrixMultiply },
	{ "__eq", matrixEquals },
	{ "Inverse", matrixInverse },
	{ "Transpose", matrixTranspose },
	{ "Transform", matrixTransform },
	{ nullptr, nullptr }
};

// Matrix.identity is a new matrix every time, since scripts can change matrices
static int matrixHelperIndex(lua_State *L) {
	if (lua_type(L, 2) == LUA_TSTRING && strcmp(lua_tostring(L, 2), "identity") == 0) {
		pushMatrix(L, ScriptMatrix());
		return 1;
	}

	return indexFunctions(L);
}

static const luaL_Reg kMatrixHelperFunctions[] = {
	{ "__index", matrixHelperIndex },
	{ "New", matrixNew },
	{ "Translation", matrixTranslate },
	{ "Translate", matrixTranslate },
	{ "Scaling", matrixScale },
	{ "Scale", matrixScale },
	{ "Perspective", matrixPerspective },
	{ "RotationAngle", matrixRotateAngle },
	{ "RotateAngle", matrixRotateAngle },
	{ "RotationAxis", matrixRotateAxis },
	{ "RotateAxis", matrixRotateAxis },
	{ "RotationQuaternion", matrixRotateQuaternion },
	{ "RotateQuaternion", matrixRotateQuaternion },
	{ nullptr, nullptr }
};

// Quaternion, which the puzzles of Nancy20 blend rotations with

static void pushQuaternion(lua_State *L, const ScriptQuaternion &quaternion) {
	pushValue(L, kQuaternionType, quaternion);
}

static const ScriptQuaternion &checkQuaternion(lua_State *L, int index) {
	return *(ScriptQuaternion *)checkUserData(L, index, kQuaternionType);
}

// Quaternion:New(quaternion), a copy
static int quaternionNew(lua_State *L) {
	pushQuaternion(L, checkQuaternion(L, 2));
	return 1;
}

// Quaternion:RotateZ(angle), a turn by the angle in radians around the z axis
static int quaternionRotateZ(lua_State *L) {
	const float angle = (float)luaL_checknumber(L, 2);

	ScriptQuaternion quaternion;
	quaternion.z = sinf(angle / 2.0f);
	quaternion.w = cosf(angle / 2.0f);
	pushQuaternion(L, quaternion);
	return 1;
}

// quaternion:Slerp(other, amount), the rotation that far along the way to the
// other one, going the short way around
static int quaternionSlerp(lua_State *L) {
	const ScriptQuaternion &a = checkQuaternion(L, 1);
	const ScriptQuaternion &b = checkQuaternion(L, 2);
	float amount = (float)luaL_checknumber(L, 3);
	float rest = 1.0f - amount;

	float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	if (dot < 0.0f) {
		amount = -amount;
		dot = -dot;
	}

	if (1.0f - dot > 0.001f) {
		const float theta = acosf(dot);
		rest = sinf(theta * rest) / sinf(theta);
		amount = sinf(theta * amount) / sinf(theta);
	}

	ScriptQuaternion result;
	result.x = rest * a.x + amount * b.x;
	result.y = rest * a.y + amount * b.y;
	result.z = rest * a.z + amount * b.z;
	result.w = rest * a.w + amount * b.w;
	pushQuaternion(L, result);
	return 1;
}

static const luaL_Reg kQuaternionFunctions[] = {
	{ "__index", indexFunctions },
	{ "Slerp", quaternionSlerp },
	{ nullptr, nullptr }
};

static const luaL_Reg kQuaternionHelperFunctions[] = {
	{ "__index", indexFunctions },
	{ "New", quaternionNew },
	{ "RotationZ", quaternionRotateZ },
	{ "RotateZ", quaternionRotateZ },
	{ nullptr, nullptr }
};

// rect:Transform(matrix)
static int rectTransform(lua_State *L) {
	pushRect(L, transformRect(checkRect(L, 1), checkMatrix(L, 2)));
	return 1;
}

void registerMathTypes(lua_State *L) {
	registerType(L, kVectorType, kVectorFunctions);
	registerGlobal(L, "Vector", kVectorHelperType, kVectorHelperFunctions);

	registerType(L, kMatrixType, kMatrixFunctions);
	registerGlobal(L, "Matrix", kMatrixHelperType, kMatrixHelperFunctions);

	registerType(L, kQuaternionType, kQuaternionFunctions);
	registerGlobal(L, "Quaternion", kQuaternionHelperType, kQuaternionHelperFunctions);

	// Rectangles can be moved by a matrix as well
	luaL_getmetatable(L, "ND.Rect");
	lua_pushcfunction(L, rectTransform);
	lua_setfield(L, -2, "Transform");
	lua_pop(L, 1);
}

} // End of namespace Script
} // End of namespace Nancy
