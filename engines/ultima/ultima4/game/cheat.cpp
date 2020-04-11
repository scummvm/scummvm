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

#include "ultima/ultima4/game/cheat.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/stats.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/sound/music.h"
#include "common/debug.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima4 {

CheatMenuController::CheatMenuController(GameController *gc) : _game(gc) {
}

bool CheatMenuController::keyPressed(int key) {
	bool valid = true;

	switch (key) {
	case 'h': {
		screenMessage("Help:\n"
		              "F1-F8 - +Virtue\n"
		              "(more)");

		ReadChoiceController pauseController("");
		eventHandler->pushController(&pauseController);
		pauseController.waitFor();

		screenMessage("\n"
		              "t - Transports\n"
		              "(more)");

		eventHandler->pushController(&pauseController);
		pauseController.waitFor();
		break;
	}

	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_SPACE:
		screenMessage("Nothing\n");
		break;

	default:
		valid = false;
		break;
	}

	if (valid) {
		doneWaiting();
		screenPrompt();
	}

	return valid;
}

bool WindCmdController::keyPressed(int key) {
	switch (key) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_RIGHT:
		g_context->_windDirection = keyToDirection(key);
		screenMessage("Wind %s!\n", getDirectionName(static_cast<Direction>(g_context->_windDirection)));
		doneWaiting();
		return true;

	case 'l':
		g_context->_windLock = !g_context->_windLock;
		screenMessage("Wind direction is %slocked!\n", g_context->_windLock ? "" : "un");
		doneWaiting();
		return true;
	}

	return KeyHandler::defaultHandler(key, NULL);
}

} // End of namespace Ultima4
} // End of namespace Ultima
