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

#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/gfx/textcolor.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/tileset.h"

namespace Ultima {
namespace Ultima4 {

CreatureMgr *CreatureMgr::_instance = nullptr;

bool isCreature(Object *punknown) {
	Creature *m;
	if ((m = dynamic_cast<Creature *>(punknown)) != nullptr)
		return true;
	else
		return false;
}

Creature::Creature(MapTile tile) : Object(Object::CREATURE),
		_id(0), _leader(0), _baseHp(0), _hp(0), _xp(0), _ranged(0),
		_leavesTile(false), _mAttr(MATTR_STEALFOOD),
		_movementAttr(MATTR_STATIONARY), _slowedType(SLOWED_BY_NOTHING),
		_encounterSize(0), _resists(0), _spawn(0) {
	const Creature *m = creatureMgr->getByTile(tile);
	if (m)
		*this = *m;
}

void Creature::load(const ConfigElement &conf) {
	uint idx;

	static const struct {
		const char *name;
		uint mask;
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
		uint mask;
	} steals[] = {
		{ "food", MATTR_STEALFOOD },
		{ "gold", MATTR_STEALGOLD }
	};

	/* casts="" */
	static const struct {
		const char *name;
		uint mask;
	} casts[] = {
		{ "sleep", MATTR_CASTS_SLEEP },
		{ "negate", MATTR_NEGATE }
	};

	/* movement="" */
	static const struct {
		const char *name;
		uint mask;
	} movement[] = {
		{ "none", MATTR_STATIONARY },
		{ "wanders", MATTR_WANDERS }
	};

	/* boolean attributes that affect movement */
	static const struct {
		const char *name;
		uint mask;
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

	_name = conf.getString("name");
	_id = static_cast<unsigned short>(conf.getInt("id"));

	/* Get the leader if it's been included, otherwise the leader is itself */
	_leader = static_cast<byte>(conf.getInt("leader", _id));

	_xp = static_cast<unsigned short>(conf.getInt("exp"));
	_ranged = conf.getBool("ranged");
	setTile(g_tileSets->findTileByName(conf.getString("tile")));

	setHitTile("hit_flash");
	setMissTile("miss_flash");

	_mAttr = static_cast<CreatureAttrib>(0);
	_movementAttr = static_cast<CreatureMovementAttrib>(0);
	_resists = 0;

	/* get the encounter size */
	_encounterSize = conf.getInt("encounterSize", 0);

	/* get the base hp */
	_baseHp = conf.getInt("basehp", 0);
	/* adjust basehp according to battle difficulty setting */
	if (settings._battleDiff == "Hard")
		_baseHp *= 2;
	if (settings._battleDiff == "Expert")
		_baseHp *= 4;

	/* get the camouflaged tile */
	if (conf.exists("camouflageTile"))
		_camouflageTile = conf.getString("camouflageTile");

	/* get the ranged tile for world map attacks */
	if (conf.exists("worldrangedtile"))
		_worldRangedTile = conf.getString("worldrangedtile");

	/* get ranged hit tile */
	if (conf.exists("rangedhittile")) {
		if (conf.getString("rangedhittile") == "random")
			_mAttr = static_cast<CreatureAttrib>(_mAttr | MATTR_RANDOMRANGED);
		else
			setHitTile(conf.getString("rangedhittile"));
	}

	/* get ranged miss tile */
	if (conf.exists("rangedmisstile")) {
		if (conf.getString("rangedmisstile") ==  "random")
			_mAttr = static_cast<CreatureAttrib>(_mAttr | MATTR_RANDOMRANGED);
		else
			setMissTile(conf.getString("rangedmisstile"));
	}

	/* find out if the creature leaves a tile behind on ranged attacks */
	_leavesTile = conf.getBool("leavestile");

	/* get effects that this creature is immune to */
	for (idx = 0; idx < sizeof(effects) / sizeof(effects[0]); idx++) {
		if (conf.getString("resists") == effects[idx].name) {
			_resists = effects[idx].effect;
		}
	}

	/* Load creature attributes */
	for (idx = 0; idx < sizeof(booleanAttributes) / sizeof(booleanAttributes[0]); idx++) {
		if (conf.getBool(booleanAttributes[idx].name)) {
			_mAttr = static_cast<CreatureAttrib>(_mAttr | booleanAttributes[idx].mask);
		}
	}

	/* Load boolean attributes that affect movement */
	for (idx = 0; idx < sizeof(movementBoolean) / sizeof(movementBoolean[0]); idx++) {
		if (conf.getBool(movementBoolean[idx].name)) {
			_movementAttr = static_cast<CreatureMovementAttrib>(_movementAttr | movementBoolean[idx].mask);
		}
	}

	/* steals="" */
	for (idx = 0; idx < sizeof(steals) / sizeof(steals[0]); idx++) {
		if (conf.getString("steals") == steals[idx].name) {
			_mAttr = static_cast<CreatureAttrib>(_mAttr | steals[idx].mask);
		}
	}

	/* casts="" */
	for (idx = 0; idx < sizeof(casts) / sizeof(casts[0]); idx++) {
		if (conf.getString("casts") == casts[idx].name) {
			_mAttr = static_cast<CreatureAttrib>(_mAttr | casts[idx].mask);
		}
	}

	/* movement="" */
	for (idx = 0; idx < sizeof(movement) / sizeof(movement[0]); idx++) {
		if (conf.getString("movement") == movement[idx].name) {
			_movementAttr = static_cast<CreatureMovementAttrib>(_movementAttr | movement[idx].mask);
		}
	}

	if (conf.exists("spawnsOnDeath")) {
		_mAttr = static_cast<CreatureAttrib>(_mAttr | MATTR_SPAWNSONDEATH);
		_spawn = static_cast<byte>(conf.getInt("spawnsOnDeath"));
	}

	/* Figure out which 'slowed' function to use */
	_slowedType = SLOWED_BY_TILE;
	if (sails())
		/* sailing creatures (pirate ships) */
		_slowedType = SLOWED_BY_WIND;
	else if (flies() || isIncorporeal())
		/* flying creatures (dragons, bats, etc.) and incorporeal creatures (ghosts, zorns) */
		_slowedType = SLOWED_BY_NOTHING;
}

bool Creature::isAttackable() const {
	if (_mAttr & MATTR_NONATTACKABLE)
		return false;
	/* can't attack horse transport */
	if (_tile.getTileType()->isHorse() && getMovementBehavior() == MOVEMENT_FIXED)
		return false;
	return true;
}

int  Creature::getDamage() const {
	int damage, val, x;
	val = _baseHp;
	x = xu4_random(val >> 2);
	damage = (x >> 4) + ((x >> 2) & 0xfc);
	damage += x % 10;
	return damage;
}

int Creature::setInitialHp(int points) {
	if (points < 0)
		_hp = xu4_random(_baseHp) | (_baseHp / 2);
	else
		_hp = points;

	/* make sure the creature doesn't flee initially */
	if (_hp < 24) _hp = 24;

	return _hp;
}

void Creature::setRandomRanged() {
	switch (xu4_random(4)) {
	case 0:
		_rangedHitTile = _rangedMissTile = "poison_field";
		break;
	case 1:
		_rangedHitTile = _rangedMissTile = "energy_field";
		break;
	case 2:
		_rangedHitTile = _rangedMissTile = "fire_field";
		break;
	case 3:
		_rangedHitTile = _rangedMissTile = "sleep_field";
		break;
	}
}

CreatureStatus Creature::getState() const {
	int heavy_threshold, light_threshold, crit_threshold;

	crit_threshold = _baseHp >> 2;  /* (basehp / 4) */
	heavy_threshold = _baseHp >> 1; /* (basehp / 2) */
	light_threshold = crit_threshold + heavy_threshold;

	if (_hp <= 0)
		return MSTAT_DEAD;
	else if (_hp < 24)
		return MSTAT_FLEEING;
	else if (_hp < crit_threshold)
		return MSTAT_CRITICAL;
	else if (_hp < heavy_threshold)
		return MSTAT_HEAVILYWOUNDED;
	else if (_hp < light_threshold)
		return MSTAT_LIGHTLYWOUNDED;
	else
		return MSTAT_BARELYWOUNDED;

}

bool Creature::specialAction() {
	bool retval = false;

	int dx = abs(g_context->_location->_coords.x - _coords.x);
	int dy = abs(g_context->_location->_coords.y - _coords.y);
	int mapdist = g_context->_location->_coords.distance(_coords, g_context->_location->_map);

	/* find out which direction the avatar is in relation to the creature */
	MapCoords mapcoords(_coords);
	int dir = mapcoords.getRelativeDirection(g_context->_location->_coords, g_context->_location->_map);

	//Init outside of switch
	int broadsidesDirs = 0;

	switch (_id) {
	case LAVA_LIZARD_ID:
	case SEA_SERPENT_ID:
	case HYDRA_ID:
	case DRAGON_ID:

		/* A 50/50 chance they try to range attack when you're close enough
		   and not in a city
		   Note: Monsters in settlements in U3 do fire on party
		*/
		if (mapdist <= 3 && xu4_random(2) == 0 && (g_context->_location->_context & CTX_CITY) == 0) {
			Std::vector<Coords> path = gameGetDirectionalActionPath(dir, MASK_DIR_ALL, _coords,
			                           1, 3, nullptr, false);
			for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
				if (creatureRangeAttack(*i, this))
					break;
			}
		}

		break;

	case PIRATE_ID:

		/* Fire cannon: Pirates only fire broadsides and only when they can hit you :) */
		retval = true;
		broadsidesDirs = dirGetBroadsidesDirs(_tile.getDirection());

		if ((((dx == 0) && (dy <= 3)) ||          /* avatar is close enough and on the same column, OR */
		        ((dy == 0) && (dx <= 3))) &&         /* avatar is close enough and on the same row, AND */
		        ((broadsidesDirs & dir) > 0)) { /* pirate ship is firing broadsides */

			// nothing (not even mountains!) can block cannonballs
			Std::vector<Coords> path = gameGetDirectionalActionPath(dir, broadsidesDirs, _coords,
			                           1, 3, nullptr, false);
			for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
				if (fireAt(*i, false))
					break;
			}
		} else
			retval = false;

		break;

	default:
		break;
	}

	return retval;
}

bool Creature::specialEffect() {
	Object *obj;
	bool retval = false;

	switch (_id) {
	case STORM_ID: {
		ObjectDeque::iterator i;

		if (_coords == g_context->_location->_coords) {

			/* damage the ship */
			if (g_context->_transportContext == TRANSPORT_SHIP) {
				/* FIXME: Check actual damage from u4dos */
				gameDamageShip(10, 30);
			}
			/* anything else but balloon damages the party */
			else if (g_context->_transportContext != TRANSPORT_BALLOON) {
				/* FIXME: formula for twister damage is guesstimated from u4dos */
				gameDamageParty(0, 75);
			}
			return true;
		}

		/* See if the storm is on top of any objects and destroy them! */
		for (i = g_context->_location->_map->_objects.begin();
		        i != g_context->_location->_map->_objects.end();) {

			obj = *i;
			if (this != obj &&
			        obj->getCoords() == _coords) {
				/* Converged with an object, destroy the object! */
				i = g_context->_location->_map->removeObject(i);
				retval = true;
			} else i++;
		}
	}
	break;

	case WHIRLPOOL_ID: {
		ObjectDeque::iterator i;

		if (_coords == g_context->_location->_coords && (g_context->_transportContext == TRANSPORT_SHIP)) {

			/* Deal 10 damage to the ship */
			gameDamageShip(-1, 10);

			/* Send the party to Locke Lake */
			g_context->_location->_coords = g_context->_location->_map->getLabel("lockelake");

			/* Teleport the whirlpool that sent you there far away from lockelake */
			this->setCoords(Coords(0, 0, 0));
			retval = true;
			break;
		}

		/* See if the whirlpool is on top of any objects and destroy them! */
		for (i = g_context->_location->_map->_objects.begin();
		        i != g_context->_location->_map->_objects.end();) {

			obj = *i;

			if (this != obj &&
			        obj->getCoords() == _coords) {

				Creature *m = dynamic_cast<Creature *>(obj);

				/* Make sure the object isn't a flying creature or object */
				if (!m || (m && (m->swims() || m->sails()) && !m->flies())) {
					/* Destroy the object it met with */
					i = g_context->_location->_map->removeObject(i);
					retval = true;
				} else {
					i++;
				}
			} else i++;
		}
	}

	default:
		break;
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
		g_context->_aura->set(Aura::NEGATE, 2);

	/*
	 * figure out what to do
	 */

	// creatures who teleport do so 1/8 of the time
	if (teleports() && xu4_random(8) == 0)
		action = CA_TELEPORT;
	// creatures who ranged attack do so 1/4 of the time.  Make sure
	// their ranged attack is not negated!
	else if (_ranged != 0 && xu4_random(4) == 0 &&
	         (_rangedHitTile != "magic_flash" || (*g_context->_aura != Aura::NEGATE)))
		action = CA_RANGED;
	// creatures who cast sleep do so 1/4 of the time they don't ranged attack
	else if (castsSleep() && (*g_context->_aura != Aura::NEGATE) && (xu4_random(4) == 0))
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
	if (target == nullptr)
		return;

	if (action == CA_ATTACK && dist > 1)
		action = CA_ADVANCE;

	/* let's see if the creature blends into the background, or if he appears... */
	if (camouflages() && !hideOrShow())
		return; /* creature is hidden -- no action! */

	switch (action) {
	case CA_ATTACK:
		soundPlay(SOUND_NPC_ATTACK, false);                                    // NPC_ATTACK, melee

		if (controller->attackHit(this, target)) {
			soundPlay(SOUND_PC_STRUCK, false);                                 // PC_STRUCK, melee and ranged
			GameController::flashTile(target->getCoords(), "hit_flash", 4);


			if (!dealDamage(target, getDamage()))
				target = nullptr;

			if (target && isPartyMember(target)) {
				/* steal gold if the creature steals gold */
				if (stealsGold() && xu4_random(4) == 0) {
					soundPlay(SOUND_ITEM_STOLEN, false);                       // ITEM_STOLEN, gold
					g_context->_party->adjustGold(-(xu4_random(0x3f)));
				}

				/* steal food if the creature steals food */
				if (stealsFood()) {
					soundPlay(SOUND_ITEM_STOLEN, false);                       // ITEM_STOLEN, food
					g_context->_party->adjustFood(-2500);
				}
			}
		} else {
			GameController::flashTile(target->getCoords(), "miss_flash", 1);
		}
		break;

	case CA_CAST_SLEEP: {
		g_screen->screenMessage("\nSleep!\n");

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
			new_c = Coords(xu4_random(map->_width), xu4_random(map->_height), g_context->_location->_coords.z);

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
				g_screen->screenMessage("\n%c%s Flees!%c\n", FG_YELLOW, _name.c_str(), FG_WHITE);

				/* Congrats, you have a heart! */
				if (isGood())
					g_context->_party->adjustKarma(KA_SPARED_GOOD);

				map->removeObject(this);
			}
		}
		break;
	}
	}
	this->animateMovement();
}

void Creature::addStatus(StatusType s) {
	if (_status.size() && _status.back() > s) {
		StatusType prev = _status.back();
		_status.pop_back();
		_status.push_back(s);
		_status.push_back(prev);
	} else _status.push_back(s);
}

void Creature::applyTileEffect(TileEffect effect) {
	if (effect != EFFECT_NONE) {
		gameUpdateScreen();

		switch (effect) {
		case EFFECT_SLEEP:
			/* creature fell asleep! */
			if ((_resists != EFFECT_SLEEP) &&
			        (xu4_random(0xFF) >= _hp))
				putToSleep();
			break;

		case EFFECT_LAVA:
		case EFFECT_FIRE:
			/* deal 0 - 127 damage to the creature if it is not immune to fire damage */
			if ((_resists != EFFECT_FIRE) && (_resists != EFFECT_LAVA))
				applyDamage(xu4_random(0x7F), false);
			break;

		case EFFECT_POISONFIELD:
			/* deal 0 - 127 damage to the creature if it is not immune to poison field damage */
			if (_resists != EFFECT_POISONFIELD)
				applyDamage(xu4_random(0x7F), false);
			break;

		case EFFECT_POISON:
		default:
			break;
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
	if (!settings._enhancements || !settings._enhancementsOptions._slimeDivides)
		return false;

	/* make sure there's a place to put the divided creature! */
	if (d != DIR_NONE) {
		MapCoords coords(getCoords());

		g_screen->screenMessage("%s Divides!\n", _name.c_str());

		/* find a spot to put our new creature */
		coords.move(d, map);

		/* create our new creature! */
		Creature *addedCreature = map->addCreature(this, coords);
		int dividedHp = (this->_hp + 1) / 2;
		addedCreature->_hp = dividedHp;
		this->_hp = dividedHp;
		return true;
	}
	return false;
}

bool Creature::spawnOnDeath() {
	Map *map = getMap();

	/* this is a game enhancement, make sure it's turned on! */
	if (!settings._enhancements || !settings._enhancementsOptions._gazerSpawnsInsects)
		return false;

	/* make sure there's a place to put the divided creature! */
	MapCoords coords(getCoords());

	/* create our new creature! */
	map->addCreature(creatureMgr->getById(_spawn), coords);
	return true;
}

StatusType Creature::getStatus() const {
	return _status.back();
}

bool Creature::isAsleep() const {
	for (StatusList::const_iterator itr = this->_status.begin();
	        itr != this->_status.end();
	        ++itr)
		if (*itr == STAT_SLEEPING)
			return true;
	return false;
}

bool Creature::hideOrShow() {
	/* find the nearest opponent */
	int dist;

	/* ok, now we've got the nearest party member.  Now, see if they're close enough */
	if (nearestOpponent(&dist, false) != nullptr) {
		if ((dist < 5) && !isVisible())
			setVisible(); /* show yourself */
		else if (dist >= 5)
			setVisible(false); /* hide and take no action! */
	}

	return isVisible();
}

Creature *Creature::nearestOpponent(int *dist, bool ranged) {
	Creature *opponent = nullptr;
	int d, leastDist = 0xFFFF;
	ObjectDeque::iterator i;
	bool jinx = (*g_context->_aura == Aura::JINX);
	Map *map = getMap();

	for (i = map->_objects.begin(); i != map->_objects.end(); ++i) {
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
				opponent = dynamic_cast<Creature *>(*i);
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
	for (i = _status.begin(); i != _status.end();) {
		if (*i == s)
			i = _status.erase(i);
		else
			i++;
	}

	// Just to be sure, if a player is poisoned from a savegame, then they won't have
	// a STAT_GOOD in the stack yet.
	if (_status.empty())
		addStatus(STAT_GOOD);
}

void Creature::setStatus(StatusType s) {
	_status.clear();
	this->addStatus(s);
}

void Creature::wakeUp() {
	removeStatus(STAT_SLEEPING);
	setAnimated(); /* reanimate creature */
}

bool Creature::applyDamage(int damage, bool byplayer) {
	/* deal the damage */
	if (_id != LORDBRITISH_ID)
		AdjustValueMin(_hp, -damage, 0);

	switch (getState()) {
	case MSTAT_DEAD:
		if (byplayer)
			g_screen->screenMessage("%c%s Killed!%c\nExp. %d\n", FG_RED, _name.c_str(), FG_WHITE, _xp);
		else
			g_screen->screenMessage("%c%s Killed!%c\n", FG_RED, _name.c_str(), FG_WHITE);

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
		g_screen->screenMessage("%c%s Fleeing!%c\n", FG_YELLOW, _name.c_str(), FG_WHITE);
		break;

	case MSTAT_CRITICAL:
		g_screen->screenMessage("%s Critical!\n", _name.c_str());
		break;

	case MSTAT_HEAVILYWOUNDED:
		g_screen->screenMessage("%s Heavily Wounded!\n", _name.c_str());
		break;

	case MSTAT_LIGHTLYWOUNDED:
		g_screen->screenMessage("%s Lightly Wounded!\n", _name.c_str());
		break;

	case MSTAT_BARELYWOUNDED:
		g_screen->screenMessage("%s Barely Wounded!\n", _name.c_str());
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
	if (_instance == nullptr) {
		_instance = new CreatureMgr();
		_instance->loadAll();
	}
	return _instance;
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
		_creatures[m->getId()] = m;
	}
}

Creature *CreatureMgr::getByTile(MapTile tile) {
	CreatureMap::const_iterator i;

	for (i = _creatures.begin(); i != _creatures.end(); i++) {
		if (i->_value->getTile() == tile)
			return i->_value;
	}

//    if (tile.id)
//      warning("Did not find creature for tile %d", tile.id);
	return nullptr;
}

Creature *CreatureMgr::getById(CreatureId id) {
	CreatureMap::const_iterator i = _creatures.find(id);
	if (i != _creatures.end())
		return i->_value;
	else
		return nullptr;
}

Creature *CreatureMgr::getByName(Common::String name) {
	CreatureMap::const_iterator i;
	for (i = _creatures.begin(); i != _creatures.end(); i++) {
		if (scumm_stricmp(i->_value->getName().c_str(), name.c_str()) == 0)
			return i->_value;
	}
	return nullptr;
}

Creature *CreatureMgr::randomForTile(const Tile *tile) {
	/* FIXME: this is too dependent on the tile system, and easily
	   broken when tileset changes are made.  Methinks the logic
	   behind this should be moved to monsters.xml or another conf
	   file */

	int era;
	TileId randTile;

	if (tile->isSailable()) {
		randTile = _creatures.find(PIRATE_ID)->_value->getTile().getId();
		randTile += xu4_random(7);
		return getByTile(randTile);
	} else if (tile->isSwimable()) {
		randTile = _creatures.find(NIXIE_ID)->_value->getTile().getId();
		randTile += xu4_random(5);
		return getByTile(randTile);
	}

	if (!tile->isCreatureWalkable())
		return nullptr;

	//if (c->saveGame->_moves > 100000) // FIXME: what's 100,000 moves all about (if anything)?
	if (g_ultima->_saveGame->_moves > 30000)
		era = 0x0f;
	else if (g_ultima->_saveGame->_moves > 20000)
		era = 0x07;
	else
		era = 0x03;

	randTile = _creatures.find(ORC_ID)->_value->getTile().getId();
	randTile += era & xu4_random(0x10) & xu4_random(0x10);
	return getByTile(randTile);
}

Creature *CreatureMgr::randomForDungeon(int dngLevel) {
	int adjustedDngLevel = dngLevel + 1;
	size_t range = adjustedDngLevel < 5 ? 3 : 4;
	CreatureId monster = STORM_ID + adjustedDngLevel + xu4_random(range);
	if (monster >= MIMIC_ID)
		++monster;

	return getById(monster);
}

Creature *CreatureMgr::randomAmbushing() {
	CreatureMap::const_iterator i;
	int numAmbushingCreatures = 0,
	    randCreature;

	/* first, find out how many creatures exist that might ambush you */
	for (i = _creatures.begin(); i != _creatures.end(); i++) {
		if (i->_value->ambushes())
			numAmbushingCreatures++;
	}

	if (numAmbushingCreatures > 0) {
		/* now, randomely select one of them */
		randCreature = xu4_random(numAmbushingCreatures);
		numAmbushingCreatures = 0;

		/* now, find the one we selected */
		for (i = _creatures.begin(); i != _creatures.end(); i++) {
			if (i->_value->ambushes()) {
				/* found the creature - return it! */
				if (numAmbushingCreatures == randCreature)
					return i->_value;
				/* move on to the next creature */
				else
					numAmbushingCreatures++;
			}
		}
	}

	error("failed to find an ambushing creature");
	return nullptr;
}

} // End of namespace Ultima4
} // End of namespace Ultima
