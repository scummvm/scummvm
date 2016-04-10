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
		_model(nullptr),
		_anim(nullptr),
		_animTime(-1),
		_previousAnim(nullptr),
		_previousAnimTime(-1),
		_blendAnim(nullptr),
		_blendAnimTime(-1),
		_blendTimeRemaining(0)
		{

}

AnimHandler::~AnimHandler() {
}

void AnimHandler::setAnim(SkeletonAnim *anim) {
	if (_anim == anim) {
		return;
	}

	if (_previousAnim) {
		if (_previousAnim != anim) {
			// Start blending the new animation up with the previous one
			startBlending();
		} else {
			// The previous animation is the same as the one last used for rendering
			// Stop blending in case it was started by another anim change since the last frame
			stopBlending();
		}
	}

	_anim = anim;
	_animTime = 0;
}

void AnimHandler::setModel(Model *model) {
	_model = model;
}

void AnimHandler::setNode(uint32 time, BoneNode *bone, const BoneNode *parent) {
	const Common::Array<BoneNode *> &bones = _model->getBones();

	if (_blendTimeRemaining <= 0) {
		_anim->getCoordForBone(time, bone->_idx, bone->_animPos, bone->_animRot);
	} else {
		// Blend the coordinates of the previous and the current animation
		Math::Vector3d previousAnimPos, animPos;
		Math::Quaternion previousAnimRot, animRot;
		_blendAnim->getCoordForBone(_blendAnimTime, bone->_idx, previousAnimPos, previousAnimRot);
		_anim->getCoordForBone(time, bone->_idx, animPos, animRot);

		float blendingRatio = 1.0 - _blendTimeRemaining / (float)_blendDuration;

		bone->_animPos = previousAnimPos + (animPos - previousAnimPos) * blendingRatio;
		bone->_animRot = previousAnimRot.slerpQuat(animRot, blendingRatio);
	}

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
	int32 deltaTime = time - _animTime;
	if (deltaTime < 0 || time > _blendDuration / 2) {
		deltaTime = 33;
	}

	// Store the last anim that was actually used for rendering
	_previousAnim = _anim;
	_previousAnimTime = time;

	updateBlending(deltaTime);

	// Start at root bone
	// For each child
	//  - Set childs animation coordinate
	//  - Process that childs children

	const Common::Array<BoneNode *> &bones = _model->getBones();
	if (deltaTime >= 0) {
		setNode(time, bones[0], nullptr);
		_animTime = time;
	}
}

void AnimHandler::startBlending() {
	_blendTimeRemaining = _blendDuration;
	_blendAnim = _previousAnim;
	_blendAnimTime = _previousAnimTime;
}

void AnimHandler::updateBlending(int32 deltaTime) {
	_blendTimeRemaining -= deltaTime;
	if (_blendTimeRemaining > 0) {
		// If we are blending, also update the previous animation's time
		_blendAnimTime += deltaTime;
		if (_blendAnimTime >= (int32) _blendAnim->getLength()) {
			_blendAnimTime = _blendAnim->getLength() - 1;
		}
	} else {
		// Otherwise make sure blending is not enabled
		stopBlending();
	}
}

void AnimHandler::stopBlending() {
	_blendAnim = nullptr;
	_blendAnimTime = -1;
	_blendTimeRemaining = 0;
}

void AnimHandler::resetBlending() {
	stopBlending();
	_previousAnim = nullptr;
	_previousAnimTime = -1;
}

} // End of namespace Stark
