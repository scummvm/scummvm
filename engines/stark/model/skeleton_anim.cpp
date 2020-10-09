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

#include "engines/stark/model/skeleton_anim.h"

#include "engines/stark/services/archiveloader.h"

namespace Stark {

SkeletonAnim::SkeletonAnim() :
		_id(0),
		_ver(0),
		_u1(0),
		_u2(0),
		_time(0) {
}

void SkeletonAnim::createFromStream(ArchiveReadStream *stream) {
	_id = stream->readUint32LE();
	_ver = stream->readUint32LE();
	if (_ver == 3) {
		_u1 = 0;
		_time = stream->readUint32LE();
		_u2 = stream->readUint32LE();
	} else {
		_u1 = stream->readUint32LE();
		_u2 = stream->readUint32LE();
		_time = stream->readUint32LE();
	}
	if (_u2 != 0xdeadbabe) {
		error("Wrong magic while reading animation");
	}

	uint32 num = stream->readUint32LE();
	_boneAnims.resize(num);
	for (uint32 i = 0; i < num; ++i) {
		uint32 bone = stream->readUint32LE();
		uint32 numKeys = stream->readUint32LE();

		BoneAnim &boneAnim = _boneAnims[bone];
		boneAnim._keys.resize(numKeys);
		for (uint32 j = 0; j < numKeys; ++j) {
			AnimKey &key = boneAnim._keys[j];
			key._time = stream->readUint32LE();
			key._rot = stream->readQuaternion();
			key._pos = stream->readVector3();
		}
	}
}

void SkeletonAnim::getCoordForBone(uint32 time, int boneIdx, Math::Vector3d &pos, Math::Quaternion &rot) const {
	const Common::Array<AnimKey> &keys = _boneAnims[boneIdx]._keys;

	if (keys.size() == 1) {
		// There is only one key for this bone, don't bother searching which one to use
		pos = keys[0]._pos;
		rot = keys[0]._rot;

		return;
	}

	for (Common::Array<AnimKey>::const_iterator it = keys.begin(); it < keys.end(); ++it) {
		if (it->_time > time) {
			// Between two key frames, interpolate
			const AnimKey *a = it;
			--it;
			const AnimKey *b = it;

			float t = (float)(time - b->_time) / (float)(a->_time - b->_time);

			pos = b->_pos + (a->_pos - b->_pos) * t;
			rot = b->_rot.slerpQuat(a->_rot, t);

			return;
		} else if (it->_time == time || it == keys.end() - 1){
			// At a key frame
			// If not right one but didn't find any, then use last one as default
			const AnimKey *key = it;
			pos = key->_pos;
			rot = key->_rot;
			if (it == keys.end() - 1) {
				warning("Unable to find keyframe for bone '%d' at %d ms, using default", boneIdx, time);
			}
			return;
		}
	}
}

} // End of namespace Stark
