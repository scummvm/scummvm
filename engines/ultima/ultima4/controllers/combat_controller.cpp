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

#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/controllers/read_choice_controller.h"
#include "ultima/ultima4/controllers/read_dir_controller.h"
#include "ultima/ultima4/controllers/ztats_controller.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/movement.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/game/death.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/spell.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima4/ultima4.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

CombatController *g_combat;

/**
 * Returns true if 'map' points to a Combat Map
 */
bool isCombatMap(Map *punknown) {
	CombatMap *ps;
	if ((ps = dynamic_cast<CombatMap *>(punknown)) != nullptr)
		return true;
	else
		return false;
}

/**
 * Returns a CombatMap pointer to the map
 * passed, or a CombatMap pointer to the current map
 * if no arguments were passed.
 *
 * Returns nullptr if the map provided (or current map)
 * is not a combat map.
 */
CombatMap *getCombatMap(Map *punknown) {
	Map *m = punknown ? punknown : g_context->_location->_map;
	if (!isCombatMap(m))
		return nullptr;
	else
		return dynamic_cast<CombatMap *>(m);
}

/**
 * CombatController class implementation
 */
CombatController::CombatController() : _map(nullptr) {
	init();

	g_context->_party->addObserver(this);
}

CombatController::CombatController(CombatMap *m) : _map(m) {
	init();

	g_game->setMap(_map, true, nullptr, this);
	g_context->_party->addObserver(this);
}

CombatController::CombatController(MapId id) : _map(nullptr) {
	init();

	_map = getCombatMap(mapMgr->get(id));
	g_game->setMap(_map, true, nullptr, this);
	g_context->_party->addObserver(this);
	_forceStandardEncounterSize = false;
}

CombatController::~CombatController() {
	g_context->_party->deleteObserver(this);
	g_combat = nullptr;
}

void CombatController::init() {
	g_combat = this;

	_focus = 0;
	Common::fill(&_creatureTable[0], &_creatureTable[AREA_CREATURES],
		(const Creature *)nullptr);
	_creature = nullptr;

	_camping = false;
	_forceStandardEncounterSize = false;
	_placePartyOnMap = false;
	_placeCreaturesOnMap = false;
	_winOrLose = false;
	_showMessage = false;
	_exitDir = DIR_NONE;
}

void CombatController::setActive() {
	MetaEngine::setKeybindingMode(KBMODE_COMBAT);
}

// Accessor Methods
bool CombatController::isCamping() const {
	return _camping;
}
bool CombatController::isWinOrLose() const {
	return _winOrLose;
}
Direction CombatController::getExitDir() const {
	return _exitDir;
}
byte CombatController::getFocus() const {
	return _focus;
}
CombatMap *CombatController::getMap() const {
	return _map;
}
Creature *CombatController::getCreature() const {
	return _creature;
}
PartyMemberVector *CombatController::getParty() {
	return &_party;
}
PartyMember *CombatController::getCurrentPlayer() {
	return _party[_focus];
}

void CombatController::setExitDir(Direction d) {
	_exitDir = d;
}
void CombatController::setCreature(Creature *m) {
	_creature = m;
}
void CombatController::setWinOrLose(bool worl) {
	_winOrLose = worl;
}
void CombatController::showCombatMessage(bool show) {
	_showMessage = show;
}

void CombatController::init(class Creature *m) {
	int i;

	_creature = m;
	_placeCreaturesOnMap = (m == nullptr) ? false : true;
	_placePartyOnMap = true;
	_winOrLose = true;
	_map->setDungeonRoom(false);
	_map->setAltarRoom(VIRT_NONE);
	_showMessage = true;
	_camping = false;

	/* initialize creature info */
	for (i = 0; i < AREA_CREATURES; i++) {
		_creatureTable[i] = nullptr;
	}

	for (i = 0; i < AREA_PLAYERS; i++) {
		_party.push_back(nullptr);
	}

	/* fill the creature table if a creature was provided to create */
	fillCreatureTable(m);

	/* initialize focus */
	_focus = 0;
}

void CombatController::initDungeonRoom(int room, Direction from) {
	int i;
	init(nullptr);

	assertMsg(g_context->_location->_prev->_context & CTX_DUNGEON, "Error: called initDungeonRoom from non-dungeon context");
	{
		Dungeon *dng = dynamic_cast<Dungeon *>(g_context->_location->_prev->_map);
		assert(dng);  

		DngRoom &dngRoom = dng->_rooms[room];

		/* load the dungeon room properties */
		_winOrLose = false;
		_map->setDungeonRoom(true);
		_exitDir = DIR_NONE;

		/* FIXME: this probably isn't right way to see if you're entering an altar room... but maybe it is */
		if ((g_context->_location->_prev->_map->_id != MAP_ABYSS) && (room == 0xF)) {
			/* figure out which dungeon room they're entering */
			if (g_context->_location->_prev->_coords.x == 3)
				_map->setAltarRoom(VIRT_LOVE);
			else if (g_context->_location->_prev->_coords.x <= 2)
				_map->setAltarRoom(VIRT_TRUTH);
			else
				_map->setAltarRoom(VIRT_COURAGE);
		}

		/* load in creatures and creature start coordinates */
		for (i = 0; i < AREA_CREATURES; i++) {
			if (dng->_rooms[room]._creatureTiles[i] > 0) {
				_placeCreaturesOnMap = true;
				_creatureTable[i] = creatureMgr->getByTile(dng->_rooms[room]._creatureTiles[i]);
			}
			_map->creature_start[i].x = dng->_rooms[room]._creatureStart[i].x;
			_map->creature_start[i].y = dng->_rooms[room]._creatureStart[i].y;
		}

		// Validate direction
		switch (from) {
		case DIR_WEST:
		case DIR_NORTH:
		case DIR_EAST:
		case DIR_SOUTH:
			break;
		case DIR_ADVANCE:
		case DIR_RETREAT:
		default:
			error("Invalid 'from' direction passed to initDungeonRoom()");
		}

		for (i = 0; i < AREA_PLAYERS; i++) {
			_map->player_start[i].x = dngRoom._partyStart[i][from].x;
			_map->player_start[i].y = dngRoom._partyStart[i][from].y;
		}
	}
}

void CombatController::applyCreatureTileEffects() {
	CreatureVector creatures = _map->getCreatures();
	CreatureVector::iterator i;

	for (i = creatures.begin(); i != creatures.end(); i++) {
		Creature *m = *i;
		TileEffect effect = _map->tileTypeAt(m->getCoords(), WITH_GROUND_OBJECTS)->getEffect();
		m->applyTileEffect(effect);
	}
}

void CombatController::begin() {
	bool partyIsReadyToFight = false;

	/* place party members on the map */
	if (_placePartyOnMap)
		placePartyMembers();

	/* place creatures on the map */
	if (_placeCreaturesOnMap)
		placeCreatures();

	/* if we entered an altar room, show the name */
	if (_map->isAltarRoom()) {
		g_screen->screenMessage("\nThe Altar Room of %s\n", getBaseVirtueName(_map->getAltarRoom()));
		g_context->_location->_context = static_cast<LocationContext>(g_context->_location->_context | CTX_ALTAR_ROOM);
	}

	/* if there are creatures around, start combat! */
	if (_showMessage && _placeCreaturesOnMap && _winOrLose)
		g_screen->screenMessage("\n%c****%c COMBAT %c****%c\n", FG_GREY, FG_WHITE, FG_GREY, FG_WHITE);

	/* FIXME: there should be a better way to accomplish this */
	if (!_camping) {
		g_music->playMapMusic();
	}

	/* Set focus to the first active party member, if there is one */
	for (int i = 0; i < AREA_PLAYERS; i++) {
		if (setActivePlayer(i)) {
			partyIsReadyToFight = true;
			break;
		}
	}

	if (!_camping && !partyIsReadyToFight)
		g_context->_location->_turnCompleter->finishTurn();

	eventHandler->pushController(this);
}

void CombatController::end(bool adjustKarma) {
	eventHandler->popController();

	/* The party is dead -- start the death sequence */
	if (g_context->_party->isDead()) {
		/* remove the creature */
		if (_creature)
			g_context->_location->_map->removeObject(_creature);

		g_death->start(5);
	}

	else {

		/* need to get this here because when we exit to the parent map, all the monsters are cleared */
		bool won = isWon();

		g_game->exitToParentMap();
		g_music->playMapMusic();

		if (_winOrLose) {
			if (won) {
				if (_creature) {
					if (_creature->isEvil())
						g_context->_party->adjustKarma(KA_KILLED_EVIL);
					awardLoot();
				}

				g_screen->screenMessage("\nVictory!\n\n");
			} else if (!g_context->_party->isDead()) {
				/* minus points for fleeing from evil creatures */
				if (adjustKarma && _creature && _creature->isEvil()) {
					g_screen->screenMessage("\nBattle is lost!\n\n");
					g_context->_party->adjustKarma(KA_FLED_EVIL);
				} else if (adjustKarma && _creature && _creature->isGood())
					g_context->_party->adjustKarma(KA_FLED_GOOD);
			}
		}

		/* exiting a dungeon room */
		if (_map->isDungeonRoom()) {
			g_screen->screenMessage("Leave Room!\n");
			if (_map->isAltarRoom()) {
				PortalTriggerAction action = ACTION_NONE;

				/* when exiting altar rooms, you exit to other dungeons.  Here it goes... */
				switch (_exitDir) {
				case DIR_NORTH:
					action = ACTION_EXIT_NORTH;
					break;
				case DIR_EAST:
					action = ACTION_EXIT_EAST;
					break;
				case DIR_SOUTH:
					action = ACTION_EXIT_SOUTH;
					break;
				case DIR_WEST:
					action = ACTION_EXIT_WEST;
					break;
				case DIR_NONE:
					break;
				case DIR_ADVANCE:
				case DIR_RETREAT:
				default:
					error("Invalid exit dir %d", _exitDir);
					break;
				}

				if (action != ACTION_NONE)
					usePortalAt(g_context->_location, g_context->_location->_coords, action);
			} else {
				g_screen->screenMessage("\n");
			}

			if (_exitDir != DIR_NONE) {
				g_ultima->_saveGame->_orientation = _exitDir;  /* face the direction exiting the room */
				// XXX: why north, shouldn't this be orientation?
				g_context->_location->move(DIR_NORTH, false);  /* advance 1 space outside of the room */
			}
		}

		/* remove the creature */
		if (_creature)
			g_context->_location->_map->removeObject(_creature);

		/* Make sure finishturn only happens if a new combat has not begun */
		if (! eventHandler->getController()->isCombatController())
			g_context->_location->_turnCompleter->finishTurn();
	}

	delete this;
}

void CombatController::fillCreatureTable(const Creature *creature) {
	int i, j;

	if (creature != nullptr) {
		const Creature *baseCreature = creature, *current;
		int numCreatures = initialNumberOfCreatures(creature);

		if (baseCreature->getId() == PIRATE_ID)
			baseCreature = creatureMgr->getById(ROGUE_ID);

		for (i = 0; i < numCreatures; i++) {
			current = baseCreature;

			/* find a free spot in the creature table */
			do {
				j = xu4_random(AREA_CREATURES) ;
			} while (_creatureTable[j] != nullptr);

			/* see if creature is a leader or leader's leader */
			if (creatureMgr->getById(baseCreature->getLeader()) != baseCreature && /* leader is a different creature */
			        i != (numCreatures - 1)) { /* must have at least 1 creature of type encountered */

				if (xu4_random(32) == 0)       /* leader's leader */
					current = creatureMgr->getById(creatureMgr->getById(baseCreature->getLeader())->getLeader());
				else if (xu4_random(8) == 0)   /* leader */
					current = creatureMgr->getById(baseCreature->getLeader());
			}

			/* place this creature in the creature table */
			_creatureTable[j] = current;
		}
	}
}

int  CombatController::initialNumberOfCreatures(const Creature *creature) const {
	int ncreatures;
	Map *map = g_context->_location->_prev ? g_context->_location->_prev->_map : g_context->_location->_map;

	/* if in an unusual combat situation, generally we stick to normal encounter sizes,
	   (such as encounters from sleeping in an inn, etc.) */
	if (_forceStandardEncounterSize || map->isWorldMap() || (g_context->_location->_prev && g_context->_location->_prev->_context & CTX_DUNGEON)) {
		ncreatures = xu4_random(8) + 1;

		if (ncreatures == 1) {
			if (creature && creature->getEncounterSize() > 0)
				ncreatures = xu4_random(creature->getEncounterSize()) + creature->getEncounterSize() + 1;
			else
				ncreatures = 8;
		}

		while (ncreatures > 2 * g_ultima->_saveGame->_members) {
			ncreatures = xu4_random(16) + 1;
		}
	} else {
		if (creature && creature->getId() == GUARD_ID)
			ncreatures = g_ultima->_saveGame->_members * 2;
		else
			ncreatures = 1;
	}

	return ncreatures;
}

bool CombatController::isWon() const {
	CreatureVector creatures = _map->getCreatures();
	if (creatures.size())
		return false;
	return true;
}

bool CombatController::isLost() const {
	PartyMemberVector party = _map->getPartyMembers();
	if (party.size())
		return false;
	return true;
}

void CombatController::moveCreatures() {
	Creature *m;
	CreatureVector creatures = _map->getCreatures();

	// IMPORTANT: We need to keep regenerating the creatures list,
	// because monsters may be removed if a jinxed monster kills another
	for (int i = 0; i < (int)creatures.size(); ++i) {
		m = creatures[i];
		m->act(this);

		creatures = _map->getCreatures();
		if (i < (int)creatures.size() && creatures[i] != m) {
			// Don't skip a later creature when an earlier one flees
			--i;
		}
	}
}

void CombatController::placeCreatures() {
	int i;

	for (i = 0; i < AREA_CREATURES; i++) {
		const Creature *m = _creatureTable[i];
		if (m)
			_map->addCreature(m, _map->creature_start[i]);
	}
}

void CombatController::placePartyMembers() {
	int i;
//  The following line caused a crash upon entering combat (MSVC8 binary)
//    party.clear();

	for (i = 0; i < g_context->_party->size(); i++) {
		PartyMember *p = g_context->_party->member(i);
		p->setFocus(false); // take the focus off of everyone

		/* don't place dead party members */
		if (p->getStatus() != STAT_DEAD) {
			/* add the party member to the map */
			p->setCoords(_map->player_start[i]);
			p->setMap(_map);
			_map->_objects.push_back(p);
			_party[i] = p;
		}
	}
}

bool CombatController::setActivePlayer(int player) {
	PartyMember *p = _party[player];

	if (p && !p->isDisabled()) {
		if (_party[_focus])
			_party[_focus]->setFocus(false);

		p->setFocus();
		_focus = player;

		g_screen->screenMessage("\n%s with %s\n\020", p->getName().c_str(), p->getWeapon()->getName().c_str());
		g_context->_stats->highlightPlayer(_focus);
		return true;
	}

	return false;
}

void CombatController::awardLoot() {
	Coords coords = _creature->getCoords();
	const Tile *ground = g_context->_location->_map->tileTypeAt(coords, WITHOUT_OBJECTS);

	/* add a chest, if the creature leaves one */
	if (_creature->leavesChest() &&
	        ground->isCreatureWalkable() &&
	        (!(g_context->_location->_context & CTX_DUNGEON) || ground->isDungeonFloor())) {
		MapTile chest = g_context->_location->_map->_tileSet->getByName("chest")->getId();
		g_context->_location->_map->addObject(chest, chest, coords);
	}
	/* add a ship if you just defeated a pirate ship */
	else if (_creature->getTile().getTileType()->isPirateShip()) {
		MapTile ship = g_context->_location->_map->_tileSet->getByName("ship")->getId();
		ship.setDirection(_creature->getTile().getDirection());
		g_context->_location->_map->addObject(ship, ship, coords);
	}
}

bool CombatController::attackHit(Creature *attacker, Creature *defender) {
	assertMsg(attacker != nullptr, "attacker must not be nullptr");
	assertMsg(defender != nullptr, "defender must not be nullptr");

	int attackValue = xu4_random(0x100) + attacker->getAttackBonus();
	int defenseValue = defender->getDefense();

	return attackValue > defenseValue;
}

bool CombatController::attackAt(const Coords &coords, PartyMember *attacker, int dir, int range, int distance) {
	const Weapon *weapon = attacker->getWeapon();
	bool wrongRange = weapon->rangeAbsolute() && (distance != range);

	MapTile hittile = _map->_tileSet->getByName(weapon->getHitTile())->getId();
	MapTile misstile = _map->_tileSet->getByName(weapon->getMissTile())->getId();

	// Check to see if something hit
	Creature *creature = _map->creatureAt(coords);

	/* If we haven't hit a creature, or the weapon's range is absolute
	   and we're testing the wrong range, stop now! */
	if (!creature || wrongRange) {

		/* If the weapon is shown as it travels, show it now */
		if (weapon->showTravel()) {
			GameController::flashTile(coords, misstile, 1);
		}

		// no target found
		return false;
	}

	/* Did the weapon miss? */
	if ((g_context->_location->_prev->_map->_id == MAP_ABYSS && !weapon->isMagic()) || /* non-magical weapon in the Abyss */
	        !attackHit(attacker, creature)) { /* player naturally missed */
		g_screen->screenMessage("Missed!\n");

		/* show the 'miss' tile */
		GameController::flashTile(coords, misstile, 1);
	} else { /* The weapon hit! */

		/* show the 'hit' tile */
		GameController::flashTile(coords, misstile, 1);
		soundPlay(SOUND_NPC_STRUCK, false, -1);                                   // NPC_STRUCK, melee hit
		GameController::flashTile(coords, hittile, 3);

		/* apply the damage to the creature */
		if (!attacker->dealDamage(creature, attacker->getDamage())) {
			creature = nullptr;
			GameController::flashTile(coords, hittile, 1);
		}
	}

	return true;
}

bool CombatController::rangedAttack(const Coords &coords, Creature *attacker) {
	MapTile hittile = _map->_tileSet->getByName(attacker->getHitTile())->getId();
	MapTile misstile = _map->_tileSet->getByName(attacker->getMissTile())->getId();

	Creature *target = isCreature(attacker) ? _map->partyMemberAt(coords) : _map->creatureAt(coords);

	/* If we haven't hit something valid, stop now */
	if (!target) {
		GameController::flashTile(coords, misstile, 1);
		return false;
	}

	/* Get the effects of the tile the creature is using */
	TileEffect effect = hittile.getTileType()->getEffect();

	/* Monster's ranged attacks never miss */

	GameController::flashTile(coords, misstile, 1);
	/* show the 'hit' tile */
	GameController::flashTile(coords, hittile, 3);

	/* These effects happen whether or not the opponent was hit */
	switch (effect) {
	case EFFECT_ELECTRICITY:
		/* FIXME: are there any special effects here? */
		soundPlay(SOUND_PC_STRUCK, false);
		g_screen->screenMessage("\n%s %cElectrified%c!\n", target->getName().c_str(), FG_BLUE, FG_WHITE);
		attacker->dealDamage(target, attacker->getDamage());
		break;

	case EFFECT_POISON:
	case EFFECT_POISONFIELD:
		/* see if the player is poisoned */
		if ((xu4_random(2) == 0) && (target->getStatus() != STAT_POISONED)) {
			// POISON_EFFECT, ranged hit
			soundPlay(SOUND_POISON_EFFECT, false);
			g_screen->screenMessage("\n%s %cPoisoned%c!\n", target->getName().c_str(), FG_GREEN, FG_WHITE);
			target->addStatus(STAT_POISONED);
		}
		// else g_screen->screenMessage("Failed.\n");
		break;

	case EFFECT_SLEEP:
		/* see if the player is put to sleep */
		if (xu4_random(2) == 0) {
			// SLEEP, ranged hit, plays even if sleep failed or PC already asleep
			soundPlay(SOUND_SLEEP, false);
			g_screen->screenMessage("\n%s %cSlept%c!\n", target->getName().c_str(), FG_PURPLE, FG_WHITE);
			target->putToSleep();
		}
		// else g_screen->screenMessage("Failed.\n");
		break;

	case EFFECT_LAVA:
	case EFFECT_FIRE:
		/* FIXME: are there any special effects here? */
		soundPlay(SOUND_PC_STRUCK, false);
		g_screen->screenMessage("\n%s %c%s Hit%c!\n", target->getName().c_str(), FG_RED,
		              effect == EFFECT_LAVA ? "Lava" : "Fiery", FG_WHITE);
		attacker->dealDamage(target, attacker->getDamage());
		break;

	default:
		/* show the appropriate 'hit' message */
		// soundPlay(SOUND_PC_STRUCK, false);
		if (hittile == g_tileSets->findTileByName("magic_flash")->getId())
			g_screen->screenMessage("\n%s %cMagical Hit%c!\n", target->getName().c_str(), FG_BLUE, FG_WHITE);
		else
			g_screen->screenMessage("\n%s Hit!\n", target->getName().c_str());
		attacker->dealDamage(target, attacker->getDamage());
		break;
	}
	GameController::flashTile(coords, hittile, 1);
	return true;
}

void CombatController::rangedMiss(const Coords &coords, Creature *attacker) {
	/* If the creature leaves a tile behind, do it here! (lava lizard, etc) */
	const Tile *ground = _map->tileTypeAt(coords, WITH_GROUND_OBJECTS);
	if (attacker->leavesTile() && ground->isWalkable())
		_map->_annotations->add(coords, _map->_tileSet->getByName(attacker->getHitTile())->getId());
}

bool CombatController::returnWeaponToOwner(const Coords &coords, int distance, int dir, const Weapon *weapon) {
	MapCoords new_coords = coords;

	MapTile misstile = _map->_tileSet->getByName(weapon->getMissTile())->getId();

	/* reverse the direction of the weapon */
	Direction returnDir = dirReverse(dirFromMask(dir));

	for (int i = distance; i > 1; i--) {
		new_coords.move(returnDir, _map);

		GameController::flashTile(new_coords, misstile, 1);
	}
	gameUpdateScreen();

	return true;
}

void CombatController::finishTurn() {
	PartyMember *player = getCurrentPlayer();
	int quick;

	/* return to party overview */
	g_context->_stats->setView(STATS_PARTY_OVERVIEW);

	if (isWon() && _winOrLose) {
		end(true);
		return;
	}

	/* make sure the player with the focus is still in battle (hasn't fled or died) */
	if (player) {
		/* apply effects from tile player is standing on */
		player->applyEffect(g_context->_location->_map->tileTypeAt(player->getCoords(), WITH_GROUND_OBJECTS)->getEffect());
	}

	quick = (*g_context->_aura == Aura::QUICKNESS) && player && (xu4_random(2) == 0) ? 1 : 0;

	/* check to see if the player gets to go again (and is still alive) */
	if (!quick || player->isDisabled()) {

		do {
			g_context->_location->_map->_annotations->passTurn();

			/* put a sleeping person in place of the player,
			   or restore an awakened member to their original state */
			if (player) {
				if (player->getStatus() == STAT_SLEEPING && (xu4_random(8) == 0))
					player->wakeUp();

				/* remove focus from the current party member */
				player->setFocus(false);

				/* eat some food */
				g_context->_party->adjustFood(-1);
			}

			/* put the focus on the next party member */
			_focus++;

			/* move creatures and wrap around at end */
			if (_focus >= g_context->_party->size()) {

				/* reset the focus to the avatar and start the party's turn over again */
				_focus = 0;

				gameUpdateScreen();
				EventHandler::sleep(50); /* give a slight pause in case party members are asleep for awhile */

				/* adjust moves */
				g_context->_party->endTurn();

				/* count down our aura (if we have one) */
				g_context->_aura->passTurn();

				/**
				 * ====================
				 * HANDLE CREATURE STUFF
				 * ====================
				 */

				/* first, move all the creatures */
				moveCreatures();

				/* then, apply tile effects to creatures */
				applyCreatureTileEffects();

				/* check to see if combat is over */
				if (isLost()) {
					end(true);
					return;
				}

				/* end combat immediately if the enemy has fled */
				else if (isWon() && _winOrLose) {
					end(true);
					return;
				}
			}

			/* get the next party member */
			player = getCurrentPlayer();

		} while (!player ||
		         player->isDisabled() || /* dead or sleeping */
		         ((g_context->_party->getActivePlayer() >= 0) && /* active player is set */
		          (_party[g_context->_party->getActivePlayer()]) && /* and the active player is still in combat */
		          !_party[g_context->_party->getActivePlayer()]->isDisabled() && /* and the active player is not disabled */
		          (g_context->_party->getActivePlayer() != _focus)));
	} else {
		g_context->_location->_map->_annotations->passTurn();
	}

#if 0
	if (focus != 0) {
		getCurrentPlayer()->act();
		finishTurn();
	} else setActivePlayer(focus);
#else
	/* display info about the current player */
	setActivePlayer(_focus);
#endif
}

void CombatController::movePartyMember(MoveEvent &event) {
	/* active player left/fled combat */
	if ((event._result & MOVE_EXIT_TO_PARENT) && (g_context->_party->getActivePlayer() == _focus)) {
		g_context->_party->setActivePlayer(-1);
		/* assign active player to next available party member */
		for (int i = 0; i < g_context->_party->size(); i++) {
			if (_party[i] && !_party[i]->isDisabled()) {
				g_context->_party->setActivePlayer(i);
				break;
			}
		}
	}

	g_screen->screenMessage("%s\n", getDirectionName(event._dir));
	if (event._result & MOVE_MUST_USE_SAME_EXIT) {
		soundPlay(SOUND_ERROR);                                                // ERROR move, all PCs must use the same exit
		g_screen->screenMessage("All must use same exit!\n");
	} else if (event._result & MOVE_BLOCKED) {
		soundPlay(SOUND_BLOCKED);                                              // BLOCKED move
		g_screen->screenMessage("%cBlocked!%c\n", FG_GREY, FG_WHITE);
	} else if (event._result & MOVE_SLOWED) {
		soundPlay(SOUND_WALK_SLOWED);                                          // WALK_SLOWED move
		g_screen->screenMessage("%cSlow progress!%c\n", FG_GREY, FG_WHITE);
	} else if (_winOrLose && getCreature()->isEvil() && (event._result & (MOVE_EXIT_TO_PARENT | MOVE_MAP_CHANGE))) {
		soundPlay(SOUND_FLEE);                                                 // FLEE move
	} else {
		soundPlay(SOUND_WALK_COMBAT);                                          // WALK_COMBAT move
	}
}

void CombatController::keybinder(KeybindingAction action) {
	MetaEngine::executeAction(action);
}

void CombatController::attack(Direction dir, int distance) {
	g_screen->screenMessage("Dir: ");

	ReadDirController dirController;
#ifdef IOS_ULTIMA4
	U4IOS::IOSDirectionHelper directionPopup;
#endif
	if (dir == DIR_NONE) {
		eventHandler->pushController(&dirController);
		dir = dirController.waitFor();
		if (dir == DIR_NONE)
			return;
	}
	g_screen->screenMessage("%s\n", getDirectionName(dir));

	PartyMember *attacker = getCurrentPlayer();

	const Weapon *weapon = attacker->getWeapon();
	int range = weapon->getRange();
	if (weapon->canChooseDistance()) {
		g_screen->screenMessage("Range: ");

		if (distance == -1) {
			int choice = ReadChoiceController::get("123456789");
			distance = choice - '0';
		}

		if (distance >= 1 && distance <= weapon->getRange()) {
			range = distance;
			g_screen->screenMessage("%d\n", range);
		} else {
			return;
		}
	}

	// the attack was already made, even if there is no valid target
	// so play the attack sound
	soundPlay(SOUND_PC_ATTACK, false);                                        // PC_ATTACK, melee and ranged


	Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL,
	                           attacker->getCoords(), 1, range,
	                           weapon->canAttackThroughObjects() ? nullptr : &Tile::canAttackOverTile,
	                           false);

	bool foundTarget = false;
	int targetDistance = path.size();
	Coords targetCoords(attacker->getCoords());
	if (path.size() > 0)
		targetCoords = path.back();

	distance = 1;
	for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
		if (attackAt(*i, attacker, MASK_DIR(dir), range, distance)) {
			foundTarget = true;
			targetDistance = distance;
			targetCoords = *i;
			break;
		}
		distance++;
	}

	// is weapon lost? (e.g. dagger)
	if (weapon->loseWhenUsed() ||
	        (weapon->loseWhenRanged() && (!foundTarget || targetDistance > 1))) {
		if (!attacker->loseWeapon())
			g_screen->screenMessage("Last One!\n");
	}

	// does weapon leave a tile behind? (e.g. flaming oil)
	const Tile *ground = _map->tileTypeAt(targetCoords, WITHOUT_OBJECTS);
	if (!weapon->leavesTile().empty() && ground->isWalkable())
		_map->_annotations->add(targetCoords, _map->_tileSet->getByName(weapon->leavesTile())->getId());

	/* show the 'miss' tile */
	if (!foundTarget) {
		GameController::flashTile(targetCoords, weapon->getMissTile(), 1);
		/* This goes here so messages are shown in the original order */
		g_screen->screenMessage("Missed!\n");
	}

	// does weapon returns to its owner? (e.g. magic axe)
	if (weapon->returns())
		returnWeaponToOwner(targetCoords, targetDistance, MASK_DIR(dir), weapon);
}

void CombatController::update(Party *party, PartyEvent &event) {
	if (event._type == PartyEvent::PLAYER_KILLED)
		g_screen->screenMessage("\n%c%s is Killed!%c\n", FG_RED, event._player->getName().c_str(), FG_WHITE);
}

/*-------------------------------------------------------------------*/

CombatMap::CombatMap() : Map(), _dungeonRoom(false), _altarRoom(VIRT_NONE), _contextual(false) {}

CreatureVector CombatMap::getCreatures() {
	ObjectDeque::iterator i;
	CreatureVector creatures;
	for (i = _objects.begin(); i != _objects.end(); i++) {
		if (isCreature(*i) && !isPartyMember(*i))
			creatures.push_back(dynamic_cast<Creature *>(*i));
	}
	return creatures;
}

PartyMemberVector CombatMap::getPartyMembers() {
	ObjectDeque::iterator i;
	PartyMemberVector party;
	for (i = _objects.begin(); i != _objects.end(); i++) {
		if (isPartyMember(*i))
			party.push_back(dynamic_cast<PartyMember *>(*i));
	}
	return party;
}

PartyMember *CombatMap::partyMemberAt(Coords coords) {
	PartyMemberVector party = getPartyMembers();
	PartyMemberVector::iterator i;

	for (i = party.begin(); i != party.end(); i++) {
		if ((*i)->getCoords() == coords)
			return *i;
	}
	return nullptr;
}

Creature *CombatMap::creatureAt(Coords coords) {
	CreatureVector creatures = getCreatures();
	CreatureVector::iterator i;

	for (i = creatures.begin(); i != creatures.end(); i++) {
		if ((*i)->getCoords() == coords)
			return *i;
	}
	return nullptr;
}

MapId CombatMap::mapForTile(const Tile *groundTile, const Tile *transport, Object *obj) {
	bool fromShip = false,
	     toShip = false;
	Object *objUnder = g_context->_location->_map->objectAt(g_context->_location->_coords);

	static Std::map<const Tile *, MapId, Std::PointerHash> tileMap;
	if (!tileMap.size()) {
		tileMap[g_tileSets->get("base")->getByName("horse")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("swamp")] = MAP_MARSH_CON;
		tileMap[g_tileSets->get("base")->getByName("grass")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("brush")] = MAP_BRUSH_CON;
		tileMap[g_tileSets->get("base")->getByName("forest")] = MAP_FOREST_CON;
		tileMap[g_tileSets->get("base")->getByName("hills")] = MAP_HILL_CON;
		tileMap[g_tileSets->get("base")->getByName("dungeon")] = MAP_DUNGEON_CON;
		tileMap[g_tileSets->get("base")->getByName("city")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("castle")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("town")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("lcb_entrance")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("bridge")] = MAP_BRIDGE_CON;
		tileMap[g_tileSets->get("base")->getByName("balloon")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("bridge_pieces")] = MAP_BRIDGE_CON;
		tileMap[g_tileSets->get("base")->getByName("shrine")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("chest")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("brick_floor")] = MAP_BRICK_CON;
		tileMap[g_tileSets->get("base")->getByName("moongate")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("moongate_opening")] = MAP_GRASS_CON;
		tileMap[g_tileSets->get("base")->getByName("dungeon_floor")] = MAP_GRASS_CON;
	}
	static Std::map<const Tile *, MapId, Std::PointerHash> dungeontileMap;
	if (!dungeontileMap.size()) {
		dungeontileMap[g_tileSets->get("dungeon")->getByName("brick_floor")] = MAP_DNG0_CON;
		dungeontileMap[g_tileSets->get("dungeon")->getByName("up_ladder")] = MAP_DNG1_CON;
		dungeontileMap[g_tileSets->get("dungeon")->getByName("down_ladder")] = MAP_DNG2_CON;
		dungeontileMap[g_tileSets->get("dungeon")->getByName("up_down_ladder")] = MAP_DNG3_CON;
		// dungeontileMap[g_tileSets->get("dungeon")->getByName("chest")] = MAP_DNG4_CON;
		// chest tile doesn't work that well
		dungeontileMap[g_tileSets->get("dungeon")->getByName("dungeon_door")] = MAP_DNG5_CON;
		dungeontileMap[g_tileSets->get("dungeon")->getByName("secret_door")] = MAP_DNG6_CON;
	}

	if (g_context->_location->_context & CTX_DUNGEON) {
		if (dungeontileMap.find(groundTile) != dungeontileMap.end())
			return dungeontileMap[groundTile];

		return MAP_DNG0_CON;
	}

	if (transport->isShip() || (objUnder && objUnder->getTile().getTileType()->isShip()))
		fromShip = true;
	if (obj->getTile().getTileType()->isPirateShip())
		toShip = true;

	if (fromShip && toShip)
		return MAP_SHIPSHIP_CON;

	/* We can fight creatures and townsfolk */
	if (obj->getType() != Object::UNKNOWN) {
		const Tile *tileUnderneath = g_context->_location->_map->tileTypeAt(obj->getCoords(), WITHOUT_OBJECTS);

		if (toShip)
			return MAP_SHORSHIP_CON;
		else if (fromShip && tileUnderneath->isWater())
			return MAP_SHIPSEA_CON;
		else if (tileUnderneath->isWater())
			return MAP_SHORE_CON;
		else if (fromShip && !tileUnderneath->isWater())
			return MAP_SHIPSHOR_CON;
	}

	if (tileMap.find(groundTile) != tileMap.end())
		return tileMap[groundTile];

	return MAP_BRICK_CON;
}

} // End of namespace Ultima4
} // End of namespace Ultima
