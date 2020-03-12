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

#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/config.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/game.h"       /* required by specialAction and specialEffect functions */
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/player.h"     /* required by specialAction and specialEffect functions */
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/screen.h"     /* FIXME: remove dependence on this */
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/textcolor.h"  /* required to change the color of screen message text */
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

CreatureMgr *CreatureMgr::instance = NULL;

bool isCreature(Object *punknown) {
    Creature *m;
    if ((m = dynamic_cast<Creature*>(punknown)) != NULL)
        return true;
    else
        return false;
}

/**
 * Creature class implementation
 */ 
Creature::Creature(MapTile tile) : 
    Object(Object::CREATURE) {
    const Creature *m = creatureMgr->getByTile(tile);
    if (m)
        *this = *m;
}

void Creature::load(const ConfigElement &conf) {
    unsigned int idx;

    static const struct {
        const char *name;
        unsigned int mask;
    } booleanAttributes[] = {
        { "undead", MATTR_UNDEAD },
        { "good", MATTR_GOOD },
        { "swims", MATTR_WATER },
        { "sails", MATTR_WATER },       
        { "cantattack", MATTR_NONATTACKABLE },       
        { "camouflage", MATTR_CAMOUFLAGE }, 
        { "wontattack", MATTR_NOATTACK },       
        { "ambushes", MATTR_AMBUSHES },
        { "incorporeal", MATTR_INCORPOREAL },
        { "nochest", MATTR_NOCHEST },
        { "divides", MATTR_DIVIDES },
        { "forceOfNature", MATTR_FORCE_OF_NATURE }
    };    
    
    /* steals="" */
    static const struct {
        const char *name;
        unsigned int mask;
    } steals[] = {
        { "food", MATTR_STEALFOOD },
        { "gold", MATTR_STEALGOLD }
    };

    /* casts="" */
    static const struct {
        const char *name;
        unsigned int mask;
    } casts[] = {
        { "sleep", MATTR_CASTS_SLEEP },
        { "negate", MATTR_NEGATE }
    };

    /* movement="" */
    static const struct {
        const char *name;
        unsigned int mask;
    } movement[] = {
        { "none", MATTR_STATIONARY },
        { "wanders", MATTR_WANDERS }
    };

    /* boolean attributes that affect movement */
    static const struct {
        const char *name;
        unsigned int mask;
    } movementBoolean[] = {
        { "swims", MATTR_SWIMS },
        { "sails", MATTR_SAILS },
        { "flies", MATTR_FLIES },
        { "teleports", MATTR_TELEPORT },
        { "canMoveOntoCreatures", MATTR_CANMOVECREATURES },
        { "canMoveOntoAvatar", MATTR_CANMOVEAVATAR }
    };

    static const struct {
        const char *name;
        TileEffect effect;
    } effects[] = {
        { "fire", EFFECT_FIRE },
        { "poison", EFFECT_POISONFIELD },
        { "sleep", EFFECT_SLEEP }
    };    

    name = conf.getString("name");
    id = static_cast<unsigned short>(conf.getInt("id"));
        
    /* Get the leader if it's been included, otherwise the leader is itself */
    leader = static_cast<unsigned char>(conf.getInt("leader", id));
        
    xp = static_cast<unsigned short>(conf.getInt("exp"));
    ranged = conf.getBool("ranged");
    setTile(Tileset::findTileByName(conf.getString("tile")));

    setHitTile("hit_flash");
    setMissTile("miss_flash");

    mattr = static_cast<CreatureAttrib>(0);
    movementAttr = static_cast<CreatureMovementAttrib>(0);
    resists = 0;

    /* get the encounter size */
    encounterSize = conf.getInt("encounterSize", 0);

    /* get the base hp */
    basehp = conf.getInt("basehp", 0);
    /* adjust basehp according to battle difficulty setting */
    if (settings.battleDiff == "Hard")
        basehp *= 2;
    if (settings.battleDiff == "Expert")
        basehp *= 4;

    /* get the camouflaged tile */
    if (conf.exists("camouflageTile"))
        camouflageTile = conf.getString("camouflageTile");

    /* get the ranged tile for world map attacks */
    if (conf.exists("worldrangedtile"))
        worldrangedtile = conf.getString("worldrangedtile");

    /* get ranged hit tile */
    if (conf.exists("rangedhittile")) {
        if (conf.getString("rangedhittile") == "random")
            mattr = static_cast<CreatureAttrib>(mattr | MATTR_RANDOMRANGED);
        else 
            setHitTile(conf.getString("rangedhittile"));
    }
        
    /* get ranged miss tile */
    if (conf.exists("rangedmisstile")) {
        if (conf.getString("rangedmisstile") ==  "random")
            mattr = static_cast<CreatureAttrib>(mattr | MATTR_RANDOMRANGED);
        else
            setMissTile(conf.getString("rangedmisstile"));
    }

    /* find out if the creature leaves a tile behind on ranged attacks */
    leavestile = conf.getBool("leavestile");

    /* get effects that this creature is immune to */
    for (idx = 0; idx < sizeof(effects) / sizeof(effects[0]); idx++) {
        if (conf.getString("resists") == effects[idx].name) {
            resists = effects[idx].effect;
        }
    }
        
    /* Load creature attributes */
    for (idx = 0; idx < sizeof(booleanAttributes) / sizeof(booleanAttributes[0]); idx++) {
        if (conf.getBool(booleanAttributes[idx].name)) {
            mattr = static_cast<CreatureAttrib>(mattr | booleanAttributes[idx].mask);
        }
    }
        
    /* Load boolean attributes that affect movement */
    for (idx = 0; idx < sizeof(movementBoolean) / sizeof(movementBoolean[0]); idx++) {
        if (conf.getBool(movementBoolean[idx].name)) {
            movementAttr = static_cast<CreatureMovementAttrib>(movementAttr | movementBoolean[idx].mask);
        }
    }

    /* steals="" */
    for (idx = 0; idx < sizeof(steals) / sizeof(steals[0]); idx++) {
        if (conf.getString("steals") == steals[idx].name) {
            mattr = static_cast<CreatureAttrib>(mattr | steals[idx].mask);
        }
    }

    /* casts="" */
    for (idx = 0; idx < sizeof(casts) / sizeof(casts[0]); idx++) {
        if (conf.getString("casts") == casts[idx].name) {
            mattr = static_cast<CreatureAttrib>(mattr | casts[idx].mask);
        }
    }

    /* movement="" */
    for (idx = 0; idx < sizeof(movement) / sizeof(movement[0]); idx++) {
        if (conf.getString("movement") == movement[idx].name) {
            movementAttr = static_cast<CreatureMovementAttrib>(movementAttr | movement[idx].mask);
        }
    }

    if (conf.exists("spawnsOnDeath")) {
        mattr = static_cast<CreatureAttrib>(mattr | MATTR_SPAWNSONDEATH);
        spawn = static_cast<unsigned char>(conf.getInt("spawnsOnDeath"));
    }

    /* Figure out which 'slowed' function to use */
    slowedType = SLOWED_BY_TILE;
    if (sails())
        /* sailing creatures (pirate ships) */
        slowedType = SLOWED_BY_WIND;
    else if (flies() || isIncorporeal())
        /* flying creatures (dragons, bats, etc.) and incorporeal creatures (ghosts, zorns) */
        slowedType = SLOWED_BY_NOTHING;
}

bool Creature::isAttackable() const  { 
    if (mattr & MATTR_NONATTACKABLE)
        return false;
    /* can't attack horse transport */
    if (tile.getTileType()->isHorse() && getMovementBehavior() == MOVEMENT_FIXED)
        return false;
    return true; 
}

int  Creature::getDamage() const {
    int damage, val, x;
    val = basehp;    
    x = xu4_random(val >> 2);
    damage = (x >> 4) + ((x >> 2) & 0xfc);
    damage += x % 10;
    return damage;
}

int Creature::setInitialHp(int points) {
    if (points < 0)
        hp = xu4_random(basehp) | (basehp / 2);
    else
        hp = points;
    
    /* make sure the creature doesn't flee initially */
    if (hp < 24) hp = 24;

    return hp;
}

void Creature::setRandomRanged() {
    switch(xu4_random(4)) {
    case 0:
        rangedhittile = rangedmisstile = "poison_field";
        break;
    case 1:
        rangedhittile = rangedmisstile = "energy_field";
        break;
    case 2:
        rangedhittile = rangedmisstile = "fire_field";
        break;
    case 3:
        rangedhittile = rangedmisstile = "sleep_field";
        break;
    }
}

CreatureStatus Creature::getState() const {
    int heavy_threshold, light_threshold, crit_threshold;
    
    crit_threshold = basehp >> 2;  /* (basehp / 4) */
    heavy_threshold = basehp >> 1; /* (basehp / 2) */
    light_threshold = crit_threshold + heavy_threshold;

    if (hp <= 0)
        return MSTAT_DEAD;
    else if (hp < 24)
        return MSTAT_FLEEING;
    else if (hp < crit_threshold)
        return MSTAT_CRITICAL;
    else if (hp < heavy_threshold)
        return MSTAT_HEAVILYWOUNDED;
    else if (hp < light_threshold)
        return MSTAT_LIGHTLYWOUNDED;
    else
        return MSTAT_BARELYWOUNDED;

}

/**
 * Performs a special action for the creature
 * Returns true if the action takes up the creatures
 * whole turn (i.e. it cant move afterwords)
 */ 
bool Creature::specialAction() {
    bool retval = false;        

    int dx = abs(c->_location->coords.x - coords.x);
    int dy = abs(c->_location->coords.y - coords.y);
    int mapdist = c->_location->coords.distance(coords, c->_location->map);

    /* find out which direction the avatar is in relation to the creature */
    MapCoords mapcoords(coords);
    int dir = mapcoords.getRelativeDirection(c->_location->coords, c->_location->map);

    //Init outside of switch
    int broadsidesDirs = 0;

    switch(id) {
    
    case LAVA_LIZARD_ID:
    case SEA_SERPENT_ID:
    case HYDRA_ID:
    case DRAGON_ID:       
        
        /* A 50/50 chance they try to range attack when you're close enough 
           and not in a city
           Note: Monsters in settlements in U3 do fire on party
        */
        if (mapdist <= 3 && xu4_random(2) == 0 && (c->_location->context & CTX_CITY) == 0) {
            Std::vector<Coords> path = gameGetDirectionalActionPath(dir, MASK_DIR_ALL, coords,
                                                               1, 3, NULL, false);
            for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
                if (creatureRangeAttack(*i, this))
                    break;
            }
        }
        
        break;

    case PIRATE_ID:
        
        /* Fire cannon: Pirates only fire broadsides and only when they can hit you :) */
        retval = true;
        broadsidesDirs = dirGetBroadsidesDirs(tile.getDirection());

        if ((((dx == 0) && (dy <= 3)) ||          /* avatar is close enough and on the same column, OR */
             ((dy == 0) && (dx <= 3))) &&         /* avatar is close enough and on the same row, AND */
            ((broadsidesDirs & dir) > 0)) { /* pirate ship is firing broadsides */

            // nothing (not even mountains!) can block cannonballs
            Std::vector<Coords> path = gameGetDirectionalActionPath(dir, broadsidesDirs, coords,
                                                               1, 3, NULL, false);
            for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
                if (fireAt(*i, false))
                    break;
            }
        }
        else
            retval = false;
        
        break;

    default: break;
    }

    return retval;
}

/**
 * Performs a special effect for the creature
 * Returns true if something special happened,
 * or false if nothing happened
 */ 
bool Creature::specialEffect() {
    Object *obj;    
    bool retval = false;
    
    switch(id) {

    case STORM_ID:
        {
            ObjectDeque::iterator i;

            if (coords == c->_location->coords) {

                /* damage the ship */
                if (c->_transportContext == TRANSPORT_SHIP) {
                    /* FIXME: Check actual damage from u4dos */                           
                    gameDamageShip(10, 30);                        
                }
                /* anything else but balloon damages the party */
                else if (c->_transportContext != TRANSPORT_BALLOON) {
                    /* FIXME: formula for twister damage is guesstimated from u4dos */
                    gameDamageParty(0, 75);
                }
                return true;
            }

            /* See if the storm is on top of any objects and destroy them! */
            for (i = c->_location->map->objects.begin();
                 i != c->_location->map->objects.end();) {

                obj = *i;
                if (this != obj &&
                    obj->getCoords() == coords) {
                    /* Converged with an object, destroy the object! */
                    i = c->_location->map->removeObject(i);
                    retval = true;
                }
                else i++;
            }
        }      
        break;
    
    case WHIRLPOOL_ID:        
        {
            ObjectDeque::iterator i;

            if (coords == c->_location->coords && (c->_transportContext == TRANSPORT_SHIP)) {                    
                                
                /* Deal 10 damage to the ship */
                gameDamageShip(-1, 10);

                /* Send the party to Locke Lake */
                c->_location->coords = c->_location->map->getLabel("lockelake");

                /* Teleport the whirlpool that sent you there far away from lockelake */
                this->setCoords(Coords(0,0,0));
                retval = true;
                break;
            }

            /* See if the whirlpool is on top of any objects and destroy them! */
            for (i = c->_location->map->objects.begin();
                 i != c->_location->map->objects.end();) {

                obj = *i;

                if (this != obj &&
                	obj->getCoords() == coords) {

                	Creature *m = dynamic_cast<Creature*>(obj);

                    /* Make sure the object isn't a flying creature or object */
                    if (!m || (m && (m->swims() || m->sails()) && !m->flies())) {
                        /* Destroy the object it met with */
                        i = c->_location->map->removeObject(i);
                        retval = true;
                    }
                    else {
                    	i++;
                    }
                }
                else i++;
            }            
        }

    default: break;
    }

    return retval;
}

void Creature::act(CombatController *controller) {
    int dist;
    CombatAction action;
    Creature *target;

    /* see if creature wakes up if it is asleep */
    if ((getStatus() == STAT_SLEEPING) && (xu4_random(8) == 0))
        wakeUp();    

    /* if the creature is still asleep, then do nothing */
    if (getStatus() == STAT_SLEEPING)
        return;

    if (negates())
        c->_aura->set(Aura::NEGATE, 2);

    /*
     * figure out what to do
     */

    // creatures who teleport do so 1/8 of the time
    if (teleports() && xu4_random(8) == 0)
        action = CA_TELEPORT;
    // creatures who ranged attack do so 1/4 of the time.  Make sure
    // their ranged attack is not negated!
    else if (ranged != 0 && xu4_random(4) == 0 && 
             (rangedhittile != "magic_flash" || (*c->_aura != Aura::NEGATE)))
        action = CA_RANGED;
    // creatures who cast sleep do so 1/4 of the time they don't ranged attack
    else if (castsSleep() && (*c->_aura != Aura::NEGATE) && (xu4_random(4) == 0))
        action = CA_CAST_SLEEP;
    else if (getState() == MSTAT_FLEEING)
        action = CA_FLEE;
    // default action: attack (or move towards) closest target
    else
        action = CA_ATTACK;        
    
    /* 
     * now find out who to do it to
     */

    target = nearestOpponent(&dist, action == CA_RANGED);    
    if (target == NULL)
        return;

    if (action == CA_ATTACK && dist > 1)
        action = CA_ADVANCE;

    /* let's see if the creature blends into the background, or if he appears... */
    if (camouflages() && !hideOrShow())
        return; /* creature is hidden -- no action! */

    switch(action) {
    case CA_ATTACK:
        soundPlay(SOUND_NPC_ATTACK, false);                                    // NPC_ATTACK, melee

        if (controller->attackHit(this, target)) {
            soundPlay(SOUND_PC_STRUCK, false);                                 // PC_STRUCK, melee and ranged
            GameController::flashTile(target->getCoords(), "hit_flash", 4);


            if (!dealDamage(target, getDamage()))
                target = NULL;

            if (target && isPartyMember(target)) {
                /* steal gold if the creature steals gold */
                if (stealsGold() && xu4_random(4) == 0) {
                    soundPlay(SOUND_ITEM_STOLEN, false);                       // ITEM_STOLEN, gold
                    c->_party->adjustGold(-(xu4_random(0x3f)));
                }
            
                /* steal food if the creature steals food */
                if (stealsFood()) {
                    soundPlay(SOUND_ITEM_STOLEN, false);                       // ITEM_STOLEN, food
                    c->_party->adjustFood(-2500);
                }
            }
        } else {
        	GameController::flashTile(target->getCoords(), "miss_flash", 1);
        }
        break;

    case CA_CAST_SLEEP: {            
        screenMessage("\nSleep!\n");

        gameSpellEffect('s', -1, static_cast<Sound>(SOUND_MAGIC)); /* show the sleep spell effect */
        
        /* Apply the sleep spell to party members still in combat */
        if (!isPartyMember(this)) {
            PartyMemberVector party = controller->getMap()->getPartyMembers();
            PartyMemberVector::iterator j;

            for (j = party.begin(); j != party.end(); j++) {
                if (xu4_random(2) == 0)
                    (*j)->putToSleep();
            }
        }
        break;
    }

    case CA_TELEPORT: {
        Coords new_c;
        bool valid = false;
        bool firstTry = true;                    
        
        while (!valid) {
            Map *map = getMap();
            new_c = Coords(xu4_random(map->width), xu4_random(map->height), c->_location->coords.z);
                
            const Tile *tile = map->tileTypeAt(new_c, WITH_OBJECTS);
            
            if (tile->isCreatureWalkable()) {
                /* If the tile would slow me down, try again! */
                if (firstTry && tile->getSpeed() != FAST)
                    firstTry = false;
                /* OK, good enough! */
                else
                    valid = true;
            }
        }
        
        /* Teleport! */
        setCoords(new_c);
        break;
    }

    case CA_RANGED: {
        // if the creature has a random tile for a ranged weapon,
        // let's switch it now!
        if (hasRandomRanged())
            setRandomRanged();

        MapCoords m_coords = getCoords(),
            p_coords = target->getCoords();

        // figure out which direction to fire the weapon
        int dir = m_coords.getRelativeDirection(p_coords);

        soundPlay(SOUND_NPC_ATTACK, false);                                    // NPC_ATTACK, ranged

        Std::vector<Coords> path = gameGetDirectionalActionPath(dir, MASK_DIR_ALL, m_coords,
                                                           1, 11, &Tile::canAttackOverTile, false);
        bool hit = false;
        for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
            if (controller->rangedAttack(*i, this)) {
                hit = true;
                break;
            }
        }
        if (!hit && path.size() > 0)
            controller->rangedMiss(path[path.size() - 1], this);

        break;
    }

    case CA_FLEE:
    case CA_ADVANCE: {
        Map *map = getMap();
        if (moveCombatObject(action, map, this, target->getCoords())) {
            Coords coords = getCoords();

            if (MAP_IS_OOB(map, coords)) {
                screenMessage("\n%c%s Flees!%c\n", FG_YELLOW, name.c_str(), FG_WHITE);
                
                /* Congrats, you have a heart! */
                if (isGood())
                    c->_party->adjustKarma(KA_SPARED_GOOD);

                map->removeObject(this);                
            }
        }
        break;
    }
    }
    this->animateMovement();
}

/**
 * Add status effects to the creature, in order of importance
 */
void Creature::addStatus(StatusType s) {
    if (status.size() && status.back() > s) {
        StatusType prev = status.back();
        status.pop_back();
        status.push_back(s);
        status.push_back(prev);
    }
    else status.push_back(s);
}

void Creature::applyTileEffect(TileEffect effect) {        
    if (effect != EFFECT_NONE) {
        gameUpdateScreen();

        switch(effect) {
        case EFFECT_SLEEP:
            /* creature fell asleep! */
            if ((resists != EFFECT_SLEEP) &&
                (xu4_random(0xFF) >= hp))
                putToSleep();            
            break;

        case EFFECT_LAVA:
        case EFFECT_FIRE:
            /* deal 0 - 127 damage to the creature if it is not immune to fire damage */
            if ((resists != EFFECT_FIRE) && (resists != EFFECT_LAVA))
                applyDamage(xu4_random(0x7F), false);
            break;

        case EFFECT_POISONFIELD:
            /* deal 0 - 127 damage to the creature if it is not immune to poison field damage */
            if (resists != EFFECT_POISONFIELD)
                applyDamage(xu4_random(0x7F), false);
            break;

        case EFFECT_POISON:
        default: break;
        }
    }
}

int Creature::getAttackBonus() const {
    return 0;
}

int Creature::getDefense() const {
    return 128;
}

bool Creature::divide() {
    Map *map = getMap();
    int dirmask = map->getValidMoves(getCoords(), getTile());
    Direction d = dirRandomDir(dirmask);

    /* this is a game enhancement, make sure it's turned on! */
    if (!settings.enhancements || !settings.enhancementsOptions.slimeDivides)
        return false;
    
    /* make sure there's a place to put the divided creature! */
    if (d != DIR_NONE) {                            
        MapCoords coords(getCoords());
        
        screenMessage("%s Divides!\n", name.c_str());

        /* find a spot to put our new creature */        
        coords.move(d, map);

        /* create our new creature! */
        Creature * addedCreature = map->addCreature(this, coords);
        int dividedHp = (this->hp + 1) / 2;
        addedCreature->hp = dividedHp;
        this->hp = dividedHp;
        return true;
    }
    return false;
}

bool Creature::spawnOnDeath() {
    Map *map = getMap();

    /* this is a game enhancement, make sure it's turned on! */
    if (!settings.enhancements || !settings.enhancementsOptions.gazerSpawnsInsects)
        return false;
    
    /* make sure there's a place to put the divided creature! */
    MapCoords coords(getCoords());
        
    /* create our new creature! */
    map->addCreature(creatureMgr->getById(spawn), coords);                
    return true;
}

StatusType Creature::getStatus() const {
    return status.back();
}

bool Creature::isAsleep() const {
	for (StatusList::const_iterator itr = this->status.begin();
		itr != this->status.end();
		++itr)
		if (*itr == STAT_SLEEPING)
			return true;
	return false;
}

/**
 * Hides or shows a camouflaged creature, depending on its distance from
 * the nearest opponent
 */
bool Creature::hideOrShow() {
    /* find the nearest opponent */
    int dist;
    
    /* ok, now we've got the nearest party member.  Now, see if they're close enough */
    if (nearestOpponent(&dist, false) != NULL) {
        if ((dist < 5) && !isVisible())
            setVisible(); /* show yourself */
        else if (dist >= 5)
            setVisible(false); /* hide and take no action! */
    }

    return isVisible();
}

Creature *Creature::nearestOpponent(int *dist, bool ranged) {
    Creature *opponent = NULL;
    int d, leastDist = 0xFFFF;    
    ObjectDeque::iterator i;
    bool jinx = (*c->_aura == Aura::JINX);
    Map *map = getMap();

    for (i = map->objects.begin(); i != map->objects.end(); ++i) {
        if (!isCreature(*i))
            continue;

        bool amPlayer = isPartyMember(this);
        bool fightingPlayer = isPartyMember(*i);

        /* if a party member, find a creature. If a creature, find a party member */
        /* if jinxed is false, find anything that isn't self */
        if ((amPlayer != fightingPlayer) || 
            (jinx && !amPlayer && *i != this)) {
            MapCoords objCoords = (*i)->getCoords();

            /* if ranged, get the distance using diagonals, otherwise get movement distance */
            if (ranged)
                d = objCoords.distance(getCoords());
            else d = objCoords.movementDistance(getCoords());
            
            /* skip target 50% of time if same distance */
            if (d < leastDist || (d == leastDist && xu4_random(2) == 0)) {
                opponent = dynamic_cast<Creature*>(*i);
                leastDist = d;
            }
        }    
    }

    if (opponent)
        *dist = leastDist;

    return opponent;
}

void Creature::putToSleep() {
    if (getStatus() != STAT_DEAD) {
        addStatus(STAT_SLEEPING);
        setAnimated(false); /* freeze creature */
    }
}

void Creature::removeStatus(StatusType s) {
    StatusList::iterator i;
    for (i = status.begin(); i != status.end();) {
        if (*i == s)
            i = status.erase(i);
        else i++;
    }

    // Just to be sure, if a player is poisoned from a savegame, then they won't have
    // a STAT_GOOD in the stack yet.
    if (status.empty())
        addStatus(STAT_GOOD);
}

void Creature::setStatus(StatusType s) {
    status.clear();
    this->addStatus(s);
}

void Creature::wakeUp() {
    removeStatus(STAT_SLEEPING);
    setAnimated(); /* reanimate creature */
}

/**
 * Applies damage to the creature.
 * Returns true if the creature still exists after the damage has been applied
 * or false, if the creature was destroyed
 *
 * If byplayer is false (when a monster is killed by walking through
 * fire or poison, or as a result of jinx) we don't report experience
 * on death
 */
bool Creature::applyDamage(int damage, bool byplayer) {
    /* deal the damage */
    if (id != LORDBRITISH_ID)
        AdjustValueMin(hp, -damage, 0);    

    switch (getState()) {

    case MSTAT_DEAD:        
        if (byplayer)
            screenMessage("%c%s Killed!%c\nExp. %d\n", FG_RED, name.c_str(), FG_WHITE, xp);
        else
            screenMessage("%c%s Killed!%c\n", FG_RED, name.c_str(), FG_WHITE);        

        /*
         * the creature is dead; let it spawns something else on
         * death (e.g. a gazer that spawns insects like in u5)
         * then remove it
         */
        if (spawnsOnDeath())
            spawnOnDeath();

        // Remove yourself from the map
        remove();
        return false;        

    case MSTAT_FLEEING:
        screenMessage("%c%s Fleeing!%c\n", FG_YELLOW, name.c_str(), FG_WHITE);
        break;

    case MSTAT_CRITICAL:
        screenMessage("%s Critical!\n", name.c_str());
        break;

    case MSTAT_HEAVILYWOUNDED:
        screenMessage("%s Heavily Wounded!\n", name.c_str());
        break;

    case MSTAT_LIGHTLYWOUNDED:
        screenMessage("%s Lightly Wounded!\n", name.c_str());
        break;

    case MSTAT_BARELYWOUNDED:
        screenMessage("%s Barely Wounded!\n", name.c_str());
        break;
    }

    /* creature is still alive and has the chance to divide - xu4 enhancement */
    if (divides() && xu4_random(2) == 0)
        divide();

    return true;
}

bool Creature::dealDamage(Creature *m, int damage) {
    return m->applyDamage(damage, isPartyMember(this));
}

/**
 * CreatureMgr class implementation
 */
CreatureMgr *CreatureMgr::getInstance() {
    if (instance == NULL) {
        instance = new CreatureMgr();
        instance->loadAll();
    }
    return instance;
}

void CreatureMgr::loadAll() {
    const Config *config = Config::getInstance();
    Std::vector<ConfigElement> creatureConfs = config->getElement("creatures").getChildren();

    for (Std::vector<ConfigElement>::iterator i = creatureConfs.begin(); i != creatureConfs.end(); i++) {
        if (i->getName() != "creature")
            continue;

        Creature *m = new Creature(0);
        m->load(*i);

        /* add the creature to the list */
        creatures[m->getId()] = m;
    }
}

/**
 * Returns a creature using a tile to find which one to create
 * or NULL if a creature with that tile cannot be found
 */ 
Creature *CreatureMgr::getByTile(MapTile tile) {
    CreatureMap::const_iterator i;

    for (i = creatures.begin(); i != creatures.end(); i++) {
        if (i->_value->getTile() == tile)
            return i->_value;        
    }

//    if (tile.id)
//    	errorWarning("Did not find creature for tile %d", tile.id);
    return NULL;
}

/**
 * Returns the creature that has the corresponding id
 * or returns NULL if no creature with that id could
 * be found.
 */
Creature *CreatureMgr::getById(CreatureId id) {
    CreatureMap::const_iterator i = creatures.find(id);
    if (i != creatures.end())
        return i->_value;
    else return NULL;
}

/**
 * Returns the creature that has the corresponding name
 * or returns NULL if no creature can be found with
 * that name (case insensitive)
 */ 
Creature *CreatureMgr::getByName(Common::String name) {
    CreatureMap::const_iterator i;
    for (i = creatures.begin(); i != creatures.end(); i++) {
        if (scumm_stricmp(i->_value->getName().c_str(), name.c_str()) == 0)
            return i->_value;
    }
    return NULL;
}

/**
 * Creates a random creature based on the tile given
 */ 
Creature *CreatureMgr::randomForTile(const Tile *tile) {
    /* FIXME: this is too dependent on the tile system, and easily
       broken when tileset changes are made.  Methinks the logic 
       behind this should be moved to monsters.xml or another conf
       file */

    int era;
    TileId randTile;

    if (tile->isSailable()) {        
		randTile = creatures.find(PIRATE_ID)->_value->getTile().getId();
        randTile += xu4_random(7);
        return getByTile(randTile);        
    }
    else if (tile->isSwimable()) {
		randTile = creatures.find(NIXIE_ID)->_value->getTile().getId();
        randTile += xu4_random(5);
        return getByTile(randTile);
    }

    if (!tile->isCreatureWalkable())
        return NULL;

    //if (c->saveGame->moves > 100000) // FIXME: what's 100,000 moves all about (if anything)?
    if (c->_saveGame->moves > 30000)
        era = 0x0f;
    else if (c->_saveGame->moves > 20000)
        era = 0x07;
    else
        era = 0x03;

	randTile = creatures.find(ORC_ID)->_value->getTile().getId();
    randTile += era & xu4_random(0x10) & xu4_random(0x10);
    return getByTile(randTile);
}


/**
 * Creates a random creature based on the dungeon level given
 */
Creature *CreatureMgr::randomForDungeon(int dngLevel) {
    // Based on u4dos observations, see:
    //  https://sourceforge.net/p/xu4/patches/37/
    int adjustedDngLevel = dngLevel + 1;
    size_t range = adjustedDngLevel < 5 ? 3 : 4;
    CreatureId monster = STORM_ID + adjustedDngLevel + xu4_random(range);
	if(monster >= MIMIC_ID)
		++monster;

	return getById(monster);
}


/**
 * Creates a random ambushing creature
 */ 
Creature *CreatureMgr::randomAmbushing() {
    CreatureMap::const_iterator i;
    int numAmbushingCreatures = 0,
        randCreature;

    /* first, find out how many creatures exist that might ambush you */
    for (i = creatures.begin(); i != creatures.end(); i++) {
        if (i->_value->ambushes())
            numAmbushingCreatures++;
    }
    
    if (numAmbushingCreatures > 0) {
        /* now, randomely select one of them */
        randCreature = xu4_random(numAmbushingCreatures);
        numAmbushingCreatures = 0;

        /* now, find the one we selected */
        for (i = creatures.begin(); i != creatures.end(); i++) {
            if (i->_value->ambushes()) {
                /* found the creature - return it! */
                if (numAmbushingCreatures == randCreature)
                    return i->_value;
                /* move on to the next creature */
                else numAmbushingCreatures++;
            }
        }
    }

    ASSERT(0, "failed to find an ambushing creature");
    return NULL;
}

} // End of namespace Ultima4
} // End of namespace Ultima
