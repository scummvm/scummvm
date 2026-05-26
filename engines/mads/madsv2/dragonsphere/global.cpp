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

#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

namespace Rooms {

// Section preloads
extern void section_1_preload();
extern void section_2_preload();
extern void section_3_preload();
extern void section_4_preload();
extern void section_5_preload();
extern void section_6_preload();
extern void section_9_preload();

extern void room_101_synchronize(Common::Serializer &s);
extern void room_102_synchronize(Common::Serializer &s);
extern void room_103_synchronize(Common::Serializer &s);
extern void room_104_synchronize(Common::Serializer &s);
extern void room_105_synchronize(Common::Serializer &s);
extern void room_106_synchronize(Common::Serializer &s);
extern void room_107_synchronize(Common::Serializer &s);
extern void room_108_synchronize(Common::Serializer &s);
extern void room_109_synchronize(Common::Serializer &s);
extern void room_110_synchronize(Common::Serializer &s);
extern void room_111_synchronize(Common::Serializer &s);
extern void room_112_synchronize(Common::Serializer &s);
extern void room_113_synchronize(Common::Serializer &s);
extern void room_114_synchronize(Common::Serializer &s);
extern void room_115_synchronize(Common::Serializer &s);
extern void room_116_synchronize(Common::Serializer &s);
extern void room_117_synchronize(Common::Serializer &s);
extern void room_118_synchronize(Common::Serializer &s);
extern void room_119_synchronize(Common::Serializer &s);
extern void room_120_synchronize(Common::Serializer &s);
extern void room_201_synchronize(Common::Serializer &s);
extern void room_203_synchronize(Common::Serializer &s);
extern void room_204_synchronize(Common::Serializer &s);
extern void room_205_synchronize(Common::Serializer &s);
extern void room_206_synchronize(Common::Serializer &s);
extern void room_301_synchronize(Common::Serializer &s);
extern void room_302_synchronize(Common::Serializer &s);
extern void room_303_synchronize(Common::Serializer &s);
extern void room_401_synchronize(Common::Serializer &s);
extern void room_402_synchronize(Common::Serializer &s);
extern void room_403_synchronize(Common::Serializer &s);
extern void room_404_synchronize(Common::Serializer &s);
extern void room_405_synchronize(Common::Serializer &s);
extern void room_406_synchronize(Common::Serializer &s);
extern void room_407_synchronize(Common::Serializer &s);
extern void room_408_synchronize(Common::Serializer &s);
extern void room_409_synchronize(Common::Serializer &s);
extern void room_410_synchronize(Common::Serializer &s);
extern void room_411_synchronize(Common::Serializer &s);
extern void room_412_synchronize(Common::Serializer &s);
extern void room_454_synchronize(Common::Serializer &s);
extern void room_501_synchronize(Common::Serializer &s);
extern void room_502_synchronize(Common::Serializer &s);
extern void room_503_synchronize(Common::Serializer &s);
extern void room_504_synchronize(Common::Serializer &s);
extern void room_505_synchronize(Common::Serializer &s);
extern void room_506_synchronize(Common::Serializer &s);
extern void room_507_synchronize(Common::Serializer &s);
extern void room_508_synchronize(Common::Serializer &s);
extern void room_509_synchronize(Common::Serializer &s);
extern void room_510_synchronize(Common::Serializer &s);
extern void room_511_synchronize(Common::Serializer &s);
extern void room_512_synchronize(Common::Serializer &s);
extern void room_557_synchronize(Common::Serializer &s);
extern void room_601_synchronize(Common::Serializer &s);
extern void room_603_synchronize(Common::Serializer &s);
extern void room_604_synchronize(Common::Serializer &s);
extern void room_605_synchronize(Common::Serializer &s);
extern void room_606_synchronize(Common::Serializer &s);
extern void room_607_synchronize(Common::Serializer &s);
extern void room_609_synchronize(Common::Serializer &s);
extern void room_612_synchronize(Common::Serializer &s);
extern void room_613_synchronize(Common::Serializer &s);
extern void room_614_synchronize(Common::Serializer &s);
extern void room_909_synchronize(Common::Serializer &s);

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
	case 6:
		section_preload_code_pointer = Rooms::section_6_preload;
		break;
	case 9:
		section_preload_code_pointer = Rooms::section_9_preload;
		break;
	}
}

void sync_room(Common::Serializer &s) {
	switch (new_room) {
	case 101: Rooms::room_101_synchronize(s); break;
	case 102: Rooms::room_102_synchronize(s); break;
	case 103: Rooms::room_103_synchronize(s); break;
	case 104: Rooms::room_104_synchronize(s); break;
	case 105: Rooms::room_105_synchronize(s); break;
	case 106: Rooms::room_106_synchronize(s); break;
	case 107: Rooms::room_107_synchronize(s); break;
	case 108: Rooms::room_108_synchronize(s); break;
	case 109: Rooms::room_109_synchronize(s); break;
	case 110: Rooms::room_110_synchronize(s); break;
	case 111: Rooms::room_111_synchronize(s); break;
	case 112: Rooms::room_112_synchronize(s); break;
	case 113: Rooms::room_113_synchronize(s); break;
	case 114: Rooms::room_114_synchronize(s); break;
	case 115: Rooms::room_115_synchronize(s); break;
	case 116: Rooms::room_116_synchronize(s); break;
	case 117: Rooms::room_117_synchronize(s); break;
	case 118: Rooms::room_118_synchronize(s); break;
	case 119: Rooms::room_119_synchronize(s); break;
	case 120: Rooms::room_120_synchronize(s); break;
	case 201: Rooms::room_201_synchronize(s); break;
	case 203: Rooms::room_203_synchronize(s); break;
	case 204: Rooms::room_204_synchronize(s); break;
	case 205: Rooms::room_205_synchronize(s); break;
	case 206: Rooms::room_206_synchronize(s); break;
	case 301: Rooms::room_301_synchronize(s); break;
	case 302: Rooms::room_302_synchronize(s); break;
	case 303: Rooms::room_303_synchronize(s); break;
	case 401: Rooms::room_401_synchronize(s); break;
	case 402: Rooms::room_402_synchronize(s); break;
	case 403: Rooms::room_403_synchronize(s); break;
	case 404: Rooms::room_404_synchronize(s); break;
	case 405: Rooms::room_405_synchronize(s); break;
	case 406: Rooms::room_406_synchronize(s); break;
	case 407: Rooms::room_407_synchronize(s); break;
	case 408: Rooms::room_408_synchronize(s); break;
	case 409: Rooms::room_409_synchronize(s); break;
	case 410: Rooms::room_410_synchronize(s); break;
	case 411: Rooms::room_411_synchronize(s); break;
	case 412: Rooms::room_412_synchronize(s); break;
	case 454: Rooms::room_454_synchronize(s); break;
	case 501: Rooms::room_501_synchronize(s); break;
	case 502: Rooms::room_502_synchronize(s); break;
	case 503: Rooms::room_503_synchronize(s); break;
	case 504: Rooms::room_504_synchronize(s); break;
	case 505: Rooms::room_505_synchronize(s); break;
	case 506: Rooms::room_506_synchronize(s); break;
	case 507: Rooms::room_507_synchronize(s); break;
	case 508: Rooms::room_508_synchronize(s); break;
	case 509: Rooms::room_509_synchronize(s); break;
	case 510: Rooms::room_510_synchronize(s); break;
	case 511: Rooms::room_511_synchronize(s); break;
	case 512: Rooms::room_512_synchronize(s); break;
	case 557: Rooms::room_557_synchronize(s); break;
	case 601: Rooms::room_601_synchronize(s); break;
	case 603: Rooms::room_603_synchronize(s); break;
	case 604: Rooms::room_604_synchronize(s); break;
	case 605: Rooms::room_605_synchronize(s); break;
	case 606: Rooms::room_606_synchronize(s); break;
	case 607: Rooms::room_607_synchronize(s); break;
	case 609: Rooms::room_609_synchronize(s); break;
	case 612: Rooms::room_612_synchronize(s); break;
	case 613: Rooms::room_613_synchronize(s); break;
	case 614: Rooms::room_614_synchronize(s); break;
	case 909: Rooms::room_909_synchronize(s); break;
	default: break;
	}
}

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
