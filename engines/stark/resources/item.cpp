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

#include "engines/stark/formats/xrc.h"
#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/model/animhandler.h"
#include "engines/stark/movement/movement.h"
#include "engines/stark/visual/actor.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/animhierarchy.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/bookmark.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/floorface.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/textureset.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/settings.h"

namespace Stark {
namespace Resources {

Object *Item::construct(Object *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kItemGlobalTemplate:
		return new GlobalItemTemplate(parent, subType, index, name);
	case kItemInventory:
		return new InventoryItem(parent, subType, index, name);
	case kItemLevelTemplate:
		return new LevelItemTemplate(parent, subType, index, name);
	case kItemStaticProp:
	case kItemAnimatedProp:
		return new FloorPositionedImageItem(parent, subType, index, name);
	case kItemBackgroundElement:
	case kItemBackground:
		return new ImageItem(parent, subType, index, name);
	case kItemModel:
		return new ModelItem(parent, subType, index, name);
	default:
		error("Unknown item subtype %d", subType);
	}
}

Item::Item(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_enabled(true),
		_characterIndex(0),
		_movement(nullptr),
		_movementSuspendedScript(nullptr) {
	_type = TYPE;
}

Item::~Item() {
	delete _movement;
}

void Item::readData(Formats::XRCReadStream *stream) {
	_enabled = stream->readBool();
	_characterIndex = stream->readSint32LE();
}

void Item::onGameLoop() {
	Object::onGameLoop();

	if (_enabled && _movement) {
		_movement->onGameLoop();

		if (_movement && _movement->hasEnded()) {
			setMovement(nullptr);
		}
	}
}

bool Item::isEnabled() const {
	return _enabled;
}

void Item::setEnabled(bool enabled) {
	if (_enabled && !enabled) {
		setMovement(nullptr);
	}

	_enabled = enabled;
}

int32 Item::getCharacterIndex() const {
	return _characterIndex;
}

Gfx::RenderEntry *Item::getRenderEntry(const Common::Point &positionOffset) {
	return nullptr;
}

Movement *Item::getMovement() const {
	return _movement;
}

void Item::setMovement(Movement *movement) {
	if (_movementSuspendedScript) {
		if (_movement && _movement->hasReachedDestination()) {
			_movementSuspendedScript->setResumeStatus(Script::kResumeComplete);
		} else {
			_movementSuspendedScript->setResumeStatus(Script::kResumeAbort);
		}
		_movementSuspendedScript = nullptr;
	}

	if (_movement && !_movement->hasEnded()) {
		_movement->stop(true);
	}

	delete _movement;
	_movement = movement;
}

void Item::setMovementSuspendedScript(Script *script) {
	_movementSuspendedScript = script;
}

void Item::printData() {
	debug("enabled: %d", _enabled);
	debug("character: %d", _characterIndex);
}

void Item::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsSint32LE(_enabled);
}

void Item::saveLoadCurrent(ResourceSerializer *serializer) {
	bool hasMovement = _movement != nullptr && !_movement->hasEnded();
	serializer->syncAsUint32LE(hasMovement, 8);

	if (hasMovement) {
		uint32 movementType = _movement != nullptr ? _movement->getType() : 0;
		serializer->syncAsUint32LE(movementType);

		if (serializer->isLoading()) {
			_movement = Movement::construct(movementType, Object::cast<ItemVisual>(this));
		}

		_movement->saveLoad(serializer);
		serializer->syncAsResourceReference(&_movementSuspendedScript);

		if (serializer->isLoading()) {
			_movement->start();
		}
	}
}

Common::Array<Common::Point> Item::listExitPositions() {
	return Common::Array<Common::Point>();
}

ItemVisual::~ItemVisual() {
	delete _renderEntry;
}

ItemVisual::ItemVisual(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Item(parent, subType, index, name),
				_renderEntry(nullptr),
				_actionAnim(nullptr),
				_animHierarchy(nullptr),
				_currentAnimActivity(-1),
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

	if (_subType != kItemModel) {
		setAnimActivity(Anim::kActionUsagePassive);
	}

	if (!_enabled) {
		setEnabled(false);
	}

	Location *location = findParent<Location>();
	if (location) {
		location->registerCharacterItem(_characterIndex, this);
	}
}

void ItemVisual::saveLoad(ResourceSerializer *serializer) {
	Item::saveLoad(serializer);

	serializer->syncAsSint32LE(_currentAnimActivity);

	serializer->syncAsResourceReference(&_animHierarchy);
	if (serializer->isLoading() && _animHierarchy) {
		setAnimHierarchy(_animHierarchy);
	}

	serializer->syncAsResourceReference(&_actionAnim);
	if (serializer->isLoading()) {
		if (_actionAnim) {
			_actionAnim->applyToItem(this);
		} else {
			setAnimActivity(_currentAnimActivity);
		}
	}
}

void ItemVisual::saveLoadCurrent(ResourceSerializer *serializer) {
	Item::saveLoadCurrent(serializer);

	// Apply the animation once again now the data from the item templates has been loaded.
	// This ensures template level textures and models are applied when loading.

	serializer->syncAsSint32LE(_currentAnimActivity, 11);
	serializer->syncAsResourceReference(&_animHierarchy, 11);
	if (serializer->isLoading()) {
		if (_animHierarchy) {
			setAnimHierarchy(_animHierarchy);
		}
	}

	serializer->syncAsResourceReference(&_actionAnim, 11);
	if (serializer->isLoading()) {
		if (_actionAnim) {
			_actionAnim->applyToItem(this);
		} else {
			setAnimActivity(_currentAnimActivity);
		}
	}
}

void ItemVisual::setEnabled(bool enabled) {
	Item::setEnabled(enabled);

	if (enabled) {
		// If an action animation was selected while the item was disabled,
		// honor it. This is not what the original engine did, but seems
		// logical and fixes the Gribbler hurting animation in the forest.
		if (!_actionAnim) {
			_animHierarchy->selectItemAnim(this);
		}
	} else {
		resetActionAnim();
		_animHierarchy->unselectItemAnim(this);
	}
}

ItemVisual *ItemVisual::getSceneInstance() {
	return this;
}

int32 ItemVisual::getAnimActivity() const {
	return _currentAnimActivity;
}

void ItemVisual::setAnimActivity(int32 activity) {
	bool animNeedsUpdate = activity != _currentAnimActivity || _actionAnim != nullptr || _animHierarchy->getCurrentAnim() == nullptr;

	resetActionAnim();

	_currentAnimActivity = activity;
	if (animNeedsUpdate && _animHierarchy) {
		_animHierarchy->setItemAnim(this, activity);
	}
}

void ItemVisual::printData() {
	Item::printData();

	debug("clickable: %d", _clickable);
}

Anim *ItemVisual::getAnim() const {
	if (_actionAnim) {
		return _actionAnim;
	}

	return _animHierarchy->getCurrentAnim();
}

Anim *ItemVisual::getActionAnim() const {
	return _actionAnim;
}

void ItemVisual::setAnimHierarchy(AnimHierarchy *animHierarchy) {
	resetActionAnim();

	if (_animHierarchy) {
		_animHierarchy->unselectItemAnim(this);
	}

	_animHierarchy = animHierarchy;
}

Visual *ItemVisual::getVisual() {
	Anim *anim = getAnim();

	if (!anim) {
		return nullptr;
	}

	return anim->getVisual();
}

int ItemVisual::getHotspotIndexForPoint(const Common::Point &point) {
	Anim *anim = getAnim();
	if (anim) {
		return anim->getPointHotspotIndex(point);
	}
	return -1;
}

bool ItemVisual::canPerformAction(uint32 action, uint32 hotspotIndex) {
	PATTable *table = findChildWithOrder<PATTable>(hotspotIndex);
	return table && table->canPerformAction(action);
}

bool ItemVisual::doAction(uint32 action, uint32 hotspotIndex) {
	PATTable *table = findChildWithOrder<PATTable>(hotspotIndex);
	if (table && table->canPerformAction(action)) {
		return table->runScriptForAction(action);
	}

	return false;
}

void ItemVisual::playActionAnim(Anim *anim) {
	resetActionAnim();

	_animHierarchy->unselectItemAnim(this);
	_actionAnim = anim;
	anim->applyToItem(this);
	anim->playAsAction(this);
}

void ItemVisual::resetActionAnim() {
	if (_actionAnim) {
		_actionAnim->removeFromItem(this);
		_actionAnim = nullptr;

		// TODO: Add a condition to this?
		_animHierarchy->selectItemAnim(this);
		if (_subType == kItemModel) {
			_animHierarchy->setItemAnim(this, Anim::kActorActivityIdle);
		}
	}
}

void ItemVisual::setPosition2D(const Common::Point &position) {
	warning("ItemVisual::setPosition2D is not implemented for this item type: %d (%s)", _subType, _name.c_str());
}

Common::String ItemVisual::getHotspotTitle(uint32 hotspotIndex) {
	PATTable *table = findChildWithOrder<PATTable>(hotspotIndex);
	Common::String title;
	if (table) {
		title = table->getName();
	} else {
		title = getName();
	}

	if (title.equalsIgnoreCase("Default April PAT")) {
		return "April"; // The same hack exists in the original
	}

	return title;
}

Common::Array<Common::Point> ItemVisual::listExitPositionsImpl() {
	Common::Array<PATTable *> pattables = listChildrenRecursive<PATTable>();
	
	Common::Array<Common::Point> positions;
	Common::Point invalidPosition(-1, -1);

	for (uint i = 0; i < pattables.size(); ++i) {
		if (pattables[i]->getDefaultAction() != PATTable::kActionExit) continue;

		Anim *anim = getAnim();
		if (!anim) continue;

		Common::Point hotspot = anim->getHotspotPosition(i);
		if (hotspot != invalidPosition) {
			hotspot += _renderEntry->getPosition();
			positions.push_back(hotspot);
		}
	}

	return positions;
}

ItemTemplate::~ItemTemplate() {
}

ItemTemplate::ItemTemplate(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Item(parent, subType, index, name),
		_meshIndex(-1),
		_textureNormalIndex(-1),
		_textureFaceIndex(-1),
		_animHierarchyIndex(-1),
		_referencedItem(nullptr),
		_instanciatedItem(nullptr) {
}

void ItemTemplate::onAllLoaded() {
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

void ItemTemplate::saveLoadCurrent(ResourceSerializer *serializer) {
	Item::saveLoadCurrent(serializer);

	serializer->syncAsSint32LE(_meshIndex);
	serializer->syncAsSint32LE(_textureNormalIndex);
	serializer->syncAsSint32LE(_textureFaceIndex);
	serializer->syncAsSint32LE(_animHierarchyIndex);
}

void ItemTemplate::setInstanciatedItem(Item *instance) {
	_instanciatedItem = instance;
}

ItemVisual *ItemTemplate::getSceneInstance() {
	if (_instanciatedItem) {
		return _instanciatedItem->getSceneInstance();
	}

	return nullptr;
}

void ItemTemplate::setStockAnimHierachy(AnimHierarchy *animHierarchy) {
	Object *animHierarchyParent = animHierarchy->findParent<Object>();

	if (animHierarchyParent == this) {
		_animHierarchyIndex = animHierarchy->getIndex();
	} else {
		_animHierarchyIndex = -1;
	}
}

void ItemTemplate::setBonesMesh(int32 index) {
	_meshIndex = index;
}

void ItemTemplate::setTexture(int32 index, uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		_textureNormalIndex = index;
	} else if (textureType == TextureSet::kTextureFace) {
		_textureFaceIndex = index;
	} else {
		error("Unknown texture type %d", textureType);
	}

	// Reset the animation to apply the changes
	ModelItem *sceneInstance = Resources::Object::cast<Resources::ModelItem>(getSceneInstance());
	sceneInstance->updateAnim();
}

void ItemTemplate::setAnimHierarchy(AnimHierarchy *animHierarchy) {
	setStockAnimHierachy(animHierarchy);

	if (_instanciatedItem) {
		_instanciatedItem->setAnimHierarchy(animHierarchy);
	}
}

GlobalItemTemplate::~GlobalItemTemplate() {
}

GlobalItemTemplate::GlobalItemTemplate(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemTemplate(parent, subType, index, name) {
	_animHierarchyIndex = 0;
}

BonesMesh *GlobalItemTemplate::findBonesMesh() {
	if (_meshIndex == -1) {
		return nullptr;
	} else {
		BonesMesh *mesh = findChildWithIndex<BonesMesh>(_meshIndex);
		if (mesh && !StarkSettings->getBoolSetting(Settings::kHighModel)) {
			BonesMesh *lowMesh = findChildWithName<BonesMesh>(mesh->getName() + "_LO_RES");
			if (lowMesh) {
				mesh = lowMesh;
			}
		}
		return mesh;
	}
}

TextureSet *GlobalItemTemplate::findTextureSet(uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		if (_textureNormalIndex == -1) {
			return nullptr;
		} else {
			return findChildWithIndex<TextureSet>(_textureNormalIndex);
		}
	} else if (textureType == TextureSet::kTextureFace) {
		if (_textureFaceIndex == -1) {
			return nullptr;
		} else {
			return findChildWithIndex<TextureSet>(_textureFaceIndex);
		}
	} else {
		error("Unknown texture type %d", textureType);
	}
}

AnimHierarchy *GlobalItemTemplate::findStockAnimHierarchy() {
	if (_animHierarchyIndex == -1) {
		return nullptr;
	} else {
		return findChildWithIndex<AnimHierarchy>(_animHierarchyIndex);
	}
}

InventoryItem::~InventoryItem() {
}

InventoryItem::InventoryItem(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name) {
}

Gfx::RenderEntry *InventoryItem::getRenderEntry(const Common::Point &positionOffset) {
	if (_enabled) {
		setAnimActivity(Anim::kUIUsageInventory);

		Visual *visual = getVisual();

		_renderEntry->setVisual(visual);
		_renderEntry->setPosition(Common::Point());
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

void InventoryItem::setEnabled(bool enabled) {
	ItemVisual::setEnabled(enabled);

	// Deselect the item in the inventory when removing it
	int16 selectedInventoryItem = StarkUserInterface->getSelectedInventoryItem();
	if (!enabled && selectedInventoryItem == getIndex()) {
		StarkUserInterface->selectInventoryItem(-1);
	}

	KnowledgeSet *inventory = StarkGlobal->getInventory();
	if (inventory) {
		if (enabled) {
			inventory->addItem(this);
		} else {
			inventory->removeItem(this);
		}
	}
}

Visual *InventoryItem::getActionVisual(bool active) const {
	if (active) {
		return _animHierarchy->getVisualForUsage(Anim::kActionUsageActive);
	} else {
		return _animHierarchy->getVisualForUsage(Anim::kActionUsagePassive);
	}
}

Visual *InventoryItem::getCursorVisual() const {
	Visual *visual = _animHierarchy->getVisualForUsage(Anim::kUIUsageUseCursorPassive);

	if (!visual) {
		visual = _animHierarchy->getVisualForUsage(Anim::kUIUsageUseCursorActive);
	}

	if (!visual) {
		visual = _animHierarchy->getVisualForUsage(Anim::kUIUsageInventory);
	}

	return visual;
}


LevelItemTemplate::~LevelItemTemplate() {
}

LevelItemTemplate::LevelItemTemplate(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemTemplate(parent, subType, index, name) {
}

void LevelItemTemplate::readData(Formats::XRCReadStream *stream) {
	ItemTemplate::readData(stream);

	_reference = stream->readResourceReference();
}

void LevelItemTemplate::onAllLoaded() {
	ItemTemplate::onAllLoaded();

	_referencedItem = _reference.resolve<ItemTemplate>();
	if (_referencedItem) {
		_referencedItem->setInstanciatedItem(this);
	}
}

BonesMesh *LevelItemTemplate::findBonesMesh() {
	if (_meshIndex == -1) {
		return _referencedItem->findBonesMesh();
	} else {
		BonesMesh *mesh = findChildWithIndex<BonesMesh>(_meshIndex);
		if (mesh && !StarkSettings->getBoolSetting(Settings::kHighModel)) {
			BonesMesh *lowMesh = findChildWithName<BonesMesh>(mesh->getName() + "_LO_RES");
			if (lowMesh) {
				mesh = lowMesh;
			}
		}
		return mesh;
	}
}

TextureSet *LevelItemTemplate::findTextureSet(uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		if (_textureNormalIndex == -1) {
			return _referencedItem->findTextureSet(textureType);
		} else {
			return findChildWithIndex<TextureSet>(_textureNormalIndex);
		}
	} else if (textureType == TextureSet::kTextureFace) {
		if (_textureFaceIndex == -1) {
			if (_referencedItem) {
				return _referencedItem->findTextureSet(textureType);
			} else {
				return nullptr;
			}
		} else {
			return findChildWithIndex<TextureSet>(_textureFaceIndex);
		}
	} else {
		error("Unknown texture type %d", textureType);
	}
}

AnimHierarchy *LevelItemTemplate::findStockAnimHierarchy() {
	if (_animHierarchyIndex == -1 && !_referencedItem) {
		_animHierarchyIndex = 0; // Prefer referenced anim to local
	}

	if (_animHierarchyIndex == -1) {
		return _referencedItem->findStockAnimHierarchy();
	} else {
		return findChildWithIndex<AnimHierarchy>(_animHierarchyIndex);
	}
}

ItemTemplate *LevelItemTemplate::getItemTemplate() const {
	return _referencedItem;
}

void LevelItemTemplate::printData() {
	ItemTemplate::printData();

	debug("reference: %s", _reference.describe().c_str());
}

FloorPositionedItem::~FloorPositionedItem() {
}

FloorPositionedItem::FloorPositionedItem(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name),
		_direction3D(0.0),
		_floorFaceIndex(-1),
		_sortKeyOverride(false),
		_sortKeyOverridenValue(0.0) {
}

Math::Vector3d FloorPositionedItem::getPosition3D() const {
	return _position3D;
}

void FloorPositionedItem::setPosition3D(const Math::Vector3d &position) {
	_position3D = position;
}

int32 FloorPositionedItem::getFloorFaceIndex() const {
	return _floorFaceIndex;
}

void FloorPositionedItem::setFloorFaceIndex(int32 faceIndex) {
	_floorFaceIndex = faceIndex;
	_sortKeyOverride = false;
}

void FloorPositionedItem::placeOnBookmark(Bookmark *target) {
	Floor *floor = StarkGlobal->getCurrent()->getFloor();

	_position3D = target->getPosition();

	// Find the floor face index the item is on
	setFloorFaceIndex(floor->findFaceContainingPoint(_position3D));

	// Set the z coordinate using the floor height at that position
	if (_floorFaceIndex < 0) {
		warning("Item '%s' has been placed out of the floor field", getName().c_str());
	}
}

void FloorPositionedItem::placeDefaultPosition() {
	Floor *floor = StarkGlobal->getCurrent()->getFloor();
	FloorFace *face = floor->getFace(0);

	_position3D = face->getCenter();

	// Find the floor face index the item is on
	setFloorFaceIndex(0);

	// Set the z coordinate using the floor height at that position
	floor->computePointHeightInFace(_position3D, 0);
}

Math::Vector3d FloorPositionedItem::getDirectionVector() const {
	Math::Matrix3 rot;
	rot.buildAroundZ(-_direction3D);

	Math::Vector3d direction(1.0, 0.0, 0.0);
	rot.transformVector(&direction);

	return direction;
}

void FloorPositionedItem::setDirection(const Math::Angle &direction) {
	_direction3D = direction.getDegrees(0.0);
}

void FloorPositionedItem::overrideSortKey(float sortKey) {
	_sortKeyOverride = true;
	_sortKeyOverridenValue = sortKey;
}

float FloorPositionedItem::getSortKey() const {
	if (_sortKeyOverride) {
		return _sortKeyOverridenValue;
	}

	Floor *floor = StarkGlobal->getCurrent()->getFloor();

	if (_floorFaceIndex == -1) {
//		warning("Undefined floor face index for item '%s'", getName().c_str());
		return floor->getDistanceFromCamera(0);
	}

	return floor->getDistanceFromCamera(_floorFaceIndex);
}

void FloorPositionedItem::saveLoad(ResourceSerializer *serializer) {
	ItemVisual::saveLoad(serializer);
	serializer->syncAsVector3d(_position3D);
	serializer->syncAsFloat(_direction3D);
	serializer->syncAsSint32LE(_floorFaceIndex);
}

FloorPositionedImageItem::~FloorPositionedImageItem() {
}

FloorPositionedImageItem::FloorPositionedImageItem(Object *parent, byte subType, uint16 index, const Common::String &name) :
		FloorPositionedItem(parent, subType, index, name) {
}

void FloorPositionedImageItem::readData(Formats::XRCReadStream *stream) {
	FloorPositionedItem::readData(stream);

	setFloorFaceIndex(stream->readSint32LE());
	_position = stream->readPoint();

	// WORKAROUND: Fix the position of various items being incorrect in the game datafiles
	Location *location = findParent<Location>();
	if (_name == "Shelves" && location && location->getName() == "April's Room") {
		_position = Common::Point(543, 77);
	} else if (_name == "Door" && location && location->getName() == "Hallway") {
		_position = Common::Point(328, 44);
	} else if (_name == "Bench" && location && location->getName() == "Outside Border House") {
		_position = Common::Point(707, 255);
	} else if (_name == "Printer" && location && location->getName() == "Archives") {
		_position = Common::Point(260, 119);
	} else if (_name == "Prop04_chair01" && location && location->getName() == "Inn Night") {
		_position = Common::Point(185, 324);
	} else if (_name == "Prop05_chair02" && location && location->getName() == "Inn Night") {
		_position = Common::Point(381, 329);
	} else if (_name == "Gargoyle" && location && location->getName() == "Below Floating Mountain") {
		_position = Common::Point(352, 0);
	} else if (_name == "Computer" && location && location->getName() == "Vanguard Laboratory") {
		_position = Common::Point(411, 141);
	}
}

Gfx::RenderEntry *FloorPositionedImageItem::getRenderEntry(const Common::Point &positionOffset) {
	if (_enabled) {
		Visual *visual = getVisual();
		_renderEntry->setVisual(visual);

		if (getAnim() && getAnim()->getSubType() == Anim::kAnimVideo) {
			// AnimVideos override the _position, but still need to move according to scroll-offsets
			_renderEntry->setPosition(positionOffset);
		} else {
			_renderEntry->setPosition(_position - positionOffset);
		}
		_renderEntry->setSortKey(getSortKey());
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

Common::Array<Common::Point> FloorPositionedImageItem::listExitPositions() {
	return listExitPositionsImpl();
}

void FloorPositionedImageItem::setPosition2D(const Common::Point &position) {
	_position = position;
}

void FloorPositionedImageItem::printData() {
	FloorPositionedItem::printData();

	debug("floorFaceIndex: %d", _floorFaceIndex);
	debug("position: x %d, y %d", _position.x, _position.y);
}

ImageItem::~ImageItem() {
}

ImageItem::ImageItem(Object *parent, byte subType, uint16 index, const Common::String &name) :
		ItemVisual(parent, subType, index, name) {
}

void ImageItem::readData(Formats::XRCReadStream *stream) {
	ItemVisual::readData(stream);

	_position = stream->readPoint();
	_reference = stream->readResourceReference();
}

Gfx::RenderEntry *ImageItem::getRenderEntry(const Common::Point &positionOffset) {
	if (_enabled) {
		Visual *visual = getVisual();
		_renderEntry->setVisual(visual);

		if (getAnim() && getAnim()->getSubType() == Anim::kAnimVideo) {
			// AnimVideos override the _position, but still need to move according to scroll-offsets
			_renderEntry->setPosition(positionOffset);
		} else {
			_renderEntry->setPosition(_position - positionOffset);
		}
	} else {
		_renderEntry->setVisual(nullptr);
	}

	return _renderEntry;
}

void ImageItem::setPosition2D(const Common::Point &position) {
	_position = position;
}

void ImageItem::printData() {
	ItemVisual::printData();

	debug("reference: %s", _reference.describe().c_str());
	debug("position: x %d, y %d", _position.x, _position.y);
}

Common::Array<Common::Point> ImageItem::listExitPositions() {
	return listExitPositionsImpl();
}

ModelItem::~ModelItem() {
	delete _animHandler;
}

ModelItem::ModelItem(Object *parent, byte subType, uint16 index, const Common::String &name) :
		FloorPositionedItem(parent, subType, index, name),
		_meshIndex(-1),
		_textureNormalIndex(-1),
		_textureFaceIndex(-1),
		_referencedItem(nullptr),
		_animHandler(nullptr) {
}

void ModelItem::readData(Formats::XRCReadStream *stream) {
	FloorPositionedItem::readData(stream);

	_reference = stream->readResourceReference();
}

void ModelItem::onAllLoaded() {
	FloorPositionedItem::onAllLoaded();

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

	_referencedItem = _reference.resolve<ItemTemplate>();
	if (_referencedItem) {
		_referencedItem->setInstanciatedItem(this);
	}

	_animHandler = new AnimHandler();
}

void ModelItem::onEnterLocation() {
	FloorPositionedItem::onEnterLocation();

	if (_referencedItem) {
		_referencedItem->setInstanciatedItem(this);
	}

	if (_referencedItem) {
		_animHierarchy = _referencedItem->findStockAnimHierarchy();
	}

	setAnimActivity(Anim::kActorActivityIdle);
}

void ModelItem::onExitLocation() {
	FloorPositionedItem::onExitLocation();

	resetActionAnim();
	if (_animHierarchy) {
		_animHierarchy->unselectItemAnim(this);
	}
}

void ModelItem::setBonesMesh(int32 index) {
	_meshIndex = index;

	if (_meshIndex != -1) {
		updateAnim();
	}
}

BonesMesh *ModelItem::findBonesMesh() {
	// Prefer retrieving the mesh from the anim hierarchy
	BonesMesh *bonesMesh = _animHierarchy->findBonesMesh();

	// Otherwise, use a children mesh, or a referenced mesh
	if (!bonesMesh) {
		if (_meshIndex == -1) {
			bonesMesh = _referencedItem->findBonesMesh();
		} else {
			bonesMesh = findChildWithIndex<BonesMesh>(_meshIndex);
			if (bonesMesh && !StarkSettings->getBoolSetting(Settings::kHighModel)) {
				BonesMesh *lowMesh = findChildWithName<BonesMesh>(bonesMesh->getName() + "_LO_RES");
				if (lowMesh) {
					bonesMesh = lowMesh;
				}
			}
		}
	}

	return bonesMesh;
}

void ModelItem::setTexture(int32 index, uint32 textureType) {
	if (textureType == TextureSet::kTextureNormal) {
		_textureNormalIndex = index;
	} else if (textureType == TextureSet::kTextureFace) {
		_textureFaceIndex = index;
	} else {
		error("Unknown texture type %d", textureType);
	}
}

TextureSet *ModelItem::findTextureSet(uint32 textureType) {
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
				if (_referencedItem) {
					textureSet = _referencedItem->findTextureSet(textureType);
				}
			} else {
				textureSet = findChildWithIndex<TextureSet>(_textureFaceIndex);
			}
		} else {
			error("Unknown texture type %d", textureType);
		}
	}

	return textureSet;
}

void ModelItem::updateAnim() {
	Anim *anim = getAnim();
	if (anim && anim->getSubType() == Anim::kAnimSkeleton) {
		anim->removeFromItem(this);
		anim->applyToItem(this);
	}
}

Gfx::RenderEntry *ModelItem::getRenderEntry(const Common::Point &positionOffset) {
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

ItemTemplate *ModelItem::getItemTemplate() const {
	return _referencedItem;
}

Anim *ModelItem::getIdleActionAnim() const {
	if (_animHierarchy) {
		return _animHierarchy->getIdleActionAnim();
	}

	return nullptr;
}

void ModelItem::saveLoadCurrent(ResourceSerializer *serializer) {
	FloorPositionedItem::saveLoadCurrent(serializer);

	serializer->syncAsSint32LE(_meshIndex);
	serializer->syncAsSint32LE(_textureNormalIndex);
	serializer->syncAsSint32LE(_textureFaceIndex);
}

void ModelItem::printData() {
	FloorPositionedItem::printData();

	debug("reference: %s", _reference.describe().c_str());
}

void ModelItem::resetAnimationBlending() {
	_animHandler->resetBlending();
}

AnimHandler *ModelItem::getAnimHandler() const {
	return _animHandler;
}

void ItemTemplate::onEnterLocation() {
	Object::onEnterLocation();

	if (_referencedItem) {
		_referencedItem->setInstanciatedItem(this);
	}
}
} // End of namespace Resources
} // End of namespace Stark
