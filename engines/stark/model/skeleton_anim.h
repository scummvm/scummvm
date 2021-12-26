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

#ifndef STARK_MODEL_SKELETON_ANIM_H
#define STARK_MODEL_SKELETON_ANIM_H

#include "math/quat.h"
#include "math/vector3d.h"
#include "common/array.h"

namespace Stark {

class ArchiveReadStream;

/**
 * Data structure responsible for skeletal animation of an actor object.
 */
class SkeletonAnim {
public:
	SkeletonAnim();

	void createFromStream(ArchiveReadStream *stream);

	/**
	 * Get the interpolated bone coordinate for a given bone at a given animation timestamp
	 */
	void getCoordForBone(uint32 time, int boneIdx, Math::Vector3d &pos, Math::Quaternion &rot) const;

	/**
	 * Get total animation length (in ms)
	 */
	uint32 getLength() const { return _time; }

	/** The number of bones a skeleton must have to play this animation */
	uint32 getBoneCount() const { return _boneAnims.size(); }

private:
	struct AnimKey {
		uint32 _time;
		Math::Quaternion _rot;
		Math::Vector3d _pos;
	};

	struct BoneAnim {
		Common::Array<AnimKey> _keys;
	};

	uint32 _id, _ver, _u1, _u2, _time;

	Common::Array<BoneAnim> _boneAnims;
};

} // End of namespace Stark

#endif // STARK_MODEL_SKELETON_ANIM_H
