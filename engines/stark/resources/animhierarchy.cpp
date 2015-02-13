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

#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/textureset.h"

namespace Stark {
namespace Resources {

AnimHierarchy::~AnimHierarchy() {
}

AnimHierarchy::AnimHierarchy(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name),
				_animIndex(0),
				_currentAnim(nullptr),
				_animHierarchy(nullptr),
				_field_5C(0) {
	_type = TYPE;
}

void AnimHierarchy::readData(Formats::XRCReadStream *stream) {
	_animationReferences.clear();

	uint32 refCount = stream->readUint32LE();
	for (uint32 i = 0; i < refCount; i++) {
		_animationReferences.push_back(stream->readResourceReference());
	}

	_animHierarchyReference = stream->readResourceReference();
	_field_5C = stream->readFloat();
}

void AnimHierarchy::onAllLoaded() {
	Object::onAllLoaded();

	_animations.clear();

	// Animations can be provided directly ...
	for (uint i = 0; i < _animationReferences.size(); i++) {
		_animations.push_back(_animationReferences[i].resolve<Anim>());
	}

	// ... or through another animation hierarchy
	_animHierarchy = _animHierarchyReference.resolve<AnimHierarchy>();
	if (_animHierarchy) {
		for (uint i = 0; i < _animHierarchy->_animationReferences.size(); i++) {
			_animations.push_back(_animHierarchy->_animationReferences[i].resolve<Anim>());
		}
	}
}

void AnimHierarchy::setItemAnim(ItemVisual *item, int32 index) {
	unselectItemAnim(item);
	_animIndex = index;
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
	for (uint i = 0; i < _animations.size(); i++) {
		if (_animations[i]->getIndex() == _animIndex) {
			_currentAnim = _animations[i];
			break;
		}
	}

	// Default to the first animation
	if (!_currentAnim && !_animations.empty()) {
		_currentAnim = _animations[0];
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

void AnimHierarchy::printData() {
	for (uint i = 0; i < _animationReferences.size(); i++) {
		debug("anim %d: %s", i, _animationReferences[i].describe().c_str());
	}

	debug("animHierarchy: %s", _animHierarchyReference.describe().c_str());
	debug("field_5C: %f", _field_5C);
}

} // End of namespace Resources
} // End of namespace Stark
