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

#include "engines/stark/model/animhandler.h"

#include "engines/stark/model/model.h"
#include "engines/stark/model/skeleton_anim.h"

namespace Stark {

AnimHandler::AnimHandler() :
		_anim(nullptr),
		_model(nullptr),
		_lastTime(-1) {

}

AnimHandler::~AnimHandler() {
}

void AnimHandler::setAnim(SkeletonAnim *anim) {
	_anim = anim;
}

void AnimHandler::setModel(Model *model) {
	_model = model;
}

void AnimHandler::setNode(uint32 time, BoneNode *bone, const BoneNode *parent) {
	const Common::Array<BoneNode *> &bones = _model->getBones();

	_anim->getCoordForBone(time, bone->_idx, bone->_animPos, bone->_animRot);

	if (parent) {
		parent->_animRot.transform(bone->_animPos);

		bone->_animPos = parent->_animPos + bone->_animPos;
		bone->_animRot = parent->_animRot * bone->_animRot;
	}

	for (uint i = 0; i < bone->_children.size(); ++i) {
		setNode(time, bones[bone->_children[i]], bone);
	}
}

void AnimHandler::animate(uint32 time) {
	const Common::Array<BoneNode *> &bones = _model->getBones();

	// Start at root bone
	// For each child
	//  - Set childs animation coordinate
	//  - Process that childs children

	if (time != _lastTime) {
		setNode(time, bones[0], nullptr);
		_lastTime = time;
	}
}

} // End of namespace Stark
