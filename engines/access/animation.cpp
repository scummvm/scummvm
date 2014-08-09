/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "access/animation.h"

namespace Access {

AnimationManager::AnimationManager(AccessEngine *vm) : Manager(vm) {
	_anim = nullptr;
	_animation = nullptr;
}

AnimationManager::~AnimationManager() {
	delete[] _anim;
	delete _animation;
}

void AnimationManager::freeAnimationData() {
	delete[] _anim;
	_anim = nullptr;
	_animation = nullptr;
}

void AnimationManager::clearTimers() {
	for (uint i = 0; i < _animationTimers.size(); ++i)
		delete _animationTimers[i];

	_animationTimers.clear();
}

Animation *AnimationManager::setAnimation(int animId) {
	Animation *anim = findAnimation(animId);

	anim->_countdownTicks = anim->_ticks;
	anim->_frameNumber = 0;

	anim->_currentLoopCount = (anim->_type != 3 && anim->_type != 4) ? 0 : 
		anim->_loopCount;
	anim->_field10 = 0;
		
	return anim;
}

void AnimationManager::setAnimTimer(Animation *anim) {

}

Animation *AnimationManager::findAnimation(int animId) {
	_animation = new Animation(_anim + READ_LE_UINT16(_anim + animId * 4 + 2));
	return _animation;
}

/*------------------------------------------------------------------------*/

Animation::Animation(const byte *data) {

}

} // End of namespace Access
