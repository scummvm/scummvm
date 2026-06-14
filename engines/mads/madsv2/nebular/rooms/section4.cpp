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

#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/nebular/rooms/section4.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/global.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

void section_4_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(4));
}

void section_4_walker() {
	g_engine->_soundManager->command(5);
	Common::String oldName = player.series_name;

	if ((new_room == 403) || (new_room == 409))
		*player.series_name = '\0';
	else if (global[kSexOfRex] == REX_FEMALE)
		Common::strcpy_s(player.series_name, "ROX");
	else
		Common::strcpy_s(player.series_name, "RXM");

	player.scaling_velocity = true;

	if (oldName != Common::String(player.series_name))
		player.walker_must_reload = true;

	pal_change_color(16, 10, 63, 63);
	pal_change_color(17, 10, 45, 45);
}

void section_4_music() {
	if (!config_file.music_flag) {
		g_engine->_soundManager->command(2);
		return;
	}

	switch (new_room) {
	case 401:
		g_engine->_soundManager->startQueuedCommands();
		if (previous_room == 402)
			g_engine->_soundManager->command(12, 64);
		else
			g_engine->_soundManager->command(12, 1);
		break;

	case 402:
		g_engine->_soundManager->startQueuedCommands();
		g_engine->_soundManager->command(12, 127);
		break;

	case 405:
	case 407:
	case 409:
	case 410:
	case 413:
		g_engine->_soundManager->command(10);
		break;

	case 408:
		g_engine->_soundManager->command(52);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
