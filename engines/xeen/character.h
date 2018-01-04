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

#ifndef XEEN_CHARACTER_H
#define XEEN_CHARACTER_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "xeen/combat.h"
#include "xeen/sprites.h"

namespace Xeen {

#define INV_ITEMS_TOTAL 9
#define MAX_SPELLS_PER_CLASS 39
#define AWARDS_TOTAL 88
#define WARZONE_AWARD 9

enum Award {
	SHANGRILA_GUILD_MEMBER = 5, GOOBER = 76, SUPER_GOOBER = 77,
	CASTLEVIEW_GUILD_MEMBER = 83, SANDCASTER_GUILD_MEMBER = 84,
	LAKESIDE_GUILD_MEMBER = 85, NECROPOLIS_GUILD_MEMBER = 86, OLYMPUS_GUILD_MEMBER = 87
};

enum BonusFlags {
	ITEMFLAG_BONUS_MASK = 0xBF, ITEMFLAG_CURSED = 0x40, ITEMFLAG_BROKEN = 0x80
};

enum ItemCategory {
	CATEGORY_WEAPON = 0, CATEGORY_ARMOR = 1, CATEGORY_ACCESSORY = 2, CATEGORY_MISC = 3,
	NUM_ITEM_CATEGORIES = 4
};

enum Sex { MALE = 0, FEMALE = 1, YES_PLEASE = 2 };

enum Race { HUMAN = 0, ELF = 1, DWARF = 2, GNOME = 3, HALF_ORC = 4 };

enum CharacterClass {
	CLASS_KNIGHT = 0, CLASS_PALADIN = 1, CLASS_ARCHER = 2, CLASS_CLERIC = 3,
	CLASS_SORCERER = 4, CLASS_ROBBER = 5, CLASS_NINJA = 6, CLASS_BARBARIAN = 7,
	CLASS_DRUID = 8, CLASS_RANGER = 9, TOTAL_CLASSES = 10, CLASS_12 = 12, CLASS_15 = 15, CLASS_16 = 16
};

enum Attribute {
	MIGHT = 0, INTELLECT = 1, PERSONALITY = 2, ENDURANCE = 3, SPEED = 4,
	ACCURACY = 5, LUCK = 6, TOTAL_ATTRIBUTES = 7
};

enum Skill {
	THIEVERY = 0, ARMS_MASTER = 1, ASTROLOGER = 2, BODYBUILDER = 3,
	CARTOGRAPHER = 4, CRUSADER = 5, DIRECTION_SENSE = 6, LINGUIST = 7,
	MERCHANT = 8, MOUNTAINEER = 9, NAVIGATOR = 10, PATHFINDER = 11,
	PRAYER_MASTER = 12, PRESTIDIGITATION = 13, SWIMMING = 14, TRACKING = 15,
	SPOT_DOORS = 16, DANGER_SENSE = 17
};

enum Condition {
	CURSED = 0, HEART_BROKEN = 1, WEAK = 2, POISONED = 3,
	DISEASED = 4, INSANE = 5, IN_LOVE = 6, DRUNK = 7, ASLEEP = 8,
	DEPRESSED = 9, CONFUSED = 10, PARALYZED = 11, UNCONSCIOUS = 12,
	DEAD = 13, STONED = 14, ERADICATED = 15,
	NO_CONDITION = 16
};

enum AttributeCategory {
	ATTR_MIGHT = 0, ATTR_INTELLECT = 1, ATTR_PERSONALITY = 2, ATTR_SPEED = 3,
	ATTR_ACCURACY = 4, ATTR_LUCK = 5, ATTR_HIT_POINTS = 6, ATTR_SPELL_POINTS = 7,
	ATTR_ARMOR_CLASS = 8, ATTR_THIEVERY  = 9
};

enum QuickAction {
	QUICK_ATTACK = 0, QUICK_SPELL = 1, QUICK_BLOCK = 2, QUICK_RUN = 3
};

class XeenEngine;
class Character;

class XeenItem {
public:
	int _material;
	uint _id;
	int _bonusFlags;
	int _frame;
public:
	/**
	 * Return the name of the item
	 */
	static const char *getItemName(ItemCategory category, uint id);
public:
	XeenItem();

	/**
	 * Clear the data for the item
	 */
	void clear();

	/**
	 * Returns true if no item is set
	 */
	bool empty() const { return _id != 0; }

	/**
	 * Synchronizes the data for the item
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Gets the elemental category for the item
	 */
	ElementalCategory getElementalCategory() const;

	/**
	 * Gets the attribute category for the item
	 */
	AttributeCategory getAttributeCategory() const;
};

class InventoryItems : public Common::Array<XeenItem> {
protected:
	Character *_character;
	ItemCategory _category;
	const char *const *_names;

	XeenEngine *getVm();
	void equipError(int itemIndex1, ItemCategory category1, int itemIndex2,
		ItemCategory category2);

	virtual Common::String getAttributes(XeenItem &item, const Common::String &classes) = 0;
public:
	InventoryItems(Character *character, ItemCategory category);
	virtual ~InventoryItems() {}

	/**
	 * Clears the set of items
	 */
	void clear();

	/**
	 * Return whether a given item passes class-based usage restrictions
	 */
	bool passRestrictions(int itemId, bool showError) const;

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
	virtual Common::String getAttributes(XeenItem &item, const Common::String &classes);
public:
	WeaponItems(Character *character) : InventoryItems(character, CATEGORY_WEAPON) {}
	virtual ~WeaponItems() {}

	/**
	 * Equip a given weapon
	 */
	virtual void equipItem(int itemIndex);

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	virtual Common::String getFullDescription(int itemIndex, int displayNum);

	/**
	 * Enchants a weapon
	 */
	virtual void enchantItem(int itemIndex, int amount);
};

class ArmorItems : public InventoryItems {
protected:
	virtual Common::String getAttributes(XeenItem &item, const Common::String &classes);
public:
	ArmorItems(Character *character) : InventoryItems(character, CATEGORY_ARMOR) {}
	virtual ~ArmorItems() {}

	/**
	 * Equip a given piece of armor
	 */
	virtual void equipItem(int itemIndex);

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	virtual Common::String getFullDescription(int itemIndex, int displayNum);

	/**
	 * Enchants an armor
	 */
	virtual void enchantItem(int itemIndex, int amount);
};

class AccessoryItems : public InventoryItems {
protected:
	virtual Common::String getAttributes(XeenItem &item, const Common::String &classes);
public:
	AccessoryItems(Character *character) : InventoryItems(character, CATEGORY_ACCESSORY) {}

	/**
	 * Equip a given accessory
	 */
	virtual void equipItem(int itemIndex);

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	virtual Common::String getFullDescription(int itemIndex, int displayNum);
};

class MiscItems : public InventoryItems {
protected:
	virtual Common::String getAttributes(XeenItem &item, const Common::String &classes);
public:
	MiscItems(Character *character) : InventoryItems(character, CATEGORY_MISC) {}
	virtual ~MiscItems() {}

	/**
	 * Assembles a full lines description for a specified item for use in
	 * the Items dialog
	 */
	virtual Common::String getFullDescription(int itemIndex, int displayNum);
};

class InventoryItemsGroup {
private:
	InventoryItems *_itemSets[4];
public:
	InventoryItemsGroup(InventoryItems &weapons, InventoryItems &armor,
		InventoryItems &accessories, InventoryItems &misc);

	InventoryItems &operator[](ItemCategory category);

	/**
	 * Breaks all the items in a given character's inventory
	 */
	void breakAllItems();
};


class AttributePair {
public:
	uint _permanent;
	uint _temporary;
public:
	AttributePair();
	void synchronize(Common::Serializer &s);
};

class Character {
private:
	/**
	 * Modifies a passed attribute value based on player's condition
	 */
	int conditionMod(Attribute attrib) const;
public:
	Common::String _name;
	Sex _sex;
	Race _race;
	int _xeenSide;
	CharacterClass _class;
	AttributePair _might;
	AttributePair _intellect;
	AttributePair _personality;
	AttributePair _endurance;
	AttributePair _speed;
	AttributePair _accuracy;
	AttributePair _luck;
	int _ACTemp;
	AttributePair _level;
	uint _birthDay;
	int _tempAge;
	int _skills[18];
	int _awards[128];
	int _spells[MAX_SPELLS_PER_CLASS];
	int _lloydMap;
	Common::Point _lloydPosition;
	bool _hasSpells;
	int8 _currentSpell;
	QuickAction _quickOption;
	InventoryItemsGroup _items;
	WeaponItems _weapons;
	ArmorItems _armor;
	AccessoryItems _accessories;
	MiscItems _misc;
	int _lloydSide;
	AttributePair _fireResistence;
	AttributePair _coldResistence;
	AttributePair _electricityResistence;
	AttributePair _poisonResistence;
	AttributePair _energyResistence;
	AttributePair _magicResistence;
	int _conditions[16];
	int _townUnknown;
	int _savedMazeId;
	int _currentHp;
	int _currentSp;
	uint _birthYear;
	uint32 _experience;
	int _currentAdventuringSpell;
	int _currentCombatSpell;

	SpriteResource *_faceSprites;
	int _rosterId;
public:
	Character();

	/**
	 * Clears the data for a character
	 */
	void clear();

	/**
	 * Synchronizes data for the character
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Returns the worst condition the character is suffering from
	 */
	Condition worstCondition() const;

	/**
	 * Returns whether the given character has a disabling condition, but still alive
	 */
	bool isDisabled() const;

	/**
	 * Returns whether the given character has a disabling condition, or is dead
	 */
	bool isDisabledOrDead() const;

	/**
	 * Returns whether the given character has a dead condition
	 */
	bool isDead() const;

	/**
	 * Get the character's age
	 */
	int getAge(bool ignoreTemp = false) const;

	/**
	 * Gets the maximum hit points for a character
	 */
	int getMaxHP() const;

	/**
	 * Gets the maximum spell points for a character
	 */
	int getMaxSP() const;

	/**
	 * Get the effective value of a given stat for the character
	 */
	uint getStat(Attribute attrib, bool baseOnly = false) const;

	/**
	 * Return the color number to use for a given stat value in the character
	 * info or quick reference dialogs
	 */
	static int statColor(int amount, int threshold);

	/**
	 * Returns the bonus the character gets for stats
	 */
	int statBonus(uint statValue) const;

	/**
	 * Returns true if the character passes a saving throw for a given attack type
	 */
	bool charSavingThrow(DamageType attackType) const;

	/**
	 * Returns true if the character is unable to perform any action
	 */
	bool noActions();

	/**
	 * Sets an award status
	 */
	void setAward(int awardId, bool value);

	/**
	 * Returns true if a character has a given award
	 */
	bool hasAward(int awardId) const;

	/**
	 * Returns the character's armor class
	 */
	int getArmorClass(bool baseOnly = false) const;

	/**
	 * Returns the thievery skill level, adjusted by class and race
	 */
	int getThievery() const;

	uint getCurrentLevel() const;

	/**
	 * Scans the character's inventory for the given item
	 */
	int itemScan(int itemId) const;

	/**
	 * Sets various attributes of a character
	 */
	void setValue(int id, uint value);

	/**
	 * Returns true if the character is a member of the current town's guild
	 */
	bool guildMember() const;

	/**
	 * Returns the experience required to reach the next level
	 */
	uint experienceToNextLevel() const;

	/**
	 * Returns the next level the character will reach
	 */
	uint nextExperienceLevel() const;

	/**
	 * Returns the character's current experience
	 */
	uint getCurrentExperience() const;

	/**
	 * Returns the number of skills the character has
	 */
	int getNumSkills() const;

	/**
	 * Returns the number of awards the character has
	 */
	int getNumAwards() const;

	/**
	 * Creates an item and adds it to the inventory
	 */
	int makeItem(int p1, int itemIndex, int p3);

	/**
	 * Add hit points to a character
	 */
	void addHitPoints(int amount);

	/**
	 * Remove hit points from the character
	 */
	void subtractHitPoints(int amount);

	/**
	 * Returns true if the character has the Xeen Slayer Sword
	 */
	bool hasSlayerSword() const;

	/**
	 * Returns true if the character has a missile weapon, such as a bow
	 */
	bool hasMissileWeapon() const;

	/**
	 * Returns a category index for a character, used such for indexing into spell data
	 */
	int getClassCategory() const;
};

} // End of namespace Xeen

#endif /* XEEN_CHARACTER_H */
