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
#include "mads/madsv2/phantom/main.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/phantom/main_menu.h"
#include "mads/madsv2/phantom/menus.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

constexpr bool SHOW_LINES = true;
constexpr byte LINE_COLOR = 2;

char *quotes;

static void main_menu_main() {
	auto &screen = *g_engine->getScreen();
	Palette palette;

	if (!kernel_game_startup(19, KERNEL_STARTUP_CURSOR | KERNEL_STARTUP_INTERRUPT | KERNEL_STARTUP_FONT,
		nullptr, nullptr)) {
		viewing_at_y = (200 - scr_work.y) >> 1;

		mouse_cursor_sprite(cursor, 7);
		mouse_show();
		mouse_force(280, 126);
		mouse_hide();

		mouse_cursor_sprite(cursor, 1);
		matte_init(0xFFFF);
		kernel_seq_init();
		kernel_message_init();
		kernel_animation_init();
		kernel_init_dynamic();

		picture_view_x = 0;
		picture_view_y = 0;

		quotes = quote_load(0, 68, 69, 70, 71, 72, 73, 74, 75, 76,
			77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
			87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
			97, 98, 99, 0);

		global_speech_load(9);
		bool valid = !kernel_room_startup(922, 0, nullptr, true, true);

		master_palette[4].r = 63;
		master_palette[4].g = 50;
		master_palette[4].b = 0;
		master_palette[5].r = 30;
		master_palette[5].g = 15;
		master_palette[5].b = 0;
		mcga_setpal_range(&master_palette, 4, 2);

		new_background = true;

		if (valid) {
			memset(&palette, 0, sizeof(palette));
			mcga_setpal(&palette);
			mouse_cursor_sprite(cursor, 1);

			if (SHOW_LINES && viewing_at_y != 0) {
				screen.hLine(0, viewing_at_y - 2, 319, LINE_COLOR);
				screen.hLine(0, scr_work.y + viewing_at_y + 1, 319, LINE_COLOR);
			}

			kernel_load_sound_driver("*#SOUND.PH9", 'N', 544, 0, 49);

			menu_control();

			if (selected_item >= 0) {
				// Zero out the first 3 entries of both magic color arrays
				for (int i = 0; i < 3; i++) {
					magic_color_values[i] = 0;
					magic_color_flags[i] = 0;
				}

				mcga_getpal(&palette);

				magic_fade_to_grey(palette, 0, 0x10, 1, 1, 0, 0, 0);
			}
		}

		free(quotes);
		kernel_unload_sound_driver();
		kernel_game_shutdown();
	}

	mcga_reset();
}

static void main_cold_data_init() {
	debugger_reset = game_debugger_reset;
	debugger_update = game_debugger;
	game_menu_routine = global_game_menu;
	game_menu_init = global_menu_system_init;
	game_menu_exit = global_menu_system_shutdown;
	game_emergency_save = global_emergency_save;

	Common::strcpy_s(config_file_name, "config.pha");
	Common::strcpy_s(save_game_key, "phan");
	Common::strcpy_s(restart_game_key, "phantom");

	Common::strcpy_s(player.series_name, "RAL");
	player.walker_must_reload = true;
	player.walker_loads_first = false;
	player.walker_visible = true;
	player.scaling_velocity = true;

	Common::strcpy_s(kernel_cheating_password, "WIDECHEW");
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

#if 0
	if (config_file.cd_version_installed) {
		strcpy(temp_buf_2, "x:\\forest.exe");
		temp_buf_2[0] = env_cd_drive;
		if (!fileio_exist(temp_buf_2)) {
			problem();
			printf("Please throw the Once Upon A Forest CD into drive %c:\n", (char)env_cd_drive);
			printf("and type 'OUAF'. If your CD-ROM drive letter has changed, run\n");
			printf("INSTALL to reconfigure this option.\n\n");
			goto done;
		}
	}

	if (!fileio_exist("config.for")) {
		problem();
		printf("Before you can run Once Upon A Forest, you need to run 'INSTALL' to configure\n");
		printf("the game for you your hardware.  Type 'INSTALL' and hit 'ENTER' when you have\n");
		printf("finished reading this.\n\n");
		goto done;
	}

	mem_you_got = mem_used + mem_avail_at_start;

	if (mem_you_got < 569000) {
		need_to_free = 569000 - mem_you_got;
		problem();
		printf("You need at least 588,000 bytes of conventional memory to play\n");
		printf("Once Upon A Forest. You'll need to free up another %6ld bytes.\n\n", need_to_free);
		/* printf("to play this game.\n\n"); */
		goto done;
	}
#endif
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

void phantom_main() {
	static const char *CMD_LINE[] = { nullptr, "-p" };

	pack_enable_pfab_explode();
	if (!env_verify())
		env_search_mode = ENV_SEARCH_CONCAT_FILES;

	bool firstTime = !ConfMan.getBool("start_game") && !ConfMan.hasKey("save_slot");
	selected_item = 0;

	while (!g_engine->shouldQuit()) {
		if (firstTime) {
			main_menu_main();
			firstTime = false;
		}

		if (!g_engine->shouldQuit()) {
			switch (selected_item) {
			case 0:
				game_main(2, CMD_LINE);
				return;

			case 4:
				// Exit
				return;

			default:
				break;
			}
		}
	}
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
