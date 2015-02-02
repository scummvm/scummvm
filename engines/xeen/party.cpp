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

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "xeen/party.h"
#include "xeen/dialogs_error.h"
#include "xeen/files.h"
#include "xeen/resources.h"
#include "xeen/saves.h"
#include "xeen/spells.h"
#include "xeen/xeen.h"

namespace Xeen {

AttributePair::AttributePair() {
	_temporary = _permanent = 0;
}

void AttributePair::synchronize(Common::Serializer &s) {
	s.syncAsByte(_permanent);
	s.syncAsByte(_temporary);
}

/*------------------------------------------------------------------------*/

Character::Character() {
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

				if (item._equipped && !(item._bonusFlags & 0xC0) && itemId < 11
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

				if (item._equipped && !(item._bonusFlags & 0xC0)) {
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

				if (item._equipped && !(item._bonusFlags & 0xC0) && itemId < 11 && itemId != 3) {
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
Common::String Character::assembleItemName(int itemIndex, int displayNum, int typeNum) {
	Spells &spells = *Party::_vm->_spells;

	switch (typeNum) {
	case 0: {
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

	case 1: {
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
		
	case 2: {
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

	case 3: {
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

/*------------------------------------------------------------------------*/

void Roster::synchronize(Common::Serializer &s) {
	if (s.isLoading())
		resize(30);

	for (uint i = 0; i < 30; ++i)
		(*this)[i].synchronize(s);
}

/*------------------------------------------------------------------------*/

XeenEngine *Party::_vm;

Party::Party(XeenEngine *vm) {
	_vm = vm;
	_partyCount = 0;
	_realPartyCount = 0;
	Common::fill(&_partyMembers[0], &_partyMembers[8], 0);
	_mazeDirection = DIR_NORTH;
	_mazeId = _priorMazeId = 0;
	_levitateActive = false;
	_automapOn = false;
	_wizardEyeActive = false;
	_clairvoyanceActive = false;
	_walkOnWaterActive = false;
	_blessed = 0;
	_powerShield = 0;
	_holyBonus = 0;
	_heroism = 0;
	_difficulty = ADVENTURER;
	_cloudsEnd = false;
	_darkSideEnd = false;
	_worldEnd = false;
	_ctr24 = 0;
	_day = 0;
	_year = 0;
	_minutes = 0;
	_food = 0;
	_lightCount = 0;
	_torchCount = 0;
	_fireResistence = 0;
	_electricityResistence = 0;
	_coldResistence = 0;
	_poisonResistence = 0;
	_deathCount = 0;
	_winCount = 0;
	_lossCount = 0;
	_gold = 0;
	_gems = 0;
	_bankGold = 0;
	_bankGems = 0;
	_totalTime = 0;
	_rested = false;

	Common::fill(&_gameFlags[0], &_gameFlags[512], false);
	Common::fill(&_worldFlags[0], &_worldFlags[128], false);
	Common::fill(&_quests[0], &_quests[64], false);
	Common::fill(&_questItems[0], &_questItems[85], 0);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		Common::fill(&_characterFlags[i][0], &_characterFlags[i][24], false);

	_combatPartyCount = 0;
	_partyDead = false;
	_newDay = false;
	_isNight = false;
	_stepped = false;
	_damageType = DT_PHYSICAL;
	_falling = false;
	_fallMaze = 0;
	_fallDamage = 0;
}

void Party::synchronize(Common::Serializer &s) {
	byte dummy[30];
	Common::fill(&dummy[0], &dummy[30], 0);

	s.syncAsByte(_partyCount);
	s.syncAsByte(_realPartyCount);
	for (int i = 0; i < 8; ++i)
		s.syncAsByte(_partyMembers[i]);
	s.syncAsByte(_mazeDirection);
	s.syncAsByte(_mazePosition.x);
	s.syncAsByte(_mazePosition.y);
	s.syncAsByte(_mazeId);

	// Game configuration flags not used in this implementation
	s.syncBytes(dummy, 3);

	s.syncAsByte(_priorMazeId);
	s.syncAsByte(_levitateActive);
	s.syncAsByte(_automapOn);
	s.syncAsByte(_wizardEyeActive);
	s.syncAsByte(_clairvoyanceActive);
	s.syncAsByte(_walkOnWaterActive);
	s.syncAsByte(_blessed);
	s.syncAsByte(_powerShield);
	s.syncAsByte(_holyBonus);
	s.syncAsByte(_heroism);
	s.syncAsByte(_difficulty);

	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithWeapons[0][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithArmor[0][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithAccessories[0][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithMisc[0][i].synchronize(s);

	s.syncAsUint16LE(_cloudsEnd);
	s.syncAsUint16LE(_darkSideEnd);
	s.syncAsUint16LE(_worldEnd);
	s.syncAsUint16LE(_ctr24);
	s.syncAsUint16LE(_day);
	s.syncAsUint16LE(_year);
	s.syncAsUint16LE(_minutes);
	s.syncAsUint16LE(_food);
	s.syncAsUint16LE(_lightCount);
	s.syncAsUint16LE(_torchCount);
	s.syncAsUint16LE(_fireResistence);
	s.syncAsUint16LE(_electricityResistence);
	s.syncAsUint16LE(_coldResistence);
	s.syncAsUint16LE(_poisonResistence);
	s.syncAsUint16LE(_deathCount);
	s.syncAsUint16LE(_winCount);
	s.syncAsUint16LE(_lossCount);
	s.syncAsUint32LE(_gold);
	s.syncAsUint32LE(_gems);
	s.syncAsUint32LE(_bankGold);
	s.syncAsUint32LE(_bankGems);
	s.syncAsUint32LE(_totalTime);
	s.syncAsByte(_rested);
	SavesManager::syncBitFlags(s, &_gameFlags[0], &_gameFlags[512]);
	SavesManager::syncBitFlags(s, &_worldFlags[0], &_worldFlags[128]);
	SavesManager::syncBitFlags(s, &_quests[0], &_quests[64]);

	for (int i = 0; i < 85; ++i)
		s.syncAsByte(_questItems[i]);

	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithWeapons[1][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithArmor[1][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithAccessories[1][i].synchronize(s);
	for (int i = 0; i < ITEMS_COUNT; ++i)
		_blacksmithMisc[1][i].synchronize(s);

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		SavesManager::syncBitFlags(s, &_characterFlags[i][0], &_characterFlags[i][24]);
	s.syncBytes(&dummy[0], 30);
}

bool Party::checkSkill(Skill skillId) {
	uint total = 0;
	for (uint i = 0; i < _activeParty.size(); ++i) {
		if (_activeParty[i]._skills[skillId]) {
			++total;

			switch (skillId) {
			case MOUNTAINEER:
			case PATHFINDER:
				// At least two characters need skill for check to return true
				if (total == 2)
					return true;
				break;
			case CRUSADER:
			case SWIMMING:
				// Entire party must have skill for check to return true
				if (total == _activeParty.size())
					return true;
				break;
			default:
				// All other skills only need to have a single player having it
				return true;
			}
		}
	}

	return false;
}

bool Party::isInParty(int charId) {
	for (int i = 0; i < 8; ++i) {
		if (_partyMembers[i] == charId)
			return true;
	}

	return false;
}

void Party::copyPartyToRoster(Roster &r) {
	for (int i = 0; i < _partyCount; ++i) {
		r[_partyMembers[i]] = _activeParty[i];
	}
}

/**
 * Adds time to the party's playtime, taking into account the effect of any
 * stat modifier changes
 */
void Party::changeTime(int numMinutes) {
	bool killed = false;

	if (((_minutes + numMinutes) / 480) != (_minutes / 480)) {
		for (int idx = 0; idx < _partyCount; ++idx) {
			Character &player = _activeParty[idx];

			if (!player._conditions[DEAD] && !player._conditions[STONED] &&
					!player._conditions[ERADICATED]) {
				for (int statNum = 0; statNum < TOTAL_STATS; ++statNum) {
					int statVal = player.getStat((Attribute)statNum);
					if (statVal < 1)
						player._conditions[DEAD] = 1;
				}
			}

			// Handle heart broken condition becoming depression
			if (player._conditions[HEART_BROKEN]) {
				if (++player._conditions[HEART_BROKEN] > 10) {
					player._conditions[HEART_BROKEN] = 0;
					player._conditions[DEPRESSED] = 1;
				}
			}

			// Handle poisoning
			if (!player._conditions[POISONED]) {
				if (_vm->getRandomNumber(1, 10) != 1 || !player.charSavingThrow(DT_ELECTRICAL))
					player._conditions[POISONED] *= 2;
				else
					// Poison wears off
					player._conditions[POISONED] = 0;
			}

			// Handle disease
			if (!player._conditions[DISEASED]) {
				if (_vm->getRandomNumber(9) != 1 || !player.charSavingThrow(DT_COLD))
					player._conditions[DISEASED] *= 2;
				else
					// Disease wears off
					player._conditions[DISEASED] = 0;
			}

			// Handle insane status
			if (player._conditions[INSANE])
				player._conditions[INSANE]++;

			if (player._conditions[DEAD]) {
				if (++player._conditions[DEAD] == 0)
					player._conditions[DEAD] = -1;
			}

			if (player._conditions[STONED]) {
				if (++player._conditions[STONED] == 0)
					player._conditions[STONED] = -1;
			}

			if (player._conditions[ERADICATED]) {
				if (++player._conditions[ERADICATED] == 0)
					player._conditions[ERADICATED] = -1;
			}

			if (player._conditions[IN_LOVE]) {
				if (++player._conditions[IN_LOVE] > 10) {
					player._conditions[IN_LOVE] = 0;
					player._conditions[HEART_BROKEN] = 1;
				}
			}

			player._conditions[WEAK] = player._conditions[DRUNK];
			player._conditions[DRUNK] = 0;

			if (player._conditions[DEPRESSED]) {
				player._conditions[DEPRESSED] = (player._conditions[DEPRESSED] + 1) % 4;
			}
		}
	}

	// Increment the time
	addTime(numMinutes);

	for (int idx = 0; idx < _partyCount; ++idx) {
		Character &player = _activeParty[idx];

		if (player._conditions[CONFUSED] && _vm->getRandomNumber(2) == 1) {
			if (player.charSavingThrow(DT_PHYSICAL)) {
				player._conditions[CONFUSED] = 0;
			} else {
				player._conditions[CONFUSED]--;
			}
		}

		if (player._conditions[PARALYZED] && _vm->getRandomNumber(4) == 1)
			player._conditions[PARALYZED]--;
	}
	
	if (killed)
		_vm->_interface->charIconsPrint(true);

	if (_isNight != (_minutes < (5 * 60) || _minutes >= (21 * 60)))
		_vm->_map->loadSky();
}

void Party::addTime(int numMinutes) {
	int day = _day;
	_minutes += numMinutes;
	
	// If the total minutes has exceeded a day, move to next one
	while (_minutes >= (24 * 60)) {
		_minutes -= 24 * 60;
		if (++_day >= 100) {
			_day -= 100;
			++_year;
		}
	}

	if ((_day % 10) == 1 || numMinutes > (24 * 60)) {
		if (_day != day) {
			warning("TODO: resetBlacksmith? and giveInterest?");
		}
	}

	if (_day != day)
		_newDay = true;

	if (_newDay && _minutes >= 300) {
		if (_vm->_mode != MODE_9 && _vm->_mode != MODE_17) {
			resetTemps();
			if (_rested || _vm->_mode == MODE_5) {
				_rested = false;
			} else {
				for (int idx = 0; idx < _partyCount; ++idx) {
					if (_activeParty[idx]._conditions[WEAK] >= 0)
						_activeParty[idx]._conditions[WEAK]++;
				}

				ErrorScroll::show(_vm, THE_PARTY_NEEDS_REST, WT_NONFREEZED_WAIT);
			}

			_vm->_interface->charIconsPrint(true);
		}

		_newDay = false;
	}
}

void Party::resetTemps() {
	for (int idx = 0; idx < _partyCount; ++idx) {
		Character &player = _activeParty[idx];

		player._magicResistence._temporary = 0;
		player._energyResistence._temporary = 0;
		player._poisonResistence._temporary = 0;
		player._electricityResistence._temporary = 0;
		player._coldResistence._temporary = 0;
		player._fireResistence._temporary = 0;
		player._ACTemp = 0;
		player._level._temporary = 0;
		player._luck._temporary = 0;
		player._accuracy._temporary = 0;
		player._speed._temporary = 0;
		player._endurance._temporary = 0;
		player._personality._temporary = 0;
		player._intellect._temporary = 0;
		player._might._temporary = 0;
	}

	_poisonResistence = 0;
	_coldResistence = 0;
	_electricityResistence = 0;
	_fireResistence = 0;
	_lightCount = 0;
	_levitateActive = false;
	_walkOnWaterActive = false;
	_wizardEyeActive = false;
	_clairvoyanceActive = false;
	_heroism = 0;
	_holyBonus = 0;
	_powerShield = 0;
	_blessed = 0;
}

void Party::handleLight() {
	Map &map = *_vm->_map;

	if (_stepped) {
		map.cellFlagLookup(_mazePosition);
		if (map._currentIsDrain && _lightCount)
			--_lightCount;

		if (checkSkill(CARTOGRAPHER)) {
			map.mazeDataCurrent()._steppedOnTiles[_mazePosition.y & 15][_mazePosition.x & 15] = true;
		}
	}

	_vm->_interface->_intrIndex1 = _lightCount || 
		(map.mazeData()._mazeFlags2 & FLAG_IS_DARK) == 0 ? 4 : 0;
}

int Party::subtract(int mode, uint amount, int whereId, ErrorWaitType wait) {
	switch (mode) {
	case 0:
		// Gold
		if (whereId) {
			if (amount <= _bankGold) {
				_bankGold -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		else {
			if (amount <= _gold) {
				_gold -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		break;

	case 1:
		// Gems
		if (whereId) {
			if (amount <= _bankGems) {
				_bankGems -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		else {
			if (amount <= _gems) {
				_gems -= amount;
			} else {
				notEnough(0, whereId, false, wait);
				return false;
			}
		}
		break;

	case 2:
		// Food
		if (amount > _food) {
			_food -= amount;
		} else {
			notEnough(5, 0, 0, wait);
			return false;
		}
		break;

	default:
		break;
	}

	return true;
}

void Party::notEnough(int consumableId, int whereId, bool mode, ErrorWaitType wait) {
	Common::String msg = Common::String::format(
		mode ? NO_X_IN_THE_Y : NOT_ENOUGH_X_IN_THE_Y,
		CONSUMABLE_NAMES[consumableId], WHERE_NAMES[whereId]);
	ErrorScroll::show(_vm, msg, wait);
}

} // End of namespace Xeen
