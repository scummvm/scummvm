/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_ANIMATIONEMI_H
#define GRIM_ANIMATIONEMI_H

#include "common/str.h"
#include "math/mathfwd.h"
#include "math/quat.h"
#include "engines/grim/object.h"

namespace Grim {

struct AnimRotation {
	Math::Quaternion _quat;
	float _time;
};

struct AnimTranslation {
	Math::Vector3d _vec;
	float _time;
};

struct Bone {
	Common::String _boneName;
	int _operation;
	int _b;
	int _c;
	int _count;
	AnimRotation **_rotations;
	AnimTranslation **_translations;
	Bone() : _rotations(NULL), _translations(NULL), _boneName(""), _operation(0) {}
	~Bone();
};

class AnimationEmi : public Object {
	void loadAnimation(Common::SeekableReadStream *data);
public:
	Common::String _name;
	float _duration;
	int _numBones;
	Bone **_bones;
	AnimationEmi(const Common::String &filename, Common::SeekableReadStream *data) : _name(""), _duration(0.0f), _numBones(0), _bones(NULL) { loadAnimation(data); }
	~AnimationEmi();
};

} // end of namespace Grim

#endif