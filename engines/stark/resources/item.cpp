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

#include "engines/stark/resources/item.h"

#include "common/debug.h"

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/animhierarchy.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/bookmark.h"
#include "engines/stark/resources/textureset.h"
#include "engines/stark/xrcreader.h"

namespace Stark {

Resource *Item::construct(Resource *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kItemSub1:
		return new Item(parent, subType, index, name); // TODO
	case kItemSub2:
		return new Item(parent, subType, index, name); // TODO
	case kItemSub3:
		return new Item(parent, subType, index, name); // TODO
	case kItemSub5:
	case kItemSub6:
		return new ItemSub56(parent, subType, index, name);
	case kItemSub7:
	case kItemSub8:
		return new ItemSub78(parent, subType, index, name);
	case kItemSub10:
		return new ItemSub10(parent, subType, index, name);
	default:
		error("Unknown item subtype %d", subType);
	}
}

Item::~Item() {
}

Item::Item(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_enabled(true),
				_field_38(0) {
	_type = TYPE;
}

void Item::readData(XRCReadStream *stream) {
	_enabled = stream->readBool();
	_field_38 = stream->readSint32LE();
}

bool Item::isEnabled() const {
	return _enabled;
}

void Item::setEnabled(bool enabled) {
	_enabled = enabled;
}

RenderEntry *Item::getRenderEntry() {
	return nullptr;
}

void Item::printData() {
	debug("enabled: %d", _enabled);
	debug("field_38: %d", _field_38);
}

ItemVisual::~ItemVisual() {
	delete _renderEntry;
}

ItemVisual::ItemVisual(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Item(parent, subType, index, name),
				_renderEntry(nullptr),
				_animHierarchy(nullptr),
				_currentAnimIndex(-1),
				_field_44(1) {
	_renderEntry = new RenderEntry(this, getName());
}

void ItemVisual::readData(XRCReadStream *stream) {
	Item::readData(stream);

	_field_44 = stream->readUint32LE();
}

void ItemVisual::onAllLoaded() {
	Item::onAllLoaded();

	_animHierarchy = findChild<AnimHierarchy>();

	if (_subType != kItemSub10) {
		setAnim(1);
	}

	if (!_enabled) {
		setEnabled(false);
	}
}

void ItemVisual::setEnabled(bool enabled) {
	Item::setEnabled(enabled);

	if (enabled) {
		_animHierarchy->selectItemAnim(this);
	} else {
		_animHierarchy->unselectItemAnim(this);
	}
}

void ItemVisual::setAnim(int32 index) {
	bool animNeedsUpdate = index != _currentAnimIndex;

	_currentAnimIndex = index;
	if (animNeedsUpdate && _animHierarchy) {
		_animHierarchy->setItemAnim(this, index);
	}
}

void ItemVisual::printData() {
	Item::printData();

	debug("field_44: %d", _field_44);
}

Anim *ItemVisual::getAnim() {
	return _animHierarchy->getCurrentAnim();
}

Visual *ItemVisual::getVisual() {
	Anim *anim = getAnim();

	if (!anim) {
		return nullptr;
	}

	return anim->getVisual();
}

ItemSub5610::~ItemSub5610() {
}

ItemSub5610::ItemSub5610(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name),
		_direction3D(0.0),
		_field_6C(-1) {
}

void ItemSub5610::placeOnBookmark(Bookmark *target) {
	// TODO: valorize the z coordinate using the floor height at that position
	_position3D = target->getPosition();
}

void ItemSub5610::setDirection(uint direction) {
	_direction3D = direction;
}

ItemSub56::~ItemSub56() {
}

ItemSub56::ItemSub56(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		ItemSub5610(parent, subType, index, name) {
}

void ItemSub56::readData(XRCReadStream *stream) {
	ItemSub5610::readData(stream);

	_field_6C = stream->readSint32LE();
	_position = stream->readPoint();
}

RenderEntry *ItemSub56::getRenderEntry() {
	if (_enabled) {
		Visual *visual = getVisual();

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition(_position);
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

void ItemSub56::printData() {
	ItemSub5610::printData();

	debug("field_6C: %d", _field_6C);
	debug("position: x %d, y %d", _position.x, _position.y);
}

ItemSub78::~ItemSub78() {
}

ItemSub78::ItemSub78(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name) {
}

void ItemSub78::readData(XRCReadStream *stream) {
	ItemVisual::readData(stream);

	_reference = stream->readResourceReference();
	_position = stream->readPoint();
}

RenderEntry *ItemSub78::getRenderEntry() {
	if (_enabled) {
		Visual *visual = getVisual();

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition(_position);
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

void ItemSub78::printData() {
	ItemVisual::printData();

	debug("reference: %s", _reference.describe().c_str());
	debug("position: x %d, y %d", _position.x, _position.y);
}

ItemSub10::~ItemSub10() {
}

ItemSub10::ItemSub10(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		ItemSub5610(parent, subType, index, name),
		_meshIndex(-1),
		_textureNormalIndex(-1),
		_textureFaceIndex(-1) {
}

void ItemSub10::readData(XRCReadStream *stream) {
	ItemSub5610::readData(stream);

	_reference = stream->readResourceReference();
}

void ItemSub10::onAllLoaded() {
	ItemSub5610::onAllLoaded();

	BonesMesh *bonesMesh = findChild<BonesMesh>();
	if (bonesMesh) {
		_meshIndex = bonesMesh->getIndex();
	}

	TextureSet *textureNormal = findChildWithSubtype<TextureSet>(TextureSet::kTextureNormal);
	if (textureNormal) {
		_textureNormalIndex = textureNormal->getIndex();
	}

	TextureSet *textureFace = findChildWithSubtype<TextureSet>(TextureSet::kTextureFace);
	if (textureFace) {
		_textureFaceIndex = textureFace->getIndex();
	}
}

BonesMesh *ItemSub10::findBonesMesh() {
	// Prefer retrieving the mesh from the anim hierarchy
	BonesMesh *bonesMesh = _animHierarchy->findBonesMesh();

	// Otherwise, use a children mesh, or a referenced mesh
	if (!bonesMesh) {
		if (_meshIndex == -1) {
			//TODO: Load from referenced item
		} else {
			bonesMesh = findChildWithIndex<BonesMesh>(_meshIndex);
		}
	}

	return bonesMesh;
}

TextureSet *ItemSub10::findTextureSet(uint32 textureType) {
	// Prefer retrieving the mesh from the anim hierarchy
	TextureSet *textureSet = _animHierarchy->findTextureSet(textureType);

	// Otherwise, use a children mesh, or a referenced mesh
	if (!textureSet) {
		if (textureType == TextureSet::kTextureNormal) {
			if (_textureNormalIndex == -1) {
				//TODO: Load from referenced item
			} else {
				textureSet = findChildWithIndex<TextureSet>(_textureNormalIndex);
			}
		} else if (textureType == TextureSet::kTextureNormal) {
			if (_textureFaceIndex == -1) {
				//TODO: Load from referenced item
			} else {
				textureSet = findChildWithIndex<TextureSet>(_textureFaceIndex);
			}
		} else {
			error("Unknown texture type %d", textureType);
		}
	}

	return textureSet;
}

RenderEntry *ItemSub10::getRenderEntry() {
	if (_enabled) {
		Visual *visual = getVisual();

		if (!visual) {
			_animHierarchy->selectItemAnim(this);
			visual = getVisual();
		}

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition3D(_position3D, _direction3D);
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

void ItemSub10::printData() {
	ItemSub5610::printData();

	debug("reference: %s", _reference.describe().c_str());
}

} // End of namespace Stark
