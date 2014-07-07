/* ResidualVM - A 3D game interpreter
*
* ResidualVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef GRIM_EMIHEAD_H
#define GRIM_EMIHEAD_H

#include "math/vector3d.h"
#include "math/quat.h"
#include "engines/grim/costume/head.h"

namespace Grim {

class EMICostume;

class EMIHead : public BaseHead {
public:
	EMIHead(EMICostume *costume);

	void setJoint(const char *joint, const Math::Vector3d &offset);
	void setLimits(float yawRange, float maxPitch, float minPitch);
	void lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) override;
	void loadJoints(ModelNode *nodes) override {}
	void saveState(SaveGame *state) const override;
	void restoreState(SaveGame *state) override;

private:
	EMICostume *_cost;
	Common::String _jointName;
	Math::Vector3d _offset;
	Math::Quaternion _headRot;
	float _yawRange;
	float _maxPitch;
	float _minPitch;
};

} // end of namespace Grim

#endif
