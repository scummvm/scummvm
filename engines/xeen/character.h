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
#include "xeen/item.h"
#include "xeen/sprites.h"

namespace Xeen {

#define INV_ITEMS_TOTAL 9
#define SPELLS_PER_CLASS 39
#define AWARDS_TOTAL 88
#define WARZONE_AWARD 9

enum Award {
	SHANGRILA_GUILD_MEMBER = 5, GOOBER = 76, SUPER_GOOBER = 77,
	CASTLEVIEW_GUILD_MEMBER = 83, SANDCASTER_GUILD_MEMBER = 84,
	LAKESIDE_GUILD_MEMBER = 85, NECROPOLIS_GUILD_MEMBER = 86, OLYMPUS_GUILD_MEMBER = 87
};

enum Sex { MALE = 0, FEMALE = 1, YES_PLEASE = 2 };

enum Race { HUMAN = 0, ELF = 1, DWARF = 2, GNOME = 3, HALF_ORC = 4 };

enum CharacterClass {
	CLASS_KNIGHT = 0, CLASS_PALADIN = 1, CLASS_ARCHER = 2, CLASS_CLERIC = 3,
	CLASS_SORCERER = 4, CLASS_ROBBER = 5, CLASS_NINJA = 6, CLASS_BARBARIAN = 7,
	CLASS_DRUID = 8, CLASS_RANGER = 9, TOTAL_CLASSES = 10
};

enum HatesClass {
	HATES_DWARF = 12, HATES_PARTY = 15, HATES_NOBODY = 16
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

enum QuickAction {
	QUICK_ATTACK = 0, QUICK_SPELL = 1, QUICK_BLOCK = 2, QUICK_RUN = 3
};

enum SpellsCategory {
	SPELLCAT_INVALID = -1, SPELLCAT_CLERICAL = 0, SPELLCAT_WIZARDRY = 1, SPELLCAT_DRUIDIC = 2
};


class XeenEngine;

class AttributePair {
public:
	int _permanent;
	int _temporary;
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
	bool _spells[SPELLS_PER_CLASS];
	int _lloydMap;
	Common::Point _lloydPosition;
	bool _hasSpells;
	int8 _currentSpell;
	QuickAction _quickOption;
	WeaponItems _weapons;
	ArmorItems _armor;
	AccessoryItems _accessories;
	MiscItems _misc;
	InventoryItemsGroup _items;
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
	static int _itemType;
public:
	/**
	 * Constructor
	 */
	Character();

	/**
	 * Constructor
	 */
	Character(const Character &src);

	/**
	 * Equality operator
	 */
	bool operator==(const Character &src) const { return src._rosterId == _rosterId; }

	/**
	 * Inequality operator
	 */
	bool operator!=(const Character &src) const { return src._rosterId != _rosterId; }

	/**
	 * Clears the data for a character
	 */
	void clear();

	/**
	 * Assignment operator
	 */
	Character &operator=(const Character &src);

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
	 * Returns the number of times a character has received a given award
	 */
	int getAwardCount(int awardId) const;

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
	ItemCategory makeItem(int p1, int itemIndex, int p3);

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
	 * Returns the spells category for the character's class
	 */
	SpellsCategory getSpellsCategory() const;

	/**
	 * Returns an expense factor for purchasing spells by certain character classes
	 */
	int getSpellsExpenseFactor() const {
		return (_class == CLASS_PALADIN || _class == CLASS_ARCHER || _class == CLASS_RANGER) ? 1 : 0;
	}

	/**
	 * Clears the character of any currently set conditions
	 */
	void clearConditions();
};

class CharacterArray : public Common::Array<Character> {
public:
	/**
	 * Returns the index of a given character in the array
	 */
	int indexOf(const Character &c);
};

} // End of namespace Xeen

#endif /* XEEN_CHARACTER_H */
