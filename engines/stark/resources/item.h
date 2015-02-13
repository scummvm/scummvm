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

#ifndef STARK_RESOURCES_ITEM_H
#define STARK_RESOURCES_ITEM_H

#include "engines/stark/resources/object.h"
#include "engines/stark/resourcereference.h"

#include "common/rect.h"
#include "common/str.h"

#include "math/vector3d.h"

namespace Stark {

class RenderEntry;
class Visual;
namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Anim;
class AnimHierarchy;
class BonesMesh;
class Bookmark;
class TextureSet;

/**
 * A scene element
 *
 * Can be a character, background, animation, ...
 */
class Item : public Object {
public:
	static const Type::ResourceType TYPE = Type::kItem;

	enum SubType {
		kItemSub1 = 1,
		kItemSub2 = 2,
		kItemSub3 = 3,
		kItemSub5 = 5,
		kItemSub6 = 6,
		kItemSub7 = 7,
		kItemSub8 = 8,
		kItemSub10 = 10
	};

	/** Item factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	Item(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Item();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	/** Is the item present in the scene */
	bool isEnabled() const;

	/** Enable or disable the item */
	virtual void setEnabled(bool enabled);

	/** Obtain the render entry to use to display the item */
	virtual RenderEntry *getRenderEntry(const Common::Point &positionOffset);

	/** Obtain the concrete instance of an item template */
	virtual Item *getSceneInstance();

protected:
	void printData() override;

	bool _enabled;
	int32 _field_38;
};

/**
 * A renderable item
 *
 * Renderable items are found in location layers
 */
class ItemVisual : public Item {
public:
	ItemVisual(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemVisual();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;
	virtual void onAllLoaded() override;

	// Item API
	void setEnabled(bool enabled) override;

	/** Define the current animation index for the item */
	void setAnim(int32 index);

protected:
	// Resource API
	void printData() override;

	Anim *getAnim();
	Visual *getVisual();

	RenderEntry *_renderEntry;

	AnimHierarchy *_animHierarchy;
	int32 _currentAnimIndex;
	uint32 _field_44;
};

/**
 * An item template
 *
 * Item templates need to be instanciated into renderable items to be displayed
 */
class ItemSub13 : public Item {
public:
	ItemSub13(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub13();

	// Resource API
	void onAllLoaded() override;

	// Item API
	Item *getSceneInstance() override;

	/** Obtain the bone mesh to use to render the item */
	virtual BonesMesh *findBonesMesh() = 0;

	/** Obtain the texture to use to render the item */
	virtual TextureSet *findTextureSet(uint32 textureType) = 0;

	/** Obtain the animation hierarchy to fetch animations from */
	virtual AnimHierarchy *findStockAnimHierarchy() = 0;

	/** Set the scene instanciation for this template */
	void setInstanciatedItem(Item *instance);

protected:
	int32 _meshIndex;
	int32 _textureNormalIndex;
	int32 _textureFaceIndex;
	int32 _animHierarchyIndex;

	Item *_instanciatedItem;
	ItemSub13 *_referencedItem;
};

/**
 * A global item template
 *
 * Global item templates are found in the global level
 */
class ItemSub1 : public ItemSub13 {
public:
	ItemSub1(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub1();

	// ItemSub13 API
	BonesMesh *findBonesMesh() override;
	TextureSet *findTextureSet(uint32 textureType) override;
	AnimHierarchy *findStockAnimHierarchy() override;

protected:
};

/**
 * A level item template
 *
 * Level item templates are found in levels so that they can be shared between
 * locations.
 */
class ItemSub3 : public ItemSub13 {
public:
	ItemSub3(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub3();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;

	// ItemSub13 API
	BonesMesh *findBonesMesh() override;
	TextureSet *findTextureSet(uint32 textureType) override;
	AnimHierarchy *findStockAnimHierarchy() override;

protected:
	void printData() override;

	ResourceReference _reference;
};

/**
 * 3D positioned item
 *
 * Items with a 3D position, used in 3D layers. The sort key determines the order
 * in which such items are drawn in.
 */
class ItemSub5610 : public ItemVisual {
public:
	ItemSub5610(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub5610();

	/** Move the item to a bookmarked position */
	void placeOnBookmark(Bookmark *target);

	/** Set the direction the item faces */
	void setDirection(uint direction);

	/** Obtain the sort value for the item, used to compute the draw order */
	float getSortKey() const;

protected:
	int32 _floorFaceIndex;
	Math::Vector3d _position3D;
	float _direction3D;
};

/**
 * 3D positioned image item
 *
 * Used to display still images or animated images in 3D layers
 */
class ItemSub56 : public ItemSub5610 {
public:
	ItemSub56(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub56();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	// Item API
	RenderEntry *getRenderEntry(const Common::Point &positionOffset) override;

protected:
	void printData() override;

	Common::Point _position;
};

/**
 * Mesh item
 *
 * Used to draw characters
 */
class ItemSub10 : public ItemSub5610 {
public:
	ItemSub10(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub10();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;
	void onEnterLocation() override;

	// Item API
	RenderEntry *getRenderEntry(const Common::Point &positionOffset) override;

	BonesMesh *findBonesMesh();
	TextureSet *findTextureSet(uint32 textureType);

protected:
	void printData() override;

	int32 _meshIndex;
	int32 _textureNormalIndex;
	int32 _textureFaceIndex;

	ResourceReference _reference;
	ItemSub13 *_referencedItem;
};

/**
 * 2D positioned image item
 *
 * Used to display background elements in 2D layers
 */
class ItemSub78 : public ItemVisual {
public:
	ItemSub78(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ItemSub78();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	// Item API
	RenderEntry *getRenderEntry(const Common::Point &positionOffset) override;

protected:
	void printData() override;

	ResourceReference _reference;
	Common::Point _position;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ITEM_H
