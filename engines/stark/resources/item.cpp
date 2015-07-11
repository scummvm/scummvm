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

#include "engines/stark/formats/xrc.h"
#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/animhierarchy.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/bookmark.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/textureset.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

Object *Item::construct(Object *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kItemSub1:
		return new ItemSub1(parent, subType, index, name);
	case kItemSub2:
		return new ItemSub2(parent, subType, index, name);
	case kItemSub3:
		return new ItemSub3(parent, subType, index, name);
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

Item::Item(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name),
				_enabled(true),
				_field_38(0) {
	_type = TYPE;
}

void Item::readData(Formats::XRCReadStream *stream) {
	_enabled = stream->readBool();
	_field_38 = stream->readSint32LE();
}

bool Item::isEnabled() const {
	return _enabled;
}

void Item::setEnabled(bool enabled) {
	_enabled = enabled;
}

Gfx::RenderEntry *Item::getRenderEntry(const Common::Point &positionOffset) {
	return nullptr;
}

Item *Item::getSceneInstance() {
	return this;
}

void Item::printData() {
	debug("enabled: %d", _enabled);
	debug("field_38: %d", _field_38);
}

void Item::saveLoad(ResourceSerializer *serializer) {
	if (!serializer->matchBytes("ITEM", 9)) {
		error("Couldn't fint the correct save header");
	}
	serializer->syncAsSint32LE(_enabled);
}

ItemVisual::~ItemVisual() {
	delete _renderEntry;
}

ItemVisual::ItemVisual(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Item(parent, subType, index, name),
				_renderEntry(nullptr),
				_animHierarchy(nullptr),
				_currentAnimKind(-1),
				_clickable(true) {
	_renderEntry = new Gfx::RenderEntry(this, getName());
}

void ItemVisual::readData(Formats::XRCReadStream *stream) {
	Item::readData(stream);

	_clickable = stream->readBool();
}

void ItemVisual::onAllLoaded() {
	Item::onAllLoaded();

	_animHierarchy = findChild<AnimHierarchy>(false);

	_renderEntry->setClickable(_clickable);

	if (_subType != kItemSub10) {
		setAnimKind(Anim::kActionUsagePassive);
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

void ItemVisual::setAnimKind(int32 usage) {
	bool animNeedsUpdate = usage != _currentAnimKind;

	_currentAnimKind = usage;
	if (animNeedsUpdate && _animHierarchy) {
		_animHierarchy->setItemAnim(this, usage);
	}
}

void ItemVisual::printData() {
	Item::printData();

	debug("clickable: %d", _clickable);
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

int ItemVisual::getHotspotIndexForPoint(Common::Point point) {
	// TODO: This breaks rather weirdly on subtype 6 and 10
	Anim *anim = getAnim();
	if (anim) {
		return anim->indexForPoint(point);
	}
	return -1;
}

bool ItemVisual::canPerformAction(uint32 action, uint32 hotspotIndex) {
	PATTable *table = findChildWithIndex<PATTable>(hotspotIndex);
	return table && table->canPerformAction(action);
}

bool ItemVisual::doAction(uint32 action, uint32 hotspotIndex) {
	PATTable *table = findChildWithIndex<PATTable>(hotspotIndex);
	if (table && table->canPerformAction(action)) {
		return table->runScriptForAction(action);
	}

	return false;
}

Common::String ItemVisual::getHotspotTitle(uint32 hotspotIndex) {
	PATTable *table = findChildWithIndex<PATTable>(hotspotIndex);
	if (table) {
		return table->getName();
	} else {
		return getName();
	}
}

ItemSub13::~ItemSub13() {
}

ItemSub13::ItemSub13(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Item(parent, subType, index, name),
		_meshIndex(-1),
		_textureNormalIndex(-1),
		_textureFaceIndex(-1),
		_animHierarchyIndex(-1),
		_referencedItem(nullptr),
		_instanciatedItem(nullptr) {
}

void ItemSub13::onAllLoaded() {
	Item::onAllLoaded();

	BonesMesh *bonesMesh = findChild<BonesMesh>(false);
	if (bonesMesh) {
		_meshIndex = bonesMesh->getIndex();
	}

	TextureSet *textureNormal = findChildWithSubtype<TextureSet>(TextureSet::kTextureNormal, false);
	if (textureNormal) {
		_textureNormalIndex = textureNormal->getIndex();
	}

	TextureSet *textureFace = findChildWithSubtype<TextureSet>(TextureSet::kTextureFace, false);
	if (textureFace) {
		_textureFaceIndex = textureFace->getIndex();
	}
}

void ItemSub13::setInstanciatedItem(Item *instance) {
	_instanciatedItem = instance;
}

Item *ItemSub13::getSceneInstance() {
	if (_instanciatedItem) {
		return _instanciatedItem->getSceneInstance();
	}

	return nullptr;
}

void ItemSub13::setBonesMesh(int32 index) {
	_meshIndex = index;
}

void ItemSub13::setTexture(int32 index, uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		_textureNormalIndex = index;
	} else if (textureType == TextureSet::kTextureFace) {
		_textureFaceIndex = index;
	} else {
		error("Unknown texture type %d", textureType);
	}

	// Reset the animation to apply the changes
	ItemSub10 *sceneInstance = Resources::Object::cast<Resources::ItemSub10>(getSceneInstance());
	sceneInstance->updateAnim();
}

ItemSub1::~ItemSub1() {
}

ItemSub1::ItemSub1(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemSub13(parent, subType, index, name) {
	_animHierarchyIndex = 0;
}

BonesMesh *ItemSub1::findBonesMesh() {
	if (_meshIndex == -1) {
		return nullptr;
	} else {
		return findChildWithIndex<BonesMesh>(_meshIndex);
	}
}

TextureSet *ItemSub1::findTextureSet(uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		if (_textureNormalIndex == -1) {
			return nullptr;
		} else {
			return findChildWithIndex<TextureSet>(_textureNormalIndex);
		}
	} else if (textureType == TextureSet::kTextureNormal) {
		if (_textureFaceIndex == -1) {
			return nullptr;
		} else {
			return findChildWithIndex<TextureSet>(_textureFaceIndex);
		}
	} else {
		error("Unknown texture type %d", textureType);
	}
}

AnimHierarchy *ItemSub1::findStockAnimHierarchy() {
	if (_animHierarchyIndex == -1) {
		return nullptr;
	} else {
		return findChildWithIndex<AnimHierarchy>(_animHierarchyIndex);
	}
}

ItemSub2::~ItemSub2() {
}

ItemSub2::ItemSub2(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name) {
}

Gfx::RenderEntry *ItemSub2::getRenderEntry(const Common::Point &positionOffset) {
	if (_enabled) {
		setAnimKind(Anim::kUIUsageInventory);

		Visual *visual = getVisual();

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition(Common::Point());
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

Visual *ItemSub2::getActionVisual(bool active) const {
	if (active) {
		return _animHierarchy->getVisualForUsage(Anim::kActionUsageActive);
	} else {
		return _animHierarchy->getVisualForUsage(Anim::kActionUsagePassive);
	}
}

Visual *ItemSub2::getCursorVisual() const {
	return _animHierarchy->getVisualForUsage(Anim::kUIUsageUseCursor);
}


ItemSub3::~ItemSub3() {
}

ItemSub3::ItemSub3(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemSub13(parent, subType, index, name) {
}

void ItemSub3::readData(Formats::XRCReadStream *stream) {
	ItemSub13::readData(stream);

	_reference = stream->readResourceReference();
}

void ItemSub3::onAllLoaded() {
	ItemSub13::onAllLoaded();

	_referencedItem = _reference.resolve<ItemSub13>();
	if (_referencedItem) {
		_referencedItem->setInstanciatedItem(this);
	}
}

BonesMesh *ItemSub3::findBonesMesh() {
	if (_meshIndex == -1) {
		return _referencedItem->findBonesMesh();
	} else {
		return findChildWithIndex<BonesMesh>(_meshIndex);
	}
}

TextureSet *ItemSub3::findTextureSet(uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		if (_textureNormalIndex == -1) {
			return _referencedItem->findTextureSet(textureType);
		} else {
			return findChildWithIndex<TextureSet>(_textureNormalIndex);
		}
	} else if (textureType == TextureSet::kTextureNormal) {
		if (_textureFaceIndex == -1) {
			return _referencedItem->findTextureSet(textureType);
		} else {
			return findChildWithIndex<TextureSet>(_textureFaceIndex);
		}
	} else {
		error("Unknown texture type %d", textureType);
	}
}

AnimHierarchy *ItemSub3::findStockAnimHierarchy() {
	if (_animHierarchyIndex == -1 && !_referencedItem) {
		_animHierarchyIndex = 0; // Prefer referenced anim to local
	}

	if (_animHierarchyIndex == -1) {
		return _referencedItem->findStockAnimHierarchy();
	} else {
		return findChildWithIndex<AnimHierarchy>(_animHierarchyIndex);
	}
}

void ItemSub3::printData() {
	ItemSub13::printData();

	debug("reference: %s", _reference.describe().c_str());
}

ItemSub5610::~ItemSub5610() {
}

ItemSub5610::ItemSub5610(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name),
		_direction3D(0.0),
		_floorFaceIndex(-1) {
}

void ItemSub5610::setPosition3D(const Math::Vector3d &position) {
	_position3D = position;
}

int32 ItemSub5610::getFloorFaceIndex() const {
	return _floorFaceIndex;
}

void ItemSub5610::setFloorFaceIndex(int32 faceIndex) {
	_floorFaceIndex = faceIndex;
}

void ItemSub5610::placeOnBookmark(Bookmark *target) {
	Global *global = StarkServices::instance().global;
	Floor *floor = global->getCurrent()->getFloor();

	_position3D = target->getPosition();

	// Find the floor face index the item is on
	setFloorFaceIndex(floor->findFaceContainingPoint(_position3D));

	// Set the z coordinate using the floor height at that position
	if (_floorFaceIndex >= 0) {
		floor->computePointHeightInFace(_position3D, _floorFaceIndex);
	} else {
		warning("Item '%s' has been place out of the floor field", getName().c_str());
	}
}

void ItemSub5610::setDirection(uint direction) {
	_direction3D = direction;
}

float ItemSub5610::getSortKey() const {
	Global *global = StarkServices::instance().global;
	Floor *floor = global->getCurrent()->getFloor();

	if (_floorFaceIndex == -1) {
//		warning("Undefined floor face index for item '%s'", getName().c_str());
		return floor->getDistanceFromCamera(0);
	}

	return floor->getDistanceFromCamera(_floorFaceIndex);
}

ItemSub56::~ItemSub56() {
}

ItemSub56::ItemSub56(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemSub5610(parent, subType, index, name) {
}

void ItemSub56::readData(Formats::XRCReadStream *stream) {
	ItemSub5610::readData(stream);

	setFloorFaceIndex(stream->readSint32LE());
	_position = stream->readPoint();
}

Gfx::RenderEntry *ItemSub56::getRenderEntry(const Common::Point &positionOffset) {
	if (_enabled) {
		Visual *visual = getVisual();

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition(_position - positionOffset);
		_renderEntry->setSortKey(getSortKey());
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

void ItemSub56::printData() {
	ItemSub5610::printData();

	debug("floorFaceIndex: %d", _floorFaceIndex);
	debug("position: x %d, y %d", _position.x, _position.y);
}

ItemSub78::~ItemSub78() {
}

ItemSub78::ItemSub78(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name) {
}

void ItemSub78::readData(Formats::XRCReadStream *stream) {
	ItemVisual::readData(stream);

	_position = stream->readPoint();
	_reference = stream->readResourceReference();
}

Gfx::RenderEntry *ItemSub78::getRenderEntry(const Common::Point &positionOffset) {
	if (_enabled) {
		Visual *visual = getVisual();

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition(_position - positionOffset);
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

ItemSub10::ItemSub10(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemSub5610(parent, subType, index, name),
		_meshIndex(-1),
		_textureNormalIndex(-1),
		_textureFaceIndex(-1),
		_referencedItem(nullptr) {
}

void ItemSub10::readData(Formats::XRCReadStream *stream) {
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

	_referencedItem = _reference.resolve<ItemSub13>();
	if (_referencedItem) {
		_referencedItem->setInstanciatedItem(this);
	}
}

void ItemSub10::onEnterLocation() {
	ItemSub5610::onEnterLocation();

	if (_referencedItem) {
		_animHierarchy = _referencedItem->findStockAnimHierarchy();
	}

	setAnimKind(Anim::kActorUsageIdle);
}

void ItemSub10::setBonesMesh(int32 index) {
	_meshIndex = index;

	if (_meshIndex != -1) {
		updateAnim();
	}
}

BonesMesh *ItemSub10::findBonesMesh() {
	// Prefer retrieving the mesh from the anim hierarchy
	BonesMesh *bonesMesh = _animHierarchy->findBonesMesh();

	// Otherwise, use a children mesh, or a referenced mesh
	if (!bonesMesh) {
		if (_meshIndex == -1) {
			bonesMesh = _referencedItem->findBonesMesh();
		} else {
			bonesMesh = findChildWithIndex<BonesMesh>(_meshIndex);
		}
	}

	return bonesMesh;
}

void ItemSub10::setTexture(int32 index, uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		_textureNormalIndex = index;
	} else if (textureType == TextureSet::kTextureFace) {
		_textureFaceIndex = index;
	} else {
		error("Unknown texture type %d", textureType);
	}
}

TextureSet *ItemSub10::findTextureSet(uint32 textureType) {
	// Prefer retrieving the mesh from the anim hierarchy
	TextureSet *textureSet = _animHierarchy->findTextureSet(textureType);

	// Otherwise, use a children mesh, or a referenced mesh
	if (!textureSet) {
		if (textureType == TextureSet::kTextureNormal) {
			if (_textureNormalIndex == -1) {
				textureSet = _referencedItem->findTextureSet(textureType);
			} else {
				textureSet = findChildWithIndex<TextureSet>(_textureNormalIndex);
			}
		} else if (textureType == TextureSet::kTextureFace) {
			if (_textureFaceIndex == -1) {
				textureSet = _referencedItem->findTextureSet(textureType);
			} else {
				textureSet = findChildWithIndex<TextureSet>(_textureFaceIndex);
			}
		} else {
			error("Unknown texture type %d", textureType);
		}
	}

	return textureSet;
}

void ItemSub10::updateAnim() {
	Anim *anim = getAnim();
	if (anim && anim->getSubType() == Anim::kAnimSkeleton) {
		anim->removeFromItem(this);
		anim->applyToItem(this);
	}
}

Gfx::RenderEntry *ItemSub10::getRenderEntry(const Common::Point &positionOffset) {
	if (_enabled) {
		Visual *visual = getVisual();

		if (!visual) {
			_animHierarchy->selectItemAnim(this);
			visual = getVisual();
		}

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition3D(_position3D, _direction3D);
		_renderEntry->setSortKey(getSortKey());
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

void ItemSub10::printData() {
	ItemSub5610::printData();

	debug("reference: %s", _reference.describe().c_str());
}

} // End of namespace Resources
} // End of namespace Stark
