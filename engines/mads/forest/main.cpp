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
#include "mads/forest/main.h"
#include "mads/animview/animview.h"
#include "mads/textview/textview.h"
#include "mads/core/env.h"
#include "mads/core/error.h"
#include "mads/core/fileio.h"
#include "mads/core/game.h"
#include "mads/core/himem.h"
#include "mads/core/kernel.h"
#include "mads/core/magic.h"
#include "mads/core/matte.h"
#include "mads/core/mcga.h"
#include "mads/core/mouse.h"
#include "mads/core/pal.h"
#include "mads/core/player.h"
#include "mads/core/quote.h"
#include "mads/core/speech.h"
#include "mads/forest/menus.h"
#include "mads/mads.h"

namespace MADS {
namespace Forest {

constexpr bool SHOW_LINES = true;
constexpr byte LINE_COLOR = 2;

char *quotes;

static void main_cold_data_init() {
	debugger_reset = game_debugger_reset;
	debugger_update = game_debugger;
	game_menu_routine = global_game_menu;
	game_menu_init = global_menu_system_init;
	game_menu_exit = global_menu_system_shutdown;
	game_emergency_save = global_emergency_save;

	Common::strcpy_s(config_file_name, "config.for");
	Common::strcpy_s(save_game_key, "oauf");
	Common::strcpy_s(restart_game_key, "oauf");

	Common::strcpy_s(player.series_name, "B");
	player.walker_must_reload = true;
	player.walker_loads_first = false;
	player.walker_visible = true;
	player.scaling_velocity = true;

	Common::strcpy_s(kernel_cheating_password, "LLAMA");
	kernel_cheating_allowed = strlen(kernel_cheating_password);

	kernel.cheating = gDebugLevel == 9 ? kernel_cheating_allowed : 0;
}

static void game_main(int argc, const char **argv) {
	int count;
	int mads_mode;
	const char *scan;

	pack_enable_pfab_explode();

	mads_mode = env_verify();

	new_section = 9;
	new_room = 901;
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

void forest_main() {
	static const char *CMD_LINE[] = { nullptr, "-p" };

	pack_enable_pfab_explode();
	if (!env_verify())
		env_search_mode = ENV_SEARCH_CONCAT_FILES;

	game_main(2, CMD_LINE);
}

} // namespace Foreste
} // namespace MADS
