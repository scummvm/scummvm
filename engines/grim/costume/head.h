/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_HEAD_H
#define GRIM_HEAD_H

#include "math/matrix4.h"

namespace Grim {

class ModelNode;
class SaveGame;

class Head {
public:
	Head();

	void setJoints(int joint1, int joint2, int joint3);
	void loadJoints(ModelNode *nodes);
	void setMaxAngles(float maxPitch, float maxYaw, float maxRoll);

	void lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix);

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

private:
	int _joint1;
	int _joint2;
	int _joint3;
	float _maxRoll;
	float _maxPitch;
	float _maxYaw;

	ModelNode *_joint1Node;
	ModelNode *_joint2Node;
	ModelNode *_joint3Node;

	Math::Angle _headPitch;
	Math::Angle _headYaw;
};

} // end of namespace Grim

#endif
