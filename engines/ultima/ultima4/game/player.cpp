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

#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/armor.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/map/tileset.h"

namespace Ultima {
namespace Ultima4 {

bool isPartyMember(Object *punknown) {
	PartyMember *pm;
	if ((pm = dynamic_cast<PartyMember *>(punknown)) != nullptr)
		return true;
	else
		return false;
}

PartyMember::PartyMember(Party *p, SaveGamePlayerRecord *pr) :
	Creature(tileForClass(pr->_class)),
	_player(pr),
	_party(p) {
	/* FIXME: we need to rename movement behaviors */
	setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
	this->_ranged = g_weapons->get(pr->_weapon)->getRange() ? 1 : 0;
	setStatus(pr->_status);
}

PartyMember::~PartyMember() {
}

void PartyMember::notifyOfChange() {
	if (_party) {
		_party->notifyOfChange(this);
	}
}

Common::String PartyMember::translate(Std::vector<Common::String> &parts) {
	if (parts.size() == 0)
		return "";
	else if (parts.size() == 1) {
		if (parts[0] == "hp")
			return xu4_to_string(getHp());
		else if (parts[0] == "max_hp")
			return xu4_to_string(getMaxHp());
		else if (parts[0] == "mp")
			return xu4_to_string(getMp());
		else if (parts[0] == "max_mp")
			return xu4_to_string(getMaxMp());
		else if (parts[0] == "str")
			return xu4_to_string(getStr());
		else if (parts[0] == "dex")
			return xu4_to_string(getDex());
		else if (parts[0] == "int")
			return xu4_to_string(getInt());
		else if (parts[0] == "exp")
			return xu4_to_string(getExp());
		else if (parts[0] == "name")
			return getName();
		else if (parts[0] == "weapon")
			return getWeapon()->getName();
		else if (parts[0] == "armor")
			return getArmor()->getName();
		else if (parts[0] == "sex") {
			Common::String var((char)getSex());
			return var;
		} else if (parts[0] == "class")
			return getClassName(getClass());
		else if (parts[0] == "level")
			return xu4_to_string(getRealLevel());
	} else if (parts.size() == 2) {
		if (parts[0] == "needs") {
			if (parts[1] == "cure") {
				if (getStatus() == STAT_POISONED)
					return "true";
				else
					return "false";
			} else if (parts[1] == "heal" || parts[1] == "fullheal") {
				if (getHp() < getMaxHp())
					return "true";
				else
					return "false";
			} else if (parts[1] == "resurrect") {
				if (getStatus() == STAT_DEAD)
					return "true";
				else
					return "false";
			}
		}
	}

	return "";
}

int PartyMember::getHp() const {
	return _player->_hp;
}

int PartyMember::getMaxMp() const {
	int max_mp = -1;

	switch (_player->_class) {
	case CLASS_MAGE:            /*  mage: 200% of int */
		max_mp = _player->_intel * 2;
		break;

	case CLASS_DRUID:           /* druid: 150% of int */
		max_mp = _player->_intel * 3 / 2;
		break;

	case CLASS_BARD:            /* bard, paladin, ranger: 100% of int */
	case CLASS_PALADIN:
	case CLASS_RANGER:
		max_mp = _player->_intel;
		break;

	case CLASS_TINKER:          /* tinker: 50% of int */
		max_mp = _player->_intel / 2;
		break;

	case CLASS_FIGHTER:         /* fighter, shepherd: no mp at all */
	case CLASS_SHEPHERD:
		max_mp = 0;
		break;

	default:
		error("invalid player class: %d", _player->_class);
	}

	/* mp always maxes out at 99 */
	if (max_mp > 99)
		max_mp = 99;

	return max_mp;
}

const Weapon *PartyMember::getWeapon() const {
	return g_weapons->get(_player->_weapon);
}

const Armor *PartyMember::getArmor() const {
	return g_armors->get(_player->_armor);
}

Common::String PartyMember::getName() const {
	return _player->_name;
}

SexType PartyMember::getSex() const {
	return _player->_sex;
}

ClassType PartyMember::getClass() const {
	return _player->_class;
}

CreatureStatus PartyMember::getState() const {
	if (getHp() <= 0)
		return MSTAT_DEAD;
	else if (getHp() < 24)
		return MSTAT_FLEEING;
	else
		return MSTAT_BARELYWOUNDED;
}

int PartyMember::getRealLevel() const {
	return _player->_hpMax / 100;
}

int PartyMember::getMaxLevel() const {
	int level = 1;
	int next = 100;

	while (_player->_xp >= next && level < 8) {
		level++;
		next <<= 1;
	}

	return level;
}

void PartyMember::addStatus(StatusType s) {
	Creature::addStatus(s);
	_player->_status = _status.back();
	notifyOfChange();
}

void PartyMember::adjustMp(int pts) {
	AdjustValueMax(_player->_mp, pts, getMaxMp());
	notifyOfChange();
}

void PartyMember::advanceLevel() {
	if (getRealLevel() == getMaxLevel())
		return;
	setStatus(STAT_GOOD);
	_player->_hpMax = getMaxLevel() * 100;
	_player->_hp = _player->_hpMax;

	/* improve stats by 1-8 each */
	_player->_str   += xu4_random(8) + 1;
	_player->_dex   += xu4_random(8) + 1;
	_player->_intel += xu4_random(8) + 1;

	if (_player->_str > 50) _player->_str = 50;
	if (_player->_dex > 50) _player->_dex = 50;
	if (_player->_intel > 50) _player->_intel = 50;

	if (_party) {
		_party->setChanged();
		PartyEvent event(PartyEvent::ADVANCED_LEVEL, this);
		event._player = this;
		_party->notifyObservers(event);
	}
}

void PartyMember::applyEffect(TileEffect effect) {
	if (getStatus() == STAT_DEAD)
		return;

	switch (effect) {
	case EFFECT_NONE:
		break;
	case EFFECT_LAVA:
	case EFFECT_FIRE:
		applyDamage(16 + (xu4_random(32)));

		/*else if (player == ALL_PLAYERS && xu4_random(2) == 0)
		    playerApplyDamage(&(c->saveGame->_players[i]), 10 + (xu4_random(25)));*/
		break;
	case EFFECT_SLEEP:
		putToSleep();
		break;
	case EFFECT_POISONFIELD:
	case EFFECT_POISON:
		if (getStatus() != STAT_POISONED) {
			soundPlay(SOUND_POISON_EFFECT, false);
			addStatus(STAT_POISONED);
		}
		break;
	case EFFECT_ELECTRICITY:
		break;
	default:
		error("invalid effect: %d", effect);
	}

	if (effect != EFFECT_NONE)
		notifyOfChange();
}

void PartyMember::awardXp(int xp) {
	AdjustValueMax(_player->_xp, xp, 9999);
	notifyOfChange();
}

bool PartyMember::heal(HealType type) {
	switch (type) {
	case HT_NONE:
		return true;

	case HT_CURE:
		if (getStatus() != STAT_POISONED)
			return false;
		removeStatus(STAT_POISONED);
		break;

	case HT_FULLHEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;
		_player->_hp = _player->_hpMax;
		break;

	case HT_RESURRECT:
		if (getStatus() != STAT_DEAD)
			return false;
		setStatus(STAT_GOOD);
		break;

	case HT_HEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;

		_player->_hp += 75 + (xu4_random(0x100) % 0x19);
		break;

	case HT_CAMPHEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;
		_player->_hp += 99 + (xu4_random(0x100) & 0x77);
		break;

	case HT_INNHEAL:
		if (getStatus() == STAT_DEAD ||
		        _player->_hp == _player->_hpMax)
			return false;
		_player->_hp += 100 + (xu4_random(50) * 2);
		break;

	default:
		return false;
	}

	if (_player->_hp > _player->_hpMax)
		_player->_hp = _player->_hpMax;

	notifyOfChange();

	return true;
}

void PartyMember::removeStatus(StatusType s) {
	Creature::removeStatus(s);
	_player->_status = _status.back();
	notifyOfChange();
}

void PartyMember::setHp(int hp) {
	_player->_hp = hp;
	notifyOfChange();
}

void PartyMember::setMp(int mp) {
	_player->_mp = mp;
	notifyOfChange();
}

EquipError PartyMember::setArmor(const Armor *a) {
	ArmorType type = a->getType();

	if (type != ARMR_NONE && _party->_saveGame->_armor[type] < 1)
		return EQUIP_NONE_LEFT;
	if (!a->canWear(getClass()))
		return EQUIP_CLASS_RESTRICTED;

	ArmorType oldArmorType = getArmor()->getType();
	if (oldArmorType != ARMR_NONE)
		_party->_saveGame->_armor[oldArmorType]++;
	if (type != ARMR_NONE)
		_party->_saveGame->_armor[type]--;

	_player->_armor = type;
	notifyOfChange();

	return EQUIP_SUCCEEDED;
}

EquipError PartyMember::setWeapon(const Weapon *w) {
	WeaponType type = w->getType();

	if (type != WEAP_HANDS && _party->_saveGame->_weapons[type] < 1)
		return EQUIP_NONE_LEFT;
	if (!w->canReady(getClass()))
		return EQUIP_CLASS_RESTRICTED;

	WeaponType old = getWeapon()->getType();
	if (old != WEAP_HANDS)
		_party->_saveGame->_weapons[old]++;
	if (type != WEAP_HANDS)
		_party->_saveGame->_weapons[type]--;

	_player->_weapon = type;
	notifyOfChange();

	return EQUIP_SUCCEEDED;
}

bool PartyMember::applyDamage(int damage, bool) {
	int newHp = _player->_hp;

	if (getStatus() == STAT_DEAD)
		return false;

	newHp -= damage;

	if (newHp < 0) {
		setStatus(STAT_DEAD);
		newHp = 0;
	}

	_player->_hp = newHp;
	notifyOfChange();

	if (isCombatMap(g_context->_location->_map) && getStatus() == STAT_DEAD) {
		Coords p = getCoords();
		Map *map = getMap();

		assert(_party);
		map->_annotations->add(p, g_tileSets->findTileByName("corpse")->getId())->setTTL(_party->size() * 2);

		{
			_party->setChanged();
			PartyEvent event(PartyEvent::PLAYER_KILLED, this);
			event._player = this;
			_party->notifyObservers(event);
		}

		/* remove yourself from the map */
		remove();
		return false;
	}

	return true;
}

int PartyMember::getAttackBonus() const {
	if (g_weapons->get(_player->_weapon)->alwaysHits() || _player->_dex >= 40)
		return 255;
	return _player->_dex;
}

int PartyMember::getDefense() const {
	return g_armors->get(_player->_armor)->getDefense();
}

bool PartyMember::dealDamage(Creature *m, int damage) {
	/* we have to record these now, because if we
	   kill the target, it gets destroyed */
	int m_xp = m->getXp();

	if (!Creature::dealDamage(m, damage)) {
		/* half the time you kill an evil creature you get a karma boost */
		awardXp(m_xp);
		return false;
	}
	return true;
}

int PartyMember::getDamage() {
	int maxDamage;

	maxDamage = g_weapons->get(_player->_weapon)->getDamage();
	maxDamage += _player->_str;
	if (maxDamage > 255)
		maxDamage = 255;

	return xu4_random(maxDamage);
}

const Common::String &PartyMember::getHitTile() const {
	return getWeapon()->getHitTile();
}

const Common::String &PartyMember::getMissTile() const {
	return getWeapon()->getMissTile();
}

bool PartyMember::isDead() {
	return getStatus() == STAT_DEAD;
}

bool PartyMember::isDisabled() {
	return (getStatus() == STAT_GOOD ||
	        getStatus() == STAT_POISONED) ? false : true;
}

int PartyMember::loseWeapon() {
	int weapon = _player->_weapon;

	notifyOfChange();

	if (_party->_saveGame->_weapons[weapon] > 0)
		return (--_party->_saveGame->_weapons[weapon]) + 1;
	else {
		_player->_weapon = WEAP_HANDS;
		return 0;
	}
}

void PartyMember::putToSleep() {
	if (getStatus() != STAT_DEAD) {
		soundPlay(SOUND_SLEEP, false);
		addStatus(STAT_SLEEPING);
		setTile(g_tileSets->findTileByName("corpse")->getId());
	}
}

void PartyMember::wakeUp() {
	removeStatus(STAT_SLEEPING);
	setTile(tileForClass(getClass()));
}

MapTile PartyMember::tileForClass(int klass) {
	const char *name = nullptr;

	switch (klass) {
	case CLASS_MAGE:
		name = "mage";
		break;
	case CLASS_BARD:
		name = "bard";
		break;
	case CLASS_FIGHTER:
		name = "fighter";
		break;
	case CLASS_DRUID:
		name = "druid";
		break;
	case CLASS_TINKER:
		name = "tinker";
		break;
	case CLASS_PALADIN:
		name = "paladin";
		break;
	case CLASS_RANGER:
		name = "ranger";
		break;
	case CLASS_SHEPHERD:
		name = "shepherd";
		break;
	default:
		error("invalid class %d in tileForClass", klass);
	}

	const Tile *tile = g_tileSets->get("base")->getByName(name);
	assertMsg(tile, "no tile found for class %d", klass);
	return tile->getId();
}

/*-------------------------------------------------------------------*/

Party::Party(SaveGame *s) : _saveGame(s), _transport(0), _torchDuration(0), _activePlayer(-1) {
	MapId map = _saveGame->_positions.back()._map;
	if (map >= MAP_DECEIT && map <= MAP_ABYSS)
		_torchDuration = _saveGame->_torchDuration;
	for (int i = 0; i < _saveGame->_members; i++) {
		// add the members to the party
		_members.push_back(new PartyMember(this, &_saveGame->_players[i]));
	}

	// set the party's transport (transport value stored in savegame
	// hardcoded to index into base tilemap)
	setTransport(g_tileMaps->get("base")->translate(_saveGame->_transport));
}

Party::~Party() {
	for (uint idx = 0; idx < _members.size(); ++idx)
		delete _members[idx];
}

void Party::notifyOfChange(PartyMember *pm, PartyEvent::Type eventType) {
	setChanged();
	PartyEvent event(eventType, pm);
	notifyObservers(event);
}

Common::String Party::translate(Std::vector<Common::String> &parts) {
	if (parts.size() == 0)
		return "";
	else if (parts.size() == 1) {
		// Translate some different items for the script
		if (parts[0] == "transport") {
			if (g_context->_transportContext & TRANSPORT_FOOT)
				return "foot";
			if (g_context->_transportContext & TRANSPORT_HORSE)
				return "horse";
			if (g_context->_transportContext & TRANSPORT_SHIP)
				return "ship";
			if (g_context->_transportContext & TRANSPORT_BALLOON)
				return "balloon";
		} else if (parts[0] == "gold")
			return xu4_to_string(_saveGame->_gold);
		else if (parts[0] == "food")
			return xu4_to_string(_saveGame->_food);
		else if (parts[0] == "members")
			return xu4_to_string(size());
		else if (parts[0] == "keys")
			return xu4_to_string(_saveGame->_keys);
		else if (parts[0] == "torches")
			return xu4_to_string(_saveGame->_torches);
		else if (parts[0] == "gems")
			return xu4_to_string(_saveGame->_gems);
		else if (parts[0] == "sextants")
			return xu4_to_string(_saveGame->_sextants);
		else if (parts[0] == "food")
			return xu4_to_string((_saveGame->_food / 100));
		else if (parts[0] == "gold")
			return xu4_to_string(_saveGame->_gold);
		else if (parts[0] == "party_members")
			return xu4_to_string(_saveGame->_members);
		else if (parts[0] == "moves")
			return xu4_to_string(_saveGame->_moves);
	} else if (parts.size() >= 2) {
		if (parts[0].findFirstOf("member") == 0) {
			// Make a new parts list, but remove the first item
			Std::vector<Common::String> new_parts = parts;
			new_parts.erase(new_parts.begin());

			// Find the member we'll be working with
			Common::String str = parts[0];
			size_t pos = str.findFirstOf("1234567890");
			if (pos != Common::String::npos) {
				str = str.substr(pos);
				int p_member = (int)strtol(str.c_str(), nullptr, 10);

				// Make the party member translate its own stuff
				if (p_member > 0)
					return member(p_member - 1)->translate(new_parts);
			}
		}

		else if (parts.size() == 2) {
			if (parts[0] == "weapon") {
				const Weapon *w = g_weapons->get(parts[1]);
				if (w)
					return xu4_to_string(_saveGame->_weapons[w->getType()]);
			} else if (parts[0] == "armor") {
				const Armor *a = g_armors->get(parts[1]);
				if (a)
					return xu4_to_string(_saveGame->_armor[a->getType()]);
			}
		}
	}
	return "";
}

void Party::adjustFood(int food) {
	// Check for cheat that disables party hunger
	if (food < 0 && g_debugger->_disableHunger)
		return;

	int oldFood = _saveGame->_food;
	AdjustValue(_saveGame->_food, food, 999900, 0);
	if ((_saveGame->_food / 100) != (oldFood / 100)) {
		notifyOfChange();
	}
}

void Party::adjustGold(int gold) {
	AdjustValue(_saveGame->_gold, gold, 9999, 0);
	notifyOfChange();
}

void Party::adjustKarma(KarmaAction action) {
	int timeLimited = 0;
	int v, newKarma[VIRT_MAX], maxVal[VIRT_MAX];

	/*
	 * make a local copy of all virtues, and adjust it according to
	 * the game rules
	 */
	for (v = 0; v < VIRT_MAX; v++) {
		newKarma[v] = _saveGame->_karma[v] == 0 ? 100 : _saveGame->_karma[v];
		maxVal[v] = _saveGame->_karma[v] == 0 ? 100 : 99;
	}

	switch (action) {
	case KA_FOUND_ITEM:
		AdjustValueMax(newKarma[VIRT_HONOR], 5, maxVal[VIRT_HONOR]);
		break;
	case KA_STOLE_CHEST:
		AdjustValueMin(newKarma[VIRT_HONESTY], -1, 1);
		AdjustValueMin(newKarma[VIRT_JUSTICE], -1, 1);
		AdjustValueMin(newKarma[VIRT_HONOR], -1, 1);
		break;
	case KA_GAVE_ALL_TO_BEGGAR:
	//  When donating all, you get +3 HONOR in Apple 2, but not in in U4DOS.
	//  TODO: Make this a configuration option.
	//  AdjustValueMax(newKarma[VIRT_HONOR], 3, maxVal[VIRT_HONOR]);
	case KA_GAVE_TO_BEGGAR:
		//  In U4DOS, we only get +2 COMPASSION, no HONOR or SACRIFICE even if
		//  donating all.
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_COMPASSION], 2, maxVal[VIRT_COMPASSION]);
		break;
	case KA_BRAGGED:
		AdjustValueMin(newKarma[VIRT_HUMILITY], -5, 1);
		break;
	case KA_HUMBLE:
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_HUMILITY], 10, maxVal[VIRT_HUMILITY]);
		break;
	case KA_HAWKWIND:
	case KA_MEDITATION:
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_SPIRITUALITY], 3, maxVal[VIRT_SPIRITUALITY]);
		break;
	case KA_BAD_MANTRA:
		AdjustValueMin(newKarma[VIRT_SPIRITUALITY], -3, 1);
		break;
	case KA_ATTACKED_GOOD:
		AdjustValueMin(newKarma[VIRT_COMPASSION], -5, 1);
		AdjustValueMin(newKarma[VIRT_JUSTICE], -5, 1);
		AdjustValueMin(newKarma[VIRT_HONOR], -5, 1);
		break;
	case KA_FLED_EVIL:
		AdjustValueMin(newKarma[VIRT_VALOR], -2, 1);
		break;
	case KA_HEALTHY_FLED_EVIL:
		AdjustValueMin(newKarma[VIRT_VALOR], -2, 1);
		AdjustValueMin(newKarma[VIRT_SACRIFICE], -2, 1);
		break;
	case KA_KILLED_EVIL:
		AdjustValueMax(newKarma[VIRT_VALOR], xu4_random(2), maxVal[VIRT_VALOR]); /* gain one valor half the time, zero the rest */
		break;
	case KA_FLED_GOOD:
		AdjustValueMax(newKarma[VIRT_COMPASSION], 2, maxVal[VIRT_COMPASSION]);
		AdjustValueMax(newKarma[VIRT_JUSTICE], 2, maxVal[VIRT_JUSTICE]);
		break;
	case KA_SPARED_GOOD:
		AdjustValueMax(newKarma[VIRT_COMPASSION], 1, maxVal[VIRT_COMPASSION]);
		AdjustValueMax(newKarma[VIRT_JUSTICE], 1, maxVal[VIRT_JUSTICE]);
		break;
	case KA_DONATED_BLOOD:
		AdjustValueMax(newKarma[VIRT_SACRIFICE], 5, maxVal[VIRT_SACRIFICE]);
		break;
	case KA_DIDNT_DONATE_BLOOD:
		AdjustValueMin(newKarma[VIRT_SACRIFICE], -5, 1);
		break;
	case KA_CHEAT_REAGENTS:
		AdjustValueMin(newKarma[VIRT_HONESTY], -10, 1);
		AdjustValueMin(newKarma[VIRT_JUSTICE], -10, 1);
		AdjustValueMin(newKarma[VIRT_HONOR], -10, 1);
		break;
	case KA_DIDNT_CHEAT_REAGENTS:
		timeLimited = 1;
		AdjustValueMax(newKarma[VIRT_HONESTY], 2, maxVal[VIRT_HONESTY]);
		AdjustValueMax(newKarma[VIRT_JUSTICE], 2, maxVal[VIRT_JUSTICE]);
		AdjustValueMax(newKarma[VIRT_HONOR], 2, maxVal[VIRT_HONOR]);
		break;
	case KA_USED_SKULL:
		/* using the skull is very, very bad... */
		for (v = 0; v < VIRT_MAX; v++)
			AdjustValueMin(newKarma[v], -5, 1);
		break;
	case KA_DESTROYED_SKULL:
		/* ...but destroying it is very, very good */
		for (v = 0; v < VIRT_MAX; v++)
			AdjustValueMax(newKarma[v], 10, maxVal[v]);
		break;
	}

	/*
	 * check if enough time has passed since last virtue award if
	 * action is time limited -- if not, throw away new values
	 */
	if (timeLimited) {
		if (((_saveGame->_moves / 16) >= 0x10000) || (((_saveGame->_moves / 16) & 0xFFFF) != _saveGame->_lastVirtue))
			_saveGame->_lastVirtue = (_saveGame->_moves / 16) & 0xFFFF;
		else
			return;
	}

	/* something changed */
	notifyOfChange();

	/*
	 * return to u4dos compatibility and handle losing of eighths
	 */
	for (v = 0; v < VIRT_MAX; v++) {
		if (maxVal[v] == 100) { /* already an avatar */
			if (newKarma[v] < 100) { /* but lost it */
				_saveGame->_karma[v] = newKarma[v];
				setChanged();
				PartyEvent event(PartyEvent::LOST_EIGHTH, 0);
				notifyObservers(event);
			} else _saveGame->_karma[v] = 0; /* return to u4dos compatibility */
		} else _saveGame->_karma[v] = newKarma[v];
	}
}

void Party::applyEffect(TileEffect effect) {
	int i;

	for (i = 0; i < size(); i++) {
		switch (effect) {
		case EFFECT_NONE:
		case EFFECT_ELECTRICITY:
			_members[i]->applyEffect(effect);
			break;
		case EFFECT_LAVA:
		case EFFECT_FIRE:
		case EFFECT_SLEEP:
			if (xu4_random(2) == 0)
				_members[i]->applyEffect(effect);
			break;
		case EFFECT_POISONFIELD:
		case EFFECT_POISON:
			if (xu4_random(5) == 0)
				_members[i]->applyEffect(effect);
			break;
		default:
			break;
		}
	}
}

bool Party::attemptElevation(Virtue virtue) {
	if (_saveGame->_karma[virtue] == 99) {
		_saveGame->_karma[virtue] = 0;
		notifyOfChange();
		return true;
	} else
		return false;
}

void Party::burnTorch(int turns) {
	_torchDuration -= turns;
	if (_torchDuration <= 0)
		_torchDuration = 0;

	_saveGame->_torchDuration = _torchDuration;

	notifyOfChange();
}

bool Party::canEnterShrine(Virtue virtue) {
	if (_saveGame->_runes & (1 << (int) virtue))
		return true;
	else
		return false;
}

bool Party::canPersonJoin(Common::String name, Virtue *v) {
	int i;

	if (name.empty())
		return 0;

	for (i = 1; i < 8; i++) {
		if (name == _saveGame->_players[i]._name) {
			if (v)
				*v = (Virtue) _saveGame->_players[i]._class;
			return true;
		}
	}
	return false;
}

void Party::damageShip(uint pts) {
	_saveGame->_shipHull -= pts;
	if ((short)_saveGame->_shipHull < 0)
		_saveGame->_shipHull = 0;

	notifyOfChange();
}

bool Party::donate(int quantity) {
	if (quantity > _saveGame->_gold)
		return false;

	adjustGold(-quantity);
	if (_saveGame->_gold > 0)
		adjustKarma(KA_GAVE_TO_BEGGAR);
	else adjustKarma(KA_GAVE_ALL_TO_BEGGAR);

	return true;
}

void Party::endTurn() {
	int i;

	_saveGame->_moves++;

	for (i = 0; i < size(); i++) {

		/* Handle player status (only for non-combat turns) */
		if ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context) {

			/* party members eat food (also non-combat) */
			if (!_members[i]->isDead())
				adjustFood(-1);

			switch (_members[i]->getStatus()) {
			case STAT_SLEEPING:
				if (xu4_random(5) == 0)
					_members[i]->wakeUp();
				break;

			case STAT_POISONED:
				/* SOLUS
				 * shouldn't play poison damage sound in combat,
				 * yet if the PC takes damage just befor combat
				 * begins, the sound is played  after the combat
				 * screen appears
				 */
				soundPlay(SOUND_POISON_DAMAGE, false);
				_members[i]->applyDamage(2);
				break;

			default:
				break;
			}
		}

		/* regenerate magic points */
		if (!_members[i]->isDisabled() && _members[i]->getMp() < _members[i]->getMaxMp())
			_saveGame->_players[i]._mp++;
	}

	/* The party is starving! */
	if ((_saveGame->_food == 0) && ((g_context->_location->_context & CTX_NON_COMBAT) == g_context->_location->_context)) {
		setChanged();
		PartyEvent event(PartyEvent::STARVING, 0);
		notifyObservers(event);
	}

	/* heal ship (25% chance it is healed each turn) */
	if ((g_context->_location->_context == CTX_WORLDMAP) && (_saveGame->_shipHull < 50) && xu4_random(4) == 0)
		healShip(1);
}

int Party::getChest() {
	int gold = xu4_random(50) + xu4_random(8) + 10;
	adjustGold(gold);

	return gold;
}

int Party::getTorchDuration() const {
	return _torchDuration;
}

void Party::healShip(uint pts) {
	_saveGame->_shipHull += pts;
	if (_saveGame->_shipHull > 50)
		_saveGame->_shipHull = 50;

	notifyOfChange();
}

bool Party::isFlying() const {
	return (_saveGame->_balloonState && _torchDuration <= 0);
}

bool Party::isImmobilized() {
	int i;
	bool immobile = true;

	for (i = 0; i < _saveGame->_members; i++) {
		if (!_members[i]->isDisabled())
			immobile = false;
	}

	return immobile;
}

bool Party::isDead() {
	int i;
	bool dead = true;

	for (i = 0; i < _saveGame->_members; i++) {
		if (!_members[i]->isDead()) {
			dead = false;
		}
	}

	return dead;
}

bool Party::isPersonJoined(Common::String name) {
	int i;

	if (name.empty())
		return false;

	for (i = 1; i < _saveGame->_members; i++) {
		if (name == _saveGame->_players[i]._name)
			return true;
	}
	return false;
}

CannotJoinError Party::join(Common::String name) {
	int i;
	SaveGamePlayerRecord tmp;

	for (i = _saveGame->_members; i < 8; i++) {
		if (name == _saveGame->_players[i]._name) {

			/* ensure avatar is experienced enough */
			if (_saveGame->_members + 1 > (_saveGame->_players[0]._hpMax / 100))
				return JOIN_NOT_EXPERIENCED;

			/* ensure character has enough karma */
			if ((_saveGame->_karma[_saveGame->_players[i]._class] > 0) &&
			        (_saveGame->_karma[_saveGame->_players[i]._class] < 40))
				return JOIN_NOT_VIRTUOUS;

			tmp = _saveGame->_players[_saveGame->_members];
			_saveGame->_players[_saveGame->_members] = _saveGame->_players[i];
			_saveGame->_players[i] = tmp;

			_members.push_back(new PartyMember(this, &_saveGame->_players[_saveGame->_members++]));
			setChanged();
			PartyEvent event(PartyEvent::MEMBER_JOINED, _members.back());
			notifyObservers(event);
			return JOIN_SUCCEEDED;
		}
	}

	return JOIN_NOT_EXPERIENCED;
}

bool Party::lightTorch(int duration, bool loseTorch) {
	if (loseTorch) {
		if (g_ultima->_saveGame->_torches <= 0)
			return false;
		g_ultima->_saveGame->_torches--;
	}

	_torchDuration += duration;
	_saveGame->_torchDuration = _torchDuration;

	notifyOfChange();

	return true;
}

void Party::quenchTorch() {
	_torchDuration = _saveGame->_torchDuration = 0;

	notifyOfChange();
}

void Party::reviveParty() {
	int i;

	for (i = 0; i < size(); i++) {
		_members[i]->wakeUp();
		_members[i]->setStatus(STAT_GOOD);
		_saveGame->_players[i]._hp = _saveGame->_players[i]._hpMax;
	}

	for (i = ARMR_NONE + 1; i < ARMR_MAX; i++)
		_saveGame->_armor[i] = 0;
	for (i = WEAP_HANDS + 1; i < WEAP_MAX; i++)
		_saveGame->_weapons[i] = 0;
	_saveGame->_food = 20099;
	_saveGame->_gold = 200;
	setTransport(g_tileSets->findTileByName("avatar")->getId());
	setChanged();
	PartyEvent event(PartyEvent::PARTY_REVIVED, 0);
	notifyObservers(event);
}

MapTile Party::getTransport() const {
	return _transport;
}

void Party::setTransport(MapTile tile) {
	// transport value stored in savegame hardcoded to index into base tilemap
	_saveGame->_transport = g_tileMaps->get("base")->untranslate(tile);
	assertMsg(_saveGame->_transport != 0, "could not generate valid savegame transport for tile with id %d\n", tile._id);

	_transport = tile;

	if (tile.getTileType()->isHorse())
		g_context->_transportContext = TRANSPORT_HORSE;
	else if (tile.getTileType()->isShip())
		g_context->_transportContext = TRANSPORT_SHIP;
	else if (tile.getTileType()->isBalloon())
		g_context->_transportContext = TRANSPORT_BALLOON;
	else g_context->_transportContext = TRANSPORT_FOOT;

	notifyOfChange();
}

void Party::setShipHull(int str) {
	int newStr = str;
	AdjustValue(newStr, 0, 99, 0);

	if (_saveGame->_shipHull != newStr) {
		_saveGame->_shipHull = newStr;
		notifyOfChange();
	}
}

Direction Party::getDirection() const {
	return _transport.getDirection();
}

void Party::setDirection(Direction dir) {
	_transport.setDirection(dir);
}

void Party::adjustReagent(int reagent, int amt) {
	int oldVal = g_ultima->_saveGame->_reagents[reagent];
	AdjustValue(g_ultima->_saveGame->_reagents[reagent], amt, 99, 0);

	if (oldVal != g_ultima->_saveGame->_reagents[reagent]) {
		notifyOfChange();
	}
}

int Party::getReagent(int reagent) const {
	return g_ultima->_saveGame->_reagents[reagent];
}

short *Party::getReagentPtr(int reagent) const {
	return &g_ultima->_saveGame->_reagents[reagent];
}

void Party::setActivePlayer(int p) {
	_activePlayer = p;
	setChanged();
	PartyEvent event(PartyEvent::ACTIVE_PLAYER_CHANGED, _activePlayer < 0 ? 0 : _members[_activePlayer]);
	notifyObservers(event);
}

int Party::getActivePlayer() const {
	return _activePlayer;
}

void Party::swapPlayers(int p1, int p2) {
	assertMsg(p1 < _saveGame->_members, "p1 out of range: %d", p1);
	assertMsg(p2 < _saveGame->_members, "p2 out of range: %d", p2);

	SaveGamePlayerRecord tmp = _saveGame->_players[p1];
	_saveGame->_players[p1] = g_ultima->_saveGame->_players[p2];
	g_ultima->_saveGame->_players[p2] = tmp;

	syncMembers();

	if (p1 == _activePlayer)
		_activePlayer = p2;
	else if (p2 == _activePlayer)
		_activePlayer = p1;

	notifyOfChange(0);
}

void Party::syncMembers() {
	_members.clear();
	for (int i = 0; i < _saveGame->_members; i++) {
		// add the members to the party
		_members.push_back(new PartyMember(this, &_saveGame->_players[i]));
	}
}

int Party::size() const {
	return _members.size();
}

PartyMember *Party::member(int index) const {
	return _members[index];
}

} // End of namespace Ultima4
} // End of namespace Ultima
