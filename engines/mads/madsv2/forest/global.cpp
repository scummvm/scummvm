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

#include "common/textconsole.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

int16 flags[40];

namespace Rooms {

// Section preloads
extern void section_1_preload();
extern void section_2_preload();
extern void section_3_preload();
extern void section_4_preload();
extern void section_5_preload();
extern void section_9_preload();

extern void room_101_synchronize(Common::Serializer &s);
extern void room_101_synchronize(Common::Serializer &s);
extern void room_103_synchronize(Common::Serializer &s);
extern void room_104_synchronize(Common::Serializer &s);
extern void room_106_synchronize(Common::Serializer &s);
extern void room_107_synchronize(Common::Serializer &s);
extern void room_199_synchronize(Common::Serializer &s);
extern void room_201_synchronize(Common::Serializer &s);
extern void room_203_synchronize(Common::Serializer &s);
extern void room_204_synchronize(Common::Serializer &s);
extern void room_205_synchronize(Common::Serializer &s);
extern void room_210_synchronize(Common::Serializer &s);
extern void room_211_synchronize(Common::Serializer &s);
extern void room_220_synchronize(Common::Serializer &s);
extern void room_221_synchronize(Common::Serializer &s);
extern void room_301_synchronize(Common::Serializer &s);
extern void room_302_synchronize(Common::Serializer &s);
extern void room_303_synchronize(Common::Serializer &s);
extern void room_304_synchronize(Common::Serializer &s);
extern void room_305_synchronize(Common::Serializer &s);
extern void room_306_synchronize(Common::Serializer &s);
extern void room_307_synchronize(Common::Serializer &s);
extern void room_308_synchronize(Common::Serializer &s);
extern void room_321_synchronize(Common::Serializer &s);
extern void room_322_synchronize(Common::Serializer &s);
extern void room_401_synchronize(Common::Serializer &s);
extern void room_402_synchronize(Common::Serializer &s);
extern void room_403_synchronize(Common::Serializer &s);
extern void room_404_synchronize(Common::Serializer &s);
extern void room_405_synchronize(Common::Serializer &s);
extern void room_420_synchronize(Common::Serializer &s);
extern void room_501_synchronize(Common::Serializer &s);
extern void room_503_synchronize(Common::Serializer &s);
extern void room_509_synchronize(Common::Serializer &s);
extern void room_510_synchronize(Common::Serializer &s);
extern void room_520_synchronize(Common::Serializer &s);
extern void room_901_synchronize(Common::Serializer &s);
extern void room_903_synchronize(Common::Serializer &s);
extern void room_904_synchronize(Common::Serializer &s);

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
	case 9:
		section_preload_code_pointer = Rooms::section_9_preload;
		break;
	}
}

void sync_room(Common::Serializer &s) {
	switch (new_room) {
	case 101: Rooms::room_101_synchronize(s); break;
	case 103: Rooms::room_103_synchronize(s); break;
	case 104: Rooms::room_104_synchronize(s); break;
	case 106: Rooms::room_106_synchronize(s); break;
	case 107: Rooms::room_107_synchronize(s); break;
	case 199: Rooms::room_199_synchronize(s); break;
	case 201: Rooms::room_201_synchronize(s); break;
	case 203: Rooms::room_203_synchronize(s); break;
	case 204: Rooms::room_204_synchronize(s); break;
	case 205: Rooms::room_205_synchronize(s); break;
	case 210: Rooms::room_210_synchronize(s); break;
	case 211: Rooms::room_211_synchronize(s); break;
	case 220: Rooms::room_220_synchronize(s); break;
	case 221: Rooms::room_221_synchronize(s); break;
	case 301: Rooms::room_301_synchronize(s); break;
	case 302: Rooms::room_302_synchronize(s); break;
	case 303: Rooms::room_303_synchronize(s); break;
	case 304: Rooms::room_304_synchronize(s); break;
	case 305: Rooms::room_305_synchronize(s); break;
	case 306: Rooms::room_306_synchronize(s); break;
	case 307: Rooms::room_307_synchronize(s); break;
	case 308: Rooms::room_308_synchronize(s); break;
	case 321: Rooms::room_321_synchronize(s); break;
	case 322: Rooms::room_322_synchronize(s); break;
	case 401: Rooms::room_401_synchronize(s); break;
	case 402: Rooms::room_402_synchronize(s); break;
	case 403: Rooms::room_403_synchronize(s); break;
	case 404: Rooms::room_404_synchronize(s); break;
	case 405: Rooms::room_405_synchronize(s); break;
	case 420: Rooms::room_420_synchronize(s); break;
	case 501: Rooms::room_501_synchronize(s); break;
	case 503: Rooms::room_503_synchronize(s); break;
	case 509: Rooms::room_509_synchronize(s); break;
	case 510: Rooms::room_510_synchronize(s); break;
	case 520: Rooms::room_520_synchronize(s); break;
	case 901: Rooms::room_901_synchronize(s); break;
	case 903: Rooms::room_903_synchronize(s); break;
	case 904: Rooms::room_904_synchronize(s); break;
	default: break;
	}
}

void global_digi_play(int num) {
	// TODO
	warning("TODO: global_digi_play");
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
