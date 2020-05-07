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

#include "ultima/ultima4/game/death.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/controllers/wait_controller.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/ultima4.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

Death *g_death;

#define REVIVE_WORLD_X 86
#define REVIVE_WORLD_Y 107
#define REVIVE_CASTLE_X 19
#define REVIVE_CASTLE_Y 8

const struct {
	int _timeout;			///< pause in seconds
	const char *_text;		///< text of message
} DEATH_MSGS[] = {
	{ 5, "\n\n\nAll is Dark...\n" },
	{ 5, "\nBut wait...\n" },
	{ 5, "Where am I?...\n" },
	{ 5, "Am I dead?...\n" },
	{ 5, "Afterlife?...\n" },
	{ 5, "You hear:\n    %s\n" },
	{ 5, "I feel motion...\n" },
	{ 5, "\nLord British says: I have pulled thy spirit and some possessions from the void.  Be more careful in the future!\n\n\020" }
};

#define N_MSGS (sizeof(DEATH_MSGS) / sizeof(DEATH_MSGS[0]))

Death::Death() : timerCount(0), timerMsg(0), deathSequenceRunning(false) {
	g_death = this;
}

Death::~Death() {
	g_death = nullptr;
}

void Death::start(int delay) {
	if (deathSequenceRunning)
		return;

	// stop playing music
	g_music->fadeOut(1000);

	deathSequenceRunning = 1;
	timerCount = 0;
	timerMsg = 0;

	WaitController waitCtrl(delay * settings._gameCyclesPerSecond);
	eventHandler->pushController(&waitCtrl);
	waitCtrl.wait();

	gameSetViewMode(VIEW_DEAD);

	eventHandler->pushKeyHandler(&KeyHandler::ignoreKeys);
	g_screen->screenDisableCursor();

	eventHandler->getTimer()->add(&deathTimer, settings._gameCyclesPerSecond);
}

void Death::deathTimer(void *data) {
	g_death->timerCount++;
	if ((g_death->timerMsg < N_MSGS) && (g_death->timerCount > DEATH_MSGS[g_death->timerMsg]._timeout)) {

		g_screen->screenMessage(DEATH_MSGS[g_death->timerMsg]._text, g_context->_party->member(0)->getName().c_str());
		g_screen->screenHideCursor();

		g_death->timerCount = 0;
		g_death->timerMsg++;

		if (g_death->timerMsg >= N_MSGS) {
			eventHandler->getTimer()->remove(&deathTimer);
			g_death->revive();
		}
	}
}

void Death::revive() {
	while (!g_context->_location->_map->isWorldMap() && g_context->_location->_prev != nullptr) {
		g_game->exitToParentMap();
	}

	eventHandler->setController(g_game);

	deathSequenceRunning = false;
	gameSetViewMode(VIEW_NORMAL);

	// Move our world map location to Lord British's Castle
	g_context->_location->_coords = g_context->_location->_map->_portals[0]->_coords;

	// Now, move the avatar into the castle and put him in front of Lord British
	g_game->setMap(mapMgr->get(100), 1, nullptr);
	g_context->_location->_coords.x = REVIVE_CASTLE_X;
	g_context->_location->_coords.y = REVIVE_CASTLE_Y;
	g_context->_location->_coords.z = 0;

	g_context->_aura->set();
	g_context->_horseSpeed = 0;
	g_context->_lastCommandTime = g_system->getMillis();
	g_music->playMapMusic();

	g_context->_party->reviveParty();

	g_screen->screenEnableCursor();
	g_screen->screenShowCursor();
	g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	g_screen->update();
}

} // End of namespace Ultima4
} // End of namespace Ultima
