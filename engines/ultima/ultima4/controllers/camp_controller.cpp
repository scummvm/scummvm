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

#include "ultima/ultima4/controllers/camp_controller.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

CampController::CampController() {
	MapId id;

	/* setup camp (possible, but not for-sure combat situation */
	if (g_context->_location->_context & CTX_DUNGEON)
		id = MAP_CAMP_DNG;
	else
		id = MAP_CAMP_CON;

	_map = getCombatMap(mapMgr->get(id));
	g_game->setMap(_map, true, nullptr, this);
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

	g_music->camp();

	g_screen->screenMessage("Resting...\n");
	g_screen->screenDisableCursor();

	EventHandler::wait_msecs(settings._campTime * 1000);

	g_screen->screenEnableCursor();

	/* Is the party ambushed during their rest? */
	if (settings._campingAlwaysCombat || (xu4_random(8) == 0)) {
		const Creature *m = creatureMgr->randomAmbushing();

		g_music->playMapMusic();
		g_screen->screenMessage("Ambushed!\n");

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
		if (((g_ultima->_saveGame->_moves / CAMP_HEAL_INTERVAL) >= 0x10000) ||
		        (((g_ultima->_saveGame->_moves / CAMP_HEAL_INTERVAL) & 0xffff) != g_ultima->_saveGame->_lastCamp))
			healed = heal();

		g_screen->screenMessage(healed ? "Party Healed!\n" : "No effect.\n");
		g_ultima->_saveGame->_lastCamp = (g_ultima->_saveGame->_moves / CAMP_HEAL_INTERVAL) & 0xffff;

		eventHandler->popController();
		g_game->exitToParentMap();
		g_music->fadeIn(CAMP_FADE_IN_TIME, true);
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

} // End of namespace Ultima4
} // End of namespace Ultima
