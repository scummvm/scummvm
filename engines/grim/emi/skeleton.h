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

#ifndef GRIM_SKELETON_H
#define GRIM_SKELETON_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "math/mathfwd.h"
#include "math/quat.h"
#include "engines/grim/object.h"
#include "engines/grim/actor.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class AnimationStateEmi;
class AnimationEmi;

struct Joint {
	Common::String _name;
	Common::String _parent;
	Math::Vector3d _trans;
	Math::Quaternion _quat;
	int _parentIndex;
	Math::Matrix4 _absMatrix;
	Math::Matrix4 _relMatrix;
	Math::Matrix4 _animMatrix;
	Math::Quaternion _animQuat;
	Math::Matrix4 _finalMatrix;
	Math::Quaternion _finalQuat;
};

struct JointAnimation {
	Math::Vector3d _pos;
	Math::Quaternion _quat;
	float _transWeight;
	float _rotWeight;
};

struct AnimationLayer {
	JointAnimation* _jointAnims;
};

class Skeleton : public Object {

	void loadSkeleton(Common::SeekableReadStream *data);
	void initBone(int index);
	void initBones();
	void resetAnim();
public:
	// Note: EMI uses priority 5 at most.
	static const int MAX_ANIMATION_LAYERS = 8;

	int _numJoints;
	Joint *_joints;

	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> JointMap;
	JointMap _jointsMap;

	Skeleton(const Common::String &filename, Common::SeekableReadStream *data);
	~Skeleton();
	void animate();
	void commitAnim();
	void addAnimation(AnimationStateEmi *anim);
	void removeAnimation(AnimationStateEmi *anim);
	int findJointIndex(const Common::String &name) const;
	bool hasJoint(const Common::String &name) const;
	Joint *getJointNamed(const Common::String &name) const;
	Joint *getParentJoint(const Joint *j) const;
	int getJointIndex(const Joint *j) const;
	AnimationLayer* getLayer(int priority) const;
private:
	AnimationLayer *_animLayers;
	Common::List<AnimationStateEmi*> _activeAnims;
};

} // end of namespace Grim

#endif
