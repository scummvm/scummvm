/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/animation.h"
#include "engines/grim/resource.h"
#include "engines/grim/model.h"
#include "engines/grim/debug.h"
#include "engines/grim/savegame.h"

namespace Grim {

Animation::Animation(const Common::String &keyframe, AnimManager *manager, int pr1, int pr2) :
	_manager(manager),
	_priority1(pr1),
	_priority2(pr2),
	_paused(true),
	_active(false),
	_time(-1),
	_fade(1.f),
	_fadeMode(None) {
	_keyframe = g_resourceloader->getKeyframe(keyframe);
}

Animation::~Animation() {
	deactivate();
}

void Animation::activate() {
	if (!_active) {
		_active = true;
		_manager->addAnimation(this, _priority1, _priority2);
	}
}

void Animation::deactivate() {
	if (_active) {
		_active = false;
		_manager->removeAnimation(this);
	}
}

void Animation::play(RepeatMode repeatMode) {
	_repeatMode = repeatMode;
	if (_repeatMode != Looping)
		_time = -1;
	_paused = false;
	activate();
}

void Animation::fade(FadeMode fadeMode, int fadeLength) {
	if (!_active) {
		if (fadeMode == FadeIn) {
			_repeatMode = PauseAtEnd;
			_time = -1;
			_fade = 0.f;
			_paused = false;
		}
	}
	_fadeMode = fadeMode;
	_fadeLength = fadeLength;
}

void Animation::pause(bool p) {
	_paused = p;
}

void Animation::stop() {
	_fadeMode = None;
	_time = -1;
	_fade = 1.f;
	_paused = false;
	deactivate();
}

bool Animation::getIsActive() const {
	return _active;
}

Animation::FadeMode Animation::getFadeMode() const {
	return _fadeMode;

}

int Animation::update(int time) {
	int newTime;
	if (_time < 0)		// For first time through
		newTime = 0;
	else if (!_paused)
		newTime = _time + time;

	int marker = 0;
	if (!_paused) {
		marker = _keyframe->getMarker(_time / 1000.f, newTime / 1000.f);
		_time = newTime;
	}

	int animLength = (int)(_keyframe->getLength() * 1000);

	if (_fadeMode != None) {
		if (_fadeMode == FadeIn) {
			_fade += (float)time / (float)_fadeLength;
			if (_fade >= 1.f) {
				_fade = 1.f;
				_fadeMode = None;
			}
		} else {
			_fade -= (float)time / (float)_fadeLength;
			if (_fade <= 0.f) {
				_fade = 0.f;
				// Don't reset the _fadeMode here. This way if fadeOut() was called
				// on a looping chore its keyframe animations will remain faded out
				// when it calls play() again.
				deactivate();
				return 0;
			}
		}
	} else {
		_fade = 1.f;
	}

	if (_time > animLength) { // What to do at end?
		switch (_repeatMode) {
			case Once:
				if (_fadeMode == None)
					deactivate();
				else
					_time = animLength;
				break;
			case Looping:
				_time = -1;
				break;
			case PauseAtEnd:
				_time = animLength;
				_paused = true;
				break;
			case FadeAtEnd:
				if (_fadeMode != FadeOut) {
					_fadeMode = FadeOut;
					_fadeLength = 250;
				}
				_time = animLength;
				break;
			default:
				Debug::warning(Debug::Keyframes, "Unknown repeat mode %d for keyframe %s", _repeatMode, _keyframe->getFilename().c_str());
		}
	}

	return marker;
}

void Animation::saveState(SaveGame *state) const {
	state->writeLESint32(_active);
	state->writeLESint32((int)_repeatMode);
	state->writeLESint32(_time);
	state->writeLESint32((int)_fadeMode);
	state->writeFloat(_fade);
	state->writeLESint32(_fadeLength);
	state->writeLESint32(_paused);
}

void Animation::restoreState(SaveGame *state) {
	bool active = state->readLESint32();
	_repeatMode = (RepeatMode)state->readLESint32();
	_time = state->readLESint32();
	_fadeMode = (FadeMode)state->readLESint32();
	_fade = state->readFloat();
	_fadeLength = state->readLESint32();
	_paused = state->readLESint32();

	if (active)
		activate();
}

/**
 * @class AnimManager
 */

AnimManager::AnimManager() {

}


void AnimManager::addAnimation(Animation *anim, int priority1, int priority2) {
	// Keep the list of animations sorted by priorities in descending order. Because
	// the animations have two different priorities, we add the animation to the list
	// with both priorities.
	Common::List<AnimationEntry>::iterator i;
	AnimationEntry entry;
	entry._anim = anim;
	entry._priority = priority1;
	entry._tagged = false;
	for (i = _activeAnims.begin(); i != _activeAnims.end(); ++i) {
		if (i->_priority < entry._priority) {
			_activeAnims.insert(i, entry);
			break;
		}
	}
	if (i == _activeAnims.end())
		_activeAnims.push_back(entry);

	entry._priority = priority2;
	entry._tagged = true;
	for (i = _activeAnims.begin(); i != _activeAnims.end(); ++i) {
		if (i->_priority < entry._priority) {
			_activeAnims.insert(i, entry);
			break;
		}
	}
	if (i == _activeAnims.end())
		_activeAnims.push_back(entry);
}

void AnimManager::removeAnimation(Animation *anim) {
	Common::List<AnimationEntry>::iterator i;
	for (i = _activeAnims.begin(); i != _activeAnims.end(); ++i) {
		if (i->_anim == anim) {
			i = _activeAnims.erase(i);
			--i;
		}
	}
}

void AnimManager::animate(ModelNode *hier, int numNodes) {
	// Apply animation to each hierarchy node separately.
	for (int i = 0; i < numNodes; i++) {
		Math::Vector3d tempPos;
		Math::Angle tempYaw = 0.0f, tempPitch = 0.0f, tempRoll = 0.0f;
		float totalWeight = 0.0f;
		float remainingWeight = 1.0f;
		int currPriority = -1;

		// The animations are layered so that animations with a higher priority
		// are played regardless of the blend weights of lower priority animations.
		// The highest priority layer gets as much weight as it wants, while the
		// next layer gets the remaining amount and so on.
		for (Common::List<AnimationEntry>::iterator j = _activeAnims.begin(); j != _activeAnims.end(); ++j) {
			if (currPriority != j->_priority) {
				currPriority = j->_priority;
				remainingWeight *= 1 - totalWeight;
				if (remainingWeight <= 0.0f)
					break;

				float weightFactor = 1.0f;
				if (totalWeight > 1.0f) {
					weightFactor = 1.0f / totalWeight;
				}
				tempPos += hier[i]._animPos * weightFactor;
				tempYaw += hier[i]._animYaw * weightFactor;
				tempPitch += hier[i]._animPitch * weightFactor;
				tempRoll += hier[i]._animRoll * weightFactor;
				hier[i]._animPos.set(0,0,0);
				hier[i]._animYaw = 0.0f;
				hier[i]._animPitch = 0.0f;
				hier[i]._animRoll = 0.0f;
				totalWeight = 0.0f;
			}

			float time = j->_anim->_time / 1000.0f;
			float weight = j->_anim->_fade * remainingWeight;
			if (j->_anim->_keyframe->animate(hier, i, time, weight, j->_tagged))
				totalWeight += j->_anim->_fade;
		}

		float weightFactor = 1.0f;
		if (totalWeight > 1.0f) {
			weightFactor = 1.0f / totalWeight;
		}
		hier[i]._animPos = hier[i]._animPos * weightFactor + tempPos;
		hier[i]._animYaw = hier[i]._animYaw * weightFactor + tempYaw;
		hier[i]._animPitch = hier[i]._animPitch * weightFactor + tempPitch;
		hier[i]._animRoll = hier[i]._animRoll * weightFactor + tempRoll;
	}
}

}
