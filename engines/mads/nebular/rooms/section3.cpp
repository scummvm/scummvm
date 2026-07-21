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

#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/global.h"
#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/global.h"
#include "mads/core/kernel.h"
#include "mads/core/pal.h"
#include "mads/core/player.h"
#include "mads/engine.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

extern void room_301_preload();
extern void room_302_preload();
extern void room_303_preload();
extern void room_304_preload();
extern void room_307_preload();
extern void room_308_preload();
extern void room_309_preload();
extern void room_310_preload();
extern void room_311_preload();
extern void room_313_preload();
extern void room_316_preload();
extern void room_318_preload();
extern void room_319_preload();
extern void room_320_preload();
extern void room_321_preload();
extern void room_322_preload();
extern void room_351_preload();
extern void room_352_preload();
extern void room_353_preload();
extern void room_354_preload();
extern void room_357_preload();
extern void room_358_preload();
extern void room_359_preload();
extern void room_360_preload();
extern void room_361_preload();
extern void room_366_preload();
extern void room_387_preload();
extern void room_388_preload();
extern void room_389_preload();
extern void room_390_preload();
extern void room_391_preload();
extern void room_399_preload();

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

void section_3_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(4));
}

void section_3_pre_parser() {
	player.need_to_walk = false;
}

void section_3_init() {
	player.scaling_velocity = true;
}

void section_3_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 301: room_preload_code_pointer = room_301_preload; break;
	case 302: room_preload_code_pointer = room_302_preload; break;
	case 303: room_preload_code_pointer = room_303_preload; break;
	case 304: room_preload_code_pointer = room_304_preload; break;
	case 307: room_preload_code_pointer = room_307_preload; break;
	case 308: room_preload_code_pointer = room_308_preload; break;
	case 309: room_preload_code_pointer = room_309_preload; break;
	case 310: room_preload_code_pointer = room_310_preload; break;
	case 311: room_preload_code_pointer = room_311_preload; break;
	case 313: room_preload_code_pointer = room_313_preload; break;
	case 316: room_preload_code_pointer = room_316_preload; break;
	case 318: room_preload_code_pointer = room_318_preload; break;
	case 319: room_preload_code_pointer = room_319_preload; break;
	case 320: room_preload_code_pointer = room_320_preload; break;
	case 321: room_preload_code_pointer = room_321_preload; break;
	case 322: room_preload_code_pointer = room_322_preload; break;
	case 351: room_preload_code_pointer = room_351_preload; break;
	case 352: room_preload_code_pointer = room_352_preload; break;
	case 353: room_preload_code_pointer = room_353_preload; break;
	case 354: room_preload_code_pointer = room_354_preload; break;
	case 357: room_preload_code_pointer = room_357_preload; break;
	case 358: room_preload_code_pointer = room_358_preload; break;
	case 359: room_preload_code_pointer = room_359_preload; break;
	case 360: room_preload_code_pointer = room_360_preload; break;
	case 361: room_preload_code_pointer = room_361_preload; break;
	case 366: room_preload_code_pointer = room_366_preload; break;
	case 387: room_preload_code_pointer = room_387_preload; break;
	case 388: room_preload_code_pointer = room_388_preload; break;
	case 389: room_preload_code_pointer = room_389_preload; break;
	case 390: room_preload_code_pointer = room_390_preload; break;
	case 391: room_preload_code_pointer = room_391_preload; break;
	case 399: room_preload_code_pointer = room_399_preload; break;
	}

	room_himem_preload(new_room, SECTION);
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

void section_3_preload() {
	section_init_code_pointer = section_3_init;
	section_room_constructor = section_3_constructor;
	section_music_reset_pointer = section_3_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
