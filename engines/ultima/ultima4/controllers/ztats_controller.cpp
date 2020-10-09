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

#include "ultima/ultima4/controllers/ztats_controller.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

bool ZtatsController::keyPressed(int key) {
	switch (key) {
	case Common::KEYCODE_RETURN:
		keybinder(KEYBIND_ESCAPE);
		return true;
	case Common::KEYCODE_UP:
	case Common::KEYCODE_LEFT:
		g_context->_stats->prevItem();
		return true;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_RIGHT:
		g_context->_stats->nextItem();
		return true;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		if (g_ultima->_saveGame->_members >= key - '0')
			g_context->_stats->setView(StatsView(STATS_CHAR1 + key - '1'));
		return true;
	case '0':
		g_context->_stats->setView(StatsView(STATS_WEAPONS));
		return true;
	default:
		return KeyHandler::defaultHandler(key, nullptr);
	}
}

void ZtatsController::keybinder(KeybindingAction action) {
	if (action == KEYBIND_ESCAPE) {
		g_context->_stats->setView(StatsView(STATS_PARTY_OVERVIEW));
		doneWaiting();
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
