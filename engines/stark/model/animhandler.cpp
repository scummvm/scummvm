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
		_framesBeforeCandidateReady(0),
		_candidateAnim(nullptr),
		_candidateAnimTime(-1),
		_blendAnim(nullptr),
		_blendAnimTime(-1),
		_blendTimeRemaining(0) {

}

AnimHandler::~AnimHandler() {
}

void AnimHandler::setAnim(SkeletonAnim *anim) {
	if (_candidateAnim == anim) {
		return;
	}

	if (_anim == anim) {
		// If we already have the correct anim, clean any candidates
		// that may have been set but not yet enacted as the active anim.
		_candidateAnim = nullptr;
		_candidateAnimTime = -1;
		_framesBeforeCandidateReady = 0;
		return;
	}

	// Don't use new animations the first frame they are set.
	// Scripts may change animation the very next frame,
	// causing animations to blend with animations that
	// were only visible for one frame, leading to animation
	// jumps. Instead store them as candidates.
	_framesBeforeCandidateReady = 2; // 2 because we are at the end of the frame
	_candidateAnim = anim;
	_candidateAnimTime = 0;
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
	if (!_anim && _candidateAnim) {
		// This is the first time we animate this item.
		enactCandidate();
	}

	if (_candidateAnim && _anim && _anim->getBoneCount() != _model->getBones().size()) {
		// We changed to an incompatible model
		enactCandidate();

		// And the anim we were previously blending with is incompatible as well
		if (_blendAnim && _blendAnim->getBoneCount() != _model->getBones().size()) {
			stopBlending();
		}
	}

	if (_candidateAnim && _framesBeforeCandidateReady > 0) {

		_candidateAnimTime = time;
		_framesBeforeCandidateReady--;

		// We need to animate here, because the model may have
		// changed from under us.
		const Common::Array<BoneNode *> &bones = _model->getBones();
		setNode(_animTime, bones[0], nullptr);
		return;
	}

	if (_candidateAnim && _framesBeforeCandidateReady <= 0) {
		if (_anim) {
			startBlending();
		}
		enactCandidate();
	}

	int32 deltaTime = time - _animTime;
	if (deltaTime < 0 || time > _blendDuration / 2) {
		deltaTime = 33;
	}

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

void AnimHandler::enactCandidate() {
	_anim = _candidateAnim;
	_animTime = _candidateAnimTime;
	_candidateAnim = nullptr;
	_candidateAnimTime = -1;
	_framesBeforeCandidateReady = 0;
}

void AnimHandler::startBlending() {
	_blendTimeRemaining = _blendDuration;
	_blendAnim = _anim;
	_blendAnimTime = _animTime;
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
	if (_candidateAnim) {
		enactCandidate();
	}
}

} // End of namespace Stark
