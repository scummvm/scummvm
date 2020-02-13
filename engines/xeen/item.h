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

#ifndef XEEN_ITEM_H
#define XEEN_ITEM_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "xeen/sprites.h"

namespace Xeen {

#define INV_ITEMS_TOTAL 9

class XeenEngine;
class Character;

enum ItemCategory {
	CATEGORY_WEAPON = 0, CATEGORY_ARMOR = 1, CATEGORY_ACCESSORY = 2, CATEGORY_MISC = 3,
	NUM_ITEM_CATEGORIES = 4
};

enum AttributeCategory {
	ATTR_MIGHT = 0, ATTR_INTELLECT = 1, ATTR_PERSONALITY = 2, ATTR_SPEED = 3,
	ATTR_ACCURACY = 4, ATTR_LUCK = 5, ATTR_HIT_POINTS = 6, ATTR_SPELL_POINTS = 7,
	ATTR_ARMOR_CLASS = 8, ATTR_THIEVERY = 9
};

enum ElementalCategory {
	ELEM_FIRE = 0, ELEM_ELECTRICITY = 1, ELEM_COLD = 2, ELEM_ACID_POISON = 3,
	ELEM_ENERGY = 4, ELEM_MAGIC = 5
};

enum WeaponId {
	XEEN_SLAYER_SWORD = 34
};

enum Effectiveness {
	EFFECTIVE_NONE = 0, EFFECTIVE_DRAGON = 1, EFFECTIVE_UNDEAD = 2, EFFECTIVE_GOLEM = 3,
	EFFECTIVE_INSECT = 4, EFFEctIVE_MONSTERS = 5, EFFECTIVE_ANIMAL = 6
};

struct ItemState {
	byte _counter : 6;		// Stores charges for Misc items, and the effective against for weapons
	bool _cursed : 1;
	bool _broken : 1;

	/**
	 * Constructor
	 */
	ItemState() : _counter(0), _cursed(false), _broken(false) {}

	/**
	 * Clear the state
	 */
	void clear() {
		_counter = 0;
		_cursed = _broken = false;
	}

	/**
	 * Returns true if the state is empty
	 */
	bool empty() const { return !_counter && !_cursed && !_broken; }

	/**
	 * Synchronizes the item's state
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Set the entire state value
	 */
	void operator=(byte val);
};

class XeenItem {
public:
	int _material;
	uint _id;
	ItemState _state;
	int _frame;
public:
	/**
	 * Return the name of the item
	 */
	static const char *getItemName(ItemCategory category, uint id);
public:
	/**
	 * Constructor
	 */
	XeenItem();

	/**
	 * Clear the data for the item
	 */
	void clear();

	/**
	 * Returns true if no item is set
	 */
	bool empty() const { return _id == 0; }

	/**
	 * Returns true if the item is cursed or broken
	 */
	bool isBad() const { return _state._cursed || _state._broken; }

	/**
	 * Returns true for weapons if it's equipped
	 */
	bool isEquipped() const { return _frame != 0; }

	/**
	 * Synchronizes the data for the item
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Gets the elemental category for the item
	 */
	ElementalCategory getElementalCategory() const;

	/**
	 * Gets the elemental category for a given material
	 */
	static ElementalCategory getElementalCategory(int material);

	/**
	 * Gets the attribute category for the item
	 */
	AttributeCategory getAttributeCategory() const;
};

class InventoryItems : public Common::Array<XeenItem> {
protected:
	Character *_character;
	ItemCategory _category;
	const char **_names;

	XeenEngine *getVm();
	void equipError(int itemIndex1, ItemCategory category1, int itemIndex2,
		ItemCategory category2);

	/**
	 * Returns a text string listing all the stats/attributes of a given item
	 */
	virtual Common::String getAttributes(XeenItem &item, const Common::String &classes) = 0;

	/**
	 * Capitalizes a passed description string that includes embedded formatting for the Items dialog
	 */
	void capitalizeItem(Common::String &name);
public:
	InventoryItems(Character *character, ItemCategory category);
	virtual ~InventoryItems() {}

	/**
	 * Clears the set of items
	 */
	void clear();

	/**
	 * Handles copying items from one character to another
	 */
	InventoryItems &operator=(const InventoryItems &src);

	/**
	 * Return whether a given item passes class-based usage restrictions
	 * @param itemId		Item Index
	 * @param suppressError	If true, no dialog is shown if the item doesn't pass restrictions
	 */
	bool passRestrictions(int itemId, bool suppressError = false) const;

	/**
	 * Return the bare name of a given inventory item
	 */
	Common::String getName(int itemIndex);

	virtual Common::String getFullDescription(int itemIndex, int displayNum = 15) = 0;

	/**
	 * Returns the identified details for an item
	 */
	Common::String getIdentifiedDetails(int itemIndex);

	/**
	 * Discard an item from the inventory
	 */
	bool discardItem(int itemIndex);

	/**
	 * Equips an item
	 */
	virtual void equipItem(int itemIndex) {}

	/**
	 * Un-equips the given item
	 */
	void removeItem(int itemIndex);

	/**
	 * Sorts the items list, removing any empty item slots to the end of the array
	 */
	void sort();

	/**
	 * Enchants an item
	 */
	virtual void enchantItem(int itemIndex, int amount);

	/**
	 * Return if the given inventory items list is full
	 */
	bool isFull() const;
};

class WeaponItems: public InventoryItems {
protected:
	/**
	 * Returns a text string listing all the stats/attributes of a given item
	 */
	Common::String getAttributes(XeenItem &item, const Common::String &classes) override;
public:
	WeaponItems(Character *character) : InventoryItems(character, CATEGORY_WEAPON) {}
	~WeaponItems() override {}

	/**
	 * Equip a given weapon
	 */
	void equipItem(int itemIndex) override;

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	Common::String getFullDescription(int itemIndex, int displayNum) override;

	/**
	 * Enchants a weapon
	 */
	void enchantItem(int itemIndex, int amount) override;

	/**
	 * Returns true if the character has an Elder weapon in Swords of Xeen
	 */
	bool hasElderWeapon() const;
};

class ArmorItems : public InventoryItems {
protected:
	/**
	 * Returns a text string listing all the stats/attributes of a given item
	 */
	Common::String getAttributes(XeenItem &item, const Common::String &classes) override;
public:
	ArmorItems(Character *character) : InventoryItems(character, CATEGORY_ARMOR) {}
	~ArmorItems() override {}

	/**
	 * Equip a given piece of armor
	 */
	void equipItem(int itemIndex) override;

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	Common::String getFullDescription(int itemIndex, int displayNum) override;

	/**
	 * Enchants an armor
	 */
	void enchantItem(int itemIndex, int amount) override;
};

class AccessoryItems : public InventoryItems {
protected:
	/**
	 * Returns a text string listing all the stats/attributes of a given item
	 */
	Common::String getAttributes(XeenItem &item, const Common::String &classes) override;
public:
	AccessoryItems(Character *character) : InventoryItems(character, CATEGORY_ACCESSORY) {}

	/**
	 * Equip a given accessory
	 */
	void equipItem(int itemIndex) override;

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	Common::String getFullDescription(int itemIndex, int displayNum) override;
};

class MiscItems : public InventoryItems {
protected:
	/**
	 * Returns a text string listing all the stats/attributes of a given item
	 */
	Common::String getAttributes(XeenItem &item, const Common::String &classes) override;
public:
	MiscItems(Character *character) : InventoryItems(character, CATEGORY_MISC) {}
	~MiscItems() override {}

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	Common::String getFullDescription(int itemIndex, int displayNum) override;
};

class InventoryItemsGroup {
private:
	Character *_owner;
public:
	InventoryItemsGroup(Character *owner) : _owner(owner) {}

	/**
	 * Returns the inventory items for a given category
	 */
	InventoryItems &operator[](ItemCategory category);

	/**
	 * Returns the inventory items for a given category
	 */
	const InventoryItems &operator[](ItemCategory category) const;

	/**
	 * Breaks all the items in a given character's inventory
	 */
	void breakAllItems();

	/**
	 * Curses or curses all the items
	 */
	void curseUncurse(bool curse);

	/**
	 * Returns true if the character has any cursed items
	 */
	bool hasCursedItems() const;
};

} // End of namespace Xeen

#endif /* XEEN_CHARACTER_H */
