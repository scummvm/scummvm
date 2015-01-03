/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_RESOURCES_CAMERA_H
#define STARK_RESOURCES_CAMERA_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "math/vector3d.h"
#include "math/vector4d.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class XRCReadStream;

class Camera : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kCamera;

	Camera(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Camera();

	// Resource API
	void onEnterLocation() override;

	void setClipPlanes(float near, float far);

protected:
	void readData(XRCReadStream *stream) override;
	void printData() override;

	Math::Vector3d _position;
	Math::Vector3d _lookAt;
	float _f1;
	float _fov;
	Common::Rect _viewport;
	Math::Vector3d _v4;

	float _nearClipPlane;
	float _farClipPlane;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_CAMERA_H
