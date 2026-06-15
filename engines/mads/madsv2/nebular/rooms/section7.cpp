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
#include "mads/madsv2/nebular/rooms/section7.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/global.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

void section_7_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(5));
}

void section_7_walker() {
	g_engine->_soundManager->command(5);

	Common::String oldName = player.series_name;

	if ((new_room == 703) || (new_room == 704) || (new_room == 705)
	 || (new_room == 707) || (new_room == 710) || (new_room == 711))
		*player.series_name = '\0';
	else if (global[kSexOfRex] == REX_MALE)
		Common::strcpy_s(player.series_name, "RXM");
	else
		Common::strcpy_s(player.series_name, "ROX");

	player.scaling_velocity = true;

	if (oldName != Common::String(player.series_name))
		player.walker_must_reload = true;

	pal_change_color(16, 10, 63, 63);
	pal_change_color(17, 10, 45, 45);
}

void section_7_music() {
	if (!config_file.music_flag) {
		g_engine->_soundManager->command(2);
		return;
	}

	switch (new_room) {
	case 701:
	case 702:
	case 704:
	case 705:
	case 751:
		g_engine->_soundManager->command(38);
		break;
	case 703:
		if (global[kMonsterAlive] == 0)
			g_engine->_soundManager->command(24);
		else
			g_engine->_soundManager->command(27);
		break;
	case 706:
	case 707:
	case 710:
	case 711:
		g_engine->_soundManager->command(25);
		break;
	default:
		break;
	}
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
