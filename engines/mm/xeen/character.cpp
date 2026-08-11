/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mm/xeen/character.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

AttributePair::AttributePair() {
	_temporary = _permanent = 0;
}

void AttributePair::synchronize(Common::Serializer &s) {
	s.syncAsByte(_permanent);
	s.syncAsByte(_temporary);
}

/*------------------------------------------------------------------------*/

int CharacterArray::indexOf(const Character &c) {
	for (uint idx = 0; idx < size(); ++idx) {
		if ((*this)[idx] == c)
			return idx;
	}

	return -1;
}

/*------------------------------------------------------------------------*/

int Character::_itemType;

Character::Character(): _weapons(this), _armor(this), _accessories(this), _misc(this), _items(this) {
	clear();
	_faceSprites = nullptr;
	_rosterId = -1;
}

Character::Character(const Character &src) : _weapons(this), _armor(this), _accessories(this), _misc(this), _items(this) {
	operator=(src);
}

void Character::clear() {
	_name = "";
	_sex = MALE;
	_race = HUMAN;
	_xeenSide = 0;
	_class = CLASS_KNIGHT;
	_ACTemp = 0;
	_birthDay = 0;
	_tempAge = 0;
	Common::fill(&_skills[0], &_skills[18], 0);
	Common::fill(&_awards[0], &_awards[128], 0);
	Common::fill(&_spells[0], &_spells[39], false);
	_lloydMap = 0;
	_hasSpells = false;
	_currentSpell = 0;
	_quickOption = QUICK_ATTACK;
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

	_might._permanent = _might._temporary = 0;
	_intellect._permanent = _intellect._temporary = 0;
	_personality._permanent = _personality._temporary = 0;
	_endurance._permanent = _endurance._temporary = 0;
	_speed._permanent = _speed._temporary = 0;
	_accuracy._permanent = _accuracy._temporary = 0;
	_luck._permanent = _luck._temporary = 0;
	_fireResistance._permanent = _fireResistance._temporary = 0;
	_coldResistance._permanent = _coldResistance._temporary = 0;
	_electricityResistance._permanent = _electricityResistance._temporary = 0;
	_poisonResistance._permanent = _poisonResistance._temporary = 0;
	_energyResistance._permanent = _energyResistance._temporary = 0;
	_magicResistance._permanent = _magicResistance._temporary = 0;
	_weapons.clear();
	_armor.clear();
	_accessories.clear();
	_misc.clear();
}

Character &Character::operator=(const Character &src) {
	clear();

	_faceSprites = src._faceSprites;
	_rosterId = src._rosterId;
	_name = src._name;
	_sex = src._sex;
	_race = src._race;
	_xeenSide = src._xeenSide;
	_class = src._class;
	_might = src._might;
	_intellect = src._intellect;
	_personality = src._personality;
	_endurance = src._endurance;
	_speed = src._speed;
	_accuracy = src._accuracy;
	_luck = src._luck;
	_ACTemp = src._ACTemp;
	_level = src._level;
	_birthDay = src._birthDay;
	_tempAge = src._tempAge;
	Common::copy(&src._skills[0], &src._skills[18], &_skills[0]);
	Common::copy(&src._awards[0], &src._awards[128], &_awards[0]);
	Common::copy(&src._spells[0], &src._spells[SPELLS_PER_CLASS], &_spells[0]);
	_lloydMap = src._lloydMap;
	_lloydPosition = src._lloydPosition;
	_hasSpells = src._hasSpells;
	_currentSpell = src._currentSpell;
	_quickOption = src._quickOption;
	_weapons = src._weapons;
	_armor = src._armor;
	_accessories = src._accessories;
	_misc = src._misc;
	_lloydSide = src._lloydSide;
	_fireResistance = src._fireResistance;
	_coldResistance = src._coldResistance;
	_electricityResistance = src._electricityResistance;
	_poisonResistance = src._poisonResistance;
	_energyResistance = src._energyResistance;
	_magicResistance = src._magicResistance;
	Common::copy(&src._conditions[0], &src._conditions[16], &_conditions[0]);
	_townUnknown = src._townUnknown;
	_savedMazeId = src._savedMazeId;
	_currentHp = src._currentHp;
	_currentSp = src._currentSp;
	_birthYear = src._birthYear;
	_experience = src._experience;
	_currentAdventuringSpell = src._currentAdventuringSpell;
	_currentCombatSpell = src._currentCombatSpell;

	for (ItemCategory category = CATEGORY_WEAPON; category <= CATEGORY_MISC; category = (ItemCategory)((int)category + 1)) {
		const InventoryItems &srcItems = src._items[category];
		InventoryItems &destItems = _items[category];
		destItems = srcItems;
	}

	return *this;
}

void Character::synchronize(Common::Serializer &s) {
	char name[16];
	Common::fill(&name[0], &name[16], '\0');
	Common::strlcpy(name, _name.c_str(), 16);
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

	// Synchronize character awards. The original packed awards 64..127 in the
	// upper nibble of the first 64 bytes. Except for award 9, which was a full
	// byte counter counting the number of times the warzone was awarded
	for (int idx = 0; idx < 64; ++idx) {
		byte b = (idx == WARZONE_AWARD) ? _awards[idx] :
			(_awards[idx] ? 0x1 : 0) | (_awards[idx + 64] ? 0x10 : 0);
		s.syncAsByte(b);
		if (s.isLoading()) {
			_awards[idx] = (idx == WARZONE_AWARD) ? b : b & 0xF;
			_awards[idx + 64] = (idx == WARZONE_AWARD) ? 0 : (b >> 4) & 0xf;
		}
	}

	// Synchronize spell list
	for (int i = 0; i < SPELLS_PER_CLASS; ++i)
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
	_fireResistance.synchronize(s);
	_coldResistance.synchronize(s);
	_electricityResistance.synchronize(s);
	_poisonResistance.synchronize(s);
	_energyResistance.synchronize(s);
	_magicResistance.synchronize(s);

	for (int i = 0; i < 16; ++i)
		s.syncAsByte(_conditions[i]);

	s.syncAsUint16LE(_townUnknown);
	s.syncAsByte(_savedMazeId);
	s.syncAsSint16LE(_currentHp);
	s.syncAsSint16LE(_currentSp);
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

bool Character::isDisabled() const {
	Condition condition = worstCondition();

	return condition == ASLEEP || condition == PARALYZED || condition == UNCONSCIOUS
		|| condition == STONED || condition == ERADICATED;
}

bool Character::isDisabledOrDead() const {
	Condition condition = worstCondition();

	return condition == ASLEEP || (condition >= PARALYZED && condition <= ERADICATED);
}

bool Character::isDead() const {
	Condition condition = worstCondition();

	return condition >= DEAD && condition <= ERADICATED;
}

int Character::getAge(bool ignoreTemp) const {
	int year = MIN(Party::_vm->_party->_year - _birthYear, (uint)254);

	return ignoreTemp ? year : year + _tempAge;
}

int Character::getMaxHP() const {
	int hp = Res.BASE_HP_BY_CLASS[_class];
	hp += statBonus(getStat(ENDURANCE));
	hp += Res.RACE_HP_BONUSES[_race];
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
		result += Res.RACE_SP_BONUSES[_race][attrib - 1];

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
		while (Res.AGE_RANGES[ageIndex] <= age)
			++ageIndex;

		attr._permanent += Res.AGE_RANGES_ADJUST[mode][ageIndex];
	}


	attr._permanent += itemScan((int)attrib);

	if (!baseOnly) {
		attr._permanent += conditionMod(attrib);
		attr._permanent += attr._temporary;
	}

	return MAX(attr._permanent, 0);
}

int Character::statColor(int amount, int threshold) {
	if (amount < 1)
		return 6;
	else if (amount > threshold)
		return 2;
	else if (amount == threshold)
		return 15;
	else if (amount >= (threshold / 4))
		return 9;
	else
		return 32;
}

int Character::statBonus(uint statValue) const {
	int idx;
	for (idx = 0; idx < ARRAYSIZE(Res.STAT_VALUES) - 1; ++idx)
		if (Res.STAT_VALUES[idx] > (int)statValue)
			break;

	return Res.STAT_BONUSES[idx];
}

bool Character::charSavingThrow(DamageType attackType) const {
	int v, vMax;

	if (attackType == DT_PHYSICAL) {
		v = statBonus(getStat(LUCK)) + getCurrentLevel();
		vMax = v + 20;
	} else {
		switch (attackType) {
		case DT_MAGICAL:
			v = _magicResistance._permanent + _magicResistance._temporary + itemScan(16);
			break;
		case DT_FIRE:
			v = _fireResistance._permanent + _fireResistance._temporary + itemScan(11);
			break;
		case DT_ELECTRICAL:
			v = _electricityResistance._permanent + _electricityResistance._temporary + itemScan(12);
			break;
		case DT_COLD:
			v = _coldResistance._permanent + _coldResistance._temporary + itemScan(13);
			break;
		case DT_POISON:
			v = _poisonResistance._permanent + _poisonResistance._temporary + itemScan(14);
			break;
		case DT_ENERGY:
			v = _energyResistance._permanent + _energyResistance._temporary + itemScan(15);
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
	case ASLEEP:
	case PARALYZED:
	case UNCONSCIOUS:
	case DEAD:
	case STONED:
	case ERADICATED: {
		Common::String msg = Common::String::format(Res.IN_NO_CONDITION, _name.c_str());
		ErrorScroll::show(Party::_vm, msg,
			Party::_vm->_mode == 17 ? WT_LOC_WAIT : WT_NONFREEZED_WAIT);
		return true;
	}
	default:
		return false;
	}
}

static int fixAwardId(int awardId) {
	int v = awardId;
	if (awardId == 73)
		v = 126;
	else if (awardId == 81)
		v = 127;

	return v;
}

void Character::setAward(int awardId, bool value) {
	_awards[fixAwardId(awardId)] = value ? 1 : 0;
}

bool Character::hasAward(int awardId) const {
	return _awards[fixAwardId(awardId)] ? true : false;
}

int Character::getAwardCount(int awardId) const {
	return _awards[fixAwardId(awardId)];
}

int Character::getArmorClass(bool baseOnly) const {
	Party &party = *Party::_vm->_party;

	int result = statBonus(getStat(SPEED)) + itemScan(9);
	if (!baseOnly)
		result += party._blessed + _ACTemp;

	return MAX(result, 0);
}

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

	// If the character doesn't have a thievery skill, then don't allow any result
	if (!_skills[THIEVERY])
		result = 0;

	return MAX(result, 0);
}

uint Character::getCurrentLevel() const {
	return MAX(_level._permanent + _level._temporary, 0);
}

int Character::itemScan(int itemId) const {
	int result = 0;

	// Weapons
	for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
		const XeenItem &item = _weapons[idx];

		if (item._frame && !item.isBad() && itemId < 11
				&& itemId != ENDURANCE && item._material >= 59 && item._material <= 130) {
			int mIndex = (int)item.getAttributeCategory();
			// There is no Endurance bonus. This enchantment category skipped.
			if (mIndex > PERSONALITY)
				++mIndex;

			if (mIndex == itemId)
				result += Res.ATTRIBUTE_BONUSES[item._material - 59];
		}
	}

	// Armor
	for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
		const XeenItem &item = _armor[idx];

		if (item._frame && !item.isBad()) {
			if (itemId < 11 && itemId != 3 && item._material >= 59 && item._material <= 130) {
				int mIndex = (int)item.getAttributeCategory();
				if (mIndex > PERSONALITY)
					++mIndex;

				if (mIndex == itemId)
					result += Res.ATTRIBUTE_BONUSES[item._material - 59];
			}

			if (itemId > 10 && item._material < 37) {
				int mIndex = item.getElementalCategory() + 11;

				if (mIndex == itemId) {
					result += Res.ELEMENTAL_RESISTANCES[item._material];
				}
			}

			if (itemId == 9) {
				result += Res.ARMOR_STRENGTHS[item._id];

				if (item._material >= 37 && item._material <= 58)
					result += Res.METAL_LAC[item._material - 37];
			}
		}
	}

	// Accessories
	for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
		const XeenItem &item = _accessories[idx];

		if (item._frame && !item.isBad()) {
			if (itemId < 11 && itemId != 3 && item._material >= 59 && item._material <= 130) {
				int mIndex = (int)item.getAttributeCategory();
				if (mIndex > PERSONALITY)
					++mIndex;

				if (mIndex == itemId) {
					result += Res.ATTRIBUTE_BONUSES[item._material - 59];
				}
			}

			if (itemId > 10 && item._material < 37) {
				int mIndex = item.getElementalCategory() + 11;

				if (mIndex == itemId)
					result += Res.ELEMENTAL_RESISTANCES[item._material];
			}
		}
	}

	return result;
}

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
		// Set party poison resistance
		party._poisonResistance = value;
		break;
	case 18:
		// Set condition
		if (value == 16) {
			// Clear all the conditions
			clearConditions();
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
		_fireResistance._permanent = value;
		break;
	case 53:
		_electricityResistance._permanent = value;
		break;
	case 54:
		_coldResistance._permanent = value;
		break;
	case 55:
		_poisonResistance._permanent = value;
		break;
	case 56:
		_energyResistance._permanent = value;
		break;
	case 57:
		_magicResistance._permanent = value;
		break;
	case 58:
		_fireResistance._temporary = value;
		break;
	case 59:
		_electricityResistance._temporary = value;
		break;
	case 60:
		_coldResistance._temporary = value;
		break;
	case 61:
		_poisonResistance._temporary = value;
		break;
	case 62:
		_energyResistance._temporary = value;
		break;
	case 63:
		_magicResistance._temporary = value;
		break;
	case 64:
		_level._permanent = value;
		break;
	case 65:
		// Set party food
		party._food = value;
		break;
	case 69:
		party._levitateCount = value;
		break;
	case 70:
		party._lightCount = value;
		break;
	case 71:
		party._fireResistance = value;
		break;
	case 72:
		party._electricityResistance = value;
		break;
	case 73:
		party._coldResistance = value;
		break;
	case 74:
		party._walkOnWaterActive = value != 0;
		party._poisonResistance = value;
		party._wizardEyeActive = value != 0;
		party._coldResistance = value;
		party._electricityResistance = value;
		party._fireResistance = value;
		party._lightCount = value;
		party._levitateCount = value;
		break;
	case 76:
		// Set day of the year (0-99)
		party._day = value;
		break;
	case 79:
		party._wizardEyeActive = true;
		break;
	case 83:
		scripts._nEdamageType = (DamageType)value;
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
	FileManager &files = *g_vm->_files;
	Party &party = *g_vm->_party;

	if (g_vm->getGameID() == GType_Swords) {
		switch (party._mazeId) {
		case 49:
			return true;
		case 53:
			return hasAward(83);
		case 63:
			return hasAward(85);
		case 92:
			return hasAward(84);
		default:
			return hasAward(87);
		}
	} else if (files._ccNum) {
		switch (party._mazeId) {
		case 29:
			return hasAward(CASTLEVIEW_GUILD_MEMBER);
		case 31:
			return hasAward(SANDCASTER_GUILD_MEMBER);
		case 33:
			return hasAward(LAKESIDE_GUILD_MEMBER);
		case 35:
			return hasAward(NECROPOLIS_GUILD_MEMBER);
		default:
			return hasAward(OLYMPUS_GUILD_MEMBER);
		}
	} else if (party._mazeId == 49) {
		return hasAward(SHANGRILA_GUILD_MEMBER);
	} else {
		return hasAward(party._mazeId - 28);
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
		assert(_level._permanent > 0);
		shift = _level._permanent - 1;
	}

	return (base * 1024000) + (Res.CLASS_EXP_LEVELS[_class] << shift);
}

uint Character::getCurrentExperience() const {
	int lev = _level._permanent - 1;
	int shift, base;

	if (lev == 0)
		return _experience;

	if (lev >= 12) {
		base = lev - 12;
		shift = 10;
	} else {
		base = 0;
		shift = lev - 1;
	}

	return (base * 1024000) + (Res.CLASS_EXP_LEVELS[_class] << shift) +
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
	for (int idx = 0; idx < AWARDS_TOTAL; ++idx) {
		if (hasAward(idx))
			++total;
	}

	return total;
}

ItemCategory Character::makeItem(int itemLevel, int itemIndex, int reason) {
	assert(1 <= itemLevel && itemLevel <= 7);
	assert(0 <= itemIndex && itemIndex < INV_ITEMS_TOTAL);

	XeenEngine *vm = Party::_vm;
	int itemOffset = vm->getGameID() == GType_Swords ? 6 : 0;

	if (!itemLevel)
		return CATEGORY_WEAPON;

	int itemId = 0;
	int categoryRN = vm->getRandomNumber(100);
	// generate no wands, no bows and less two-handed weapons on levels 6 and 7
	int subcategoryRN = vm->getRandomNumber(itemLevel < 6 ? 100 : 80);
	ItemCategory category;
	int material = 0, attrBonus = 0, miscCharges = 0, miscId = 0, enchantmentType = 0, element = 0;

	// Randomly pick a category and item Id
	if (reason == MAKE_ITEM_SPECIAL_EVENT) {
		if (_itemType < (35 + itemOffset)) {
			category = CATEGORY_WEAPON;
			itemId = _itemType;
		} else if (_itemType < (49 + itemOffset)) {
			category = CATEGORY_ARMOR;
			itemId = _itemType - (35 + itemOffset);
		} else if (_itemType < (60 + itemOffset)) {
			category = CATEGORY_ACCESSORY;
			itemId = _itemType - (49 + itemOffset);
		} else {
			category = CATEGORY_MISC;
			itemId = _itemType - (60 + itemOffset);
		}
	} else {
		switch (reason) {
		case MAKE_ITEM_ENCHANT_WEAPON:
			categoryRN = 35;
			break;
		case MAKE_ITEM_ENCHANT_ARMOR:
			categoryRN = 60;
			break;
		case MAKE_ITEM_ENCHANT_ACCESSORY: // never used
			categoryRN = 100;
			break;
		default:
			break;
		}

		if (itemLevel == 1) {
			if (categoryRN <= 40) {
				category = CATEGORY_WEAPON;
				if (subcategoryRN <= 30) {
					itemId = vm->getRandomNumber(1, 6); // swords
				} else if (subcategoryRN <= 60) {
					itemId = vm->getRandomNumber(7, 17); // one-handed
				} else if (subcategoryRN <= 85) {
					itemId = vm->getRandomNumber(18, 29); // two-handed
				} else {
					itemId = vm->getRandomNumber(30, 33); // ranged
				}
			} else if (categoryRN <= 85) {
				category = CATEGORY_ARMOR;
				itemId = vm->getRandomNumber(1, 7); // armor
			} else {
				category = CATEGORY_MISC;
				itemId = vm->getRandomNumber(1, 9); // wand
			}
		} else if (categoryRN <= 35) {
			category = CATEGORY_WEAPON;
			if (subcategoryRN <= 30) {
				itemId = vm->getRandomNumber(1, 6); // swords
			} else if (subcategoryRN <= 60) {
				itemId = vm->getRandomNumber(7, 17); // one-handed
			} else if (subcategoryRN <= 85) {
				itemId = vm->getRandomNumber(18, 29); // two-handed
			} else {
				itemId = vm->getRandomNumber(30, 33); // ranged
			}
		} else if (categoryRN <= 60) {
			category = CATEGORY_ARMOR;
			itemId = (subcategoryRN > 70) ? 8 : vm->getRandomNumber(1, 7); // shield or armor

		} else if (subcategoryRN <= 10) {
			category = CATEGORY_ARMOR;
			itemId = 9; // helm
		} else if (subcategoryRN <= 20) {
			category = CATEGORY_ARMOR;
			itemId = 13; // gauntlets
		} else if (subcategoryRN <= 35) {
			category = CATEGORY_ACCESSORY;
			itemId = 1; // ring
		} else if (subcategoryRN <= 45) {
			category = CATEGORY_ARMOR;
			itemId = 10; // boots
		} else if (subcategoryRN <= 55) {
			category = CATEGORY_ARMOR;
			itemId = vm->getRandomNumber(11, 12); // cloak, cape
		} else if (subcategoryRN <= 65) {
			category = CATEGORY_ACCESSORY;
			itemId = 2; // belt
		} else if (subcategoryRN <= 75) {
			category = CATEGORY_ACCESSORY;
			itemId = vm->getRandomNumber(3, 7); // medal-like
		} else if (subcategoryRN <= 80) {
			category = CATEGORY_ACCESSORY;
			itemId = vm->getRandomNumber(8, 10); // amulet-like
		} else {
			category = CATEGORY_MISC;
			itemId = vm->getRandomNumber(1, 9); // wand-like
		}
	}

	XeenItem &newItem = _items[category][itemIndex];
	newItem.clear();
	newItem._id = itemId;

	int enchanntmentRN = vm->getRandomNumber(1, 100);

	switch (category) {
	case CATEGORY_WEAPON:
	case CATEGORY_ARMOR:
		if (itemLevel != 1) {
			if (enchanntmentRN <= 70) {
				enchantmentType = ENCHANTMENT_TYPE_MATERIAL;
			} else if (enchanntmentRN <= 98) {
				enchantmentType = ENCHANTMENT_TYPE_ELEMENT;
			} else {
				enchantmentType = ENCHANTMENT_TYPE_ATTR_BONUS;
			}
		}
		break;

	case CATEGORY_ACCESSORY:
		if (enchanntmentRN <= 20) {
			enchantmentType = ENCHANTMENT_TYPE_MATERIAL;
		} else if (enchanntmentRN <= 60) {
			enchantmentType = ENCHANTMENT_TYPE_ELEMENT;
		} else {
			enchantmentType = ENCHANTMENT_TYPE_ATTR_BONUS;
		}
		break;

	case CATEGORY_MISC:
		newItem._material = itemId;
		enchantmentType = ENCHANTMENT_TYPE_USABLE;
		break;

	default:
		break;
	}

	if (itemLevel != 1 || category == CATEGORY_MISC) {
		int rn, enchantmentSubtype, r1, r2;
		switch (enchantmentType) {
		case ENCHANTMENT_TYPE_ELEMENT:
			rn = vm->getRandomNumber(1, 100);
			if (rn <= 25) {
				enchantmentSubtype = ELEM_FIRE;
			} else if (rn <= 45) {
				enchantmentSubtype = ELEM_ELECTRICITY;
			} else if (rn <= 60) {
				enchantmentSubtype = ELEM_COLD;
			} else if (rn <= 75) {
				enchantmentSubtype = ELEM_ACID_POISON;
			} else if (rn <= 95) {
				enchantmentSubtype = ELEM_ENERGY;
			} else {
				enchantmentSubtype = ELEM_MAGIC;
			}

			r1 = Res.MAKE_ITEM_ARR2[enchantmentSubtype][itemLevel - 1][0];
			r2 = Res.MAKE_ITEM_ARR2[enchantmentSubtype][itemLevel - 1][1];
			element = Res.MAKE_ITEM_ARR1[enchantmentSubtype] + vm->getRandomNumber(r1, r2);
			break;
		case ENCHANTMENT_TYPE_ATTR_BONUS:
			rn = vm->getRandomNumber(1, 100);
			if (rn <= 15) {
				enchantmentSubtype = ATTR_MIGHT;
			} else if (rn <= 25) {
				enchantmentSubtype = ATTR_INTELLECT;
			} else if (rn <= 35) {
				enchantmentSubtype = ATTR_PERSONALITY;
			} else if (rn <= 50) {
				enchantmentSubtype = ATTR_SPEED;
			} else if (rn <= 65) {
				enchantmentSubtype = ATTR_ACCURACY;
			} else if (rn <= 80) {
				enchantmentSubtype = ATTR_LUCK;
			} else if (rn <= 85) {
				enchantmentSubtype = ATTR_HIT_POINTS;
			} else if (rn <= 90) {
				enchantmentSubtype = ATTR_SPELL_POINTS;
			} else if (rn <= 95) {
				enchantmentSubtype = ATTR_ARMOR_CLASS;
			} else {
				enchantmentSubtype = ATTR_THIEVERY;
			}

			{
				// similar to Res.ATTRIBUTE_CATEGORIES
				const int SUBTYPE_OFFSETS[] = {0, 10, 18, 26, 34, 40, 46, 51, 57, 62, 72};

				r1 = Res.MAKE_ITEM_ARR3[enchantmentSubtype][itemLevel - 1][0];
				r2 = Res.MAKE_ITEM_ARR3[enchantmentSubtype][itemLevel - 1][1];
				attrBonus = SUBTYPE_OFFSETS[enchantmentSubtype] + vm->getRandomNumber(r1, r2);
			}
			break;

		case ENCHANTMENT_TYPE_MATERIAL:

			enchantmentSubtype = itemLevel == 7 || vm->getRandomNumber(1, 100) > 70 ? 1 : 0;
			r1 = Res.MAKE_ITEM_ARR4[enchantmentSubtype][itemLevel - 1][0];
			r2 = Res.MAKE_ITEM_ARR4[enchantmentSubtype][itemLevel - 1][1];
			material = vm->getRandomNumber(r1, r2);

			// Change from common/rare to rare/precious
			if (enchantmentSubtype)
				material += 9;
			break;

		case ENCHANTMENT_TYPE_USABLE:
			r1 = Res.MAKE_ITEM_ARR5[itemLevel][0];
			r2 = Res.MAKE_ITEM_ARR5[itemLevel][1];

			miscId = vm->getRandomNumber(r1, r2);
			miscCharges = vm->getRandomNumber(1, 8);
			break;

		default:
			break;
		}
	}

	switch (category) {
	case CATEGORY_WEAPON:
		if (itemLevel != 1) {
			newItem._material = (attrBonus ? attrBonus + 58 : 0) +
								(material ? material + 36 : 0) +
								element;


			if (vm->getRandomNumber(20) == 10) {
				// of undead/golem/dragon/etc-slaying
				newItem._state._counter = vm->getRandomNumber(EFFECTIVE_DRAGON, EFFECTIVE_ANIMAL);
			}
		}
		break;

	case CATEGORY_ARMOR:
	case CATEGORY_ACCESSORY:
		if (itemLevel != 1) {
			newItem._material = (attrBonus ? attrBonus + 58 : 0) +
								(material ? material + 36 : 0) +
								element;
		}
		break;

	case CATEGORY_MISC:
		newItem._id = miscId;
		newItem._state._counter = miscCharges;
		break;

	default:
		break;
	}

	return category;
}

void Character::addHitPoints(int amount) {
	Interface &intf = *Party::_vm->_interface;
	Common::fill(&intf._charFX[0], &intf._charFX[MAX_ACTIVE_PARTY], 0);

	if (!isDead()) {
		int maxHp = getMaxHP();
		if (_currentHp <= maxHp) {
			_currentHp = MIN(_currentHp + amount, maxHp);
			intf.spellFX(this);
		}

		if (_currentHp > 0)
			_conditions[UNCONSCIOUS] = 0;

		intf.drawParty(true);
	}

	assert(_currentHp < 65000);
	Common::fill(&intf._charFX[0], &intf._charFX[MAX_ACTIVE_PARTY], 0);
}

void Character::subtractHitPoints(int amount) {
	Debugger &debugger = *g_vm->_debugger;
	Sound &sound = *g_vm->_sound;

	// If invincibility is turned on in the debugger, ignore all damage
	if (debugger._invincible)
		return;

	// Subtract the given HP amount
	_currentHp -= amount;
	bool breakFlag = _currentHp <= (g_vm->_extOptions._durableArmor ? -80 : -10);
	assert(_currentHp < 65000);

	if (_currentHp < 1) {
		int v = getMaxHP() + _currentHp;
		if (v >= 1) {
			_conditions[UNCONSCIOUS] = 1;
			sound.playFX(38);
		} else {
			_conditions[DEAD] = 1;
			breakFlag = true;
			if (_currentHp > 0)
				_currentHp = 0;
		}

		if (breakFlag) {
			// Break any equipped armor the character has
			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				XeenItem &item = _armor[idx];
				if (item._id && item._frame)
					item._state._broken = true;
			}
		}
	}
}

bool Character::hasSlayerSword() const {
	for (uint idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
		if (_weapons[idx]._id == XEEN_SLAYER_SWORD)
			// Character has Xeen Slayer sword
			return true;
	}

	return false;
}

bool Character::hasMissileWeapon() const {
	for (uint idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
		if (_weapons[idx]._frame == 4) {
			return !isDisabledOrDead();
		}
	}

	return false;
}

SpellsCategory Character::getSpellsCategory() const {
	switch (_class) {
	case CLASS_PALADIN:
	case CLASS_CLERIC:
		return SPELLCAT_CLERICAL;

	case CLASS_ARCHER:
	case CLASS_SORCERER:
		return SPELLCAT_WIZARDRY;

	case CLASS_DRUID:
	case CLASS_RANGER:
		return SPELLCAT_DRUIDIC;

	default:
		return SPELLCAT_INVALID;
	}
}

void Character::clearConditions() {
	Common::fill(&_conditions[CURSED], &_conditions[NO_CONDITION], false);
}

} // End of namespace Xeen
} // End of namespace MM
