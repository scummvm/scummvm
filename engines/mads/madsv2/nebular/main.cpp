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

#include "common/config-manager.h"
#include "gui/saveload.h"
#include "mads/madsv2/nebular/main.h"
#include "mads/madsv2/animview/animview.h"
#include "mads/madsv2/textview/textview.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/nebular/main_menu.h"
#include "mads/madsv2/nebular/menus.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

constexpr bool SHOW_LINES = true;
constexpr byte LINE_COLOR = 2;

char *quotes;
static int mainmenu_val1;
static void *mainmenu_ptr1;
static void *mainmenu_ptr2;
static Palette black_palette;

static void timer_function() {
	// TODO
}

static void main_menu_main() {
	auto &screen = *g_engine->getScreen();
	Palette palette;
	int screenId, soundId;

	mcga_compute_retrace_parameters();
	memset(&black_palette, 0, sizeof(black_palette));

	mainmenu_val1 = 0;
	pal_init(8, 8);
	pal_white(master_palette);

	buffer_init(&scr_work, 320, 156);
	viewing_at_y = (200 - scr_work.y) >> 1;
	if (!scr_work.data)
		error("mainmenu -- didn't get work screen.");

	mainmenu_ptr1 = nullptr;

	buffer_init(&scr_orig, 320, 156);
	if (!scr_orig.data)
		error("mainmenu -- didn't get orig screen.");

	buffer_init(&scr_depth, 320, 156);
	if (!scr_depth.data)
		error("mainmenu -- didn't get orig screen.");

	buffer_fill(scr_work, 0);
	buffer_fill(scr_orig, 0);
	buffer_fill(scr_depth, 15);

	// TODO: identify what disassembly calls "nullsub_1" corresponds to (called
	// with the mode value right before video_init/mouse_init in this exact spot).
	// screen_dominant_mode looked like the closest candidate (it's a no-op stub
	// taking a single mode int, called immediately before video_init/mouse_init
	// in the equivalent kernel_game_startup() sequence), so using that for now.
	screen_dominant_mode(mcga_mode);
	video_init(mcga_mode, -1);
	mouse_init(-1, mcga_mode);

	memset(&master_palette, 0, sizeof(master_palette));
	mcga_setpal(&master_palette);

	// TODO: sub_11E31(scr_work.data, scr_work.x) - unidentified call, please
	// confirm the target function before this is implemented.

	mouse_set_view_port_loc(0, viewing_at_y, scr_work.x, scr_work.y + viewing_at_y - 1);
	mouse_set_view_port(0, 0);

	timer_install();
	keys_install();
	matte_init(0xFFFF);
	timer_activate_low_priority(timer_function);

	if (SHOW_LINES && viewing_at_y != 0) {
		screen.hLine(0, viewing_at_y - 2, 319, LINE_COLOR);
		screen.hLine(0, scr_work.y + viewing_at_y + 1, 319, LINE_COLOR);
	}

	menu_control();

	if (selected_item >= 0) {
		for (int i = 0; i < 3; i++) {
			magic_color_values[i] = 0;
			magic_color_flags[i] = 0;
		}

		mcga_getpal(&palette);
		magic_fade_to_grey(palette, 0, 0x10, 1, 1, 0, 0, 0);
	}

	kernel_unload_sound_driver();

	if (selected_item == 5) {
		buffer_free(&scr_depth);
		buffer_free(&scr_orig);
		buffer_free(&scr_work);

		srand(timer_read_dos());

		char soundName[] = "#SOUND.007";
		if (imath_random(1, 1000) > 500) {
			screenId = 996;
			soundId = 9;
		} else {
			screenId = 995;
			soundName[strlen(soundName) - 1] = '4';
			soundId = 12;
		}

		pal_init(1, 8);

		RoomPtr room = room_load(screenId, 0, nullptr, &scr_orig, &scr_depth, &scr_walk,
			&scr_special, &picture_map, &depth_map, &picture_resource,
			&depth_resource, -1, -1, 0);

		if (room) {
			mouse_hide();
			video_update(&scr_orig, 0, 0, 0, 0, 320, 200);

			// TODO: kernel_load_sound_driver(-1, -1) - only two int arguments are
			// pushed in the disassembly, which doesn't match the current 5-param
			// (name, sound_card, address, type, irq) signature. Please confirm how
			// Rex Nebular's sound driver should be loaded here.

			sound_queue(soundId);

			magic_map_to_grey_ramp(&master_palette, 0x10, 1, 1, 0, (MagicGreyPtr)&palette);

			mouse_init_cycle();
			bool flag1 = true;
			bool flag2 = false;
			long time = timer_read();

			while (flag1) {
				mouse_begin_cycle(false);

				if (keys_any()) {
					keys_get();
					flag1 = false;
					flag2 = true;
				}

				long elapsed = timer_read() - time;
				if (elapsed > 900)
					flag1 = false;

				if (mouse_stop_stroke) {
					flag1 = false;
					flag2 = true;
				}

				mouse_end_cycle(false, true);
			}

			sound_queue(1);

			if (flag2) {
				memset(&master_palette, 0, sizeof(master_palette));
				mcga_setpal(&master_palette);
			} else {
				magic_fade_to_grey(master_palette, 0, 0x10, 1, 1, 0, 0, 0);
			}

			kernel_unload_sound_driver();

			keys_remove();
			timer_remove();
			mouse_hide();

			mouse_init(0, 3);
			video_init(3, -1);
			mcga_reset();
		}
		// If room_load failed, the keys_remove/timer_remove/mouse_init/video_init/
		// mcga_reset block above is intentionally skipped, matching the disassembly
		// (which jumps straight past it to the final buffer_free/mem_free cleanup).
	} else {
		keys_remove();
		timer_remove();
		mouse_hide();

		mouse_init(0, 3);
		video_init(3, -1);
		mcga_reset();
	}

	buffer_free(&scr_depth);
	buffer_free(&scr_orig);
	buffer_free(&scr_work);

	if (mainmenu_ptr2)
		mem_free(mainmenu_ptr2);
}

static void main_cold_data_init() {
	debugger_reset = game_debugger_reset;
	debugger_update = game_debugger;

	game_menu_routine = global_game_menu;
	game_menu_init = global_menu_system_init;
	game_menu_exit = global_menu_system_shutdown;
	game_emergency_save = global_emergency_save;

	Common::strcpy_s(config_file_name, "config.rex");
	Common::strcpy_s(save_game_key, "rex");
	Common::strcpy_s(restart_game_key, "rex");

	Common::strcpy_s(player.series_name, "RAL");
	player.walker_must_reload = true;
	player.walker_loads_first = false;
	player.walker_visible = true;
	player.scaling_velocity = true;

	Common::strcpy_s(kernel_cheating_password, "WIDEPIPE");
	kernel_cheating_allowed = strlen(kernel_cheating_password);

	kernel.cheating = gDebugLevel == 9 ? kernel_cheating_allowed : 0;
}

static void game_main(int argc, const char **argv) {
	int count;
	int mads_mode;
	const char *scan;

	pack_enable_pfab_explode();

	mads_mode = env_verify();

	new_section = 1;
	new_room = 101;
	player.x = 160;
	player.y = 78;

	player.target_facing = FACING_NORTH;

	game_cold_data_init();
	main_cold_data_init();
	g_engine->readConfigFile();
	global_load_config_parameters();

	if (argc >= 2) {
		for (count = 1; count < argc; count++) {
			if (strchr("-/", argv[count][0]) != NULL) {
				for (scan = argv[count] + 1; *scan != 0; scan++) {
					flag_parse(&scan);
				}
			} else if (argv[count][0] == '?') {
				show_logo();

				if (!mads_mode) env_search_mode = ENV_SEARCH_CONCAT_FILES;
				error_dump_file("*warn2.dat");
				goto done;
			}
		}
	}

	if (report_version) {
		show_version();
		goto done;
	}

	if (fileio_exist("global.hag")) {
		art_hags_are_on_hd = true;
	} else {
		art_hags_are_on_hd = false;
	}

	himem_startup();

	himem_shutdown();

	if (!mads_mode && (env_search_mode == ENV_SEARCH_MADS_PATH))
		error("false start");

	game_control();

done:
	global_unload_config_parameters();

	if (fileio_exist("config.for")) {
		global_write_config_file();
	}
	if (chain_flag && (win_status || force_chain) && (key_abort_level < 2)) {
		warning("TODO: chain_execute");
	} else {
		if (win_status) {
			debug("(Ending: %d)", win_status);
		}
	}
}

void nebular_main() {
	static const char *CMD_LINE[] = { nullptr, "-p" };

	pack_enable_pfab_explode();
	if (!env_verify())
		env_search_mode = ENV_SEARCH_CONCAT_FILES;

	if (ConfMan.getBool("start_game") || ConfMan.hasKey("save_slot"))
		selected_item = 0;
	else if (ConfMan.getBool("start_intro"))
		selected_item = 3;
	else
		selected_item = -1;

	while (!g_engine->shouldQuit()) {
		g_engine->getScreen()->clear();

		switch (selected_item) {
		case -1:
			main_menu_main();
			break;

		case 0:
			game_main(2, CMD_LINE);
			return;

		case 1: {
			// Resume savegame
			// Get a list of saves and choose the last one
			auto saves = g_engine->listSaves();
			if (!saves.empty())
				savegame_slot = saves.back().getSaveSlot();

			// Start the game, which will also load the selected savegame
			game_main(2, CMD_LINE);
			return;
		}

		case 2:
			// Restore savegame
			game_restore_flag = 2;
			game_main(2, CMD_LINE);
			return;

		case 3:
			AnimView::animview_main("@dragon");
			selected_item = -1;
			break;

		case 4:
			// Exit
			return;

		default:
			// Credits
			TextView::textview_main("credits");
			selected_item = -1;
			break;
		}
	}
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
