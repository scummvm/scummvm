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
// Section preloads
extern void section_1_preload();
extern void section_2_preload();
extern void section_3_preload();
extern void section_4_preload();
extern void section_5_preload();
extern void section_6_preload();
extern void section_7_preload();
extern void section_8_preload();

// Room syncs
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
extern void room_150_synchronize(Common::Serializer &s);
extern void room_201_synchronize(Common::Serializer &s);
extern void room_202_synchronize(Common::Serializer &s);
extern void room_203_synchronize(Common::Serializer &s);
extern void room_204_synchronize(Common::Serializer &s);
extern void room_205_synchronize(Common::Serializer &s);
extern void room_206_synchronize(Common::Serializer &s);
extern void room_207_synchronize(Common::Serializer &s);
extern void room_208_synchronize(Common::Serializer &s);
extern void room_250_synchronize(Common::Serializer &s);
extern void room_301_synchronize(Common::Serializer &s);
extern void room_302_synchronize(Common::Serializer &s);
extern void room_303_synchronize(Common::Serializer &s);
extern void room_304_synchronize(Common::Serializer &s);
extern void room_305_synchronize(Common::Serializer &s);
extern void room_306_synchronize(Common::Serializer &s);
extern void room_307_synchronize(Common::Serializer &s);
extern void room_308_synchronize(Common::Serializer &s);
extern void room_309_synchronize(Common::Serializer &s);
extern void room_310_synchronize(Common::Serializer &s);
extern void room_401_synchronize(Common::Serializer &s);
extern void room_403_synchronize(Common::Serializer &s);
extern void room_404_synchronize(Common::Serializer &s);
extern void room_406_synchronize(Common::Serializer &s);
extern void room_407_synchronize(Common::Serializer &s);
extern void room_408_synchronize(Common::Serializer &s);
extern void room_409_synchronize(Common::Serializer &s);
extern void room_410_synchronize(Common::Serializer &s);
extern void room_453_synchronize(Common::Serializer &s);
extern void room_456_synchronize(Common::Serializer &s);
extern void room_501_synchronize(Common::Serializer &s);
extern void room_502_synchronize(Common::Serializer &s);
extern void room_504_synchronize(Common::Serializer &s);
extern void room_505_synchronize(Common::Serializer &s);
extern void room_506_synchronize(Common::Serializer &s);

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

void sync_room(Common::Serializer &s) {
	switch (new_room) {
	case 101:
		Rooms::room_101_synchronize(s);
		break;
	case 102:
		Rooms::room_102_synchronize(s);
		break;
	case 103:
		Rooms::room_103_synchronize(s);
		break;
	case 104:
		Rooms::room_104_synchronize(s);
		break;
	case 105:
		Rooms::room_105_synchronize(s);
		break;
	case 106:
		Rooms::room_106_synchronize(s);
		break;
	case 107:
		Rooms::room_107_synchronize(s);
		break;
	case 108:
		Rooms::room_108_synchronize(s);
		break;
	case 109:
		Rooms::room_109_synchronize(s);
		break;
	case 110:
		Rooms::room_110_synchronize(s);
		break;
	case 111:
		Rooms::room_111_synchronize(s);
		break;
	case 112:
		Rooms::room_112_synchronize(s);
		break;
	case 113:
		Rooms::room_113_synchronize(s);
		break;
	case 114:
		Rooms::room_114_synchronize(s);
		break;
	case 150:
		Rooms::room_150_synchronize(s);
		break;
	case 201:
		Rooms::room_201_synchronize(s);
		break;
	case 202:
		Rooms::room_202_synchronize(s);
		break;
	case 203:
		Rooms::room_203_synchronize(s);
		break;
	case 204:
		Rooms::room_204_synchronize(s);
		break;
	case 205:
		Rooms::room_205_synchronize(s);
		break;
	case 206:
		Rooms::room_206_synchronize(s);
		break;
	case 207:
		Rooms::room_207_synchronize(s);
		break;
	case 208:
		Rooms::room_208_synchronize(s);
		break;
	case 250:
		Rooms::room_250_synchronize(s);
		break;
	case 301:
		Rooms::room_301_synchronize(s);
		break;
	case 302:
		Rooms::room_302_synchronize(s);
		break;
	case 303:
		Rooms::room_303_synchronize(s);
		break;
	case 304:
		Rooms::room_304_synchronize(s);
		break;
	case 305:
		Rooms::room_305_synchronize(s);
		break;
	case 306:
		Rooms::room_306_synchronize(s);
		break;
	case 307:
		Rooms::room_307_synchronize(s);
		break;
	case 308:
		Rooms::room_308_synchronize(s);
		break;
	case 309:
		Rooms::room_309_synchronize(s);
		break;
	case 310:
		Rooms::room_310_synchronize(s);
		break;
	case 401:
		Rooms::room_401_synchronize(s);
		break;
	case 403:
		Rooms::room_403_synchronize(s);
		break;
	case 404:
		Rooms::room_404_synchronize(s);
		break;
	case 406:
		Rooms::room_406_synchronize(s);
		break;
	case 407:
		Rooms::room_407_synchronize(s);
		break;
	case 408:
		Rooms::room_408_synchronize(s);
		break;
	case 409:
		Rooms::room_409_synchronize(s);
		break;
	case 410:
		Rooms::room_410_synchronize(s);
		break;
	case 453:
		Rooms::room_453_synchronize(s);
		break;
	case 456:
		Rooms::room_456_synchronize(s);
		break;
	case 501:
		Rooms::room_501_synchronize(s);
		break;
	case 502:
		Rooms::room_502_synchronize(s);
		break;
	case 504:
		Rooms::room_504_synchronize(s);
		break;
	case 505:
		Rooms::room_505_synchronize(s);
		break;
	case 506:
		Rooms::room_506_synchronize(s);
		break;

	default:
		break;
	}
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
