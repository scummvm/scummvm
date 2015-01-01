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

#include "common/debug.h"

#include "engines/stark/resources/animhierarchy.h"
#include "engines/stark/xrcreader.h"

namespace Stark {

AnimHierarchy::~AnimHierarchy() {
}

AnimHierarchy::AnimHierarchy(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_animIndex(0),
				_currentAnim(nullptr),
				_field_5C(0) {
	_type = TYPE;
}

void AnimHierarchy::readData(XRCReadStream *stream) {
	_animationReferences.clear();

	uint32 refCount = stream->readUint32LE();
	for (uint32 i = 0; i < refCount; i++) {
		_animationReferences.push_back(stream->readResourceReference());
	}

	_animHierarchyReference = stream->readResourceReference();
	_field_5C = stream->readFloat();
}

void AnimHierarchy::setItemAnim(ItemVisual *item, int32 index) {
	unselectItemAnim(item);
	_animIndex = index;
	selectItemAnim(item);
}

void AnimHierarchy::unselectItemAnim(ItemVisual *item) {
	//TODO
}

void AnimHierarchy::selectItemAnim(ItemVisual *item) {
	//TODO
}

void AnimHierarchy::printData() {
	for (uint i = 0; i < _animationReferences.size(); i++) {
		debug("anim %d: %s", i, _animationReferences[i].describe().c_str());
	}

	debug("animHierarchy: %s", _animHierarchyReference.describe().c_str());
	debug("field_5C: %f", _field_5C);
}

} // End of namespace Stark
