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

#ifndef STARK_RESOURCES_LIGHT_H
#define STARK_RESOURCES_LIGHT_H

#include "common/str.h"

#include "math/vector3d.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Gfx {
struct LightEntry;
}

namespace Resources {

/**
 * A light source
 */
class Light : public Object {
public:
	static const Type::ResourceType TYPE = Type::kLight;

	Light(Object *parent, byte subType, uint16 index, const Common::String &name);
	~Light() override;

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onPostRead() override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Get the rendering object used to represent this light */
	Gfx::LightEntry *getLightEntry();

	/** Change this light's diffuse color */
	void setColor(int32 red, int32 green, int32 blue);

	/** Change this light's position, in world coordinates */
	void setPosition(const Math::Vector3d &position);

protected:
	void printData() override;

	Math::Vector3d _color;
	Math::Vector3d _position;
	Math::Vector3d _direction;
	float _innerConeAngle;
	float _outerConeAngle;
	float _falloffNear;
	float _falloffFar;
	float _multiplier;

	Gfx::LightEntry *_lightEntry;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_LIGHT_H
