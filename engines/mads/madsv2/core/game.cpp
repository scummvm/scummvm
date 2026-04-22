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
#include "common/debug.h"
#include "mads/madsv2/engine.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/mads.h"
#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/lib.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/echo.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/demo.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/lock.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/btype.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/copy.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/extra.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/screen.h"

namespace MADS {
namespace MADSV2 {

extern long mem_used;
extern long mem_avail_at_start;
extern int16 room_state[40];


#define MOVE_YOUR_BUTT_TIMEOUT 3600
#define MAX_SPEECH_FILES_PER_ROOM 100

Buffer scr_live = { video_y, video_x, nullptr };
char config_file_name[20];
int win_status = WIN_NOTHING;

int art_hags_are_on_hd;
byte game_restore_flag = false;         /* Flag if restoring game */
byte game_autosaved = false;         /* Flag if autosaved      */
byte game_mouse_cursor_fix = false;     /* Use special cursor fix */
word abort_value = 0;
long abort_clock;
char chain_line[80];
int  chain_flag = false;
int  force_chain = false;
int  key_abort_level = 0;
int  report_version = false;

void (*game_menu_routine)() = NULL;   /* Game Menu routines      */
void (*game_menu_init)() = NULL;
void (*game_menu_exit)() = NULL;
void (*game_emergency_save)() = NULL;

int debugger = false;
int debugger_state = DEBUGGER_MAIN;
int debugger_matte_before = false;
int debugger_memory_skip = 0;        /* For paging up/down      */
int debugger_memory_all = false;    /* Not showing ALL memory  */
int debugger_memory_keywait = false;    /* Not waiting for memory  */
void (*debugger_reset)() = NULL;     /* Debugger reset routine  */
void (*debugger_update)() = NULL;     /* Debugger update routine */
int int_sprite[6];
int selected_intro = false;
long correction_clock;

char save_game_key[8];
char restart_game_key[40];
char save_game_buf[20];

int last_keypressed = -1;

/* section specific overlays */
void (*section_preload_code_pointer)() = NULL;
void (*section_init_code_pointer)() = NULL;
void (*section_room_constructor)() = NULL;
void (*section_daemon_code_pointer)() = NULL;
void (*section_pre_parser_code_pointer)() = NULL;
void (*section_parser_code_pointer)() = NULL;
void (*section_error_code_pointer)() = NULL;
void (*section_music_reset_pointer)() = NULL;

/* room specific overlays */
void (*room_preload_code_pointer)() = NULL;
void (*room_init_code_pointer)() = NULL;
void (*room_daemon_code_pointer)() = NULL;
void (*room_pre_parser_code_pointer)() = NULL;
void (*room_parser_code_pointer)() = NULL;
void (*room_error_code_pointer)() = NULL;
void (*room_shutdown_code_pointer)() = NULL;

int game_any_emergency = false;

struct Heres_A {
	char speech_file[13];
	long size;
};

int game_keystroke;
int game_any_keystroke;

int lets_get_a_move_on_anim = true;

long noise_clock;
long noise_timer;
long noise_length = -1;

byte move_your_butt_enabled = 1;
long move_your_butt_clock;
long move_your_butt_timer;
int move_your_butt_anim_handle = -1;
int move_your_butt_anim_frame = -1;

byte last_bird_sound = (byte)-1;

char game_save_file[13] = "SAVES.DIR";   /* Save directory file      */
char *game_save_directory;                  /* Save directory pointer   */

int  game_preserve_handle;                      /* scr_depth preserve       */

Heap game_menu_heap;                            /* Custom heap for menu     */
Popup *game_menu_popup;                     /* Popup structure for menu */

int debugger_previous = DEBUGGER_NONE;
int debugger_watch = 0;
int debugger_watch_index[DEBUGGER_MAX_WATCH];

int  sound_board_address = 0;
int  sound_board_type = 0;
int  sound_board_irq = 0;

static int previous_running = -1000;

static void game_control_loop();


void game_cold_data_init() {
	game.going = true;

	kernel.fx = 0;
	kernel.video_mode = mcga_mode;
	kernel.translating = false;
	kernel.sound_card = 'R';
	kernel.force_restart = false;

	kernel.cheating = (byte)kernel_cheating_forbidden;

	kernel.cursor_x[0] = video_x >> 1;
	kernel.cursor_x[1] = 8;
	kernel.cursor_y[0] = display_y >> 1;
	kernel.cursor_y[1] = inter_base_y + 5;

	game.difficulty = -1;

	kernel.paused = false;
	kernel.cause_pause = false;
	kernel.frame_by_frame = false;

	kernel.mouse_cursor_point = false;
	kernel.memory_tracking = false;

	kernel.teleported_in = false;
	kernel.disable_fastwalk = false;

	game.last_save = -1;

	Common::strcpy_s(kernel.interface, "*I0.AA");

	magic_low_fade_bound = 21;
	magic_high_fade_bound = 251;
}


static int scan_past(const char **myscan, char scan) {
	int found = false;

	while (**myscan && (**myscan != scan)) (*myscan)++;
	if ((scan) && (**myscan == scan)) {
		(*myscan)++;
		found = true;
	}
	if (!**myscan) {
		(*myscan)--;
	}

	return found;
}

void flag_parse(const char **myscan) {
	long mem_max;
	// long mem_avail;
	if (kernel.cheating == (byte)kernel_cheating_forbidden) {
		if (scumm_stricmp(*myscan, kernel_cheating_password) == 0) {
			kernel.cheating = (byte)kernel_cheating_allowed;
			scan_past(myscan, 0);
			goto done;
		}
	}

	switch (toupper((int)**myscan)) {
	case 'A':
		if (scan_past(myscan, ':')) {
			chain_flag = true;
			Common::strcpy_s(chain_line, *myscan);
			scan_past(myscan, 0);
		}
		break;

		// case 'A':
		// if (scan_past(myscan, ':')) {
		// abort_value = atoi(*myscan);
		// scan_past(myscan, 0);
		// }
		// break;
	case 'C':
		if (scan_past(myscan, ':')) {
			kernel.sound_card = **myscan;
			if (scan_past(myscan, ',')) {
				// pl sound_board_address = xtoi(*myscan);
				if (scan_past(myscan, ',')) {
					// pl sound_board_type  = xtoi(*myscan);
					scan_past(myscan, 0);
				}
			}
		}
		break;

	case 'D':
		if (scan_past(myscan, ':')) {
			game.difficulty = (byte)atoi(*myscan);
			scan_past(myscan, 0);
		}
		break;

	case 'F':
		if (kernel.cheating) {
			kernel.frame_by_frame = true;
		}
		break;

	case 'H':
		if (scan_past(myscan, ':')) {
			while (**myscan) {
				switch (toupper(**myscan)) {
				case 'E':
					himem_preload_ems_disabled = true;
					break;
				case 'X':
					himem_preload_xms_disabled = true;
					break;
				case 'U':
					xms_disabled = true;
					break;
				}
				(*myscan)++;
			}
			scan_past(myscan, 0);
		} else {
			himem_preload_ems_disabled = true;
			himem_preload_xms_disabled = true;
		}
		break;

	case 'I':
		selected_intro = true;
		break;

	case 'K':
		inter_report_hotspots = true;
		// config_file.interface_hotspots = INTERFACE_BRAINDEAD;
		// inter_report_hotspots = !inter_report_hotspots;
		break;

	case 'L':
#ifdef debug_enable_logfiles
		logfile_enabled = true;
#endif
		break;

	case 'M':
		if (scan_past(myscan, ':')) {
			mem_max = atol(*myscan);
			mem_get_name(mem_max, "$HIDE$");
			// mem_avail = mem_get_avail() - mem_program_block();
			// if (mem_avail > mem_max) {
			// mem_get_name(mem_avail - mem_max, "$HIDE$");
			// }
			scan_past(myscan, 0);
		}
		break;

	case 'O':
		// config_file.inventory_mode = INVENTORY_SQUAT;
		// inter_spinning_objects = false;
		break;

	case 'P':
		env_search_mode = ENV_SEARCH_CONCAT_FILES;
		break;

	case 'R':
		if (scan_past(myscan, ':')) {
			game_save_name(atoi(*myscan));
			scan_past(myscan, 0);
			game_restore_flag = 1;
		}
		break;

	case 'S':
		game_restore_flag = 2;
		break;

	case 'T':
		if (kernel.cheating) {
			if (scan_past(myscan, ':')) {
				kernel.teleported_in = true;
				new_room = atoi(*myscan);
				new_section = new_room / 100;
				if (scan_past(myscan, ',')) {
					room_id = atoi(*myscan);
					section_id = room_id / 100;
					if (scan_past(myscan, ',')) {
						player.target_facing = atoi(*myscan);
						scan_past(myscan, 0);
					}
				}
			}
		}
		break;

	case 'U':
		game_mouse_cursor_fix = true;
		break;

	case 'V':
		report_version = true;
		break;

	case 'Y':
		// lock_hash_value = 0;
		break;

	case 'Z':
		if (kernel.cheating) {
			debugger = true;
			if (scan_past(myscan, ':')) {
				debugger_state = atoi(*myscan);
				if (debugger_state == DEBUGGER_MATTE) {
					debugger_matte_before = true;
				}
				if (debugger_state == DEBUGGER_MEMORY) {
					game_exec_function(debugger_reset);
					mem_manager_update = debugger_update;
				}
				if (debugger_state == DEBUGGER_PALETTE) {
					game_exec_function(debugger_reset);
					pal_manager_update = debugger_update;
				}
				if (scan_past(myscan, ':')) {
					while (**myscan) {
						switch (toupper((int)**myscan)) {
						case 'A':
							debugger_memory_all = true;
							break;
						case 'K':
							debugger_memory_keywait = true;
							break;
						}
						(*myscan)++;
					}
				}
				scan_past(myscan, 0);
			}
		}
		break;

	}

done:
	;
}

void show_version() {
	echo(" ", true);
	echo(global_release_name, true);
	echo("  Release Version ", false);
	echo(global_release_version, false);
	echo(" -- ", false);
	echo(global_release_date, true);
	echo("  MADS Library Version ", false);
	echo(mads_dev_lib_version, false);
	echo(" -- ", false);
	echo(mads_dev_lib_date, true);
	echo("  Copyright (c) ", false);
	echo(global_release_copyright, false);
	echo(" by Sanctuary Woods Multimedia Corp.", true);
	echo(" ", true);
	// echo ("  FOR INTERNAL USE ONLY", true);
	// echo (" ", true);
}


void show_logo() {
	show_version();
}


void problem() {
	debug("\nA slight problem . . .\n\n");
}

static void show_walk() {
	int y, x;
	int xx, yy;
	int ox, oy;
	byte *scan;
	int walk;

	for (y = 0; y < display_y; y++) {
		ox = picture_map.pan_base_x;
		oy = y + picture_map.pan_base_y;
		scan = buffer_pointer(&scr_orig, ox, oy);
		for (x = 0; x < video_x; x++) {
			xx = x + picture_view_x;
			yy = y + picture_view_y;
			walk = attr_walk(&scr_walk, xx, yy);

			if (walk) {
				*scan = 2;
			} else if (*scan == 2) {
				*scan = 0;
			}

			scan++;
		}
	}

	matte_refresh_work();
}


static void game_fix_save_name() {
	char *mark;

	mads_strupr(save_game_buf);

	mark = strchr(save_game_buf, '.');
	if (mark != NULL) *mark = 0;
	Common::strcat_s(save_game_buf, ".SAV");
}


void game_save_name(int id) {
	Common::strcpy_s(save_game_buf, save_game_key);
	env_catint(save_game_buf, id, 3);
	game_fix_save_name();
}




static void game_player_status() {
	int image, flags, count;
	char work_buf[80];
	char temp_buf_3[80];

	image = -1;
	flags = -9;
	for (count = 0; count < (int)image_marker; count++) {
		if (image_list[count].segment_id == KERNEL_SEGMENT_PLAYER) {
			if (image_list[count].flags >= flags) {
				image = count;
				flags = image_list[count].flags;
			}
		}
	}
	Common::strcpy_s(temp_buf_3, "Room: ");
	Common::strcat_s(temp_buf_3, mads_itoa(room_id, work_buf, 10));
	Common::strcat_s(temp_buf_3, " (From: ");
	Common::strcat_s(temp_buf_3, mads_itoa(previous_room, work_buf, 10));
	Common::strcat_s(temp_buf_3, ")");

	popup_alert(20, "PLAYER GRAPHICS STATUS",
		"  ",
		temp_buf_3,
		// temp_buf,
		// temp_buf_2,
		// temp_buf_4,
		// temp_buf_5,
		NULL);
}



int game_parse_keystroke(int mykey) {
	int count;
	int16 temp;
	long big_temp;
	int16 move_object;
	int16 move_target;
	int16 change_flag;
	int current_mode;
	int num;
	int unpause = 0;
	int x = 0, y = 0;
	char temp_buf[80], temp_buf_2[80];

	if (kernel.cheating == (byte)kernel_cheating_allowed) {

		mykey = main_cheating_key(mykey);

		switch (mykey) {
		case ctrl_a_key:
			kernel.memory_tracking = (byte)(!kernel.memory_tracking);
			break;

		case ctrl_b_key:
			kernel.player_tracking = (byte)(!kernel.player_tracking);
			break;

		case ctrl_d_key:
			// temp = game.difficulty;
			// if (!popup_get_number (&temp, "CHANGE DIFFICULTY FACTOR", "New Factor:", 3)) {
			  // game.difficulty = (byte)temp;
			// }
			break;

		case ctrl_e_key:
			mcga_shakes = 350;
			break;

		case ctrl_f_key:
			kernel.frame_by_frame = (byte)!(int)kernel.frame_by_frame;
			kernel.paused = false;
			break;

		case ctrl_g_key:
			change_flag = 0;
			if (!popup_get_number(&change_flag, "CHANGE GLOBAL", "Flag #:", 3)) {
				if (change_flag >= 0) {
					Common::strcpy_s(temp_buf, "GLOBAL #");
					mads_itoa(change_flag, temp_buf_2, 10);
					Common::strcat_s(temp_buf, temp_buf_2);
					popup_get_number(&global[change_flag], temp_buf, "Value:", 5);
				}
			}
			break;

		case ctrl_j_key:
			kernel.cheating = 0;
			popup_alert(22, "Cheating disabled.", NULL);
			break;

		case ctrl_l_key:
			move_object = 0;
			if (!popup_get_number(&move_object, "Examine Object", "Object #:", 3)) {
				big_temp = move_object + 800;
				if (!popup_get_long(&big_temp, "Object Message", "Message #:", 5)) {
					object_examine(move_object, big_temp, 0);
				}
			}
			break;

		case ctrl_n_key:
			temp = 0;
			if (!popup_get_number(&temp, "Activate Conversation", "Conv #:", 3)) {
				conv_flush();
				conv_get(temp);
				conv_run(temp);
			}
			break;

		case ctrl_o_key:
			move_object = -1;
			if (active_inven >= 0) move_object = inven[active_inven];
			if (!popup_get_number(&move_object, "MOVE OBJ", "Obj #:", 3)) {
				if (move_object >= 0) {
					move_target = object[move_object].location;
					popup_get_number(&move_target, "MOVE OBJ TO", "Loc:", 3);
					inter_move_object(move_object, move_target);
					// kernel.force_restart = true;
				}
			}
			break;

		case ctrl_p_key:
			game_player_status();
			break;

		case ctrl_r_key:
			kernel_panning_speed = (kernel_panning_speed + 1) % 3;
			game_set_camera_speed();
			switch (kernel_panning_speed) {
			case PANNING_INSTANT:
				// popup_alert (22, "Pan INS", NULL);
				break;

			case PANNING_MEDIUM:
				// popup_alert (22, "Pan MEDIUM.", NULL);
				break;

			case PANNING_SMOOTH:
			default:
				// popup_alert (22, "Panning = SMOOTH.", NULL);
				break;
			}
			break;

			// case ctrl_r_key:
			// box_param.font_spacing = 1 - box_param.font_spacing;
			// break;
		case ctrl_t_key:
			popup_get_number(&new_room, "TELEPORT", "New Room:", 3);
			kernel.teleported_in = (byte)(mykey == ctrl_t_key);
			break;

		case ctrl_u_key:
			player.turn_to_facing = player_clockwise[player.turn_to_facing];
			break;

		case bksp_key:
			temp = false;
			for (count = KERNEL_MAX_ANIMATIONS - 1; (!temp) && (count >= 0); count--) {
				if (kernel_anim[count].anim != NULL) {
					temp = true;
					kernel_anim[count].frame = kernel_anim[count].anim->num_frames - 1;
				}
			}
			break;

		case ctrl_w_key:
			// popup_get_string (player.series_name, "WALKER SERIES", "Series:", 8);
			// player.force_series = (byte)strlen(player.series_name);
			// if (!scumm_stricmp(player.series_name, "NULL")) player.series_name[0] = 0;
			// kernel.force_restart = true;
			break;

		case ctrl_z_key:
			kernel.force_restart = true;
			break;

		case alt_b_key:
			// if (!popup_get_string (box_param.name, "POPUP BOX SERIES", "Series:", 16)) {
			  // kernel.force_restart = true;
			// }
			break;

		case ctrl_y_key:
			break;

		case alt_i_key:
			player.y--;
			break;

		case f10_key:
			break;

		case alt_d_key:
			mem_manager_update = NULL;
			pal_manager_update = NULL;
			if (debugger && (debugger_state != DEBUGGER_MAIN)) {
				debugger_state = DEBUGGER_MAIN;
			} else {
				debugger = !debugger;
			}
			game_exec_function(debugger_reset);
			break;

		case alt_h_key:
			debugger = true;
			debugger_state = DEBUGGER_HELP;
			mem_manager_update = NULL;
			pal_manager_update = NULL;
			break;

		case alt_p_key:
			debugger = true;
			debugger_state = DEBUGGER_PALETTE;
			mem_manager_update = NULL;
			pal_manager_update = debugger_update;
			break;

		case alt_t_key:
			debugger = true;
			debugger_state = DEBUGGER_STATE;
			mem_manager_update = NULL;
			pal_manager_update = debugger_update;
			break;

		case alt_l_key:
			debugger = true;
			debugger_state = DEBUGGER_SCRATCH;
			mem_manager_update = NULL;
			pal_manager_update = NULL;
			break;

		case alt_g_key:
			debugger = true;
			debugger_state = DEBUGGER_GLOBAL;
			mem_manager_update = NULL;
			pal_manager_update = NULL;
			break;

		case alt_n_key:
			debugger = true;
			debugger_state = DEBUGGER_CONVERSATION;
			mem_manager_update = NULL;
			pal_manager_update = NULL;
			break;

		case alt_o_key:
			debugger = true;
			debugger_state = DEBUGGER_MEMORY;
			mem_manager_update = debugger_update;
			pal_manager_update = NULL;
			break;

		case alt_y_key:
			if (!debugger || (debugger_state != DEBUGGER_MATTE)) {
				debugger_matte_before = false;
			} else {
				debugger_matte_before = !debugger_matte_before;
			}
			debugger = true;
			debugger_state = DEBUGGER_MATTE;
			mem_manager_update = debugger_update;
			pal_manager_update = NULL;
			break;

		case alt_f_key:
			game_exec_function(debugger_reset);
			break;

		case alt_a_key:
			//       if (debugger_watch < DEBUGGER_MAX_WATCH) {
			// temp = -1;
			// if (debugger_watch > 0) temp = debugger_watch_index[debugger_watch - 1];
			// if (!popup_get_number (&temp, "ADD GLOBAL WATCH", "Variable #:", 3)) {
			// if ((temp >= 0) && (temp < 500)) {
			// for (count = 0; count < debugger_watch; count++) {
			// if (debugger_watch_index[count] == temp) {
			// temp = -1;
			// }
			// }
			//
			// if (temp > 0) {
			// debugger_watch_index[debugger_watch++] = temp;
			// }
			// }
			// }
			// }
			break;

		case alt_e_key:
			if (debugger_watch) debugger_watch--;
			debugger_previous = DEBUGGER_NONE;
			break;

		case '{':
			if (debugger_memory_skip > 0) debugger_memory_skip--;
			break;

		case '}':
			if (debugger_memory_skip < 10) debugger_memory_skip++;
			break;

		case '\\':
			debugger_memory_all = !debugger_memory_all;
			break;

		case '?':
			debugger_memory_keywait = !debugger_memory_keywait;
			break;

		case 'r':
		case 'R':
			if (section_id != 9) {
				keys_remove();
				popup_alert(22, "Key int NO.", NULL);
			}
			break;

		case ins_key:
			keys_install();
			popup_alert(22, "Keyb int YES.", NULL);
			break;

		case alt_m_key:
			player.y++;
			break;

		case alt_k_key:
			player.x++;
			break;

		case alt_j_key:
			player.x--;
			break;


		case alt_c_key:
			player.commands_allowed = !player.commands_allowed;
			break;

		case alt_w_key:
			// player.walk_freedom  = !player.walk_freedom;
			// player.walk_anywhere = player.walk_freedom;
			// if (player.walk_freedom) {
			  // popup_alert (26, "Player walks anywhere.", NULL);
			// } else {
			  // popup_alert (26, "Player walk restricted.", NULL);
			// }
			break;

		case alt_v_key:
			player.walker_visible = !player.walker_visible;
			break;

		case alt_z_key:
			player.x = mouse_x + picture_view_x;
			player.y = MIN(mouse_y, display_y) + picture_view_y;
			break;


		case '[':
			camera_jump_to(MAX(0, picture_view_x - 10), picture_view_y);
			break;

		case ']':
			camera_jump_to(MIN(picture_view_x + 10, picture_map.total_x_size - video_x), picture_view_y);
			break;

		case '=':
			camera_jump_to(picture_view_x, MAX(0, picture_view_y - 10));
			break;

		case '\'':
			camera_jump_to(picture_view_x, MIN(picture_view_y + 10, picture_map.total_y_size - display_y));
			break;


		case ctrl_c_key:
			kernel.mouse_cursor_point = (byte)(!kernel.mouse_cursor_point);
			break;
#if 0
		case ctrl_s_key:
			Common::strcpy_s(temp_buf, "d322u001");

			if (!popup_get_string(temp_buf, "Speech Play", "File:", 14)) {
				Common::strcpy_s(temp_buf_2, "*");
				Common::strcat_s(temp_buf_2, temp_buf);
				Common::strcat_s(temp_buf_2, ".rac");

				// if exist RAC file
				if (env_exist(temp_buf_2)) {
					digi_play(temp_buf, 1);
					digi_trigger_dialog = false;

				} else {
					Common::strcpy_s(temp_buf_2, "*");
					Common::strcat_s(temp_buf_2, temp_buf);
					Common::strcat_s(temp_buf_2, ".raw");

					// if exist RAW
					if (env_exist(temp_buf_2)) {
						digi_play(temp_buf, 1);
						digi_trigger_dialog = false;

					} else {
						popup_alert(22, temp_buf, "does not exist!", NULL);
					}
				}
			}
			break;
#endif
		case alt_f1_key:
			if (room->front_y > room->back_y) room->front_y--;
			kernel_room_bound_dif = room->front_y - room->back_y;
			break;

		case alt_f2_key:
			room->front_y++;
			kernel_room_bound_dif = room->front_y - room->back_y;
			break;

		case alt_f3_key:
			if (room->front_scale > room->back_scale) room->front_scale--;
			kernel_room_scale_dif = room->front_scale - room->back_scale;
			break;

		case alt_f4_key:
			room->front_scale++;
			kernel_room_scale_dif = room->front_scale - room->back_scale;
			break;


		case alt_f5_key:
			room->back_y--;
			kernel_room_bound_dif = room->front_y - room->back_y;
			break;

		case alt_f6_key:
			if (room->back_y < room->front_y) room->back_y++;
			kernel_room_bound_dif = room->front_y - room->back_y;
			break;

		case alt_f7_key:
			room->back_scale--;
			kernel_room_scale_dif = room->front_scale - room->back_scale;
			break;

		case alt_f8_key:
			if (room->back_scale < room->front_scale) room->back_scale++;
			kernel_room_scale_dif = room->front_scale - room->back_scale;
			break;


		case 'I':
		case 'i':
			num = (mykey == 'i') ? 1 : 5;
			y   = conv_control.y[conv_control.person_speaking];
			if (y & POPUP_CENTER) y = 0;
			y = MAX(y - num, 0);
			conv_control.y[conv_control.person_speaking] = y;
			conv_regenerate_last_message();
			break;

		case 'M':
		case 'm':
			num = (mykey == 'm') ? 1 : 5;
			conv_control.y[conv_control.person_speaking] += num;
			conv_regenerate_last_message();
			break;

		case 'J':
		case 'j':
			num = (mykey == 'j') ? 1 : 5;
			x   = conv_control.x[conv_control.person_speaking];
			if (x & POPUP_CENTER) x = 0;
			x = MAX(x - num, 0);
			conv_control.x[conv_control.person_speaking] = x;
			conv_regenerate_last_message();
			break;

		case 'K':
		case 'k':
			num = (mykey == 'k') ? 1 : 5;
			conv_control.x[conv_control.person_speaking] += num;
			conv_regenerate_last_message();
			break;

		case ',':
		case '<':
			num = (mykey == ',') ? 1 : 5;
			x = conv_control.width[conv_control.person_speaking];
			x = MAX(x - num, 10);
			conv_control.width[conv_control.person_speaking] = x;
			conv_regenerate_last_message();
			break;

		case '.':
		case '>':
			num = (mykey == '.') ? 1 : 5;
			x = conv_control.width[conv_control.person_speaking];
			x = MIN(x + num, 35);
			conv_control.width[conv_control.person_speaking] = x;
			conv_regenerate_last_message();
			break;

		case 0:
			break;

		default:
			unpause++;
			break;
		}
	}

	if (kernel.cheating < (byte)kernel_cheating_allowed) {
		if (mykey == (kernel_cheating_password[kernel.cheating] - '@')) {
			kernel.cheating++;
			mykey = 0;
			if (kernel.cheating >= (byte)kernel_cheating_allowed) {
				popup_alert(22, "CHEATING ENABLED", NULL);
			}
		} else {
			kernel.cheating = 0;
		}
	}

	mykey = main_normal_key(mykey);

	switch (mykey) {
	case space_key:
		global[3] = true;  // player_hyperwalked
		if (!kernel.paused) {
			if (player.walking && (new_room == room_id) && (player.walk_off_edge == 0)) {
				if (!kernel.disable_fastwalk || (kernel.cheating == (byte)kernel_cheating_allowed)) {
					if (buffer_legal(scr_walk, room->xs,
						player.x, player.y,
						player.target_x, player.target_y) == LEGAL) {
						player.x = player.target_x;
						player.y = player.target_y;
					}
				}
			}
		} else {
			kernel.paused = false;
		}
		break;

	case esc_key:
	case f1_key:
		if (room_id != 199 && section_id != 9) {
			if (kernel.activate_menu) {
				kernel.activate_menu = GAME_NO_MENU;
			} else {
				kernel.activate_menu = GAME_MAIN_MENU;
				kernel.paused = false;
			}
		}
		break;

	case f2_key:
	case alt_s_key:
		if (room_id != 199 && section_id != 9) {
			kernel.activate_menu = GAME_SAVE_MENU;
		}
		break;

	case f3_key:
	case alt_r_key:
		if (room_id != 199) {
			kernel.activate_menu = GAME_RESTORE_MENU;
		}
		break;

	case f4_key:
		// kernel.activate_menu = GAME_SCORE_MENU;
		break;

	case f5_key:
		if (room_id != 199 && section_id != 9) {
			kernel.activate_menu = GAME_OPTIONS_MENU;
		}
		break;

	case f6_key:
		if (room_id != 199 && section_id != 9) {
			kernel.activate_menu = GAME_CD_MENU;
		}
		break;

	case alt_q_key:
	case alt_x_key:
	case ctrl_q_key:
		if (room_id != 199) {
			game.going = false;
			kernel.paused = false;
			key_abort_level = 2;
		}
		break;

	case tab_key:
		current_mode = (mouse_y > display_y) ? 1 : 0;
		kernel.cursor_x[current_mode] = mouse_x;
		kernel.cursor_y[current_mode] = mouse_y;
		mouse_force(kernel.cursor_x[1 - current_mode], kernel.cursor_y[1 - current_mode]);
		break;

	case ctrl_k_key:
		inter_report_hotspots = !inter_report_hotspots;
		// config_file.interface_hotspots = inter_report_hotspots ? INTERFACE_BRAINDEAD : INTERFACE_MACINTOSH;
		inter_init_sentence();
		break;

	case ctrl_v_key:
		Common::strcpy_s(temp_buf, "Ver: ");
		Common::strcat_s(temp_buf, global_release_version);
		Common::strcat_s(temp_buf, " ");
		Common::strcat_s(temp_buf, global_release_date);

		Common::strcpy_s(temp_buf_2, "Lib ver: ");
		Common::strcat_s(temp_buf_2, mads_dev_lib_version);
		Common::strcat_s(temp_buf_2, " ");
		Common::strcat_s(temp_buf_2, mads_dev_lib_date);

		popup_alert(28, "GAME RELEASE VER INFO",
			"  ",
			temp_buf,
			temp_buf_2,
			NULL);
		break;

		// case I_key:
		// case B_key:
		// case i_key:
		// case b_key:
		  // if (room_id != 199 && section_id != 9 &&
			  // player.commands_allowed &&
			  // !kernel.trigger &&
			  // inter_input_mode == INTER_LIMITED_SENTENCES &&
	//          !global[2]) {*/  /* inventory_is_displayed
			// display_inventory();
		  // }
		  // break;
	case 0:
		break;

	default:
		unpause++;
		break;
	}

	if (mykey >= 0) last_keypressed = mykey;

	if (unpause >= 2) {
		kernel.paused = false;
	}

	return (unpause >= 2);
}



void game_pause_mode() {
	int mykey = -1;
	long clock_save;
	long clock_now;

	clock_save = timer_read();

	kernel.paused = true;

	while (kernel.paused) {
		while (!keys_any() && !keys_special_button);
		while (keys_any()) {
			mykey = keys_get();
			game_parse_keystroke(mykey);
		}
		if (keys_special_button) kernel.paused = false;
	}

	kernel.cause_pause = false;

	clock_now = timer_read();
	if (clock_now - clock_save > player.frame_delay) {
		*timer_address = kernel.clock = clock_save + player.frame_delay;
	}

	kernel.paused = false;
}



void game_error_service() {
	if (inter_spinning_objects || inter_animation_running) {
		if (mem_last_alloc_failed) {
			error_dump_file("*warn1.dat");
		}
	}
}

static void game_wait_cursor() {
	cursor_id = 2;
	cursor_id = MIN(cursor_id, cursor->num_sprites);
	if (cursor_id != cursor_last) {
		mouse_cursor_sprite(cursor, cursor_id);
		cursor_last = cursor_id;
	}
}

void game_set_camera_speed() {
	switch (kernel_panning_speed) {
	case PANNING_INSTANT:
		camera_x.pan_velocity = CAMERA_DEFAULT_X_VELOCITY * 80;
		camera_y.pan_velocity = CAMERA_DEFAULT_Y_VELOCITY * 80;
		break;

	case PANNING_MEDIUM:
		camera_x.pan_velocity = CAMERA_DEFAULT_X_VELOCITY << 1;
		camera_y.pan_velocity = CAMERA_DEFAULT_Y_VELOCITY << 1;
		break;

	case PANNING_SMOOTH:
	default:
		camera_x.pan_velocity = CAMERA_DEFAULT_X_VELOCITY;
		camera_y.pan_velocity = CAMERA_DEFAULT_Y_VELOCITY;
		break;
	}
}

void game_control() {
	int count, color;
	int result = COPY_SUCCEED;
	bool aborted_conv = true;

	// Start up game level functions
	error_service_routine_2 = game_error_service;

	if (game_mouse_cursor_fix) {
		mouse_disable_scale();
	}

	game.going = (byte)!kernel_game_startup(mcga_mode, KERNEL_STARTUP_ALL_FLAGS,
		global_release_version, global_release_date);

	game_exec_function(game_menu_init);

	conv_system_init();

	if (ConfMan.hasKey("save_slot")) {
		// Flag to do a savegame load
		game_restore_flag = 1;

	} else {
		result = main_copy_verify();
		if (result == COPY_FAIL) {
			game.going = false;
			force_chain = true;
			game_restore_flag = false;
			// new_room    = 804;
			// global_init_code();
			// global[copy_protect_failed] = true;
			error_report(ERROR_COPY_PROTECTION, SEVERE, MODULE_LOCK, 0, 0);
		} else if (result == COPY_ESCAPE) {
			game.going = false;
			force_chain = true;
		}
	}

	kernel.clock = timer_read();

	if (abort_value > 0) {
		abort_clock = kernel.clock + ((long)abort_value * 60);
	}

	if (!game_restore_flag && (result != COPY_FAIL) && (result != COPY_ESCAPE)) {
		// Get difficulty level if new game
		if (!kernel.teleported_in && (game.difficulty == -1)) {
			// Difficulty menu
			if (g_engine->getGameID() == GType_Phantom)
				kernel.activate_menu = GAME_DIFFICULTY_MENU;
			game_exec_function(game_menu_routine);
			if (!game.going)
				return;

			game_wait_cursor();

			previous_section = 0;
			previous_room = 0;

			section_id = KERNEL_STARTING_GAME;
			room_id = KERNEL_STARTING_GAME;
		}
	}

	if (game.difficulty < 0) game.difficulty = HARD_MODE;

	if ((result != COPY_FAIL) && (result != COPY_ESCAPE)) {
		ems_paging_mode(EMS_PAGING_GLOBAL);
		global_init_code();
	}

	if (game_restore_flag && (result != COPY_FAIL) && (result != COPY_ESCAPE)) {
		if (game_restore_flag == 2) {
			save_game_buf[0] = 0;
			kernel.activate_menu = GAME_RESTORE_MENU;
			game_exec_function(game_menu_routine);
			if (!save_game_buf[0]) {
				game.going = false;
				force_chain = true;
			}
		} else {
			// Savegame load from GMM
			g_engine->loadGameState(ConfMan.getInt("save_slot"));
		}
	}

	// Game level control loop
	int_sprite[fx_int_journal] = -1;

	while (game.going) {

		// Start up next section
		kernel_mode = KERNEL_SECTION_PRELOAD;

		global_sound_driver();

		ems_paging_mode(EMS_PAGING_SECTION);

		global_section_constructor();
		game_exec_function(section_preload_code_pointer);

		game.going = (byte)!kernel_section_startup(new_section);

		// Load sound driver
		kernel_load_sound_driver(kernel.sound_driver, kernel.sound_card, sound_board_address, sound_board_type, sound_board_irq);

		kernel_mode = KERNEL_SECTION_INIT;

		game_exec_function(section_init_code_pointer);

		matte_init(true);


		// Section level control loop
		while ((new_section == section_id) && game.going) {

			// Load up next room
			kernel_mode = KERNEL_ROOM_PRELOAD;

			// Return of the Phantom has this hardcoded true, due to text cutscenes like
			// "5 Minutes Later" zeroing out the palette
			player.walker_must_reload = (byte)(g_engine->getGameID() == GType_Phantom ||
				!player.walker_is_loaded);

			quote_emergency = false;
			// vocab_emergency = false;
			game_wait_cursor();

#ifdef demo
			demo_verify();
#endif

			kernel.quotes = NULL;

			// vocab_init_active();
			kernel_init_dynamic();

			game_exec_function(section_room_constructor);

			player.commands_allowed = true;
			player.walker_visible = true;
			player.walk_anywhere = false;

			text_default_x = POPUP_CENTER;
			text_default_y = POPUP_CENTER;

			player_discover_room(new_room);

			// Reset interface mode
			inter_input_mode = INTER_BUILDING_SENTENCES;
			inter_force_rescan = true;

			scrollbar_active = 0;

			player.walker_loads_first = true;

			picture_view_x = 0;
			picture_view_y = 0;

			ems_paging_mode(EMS_PAGING_ROOM);

			kernel_initial_variant = 0;

			kernel.disable_fastwalk = false;

			game_exec_function(room_preload_code_pointer);

			if (player.walker_must_reload || !player.walker_loads_first) {

				if (player.walker_is_loaded) {
					player_dump_walker();
				}
#if 0
				if (int_sprite[fx_int_journal] != -1 && room_id != KERNEL_RESTORING_GAME) {
					matte_deallocate_series(int_sprite[fx_int_candle_on], true);
					matte_deallocate_series(int_sprite[fx_int_dooropen], true);
					matte_deallocate_series(int_sprite[fx_int_exit], true);
					matte_deallocate_series(int_sprite[fx_int_candle], true);
					matte_deallocate_series(int_sprite[fx_int_backpack], true);
					matte_deallocate_series(int_sprite[fx_int_journal], true);
					int_sprite[fx_int_journal] = -1;
				}

				g_engine->section_music(section_id);
#endif
				pal_init(KERNEL_RESERVED_LOW_COLORS, KERNEL_RESERVED_HIGH_COLORS);

				matte_init(true);
#if 0
				if (!player.walker_is_loaded) {
					int_sprite[fx_int_journal] = kernel_load_series("*journal", false);
					int_sprite[fx_int_backpack] = kernel_load_series("*backpack", false);
					int_sprite[fx_int_candle] = kernel_load_series("*candle", false);
					int_sprite[fx_int_exit] = kernel_load_series("*door", false);
					int_sprite[fx_int_dooropen] = kernel_load_series("*dooropen", false);
					int_sprite[fx_int_candle_on] = kernel_load_series("*candleon", false);
				}
#endif
			} else {
				player_preserve_palette();
#if 0
				extra_inven_preserve_palette();
#endif
			}

			pal_activate_shadow(&kernel_shadow_main);
			master_shadow->num_shadow_colors = 3;
			for (count = 0; count < master_shadow->num_shadow_colors; count++) {
				color = PAL_FORCE_SHADOW + count;
				master_shadow->shadow_color[count] = color;
			}

			if (!player.walker_is_loaded) {
				if (player.walker_loads_first) {

					game.going |= !player_load_series(NULL);

					player.walker_loaded_first = true;
				}
			}

			for (count = 0; count < master_shadow->num_shadow_colors; count++) {
				color = PAL_FORCE_SHADOW + count;
				color_status[color] = 0;
				// error_watch_point ("color_status", count, 0);
			}


			game.going = (byte)!kernel_room_startup(new_room, kernel_initial_variant, kernel.interface, false);

			camera_init_default();

			game_set_camera_speed();

			sound_queue_hold();

			// Load current player walker set
			if (!player.walker_is_loaded) {
				game.going |= !player_load_series(NULL);
				player.walker_loaded_first = false;
			}

			// Set up control structures for new room
			mouse_init_cycle();

			left_command = -1;  // Left mouse main verb
			left_inven = -1;  // Left mouse inventory
			left_action = -1;  // Left mouse secondary verb

			inter_init_sentence();

			player_set_final_facing();
			player.facing = player.turn_to_facing;

			player_cancel_command();

			kernel_mode = KERNEL_ROOM_INIT;

			// Use a graphics fade in for room transition
			if (kernel_screen_fade == SCREEN_FADE_SMOOTH) {
				// kernel.fx = MATTE_FX_CIRCLE_OUT_SLOW;
				kernel.fx = MATTE_FX_FADE_THRU_BLACK;
			} else if (kernel_screen_fade == SCREEN_FADE_FAST) {
				// kernel.fx = MATTE_FX_CIRCLE_OUT_SLOW;
				kernel.fx = MATTE_FX_FAST_THRU_BLACK;
			} else {
				// kernel.fx = MATTE_FX_CIRCLE_OUT_SLOW;
				kernel.fx = MATTE_FX_FAST_AND_FANCY;
			}

			kernel.trigger = 0;

			correction_clock = kernel.clock = timer_read();

			kernel_animation_init();

			if (previous_room == KERNEL_RESTORING_GAME) {
				camera_jump_to(camera_old_x_target, camera_old_y_target);
			}

			for (count = 0; count < IMAGE_INTER_LIST_SIZE; ++count) {
				image_inter_list[count].series_id = 0;
			}


			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;

			global_room_init();

			game_exec_function(room_init_code_pointer);
#if 0
			// paul - oh no! magic numbers!
			stamp_sprite_to_interface(BP_X, BP_Y, 1, int_sprite[fx_int_backpack]);
			if (global[5]) {  // candle_is_on
				stamp_sprite_to_interface(CANDLE_X, CANDLE_Y, 1, int_sprite[fx_int_candle_on]);
			} else {
				stamp_sprite_to_interface(CANDLE_X, CANDLE_Y, 1, int_sprite[fx_int_candle]);
			}
			stamp_sprite_to_interface(DOOR_X, DOOR_Y, 1, int_sprite[fx_int_exit]);

			if (room_id != 199) {  // Taranjeet, if this is not journal room
				stamp_sprite_to_interface(JOURNAL_X, JOURNAL_Y, 1, int_sprite[fx_int_journal]);
			}
#endif
			scr_work.data = buffer_pointer(&scr_main, 0, viewing_at_y);
			if (viewing_at_y) {
				buffer_rect_fill(scr_main, 0, 0, video_x, viewing_at_y, 0);
				buffer_rect_fill(scr_main, 0, viewing_at_y + scr_work.y, video_x, video_y, 0);
			}

			// pl conv_restore_running = -1;
			player.target_x = player.x;
			player.target_y = player.y;
			player.target_facing = player.turn_to_facing = player.facing;
			player_select_series();
			player_new_stop_walker();

			player.walker_been_visible = player.walker_visible;

			player.special_code = attr_special(&scr_special, player.x, player.y);

			// Reset player clock to make sure walker appears in first frame
			player.clock = kernel.clock;
			player_stationary_update();

			if (active_inven >= 0) {
				inter_spin_object(inven[active_inven]);
			} else {
				inter_turn_off_object();
			}

			// Just in case something goes horribly wrong.
			error_service_routine = game_emergency_save;

			// Call main loop as long as player remains in same room
			kernel_mode = KERNEL_ACTIVE_CODE;

			kernel.teleported_in = false;

			ems_paging_mode(EMS_PAGING_SYSTEM);

			if ((quote_emergency /* || vocab_emergency */) && !game_any_emergency) {
				room_id = previous_room;
				game_any_emergency = true;
				goto emergency;
			} else {
				game_any_emergency = false;
			}

			game_control_loop();

			// **********************************************************************************************
													   // LEAVE ROOM
			// **********************************************************************************************
			if (speech_system_active && speech_on)
				   speech_all_off();

emergency:
			game_wait_cursor();

			kernel_mode = KERNEL_ROOM_PRELOAD;

			if (!game.going)
				aborted_conv = conv_control.running;
			conv_abort();

			// Shutdown the current room structures
			if (kernel.quotes != NULL)
				mem_free(kernel.quotes);

			kernel_abort_all_animations();

			kernel.force_restart = false;

			inter_turn_off_object();
			kernel_unload_all_series();

			game_exec_function(room_shutdown_code_pointer);

			if (!player.walker_loaded_first) {
				player.walker_is_loaded = false;
				player.walker_must_reload = true;
			}

			kernel_room_shutdown();

			conv_flush();

			new_section = new_room / 100;

			// Flush all EMS/XMS preloads at the room level
			himem_flush(ROOM);

			if (!game.going && !win_status) {
				conv_control.running = aborted_conv;

				g_engine->saveAutosaveIfEnabled();
				game_autosaved = true;
			} else {
				game_autosaved = false;
			}
		}

		player_dump_walker();

		if (room_id == KERNEL_RESTORING_GAME) {
			kernel_room_series_marker = 0;
			kernel_unload_all_series();
		}

		if (room_id != KERNEL_RESTORING_GAME && g_engine->getGameID() == GType_Forest) {
			if (int_sprite[fx_int_journal] != -1) {
				matte_deallocate_series(int_sprite[fx_int_candle_on], true);
				matte_deallocate_series(int_sprite[fx_int_dooropen], true);
				matte_deallocate_series(int_sprite[fx_int_exit], true);
				matte_deallocate_series(int_sprite[fx_int_candle], true);
				matte_deallocate_series(int_sprite[fx_int_backpack], true);
				matte_deallocate_series(int_sprite[fx_int_journal], true);
				int_sprite[fx_int_journal] = -1;
			}
		}

		pal_unlock();

		game_wait_cursor();

		// pl kernel_unload_sound_driver();
		// Shut down current section
		kernel_section_shutdown();

		// Flush all EMS/XMS preloads at the section level
		himem_flush(SECTION);
	}

	// Shut down the game
	game_exec_function(game_menu_exit);

	kernel_game_shutdown();

	// pl conv_system_cleanup();
	mcga_reset();
}

void game_exec_function(void (*(target))()) {
	if (target)
		target();
}

/**
 * Perform any system maintenance for this frame.  Mostly involves
 * reading the keyboard.
 */
static void game_system_maintenance() {
	int unparsed_key = false;

	global[3] = false;  // player_hyperwalked

	if (keys_any()) {
		game_any_keystroke = true;
		game_keystroke = keys_get();
		unparsed_key = game_parse_keystroke(game_keystroke);
	} else {
		game_any_keystroke = false;
	}

	if (section_id != 9 && room_id != 199) {
		if (((mouse_status & 3) == 3) && player.commands_allowed) {
			// kernel.force_restart = true;
			kernel.activate_menu = GAME_MAIN_MENU;
			inter_init_sentence();
			inter_sentence_ready = false;
			matte_refresh_work();
		}
	}

	if (unparsed_key || inter_auxiliary_click) {
		conv_control.popup_clock = kernel.clock;
		inter_auxiliary_click = false;
	}
}


void do_interface_for_ouaf() {
	if (mouse_y > 156 &&
		mouse_stop_stroke &&
		player.commands_allowed &&
		/* global[4] == -1 && */
		!kernel.trigger &&
		/* player.command_ready && */
		inter_input_mode == INTER_LIMITED_SENTENCES &&
		!global[2]  /* inventory_is_displayed */
		/* pl conv_control.running < 0 */) {
		if (room_id == 199) {  // Taranjeet's Journal

			leave_journal();

		} else if (mouse_x < 64) {
			display_journal();
		} else if (mouse_x < 139) {
			display_inventory();
		} else if (mouse_x < 195) {
		} else if (mouse_x < 250) {
			solve_me_selected();
		} else {
			door_selected();
		}
	}


	if (kernel.trigger == 40) {
		display_inventory();
	}
}


/**
 * Calls, in proper order, all daemon code for this framing round.
 */
static void game_daemon_code() {
#if 0
	global[4] = -1;  // turn off global[player_selected_object]

	digi_read_another_chunk();

	if (global[9]) midi_loop();  // please loop the damn music

	if (section_id != 9) {
		do_interface_for_ouaf();
	}
#endif
	kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;

	game_exec_function(room_daemon_code_pointer);
	game_exec_function(section_daemon_code_pointer);

	global_daemon_code();

	if (kernel.trigger_mode == KERNEL_TRIGGER_DAEMON) {
		kernel.trigger = 0;
	}
	player.trigger = 0;
}


/**
 * Handles incoming player commands BEFORE any walk action can take
 * place.  Player walk for the command can be postponed by setting
 * player.ready_to_walk to false, and then returning it to true when
 * ready for the walk to begin (additional player commands are auto-
 * matically disabled during the interim period).  Player walk for
 * the command can be cancelled permanently by setting the value of
 * player.need_to_walk to false.
 */
static void game_handle_preparse() {
	// if (global[4] >= 0 || global[5]) player.command_ready = true;
	// if (global[4] >= 0 || global[5]) player.command_error = false;
	// if (global[4] >= 0 ) player.command_ready = true;
	// if (global[4] >= 0 ) player.command_error = false;
	if ((inter_input_mode == INTER_BUILDING_SENTENCES) ||
		(inter_input_mode == INTER_LIMITED_SENTENCES)) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_PREPARSE;

		// if (global[4] >= 0) {
		  // player_verb = -1;
		  // player_main_noun = -1;
		  // player_second_noun = -1;
		// }
		global_pre_parser_code();
		game_exec_function(section_pre_parser_code_pointer);
		game_exec_function(room_pre_parser_code_pointer);

		if (kernel.trigger_mode == KERNEL_TRIGGER_PREPARSE) {
			kernel.trigger = 0;
		}
	}
}



/**
 * Handles incoming player commands -- they are filtered down through
 * various levels of parser code and error code.
 */
static void game_handle_command() {
	int handled_this_one;
	int kernel_trigger_in;

	kernel_trigger_in = kernel.trigger;

	if (conv_control.running >= 0) {
		player.look_around = false;
		if ((conv_control.status == CONV_STATUS_WAIT_AUTO) ||
			(conv_control.status == CONV_STATUS_WAIT_ENTRY)) {
			player.commands_allowed = false;
		}
	}

	handled_this_one = false;
	if (kernel.trigger)
		player.command_ready = true;

	kernel.trigger_setup_mode = KERNEL_TRIGGER_PARSER;

	if (kernel.trigger && (kernel.trigger_mode == KERNEL_TRIGGER_PARSER) &&
			player.command_error) {
		error_report(ERROR_ORPHANED_TRIGGER, WARNING, MODULE_KERNEL, kernel.trigger, 0);
	}

	if (!player.command_error &&
			(player.command_ready || kernel.trigger)) {
		game_exec_function(room_parser_code_pointer);

		handled_this_one = !player.command_ready;
	}

	if (conv_control.running >= 0) {
		player.command_ready = false;
		goto done;
	}

	if (player.command_ready || kernel.trigger) {
		game_exec_function(section_parser_code_pointer);
		handled_this_one = !player.command_ready;
	}

	if ((player.command_ready || kernel.trigger) &&
		(!handled_this_one) && (!player.command_error)) {
		global_parser_code();
	}

	if (player.look_around) goto done;

	if (player.command_ready) {
		player.command_error = true;
		game_exec_function(room_error_code_pointer);
	}

	if (player.command_ready) {
		player.command_error = true;
		game_exec_function(section_error_code_pointer);
	}

	if (player.command_ready) {
		global_error_code();
	}

done:
	player.command_ready = false;

	if (kernel.trigger_mode == KERNEL_TRIGGER_PARSER) {
		if (kernel.trigger == kernel_trigger_in) {
			kernel.trigger = 0;
		}
	}

	if (conv_control.running >= 0) {
		if ((conv_control.status == CONV_STATUS_WAIT_AUTO) ||
			(conv_control.status == CONV_STATUS_WAIT_ENTRY)) {
			conv_update(true);
		}
	}
}

void do_looping_sounds() {
	switch (room_id) {
	case 304:
		// do water trickle
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 107);  // PLAY_MORE_TRICKLE
		break;

	case 305:
		// do bird crowd
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 109);  // DO_CROWD
		break;

	case 306:
		// if global[phineas_status] <= PHIN_IS_IN_CONTROL_AGAIN) {
		if (global[66] <= 2) {
			// bird crowd talking
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 117);  // DO_CROWD
		} else {
			// water flowing
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 118);  // DO_WATER
		}
		break;

	case 401:
		// do dragon noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 106);  // DRAGON_NOISE
		break;

	case 403:
		// do water noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 105);  // WATER
		break;

	case 404:
		// do dragon noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 110);  // DRAGON_NOISE
		break;

	case 405:
		// do dragon noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 106);  // DRAGON_NOISE
		break;

	case 210:
		// rush MORE_RUSH_RUSH
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 109);  // MORE_RUSH_RUSH
		break;

	}
}

/**
 * This is the main inner graphics control loop for the game,
 * and is executed once for each framing round.
 */
static void game_main_loop() {
	int id;
	int preparse_flag = false;
	int temp_message = 0;
	int temp_message_2 = 0;
	int temp_message_3 = 0;
	int temp_message_4 = 0;
	long one_clock, two_clock;
	static char temp_buf[20];
	static char temp_buf_2[20];
	static char temp_buf_3[20];
	static char temp_buf_4[20];

#if 0
	// these 3 are for the background efx
	int yy;
	long dif;
	if (global[10]) {  // please play the damn targets

		// this is for the background sound efx
		dif = kernel.clock - noise_clock;
		if ((dif >= 0) && (dif <= 4)) {
			noise_timer += dif;
		} else {
			noise_timer += 1;
		}
		noise_clock = kernel.clock;

		if (noise_length == -1) {
			if (room_id == 220 || room_id == 221 || room_id == 307 || room_id == 322 || room_id == 420 ||
				room_id == 308 || room_id == 204 || room_id == 211) {
				// night time
				noise_length = imath_random(50, 80);

			} else {
				// day time
				noise_length = imath_random(150, 220);
			}
		}

		if (noise_timer >= (noise_length + global[13])) {
			if (room_id == 220 || room_id == 221 || room_id == 307 || room_id == 322 || room_id == 420 ||
				room_id == 308 || room_id == 204 || room_id == 211) {
				// night time
				// digi_initial_volume (75);
				digi_trigger_effect = false;
				digi_play_build(220, '_', 5, 2);  // EFFECT
				// digi_trigger_ambiance = false;
			} else {
				// day time
				yy = imath_random(1, 6);
				while (last_bird_sound == (byte)yy) yy = imath_random(1, 6);

				last_bird_sound = (byte)yy;

				// digi_initial_volume (imath_random (25, 100));
				digi_trigger_effect = false;

				switch (yy) {
				case 1: digi_play_build(321, '_', 500, 2); break;  // EFFECT
				case 2: digi_play_build(321, '_', 501, 2); break;  // EFFECT
				case 3: digi_play_build(321, '_', 502, 2); break;  // EFFECT
				case 4: digi_play_build(321, '_', 503, 2); break;  // EFFECT
				case 5: digi_play_build(321, '_', 504, 2); break;  // EFFECT
				case 6: digi_play_build(321, '_', 505, 2); break;  // EFFECT

				}
			}
			// digi_trigger_ambiance = false;
			noise_length = -1;
			noise_timer = 0;
		}
	}

	if (player.walker_visible && player.commands_allowed && section_id != 9 && lets_get_a_move_on_anim &&
		!player.walking && !player.need_to_walk && move_your_butt_anim_handle == -1) {
		move_your_butt_enabled = 1;
	} else {
		move_your_butt_enabled = 0;
		move_your_butt_timer = 0;
	}

	if (move_your_butt_enabled) {

		// this is for the background sound efx
		dif = kernel.clock - move_your_butt_clock;
		if ((dif >= 0) && (dif <= 4)) {
			move_your_butt_timer += dif;
		} else {
			move_your_butt_timer += 1;
		}
		move_your_butt_clock = kernel.clock;

		if (move_your_butt_timer >= MOVE_YOUR_BUTT_TIMEOUT) {
			player.commands_allowed = false;
			player.walker_visible = false;
			player.clock = kernel.clock;
			move_your_butt_anim_handle = kernel_run_animation("*b_2t", 0);
			extra_change_animation(move_your_butt_anim_handle, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, move_your_butt_anim_handle, KERNEL_NOW, 0);

			switch (player.facing) {
			case 8:
				kernel_reset_animation(move_your_butt_anim_handle, 21);
				break;

			case 9:
				kernel_reset_animation(move_your_butt_anim_handle, 22);
				break;

			case 6:
				kernel_reset_animation(move_your_butt_anim_handle, 23);
				break;

			case 7:
				kernel_reset_animation(move_your_butt_anim_handle, 26);
				break;

			case 4:
				kernel_reset_animation(move_your_butt_anim_handle, 27);
				break;

			default:
				kernel_reset_animation(move_your_butt_anim_handle, 24);
				break;
			}

			do_looping_sounds();
		}
	}

	if (move_your_butt_anim_handle != -1) {
		if (kernel_anim[move_your_butt_anim_handle].frame != move_your_butt_anim_frame) {
			move_your_butt_anim_frame = kernel_anim[move_your_butt_anim_handle].frame;

			switch (move_your_butt_anim_frame) {

			case 24:  // ab in a 2
			case 28:  // ab in a 2
				// AB we need to keep moving, michelle needs those herbs
				digi_trigger_dialog = false;
				digi_play_build_ii('b', 4, 1);
				// digi_send_dialog_trigger = false;
				kernel_reset_animation(move_your_butt_anim_handle, 0);
				move_your_butt_anim_frame = 0;
				break;

			case 20:  // end of talking
				// end AB we need to keep moving, michelle needs those herbs
				dont_frag_the_palette();
				kernel_abort_animation(move_your_butt_anim_handle);
				move_your_butt_anim_handle = -1;

				move_your_butt_timer = 0;
				player.walker_visible = true;
				player.commands_allowed = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				player_demand_facing(FACING_SOUTH);

				if (config_file.misc2) {
					do_looping_sounds();
				}
				break;
			}
		}
	}


	// Clear any remaining player command message from screen
#endif

	if (inter_sentence_ready || !player.commands_allowed) {
		inter_init_sentence();
		inter_sentence_ready = false;
	}

	// Interface processing -- accept new commands
	if (!kernel.trigger && !player.trigger) {
		if (kernel_dynamic_changed) kernel_refresh_dynamic();
		inter_main_loop(player.commands_allowed && !player.need_to_walk && !kernel.fx);
	}

	// Get official timing clock (in 60th/sec) for this round
	if (!kernel.fx) {
		kernel.clock = timer_read();
	}

	// Update camera view if necessary
	camera_update();

	// If new command came in, accept it and set up to perform any necessary walking
	if (inter_sentence_ready &&
		player.commands_allowed &&
		!player.need_to_walk &&
		!kernel.trigger &&
		!player.trigger) {
		player_new_command();
		if (inter_input_mode != INTER_CONVERSATION) {
			global_verb_filter();
		}
		preparse_flag = true;
	}

	if (preparse_flag || /*global[4] >= 0 ||*/
		((kernel.trigger) && (kernel.trigger_mode == KERNEL_TRIGGER_PREPARSE))) {
		game_handle_preparse();
	}

	preparse_flag = false;

	player_new_walk();

	// Handle parser commands and parser triggers
	if ((player.command_ready && !player.walking && !player.need_to_walk && (player.facing == player.turn_to_facing) && !kernel.trigger) ||
		(kernel.trigger && (kernel.trigger_mode == KERNEL_TRIGGER_PARSER))/* ||
		global[4] >= 0*/) {
		game_handle_command();  /* room,sect,global parser*/
	}

	if (new_room != room_id) {
		conv_abort();
		kernel_doom_all_animations();
		goto skip_frame;
	}

	// Execute daemon code, and do system maintenance
	if (!kernel.trigger || (kernel.trigger_mode == KERNEL_TRIGGER_DAEMON)) {
		game_daemon_code();  // interface, room,sect,global daemon
	}

	game_system_maintenance();

	if (new_room != room_id) {
		conv_abort();
		kernel_doom_all_animations();
		goto skip_frame;
	}

	// Update all active conversations
	if (!kernel.trigger) {
		if (conv_control.running >= 0) {
			if (!camera_x.panning && !camera_y.panning) {
				conv_update(false);
			}
		}
	}

	// Update the player image, if it is time to do so
	if ((kernel.clock >= player.clock) && !kernel.trigger) {

		player.clock = kernel.clock + player.frame_delay;

		if (player.walking) {
			player_keep_walking();
		} else {
			player_stationary_update();
		}

		player_set_sprite();
		player_set_image();
	}

	cursor_id = 1;

	if ((inter_awaiting == AWAITING_COMMAND) && !mouse_button) {
		if (inter_spot_class == STROKE_INTERFACE) {
			id = inter_spot_index - spot_base[STROKE_INTERFACE - 1];
			if (id < room_num_spots) {
				id = room_num_spots - (id + 1);
				cursor_id = room_spots[id].cursor_number;
			} else {
				id -= room_num_spots;
				cursor_id = kernel_dynamic_hot[id].cursor;
			}
			if (!cursor_id) cursor_id = 1;
		}
	}
	if (!player.commands_allowed && ((conv_control.running < 0) || conv_control.status == CONV_STATUS_HOLDING))
		cursor_id = 2;
	if ((conv_control.running >= 0) && (cursor_id > 2))
		cursor_id = 1;

	if (section_id != 9 || room_id == 904) {
		cursor_id = MIN(cursor_id, cursor->num_sprites);
		if (cursor_id != cursor_last) {
			mouse_cursor_sprite(cursor, cursor_id);
			cursor_last = cursor_id;
		}
	}


	// Update any active graphics sequences
	kernel_seq_update_all();

	// Update any active animations
	kernel_process_all_animations();

	// Special mouse cursor update mode
	if (kernel.mouse_cursor_point) {
		Common::strcpy_s(temp_buf, "(");
		mads_itoa(mouse_x, &temp_buf[1], 10);
		Common::strcat_s(temp_buf, ",");
		mads_itoa(mouse_y, &temp_buf[strlen(temp_buf)], 10);
		Common::strcat_s(temp_buf, ") ");
		temp_message = kernel_message_add(temp_buf, 5, 5, 515, 1, 0, 0);
	}

	// if (kernel.memory_tracking) {
	  // Common::strcpy_s (temp_buf_2, "(Mem: ");
	  // ltoa (mem_get_avail(), &temp_buf_2[6], 10);
	  // Common::strcat_s (temp_buf_2, ") ");
	  // temp_message_2 = kernel_message_add (temp_buf_2, 315, 5, 515, 1, 0, KERNEL_MESSAGE_RIGHT);
	// }
	// if (kernel.player_tracking) {
	  // Common::strcpy_s (temp_buf_3, "(Plyr: ");
	  // mads_itoa (player.x, &temp_buf_3[strlen(temp_buf_3)], 10);
	  // Common::strcat_s (temp_buf_3, ",");
	  // mads_itoa (player.y, &temp_buf_3[strlen(temp_buf_3)], 10);
	  // Common::strcat_s (temp_buf_3, ")");
	  // yy = 5;
	  // if (kernel.mouse_cursor_point) yy += 15;
	  // temp_message_3 = kernel_message_add (temp_buf_3, 5, yy, 515, 1, 0, 0);
	  // Common::strcpy_s (temp_buf_4, "Scale %");
	  // mads_itoa (player.scale, &temp_buf_4[strlen(temp_buf_4)], 10);
	  // yy += 15;
	  // temp_message_4 = kernel_message_add (temp_buf_4, 5, yy, 515, 1, 0, 0);
	// }
	// Update any messages
	if (!kernel.trigger) {
		if (kernel.force_restart || (new_room != room_id)) kernel_message_purge();
		kernel_message_update_all();
	}

	// Matte out the next graphics frame
	if (!viewing_at_y) {
		matte_inter_frame(!kernel.fx, kernel.fx);
		if (kernel.fx) {
			inter_prepare_background();
		}
	} else {
		image_inter_marker = 0;
	}

	if (debugger && (debugger_state == DEBUGGER_MATTE) && debugger_matte_before) game_exec_function(debugger_update);

	matte_frame(kernel.fx, kernel.fx);

	if (debugger && ((debugger_state != DEBUGGER_MATTE) || !debugger_matte_before)) game_exec_function(debugger_update);

	if (kernel.fx) {
		one_clock = correction_clock;
		two_clock = timer_read();
		kernel_seq_correction(one_clock, two_clock);
		kernel_message_correction(one_clock, two_clock);
	}

	if (kernel.mouse_cursor_point) {
		kernel_message_delete(temp_message);
	}

	if (kernel.memory_tracking) {
		kernel_message_delete(temp_message_2);
	}

	if (kernel.player_tracking) {
		kernel_message_delete(temp_message_3);
		kernel_message_delete(temp_message_4);
	}

	if (kernel.cause_pause || kernel.frame_by_frame) game_pause_mode();

skip_frame:

	// if (mcga_shakes) mcga_shake();
	if (kernel.fx) {
		cycling_active = true;
	}

	kernel.fx = 0;

	kernel_abort_doomed_animations();

	while ((mouse_clock == timer_read()) && !kernel.trigger);
}

static void game_control_loop() {
	game_any_keystroke = false;

	if (debugger) game_exec_function(debugger_reset);

	while ((new_room == room_id) && game.going && !kernel.force_restart) {

		game_main_loop();

		if (kernel.activate_menu) {
			if (!kernel.trigger && player.commands_allowed) {
				game_exec_function(game_menu_routine);

				if (game_menu_routine == NULL) game.going = false;

				kernel.activate_menu = GAME_NO_MENU;
			}
		}

		if (abort_value > 0) {
			if (kernel.clock >= abort_clock) {
				error_report(ERROR_TIME_LIMIT_EXPIRED, SEVERE, MODULE_GAME, abort_value, 0);
			}
		}
	}
#if 0
	digi_stop(1);
	digi_stop(2);
	digi_stop(3);
#endif
	if (debugger) game_exec_function(debugger_reset);
}

void chain_execute() {
	error("TODO: chain_execute");
}

/**
 * Reads the list of save files.
 */
static void game_read_save_directory() {
	SaveStateList list = g_engine->listSaves();
	memset(game_save_directory, 0, GAME_MAX_SAVE_SLOTS * (GAME_MAX_SAVE_LENGTH + 1));

	for (auto it = list.begin(); it != list.end(); ++it) {
		char *slot = game_save_directory + it->getSaveSlot() * (GAME_MAX_SAVE_LENGTH + 1);
		Common::strcpy_s(slot, GAME_MAX_SAVE_LENGTH + 1, it->getDescription().c_str());
	}
}

void game_menu_setup() {
	long mem_to_get;

	game_preserve_handle = BUFFER_NOT_PRESERVED;
	game_menu_heap.base = NULL;

	// Get enough memory to work with.  If none available from the
	// main heap, attempt to preserve the depth buffer and steal
	// its memory temporarily.
	if (mem_get_avail() >= GAME_MENU_HEAP) {
		heap_create(&game_menu_heap, MODULE_GAME_MENU, GAME_MENU_HEAP, "MENUHEAP");
	} else if (scr_depth.data != NULL) {
		game_preserve_handle = buffer_preserve(&scr_depth, BUFFER_PRESERVE, -1,
			0, 0,
			scr_orig.x >> 1, scr_depth.y);
		if (game_preserve_handle != BUFFER_NOT_PRESERVED) {
			heap_declare(&game_menu_heap, MODULE_GAME_MENU,
				(char *)scr_depth.data, GAME_MENU_HEAP);
		}
	}

	if (game_menu_heap.base == NULL) {
		error_report(ERROR_NO_MORE_MEMORY, SEVERE, MODULE_GAME_MENU, GAME_MENU_HEAP, 2);
	}

	// Get memory for save slots
	mem_to_get = (long)GAME_MAX_SAVE_SLOTS * (GAME_MAX_SAVE_LENGTH + 1);
	game_save_directory = (char *)heap_get(&game_menu_heap, mem_to_get);

	// Get memory for popup
	mem_to_get = GAME_DIALOG_HEAP;
	game_menu_popup = (Popup *)heap_get(&game_menu_heap, mem_to_get);

	// Read the save list
	game_read_save_directory();

	cursor_id = 1;
	cursor_id = MIN(cursor_id, cursor->num_sprites);
	if (cursor_id != cursor_last) {
		mouse_cursor_sprite(cursor, cursor_id);
		cursor_last = cursor_id;
	}
}

void game_menu_shutdown() {
	heap_destroy(&game_menu_heap);

	if (game_preserve_handle != BUFFER_NOT_PRESERVED) {
		buffer_restore(&scr_depth, game_preserve_handle, -1,
			0, 0, scr_orig.x >> 1, scr_depth.y);
	}
}

static void debugger_name(const char *name, int where) {
	screen_printf(where, 0, "%s", name);
}

static void game_file_show(const char *filename, int show_at) {
	Common::SeekableReadStream *handle = NULL;

	handle = env_open(filename, "rt");
	if (handle == NULL) {
		screen_printf(0, 2, "No open %s", filename);
		goto done;
	}

	while (!handle->eos()) {
		Common::String line = handle->readLine();
		screen_printf(2, show_at++, "%-79s", line.c_str());
	}

done:
	delete handle;
}

void game_debugger_reset() {
	//screen = mono_text_video;

	screen_normal_color = colorbyte(hi_white, black);
	screen_hilite_color = screen_normal_color + 128;

	screen_clear(0);

	debugger_previous = DEBUGGER_NONE;
}

static void game_main_update() {
	char temp_buf[80];
	char temp_buf_2[80];

	Common::strcpy_s(temp_buf, "");
	Common::strcpy_s(temp_buf_2, "");

	debugger_name("1", 30);

	screen_printf(0, 2, "%-3d, %-3d Mem: %-6ld Min: %-6ld", room_id, previous_room, mem_get_avail(), mem_min_free);

	screen_printf(0, 4, "%s @ %3d, %3d Dpt: %d   ", player.series_name, player.x, player.y, player.depth);
	// screen_printf (0, 5, "        Series: %d   Sprite: %-2d   Mirror: %-2d   Frame Rate: %d    ", player.series, player.sprite, player.mirror, player.frame_delay);
	screen_printf(0, 7, "Sc: %-3d  Fr %d => %d%, Bk %d => %d%", player.scale, room->front_y, room->front_scale, room->back_y, room->back_scale);

	if (!player.walker_visible) {
		Common::strcat_s(temp_buf, "Invis ");
	}

	if (attr_walk(&scr_walk, player.x, player.y)) {
		Common::strcat_s(temp_buf, "WALKCODE!");
	}

	screen_printf(0, 9, "%-79s", temp_buf);

	screen_printf(0, 13, "%-6ld  Tr: %d  Mse: (%4d,%4d)", kernel.clock, kernel.trigger, mouse_x, mouse_y);

	if (kernel.frame_by_frame) {
		Common::strcat_s(temp_buf, "FxFrame ");
	}

	if (kernel.paused || (kernel.frame_by_frame && !keys_special_button)) {
		Common::strcat_s(temp_buf, "~P~A~U~S~E ");
	}

	screen_printf(0, 14, "%-79s", temp_buf);
}

void update_colors(int offset) {
}

static void game_palette_update() {
	int count;
	int count2;
	int x, y;
	long handle;
	long any_flag;
	long walker_flag;
	long picture_flag;
	long in_walker;
	long in_picture;
	long in_any;
	byte item;
	byte attr;
	byte *pointer;
	byte *pointer2;
	int free;
	int delta;
	int mykey;
	int shit = 0;
	static int old_free = 0;

	if (pal_manager_active && debugger_memory_keywait) {
		screen_normal_color = screen_hilite_color;
	}

	debugger_name("2", 23);

	screen_normal_color = colorbyte(hi_white, black);

	update_colors(0);

	game_file_show("*PALETTE.DB", 2);

	for (count = 0; count < 256; count++) {
		y = (count / 16) + 2;
		x = count % 16;

		pointer = screen + (y * 160) + (x << 1);
		pointer2 = pointer + 1;

		if (count < 4) {
			item = 'M';
		} else if (count < 16) {
			item = 'I';
		} else if (count < KERNEL_RESERVED_LOW_COLORS) {
			item = 'Q';
		} else if (count >= 252) {
			item = 'X';
		} else if (count >= 246) {
			item = 'O';
		} else if (color_status[count] & PAL_CYCLE) {
			item = 'c';
		} else if (color_status[count]) {
			handle = picture_resource.color_handle;
			picture_flag = 1 << handle;
			any_flag = picture_flag;

			walker_flag = 0;
			for (count2 = 0; count2 < player.num_series; count2++) {
				handle = series_list[count2 + player.series_base]->color_handle;
				walker_flag |= 1 << handle;
			}

			any_flag |= walker_flag;
			any_flag = ~any_flag;

			in_picture = color_status[count] & picture_flag;
			in_walker = color_status[count] & walker_flag;
			in_any = color_status[count] & any_flag;

			if (color_status[count] & in_picture) {
				if (color_status[count] & in_any) {
					if (color_status[count] & in_walker) {
						item = '!';
					} else {
						item = ':';
					}
				} else if (color_status[count] & in_walker) {
					item = '*';
				} else {
					item = '+';
				}
			} else if (color_status[count] & in_walker) {
				if (color_status[count] & in_any) {
					item = '%';
				} else {
					item = '/';
				}
			} else {
				item = '?';
			}
		} else {
			item = '-';
		}

		if (color_status[count] & PAL_RESERVED) {
			attr = 15;
		} else if (color_status[count] & PAL_CYCLE) {
			attr = 7 + 16;
		} else {
			attr = 7;
		}

		*pointer = item;
		*pointer2 = attr;
	}

	if (pal_manager_active) {

		if ((pal_manager_active != 2) && (pal_manager_active != 3)) {
			loader_last[0] = 0;
		}

		screen_printf(0, 20, "Ldr: %-13s", loader_last);
		screen_printf(0, 21, "Md: %d", pal_manager_active);
		screen_printf(0, 22, "Mode: %d", pal_manager_active);

		free = 0;
		for (count = 0; count < 256; count++) {
			if (!color_status[count]) free++;
		}

		if (pal_manager_active == 1) {
			old_free = 0;
			delta = 0;
		} else {
			delta = old_free - free;
		}

		// screen_printf (0, 22, "Free: %-3d", free);
		// screen_printf (0, 23, "Previous: %-3d", old_free);
		screen_printf(0, 24, "Added: %-3d", delta);


		old_free = free;
	}

	if (pal_manager_active && debugger_memory_keywait) {

top:

		mykey = keys_get();
		debugger_name("2", 23);
		if (mykey == a_key) {
			shit += 45;
			if (shit == 255) shit = 0;
			if ((shit + 45) > 255) {
				shit = 210;
			}
			update_colors(shit);
			goto top;
		}
		if (mykey == esc_key) {
			pal_manager_update = NULL;
			debugger_memory_keywait = false;
			error_report(ERROR_BREAK_POINT, WARNING, MODULE_UNKNOWN, 0, 0);
		}
	}
}

static void game_scratch_update() {
	int count;

	debugger_name("3", 23);

	for (count = 0; count < 220; count += 10) {
		screen_printf(0, (count / 10) + 2, "%4d => %04x  %04x  %04x  %04x  %04x  %04x  %04x  %04x  %04x  %04x",
			count,
			game.scratch[count],
			game.scratch[count + 1],
			game.scratch[count + 2],
			game.scratch[count + 3],
			game.scratch[count + 4],
			game.scratch[count + 5],
			game.scratch[count + 6],
			game.scratch[count + 7],
			game.scratch[count + 8],
			game.scratch[count + 9]);
	}
}

static void game_global_update() {
	int count;

	debugger_name("4", 23);

	for (count = 0; count < 220; count += 10) {
		screen_printf(0, (count / 10) + 2, "%4d => %04x  %04x  %04x  %04x  %04x  %04x  %04x  %04x  %04x  %04x",
			count,
			global[count],
			global[count + 1],
			global[count + 2],
			global[count + 3],
			global[count + 4],
			global[count + 5],
			global[count + 6],
			global[count + 7],
			global[count + 8],
			global[count + 9]);
	}
}

static void game_help_update() {
	debugger_name("5", 23);

	game_file_show("*HELP.DB", 2);
}

static void game_conversation() {
	int x, y;
	int count, count2;
	int temp;
	int my_status;
	char temp_buf[80];
	Conv *my_conv;
	ConvData *my_data;

	if (conv_control.running != previous_running) {
		game_debugger_reset();
	}

	previous_running = conv_control.running;

	debugger_name("6", 23);

	if (conv_control.running < 0) {
		screen_printf(0, 2, "(Conversation system inactive).");
		goto done;
	} else {
		screen_printf(0, 2, "Conversation: %d", conv_control.running);
	}

	my_conv = conv[conv_control.index];
	my_data = conv_data[conv_control.index];

	temp_buf[0] = 0;
	if (conv_control.status == CONV_STATUS_HOLDING) {
		my_status = conv_control.hold_status;
	} else {
		my_status = conv_control.status;
	}

	switch (my_status) {
	case CONV_STATUS_NEXT_NODE:
		Common::strcpy_s(temp_buf, "Next");
		break;

	case CONV_STATUS_WAIT_AUTO:
		Common::strcpy_s(temp_buf, "W-auto");
		break;

	case CONV_STATUS_WAIT_ENTRY:
		Common::strcpy_s(temp_buf, "W-entry");
		break;

	case CONV_STATUS_EXECUTE:
		Common::strcpy_s(temp_buf, "Execute");
		break;

	case CONV_STATUS_REPLY:
	default:
		Common::strcpy_s(temp_buf, "Reply");
		break;
	}

	if (conv_control.status == CONV_STATUS_HOLDING) {
		Common::strcat_s(temp_buf, " HOLD");
	}

	if (conv_control.popup_is_up) {
		Common::strcat_s(temp_buf, " Popup");
	}

	screen_printf(0, 4, "Status: %-70s", temp_buf);

	screen_printf(0, 6, "Node: %-3d    Entry: %-3d", conv_control.node, conv_control.entry);

	screen_printf(0, 8, "Speaker: %-3d                 (Me: %-3d  You: %-3d)",
		conv_control.person_speaking, conv_control.me_trigger, conv_control.you_trigger);
	temp = conv_control.person_speaking;

	screen_printf(0, 9, "  X: %-6d   <=J  K=>", conv_control.x[temp]);
	screen_printf(0, 10, "  Y: %-6d   <=I  M=>", conv_control.y[temp]);
	screen_printf(0, 11, "  W: %-6d   <=,  .=>", conv_control.width[temp]);

	for (count = 23; count < 143; count += 10) {
		y = ((count - 23) / 10) + 13;
		x = 0;
		if (count < my_conv->num_variables) {
			x = screen_printf(x, y, "%4d => ", count);
		}
		for (count2 = count; count2 < count + 10; count2++) {
			if (count2 < my_conv->num_variables) {
				x = screen_printf(x, y, "%04x  ", *conv_get_variable(count2));
			}
		}
	}

done:
	;
}

static void game_matte() {
	int count;
	int image_max;
	int line;
	int series_id;
	char flags_buf[80];
	char name_buf[80];
	char sprite_buf[80];

	Common::strcpy_s(flags_buf, "");
	Common::strcpy_s(name_buf, "");
	Common::strcpy_s(sprite_buf, "");

	debugger_name("7", 23);

	image_max = MIN<int>(23, image_marker);

	for (count = 0; count < 23; count++) {
		line = count + 2;
		if (count < image_max) {
			switch (image_list[count].flags) {
			case IMAGE_UPDATE:
				// Common::strcpy_s (flags_buf, "Update");
				break;

			case IMAGE_STATIC:
				// Common::strcpy_s (flags_buf, "Static");
				break;

			case IMAGE_ERASE:
				// Common::strcpy_s (flags_buf, "Erase");
				break;

			case IMAGE_REFRESH:
				// Common::strcpy_s (flags_buf, "Refresh");
				break;

			case IMAGE_DELTA:
				// Common::strcpy_s (flags_buf, "Delta");
				break;

			default:
				Common::sprintf_s(flags_buf, "(%d)", image_list[count].flags);
				break;
			}
			if (image_list[count].flags != IMAGE_REFRESH) {
				series_id = image_list[count].series_id;
				Common::strcpy_s(name_buf, series_name[series_id]);
				if (image_list[count].sprite_id & MIRROR_MASK) {
					Common::sprintf_s(sprite_buf, "%dm", image_list[count].sprite_id & SPRITE_MASK);
				} else {
					Common::sprintf_s(sprite_buf, "%d", image_list[count].sprite_id);
				}
			} else {
				series_id = -1;
				Common::strcpy_s(name_buf, "sys");
				Common::strcpy_s(sprite_buf, " ");
			}

			screen_printf(0, line, "%02d) %-8s = %-7s  0x%02x  (%4d,%4d)  Spr:%-4s  Dep:%-2d  Sc:%-3d",
				count,
				name_buf,
				flags_buf,
				image_list[count].segment_id,
				image_list[count].x,
				image_list[count].y,
				sprite_buf,
				image_list[count].depth,
				image_list[count].scale);

		} else {
			screen_printf(0, line, "%02d) %-8s = %-7s %56s", count, "sys", "Empty", " ");
		}
	}
}

static void game_memory() {
	if (mem_manager_active && debugger_memory_keywait) {
		screen_normal_color = screen_hilite_color;
	}

	debugger_name("8", 23);

	screen_normal_color = colorbyte(hi_white, black);

	screen_printf(0, 2, "Cur: %-6ld  Min: %-6ld  Max: %-6ld",
		mem_conv_get_avail(), mem_min_free, mem_max_free);

	screen_printf(0, 8, "(DOS MCB memory map not available on this platform)");

	screen_normal_color = colorbyte(hi_white, black);

	if (mem_manager_active && debugger_memory_keywait) {
		int mykey = keys_get();
		debugger_name("8", 23);
		if (mykey == esc_key) {
			mem_manager_update = NULL;
			debugger_memory_keywait = false;
			error_report(ERROR_BREAK_POINT, WARNING, MODULE_UNKNOWN, 0, 0);
		}
	}
}

void state_display(int off, int x, int y) {
	screen_printf(x, y, "%d", room_state[off]);
}

static void game_state() {
	int count;

	debugger_name("9", 23);

	screen_printf(0, 2, "101");
	screen_printf(0, 3, "103");
	screen_printf(0, 4, "104");
	screen_printf(0, 5, "106");
	screen_printf(0, 6, "107");
	screen_printf(0, 7, "110");
	screen_printf(0, 8, "199");
	screen_printf(0, 9, "201");
	screen_printf(0, 10, "203");
	screen_printf(0, 11, "204");
	screen_printf(0, 12, "205");
	screen_printf(0, 13, "210");
	screen_printf(0, 14, "211");
	screen_printf(0, 15, "220");
	screen_printf(0, 16, "221");
	screen_printf(0, 17, "301");
	screen_printf(0, 18, "302");
	screen_printf(0, 19, "303");
	screen_printf(0, 20, "304");

	screen_printf(30, 2, "305");
	screen_printf(30, 3, "306");
	screen_printf(30, 4, "307");
	screen_printf(30, 5, "308");
	screen_printf(30, 6, "320");
	screen_printf(30, 7, "321");
	screen_printf(30, 8, "322");
	screen_printf(30, 9, "401");
	screen_printf(30, 10, "402");
	screen_printf(30, 11, "403");
	screen_printf(30, 12, "404");
	screen_printf(30, 13, "405");
	screen_printf(30, 14, "420");
	screen_printf(30, 15, "501");
	screen_printf(30, 16, "503");
	screen_printf(30, 17, "509");
	screen_printf(30, 18, "510");
	screen_printf(30, 19, "520");
	screen_printf(30, 20, "521");


	for (count = 0; count < 19; count++) {
		state_display(count, 5, 2 + count);
	}

	for (count = 19; count < 38; count++) {
		state_display(count, 35, count - 17);
	}
}

void game_debugger() {
	if (debugger_state != debugger_previous) game_debugger_reset();

	switch (debugger_state) {
	case DEBUGGER_MAIN:
		game_main_update();
		break;

	case DEBUGGER_HELP:
		if (debugger_state != debugger_previous) {
			game_help_update();
		}
		break;

	case DEBUGGER_PALETTE:
		if ((debugger_state != debugger_previous) || (pal_manager_active)) {
			game_palette_update();
		}
		break;

	case DEBUGGER_SCRATCH:
		game_scratch_update();
		break;

	case DEBUGGER_GLOBAL:
		game_global_update();
		break;

	case DEBUGGER_CONVERSATION:
		game_conversation();
		break;

	case DEBUGGER_MATTE:
		game_matte();
		break;

	case DEBUGGER_MEMORY:
		game_memory();
		break;

	case 8:  // DEBUGGER_STATE
		game_state();
		break;
	}

	debugger_previous = debugger_state;
}

int main_cheating_key(int mykey) {
	return g_engine->main_cheating_key(mykey);
}

int main_normal_key(int mykey) {
	return g_engine->main_normal_key(mykey);
}

int main_copy_verify() {
	return g_engine->main_copy_verify();
}

} // namespace MADSV2
} // namespace MADS
