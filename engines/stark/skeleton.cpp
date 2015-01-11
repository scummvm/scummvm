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

#include "engines/stark/skeleton.h"

#include "engines/stark/gfx/coordinate.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/skeleton_anim.h"
#include "engines/stark/stark.h"

#include "common/stream.h"

namespace Stark {

Skeleton::Skeleton() :
		_anim(nullptr),
		_lastTime(-1) {

}

Skeleton::~Skeleton() {
	for (Common::Array<BoneNode *>::iterator it = _bones.begin(); it != _bones.end(); ++it)
		delete *it;
}

void Skeleton::readFromStream(ArchiveReadStream *stream) {
	uint32 numBones = stream->readUint32LE();
	for (uint32 i = 0; i < numBones; ++i) {
		BoneNode *node = new BoneNode();
		node->_name = stream->readString();
		node->_u1 = stream->readFloat();

		uint32 len = stream->readUint32LE();
		for (uint32 j = 0; j < len; ++j)
			node->_children.push_back(stream->readUint32LE());

		node->_idx = _bones.size();
		_bones.push_back(node);
	}

	for (uint32 i = 0; i < numBones; ++i) {
		BoneNode *node = _bones[i];
		for (uint j = 0; j < node->_children.size(); ++j) {
			_bones[node->_children[j]]->_parent = i;
		}
	}
}

void Skeleton::setAnim(SkeletonAnim *anim) {
	_anim = anim;
}

void Skeleton::setNode(uint32 time, BoneNode *bone, const Coordinate &parentCoord) {
	Coordinate animCoord = _anim->getCoordForBone(time, bone->_idx);
	animCoord.rotate(parentCoord);
	bone->_animPos = parentCoord + animCoord;

	for (uint i = 0; i < bone->_children.size(); ++i) {
		setNode(time, _bones[bone->_children[i]], bone->_animPos);
	}
}

void Skeleton::animate(uint32 time) {
	// Start at root bone
	// For each child
	//  - Set childs animation coordinate
	//  - Process that childs children

	if (time != _lastTime) {
		setNode(time, _bones[0], Coordinate());
		_lastTime = time;
	}
}

} // End of namespace Stark
