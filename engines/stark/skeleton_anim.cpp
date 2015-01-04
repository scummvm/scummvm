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

#include "engines/stark/skeleton_anim.h"
#include "engines/stark/skeleton.h"
#include "engines/stark/stark.h"

#include "common/stream.h"

namespace Stark {


SkeletonAnim::SkeletonAnim() {

}

SkeletonAnim::~SkeletonAnim() {
	for (Common::Array<AnimNode *>::iterator it = _anims.begin(); it != _anims.end(); ++it)
		delete *it;
}

void SkeletonAnim::createFromStream(Common::ReadStream *stream) {
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
	_anims.resize(num);
	for (uint32 i = 0; i < num; ++i) {
		AnimNode *node = new AnimNode();
		node->_bone = stream->readUint32LE();
		uint32 numKeys = stream->readUint32LE();

		for (uint32 j = 0; j < numKeys; ++j) {
			AnimKey *key = new AnimKey();
			key->_time = stream->readUint32LE();
			char *ptr = new char[7 * 4];
			stream->read(ptr, 7 * 4);
			key->_rot = Math::Vector3d(get_float(ptr), get_float(ptr + 4), get_float(ptr + 8));
			key->_rotW = get_float(ptr + 12);
			key->_pos = Math::Vector3d(get_float(ptr + 16), get_float(ptr + 20), get_float(ptr + 24));
			node->_keys.push_back(key);
		}

		_anims[node->_bone] = node;
	}
}

Coordinate SkeletonAnim::getCoordForBone(uint32 time, int boneIdx) {
	Coordinate c;

	for (Common::Array<AnimKey *>::iterator it = _anims[boneIdx]->_keys.begin(); it < _anims[boneIdx]->_keys.end(); ++it) {
		if ((*it)->_time == time) {
			AnimKey *key = *it;
			c.setTranslation(key->_pos.x(), key->_pos.y(), key->_pos.z());
			c.setRotation(key->_rotW, key->_rot.x(), key->_rot.y(), key->_rot.z());
			break;

		} else if ((*it)->_time > time) {
			// LERP for the time being - works, though potentially looks odd...
			AnimKey *a = *it;
			--it;
			AnimKey *b = *it;

			float t = (float)(time - b->_time) / (float)(a->_time - b->_time);

			c.setTranslation(b->_pos.x() + (a->_pos.x() - b->_pos.x()) * t, b->_pos.y() + (a->_pos.y() - b->_pos.y()) * t, b->_pos.z() + (a->_pos.z() - b->_pos.z()) * t);
			c.setRotation(b->_rotW + (a->_rotW - b->_rotW) * t, b->_rot.x() + (a->_rot.x() - b->_rot.x()) * t, b->_rot.y()+ (a->_rot.y() - b->_rot.y()) * t, b->_rot.z() + (a->_rot.z() - b->_rot.z()) * t);

			break;
		}
	}

	return c;
}

} // End of namespace Stark
