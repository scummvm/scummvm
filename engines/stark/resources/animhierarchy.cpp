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

#include "engines/stark/resources/animhierarchy.h"

#include "common/debug.h"
#include "common/random.h"

#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/textureset.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

AnimHierarchy::~AnimHierarchy() {
}

AnimHierarchy::AnimHierarchy(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_currentActivity(0),
		_currentAnim(nullptr),
		_field_5C(0),
		_idleActionsFrequencySum(0) {
	_type = TYPE;
}

void AnimHierarchy::readData(Formats::XRCReadStream *stream) {
	_animationReferences.clear();

	uint32 refCount = stream->readUint32LE();
	for (uint32 i = 0; i < refCount; i++) {
		_animationReferences.push_back(stream->readResourceReference());
	}

	_parentAnimHierarchyReference = stream->readResourceReference();
	_field_5C = stream->readFloatLE();
}

void AnimHierarchy::onAllLoaded() {
	Object::onAllLoaded();

	loadActivityAnimations();
	loadIdleAnimations();
}

void AnimHierarchy::loadActivityAnimations() {
	AnimHierarchy *parentHierarchy = _parentAnimHierarchyReference.resolve<AnimHierarchy>();

	// Activity animations are inherited from the parent ...
	if (parentHierarchy) {
		_activityAnimations = parentHierarchy->_activityAnimations;
	}

	// ... but can be overridden
	for (uint i = 0; i < _animationReferences.size(); i++) {
		Anim *anim = _animationReferences[i].resolve<Anim>();

		bool inserted = false;
		for (uint j = 0; j < _activityAnimations.size(); j++) {
			if (_activityAnimations[j]->getActivity() == anim->getActivity()) {
				_activityAnimations[j] = anim;
				inserted = true;
			}
		}

		if (!inserted) {
			_activityAnimations.push_back(anim);
		}
	}
}

void AnimHierarchy::loadIdleAnimations() {
	AnimHierarchy *parentHierarchy = _parentAnimHierarchyReference.resolve<AnimHierarchy>();
	if (parentHierarchy) {
		_idleAnimations = parentHierarchy->_idleAnimations;
	}

	for (uint i = 0; i < _animationReferences.size(); i++) {
		Anim *anim = _animationReferences[i].resolve<Anim>();
		if (anim->getActivity() == Anim::kActorActivityIdleAction) {
			_idleAnimations.push_back(anim);
		}
	}

	_idleActionsFrequencySum = 0;
	for (uint i = 0; i < _idleAnimations.size(); i++) {
		_idleActionsFrequencySum += _idleAnimations[i]->getIdleActionFrequency();
	}
}

void AnimHierarchy::setItemAnim(ItemVisual *item, int32 activity) {
	unselectItemAnim(item);
	_currentActivity = activity;
	selectItemAnim(item);
}

void AnimHierarchy::unselectItemAnim(ItemVisual *item) {
	if (_currentAnim && _currentAnim->isInUse()) {
		_currentAnim->removeFromItem(item);
	}

	_currentAnim = nullptr;
}

void AnimHierarchy::selectItemAnim(ItemVisual *item) {
	// Search for an animation with the appropriate index
	for (uint i = 0; i < _activityAnimations.size(); i++) {
		if (_activityAnimations[i]->getActivity() == _currentActivity) {
			_currentAnim = _activityAnimations[i];
			break;
		}
	}

	// Default to the first animation
	if (!_currentAnim && !_activityAnimations.empty()) {
		_currentAnim = _activityAnimations[0];
	}

	if (!_currentAnim) {
		error("Failed to set an animation for item %s", item->getName().c_str());
	}

	if (!_currentAnim->isInUse()) {
		_currentAnim->applyToItem(item);
	}
}

Anim *AnimHierarchy::getCurrentAnim() {
	return _currentAnim;
}

BonesMesh *AnimHierarchy::findBonesMesh() {
	return findChild<BonesMesh>();
}

TextureSet *AnimHierarchy::findTextureSet(uint32 textureType) {
	return findChildWithSubtype<TextureSet>(textureType);
}

Anim *AnimHierarchy::getAnimForActivity(uint32 activity) {
	// Search for an animation with the appropriate use
	for (uint i = 0; i < _activityAnimations.size(); i++) {
		if (_activityAnimations[i]->getActivity() == activity) {
			return _activityAnimations[i];
		}
	}

	return nullptr;
}

Visual *AnimHierarchy::getVisualForUsage(uint32 usage) {
	Anim *anim = getAnimForActivity(usage);
	if (anim) {
		return anim->getVisual();
	}

	return nullptr;
}

Anim *AnimHierarchy::getIdleActionAnim() const {
	if (_idleActionsFrequencySum == 0) {
		return nullptr; // There are no idle animations
	}

	int pick = StarkRandomSource->getRandomNumber(_idleActionsFrequencySum - 1);
	for (uint i = 0; i < _idleAnimations.size(); i++) {
		pick -= _idleAnimations[i]->getIdleActionFrequency();

		if (pick < 0) {
			return _idleAnimations[i];
		}
	}

	return nullptr;
}

void AnimHierarchy::printData() {
	for (uint i = 0; i < _animationReferences.size(); i++) {
		debug("anim %d: %s", i, _animationReferences[i].describe().c_str());
	}

	debug("animHierarchy: %s", _parentAnimHierarchyReference.describe().c_str());
	debug("field_5C: %f", _field_5C);
}

} // End of namespace Resources
} // End of namespace Stark
