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

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/echo.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/demo.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/lock.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"

namespace MADS {
namespace MADSV2 {

#define MESSAGE_COLOR_3 (((KERNEL_MESSAGE_COLOR_BASE_3 + 1) << 8) + KERNEL_MESSAGE_COLOR_BASE_3)
#define MESSAGE_COLOR           (((KERNEL_MESSAGE_COLOR_BASE   + 1) << 8) + KERNEL_MESSAGE_COLOR_BASE)

extern int first_inven;

int16 room_state[OMR];

byte video_mode;
RoomPtr room     = NULL;
int room_id      = KERNEL_STARTING_GAME;
int section_id   = KERNEL_STARTING_GAME;
int room_variant = 0;

int16 new_room   = 101;
int16 new_section  = 1;

int16 previous_room    = 0;
int16 previous_section = 0;

int kernel_initial_variant = 0;

HotPtr room_spots     = NULL;
int    room_num_spots = 0;

int kernel_room_series_marker = 0;

int kernel_room_bound_dif;
int kernel_room_scale_dif;

int kernel_allow_peel = false;

int kernel_panning_speed = 0;
int kernel_screen_fade   = 0;

Animation kernel_anim[KERNEL_MAX_ANIMATIONS];

ShadowList kernel_shadow_main = { 0, { 0 } };
ShadowList kernel_shadow_inter = { 1, { 15 } };

int kernel_ok_to_fail_load = false;

int kernel_mode = KERNEL_GAME_LOAD;

char kernel_cheating_password[16];
int  kernel_cheating_allowed   = 0;
int  kernel_cheating_forbidden = 0;

KernelDynamicHotSpot kernel_dynamic_hot[KERNEL_MAX_DYNAMIC];
int kernel_num_dynamic     = 0;
int kernel_dynamic_changed = 0;

SeriesPtr cursor = NULL;                /* Mouse cursor series */

int cursor_id   = 1;
int cursor_last = -1;

Kernel     kernel;                      /* Kernel data            */
KernelGame game;                        /* Kernel level game data */

Sequence sequence_list[KERNEL_MAX_SEQUENCES];

int stop_speech_on_run_anim = true;

KernelMessage kernel_message[KERNEL_MAX_MESSAGES];
FontPtr kernel_message_font;
int kernel_message_spacing;
static char kernel_work_name[20];
int kernel_sound_handle = 0;

int random_message_handle[KERNEL_MAX_RANDOM_MESSAGES];  /* List of active handles    */
int random_message_quote[KERNEL_MAX_RANDOM_MESSAGES];  /* List of active quote id's */
int random_max_messages = 1;                            /* # of active handles/ids   */

int random_quote_list[KERNEL_MAX_RANDOM_QUOTES];        /* List of available quote id's */
int random_quote_list_size = 0;                         /* # of available quote id's    */

int random_min_x = 0;                   /* X location allowable range */
int random_max_x = video_x;

int random_min_y = 0;                   /* Y location allowable range */
int random_max_y = display_y;
int random_spacing;                     /* Y location minimum spacing */

int random_teletype_rate = 0;           /* Rate for teletype          */

int random_message_color;               /* Color scheme for message   */
int random_message_duration;            /* Duration of messages       */

char kernel_interface_loaded[40] = "";


static void kernel_seq_image(SequencePtr sequence, ImagePtr image, int sequence_id);
static void kernel_reconstruct_screen(int anim_handle);
static void kernel_animation_get_sprite(int handle, int id);

void KernelGame::synchronize(Common::Serializer &s) {
	s.syncAsByte(going);
	s.skip(1);
	s.syncBytes(scratch, KERNEL_SCRATCH_SIZE);
	s.syncAsByte(difficulty);
	s.skip(1);
	s.syncAsSint16LE(last_save);
}


int kernel_load_vocab() {
	int error_flag;
	int count;
	int count2;
#ifdef log_vocab
	FILE *handle;
	long before, after;
#endif

	// Load all main command verbs
	for (count = 0; count < INTER_COMMANDS; count++) {
		vocab_make_active(command[count].id);
	}

	// Load all object names, and all verbs associated with objects
	for (count = 0; count < num_objects; count++) {
		vocab_make_active(object[count].vocab_id);

		for (count2 = 0; count2 < (int)object[count].num_verbs; count2++) {
			vocab_make_active(object[count].verb[count2].id);
		}
	}

	// Load vocabulary for this room's hot spots
	for (count = 0; count < room_num_spots; count++) {
		vocab_make_active(room_spots[count].vocab);
		if (room_spots[count].verb > 0) {
			vocab_make_active(room_spots[count].verb);
		}
	}

#ifdef log_vocab
	before = mem_get_avail();
#endif

	error_flag = vocab_load_active();

#ifdef log_vocab
	after = mem_get_avail();
	if (fileio_exist("vocab.log")) {
		handle = fopen("vocab.log", "wt");
		fprintf(handle, "Room %d   Vocab words: %d    Memory: %ld\n",
			room_id, vocab_active, before - after);
		fclose(handle);
	}
#endif

	return error_flag;
}

void kernel_game_shutdown() {
	int check_mode;

	sprite_free(&box_param.menu, true);
	sprite_free(&box_param.logo, true);
	sprite_free(&box_param.series, true);

	vocab_unload_active();

	// Drop cursor
	if (cursor != NULL) mem_free(cursor);
	cursor = NULL;

	// Free main video work buffer
	pack_set_special_buffer(NULL, NULL);

	object_unload();
	// inter_deallocate_objects();
	popup_available = false;

	// Remove special keyboard handler
	keys_remove();

	// Unload interface fonts
	if (font_misc != NULL) mem_free(font_misc);
	if (font_menu != NULL) mem_free(font_menu);
	if (font_conv != NULL) mem_free(font_conv);
	if (font_inter != NULL) mem_free(font_inter);
	if (font_main != NULL) mem_free(font_main);

	font_main = font_conv = font_inter = NULL;

	// Deallocate main screen buffer
	if (work_screen_ems_handle < 0)
		buffer_free(&scr_main);

	// Turn of speech system
	if (speech_system_active)
		speech_shutdown();

	// Return video to text mode
	mouse_hide();
	check_mode = video_mode;
	mouse_init(false, text_mode);
	video_init(text_mode, (check_mode != text_mode));

	// Deallocate EMS/XMS memory
	himem_shutdown();

	// Remove timer interrupt stuff
	timer_activate_low_priority(NULL);

	timer_remove();
}

void kernel_force_refresh() {
	int count;
	int purge_flag = true;

	for (count = 0; count < (int)image_marker; count++) {
		if (image_list[count].flags == IMAGE_REFRESH) {
			purge_flag = false;
		}
	}

	if (purge_flag) matte_refresh_work();
}

int kernel_game_startup(int game_video_mode, int load_flag,
		const char *release_version, const char *release_date) {
	int error_flag = true;
	int count, count2;
	int ems_temp;
	int pages;
	int reserve[EMS_PAGING_CLASSES];
	byte *interrupt_stack;
#ifdef demo
	char temp_buf[20];
#endif
#ifndef disable_error_check
	int error_code = 0;
#endif

	// Set up EMS/XMS paging system, if any
	himem_startup();

	speech_init();

	// ScummVM doesn't need EMS/XMS
#if 0
	int ems_error = true;

	if (ems_exists) {
		work_screen_ems_handle = ems_get_page_handle(4);
		if (work_screen_ems_handle >= 0) {
			ems_error = false;
		}
	}

	if (ems_error) {
		if (ems_exists) {
			error_report(ERROR_NO_MORE_EMS, SEVERE, MODULE_KERNEL, ems_pages, work_screen_ems_handle);
		} else {
			error_report(ERROR_KERNEL_NO_EMS, SEVERE, MODULE_KERNEL, ems_exists, work_screen_ems_handle);
		}
	}
#endif
	if (ems_exists) {
		if (load_flag & KERNEL_STARTUP_POPUP) {
			object_ems_handle = ems_get_page_handle(4);

			ems_temp = ems_get_page_handle(4);
			if (ems_temp >= 0) popup_preserve_initiator[0] = ems_temp;

			ems_temp = ems_get_page_handle(4);
			if (ems_temp >= 0) popup_preserve_initiator[1] = ems_temp;

			ems_temp = ems_get_page_handle(4);
			if (ems_temp >= 0) popup_preserve_initiator[2] = ems_temp;
		}
	}


	if (ems_exists) {
		pages = ems_pages_free;
		for (count = 0; count < EMS_PAGING_CLASSES; count++) {
			reserve[count] = 0;
		}
		if (pages >= 4) {
			reserve[EMS_PAGING_SYSTEM] = 4;
			pages -= 4;
		}
		ems_temp = MIN(pages >> 1, 64);
		reserve[EMS_PAGING_ROOM] = ems_temp;
		pages -= ems_temp;

		ems_temp = pages >> 2;
		reserve[EMS_PAGING_SECTION] = ems_temp;
		pages -= ems_temp;

		for (count = 0; count < EMS_PAGING_CLASSES; count++) {
			ems_paging_reserve[count] = 0;
			for (count2 = count - 1; count2 >= 0; count2--) {
				ems_paging_reserve[count] += reserve[count2];
			}
		}
	}

	// Some preliminary copy protection stuff
	// lock_preliminary_check();
	// Initialize sound driver jump table
	// pl sound_driver_null();
	timer_set_sound_flag(0);

	// Video initialization
	screen_dominant_mode(game_video_mode);
	video_init(game_video_mode, (game_video_mode != text_mode));
	mouse_init(true, game_video_mode);

	if (game_video_mode == mcga_mode) {
		mcga_compute_retrace_parameters();
	}

	// Initialize the main screen work buffer & its sub-buffers
	if (work_screen_ems_handle >= 0) {
		scr_main.x = video_x;
		scr_main.y = video_y;
		scr_main.data = ems_page[0];
		ems_map_buffer(work_screen_ems_handle);
	} else {
		buffer_init_name(&scr_main, video_x, video_y, "$scrmain");
	}
	if (scr_main.data == NULL) {
#ifndef disable_error_check
		error_code = ERROR_NO_MORE_MEMORY;
#endif
		goto done;
	}

	scr_work.x = scr_inter.x = video_x;
	scr_work.y = display_y;
	scr_inter.y = inter_size_y;

	scr_work.data = scr_main.data;
	scr_inter.data = (byte *)mem_normalize(buffer_pointer(&scr_main, 0, inter_base_y));

	buffer_fill(scr_main, 0);

	// Load the main interface fonts
	if (load_flag & KERNEL_STARTUP_FONT) {
		font_main = font_load("*FONTMAIN.FF");
		font_inter = font_load("*FONTINTR.FF");
		font_conv = font_load("*FONTCONV.FF");
		font_menu = font_load("*FONTMENU.FF");
		font_misc = font_load("*FONTMISC.FF");

		if ((font_main == NULL) || (font_inter == NULL) ||
			(font_conv == NULL) || (font_menu == NULL) ||
			(font_misc == NULL)) {
#ifndef disable_error_check
			error_code = ERROR_KERNEL_NO_FONTS;
#endif
			goto done;
		}
	}


	// Install timer handler & low priority cycling interrupt
	if (load_flag & KERNEL_STARTUP_INTERRUPT) {
		timer_install();

		cycling_active = false;
		timer_activate_low_priority(cycle_colors);
		keys_install();
	}

	// Log in demo copy
#ifdef demo
	if (game_video_mode != text_mode) demo_log_in(release_version, release_date);
#endif

	// Mention EMS paging situation
#ifdef demo
	if (ems_paging_active) {
		ltoa(((long)ems_pages * EMS_PAGE_SIZE) >> 10, temp_buf, 10);
		echo(temp_buf, false);
		echo("K of EMS memory available.", true);
	} else {
		echo("EMS memory not available.", true);
	}

	if (xms_exists) {
		echo("XMS memory system detected.", true);
	}
#endif

	// Load the objects list
	if (load_flag & KERNEL_STARTUP_OBJECTS) {
		if (object_load()) {
#ifndef disable_error_check
			error_code = ERROR_KERNEL_NO_OBJECTS;
#endif
			goto done;
		}
		if (inven_num_objects > 0) {
			active_inven = 0;
		}
	}

	// Allow packing routines to use lower interrupt stack
	interrupt_stack = timer_get_interrupt_stack();
	pack_set_special_buffer(interrupt_stack, NULL);

	// Initialize player data structures
	if (load_flag & KERNEL_STARTUP_PLAYER)
		player_init();

	popup_available = true;

	Common::strcpy_s(box_param.name, "*BOX.SS");

	if (load_flag & KERNEL_STARTUP_CURSOR) {
		// Wipe palette & prepare for cursor
		pal_init(KERNEL_RESERVED_LOW_COLORS, KERNEL_RESERVED_HIGH_COLORS);
		pal_white(master_palette);
		if (video_mode == mcga_mode) {
			mcga_setpal_range(&master_palette, 0, 4);
		}

		// Load cursor sprite series
		cursor = sprite_series_load("*CURSOR.SS", PAL_MAP_RESERVED);
		if (cursor == NULL) {
#ifndef disable_error_check
			error_code = ERROR_KERNEL_NO_CURSOR;
#endif
			goto done;
		}

		// Activate main cursor sprite as mouse cursor
		cursor_last = cursor_id = (cursor->num_sprites > 1) ? 2 : 1;
		mouse_cursor_sprite(cursor, cursor_id);
	}

	if (load_flag & KERNEL_STARTUP_VOCAB) {
		vocab_load_active();
	}

	if (load_flag & KERNEL_STARTUP_POPUP) {
		if (popup_box_load()) {
			error_code = ERROR_KERNEL_NO_POPUP;
			goto done;
		}
	}

	buffer_fill(scr_live, 0);
	mcga_setpal(&master_palette);

	error_flag = false;

done:
	if (load_flag & KERNEL_STARTUP_CURSOR_SHOW)
		mouse_show();

	if (error_flag) {
#ifndef disable_error_check
		error_check_memory();
		error_report(error_code, ERROR, MODULE_KERNEL, 0, 0);
#endif
		kernel_game_shutdown();
	}

	return error_flag;
}

void kernel_section_shutdown() {
}

int kernel_section_startup(int newSection) {
	int error_flag = true;

	// Make note of new section number
	previous_section = section_id;
	section_id = newSection;

	error_flag = false;

	return error_flag;
}

void kernel_room_shutdown() {
	inter_deallocate_objects();

	if (inter_anim) {
		anim_unload((AnimPtr)inter_anim);
		buffer_free(&scr_inter_orig);
		mem_free(inter_anim);
		inter_anim = nullptr;
	} else if (scr_inter_orig.data) {
		buffer_free(&scr_inter_orig);
	}

	// Dump the room hot spots
	if (room_spots != NULL) {
		mem_free(room_spots);
		room_spots = NULL;
		room_num_spots = 0;
	}

	// Remove our palette shadowing list
	pal_activate_shadow(NULL);

	if (room != NULL) {
		room_unload(room,
			&scr_orig,
			&scr_depth,
			&scr_walk,
			&scr_special,
			&picture_map,
			&depth_map);
		room = NULL;
	}
}

int kernel_room_startup(int newRoom, int initial_variant, const char *interface,
		bool new_palette, bool barebones) {
	int error_flag = true;
	int load_flags;
#ifndef disable_error_check
	int error_code = 0;
	int error_data = 0;
#endif

	// Make a note of the new room number & variant
	previous_room = room_id;
	room_id = newRoom;
	room_variant = initial_variant;

	scr_inter_orig.data = nullptr;

	// Start a brand new palette, reserving the proper # of colors
	if (new_palette)
		pal_init(KERNEL_RESERVED_LOW_COLORS, KERNEL_RESERVED_HIGH_COLORS);

	pal_white(master_palette);

	// Load up popup box frame
	if (g_engine->getGameID() == GType_Phantom && popup_box_load()) {
		error_code = ERROR_KERNEL_NO_POPUP;
		goto done;
	}

	// Initialize the matteing system
	matte_init(false);

	// Initialize graphics sequence data structures
	kernel_seq_init();
	kernel_message_init();

	// Activate the main shadow list
	pal_activate_shadow(&kernel_shadow_main);

	// Load header, picture, and attribute screen for this room
	load_flags = ROOM_LOAD_HARD_SHADOW;
	if (kernel.translating) load_flags |= ROOM_LOAD_TRANSLATE;

	room = room_load(room_id, room_variant, NULL,
		&scr_orig,
		&scr_depth,
		&scr_walk,
		&scr_special,
		&picture_map,
		&depth_map,
		&picture_resource,
		&depth_resource,
		-1,
		-1,
		load_flags);
	if (room == NULL) {
#ifndef disable_error_check
		error_data = room_load_error;
		error_code = ERROR_KERNEL_NO_ROOM;
#endif
		goto done;
	}

	tile_pan(&picture_map, picture_view_x, picture_view_y);
	tile_pan(&depth_map, picture_view_x, picture_view_y);

	// Set up color cycling table for this room
	cycle_init(&room->cycle_list, false);

	// Initialize rail-system parameters for this room
	rail_num_nodes = room->num_rails + 2;
	rail_base = &room->rail[0];

	rail_connect_all_nodes();

	// Make preliminary scaling computations
	kernel_room_bound_dif = room->front_y - room->back_y;
	kernel_room_scale_dif = room->front_scale - room->back_scale;

	// Initialize the graphics image lists
	image_marker = 1;
	image_list[0].flags = IMAGE_REFRESH;
	image_list[0].segment_id = KERNEL_SEGMENT_SYSTEM;

	// Set up graphics window locations
	viewing_at_y = 0;
	inter_viewing_at_y = inter_base_y;

	// Mark the boundary between interface and room sprite series
	kernel_room_series_marker = series_list_marker;

	if (barebones) {
		room_spots = nullptr;
		goto finish;
	}

	// Load up the room's hotspot table
	room_spots = room_load_hotspots(room_id, &room_num_spots);
	if (room_spots == NULL) {
#ifndef disable_error_check
		error_code = ERROR_KERNEL_NO_HOTSPOTS;
#endif
		goto done;
	}

	kernel_load_vocab();

	pal_activate_shadow(&kernel_shadow_inter);
	load_flags = ANIM_LOAD_BACKGROUND | ANIM_INTERFACE;
	if (kernel.translating)
		load_flags |= ANIM_LOAD_TRANSLATE;
	inter_anim = (AnimInterPtr)anim_load(interface, &scr_inter_orig, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, load_flags);
	if (!inter_anim) goto done;

	if (!inter_anim->font) {
		mem_free(inter_anim);
		inter_anim = nullptr;
	}

	// Set up interface background screen
	kernel_set_interface_mode(inter_input_mode);

	// Mouse cursor on
	mouse_show();

	inter_allocate_objects();

finish:
	error_flag = false;

done:
	if (error_flag) {
#ifndef disable_error_check
		error_check_memory();
		error_report(error_code, ERROR, MODULE_KERNEL, room_id, error_data);
#endif
		kernel_room_shutdown();
	}
	return error_flag;
}

void kernel_unload_all_series() {
	int count;

	// Unload all series (but don't unload those for the interface background)
	for (count = series_list_marker - 1; count >= kernel_room_series_marker; count--) {
		if (series_user[count] > 1) series_user[count] = 1;
		matte_deallocate_series(count, true);
	}
}

int kernel_load_series(const char *name, int load_flags) {
	int handle = -2;

	if (kernel.translating) load_flags |= SPRITE_LOAD_TRANSLATE;

	handle = matte_load_series(name, load_flags, 0);

	if ((handle < 0) && !kernel_ok_to_fail_load) {
#ifndef disable_error_check
		Common::strcpy_s(error_string, name);
		error_report(ERROR_SERIES_LOAD_FAILED, WARNING, MODULE_KERNEL, handle, sprite_error);
#endif
	}

	return handle;
}

void kernel_flip_hotspot(int vocab_code, int active) {
	int count;

	for (count = 0; count < room_num_spots; count++) {
		if (room_spots[count].vocab == vocab_code) {
			room_spots[count].active = (byte)active;
			hspot_toggle(STROKE_INTERFACE, count, active);
		}
	}
}

void kernel_flip_hotspot_loc(int vocab_code, int active, int x, int y) {
	int count;

	for (count = 0; count < room_num_spots; count++) {
		if (room_spots[count].vocab == vocab_code) {
			if ((x >= room_spots[count].ul_x) &&
				(x <= room_spots[count].lr_x) &&
				(y >= room_spots[count].ul_y) &&
				(y <= room_spots[count].lr_y)) {
				room_spots[count].active = (byte)active;
				hspot_toggle(STROKE_INTERFACE, count, active);
			}
		}
	}
}

void kernel_seq_init() {
	int count;

	for (count = 0; count < KERNEL_MAX_SEQUENCES; count++) {
		sequence_list[count].active_flag = false;
		sequence_list[count].dynamic_hotspot = -1;
	}
}

int kernel_seq_add(int series_id, int mirror, int initial_sprite,
		int low_sprite, int high_sprite, int loop_mode, int loop_direction,
		int depth, int scale, int auto_locating, int x, int y,
		word ticks, word interval_ticks, word start_ticks, int expire) {
	int result = -1;
	int id = -1;
	int found = false;
	int count;

	for (count = 0; !found && (count < KERNEL_MAX_SEQUENCES); count++) {
		id = count;
		found = !sequence_list[count].active_flag;
	}

	if (!found) {
#if !defined(disable_error_check)
		error_report(ERROR_SEQUENCE_LIST_FULL, WARNING, MODULE_KERNEL, KERNEL_MAX_SEQUENCES, 0);
#endif
		goto done;
	}

	if (low_sprite <= 0) {
		low_sprite = 1;
	}

	if (high_sprite <= 0) {
		high_sprite = series_list[series_id]->num_sprites;
	}

	if (high_sprite == low_sprite) {
		loop_direction = 0;
	}

	sequence_list[id].active_flag = true;
	sequence_list[id].series_id = (byte)series_id;
	sequence_list[id].mirror = (byte)mirror;
	sequence_list[id].sprite = initial_sprite;
	sequence_list[id].start_sprite = low_sprite;
	sequence_list[id].end_sprite = high_sprite;
	sequence_list[id].loop_mode = loop_mode;
	sequence_list[id].loop_direction = loop_direction;
	sequence_list[id].depth = (byte)depth;
	sequence_list[id].scale = (byte)scale;
	sequence_list[id].auto_locating = (byte)auto_locating;
	sequence_list[id].x = x;
	sequence_list[id].y = y;
	sequence_list[id].ticks = ticks;
	sequence_list[id].interval_ticks = interval_ticks;
	sequence_list[id].base_time = kernel.clock + start_ticks;
	sequence_list[id].expire = (byte)expire;
	sequence_list[id].expired = false;

	sequence_list[id].motion = 0;
	sequence_list[id].dynamic_hotspot = -1;

	sequence_list[id].num_triggers = 0;
	sequence_list[id].trigger_dest = (byte)kernel.trigger_setup_mode;

	for (count = 0; count < 3; count++) {
		sequence_list[id].trigger_words[count] = player2.words[count];
	}

	sequence_list[id].last_image.flags = -1;

	result = id;

done:
	return result;
}

int kernel_seq_forward(int series_id, int mirror, word ticks, word interval_ticks,
		word start_ticks, int expire) {
	int depth;
	SpritePtr sprite;

	sprite = &series_list[series_id]->index[0];

	depth = attr_depth(&depth_map,
		sprite->x + (sprite->xs >> 1),
		sprite->y + (sprite->ys >> 1)) - 1;

	return (kernel_seq_add(series_id, mirror, 1, 0, 0, AA_LINEAR, 1,
		depth, 100, true, 0, 0, ticks, interval_ticks,
		start_ticks, expire));
}

int kernel_seq_pingpong(int series_id, int mirror,
	word ticks, word interval_ticks,
	word start_ticks,
	int expire) {
	int depth;
	SpritePtr sprite;

	sprite = &series_list[series_id]->index[0];

	depth = attr_depth(&depth_map,
		sprite->x + (sprite->xs >> 1),
		sprite->y + (sprite->ys >> 1)) - 1;

	return (kernel_seq_add(series_id, mirror, 1, 0, 0, AA_PINGPONG, 1,
		depth, 100, true, 0, 0, ticks, interval_ticks,
		start_ticks, expire));
}

int kernel_seq_backward(int series_id, int mirror, word ticks, word interval_ticks,
		word start_ticks, int expire) {
	int depth;
	SpritePtr sprite;

	sprite = &series_list[series_id]->index[0];

	depth = attr_depth(&depth_map,
		sprite->x + (sprite->xs >> 1),
		sprite->y + (sprite->ys >> 1)) - 1;

	return kernel_seq_add(series_id, mirror,
		series_list[series_id]->num_sprites,
		0, 0, AA_LINEAR, -1, depth, 100, true, 0, 0,
		ticks, interval_ticks, start_ticks, expire);
}

int kernel_seq_backward_scroll(int series_id, int mirror,
	word ticks, word interval_ticks,
	word start_ticks,
	int expire) {
	int depth = 0;

	return (kernel_seq_add(series_id, mirror,
		series_list[series_id]->num_sprites,
		0, 0, AA_LINEAR, -1, depth, 100, true, 0, 0,
		ticks, interval_ticks, start_ticks, expire));
}

void kernel_synch(int slave_type, int slave_id, int master_type, int master_id) {
	long master_time;

	switch (master_type) {
	case KERNEL_SERIES:
		master_time = sequence_list[master_id].base_time;
		break;

	case KERNEL_ANIM:
		master_time = kernel_anim[master_id].next_clock;
		break;

	case KERNEL_NOW:
		master_time = kernel.clock + (long)master_id;
		break;

	case KERNEL_PLAYER:
	default:
		master_time = player.clock;
		break;
	}

	switch (slave_type) {
	case KERNEL_SERIES:
		sequence_list[slave_id].base_time = master_time;
		break;

	case KERNEL_PLAYER:
		player.clock = master_time;
		break;

	case KERNEL_ANIM:
		kernel_anim[slave_id].next_clock = master_time;
		break;
	}
}

void kernel_player_expire(int sequence_id) {
	sequence_list[sequence_id].expired = true;
	sequence_list[sequence_id].base_time = player.clock;
}

void kernel_seq_depth(int sequence_id, int depth) {
	sequence_list[sequence_id].depth = (byte)depth;
}

void kernel_seq_scale(int sequence_id, int scale) {
	sequence_list[sequence_id].scale = (byte)scale;
}

void kernel_seq_loc(int sequence_id, int x, int y) {
	sequence_list[sequence_id].x = x;
	sequence_list[sequence_id].y = y;
	sequence_list[sequence_id].auto_locating = false;
}

void kernel_seq_motion(int sequence_id, int flags,
	int delta_x_times_100,
	int delta_y_times_100) {
	sequence_list[sequence_id].motion = (byte)(KERNEL_MOTION | flags);
	sequence_list[sequence_id].sign_x = sgn(delta_x_times_100);
	sequence_list[sequence_id].sign_y = sgn(delta_y_times_100);
	sequence_list[sequence_id].delta_x = abs(delta_x_times_100);
	sequence_list[sequence_id].delta_y = abs(delta_y_times_100);
	sequence_list[sequence_id].accum_x = 0;
	sequence_list[sequence_id].accum_y = 0;
}

void kernel_seq_range(int sequence_id, int first, int last) {
	int num_sprites;
	int from, unto;
	SequencePtr sequence;

	sequence = &sequence_list[sequence_id];

	num_sprites = series_list[sequence->series_id]->num_sprites;

	switch (first) {
	case KERNEL_FIRST:
	case KERNEL_DEFAULT:
		from = 1;
		break;
	case KERNEL_LAST:
		from = num_sprites;
		break;
	default:
		from = first;
		break;
	}

	switch (last) {
	case KERNEL_FIRST:
		unto = 1;
		break;
	case KERNEL_LAST:
	case KERNEL_DEFAULT:
		unto = num_sprites;
		break;
	default:
		unto = last;
		break;
	}

	sequence->start_sprite = from;
	sequence->end_sprite = unto;

	sequence->sprite = (sequence->loop_direction >= 0) ? from : unto;
}

int kernel_seq_stamp(int series_id, int mirror, int sprite) {
	int id;

	id = kernel_seq_forward(series_id, mirror, 32767, 0, 0, 0);
	if (id >= 0) {
		kernel_seq_range(id, sprite, sprite);
		sequence_list[id].loop_direction = AA_STAMP;
	}
	return (id);
}

int kernel_seq_trigger(int sequence_id,
	int trigger_type,
	int trigger_sprite,
	int trigger_code) {
	int error_code = true;
	int id;
	SequencePtr sequence;

	sequence = &sequence_list[sequence_id];

	if (sequence->num_triggers >= KERNEL_MAX_TRIGGERS) goto done;

	id = sequence->num_triggers++;

	sequence->trigger_type[id] = (byte)trigger_type;
	sequence->trigger_sprite[id] = trigger_sprite;
	sequence->trigger_code[id] = (byte)trigger_code;

	error_code = false;

done:
	return error_code;
}

int kernel_timing_trigger(int ticks, int trigger_code) {
	int id = -1;
	int found = false;
	int count;

	for (count = 0; !found && (count < KERNEL_MAX_SEQUENCES); count++) {
		id = count;
		found = !sequence_list[count].active_flag;
	}

	if (!found) {
#if !defined(disable_error_check)
		error_report(ERROR_SEQUENCE_LIST_FULL, WARNING, MODULE_KERNEL, KERNEL_MAX_SEQUENCES, 0);
#endif
		goto done;
	}

	sequence_list[id].active_flag = true;
	sequence_list[id].series_id = KERNEL_SPECIAL_TIMING;

	sequence_list[id].ticks = ticks;
	sequence_list[id].interval_ticks = 0;
	sequence_list[id].base_time = kernel.clock + ticks;
	sequence_list[id].expire = 1;
	sequence_list[id].expired = false;

	sequence_list[id].num_triggers = 0;
	sequence_list[id].trigger_dest = (byte)kernel.trigger_setup_mode;

	for (count = 0; count < 3; count++) {
		sequence_list[id].trigger_words[count] = player2.words[count];
	}

	kernel_seq_trigger(id, KERNEL_TRIGGER_EXPIRE, 0, trigger_code);

done:
	return id;
}

int kernel_seq_purge(int sequence_id) {
	int count;
	int purged_any = -1;

	for (count = 0; count < (int)image_marker; count++) {
		if (image_list[count].segment_id == (byte)sequence_id) {
			image_list[count].flags = IMAGE_ERASE;
			purged_any = count;
		}
	}

	return purged_any;
}

void kernel_seq_full_update() {
	int count, id;

	for (count = 0; count < KERNEL_MAX_SEQUENCES; count++) {
		if (sequence_list[count].active_flag) {
			if ((int)(sequence_list[count].series_id) != KERNEL_SPECIAL_TIMING) {
				id = matte_allocate_image();
				if (id >= 0) {
					if (sequence_list[count].last_image.flags >= 0) {
						image_list[id] = sequence_list[count].last_image;
					} else {
						kernel_seq_image(&sequence_list[count], &image_list[id], count);
					}
				}
			}
		}
	}
}

void kernel_seq_correction(long old_clock, long new_clock) {
	int count;
	SequencePtr sequence;

	for (count = 0; count < KERNEL_MAX_SEQUENCES; count++) {
		sequence = &sequence_list[count];
		if (sequence->active_flag) {
			sequence->base_time += (new_clock - old_clock);
		}
	}
}

void kernel_draw_to_background(int series_id, int sprite_id,
	int x, int y,
	int depth, int scale) {

	// WORKAROUND: In the ROTP mask puzzle room, the x/y are passed
	// as KERNEL_HOME, but sprite_id == KERNEL_LAST, which is negative
	int sprite_index = (sprite_id != KERNEL_LAST) ? sprite_id :
		series_list[series_id]->num_sprites;

	if (x == KERNEL_HOME) {
		x = series_list[series_id]->index[sprite_index - 1].x;
	}

	if (y == KERNEL_HOME) {
		y = series_list[series_id]->index[sprite_index - 1].y;
	}

	sprite_draw_3d_scaled_big(series_list[series_id],
		sprite_index,
		&scr_orig, &scr_depth,
		x - picture_map.pan_base_x,
		y - picture_map.pan_base_y,
		depth, scale, 0, 0);

	matte_refresh_work();
}

void kernel_seq_delete(int sequence_id) {
	if (sequence_list[sequence_id].active_flag) {
		if (sequence_list[sequence_id].dynamic_hotspot >= 0) {
			kernel_delete_dynamic(sequence_list[sequence_id].dynamic_hotspot);
		}
	}

	sequence_list[sequence_id].active_flag = false;

	if (!sequence_list[sequence_id].expired) {
		kernel_reconstruct_screen(-1);
	} else {
		kernel_seq_purge(sequence_id);
	}
}

static void kernel_seq_image(SequencePtr sequence, ImagePtr image, int sequence_id) {
	image->flags = series_list[sequence->series_id]->delta_series ? IMAGE_DELTA : IMAGE_UPDATE;
	image->segment_id = (byte)sequence_id;
	image->series_id = sequence->series_id;
	image->sprite_id = sequence->sprite | (sequence->mirror ? MIRROR_MASK : 0);

	image->depth = sequence->depth;
	image->scale = sequence->scale;

	if (!sequence->auto_locating) {
		image->x = sequence->x;
		image->y = sequence->y;
	} else {
		image->x = series_list[image->series_id]->index[sequence->sprite - 1].x;
		image->y = series_list[image->series_id]->index[sequence->sprite - 1].y;
	}

	sequence->last_image = *image;
}

static int kernel_seq_update(SequencePtr sequence, int sequence_id) {
	int id;
	int count;
	int cycling = false;
	int trigger = -1;
	int x, y, xs, ys;
	int x1, y1;
	int x2, y2;
	int purged;
	ImagePtr image;

	purged = kernel_seq_purge(sequence_id);

	if (purged >= 0) {
		if (sequence->loop_direction == AA_STAMP) {
			image_list[purged].flags = IMAGE_STATIC;
			goto done;
		}
	}

	if (sequence->expired) {
		kernel_seq_delete(sequence_id);
		goto done;
	} else {
		if (sequence->series_id == KERNEL_SPECIAL_TIMING) {
			sequence->expired = true;
			goto triggers;
		}

		id = matte_allocate_image();
		if (id < 0) {
			sequence->expired = true;
			goto triggers;
		}

		image = &image_list[id];

		kernel_seq_image(sequence, image, sequence_id);

	}

	if (sequence->motion ||
		(sequence->dynamic_hotspot >= 0)) {
		xs = (series_list[sequence->series_id]->index[sequence->sprite - 1].xs * sequence->scale) / 200;
		ys = (series_list[sequence->series_id]->index[sequence->sprite - 1].ys * sequence->scale) / 100;
		x = image->x;
		y = image->y;

		if (sequence->motion & KERNEL_MOTION) {
			sequence->accum_x += sequence->delta_x;
			while (sequence->accum_x >= 100) {
				sequence->accum_x -= 100;
				sequence->x += sequence->sign_x;
			}
			sequence->accum_y += sequence->delta_y;
			while (sequence->accum_y >= 100) {
				sequence->accum_y -= 100;
				sequence->y += sequence->sign_y;
			}

			if (sequence->motion & KERNEL_MOTION_OFFSCREEN) {
				if ((((x + xs) < 0) || ((x - xs) >= picture_map.total_x_size)) ||
					((y < 0) || ((y - ys) >= picture_map.total_y_size))) {
					cycling = true;
					sequence->expired = true;
				}
			}
		}

		if (sequence->dynamic_hotspot >= 0) {
			x1 = x - xs;
			x2 = x + xs;
			y1 = y - ys;
			y2 = y;
			x1 = MAX(0, x1);
			y1 = MAX(0, y1);
			x2 = MIN(picture_map.total_x_size - 1, x2);
			y2 = MIN(picture_map.total_y_size - 1, y2);
			kernel_dynamic_hot[sequence->dynamic_hotspot].x = x1;
			kernel_dynamic_hot[sequence->dynamic_hotspot].y = y1;
			kernel_dynamic_hot[sequence->dynamic_hotspot].xs = (x2 - x1) + 1;
			kernel_dynamic_hot[sequence->dynamic_hotspot].ys = (y2 - y1) + 1;
			kernel_dynamic_hot[sequence->dynamic_hotspot].valid = true;
			kernel_dynamic_changed = true;
		}
	}

	if (sequence->start_sprite != sequence->end_sprite) {
		sequence->sprite += sequence->loop_direction;
	}

	if (sequence->sprite < sequence->start_sprite) {
		cycling = true;
		if (sequence->loop_mode == AA_PINGPONG) {
			sequence->sprite = sequence->start_sprite + 1;
			sequence->loop_direction = 1;
		} else {
			sequence->sprite = sequence->end_sprite;
		}
	} else if (sequence->sprite > sequence->end_sprite) {
		cycling = true;
		if (sequence->loop_mode == AA_PINGPONG) {
			sequence->sprite = sequence->end_sprite - 1;
			sequence->loop_direction = -1;
		} else {
			sequence->sprite = sequence->start_sprite;
		}
	}

	if (cycling) {
		if (sequence->expire) {
			sequence->expire--;
			if (!sequence->expire) {
				sequence->expired = true;
			}
		}
	}

triggers:
	for (count = 0; count < (int)sequence->num_triggers; count++) {
		switch (sequence->trigger_type[count]) {
		case KERNEL_TRIGGER_EXPIRE:
			if (sequence->expired) trigger = count;
			break;
		case KERNEL_TRIGGER_LOOP:
			if (cycling) trigger = count;
			break;
		case KERNEL_TRIGGER_SPRITE:
		default:
			if ((sequence->sprite == sequence->trigger_sprite[count]) ||
				(sequence->trigger_sprite[count] == 0)) {
				trigger = count;
			}
			break;
		}
	}

	if (trigger >= 0) {
		kernel.trigger = sequence->trigger_code[trigger];
		kernel.trigger_mode = sequence->trigger_dest;
		if (kernel.trigger_mode != KERNEL_TRIGGER_DAEMON) {
			for (count = 0; count < 3; count++) {
				player2.words[count] = sequence->trigger_words[count];
			}
		}
	}

	if (sequence->series_id == KERNEL_SPECIAL_TIMING) {
		sequence->active_flag = false;
	}

done:
	return cycling;
}

void kernel_seq_update_all() {
	int count;
	int ok_to_update;
	SequencePtr sequence;

	for (count = 0; (count < KERNEL_MAX_SEQUENCES); count++) {
		sequence = &sequence_list[count];
		if (sequence->active_flag) {
			if (kernel.clock >= sequence->base_time) {
				ok_to_update = (kernel.fx || !kernel.trigger ||
					sequence->expired || !sequence->num_triggers);
				if (ok_to_update) {
					sequence->base_time = kernel.clock + sequence->ticks;
					if (kernel_seq_update(sequence, count)) {
						sequence->base_time += sequence->interval_ticks;
					}
				}
			}
		}
	}
}

void kernel_seq_player(int sequence_id, int synch_me) {
	kernel_seq_loc(sequence_id, player.x, player.y + ((player.center_of_gravity * player.scale) / 100));
	kernel_seq_depth(sequence_id, player.depth);
	kernel_seq_scale(sequence_id, player.scale);

	if (synch_me) {
		kernel_synch(KERNEL_SERIES, sequence_id,
			KERNEL_PLAYER, 0);
	}
}

void kernel_animation_init() {
	int count;

	for (count = 0; count < KERNEL_MAX_ANIMATIONS; count++) {
		kernel_anim[count].anim = NULL;
		kernel_anim[count].cycled = false;
		kernel_anim[count].repeat = false;
	}
}

int kernel_run_animation(const char *name, int trigger_code) {
	int found = -1;
	int error_flag = true;
	int count;
	int load_flags;
	int id;
	long largest_block;

#if 0
	if (stop_speech_on_run_anim) {
		digi_stop(1);
		digi_stop(2);
		digi_stop(3);
	}
#endif

	anim_error = -2;

	for (count = 0; (found < 0) && (count < KERNEL_MAX_ANIMATIONS); count++) {
		if (kernel_anim[count].anim == NULL) {
			found = count;
		}
	}

	if (found < 0) goto done;

	load_flags = 0;
	if (kernel.translating) load_flags |= ANIM_LOAD_TRANSLATE;
	kernel_anim[found].anim = anim_load(name,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL, load_flags);
	if (kernel_anim[found].anim == NULL) goto done;

	kernel_anim[found].messages = 0;
	kernel_anim[found].dynamic_hotspot = -1;

	kernel_anim[found].sprite_loaded = -1;
	if (kernel_anim[found].anim->misc_any_packed) {
		kernel_anim[found].buffer_id = -1;
		id = kernel_anim[found].anim->series_id[kernel_anim[found].anim->misc_packed_series];
		memcpy(&largest_block, &series_list[id]->misc_largest_block, sizeof(long));
		if (mem_get_avail() - 128 >= largest_block) {
			mem_free(series_list[id]->arena);
			series_list[id]->arena = (byte *)mem_get_name(largest_block * 2, "$arena$");
			if (series_list[id]->arena == NULL) {
				series_list[id]->arena = (byte *)mem_get_name(largest_block, "$arena$");
				anim_error = -1;
				if (series_list[id]->arena == NULL) goto done;
			} else {
				kernel_anim[found].buffer[0] = series_list[id]->arena;
				kernel_anim[found].buffer[1] = (byte *)mem_normalize(series_list[id]->arena + largest_block);
				kernel_anim[found].buffer_id = 0;
			}
		}

		kernel_animation_get_sprite(found, 1);
	}

	if (kernel_mode == KERNEL_ACTIVE_CODE) kernel_new_palette();

	kernel_anim[found].frame = 0;
	kernel_anim[found].image = 0;
	kernel_anim[found].next_clock = kernel.clock;

	kernel_anim[found].view_changes = false;

	kernel_anim[found].trigger_code = trigger_code;
	kernel_anim[found].trigger_mode = kernel.trigger_setup_mode;
	for (count = 0; count < 3; count++) {
		kernel_anim[found].trigger_words[count] = player2.words[count];
	}

	for (count = 0; count < kernel_anim[found].anim->num_speech; count++) {
		kernel_anim[found].anim->speech[count].flags = (word)-1;
	}

	error_flag = false;

	kernel_anim[found].last_frame = -1;

done:
	if (error_flag) {
		if (found >= 0) kernel_abort_animation(found);
#ifndef disable_error_check
		Common::strcpy_s(error_string, name);
		error_report(ERROR_KERNEL_NO_ANIMATION, WARNING, MODULE_KERNEL, trigger_code, anim_error);
#endif
	}

	anim_error = 0;
	return found;
}

int kernel_run_animation_talk(char thing, int num, int trigger_code) {
	char test[20];
	char crap[6];
	int feedback;

	Common::strcpy_s(test, "*talk_");

	if (thing == 'r') {
		Common::strcat_s(test, "r");
	} else if (thing == 'b') {
		Common::strcat_s(test, "b");
	} else if (thing == 'e') {
		Common::strcat_s(test, "e");
	}

	mads_itoa(num, crap, 10);
	Common::strcat_s(test, crap);

	feedback = kernel_run_animation(test, trigger_code);
	return feedback;
}

int kernel_run_animation_disp(char thing, int num, int trigger_code) {
	char test[20];
	char crap[6];
	int feedback;

	Common::strcpy_s(test, "*disp_");

	if (thing == 'r') {
		Common::strcat_s(test, "ru");
	} else if (thing == 'b') {
		Common::strcat_s(test, "ab");
	} else if (thing == 'e') {
		Common::strcat_s(test, "ed");
	}

	mads_itoa(num, crap, 10);
	Common::strcat_s(test, crap);

	feedback = kernel_run_animation(test, trigger_code);
	return (feedback);
}

int kernel_run_animation_write(int trigger_code) {
	int feedback;

	feedback = kernel_run_animation("*write_e", trigger_code);
	return (feedback);
}

int kernel_run_animation_point(int num, int trigger_code) {
	char test[20];
	char crap[6];
	int feedback;

	Common::strcpy_s(test, "*point_b");

	mads_itoa(num, crap, 10);
	Common::strcat_s(test, crap);

	feedback = kernel_run_animation(test, trigger_code);
	return feedback;
}

static void kernel_animation_get_sprite(int handle, int id) {
	int series_id;
	byte *pointer;

	if (id != kernel_anim[handle].sprite_loaded) {
		series_id = kernel_anim[handle].anim->series_id[kernel_anim[handle].anim->misc_packed_series];
		if (kernel_anim[handle].buffer_id >= 0) {
			pointer = kernel_anim[handle].buffer[kernel_anim[handle].buffer_id];
			kernel_anim[handle].buffer_id = 1 - kernel_anim[handle].buffer_id;
		} else {
			pointer = series_list[series_id]->arena;
		}
		if (sprite_data_load(series_list[series_id], id, pointer)) {
			error_report(ERROR_SPRITE_DATA_LOAD_FAILED, ERROR, MODULE_KERNEL, id, series_id);
		}

		kernel_anim[handle].sprite_loaded = id;
	}
}

void kernel_reset_animation(int handle, int frame) {
	if (kernel_anim[handle].anim != NULL) {
		kernel_anim[handle].frame = frame;
		kernel_anim[handle].image = 0;
		kernel_anim[handle].doomed = false;
	}
}

static void kernel_hot_check(int hot, int id, int seg_id) {
	int count;
	int x, y, xs, ys;
	int x1, y1, x2, y2;
	byte scale;

	for (count = 0; count < KERNEL_DYNAMIC_MAX_SEGMENTS; count++) {
		if (seg_id == (int)kernel_dynamic_hot[hot].auto_segment[count]) {

			scale = image_list[id].scale;
			int spriteIndex = (image_list[id].sprite_id & SPRITE_MASK) - 1;

			if (scale == IMAGE_UNSCALED) {
				xs = series_list[image_list[id].series_id]->index[spriteIndex].xs;
				ys = series_list[image_list[id].series_id]->index[spriteIndex].ys;
				x = image_list[id].x;
				y = image_list[id].y;
				x1 = x;
				y1 = y;
				x2 = x + xs - 1;
				y2 = y + ys - 1;
			} else {
				xs = (series_list[image_list[id].series_id]->index[spriteIndex].xs * image_list[id].scale) / 200;
				ys = (series_list[image_list[id].series_id]->index[spriteIndex].ys * image_list[id].scale) / 100;
				x = image_list[id].x;
				y = image_list[id].y;
				x1 = x - xs;
				x2 = x + xs;
				y1 = y - ys;
				y2 = y;
			}

			if ((xs > 0) && (ys > 0)) {
				x1 = MAX(0, x1);
				y1 = MAX(0, y1);
				x2 = MIN(picture_map.total_x_size - 1, x2);
				y2 = MIN(picture_map.total_y_size - 1, y2);
				xs = (x2 - x1) + 1;
				ys = (y2 - y1) + 1;
				if ((xs > 0) && (ys > 0)) {
					kernel_dynamic_hot[hot].x = x1;
					kernel_dynamic_hot[hot].y = y1;
					kernel_dynamic_hot[hot].xs = xs;
					kernel_dynamic_hot[hot].ys = ys;
					kernel_dynamic_hot[hot].valid = true;
				}
			}
		}
	}
}

static void kernel_process_animation(int handle, int asynchronous) {
	int view_changed = false;
	int image_base;
	int count;
	int match;
	int id;
	int clock_frame;
	int hot, seg_id;
	word temp1, temp2;

	if (kernel_anim[handle].anim->misc_any_packed) {
		id = -1;
		match = kernel_anim[handle].anim->series_id[kernel_anim[handle].anim->misc_packed_series];
		for (count = kernel_anim[handle].image;
			(count < kernel_anim[handle].anim->num_images) &&
			(kernel_anim[handle].anim->image[count].flags <= kernel_anim[handle].frame);
			count++) {
			if (kernel_anim[handle].anim->image[count].series_id == (byte)match) {
				id = kernel_anim[handle].anim->image[count].sprite_id;
			}
		}
		if (id >= 0) {
			kernel_animation_get_sprite(handle, id);
		}
	}

	if (kernel.clock < kernel_anim[handle].next_clock) goto done;

	for (count = 0; count < (int)image_marker; count++) {
		if (image_list[count].segment_id == (byte)(KERNEL_SEGMENT_ANIMATION + handle)) {
			image_list[count].flags = IMAGE_ERASE;
		}
	}

	kernel_anim[handle].cycled = false;
	if (kernel_anim[handle].frame >= kernel_anim[handle].anim->num_frames) {
		if (kernel_anim[handle].repeat) {
			kernel_anim[handle].frame = 0;
			kernel_anim[handle].image = 0;
			kernel_anim[handle].cycled = true;
		} else {
			kernel_anim[handle].doomed = true;
			goto done;
		}
	}

	if (!asynchronous) {
		if (kernel_anim[handle].anim->frame[kernel_anim[handle].frame].sound) {
			// pl sound_play(kernel_anim[handle].anim->frame[kernel_anim[handle].frame].sound);
		}

		if ((kernel_anim[handle].anim->misc_peel_x != 0) || (kernel_anim[handle].anim->misc_peel_y != 0)) {
			buffer_peel_horiz(&scr_orig, kernel_anim[handle].anim->misc_peel_x);
			buffer_peel_vert(&scr_orig, kernel_anim[handle].anim->misc_peel_y, NULL, 0);
			matte_refresh_work();
			if (!kernel_allow_peel) error_report(ERROR_PEELING_DISABLED, ERROR, MODULE_KERNEL, handle, 0);
		}

		if (kernel_anim[handle].view_changes) {
			if (kernel_anim[handle].anim->frame[kernel_anim[handle].frame].view_x != (word)picture_view_x) {
				picture_view_x = kernel_anim[handle].anim->frame[kernel_anim[handle].frame].view_x;
				view_changed = true;
			}

			if (kernel_anim[handle].anim->frame[kernel_anim[handle].frame].view_y != (word)picture_view_y) {
				picture_view_y = kernel_anim[handle].anim->frame[kernel_anim[handle].frame].view_y;
				view_changed = true;
			}
		}
	}

	if (view_changed) {
		id = matte_allocate_image();
		image_list[id].segment_id = KERNEL_SEGMENT_SYSTEM;
		image_list[id].flags = IMAGE_REFRESH;
		camera_jump_to(picture_view_x, picture_view_y);
	}

	image_base = image_marker;

	hot = kernel_anim[handle].dynamic_hotspot;
	if (hot >= 0) {
		kernel_dynamic_hot[hot].x = 0;
		kernel_dynamic_hot[hot].y = 0;
		kernel_dynamic_hot[hot].xs = 0;
		kernel_dynamic_hot[hot].ys = 0;
		kernel_dynamic_hot[hot].valid = false;
		kernel_dynamic_changed = true;
	}

	while ((kernel_anim[handle].image < kernel_anim[handle].anim->num_images) &&
		(kernel_anim[handle].anim->image[kernel_anim[handle].image].flags <= kernel_anim[handle].frame)) {
		if (kernel_anim[handle].anim->image[kernel_anim[handle].image].flags == kernel_anim[handle].frame) {
			match = false;
			for (count = 0; !match && (count < image_base); count++) {
				if (image_list[count].segment_id == (byte)(KERNEL_SEGMENT_ANIMATION + handle)) {
					if (memcmp(&image_list[count].series_id,
						&kernel_anim[handle].anim->image[kernel_anim[handle].image].series_id, 9) == 0) {
						image_list[count].flags = 0;

						if (hot >= 0) {
							seg_id = kernel_anim[handle].anim->image[kernel_anim[handle].image].segment_id;
							kernel_hot_check(hot, count, seg_id);
						}

						match = true;
					}
				}
			}

			if (!match) {
				id = matte_allocate_image();
				image_list[id] = kernel_anim[handle].anim->image[kernel_anim[handle].image];

				seg_id = image_list[id].segment_id;

				// image_list[id].segment_id += KERNEL_SEGMENT_ANIMATION;
				image_list[id].segment_id = (byte)(KERNEL_SEGMENT_ANIMATION + handle);
				image_list[id].flags = series_list[image_list[id].series_id]->delta_series ? IMAGE_DELTA : IMAGE_UPDATE;
				// if (kernel_anim[handle].anim->misc_any_packed) {
				// if (image_list[id].series_id == (byte)kernel_anim[handle].anim->series_id[kernel_anim[handle].anim->misc_packed_series]) {
				// series_id = image_list[id].series_id;
				// sprite_data_load (series_list[series_id], image_list[id].sprite_id, series_list[series_id]->arena);
				// }
				// }
				if (hot >= 0) {
					kernel_hot_check(hot, id, seg_id);
				}
			}
		}
		kernel_anim[handle].image++;
	}

	for (count = 0; count < kernel_anim[handle].anim->num_speech; count++) {
		if ((int16)(kernel_anim[handle].anim->speech[count].flags) >= 0) {
			if ((kernel_anim[handle].frame < kernel_anim[handle].anim->speech[count].first_frame) ||
				(kernel_anim[handle].frame > kernel_anim[handle].anim->speech[count].last_frame)) {
				kernel_message_delete(kernel_anim[handle].anim->speech[count].flags);
				kernel_anim[handle].anim->speech[count].flags = (word)-1;
				kernel_anim[handle].messages--;
			}
		} else {
			if ((kernel_anim[handle].frame >= kernel_anim[handle].anim->speech[count].first_frame) &&
				(kernel_anim[handle].frame <= kernel_anim[handle].anim->speech[count].last_frame)) {

				switch (kernel_anim[handle].messages) {
				case 1:
					temp1 = KERNEL_MESSAGE_COLOR_BASE_2;
					break;

				case 2:
					temp1 = KERNEL_MESSAGE_COLOR_BASE;
					break;

				default:
					temp1 = KERNEL_MESSAGE_COLOR_BASE_3;
					break;
				}

				pal_change_color(temp1,
					kernel_anim[handle].anim->speech[count].color[0].r,
					kernel_anim[handle].anim->speech[count].color[0].g,
					kernel_anim[handle].anim->speech[count].color[0].b);

				pal_change_color(temp1 + 1,
					kernel_anim[handle].anim->speech[count].color[1].r,
					kernel_anim[handle].anim->speech[count].color[1].g,
					kernel_anim[handle].anim->speech[count].color[1].b);

				temp2 = ((temp1 + 1) << 8) + temp1;

				kernel_anim[handle].anim->speech[count].flags =
					kernel_message_add(kernel_anim[handle].anim->speech[count].text,
						kernel_anim[handle].anim->speech[count].x,
						kernel_anim[handle].anim->speech[count].y,
						temp2, 9999999, 0, 0);

				kernel_anim[handle].messages++;
			}
		}
	}

	kernel_anim[handle].last_frame = kernel_anim[handle].frame;
	kernel_anim[handle].frame++;

	if (!asynchronous) {
		if (kernel_anim[handle].frame == kernel_anim[handle].anim->num_frames) {
			if (kernel_anim[handle].trigger_code) {
				kernel.trigger = kernel_anim[handle].trigger_code;
				kernel.trigger_mode = kernel_anim[handle].trigger_mode;
				if (kernel.trigger_mode != KERNEL_TRIGGER_DAEMON) {
					for (count = 0; count < 3; count++) {
						player2.words[count] = kernel_anim[handle].trigger_words[count];
					}
				}
			}
		}
	}

	clock_frame = MIN(kernel_anim[handle].frame, kernel_anim[handle].anim->num_frames - 1);
	kernel_anim[handle].next_clock = kernel.clock + kernel_anim[handle].anim->frame[clock_frame].ticks;

done:
	;
}

void kernel_process_all_animations() {
	int count;
	int ok_to_update;

	for (count = 0; count < KERNEL_MAX_ANIMATIONS; count++) {
		if (kernel_anim[count].anim != NULL) {
			ok_to_update = (kernel.fx || !kernel.trigger ||
				(kernel_anim[count].frame != kernel_anim[count].anim->num_frames - 1));
			if (ok_to_update) {
				kernel_process_animation(count, false);
			}
		}
	}
}

static void kernel_reconstruct_screen(int anim_handle) {
	int count;
	int player_found;
	int old_frame;
	long old_clock;

	player_found = false;
	for (count = 0; count < (int)image_marker; count++) {
		if (image_list[count].flags >= 0) {
			if (image_list[count].segment_id == KERNEL_SEGMENT_PLAYER) {
				player_found = true;
			}
		}
	}

	image_marker = 0;
	matte_refresh_work();
	kernel_seq_full_update();

	for (count = 0; count < KERNEL_MAX_ANIMATIONS; count++) {
		if (!kernel_anim[count].doomed) {
			if (kernel_anim[count].anim != NULL) {
				if (count != anim_handle) {
					if (kernel_anim[count].last_frame >= 0) {
						old_frame = kernel_anim[count].frame;
						old_clock = kernel_anim[count].next_clock;
						kernel_anim[count].frame = kernel_anim[count].last_frame;
						kernel_anim[count].image = 0;
						kernel_anim[count].next_clock = kernel.clock;

						kernel_process_animation(count, true);

						kernel_anim[count].next_clock = old_clock;
						kernel_anim[count].frame = old_frame;
						kernel_anim[count].image = 0;
					}
				}
			}
		}
	}

	if (player.walker_visible && player_found) {
		player.sprite_changed = true;
		player_set_image();
	}
}

void kernel_abort_animation(int handle) {
	int count;

	if (kernel_anim[handle].anim != NULL) {
		if (!kernel_anim[handle].doomed) {

			kernel_reconstruct_screen(handle);
		}

		for (count = 0; count < kernel_anim[handle].anim->num_speech; count++) {
			if ((int16)(kernel_anim[handle].anim->speech[count].flags) >= 0) {
				kernel_message_delete(kernel_anim[handle].anim->speech[count].flags);
			}
		}

		if (kernel_anim[handle].dynamic_hotspot >= 0) {
			kernel_delete_dynamic(kernel_anim[handle].dynamic_hotspot);
		}

		kernel_anim[handle].repeat = false;
		anim_unload(kernel_anim[handle].anim);
		kernel_anim[handle].anim = NULL;
	}

	kernel_anim[handle].doomed = false;

	go_ahead_and_frag_the_palette();
}

void kernel_abort_all_animations() {
	int count;

	for (count = KERNEL_MAX_ANIMATIONS - 1; count >= 0; count--) {
		kernel_abort_animation(count);
	}
}

void kernel_doom_all_animations() {
	int count;

	for (count = 0; count < KERNEL_MAX_ANIMATIONS; count++) {
		kernel_anim[count].doomed = true;
	}
}

void kernel_abort_doomed_animations() {
	int count;

	for (count = KERNEL_MAX_ANIMATIONS - 1; count >= 0; count--) {
		if (kernel_anim[count].doomed) {
			kernel_abort_animation(count);
		}
	}
}

void kernel_message_init() {
	int count;

	for (count = 0; count < KERNEL_MAX_MESSAGES; count++) {
		kernel_message[count].flags = 0;
	}

	kernel_message_font = font_conv;
	kernel_message_spacing = -1;
}

int kernel_message_add(char *text, int x, int y, int color,
	long time_on_screen, int trigger_code,
	int flags) {
	int result = -1;
	int id = -1;
	int count;
	KernelMessagePtr my_message = NULL;

	for (count = 0; (id < 0) && (count < KERNEL_MAX_MESSAGES); count++) {
		my_message = &kernel_message[count];
		if (!(my_message->flags & KERNEL_MESSAGE_ACTIVE)) id = count;
	}

	if (id < 0) {
		if (trigger_code) {
			error_report(ERROR_KERNEL_MESSAGE_LIST_FULL, ERROR, MODULE_KERNEL, KERNEL_MAX_MESSAGES, trigger_code);
		}
		goto done;
	}

	my_message->message = text;

	my_message->flags = (KERNEL_MESSAGE_ACTIVE | flags);
	my_message->color = color;
	my_message->x = x;
	my_message->y = y;
	my_message->matte_message_handle = -1;
	my_message->expire_ticks = time_on_screen;
	my_message->update_time = kernel.clock;
	my_message->trigger_code = (byte)trigger_code;
	my_message->trigger_dest = (byte)kernel.trigger_setup_mode;

	for (count = 0; count < 3; count++) {
		my_message->trigger_words[count] = player2.words[count];
	}

	if (flags & KERNEL_MESSAGE_PLAYER) my_message->update_time = player.clock;

	result = id;

done:
	return result;
}

void kernel_message_teletype(int id, int rate, int quote) {
	if (id >= 0) {
		kernel_message[id].flags |= KERNEL_MESSAGE_TELETYPE;
		if (quote) kernel_message[id].flags |= KERNEL_MESSAGE_QUOTE;
		kernel_message[id].strobe_marker = 0;
		kernel_message[id].strobe_rate = rate;
		kernel_message[id].strobe_time = kernel.clock;
		kernel_message[id].strobe_save = *kernel_message[id].message;
		kernel_message[id].strobe_save_2 = *(kernel_message[id].message + 1);
		if (kernel_message[id].flags & KERNEL_MESSAGE_PLAYER) {
			kernel_message[id].strobe_time = player.clock;
		}
		kernel_message[id].update_time = kernel_message[id].strobe_time;
	}
}

void kernel_message_attach(int id, int sequence) {
	if (id >= 0) {
		kernel_message[id].flags |= KERNEL_MESSAGE_ATTACHED;
		kernel_message[id].sequence_id = (byte)sequence;
	}
}

void kernel_message_anim(int id, int anim, int segment) {
	if (id >= 0) {
		kernel_message[id].flags |= KERNEL_MESSAGE_ANIM;
		kernel_message[id].sequence_id = (byte)anim;
		kernel_message[id].segment_id = (byte)segment;
	}
}

void kernel_message_delete(int id) {
	if (kernel_message[id].flags & KERNEL_MESSAGE_ACTIVE) {
		if (kernel_message[id].flags & KERNEL_MESSAGE_TELETYPE) {
			kernel_message[id].message[kernel_message[id].strobe_marker] = kernel_message[id].strobe_save;
			kernel_message[id].message[kernel_message[id].strobe_marker + 1] = kernel_message[id].strobe_save_2;
		}
		if (kernel_message[id].matte_message_handle >= 0) {
			matte_clear_message(kernel_message[id].matte_message_handle);
		}
		kernel_message[id].flags &= ~KERNEL_MESSAGE_ACTIVE;
	}
}

void kernel_message_purge() {
	int count;
	for (count = 0; count < KERNEL_MAX_MESSAGES; count++) {
		kernel_message_delete(count);
	}

	kernel_random_purge();
}

int kernel_message_player(int quote_id, long delay, int trigger) {
	int id;

	id = kernel_message_add(quote_string(kernel.quotes, quote_id),
		0, 0, MESSAGE_COLOR, delay, trigger,
		KERNEL_MESSAGE_PLAYER | KERNEL_MESSAGE_CENTER);

	return id;
}

static void kernel_message_update(KernelMessagePtr my_message) {
	int count;
	int x, y;
	int xx, yy;
	int x2;
	int ys;
	int width;
	int matte_id;
	int segment_id;
	int strobe_flag = false;
	int frame;
	int image = -1;
	Animation *anim = NULL;
	SequencePtr sequence = NULL;
	ImagePtr imgList = NULL;

	if (my_message->flags & KERNEL_MESSAGE_EXPIRED) {
		matte_clear_message(my_message->matte_message_handle);
		my_message->flags &= ~KERNEL_MESSAGE_ACTIVE;
		goto done;
	}

	if (!(my_message->flags & KERNEL_MESSAGE_TELETYPE)) {
		my_message->expire_ticks -= KERNEL_MESSAGE_INTERVAL;
	}

	if (my_message->flags & KERNEL_MESSAGE_ATTACHED) {
		sequence = &sequence_list[my_message->sequence_id];
		if (sequence->expired || !sequence->active_flag) my_message->expire_ticks = 0;
	}

	if (my_message->flags & KERNEL_MESSAGE_ANIM) {
		anim = &kernel_anim[my_message->sequence_id];
		segment_id = my_message->segment_id;
		if (anim->doomed || (anim->anim == NULL)) {
			my_message->expire_ticks = 0;
		} else {
			frame = anim->last_frame;
			imgList = anim->anim->image;
			for (count = 0; (image < 0) && (count < anim->anim->num_images); count++) {
				if (imgList[count].flags == frame) {
					if (imgList[count].segment_id == (byte)segment_id) {
						image = count;
					}
				}
			}
			if (image < 0) my_message->expire_ticks = 0;
		}
	}

	if (my_message->expire_ticks <= 0) {
		if (!kernel.trigger) {
			my_message->flags |= KERNEL_MESSAGE_EXPIRED;
			if (my_message->trigger_code) {
				kernel.trigger = my_message->trigger_code;
				kernel.trigger_mode = my_message->trigger_dest;
				if (kernel.trigger_mode != KERNEL_TRIGGER_DAEMON) {
					for (count = 0; count < 3; count++) {
						player2.words[count] = my_message->trigger_words[count];
					}
				}
			}
		}
	}

	my_message->update_time = kernel.clock + KERNEL_MESSAGE_INTERVAL;

	x = my_message->x;
	y = my_message->y;

	xx = 0;
	yy = 0;

	if (my_message->flags & KERNEL_MESSAGE_ANIM) {
		xx = imgList[image].x - picture_view_x;
		yy = imgList[image].y - picture_view_y;
	}

	if (my_message->flags & KERNEL_MESSAGE_ATTACHED) {
		if (!sequence->auto_locating) {
			xx = sequence->x;
			yy = sequence->y;
		} else {
			xx = series_list[sequence->series_id]->index[sequence->sprite - 1].x;
			yy = series_list[sequence->series_id]->index[sequence->sprite - 1].y;
		}
	}

	if (my_message->flags & KERNEL_MESSAGE_PLAYER) {
		if (player.walker_been_visible) {
			ys = (50 + (series_list[player.series_base + player.series]->index[player.sprite - 1].ys * player.scale)) / 100;
			xx = player.x;
			yy = player.y + ((50 + (player.center_of_gravity * player.scale)) / 100) - ys;
			yy -= 15;
		} else {
			xx = video_x >> 1;
			yy = display_y >> 1;
		}
	}

	xx += x;
	yy += y;

	if (my_message->flags & KERNEL_MESSAGE_TELETYPE) {
		if (kernel.clock >= my_message->strobe_time) {
			my_message->message[my_message->strobe_marker++] = my_message->strobe_save;
			my_message->message[my_message->strobe_marker] = my_message->strobe_save_2;
			my_message->strobe_save = my_message->message[my_message->strobe_marker];
			my_message->strobe_save_2 = my_message->message[my_message->strobe_marker + 1];
			if (!my_message->strobe_save) {
				my_message->message[my_message->strobe_marker] = 0;
				my_message->flags &= ~KERNEL_MESSAGE_TELETYPE;
			} else if (my_message->flags & KERNEL_MESSAGE_QUOTE) {
				my_message->message[my_message->strobe_marker] = '"';
				my_message->message[my_message->strobe_marker + 1] = 0;
			}
			my_message->update_time = my_message->strobe_time = kernel.clock + my_message->strobe_rate;
			strobe_flag = true;
		}
	}

	width = font_string_width(kernel_message_font, my_message->message, kernel_message_spacing);

	if (my_message->flags & (KERNEL_MESSAGE_CENTER | KERNEL_MESSAGE_RIGHT)) {
		if (my_message->flags & KERNEL_MESSAGE_CENTER) {
			xx -= (width >> 1);
		} else {
			xx -= width;
		}
	}

	x2 = xx + width;
	if (x2 > video_x) xx -= (x2 - video_x);

	xx = MAX(0, MIN(video_x - 1, xx));
	yy = MAX(0, MIN(display_y - 1, yy));

	if (my_message->matte_message_handle >= 0) {
		if (strobe_flag ||
			(xx != message_list[my_message->matte_message_handle].x) ||
			(yy != message_list[my_message->matte_message_handle].y)) {
			matte_clear_message(my_message->matte_message_handle);
			my_message->matte_message_handle = -1;
		}
	}

	if (my_message->matte_message_handle < 0) {
		matte_id = matte_add_message(kernel_message_font, my_message->message, xx, yy, my_message->color, kernel_message_spacing);
		if (matte_id < 0) goto done;
		my_message->matte_message_handle = matte_id;
	}

done:
	;
}

void kernel_message_update_all() {
	int count;

	for (count = 0; (count < KERNEL_MAX_MESSAGES) && !kernel.trigger; count++) {
		if (kernel_message[count].flags & KERNEL_MESSAGE_ACTIVE) {
			if (kernel.clock >= kernel_message[count].update_time) {
				kernel_message_update(&kernel_message[count]);
			}
		}
	}
}

void kernel_message_correction(long old_clock, long new_clock) {
	int count;

	for (count = 0; (count < KERNEL_MAX_MESSAGES); count++) {
		if (kernel_message[count].flags & KERNEL_MESSAGE_ACTIVE) {
			kernel_message[count].update_time += (new_clock - old_clock);
		}
	}
}

int kernel_add_dynamic(int vocab_id, int verb_id, byte syntax,
	int auto_sequence,
	int x, int y, int xs, int ys) {
	int id = -1;
	int count;

	for (count = 0; (id < 0) && (count < KERNEL_MAX_DYNAMIC); count++) {
		if (!kernel_dynamic_hot[count].flags) {
			id = count;
		}
	}

	if (id < 0) {
		error_report(ERROR_DYNAMIC_HOTSPOT_OVERFLOW, WARNING, MODULE_KERNEL, id, KERNEL_MAX_DYNAMIC);
		goto done;
	}

	kernel_dynamic_hot[id].flags = true;
	kernel_dynamic_hot[id].vocab_id = vocab_id;
	kernel_dynamic_hot[id].auto_sequence = auto_sequence;
	kernel_dynamic_hot[id].x = x;
	kernel_dynamic_hot[id].y = y;
	kernel_dynamic_hot[id].xs = xs;
	kernel_dynamic_hot[id].ys = ys;

	kernel_dynamic_hot[id].feet_x = WALK_DIRECT_2;
	kernel_dynamic_hot[id].feet_y = 0;
	kernel_dynamic_hot[id].facing = 5;

	kernel_dynamic_hot[id].verb_id = verb_id;
	kernel_dynamic_hot[id].prep = PREP_IN;
	kernel_dynamic_hot[id].syntax = syntax;

	kernel_dynamic_hot[id].cursor = 0;

	kernel_dynamic_hot[id].valid = true;

	kernel_dynamic_hot[id].auto_anim = -1;
	for (count = 0; count < KERNEL_DYNAMIC_MAX_SEGMENTS; count++) {
		kernel_dynamic_hot[id].auto_segment[count] = KERNEL_DYNAMIC_NO_ANIM;
	}


	kernel_num_dynamic++;
	kernel_dynamic_changed = true;

	if (auto_sequence >= 0) {
		sequence_list[auto_sequence].dynamic_hotspot = id;
		kernel_dynamic_hot[id].valid = false;
	}

done:
	return (id);
}

void kernel_dynamic_anim(int id, int anim_id, int segment) {
	int count;
	int found = false;

	if ((id >= 0) && (id < KERNEL_MAX_DYNAMIC)) {
		if (kernel_anim[anim_id].anim != NULL) {
			kernel_anim[anim_id].dynamic_hotspot = id;
			if (kernel_dynamic_hot[id].auto_anim < 0) {
				kernel_dynamic_hot[id].valid = false;
			}
			kernel_dynamic_hot[id].auto_anim = (char)anim_id;
			for (count = 0; !found && (count < KERNEL_DYNAMIC_MAX_SEGMENTS); count++) {
				if (kernel_dynamic_hot[id].auto_segment[count] == KERNEL_DYNAMIC_NO_ANIM) {
					kernel_dynamic_hot[id].auto_segment[count] = (byte)segment;
					found = true;
				}
			}

			if (!found) {
				error_report(ERROR_DYNAMIC_HOTSPOT_OVERFLOW, ERROR, MODULE_KERNEL, -9999, id);
			}

			kernel_dynamic_changed = true;
		}
	}
}

int kernel_dynamic_walk(int id, int feet_x, int feet_y, int facing) {
	if (id >= 0) {
		kernel_dynamic_hot[id].feet_x = feet_x;
		kernel_dynamic_hot[id].feet_y = feet_y;
		kernel_dynamic_hot[id].facing = (byte)facing;
	}
	return (id);
}

int kernel_dynamic_cursor(int id, int cursorNum) {
	if (id >= 0) {
		kernel_dynamic_hot[id].cursor = (byte)cursorNum;
	}

	return id;
}

void kernel_delete_dynamic(int id) {
	if (kernel_dynamic_hot[id].flags) {
		if (kernel_dynamic_hot[id].auto_sequence >= 0) {
			sequence_list[kernel_dynamic_hot[id].auto_sequence].dynamic_hotspot = -1;
		}
		if (kernel_dynamic_hot[id].auto_anim >= 0) {
			kernel_anim[kernel_dynamic_hot[id].auto_anim].dynamic_hotspot = -1;
		}
		kernel_dynamic_hot[id].flags = false;
		kernel_num_dynamic--;
		kernel_dynamic_changed = true;
	}
}

void kernel_purge_dynamic() {
	int count;

	for (count = 0; count < KERNEL_MAX_DYNAMIC; count++) {
		kernel_delete_dynamic(count);
	}
	kernel_num_dynamic = 0;
	kernel_dynamic_changed = true;
}

void kernel_init_dynamic() {
	int count;

	for (count = 0; count < KERNEL_MAX_DYNAMIC; count++) {
		kernel_dynamic_hot[count].flags = false;
	}
	kernel_num_dynamic = 0;
	kernel_dynamic_changed = 0;
}

int  kernel_dynamic_consecutive(int id) {
	int scan;

	for (scan = 0; (id >= 0) && (scan < KERNEL_MAX_DYNAMIC); scan++) {
		if (kernel_dynamic_hot[scan].flags && kernel_dynamic_hot[scan].valid) {
			id--;
			if (id < 0) goto done;
		}
	}

	if (id >= 0) scan = -1;

done:
	return (scan);
}

void kernel_refresh_dynamic() {
	int count;

	numspots = inter_base_hotspots;

	for (count = 0; count < KERNEL_MAX_DYNAMIC; count++) {
		if (kernel_dynamic_hot[count].flags && kernel_dynamic_hot[count].valid && ((inter_input_mode == INTER_BUILDING_SENTENCES) || (inter_input_mode == INTER_LIMITED_SENTENCES))) {
			hspot_add(kernel_dynamic_hot[count].x, kernel_dynamic_hot[count].y,
				kernel_dynamic_hot[count].x + kernel_dynamic_hot[count].xs - 1,
				kernel_dynamic_hot[count].y + kernel_dynamic_hot[count].ys - 1,
				STROKE_DYNAMIC | STROKE_INTERFACE, kernel_dynamic_hot[count].vocab_id,
				RELATIVE_MODE);
			inter_force_rescan = true;
		}
	}

	kernel_dynamic_changed = false;
}

char *kernel_full_name(int my_room, char type, int num, char *text, int ext) {
	char temp[2];

	if (my_room > 0) {
		if (my_room >= 100) {
			Common::strcpy_s(kernel_work_name, "*RM");
		} else {
			Common::strcpy_s(kernel_work_name, "*SC");
		}
		env_catint(kernel_work_name, my_room, 3);
	} else {
		Common::strcpy_s(kernel_work_name, "*");
	}

	temp[0] = type;
	temp[1] = 0;
	Common::strcat_s(kernel_work_name, temp);

	if ((num >= 0) && (ext < KERNEL_TT)) {
		if (num > 9) {
			env_catint(kernel_work_name, num, 2);
		} else {
			env_catint(kernel_work_name, num, 1);
		}
	}

	if (text != NULL)
		Common::strcat_s(kernel_work_name, text);

	switch (ext) {
	case KERNEL_SS:
		Common::strcat_s(kernel_work_name, ".SS");
		break;
	case KERNEL_AA:
		Common::strcat_s(kernel_work_name, ".AA");
		break;
	case KERNEL_DAT:
		Common::strcat_s(kernel_work_name, ".DAT");
		break;
	case KERNEL_HH:
		Common::strcat_s(kernel_work_name, ".HH");
		break;
	case KERNEL_ART:
		Common::strcat_s(kernel_work_name, ".ART");
		break;
	case KERNEL_INT:
		Common::strcat_s(kernel_work_name, ".INT");
		break;
	case KERNEL_TT:
		Common::strcat_s(kernel_work_name, ".TT");
		break;
	case KERNEL_MM:
		Common::strcat_s(kernel_work_name, ".MM");
		break;
	case KERNEL_WW:
		Common::strcat_s(kernel_work_name, ".WW");
		break;
	}

	if ((num >= 0) && (ext >= KERNEL_TT)) {
		temp[0] = (byte)num;
		temp[1] = 0;
		Common::strcat_s(kernel_work_name, temp);
	}

	return kernel_work_name;
}

char *kernel_name(char type, int num) {
	return kernel_full_name(room_id, type, num, NULL, KERNEL_NONE);
}

char *kernel_interface_name(int num) {
	return kernel_full_name(0, 'I', num, NULL, KERNEL_AA);
}

void kernel_unload_sound_driver() {
}

int kernel_load_sound_driver(const char *name, char sound_card_, int sound_board_address_, int sound_board_type_, int sound_board_irq_) {
	// Get the section number from the end of the driver filename, and use it to initialize
	// the sound system; we provide our own implementation of the drivers
	int sectionNum = *(name + strlen(name) - 1) - '0';
	assert((sectionNum >= 1 && sectionNum <= 5) || sectionNum == 9);

	g_engine->_soundManager->init(sectionNum);

	return 0;
}

void kernel_load_variant(int variant) {
	room_variant = variant;

	if (room_load_variant(room_id, room_variant, NULL, room,
		&scr_depth,
		&scr_walk,
		&scr_special,
		&depth_map,
		&depth_resource,
		-1)) {
		error_report(ERROR_VARIANT_LOAD_FAILURE, WARNING, MODULE_KERNEL, room_load_error, (room_id * 10) + room_variant);
	}

	rail_connect_all_nodes();

	camera_jump_to(picture_view_x, picture_view_y);
}

void kernel_new_palette() {
	int palette_base, palette_size;

	palette_base = KERNEL_RESERVED_LOW_COLORS;
	if (cycling_active) {
		if (cycle_list.num_cycles) {
			palette_base = cycle_list.table[0].first_palette_color + total_cycle_colors;
		}
	}
	palette_size = 256 - palette_base;

	mcga_setpal_range(&master_palette, palette_base, palette_size);
}

void kernel_dump_quotes() {
	if (kernel.quotes != NULL) {
		mem_free(kernel.quotes);
		kernel.quotes = NULL;
	}
}

void kernel_dump_all() {
	kernel_dump_quotes();
	kernel_unload_all_series();
	kernel_seq_init();
	image_marker = 0;
	matte_refresh_work();
}

void kernel_dump_walker_only() {
	int count;
	int marker;

	marker = player.series_base;

	for (count = 0; count < 8; count++) {
		if (player.available[count]) {
			sprite_free(&series_list[marker++], true);
			player.available[count] = false;
		}
	}

	image_marker = 0;
	matte_refresh_work();

	player.walker_visible = false;
}

void kernel_random_purge() {
	int count;

	for (count = 0; count < KERNEL_MAX_RANDOM_MESSAGES; count++) {
		random_message_handle[count] = -1;
		random_message_quote[count] = -1;
	}
}

void kernel_random_messages_init(int max_messages_at_once,
		int min_x, int max_x, int min_y, int max_y, int min_y_spacing,
		int teletype_rate, int color, int duration, int quote_id, ...) {
	va_list marker;
	int my_quote = quote_id;

	random_max_messages = max_messages_at_once;
	random_min_x = min_x;
	random_max_x = max_x;
	random_min_y = min_y;
	random_max_y = max_y;
	random_spacing = min_y_spacing;
	random_teletype_rate = teletype_rate;
	random_message_color = color;
	random_message_duration = duration;

	random_quote_list_size = 0;

	va_start(marker, quote_id);
	while (my_quote > 0) {
		if (random_quote_list_size < KERNEL_MAX_RANDOM_QUOTES) {
			random_quote_list[random_quote_list_size++] = my_quote;
		}

		my_quote = va_arg(marker, int);
	}

	kernel_random_purge();
}

int kernel_check_random() {
	int count;
	int sum = 0;

	for (count = 0; count < random_max_messages; count++) {
		if (random_message_handle[count] >= 0) sum++;
	}

	return sum;
}

void kernel_random_message_server() {
	if ((kernel.trigger >= KERNEL_RANDOM_MESSAGE_TRIGGER) &&
		(kernel.trigger < KERNEL_RANDOM_MESSAGE_TRIGGER + random_max_messages)) {
		random_message_handle[kernel.trigger - KERNEL_RANDOM_MESSAGE_TRIGGER] = -1;
		random_message_quote[kernel.trigger - KERNEL_RANDOM_MESSAGE_TRIGGER] = -1;
	}
}

int kernel_generate_random_message(int chance_major, int chance_minor) {
	int count, count2, scan;
	int bad;
	int generated_one;
	int idx, quote;
	int last_y;
	int message_x, message_y;
	int crash_timeout = 0;

	generated_one = false;

	for (count = 0; count < random_max_messages; count++) {
		if (random_message_handle[count] < 0) {

			// Don't allow two phrases to teletype at once
			bad = false;
			for (scan = 0; scan < random_max_messages; scan++) {
				if (random_message_handle[scan] >= 0) {
					if (kernel_message[random_message_handle[scan]].flags & KERNEL_MESSAGE_TELETYPE) {
						bad = true;
					}
				}
			}

			// Check random chance for message to appear
			if ((imath_random(1, chance_major) <= chance_minor) && !bad) {

				// Pick randomly from our list of allowable quotes
				do {
					idx = imath_random(0, random_quote_list_size - 1);
					quote = random_quote_list[idx];
					bad = false;
					for (scan = 0; scan < random_max_messages; scan++) {
						if (quote == random_message_quote[scan]) {
							bad = true;
						}
					}
				} while (bad);

				random_message_quote[count] = quote;

				// Put message in a random location
				message_x = imath_random(random_min_x, random_max_x);

				// Be sure Y values are properly spaced
				crash_timeout = 0;

				do {
					if (crash_timeout++ > 100) goto done;
					bad = false;
					message_y = imath_random(random_min_y, random_max_y);
					for (count2 = 0; count2 < random_max_messages; count2++) {
						if (random_message_handle[count2] >= 0) {
							last_y = kernel_message[random_message_handle[count2]].y;
							if ((message_y >= (last_y - random_spacing)) &&
								(message_y <= (last_y + random_spacing))) {
								bad = true;
							}
						}
					}
				} while (bad);

				// Put our new message in the list
				random_message_handle[count] =
					kernel_message_add(quote_string(kernel.quotes, random_message_quote[count]),
						message_x, message_y, random_message_color, random_message_duration,
						KERNEL_RANDOM_MESSAGE_TRIGGER + count, 0);
				if (random_teletype_rate > 0) {
					if (random_message_handle[count] >= 0) {
						kernel_message_teletype(random_message_handle[count], random_teletype_rate, true);
					}
				}

				generated_one = true;
			}
		}
	}

done:
	return generated_one;
}

void kernel_set_interface_mode(int mode) {
	if (mode != inter_input_mode) {
		char fname[80];
		Common::strcpy_s(fname, kernel.interface);
		char *dot = strchr(fname, '.');

		if (dot) {
			*dot = '\0';
			if (mode != INTER_BUILDING_SENTENCES)
				Common::strcat_s(fname, "A");
			Common::strcat_s(fname, ".INT");

			buffer_free(&scr_inter_orig);
			inter_load_background(fname, &scr_inter_orig);
		}
	}

	inter_input_mode = mode;

	image_inter_marker = 1;
	image_inter_list[0].flags = IMAGE_REFRESH;
	image_inter_list[0].segment_id = (byte)-1;

	// Set up interface animation clock
	inter_base_time = timer_read();

	left_command = -1;
	left_action = -1;
	left_inven = -1;

	// Initialize interface work area
	if (!viewing_at_y) {
		buffer_rect_copy(scr_inter_orig, scr_inter, 0, 0, video_x, inter_size_y);

		// Initialize interface grammar driver
		if (kernel_mode == KERNEL_ACTIVE_CODE) matte_inter_frame(false, false);
	}

	inter_init_sentence();
	inter_setup_hotspots();

	if (!viewing_at_y)
		inter_prepare_background();

	kernel_refresh_dynamic();
}

void kernel_room_scale(int front_y, int front_scale,
	int back_y, int back_scale) {
	room->front_y = front_y;
	room->front_scale = front_scale;
	room->back_y = back_y;
	room->back_scale = back_scale;

	kernel_room_bound_dif = room->front_y - room->back_y;
	kernel_room_scale_dif = room->front_scale - room->back_scale;
}

void kernel_background_shutdown() {
	// Remove our palette shadowing list
	pal_activate_shadow(NULL);

	// Dump the picture & attribute buffers, along with the room header
	if (room != NULL) {
		room_unload(room,
			&scr_orig,
			&scr_depth,
			&scr_walk,
			&scr_special,
			&picture_map,
			&depth_map);
		room = NULL;
	}
}

int kernel_background_startup(int newRoom, int initial_variant) {
	int error_flag = true;
	int load_flags;
	int error_code = 0;
	int error_data = 0;

	// Make a note of the new room number & variant
	previous_room = room_id;
	room_id = newRoom;
	room_variant = initial_variant;

	// Start a brand new palette, reserving the proper # of colors
	pal_init(KERNEL_RESERVED_LOW_COLORS, KERNEL_RESERVED_HIGH_COLORS);
	pal_white(master_palette);

	// Initialize the matteing system
	matte_init(false);

	// Initialize graphics sequence data structures
	kernel_seq_init();
	kernel_message_init();

	// Activate the main shadow list
	pal_activate_shadow(&kernel_shadow_main);

	// Load header, picture, and attribute screen for this room
	load_flags = ROOM_LOAD_HARD_SHADOW;
	if (kernel.translating) load_flags |= ROOM_LOAD_TRANSLATE;

	room = room_load(room_id, room_variant, NULL,
		&scr_orig,
		&scr_depth,
		&scr_walk,
		&scr_special,
		&picture_map,
		&depth_map,
		&picture_resource,
		&depth_resource,
		-1,
		-1,
		load_flags);
	if (room == NULL) {
		error_data = room_load_error;
		error_code = ERROR_KERNEL_NO_ROOM;
		goto done;
	}

	tile_pan(&picture_map, picture_view_x, picture_view_y);
	tile_pan(&depth_map, picture_view_x, picture_view_y);

	// Set up color cycling table for this room
	cycle_init(&room->cycle_list, false);

	// Initialize the graphics image lists
	image_marker = 1;
	image_list[0].flags = IMAGE_REFRESH;
	image_list[0].segment_id = KERNEL_SEGMENT_SYSTEM;

	// Mark the boundary between interface and room sprite series
	kernel_room_series_marker = series_list_marker;

	error_flag = false;

done:
	if (error_flag) {
		error_check_memory();
		error_report(error_code, ERROR, MODULE_KERNEL, room_id, error_data);
		kernel_background_shutdown();
	}

	return error_flag;
}

void init_kernel() {
	memset(room_state, 0, sizeof(room_state));
	video_mode = 0;
	room = NULL;
	room_id = KERNEL_STARTING_GAME;
	section_id = KERNEL_STARTING_GAME;
	room_variant = 0;
	new_room = 101;
	new_section = 1;
	previous_room = 0;
	previous_section = 0;
	kernel_initial_variant = 0;
	room_spots = NULL;
	room_num_spots = 0;
	kernel_room_series_marker = 0;
	kernel_room_bound_dif = 0;
	kernel_room_scale_dif = 0;
	kernel_allow_peel = false;
	kernel_panning_speed = 0;
	kernel_screen_fade = 0;
	memset(kernel_anim, 0, sizeof(kernel_anim));
	memset(&kernel_shadow_main, 0, sizeof(ShadowList));
	memset(&kernel_shadow_inter, 0, sizeof(ShadowList));
	kernel_shadow_inter.num_shadow_colors = 1;
	kernel_shadow_inter.shadow_color[0] = 15;
	kernel_ok_to_fail_load = false;
	kernel_mode = KERNEL_GAME_LOAD;
	memset(kernel_cheating_password, 0, sizeof(kernel_cheating_password));
	kernel_cheating_allowed = 0;
	kernel_cheating_forbidden = 0;
	memset(kernel_dynamic_hot, 0, sizeof(kernel_dynamic_hot));
	kernel_num_dynamic = 0;
	kernel_dynamic_changed = 0;
	cursor = NULL;
	cursor_id = 1;
	cursor_last = -1;
	memset(&kernel, 0, sizeof(Kernel));
	memset(&game, 0, sizeof(KernelGame));
	memset(sequence_list, 0, sizeof(sequence_list));
	stop_speech_on_run_anim = true;
	memset(kernel_message, 0, sizeof(kernel_message));
	kernel_message_font = NULL;
	kernel_message_spacing = 0;
	kernel_sound_handle = 0;
	memset(random_message_handle, 0, sizeof(random_message_handle));
	memset(random_message_quote, 0, sizeof(random_message_quote));
	random_max_messages = 1;
	memset(random_quote_list, 0, sizeof(random_quote_list));
	random_quote_list_size = 0;
	random_min_x = 0;
	random_max_x = video_x;
	random_min_y = 0;
	random_max_y = display_y;
	random_spacing = 0;
	random_teletype_rate = 0;
	random_message_color = 0;
	random_message_duration = 0;
	memset(kernel_interface_loaded, 0, sizeof(kernel_interface_loaded));
}

} // namespace MADSV2
} // namespace MADS
