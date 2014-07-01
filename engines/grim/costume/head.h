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

#ifndef GRIM_HEAD_H
#define GRIM_HEAD_H

#include "math/matrix4.h"

namespace Grim {

class ModelNode;
class SaveGame;

class BaseHead {
public:
	virtual ~BaseHead() {}

	virtual void lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) = 0;
	virtual void saveState(SaveGame *state) const = 0;
	virtual void restoreState(SaveGame *state) = 0;
	virtual void loadJoints(ModelNode *nodes) = 0;
};

class Head : public BaseHead {
public:
	class Joint {
	public:
		Joint();

		void init(ModelNode *node);

		void orientTowards(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix,
						   float maxPitch, float maxYaw, float maxRoll, float constrain);

		void saveState(SaveGame *state) const;
		void restoreState(SaveGame *state);

	private:
		ModelNode *_node;

		Math::Angle _pitch;
		Math::Angle _yaw;
		Math::Angle _roll;
	};

	Head();

	void setJoints(int joint1, int joint2, int joint3);
	void loadJoints(ModelNode *nodes);
	void setMaxAngles(float maxPitch, float maxYaw, float maxRoll);

	void lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix);

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

	int getJoint1() const { return _joint1Node; }
	int getJoint2() const { return _joint2Node; }
	int getJoint3() const { return _joint3Node; }

private:
	int _joint1Node;
	int _joint2Node;
	int _joint3Node;
	float _maxRoll;
	float _maxPitch;
	float _maxYaw;

	// Specifies the three head joint bones of this character.
	// These joint bones are animated by the moveHead function to make
	// the characters face different directions.
	// Note that for some characters, these variables may all be equal.
	Joint _joint1;
	Joint _joint2;
	Joint _joint3;
};

} // end of namespace Grim

#endif
