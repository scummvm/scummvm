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

#ifndef CAMERA_H
#define CAMERA_H

#include "common/serializer.h"
#include "common/rect.h"
#include "math/vector2d.h"
#include "math/vector3d.h"
#include "math/matrix4.h"

namespace Alcachofa {

class Personaje;

static constexpr const int16_t kBaseScale = 300; ///< this number pops up everywhere in the engine
static constexpr const float kInvBaseScale = 1.0f / kBaseScale;

class Camera {
public:
	inline Math::Angle rotation() const { return _rotation; }
	inline Math::Vector2d &shake() { return _shake; }

	void update();
	Math::Vector3d transform2Dto3D(Math::Vector3d v) const;
	Math::Vector3d transform3Dto2D(Math::Vector3d v) const;
	void setRoomBounds(Common::Point bgSize, int16 bgScale);

private:
	static constexpr const float kAccelerationThreshold = 2.89062f;
	static constexpr const float kAcceleration = 3.94922f;

	Math::Vector3d setAppliedCenter(Math::Vector3d center);
	void setupMatricesAround(Math::Vector3d center);

	uint32 _lastUpdateTime = 0;
	float
		_scale = 1.0f,
		_roomScale = 1.0f;
	Math::Angle _rotation;
	Math::Vector2d
		_roomMin = Math::Vector2d(-10000, -10000),
		_roomMax = Math::Vector2d(10000, 10000),
		_shake;
	Math::Vector3d
		_usedCenter = Math::Vector3d(512, 384, 0),
		_appliedCenter;
	Math::Matrix4
		_mat3Dto2D,
		_mat2Dto3D;
};

}

#endif // CAMERA_H
