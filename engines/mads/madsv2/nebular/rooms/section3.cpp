/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

void section_3_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(4));
}

void section_3_walker() {
	g_engine->_soundManager->command(5);

	Common::String oldName = player.series_name;
	Common::strcpy_s(player.series_name, (global[kSexOfRex] == REX_MALE) ? "RXM" : "ROX");

	if ((new_room == 313) || (new_room == 366)
			|| ((new_room >= 301) && (new_room <= 303))
			|| ((new_room == 304) && (room_id == 303))
			|| ((new_room == 311) && (room_id == 304))
			|| ((new_room >= 308) && (new_room <= 310))
			|| ((new_room >= 319) && (new_room <= 322))
			|| ((new_room >= 387) && (new_room <= 391))) {
		*player.series_name = '\0';
		player.walker_must_reload = true;
	}

	player.scaling_velocity = true;
	if (oldName != Common::String(player.series_name))
		player.walker_must_reload = true;

	pal_change_color(16, 10, 63, 63);
	pal_change_color(17, 10, 45, 45);
}

void section_3_music() {
	if (!config_file.music_flag) {
		g_engine->_soundManager->command(2);
		return;
	}

	switch (new_room) {
	case 301:
	case 302:
	case 303:
	case 304:
	case 308:
	case 309:
	case 310:
		g_engine->_soundManager->command(11);
		break;

	case 311:
		if (previous_room == 304)
			g_engine->_soundManager->command(11);
		else
			g_engine->_soundManager->command(10);
		break;

	case 313:
	case 316:
	case 320:
	case 322:
	case 357:
	case 358:
	case 359:
	case 360:
	case 361:
	case 387:
	case 388:
	case 389:
	case 390:
	case 391:
	case 399:
		g_engine->_soundManager->command(10);
		break;

	case 318:
		if ((previous_room == 357) || (previous_room == 407))
			g_engine->_soundManager->command(10);
		else if (previous_room == 319)
			g_engine->_soundManager->command(16);
		else
			g_engine->_soundManager->command(3);

		g_engine->_soundManager->command(50);
		break;

	case 319:
		g_engine->_soundManager->command(16);
		break;

	case 321:
		g_engine->_soundManager->command(18);
		break;

	default:
		break;
	}
}

void section_3_pre_parser() {
	player.need_to_walk = false;
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
