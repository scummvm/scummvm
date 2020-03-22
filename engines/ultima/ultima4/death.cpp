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
#include "ultima/ultima4/death.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/city.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/mapmgr.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/player.h"
#include "ultima/ultima4/portal.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/stats.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

#define REVIVE_WORLD_X 86
#define REVIVE_WORLD_Y 107
#define REVIVE_CASTLE_X 19
#define REVIVE_CASTLE_Y 8

int timerCount;
unsigned int timerMsg;
int deathSequenceRunning = 0;

void deathTimer(void *data);
void deathRevive(void);

const struct {
	int timeout;                /* pause in seconds */
	const char *text;           /* text of message */
} deathMsgs[] = {
	{ 5, "\n\n\nAll is Dark...\n" },
	{ 5, "\nBut wait...\n" },
	{ 5, "Where am I?...\n" },
	{ 5, "Am I dead?...\n" },
	{ 5, "Afterlife?...\n" },
	{ 5, "You hear:\n    %s\n" },
	{ 5, "I feel motion...\n" },
	{ 5, "\nLord British says: I have pulled thy spirit and some possessions from the void.  Be more careful in the future!\n\n\020" }
};

#define N_MSGS (sizeof(deathMsgs) / sizeof(deathMsgs[0]))

void deathStart(int delay) {
	if (deathSequenceRunning)
		return;

	// stop playing music
	musicMgr->fadeOut(1000);

	deathSequenceRunning = 1;
	timerCount = 0;
	timerMsg = 0;

	WaitController waitCtrl(delay * settings._gameCyclesPerSecond);
	eventHandler->pushController(&waitCtrl);
	waitCtrl.wait();

	gameSetViewMode(VIEW_DEAD);

	eventHandler->pushKeyHandler(&KeyHandler::ignoreKeys);
	screenDisableCursor();

	eventHandler->getTimer()->add(&deathTimer, settings._gameCyclesPerSecond);
}

void deathTimer(void *data) {

	timerCount++;
	if ((timerMsg < N_MSGS) && (timerCount > deathMsgs[timerMsg].timeout)) {

		screenMessage(deathMsgs[timerMsg].text, g_context->_party->member(0)->getName().c_str());
		screenHideCursor();

		timerCount = 0;
		timerMsg++;

		if (timerMsg >= N_MSGS) {
			eventHandler->getTimer()->remove(&deathTimer);
			deathRevive();
		}
	}
}

void deathRevive() {
	while (!g_context->_location->_map->isWorldMap() && g_context->_location->_prev != NULL) {
		g_game->exitToParentMap();
	}

	eventHandler->setController(g_game);

	deathSequenceRunning = 0;
	gameSetViewMode(VIEW_NORMAL);

	/* Move our world map location to Lord British's Castle */
	g_context->_location->_coords = g_context->_location->_map->_portals[0]->_coords;

	/* Now, move the avatar into the castle and put him
	   in front of Lord British */
	g_game->setMap(mapMgr->get(100), 1, NULL);
	g_context->_location->_coords.x = REVIVE_CASTLE_X;
	g_context->_location->_coords.y = REVIVE_CASTLE_Y;
	g_context->_location->_coords.z = 0;

	g_context->_aura->set();
	g_context->_horseSpeed = 0;
	g_context->_lastCommandTime = g_system->getMillis();
	musicMgr->play();

	g_context->_party->reviveParty();

	screenEnableCursor();
	screenShowCursor();
	g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	screenRedrawScreen();
}

} // End of namespace Ultima4
} // End of namespace Ultima
