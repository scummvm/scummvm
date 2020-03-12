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

#include "ultima/ultima4/player.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/armor.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/mapmgr.h"
#include "ultima/ultima4/names.h"
#include "ultima/ultima4/tilemap.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/utils.h"
#include "ultima/ultima4/weapon.h"

namespace Ultima {
namespace Ultima4 {

bool isPartyMember(Object *punknown) {
    PartyMember *pm;
    if ((pm = dynamic_cast<PartyMember*>(punknown)) != NULL)
        return true;
    else 
        return false;
}

/**
 * PartyMember class implementation
 */ 
PartyMember::PartyMember(Party *p, SaveGamePlayerRecord *pr) : 
    Creature(tileForClass(pr->klass)),
    player(pr),
    party(p)
{
    /* FIXME: we need to rename movement behaviors */
    setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
    this->ranged = Weapon::get(pr->weapon)->getRange() ? 1 : 0;
    setStatus(pr->status);
}

PartyMember::~PartyMember() {
}

/**
 * Notify the party that this player has changed somehow
 */
void PartyMember::notifyOfChange() {
    if (party) {
        party->notifyOfChange(this);
    }
}

/**
 * Provides some translation information for scripts
 */
Common::String PartyMember::translate(Std::vector<Common::String>& parts) {
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
    }
    else if (parts.size() == 2) {
        if (parts[0] == "needs") {
            if (parts[1] == "cure") {
                if (getStatus() == STAT_POISONED)
                    return "true";
                else return "false";
            }
            else if (parts[1] == "heal" || parts[1] == "fullheal") {
                if (getHp() < getMaxHp())
                    return "true";
                else return "false";
            }
            else if (parts[1] == "resurrect") {
                if (getStatus() == STAT_DEAD)
                    return "true";
                else return "false";
            }
        }        
    }    
    return "";
}

int PartyMember::getHp() const      { return player->hp; }

/**
 * Determine the most magic points a character could have
 * given his class and intelligence.
 */
int PartyMember::getMaxMp() const {
    int max_mp = -1;

    switch (player->klass) {
    case CLASS_MAGE:            /*  mage: 200% of int */
        max_mp = player->intel * 2;
        break;

    case CLASS_DRUID:           /* druid: 150% of int */
        max_mp = player->intel * 3 / 2;
        break;

    case CLASS_BARD:            /* bard, paladin, ranger: 100% of int */
    case CLASS_PALADIN:
    case CLASS_RANGER:
        max_mp = player->intel;
        break;

    case CLASS_TINKER:          /* tinker: 50% of int */
        max_mp = player->intel / 2;
        break;

    case CLASS_FIGHTER:         /* fighter, shepherd: no mp at all */
    case CLASS_SHEPHERD:
        max_mp = 0;
        break;

    default:
        ASSERT(0, "invalid player class: %d", player->klass);
    }

    /* mp always maxes out at 99 */
    if (max_mp > 99)
        max_mp = 99;

    return max_mp;
}

const Weapon *PartyMember::getWeapon() const { return Weapon::get(player->weapon); }
const Armor *PartyMember::getArmor() const   { return Armor::get(player->armor); }
Common::String PartyMember::getName() const          { return player->name; }
SexType PartyMember::getSex() const          { return player->sex; }
ClassType PartyMember::getClass() const      { return player->klass; }

CreatureStatus PartyMember::getState() const {
    if (getHp() <= 0)
        return MSTAT_DEAD;
    else if (getHp() < 24)
        return MSTAT_FLEEING;
    else 
        return MSTAT_BARELYWOUNDED;
}

/**
 * Determine what level a character has.
 */
int PartyMember::getRealLevel() const {
    return player->hpMax / 100;
}

/**
 * Determine the highest level a character could have with the number
 * of experience points he has.
 */
int PartyMember::getMaxLevel() const {
    int level = 1;
    int next = 100;

    while (player->xp >= next && level < 8) {
        level++;
        next <<= 1;
    }

    return level;
}

/**
 * Adds a status effect to the player
 */ 
void PartyMember::addStatus(StatusType s) {
    Creature::addStatus(s);
    player->status = status.back();
    notifyOfChange();
}

/**
 * Adjusts the player's mp by 'pts'
 */
void PartyMember::adjustMp(int pts) {
    AdjustValueMax(player->mp, pts, getMaxMp());
    notifyOfChange();
}

/**
 * Advances the player to the next level if they have enough experience
 */
void PartyMember::advanceLevel() {
    if (getRealLevel() == getMaxLevel())
        return;
    setStatus(STAT_GOOD);    
    player->hpMax = getMaxLevel() * 100;
    player->hp = player->hpMax;

    /* improve stats by 1-8 each */
    player->str   += xu4_random(8) + 1;
    player->dex   += xu4_random(8) + 1;
    player->intel += xu4_random(8) + 1;

    if (player->str > 50) player->str = 50;
    if (player->dex > 50) player->dex = 50;
    if (player->intel > 50) player->intel = 50;

    if (party) {
        party->setChanged();
        PartyEvent event(PartyEvent::ADVANCED_LEVEL, this);
        event.player = this;
        party->notifyObservers(event);
    }    
}

/**
 * Apply an effect to the party member
 */
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
            playerApplyDamage(&(c->saveGame->players[i]), 10 + (xu4_random(25)));*/
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
    case EFFECT_ELECTRICITY: break;
    default:
        ASSERT(0, "invalid effect: %d", effect);
    }

    if (effect != EFFECT_NONE)
        notifyOfChange();
}

/**
 * Award a player experience points.  Maxs out the players xp at 9999.
 */
void PartyMember::awardXp(int xp) {
    AdjustValueMax(player->xp, xp, 9999);
    notifyOfChange();
}

/**
 * Perform a certain type of healing on the party member
 */
bool PartyMember::heal(HealType type) {
    switch(type) {

    case HT_NONE:
        return true;

    case HT_CURE:
        if (getStatus() != STAT_POISONED)
            return false;
        removeStatus(STAT_POISONED);
        break;

    case HT_FULLHEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;
        player->hp = player->hpMax;
        break;

    case HT_RESURRECT:
        if (getStatus() != STAT_DEAD)
            return false;
        setStatus(STAT_GOOD);
        break;

    case HT_HEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;        

        player->hp += 75 + (xu4_random(0x100) % 0x19);
        break;

    case HT_CAMPHEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;        
        player->hp += 99 + (xu4_random(0x100) & 0x77);
        break;

    case HT_INNHEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;        
        player->hp += 100 + (xu4_random(50) * 2);
        break;

    default:
        return false;
    }

    if (player->hp > player->hpMax)
        player->hp = player->hpMax;
    
    notifyOfChange();
    
    return true;
}

/**
 * Remove status effects from the party member
 */
void PartyMember::removeStatus(StatusType s) {
    Creature::removeStatus(s);
    player->status = status.back();
    notifyOfChange();
}

void PartyMember::setHp(int hp) {
    player->hp = hp;
    notifyOfChange();
}

void PartyMember::setMp(int mp) {
    player->mp = mp;
    notifyOfChange();
}

EquipError PartyMember::setArmor(const Armor *a) {
    ArmorType type = a->getType();

    if (type != ARMR_NONE && party->saveGame->armor[type] < 1)
        return EQUIP_NONE_LEFT;
    if (!a->canWear(getClass()))
        return EQUIP_CLASS_RESTRICTED;

    ArmorType oldArmorType = getArmor()->getType();
    if (oldArmorType != ARMR_NONE)
        party->saveGame->armor[oldArmorType]++;
    if (type != ARMR_NONE)
        party->saveGame->armor[type]--;

    player->armor = type;
    notifyOfChange();

    return EQUIP_SUCCEEDED;
}

EquipError PartyMember::setWeapon(const Weapon *w) {
    WeaponType type = w->getType();

    if (type != WEAP_HANDS && party->saveGame->weapons[type] < 1)
        return EQUIP_NONE_LEFT;
    if (!w->canReady(getClass()))
        return EQUIP_CLASS_RESTRICTED;

    WeaponType old = getWeapon()->getType();
    if (old != WEAP_HANDS)
        party->saveGame->weapons[old]++;
    if (type != WEAP_HANDS)
        party->saveGame->weapons[type]--;

    player->weapon = type;
    notifyOfChange();

    return EQUIP_SUCCEEDED;
}

/**
 * Applies damage to a player, and changes status to dead if hit
 * points drop below zero.
 * 
 * Byplayer is ignored for now, since it should always be false for U4.  (Is
 * there anything special about being killed by a party member in U5?)  Also
 * keeps interface consistent for virtual base function Creature::applydamage()
 */
bool PartyMember::applyDamage(int damage, bool) {
    int newHp = player->hp;

    if (getStatus() == STAT_DEAD)
        return false;

    newHp -= damage;

    if (newHp < 0) {
        setStatus(STAT_DEAD);
        newHp = 0;
    }
    
    player->hp = newHp;
    notifyOfChange();

    if (isCombatMap(c->_location->map) && getStatus() == STAT_DEAD) {
        Coords p = getCoords();                    
        Map *map = getMap();
        map->annotations->add(p, Tileset::findTileByName("corpse")->getId())->setTTL(party->size() * 2);

        if (party) {
            party->setChanged();
            PartyEvent event(PartyEvent::PLAYER_KILLED, this);
            event.player = this;
            party->notifyObservers(event);
        }

        /* remove yourself from the map */
        remove();        
        return false;
    }

    return true;
}

int PartyMember::getAttackBonus() const {
    if (Weapon::get(player->weapon)->alwaysHits() || player->dex >= 40)
	return 255;
    return player->dex;
}

int PartyMember::getDefense() const {
    return Armor::get(player->armor)->getDefense();
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

/**
 * Calculate damage for an attack.
 */
int PartyMember::getDamage() {
    int maxDamage;

    maxDamage = Weapon::get(player->weapon)->getDamage();
    maxDamage += player->str;
    if (maxDamage > 255)
        maxDamage = 255;

    return xu4_random(maxDamage);
}

/**
 * Returns the tile that will be displayed when the party
 * member's attack hits
 */
const Common::String &PartyMember::getHitTile() const {
    return getWeapon()->getHitTile();
}

/**
 * Returns the tile that will be displayed when the party
 * member's attack fails
 */
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

/**
 * Lose the equipped weapon for the player (flaming oil, ranged daggers, etc.)
 * Returns the number of weapons left of that type, including the one in
 * the players hand
 */
int PartyMember::loseWeapon() {
    int weapon = player->weapon;
    
    notifyOfChange();

    if (party->saveGame->weapons[weapon] > 0)
        return (--party->saveGame->weapons[weapon]) + 1;
    else {
        player->weapon = WEAP_HANDS;
        return 0;
    }
}

/**
 * Put the party member to sleep
 */
void PartyMember::putToSleep() {    
    if (getStatus() != STAT_DEAD) {
        soundPlay(SOUND_SLEEP, false);
        addStatus(STAT_SLEEPING);
        setTile(Tileset::findTileByName("corpse")->getId());
    }
}

/**
 * Wakes up the party member
 */
void PartyMember::wakeUp() {
    removeStatus(STAT_SLEEPING);    
    setTile(tileForClass(getClass()));
}

MapTile PartyMember::tileForClass(int klass) {
    const char *name = NULL;

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
        ASSERT(0, "invalid class %d in tileForClass", klass);
    }

    const Tile *tile = Tileset::get("base")->getByName(name);
    ASSERT(tile, "no tile found for class %d", klass);
    return tile->getId();
}

/**
 * Party class implementation
 */ 
Party::Party(SaveGame *s) : saveGame(s), transport(0), torchduration(0), activePlayer(-1) {
    if (MAP_DECEIT <= saveGame->location && saveGame->location <= MAP_ABYSS)
        torchduration = saveGame->torchduration;
    for (int i = 0; i < saveGame->members; i++) {
        // add the members to the party
        members.push_back(new PartyMember(this, &saveGame->players[i]));
    }    

    // set the party's transport (transport value stored in savegame
    // hardcoded to index into base tilemap)
    setTransport(TileMap::get("base")->translate(saveGame->transport));
}

Party::~Party() {

}

/**
 * Notify the party that something about it has changed
 */
void Party::notifyOfChange(PartyMember *pm, PartyEvent::Type eventType) {
    setChanged();
    PartyEvent event(eventType, pm);
    notifyObservers(event);
}

Common::String Party::translate(Std::vector<Common::String>& parts) {        
    if (parts.size() == 0)
        return "";
    else if (parts.size() == 1) {
        // Translate some different items for the script
        if (parts[0] == "transport") {
            if (c->_transportContext & TRANSPORT_FOOT)
                return "foot";
            if (c->_transportContext & TRANSPORT_HORSE)
                return "horse";
            if (c->_transportContext & TRANSPORT_SHIP)
                return "ship";
            if (c->_transportContext & TRANSPORT_BALLOON)
                return "balloon";
        }
        else if (parts[0] == "gold")
            return xu4_to_string(saveGame->gold);
        else if (parts[0] == "food")
            return xu4_to_string(saveGame->food);
        else if (parts[0] == "members")
            return xu4_to_string(size());
        else if (parts[0] == "keys")        
            return xu4_to_string(saveGame->keys);
        else if (parts[0] == "torches")
            return xu4_to_string(saveGame->torches);
        else if (parts[0] == "gems")
            return xu4_to_string(saveGame->gems);
        else if (parts[0] == "sextants")
            return xu4_to_string(saveGame->sextants);
        else if (parts[0] == "food")
            return xu4_to_string((saveGame->food / 100));
        else if (parts[0] == "gold")
            return xu4_to_string(saveGame->gold);
        else if (parts[0] == "party_members")
            return xu4_to_string(saveGame->members);
        else if (parts[0] == "moves")
            return xu4_to_string(saveGame->moves);
    }
    else if (parts.size() >= 2) {
        if (parts[0].findFirstOf("member") == 0) {
            // Make a new parts list, but remove the first item
            Std::vector<Common::String> new_parts = parts;
            new_parts.erase(new_parts.begin());

            // Find the member we'll be working with
            Common::String str = parts[0];
            Common::String::value_type pos = str.findFirstOf("1234567890");
            if (pos != Common::String::npos) {
                str = str.substr(pos);
                int p_member = (int)strtol(str.c_str(), NULL, 10);

                // Make the party member translate its own stuff
                if (p_member > 0)
                    return member(p_member - 1)->translate(new_parts);
            }
        }

        else if (parts.size() == 2) {
            if (parts[0] == "weapon") {
                const Weapon *w = Weapon::get(parts[1]);
                if (w)
                    return xu4_to_string(saveGame->weapons[w->getType()]);
            }
            else if (parts[0] == "armor") {
                const Armor *a = Armor::get(parts[1]);
                if (a)
                    return xu4_to_string(saveGame->armor[a->getType()]);
            }
        }
    }    
    return "";
}

void Party::adjustFood(int food) {
    int oldFood = saveGame->food;    
    AdjustValue(saveGame->food, food, 999900, 0);
    if ((saveGame->food / 100) != (oldFood / 100)) {
        notifyOfChange();
    }
}

void Party::adjustGold(int gold) {
    AdjustValue(saveGame->gold, gold, 9999, 0);    
    notifyOfChange();
}

/**
 * Adjusts the avatar's karma level for the given action.  Notify
 * observers with a lost eighth event if the player has lost
 * avatarhood.
 */
void Party::adjustKarma(KarmaAction action) {
    int timeLimited = 0;
    int v, newKarma[VIRT_MAX], maxVal[VIRT_MAX];

    /*
     * make a local copy of all virtues, and adjust it according to
     * the game rules
     */
    for (v = 0; v < VIRT_MAX; v++) {
        newKarma[v] = saveGame->karma[v] == 0 ? 100 : saveGame->karma[v];
        maxVal[v] = saveGame->karma[v] == 0 ? 100 : 99;
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
        if (((saveGame->moves / 16) >= 0x10000) || (((saveGame->moves / 16) & 0xFFFF) != saveGame->lastvirtue))
            saveGame->lastvirtue = (saveGame->moves / 16) & 0xFFFF;
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
                saveGame->karma[v] = newKarma[v];
                setChanged();
                PartyEvent event(PartyEvent::LOST_EIGHTH, 0);
                notifyObservers(event);
            }
            else saveGame->karma[v] = 0; /* return to u4dos compatibility */
        }
        else saveGame->karma[v] = newKarma[v];
    }
}

/**
 * Apply effects to the entire party
 */
void Party::applyEffect(TileEffect effect) {
    int i;

    for (i = 0; i < size(); i++) {
        switch(effect) {
        case EFFECT_NONE:
        case EFFECT_ELECTRICITY:
            members[i]->applyEffect(effect);
        case EFFECT_LAVA:
        case EFFECT_FIRE:        
        case EFFECT_SLEEP:
            if (xu4_random(2) == 0)
                members[i]->applyEffect(effect);
        case EFFECT_POISONFIELD:
        case EFFECT_POISON:
            if (xu4_random(5) == 0)
                members[i]->applyEffect(effect);
        }        
    }
}

/**
 * Attempt to elevate in the given virtue
 */
bool Party::attemptElevation(Virtue virtue) {
    if (saveGame->karma[virtue] == 99) {
        saveGame->karma[virtue] = 0;
        notifyOfChange();
        return true;
    } else
        return false;
}

/**
 * Burns a torch's duration down a certain number of turns
 */
void Party::burnTorch(int turns) {
    torchduration -= turns;
    if (torchduration <= 0)
        torchduration = 0;

    saveGame->torchduration = torchduration;

    notifyOfChange();
}

/**
 * Returns true if the party can enter the shrine
 */
bool Party::canEnterShrine(Virtue virtue) {
    if (saveGame->runes & (1 << (int) virtue))
        return true;
    else
        return false;
}

/**
 * Returns true if the person can join the party
 */
bool Party::canPersonJoin(Common::String name, Virtue *v) {
    int i;

    if (name.empty())
        return 0;    

    for (i = 1; i < 8; i++) {
        if (name == saveGame->players[i].name) {
            if (v)
                *v = (Virtue) saveGame->players[i].klass;
            return true;
        }
    }
    return false;
}

/**
 * Damages the party's ship
 */
void Party::damageShip(unsigned int pts) {
    saveGame->shiphull -= pts;
    if ((short)saveGame->shiphull < 0)
        saveGame->shiphull = 0;
    
    notifyOfChange();
}

/**
 * Donates 'quantity' gold. Returns true if the donation succeeded,
 * or false if there was not enough gold to make the donation
 */
bool Party::donate(int quantity) {
    if (quantity > saveGame->gold)
        return false;

    adjustGold(-quantity);
    if (saveGame->gold > 0)
        adjustKarma(KA_GAVE_TO_BEGGAR);
    else adjustKarma(KA_GAVE_ALL_TO_BEGGAR);

    return true;
}

/**
 * Ends the party's turn
 */
void Party::endTurn() {
    int i;    
    
    saveGame->moves++;   
   
    for (i = 0; i < size(); i++) {

        /* Handle player status (only for non-combat turns) */
        if ((c->_location->context & CTX_NON_COMBAT) == c->_location->context) {
            
            /* party members eat food (also non-combat) */
            if (!members[i]->isDead())
                adjustFood(-1);

            switch (members[i]->getStatus()) {
            case STAT_SLEEPING:
                if (xu4_random(5) == 0)
                    members[i]->wakeUp();                    
                break;

            case STAT_POISONED:
                /* SOLUS
                 * shouldn't play poison damage sound in combat,
                 * yet if the PC takes damage just befor combat
                 * begins, the sound is played  after the combat
                 * screen appears
                 */
                soundPlay(SOUND_POISON_DAMAGE, false);
                members[i]->applyDamage(2);
                break;

            default:
                break;
            }
        }

        /* regenerate magic points */
        if (!members[i]->isDisabled() && members[i]->getMp() < members[i]->getMaxMp())
            saveGame->players[i].mp++;
    }

    /* The party is starving! */
    if ((saveGame->food == 0) && ((c->_location->context & CTX_NON_COMBAT) == c->_location->context)) {
        setChanged();
        PartyEvent event(PartyEvent::STARVING, 0);
        notifyObservers(event);
    }
    
    /* heal ship (25% chance it is healed each turn) */
    if ((c->_location->context == CTX_WORLDMAP) && (saveGame->shiphull < 50) && xu4_random(4) == 0)
        healShip(1);
}

/**
 * Adds a chest worth of gold to the party's inventory
 */
int Party::getChest() {
    int gold = xu4_random(50) + xu4_random(8) + 10;
    adjustGold(gold);    

    return gold;
}

/**
 * Returns the number of turns a currently lit torch will last (or 0 if no torch lit)
 */
int Party::getTorchDuration() const {
    return torchduration;
}

/**
 * Heals the ship's hull strength by 'pts' points
 */
void Party::healShip(unsigned int pts) {
    saveGame->shiphull += pts;
    if (saveGame->shiphull > 50)
        saveGame->shiphull = 50;

    notifyOfChange();
}

/**
 * Returns true if the balloon is currently in the air
 */
bool Party::isFlying() const {
    return (saveGame->balloonstate && torchduration <= 0);        
}

/**
 * Whether or not the party can make an action.
 */
bool Party::isImmobilized() {
    int i;
    bool immobile = true;

    for (i = 0; i < saveGame->members; i++) {
        if (!members[i]->isDisabled())
            immobile = false;
    }

    return immobile;
}

/**
 * Whether or not all the party members are dead.
 */
bool Party::isDead() {
    int i;
    bool dead = true;

    for (i = 0; i < saveGame->members; i++) {
        if (!members[i]->isDead()) {
            dead = false;
        }
    }

    return dead;
}

/**
 * Returns true if the person with that name
 * is already in the party
 */
bool Party::isPersonJoined(Common::String name) {
    int i;

    if (name.empty())
        return false;

    for (i = 1; i < saveGame->members; i++) {
        if (name == saveGame->players[i].name)
            return true;
    }
    return false;
}

/**
 * Attempts to add the person to the party.
 * Returns JOIN_SUCCEEDED if successful.
 */
CannotJoinError Party::join(Common::String name) {
    int i;
    SaveGamePlayerRecord tmp;

    for (i = saveGame->members; i < 8; i++) {
        if (name == saveGame->players[i].name) {

            /* ensure avatar is experienced enough */
            if (saveGame->members + 1 > (saveGame->players[0].hpMax / 100))
                return JOIN_NOT_EXPERIENCED;

            /* ensure character has enough karma */
            if ((saveGame->karma[saveGame->players[i].klass] > 0) &&
                (saveGame->karma[saveGame->players[i].klass] < 40))
                return JOIN_NOT_VIRTUOUS;

            tmp = saveGame->players[saveGame->members];
            saveGame->players[saveGame->members] = saveGame->players[i];
            saveGame->players[i] = tmp;            

            members.push_back(new PartyMember(this, &saveGame->players[saveGame->members++]));
            setChanged();
            PartyEvent event(PartyEvent::MEMBER_JOINED, members.back());
            notifyObservers(event);
            return JOIN_SUCCEEDED;
        }
    }

    return JOIN_NOT_EXPERIENCED;
}

/**
 * Lights a torch with a default duration of 100
 */
bool Party::lightTorch(int duration, bool loseTorch) {
    if (loseTorch) { 
        if (c->_saveGame->torches <= 0)
            return false;
        c->_saveGame->torches--;
    }
    
    torchduration += duration;
    saveGame->torchduration = torchduration;

    notifyOfChange();

    return true;
}

/**
 * Extinguishes a torch
 */
void Party::quenchTorch() {
    torchduration = saveGame->torchduration = 0;

    notifyOfChange();
}

/**
 * Revives the party after the entire party has been killed
 */
void Party::reviveParty() {
    int i;

    for (i = 0; i < size(); i++) {
        members[i]->wakeUp();
        members[i]->setStatus(STAT_GOOD);
        saveGame->players[i].hp = saveGame->players[i].hpMax;
    }
    
    for (int i = ARMR_NONE + 1; i < ARMR_MAX; i++)
        saveGame->armor[i] = 0;
    for (int i = WEAP_HANDS + 1; i < WEAP_MAX; i++)
        saveGame->weapons[i] = 0;
    saveGame->food = 20099;
    saveGame->gold = 200;
    setTransport(Tileset::findTileByName("avatar")->getId());
    setChanged();
    PartyEvent event(PartyEvent::PARTY_REVIVED, 0);
    notifyObservers(event);
}

MapTile Party::getTransport() const {
    return transport;
}

void Party::setTransport(MapTile tile) {
    // transport value stored in savegame hardcoded to index into base tilemap
    saveGame->transport = TileMap::get("base")->untranslate(tile);
    ASSERT(saveGame->transport != 0, "could not generate valid savegame transport for tile with id %d\n", tile._id);

    transport = tile;
    
    if (tile.getTileType()->isHorse())
        c->_transportContext = TRANSPORT_HORSE;
    else if (tile.getTileType()->isShip())
        c->_transportContext = TRANSPORT_SHIP;
    else if (tile.getTileType()->isBalloon())
        c->_transportContext = TRANSPORT_BALLOON;
    else c->_transportContext = TRANSPORT_FOOT;

    notifyOfChange();
}

void Party::setShipHull(int str) {
    int newStr = str;
    AdjustValue(newStr, 0, 99, 0);

    if (saveGame->shiphull != newStr) {
        saveGame->shiphull = newStr;
        notifyOfChange();
    }
}

Direction Party::getDirection() const {
    return transport.getDirection();
}

void Party::setDirection(Direction dir) {
    transport.setDirection(dir);
}

void Party::adjustReagent(int reagent, int amt) {
    int oldVal = c->_saveGame->reagents[reagent];
    AdjustValue(c->_saveGame->reagents[reagent], amt, 99, 0);

    if (oldVal != c->_saveGame->reagents[reagent]) {        
        notifyOfChange();
    }
}

int Party::getReagent(int reagent) const {
    return c->_saveGame->reagents[reagent];
}

short* Party::getReagentPtr(int reagent) const {
    return &c->_saveGame->reagents[reagent];
}

void Party::setActivePlayer(int p) {
    activePlayer = p;
    setChanged();
    PartyEvent event(PartyEvent::ACTIVE_PLAYER_CHANGED, activePlayer < 0 ? 0 : members[activePlayer] );
    notifyObservers(event);
}

int Party::getActivePlayer() const {
    return activePlayer;
}

void Party::swapPlayers(int p1, int p2) {
    ASSERT(p1 < saveGame->members, "p1 out of range: %d", p1);
    ASSERT(p2 < saveGame->members, "p2 out of range: %d", p2);

    SaveGamePlayerRecord tmp = saveGame->players[p1];
    saveGame->players[p1] = c->_saveGame->players[p2];
    c->_saveGame->players[p2] = tmp;

    syncMembers();

    if (p1 == activePlayer)
        activePlayer = p2;
    else if (p2 == activePlayer)
        activePlayer = p1;

    notifyOfChange(0);
}

void Party::syncMembers() {
    members.clear();
    for (int i = 0; i < saveGame->members; i++) {
        // add the members to the party
        members.push_back(new PartyMember(this, &saveGame->players[i]));
    }
}

/**
 * Returns the size of the party
 */
int Party::size() const {
    return members.size();
}

/**
 * Returns a pointer to the party member indicated
 */
PartyMember *Party::member(int index) const {
    return members[index];
}

} // End of namespace Ultima4
} // End of namespace Ultima
