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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/camp.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/city.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/mapmgr.h"
#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/music.h"
#include "ultima/ultima4/names.h"
#include "ultima/ultima4/object.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/player.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/stats.h"
#include "ultima/ultima4/graphics/tileset.h"
#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

void campTimer(void *data);
void campEnd(void);
int campHeal(HealType heal_type);
void innTimer(void *data);

CampController::CampController() {
	MapId id;

	/* setup camp (possible, but not for-sure combat situation */
	if (g_context->_location->_context & CTX_DUNGEON)
		id = MAP_CAMP_DNG;
	else
		id = MAP_CAMP_CON;

	_map = getCombatMap(mapMgr->get(id));
	g_game->setMap(_map, true, NULL, this);
}

void CampController::init(Creature *m) {
	CombatController::init(m);
	_camping = true;
}

void CampController::begin() {
	// make sure everyone's asleep
	for (int i = 0; i < g_context->_party->size(); i++)
		g_context->_party->member(i)->putToSleep();

	CombatController::begin();

	musicMgr->camp();

	screenMessage("Resting...\n");
	screenDisableCursor();

	EventHandler::wait_msecs(settings._campTime * 1000);

	screenEnableCursor();

	/* Is the party ambushed during their rest? */
	if (settings._campingAlwaysCombat || (xu4_random(8) == 0)) {
		const Creature *m = creatureMgr->randomAmbushing();

		musicMgr->play();
		screenMessage("Ambushed!\n");

		/* create an ambushing creature (so it leaves a chest) */
		setCreature(g_context->_location->_prev->_map->addCreature(m, g_context->_location->_prev->_coords));

		/* fill the creature table with creatures and place them */
		fillCreatureTable(m);
		placeCreatures();

		/* creatures go first! */
		finishTurn();
	} else {
		/* Wake everyone up! */
		for (int i = 0; i < g_context->_party->size(); i++)
			g_context->_party->member(i)->wakeUp();

		/* Make sure we've waited long enough for camping to be effective */
		bool healed = false;
		if (((g_context->_saveGame->_moves / CAMP_HEAL_INTERVAL) >= 0x10000) ||
		        (((g_context->_saveGame->_moves / CAMP_HEAL_INTERVAL) & 0xffff) != g_context->_saveGame->_lastCamp))
			healed = heal();

		screenMessage(healed ? "Party Healed!\n" : "No effect.\n");
		g_context->_saveGame->_lastCamp = (g_context->_saveGame->_moves / CAMP_HEAL_INTERVAL) & 0xffff;

		eventHandler->popController();
		g_game->exitToParentMap();
		musicMgr->fadeIn(CAMP_FADE_IN_TIME, true);
		delete this;
	}
}

void CampController::end(bool adjustKarma) {
	// wake everyone up!
	for (int i = 0; i < g_context->_party->size(); i++)
		g_context->_party->member(i)->wakeUp();
	CombatController::end(adjustKarma);
}

bool CampController::heal() {
	// restore each party member to max mp, and restore some hp
	bool healed = false;
	for (int i = 0; i < g_context->_party->size(); i++) {
		PartyMember *m = g_context->_party->member(i);
		m->setMp(m->getMaxMp());
		if ((m->getHp() < m->getMaxHp()) && m->heal(HT_CAMPHEAL))
			healed = true;
	}

	return healed;
}

InnController::InnController() {
	_map = NULL;
	/*
	 * Normally in cities, only one opponent per encounter; inn's
	 * override this to get the regular encounter size.
	 */
	_forceStandardEncounterSize = true;
}

void InnController::begin() {
	/* first, show the avatar before sleeping */
	gameUpdateScreen();

	/* in the original, the vendor music plays straight through sleeping */
	if (settings._enhancements)
		musicMgr->fadeOut(INN_FADE_OUT_TIME); /* Fade volume out to ease into rest */

	EventHandler::wait_msecs(INN_FADE_OUT_TIME);

	/* show the sleeping avatar */
	g_context->_party->setTransport(g_context->_location->_map->_tileset->getByName("corpse")->getId());
	gameUpdateScreen();

	screenDisableCursor();

	EventHandler::wait_msecs(settings._innTime * 1000);

	screenEnableCursor();

	/* restore the avatar to normal */
	g_context->_party->setTransport(g_context->_location->_map->_tileset->getByName("avatar")->getId());
	gameUpdateScreen();

	/* the party is always healed */
	heal();

	/* Is there a special encounter during your stay? */
	// mwinterrowd suggested code, based on u4dos
	if (g_context->_party->member(0)->isDead()) {
		maybeMeetIsaac();
	} else {
		if (xu4_random(8) != 0) {
			maybeMeetIsaac();
		} else {
			maybeAmbush();
		}
	}

	screenMessage("\nMorning!\n");
	screenPrompt();

	musicMgr->fadeIn(INN_FADE_IN_TIME, true);
}

bool InnController::heal() {
	// restore each party member to max mp, and restore some hp
	bool healed = false;
	for (int i = 0; i < g_context->_party->size(); i++) {
		PartyMember *m = g_context->_party->member(i);
		m->setMp(m->getMaxMp());
		if ((m->getHp() < m->getMaxHp()) && m->heal(HT_INNHEAL))
			healed = true;
	}

	return healed;
}


void InnController::maybeMeetIsaac() {
	// Does Isaac the Ghost pay a visit to the Avatar?
	//  if ((location == skara_brae) && (random(4) = 0) {
	//          // create Isaac the Ghost
	//  }
	if ((g_context->_location->_map->_id == 11) && (xu4_random(4) == 0)) {
		City *city = dynamic_cast<City *>(g_context->_location->_map);

		if (city->_extraDialogues.size() == 1 &&
		        city->_extraDialogues[0]->getName() == "Isaac") {

			Coords coords(27, xu4_random(3) + 10, g_context->_location->_coords.z);

			// If Isaac is already around, just bring him back to the inn
			for (ObjectDeque::iterator i = g_context->_location->_map->_objects.begin();
			        i != g_context->_location->_map->_objects.end();
			        i++) {
				Person *p = dynamic_cast<Person *>(*i);
				if (p && p->getName() == "Isaac") {
					p->setCoords(coords);
					return;
				}
			}

			// Otherwise, we need to create Isaac
			Person *Isaac;
			Isaac = new Person(creatureMgr->getById(GHOST_ID)->getTile());

			Isaac->setMovementBehavior(MOVEMENT_WANDER);

			Isaac->setDialogue(city->_extraDialogues[0]);
			Isaac->getStart() = coords;
			Isaac->setPrevTile(Isaac->getTile());

			// Add Isaac near the Avatar
			city->addPerson(Isaac);
		}
	}
}

void InnController::maybeAmbush() {
	if (settings._innAlwaysCombat || (xu4_random(8) == 0)) {
		MapId mapid;
		Creature *creature;
		bool showMessage = true;

		/* Rats seem much more rare than meeting rogues in the streets */
		if (xu4_random(4) == 0) {
			/* Rats! */
			mapid = MAP_BRICK_CON;
			creature = g_context->_location->_map->addCreature(creatureMgr->getById(RAT_ID), g_context->_location->_coords);
		} else {
			/* While strolling down the street, attacked by rogues! */
			mapid = MAP_INN_CON;
			creature = g_context->_location->_map->addCreature(creatureMgr->getById(ROGUE_ID), g_context->_location->_coords);
			screenMessage("\nIn the middle of the night while out on a stroll...\n\n");
			showMessage = false;
		}


		_map = getCombatMap(mapMgr->get(mapid));
		g_game->setMap(_map, true, NULL, this);

		init(creature);
		showCombatMessage(showMessage);
		CombatController::begin();
	}
}

void InnController::awardLoot() {
	// never get a chest from inn combat
}

} // End of namespace Ultima4
} // End of namespace Ultima
