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

#ifndef WATCHMAKER_TYPES3D_H
#define WATCHMAKER_TYPES3D_H

#include "common/stream.h"
#include "math/vector3d.h"
#include "watchmaker/types.h"

namespace Watchmaker {

struct t3dV2F {
	t3dF32  x = 0.0f, y = 0.0f;                           // 2d Vector
public:
	constexpr t3dV2F() = default;
	constexpr t3dV2F(float x, float y, float z) : x(x), y(y) {}
};

struct t3dV3F {
	t3dF32  x = 0.0f, y = 0.0f, z = 0.0f;                        // 3d vector
public:
	constexpr t3dV3F() = default;
	constexpr t3dV3F(float x, float y, float z) : x(x), y(y), z(z) {}
	t3dV3F(const Math::Vector3d &vector) {
		x = vector.x();
		y = vector.y();
		z = vector.z();
	}
	t3dV3F(Common::SeekableReadStream &stream) {
		x = stream.readFloatLE();                                                    // Legge Pos
		y = stream.readFloatLE();
		z = stream.readFloatLE();
	}
	static t3dV3F fromStreamAsBytes(Common::SeekableReadStream &stream) {
		t3dF32 x = stream.readByte();
		t3dF32 y = stream.readByte();
		t3dF32 z = stream.readByte();
		return t3dV3F(x, y, z);
	}
	t3dV3F operator+(const t3dV3F &rhs) const {
		return t3dV3F(
			x + rhs.x,
		    y + rhs.y,
		    z + rhs.z
	    );
	}
	t3dV3F operator-(const t3dV3F &rhs) const {
		return t3dV3F(
		    x + rhs.x,
		    y + rhs.y,
		    z + rhs.z
		);
	}
	t3dV3F operator-() const {
		return t3dV3F(
		    -x,
		    -y,
		    -z
		);
	}
	t3dV3F operator*(float scalar) const {
		return t3dV3F(
		    x * scalar,
		    y * scalar,
		    z * scalar
		);
	}
	t3dV3F &operator*=(float scalar) {
		this->x *= scalar;
		this->y *= scalar;
		this->z *= scalar;
		return *this;
	}
	bool operator==(const t3dV3F &rhs) const {
		return this->x == rhs.x && this->y == rhs.y && this->z == rhs.z;
	}
	bool operator!=(const t3dV3F &rhs) const {
		return !(*this == rhs);
	}
};

struct t3dNORMAL {
	t3dV3F  n;                              //normal coords         12
	t3dF32  tn = 0.0f;                             //normal coords in light space
	t3dF32  dist = 0.0f;                           //dist from plane       4
	t3dF32  tras_n = 0.0f;                         //transformed normal    4
	uint8   flag = 0;                           //flags                 1
public:
	constexpr t3dNORMAL() = default;
	t3dNORMAL(Common::SeekableReadStream &stream) {
		n = t3dV3F(stream); // Direzione
		dist = -stream.readFloatLE(); // Distanza-Dot
	}
};

typedef Common::SharedPtr<t3dNORMAL> NormalPtr;
typedef Common::Array<NormalPtr> NormalList; // TODO: Not necessarily the prettiest solution, but chosen to ensure that changes to copies are shared.

} // End of namespace Watchmaker

#endif // WATCHMAKER_TYPES3D_H
