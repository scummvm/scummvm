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
#include "mads/madsv2/nebular/rooms/section6.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/global.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

void section_8_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(5));
}

void section_8_walker() {
	g_engine->_soundManager->command(5);

	if ((global[kFromCockpit] && !global[kExitShip]) ||
			new_room == 804 || new_room == 805 ||
			new_room == 808 || new_room == 810) {
		*player.series_name = '\0';
	} else {
		Common::strcpy_s(player.series_name, global[kSexOfRex] == SEX_FEMALE ? "ROX" : "RXM");
	}

	pal_change_color(16, 0x0A, 0x3F, 0x3F);
	pal_change_color(17, 0x0A, 0x2D, 0x2D);
}

void section_8_music() {
	if (!config_file.music_flag)
		g_engine->_soundManager->command(2);
	else {
		switch (new_room) {
		case 801:
		case 802:
		case 803:
		case 804:
		case 806:
		case 807:
		case 808:
			g_engine->_soundManager->command(20);
			break;

		case 805:
			g_engine->_soundManager->command(23);
			break;

		case 810:
			g_engine->_soundManager->command(10);
			break;

		default:
			break;
		}
	}
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
