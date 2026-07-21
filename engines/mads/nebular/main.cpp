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
#include "mads/nebular/main.h"
#include "mads/animview/animview.h"
#include "mads/textview/textview.h"
#include "mads/core/env.h"
#include "mads/core/error.h"
#include "mads/core/fileio.h"
#include "mads/core/game.h"
#include "mads/core/himem.h"
#include "mads/core/imath.h"
#include "mads/core/keys.h"
#include "mads/core/kernel.h"
#include "mads/core/magic.h"
#include "mads/core/matte.h"
#include "mads/core/mcga.h"
#include "mads/core/mouse.h"
#include "mads/core/pal.h"
#include "mads/core/player.h"
#include "mads/core/quote.h"
#include "mads/core/screen.h"
#include "mads/core/sound.h"
#include "mads/core/speech.h"
#include "mads/core/timer.h"
#include "mads/core/video.h"
#include "mads/nebular/main_menu.h"
#include "mads/nebular/menus.h"
#include "mads/engine.h"

namespace MADS {
namespace RexNebular {

constexpr bool SHOW_LINES = true;
constexpr byte LINE_COLOR = 2;

char *quotes;
static Palette black_palette;

static void main_menu_main() {
	auto &scr_screen = *g_engine->getScreen();
	Palette palette;
	int screenId, soundId;

	mcga_compute_retrace_parameters();
	memset(&black_palette, 0, sizeof(black_palette));

	pal_init(8, 8);
	pal_white(master_palette);

	buffer_init(&scr_work, 320, 156);
	viewing_at_y = (200 - scr_work.y) >> 1;
	assert(scr_work.data);

	buffer_init(&scr_orig, 320, 156);
	assert(scr_orig.data);

	buffer_init(&scr_depth, 320, 156);
	assert(scr_depth.data);

	buffer_fill(scr_work, 0);
	buffer_fill(scr_orig, 0);
	buffer_fill(scr_depth, 15);

	screen_dominant_mode(mcga_mode);
	video_init(mcga_mode, -1);
	mouse_init(-1, mcga_mode);

	memset(&master_palette, 0, sizeof(master_palette));
	mcga_setpal(&master_palette);

	mouse_set_work_buffer(scr_work.data, scr_work.x);
	mouse_set_view_port_loc(0, viewing_at_y, scr_work.x, scr_work.y + viewing_at_y - 1);
	mouse_set_view_port(0, 0);

	timer_install();
	keys_install();
	matte_init(0xFFFF);

	if (viewing_at_y != 0) {
		scr_screen.hLine(0, viewing_at_y - 2, 319, LINE_COLOR);
		scr_screen.hLine(0, scr_work.y + viewing_at_y + 1, 319, LINE_COLOR);
	}

	menu_control();

	if (selected_item >= 0) {
		for (int i = 0; i < 3; i++) {
			magic_color_values[i] = 0;
			magic_color_flags[i] = 0;
		}

		mcga_getpal(&palette);
		magic_fade_to_grey(palette, nullptr, 0, 256, 0, 1, 1, 16);
	}

	kernel_unload_sound_driver();

	if (selected_item == 5) {
		buffer_free(&scr_depth);
		buffer_free(&scr_orig);
		buffer_free(&scr_work);

		int sectionNum = 7;
		if (imath_random(1, 1000) > 500) {
			screenId = 996;
			soundId = 9;
		} else {
			screenId = 995;
			sectionNum = 4;
			soundId = 12;
		}

		pal_init(1, 8);

		room = room_load(screenId, 0, nullptr, &scr_orig, &scr_depth, &scr_walk,
			&scr_special, &picture_map, &depth_map, &picture_resource,
			&depth_resource, -1, -1, 0);

		if (room) {
			mouse_hide();
			video_update(&scr_orig, 0, 0, 0, 0, 320, 200);

			g_engine->_soundManager->init(sectionNum);
			sound_queue(soundId);

			magic_fade_to_grey(master_palette, (byte *)&palette, 0, 256, 0, 1, 1, 16);

			mouse_init_cycle();
			bool flag1 = true;
			bool flag2 = false;
			long time = timer_read();

			while (!g_engine->shouldQuit() && flag1) {
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
				magic_fade_to_grey(master_palette, nullptr, 0, 256, 0, 1, 1, 16);
			}

			kernel_unload_sound_driver();

			keys_remove();
			timer_remove();
			mouse_hide();

			mouse_init(0, 3);
			video_init(3, -1);
			mcga_reset();
		}
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
	Palette palette;

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

			if (selected_item >= 0) {
				Common::fill(magic_color_values, magic_color_values + 3, 0);
				Common::fill(magic_color_flags, magic_color_flags + 3, 0);
				mcga_getpal(&palette);
				magic_fade_to_grey(palette, nullptr, 0, 256, 0, 1, 1, 16);
			}
			break;

		case 0:
			// Start Game
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
			// Intro
			AnimView::animview_main("@rexopen");
			selected_item = -1;
			break;

		case 3:
			// Credits
			TextView::textview_main("credits");
			selected_item = -1;
			break;

		case 4:
			// Quotes
			TextView::textview_main("quotes");
			selected_item = -1;
			break;

		case 17:
			// Endgame cutscene
			AnimView::animview_main("@rexend1");
			AnimView::animview_main("@rexend2");
			AnimView::animview_main("@rexend3");
			TextView::textview_main("ending4");
			selected_item = -1;
			break;

		case 33:
			TextView::textview_main("ending1");
			TextView::textview_main("ending2");
			TextView::textview_main("credits");
			selected_item = -1;
			break;

		case 5:
		default:
			// Exit
			return;
		}
	}
}

} // namespace RexNebular
} // namespace MADS
