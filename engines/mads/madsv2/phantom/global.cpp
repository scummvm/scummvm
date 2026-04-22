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

#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

namespace Rooms {
extern void section_1_preload();
extern void section_2_preload();
extern void section_3_preload();
extern void section_4_preload();
extern void section_5_preload();
extern void section_6_preload();
extern void section_7_preload();
extern void section_8_preload();
} // namespace Rooms

void global_section_constructor() {
	section_preload_code_pointer = NULL;
	section_room_constructor = NULL;
	section_init_code_pointer = NULL;
	section_parser_code_pointer = NULL;
	section_daemon_code_pointer = NULL;

	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_section) {
	case 1:
		section_preload_code_pointer = Rooms::section_1_preload;
		break;
	case 2:
		section_preload_code_pointer = Rooms::section_2_preload;
		break;
	case 3:
		section_preload_code_pointer = Rooms::section_3_preload;
		break;
	case 4:
		section_preload_code_pointer = Rooms::section_4_preload;
		break;
	case 5:
		section_preload_code_pointer = Rooms::section_5_preload;
		break;
	}
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
