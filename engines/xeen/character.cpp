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

#include "xeen/character.h"
#include "xeen/dialogs_error.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

XeenItem::XeenItem() {
	clear();
}

void XeenItem::clear() {
	_material = _id = _bonusFlags = 0;
	_frame = 0;
}

void XeenItem::synchronize(Common::Serializer &s) {
	s.syncAsByte(_material);
	s.syncAsByte(_id);
	s.syncAsByte(_bonusFlags);
	s.syncAsByte(_frame);
}

int XeenItem::getElementalCategory() const {
	int idx;
	for (idx = 0; ELEMENTAL_CATEGORIES[idx] < _material; ++idx)
		;

	return idx;
}

int XeenItem::getAttributeCategory() const {
	int m = _material - 59;
	int idx;
	for (idx = 0; ATTRIBUTE_CATEGORIES[idx] < m; ++idx)
		;

	return idx;
}

/*------------------------------------------------------------------------*/

InventoryItems::InventoryItems(Character *character, ItemCategory category):
		_character(character), _category(category) {
	resize(INV_ITEMS_TOTAL);

	static const char *const *NAMES[4] = { 
		WEAPON_NAMES, ARMOR_NAMES, ACCESSORY_NAMES, MISC_NAMES 
	};
	_names = NAMES[category];
}

/**
* Return whether a given item passes class-based usage restrictions
*/
bool InventoryItems::passRestrictions(int itemId, bool showError) const {
	CharacterClass charClass = _character->_class;

	switch (charClass) {
	case CLASS_KNIGHT:
	case CLASS_PALADIN:
		return true;

	case CLASS_ARCHER:
	case CLASS_CLERIC:
	case CLASS_SORCERER:
	case CLASS_ROBBER:
	case CLASS_NINJA:
	case CLASS_BARBARIAN:
	case CLASS_DRUID:
	case CLASS_RANGER: {
		if (!(ITEM_RESTRICTIONS[itemId + RESTRICTION_OFFSETS[_category]] &
			(1 << (charClass - CLASS_ARCHER))))
			return true;
		break;
	}

	default:
		break;
	}

	Common::String name = _names[itemId];
	if (showError) {
		Common::String msg = Common::String::format(NOT_PROFICIENT,
			CLASS_NAMES[charClass], name.c_str());
		ErrorScroll::show(Party::_vm, msg, WT_FREEZE_WAIT);
	}

	return false;
}

/**
 * Return the bare name of a given inventory item
 */
Common::String InventoryItems::getName(int itemIndex) {
	int id = operator[](itemIndex)._id;
	return _names[id];
}

/**
 * Discard an item from the inventory
 */
void InventoryItems::discardItem(int itemIndex) {
	operator[](itemIndex).clear();
	sort();
}

/**
 * Sorts the items list, removing any empty item slots to the end of the array
 */
void InventoryItems::sort() {
	for (uint idx = 0; idx < size(); ++idx) {
		if (operator[](idx)._id == 0) {
			// Found empty slot
			operator[](idx).clear();

			// Scan through the rest of the list to find any item
			for (uint idx2 = idx + 1; idx2 < size(); ++idx2) {
				if (operator[](idx2)._id) {
					// Found an item, so move it into the blank slot
					operator[](idx) = operator[](idx2);
					operator[](idx2).clear();
					break;
				}
			}
		}
	}
}

void InventoryItems::removeItem(int itemIndex) {
	error("TODO");
}

void InventoryItems::equipError(int itemIndex1, ItemCategory category1, int itemIndex2,
		ItemCategory category2) {
	XeenEngine *vm = Party::_vm;

	if (itemIndex1 >= 0) {
		Common::String itemName1 = _character->_items[category1].getName(itemIndex1);
		Common::String itemName2 = _character->_items[category2].getName(itemIndex2);

		ErrorDialog::show(vm, Common::String::format(REMOVE_X_TO_EQUIP_Y,
			itemName1.c_str(), itemName2.c_str()));
	} else {
		ErrorDialog::show(vm, Common::String::format(EQUIPPED_ALL_YOU_CAN,
			(itemIndex1 == -1) ? RING : MEDAL));
	}
}

/*------------------------------------------------------------------------*/

/**
 * Equip a given weapon
 */
void WeaponItems::equipItem(int itemIndex) {
	XeenItem &item = operator[](itemIndex);

	if (item._id <= 17) {
		if (passRestrictions(item._id, false)) {
			for (uint idx = 0; idx < size(); ++idx) {
				XeenItem &i = operator[](idx);
				if (i._frame == 13 || i._frame == 1) {
					equipError(itemIndex, CATEGORY_WEAPON, idx, CATEGORY_WEAPON);
					return;
				}
			}

			item._frame = 1;
		}
	} else if (item._id >= 30 && item._id <= 33) {
		if (passRestrictions(item._id, false)) {
			for (uint idx = 0; idx < size(); ++idx) {
				XeenItem &i = operator[](idx);
				if (i._frame == 4) {
					equipError(itemIndex, CATEGORY_WEAPON, idx, CATEGORY_WEAPON);
					return;
				}
			}

			item._frame = 4;
		}
	} else {
		if (passRestrictions(item._id, false)) {
			for (uint idx = 0; idx < size(); ++idx) {
				XeenItem &i = operator[](idx);
				if (i._frame == 13 || i._frame == 1) {
					equipError(itemIndex, CATEGORY_WEAPON, idx, CATEGORY_WEAPON);
					return;
				}
			}

			for (uint idx = 0; idx < _character->_armor.size(); ++idx) {
				XeenItem &i = _character->_armor[idx];
				if (i._frame == 2) {
					equipError(itemIndex, CATEGORY_WEAPON, idx, CATEGORY_ARMOR);
					return;
				}
			}

			item._frame = 13;
		}
	}
}

/*------------------------------------------------------------------------*/

/**
 * Equip a given piece of armor
 */
void ArmorItems::equipItem(int itemIndex) {
	XeenItem &item = operator[](itemIndex);

	if (item._id <= 7) {
		if (passRestrictions(item._id, false)) {
			for (uint idx = 0; idx < size(); ++idx) {
				XeenItem &i = operator[](idx);
				if (i._frame == 9) {
					equipError(itemIndex, CATEGORY_ARMOR, idx, CATEGORY_ARMOR);
					return;
				}
			}

			item._frame = 3;
		}
	} else if (item._id == 8) {
		if (passRestrictions(item._id, false)) {
			for (uint idx = 0; idx < size(); ++idx) {
				XeenItem &i = operator[](idx);
				if (i._frame == 2) {
					equipError(itemIndex, CATEGORY_ARMOR, idx, CATEGORY_ARMOR);
					return;
				}
			}

			for (uint idx = 0; idx < _character->_weapons.size(); ++idx) {
				XeenItem &i = _character->_weapons[idx];
				if (i._frame == 13) {
					equipError(itemIndex, CATEGORY_ARMOR, idx, CATEGORY_WEAPON);
					return;
				}
			}

			item._frame = 2;
		}
	} else if (item._id == 9) {
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 5) {
				equipError(itemIndex, CATEGORY_ARMOR, idx, CATEGORY_ARMOR);
				return;
			}
		}

		item._frame = 5;
	} else if (item._id == 10) {
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 9) {
				equipError(itemIndex, CATEGORY_ARMOR, idx, CATEGORY_ARMOR);
				return;
			}
		}

		item._frame = 9;
	} else if (item._id <= 12) {
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 10) {
				equipError(itemIndex, CATEGORY_ARMOR, idx, CATEGORY_ARMOR);
				return;
			}
		}

		item._frame = 10;
	} else {
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 6) {
				equipError(itemIndex, CATEGORY_ARMOR, idx, CATEGORY_ARMOR);
				return;
			}
		}

		item._frame = 6;
	}
}

/*------------------------------------------------------------------------*/

/**
 * Equip a given accessory
 */
void AccessoryItems::equipItem(int itemIndex) {
	XeenItem &item = operator[](itemIndex);

	if (item._id == 1) {
		int count = 0;
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 8)
				++count;
		}

		if (count <= 1)
			item._frame = 8;
		else
			equipError(-1, CATEGORY_ACCESSORY, itemIndex, CATEGORY_ACCESSORY);
	} else if (item._id == 2) {
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 12) {
				equipError(itemIndex, CATEGORY_ACCESSORY, idx, CATEGORY_ACCESSORY);
				return;
			}
		}
	} else if (item._id <= 7) {
		int count = 0;
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 7)
				++count;
		}

		if (count <= 1)
			item._frame = 7;
		else
			equipError(-2, CATEGORY_ACCESSORY, itemIndex, CATEGORY_ACCESSORY);
	} else {
		for (uint idx = 0; idx < size(); ++idx) {
			XeenItem &i = operator[](idx);
			if (i._frame == 11) {
				equipError(itemIndex, CATEGORY_ACCESSORY, idx, CATEGORY_ACCESSORY);
				return;
			}
		}

		item._frame = 11;
	}
}
/*------------------------------------------------------------------------*/

InventoryItemsGroup::InventoryItemsGroup(InventoryItems &weapons, InventoryItems &armor,
		InventoryItems &accessories, InventoryItems &misc) {
	_itemSets[0] = &weapons;
	_itemSets[1] = &armor;
	_itemSets[2] = &accessories;
	_itemSets[3] = &misc;
}

InventoryItems &InventoryItemsGroup::operator[](ItemCategory category) {
	return *_itemSets[category];
}

/*------------------------------------------------------------------------*/


void AttributePair::synchronize(Common::Serializer &s) {
	s.syncAsByte(_permanent);
	s.syncAsByte(_temporary);
}

/*------------------------------------------------------------------------*/

AttributePair::AttributePair() {
	_temporary = _permanent = 0;
}

/*------------------------------------------------------------------------*/

Character::Character():
		_weapons(this), _armor(this), _accessories(this), _misc(this),
		_items(_weapons, _armor, _accessories, _misc) {
	_sex = MALE;
	_race = HUMAN;
	_xeenSide = 0;
	_class = CLASS_KNIGHT;
	_ACTemp = 0;
	_birthDay = 0;
	_tempAge = 0;
	Common::fill(&_skills[0], &_skills[18], 0);
	Common::fill(&_awards[0], &_awards[128], false);
	Common::fill(&_spells[0], &_spells[39], 0);
	_lloydMap = 0;
	_hasSpells = false;
	_currentSpell = 0;
	_quickOption = 0;
	_lloydSide = 0;
	Common::fill(&_conditions[0], &_conditions[16], 0);
	_townUnknown = 0;
	_savedMazeId = 0;
	_currentHp = 0;
	_currentSp = 0;
	_birthYear = 0;
	_experience = 0;
	_currentAdventuringSpell = 0;
	_currentCombatSpell = 0;
}

void Character::synchronize(Common::Serializer &s) {
	char name[16];
	Common::fill(&name[0], &name[16], '\0');
	strncpy(name, _name.c_str(), 16);
	s.syncBytes((byte *)name, 16);

	if (s.isLoading())
		_name = Common::String(name);

	s.syncAsByte(_sex);
	s.syncAsByte(_race);
	s.syncAsByte(_xeenSide);
	s.syncAsByte(_class);

	_might.synchronize(s);
	_intellect.synchronize(s);
	_personality.synchronize(s);
	_endurance.synchronize(s);
	_speed.synchronize(s);
	_accuracy.synchronize(s);
	_luck.synchronize(s);
	s.syncAsByte(_ACTemp);
	_level.synchronize(s);
	s.syncAsByte(_birthDay);
	s.syncAsByte(_tempAge);
	
	// Synchronize the skill list
	for (int idx = 0; idx < 18; ++idx)
		s.syncAsByte(_skills[idx]);

	// Synchronize character awards
	for (int idx = 0; idx < 64; ++idx) {
		byte b = (_awards[idx] ? 1 : 0) | (_awards[idx + 64] ? 0x10 : 0);
		s.syncAsByte(b);
		if (s.isLoading()) {
			_awards[idx] = (b & 0xF) != 0;
			_awards[idx + 64] = (b & 0xF0) != 0;
		}
	}

	// Synchronize spell list
	for (int i = 0; i < MAX_SPELLS_PER_CLASS - 1; ++i)
		s.syncAsByte(_spells[i]);	
	s.syncAsByte(_lloydMap);
	s.syncAsByte(_lloydPosition.x);
	s.syncAsByte(_lloydPosition.y);
	s.syncAsByte(_hasSpells);
	s.syncAsByte(_currentSpell);
	s.syncAsByte(_quickOption);

	for (int i = 0; i < 9; ++i)
		_weapons[i].synchronize(s);
	for (int i = 0; i < 9; ++i)
		_armor[i].synchronize(s);
	for (int i = 0; i < 9; ++i)
		_accessories[i].synchronize(s);
	for (int i = 0; i < 9; ++i)
		_misc[i].synchronize(s);

	s.syncAsByte(_lloydSide);
	_fireResistence.synchronize(s);
	_coldResistence.synchronize(s);
	_electricityResistence.synchronize(s);
	_poisonResistence.synchronize(s);
	_energyResistence.synchronize(s);
	_magicResistence.synchronize(s);
	
	for (int i = 0; i < 16; ++i)
		s.syncAsByte(_conditions[i]);

	s.syncAsUint16LE(_townUnknown);
	s.syncAsByte(_savedMazeId);
	s.syncAsUint16LE(_currentHp);
	s.syncAsUint16LE(_currentSp);
	s.syncAsUint16LE(_birthYear);
	s.syncAsUint32LE(_experience);
	s.syncAsByte(_currentAdventuringSpell);
	s.syncAsByte(_currentCombatSpell);
}

Condition Character::worstCondition() const {
	for (int cond = ERADICATED; cond >= CURSED; --cond) {
		if (_conditions[cond])
			return (Condition)cond;
	}

	return NO_CONDITION;
}

int Character::getAge(bool ignoreTemp) const {
	int year = MIN(Party::_vm->_party->_year - _birthYear, (uint)254);

	return ignoreTemp ? year : year + _tempAge;
}

int Character::getMaxHP() const {
	int hp = BASE_HP_BY_CLASS[_class];
	hp += statBonus(getStat(ENDURANCE));
	hp += RACE_HP_BONUSES[_race];
	if (_skills[BODYBUILDER])
		++hp;
	if (hp < 1)
		hp = 1;

	hp *= getCurrentLevel();
	hp += itemScan(7);

	return MAX(hp, 0);
}

int Character::getMaxSP() const {
	int result = 0;
	bool flag = false;
	int amount = 0;
	Attribute attrib;
	Skill skill;

	if (!_hasSpells)
		return 0;

	if (_class == CLASS_SORCERER || _class == CLASS_ARCHER) {
		attrib = INTELLECT;
		skill = PRESTIDIGITATION;
	} else {
		attrib = PERSONALITY;
		skill = PRAYER_MASTER;
	}
	if (_class == CLASS_DRUID || _class == CLASS_RANGER)
		skill = ASTROLOGER;

	for (;;) {
		// Get the base number of spell points
		result = statBonus(getStat(attrib)) + 3;
		result += RACE_SP_BONUSES[_race][attrib - 1];

		if (_skills[skill])
			result += 2;
		if (result < 1)
			result = 1;

		// Multiply it by the character's level
		result *= getCurrentLevel();

		// Classes other than sorcerer, clerics, and druids only get half the SP
		if (_class != CLASS_SORCERER && _class != CLASS_CLERIC && _class != CLASS_DRUID)
			result /= 2;

		if (flag || (_class != CLASS_DRUID && _class != CLASS_RANGER))
			break;

		// Druids and rangers get bonuses averaged on both personality and intellect
		attrib = INTELLECT;
		flag = true;
		amount = result;
	}
	if (flag)
		result = (amount + result) / 2;

	result += itemScan(8);
	if (result < 0)
		result = 0;

	return result;
}

/**
 * Get the effective value of a given stat for the character
 */
uint Character::getStat(Attribute attrib, bool baseOnly) const {
	AttributePair attr;
	int mode = 0;

	switch (attrib) {
	case MIGHT:
		attr = _might;
		break;
	case INTELLECT:
		attr = _intellect;
		mode = 1;
		break;
	case PERSONALITY:
		attr = _personality;
		mode = 1;
		break;
	case ENDURANCE:
		attr = _endurance;
		break;
	case SPEED:
		attr = _speed;
		break;
	case ACCURACY:
		attr = _accuracy;
		break;
	case LUCK:
		attr = _luck;
		mode = 2;
		break;
	default:
		return 0;
	}

	// All the attributes except luck are affected by the character's age
	if (mode < 2) {
		int age = getAge(false);
		int ageIndex = 0;
		while (AGE_RANGES[ageIndex] <= age)
			++ageIndex;

		attr._permanent += AGE_RANGES_ADJUST[mode][ageIndex];
	}


	attr._permanent += itemScan((int)attrib);

	if (!baseOnly) {
		attr._permanent += conditionMod(attrib);
		attr._permanent += attr._temporary;
	}

	return MAX(attr._permanent, (uint)0);
}

/**
 * Return the color number to use for a given stat value in the character
 * info or quick reference dialogs
 */
int Character::statColor(int amount, int threshold) {
	if (amount < 1)
		return 6;
	else if (amount > threshold)
		return 2;
	else if (amount == threshold)
		return 15;
	else if (amount <= (threshold / 4))
		return 9;
	else
		return 32;
}

int Character::statBonus(uint statValue) const {
	int idx;
	for (idx = 0; STAT_VALUES[idx] <= statValue; ++idx)
		;

	return STAT_BONUSES[idx];
}

bool Character::charSavingThrow(DamageType attackType) const {
	int v, vMax;

	if (attackType == DT_PHYSICAL) {
		v = statBonus(getStat(LUCK)) + getCurrentLevel();
		vMax = v + 20;
	} else {
		switch (attackType) {
		case DT_MAGICAL:
			v = _magicResistence._permanent + _magicResistence._temporary + itemScan(16);
			break;
		case DT_FIRE:
			v = _fireResistence._permanent + _fireResistence._temporary + itemScan(11);
			break;
		case DT_ELECTRICAL:
			v = _electricityResistence._permanent + _electricityResistence._temporary + itemScan(12);
			break;
		case DT_COLD:
			v = _coldResistence._permanent + _coldResistence._temporary + itemScan(13);
			break;
		case DT_POISON:
			v = _poisonResistence._permanent + _poisonResistence._temporary + itemScan(14);
			break;
		case DT_ENERGY:
			v = _energyResistence._permanent + _energyResistence._temporary + itemScan(15);
			break;
		default:
			v = 0;
			break;
		}

		vMax = v + 40;
	}

	return Party::_vm->getRandomNumber(1, vMax) <= v;
}

bool Character::noActions() {
	Condition condition = worstCondition();

	switch (condition) {
	case CURSED:
	case POISONED:
	case DISEASED:
	case INSANE:
	case IN_LOVE:
	case DRUNK: {
		Common::String msg = Common::String::format(IN_NO_CONDITION, _name.c_str());
		ErrorScroll::show(Party::_vm, msg, 
			Party::_vm->_mode == 17 ? WT_2 : WT_NONFREEZED_WAIT);
		return true;
	}
	default:
		return false;
	}
}

void Character::setAward(int awardId, bool value) {
	int v = awardId;
	if (awardId == 73)
		v = 126;
	else if (awardId == 81)
		v = 127;

	_awards[v] = value;
}

bool Character::hasAward(int awardId) const {
	int v = awardId;
	if (awardId == 73)
		v = 126;
	else if (awardId == 81)
		v = 127;

	return _awards[v];
}

int Character::getArmorClass(bool baseOnly) const {
	Party &party = *Party::_vm->_party;

	int result = statBonus(getStat(SPEED)) + itemScan(9);
	if (!baseOnly)
		result += party._blessed + _ACTemp;

	return MAX(result, 0);
}

/**
 * Returns the thievery skill level, adjusted by class and race
 */
int Character::getThievery() const {
	int result = getCurrentLevel() * 2;

	if (_class == CLASS_NINJA)
		result += 15;
	else if (_class == CLASS_ROBBER)
		result += 30;

	switch (_race) {
	case ELF:
	case GNOME:
		result += 10;
		break;
	case DWARF:
		result += 5;
		break;
	case HALF_ORC:
		result -= 10;
		break;
	default:
		break;
	}

	result += itemScan(10);

	// If the character doesn't have a thievery skill, then do'nt allow any result
	if (!_skills[THIEVERY])
		result = 0;

	return MAX(result, 0);
}

uint Character::getCurrentLevel() const {
	return MAX(_level._permanent + _level._temporary, (uint)0);
}

int Character::itemScan(int itemId) const {
	int result = 0;

	for (int accessIdx = 0; accessIdx < 3; ++accessIdx) {
		switch (accessIdx) {
		case 0:
			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				const XeenItem &item = _weapons[idx];

				if (item._frame && !(item._bonusFlags & 0xC0) && itemId < 11
						&& itemId != 3 && item._material >= 59 && item._material <= 130) {
					int mIndex = item.getAttributeCategory();
					if (mIndex > 2)
						++mIndex;

					if (mIndex == itemId)
						result += ATTRIBUTE_BONUSES[item._material - 59];
				}
			}
			break;

		case 1:
			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				const XeenItem &item = _armor[idx];

				if (item._frame && !(item._bonusFlags & 0xC0)) {
					if (itemId < 11 && itemId != 3 && item._material >= 59 && item._material <= 130) {
						int mIndex = item.getAttributeCategory();
						if (mIndex > 2)
							++mIndex;

						if (mIndex == itemId)
							result += ATTRIBUTE_BONUSES[item._material - 59];
					}

					if (itemId > 10 && item._material < 37) {
						int mIndex = item.getElementalCategory() + 11;

						if (mIndex == itemId) {
							result += ELEMENTAL_RESISTENCES[item._material];
						}
					}

					if (itemId == 9) {
						result += ARMOR_STRENGTHS[item._id];

						if (item._material >= 37 && item._material <= 58)
							result += METAL_LAC[item._material - 37];
					}
				}
			}
			break;

		case 2:
			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				const XeenItem &item = _accessories[idx];

				if (item._frame && !(item._bonusFlags & 0xC0) && itemId < 11 && itemId != 3) {
					if (item._material >= 59 && item._material <= 130) {
						int mIndex = item.getAttributeCategory();
						if (mIndex > 2)
							++mIndex;

						if (mIndex == itemId) {
							result += ATTRIBUTE_BONUSES[item._material - 59];
						}
					}

					if (itemId > 10 && item._material < 37) {
						int mIndex = item.getElementalCategory() + 11;
						
						if (mIndex == itemId)
							result += ELEMENTAL_RESISTENCES[item._material];
					}
				}
			}
			break;
		}
	};

	return result;
}

/**
 * Modifies a passed attribute value based on player's condition
 */
int Character::conditionMod(Attribute attrib) const {
	if (_conditions[DEAD] || _conditions[STONED] || _conditions[ERADICATED])
		return 0;

	int v[7];
	Common::fill(&v[0], &v[7], 0);
	if (_conditions[CURSED])
		v[6] -= _conditions[CURSED];

	if (_conditions[INSANE]) {
		v[2] -= _conditions[INSANE];
		v[1] -= _conditions[INSANE];
		v[5] -= _conditions[INSANE];
		v[0] -= _conditions[INSANE];
		v[4] -= _conditions[INSANE];
	}

	if (_conditions[POISONED]) {
		v[0] -= _conditions[POISONED];
		v[4] -= _conditions[POISONED];
		v[5] -= _conditions[POISONED];
	}

	if (_conditions[DISEASED]) {
		v[3] -= _conditions[DISEASED];
		v[2] -= _conditions[DISEASED];
		v[1] -= _conditions[DISEASED];
	}

	for (int idx = 0; idx < 7; ++idx) {
		v[idx] -= _conditions[HEART_BROKEN];
		v[idx] -= _conditions[IN_LOVE];
		v[idx] -= _conditions[WEAK];
		v[idx] -= _conditions[DRUNK];
	}

	return v[attrib];
}

void Character::setValue(int id, uint value) {
	Party &party = *Party::_vm->_party;
	Scripts &scripts = *Party::_vm->_scripts;

	switch (id) {
	case 3:
		// Set character sex
		_sex = (Sex)value;
		break;
	case 4:
		// Set race
		_race = (Race)value;
		break;
	case 5:
		// Set class
		_class = (CharacterClass)value;
		break;
	case 8:
		// Set the current Hp
		_currentHp = value;
		break;
	case 9:
		// Set the current Sp
		_currentSp = value;
		break;
	case 10:
	case 77:
		// Set temporary armor class
		_ACTemp = value;
		break;
	case 11:
		// Set temporary level
		_level._temporary = value;
		break;
	case 12:
		// Set the character's temporary age
		_tempAge = value;
		break;
	case 16:
		// Set character experience
		_experience = value;
		break;
	case 17:
		// Set party poison resistence
		party._poisonResistence = value;
		break;
	case 18:
		// Set condition
		if (value == 16) {
			// Clear all the conditions
			Common::fill(&_conditions[CURSED], &_conditions[NO_CONDITION], false);
		} else if (value == 6) {
			_conditions[value] = 1;
		} else {
			++_conditions[value];
		}

		if (value >= DEAD && value <= ERADICATED && _currentHp > 0)
			_currentHp = 0;
		break;
	case 25:
		// Set time of day in minutes (0-1440)
		party._minutes = value;
		break;
	case 34:
		// Set party gold
		party._gold = value;
		break;
	case 35:
		// Set party gems
		party._gems = value;
		break;
	case 37:
		_might._temporary = value;
		break;
	case 38:
		_intellect._temporary = value;
		break;
	case 39:
		_personality._temporary = value;
		break;
	case 40:
		_endurance._temporary = value;
		break;
	case 41:
		_speed._temporary = value;
		break;
	case 42:
		_accuracy._temporary = value;
		break;
	case 43:
		_luck._temporary = value;
		break;
	case 45:
		_might._permanent = value;
		break;
	case 46:
		_intellect._permanent = value;
		break;
	case 47:
		_personality._permanent = value;
		break;
	case 48:
		_endurance._permanent = value;
		break;
	case 49:
		_speed._permanent = value;
		break;
	case 50:
		_accuracy._permanent = value;
		break;
	case 51:
		_luck._permanent = value;
		break;
	case 52:
		_fireResistence._permanent = value;
		break;
	case 53:
		_electricityResistence._permanent = value;
		break;
	case 54:
		_coldResistence._permanent = value;
		break;
	case 55:
		_poisonResistence._permanent = value;
		break;
	case 56:
		_energyResistence._permanent = value;
		break;
	case 57:
		_magicResistence._permanent = value;
		break;
	case 58:
		_fireResistence._temporary = value;
		break;
	case 59:
		_electricityResistence._temporary = value;
		break;
	case 60:
		_coldResistence._temporary = value;
		break;
	case 61:
		_poisonResistence._temporary = value;
		break;
	case 62:
		_energyResistence._temporary = value;
		break;
	case 63:
		_magicResistence._temporary = value;
		break;
	case 64:
		_level._permanent = value;
		break;
	case 65:
		// Set party food
		party._food = value;
		break;
	case 69:
		// Set levitate active
		party._levitateActive = value != 0;
		break;
	case 70:
		party._lightCount = value;
		break;
	case 71:
		party._fireResistence = value;
		break;
	case 72:
		party._electricityResistence = value;
		break;
	case 73:
		party._coldResistence = value;
		break;
	case 74:
		party._walkOnWaterActive = value != 0;
		party._poisonResistence = value;
		party._wizardEyeActive = value != 0;
		party._coldResistence = value;
		party._electricityResistence = value;
		party._fireResistence = value;
		party._lightCount = value;
		party._levitateActive = value != 0;
		break;
	case 76:
		// Set day of the year (0-99)
		party._day = value;
		break;
	case 79:
		party._wizardEyeActive = true;
		break;
	case 83:
		scripts._nEdamageType = value;
		break;
	case 84:
		party._mazeDirection = (Direction)value;
		break;
	case 85:
		party._year = value;
		break;
	case 94:
		party._walkOnWaterActive = value != 0;
		break;
	default:
		break;
	}
}

bool Character::guildMember() const {
	Party &party = *Party::_vm->_party;

	if (party._mazeId == 49 && !Party::_vm->_files->_isDarkCc) {
		return hasAward(5);
	}

	switch (party._mazeId) {
	case 29:
		return hasAward(83);
	case 31:
		return hasAward(84);
	case 33:
		return hasAward(85);
	case 35:
		return hasAward(86);
	default:
		return hasAward(87);
	}
}

uint Character::experienceToNextLevel() const {
	uint next = nextExperienceLevel();
	uint curr = getCurrentExperience();
	return (curr >= next) ? 0 : next - curr;
}

uint Character::nextExperienceLevel() const {
	int shift, base;
	if (_level._permanent >= 12) {
		base = _level._permanent - 12;
		shift = 10;
	} else {
		base = 0;
		shift = _level._permanent - 1;
	}

	return (base * 1024000) + (CLASS_EXP_LEVELS[_class] << shift);
}

uint Character::getCurrentExperience() const {
	int lev = _level._permanent - 1;
	int shift, base;

	if (lev > 0 && lev < 12)
		return _experience;

	if (lev >= 12) {
		base = lev - 12;
		shift = 10;
	} else {
		base = 0;
		shift = lev - 1;
	}

	return (base * 1024000) + (CLASS_EXP_LEVELS[_class] << shift) +
		_experience;
}


int Character::getNumSkills() const {
	int total = 0;
	for (int idx = THIEVERY; idx <= DANGER_SENSE; ++idx) {
		if (_skills[idx])
			++total;
	}

	return total;
}

int Character::getNumAwards() const {
	int total = 0;
	for (int idx = 0; idx < 88; ++idx) {
		if (hasAward(idx))
			++total;
	}

	return total;
}

/**
 * Assembles a full lines description for a specified item for use in
 * the Items dialog
 */
Common::String Character::assembleItemName(int itemIndex, int displayNum, 
		ItemCategory category) {
	Spells &spells = *Party::_vm->_spells;

	switch (category) {
	case CATEGORY_WEAPON: {
		// Weapons
		XeenItem &i = _weapons[itemIndex];
		return Common::String::format("\f%02u%s%s%s\f%02u%s%s%s", displayNum,
			!i._bonusFlags ? spells._maeNames[i._material] : "",
			(i._bonusFlags & ITEMFLAG_BROKEN) ? ITEM_BROKEN : "",
			(i._bonusFlags & ITEMFLAG_CURSED) ? ITEM_CURSED : "",
			WEAPON_NAMES[i._id],
			!i._bonusFlags ? "" : BONUS_NAMES[i._bonusFlags & ITEMFLAG_BONUS_MASK],
			(i._bonusFlags & (ITEMFLAG_BROKEN | ITEMFLAG_CURSED)) || 
				!i._bonusFlags ? "\b " : ""
		);
	}

	case CATEGORY_ARMOR: {
		// Armor
		XeenItem &i = _armor[itemIndex];
		return Common::String::format("\f%02u%s%s%s\f%02u%s%s", displayNum,
			!i._bonusFlags ? "" : spells._maeNames[i._material],
			(i._bonusFlags & ITEMFLAG_BROKEN) ? ITEM_BROKEN : "",
			(i._bonusFlags & ITEMFLAG_CURSED) ? ITEM_CURSED : "",
			ARMOR_NAMES[i._id],
			(i._bonusFlags & (ITEMFLAG_BROKEN | ITEMFLAG_CURSED)) ||
				!i._bonusFlags ? "\b " : ""
		);
	}
		
	case CATEGORY_ACCESSORY: {
		// Accessories
		XeenItem &i = _accessories[itemIndex];
		return Common::String::format("\f%02u%s%s%s\f%02u%s%s", displayNum,
			!i._bonusFlags ? "" : spells._maeNames[i._material],
			(i._bonusFlags & ITEMFLAG_BROKEN) ? ITEM_BROKEN : "",
			(i._bonusFlags & ITEMFLAG_CURSED) ? ITEM_CURSED : "",
			ARMOR_NAMES[i._id],
			(i._bonusFlags & (ITEMFLAG_BROKEN | ITEMFLAG_CURSED)) ||
				!i._bonusFlags ? "\b " : ""
		);
	}

	case CATEGORY_MISC: {
		// Misc
		XeenItem &i = _misc[itemIndex];
		return Common::String::format("\f%02u%s%s%s\f%02u%s%s", displayNum,
			!i._bonusFlags ? "" : spells._maeNames[i._material],
			(i._bonusFlags & ITEMFLAG_BROKEN) ? ITEM_BROKEN : "",
			(i._bonusFlags & ITEMFLAG_CURSED) ? ITEM_CURSED : "",
			ARMOR_NAMES[i._id],
			(i._bonusFlags & (ITEMFLAG_BROKEN | ITEMFLAG_CURSED)) ||
				!i._id ? "\b " : ""
		);
	}
	default:
		return "";
	}
}

} // End of namespace Xeen
