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

#ifndef STARK_RESOURCES_FLOOR_FACE_H
#define STARK_RESOURCES_FLOOR_FACE_H

#include "common/array.h"
#include "common/str.h"

#include "math/vector3d.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class XRCReadStream;

class FloorFace : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kFloorFace;

	FloorFace(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~FloorFace();

	// Resource API
	virtual void onAllLoaded() override;

	/** Return true if the point is inside the face when both are projected on a Z=0 plane*/
	bool isPointInside(const Math::Vector3d &point) const;

	/** Fill the z coordinate of the point so that it is on the plane */
	void computePointHeight(Math::Vector3d &point) const;

protected:
	void readData(XRCReadStream *stream) override;
	void printData() override;

	int16 _indices[3];
	Math::Vector3d _vertices[3];

	float _unk1;
	float _unk2;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_FLOOR_FACE_H
