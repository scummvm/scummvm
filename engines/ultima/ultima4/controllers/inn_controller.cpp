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

#include "ultima/ultima4/controllers/inn_controller.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/map/mapmgr.h"

namespace Ultima {
namespace Ultima4 {

InnController::InnController() {
	_map = nullptr;
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
		g_music->fadeOut(INN_FADE_OUT_TIME); /* Fade volume out to ease into rest */

	EventHandler::wait_msecs(INN_FADE_OUT_TIME);

	/* show the sleeping avatar */
	g_context->_party->setTransport(g_context->_location->_map->_tileSet->getByName("corpse")->getId());
	gameUpdateScreen();

	g_screen->screenDisableCursor();

	EventHandler::wait_msecs(settings._innTime * 1000);

	g_screen->screenEnableCursor();

	/* restore the avatar to normal */
	g_context->_party->setTransport(g_context->_location->_map->_tileSet->getByName("avatar")->getId());
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

	g_screen->screenMessage("\nMorning!\n");
	g_screen->screenPrompt();

	g_music->fadeIn(INN_FADE_IN_TIME, true);
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
		assert(city);

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
			Person *isaac = new Person(creatureMgr->getById(GHOST_ID)->getTile());

			isaac->setMovementBehavior(MOVEMENT_WANDER);

			isaac->setDialogue(city->_extraDialogues[0]);
			isaac->getStart() = coords;
			isaac->setPrevTile(isaac->getTile());

			// Add Isaac near the Avatar
			city->addPerson(isaac);

			delete isaac;
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
			g_screen->screenMessage("\nIn the middle of the night while out on a stroll...\n\n");
			showMessage = false;
		}


		_map = getCombatMap(mapMgr->get(mapid));
		g_game->setMap(_map, true, nullptr, this);

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
